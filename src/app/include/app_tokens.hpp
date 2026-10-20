#pragma once

#include <string>
#include <cstdint>

namespace app::tokens {

constexpr int kMaxApps = 4;

struct Info {
    bool used = false;
    std::string name;
    int64_t created_at = 0;
};

// Long-lived per-app bearer token (256 bit random). Only HMAC_root verifiers are stored in NVS.
bool issue(const std::string& name, std::string& token_hex);
bool verify(const std::string& token);
void list(Info out[kMaxApps]);
bool revoke(int slot);
bool rename(int slot, const std::string& name);
void revoke_all();

}  // namespace app::tokens
