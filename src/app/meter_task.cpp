#include "meter_task.hpp"

#include "app_state.hpp"
#include "config_store.hpp"
#include "mqtt_manager.hpp"
#include "energy_stats.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sml/sml_reader.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static const char* TAG = "MeterTask";
static uart_port_t s_uart_port = UART_NUM_1;

namespace {

using ObisCode = std::array<uint8_t, 6>;

bool obis_in(const ObisCode& obis, const std::vector<ObisCode>& aliases) {
    for (const auto& candidate : aliases) {
        if (std::equal(obis.begin(), obis.end(), candidate.begin())) {
            return true;
        }
    }
    return false;
}

bool parse_obis_code(const std::string& text, ObisCode& out) {
    unsigned int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    if (std::sscanf(text.c_str(), "%u-%u:%u.%u.%u*%u", &a, &b, &c, &d, &e, &f) != 6) {
        return false;
    }
    if (a > 255u || b > 255u || c > 255u || d > 255u || e > 255u || f > 255u) {
        return false;
    }
    out = ObisCode{
        static_cast<uint8_t>(a), static_cast<uint8_t>(b), static_cast<uint8_t>(c),
        static_cast<uint8_t>(d), static_cast<uint8_t>(e), static_cast<uint8_t>(f),
    };
    return true;
}

std::vector<ObisCode> parse_obis_aliases(const std::string& text) {
    std::vector<ObisCode> out;
    std::string token;
    auto flush_token = [&]() {
        if (token.empty()) return;
        ObisCode code{};
        if (parse_obis_code(token, code)) {
            out.push_back(code);
        }
        token.clear();
    };

    for (char ch : text) {
        if (ch == ',' || ch == ';' || std::isspace(static_cast<unsigned char>(ch))) {
            flush_token();
        } else {
            token.push_back(ch);
        }
    }
    flush_token();
    return out;
}

struct ObisRuntimeMap {
    std::vector<ObisCode> import_wh{};
    std::vector<ObisCode> export_wh{};
    std::vector<ObisCode> power_net_w{};
    std::vector<ObisCode> power_import_w{};
    std::vector<ObisCode> power_export_w{};
    std::vector<ObisCode> voltage_l1_v{};
    std::vector<ObisCode> voltage_l2_v{};
    std::vector<ObisCode> voltage_l3_v{};
    std::vector<ObisCode> current_l1_a{};
    std::vector<ObisCode> current_l2_a{};
    std::vector<ObisCode> current_l3_a{};
    std::vector<ObisCode> frequency_hz{};
    std::vector<ObisCode> pf_l1{};
};

std::vector<ObisCode> parse_with_fallback(
    const std::string& configured,
    const std::string& fallback,
    const char* field_name,
    app::AppState& st
) {
    if (configured.empty()) return {};
    auto parsed = parse_obis_aliases(configured);
    if (!parsed.empty()) {
        return parsed;
    }
    st.push_log("W", TAG, (std::string("Ungueltiges OBIS-Mapping fuer ") + field_name + ", nutze Default").c_str());
    parsed = parse_obis_aliases(fallback);
    return parsed;
}

ObisRuntimeMap build_obis_runtime_map(const app::MeterConfig& cfg, app::AppState& st) {
    ObisRuntimeMap map{};
    const app::MeterConfig defaults{};

    map.import_wh = parse_with_fallback(cfg.obis_import_wh, defaults.obis_import_wh, "import_wh", st);
    map.export_wh = parse_with_fallback(cfg.obis_export_wh, defaults.obis_export_wh, "export_wh", st);
    map.power_net_w = parse_with_fallback(cfg.obis_power_net_w, defaults.obis_power_net_w, "power_net_w", st);
    map.power_import_w = parse_with_fallback(cfg.obis_power_import_w, defaults.obis_power_import_w, "power_import_w", st);
    map.power_export_w = parse_with_fallback(cfg.obis_power_export_w, defaults.obis_power_export_w, "power_export_w", st);
    map.voltage_l1_v = parse_with_fallback(cfg.obis_voltage_l1_v, defaults.obis_voltage_l1_v, "voltage_l1_v", st);
    map.voltage_l2_v = parse_with_fallback(cfg.obis_voltage_l2_v, defaults.obis_voltage_l2_v, "voltage_l2_v", st);
    map.voltage_l3_v = parse_with_fallback(cfg.obis_voltage_l3_v, defaults.obis_voltage_l3_v, "voltage_l3_v", st);
    map.current_l1_a = parse_with_fallback(cfg.obis_current_l1_a, defaults.obis_current_l1_a, "current_l1_a", st);
    map.current_l2_a = parse_with_fallback(cfg.obis_current_l2_a, defaults.obis_current_l2_a, "current_l2_a", st);
    map.current_l3_a = parse_with_fallback(cfg.obis_current_l3_a, defaults.obis_current_l3_a, "current_l3_a", st);
    map.frequency_hz = parse_with_fallback(cfg.obis_frequency_hz, defaults.obis_frequency_hz, "frequency_hz", st);
    map.pf_l1 = parse_with_fallback(cfg.obis_pf_l1, defaults.obis_pf_l1, "pf_l1", st);
    return map;
}

int32_t to_i32_rounded(double v) {
    if (v > static_cast<double>(INT32_MAX)) return INT32_MAX;
    if (v < static_cast<double>(INT32_MIN)) return INT32_MIN;
    return static_cast<int32_t>(std::llround(v));
}

std::string make_entry_log(const sml::ObisValue& v, bool valid_crc) {
    char buf[256];
    const char* unit = v.has_unit ? sml::unit_to_string(v.unit).c_str() : "";
    std::snprintf(buf, sizeof(buf),
                  "{\"obis\":\"%s\",\"value_raw\":%lld,\"scaler\":%d,\"unit\":\"%s\",\"value\":%.6f,\"timestamp\":%lld,\"valid_crc\":%s}",
                  v.obis_str.c_str(),
                  static_cast<long long>(v.value_raw),
                  static_cast<int>(v.scaler),
                  unit,
                  v.value,
                  static_cast<long long>(esp_timer_get_time() / 1000),
                  valid_crc ? "true" : "false");
    return std::string(buf);
}

std::string replace_all(std::string text, const std::string& needle, const std::string& replacement) {
    if (needle.empty()) return text;
    std::string::size_type pos = 0;
    while ((pos = text.find(needle, pos)) != std::string::npos) {
        text.replace(pos, needle.size(), replacement);
        pos += replacement.size();
    }
    return text;
}

int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

std::string decode_escapes(const std::string& in) {
    std::string out;
    out.reserve(in.size());

    for (size_t i = 0; i < in.size(); ++i) {
        const char c = in[i];
        if (c != '\\' || i + 1 >= in.size()) {
            out.push_back(c);
            continue;
        }

        const char n = in[++i];
        switch (n) {
            case 'r': out.push_back('\r'); break;
            case 'n': out.push_back('\n'); break;
            case 't': out.push_back('\t'); break;
            case '\\': out.push_back('\\'); break;
            case 'x': {
                if (i + 2 < in.size()) {
                    const int hi = hex_nibble(in[i + 1]);
                    const int lo = hex_nibble(in[i + 2]);
                    if (hi >= 0 && lo >= 0) {
                        out.push_back(static_cast<char>((hi << 4) | lo));
                        i += 2;
                        break;
                    }
                }
                out.push_back('x');
                break;
            }
            default:
                out.push_back(n);
                break;
        }
    }

    return out;
}

void send_optional_login(uart_port_t port, const app::MeterConfig& cfg, app::AppState& st) {
    if (cfg.login_cmd.empty()) return;

    const std::string with_pin = replace_all(cfg.login_cmd, "{PIN}", cfg.meter_pin);
    const std::string payload = decode_escapes(with_pin);
    if (payload.empty()) return;

    uart_flush_input(port);
    const int written = uart_write_bytes(port, payload.data(), payload.size());
    if (written > 0) {
        st.io.tx_bytes.fetch_add(static_cast<uint32_t>(written));
        st.io.tx_frames.fetch_add(1u);
        st.io.wakeup_count.fetch_add(1u);
    }
    uart_wait_tx_done(port, pdMS_TO_TICKS(250));

    const uint16_t wait_ms = (cfg.login_wait_ms < 20u) ? 20u : cfg.login_wait_ms;
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
}

} // namespace

namespace app {

MeterTask& MeterTask::get() {
    static MeterTask inst;
    return inst;
}

bool MeterTask::is_running() const {
    return task_running_.load();
}

void MeterTask::init(int tx_pin, int rx_pin, int uart_num) {
    if (uart_installed_) return;

    tx_pin_ = tx_pin;
    rx_pin_ = rx_pin;
    s_uart_port = static_cast<uart_port_t>(uart_num);

    uart_config_t uc{};
    uc.baud_rate = 9600;
    uc.data_bits = UART_DATA_8_BITS;
    uc.parity = UART_PARITY_DISABLE;
    uc.stop_bits = UART_STOP_BITS_1;
    uc.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uc.source_clk = UART_SCLK_APB;

    uart_driver_install(s_uart_port, 4096, 512, 0, nullptr, 0);
    uart_param_config(s_uart_port, &uc);
    uart_set_pin(s_uart_port, tx_pin_, rx_pin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    gpio_pullup_en(static_cast<gpio_num_t>(rx_pin_));
    uart_set_rx_full_threshold(s_uart_port, 16);
    uart_set_rx_timeout(s_uart_port, 10);

    uart_installed_ = true;
    ESP_LOGI(TAG, "UART%d installed TX=%d RX=%d @ 9600 8N1 (passive SML)", uart_num, tx_pin, rx_pin);
}

void MeterTask::start() {
    auto& st = AppState::get();
    if (is_running()) {
        ESP_LOGW(TAG, "task already running - trigger");
        trigger();
        return;
    }

    stop_requested_ = false;
    task_running_.store(true);
    xTaskCreate(task_fn, "meter_task", 12288, this, 12, &task_handle_);
    ESP_LOGI(TAG, "meter task started (continuous=%s)", st.continuous.load() ? "yes" : "no");
}

void MeterTask::stop() {
    AppState::get().continuous.store(false);
    stop_requested_ = true;
    ESP_LOGI(TAG, "stop requested");
}

void MeterTask::start_continuous() {
    AppState::get().continuous.store(true);
    start();
}

void MeterTask::trigger() {
    if (task_handle_) {
        xTaskNotifyGive(task_handle_);
    }
}

void MeterTask::task_fn(void* arg) {
    static_cast<MeterTask*>(arg)->run();
    vTaskDelete(nullptr);
}

void MeterTask::run() {
    auto& st = AppState::get();

    while (!stop_requested_) {
        st.set_job_state(JobState::Running);
        st.set_job_error("");
        st.push_log("I", TAG, "=== SML-Ablesung gestartet ===");
        st.io.reset();

        MeterConfig cfg;
        ConfigStore::get().load_meter(cfg);
        const ObisRuntimeMap obis_map = build_obis_runtime_map(cfg, st);

        uart_set_parity(s_uart_port, UART_PARITY_DISABLE);
        uart_flush_input(s_uart_port);
        send_optional_login(s_uart_port, cfg, st);

        sml::Reader reader(s_uart_port);
        const auto rr = reader.read_once(12000);

        st.io.rx_bytes.store(static_cast<uint32_t>(rr.rx_bytes));
        st.io.rx_frames.store(rr.ok ? 1u : 0u);
        st.io.crc_errors.store((!rr.valid_crc && rr.frame_bytes > 0) ? 1u : 0u);
        st.io.flag_ident.store(rr.frame_bytes > 0);
        st.io.flag_logon.store(rr.ok);
        st.io.flag_auth.store(rr.valid_crc);

        if (!rr.ok) {
            st.set_job_state(JobState::Error);
            st.set_job_error(rr.error.empty() ? "SML-Read fehlgeschlagen" : rr.error);
            st.push_log("E", TAG, rr.error.empty() ? "SML-Read fehlgeschlagen" : rr.error.c_str());
        } else {
            MeterData md;
            md.manufacturer = cfg.manufacturer.empty() ? "Unknown" : cfg.manufacturer;
            md.model = cfg.model.empty() ? cfg.profile_name : cfg.model;
            md.populated = true;

            bool import_seen = false;
            bool export_seen = false;
            bool power_seen = false;
            bool import_power_seen = false;
            bool export_power_seen = false;

            for (const auto& v : rr.values) {
                st.push_log("I", TAG, make_entry_log(v, rr.valid_crc).c_str());

                if (!import_seen && obis_in(v.obis, obis_map.import_wh)) {
                    md.fwd_active_wh = to_i32_rounded(v.value);
                    md.has_fwd_active_wh = true;
                    import_seen = true;
                } else if (!export_seen && obis_in(v.obis, obis_map.export_wh)) {
                    md.rev_active_wh = to_i32_rounded(v.value);
                    md.has_rev_active_wh = true;
                    export_seen = true;
                } else if (obis_in(v.obis, obis_map.power_net_w)) {
                    const int32_t p = to_i32_rounded(v.value);
                    if (p >= 0) {
                        md.fwd_w = p;
                        md.rev_w = 0;
                    } else {
                        md.fwd_w = 0;
                        md.rev_w = -p;
                    }
                    power_seen = true;
                } else if (obis_in(v.obis, obis_map.power_import_w)) {
                    md.fwd_w = std::abs(to_i32_rounded(v.value));
                    import_power_seen = true;
                } else if (obis_in(v.obis, obis_map.power_export_w)) {
                    md.rev_w = std::abs(to_i32_rounded(v.value));
                    export_power_seen = true;
                } else if (obis_in(v.obis, obis_map.voltage_l1_v)) {
                    md.v_l1_mv = to_i32_rounded(v.value * 1000.0);
                    md.has_v_l1 = true;
                } else if (obis_in(v.obis, obis_map.voltage_l2_v)) {
                    md.v_l2_mv = to_i32_rounded(v.value * 1000.0);
                    md.has_v_l2 = true;
                } else if (obis_in(v.obis, obis_map.voltage_l3_v)) {
                    md.v_l3_mv = to_i32_rounded(v.value * 1000.0);
                    md.has_v_l3 = true;
                } else if (obis_in(v.obis, obis_map.current_l1_a)) {
                    md.i_l1_ma = to_i32_rounded(v.value * 1000.0);
                    md.has_i_l1 = true;
                } else if (obis_in(v.obis, obis_map.current_l2_a)) {
                    md.i_l2_ma = to_i32_rounded(v.value * 1000.0);
                    md.has_i_l2 = true;
                } else if (obis_in(v.obis, obis_map.current_l3_a)) {
                    md.i_l3_ma = to_i32_rounded(v.value * 1000.0);
                    md.has_i_l3 = true;
                } else if (obis_in(v.obis, obis_map.frequency_hz)) {
                    md.freq_mhz = to_i32_rounded(v.value * 1000.0);
                    md.has_frequency = true;
                } else if (obis_in(v.obis, obis_map.pf_l1)) {
                    md.pf_l1 = to_i32_rounded(v.value * 10.0);
                    md.has_pf_l1 = true;
                } else {
                    std::string unk = "Unbekanntes OBIS protokolliert: " + v.obis_str;
                    st.push_log("I", TAG, unk.c_str());
                }
            }

            if (!power_seen && (import_power_seen || export_power_seen)) {
                // Fallback for meters that provide 1.7.0 + 2.7.0 instead of signed 16.7.0.
                power_seen = true;
            }
            md.has_power = power_seen;

            if (!import_seen) st.push_log("W", TAG, "OBIS 1-0:1.8.0*255 nicht gefunden");
            if (!export_seen) st.push_log("W", TAG, "OBIS 1-0:2.8.0*255 nicht gefunden");
            if (!power_seen)  st.push_log("W", TAG, "OBIS 1-0:16.7.0*255 nicht gefunden");

            st.set_meter_data(md);
            MqttManager::get().publish(md);
            EnergyStats::get().record(md.fwd_w - md.rev_w, md.fwd_active_wh, md.rev_active_wh);
            st.set_job_state(JobState::Done);
            st.push_log("I", TAG, rr.valid_crc
                ? "=== SML-Ablesung erfolgreich (CRC OK) ==="
                : "=== SML-Ablesung mit CRC-Warnung ===");
        }

        if (!st.continuous.load() || stop_requested_) break;

        uint32_t remaining_ms = st.read_interval_s.load() * 1000u;
        while (remaining_ms > 0 && !stop_requested_) {
            const uint32_t slice = (remaining_ms > 1000u) ? 1000u : remaining_ms;
            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(slice)) > 0) break;
            remaining_ms -= slice;
        }
    }

    st.set_job_state(JobState::Idle);
    ESP_LOGI(TAG, "meter task exiting");
    task_running_.store(false);
    task_handle_ = nullptr;
}

} // namespace app
