#include "secret_store.hpp"

#include <cstring>

#include "esp_random.h"
#include "key_provider.hpp"
#include "mbedtls/platform_util.h"
#include "psa/crypto.h"

namespace app::secrets {

namespace {
constexpr uint8_t kVersion = 1;
constexpr size_t kNonce = 12;
constexpr size_t kTag = 16;

bool derive(const char* context, uint8_t out[32])
{
    return keys::provider().hmac_sha256(reinterpret_cast<const uint8_t*>(context), std::strlen(context), out);
}

std::string aad_for(const std::string& label)
{
    std::string aad("smleasy/nvs/v1");
    aad.push_back('\0');
    aad.push_back(static_cast<char>(kVersion));
    aad += label;
    return aad;
}

bool import_aes(const uint8_t key_bytes[32], psa_key_id_t& id)
{
    if (psa_crypto_init() != PSA_SUCCESS) return false;
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attr, PSA_ALG_GCM);
    psa_set_key_type(&attr, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&attr, 256);
    return psa_import_key(&attr, key_bytes, 32, &id) == PSA_SUCCESS;
}
}  // namespace

bool seal(const std::string& label, const std::string& plain, std::vector<uint8_t>& blob)
{
    uint8_t k[32];
    if (!derive("smleasy/local-storage/v1", k)) return false;
    psa_key_id_t id = 0;
    const bool imported = import_aes(k, id);
    mbedtls_platform_zeroize(k, sizeof(k));
    if (!imported) return false;

    blob.assign(1 + kNonce + plain.size() + kTag, 0);
    blob[0] = kVersion;
    esp_fill_random(blob.data() + 1, kNonce);
    const std::string aad = aad_for(label);
    size_t out_len = 0;
    const psa_status_t st = psa_aead_encrypt(
        id, PSA_ALG_GCM, blob.data() + 1, kNonce,
        reinterpret_cast<const uint8_t*>(aad.data()), aad.size(),
        reinterpret_cast<const uint8_t*>(plain.data()), plain.size(),
        blob.data() + 1 + kNonce, plain.size() + kTag, &out_len);
    psa_destroy_key(id);
    if (st != PSA_SUCCESS || out_len != plain.size() + kTag) {
        blob.clear();
        return false;
    }
    return true;
}

bool open(const std::string& label, const uint8_t* blob, size_t len, std::string& plain)
{
    if (len < 1 + kNonce + kTag || blob[0] != kVersion) return false;
    uint8_t k[32];
    if (!derive("smleasy/local-storage/v1", k)) return false;
    psa_key_id_t id = 0;
    const bool imported = import_aes(k, id);
    mbedtls_platform_zeroize(k, sizeof(k));
    if (!imported) return false;

    const size_t ct_len = len - 1 - kNonce;
    std::string out(ct_len - kTag, '\0');
    const std::string aad = aad_for(label);
    size_t out_len = 0;
    const psa_status_t st = psa_aead_decrypt(
        id, PSA_ALG_GCM, blob + 1, kNonce,
        reinterpret_cast<const uint8_t*>(aad.data()), aad.size(),
        blob + 1 + kNonce, ct_len,
        reinterpret_cast<uint8_t*>(&out[0]), out.size() + 1, &out_len);
    psa_destroy_key(id);
    if (st != PSA_SUCCESS) return false;
    out.resize(out_len);
    plain = std::move(out);
    return true;
}

bool password_verifier(const std::string& password, uint8_t out[32])
{
    std::string msg("smleasy/web-password/v1");
    msg.push_back('\0');
    msg += password;
    const bool ok = keys::provider().hmac_sha256(reinterpret_cast<const uint8_t*>(msg.data()), msg.size(), out);
    mbedtls_platform_zeroize(&msg[0], msg.size());
    return ok;
}

}  // namespace app::secrets
