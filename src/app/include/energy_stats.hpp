#pragma once
#include <cstdint>
#include <deque>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace app {

struct PowerSample {
    uint32_t t_epoch_s;  ///< wall-clock seconds since epoch (0 if clock not yet synced)
    int32_t  power_w;    ///< net power at sample time (positive = import)
};

struct StatsSnapshot {
    double today_import_kwh{0};
    double today_export_kwh{0};
    double yesterday_import_kwh{0};
    double yesterday_export_kwh{0};
    double month_import_kwh{0};
    double month_export_kwh{0};
    bool   time_synced{false};
};

/// Energy statistics from the meter's own counters. The current-day baseline and
/// completed previous-day totals are persisted; monthly totals and history stay in RAM.
class EnergyStats {
public:
    static EnergyStats& get();

    /// Feed one meter reading. fwd_wh/rev_wh are the meter's cumulative energy
    /// counters (Wh); power_net_w is the instantaneous net power (W).
    void record(int32_t power_net_w, int32_t fwd_wh, int32_t rev_wh);

    StatsSnapshot snapshot() const;

    /// JSON array of {"t":epoch_s,"p":watts}, oldest first.
    std::string history_json() const;

    /// Ring buffer capacity — sized for ~6h of samples at the default 30s
    /// read interval so it stays well within the ESP32-C3's ~320KB RAM budget.
    static constexpr size_t kMaxSamples = 720;

private:
    EnergyStats();
    EnergyStats(const EnergyStats&) = delete;
    EnergyStats& operator=(const EnergyStats&) = delete;

    void check_rollover(int day_of_year, int year, int month);
    void load_persisted_day();
    void persist_day_snapshot() const;

    mutable SemaphoreHandle_t mutex_{nullptr};
    std::deque<PowerSample> hist_;

    bool    have_prev_{false};
    int32_t prev_fwd_wh_{0};
    int32_t prev_rev_wh_{0};

    int day_{-1}, year_{-1}, month_{-1};
    bool have_day_start_{false};
    int32_t day_start_import_wh_{0};
    int32_t day_start_export_wh_{0};

    double today_import_kwh_{0}, today_export_kwh_{0};
    double yesterday_import_kwh_{0}, yesterday_export_kwh_{0};
    double month_import_kwh_{0}, month_export_kwh_{0};
};

} // namespace app
