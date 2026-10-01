#include "wifi_manager.hpp"
#include "config_store.hpp"
#include "app_state.hpp"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

static const char* TAG = "WifiMgr";

static EventGroupHandle_t s_wifi_eg = nullptr;
static constexpr EventBits_t STA_CONNECTED_BIT = BIT0;
static constexpr EventBits_t STA_FAILED_BIT    = BIT1;
static int s_retry = 0;
static constexpr int kMaxRetry = 5;
static bool s_wifi_initialized = false;
static bool s_sntp_started = false;

static void start_sntp_once()
{
    if (s_sntp_started) return;
    s_sntp_started = true;
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

static void ensure_wifi_initialized()
{
    if (s_wifi_initialized) {
        return;
    }
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&init_cfg) == ESP_OK) {
        s_wifi_initialized = true;
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t base,
                                int32_t id, void* data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry < kMaxRetry) {
            esp_wifi_connect();
            ++s_retry;
            ESP_LOGI(TAG, "STA reconnect attempt %d/%d", s_retry, kMaxRetry);
        } else {
            xEventGroupSetBits(s_wifi_eg, STA_FAILED_BIT);
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        auto* event = static_cast<ip_event_got_ip_t*>(data);
        char buf[20];
        snprintf(buf, sizeof(buf), IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_eg, STA_CONNECTED_BIT);
        ESP_LOGI(TAG, "STA IP: %s", buf);
    } else if (base == IP_EVENT && id == IP_EVENT_GOT_IP6) {
        auto* event = static_cast<ip_event_got_ip6_t*>(data);
        const auto address_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
        if (address_type != ESP_IP6_ADDR_IS_GLOBAL && address_type != ESP_IP6_ADDR_IS_UNIQUE_LOCAL) {
            return;
        }
        char buf[48];
        snprintf(buf, sizeof(buf), IPV6STR, IPV62STR(event->ip6_info.ip));
        const bool is_global = address_type == ESP_IP6_ADDR_IS_GLOBAL;
        app::WifiManager::get().set_ipv6(buf, is_global);
        ESP_LOGI(TAG, "STA IPv6 %s: %s", is_global ? "global" : "ULA", buf);
    }
}

namespace app {

WifiManager& WifiManager::get() {
    static WifiManager inst;
    return inst;
}

bool WifiManager::start()
{
    WifiConfig cfg;
    bool haveConfig = ConfigStore::get().load_wifi(cfg);
    have_wifi_credentials_ = haveConfig && !cfg.ssid.empty();

    esp_netif_init();
    esp_event_loop_create_default();
    s_wifi_eg = xEventGroupCreate();

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,  &wifi_event_handler, nullptr);
    esp_event_handler_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, &wifi_event_handler, nullptr);
    esp_event_handler_register(IP_EVENT,   IP_EVENT_GOT_IP6,     &wifi_event_handler, nullptr);

    // Generate AP SSID from MAC
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    char ap_name[32];
    snprintf(ap_name, sizeof(ap_name), "Smartmeter-%02X%02X%02X",
             mac[3], mac[4], mac[5]);
    ap_ssid_ = ap_name;

    if (have_wifi_credentials_) {
        if (start_sta(cfg.ssid, cfg.password)) {
            // Connected: no fallback AP required.
            return true;
        }
        ESP_LOGW(TAG, "STA failed — falling back to AP");
    }

    start_ap(ap_name);
    schedule_ap_timeout(kApFallbackWindowMs);
    AppState::get().push_log("W", TAG,
        ("STA not configured or failed — AP active for 20 min: " + std::string(ap_name)).c_str());
    return false;
}

bool WifiManager::reconnect(const std::string& ssid, const std::string& pass)
{
    if (ssid.empty() || !s_wifi_initialized || !s_wifi_eg) return false;
    sta_connected_ = false;
    ap_active_ = false;
    ip_.clear();
    ipv6_.clear();
    ipv6_is_global_ = false;
    xEventGroupClearBits(s_wifi_eg, STA_CONNECTED_BIT | STA_FAILED_BIT);
    esp_wifi_stop();
    const bool connected = start_sta(ssid, pass);
    if (!connected) {
        start_ap(ap_ssid_.empty() ? "Smartmeter" : ap_ssid_);
    }
    return connected;
}

bool WifiManager::start_sta(const std::string& ssid, const std::string& pass)
{
    s_retry = 0;
    ipv6_.clear();
    ipv6_is_global_ = false;
    if (!esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"))
        esp_netif_create_default_wifi_sta();
    esp_netif_t* sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif) esp_netif_create_ip6_linklocal(sta_netif);
    ensure_wifi_initialized();
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wcfg{};
    strncpy(reinterpret_cast<char*>(wcfg.sta.ssid),
            ssid.c_str(), sizeof(wcfg.sta.ssid) - 1);
    strncpy(reinterpret_cast<char*>(wcfg.sta.password),
            pass.c_str(), sizeof(wcfg.sta.password) - 1);
    // OPEN is the minimum threshold, so WPA/WPA2/WPA3 networks are accepted.
    // The password still determines whether the actual association succeeds.
    wcfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
    wcfg.sta.pmf_cfg.capable = true;
    wcfg.sta.pmf_cfg.required = false;

    const esp_err_t config_err = esp_wifi_set_config(WIFI_IF_STA, &wcfg);
    const esp_err_t start_err = esp_wifi_start();
    const esp_err_t connect_err = start_err == ESP_OK ? esp_wifi_connect() : start_err;
    if (config_err != ESP_OK || start_err != ESP_OK || connect_err != ESP_OK) {
        ESP_LOGE(TAG, "STA setup failed: config=%s start=%s connect=%s",
                 esp_err_to_name(config_err), esp_err_to_name(start_err),
                 esp_err_to_name(connect_err));
        if (start_err == ESP_OK) esp_wifi_stop();
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(s_wifi_eg,
        STA_CONNECTED_BIT | STA_FAILED_BIT,
        pdFALSE, pdFALSE,
        pdMS_TO_TICKS(kStaTimeoutMs));

    if (bits & STA_CONNECTED_BIT) {
        // Retrieve assigned IP
        esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_ip_info_t info{};
        esp_netif_get_ip_info(netif, &info);
        char buf[20];
        snprintf(buf, sizeof(buf), IPSTR, IP2STR(&info.ip));
        ip_ = buf;
        sta_ssid_ = ssid;
        sta_connected_ = true;
        ESP_LOGI(TAG, "STA connected, IP=%s", ip_.c_str());
        esp_ip6_addr_t ip6{};
        if (netif && esp_netif_get_ip6_linklocal(netif, &ip6) == ESP_OK) {
            char ipv6_buf[48];
            snprintf(ipv6_buf, sizeof(ipv6_buf), IPV6STR, IPV62STR(ip6));
            ESP_LOGI(TAG, "STA IPv6 link-local (not routable): %s", ipv6_buf);
        }
        start_sntp_once();
        return true;
    }
    ESP_LOGE(TAG, "STA connection timeout for SSID '%s'", ssid.c_str());
    esp_wifi_stop();
    return false;
}

void WifiManager::start_ap(const std::string& ssid)
{
    if (!esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"))
        esp_netif_create_default_wifi_ap();
    ensure_wifi_initialized();

    wifi_config_t wcfg{};
    strncpy(reinterpret_cast<char*>(wcfg.ap.ssid),
            ssid.c_str(), sizeof(wcfg.ap.ssid) - 1);
    wcfg.ap.ssid_len       = static_cast<uint8_t>(ssid.size());
    wcfg.ap.channel        = 1;
    strncpy(reinterpret_cast<char*>(wcfg.ap.password),
            "12345678", sizeof(wcfg.ap.password) - 1);
    wcfg.ap.authmode       = WIFI_AUTH_WPA2_PSK;
    wcfg.ap.max_connection = 4;

    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_set_config(WIFI_IF_AP, &wcfg);
    esp_wifi_start();
    ap_active_ = true;
    ap_ssid_ = ssid;
    ip_        = "192.168.4.1";
    ESP_LOGI(TAG, "AP started: SSID=%s", ssid.c_str());
}

void WifiManager::stop_ap()
{
    if (!ap_active_) {
        return;
    }

    if (sta_connected_) {
        esp_wifi_set_mode(WIFI_MODE_STA);
    } else {
        esp_wifi_set_mode(WIFI_MODE_NULL);
        esp_wifi_stop();
    }

    ap_active_ = false;
    ESP_LOGI(TAG, "AP stopped");
}

void WifiManager::schedule_ap_timeout(uint32_t timeout_ms)
{
    struct TimeoutCtx {
        WifiManager* mgr;
        uint32_t timeout_ms;
    };

    auto* ctx = new TimeoutCtx{this, timeout_ms};
    if (xTaskCreate(&WifiManager::ap_timeout_task_fn, "ap_timeout", 4096, ctx, 5, nullptr) != pdPASS) {
        delete ctx;
        ESP_LOGW(TAG, "Failed to start AP timeout task");
    }
}

void WifiManager::ap_timeout_task_fn(void* arg)
{
    struct TimeoutCtx {
        WifiManager* mgr;
        uint32_t timeout_ms;
    };

    TimeoutCtx* ctx = static_cast<TimeoutCtx*>(arg);
    const uint32_t timeout_ms = ctx->timeout_ms;
    WifiManager* mgr = ctx->mgr;
    delete ctx;

    vTaskDelay(pdMS_TO_TICKS(timeout_ms));

    if (mgr->is_ap_active()) {
        mgr->stop_ap();
        AppState::get().push_log("W", TAG, "AP fallback timeout reached (20 min) - AP disabled");
    }

    vTaskDelete(nullptr);
}

} // namespace app
