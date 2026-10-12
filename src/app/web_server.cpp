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
#if ESP_IDF_VERSION_MAJOR >= 6
#include "esp_app_desc.h"
#endif
#include "esp_app_format.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "mbedtls/base64.h"
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"
#if CONFIG_LWIP_IPV6
#include "lwip/ip6_addr.h"
#endif

// Local HTTPS (port 443) runs next to HTTP (port 80) on IDF 6 builds only.
#if ESP_IDF_VERSION_MAJOR >= 6 && CONFIG_ESP_HTTPS_SERVER_ENABLE
#define SMLEASY_LOCAL_HTTPS 1
#include "esp_https_server.h"
#include "esp_random.h"
#include "acme_client.hpp"
#include "tls_certs.hpp"
#else
#define SMLEASY_LOCAL_HTTPS 0
#endif

// Values read from Home Assistant (REST / MQTT) for the dashboard; IDF 6 (S3) builds only.
#if ESP_IDF_VERSION_MAJOR >= 6
#define SMLEASY_EXT_DATA 1
#include "ext_data.hpp"
#else
#define SMLEASY_EXT_DATA 0
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <map>
#include <mutex>
#include <string>
#include <vector>

static const char* TAG = "WebServer";
#if !CONFIG_IDF_TARGET_ESP32S3
static std::atomic<int> s_github_ota_state{0};
#endif

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

[[maybe_unused]] static esp_err_t read_body_limited(httpd_req_t* req, std::string& out, size_t limit)
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
#if CONFIG_LWIP_IPV6
        if (inet_pton(AF_INET6, address.c_str(), network.data()) != 1 ||
            parsed_prefix < 0 || parsed_prefix > 128) return false;
        family = AF_INET6;
#else
        return false;
#endif
    }
    prefix = static_cast<int>(parsed_prefix);
    return true;
}

static bool prefix_matches(const uint8_t* address, const uint8_t* network, int prefix)
{
    const int full_bytes = prefix / 8;
    const int remaining_bits = prefix % 8;
    if (full_bytes && memcmp(address, network, full_bytes) != 0) return false;
    if (remaining_bits) {
        const uint8_t mask = static_cast<uint8_t>(0xffu << (8 - remaining_bits));
        if ((address[full_bytes] & mask) != (network[full_bytes] & mask)) return false;
    }
    return true;
}

#if CONFIG_LWIP_IPV6
static bool is_ipv4_mapped(const uint8_t* address)
{
    static constexpr uint8_t mapped_prefix[12] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff
    };
    return memcmp(address, mapped_prefix, sizeof(mapped_prefix)) == 0;
}
#endif

static bool cidr_matches(const sockaddr_storage& peer, const std::string& cidr)
{
    int family = 0, prefix = 0;
    std::array<uint8_t, 16> network{};
    if (!parse_cidr(cidr, family, network, prefix)) return false;

    if (peer.ss_family == AF_INET && family == AF_INET) {
        const auto* address = reinterpret_cast<const sockaddr_in*>(&peer);
        return prefix_matches(
            reinterpret_cast<const uint8_t*>(&address->sin_addr.s_addr),
            network.data(), prefix);
    }

#if CONFIG_LWIP_IPV6
    if (peer.ss_family == AF_INET6) {
        const auto* address = reinterpret_cast<const sockaddr_in6*>(&peer);
        const uint8_t* bytes = address->sin6_addr.s6_addr;
        if (family == AF_INET6) {
            return prefix_matches(bytes, network.data(), prefix);
        }
        if (family == AF_INET && is_ipv4_mapped(bytes)) {
            return prefix_matches(bytes + 12, network.data(), prefix);
        }
    }
#endif

    return false;
}

static bool is_private_ipv4(uint32_t ip)
{
    return (ip & 0xff000000u) == 0x0a000000u ||
           (ip & 0xfff00000u) == 0xac100000u ||
           (ip & 0xffff0000u) == 0xc0a80000u;
}

static bool is_private_address(const sockaddr_storage& peer)
{
    if (peer.ss_family == AF_INET) {
        const auto* address = reinterpret_cast<const sockaddr_in*>(&peer);
        const uint32_t ip = ntohl(address->sin_addr.s_addr);
        return is_private_ipv4(ip);
    }
#if CONFIG_LWIP_IPV6
    if (peer.ss_family == AF_INET6) {
        const auto* address = reinterpret_cast<const sockaddr_in6*>(&peer);
        const uint8_t* bytes = address->sin6_addr.s6_addr;
        if (is_ipv4_mapped(bytes)) {
            const uint32_t ip = (static_cast<uint32_t>(bytes[12]) << 24) |
                                (static_cast<uint32_t>(bytes[13]) << 16) |
                                (static_cast<uint32_t>(bytes[14]) << 8) |
                                static_cast<uint32_t>(bytes[15]);
            return is_private_ipv4(ip);
        }
        const bool loopback = bytes[0] == 0 && bytes[1] == 0 &&
            bytes[2] == 0 && bytes[3] == 0 && bytes[4] == 0 && bytes[5] == 0 &&
            bytes[6] == 0 && bytes[7] == 0 && bytes[8] == 0 && bytes[9] == 0 &&
            bytes[10] == 0 && bytes[11] == 0 && bytes[12] == 0 &&
            bytes[13] == 0 && bytes[14] == 0 && bytes[15] == 1;
        const bool unique_local = (bytes[0] & 0xfeu) == 0xfcu;
        const bool link_local = bytes[0] == 0xfeu && (bytes[1] & 0xc0u) == 0x80u;
        return loopback || unique_local || link_local;
    }
#endif
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
#if CONFIG_LWIP_IPV6
    if (peer.ss_family == AF_INET6) {
        const auto* address = reinterpret_cast<const sockaddr_in6*>(&peer);
        if (app::WifiManager::get().is_in_local_ip6_prefix(address->sin6_addr.s6_addr)) return ESP_OK;
    }
#endif
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
    char peer_text[46] = "?";
    if (peer.ss_family == AF_INET6) {
#if CONFIG_LWIP_IPV6
        inet_ntop(AF_INET6, &reinterpret_cast<const sockaddr_in6*>(&peer)->sin6_addr,
                  peer_text, sizeof(peer_text));
#endif
    } else if (peer.ss_family == AF_INET) {
        inet_ntop(AF_INET, &reinterpret_cast<const sockaddr_in*>(&peer)->sin_addr,
                  peer_text, sizeof(peer_text));
    }
    ESP_LOGW(TAG, "blocked web client %s outside allowed networks", peer_text);
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

#if !CONFIG_IDF_TARGET_ESP32S3
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
    http_cfg.buffer_size_tx = 2048;
    http_cfg.max_redirection_count = 5;
    http_cfg.keep_alive_enable = true;
    http_cfg.crt_bundle_attach = esp_crt_bundle_attach;

    ESP_LOGI(TAG, "Starting GitHub OTA from %s", url);
#if ESP_IDF_VERSION_MAJOR >= 6
    esp_https_ota_config_t ota_cfg{};
    ota_cfg.http_config = &http_cfg;
    const esp_err_t err = esp_https_ota(&ota_cfg);
#else
    const esp_err_t err = esp_https_ota(&http_cfg);
#endif
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
#endif

#if SMLEASY_LOCAL_HTTPS
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

// 0 idle, 1 requesting, 2 issued, 3 failed
static std::atomic<int> s_acme_state{0};
static std::atomic<int> s_acme_last_error{ESP_OK};
// A certificate request and the pre-flight checks both own port 80 and must not overlap.
static std::atomic<bool> s_acme_busy{false};

// Pre-flight checks: 0 idle, 1 running, 2 passed, 3 failed
struct CheckEntry {
    std::string state{"pending"};
    std::string detail;
};
static constexpr char kCheckIds[3][9] = {"internet", "dns", "http"};
static CheckEntry s_checks[3];
static std::string s_checks_fqdn;
static std::string s_terms_url;
static int64_t s_checks_done_epoch = 0;
static std::atomic<int> s_checks_state{0};
static std::mutex s_checks_mutex;
static constexpr char kStagingDirectory[] = "https://acme-staging-v02.api.letsencrypt.org/directory";

static void set_check(int index, const char* state, const std::string& detail)
{
    {
        std::lock_guard<std::mutex> lock(s_checks_mutex);
        s_checks[index].state = state;
        s_checks[index].detail = detail;
    }
    if (strcmp(state, "running") != 0 && strcmp(state, "pending") != 0)
        acme_client::add_log("Check %s: %s - %s", kCheckIds[index], state, detail.c_str());
}

#if CONFIG_LWIP_IPV6
static bool parse_ipv6(const std::string& text, in6_addr& out)
{
    return inet_pton(AF_INET6, text.c_str(), &out) == 1;
}
static bool is_global_ipv6(const std::string& text)
{
    in6_addr address{};
    return parse_ipv6(text, address) && (address.s6_addr[0] & 0xe0u) == 0x20u;
}
static bool same_ipv6(const std::string& a, const std::string& b)
{
    in6_addr x{}, y{};
    return parse_ipv6(a, x) && parse_ipv6(b, y) && memcmp(&x, &y, sizeof(x)) == 0;
}
#else
static bool is_global_ipv6(const std::string&) { return false; }
static bool same_ipv6(const std::string&, const std::string&) { return false; }
#endif

// Keeps the leaf and the first intermediate; clients already trust the root, and the
// 20 KB NVS partition cannot hold a longer chain.
static std::string trim_certificate_chain(const std::string& pem)
{
    constexpr char kEnd[] = "-----END CERTIFICATE-----";
    size_t end = 0;
    for (int i = 0; i < 2; ++i) {
        end = pem.find(kEnd, end);
        if (end == std::string::npos) return pem;
        end += sizeof(kEnd) - 1;
    }
    return pem.substr(0, end) + "\n";
}

static std::string peer_address_text(httpd_req_t* req)
{
    sockaddr_storage peer{};
    socklen_t peer_len = sizeof(peer);
    char text[46] = "?";
    if (getpeername(httpd_req_to_sockfd(req), reinterpret_cast<sockaddr*>(&peer), &peer_len) == 0) {
        if (peer.ss_family == AF_INET) {
            inet_ntop(AF_INET, &reinterpret_cast<const sockaddr_in*>(&peer)->sin_addr, text, sizeof(text));
        }
#if CONFIG_LWIP_IPV6
        else if (peer.ss_family == AF_INET6) {
            inet_ntop(AF_INET6, &reinterpret_cast<const sockaddr_in6*>(&peer)->sin6_addr, text, sizeof(text));
        }
#endif
    }
    return text;
}

static std::atomic<bool> s_redirect_mode{false};
// The redirect only starts after a stable uptime, so a faulty HTTPS setup cannot lock out the dashboard.
static constexpr int64_t kRedirectDelayUs = 5LL * 60 * 1000 * 1000;

static bool https_redirect_wanted()
{
    if (esp_timer_get_time() < kRedirectDelayUs) return false;
    app::TlsConfig cfg;
    app::ConfigStore::get().load_tls(cfg);
    // No PEM parsing here: start() runs on the small main task stack.
    if (cfg.certificate_pem.compare(0, 10, "-----BEGIN") != 0 ||
        cfg.private_key_pem.compare(0, 10, "-----BEGIN") != 0) return false;
    // Staging certificates are not trusted by browsers, so the dashboard stays reachable on HTTP.
    return !(cfg.mode == "letsencrypt" && cfg.acme_staging);
}

static esp_err_t handle_https_redirect(httpd_req_t* req)
{
    char host[96] = {};
    if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK || host[0] == '\0') {
        app::TlsConfig cfg;
        app::ConfigStore::get().load_tls(cfg);
        snprintf(host, sizeof(host), "%s", cfg.fqdn.c_str());
    }
    // Strip the port and accept only hostname/IP characters (no header injection, no open redirect).
    std::string name;
    bool bracket = host[0] == '[';
    for (const char* c = host; *c; ++c) {
        if (bracket) {
            name += *c;
            if (*c == ']') break;
        } else if (*c == ':') {
            break;
        } else {
            name += *c;
        }
    }
    for (char ch : name) {
        if (!isalnum(static_cast<unsigned char>(ch)) && ch != '.' && ch != '-' &&
            ch != '[' && ch != ']' && ch != ':' && ch != '%') return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad host");
    }
    if (name.empty()) return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad host");
    std::string uri = req->uri;
    if (uri.empty() || uri[0] != '/' || uri.find_first_of("\r\n") != std::string::npos) uri = "/";
    const std::string location = "https://" + name + uri;
    httpd_resp_set_status(req, "308 Permanent Redirect");
    httpd_resp_set_hdr(req, "Location", location.c_str());
    return httpd_resp_send(req, nullptr, 0);
}

// Switches port 80 between dashboard and redirect once the uptime/certificate state calls for it.
static void redirect_switch_task(void*)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        if (s_acme_busy.load() || !app::WifiManager::get().is_sta_connected()) continue;
        if (https_redirect_wanted() == s_redirect_mode.load()) continue;
        ESP_LOGI(TAG, "switching port 80 to %s", s_redirect_mode.load() ? "dashboard" : "HTTPS redirect");
        auto& web_server = app::WebServer::get();
        web_server.stop_http();
        if (web_server.start(80) != ESP_OK) ESP_LOGE(TAG, "failed to restart port 80");
    }
}

static void acme_issue_task(void*)
{
    auto& web_server = app::WebServer::get();
    acme_client::init();
    acme_client::clear_log();
    // Let the HTTP response of the triggering request leave before port 80 is swapped.
    vTaskDelay(pdMS_TO_TICKS(1000));
    web_server.stop_http();
    // The TLS server sessions and the ACME TLS client together exhaust the heap.
    web_server.stop_https();
    acme_client::add_log("Free heap before request: %u bytes", static_cast<unsigned>(esp_get_free_heap_size()));

    esp_err_t err = web_server.start_acme_challenge_listener();
    if (err == ESP_OK) acme_client::add_log("Dashboard on port 80 paused; challenge listener active");
    else acme_client::add_log("Port 80 could not be opened for the challenge");
    app::TlsConfig cfg;
    app::ConfigStore::get().load_tls(cfg);
    acme_client::Result result;
    if (err == ESP_OK) {
        acme_client::Request request;
        request.directory_url = cfg.acme_staging
            ? "https://acme-staging-v02.api.letsencrypt.org/directory"
            : "https://acme-v02.api.letsencrypt.org/directory";
        request.fqdn = cfg.fqdn;
        request.email = cfg.email;
        request.account_key_pem = cfg.acme_account_key_pem;
        request.account_url = cfg.acme_account_url;
        request.terms_accepted = cfg.acme_terms_accepted;
        err = acme_client::issue_http01(request, result);
    }
    web_server.stop_acme_challenge_listener();

    if (!result.account_key_pem.empty()) cfg.acme_account_key_pem = result.account_key_pem;
    if (!result.account_url.empty()) cfg.acme_account_url = result.account_url;
    bool stored = false;
    if (err == ESP_OK) {
        cfg.certificate_pem = trim_certificate_chain(result.certificate_chain_pem);
        cfg.private_key_pem = result.private_key_pem;
        // The self-signed fallback is regenerated on demand and only costs NVS space now.
        cfg.fallback_certificate_pem.clear();
        cfg.fallback_private_key_pem.clear();
        cfg.last_issued_epoch = static_cast<int64_t>(time(nullptr));
        ESP_LOGI(TAG, "ACME certificate chain: %u bytes received, %u bytes stored",
                 static_cast<unsigned>(result.certificate_chain_pem.size()),
                 static_cast<unsigned>(cfg.certificate_pem.size()));
        stored = app::ConfigStore::get().save_tls(cfg);
        if (!stored) {
            err = ESP_FAIL;
            acme_client::add_log("Storing the certificate failed (%u bytes)",
                                 static_cast<unsigned>(cfg.certificate_pem.size()));
        }
    }
    if (!stored && (!result.account_key_pem.empty() || !result.account_url.empty())) {
        // Keep the ACME account even when the certificate could not be stored, so retries reuse it.
        app::TlsConfig account_only;
        app::ConfigStore::get().load_tls(account_only);
        account_only.acme_account_key_pem = cfg.acme_account_key_pem;
        account_only.acme_account_url = cfg.acme_account_url;
        app::ConfigStore::get().save_tls(account_only);
    }

    if (web_server.start(80) != ESP_OK)
        ESP_LOGE(TAG, "failed to restore HTTP dashboard after ACME");
    acme_client::add_log("Dashboard on port 80 restored");
    if (stored) {
        acme_client::add_log("Certificate stored (%u bytes); restarting HTTPS",
                             static_cast<unsigned>(cfg.certificate_pem.size()));
        web_server.start_https_async(true);
        app::AppState::get().push_log("I", TAG, "Let's Encrypt certificate issued");
        // Test results and log are single use; a stale "passed" must not unlock another request.
        {
            std::lock_guard<std::mutex> lock(s_checks_mutex);
            for (auto& check : s_checks) check = CheckEntry{};
            s_checks_fqdn.clear();
            s_checks_done_epoch = 0;
            s_checks_state.store(0);
        }
        acme_client::clear_log();
        s_acme_last_error.store(ESP_OK);
        s_acme_state.store(2);
    } else {
        acme_client::add_log("Request failed: %s", esp_err_to_name(err));
        ESP_LOGE(TAG, "ACME HTTP-01 issuance failed: %s", esp_err_to_name(err));
        app::AppState::get().push_log("E", TAG, "Let's Encrypt issuance failed");
        s_acme_last_error.store(err);
        s_acme_state.store(3);
    }
    s_acme_busy.store(false);
    vTaskDelete(nullptr);
}

static bool start_acme_issuance()
{
    bool expected = false;
    if (!s_acme_busy.compare_exchange_strong(expected, true)) return false;
    s_acme_state.store(1);
    if (xTaskCreate(acme_issue_task, "acme_issue", 16384, nullptr, 5, nullptr) != pdPASS) {
        s_acme_last_error.store(ESP_ERR_NO_MEM);
        s_acme_state.store(3);
        s_acme_busy.store(false);
        return false;
    }
    return true;
}

static void acme_check_task(void*)
{
    auto& web_server = app::WebServer::get();
    acme_client::init();
    acme_client::clear_log();
    app::TlsConfig cfg;
    app::ConfigStore::get().load_tls(cfg);
    {
        std::lock_guard<std::mutex> lock(s_checks_mutex);
        for (auto& check : s_checks) check = CheckEntry{};
        s_checks_fqdn = cfg.fqdn;
    }
    s_checks_state.store(1);

    // 1: internet access, clock, reachability of the CA
    set_check(0, "running", "");
    acme_client::DirectoryInfo info;
    bool internet_ok = false;
    if (time(nullptr) < 1700000000) {
        set_check(0, "fail", "Clock is not synchronised yet (NTP); try again in a minute");
    } else if (acme_client::fetch_directory_info(kStagingDirectory, info) != ESP_OK) {
        set_check(0, "fail", "Let's Encrypt is not reachable from this device (internet, DNS or IPv6 route)");
    } else {
        internet_ok = true;
        {
            std::lock_guard<std::mutex> lock(s_checks_mutex);
            s_terms_url = info.terms_url;
        }
        set_check(0, "pass", "Internet and clock are fine; Let's Encrypt is reachable");
    }

    // 2: public DNS must point to this device
    bool dns_ok = false;
    if (!internet_ok) {
        set_check(1, "skipped", "Skipped because the previous check failed");
    } else {
        set_check(1, "running", "");
        std::vector<std::string> aaaa, a;
        const esp_err_t error6 = acme_client::resolve_public(cfg.fqdn, "AAAA", aaaa);
        const esp_err_t error4 = acme_client::resolve_public(cfg.fqdn, "A", a);
        std::vector<std::string> own;
        for (const auto& address : app::WifiManager::get().get_ip6_addresses())
            if (is_global_ipv6(address)) own.push_back(address);
        bool matches = false;
        for (const auto& record : aaaa)
            for (const auto& address : own) matches = matches || same_ipv6(record, address);
        if (error6 != ESP_OK && error4 != ESP_OK) {
            set_check(1, "fail", "The public DNS lookup (dns.google) failed");
        } else if (matches) {
            dns_ok = true;
            set_check(1, "pass", "The AAAA record points to this device (" + aaaa[0] + ")");
        } else if (!aaaa.empty()) {
            set_check(1, "fail", "The AAAA record " + aaaa[0] + " does not match this device" +
                      (own.empty() ? " (no global IPv6 address yet)" : " (" + own[0] + ")"));
        } else if (!a.empty()) {
            dns_ok = true;
            set_check(1, "warn", "No AAAA record; A record " + a[0] +
                      " found. Port 80 of that IPv4 address must be forwarded to this device");
        } else {
            set_check(1, "fail", "No AAAA or A record found for " + cfg.fqdn);
        }
    }

    // 3: reachability from the internet, proven by a Let's Encrypt staging validation
    bool http_ok = false;
    if (!internet_ok || !dns_ok) {
        set_check(2, "skipped", "Skipped because a previous check failed");
    } else {
        set_check(2, "running", "");
        web_server.stop_http();
        esp_err_t err = web_server.start_acme_challenge_listener();
        if (err == ESP_OK) {
            acme_client::Request request;
            request.directory_url = kStagingDirectory;
            request.fqdn = cfg.fqdn;
            request.email = cfg.email;
            request.terms_accepted = cfg.acme_terms_accepted;
            request.validate_only = true;
            acme_client::Result result;
            err = acme_client::issue_http01(request, result);
        }
        web_server.stop_acme_challenge_listener();
        if (web_server.start(80) != ESP_OK) ESP_LOGE(TAG, "failed to restore HTTP dashboard after check");
        if (err == ESP_OK) {
            http_ok = true;
            set_check(2, "pass", "Let's Encrypt reached port 80 on this device and validated the hostname");
        } else {
            const std::string reason = acme_client::last_error();
            set_check(2, "fail", reason.empty() ? std::string("Validation failed: ") + esp_err_to_name(err) : reason);
        }
    }

    const bool passed = internet_ok && dns_ok && http_ok;
    {
        std::lock_guard<std::mutex> lock(s_checks_mutex);
        s_checks_done_epoch = passed ? static_cast<int64_t>(time(nullptr)) : 0;
    }
    s_checks_state.store(passed ? 2 : 3);
    s_acme_busy.store(false);
    vTaskDelete(nullptr);
}

static bool start_acme_checks()
{
    bool expected = false;
    if (!s_acme_busy.compare_exchange_strong(expected, true)) return false;
    s_checks_state.store(1);
    if (xTaskCreate(acme_check_task, "acme_check", 16384, nullptr, 5, nullptr) != pdPASS) {
        s_checks_state.store(3);
        s_acme_busy.store(false);
        return false;
    }
    return true;
}

static void acme_renewal_task(void*)
{
    for (;;) {
        const uint32_t delay_seconds = 3600u + (esp_random() % 3600u);
        vTaskDelay(pdMS_TO_TICKS(delay_seconds * 1000u));
        app::TlsConfig cfg;
        app::ConfigStore::get().load_tls(cfg);
        if (cfg.mode != "letsencrypt" || !cfg.acme_terms_accepted ||
            !app::WifiManager::get().is_sta_connected()) continue;
        const time_t now = time(nullptr);
        if (now < 1700000000 || cfg.last_issued_epoch <= 0) continue;
        const int64_t interval_seconds = static_cast<int64_t>(cfg.renewal_interval_days) * 86400;
        if (now < cfg.last_issued_epoch + interval_seconds) continue;
        start_acme_issuance();
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
    http_cfg.max_uri_handlers = 56;
    http_cfg.stack_size = SMLEASY_LOCAL_HTTPS ? 10240 : 8192;
#if SMLEASY_LOCAL_HTTPS
    http_cfg.max_open_sockets = 6;
#endif
    http_cfg.lru_purge_enable = true;
    http_cfg.recv_wait_timeout = 60;
    http_cfg.send_wait_timeout = 60;
    http_cfg.open_fn = dashboard_network_filter;
#if SMLEASY_LOCAL_HTTPS
    // With a browser-trusted certificate, port 80 only redirects to HTTPS.
    const bool redirect_only = port == 80 && https_redirect_wanted();
    s_redirect_mode.store(redirect_only);
    if (redirect_only) {
        http_cfg.uri_match_fn = httpd_uri_match_wildcard;
        http_cfg.max_open_sockets = 3;
        http_cfg.stack_size = 6144;
        http_cfg.recv_wait_timeout = 10;
        http_cfg.send_wait_timeout = 10;
    }
#endif
    if (httpd_start(&server_, &http_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "HTTP dashboard start failed");
        return ESP_FAIL;
    }

#if SMLEASY_LOCAL_HTTPS
    if (redirect_only) {
        const httpd_uri_t get_uri = { "/*", HTTP_GET, handle_https_redirect, nullptr };
        const httpd_uri_t post_uri = { "/*", HTTP_POST, handle_https_redirect, nullptr };
        httpd_register_uri_handler(server_, &get_uri);
        httpd_register_uri_handler(server_, &post_uri);
        ESP_LOGI(TAG, "HTTP port 80 redirects to HTTPS");
    } else
#endif
    register_routes(server_);

#if CONFIG_LWIP_IPV6
    ESP_LOGI(TAG, "HTTP dashboard listening on IPv4/IPv6 port %u", port);
#else
    ESP_LOGI(TAG, "HTTP recovery dashboard started on IPv4 port %u", port);
#endif
#if SMLEASY_LOCAL_HTTPS
    if (port == 80) {
        acme_client::init();
        if (!https_server_) start_https_async();
        static std::atomic<bool> renewal_started{false};
        bool expected = false;
        if (renewal_started.compare_exchange_strong(expected, true) &&
            xTaskCreate(acme_renewal_task, "acme_renew", 4096, nullptr, 3, nullptr) != pdPASS)
            ESP_LOGW(TAG, "ACME renewal task could not be created");
        static std::atomic<bool> switch_started{false};
        expected = false;
        if (switch_started.compare_exchange_strong(expected, true) &&
            xTaskCreate(redirect_switch_task, "port80_switch", 4096, nullptr, 3, nullptr) != pdPASS)
            ESP_LOGW(TAG, "port 80 switch task could not be created");
    }
#endif
    return ESP_OK;
}

void WebServer::register_routes(httpd_handle_t server)
{
    const httpd_uri_t routes[] = {
        { "/",                    HTTP_GET,  handle_root,          nullptr },
        { "/config",              HTTP_GET,  handle_config_page,   nullptr },
        { "/style.css",           HTTP_GET,  handle_style_css,     nullptr },
        { "/app.js",              HTTP_GET,  handle_app_js,        nullptr },
        { "/api/status",          HTTP_GET,  handle_status,        nullptr },
        { "/api/log",             HTTP_GET,  handle_log,           nullptr },
        { "/api/history",         HTTP_GET,  handle_history,       nullptr },
        { "/api/stats",           HTTP_GET,  handle_stats,         nullptr },
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
        { "/api/config/daily-baseline", HTTP_GET, handle_daily_baseline_get, nullptr },
        { "/api/config/daily-baseline", HTTP_POST, handle_daily_baseline_save, nullptr },
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
#if SMLEASY_LOCAL_HTTPS
        { "/api/config/tls",      HTTP_GET,  handle_tls_config_get,    nullptr },
        { "/api/config/tls",      HTTP_POST, handle_tls_config_save,   nullptr },
        { "/api/acme/request",    HTTP_POST, handle_acme_request,      nullptr },
        { "/api/acme/check",      HTTP_POST, handle_acme_check,        nullptr },
        { "/api/acme/status",     HTTP_GET,  handle_acme_status,       nullptr },
        { "/tls",                 HTTP_GET,  handle_tls_page,          nullptr },
#endif
#if SMLEASY_EXT_DATA
        { "/ext",                 HTTP_GET,  handle_ext_page,          nullptr },
        { "/api/config/ext",      HTTP_GET,  handle_ext_config_get,    nullptr },
        { "/api/config/ext",      HTTP_POST, handle_ext_config_save,   nullptr },
        { "/api/ext/values",      HTTP_GET,  handle_ext_values,        nullptr },
        { "/api/ext/refresh",     HTTP_POST, handle_ext_refresh,       nullptr },
        { "/api/ext/automation.yaml", HTTP_GET, handle_ext_automation,  nullptr },
#endif
    };
    for (const auto& r : routes) httpd_register_uri_handler(server, &r);
}

// ── local HTTPS (port 443, parallel to HTTP) ────────────────────────────────────

#if SMLEASY_LOCAL_HTTPS
static std::atomic<bool> s_https_busy{false};

void WebServer::https_task(void* restart)
{
    auto& web_server = WebServer::get();
    if (restart) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        web_server.stop_https();
    }
    if (web_server.start_https() != ESP_OK)
        ESP_LOGW(TAG, "HTTPS listener unavailable; HTTP dashboard keeps running");
    s_https_busy.store(false);
    vTaskDelete(nullptr);
}

void WebServer::start_https_async(bool restart)
{
    bool expected = false;
    if (!s_https_busy.compare_exchange_strong(expected, true)) return;
    if (xTaskCreate(https_task, "https_start", 10240,
                    restart ? reinterpret_cast<void*>(1) : nullptr, 4, nullptr) != pdPASS) {
        s_https_busy.store(false);
        ESP_LOGW(TAG, "HTTPS start task could not be created");
    }
}

esp_err_t WebServer::start_https()
{
    if (https_server_) return ESP_OK;

    TlsConfig tls;
    ConfigStore::get().load_tls(tls);
    std::string certificate = tls.certificate_pem;
    std::string private_key = tls.private_key_pem;
    bool self_signed = false;
    if (!tls_certs::valid_pair(certificate, private_key)) {
        if (!tls_certs::valid_pair(tls.fallback_certificate_pem, tls.fallback_private_key_pem)) {
            const std::string fqdn = valid_fqdn(tls.fqdn) ? tls.fqdn : "smleasy.local";
            if (!tls_certs::generate_self_signed(fqdn, tls.fallback_certificate_pem,
                                                 tls.fallback_private_key_pem) ||
                !ConfigStore::get().save_tls(tls)) {
                ESP_LOGE(TAG, "no usable TLS certificate, HTTPS disabled");
                return ESP_FAIL;
            }
            ESP_LOGI(TAG, "generated self-signed certificate for %s", fqdn.c_str());
        }
        certificate = tls.fallback_certificate_pem;
        private_key = tls.fallback_private_key_pem;
        self_signed = true;
    }

    // The IDF default macro omits trailing members; silence only that known warning here.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    httpd_ssl_config_t ssl_cfg = HTTPD_SSL_CONFIG_DEFAULT();
#pragma GCC diagnostic pop
    ssl_cfg.httpd.max_uri_handlers = 56;
    ssl_cfg.httpd.stack_size = 10240;
    ssl_cfg.httpd.max_open_sockets = 3;
    ssl_cfg.httpd.lru_purge_enable = true;
    ssl_cfg.httpd.recv_wait_timeout = 30;
    ssl_cfg.httpd.send_wait_timeout = 30;
    // The HTTP server already owns the default control port.
    ssl_cfg.httpd.ctrl_port = ESP_HTTPD_DEF_CTRL_PORT + 1;
    ssl_cfg.httpd.open_fn = dashboard_network_filter;
    ssl_cfg.servercert = reinterpret_cast<const uint8_t*>(certificate.c_str());
    ssl_cfg.servercert_len = certificate.size() + 1;
    ssl_cfg.prvtkey_pem = reinterpret_cast<const uint8_t*>(private_key.c_str());
    ssl_cfg.prvtkey_len = private_key.size() + 1;

    httpd_handle_t handle = nullptr;
    const esp_err_t err = httpd_ssl_start(&handle, &ssl_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTPS server start failed: %s", esp_err_to_name(err));
        return err;
    }
    register_routes(handle);
    https_server_ = handle;
    ESP_LOGI(TAG, "HTTPS dashboard listening on port %u (%s certificate)",
             ssl_cfg.port_secure, self_signed ? "self-signed" : "configured");
    return ESP_OK;
}

void WebServer::stop_https()
{
    if (https_server_) {
        httpd_ssl_stop(https_server_);
        https_server_ = nullptr;
    }
}

esp_err_t WebServer::handle_tls_config_get(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    TlsConfig cfg;
    ConfigStore::get().load_tls(cfg);
    const bool has_manual = tls_certs::valid_pair(cfg.certificate_pem, cfg.private_key_pem);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "mode", cfg.mode.c_str());
    cJSON_AddStringToObject(root, "fqdn", cfg.fqdn.c_str());
    cJSON_AddStringToObject(root, "email", cfg.email.c_str());
    cJSON_AddBoolToObject(root, "acme_terms_accepted", cfg.acme_terms_accepted);
    cJSON_AddBoolToObject(root, "acme_staging", cfg.acme_staging);
    cJSON_AddNumberToObject(root, "renewal_interval_days", cfg.renewal_interval_days);
    cJSON_AddNumberToObject(root, "last_issued_epoch", static_cast<double>(cfg.last_issued_epoch));
    const char* source = !has_manual ? "self-signed"
        : (cfg.mode == "letsencrypt" && cfg.last_issued_epoch > 0) ? "letsencrypt" : "manual";
    cJSON_AddStringToObject(root, "certificate_source", source);
    cJSON_AddBoolToObject(root, "has_certificate", has_manual);
    int64_t not_after = 0;
    std::string cert_cn;
    bool cert_staging = false;
    if (has_manual && tls_certs::certificate_info(cfg.certificate_pem, not_after, cert_cn, cert_staging)) {
        const int64_t now = static_cast<int64_t>(time(nullptr));
        cJSON_AddStringToObject(root, "cert_cn", cert_cn.c_str());
        cJSON_AddBoolToObject(root, "cert_staging", cert_staging);
        cJSON_AddNumberToObject(root, "cert_not_after_epoch", static_cast<double>(not_after));
        cJSON_AddNumberToObject(root, "cert_days_left",
                                now > 1700000000 ? static_cast<double>((not_after - now) / 86400) : -1);
    }
    cJSON_AddBoolToObject(root, "has_fallback_certificate", !cfg.fallback_certificate_pem.empty());
    cJSON_AddBoolToObject(root, "https_active", WebServer::get().is_https_running());
    cJSON_AddNumberToObject(root, "https_port", 443);
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
    auto get_string = [json](const char* name) -> std::string {
        cJSON* value = cJSON_GetObjectItem(json, name);
        return cJSON_IsString(value) && value->valuestring ? value->valuestring : "";
    };
    const std::string fqdn = get_string("fqdn");
    const std::string email = get_string("email");
    const std::string mode_value = get_string("mode");
    const std::string certificate = get_string("certificate_pem");
    const std::string private_key = get_string("private_key_pem");
    const bool clear_certificate = cJSON_IsTrue(cJSON_GetObjectItem(json, "clear_certificate"));
    const cJSON* terms_item = cJSON_GetObjectItem(json, "acme_terms_accepted");
    const cJSON* staging_item = cJSON_GetObjectItem(json, "acme_staging");
    const cJSON* interval_item = cJSON_GetObjectItem(json, "renewal_interval_days");
    const bool terms_given = cJSON_IsBool(terms_item);
    const bool terms_accepted = cJSON_IsTrue(terms_item);
    const bool staging_given = cJSON_IsBool(staging_item);
    const bool staging = cJSON_IsTrue(staging_item);
    const int interval_days = cJSON_IsNumber(interval_item) ? interval_item->valueint : 0;
    cJSON_Delete(json);

    if ((!fqdn.empty() && !valid_fqdn(fqdn)) || certificate.empty() != private_key.empty() ||
        (!email.empty() && !valid_email(email)) ||
        (!mode_value.empty() && mode_value != "manual" && mode_value != "letsencrypt") ||
        (interval_days != 0 && (interval_days < 1 || interval_days > 60))) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "invalid hostname, email, mode, interval or incomplete PEM key pair");
        return ESP_FAIL;
    }
    if (!certificate.empty() && !tls_certs::valid_pair(certificate, private_key)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid PEM certificate/key pair");
        return ESP_FAIL;
    }

    TlsConfig cfg;
    ConfigStore::get().load_tls(cfg);
    const bool fqdn_changed = cfg.fqdn != fqdn;
    const bool staging_changed = staging_given && cfg.acme_staging != staging;
    if (fqdn_changed) {
        cfg.fallback_certificate_pem.clear();
        cfg.fallback_private_key_pem.clear();
    }
    cfg.fqdn = fqdn;
    cfg.email = email;
    if (!mode_value.empty()) cfg.mode = mode_value;
    if (terms_given) cfg.acme_terms_accepted = terms_accepted;
    if (staging_given) cfg.acme_staging = staging;
    if (interval_days != 0) cfg.renewal_interval_days = static_cast<uint16_t>(interval_days);
    // An ACME account belongs to one directory; a changed host or environment invalidates the old certificate.
    if (staging_changed) cfg.acme_account_url.clear();
    if ((fqdn_changed || staging_changed) && certificate.empty()) {
        cfg.certificate_pem.clear();
        cfg.private_key_pem.clear();
        cfg.last_issued_epoch = 0;
    }
    if (cfg.mode == "letsencrypt" &&
        (!valid_fqdn(cfg.fqdn) || !valid_email(cfg.email) || !cfg.acme_terms_accepted)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "Let's Encrypt needs a valid hostname, email and accepted terms");
        return ESP_FAIL;
    }
    if (clear_certificate) {
        cfg.certificate_pem.clear();
        cfg.private_key_pem.clear();
    }
    if (!certificate.empty()) {
        cfg.certificate_pem = certificate;
        cfg.private_key_pem = private_key;
    }
    if (!ConfigStore::get().save_tls(cfg)) return send_json(req, "{\"ok\":false}");
    // Only certificate-relevant changes need a new HTTPS listener; ACME settings alone do not.
    const bool restart_https = fqdn_changed || staging_changed || clear_certificate ||
                               !certificate.empty();
    const esp_err_t sent = send_json(req, restart_https
        ? "{\"ok\":true,\"https_restarting\":true}" : "{\"ok\":true,\"https_restarting\":false}");
    if (restart_https) WebServer::get().start_https_async(true);
    return sent;
}

esp_err_t WebServer::handle_acme_challenge(httpd_req_t* req)
{
    constexpr char prefix[] = "/.well-known/acme-challenge/";
    const std::string uri = req->uri;
    std::string response;
    const std::string token = uri.rfind(prefix, 0) == 0 ? uri.substr(sizeof(prefix) - 1) : "";
    const bool known = !token.empty() && token.find('/') == std::string::npos &&
                       acme_client::get_http01_challenge(token.c_str(), response);
    acme_client::add_log("Challenge request from %s: %s", peer_address_text(req).c_str(),
                         known ? "answered" : "unknown token (404)");
    if (!known) {
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
    if (cfg.mode != "letsencrypt" || !cfg.acme_terms_accepted ||
        !valid_fqdn(cfg.fqdn) || !valid_email(cfg.email)) {
        httpd_resp_set_status(req, "409 Conflict");
        return send_json(req, "{\"ok\":false,\"reason\":\"Let's Encrypt mode with hostname, email and accepted terms is required\"}");
    }
    bool checks_passed = false;
    {
        std::lock_guard<std::mutex> lock(s_checks_mutex);
        const int64_t now = static_cast<int64_t>(time(nullptr));
        checks_passed = s_checks_state.load() == 2 && s_checks_fqdn == cfg.fqdn &&
                        s_checks_done_epoch > 0 && now - s_checks_done_epoch <= 3600;
    }
    if (!checks_passed) {
        httpd_resp_set_status(req, "409 Conflict");
        return send_json(req, "{\"ok\":false,\"reason\":\"Run the pre-flight checks first; they must pass for this hostname within the last hour\"}");
    }
    if (!start_acme_issuance()) {
        httpd_resp_set_status(req, "409 Conflict");
        return send_json(req, "{\"ok\":false,\"reason\":\"Another certificate request or check is already running\"}");
    }
    return send_json(req, "{\"ok\":true,\"state\":\"requesting\"}");
}

esp_err_t WebServer::handle_acme_check(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    TlsConfig cfg;
    ConfigStore::get().load_tls(cfg);
    if (cfg.mode != "letsencrypt" || !cfg.acme_terms_accepted ||
        !valid_fqdn(cfg.fqdn) || !valid_email(cfg.email)) {
        httpd_resp_set_status(req, "409 Conflict");
        return send_json(req, "{\"ok\":false,\"reason\":\"Accept the terms and enter a valid hostname and email first\"}");
    }
    if (!start_acme_checks()) {
        httpd_resp_set_status(req, "409 Conflict");
        return send_json(req, "{\"ok\":false,\"reason\":\"Another certificate request or check is already running\"}");
    }
    return send_json(req, "{\"ok\":true,\"checks_state\":\"running\"}");
}

esp_err_t WebServer::handle_acme_status(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    int state = s_acme_state.load();
    // "issued" is reported once so a reloaded page does not show old results.
    if (state == 2 && !s_acme_busy.load()) s_acme_state.compare_exchange_strong(state, 0);
    const char* status = state == 1 ? "requesting" : state == 2 ? "issued" :
                         state == 3 ? "failed" : "idle";
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "state", status);
    cJSON_AddBoolToObject(root, "ok", state != 3);
    const int last_error = s_acme_last_error.load();
    cJSON_AddStringToObject(root, "error", last_error == ESP_OK ? "" : esp_err_to_name(last_error));
    cJSON_AddBoolToObject(root, "busy", s_acme_busy.load());
    {
        std::lock_guard<std::mutex> lock(s_checks_mutex);
        const int checks = s_checks_state.load();
        cJSON_AddStringToObject(root, "checks_state", checks == 1 ? "running" : checks == 2 ? "passed" :
                                checks == 3 ? "failed" : "idle");
        cJSON_AddStringToObject(root, "checks_fqdn", s_checks_fqdn.c_str());
        const int64_t now = static_cast<int64_t>(time(nullptr));
        cJSON_AddNumberToObject(root, "checks_age_s", s_checks_done_epoch > 0 ? static_cast<double>(now - s_checks_done_epoch) : -1);
        cJSON_AddStringToObject(root, "terms_url", s_terms_url.c_str());
        cJSON* list = cJSON_AddArrayToObject(root, "checks");
        for (int i = 0; i < 3; ++i) {
            cJSON* entry = cJSON_CreateObject();
            cJSON_AddStringToObject(entry, "id", kCheckIds[i]);
            cJSON_AddStringToObject(entry, "state", s_checks[i].state.c_str());
            cJSON_AddStringToObject(entry, "detail", s_checks[i].detail.c_str());
            cJSON_AddItemToArray(list, entry);
        }
    }
    cJSON* log = cJSON_AddArrayToObject(root, "log");
    for (const auto& line : acme_client::get_log()) cJSON_AddItemToArray(log, cJSON_CreateString(line.c_str()));
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{\"state\":\"idle\",\"ok\":true}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::start_acme_challenge_listener()
{
    if (challenge_server_) return ESP_OK;
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.server_port = 80;
    cfg.ctrl_port = ESP_HTTPD_DEF_CTRL_PORT + 2;
    cfg.max_open_sockets = 3;
    cfg.stack_size = 6144;
    cfg.lru_purge_enable = true;
    cfg.recv_wait_timeout = 10;
    cfg.send_wait_timeout = 10;
    cfg.uri_match_fn = httpd_uri_match_wildcard;
    // No network filter: the ACME validation servers connect from the public internet.
    if (httpd_start(&challenge_server_, &cfg) != ESP_OK) {
        challenge_server_ = nullptr;
        ESP_LOGE(TAG, "ACME challenge listener could not bind port 80");
        return ESP_FAIL;
    }
    const httpd_uri_t uri = { "/.well-known/acme-challenge/*", HTTP_GET, handle_acme_challenge, nullptr };
    httpd_register_uri_handler(challenge_server_, &uri);
    ESP_LOGI(TAG, "ACME challenge listener active on port 80");
    return ESP_OK;
}

void WebServer::stop_acme_challenge_listener()
{
    if (challenge_server_) {
        httpd_stop(challenge_server_);
        challenge_server_ = nullptr;
    }
}

void WebServer::stop_http()
{
    if (server_) {
        httpd_stop(server_);
        server_ = nullptr;
    }
}
#else
void WebServer::start_https_async(bool) {}
void WebServer::stop_https() {}
esp_err_t WebServer::start_acme_challenge_listener() { return ESP_ERR_NOT_SUPPORTED; }
void WebServer::stop_acme_challenge_listener() {}
void WebServer::stop_http()
{
    if (server_) {
        httpd_stop(server_);
        server_ = nullptr;
    }
}
#endif

void WebServer::stop() {
    stop_acme_challenge_listener();
    stop_https();
    stop_http();
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

esp_err_t WebServer::handle_tls_page(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, kTlsHtml, HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebServer::handle_style_css(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, kStyleCss, HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebServer::handle_app_js(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    // Stream from flash in small chunks: a ~160 KB heap copy per request fails (and can reset the device) under memory pressure.
#if CONFIG_IDF_TARGET_ESP32S3
    constexpr char kPrefix[] = "window.GITHUB_OTA_ENABLED = false;\n";
#else
    constexpr char kPrefix[] = "window.GITHUB_OTA_ENABLED = true;\n";
#endif
    esp_err_t err = httpd_resp_send_chunk(req, kPrefix, sizeof(kPrefix) - 1);
    constexpr size_t kChunk = 2048;
    const size_t total = strlen(kAppJs);
    for (size_t offset = 0; err == ESP_OK && offset < total; offset += kChunk) {
        err = httpd_resp_send_chunk(req, kAppJs + offset, static_cast<ssize_t>(std::min(kChunk, total - offset)));
    }
    if (err != ESP_OK) return err;
    return httpd_resp_send_chunk(req, nullptr, 0);
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
    cJSON_AddBoolToObject(root, "https", WebServer::get().is_https_running());
    cJSON_AddBoolToObject(root, "tls_supported", SMLEASY_LOCAL_HTTPS != 0);
    cJSON_AddBoolToObject(root, "ext_supported", SMLEASY_EXT_DATA != 0);
#if SMLEASY_EXT_DATA
    cJSON_AddStringToObject(root, "ha_input_base", MqttManager::input_topic_base().c_str());
#endif
    cJSON_AddStringToObject(root, "ip6_sta", WifiManager::get().is_sta_connected()
                                               ? WifiManager::get().get_ip6().c_str() : "");
    cJSON* ip6_addresses = cJSON_AddArrayToObject(root, "ip6_addresses");
    if (ip6_addresses && WifiManager::get().is_sta_connected()) {
        for (const auto& address : WifiManager::get().get_ip6_addresses()) {
            cJSON_AddItemToArray(ip6_addresses, cJSON_CreateString(address.c_str()));
        }
    }
    cJSON_AddStringToObject(root, "ip_ap",   WifiManager::get().is_ap_active()
                                               ? "192.168.4.1" : "");
    cJSON_AddStringToObject(root, "ssid_sta", WifiManager::get().sta_ssid().c_str());
    cJSON_AddStringToObject(root, "ssid_ap",  WifiManager::get().is_ap_active()
                                               ? WifiManager::get().ap_ssid().c_str() : "");
    cJSON_AddNumberToObject(root, "uptime_s", (double)(esp_timer_get_time() / 1000000LL));
    cJSON_AddNumberToObject(root, "tx_pin", MeterTask::get().tx_pin());
    cJSON_AddNumberToObject(root, "rx_pin", MeterTask::get().rx_pin());
#if ESP_IDF_VERSION_MAJOR >= 6
    cJSON_AddStringToObject(root, "app_version", esp_app_get_description()->version);
#else
    cJSON_AddStringToObject(root, "app_version", esp_ota_get_app_description()->version);
#endif
    cJSON_AddStringToObject(root, "idf_version", IDF_VER);

    // Daily totals use the persisted first-reading baseline; month totals remain in RAM.
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
    cJSON_AddNumberToObject(root, "week_import_kwh",         stats.week_import_kwh);
    cJSON_AddNumberToObject(root, "week_export_kwh",         stats.week_export_kwh);
    cJSON_AddNumberToObject(root, "cost_week",
        stats.week_cost_final + stats.today_import_kwh * tariff.price_import_kwh - stats.today_export_kwh * tariff.price_export_kwh);
    cJSON_AddNumberToObject(root, "cost_month",
        stats.month_cost_final + stats.today_import_kwh * tariff.price_import_kwh - stats.today_export_kwh * tariff.price_export_kwh);
    if (stats.have_house) {
        cJSON_AddNumberToObject(root, "today_house_kwh", stats.today_house_kwh);
        cJSON_AddNumberToObject(root, "week_house_kwh",  stats.week_house_kwh);
        cJSON_AddNumberToObject(root, "month_house_kwh", stats.month_house_kwh);
    }
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

// ── /api/stats (days / weeks / months incl. PV and battery) ─────────────────

esp_err_t WebServer::handle_stats(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    TariffConfig tariff;
    ConfigStore::get().load_tariff(tariff);
    return send_json(req, EnergyStats::get().stats_json(tariff.price_import_kwh,
                                                        tariff.price_export_kwh, tariff.currency.c_str()));
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

// ── /api/config/daily-baseline ────────────────────────────────────────────────

esp_err_t WebServer::handle_daily_baseline_get(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    const DailyBaselineSnapshot baseline = EnergyStats::get().daily_baseline();
    const StatsSnapshot stats = EnergyStats::get().snapshot();
    const MeterData meter = AppState::get().get_meter_data();
    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "has_baseline", baseline.available);
    cJSON_AddBoolToObject(root, "time_synced", stats.time_synced);
    cJSON_AddBoolToObject(root, "has_reading",
        meter.populated && meter.has_fwd_active_wh && meter.has_rev_active_wh);
    cJSON_AddNumberToObject(root, "import_start_wh", baseline.import_wh);
    cJSON_AddNumberToObject(root, "export_start_wh", baseline.export_wh);
    cJSON_AddNumberToObject(root, "current_import_wh", meter.fwd_active_wh);
    cJSON_AddNumberToObject(root, "current_export_wh", meter.rev_active_wh);
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_daily_baseline_save(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    std::string raw;
    if (req->content_len == 0 || req->content_len > 512 || read_body(req, raw) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid daily baseline request");
        return ESP_FAIL;
    }
    cJSON* json = cJSON_ParseWithLength(raw.c_str(), raw.size());
    cJSON* import_value = json ? cJSON_GetObjectItem(json, "import_start_wh") : nullptr;
    cJSON* export_value = json ? cJSON_GetObjectItem(json, "export_start_wh") : nullptr;
    if (!cJSON_IsNumber(import_value) || !cJSON_IsNumber(export_value) ||
        import_value->valuedouble <= 0 || export_value->valuedouble <= 0 ||
        import_value->valuedouble > INT32_MAX || export_value->valuedouble > INT32_MAX ||
        import_value->valuedouble != static_cast<int32_t>(import_value->valuedouble) ||
        export_value->valuedouble != static_cast<int32_t>(export_value->valuedouble)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "daily baselines must be positive whole Wh values");
        return ESP_FAIL;
    }
    const int32_t import_start_wh = static_cast<int32_t>(import_value->valuedouble);
    const int32_t export_start_wh = static_cast<int32_t>(export_value->valuedouble);
    cJSON_Delete(json);

    if (!EnergyStats::get().set_daily_baseline(import_start_wh, export_start_wh)) {
        httpd_resp_set_status(req, "409 Conflict");
        return send_json(req, "{\"ok\":false,\"reason\":\"requires_synced_clock_current_reading_valid_values_and_nvs\"}");
    }
    return send_json(req, "{\"ok\":true}");
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
#if CONFIG_IDF_TARGET_ESP32S3
    httpd_resp_set_status(req, "403 Forbidden");
    return send_json(req, "{\"ok\":false,\"state\":\"disabled\"}");
#else
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
#endif
}

esp_err_t WebServer::handle_ota_github(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
#if CONFIG_IDF_TARGET_ESP32S3
    httpd_resp_set_status(req, "403 Forbidden");
    return send_json(req, "{\"ok\":false,\"reason\":\"disabled\"}");
#else
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
#endif
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

esp_err_t WebServer::handle_ext_page(httpd_req_t* req) {
    REQUIRE_AUTH(req);
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, kExtHtml, HTTPD_RESP_USE_STRLEN);
}

#if SMLEASY_EXT_DATA
// ── /api/config/ext, /api/ext/values, /api/ext/refresh ────────────────────────

static bool valid_entity_id(const std::string& id)
{
    const size_t dot = id.find('.');
    if (dot == std::string::npos || dot == 0 || dot + 1 >= id.size() || id.size() > 128) return false;
    for (size_t i = 0; i < id.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(id[i]);
        if (i == dot) continue;
        if (!(std::islower(ch) || std::isdigit(ch) || ch == '_')) return false;
    }
    return id.find('.', dot + 1) == std::string::npos;
}

static bool valid_mqtt_topic(const std::string& topic)
{
    if (topic.empty() || topic.size() > 128) return false;
    for (unsigned char ch : topic) {
        if (ch < 0x21 || ch == 0x7f || ch == '+' || ch == '#') return false;
    }
    return true;
}

static bool valid_push_name(const std::string& name)
{
    if (name.empty() || name.size() > 32) return false;
    for (const char c : name)
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) return false;
    return true;
}

static bool valid_json_path(const std::string& path)
{
    if (path.size() > 40) return false;
    for (unsigned char ch : path) {
        if (!(std::isalnum(ch) || ch == '_' || ch == '-' || ch == '.')) return false;
    }
    return true;
}

static bool printable_text(const std::string& text, size_t max_size)
{
    if (text.size() > max_size) return false;
    for (unsigned char ch : text) {
        if (ch < 0x20 || ch == 0x7f) return false;
    }
    return true;
}

static esp_err_t ext_reject(httpd_req_t* req, const std::string& reason)
{
    httpd_resp_set_status(req, "400 Bad Request");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "ok", false);
    cJSON_AddStringToObject(root, "reason", reason.c_str());
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{\"ok\":false}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_ext_config_get(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    ExtDataConfig cfg;
    ConfigStore::get().load_ext(cfg);
    HaConfig ha;
    ConfigStore::get().load_ha(ha);
    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "enabled", cfg.enabled);
    cJSON_AddStringToObject(root, "ha_url", cfg.ha_url.c_str());
    cJSON_AddBoolToObject(root, "token_set", !cfg.ha_token.empty());
    cJSON_AddNumberToObject(root, "poll_s", cfg.poll_s);
    cJSON_AddBoolToObject(root, "mqtt_enabled", ha.enabled);
    cJSON_AddBoolToObject(root, "mqtt_connected", MqttManager::get().is_connected());
    cJSON* slots = cJSON_AddArrayToObject(root, "slots");
    for (const auto& slot : cfg.slots) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "label", slot.label.c_str());
        cJSON_AddStringToObject(item, "source", slot.source.c_str());
        cJSON_AddStringToObject(item, "ref", slot.ref.c_str());
        cJSON_AddStringToObject(item, "unit", slot.unit.c_str());
        cJSON_AddStringToObject(item, "key", slot.key.c_str());
        cJSON_AddStringToObject(item, "role", slot.role.c_str());
        cJSON_AddItemToArray(slots, item);
    }
    char* text = cJSON_PrintUnformatted(root);
    std::string body(text ? text : "{}");
    cJSON_free(text);
    cJSON_Delete(root);
    return send_json(req, body);
}

esp_err_t WebServer::handle_ext_config_save(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    std::string raw;
    if (read_body_limited(req, raw, 6000) != ESP_OK) return ext_reject(req, "request too large or unreadable");
    cJSON* json = cJSON_ParseWithLength(raw.c_str(), raw.size());
    if (!json) return ext_reject(req, "invalid JSON");

    ExtDataConfig cfg;
    ConfigStore::get().load_ext(cfg);   // keeps the stored token unless a new one is sent
    cfg.slots.clear();
    const cJSON* enabled = cJSON_GetObjectItem(json, "enabled");
    if (cJSON_IsBool(enabled)) cfg.enabled = cJSON_IsTrue(enabled);
    const cJSON* url = cJSON_GetObjectItem(json, "ha_url");
    if (cJSON_IsString(url) && url->valuestring) cfg.ha_url = url->valuestring;
    const cJSON* token = cJSON_GetObjectItem(json, "ha_token");
    if (cJSON_IsString(token) && token->valuestring && token->valuestring[0]) cfg.ha_token = token->valuestring;
    if (cJSON_IsTrue(cJSON_GetObjectItem(json, "clear_token"))) cfg.ha_token.clear();
    const cJSON* poll = cJSON_GetObjectItem(json, "poll_s");
    if (cJSON_IsNumber(poll)) cfg.poll_s = static_cast<uint16_t>(std::clamp(poll->valueint, 10, 600));

    while (!cfg.ha_url.empty() && cfg.ha_url.back() == '/') cfg.ha_url.pop_back();
    if (!cfg.ha_url.empty() &&
        (!(cfg.ha_url.rfind("http://", 0) == 0 || cfg.ha_url.rfind("https://", 0) == 0) ||
         cfg.ha_url.find(' ') != std::string::npos || !printable_text(cfg.ha_url, 200))) {
        cJSON_Delete(json);
        return ext_reject(req, "Home Assistant URL must start with http:// or https://");
    }
    if (!printable_text(cfg.ha_token, 512)) {
        cJSON_Delete(json);
        return ext_reject(req, "invalid access token");
    }

    const cJSON* slots = cJSON_GetObjectItem(json, "slots");
    const int slot_count = cJSON_IsArray(slots) ? cJSON_GetArraySize(slots) : 0;
    auto text_of = [](const cJSON* item, const char* name) {
        const cJSON* value = cJSON_GetObjectItem(item, name);
        return std::string(cJSON_IsString(value) && value->valuestring ? value->valuestring : "");
    };
    bool needs_rest = false;
    for (int i = 0; i < slot_count; ++i) {
        const cJSON* item = cJSON_GetArrayItem(slots, i);
        ExtSlot slot;
        slot.label = text_of(item, "label");
        slot.source = text_of(item, "source");
        slot.ref = text_of(item, "ref");
        slot.unit = text_of(item, "unit");
        slot.key = text_of(item, "key");
        slot.role = text_of(item, "role");
        if (slot.ref.empty() && slot.label.empty()) continue;   // empty form row
        const std::string where = "row " + std::to_string(i + 1) + ": ";
        if (cfg.slots.size() >= ExtDataConfig::kMaxSlots) {
            cJSON_Delete(json);
            return ext_reject(req, "at most 8 values are supported");
        }
        if (slot.source != "rest" && slot.source != "mqtt" && slot.source != "push") {
            cJSON_Delete(json);
            return ext_reject(req, where + "unknown source");
        }
        if (!printable_text(slot.label, 40) || !printable_text(slot.unit, 12)) {
            cJSON_Delete(json);
            return ext_reject(req, where + "label or unit too long");
        }
        if (slot.source == "rest" && !valid_entity_id(slot.ref)) {
            cJSON_Delete(json);
            return ext_reject(req, where + "entity id must look like sensor.battery_soc");
        }
        if (slot.source == "push" && !valid_push_name(slot.ref)) {
            cJSON_Delete(json);
            return ext_reject(req, where + "short name must be 1-32 characters a-z, 0-9 or _");
        }
        if ((slot.source == "mqtt" || slot.source == "push") && !valid_json_path(slot.key)) {
            cJSON_Delete(json);
            return ext_reject(req, where + "invalid JSON key");
        }
        if (slot.source == "mqtt" && (!valid_mqtt_topic(slot.ref) || !valid_json_path(slot.key))) {
            cJSON_Delete(json);
            return ext_reject(req, where + "invalid MQTT topic (no wildcards) or JSON key");
        }
        needs_rest = needs_rest || slot.source == "rest";
        cfg.slots.push_back(std::move(slot));
    }
    cJSON_Delete(json);
    if (cfg.enabled && needs_rest && (cfg.ha_url.empty() || cfg.ha_token.empty()))
        return ext_reject(req, "REST values need the Home Assistant URL and an access token");

    if (!ConfigStore::get().save_ext(cfg)) return send_json(req, "{\"ok\":false}");
    ExtData::get().apply_config();
    return send_json(req, "{\"ok\":true}");
}

esp_err_t WebServer::handle_ext_automation(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    ExtDataConfig cfg;
    ConfigStore::get().load_ext(cfg);
    const std::string base = MqttManager::input_topic_base();
    std::string yaml =
        "# SMLEasy: Home-Assistant-Automation (ab HA 2024.10)\n"
        "# Ersetze jede Zeile 'entity_id' durch deine Entit\xC3\xA4t und f\xC3\xBCge die Automation in\n"
        "# Einstellungen > Automatisierungen > Neue Automatisierung > YAML ein.\n"
        "alias: SMLEasy Werte senden\n"
        "mode: queued\n"
        "triggers:\n";
    std::string actions;
    int count = 0;
    for (const auto& slot : cfg.slots) {
        if (slot.source != "push" || slot.ref.empty()) continue;
        ++count;
        yaml += "  - trigger: state\n    entity_id: sensor." + slot.ref + "   # " + slot.label + "\n    id: " + slot.ref + "\n";
        actions +=
            "      - conditions:\n"
            "          - condition: trigger\n"
            "            id: " + slot.ref + "\n"
            "        sequence:\n"
            "          - action: mqtt.publish\n"
            "            data:\n"
            "              topic: " + base + slot.ref + "\n"
            "              payload: \"{{ trigger.to_state.state }}\"\n"
            "              retain: true\n";
    }
    if (!count) {
        yaml += "  []   # Keine Werte mit Quelle 'HA sendet' gespeichert.\n";
    } else {
        yaml +=
            "conditions:\n"
            "  - condition: template\n"
            "    value_template: \"{{ trigger.to_state.state | is_number }}\"\n"
            "actions:\n"
            "  - choose:\n" + actions;
    }
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, yaml.c_str(), yaml.size());
}

esp_err_t WebServer::handle_ext_values(httpd_req_t* req)
{
    return send_json(req, ExtData::get().values_json());
}

esp_err_t WebServer::handle_ext_refresh(httpd_req_t* req)
{
    REQUIRE_AUTH(req);
    ExtData::get().refresh();
    return send_json(req, "{\"ok\":true}");
}
#endif

esp_err_t WebServer::handle_favicon(httpd_req_t* req) {
    httpd_resp_set_type(req, "image/svg+xml");
    httpd_resp_set_hdr(req, "Cache-Control", "max-age=86400");
    return httpd_resp_send(req, kLogoSvg, HTTPD_RESP_USE_STRLEN);
}

} // namespace app
