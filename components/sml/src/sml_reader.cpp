#include "sml/sml_reader.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "esp_log.h"
#include "esp_timer.h"

namespace {

static const char* TAG = "SmlReader";

constexpr std::array<uint8_t, 8> kSmlStart = {0x1Bu,0x1Bu,0x1Bu,0x1Bu,0x01u,0x01u,0x01u,0x01u};
constexpr std::array<uint8_t, 5> kSmlEndPrefix = {0x1Bu,0x1Bu,0x1Bu,0x1Bu,0x1Au};

uint16_t crc16_x25(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            if ((crc & 0x0001u) != 0u) {
                crc = static_cast<uint16_t>((crc >> 1) ^ 0x8408u);
            } else {
                crc = static_cast<uint16_t>(crc >> 1);
            }
        }
    }
    return static_cast<uint16_t>(~crc);
}

struct TlHeader {
    bool ok{false};
    uint8_t type{0};
    size_t content_len{0};
    size_t tl_len{0};
};

struct Node {
    enum class Type : uint8_t {
        Optional,
        Bool,
        Int,
        Uint,
        Octet,
        List,
        Unknown
    };

    Type type{Type::Unknown};
    bool bool_value{false};
    int64_t int_value{0};
    uint64_t uint_value{0};
    std::vector<uint8_t> octets{};
    std::vector<Node> list{};
};

TlHeader parse_tl(const uint8_t* data, size_t len, size_t offset) {
    TlHeader out{};
    if (offset >= len) return out;

    const uint8_t first = data[offset];
    if (first == 0x00u) {
        out.ok = true;
        out.type = 0;
        out.content_len = 0;
        out.tl_len = 1;
        return out;
    }

    size_t pos = offset;
    size_t nibbled_len = 0;
    size_t tl_bytes = 0;
    uint8_t type = 0;

    while (pos < len) {
        const uint8_t b = data[pos++];
        if (tl_bytes == 0) {
            type = static_cast<uint8_t>((b >> 4) & 0x07u);
            nibbled_len = static_cast<size_t>(b & 0x0Fu);
        } else {
            nibbled_len = (nibbled_len << 4) | static_cast<size_t>(b & 0x0Fu);
        }
        ++tl_bytes;
        if ((b & 0x80u) == 0u) break;
        if (tl_bytes > 8) return out;
    }

    if (tl_bytes == 0 || (offset + tl_bytes) > len) return out;
    if (nibbled_len < tl_bytes) return out;

    out.ok = true;
    out.type = type;
    out.tl_len = tl_bytes;
    out.content_len = (type == 7u) ? nibbled_len : nibbled_len - tl_bytes;
    return out;
}

bool parse_node(const uint8_t* data, size_t len, size_t& offset, Node& out) {
    const TlHeader tl = parse_tl(data, len, offset);
    if (!tl.ok) return false;

    if ((offset + tl.tl_len) > len) return false;
    const size_t content_offset = offset + tl.tl_len;

    if (data[offset] == 0x00u || data[offset] == 0x01u) {
        out.type = Node::Type::Optional;
        offset += 1;
        return true;
    }

    if ((content_offset + tl.content_len) > len) return false;

    switch (tl.type) {
        case 0: {
            out.type = Node::Type::Octet;
            out.octets.assign(data + content_offset, data + content_offset + tl.content_len);
            offset = content_offset + tl.content_len;
            return true;
        }
        case 4: {
            out.type = Node::Type::Bool;
            out.bool_value = (tl.content_len > 0 && data[content_offset] != 0u);
            offset = content_offset + tl.content_len;
            return true;
        }
        case 5: {
            out.type = Node::Type::Int;
            if (tl.content_len == 0 || tl.content_len > 8) return false;
            int64_t v = 0;
            const bool neg = (data[content_offset] & 0x80u) != 0u;
            for (size_t i = 0; i < tl.content_len; ++i) {
                v = (v << 8) | data[content_offset + i];
            }
            if (neg && tl.content_len < 8) {
                const int shift = static_cast<int>((8 - tl.content_len) * 8);
                v = (v << shift) >> shift;
            }
            out.int_value = v;
            offset = content_offset + tl.content_len;
            return true;
        }
        case 6: {
            out.type = Node::Type::Uint;
            if (tl.content_len == 0 || tl.content_len > 8) return false;
            uint64_t v = 0;
            for (size_t i = 0; i < tl.content_len; ++i) {
                v = (v << 8) | data[content_offset + i];
            }
            out.uint_value = v;
            offset = content_offset + tl.content_len;
            return true;
        }
        case 7: {
            out.type = Node::Type::List;
            out.list.clear();
            out.list.reserve(tl.content_len);
            size_t p = content_offset;
            for (size_t i = 0; i < tl.content_len; ++i) {
                Node child{};
                if (!parse_node(data, len, p, child)) return false;
                out.list.push_back(std::move(child));
            }
            offset = p;
            return true;
        }
        default: {
            out.type = Node::Type::Unknown;
            offset = content_offset + tl.content_len;
            return true;
        }
    }
}

bool node_to_i64(const Node& n, int64_t& out, bool& is_signed) {
    if (n.type == Node::Type::Int) {
        out = n.int_value;
        is_signed = true;
        return true;
    }
    if (n.type == Node::Type::Uint) {
        if (n.uint_value > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            out = std::numeric_limits<int64_t>::max();
        } else {
            out = static_cast<int64_t>(n.uint_value);
        }
        is_signed = false;
        return true;
    }
    return false;
}

bool node_to_i8(const Node& n, int8_t& out) {
    int64_t v = 0;
    bool is_signed = false;
    if (!node_to_i64(n, v, is_signed)) return false;
    if (v < -128 || v > 127) return false;
    out = static_cast<int8_t>(v);
    return true;
}

bool node_to_u8(const Node& n, uint8_t& out) {
    if (n.type == Node::Type::Uint) {
        if (n.uint_value > 255u) return false;
        out = static_cast<uint8_t>(n.uint_value);
        return true;
    }
    if (n.type == Node::Type::Int) {
        if (n.int_value < 0 || n.int_value > 255) return false;
        out = static_cast<uint8_t>(n.int_value);
        return true;
    }
    return false;
}

double apply_scaler(int64_t raw, int8_t scaler) {
    if (scaler == 0) return static_cast<double>(raw);
    return static_cast<double>(raw) * std::pow(10.0, static_cast<double>(scaler));
}

bool looks_like_list_entry(const Node& n) {
    if (n.type != Node::Type::List) return false;
    if (n.list.size() < 6) return false;
    const Node& obis = n.list[0];
    if (obis.type != Node::Type::Octet || obis.octets.size() != 6) return false;
    bool signed_value = false;
    int64_t raw = 0;
    return node_to_i64(n.list[5], raw, signed_value);
}

void collect_list_entries(const Node& n, std::vector<sml::ObisValue>& out_values) {
    if (looks_like_list_entry(n)) {
        sml::ObisValue v{};
        const Node& obis = n.list[0];
        for (size_t i = 0; i < 6; ++i) v.obis[i] = obis.octets[i];
        v.obis_str = sml::obis_to_string(v.obis);

        bool is_signed = false;
        int64_t raw = 0;
        if (node_to_i64(n.list[5], raw, is_signed)) {
            v.value_raw = raw;
            v.is_signed = is_signed;
        }

        if (n.list.size() > 4) {
            const Node& scaler_node = n.list[4];
            int8_t scaler = 0;
            if (node_to_i8(scaler_node, scaler)) {
                v.scaler = scaler;
                v.has_scaler = true;
            }
        }

        if (n.list.size() > 3) {
            const Node& unit_node = n.list[3];
            uint8_t unit = 0;
            if (node_to_u8(unit_node, unit)) {
                v.unit = unit;
                v.has_unit = true;
            }
        }

        v.value = apply_scaler(v.value_raw, v.scaler);
        out_values.push_back(std::move(v));
    }

    if (n.type == Node::Type::List) {
        for (const Node& c : n.list) {
            collect_list_entries(c, out_values);
        }
    }
}

std::string to_hex_preview(const std::vector<uint8_t>& bytes, size_t max_count) {
    std::string out;
    const size_t n = std::min(bytes.size(), max_count);
    out.reserve(n * 3);
    for (size_t i = 0; i < n; ++i) {
        char b[4];
        std::snprintf(b, sizeof(b), "%02X", bytes[i]);
        out += b;
        if (i + 1 < n) out.push_back(' ');
    }
    return out;
}

} // namespace

namespace sml {

std::string obis_to_string(const std::array<uint8_t, 6>& obis) {
    char b[64];
    std::snprintf(b, sizeof(b), "%u-%u:%u.%u.%u*%u",
                  obis[0], obis[1], obis[2], obis[3], obis[4], obis[5]);
    return std::string(b);
}

std::string unit_to_string(uint8_t unit) {
    switch (unit) {
        case 0x1B: return "W";
        case 0x1E: return "Wh";
        case 0x20: return "varh";
        case 0x23: return "V";
        case 0x21: return "A";
        case 0x2C: return "Hz";
        default: {
            char b[16];
            std::snprintf(b, sizeof(b), "u%u", unit);
            return std::string(b);
        }
    }
}

ReadResult Reader::read_once(uint32_t timeout_ms) {
    ReadResult rr{};

    std::vector<uint8_t> stream;
    stream.reserve(4096);

    const int64_t deadline_us = esp_timer_get_time() + static_cast<int64_t>(timeout_ms) * 1000;

    std::array<uint8_t, 256> rx{};

    while (esp_timer_get_time() < deadline_us) {
        const int got = uart_read_bytes(port_, rx.data(), rx.size(), pdMS_TO_TICKS(100));
        if (got > 0) {
            rr.rx_bytes += static_cast<size_t>(got);
            stream.insert(stream.end(), rx.begin(), rx.begin() + got);

            const auto start_it = std::search(stream.begin(), stream.end(), kSmlStart.begin(), kSmlStart.end());
            if (start_it == stream.end()) {
                if (stream.size() > 2048) {
                    stream.erase(stream.begin(), stream.end() - 512);
                }
                continue;
            }

            if (start_it != stream.begin()) {
                stream.erase(stream.begin(), start_it);
            }

            const auto end_it = std::search(stream.begin() + 8, stream.end(), kSmlEndPrefix.begin(), kSmlEndPrefix.end());
            if (end_it == stream.end()) {
                continue;
            }

            const size_t end_pos = static_cast<size_t>(std::distance(stream.begin(), end_it));
            const size_t after_end = end_pos + kSmlEndPrefix.size();
            if (stream.size() < after_end + 3) {
                continue;
            }

            const uint8_t fill = stream[after_end];
            const size_t total_len = after_end + 1 + 2;
            if (stream.size() < total_len) {
                continue;
            }

            rr.frame_bytes = total_len;

            const uint8_t crc_lo = stream[total_len - 2];
            const uint8_t crc_hi = stream[total_len - 1];
            const uint16_t crc_wire_le = static_cast<uint16_t>(crc_lo | (static_cast<uint16_t>(crc_hi) << 8));
            const uint16_t crc_wire_be = static_cast<uint16_t>((static_cast<uint16_t>(crc_lo) << 8) | crc_hi);

            const uint16_t crc_calc = crc16_x25(stream.data(), total_len - 2);
            rr.valid_crc = (crc_calc == crc_wire_le) || (crc_calc == crc_wire_be);

            const size_t payload_begin = kSmlStart.size();
            size_t payload_end = end_pos;

            if (fill <= 3u && payload_end >= fill) {
                bool pad_ok = true;
                for (size_t i = 0; i < fill; ++i) {
                    if (stream[payload_end - 1 - i] != 0x00u) {
                        pad_ok = false;
                        break;
                    }
                }
                if (pad_ok) payload_end -= fill;
            }

            if (payload_end <= payload_begin) {
                rr.error = "SML payload leer";
                return rr;
            }

            size_t off = payload_begin;
            while (off < payload_end) {
                Node root{};
                if (!parse_node(stream.data(), payload_end, off, root)) {
                    rr.error = "SML TL-Dekodierung fehlgeschlagen";
                    ESP_LOGW(TAG, "TL decode failed, frame preview: %s",
                             to_hex_preview(stream, 64).c_str());
                    return rr;
                }
                collect_list_entries(root, rr.values);
            }
            if (rr.values.empty()) {
                rr.error = "Keine SML ListEntry-Daten gefunden";
                return rr;
            }

            rr.ok = true;
            return rr;
        }
    }

    rr.error = "Timeout: kein vollständiger SML-Frame";
    return rr;
}

} // namespace sml
