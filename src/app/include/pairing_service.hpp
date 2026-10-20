#pragma once

#include <string>
#include "esp_err.h"

namespace app::pairing {

struct Status {
    bool open = false;
    int remaining_s = 0;
    std::string request_id;
    std::string name;
    std::string code;
    std::string error;
};

esp_err_t open(bool (*allow_peer)(int));
void close();
Status status();
bool decide(const std::string& request_id, bool approve);

} // namespace app::pairing
