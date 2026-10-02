#pragma once

#include "esp_err.h"

#include <string>

namespace acme_client {

struct Request {
    std::string directory_url;
    std::string fqdn;
    std::string email;
    std::string account_key_pem;
    std::string account_url;
    bool terms_accepted{false};
};

struct Result {
    std::string certificate_chain_pem;
    std::string private_key_pem;
    std::string account_key_pem;
    std::string account_url;
};

esp_err_t init();
esp_err_t issue_http01(const Request& request, Result& result);
void clear_http01_challenge();
bool get_http01_challenge(const char* token, std::string& response);

} // namespace acme_client