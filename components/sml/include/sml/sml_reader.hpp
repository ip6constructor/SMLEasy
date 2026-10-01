#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "driver/uart.h"

namespace sml {

struct ObisValue {
    std::array<uint8_t, 6> obis{};
    std::string obis_str{};
    int64_t value_raw{0};
    int8_t scaler{0};
    uint8_t unit{0};
    bool has_scaler{false};
    bool has_unit{false};
    bool is_signed{false};
    double value{0.0};
};

struct ReadResult {
    bool ok{false};
    bool valid_crc{false};
    std::string error{};
    std::vector<ObisValue> values{};
    size_t rx_bytes{0};
    size_t frame_bytes{0};
};

class Reader {
public:
    explicit Reader(uart_port_t port) noexcept : port_(port) {}

    ReadResult read_once(uint32_t timeout_ms);

private:
    uart_port_t port_;
};

std::string obis_to_string(const std::array<uint8_t, 6>& obis);
std::string unit_to_string(uint8_t unit);

} // namespace sml
