#include "app_tokens.hpp"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <mutex>

#include "esp_random.h"
#include "esp_log.h"
#include "key_provider.hpp"
#include "mbedtls/platform_util.h"
#include "nvs.h"

namespace app::tokens {

namespace {
constexpr const char* kNs = "app_tok";
std::mutex mutex;

bool verifier(const std::string& token, uint8_t out[32])
{
    std::string msg("smleasy/app-token/v1");
    msg.push_back('\0');
    msg += token;
    const bool ok = keys::provider().hmac_sha256(reinterpret_cast<const uint8_t*>(msg.data()), msg.size(), out);
    mbedtls_platform_zeroize(&msg[0], msg.size());
    return ok;
}

std::string key(char kind, int slot)
{
    char k[8];
    std::snprintf(k, sizeof(k), "%c%d", kind, slot);
    return k;
}

bool slot_verifier(nvs_handle_t h, int slot, uint8_t out[32])
{
    size_t n = 32;
    return nvs_get_blob(h, key('v', slot).c_str(), out, &n) == ESP_OK && n == 32;
}
}  // namespace

bool issue(const std::string& name, std::string& token_hex)
{
    std::lock_guard<std::mutex> lock(mutex);
    nvs_handle_t h;
    if (nvs_open(kNs, NVS_READWRITE, &h) != ESP_OK) return false;
    int free_slot = -1;
    for (int i = 0; i < kMaxApps && free_slot < 0; ++i) {
        uint8_t v[32];
        if (!slot_verifier(h, i, v)) free_slot = i;
    }
    if (free_slot < 0) {
        nvs_close(h);
        return false;
    }
    uint8_t raw[32];
    esp_fill_random(raw, sizeof(raw));
    char hex[65];
    for (size_t i = 0; i < sizeof(raw); ++i) std::snprintf(hex + 2 * i, 3, "%02x", raw[i]);
    mbedtls_platform_zeroize(raw, sizeof(raw));
    token_hex = hex;
    uint8_t v[32];
    bool ok = verifier(token_hex, v);
    if (ok) ok = nvs_set_blob(h, key('v', free_slot).c_str(), v, sizeof(v)) == ESP_OK;
    if (ok) ok = nvs_set_str(h, key('n', free_slot).c_str(), name.substr(0, 40).c_str()) == ESP_OK;
    const int64_t now = std::time(nullptr);
    if (ok) ok = nvs_set_i64(h, key('t', free_slot).c_str(), now >= 1700000000 ? now : 0) == ESP_OK;
    if (ok) ok = nvs_commit(h) == ESP_OK;
    if (!ok) {
        bool cleaned = true;
        for (char kind : {'v', 'n', 't'}) {
            const esp_err_t err = nvs_erase_key(h, key(kind, free_slot).c_str());
            if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) cleaned = false;
        }
        if (nvs_commit(h) != ESP_OK) cleaned = false;
        ESP_LOGE("AppTokens", "token creation failed; partial entry cleanup %s", cleaned ? "complete" : "failed");
    }
    nvs_close(h);
    if (!ok) token_hex.clear();
    return ok;
}

bool verify(const std::string& token)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (token.size() != 64) return false;
    uint8_t calc[32];
    if (!verifier(token, calc)) return false;
    nvs_handle_t h;
    if (nvs_open(kNs, NVS_READONLY, &h) != ESP_OK) return false;
    uint8_t match = 0;
    for (int i = 0; i < kMaxApps; ++i) {
        uint8_t v[32];
        if (!slot_verifier(h, i, v)) continue;
        uint8_t diff = 0;
        for (size_t j = 0; j < 32; ++j) diff |= v[j] ^ calc[j];
        if (diff == 0) match = 1;
    }
    nvs_close(h);
    return match != 0;
}

void list(Info out[kMaxApps])
{
    std::lock_guard<std::mutex> lock(mutex);
    nvs_handle_t h;
    if (nvs_open(kNs, NVS_READONLY, &h) != ESP_OK) return;
    for (int i = 0; i < kMaxApps; ++i) {
        uint8_t v[32];
        out[i].used = slot_verifier(h, i, v);
        if (!out[i].used) continue;
        char name[48] = {};
        size_t n = sizeof(name);
        if (nvs_get_str(h, key('n', i).c_str(), name, &n) == ESP_OK) out[i].name = name;
        nvs_get_i64(h, key('t', i).c_str(), &out[i].created_at);
    }
    nvs_close(h);
}

bool revoke(int slot)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (slot < 0 || slot >= kMaxApps) return false;
    nvs_handle_t h;
    if (nvs_open(kNs, NVS_READWRITE, &h) != ESP_OK) return false;
    uint8_t v[32];
    bool ok = slot_verifier(h, slot, v);
    if (ok) ok = nvs_erase_key(h, key('v', slot).c_str()) == ESP_OK;
    if (ok) ok = nvs_erase_key(h, key('n', slot).c_str()) == ESP_OK;
    const esp_err_t time_result = nvs_erase_key(h, key('t', slot).c_str());
    if (time_result != ESP_OK && time_result != ESP_ERR_NVS_NOT_FOUND) ok = false;
    if (ok) ok = nvs_commit(h) == ESP_OK;
    nvs_close(h);
    return ok;
}

bool rename(int slot, const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (slot < 0 || slot >= kMaxApps || name.empty() || name.size() > 40) return false;
    for (unsigned char c : name) if (c < 32) return false;
    nvs_handle_t h;
    if (nvs_open(kNs, NVS_READWRITE, &h) != ESP_OK) return false;
    uint8_t v[32];
    const bool ok = slot_verifier(h, slot, v) &&
        nvs_set_str(h, key('n', slot).c_str(), name.c_str()) == ESP_OK && nvs_commit(h) == ESP_OK;
    nvs_close(h);
    return ok;
}

void revoke_all()
{
    for (int i = 0; i < kMaxApps; ++i) revoke(i);
}

}  // namespace app::tokens
