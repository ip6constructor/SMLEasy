#pragma once

#include "esp_err.h"

#include <string>
#include <vector>

namespace acme_client {

struct Request {
    std::string directory_url;
    std::string fqdn;
    std::string email;
    std::string account_key_pem;
    std::string account_url;
    bool terms_accepted{false};
    /// Rehearsal: throwaway account, stops after the domain validation, no certificate.
    bool validate_only{false};
};

struct Result {
    std::string certificate_chain_pem;
    std::string private_key_pem;
    std::string account_key_pem;
    std::string account_url;
};

struct DirectoryInfo {
    std::string terms_url;
};

esp_err_t init();
esp_err_t issue_http01(const Request& request, Result& result);
/// Reads the ACME directory (reachability test) and the terms-of-service URL.
esp_err_t fetch_directory_info(const std::string& directory_url, DirectoryInfo& info);
/// Public DNS lookup over HTTPS (dns.google); type is "A" or "AAAA".
esp_err_t resolve_public(const std::string& name, const char* type, std::vector<std::string>& answers);
/// Last failure reason reported by the CA or the client (empty if none).
std::string last_error();
void clear_http01_challenge();
bool get_http01_challenge(const char* token, std::string& response);

/// Progress log of the last certificate request (ring buffer, newest last).
void add_log(const char* format, ...) __attribute__((format(printf, 1, 2)));
void clear_log();
std::vector<std::string> get_log();

} // namespace acme_client