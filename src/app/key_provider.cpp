#include "key_provider.hpp"

#include <cstring>

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_random.h"
#include "psa/crypto.h"
#include "mbedtls/platform_util.h"
#include "nvs.h"

namespace app::keys {

namespace {
const char* TAG = "keys";
constexpr const char* kNs = "devkey";
constexpr const char* kRoot = "root";

class DevKeyProvider : public KeyProvider {
public:
    Kind kind() const override { return Kind::Development; }

    bool hmac_sha256(const uint8_t* msg, size_t len, uint8_t out[32]) override
    {
        uint8_t root[32];
        if (!load_root(root)) return false;
        bool ok = false;
        if (psa_crypto_init() == PSA_SUCCESS) {
            psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
            psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_SIGN_MESSAGE);
            psa_set_key_algorithm(&attr, PSA_ALG_HMAC(PSA_ALG_SHA_256));
            psa_set_key_type(&attr, PSA_KEY_TYPE_HMAC);
            psa_key_id_t key = 0;
            if (psa_import_key(&attr, root, sizeof(root), &key) == PSA_SUCCESS) {
                size_t out_len = 0;
                ok = psa_mac_compute(key, PSA_ALG_HMAC(PSA_ALG_SHA_256), msg, len, out, 32, &out_len) ==
                         PSA_SUCCESS && out_len == 32;
                psa_destroy_key(key);
            }
        }
        mbedtls_platform_zeroize(root, sizeof(root));
        return ok;
    }

private:
    static bool load_root(uint8_t root[32])
    {
        nvs_handle_t h;
        if (nvs_open(kNs, NVS_READWRITE, &h) != ESP_OK) return false;
        size_t len = 32;
        bool ok = nvs_get_blob(h, kRoot, root, &len) == ESP_OK && len == 32;
        if (!ok) {
            // Wi-Fi is up when this runs, so the RF entropy source feeds the RNG.
            esp_fill_random(root, 32);
            ok = nvs_set_blob(h, kRoot, root, 32) == ESP_OK && nvs_commit(h) == ESP_OK;
            ESP_LOGW(TAG, "development root created (not production-grade)");
        }
        nvs_close(h);
        return ok;
    }
};
}  // namespace

KeyProvider& provider()
{
    static DevKeyProvider p;
    return p;
}

const char* kind_name(Kind k)
{
    return k == Kind::EFuse ? "efuse" : "development";
}

std::string device_id_hex()
{
    static const char ctx[] = "smleasy/device-id/v1";
    uint8_t msg[sizeof(ctx) + 6];
    std::memcpy(msg, ctx, sizeof(ctx) - 1);
    msg[sizeof(ctx) - 1] = 0;
    uint8_t mac[6] = {};
    esp_efuse_mac_get_default(mac);
    std::memcpy(msg + sizeof(ctx), mac, 6);
    uint8_t mac_out[32];
    if (!provider().hmac_sha256(msg, sizeof(msg), mac_out)) return "";
    static const char hex[] = "0123456789abcdef";
    std::string id;
    for (int i = 0; i < 16; i++) {
        id += hex[mac_out[i] >> 4];
        id += hex[mac_out[i] & 15];
    }
    return id;
}

}  // namespace app::keys
