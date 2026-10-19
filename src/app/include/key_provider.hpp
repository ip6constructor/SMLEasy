#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace app::keys {

enum class Kind { Development, EFuse };

// Root-based HMAC-SHA256 only; the root itself can never be read back (see docs/SCHLUESSEL-UND-PAIRING.md §2).
class KeyProvider {
public:
    virtual ~KeyProvider() = default;
    virtual Kind kind() const = 0;
    virtual bool hmac_sha256(const uint8_t* msg, size_t len, uint8_t out[32]) = 0;
};

// Development provider: random 32-byte root generated on first use and stored in NVS (readable from flash).
// NOT production-grade; replaced by an eFuse HMAC_UP provider after explicit per-device approval.
KeyProvider& provider();

// Public device ID: trunc128(HMAC_root("smleasy/device-id/v1" || 0x00 || factory_mac)), hex.
std::string device_id_hex();

const char* kind_name(Kind k);

}  // namespace app::keys
