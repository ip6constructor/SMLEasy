#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "mdns.h"

#include "rtc_log.hpp"
#include "wifi_manager.hpp"
#include "web_server.hpp"
#include "meter_task.hpp"
#include "app_state.hpp"
#include "config_store.hpp"
#include "mqtt_manager.hpp"

#include <cstdio>
#include <cctype>
#include <cstring>

static const char* TAG = "main";

// ── Web log hook ──────────────────────────────────────────────────────────────
// Intercept every ESP_LOGx call and push it into the AppState ring buffer so
// it appears in the web UI live log — including all component-level messages
// (SessionManager, MEPHunter, FrameHandler, etc.) that were only visible on
// the serial monitor before.
//
// Format produced by ESP-IDF vprintf output:
//   I (12345) TAG: message\n
//   ^level     ^tag  ^msg
static vprintf_like_t s_prev_log_fn = nullptr;

static int web_log_vprintf(const char* fmt, va_list args)
{
    // Copy BEFORE the first consumer exhausts the va_list
    va_list args2;
    va_copy(args2, args);

    int ret = 0;
    if (s_prev_log_fn) ret = s_prev_log_fn(fmt, args);

    // Skip during early boot (FreeRTOS scheduler not yet running)
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
        va_end(args2);
        return ret;
    }

    // Format the complete log line
    char buf[280];
    vsnprintf(buf, sizeof(buf), fmt, args2);
    va_end(args2);

    // Strip ANSI escape sequences (\033[...m) and trailing whitespace
    char clean[256];
    int ci = 0;
    for (int i = 0; buf[i] && ci < 254; ++i) {
        if (buf[i] == '\033') {
            while (buf[i] && buf[i] != 'm') ++i;
            continue;
        }
        if (buf[i] == '\n' || buf[i] == '\r') continue;
        clean[ci++] = buf[i];
    }
    clean[ci] = '\0';
    if (ci == 0) return ret;

    // Determine level from the first character (I/W/E/D/V)
    const char* level = "I";
    if      (clean[0] == 'W') level = "W";
    else if (clean[0] == 'E') level = "E";
    else if (clean[0] == 'D') level = "D";

    // Try to split "L (ts) TAG: msg" into tag + msg for nicer display
    // Fall back to pushing the full line as msg with empty tag
    const char* tag = "";
    const char* msg = clean;
    const char* rp  = strchr(clean, ')');
    if (rp && rp[1] == ' ') {
        const char* tp = rp + 2;
        const char* cp = strchr(tp, ':');
        if (cp && cp[1] == ' ') {
            static char s_tag[32];
            static char s_msg[220];
            size_t tlen = static_cast<size_t>(cp - tp);
            if (tlen >= sizeof(s_tag)) tlen = sizeof(s_tag) - 1;
            strncpy(s_tag, tp, tlen); s_tag[tlen] = '\0';
            strncpy(s_msg, cp + 2, sizeof(s_msg) - 1); s_msg[sizeof(s_msg)-1] = '\0';
            tag = s_tag;
            msg = s_msg;
        }
    }

    app::AppState::get().push_log(level, tag, msg);

    // Also write into RTC RAM ring buffer so last N lines survive a crash/reboot
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        char line[128];
        // Use separate snprintf calls to avoid format-truncation warning.
        // level/tag are short; msg may be up to ~120 chars.
        int n = snprintf(line, sizeof(line), "[%.2s] %.20s: ", level, tag);
        if (n > 0 && n < (int)sizeof(line)) {
            strncpy(line + n, msg, sizeof(line) - (size_t)n - 1);
            line[sizeof(line) - 1] = '\0';
        }
        rtc_log_push(line);
    }

    return ret;
}

// ── Optical UART peripheral number ───────────────────────────────────────────
static constexpr int kOpticalUartNr = 1;    ///< UART peripheral number (1 or 2)

/// Initialise NVS and check firmware version.
/// Erase NVS only when its storage format cannot be initialized. Firmware
/// version changes preserve user configuration and update only the version key.
static void nvs_init_with_version_check()
{
    // First init — needed to open handles
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS corrupt/full — erasing");
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Current firmware version string (set by PROJECT_VER in CMakeLists.txt)
    const char* fw_ver = esp_ota_get_app_description()->version;

    // Read stored version from NVS namespace "sys"
    nvs_handle_t h;
    bool version_match = false;
    char stored[32] = "<none>";
    if (nvs_open("sys", NVS_READONLY, &h) == ESP_OK) {
        size_t len = sizeof(stored);
        if (nvs_get_str(h, "fw_ver", stored, &len) == ESP_OK) {
            version_match = (strncmp(stored, fw_ver, sizeof(stored)) == 0);
        }
        nvs_close(h);
    }

    if (!version_match) {
        ESP_LOGI(TAG, "NVS version changed (stored='%s' running='%s') — preserving configuration",
                 stored, fw_ver);
        if (nvs_open("sys", NVS_READWRITE, &h) == ESP_OK) {
            nvs_set_str(h, "fw_ver", fw_ver);
            nvs_commit(h);
            nvs_close(h);
            ESP_LOGI(TAG, "NVS version written: %s", fw_ver);
        }
    } else {
        ESP_LOGI(TAG, "NVS version OK: %s", fw_ver);
    }
}

static void clear_power_cycle_counter_task(void*)
{
    vTaskDelay(pdMS_TO_TICKS(60000));
    nvs_handle_t h;
    if (nvs_open("sys", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u8(h, "power_boots", 0);
        nvs_commit(h);
        nvs_close(h);
    }
    vTaskDelete(nullptr);
}

static void track_power_cycles()
{
    if (esp_reset_reason() != ESP_RST_POWERON) return;
    nvs_handle_t h;
    if (nvs_open("sys", NVS_READWRITE, &h) != ESP_OK) return;
    uint8_t boots = 0;
    nvs_get_u8(h, "power_boots", &boots);
    ++boots;
    if (boots >= 5) {
        if (app::ConfigStore::get().reset_all_config()) {
            ESP_LOGW(TAG, "5 power-on cycles detected within the reset window: all user configuration reset");
        } else {
            ESP_LOGE(TAG, "power-cycle reset requested, but configuration reset failed");
        }
        boots = 0;
    }
    nvs_set_u8(h, "power_boots", boots);
    nvs_commit(h);
    nvs_close(h);
    if (boots > 0) {
        xTaskCreate(clear_power_cycle_counter_task, "clear_boots", 2048, nullptr, 2, nullptr);
    }
}

static void start_mdns(const std::string& configured_name)
{
    uint8_t mac[6] = {};
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    std::string base;
    for (char c : configured_name) {
        if (std::isalnum(static_cast<unsigned char>(c))) base += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        else if (base.empty() || base.back() != '-') base += '-';
    }
    while (!base.empty() && base.back() == '-') base.pop_back();
    if (base.empty()) base = "easysml";
    char hostname[64];
    char instance[32];
    snprintf(hostname, sizeof(hostname), "%s-%02X", base.c_str(), mac[5]);
    snprintf(instance, sizeof(instance), "%s-%02X", configured_name.empty() ? "EasySML" : configured_name.c_str(), mac[5]);

    if (mdns_init() != ESP_OK) {
        ESP_LOGE(TAG, "mDNS init failed");
        return;
    }
    mdns_hostname_set(hostname);
    mdns_instance_name_set(instance);
    mdns_service_add(instance, "_http", "_tcp", 80, nullptr, 0);
    ESP_LOGI(TAG, "mDNS available at http://%s.local", hostname);
}

extern "C" void app_main()
{
    // Install web log hook — must be first so all subsequent ESP_LOGx calls
    // appear in the web UI live log.
    s_prev_log_fn = esp_log_set_vprintf(web_log_vprintf);

    ESP_LOGI(TAG, "ESP32-C3 Optical SML Reader — starting");

    // ── NVS: init + firmware version check ───────────────────────────────────
    nvs_init_with_version_check();
    track_power_cycles();

    // ── Load interval + persistent login state into AppState ─────────────────
    app::MeterConfig cfg;
    if (app::ConfigStore::get().load_meter(cfg)) {
        app::AppState::get().read_interval_s.store(cfg.interval_s);
        app::AppState::get().last_login_ok.store(cfg.last_login_ok);
    }

    // ── UART driver installation (TX=1, RX=3 — WiFi IR Lesekopf V32 / ESP32-C3) ─
    app::MeterTask::get().init(1, 3, kOpticalUartNr);

    // ── WiFi (blocks until connected or AP active) ────────────────────────────
    app::WifiManager::get().start();
    app::HaConfig ha_cfg;
    app::ConfigStore::get().load_ha(ha_cfg);
    start_mdns(ha_cfg.device_name);

    // ── HTTP server ──────────────────────────────────────────────────────────
    if (app::WebServer::get().start() != ESP_OK) {
        ESP_LOGE(TAG, "Web server start failed");
    }

    // ── MQTT / Home Assistant Discovery ─────────────────────────────────────
    {
        app::HaConfig hcfg;
        app::ConfigStore::get().load_ha(hcfg);
        app::MqttManager::get().init(hcfg);
    }

    ESP_LOGI(TAG, "Boot complete — IP: %s", app::WifiManager::get().get_ip().c_str());
    app::MeterTask::get().start_continuous();
}
