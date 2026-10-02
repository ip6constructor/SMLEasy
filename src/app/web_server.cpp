#include "web_server.hpp"
#include "web_assets.hpp"
#include "app_state.hpp"
#include "config_store.hpp"
#include "meter_task.hpp"
#include "wifi_manager.hpp"
#include "mqtt_manager.hpp"
#include "energy_stats.hpp"
#include "esp_timer.h"
#include "esp_wifi.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "mbedtls/base64.h"
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"

#include <array>
#include <atomic>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

static const char* TAG = "WebServer";
static std::atomic<int> s_github_ota_state{0};

static esp_err_t send_json(httpd_req_t* req, const std::string& body)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, body.c_str(), static_cast<ssize_t>(body.size()));
}

static esp_err_t read_body(httpd_req_t* req, std::string& out)
{
    out.resize(req->content_len);
    if (req->content_len == 0) return ESP_OK;
    int received = httpd_req_recv(req, &out[0], req->content_len);
    if (received <= 0) {
        out.clear();
        return ESP_FAIL;
    }
    out.resize(static_cast<size_t>(received));
    return ESP_OK;
}

static esp_err_t read_body_limited(httpd_req_t* req, std::string& out, size_t limit)
{
    if (req->content_len == 0 || req->content_len > limit) return ESP_FAIL;
    out.resize(req->content_len);
    size_t offset = 0;
    while (offset < out.size()) {
        const int received = httpd_req_recv(req, &out[offset], out.size() - offset);
        if (received <= 0) {
            out.clear();
            return ESP_FAIL;
        }
        offset += static_cast<size_t>(received);
    }
    return ESP_OK;
}

static bool parse_cidr(const std::string& cidr, int& family,
                       std::array<uint8_t, 16>& network, int& prefix)
{
    const size_t slash = cidr.find('/');
    if (slash == std::string::npos || slash == 0 || slash + 1 >= cidr.size()) return false;
    const std::string address = cidr.substr(0, slash);
    char* end = nullptr;
    const long parsed_prefix = strtol(cidr.c_str() + slash + 1, &end, 10);
    if (!end || *end != '\0') return false;

    ip4_addr_t ip4{};
    if (ip4addr_aton(address.c_str(), &ip4)) {
        family = AF_INET;
        if (parsed_prefix < 0 || parsed_prefix > 32) return false;
        memcpy(network.data(), &ip4.addr, sizeof(ip4.addr));
    } else {
        return false;
    }
    prefix = static_cast<int>(parsed_prefix);
    return true;
}

static bool cidr_matches(const sockaddr_storage& peer, const std::string& cidr)
{
    int family = 0, prefix = 0;
    std::array<uint8_t, 16> network{};
    if (!parse_cidr(cidr, family, network, prefix)) return false;

    if (peer.ss_family == AF_INET && family == AF_INET) {
        const auto* address = reinterpret_cast<const sockaddr_in*>(&peer);
        const auto* bytes = reinterpret_cast<const uint8_t*>(&address->sin_addr.s_addr);
        const int full_bytes = prefix / 8;
        const int remaining_bits = prefix % 8;
        if (full_bytes && memcmp(bytes, network.data(), full_bytes) != 0) return false;
        if (remaining_bits) {
            const uint8_t mask = static_cast<uint8_t>(0xffu << (8 - remaining_bits));
            if ((bytes[full_bytes] & mask) != (network[full_bytes] & mask)) return false;
        }
        return true;
    }

    return false;
}

static bool is_private_address(const sockaddr_storage& peer)
{
    if (peer.ss_family == AF_INET) {
        const auto* address = reinterpret_cast<const sockaddr_in*>(&peer);
        const uint32_t ip = ntohl(address->sin_addr.s_addr);
        return (ip & 0xff000000u) == 0x0a000000u ||
               (ip & 0xfff00000u) == 0xac100000u ||
               (ip & 0xffff0000u) == 0xc0a80000u;
    }
    return false;
}

static bool valid_cidr_list(const std::string& list)
{
    if (list.size() > 512) return false;
    size_t start = 0;
    while (start < list.size()) {
        const size_t end = list.find_first_of(",; \t\r\n", start);
        const std::string item = list.substr(start, end == std::string::npos ? end : end - start);
        if (!item.empty()) {
            int family = 0, prefix = 0;
            std::array<uint8_t, 16> network{};
            if (!parse_cidr(item, family, network, prefix)) return false;
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return true;
}

static esp_err_t dashboard_network_filter(httpd_handle_t, int sockfd)
{
    sockaddr_storage peer{};
    socklen_t peer_len = sizeof(peer);
    if (getpeername(sockfd, reinterpret_cast<sockaddr*>(&peer), &peer_len) != 0) {
        shutdown(sockfd, SHUT_RDWR);
        return ESP_FAIL;
    }
    if (is_private_address(peer)) return ESP_OK;

    app::UiConfig cfg;
    app::ConfigStore::get().load_ui(cfg);
    size_t start = 0;
    while (start < cfg.allowed_networks.size()) {
        const size_t end = cfg.allowed_networks.find_first_of(",; \t\r\n", start);
        const std::string item = cfg.allowed_networks.substr(
            start, end == std::string::npos ? end : end - start);
        if (!item.empty() && cidr_matches(peer, item)) return ESP_OK;
        if (end == std::string::npos) break;
        start = end + 1;
    }
    ESP_LOGW(TAG, "blocked web client outside allowed networks");
    shutdown(sockfd, SHUT_RDWR);
    return ESP_FAIL;
}

static bool require_auth(httpd_req_t* req)
{
    char header[256] = {};
    char decoded[192] = {};
    size_t decoded_len = 0;
    app::WebAuthConfig auth;
    app::ConfigStore::get().load_web_auth(auth);
    const bool header_ok = httpd_req_get_hdr_value_str(req, "Authorization",
                                                        header, sizeof(header)) == ESP_OK;
    if (header_ok && strncmp(header, "Basic ", 6) == 0 &&
        mbedtls_base64_decode(reinterpret_cast<unsigned char*>(decoded), sizeof(decoded) - 1,
                              &decoded_len, reinterpret_cast<const unsigned char*>(header + 6),
                              strlen(header + 6)) == 0) {
        decoded[decoded_len] = '\0';
        const std::string expected = auth.username + ":" + auth.password;
        if (expected == decoded) return true;
    }
    httpd_resp_set_status(req, "401 Unauthorized");
    httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"EasySML\"");
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    httpd_resp_send(req, "Authentication required", HTTPD_RESP_USE_STRLEN);
    return false;
}

#define REQUIRE_AUTH(req) do { if (!require_auth(req)) return ESP_FAIL; } while (0)

static void ota_reboot_task(void*)
{
    vTaskDelay(pdMS_TO_TICKS(2500));
    esp_restart();
    vTaskDelete(nullptr);
}

static bool is_semver(const std::string& version)
{
    int segments = 1;
    bool digit_seen = false;
    for (char ch : version) {
        if (ch >= '0' && ch <= '9') {
            digit_seen = true;
        } else if (ch == '.' && digit_seen && segments < 3) {
            ++segments;
            digit_seen = false;
        } else {
            return false;
        }
    }
    return segments == 3 && digit_seen;
}

static bool is_allowed_github_firmware_url(const std::string& url)
{
    constexpr char kPrefix[] = "https://github.com/ip6constructor/SMLEasy/releases/download/";
    if (url.rfind(kPrefix, 0) != 0) return false;
    const std::string asset_path = url.substr(sizeof(kPrefix) - 1);
    const size_t slash = asset_path.find('/');
    if (slash == std::string::npos || slash + 1 >= asset_path.size()) return false;
    const std::string tag = asset_path.substr(0, slash);
    if (tag.size() < 2 || tag[0] != 'v') return false;
    const std::string version = tag.substr(1);
    if (!is_semver(version)) return false;
    return asset_path.substr(slash + 1) == "SMLEasy-" + version + ".bin";
}

static void github_ota_task(void* arg)
{
    char* url = static_cast<char*>(arg);
    esp_http_client_config_t http_cfg{};
    http_cfg.url = url;
    http_cfg.timeout_ms = 30000;
    http_cfg.buffer_size = 4096;
    http_cfg.max_redirection_count = 5;
    http_cfg.keep_alive_enable = true;
    http_cfg.crt_bundle_attach = esp_crt_bundle_attach;

    ESP_LOGI(TAG, "Starting GitHub OTA from %s", url);
    const esp_err_t err = esp_https_ota(&http_cfg);
    free(url);
    if (err == ESP_OK) {
        s_github_ota_state.store(2);
        app::AppState::get().push_log("I", TAG, "GitHub OTA verified; restarting");
        vTaskDelay(pdMS_TO_TICKS(1500));
        esp_restart();
    }
    ESP_LOGE(TAG, "GitHub OTA failed: %s", esp_err_to_name(err));
    app::AppState::get().push_log("E", TAG, "GitHub OTA failed");
    s_github_ota_state.store(3);
    vTaskDelete(nullptr);
}

#if 0 // Temporarily disabled while stabilizing the IPv4 HTTP recovery firmware.
static bool valid_fqdn(const std::string& fqdn)
{
    if (fqdn.empty() || fqdn.size() > 253 || fqdn.find('.') == std::string::npos) return false;
    size_t label_start = 0;
    for (size_t i = 0; i <= fqdn.size(); ++i) {
        if (i == fqdn.size() || fqdn[i] == '.') {
            const size_t label_size = i - label_start;
            if (label_size == 0 || label_size > 63 || fqdn[label_start] == '-' || fqdn[i - 1] == '-') return false;
            label_start = i + 1;
        } else {
            const unsigned char ch = static_cast<unsigned char>(fqdn[i]);
            if (!isalnum(ch) && ch != '-') return false;
        }
    }
    return true;
}

static bool valid_email(const std::string& email)
{
    const size_t at = email.find('@');
    return at != std::string::npos && at > 0 && at + 1 < email.size() &&
           email.find('@', at + 1) == std::string::npos &&
           email.find('.', at + 1) != std::string::npos &&
           email.size() <= 254;
}

static bool valid_tls_pem_pair(const app::TlsConfig& cfg)
{
    if (cfg.certificate_pem.empty() || cfg.private_key_pem.empty()) return false;
    mbedtls_x509_crt certificate;
    mbedtls_pk_context private_key;
    mbedtls_x509_crt_init(&certificate);
    mbedtls_pk_init(&private_key);
    const int cert_result = mbedtls_x509_crt_parse(
        &certificate, reinterpret_cast<const unsigned char*>(cfg.certificate_pem.c_str()),
        cfg.certificate_pem.size() + 1);
    const int key_result = mbedtls_pk_parse_key(
        &private_key, reinterpret_cast<const unsigned char*>(cfg.private_key_pem.c_str()),
        cfg.private_key_pem.size() + 1, nullptr, 0);
    const bool valid = cert_result == 0 && key_result == 0 &&
        mbedtls_pk_check_pair(&certificate.pk, &private_key) == 0;
    mbedtls_pk_free(&private_key);
    mbedtls_x509_crt_free(&certificate);
    return valid;
}

static void append_der_length(std::vector<uint8_t>& output, size_t length)
{
    if (length < 128) {
        output.push_back(static_cast<uint8_t>(length));
    } else if (length < 256) {
        output.push_back(0x81);
        output.push_back(static_cast<uint8_t>(length));
    } else {
        output.push_back(0x82);
        output.push_back(static_cast<uint8_t>(length >> 8));
        output.push_back(static_cast<uint8_t>(length));
    }
}

static bool generate_self_signed_fallback(app::TlsConfig& cfg)
{
    const std::string fqdn = valid_fqdn(cfg.fqdn) ? cfg.fqdn : "smleasy.local";
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng;
    mbedtls_pk_context key;
    mbedtls_x509write_cert writer;
    mbedtls_mpi serial;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&rng);
    mbedtls_pk_init(&key);
    mbedtls_x509write_crt_init(&writer);
    mbedtls_mpi_init(&serial);

    const char* personalization = "SMLEasy self-signed TLS fallback";
    int result = mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy,
        reinterpret_cast<const unsigned char*>(personalization), strlen(personalization));
    if (result == 0) result = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (result == 0) result = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1,
        mbedtls_pk_ec(key), mbedtls_ctr_drbg_random, &rng);
    if (result == 0) result = mbedtls_mpi_lset(&serial, 1);

    std::string subject = "CN=" + fqdn;
    mbedtls_x509write_crt_set_version(&writer, MBEDTLS_X509_CRT_VERSION_3);
    if (result == 0) result = mbedtls_x509write_crt_set_serial(&writer, &serial);
    if (result == 0) result = mbedtls_x509write_crt_set_subject_name(&writer, subject.c_str());
    if (result == 0) result = mbedtls_x509write_crt_set_issuer_name(&writer, subject.c_str());
    mbedtls_x509write_crt_set_subject_key(&writer, &key);
    mbedtls_x509write_crt_set_issuer_key(&writer, &key);
    mbedtls_x509write_crt_set_md_alg(&writer, MBEDTLS_MD_SHA256);
    if (result == 0) result = mbedtls_x509write_crt_set_basic_constraints(&writer, 0, -1);
    if (result == 0) result = mbedtls_x509write_crt_set_key_usage(&writer,
        MBEDTLS_X509_KU_DIGITAL_SIGNATURE | MBEDTLS_X509_KU_KEY_ENCIPHERMENT);

    std::vector<uint8_t> san_value;
    std::vector<uint8_t> dns_name;
    dns_name.push_back(0x82);
    append_der_length(dns_name, fqdn.size());
    dns_name.insert(dns_name.end(), fqdn.begin(), fqdn.end());
    san_value.push_back(0x30);
    append_der_length(san_value, dns_name.size());
    san_value.insert(san_value.end(), dns_name.begin(), dns_name.end());
    if (result == 0) result = mbedtls_x509write_crt_set_extension(&writer,
        MBEDTLS_OID_SUBJECT_ALT_NAME, MBEDTLS_OID_SIZE(MBEDTLS_OID_SUBJECT_ALT_NAME),
        0, san_value.data(), san_value.size());

    time_t now = time(nullptr);
    if (now < 1700000000) now = 1735689600;
    struct tm not_before_tm{}, not_after_tm{};
    gmtime_r(&now, &not_before_tm);
    now += 5LL * 365LL * 24LL * 60LL * 60LL;
    gmtime_r(&now, &not_after_tm);
    char not_before[16]{}, not_after[16]{};
    strftime(not_before, sizeof(not_before), "%Y%m%d%H%M%S", &not_before_tm);
    strftime(not_after, sizeof(not_after), "%Y%m%d%H%M%S", &not_after_tm);
    if (result == 0) result = mbedtls_x509write_crt_set_validity(&writer, not_before, not_after);

    unsigned char certificate_buffer[4096]{};
    unsigned char key_buffer[2048]{};
    if (result == 0) result = mbedtls_x509write_crt_pem(&writer, certificate_buffer,
        sizeof(certificate_buffer), mbedtls_ctr_drbg_random, &rng);
    if (result == 0) result = mbedtls_pk_write_key_pem(&key, key_buffer, sizeof(key_buffer));
    if (result == 0) {
        cfg.fallback_certificate_pem = reinterpret_cast<char*>(certificate_buffer);
        cfg.fallback_private_key_pem = reinterpret_cast<char*>(key_buffer);
        result = app::ConfigStore::get().save_tls(cfg) ? 0 : -1;
    }

    mbedtls_mpi_free(&serial);
    mbedtls_x509write_crt_free(&writer);
    mbedtls_pk_free(&key);
    mbedtls_ctr_drbg_free(&rng);
    mbedtls_entropy_free(&entropy);
    if (result != 0) ESP_LOGE(TAG, "self-signed fallback generation failed: -0x%04x", -result);
    return result == 0;
}

struct TlsFallbackTaskContext {
    app::TlsConfig* config;
    SemaphoreHandle_t completed;
    bool succeeded;
};

static void tls_fallback_generation_task(void* arg)
{
    auto* context = static_cast<TlsFallbackTaskContext*>(arg);
    context->succeeded = generate_self_signed_fallback(*context->config);
    xSemaphoreGive(context->completed);
    vTaskDelete(nullptr);
}

static bool generate_self_signed_fallback_with_stack(app::TlsConfig& cfg)
{
    SemaphoreHandle_t completed = xSemaphoreCreateBinary();
    if (!completed) return false;
    TlsFallbackTaskContext context{&cfg, completed, false};
    if (xTaskCreate(tls_fallback_generation_task, "tls_cert_gen", 8192,
                    &context, 4, nullptr) != pdPASS) {
        vSemaphoreDelete(completed);
        return false;
    }
    xSemaphoreTake(completed, portMAX_DELAY);
    vSemaphoreDelete(completed);
    return context.succeeded;
}

static void acme_issue_task(void*)
{
    auto& web_server = app::WebServer::get();
    const bool restore_http_recovery = !web_server.is_https();
    if (restore_http_recovery) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        web_server.stop();
        if (web_server.start_acme_challenge_listener() != ESP_OK) {
            web_server.stop_acme_challenge_listener();
            web_server.start(80);
            s_acme_issuance_state.store(3);
            vTaskDelete(nullptr);
            return;
        }
    }
    app::TlsConfig cfg;
    app::ConfigStore::get().load_tls(cfg);
    acme_client::Request request;
    request.directory_url = cfg.acme_staging
        ? "https://acme-staging-v02.api.letsencrypt.org/directory"
        : "https://acme-v02.api.letsencrypt.org/directory";
    request.fqdn = cfg.fqdn;
    request.email = cfg.email;
    request.account_key_pem = cfg.acme_account_key_pem;
    request.account_url = cfg.acme_account_url;
    request.terms_accepted = cfg.acme_terms_accepted;
    acme_client::Result result;
    const esp_err_t err = acme_client::issue_http01(request, result);
    if (!result.account_key_pem.empty()) cfg.acme_account_key_pem = result.account_key_pem;
    if (!result.account_url.empty()) cfg.acme_account_url = result.account_url;
    if (err == ESP_OK) {
        cfg.certificate_pem = result.certificate_chain_pem;
        cfg.private_key_pem = result.private_key_pem;
        cfg.last_issued_epoch = static_cast<int64_t>(time(nullptr));
        if (app::ConfigStore::get().save_tls(cfg)) {
            s_acme_issuance_state.store(2);
            app::AppState::get().push_log("I", TAG, "Let's Encrypt certificate issued");
            vTaskDelay(pdMS_TO_TICKS(2500));
            esp_restart();
        }
    }
    if (!result.account_key_pem.empty() || !result.account_url.empty())
        app::ConfigStore::get().save_tls(cfg);
    ESP_LOGE(TAG, "ACME HTTP-01 issuance failed: %s", esp_err_to_name(err));
    app::AppState::get().push_log("E", TAG, "Let's Encrypt issuance failed");
    if (restore_http_recovery) {
        web_server.stop_acme_challenge_listener();
        if (web_server.start(80) != ESP_OK)
            ESP_LOGE(TAG, "failed to restore HTTP recovery dashboard after ACME error");
    }
    s_acme_issuance_state.store(3);
    vTaskDelete(nullptr);
}

static void acme_renewal_task(void*)
{
    for (;;) {
        const uint32_t delay_seconds = 3600u + (esp_random() % 3600u);
        vTaskDelay(pdMS_TO_TICKS(delay_seconds * 1000u));
        app::TlsConfig cfg;
        app::ConfigStore::get().load_tls(cfg);
        if (cfg.mode != "letsencrypt" || !app::WifiManager::get().is_sta_connected()) continue;
        const time_t now = time(nullptr);
        if (now < 1700000000) continue;
        if (cfg.last_issued_epoch <= 0) continue;
        const int64_t interval_seconds = static_cast<int64_t>(cfg.renewal_interval_days) * 86400;
        if (now < cfg.last_issued_epoch + interval_seconds) continue;
        int expected = s_acme_issuance_state.load();
        if (expected == 1 || !s_acme_issuance_state.compare_exchange_strong(expected, 1)) continue;
        if (xTaskCreate(acme_issue_task, "acme_issue", 16384, nullptr, 5, nullptr) != pdPASS)
            s_acme_issuance_state.store(3);
    }
}
#endif

namespace app {

WebServer& WebServer::get() {
    static WebServer inst;
    return inst;
}

// ── start / stop ──────────────────────────────────────────────────────────────

esp_err_t WebServer::start(uint16_t port)
{
    httpd_config_t http_cfg = HTTPD_DEFAULT_CONFIG();
    http_cfg.server_port = port;
    http_cfg.max_uri_handlers = 40;
    http_cfg.stack_size = 8192;
    http_cfg.lru_purge_enable = true;
    http_cfg.recv_wait_timeout = 60;
    http_cfg.send_wait_timeout = 60;
    http_cfg.open_fn = dashboard_network_filter;
    if (httpd_start(&server_, &http_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "HTTP dashboard start failed");
        return ESP_FAIL;
    }

    const httpd_uri_t routes[] = {
        { "/",                    HTTP_GET,  handle_root,          nullptr },
        { "/config",              HTTP_GET,  handle_config_page,   nullptr },
        { "/style.css",           HTTP_GET,  handle_style_css,     nullptr },
        { "/app.js",              HTTP_GET,  handle_app_js,        nullptr },
        { "/api/status",          HTTP_GET,  handle_status,        nullptr },
        { "/api/log",             HTTP_GET,  handle_log,           nullptr },
        { "/api/history",         HTTP_GET,  handle_history,       nullptr },
        { "/api/wifi/scan",       HTTP_GET,  handle_wifi_scan,      nullptr },
        { "/api/config/meter",    HTTP_GET,  handle_config_get,    nullptr },
        { "/api/config/meter",    HTTP_POST, handle_config_save,   nullptr },
        { "/api/config/wifi",     HTTP_POST, handle_wifi_save,         nullptr },
        { "/api/config/auth",     HTTP_GET,  handle_auth_get,          nullptr },
        { "/api/config/auth",     HTTP_POST, handle_auth_save,         nullptr },
        { "/api/config/language", HTTP_GET,  handle_ui_language_get,   nullptr },
        { "/api/config/language", HTTP_POST, handle_ui_language_save,  nullptr },
        { "/api/config/networks", HTTP_GET,  handle_access_networks_get, nullptr },
        { "/api/config/networks", HTTP_POST, handle_access_networks_save, nullptr },
        { "/api/config/ha",       HTTP_GET,  handle_ha_config_get,     nullptr },
        { "/api/config/ha",       HTTP_POST, handle_ha_config_save,    nullptr },
        { "/api/config/tariff",   HTTP_GET,  handle_tariff_get,        nullptr },
        { "/api/config/tariff",   HTTP_POST, handle_tariff_save,       nullptr },
        { "/api/start",           HTTP_POST, handle_start,            nullptr },
        { "/api/start_continuous", HTTP_POST, handle_start_continuous, nullptr },
        { "/api/stop",            HTTP_POST, handle_stop,             nullptr },
        { "/api/reboot",          HTTP_POST, handle_reboot,        nullptr },
        { "/api/reset_counters",  HTTP_POST, handle_reset_counters,nullptr },
        { "/api/ota",             HTTP_POST, handle_ota,           nullptr },
        { "/api/ota/github",      HTTP_POST, handle_ota_github,    nullptr },
        { "/api/ota/github/status", HTTP_GET, handle_ota_github_status, nullptr },
        { "/api/meter",           HTTP_GET,  handle_meter,         nullptr },
        { "/favicon.ico",          HTTP_GET,  handle_favicon,       nullptr },
    };
    for (const auto& r : routes) httpd_register_uri_handler(server_, &r);

    ESP_LOGI(TAG, "HTTP recovery dashboard started on IPv4 port %u", port);
    return ESP_OK;
}

void WebServer::stop() {
    if (server_) {
        httpd_stop(server_);
        server_ = nullptr;
    }
}

// ── static assets ─────────────────────────────────────────────────────────────

esp_err_t WebServer::handle_root(httpd_req_t* req) {
    // If query parameters are present, treat them as a config-save shortcut
    char qbuf[512] = {};
    if (httpd_req_get_url_query_str(req, qbuf, sizeof(qbuf)) == ESP_OK && qbuf[0] != '\0') {
        MeterConfig cfg;
        ConfigStore::get().load_meter(cfg);
        char val[128] = {};
        if (httpd_query_key_value(qbuf, "interval_s", val, sizeof(val)) == ESP_OK)
            cfg.interval_s = static_cast<uint32_t>(std::stoul(val));
        if (httpd_query_key_value(qbuf, "uart_debug", val, sizeof(val)) == ESP_OK)
            cfg.uart_debug = (val[0] == '1' || val[0] == 't' || val[0] == 'o');
        if (cfg.interval_s < 5) cfg.interval_s = 5;
        ConfigStore::get().save_meter(cfg);
        AppState::get().read_interval_s.store(cfg.interval_s);
        ESP_LOGI(TAG, "Config updated via URL query params");
        // Redirect to clean root URL so a browser refresh doesn't re-apply
        httpd_resp_set_status(req, "302 Found");
        httpd_resp_set_hdr(req, "Location", "/");
        return httpd_resp_send(req, nullptr, 0);
    }
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, kIndexHtml, HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebServer::handle_config_page(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, kConfigHtml, HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebServer::handle_style_css(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Cache-Control", "max-age=3600");
    return httpd_resp_send(req, kStyleCss, HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebServer::handle_app_js(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, kAppJs, HTTPD_RESP_USE_STRLEN);
}

// ── /api/status ───────────────────────────────────────────────────────────────

esp_err_t WebServer::handle_status(httpd_req_t* req) {
    auto& st = AppState::get();
    MeterData md = st.get_meter_data();
    auto& io     = st.io;

    auto js = [](JobState s) -> const char* {
        switch(s) {
            case JobState::Running: return "Running";
            case JobState::Done:    return "Done";
            case JobState::Error:   return "Error";
            default:                return "Idle";
        }
    };

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "job_state",    js(st.job_state()));
    cJSON_AddStringToObject(root, "job_error",    st.job_error().c_str());
    cJSON_AddBoolToObject  (root, "populated",    md.populated);
    cJSON_AddBoolToObject  (root, "has_fwd_active_wh", md.has_fwd_active_wh);
    cJSON_AddBoolToObject  (root, "has_rev_active_wh", md.has_rev_active_wh);
    cJSON_AddBoolToObject  (root, "has_power", md.has_power);
    cJSON_AddBoolToObject  (root, "has_v_l1", md.has_v_l1);
    cJSON_AddBoolToObject  (root, "has_v_l2", md.has_v_l2);
    cJSON_AddBoolToObject  (root, "has_v_l3", md.has_v_l3);
    cJSON_AddBoolToObject  (root, "has_i_l1", md.has_i_l1);
    cJSON_AddBoolToObject  (root, "has_i_l2", md.has_i_l2);
    cJSON_AddBoolToObject  (root, "has_i_l3", md.has_i_l3);
    cJSON_AddBoolToObject  (root, "has_frequency", md.has_frequency);
    cJSON_AddBoolToObject  (root, "has_pf_l1", md.has_pf_l1);
    cJSON_AddStringToObject(root, "manufacturer", md.manufacturer.c_str());
    cJSON_AddStringToObject(root, "model",        md.model.c_str());
    cJSON_AddStringToObject(root, "fw_version",   md.fw_version.c_str());
    cJSON_AddStringToObject(root, "serial_bcd",   md.serial_bcd.c_str());
    cJSON_AddStringToObject(root, "utility_serial",md.utility_serial.c_str());
    cJSON_AddStringToObject(root, "meter_time",   md.meter_time.c_str());
    cJSON_AddNumberToObject(root, "fwd_active_wh",    md.fwd_active_wh);
    cJSON_AddNumberToObject(root, "rev_active_wh",    md.rev_active_wh);
    cJSON_AddNumberToObject(root, "import_react_varh",md.import_react_varh);
    cJSON_AddNumberToObject(root, "export_react_varh",md.export_react_varh);
    cJSON_AddNumberToObject(root, "fwd_w",   md.fwd_w);
    cJSON_AddNumberToObject(root, "rev_w",   md.rev_w);
    cJSON_AddNumberToObject(root, "v_l1_mv", md.v_l1_mv);
    cJSON_AddNumberToObject(root, "v_l2_mv", md.v_l2_mv);
    cJSON_AddNumberToObject(root, "v_l3_mv", md.v_l3_mv);
    cJSON_AddNumberToObject(root, "i_l1_ma", md.i_l1_ma);
    cJSON_AddNumberToObject(root, "i_l2_ma", md.i_l2_ma);
    cJSON_AddNumberToObject(root, "i_l3_ma", md.i_l3_ma);
    cJSON_AddNumberToObject(root, "freq_mhz",md.freq_mhz);
    cJSON_AddNumberToObject(root, "pf_l1",   md.pf_l1);
    cJSON_AddBoolToObject  (root, "has_alarms",   md.has_alarms);
    cJSON_AddStringToObject(root, "alarm_list",   md.alarm_list.c_str());
    cJSON_AddNumberToObject(root, "tx_bytes",    io.tx_bytes.load());
    cJSON_AddNumberToObject(root, "rx_bytes",    io.rx_bytes.load());
    cJSON_AddNumberToObject(root, "tx_frames",   io.tx_frames.load());
    cJSON_AddNumberToObject(root, "rx_frames",   io.rx_frames.load());
    cJSON_AddNumberToObject(root, "crc_errors",  io.crc_errors.load());
    cJSON_AddNumberToObject(root, "nak_count",   io.nak_count.load());
    cJSON_AddNumberToObject(root, "ack_count",   io.ack_count.load());
    cJSON_AddNumberToObject(root, "wakeup_count",io.wakeup_count.load());
    cJSON_AddBoolToObject  (root, "flag_ident",  io.flag_ident.load());
    cJSON_AddBoolToObject  (root, "flag_logon",  io.flag_logon.load());
    cJSON_AddBoolToObject  (root, "flag_auth",   io.flag_auth.load());
    cJSON_AddBoolToObject  (root, "last_login_ok", st.last_login_ok.load());
    MeterConfig meter_cfg;
    ConfigStore::get().load_meter(meter_cfg);
    cJSON_AddBoolToObject  (root, "login_required", !meter_cfg.login_cmd.empty());
    cJSON_AddBoolToObject  (root, "continuous", st.continuous.load());
    cJSON_AddBoolToObject  (root, "wifi_connected",
        WifiManager::get().is_sta_connected() || WifiManager::get().is_ap_active());
    cJSON_AddStringToObject(root, "ip",      WifiManager::get().get_ip().c_str());
    cJSON_AddStringToObject(root, "ssid",    WifiManager::get().is_sta_connected()
                                               ? WifiManager::get().sta_ssid().c_str()
                                               : WifiManager::get().ap_ssid().c_str());
    cJSON_AddStringToObject(root, "ip_sta",  WifiManager::get().is_sta_connected()
                                               ? WifiManager::get().get_ip().c_str() : "");
    cJSON_AddStringToObject(root, "ip_ap",   WifiManager::get().is_ap_active()
                                               ? "192.168.4.1" : "");
    cJSON_AddStringToObject(root, "ssid_sta", WifiManager::get().sta_ssid().c_str());
    cJSON_AddStringToObject(root, "ssid_ap",  WifiManager::get().is_ap_active()
                                               ? WifiManager::get().ap_ssid().c_str() : "");
    cJSON_AddNumberToObject(root, "uptime_s", (double)(esp_timer_get_time() / 1000000LL));
    cJSON_AddNumberToObject(root, "tx_pin", MeterTask::get().tx_pin());
    cJSON_AddNumberToObject(root, "rx_pin", MeterTask::get().rx_pin());
    cJSON_AddStringToObject(root, "app_version", esp_ota_get_app_description()->version);
    cJSON_AddStringToObject(root, "idf_version", IDF_VER);

    // Daily/monthly energy statistics + cost estimate (in-RAM only, see EnergyStats).
    StatsSnapshot stats = EnergyStats::get().snapshot();
    TariffConfig tariff;
    ConfigStore::get().load_tariff(tariff);
    cJSON_AddBoolToObject  (root, "time_synced",            stats.time_synced);
    cJSON_AddNumberToObject(root, "today_import_kwh",        stats.today_import_kwh);
    cJSON_AddNumberToObject(root, "today_export_kwh",        stats.today_export_kwh);
    cJSON_AddNumberToObject(root, "yesterday_import_kwh",    stats.yesterday_import_kwh);
    cJSON_AddNumberToObject(root, "yesterday_export_kwh",    stats.yesterday_export_kwh);
    cJSON_AddNumberToObject(root, "month_import_kwh",        stats.month_import_kwh);
    cJSON_AddNumberToObject(root, "month_export_kwh",        stats.month_export_kwh);
    cJSON_AddNumberToObject(root, "cost_today",
        stats.today_import_kwh * tariff.price_import_kwh - stats.today_export_kwh * tariff.price_export_kwh);
    cJSON_AddNumberToObject(root, "cost_month",
        stats.month_import_kwh * tariff.price_import_kwh - stats.month_export_kwh * tariff.price_export_kwh);
    cJSON_AddStringToObject(root, "currency",                tariff.currency.c_str());
    cJSON_AddNumberToObject(root, "price_import_kwh",        tariff.price_import_kwh);
    cJSON_AddNumberToObject(root, "price_export_kwh",        tariff.price_export_kwh);

    char* s = cJSON_PrintUnformatted(root);
    std::string body(s);
    cJSON_free(s);
    cJSON_Delete(root);
    return send_json(req, body);
}

// ── /api/log?after=N ──────────────────────────────────────────────────────────

esp_err_t WebServer::handle_log(httpd_req_t* req) {
    char buf[32] = {};
    uint32_t after = 0;
    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
        char val[20] = {};
        if (httpd_query_key_value(buf, "after", val, sizeof(val)) == ESP_OK) {
            after = static_cast<uint32_t>(strtoul(val, nullptr, 10));
        }
    }
    std::string body = AppState::get().log_json(after);
    return send_json(req, body);
}

// ── /api/history (in-RAM power history for charts) ────────────────────────────

esp_err_t WebServer::handle_history(httpd_req_t* req) {
    return send_json(req, EnergyStats::get().history_json());
}

// ── /api/wifi/scan ───────────────────────────────────────────────────────────

esp_err_t WebServer::handle_wifi_scan(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    if (esp_wifi_scan_start(nullptr, true) != ESP_OK) {
        return send_json(req, "{\"ok\":false,\"networks\":[]}");
    }

    uint16_t count = 0;
    if (esp_wifi_scan_get_ap_num(&count) != ESP_OK) {
        return send_json(req, "{\"ok\":false,\"networks\":[]}");
    }
    std::vector<wifi_ap_record_t> records(count);
    if (count > 0 && esp_wifi_scan_get_ap_records(&count, records.data()) != ESP_OK) {
        return send_json(req, "{\"ok\":false,\"networks\":[]}");
    }

    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "ok", true);
    cJSON* networks = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "networks", networks);
    std::map<std::string, wifi_ap_record_t> strongest;
    for (uint16_t i = 0; i < count; ++i) {
        if (records[i].ssid[0] == '\0') continue;
        const std::string ssid(reinterpret_cast<const char*>(records[i].ssid));
        const auto current = strongest.find(ssid);
        if (current == strongest.end() || records[i].rssi > current->second.rssi) {
            strongest[ssid] = records[i];
        }
    }
    for (const auto& entry : strongest) {
        const wifi_ap_record_t& record = entry.second;
        cJSON* network = cJSON_CreateObject();
        cJSON_AddStringToObject(network, "ssid", entry.first.c_str());
        cJSON_AddNumberToObject(network, "rssi", record.rssi);
        cJSON_AddNumberToObject(network, "channel", record.primary);
        cJSON_AddNumberToObject(network, "auth", record.authmode);
        cJSON_AddItemToArray(networks, network);
    }
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{\"ok\":false,\"networks\":[]}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

// ── /api/config/tariff (GET + POST) ───────────────────────────────────────────

esp_err_t WebServer::handle_tariff_get(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    TariffConfig cfg;
    ConfigStore::get().load_tariff(cfg);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "price_import_kwh", cfg.price_import_kwh);
    cJSON_AddNumberToObject(root, "price_export_kwh", cfg.price_export_kwh);
    cJSON_AddStringToObject(root, "currency",         cfg.currency.c_str());
    char* s = cJSON_PrintUnformatted(root);
    std::string body(s);
    cJSON_free(s);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_tariff_save(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    std::string raw;
    if (read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "read error");
        return ESP_FAIL;
    }
    cJSON* j = cJSON_ParseWithLength(raw.c_str(), raw.size());
    if (!j) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "bad json");
        return ESP_FAIL;
    }
    TariffConfig cfg;
    ConfigStore::get().load_tariff(cfg);
    cJSON* item = cJSON_GetObjectItem(j, "price_import_kwh");
    if (cJSON_IsNumber(item)) cfg.price_import_kwh = item->valuedouble;
    item = cJSON_GetObjectItem(j, "price_export_kwh");
    if (cJSON_IsNumber(item)) cfg.price_export_kwh = item->valuedouble;
    item = cJSON_GetObjectItem(j, "currency");
    if (cJSON_IsString(item)) cfg.currency = item->valuestring;
    cJSON_Delete(j);
    bool ok = ConfigStore::get().save_tariff(cfg);
    return send_json(req, ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

// ── /api/config/meter (GET) ───────────────────────────────────────────────────

static cJSON* meter_profile_json(const MeterConfig& cfg)
{
    cJSON* profile = cJSON_CreateObject();
    cJSON_AddStringToObject(profile, "id", cfg.profile_id.c_str());
    cJSON_AddStringToObject(profile, "name", cfg.profile_name.c_str());
    cJSON_AddStringToObject(profile, "manufacturer", cfg.manufacturer.c_str());
    cJSON_AddStringToObject(profile, "model", cfg.model.c_str());
    cJSON_AddBoolToObject(profile, "pin_required", cfg.login_cmd.find("{PIN}") != std::string::npos);
    cJSON_AddStringToObject(profile, "login_cmd", cfg.login_cmd.c_str());
    cJSON_AddNumberToObject(profile, "login_wait_ms", cfg.login_wait_ms);
    cJSON* obis = cJSON_CreateObject();
    cJSON_AddStringToObject(obis, "import_wh", cfg.obis_import_wh.c_str());
    cJSON_AddStringToObject(obis, "export_wh", cfg.obis_export_wh.c_str());
    cJSON_AddStringToObject(obis, "power_net_w", cfg.obis_power_net_w.c_str());
    cJSON_AddStringToObject(obis, "power_import_w", cfg.obis_power_import_w.c_str());
    cJSON_AddStringToObject(obis, "power_export_w", cfg.obis_power_export_w.c_str());
    cJSON_AddStringToObject(obis, "voltage_l1_v", cfg.obis_voltage_l1_v.c_str());
    cJSON_AddStringToObject(obis, "voltage_l2_v", cfg.obis_voltage_l2_v.c_str());
    cJSON_AddStringToObject(obis, "voltage_l3_v", cfg.obis_voltage_l3_v.c_str());
    cJSON_AddStringToObject(obis, "current_l1_a", cfg.obis_current_l1_a.c_str());
    cJSON_AddStringToObject(obis, "current_l2_a", cfg.obis_current_l2_a.c_str());
    cJSON_AddStringToObject(obis, "current_l3_a", cfg.obis_current_l3_a.c_str());
    cJSON_AddStringToObject(obis, "frequency_hz", cfg.obis_frequency_hz.c_str());
    cJSON_AddStringToObject(obis, "pf_l1", cfg.obis_pf_l1.c_str());
    cJSON_AddItemToObject(profile, "obis", obis);
    return profile;
}

esp_err_t WebServer::handle_config_get(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    MeterConfig cfg;
    ConfigStore::get().load_meter(cfg);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "interval_s", cfg.interval_s);
    cJSON_AddBoolToObject  (root, "uart_debug",      cfg.uart_debug);
    cJSON_AddStringToObject(root, "login_cmd", cfg.login_cmd.c_str());
    cJSON_AddNumberToObject(root, "login_wait_ms", cfg.login_wait_ms);
    cJSON_AddBoolToObject  (root, "has_meter_pin", !cfg.meter_pin.empty());
    cJSON_AddStringToObject(root, "profile_id", cfg.profile_id.c_str());
    cJSON_AddStringToObject(root, "profile_name", cfg.profile_name.c_str());
    cJSON_AddStringToObject(root, "manufacturer", cfg.manufacturer.c_str());
    cJSON_AddStringToObject(root, "model", cfg.model.c_str());
    if (!cfg.previous_profile_json.empty()) {
        cJSON* previous = cJSON_Parse(cfg.previous_profile_json.c_str());
        if (cJSON_IsObject(previous)) cJSON_AddItemToObject(root, "previous_profile", previous);
        else cJSON_Delete(previous);
    }
    cJSON* obis = cJSON_CreateObject();
    cJSON_AddStringToObject(obis, "import_wh", cfg.obis_import_wh.c_str());
    cJSON_AddStringToObject(obis, "export_wh", cfg.obis_export_wh.c_str());
    cJSON_AddStringToObject(obis, "power_net_w", cfg.obis_power_net_w.c_str());
    cJSON_AddStringToObject(obis, "power_import_w", cfg.obis_power_import_w.c_str());
    cJSON_AddStringToObject(obis, "power_export_w", cfg.obis_power_export_w.c_str());
    cJSON_AddStringToObject(obis, "voltage_l1_v", cfg.obis_voltage_l1_v.c_str());
    cJSON_AddStringToObject(obis, "voltage_l2_v", cfg.obis_voltage_l2_v.c_str());
    cJSON_AddStringToObject(obis, "voltage_l3_v", cfg.obis_voltage_l3_v.c_str());
    cJSON_AddStringToObject(obis, "current_l1_a", cfg.obis_current_l1_a.c_str());
    cJSON_AddStringToObject(obis, "current_l2_a", cfg.obis_current_l2_a.c_str());
    cJSON_AddStringToObject(obis, "current_l3_a", cfg.obis_current_l3_a.c_str());
    cJSON_AddStringToObject(obis, "frequency_hz", cfg.obis_frequency_hz.c_str());
    cJSON_AddStringToObject(obis, "pf_l1", cfg.obis_pf_l1.c_str());
    cJSON_AddItemToObject(root, "obis", obis);
    char* s = cJSON_PrintUnformatted(root);
    std::string body(s);
    cJSON_free(s); cJSON_Delete(root);
    return send_json(req, body);
}

// ── /api/config/meter (POST) ──────────────────────────────────────────────────

esp_err_t WebServer::handle_config_save(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    std::string raw;
    if (read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "read error");
        return ESP_FAIL;
    }
    cJSON* j = cJSON_ParseWithLength(raw.c_str(), raw.size());
    if (!j) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "bad json");
        return ESP_FAIL;
    }
    MeterConfig cfg;
    ConfigStore::get().load_meter(cfg);
    cJSON* profile_id = cJSON_GetObjectItem(j, "profile_id");
    if (cJSON_IsString(profile_id) && profile_id->valuestring) {
        const std::string requested_id = profile_id->valuestring;
        if (requested_id != cfg.profile_id) {
            cJSON* previous = meter_profile_json(cfg);
            char* text = cJSON_PrintUnformatted(previous);
            cfg.previous_profile_json = text ? text : "{}";
            cJSON_free(text);
            cJSON_Delete(previous);
            cfg.profile_id = requested_id;
        }
    }
    auto set_profile_text = [j](const char* key, std::string& target) {
        cJSON* value = cJSON_GetObjectItem(j, key);
        if (cJSON_IsString(value) && value->valuestring) target = value->valuestring;
    };
    set_profile_text("profile_name", cfg.profile_name);
    set_profile_text("manufacturer", cfg.manufacturer);
    set_profile_text("model", cfg.model);
    if (cJSON_IsNumber(cJSON_GetObjectItem(j, "interval_s")))
        cfg.interval_s = static_cast<uint32_t>(cJSON_GetObjectItem(j, "interval_s")->valuedouble);
    {
        cJSON* item = cJSON_GetObjectItem(j, "uart_debug");
        if (cJSON_IsBool(item))        cfg.uart_debug = cJSON_IsTrue(item);
        else if (cJSON_IsNumber(item)) cfg.uart_debug = (item->valuedouble != 0.0);
    }
    {
        cJSON* item = cJSON_GetObjectItem(j, "login_cmd");
        if (cJSON_IsString(item)) cfg.login_cmd = item->valuestring;
    }
    {
        cJSON* item = cJSON_GetObjectItem(j, "login_wait_ms");
        if (cJSON_IsNumber(item)) cfg.login_wait_ms = static_cast<uint16_t>(item->valuedouble);
    }
    {
        cJSON* item = cJSON_GetObjectItem(j, "meter_pin");
        if (cJSON_IsString(item)) cfg.meter_pin = item->valuestring;
    }
    {
        cJSON* item = cJSON_GetObjectItem(j, "clear_meter_pin");
        if (cJSON_IsBool(item) && cJSON_IsTrue(item)) cfg.meter_pin.clear();
    }

    auto set_obis = [](cJSON* obj, const char* key, std::string& out) {
        if (!obj) return;
        cJSON* item = cJSON_GetObjectItem(obj, key);
        if (cJSON_IsString(item)) out = item->valuestring;
    };

    cJSON* obis = cJSON_GetObjectItem(j, "obis");
    if (cJSON_IsObject(obis)) {
        set_obis(obis, "import_wh", cfg.obis_import_wh);
        set_obis(obis, "export_wh", cfg.obis_export_wh);
        set_obis(obis, "power_net_w", cfg.obis_power_net_w);
        set_obis(obis, "power_import_w", cfg.obis_power_import_w);
        set_obis(obis, "power_export_w", cfg.obis_power_export_w);
        set_obis(obis, "voltage_l1_v", cfg.obis_voltage_l1_v);
        set_obis(obis, "voltage_l2_v", cfg.obis_voltage_l2_v);
        set_obis(obis, "voltage_l3_v", cfg.obis_voltage_l3_v);
        set_obis(obis, "current_l1_a", cfg.obis_current_l1_a);
        set_obis(obis, "current_l2_a", cfg.obis_current_l2_a);
        set_obis(obis, "current_l3_a", cfg.obis_current_l3_a);
        set_obis(obis, "frequency_hz", cfg.obis_frequency_hz);
        set_obis(obis, "pf_l1", cfg.obis_pf_l1);
    }

    // Flat keys are accepted for compatibility with simple clients.
    set_obis(j, "obis_import_wh", cfg.obis_import_wh);
    set_obis(j, "obis_export_wh", cfg.obis_export_wh);
    set_obis(j, "obis_power_net_w", cfg.obis_power_net_w);
    set_obis(j, "obis_power_import_w", cfg.obis_power_import_w);
    set_obis(j, "obis_power_export_w", cfg.obis_power_export_w);
    set_obis(j, "obis_voltage_l1_v", cfg.obis_voltage_l1_v);
    set_obis(j, "obis_voltage_l2_v", cfg.obis_voltage_l2_v);
    set_obis(j, "obis_voltage_l3_v", cfg.obis_voltage_l3_v);
    set_obis(j, "obis_current_l1_a", cfg.obis_current_l1_a);
    set_obis(j, "obis_current_l2_a", cfg.obis_current_l2_a);
    set_obis(j, "obis_current_l3_a", cfg.obis_current_l3_a);
    set_obis(j, "obis_frequency_hz", cfg.obis_frequency_hz);
    set_obis(j, "obis_pf_l1", cfg.obis_pf_l1);

    if (cfg.interval_s < 5) cfg.interval_s = 5;
    if (cfg.login_wait_ms < 20u) cfg.login_wait_ms = 20u;
    if (cfg.login_wait_ms > 5000u) cfg.login_wait_ms = 5000u;

    AppState::get().read_interval_s.store(cfg.interval_s);
    bool ok = ConfigStore::get().save_meter(cfg);
    cJSON_Delete(j);
    return send_json(req, ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

// ── /api/config/wifi (POST) ───────────────────────────────────────────────────

esp_err_t WebServer::handle_wifi_save(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    std::string raw;
    if (read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "read error");
        return ESP_FAIL;
    }
    cJSON* j = cJSON_ParseWithLength(raw.c_str(), raw.size());
    if (!j) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "bad json");
        return ESP_FAIL;
    }
    WifiConfig cfg;
    if (cJSON_IsString(cJSON_GetObjectItem(j, "ssid")))
        cfg.ssid = cJSON_GetObjectItem(j, "ssid")->valuestring;
    if (cJSON_IsString(cJSON_GetObjectItem(j, "password")))
        cfg.password = cJSON_GetObjectItem(j, "password")->valuestring;
    cJSON_Delete(j);
    bool ok = ConfigStore::get().save_wifi(cfg);
    if (!ok) return send_json(req, "{\"ok\":false,\"connected\":false}");
    const bool connected = WifiManager::get().reconnect(cfg.ssid, cfg.password);
    return send_json(req, connected ? "{\"ok\":true,\"connected\":true}"
                                    : "{\"ok\":true,\"connected\":false}");
}

esp_err_t WebServer::handle_auth_get(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    WebAuthConfig auth;
    ConfigStore::get().load_web_auth(auth);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "username", auth.username.c_str());
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{\"username\":\"admin\"}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_auth_save(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    std::string raw;
    if (read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "read error");
        return ESP_FAIL;
    }
    cJSON* json = cJSON_ParseWithLength(raw.c_str(), raw.size());
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "bad json");
        return ESP_FAIL;
    }
    WebAuthConfig current;
    ConfigStore::get().load_web_auth(current);
    auto* password = cJSON_GetObjectItem(json, "password");
    if (cJSON_IsString(password) && password->valuestring && password->valuestring[0] != '\0')
        current.password = password->valuestring;
    cJSON_Delete(json);
    return send_json(req, ConfigStore::get().save_web_auth(current)
        ? "{\"ok\":true}" : "{\"ok\":false}");
}

esp_err_t WebServer::handle_ui_language_get(httpd_req_t* req)
{
    UiConfig cfg;
    ConfigStore::get().load_ui(cfg);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "language", cfg.language.c_str());
    cJSON_AddStringToObject(root, "allowed_networks", cfg.allowed_networks.c_str());
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{\"language\":\"auto\",\"allowed_networks\":\"\"}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_ui_language_save(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    std::string raw;
    if (req->content_len > 128 || read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid request body");
        return ESP_FAIL;
    }
    cJSON* json = cJSON_ParseWithLength(raw.c_str(), raw.size());
    cJSON* language = json ? cJSON_GetObjectItem(json, "language") : nullptr;
    if (!cJSON_IsString(language) || !language->valuestring) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "language is required");
        return ESP_FAIL;
    }
    const std::string value = language->valuestring;
    cJSON_Delete(json);
    if (value != "auto" && value != "en" && value != "de" &&
        value != "nl" && value != "fr" && value != "pl") {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unsupported language");
        return ESP_FAIL;
    }
    UiConfig cfg;
    ConfigStore::get().load_ui(cfg);
    cfg.language = value;
    return send_json(req, ConfigStore::get().save_ui(cfg)
        ? "{\"ok\":true}" : "{\"ok\":false}");
}

esp_err_t WebServer::handle_access_networks_get(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    UiConfig cfg;
    ConfigStore::get().load_ui(cfg);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "allowed_networks", cfg.allowed_networks.c_str());
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{\"allowed_networks\":\"\"}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_access_networks_save(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    std::string raw;
    if (req->content_len > 1024 || read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid request body");
        return ESP_FAIL;
    }
    cJSON* json = cJSON_ParseWithLength(raw.c_str(), raw.size());
    cJSON* networks = json ? cJSON_GetObjectItem(json, "allowed_networks") : nullptr;
    if (!cJSON_IsString(networks) || !networks->valuestring) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "allowed_networks is required");
        return ESP_FAIL;
    }
    const std::string value = networks->valuestring;
    cJSON_Delete(json);
    if (!valid_cidr_list(value)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid CIDR network list");
        return ESP_FAIL;
    }
    UiConfig cfg;
    ConfigStore::get().load_ui(cfg);
    cfg.allowed_networks = value;
    return send_json(req, ConfigStore::get().save_ui(cfg)
        ? "{\"ok\":true}" : "{\"ok\":false}");
}

#if 0 // Temporarily disabled with HTTPS and ACME.
#if 0 // Temporarily disabled during IPv4-only HTTP recovery.
esp_err_t WebServer::handle_tls_config_get(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    TlsConfig cfg;
    ConfigStore::get().load_tls(cfg);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "mode", cfg.mode.c_str());
    cJSON_AddStringToObject(root, "fqdn", cfg.fqdn.c_str());
    cJSON_AddStringToObject(root, "email", cfg.email.c_str());
    cJSON_AddNumberToObject(root, "renewal_interval_days", cfg.renewal_interval_days);
    cJSON_AddBoolToObject(root, "acme_terms_accepted", cfg.acme_terms_accepted);
    cJSON_AddBoolToObject(root, "acme_staging", cfg.acme_staging);
    cJSON_AddBoolToObject(root, "self_signed_enabled", cfg.self_signed_enabled);
    cJSON_AddBoolToObject(root, "has_certificate", !cfg.certificate_pem.empty());
    cJSON_AddBoolToObject(root, "has_fallback_certificate", !cfg.fallback_certificate_pem.empty());
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_tls_config_save(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    std::string raw;
    if (read_body_limited(req, raw, 9000) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid or oversized TLS configuration");
        return ESP_FAIL;
    }
    cJSON* json = cJSON_ParseWithLength(raw.c_str(), raw.size());
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid JSON");
        return ESP_FAIL;
    }
    auto get_string = [json](const char* name) -> const char* {
        cJSON* value = cJSON_GetObjectItem(json, name);
        return cJSON_IsString(value) && value->valuestring ? value->valuestring : nullptr;
    };
    const char* mode = get_string("mode");
    const char* fqdn = get_string("fqdn");
    const char* email = get_string("email");
    const char* certificate = get_string("certificate_pem");
    const char* private_key = get_string("private_key_pem");
    cJSON* staging = cJSON_GetObjectItem(json, "acme_staging");
    cJSON* terms = cJSON_GetObjectItem(json, "acme_terms_accepted");
    const bool terms_accepted = cJSON_IsTrue(terms);
    cJSON* self_signed = cJSON_GetObjectItem(json, "self_signed_enabled");
    const std::string certificate_value = certificate ? certificate : "";
    const std::string private_key_value = private_key ? private_key : "";
    cJSON* interval = cJSON_GetObjectItem(json, "renewal_interval_days");
    if (!mode || !fqdn || !email || !certificate || !private_key || !cJSON_IsNumber(interval)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "missing TLS configuration fields");
        return ESP_FAIL;
    }
    TlsConfig cfg;
    ConfigStore::get().load_tls(cfg);
    const std::string previous_fqdn = cfg.fqdn;
    const bool previous_staging = cfg.acme_staging;
    cfg.mode = mode;
    cfg.fqdn = fqdn;
    cfg.email = email;
    cfg.renewal_interval_days = static_cast<uint16_t>(interval->valueint);
    if (staging) cfg.acme_staging = cJSON_IsTrue(staging);
    if (terms) cfg.acme_terms_accepted = terms_accepted;
    if (self_signed) cfg.self_signed_enabled = cJSON_IsTrue(self_signed);
    const bool fqdn_changed = cfg.fqdn != previous_fqdn;
    const bool acme_environment_changed = cfg.acme_staging != previous_staging;
    cJSON_Delete(json);

    if (fqdn_changed) {
        cfg.fallback_certificate_pem.clear();
        cfg.fallback_private_key_pem.clear();
    }
    if (acme_environment_changed) cfg.acme_account_url.clear();
    if ((fqdn_changed || acme_environment_changed) && certificate_value.empty()) {
        cfg.certificate_pem.clear();
        cfg.private_key_pem.clear();
        cfg.last_issued_epoch = 0;
    }

    if (!valid_fqdn(cfg.fqdn) || !valid_email(cfg.email) ||
        cfg.renewal_interval_days < 1 || cfg.renewal_interval_days > 60 ||
        (certificate_value.empty() != private_key_value.empty())) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid hostname, email, interval, or PEM key pair");
        return ESP_FAIL;
    }
    if (!certificate_value.empty()) {
        cfg.certificate_pem = certificate_value;
        cfg.private_key_pem = private_key_value;
        if (!valid_tls_pem_pair(cfg)) {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid PEM certificate/key pair");
            return ESP_FAIL;
        }
    }
    if (cfg.mode == "letsencrypt" && (cfg.fqdn.empty() || cfg.email.empty())) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Let's Encrypt requires FQDN and email");
        return ESP_FAIL;
    }
    if (cfg.mode == "letsencrypt" && !cfg.acme_terms_accepted) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Let's Encrypt terms must be accepted");
        return ESP_FAIL;
    }
    if (!ConfigStore::get().save_tls(cfg)) return send_json(req, "{\"ok\":false}");
    send_json(req, "{\"ok\":true,\"restarting\":true}");
    if (xTaskCreate(ota_reboot_task, "tls_reboot", 2048, nullptr, 5, nullptr) != pdPASS)
        ESP_LOGE(TAG, "TLS config saved; automatic reboot task could not be started");
    return ESP_OK;
}

esp_err_t WebServer::handle_acme_challenge(httpd_req_t* req)
{
    constexpr char prefix[] = "/.well-known/acme-challenge/";
    const std::string uri = req->uri;
    if (uri.rfind(prefix, 0) != 0) {
        httpd_resp_set_status(req, "404 Not Found");
        return httpd_resp_send(req, "Not Found", HTTPD_RESP_USE_STRLEN);
    }
    const std::string token = uri.substr(sizeof(prefix) - 1);
    if (token.empty() || token.find('/') != std::string::npos) {
        httpd_resp_set_status(req, "404 Not Found");
        return httpd_resp_send(req, "Not Found", HTTPD_RESP_USE_STRLEN);
    }
    std::string response;
    if (!acme_client::get_http01_challenge(token.c_str(), response)) {
        httpd_resp_set_status(req, "404 Not Found");
        return httpd_resp_send(req, "Not Found", HTTPD_RESP_USE_STRLEN);
    }
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    return httpd_resp_send(req, response.c_str(), static_cast<ssize_t>(response.size()));
}

esp_err_t WebServer::handle_acme_request(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    TlsConfig cfg;
    ConfigStore::get().load_tls(cfg);
    if (cfg.mode != "letsencrypt") {
        httpd_resp_set_status(req, "409 Conflict");
        return httpd_resp_send(req, "Let's Encrypt mode and HTTP-01 listener are required", HTTPD_RESP_USE_STRLEN);
    }
    if (!cfg.acme_terms_accepted) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Let's Encrypt terms must be accepted");
        return ESP_FAIL;
    }
    int expected = s_acme_issuance_state.load();
    if (expected == 1 || !s_acme_issuance_state.compare_exchange_strong(expected, 1)) {
        httpd_resp_set_status(req, "409 Conflict");
        return send_json(req, "{\"ok\":false,\"reason\":\"ACME request already running\"}");
    }
    if (xTaskCreate(acme_issue_task, "acme_issue", 16384, nullptr, 5, nullptr) != pdPASS) {
        s_acme_issuance_state.store(3);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "could not start ACME task");
        return ESP_FAIL;
    }
    return send_json(req, "{\"ok\":true,\"state\":\"requesting\"}");
}

esp_err_t WebServer::handle_acme_status(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    const int state = s_acme_issuance_state.load();
    const char* status = state == 1 ? "requesting" : state == 2 ? "issued" :
                         state == 3 ? "failed" : "idle";
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "state", status);
    cJSON_AddBoolToObject(root, "ok", state != 3);
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{\"state\":\"idle\",\"ok\":true}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}
#endif

#endif

// ── /api/start, /api/stop, /api/reboot, /api/reset_counters ──────────────────

esp_err_t WebServer::handle_start(httpd_req_t* req) {
    if (MeterTask::get().is_running()) {
        return send_json(req, "{\"ok\":false,\"reason\":\"already_running\"}");
    }
    // Single-shot: run once, then stop automatically
    AppState::get().continuous.store(false);
    MeterTask::get().start();
    return send_json(req, "{\"ok\":true,\"mode\":\"single\"}");
}

esp_err_t WebServer::handle_start_continuous(httpd_req_t* req) {
    if (MeterTask::get().is_running()) {
        return send_json(req, "{\"ok\":false,\"reason\":\"already_running\"}");
    }
    MeterTask::get().start_continuous();
    return send_json(req, "{\"ok\":true,\"mode\":\"continuous\"}");
}

esp_err_t WebServer::handle_stop(httpd_req_t* req) {
    MeterTask::get().stop();
    return send_json(req, "{\"ok\":true}");
}

esp_err_t WebServer::handle_reboot(httpd_req_t* req) {
    send_json(req, "{\"ok\":true}");
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
    return ESP_OK;
}

esp_err_t WebServer::handle_reset_counters(httpd_req_t* req) {
    AppState::get().io.reset();
    return send_json(req, "{\"ok\":true}");
}

// ── /api/meter (GET — raw JSON of last MeterData) ─────────────────────────────

esp_err_t WebServer::handle_meter(httpd_req_t* req) {
    return handle_status(req);  // reuse full status
}

esp_err_t WebServer::handle_ota_github_status(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    const int state = s_github_ota_state.load();
    const char* status = state == 1 ? "downloading" : state == 2 ? "restarting" :
                         state == 3 ? "failed" : "idle";
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "state", status);
    cJSON_AddBoolToObject(root, "ok", state != 3);
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{\"state\":\"idle\",\"ok\":true}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_ota_github(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    if (s_github_ota_state.load() == 1) {
        httpd_resp_set_status(req, "409 Conflict");
        return send_json(req, "{\"ok\":false,\"reason\":\"update_running\"}");
    }
    if (req->content_len == 0 || req->content_len > 1024) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid request body");
        return ESP_FAIL;
    }
    std::string raw;
    if (read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "read error");
        return ESP_FAIL;
    }
    cJSON* json = cJSON_ParseWithLength(raw.c_str(), raw.size());
    cJSON* asset_url = json ? cJSON_GetObjectItem(json, "url") : nullptr;
    if (!cJSON_IsString(asset_url) || !asset_url->valuestring) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "firmware URL is required");
        return ESP_FAIL;
    }
    const std::string url = asset_url->valuestring;
    cJSON_Delete(json);
    if (!is_allowed_github_firmware_url(url)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unsupported GitHub firmware asset");
        return ESP_FAIL;
    }

    char* task_url = static_cast<char*>(malloc(url.size() + 1));
    if (!task_url) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "out of memory");
        return ESP_FAIL;
    }
    memcpy(task_url, url.c_str(), url.size() + 1);
    s_github_ota_state.store(1);
    if (xTaskCreate(github_ota_task, "github_ota", 8192, task_url, 5, nullptr) != pdPASS) {
        free(task_url);
        s_github_ota_state.store(3);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "could not start OTA task");
        return ESP_FAIL;
    }
    return send_json(req, "{\"ok\":true,\"state\":\"downloading\"}");
}

// ── /api/ota (POST — binary firmware image) ───────────────────────────────────

esp_err_t WebServer::handle_ota(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    // Optional: save config from query params before flashing (e.g. ?interval_s=30&uart_debug=on)
    char qbuf[512] = {};
    if (httpd_req_get_url_query_str(req, qbuf, sizeof(qbuf)) == ESP_OK && qbuf[0] != '\0') {
        MeterConfig cfg;
        ConfigStore::get().load_meter(cfg);
        char val[128] = {};
        if (httpd_query_key_value(qbuf, "interval_s", val, sizeof(val)) == ESP_OK)
            cfg.interval_s = static_cast<uint32_t>(std::stoul(val));
        if (httpd_query_key_value(qbuf, "uart_debug", val, sizeof(val)) == ESP_OK)
            cfg.uart_debug = (val[0] == '1' || val[0] == 't' || val[0] == 'o');
        if (cfg.interval_s < 5) cfg.interval_s = 5;
        ConfigStore::get().save_meter(cfg);
        AppState::get().read_interval_s.store(cfg.interval_s);
        ESP_LOGI(TAG, "Config updated via OTA query params");
    }

    esp_ota_handle_t handle = 0;
    const esp_partition_t* update_part = esp_ota_get_next_update_partition(nullptr);
    if (!update_part) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "no OTA partition");
        return ESP_FAIL;
    }
    if (req->content_len == 0 || req->content_len > update_part->size) {
        ESP_LOGE(TAG, "OTA image size %u exceeds partition size %u",
                 static_cast<unsigned>(req->content_len),
                 static_cast<unsigned>(update_part->size));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "firmware image too large");
        return ESP_FAIL;
    }

    esp_err_t err = esp_ota_begin(update_part, OTA_WITH_SEQUENTIAL_WRITES, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "ota_begin failed");
        return ESP_FAIL;
    }

    char buf[1024];
    size_t remaining = req->content_len;
    bool write_ok = true;
    esp_err_t write_err = ESP_OK;
    while (remaining > 0 && write_ok) {
        int recv = httpd_req_recv(req, buf,
                   remaining < sizeof(buf) ? remaining : sizeof(buf));
        if (recv <= 0) {
            write_ok = false;
            ESP_LOGE(TAG, "OTA receive failed after %u bytes: %d",
                     static_cast<unsigned>(req->content_len - remaining), recv);
            break;
        }
        write_err = esp_ota_write(handle, buf, static_cast<size_t>(recv));
        if (write_err != ESP_OK) {
            write_ok = false;
            ESP_LOGE(TAG, "OTA write failed after %u bytes: %s",
                     static_cast<unsigned>(req->content_len - remaining),
                     esp_err_to_name(write_err));
            break;
        }
        remaining -= static_cast<size_t>(recv);
    }

    const esp_err_t end_err = write_ok ? esp_ota_end(handle) : esp_ota_abort(handle);
    if (!write_ok || end_err != ESP_OK) {
        ESP_LOGE(TAG, "OTA failed: write=%s end=%s", esp_err_to_name(write_err),
                 esp_err_to_name(end_err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "ota write failed");
        return ESP_FAIL;
    }

    if (esp_ota_set_boot_partition(update_part) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "set boot failed");
        return ESP_FAIL;
    }

    AppState::get().push_log("I", TAG, "OTA OK — Neustart …");
    send_json(req, "{\"ok\":true}");
    if (xTaskCreate(ota_reboot_task, "ota_reboot", 2048, nullptr, 5, nullptr) != pdPASS) {
        ESP_LOGE(TAG, "OTA succeeded, but reboot task could not be started");
    }
    return ESP_OK;
}

// ── /api/config/ha (GET + POST) ───────────────────────────────────────────────

esp_err_t WebServer::handle_ha_config_get(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    HaConfig cfg;
    ConfigStore::get().load_ha(cfg);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject  (root, "enabled",      cfg.enabled);
    cJSON_AddStringToObject(root, "broker_uri",   cfg.broker_uri.c_str());
    cJSON_AddStringToObject(root, "username",     cfg.username.c_str());
    // password intentionally omitted from GET response
    cJSON_AddStringToObject(root, "device_name",  cfg.device_name.c_str());
    cJSON_AddStringToObject(root, "ha_prefix",    cfg.ha_prefix.c_str());
    cJSON_AddBoolToObject  (root, "connected",    MqttManager::get().is_connected());
    char* s = cJSON_PrintUnformatted(root);
    std::string body(s); cJSON_free(s); cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_ha_config_save(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    std::string raw;
    if (read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "read error");
        return ESP_FAIL;
    }
    cJSON* j = cJSON_ParseWithLength(raw.c_str(), raw.size());
    if (!j) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "bad json");
        return ESP_FAIL;
    }
    HaConfig cfg;
    ConfigStore::get().load_ha(cfg); // preserve password if not re-sent
    auto getstr = [&](const char* key, std::string& out) {
        cJSON* item = cJSON_GetObjectItem(j, key);
        if (cJSON_IsString(item)) out = item->valuestring;
    };
    auto getbool = [&](const char* key, bool& out) {
        cJSON* item = cJSON_GetObjectItem(j, key);
        if (cJSON_IsBool(item))        out = cJSON_IsTrue(item);
        else if (cJSON_IsNumber(item)) out = (item->valuedouble != 0.0);
    };
    getbool("enabled",      cfg.enabled);
    getstr ("broker_uri",   cfg.broker_uri);
    getstr ("username",     cfg.username);
    getstr ("password",     cfg.password);
    getstr ("device_name",  cfg.device_name);
    getstr ("ha_prefix",    cfg.ha_prefix);
    cJSON_Delete(j);
    bool ok = ConfigStore::get().save_ha(cfg);
    if (ok) MqttManager::get().init(cfg);  // apply immediately, no reboot needed
    return send_json(req, ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

esp_err_t WebServer::handle_favicon(httpd_req_t* req) {
    httpd_resp_set_type(req, "image/svg+xml");
    httpd_resp_set_hdr(req, "Cache-Control", "max-age=86400");
    return httpd_resp_send(req, kLogoSvg, HTTPD_RESP_USE_STRLEN);
}

} // namespace app
