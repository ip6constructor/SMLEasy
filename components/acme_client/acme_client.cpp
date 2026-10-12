// ACME (RFC 8555) HTTP-01 client for ESP-IDF 6 / mbedTLS 4 (PSA crypto).
#include "acme_client.hpp"

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mbedtls/base64.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_csr.h"
#include "psa/crypto.h"

#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <strings.h>
#include <string>
#include <vector>

static const char* TAG = "AcmeClient";
static SemaphoreHandle_t s_challenge_mutex = nullptr;
static std::string s_challenge_token;
static std::string s_challenge_response;
static SemaphoreHandle_t s_log_mutex = nullptr;
static std::deque<std::string> s_log_lines;
static constexpr size_t kMaxLogLines = 40;
static std::string s_last_error;

namespace acme_client {
namespace {

static void note_v(const char* format, va_list args)
{
    char text[220];
    vsnprintf(text, sizeof(text), format, args);
    ESP_LOGI(TAG, "%s", text);
    if (!s_log_mutex) return;
    char stamp[16];
    const time_t now = time(nullptr);
    if (now > 1700000000) {
        struct tm parts{};
        gmtime_r(&now, &parts);
        strftime(stamp, sizeof(stamp), "%H:%M:%SZ", &parts);
    } else {
        snprintf(stamp, sizeof(stamp), "+%lus",
                 static_cast<unsigned long>(xTaskGetTickCount() / configTICK_RATE_HZ));
    }
    xSemaphoreTake(s_log_mutex, portMAX_DELAY);
    s_log_lines.push_back(std::string(stamp) + "  " + text);
    while (s_log_lines.size() > kMaxLogLines) s_log_lines.pop_front();
    xSemaphoreGive(s_log_mutex);
}

static void note(const char* format, ...) __attribute__((format(printf, 1, 2)));
static void note(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    note_v(format, args);
    va_end(args);
}

static constexpr char kProductionDirectory[] = "https://acme-v02.api.letsencrypt.org/directory";

struct HttpResponse {
    std::string body;
    std::string nonce;
    std::string location;
    int status{0};
};

// Volatile PSA ECDSA P-256 key used to sign JWS requests.
struct AccountKey {
    mbedtls_svc_key_id_t id{MBEDTLS_SVC_KEY_ID_INIT};
    ~AccountKey() {
        if (id != MBEDTLS_SVC_KEY_ID_INIT) psa_destroy_key(id);
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
    response = HttpResponse{};
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

static void set_p256_sign_attributes(psa_key_attributes_t& attributes)
{
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
}

// Loads the stored account key (PEM) or creates a new one; output_pem is what gets stored.
static bool prepare_account_key(AccountKey& key, const std::string& stored_pem,
                                std::string& output_pem)
{
    if (psa_crypto_init() != PSA_SUCCESS) return false;
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    bool ok = false;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    if (!stored_pem.empty()) {
        if (mbedtls_pk_parse_key(&pk, reinterpret_cast<const unsigned char*>(stored_pem.c_str()),
                                 stored_pem.size() + 1, nullptr, 0) == 0 &&
            mbedtls_pk_get_psa_attributes(&pk, PSA_KEY_USAGE_SIGN_HASH, &attributes) == 0) {
            set_p256_sign_attributes(attributes);
            ok = mbedtls_pk_import_into_psa(&pk, &attributes, &key.id) == 0;
            if (ok) output_pem = stored_pem;
        }
    } else {
        psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&attributes, 256);
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_EXPORT);
        set_p256_sign_attributes(attributes);
        if (psa_generate_key(&attributes, &key.id) == PSA_SUCCESS &&
            mbedtls_pk_copy_from_psa(key.id, &pk) == 0) {
            std::vector<unsigned char> pem(1024, 0);
            if (mbedtls_pk_write_key_pem(&pk, pem.data(), pem.size()) == 0) {
                output_pem = reinterpret_cast<const char*>(pem.data());
                ok = true;
            }
        }
    }
    psa_reset_key_attributes(&attributes);
    mbedtls_pk_free(&pk);
    return ok;
}

static bool sha256(const std::string& input, std::array<unsigned char, 32>& hash)
{
    size_t length = 0;
    return psa_hash_compute(PSA_ALG_SHA_256,
                            reinterpret_cast<const unsigned char*>(input.data()), input.size(),
                            hash.data(), hash.size(), &length) == PSA_SUCCESS && length == hash.size();
}

static bool make_jwk(const AccountKey& key, std::string& jwk)
{
    std::array<unsigned char, 65> pub{};
    size_t length = 0;
    if (psa_export_public_key(key.id, pub.data(), pub.size(), &length) != PSA_SUCCESS ||
        length != pub.size() || pub[0] != 0x04) return false;
    jwk = "{\"crv\":\"P-256\",\"kty\":\"EC\",\"x\":\"" +
          base64url(pub.data() + 1, 32) + "\",\"y\":\"" +
          base64url(pub.data() + 33, 32) + "\"}";
    return true;
}

static bool make_jwk_thumbprint(const std::string& jwk, std::string& thumbprint)
{
    std::array<unsigned char, 32> hash{};
    if (!sha256(jwk, hash)) return false;
    thumbprint = base64url(hash.data(), hash.size());
    return !thumbprint.empty();
}

// PSA returns ECDSA signatures as raw r||s, which is exactly the JWS ES256 format.
static bool sign_es256(const AccountKey& key, const std::string& input, std::string& signature)
{
    std::array<unsigned char, 32> hash{};
    if (!sha256(input, hash)) return false;
    std::array<unsigned char, 64> raw{};
    size_t length = 0;
    if (psa_sign_hash(key.id, PSA_ALG_ECDSA(PSA_ALG_SHA_256), hash.data(), hash.size(),
                      raw.data(), raw.size(), &length) != PSA_SUCCESS || length != raw.size())
        return false;
    signature = base64url(raw.data(), raw.size());
    return !signature.empty();
}

static bool make_jws(const AccountKey& key, const std::string& jwk, const std::string& kid,
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
    if (!sign_es256(key, signing_input, signature)) return false;
    cJSON* outer = cJSON_CreateObject();
    cJSON_AddStringToObject(outer, "protected", protected64.c_str());
    cJSON_AddStringToObject(outer, "payload", payload64.c_str());
    cJSON_AddStringToObject(outer, "signature", signature.c_str());
    jws = print_json(outer);
    cJSON_Delete(outer);
    return !jws.empty();
}

static bool send_signed(const AccountKey& key, const std::string& jwk, const std::string& kid,
                        std::string& nonce, const std::string& url, const std::string& payload,
                        const char* accept, HttpResponse& reply)
{
    if (nonce.empty()) return false;
    std::string jws;
    if (!make_jws(key, jwk, kid, nonce, url, payload, jws)) return false;
    if (http_request(url, HTTP_METHOD_POST, jws, accept, reply) != ESP_OK) return false;
    if (!reply.nonce.empty()) nonce = reply.nonce;
    if (reply.status < 200 || reply.status >= 300) {
        std::string detail = reply.body.substr(0, 160);
        if (cJSON* error = cJSON_Parse(reply.body.c_str())) {
            const std::string parsed = json_string(error, "detail");
            if (!parsed.empty()) detail = parsed.substr(0, 160);
            cJSON_Delete(error);
        }
        note("ACME request failed (HTTP %d): %s", reply.status, detail.c_str());
        s_last_error = detail;
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

// Creates a fresh certificate key pair and a DER CSR (base64url) for the FQDN.
static bool generate_csr(const std::string& fqdn, std::string& private_key_pem,
                         std::string& csr_der_b64)
{
    mbedtls_pk_context key;
    mbedtls_x509write_csr csr;
    mbedtls_pk_init(&key);
    mbedtls_x509write_csr_init(&csr);

    int result = psa_crypto_init() == PSA_SUCCESS ? 0 : -1;
    mbedtls_svc_key_id_t key_id = MBEDTLS_SVC_KEY_ID_INIT;
    if (result == 0) {
        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
        psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&attributes, 256);
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_EXPORT);
        set_p256_sign_attributes(attributes);
        if (psa_generate_key(&attributes, &key_id) != PSA_SUCCESS) result = -2;
        psa_reset_key_attributes(&attributes);
    }
    if (result == 0) result = mbedtls_pk_copy_from_psa(key_id, &key);
    if (key_id != MBEDTLS_SVC_KEY_ID_INIT) psa_destroy_key(key_id);

    mbedtls_x509write_csr_set_key(&csr, &key);
    mbedtls_x509write_csr_set_md_alg(&csr, MBEDTLS_MD_SHA256);
    const std::string subject = "CN=" + fqdn;
    if (result == 0) result = mbedtls_x509write_csr_set_subject_name(&csr, subject.c_str());

    mbedtls_x509_san_list san{};
    san.node.type = MBEDTLS_X509_SAN_DNS_NAME;
    san.node.san.unstructured_name.tag = MBEDTLS_ASN1_IA5_STRING;
    san.node.san.unstructured_name.p =
        reinterpret_cast<unsigned char*>(const_cast<char*>(fqdn.c_str()));
    san.node.san.unstructured_name.len = fqdn.size();
    san.next = nullptr;
    if (result == 0) result = mbedtls_x509write_csr_set_subject_alternative_name(&csr, &san);

    std::vector<unsigned char> csr_buffer(1024, 0);
    if (result == 0) {
        // The DER data is written to the end of the buffer; the return value is its length.
        const int der_length = mbedtls_x509write_csr_der(&csr, csr_buffer.data(), csr_buffer.size());
        if (der_length < 0) result = der_length;
        else csr_der_b64 = base64url(csr_buffer.data() + csr_buffer.size() - der_length,
                                     static_cast<size_t>(der_length));
    }
    std::vector<unsigned char> key_buffer(1024, 0);
    if (result == 0) result = mbedtls_pk_write_key_pem(&key, key_buffer.data(), key_buffer.size());
    if (result == 0) private_key_pem = reinterpret_cast<const char*>(key_buffer.data());

    mbedtls_x509write_csr_free(&csr);
    mbedtls_pk_free(&key);
    if (result != 0) ESP_LOGE(TAG, "CSR generation failed: %d", result);
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

esp_err_t fetch_directory_info(const std::string& directory_url, DirectoryInfo& info)
{
    if (directory_url.rfind("https://", 0) != 0) return ESP_ERR_INVALID_ARG;
    HttpResponse reply;
    if (http_request(directory_url, HTTP_METHOD_GET, "", "application/json", reply) != ESP_OK ||
        reply.status != 200) return ESP_FAIL;
    cJSON* directory = cJSON_Parse(reply.body.c_str());
    if (!directory) return ESP_FAIL;
    const cJSON* meta = cJSON_GetObjectItemCaseSensitive(directory, "meta");
    info.terms_url = meta ? json_string(meta, "termsOfService") : "";
    cJSON_Delete(directory);
    return ESP_OK;
}

esp_err_t resolve_public(const std::string& name, const char* type, std::vector<std::string>& answers)
{
    answers.clear();
    if (name.empty() || name.size() > 253 || !type) return ESP_ERR_INVALID_ARG;
    for (char ch : name) {
        if (!isalnum(static_cast<unsigned char>(ch)) && ch != '.' && ch != '-') return ESP_ERR_INVALID_ARG;
    }
    const bool ipv6 = strcmp(type, "AAAA") == 0;
    if (!ipv6 && strcmp(type, "A") != 0) return ESP_ERR_INVALID_ARG;
    static const char* const kResolvers[] = {
        "https://dns.google/resolve?name=", "https://cloudflare-dns.com/dns-query?name="};
    cJSON* json = nullptr;
    for (const char* resolver : kResolvers) {
        HttpResponse reply;
        const std::string url = std::string(resolver) + name + "&type=" + type;
        const esp_err_t err = http_request(url, HTTP_METHOD_GET, "", "application/dns-json", reply);
        if (err == ESP_OK && reply.status == 200) {
            json = cJSON_Parse(reply.body.c_str());
            if (json) break;
        }
        ESP_LOGW(TAG, "DNS lookup %s %s failed: %s, HTTP %d", resolver, type,
                 esp_err_to_name(err), reply.status);
        note("DNS lookup via %s failed (%s, HTTP %d)", resolver, esp_err_to_name(err), reply.status);
    }
    if (!json) return ESP_FAIL;
    const cJSON* list = cJSON_GetObjectItemCaseSensitive(json, "Answer");
    for (int i = 0; cJSON_IsArray(list) && i < cJSON_GetArraySize(list); ++i) {
        const cJSON* entry = cJSON_GetArrayItem(list, i);
        const cJSON* record_type = cJSON_GetObjectItemCaseSensitive(entry, "type");
        if (cJSON_IsNumber(record_type) && record_type->valueint == (ipv6 ? 28 : 1)) {
            const std::string data = json_string(entry, "data");
            if (!data.empty()) answers.push_back(data);
        }
    }
    cJSON_Delete(json);
    return ESP_OK;
}

std::string last_error()
{
    return s_last_error;
}

esp_err_t init()
{
    if (!s_log_mutex) s_log_mutex = xSemaphoreCreateMutex();
    if (s_challenge_mutex) return s_log_mutex ? ESP_OK : ESP_ERR_NO_MEM;
    s_challenge_mutex = xSemaphoreCreateMutex();
    return s_challenge_mutex && s_log_mutex ? ESP_OK : ESP_ERR_NO_MEM;
}

void add_log(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    note_v(format, args);
    va_end(args);
}

void clear_log()
{
    if (!s_log_mutex) return;
    xSemaphoreTake(s_log_mutex, portMAX_DELAY);
    s_log_lines.clear();
    xSemaphoreGive(s_log_mutex);
}

std::vector<std::string> get_log()
{
    std::vector<std::string> lines;
    if (!s_log_mutex) return lines;
    xSemaphoreTake(s_log_mutex, portMAX_DELAY);
    lines.assign(s_log_lines.begin(), s_log_lines.end());
    xSemaphoreGive(s_log_mutex);
    return lines;
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
    if (init() != ESP_OK || request.fqdn.empty() || request.email.empty() || !request.terms_accepted)
        return ESP_ERR_INVALID_ARG;

    s_last_error.clear();
    note("%s for %s (%s)", request.validate_only ? "Validation test" : "Certificate request",
         request.fqdn.c_str(),
         request.directory_url.find("staging") != std::string::npos ? "staging" : "production");
    AccountKey key;
    // A rehearsal always uses a fresh throwaway account so a cached authorization cannot hide a problem.
    const std::string stored_key = request.validate_only ? std::string() : request.account_key_pem;
    if (!prepare_account_key(key, stored_key, result.account_key_pem)) {
        note("Account key could not be prepared");
        return ESP_FAIL;
    }
    std::string jwk, thumbprint;
    if (!make_jwk(key, jwk) || !make_jwk_thumbprint(jwk, thumbprint)) {
        note("Account key is not an ECDSA P-256 key");
        return ESP_FAIL;
    }

    const std::string directory_url = request.directory_url.empty()
        ? kProductionDirectory : request.directory_url;
    if (directory_url.rfind("https://", 0) != 0) return ESP_ERR_INVALID_ARG;
    HttpResponse reply;
    if (http_request(directory_url, HTTP_METHOD_GET, "", "application/json", reply) != ESP_OK ||
        reply.status != 200) {
        note("ACME directory request failed (HTTP %d); check internet access and the clock", reply.status);
        return ESP_FAIL;
    }
    note("ACME directory loaded");
    cJSON* directory = cJSON_Parse(reply.body.c_str());
    if (!directory) return ESP_FAIL;
    const std::string new_nonce_url = json_string(directory, "newNonce");
    const std::string new_account_url = json_string(directory, "newAccount");
    const std::string new_order_url = json_string(directory, "newOrder");
    cJSON_Delete(directory);
    if (new_nonce_url.empty() || new_account_url.empty() || new_order_url.empty()) return ESP_FAIL;

    std::string nonce;
    if (!get_nonce(new_nonce_url, nonce)) {
        note("Could not obtain an ACME nonce");
        return ESP_FAIL;
    }
    std::string account_url = request.validate_only ? std::string() : request.account_url;
    if (account_url.empty()) {
        cJSON* account_payload = cJSON_CreateObject();
        cJSON* contacts = cJSON_CreateArray();
        const std::string contact = "mailto:" + request.email;
        cJSON_AddItemToArray(contacts, cJSON_CreateString(contact.c_str()));
        cJSON_AddItemToObject(account_payload, "contact", contacts);
        cJSON_AddBoolToObject(account_payload, "termsOfServiceAgreed", request.terms_accepted);
        const std::string payload = print_json(account_payload);
        cJSON_Delete(account_payload);
        if (!send_signed(key, jwk, "", nonce, new_account_url, payload,
                         "application/json", reply) || reply.location.empty()) return ESP_FAIL;
        account_url = reply.location;
        note("ACME account created");
    } else {
        note("ACME account reused");
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
    if (!send_signed(key, jwk, account_url, nonce, new_order_url, order_body,
                     "application/json", reply) || reply.location.empty()) return ESP_FAIL;
    const std::string order_url = reply.location;
    note("Order created");

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
    if (!send_signed(key, jwk, account_url, nonce, authorization_url, "",
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
        if (challenge_url.empty() || challenge_token.empty()) {
            note("No http-01 challenge offered");
            return ESP_FAIL;
        }
        publish_challenge(challenge_token, challenge_token + "." + thumbprint);
        note("HTTP-01 challenge published; asking the CA to validate http://%s/.well-known/acme-challenge/...",
             request.fqdn.c_str());
        const bool challenge_started = send_signed(key, jwk, account_url, nonce,
            challenge_url, "{}", "application/json", reply);
        if (!challenge_started) {
            clear_http01_challenge();
            return ESP_FAIL;
        }
        for (int attempt = 0; attempt < 60; ++attempt) {
            vTaskDelay(pdMS_TO_TICKS(2000));
            if (!send_signed(key, jwk, account_url, nonce, authorization_url, "",
                             "application/json", reply)) break;
            authorization = cJSON_Parse(reply.body.c_str());
            if (!authorization) break;
            const std::string status = json_string(authorization, "status");
            std::string error_detail;
            const cJSON* check_list = cJSON_GetObjectItemCaseSensitive(authorization, "challenges");
            for (int i = 0; cJSON_IsArray(check_list) && i < cJSON_GetArraySize(check_list); ++i) {
                const cJSON* error = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(check_list, i), "error");
                if (error) error_detail = json_string(error, "detail");
            }
            cJSON_Delete(authorization);
            authorization = nullptr;
            if (status == "valid") {
                note("Validation succeeded");
                authorization_valid = true;
                break;
            }
            if (status == "invalid") {
                s_last_error = error_detail.empty() ? "Validation failed" : error_detail;
                note("Validation failed: %s", error_detail.empty() ? "no detail from the CA" : error_detail.c_str());
                note("Check the DNS AAAA record and that port 80 reaches this device from the internet");
                break;
            }
            if (attempt % 3 == 0) note("Waiting for validation (status: %s)", status.c_str());
        }
        clear_http01_challenge();
    } else {
        cJSON_Delete(authorization);
        note("Domain already validated");
    }
    if (!authorization_valid) return ESP_FAIL;

    if (request.validate_only) {
        note("Test passed: the CA reached port 80 and validated the hostname");
        return ESP_OK;
    }

    std::string certificate_key_pem, csr_der_b64;
    if (!generate_csr(request.fqdn, certificate_key_pem, csr_der_b64)) {
        note("Certificate signing request could not be created");
        return ESP_FAIL;
    }
    note("Finalizing order");
    cJSON* finalize_payload = cJSON_CreateObject();
    cJSON_AddStringToObject(finalize_payload, "csr", csr_der_b64.c_str());
    const std::string finalize_body = print_json(finalize_payload);
    cJSON_Delete(finalize_payload);
    if (!send_signed(key, jwk, account_url, nonce, finalize_url, finalize_body,
                     "application/json", reply)) return ESP_FAIL;

    std::string certificate_url;
    for (int attempt = 0; attempt < 60; ++attempt) {
        order = cJSON_Parse(reply.body.c_str());
        if (order) {
            const std::string status = json_string(order, "status");
            certificate_url = json_string(order, "certificate");
            cJSON_Delete(order);
            if (status == "valid" && !certificate_url.empty()) break;
            if (status == "invalid") {
                note("Order became invalid");
                return ESP_FAIL;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        if (!send_signed(key, jwk, account_url, nonce, order_url, "",
                         "application/json", reply)) return ESP_FAIL;
    }
    if (certificate_url.empty()) {
        note("Timed out waiting for the certificate");
        return ESP_ERR_TIMEOUT;
    }

    if (!send_signed(key, jwk, account_url, nonce, certificate_url, "",
                     "application/pem-certificate-chain", reply)) return ESP_FAIL;
    if (reply.body.find("BEGIN CERTIFICATE") == std::string::npos) {
        note("Certificate download returned no certificate");
        return ESP_FAIL;
    }
    note("Certificate downloaded (%u bytes)", static_cast<unsigned>(reply.body.size()));
    result.certificate_chain_pem = reply.body;
    result.private_key_pem = certificate_key_pem;
    clear_http01_challenge();
    return ESP_OK;
}

} // namespace acme_client
