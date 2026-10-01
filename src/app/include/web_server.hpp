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

private:
    WebServer() = default;
    static esp_err_t handle_root(httpd_req_t*);
    static esp_err_t handle_config_page(httpd_req_t*);
    static esp_err_t handle_style_css(httpd_req_t*);
    static esp_err_t handle_app_js(httpd_req_t*);
    static esp_err_t handle_status(httpd_req_t*);
    static esp_err_t handle_log(httpd_req_t*);
    static esp_err_t handle_history(httpd_req_t*);
    static esp_err_t handle_wifi_scan(httpd_req_t*);
    static esp_err_t handle_config_get(httpd_req_t*);
    static esp_err_t handle_config_save(httpd_req_t*);
    static esp_err_t handle_wifi_save(httpd_req_t*);
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
    static esp_err_t handle_start(httpd_req_t*);            ///< Single-shot read
    static esp_err_t handle_start_continuous(httpd_req_t*); ///< Repeating reads
    static esp_err_t handle_stop(httpd_req_t*);
    static esp_err_t handle_reboot(httpd_req_t*);
    static esp_err_t handle_reset_counters(httpd_req_t*);
    static esp_err_t handle_ota(httpd_req_t*);
    static esp_err_t handle_ota_github(httpd_req_t*);
    static esp_err_t handle_ota_github_status(httpd_req_t*);
    static esp_err_t handle_acme_challenge(httpd_req_t*);
    static esp_err_t handle_tls_config_get(httpd_req_t*);
    static esp_err_t handle_tls_config_save(httpd_req_t*);
    static esp_err_t handle_acme_request(httpd_req_t*);
    static esp_err_t handle_acme_status(httpd_req_t*);
    static esp_err_t handle_meter(httpd_req_t*);
    static esp_err_t handle_favicon(httpd_req_t*);   ///< 204 No Content for /favicon.ico

    httpd_handle_t server_{nullptr};
    httpd_handle_t challenge_server_{nullptr};
    bool server_is_https_{false};
};

} // namespace app
