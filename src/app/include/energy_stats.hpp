#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

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
    double week_import_kwh{0};
    double week_export_kwh{0};
    double week_cost_final{0};   ///< cost of completed days of this week (tariff of that day)
    double month_cost_final{0};  ///< cost of completed days of this month
    double today_house_kwh{0};   ///< import - export + PV - battery charge + battery discharge
    double week_house_kwh{0};
    double month_house_kwh{0};
    bool   have_house{false};    ///< true when PV or battery energy is known
    bool   time_synced{false};
};

/// One completed day (key = yyyymmdd) or month (key = yyyymm). Cost is stored in
/// thousandths of the tariff currency, computed with the tariff valid at close time.
struct PeriodRecord {
    uint32_t key;
    uint32_t import_wh;
    uint32_t export_wh;
    int32_t  cost_milli;   ///< net cost: import*price - export*feed-in
    uint32_t pv_wh;        ///< PV production (from Home Assistant), 0 if unknown
    uint32_t bat_chg_wh;   ///< energy into the battery
    uint32_t bat_dis_wh;   ///< energy out of the battery
    int32_t  sav_milli;    ///< saving from PV used in the house: (pv - export) * import price
};

struct DailyBaselineSnapshot {
    int32_t import_wh{0};
    int32_t export_wh{0};
    bool available{false};
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
    DailyBaselineSnapshot daily_baseline() const;
    bool set_daily_baseline(int32_t import_wh, int32_t export_wh);

    /// JSON array of {"t":epoch_s,"p":watts}, oldest first.
    std::string history_json() const;

    /// Long-term statistics: completed days (kept kMaxDays), weeks derived from them and
    /// months (kept kMaxMonths), each including the running period flagged "p":1.
    /// Prices are only used for the running day; closed periods keep their stored cost.
    std::string stats_json(double price_import, double price_export, const char* currency) const;

    static constexpr size_t kMaxDays = 60;
    static constexpr size_t kMaxMonths = 36;

    /// Ring buffer capacity — sized for ~6h of samples at the default 30s
    /// read interval so it stays well within the ESP32-C3's ~320KB RAM budget.
    static constexpr size_t kMaxSamples = 720;

private:
    EnergyStats();
    EnergyStats(const EnergyStats&) = delete;
    EnergyStats& operator=(const EnergyStats&) = delete;

    void check_rollover(int day_of_year, int year, int month);
    void load_persisted_day();
    bool persist_day_snapshot() const;
    void finalize_day();
    bool persist_history() const;
    void load_history();

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

    void integrate_ha_power();
    void persist_accumulators() const;

    double pv_wh_{0}, bat_chg_wh_{0}, bat_dis_wh_{0};
    int64_t last_integration_us_{0};
    int64_t last_acc_persist_us_{0};

    uint32_t date_key_{0};  ///< yyyymmdd of the running day, 0 = unknown
    bool    have_late_{false};
    int32_t late_fwd_wh_{0}, late_rev_wh_{0};
    std::vector<PeriodRecord> days_;
    std::vector<PeriodRecord> months_;
};

} // namespace app
