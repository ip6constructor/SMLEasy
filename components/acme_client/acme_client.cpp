#include "acme_client.hpp"

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mbedtls/base64.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/md.h"
#include "mbedtls/oid.h"
#include "mbedtls/pk.h"
#include "mbedtls/sha256.h"
#include "mbedtls/x509_csr.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <strings.h>
#include <string>
#include <vector>

static const char* TAG = "AcmeClient";
static SemaphoreHandle_t s_challenge_mutex = nullptr;
static std::string s_challenge_token;
static std::string s_challenge_response;

namespace acme_client {
namespace {

static constexpr char kProductionDirectory[] = "https://acme-v02.api.letsencrypt.org/directory";

struct HttpResponse {
    std::string body;
    std::string nonce;
    std::string location;
    int status{0};
};

struct CryptoContext {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng;
    mbedtls_pk_context account_key;
    bool initialized{false};

    CryptoContext() {
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&rng);
        mbedtls_pk_init(&account_key);
        const char* personalization = "SMLEasy ACME account";
        initialized = mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy,
            reinterpret_cast<const unsigned char*>(personalization), strlen(personalization)) == 0;
    }
    ~CryptoContext() {
        mbedtls_pk_free(&account_key);
        mbedtls_ctr_drbg_free(&rng);
        mbedtls_entropy_free(&entropy);
    }
};

static esp_err_t http_event(esp_http_client_event_t* event)
{
    auto* response = static_cast<HttpResponse*>(event->user_data);
    if (!response) return ESP_OK;
    if (event->event_id == HTTP_EVENT_ON_HEADER && event->header_key && event->header_value) {
        if (strcasecmp(event->header_key, "Replay-Nonce") == 0)
            response->nonce = event->header_value;
        else if (strcasecmp(event->header_key, "Location") == 0)
            response->location = event->header_value;
    } else if (event->event_id == HTTP_EVENT_ON_DATA && event->data && event->data_len > 0) {
        response->body.append(static_cast<const char*>(event->data), event->data_len);
    }
    return ESP_OK;
}

static esp_err_t http_request(const std::string& url, esp_http_client_method_t method,
                              const std::string& body, const char* accept,
                              HttpResponse& response)
{
    esp_http_client_config_t config{};
    config.url = url.c_str();
    config.method = method;
    config.timeout_ms = 30000;
    config.buffer_size = 4096;
    config.event_handler = http_event;
    config.user_data = &response;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return ESP_ERR_NO_MEM;
    if (accept) esp_http_client_set_header(client, "Accept", accept);
    if (method == HTTP_METHOD_POST) {
        esp_http_client_set_header(client, "Content-Type", "application/jose+json");
        esp_http_client_set_post_field(client, body.data(), static_cast<int>(body.size()));
    }
    const esp_err_t result = esp_http_client_perform(client);
    response.status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    return result;
}

static std::string base64url(const unsigned char* data, size_t size)
{
    if (size == 0) return {};
    std::string encoded(((size + 2) / 3) * 4 + 1, '\0');
    size_t output_size = 0;
    if (mbedtls_base64_encode(reinterpret_cast<unsigned char*>(&encoded[0]), encoded.size(),
                              &output_size, data, size) != 0) return {};
    encoded.resize(output_size);
    for (char& ch : encoded) {
        if (ch == '+') ch = '-';
        else if (ch == '/') ch = '_';
    }
    while (!encoded.empty() && encoded.back() == '=') encoded.pop_back();
    return encoded;
}

static std::string json_string(const cJSON* json, const char* name)
{
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(json, name);
    return cJSON_IsString(item) && item->valuestring ? item->valuestring : "";
}

static std::string print_json(cJSON* json)
{
    char* text = cJSON_PrintUnformatted(json);
    std::string result(text ? text : "");
    cJSON_free(text);
    return result;
}

static bool prepare_account_key(CryptoContext& crypto, const std::string& stored_pem,
                                std::string& output_pem)
{
    if (!crypto.initialized) return false;
    if (!stored_pem.empty()) {
        if (mbedtls_pk_parse_key(&crypto.account_key,
            reinterpret_cast<const unsigned char*>(stored_pem.c_str()), stored_pem.size() + 1,
            nullptr, 0) != 0) return false;
        output_pem = stored_pem;
        return true;
    }
    if (mbedtls_pk_setup(&crypto.account_key,
        mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) != 0) return false;
    if (mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, mbedtls_pk_ec(crypto.account_key),
        mbedtls_ctr_drbg_random, &crypto.rng) != 0) return false;
    std::vector<unsigned char> pem(2048, 0);
    if (mbedtls_pk_write_key_pem(&crypto.account_key, pem.data(), pem.size()) != 0) return false;
    output_pem = reinterpret_cast<const char*>(pem.data());
    return true;
}

static bool make_jwk(mbedtls_pk_context& key, std::string& jwk)
{
    const auto* ec = mbedtls_pk_ec(key);
    if (!ec || ec->grp.id != MBEDTLS_ECP_DP_SECP256R1) return false;
    std::array<unsigned char, 32> x{}, y{};
    if (mbedtls_mpi_write_binary(&ec->Q.X, x.data(), x.size()) != 0 ||
        mbedtls_mpi_write_binary(&ec->Q.Y, y.data(), y.size()) != 0) return false;
    jwk = "{\"crv\":\"P-256\",\"kty\":\"EC\",\"x\":\"" +
          base64url(x.data(), x.size()) + "\",\"y\":\"" +
          base64url(y.data(), y.size()) + "\"}";
    return true;
}

static bool make_jwk_thumbprint(const std::string& jwk, std::string& thumbprint)
{
    std::array<unsigned char, 32> hash{};
    if (mbedtls_sha256_ret(reinterpret_cast<const unsigned char*>(jwk.data()), jwk.size(),
                           hash.data(), 0) != 0) return false;
    thumbprint = base64url(hash.data(), hash.size());
    return !thumbprint.empty();
}

static bool sign_es256(CryptoContext& crypto, const std::string& input, std::string& signature)
{
    auto* ec = mbedtls_pk_ec(crypto.account_key);
    if (!ec || ec->grp.id != MBEDTLS_ECP_DP_SECP256R1) return false;
    std::array<unsigned char, 32> hash{};
    if (mbedtls_sha256_ret(reinterpret_cast<const unsigned char*>(input.data()), input.size(),
                           hash.data(), 0) != 0) return false;
    mbedtls_mpi r, s;
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    const int result = mbedtls_ecdsa_sign(&ec->grp, &r, &s, &ec->d, hash.data(), hash.size(),
        mbedtls_ctr_drbg_random, &crypto.rng);
    std::array<unsigned char, 64> raw{};
    const bool encoded = result == 0 &&
        mbedtls_mpi_write_binary(&r, raw.data(), 32) == 0 &&
        mbedtls_mpi_write_binary(&s, raw.data() + 32, 32) == 0;
    if (encoded) signature = base64url(raw.data(), raw.size());
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    return encoded && !signature.empty();
}

static bool make_jws(CryptoContext& crypto, const std::string& jwk, const std::string& kid,
                     const std::string& nonce, const std::string& url,
                     const std::string& payload, std::string& jws)
{
    cJSON* protected_json = cJSON_CreateObject();
    cJSON_AddStringToObject(protected_json, "alg", "ES256");
    cJSON_AddStringToObject(protected_json, "nonce", nonce.c_str());
    cJSON_AddStringToObject(protected_json, "url", url.c_str());
    if (!kid.empty()) cJSON_AddStringToObject(protected_json, "kid", kid.c_str());
    else cJSON_AddItemToObject(protected_json, "jwk", cJSON_Parse(jwk.c_str()));
    const std::string protected_json_text = print_json(protected_json);
    cJSON_Delete(protected_json);
    const std::string protected64 = base64url(
        reinterpret_cast<const unsigned char*>(protected_json_text.data()), protected_json_text.size());
    const std::string payload64 = base64url(
        reinterpret_cast<const unsigned char*>(payload.data()), payload.size());
    if (protected64.empty()) return false;
    const std::string signing_input = protected64 + "." + payload64;
    std::string signature;
    if (!sign_es256(crypto, signing_input, signature)) return false;
    cJSON* outer = cJSON_CreateObject();
    cJSON_AddStringToObject(outer, "protected", protected64.c_str());
    cJSON_AddStringToObject(outer, "payload", payload64.c_str());
    cJSON_AddStringToObject(outer, "signature", signature.c_str());
    jws = print_json(outer);
    cJSON_Delete(outer);
    return !jws.empty();
}

static bool send_signed(CryptoContext& crypto, const std::string& jwk, const std::string& kid,
                        std::string& nonce, const std::string& url, const std::string& payload,
                        const char* accept, HttpResponse& reply)
{
    if (nonce.empty()) return false;
    std::string jws;
    if (!make_jws(crypto, jwk, kid, nonce, url, payload, jws)) return false;
    if (http_request(url, HTTP_METHOD_POST, jws, accept, reply) != ESP_OK) return false;
    if (!reply.nonce.empty()) nonce = reply.nonce;
    if (reply.status < 200 || reply.status >= 300) {
        ESP_LOGE(TAG, "ACME request failed with HTTP %d", reply.status);
        return false;
    }
    return true;
}

static bool get_nonce(const std::string& url, std::string& nonce)
{
    HttpResponse reply;
    if (http_request(url, HTTP_METHOD_HEAD, "", nullptr, reply) != ESP_OK ||
        reply.status < 200 || reply.status >= 300 || reply.nonce.empty()) return false;
    nonce = reply.nonce;
    return true;
}

static bool generate_csr(CryptoContext& crypto, const std::string& fqdn,
                         std::string& private_key_pem, std::string& csr_der_b64)
{
    mbedtls_pk_context certificate_key;
    mbedtls_pk_init(&certificate_key);
    int result = mbedtls_pk_setup(&certificate_key,
        mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (result == 0) result = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1,
        mbedtls_pk_ec(certificate_key), mbedtls_ctr_drbg_random, &crypto.rng);

    mbedtls_x509write_csr csr;
    mbedtls_x509write_csr_init(&csr);
    mbedtls_x509write_csr_set_key(&csr, &certificate_key);
    mbedtls_x509write_csr_set_md_alg(&csr, MBEDTLS_MD_SHA256);
    const std::string subject = "CN=" + fqdn;
    if (result == 0) result = mbedtls_x509write_csr_set_subject_name(&csr, subject.c_str());

    std::vector<uint8_t> dns_name;
    dns_name.push_back(0x82);
    if (fqdn.size() < 128) dns_name.push_back(static_cast<uint8_t>(fqdn.size()));
    else {
        dns_name.push_back(0x81);
        dns_name.push_back(static_cast<uint8_t>(fqdn.size()));
    }
    dns_name.insert(dns_name.end(), fqdn.begin(), fqdn.end());
    std::vector<uint8_t> san_value;
    san_value.push_back(0x30);
    if (dns_name.size() < 128) san_value.push_back(static_cast<uint8_t>(dns_name.size()));
    else {
        san_value.push_back(0x81);
        san_value.push_back(static_cast<uint8_t>(dns_name.size()));
    }
    san_value.insert(san_value.end(), dns_name.begin(), dns_name.end());
    if (result == 0) result = mbedtls_x509write_csr_set_extension(&csr,
        MBEDTLS_OID_SUBJECT_ALT_NAME, MBEDTLS_OID_SIZE(MBEDTLS_OID_SUBJECT_ALT_NAME),
        san_value.data(), san_value.size());

    std::vector<unsigned char> csr_buffer(4096, 0);
    if (result == 0) {
        const int der_length = mbedtls_x509write_csr_der(&csr, csr_buffer.data(), csr_buffer.size(),
            mbedtls_ctr_drbg_random, &crypto.rng);
        if (der_length < 0) result = der_length;
        else csr_der_b64 = base64url(csr_buffer.data() + csr_buffer.size() - der_length,
                                     static_cast<size_t>(der_length));
    }
    std::vector<unsigned char> key_buffer(2048, 0);
    if (result == 0) result = mbedtls_pk_write_key_pem(&certificate_key, key_buffer.data(), key_buffer.size());
    if (result == 0) private_key_pem = reinterpret_cast<const char*>(key_buffer.data());

    mbedtls_x509write_csr_free(&csr);
    mbedtls_pk_free(&certificate_key);
    return result == 0 && !csr_der_b64.empty() && !private_key_pem.empty();
}

static void publish_challenge(const std::string& token, const std::string& response)
{
    if (!s_challenge_mutex) return;
    xSemaphoreTake(s_challenge_mutex, portMAX_DELAY);
    s_challenge_token = token;
    s_challenge_response = response;
    xSemaphoreGive(s_challenge_mutex);
}

} // namespace

esp_err_t init()
{
    if (s_challenge_mutex) return ESP_OK;
    s_challenge_mutex = xSemaphoreCreateMutex();
    return s_challenge_mutex ? ESP_OK : ESP_ERR_NO_MEM;
}

void clear_http01_challenge()
{
    if (!s_challenge_mutex) return;
    xSemaphoreTake(s_challenge_mutex, portMAX_DELAY);
    s_challenge_token.clear();
    s_challenge_response.clear();
    xSemaphoreGive(s_challenge_mutex);
}

bool get_http01_challenge(const char* token, std::string& response)
{
    if (!token || !s_challenge_mutex) return false;
    xSemaphoreTake(s_challenge_mutex, portMAX_DELAY);
    const bool matches = s_challenge_token == token && !s_challenge_response.empty();
    if (matches) response = s_challenge_response;
    xSemaphoreGive(s_challenge_mutex);
    return matches;
}

esp_err_t issue_http01(const Request& request, Result& result)
{
    if (init() != ESP_OK || request.fqdn.empty() || request.email.empty())
        return ESP_ERR_INVALID_ARG;

    CryptoContext crypto;
    if (!prepare_account_key(crypto, request.account_key_pem, result.account_key_pem))
        return ESP_FAIL;
    std::string jwk, thumbprint;
    if (!make_jwk(crypto.account_key, jwk) || !make_jwk_thumbprint(jwk, thumbprint))
        return ESP_FAIL;

    const std::string directory_url = request.directory_url.empty()
        ? kProductionDirectory : request.directory_url;
    if (directory_url.rfind("https://", 0) != 0) return ESP_ERR_INVALID_ARG;
    HttpResponse reply;
    if (http_request(directory_url, HTTP_METHOD_GET, "", "application/json", reply) != ESP_OK ||
        reply.status != 200) return ESP_FAIL;
    cJSON* directory = cJSON_Parse(reply.body.c_str());
    if (!directory) return ESP_FAIL;
    const std::string new_nonce_url = json_string(directory, "newNonce");
    const std::string new_account_url = json_string(directory, "newAccount");
    const std::string new_order_url = json_string(directory, "newOrder");
    cJSON_Delete(directory);
    if (new_nonce_url.empty() || new_account_url.empty() || new_order_url.empty()) return ESP_FAIL;

    std::string nonce;
    if (!get_nonce(new_nonce_url, nonce)) return ESP_FAIL;
    std::string account_url = request.account_url;
    if (account_url.empty()) {
        cJSON* account_payload = cJSON_CreateObject();
        cJSON* contacts = cJSON_CreateArray();
        const std::string contact = "mailto:" + request.email;
        cJSON_AddItemToArray(contacts, cJSON_CreateString(contact.c_str()));
        cJSON_AddItemToObject(account_payload, "contact", contacts);
        cJSON_AddBoolToObject(account_payload, "termsOfServiceAgreed", true);
        const std::string payload = print_json(account_payload);
        cJSON_Delete(account_payload);
        if (!send_signed(crypto, jwk, "", nonce, new_account_url, payload,
                         "application/json", reply) || reply.location.empty()) return ESP_FAIL;
        account_url = reply.location;
    }
    if (account_url.rfind("https://", 0) != 0) return ESP_FAIL;
    result.account_url = account_url;

    cJSON* order_payload = cJSON_CreateObject();
    cJSON* identifiers = cJSON_CreateArray();
    cJSON* identifier = cJSON_CreateObject();
    cJSON_AddStringToObject(identifier, "type", "dns");
    cJSON_AddStringToObject(identifier, "value", request.fqdn.c_str());
    cJSON_AddItemToArray(identifiers, identifier);
    cJSON_AddItemToObject(order_payload, "identifiers", identifiers);
    const std::string order_body = print_json(order_payload);
    cJSON_Delete(order_payload);
    if (!send_signed(crypto, jwk, account_url, nonce, new_order_url, order_body,
                     "application/json", reply) || reply.location.empty()) return ESP_FAIL;
    const std::string order_url = reply.location;

    cJSON* order = cJSON_Parse(reply.body.c_str());
    if (!order) return ESP_FAIL;
    const cJSON* authorizations = cJSON_GetObjectItemCaseSensitive(order, "authorizations");
    const cJSON* first_authorization = cJSON_IsArray(authorizations)
        ? cJSON_GetArrayItem(authorizations, 0) : nullptr;
    const std::string authorization_url = cJSON_IsString(first_authorization)
        ? first_authorization->valuestring : "";
    const std::string finalize_url = json_string(order, "finalize");
    cJSON_Delete(order);
    if (authorization_url.empty() || finalize_url.empty()) return ESP_FAIL;

    bool authorization_valid = false;
    if (!send_signed(crypto, jwk, account_url, nonce, authorization_url, "",
                     "application/json", reply)) return ESP_FAIL;
    cJSON* authorization = cJSON_Parse(reply.body.c_str());
    if (!authorization) return ESP_FAIL;
    authorization_valid = json_string(authorization, "status") == "valid";
    if (!authorization_valid) {
        const cJSON* challenges = cJSON_GetObjectItemCaseSensitive(authorization, "challenges");
        std::string challenge_url, challenge_token;
        if (cJSON_IsArray(challenges)) {
            const int count = cJSON_GetArraySize(challenges);
            for (int i = 0; i < count; ++i) {
                const cJSON* challenge = cJSON_GetArrayItem(challenges, i);
                if (json_string(challenge, "type") == "http-01") {
                    challenge_url = json_string(challenge, "url");
                    challenge_token = json_string(challenge, "token");
                    break;
                }
            }
        }
        cJSON_Delete(authorization);
        if (challenge_url.empty() || challenge_token.empty()) return ESP_FAIL;
        publish_challenge(challenge_token, challenge_token + "." + thumbprint);
        const bool challenge_started = send_signed(crypto, jwk, account_url, nonce,
            challenge_url, "{}", "application/json", reply);
        if (!challenge_started) {
            clear_http01_challenge();
            return ESP_FAIL;
        }
        for (int attempt = 0; attempt < 60; ++attempt) {
            vTaskDelay(pdMS_TO_TICKS(2000));
            if (!send_signed(crypto, jwk, account_url, nonce, authorization_url, "",
                             "application/json", reply)) break;
            authorization = cJSON_Parse(reply.body.c_str());
            if (!authorization) break;
            const std::string status = json_string(authorization, "status");
            cJSON_Delete(authorization);
            authorization = nullptr;
            if (status == "valid") {
                authorization_valid = true;
                break;
            }
            if (status == "invalid") break;
        }
        clear_http01_challenge();
    } else {
        cJSON_Delete(authorization);
    }
    if (!authorization_valid) return ESP_FAIL;

    std::string certificate_key_pem, csr_der_b64;
    if (!generate_csr(crypto, request.fqdn, certificate_key_pem, csr_der_b64)) return ESP_FAIL;
    cJSON* finalize_payload = cJSON_CreateObject();
    cJSON_AddStringToObject(finalize_payload, "csr", csr_der_b64.c_str());
    const std::string finalize_body = print_json(finalize_payload);
    cJSON_Delete(finalize_payload);
    if (!send_signed(crypto, jwk, account_url, nonce, finalize_url, finalize_body,
                     "application/json", reply)) return ESP_FAIL;

    std::string certificate_url;
    for (int attempt = 0; attempt < 60; ++attempt) {
        order = cJSON_Parse(reply.body.c_str());
        if (order) {
            const std::string status = json_string(order, "status");
            certificate_url = json_string(order, "certificate");
            cJSON_Delete(order);
            if (status == "valid" && !certificate_url.empty()) break;
            if (status == "invalid") return ESP_FAIL;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        if (!send_signed(crypto, jwk, account_url, nonce, order_url, "",
                         "application/json", reply)) return ESP_FAIL;
    }
    if (certificate_url.empty()) return ESP_ERR_TIMEOUT;

    if (!send_signed(crypto, jwk, account_url, nonce, certificate_url, "",
                     "application/pem-certificate-chain", reply)) return ESP_FAIL;
    if (reply.body.find("BEGIN CERTIFICATE") == std::string::npos) return ESP_FAIL;
    result.certificate_chain_pem = reply.body;
    result.private_key_pem = certificate_key_pem;
    clear_http01_challenge();
    return ESP_OK;
}

} // namespace acme_client