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
#include "cJSON.h"
#include "mbedtls/base64.h"

#include <cstring>
#include <map>
#include <string>
#include <vector>

static const char* TAG = "WebServer";

// ── helpers ───────────────────────────────────────────────────────────────────

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

namespace app {

WebServer& WebServer::get() {
    static WebServer inst;
    return inst;
}

// ── start / stop ──────────────────────────────────────────────────────────────

esp_err_t WebServer::start(uint16_t port)
{
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.server_port       = port;
    cfg.max_uri_handlers  = 40;
    cfg.stack_size        = 8192;
    cfg.lru_purge_enable  = true;
    cfg.recv_wait_timeout = 60;
    cfg.send_wait_timeout = 60;

    if (httpd_start(&server_, &cfg) != ESP_OK) {
        ESP_LOGE(TAG, "httpd_start failed");
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
        { "/api/meter",           HTTP_GET,  handle_meter,         nullptr },
        { "/favicon.ico",          HTTP_GET,  handle_favicon,       nullptr },
    };
    for (const auto& r : routes) httpd_register_uri_handler(server_, &r);

    ESP_LOGI(TAG, "HTTP server started on port %u", port);
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
    cJSON_AddStringToObject(root, "ipv6",    WifiManager::get().get_ipv6().c_str());
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
