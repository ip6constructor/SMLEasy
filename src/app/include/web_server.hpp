#pragma once
#include "esp_err.h"
#include "esp_http_server.h"

namespace app {

/// HTTP dashboard + REST API
class WebServer {
public:
    static WebServer& get();
    esp_err_t start(uint16_t port = 80);
    void      stop();
    [[nodiscard]] bool is_running() const { return server_ != nullptr; }
    [[nodiscard]] bool is_https_running() const { return https_server_ != nullptr; }
    /// Starts (or with restart=true re-starts) the HTTPS listener on port 443 in a
    /// background task. Failures never affect the HTTP dashboard on port 80.
    void      start_https_async(bool restart = false);
    /// ACME HTTP-01: swaps the dashboard on port 80 for a challenge-only listener.
    esp_err_t start_acme_challenge_listener();
    void      stop_acme_challenge_listener();
    void      stop_http();
    void      stop_https();

private:
    WebServer() = default;
    /// lan_rest_only restricts registration to kLanReadOnlyRoutes (used while port 80 otherwise
    /// redirects to HTTPS, so local integrations like Home Assistant keep plain-HTTP polling).
    static void register_routes(httpd_handle_t server, bool lan_rest_only = false);
    static void https_task(void* restart);
    esp_err_t start_https();
    static esp_err_t handle_tls_config_get(httpd_req_t*);
    static esp_err_t handle_pair_page(httpd_req_t*);
    static esp_err_t handle_pairing_status(httpd_req_t*);
    static esp_err_t handle_pairing_open(httpd_req_t*);
    static esp_err_t handle_pairing_close(httpd_req_t*);
    static esp_err_t handle_pairing_claim(httpd_req_t*);
    static esp_err_t handle_pairing_decide(httpd_req_t*);
    static esp_err_t handle_pairing_revoke(httpd_req_t*);
    static esp_err_t handle_pairing_rename(httpd_req_t*);
    static esp_err_t handle_tls_config_save(httpd_req_t*);
    static esp_err_t handle_acme_challenge(httpd_req_t*);
    static esp_err_t handle_acme_request(httpd_req_t*);
    static esp_err_t handle_acme_check(httpd_req_t*);
    static esp_err_t handle_acme_status(httpd_req_t*);
    static esp_err_t handle_tls_page(httpd_req_t*);
    static esp_err_t handle_ext_page(httpd_req_t*);
    static esp_err_t handle_ext_config_get(httpd_req_t*);
    static esp_err_t handle_ext_config_save(httpd_req_t*);
    static esp_err_t handle_ext_values(httpd_req_t*);
    static esp_err_t handle_ext_refresh(httpd_req_t*);
    static esp_err_t handle_root(httpd_req_t*);
    static esp_err_t handle_config_page(httpd_req_t*);
    static esp_err_t handle_style_css(httpd_req_t*);
    static esp_err_t handle_app_js(httpd_req_t*);
    static esp_err_t handle_house_jpg(httpd_req_t*);
    static esp_err_t handle_status(httpd_req_t*);
    static esp_err_t handle_log(httpd_req_t*);
    static esp_err_t handle_history(httpd_req_t*);
    static esp_err_t handle_stats(httpd_req_t*);
    static esp_err_t handle_wifi_scan(httpd_req_t*);
    static esp_err_t handle_config_get(httpd_req_t*);
    static esp_err_t handle_config_save(httpd_req_t*);
    static esp_err_t handle_wifi_save(httpd_req_t*);
    static esp_err_t handle_hostname_save(httpd_req_t*);
    static esp_err_t handle_auth_get(httpd_req_t*);
    static esp_err_t handle_auth_save(httpd_req_t*);
    static esp_err_t handle_ui_language_get(httpd_req_t*);
    static esp_err_t handle_ui_language_save(httpd_req_t*);
    static esp_err_t handle_access_networks_get(httpd_req_t*);
    static esp_err_t handle_access_networks_save(httpd_req_t*);
    static esp_err_t handle_ha_config_get(httpd_req_t*);
    static esp_err_t handle_ha_config_save(httpd_req_t*);
    static esp_err_t handle_tariff_get(httpd_req_t*);
    static esp_err_t handle_tariff_save(httpd_req_t*);
    static esp_err_t handle_daily_baseline_get(httpd_req_t*);
    static esp_err_t handle_daily_baseline_save(httpd_req_t*);
    static esp_err_t handle_stats_pv(httpd_req_t*);
    static esp_err_t handle_start(httpd_req_t*);            ///< Single-shot read
    static esp_err_t handle_start_continuous(httpd_req_t*); ///< Repeating reads
    static esp_err_t handle_stop(httpd_req_t*);
    static esp_err_t handle_reboot(httpd_req_t*);
    static esp_err_t handle_reset_counters(httpd_req_t*);
    static esp_err_t handle_ota(httpd_req_t*);
    static esp_err_t handle_ota_github(httpd_req_t*);
    static esp_err_t handle_ota_github_status(httpd_req_t*);
    static esp_err_t handle_meter(httpd_req_t*);
    static esp_err_t handle_favicon(httpd_req_t*);   ///< 204 No Content for /favicon.ico

    httpd_handle_t server_{nullptr};
    httpd_handle_t https_server_{nullptr};
    httpd_handle_t challenge_server_{nullptr};
};

} // namespace app
