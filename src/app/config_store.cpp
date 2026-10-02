#include "config_store.hpp"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include <vector>

static const char* TAG = "ConfigStore";

namespace app {

ConfigStore& ConfigStore::get() {
    static ConfigStore inst;
    return inst;
}

// ── NVS helpers ───────────────────────────────────────────────────────────────

static bool nvs_get_str(nvs_handle_t h, const char* key, std::string& out)
{
    size_t len = 0;
    if (::nvs_get_str(h, key, nullptr, &len) != ESP_OK) return false;
    std::vector<char> buf(len);
    if (::nvs_get_str(h, key, buf.data(), &len) != ESP_OK) return false;
    out.assign(buf.data(), len > 0 ? len - 1 : 0); // strip NUL terminator
    return true;
}

static bool nvs_get_u32(nvs_handle_t h, const char* key, uint32_t& out)
{
    return ::nvs_get_u32(h, key, &out) == ESP_OK;
}

static bool nvs_get_u16(nvs_handle_t h, const char* key, uint16_t& out)
{
    return ::nvs_get_u16(h, key, &out) == ESP_OK;
}

static bool nvs_get_u8(nvs_handle_t h, const char* key, uint8_t& out)
{
    return ::nvs_get_u8(h, key, &out) == ESP_OK;
}

// ── MeterConfig ───────────────────────────────────────────────────────────────

bool ConfigStore::load_meter(MeterConfig& out)
{
    nvs_handle_t h;
    if (nvs_open(kNsMeter, NVS_READONLY, &h) != ESP_OK) {
        ESP_LOGW(TAG, "meter_cfg namespace not found — using defaults");
        return false;
    }
    nvs_get_u32(h, "interval_s",  out.interval_s);
    uint8_t dbg = out.uart_debug ? 1 : 0;
    nvs_get_u8(h, "uart_dbg",    dbg);
    out.uart_debug = (dbg != 0);
    uint8_t lgn = out.last_login_ok ? 1 : 0;
    nvs_get_u8(h, "last_lgn",   lgn);
    out.last_login_ok = (lgn != 0);
    nvs_get_str(h, "profile_id", out.profile_id);
    nvs_get_str(h, "profile_name", out.profile_name);
    nvs_get_str(h, "manufacturer", out.manufacturer);
    nvs_get_str(h, "model", out.model);
    nvs_get_str(h, "prev_profile", out.previous_profile_json);
    uint16_t lwait = out.login_wait_ms;
    nvs_get_u16(h, "l_wait", lwait);
    out.login_wait_ms = lwait;
    nvs_get_str(h, "m_pin", out.meter_pin);
    nvs_get_str(h, "l_cmd", out.login_cmd);

    nvs_get_str(h, "ob_imp", out.obis_import_wh);
    nvs_get_str(h, "ob_exp", out.obis_export_wh);
    nvs_get_str(h, "ob_pnet", out.obis_power_net_w);
    nvs_get_str(h, "ob_pimp", out.obis_power_import_w);
    nvs_get_str(h, "ob_pexp", out.obis_power_export_w);
    nvs_get_str(h, "ob_v1", out.obis_voltage_l1_v);
    nvs_get_str(h, "ob_v2", out.obis_voltage_l2_v);
    nvs_get_str(h, "ob_v3", out.obis_voltage_l3_v);
    nvs_get_str(h, "ob_i1", out.obis_current_l1_a);
    nvs_get_str(h, "ob_i2", out.obis_current_l2_a);
    nvs_get_str(h, "ob_i3", out.obis_current_l3_a);
    nvs_get_str(h, "ob_fq", out.obis_frequency_hz);
    nvs_get_str(h, "ob_pf1", out.obis_pf_l1);

    nvs_close(h);
    ESP_LOGD(TAG, "meter cfg loaded: interval=%" PRIu32 "s uart_dbg=%d",
             out.interval_s, (int)out.uart_debug);
    return true;
}

bool ConfigStore::save_meter(const MeterConfig& cfg)
{
    nvs_handle_t h;
    if (nvs_open(kNsMeter, NVS_READWRITE, &h) != ESP_OK) {
        ESP_LOGE(TAG, "cannot open meter_cfg for write");
        return false;
    }
    nvs_set_u32(h, "interval_s", cfg.interval_s);
    nvs_set_u8 (h, "uart_dbg",    cfg.uart_debug       ? 1 : 0);
    nvs_set_u8 (h, "last_lgn",    cfg.last_login_ok    ? 1 : 0);
    nvs_set_str(h, "profile_id", cfg.profile_id.c_str());
    nvs_set_str(h, "profile_name", cfg.profile_name.c_str());
    nvs_set_str(h, "manufacturer", cfg.manufacturer.c_str());
    nvs_set_str(h, "model", cfg.model.c_str());
    nvs_set_str(h, "prev_profile", cfg.previous_profile_json.c_str());
    nvs_set_u16(h, "l_wait",      cfg.login_wait_ms);
    nvs_set_str(h, "m_pin",       cfg.meter_pin.c_str());
    nvs_set_str(h, "l_cmd",       cfg.login_cmd.c_str());
    nvs_set_str(h, "ob_imp", cfg.obis_import_wh.c_str());
    nvs_set_str(h, "ob_exp", cfg.obis_export_wh.c_str());
    nvs_set_str(h, "ob_pnet", cfg.obis_power_net_w.c_str());
    nvs_set_str(h, "ob_pimp", cfg.obis_power_import_w.c_str());
    nvs_set_str(h, "ob_pexp", cfg.obis_power_export_w.c_str());
    nvs_set_str(h, "ob_v1", cfg.obis_voltage_l1_v.c_str());
    nvs_set_str(h, "ob_v2", cfg.obis_voltage_l2_v.c_str());
    nvs_set_str(h, "ob_v3", cfg.obis_voltage_l3_v.c_str());
    nvs_set_str(h, "ob_i1", cfg.obis_current_l1_a.c_str());
    nvs_set_str(h, "ob_i2", cfg.obis_current_l2_a.c_str());
    nvs_set_str(h, "ob_i3", cfg.obis_current_l3_a.c_str());
    nvs_set_str(h, "ob_fq", cfg.obis_frequency_hz.c_str());
    nvs_set_str(h, "ob_pf1", cfg.obis_pf_l1.c_str());

    esp_err_t err = nvs_commit(h);
    nvs_close(h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "meter cfg saved");
    return true;
}

// ── WifiConfig ────────────────────────────────────────────────────────────────

bool ConfigStore::load_wifi(WifiConfig& out)
{
    nvs_handle_t h;
    if (nvs_open(kNsWifi, NVS_READONLY, &h) != ESP_OK) {
        ESP_LOGW(TAG, "wifi_cfg namespace not found");
        return false;
    }
    const bool ssid_ok = nvs_get_str(h, "ssid", out.ssid);
    const bool pass_ok = nvs_get_str(h, "pass", out.password);
    nvs_close(h);
    ESP_LOGI(TAG, "wifi cfg loaded: ssid=%s password=%s", out.ssid.c_str(),
             pass_ok ? "stored" : "missing");
    return ssid_ok;
}

bool ConfigStore::save_wifi(const WifiConfig& cfg)
{
    nvs_handle_t h;
    if (nvs_open(kNsWifi, NVS_READWRITE, &h) != ESP_OK) {
        ESP_LOGE(TAG, "cannot open wifi_cfg for write");
        return false;
    }
    const esp_err_t ssid_err = nvs_set_str(h, "ssid", cfg.ssid.c_str());
    const esp_err_t pass_err = nvs_set_str(h, "pass", cfg.password.c_str());
    esp_err_t err = nvs_commit(h);
    nvs_close(h);
    if (ssid_err != ESP_OK || pass_err != ESP_OK || err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "wifi cfg saved: ssid=%s", cfg.ssid.c_str());
    return true;
}

bool ConfigStore::load_web_auth(WebAuthConfig& out)
{
    nvs_handle_t h;
    if (nvs_open(kNsAuth, NVS_READONLY, &h) != ESP_OK) return false;
    nvs_get_str(h, "user", out.username);
    nvs_get_str(h, "pass", out.password);
    nvs_close(h);
    return !out.username.empty() && !out.password.empty();
}

bool ConfigStore::save_web_auth(const WebAuthConfig& cfg)
{
    if (cfg.username.empty() || cfg.password.empty()) return false;
    nvs_handle_t h;
    if (nvs_open(kNsAuth, NVS_READWRITE, &h) != ESP_OK) return false;
    const esp_err_t user_err = nvs_set_str(h, "user", cfg.username.c_str());
    const esp_err_t pass_err = nvs_set_str(h, "pass", cfg.password.c_str());
    const esp_err_t commit_err = nvs_commit(h);
    nvs_close(h);
    return user_err == ESP_OK && pass_err == ESP_OK && commit_err == ESP_OK;
}

bool ConfigStore::load_ui(UiConfig& out)
{
    nvs_handle_t h;
    if (nvs_open(kNsUi, NVS_READONLY, &h) != ESP_OK) return false;
    nvs_get_str(h, "lang", out.language);
    nvs_get_str(h, "networks", out.allowed_networks);
    nvs_close(h);
    return true;
}

bool ConfigStore::save_ui(const UiConfig& cfg)
{
    nvs_handle_t h;
    if (nvs_open(kNsUi, NVS_READWRITE, &h) != ESP_OK) return false;
    const esp_err_t set_err = nvs_set_str(h, "lang", cfg.language.c_str());
    const esp_err_t networks_err = nvs_set_str(h, "networks", cfg.allowed_networks.c_str());
    const esp_err_t commit_err = nvs_commit(h);
    nvs_close(h);
    return set_err == ESP_OK && networks_err == ESP_OK && commit_err == ESP_OK;
}

bool ConfigStore::reset_all_config()
{
    bool ok = true;
    constexpr const char* namespaces[] = {
        kNsWifi, kNsAuth, kNsUi, kNsTls, kNsMeter, kNsHa, kNsMqttLegacy, kNsTariff,
    };

    for (const char* name : namespaces) {
        nvs_handle_t handle;
        esp_err_t err = nvs_open(name, NVS_READONLY, &handle);
        if (err == ESP_ERR_NVS_NOT_FOUND) continue;
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "cannot open namespace '%s' for reset: %s", name, esp_err_to_name(err));
            ok = false;
            continue;
        }
        nvs_close(handle);

        err = nvs_open(name, NVS_READWRITE, &handle);
        if (err == ESP_OK) {
            err = nvs_erase_all(handle);
            if (err == ESP_OK) err = nvs_commit(handle);
            nvs_close(handle);
        }
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "cannot reset namespace '%s': %s", name, esp_err_to_name(err));
            ok = false;
        }
    }
    return ok;
}

// ── HaConfig ──────────────────────────────────────────────────────────────────

bool ConfigStore::load_ha(HaConfig& out)
{
    nvs_handle_t h;
    bool legacy = false;
    if (nvs_open(kNsHa, NVS_READONLY, &h) != ESP_OK) {
        if (nvs_open(kNsMqttLegacy, NVS_READONLY, &h) != ESP_OK) {
            ESP_LOGD(TAG, "ha_cfg namespace not found — using defaults");
            return false;
        }
        legacy = true;
    }
    uint8_t en = out.enabled ? 1 : 0;
    if (legacy) {
        nvs_get_u8 (h, "mqtt_en",      en);
        nvs_get_str(h, "mqtt_uri",     out.broker_uri);
        nvs_get_str(h, "mqtt_user",    out.username);
        nvs_get_str(h, "mqtt_pass",    out.password);
        nvs_get_str(h, "mqtt_dev",     out.device_name);
        nvs_get_str(h, "mqtt_ha_pref", out.ha_prefix);
    } else {
        nvs_get_u8 (h, "ha_en",        en);
        nvs_get_str(h, "ha_uri",       out.broker_uri);
        nvs_get_str(h, "ha_user",      out.username);
        nvs_get_str(h, "ha_pass",      out.password);
        nvs_get_str(h, "ha_dev",       out.device_name);
        nvs_get_str(h, "ha_pref",      out.ha_prefix);
    }
    out.enabled = (en != 0);
    nvs_close(h);
    ESP_LOGI(TAG, "ha cfg loaded: enabled=%d uri=%s dev=%s",
             (int)out.enabled, out.broker_uri.c_str(), out.device_name.c_str());
    return true;
}

bool ConfigStore::save_ha(const HaConfig& cfg)
{
    nvs_handle_t h;
    if (nvs_open(kNsHa, NVS_READWRITE, &h) != ESP_OK) {
        ESP_LOGE(TAG, "cannot open ha_cfg for write");
        return false;
    }
    nvs_set_u8 (h, "ha_en",    cfg.enabled      ? 1 : 0);
    nvs_set_str(h, "ha_uri",   cfg.broker_uri.c_str());
    nvs_set_str(h, "ha_user",  cfg.username.c_str());
    nvs_set_str(h, "ha_pass",  cfg.password.c_str());
    nvs_set_str(h, "ha_dev",   cfg.device_name.c_str());
    nvs_set_str(h, "ha_pref",  cfg.ha_prefix.c_str());
    esp_err_t err = nvs_commit(h);
    nvs_close(h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit (ha) failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "ha cfg saved: enabled=%d uri=%s", (int)cfg.enabled, cfg.broker_uri.c_str());
    return true;
}

// ── TariffConfig ──────────────────────────────────────────────────────────────

bool ConfigStore::load_tariff(TariffConfig& out)
{
    nvs_handle_t h;
    if (nvs_open(kNsTariff, NVS_READONLY, &h) != ESP_OK) {
        ESP_LOGD(TAG, "tariff_cfg namespace not found — using defaults");
        return false;
    }
    std::string s;
    if (nvs_get_str(h, "p_imp", s) && !s.empty()) {
        char* end = nullptr;
        const double value = std::strtod(s.c_str(), &end);
        if (end != s.c_str() && *end == '\0') out.price_import_kwh = value;
    }
    if (nvs_get_str(h, "p_exp", s) && !s.empty()) {
        char* end = nullptr;
        const double value = std::strtod(s.c_str(), &end);
        if (end != s.c_str() && *end == '\0') out.price_export_kwh = value;
    }
    nvs_get_str(h, "cur", out.currency);
    nvs_close(h);
    ESP_LOGI(TAG, "tariff cfg loaded: import=%.4f export=%.4f %s",
             out.price_import_kwh, out.price_export_kwh, out.currency.c_str());
    return true;
}

bool ConfigStore::save_tariff(const TariffConfig& cfg)
{
    nvs_handle_t h;
    if (nvs_open(kNsTariff, NVS_READWRITE, &h) != ESP_OK) {
        ESP_LOGE(TAG, "cannot open tariff_cfg for write");
        return false;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%.4f", cfg.price_import_kwh);
    nvs_set_str(h, "p_imp", buf);
    snprintf(buf, sizeof(buf), "%.4f", cfg.price_export_kwh);
    nvs_set_str(h, "p_exp", buf);
    nvs_set_str(h, "cur", cfg.currency.c_str());
    esp_err_t err = nvs_commit(h);
    nvs_close(h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit (tariff) failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "tariff cfg saved: import=%.4f export=%.4f", cfg.price_import_kwh, cfg.price_export_kwh);
    return true;
}

} // namespace app
