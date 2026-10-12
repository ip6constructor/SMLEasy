#pragma once
#include <cstdint>
#include <string>

namespace app::tls_certs {

/// True if both PEM strings parse and the private key matches the certificate.
bool valid_pair(const std::string& certificate_pem, const std::string& private_key_pem);

/// Reads expiry (Unix time), subject CN and whether the issuer is a Let's Encrypt staging CA
/// from the first certificate in the PEM. Cheap enough for the HTTP server task.
bool certificate_info(const std::string& certificate_pem, int64_t& not_after_epoch,
                      std::string& subject_cn, bool& staging_issuer);

/// Generates an EC P-256 self-signed certificate (SAN = fqdn) and its private key as PEM.
/// Needs ~8 KiB of stack; run it from a task, not from the HTTP server task.
bool generate_self_signed(const std::string& fqdn, std::string& certificate_pem,
                          std::string& private_key_pem);

} // namespace app::tls_certs
