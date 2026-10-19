#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace app::secrets {

// AES-256-GCM sealing of NVS secrets with K_store = HMAC_root("smleasy/local-storage/v1").
// Blob layout: version(1) | nonce(12, random) | ciphertext | tag(16). AAD binds version and label.
bool seal(const std::string& label, const std::string& plain, std::vector<uint8_t>& blob);
bool open(const std::string& label, const uint8_t* blob, size_t len, std::string& plain);

// HMAC_root verifier for the web password (the password itself is never stored).
bool password_verifier(const std::string& password, uint8_t out[32]);

}  // namespace app::secrets
