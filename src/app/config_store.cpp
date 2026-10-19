#include "config_store.hpp"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_idf_version.h"
#if ESP_IDF_VERSION_MAJOR >= 6
#define SMLEASY_SEAL_SECRETS 1
#include "secret_store.hpp"
#endif
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

// Secrets: sealed blob under "<key>.e" (IDF 6 path); plaintext key only as legacy fallback.
static bool secret_get(nvs_handle_t h, const char* ns, const char* key, std::string& out)
{
#ifdef SMLEASY_SEAL_SECRETS
    const std::string bkey = std::string(key) + ".e";
    size_t len = 0;
    if (::nvs_get_blob(h, bkey.c_str(), nullptr, &len) == ESP_OK && len > 0) {
        std::vector<uint8_t> blob(len);
        if (::nvs_get_blob(h, bkey.c_str(), blob.data(), &len) == ESP_OK &&
            app::secrets::open(std::string(ns) + "/" + key, blob.data(), len, out))
            return true;
        ESP_LOGE(TAG, "sealed secret %s/%s unreadable", ns, key);
        return false;
    }
#else
    (void)ns;
#endif
    return nvs_get_str(h, key, out);
}

static esp_err_t secret_set(nvs_handle_t h, const char* ns, const char* key, const std::string& value)
{
#ifdef SMLEASY_SEAL_SECRETS
    std::vector<uint8_t> blob;
    if (app::secrets::seal(std::string(ns) + "/" + key, value, blob)) {
        const std::string bkey = std::string(key) + ".e";
        esp_err_t err = ::nvs_set_blob(h, bkey.c_str(), blob.data(), blob.size());
        if (err == ESP_OK) {
            const esp_err_t e = ::nvs_erase_key(h, key);  // drop legacy plaintext
            if (e != ESP_OK && e != ESP_ERR_NVS_NOT_FOUND) err = e;
        }
        return err;
    }
    ESP_LOGE(TAG, "sealing %s/%s failed, not storing", ns, key);
    return ESP_FAIL;
#else
    (void)ns;
    return ::nvs_set_str(h, key, value.c_str());
#endif
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
    const bool pass_ok = secret_get(h, kNsWifi, "pass", out.password);
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
    const esp_err_t pass_err = secret_set(h, kNsWifi, "pass", cfg.password);
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
    std::string legacy;
    if (nvs_get_str(h, "pass", legacy)) out.password = legacy;
#ifdef SMLEASY_SEAL_SECRETS
    else { uint8_t pv[32]; size_t n = sizeof(pv); if (::nvs_get_blob(h, "pv", pv, &n) == ESP_OK) out.password.clear(); }
#endif
    nvs_close(h);
    return !out.username.empty();
}

bool ConfigStore::verify_web_auth(const std::string& user, const std::string& pass)
{
    WebAuthConfig cfg;
    load_web_auth(cfg);
    if (user != cfg.username) return false;
#ifdef SMLEASY_SEAL_SECRETS
    nvs_handle_t h;
    if (nvs_open(kNsAuth, NVS_READONLY, &h) == ESP_OK) {
        uint8_t stored[32];
        size_t n = sizeof(stored);
        const bool have = ::nvs_get_blob(h, "pv", stored, &n) == ESP_OK && n == sizeof(stored);
        nvs_close(h);
        if (have) {
            uint8_t calc[32];
            if (!secrets::password_verifier(pass, calc)) return false;
            uint8_t diff = 0;
            for (size_t i = 0; i < 32; ++i) diff |= stored[i] ^ calc[i];
            return diff == 0;
        }
    }
#endif
    // Legacy plaintext or built-in default: verify, then migrate to the verifier.
    if (cfg.password.empty() || pass.size() != cfg.password.size()) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < pass.size(); ++i) diff |= static_cast<uint8_t>(pass[i] ^ cfg.password[i]);
    if (diff != 0) return false;
#ifdef SMLEASY_SEAL_SECRETS
    save_web_auth(cfg);
#endif
    return true;
}

bool ConfigStore::save_web_auth(const WebAuthConfig& cfg)
{
    if (cfg.username.empty() || cfg.password.empty()) return false;
    nvs_handle_t h;
    if (nvs_open(kNsAuth, NVS_READWRITE, &h) != ESP_OK) return false;
    esp_err_t pass_err;
    const esp_err_t user_err = nvs_set_str(h, "user", cfg.username.c_str());
#ifdef SMLEASY_SEAL_SECRETS
    uint8_t pv[32];
    if (!secrets::password_verifier(cfg.password, pv)) { nvs_close(h); return false; }
    pass_err = ::nvs_set_blob(h, "pv", pv, sizeof(pv));
    if (pass_err == ESP_OK) {
        const esp_err_t e = ::nvs_erase_key(h, "pass");
        if (e != ESP_OK && e != ESP_ERR_NVS_NOT_FOUND) pass_err = e;
    }
#else
    pass_err = nvs_set_str(h, "pass", cfg.password.c_str());
#endif
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

bool ConfigStore::load_tls(TlsConfig& out)
{
    nvs_handle_t h;
    if (nvs_open(kNsTls, NVS_READONLY, &h) != ESP_OK) return false;
    nvs_get_str(h, "mode", out.mode);
    nvs_get_str(h, "fqdn", out.fqdn);
    nvs_get_str(h, "email", out.email);
    nvs_get_str(h, "cert", out.certificate_pem);
    secret_get(h, kNsTls, "privkey", out.private_key_pem);
    nvs_get_str(h, "fb_cert", out.fallback_certificate_pem);
    secret_get(h, kNsTls, "fb_key", out.fallback_private_key_pem);
    secret_get(h, kNsTls, "acme_key", out.acme_account_key_pem);
    nvs_get_str(h, "acme_url", out.acme_account_url);
    uint8_t terms = out.acme_terms_accepted ? 1 : 0;
    nvs_get_u8(h, "acme_terms", terms);
    out.acme_terms_accepted = terms != 0;
    uint8_t staging = out.acme_staging ? 1 : 0;
    nvs_get_u8(h, "acme_staging", staging);
    out.acme_staging = staging != 0;
    uint8_t self_signed = out.self_signed_enabled ? 1 : 0;
    nvs_get_u8(h, "self_signed", self_signed);
    out.self_signed_enabled = self_signed != 0;
    nvs_get_u16(h, "renew_days", out.renewal_interval_days);
    nvs_get_i64(h, "issued_at", &out.last_issued_epoch);
    nvs_close(h);
    return true;
}

bool ConfigStore::save_tls(const TlsConfig& cfg)
{
    if (cfg.mode != "manual" && cfg.mode != "letsencrypt") return false;
    if (cfg.fqdn.size() > 253 || cfg.email.size() > 254 ||
        cfg.certificate_pem.size() > 3800 || cfg.private_key_pem.size() > 3800 ||
        cfg.fallback_certificate_pem.size() > 3800 || cfg.fallback_private_key_pem.size() > 3800 ||
        cfg.acme_account_key_pem.size() > 3800 || cfg.acme_account_url.size() > 512 ||
        cfg.renewal_interval_days < 1 || cfg.renewal_interval_days > 60) {
        ESP_LOGE(TAG, "TLS config rejected: cert=%u key=%u fb_cert=%u fb_key=%u acme_key=%u bytes",
                 static_cast<unsigned>(cfg.certificate_pem.size()),
                 static_cast<unsigned>(cfg.private_key_pem.size()),
                 static_cast<unsigned>(cfg.fallback_certificate_pem.size()),
                 static_cast<unsigned>(cfg.fallback_private_key_pem.size()),
                 static_cast<unsigned>(cfg.acme_account_key_pem.size()));
        return false;
    }
    nvs_handle_t h;
    if (nvs_open(kNsTls, NVS_READWRITE, &h) != ESP_OK) return false;
    esp_err_t err = nvs_set_str(h, "mode", cfg.mode.c_str());
    if (err == ESP_OK) err = nvs_set_str(h, "fqdn", cfg.fqdn.c_str());
    if (err == ESP_OK) err = nvs_set_str(h, "email", cfg.email.c_str());
    if (err == ESP_OK) err = nvs_set_str(h, "cert", cfg.certificate_pem.c_str());
    if (err == ESP_OK) err = secret_set(h, kNsTls, "privkey", cfg.private_key_pem);
    if (err == ESP_OK) err = nvs_set_str(h, "fb_cert", cfg.fallback_certificate_pem.c_str());
    if (err == ESP_OK) err = secret_set(h, kNsTls, "fb_key", cfg.fallback_private_key_pem);
    if (err == ESP_OK) err = secret_set(h, kNsTls, "acme_key", cfg.acme_account_key_pem);
    if (err == ESP_OK) err = nvs_set_str(h, "acme_url", cfg.acme_account_url.c_str());
    if (err == ESP_OK) err = nvs_set_u8(h, "acme_terms", cfg.acme_terms_accepted ? 1 : 0);
    if (err == ESP_OK) err = nvs_set_u8(h, "acme_staging", cfg.acme_staging ? 1 : 0);
    if (err == ESP_OK) err = nvs_set_u8(h, "self_signed", cfg.self_signed_enabled ? 1 : 0);
    if (err == ESP_OK) err = nvs_set_u16(h, "renew_days", cfg.renewal_interval_days);
    if (err == ESP_OK) err = nvs_set_i64(h, "issued_at", cfg.last_issued_epoch);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    if (err != ESP_OK) ESP_LOGE(TAG, "saving TLS config failed: %s", esp_err_to_name(err));
    return err == ESP_OK;
}

bool ConfigStore::reset_all_config()
{
    bool ok = true;
    constexpr const char* namespaces[] = {
        kNsWifi, kNsAuth, kNsUi, kNsTls, kNsMeter, kNsHa, kNsMqttLegacy, kNsTariff, kNsExt, "app_tok",
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

// ── ExtDataConfig (values read from Home Assistant) ──────────────────────────

static bool clean_text(const std::string& value, size_t max_size)
{
    if (value.size() > max_size) return false;
    for (unsigned char ch : value) {
        if (ch < 0x20 || ch == 0x7f) return false;
    }
    return true;
}

// Slots are stored as one string: fields separated by TAB, slots by LF.
static std::string encode_slots(const std::vector<ExtSlot>& slots)
{
    std::string out;
    for (const auto& slot : slots) {
        out += slot.label + '\t' + slot.source + '\t' + slot.ref + '\t' + slot.unit + '\t' + slot.key + '\t' + slot.role + '\n';
    }
    return out;
}

static std::vector<ExtSlot> decode_slots(const std::string& text)
{
    std::vector<ExtSlot> slots;
    size_t line_start = 0;
    while (line_start < text.size() && slots.size() < ExtDataConfig::kMaxSlots) {
        size_t line_end = text.find('\n', line_start);
        if (line_end == std::string::npos) line_end = text.size();
        const std::string line = text.substr(line_start, line_end - line_start);
        std::string fields[6];
        size_t field = 0, start = 0;
        for (size_t i = 0; i <= line.size() && field < 6; ++i) {
            if (i == line.size() || line[i] == '\t') {
                fields[field++] = line.substr(start, i - start);
                start = i + 1;
            }
        }
        if (field >= 5) {
            ExtSlot slot;
            slot.label = fields[0];
            slot.source = fields[1];
            slot.ref = fields[2];
            slot.unit = fields[3];
            slot.key = fields[4];
            slot.role = fields[5];
            slots.push_back(slot);
        }
        line_start = line_end + 1;
    }
    return slots;
}

bool ConfigStore::load_ext(ExtDataConfig& out)
{
    nvs_handle_t h;
    if (nvs_open(kNsExt, NVS_READONLY, &h) != ESP_OK) {
        out.enabled = true;
        return false;
    }
    uint8_t enabled = 1; // Home Assistant push values are on by default
    nvs_get_u8(h, "en", enabled);
    out.enabled = enabled != 0;
    nvs_get_str(h, "url", out.ha_url);
    secret_get(h, kNsExt, "token", out.ha_token);
    nvs_get_u16(h, "poll", out.poll_s);
    std::string slots;
    if (nvs_get_str(h, "slots", slots)) out.slots = decode_slots(slots);
    nvs_close(h);
    // The removed "HA sendet" source is no longer supported.
    for (size_t i = out.slots.size(); i-- > 0;)
        if (out.slots[i].source == "push") out.slots.erase(out.slots.begin() + static_cast<long>(i));
    return true;
}

bool ConfigStore::save_ext(const ExtDataConfig& cfg)
{
    if (!clean_text(cfg.ha_url, 200) || !clean_text(cfg.ha_token, 512) ||
        cfg.poll_s < 10 || cfg.poll_s > 600 || cfg.slots.size() > ExtDataConfig::kMaxSlots) {
        ESP_LOGE(TAG, "external data config rejected");
        return false;
    }
    for (const auto& slot : cfg.slots) {
        if (!clean_text(slot.label, 40) || !clean_text(slot.ref, 128) || !clean_text(slot.unit, 12) ||
            !clean_text(slot.key, 40) ||
            (slot.role != "" && slot.role != "pv" && slot.role != "battery" && slot.role != "battery_neg" &&
             slot.role != "pv_kwh" && slot.role != "bat_chg_kwh" && slot.role != "bat_dis_kwh" && slot.role != "wb_kwh" &&
             slot.role != "pv_east" && slot.role != "pv_south" && slot.role != "pv_west" &&
             slot.role != "wb_power" && slot.role != "wb_session_kwh" &&
             slot.role != "car_name" && slot.role != "wb_mode") ||
            (slot.source != "rest" && slot.source != "mqtt")) {
            ESP_LOGE(TAG, "external data slot rejected");
            return false;
        }
    }
    const std::string encoded_slots = encode_slots(cfg.slots);
    if (encoded_slots.size() > 3800) {
        ESP_LOGE(TAG, "external data slots too large for NVS");
        return false;
    }
    nvs_handle_t h;
    if (nvs_open(kNsExt, NVS_READWRITE, &h) != ESP_OK) return false;
    esp_err_t err = nvs_set_u8(h, "en", cfg.enabled ? 1 : 0);
    if (err == ESP_OK) err = nvs_set_str(h, "url", cfg.ha_url.c_str());
    if (err == ESP_OK) err = secret_set(h, kNsExt, "token", cfg.ha_token);
    if (err == ESP_OK) err = nvs_set_u16(h, "poll", cfg.poll_s);
    if (err == ESP_OK) err = nvs_set_str(h, "slots", encoded_slots.c_str());
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    if (err != ESP_OK) ESP_LOGE(TAG, "saving external data config failed: %s", esp_err_to_name(err));
    return err == ESP_OK;
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
        secret_get(h, kNsMqttLegacy, "mqtt_pass", out.password);
        nvs_get_str(h, "mqtt_dev",     out.device_name);
        nvs_get_str(h, "mqtt_ha_pref", out.ha_prefix);
    } else {
        nvs_get_u8 (h, "ha_en",        en);
        nvs_get_str(h, "ha_uri",       out.broker_uri);
        nvs_get_str(h, "ha_user",      out.username);
        secret_get(h, kNsHa, "ha_pass", out.password);
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
    secret_set(h, kNsHa, "ha_pass", cfg.password);
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
    if (nvs_get_str(h, "p_wb", s) && !s.empty()) {
        char* end = nullptr;
        const double value = std::strtod(s.c_str(), &end);
        if (end != s.c_str() && *end == '\0') out.wallbox_refund_kwh = value;
    }
    nvs_get_str(h, "cur", out.currency);
    static const char* const kMod[3] = {"pm_e", "pm_s", "pm_w"};
    static const char* const kWp[3] = {"pw_e", "pw_s", "pw_w"};
    for (int i = 0; i < 3; ++i) {
        nvs_get_u16(h, kMod[i], out.pv_modules[i]);
        nvs_get_u16(h, kWp[i], out.pv_module_wp[i]);
    }
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
    snprintf(buf, sizeof(buf), "%.4f", cfg.wallbox_refund_kwh);
    nvs_set_str(h, "p_wb", buf);
    nvs_set_str(h, "cur", cfg.currency.c_str());
    static const char* const kMod[3] = {"pm_e", "pm_s", "pm_w"};
    static const char* const kWp[3] = {"pw_e", "pw_s", "pw_w"};
    for (int i = 0; i < 3; ++i) {
        nvs_set_u16(h, kMod[i], cfg.pv_modules[i]);
        nvs_set_u16(h, kWp[i], cfg.pv_module_wp[i]);
    }
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
