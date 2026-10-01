#pragma once
#include <atomic>
#include <cstdint>
#include <deque>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace app {

struct IoCounters {
    std::atomic<uint32_t> tx_bytes{0};
    std::atomic<uint32_t> rx_bytes{0};
    std::atomic<uint32_t> tx_frames{0};
    std::atomic<uint32_t> rx_frames{0};
    std::atomic<uint32_t> crc_errors{0};
    std::atomic<uint32_t> nak_count{0};
    std::atomic<uint32_t> ack_count{0};
    std::atomic<uint32_t> wakeup_count{0};
    // Session init flags (set by meter_task as each phase succeeds)
    std::atomic<bool>     flag_ident{false};
    std::atomic<bool>     flag_logon{false};
    std::atomic<bool>     flag_auth{false};
    void reset() noexcept {
        tx_bytes=0; rx_bytes=0; tx_frames=0;
        rx_frames=0; crc_errors=0; nak_count=0;
        ack_count=0; wakeup_count=0;
        flag_ident=false; flag_logon=false; flag_auth=false;
    }
};

enum class JobState : uint8_t { Idle=0, Running=1, Done=2, Error=3 };

struct MeterData {
    std::string manufacturer, model, fw_version;
    std::string serial_bcd;      ///< 16-digit decoded from BT01 BCD
    std::string utility_serial;  ///< ET03
    std::string meter_time;

    int32_t  fwd_active_wh{0}, rev_active_wh{0};
    int32_t  import_react_varh{0}, export_react_varh{0};
    uint16_t demand_reset_count{0};

    int32_t fwd_w{0}, rev_w{0};
    int32_t freq_mhz{0};
    int32_t v_l1_mv{0}, v_l2_mv{0}, v_l3_mv{0};
    int32_t i_l1_ma{0}, i_l2_ma{0}, i_l3_ma{0};
    int32_t pf_l1{0},   pf_l2{0},   pf_l3{0};

    bool has_fwd_active_wh{false}, has_rev_active_wh{false};
    bool has_power{false};
    bool has_v_l1{false}, has_v_l2{false}, has_v_l3{false};
    bool has_i_l1{false}, has_i_l2{false}, has_i_l3{false};
    bool has_frequency{false}, has_pf_l1{false};
    bool        has_alarms{false};
    std::string alarm_list;
    bool        populated{false};
};

class AppState {
public:
    static AppState& get();

    MeterData   get_meter_data() const;
    void        set_meter_data(const MeterData& d);

    JobState    job_state() const;
    void        set_job_state(JobState s);
    std::string job_error() const;
    void        set_job_error(const std::string& e);

    void        push_log(const char* level, const char* tag, const char* msg);
    std::string log_json(uint32_t after_seq) const;
    uint32_t    log_seq() const;

    static constexpr size_t kMaxLogLines = 500;

    IoCounters io;
    std::atomic<uint32_t> read_interval_s{30};
    std::atomic<bool>     continuous{true};
    std::atomic<bool>     last_login_ok{false}; ///< mirrors NVS last_lgn; set/reset by meter_task

private:
    AppState();
    AppState(const AppState&) = delete;
    AppState& operator=(const AppState&) = delete;

    mutable SemaphoreHandle_t mutex_{nullptr};
    MeterData   meter_data_;
    JobState    job_state_{JobState::Idle};
    std::string job_error_;

    struct LogLine { uint32_t seq; std::string level, tag, msg; };
    std::deque<LogLine> log_lines_;
    uint32_t            log_seq_{0};
};

} // namespace app
