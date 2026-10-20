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
    // Index 0 = today, 1 = week, 2 = month (running period included).
    double pv_kwh[3]{}, chg_kwh[3]{}, dis_kwh[3]{}, wb_kwh[3]{};
    double wb_result[3]{}, saving[3]{};  ///< currency units
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
    uint32_t wb_wh;        ///< wallbox energy (company car), 0 if unknown
    int32_t  wb_milli;     ///< wallbox result: reimbursement - import price * energy
};

/// Compact stored period (20 bytes). Tiered retention: days (current month length), weeks (up to 26 weeks), months (up to 24 months), years (older).
/// Energy and money are stored in units of 1 / 10 / 100 / 1000 Wh resp. 0.01 / 0.1 / 1 / 10 currency for day / week / month / year.
#pragma pack(push, 1)
struct DayRec {
    uint16_t day;      ///< bits 0-13: days since 2020-01-01 (week: Monday, month: 1st), bits 14-15: 0 day, 1 week, 2 month, 3 year (Jan 1st)
    uint16_t imp, exp, pv, chg, dis, wb;
    int16_t  cost_c, sav_c, wbres_c;
};
#pragma pack(pop)

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
    /// Corrects the PV energy of the running day or a stored single day (key yyyymmdd).
    bool set_pv_wh(uint32_t key, uint32_t pv_wh, double price_import);

    /// JSON array of {"t":epoch_s,"p":watts}, oldest first.
    std::string history_json() const;

    /// Long-term statistics: completed days (kept kMaxDays), weeks derived from them and
    /// months derived from them, each including the running period flagged "p":1.
    /// Prices are only used for the running day; closed periods keep their stored cost.
    std::string stats_json(double price_import, double price_export, const char* currency) const;

    static constexpr int kWeekRetention  = 182;  ///< weeks older than this become month sums (~26 weeks)
    static constexpr int kMonthRetention = 730;  ///< months older than this become year sums (24 months)
    static constexpr size_t kMaxRecords = 3000;  ///< below the 3276 records of one history slot

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
    bool write_history(const std::vector<DayRec>& records, uint32_t seq) const;
    void flush_history();  ///< writes pending history outside mutex_ so readers never wait for flash erase
    void load_history();
    void compact_history();

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
    // Last seen cumulative HA counters (Wh); energy of a period = difference of the counter.
    double last_cnt_wh_[4]{0, 0, 0, 0};
    bool   have_cnt_[4]{false, false, false, false};
    // Counter values at the start of the running day (pv, charge, discharge, wallbox); today = counter - start.
    double day_start_cnt_wh_[4]{0, 0, 0, 0};
    bool   have_start_cnt_[4]{false, false, false, false};
    double wb_wh_{0};
    int64_t last_integration_us_{0};
    int64_t last_acc_persist_us_{0};
    int64_t last_acc_hour_{0};

    uint32_t date_key_{0};  ///< yyyymmdd of the running day, 0 = unknown
    bool    have_late_{false};
    int32_t late_fwd_wh_{0}, late_rev_wh_{0};
    std::vector<DayRec> days_;
    const void* part_{nullptr};  ///< "history" partition, null on devices with the old table
    uint32_t hist_seq_{0};
    bool hist_dirty_{false};
};

} // namespace app
