#include "energy_stats.hpp"
#include "nvs.h"
#include "config_store.hpp"
#include "esp_idf_version.h"
#include "esp_timer.h"
#if ESP_IDF_VERSION_MAJOR >= 6
#include "ext_data.hpp"
#endif
#include "esp_log.h"
#include <inttypes.h>
#include <algorithm>
#include <ctime>
#include <cstdio>

namespace app {

static const char* TAG = "EnergyStats";
static constexpr int64_t kDailyBaselineToleranceWh = 400000;

static int64_t days_from_civil(int y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(doe) - 719468;
}

static uint32_t civil_key_from_days(int64_t z)
{
    z += 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int64_t y = static_cast<int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned d = doy - (153 * mp + 2) / 5 + 1;
    const unsigned m = mp < 10 ? mp + 3 : mp - 9;
    y += m <= 2;
    return static_cast<uint32_t>(y) * 10000 + m * 100 + d;
}

static int32_t saving_milli_of(uint32_t pv_wh, uint32_t export_wh, double pi)
{
    if (pv_wh <= export_wh) return 0;
    return static_cast<int32_t>((pv_wh - export_wh) * pi + 0.5);
}

static void add_period(PeriodRecord& a, const PeriodRecord& b)
{
    a.import_wh += b.import_wh;
    a.export_wh += b.export_wh;
    a.cost_milli += b.cost_milli;
    a.pv_wh += b.pv_wh;
    a.bat_chg_wh += b.bat_chg_wh;
    a.bat_dis_wh += b.bat_dis_wh;
    a.sav_milli += b.sav_milli;
}

/// yyyymmdd of the Monday that starts the ISO week containing the given date.
static uint32_t week_key(uint32_t date)
{
    const int64_t days = days_from_civil(static_cast<int>(date / 10000),
                                         (date / 100) % 100, date % 100);
    const int64_t weekday = ((days + 3) % 7 + 7) % 7;  // 0 = Monday
    return civil_key_from_days(days - weekday);
}

static int32_t cost_milli_of(uint32_t import_wh, uint32_t export_wh, double pi, double pe)
{
    const double c = (import_wh * pi - export_wh * pe);  // Wh * currency/kWh = milli-currency
    return static_cast<int32_t>(c >= 0 ? c + 0.5 : c - 0.5);
}

EnergyStats& EnergyStats::get() {
    static EnergyStats inst;
    return inst;
}

EnergyStats::EnergyStats() {
    mutex_ = xSemaphoreCreateMutex();
    load_persisted_day();
    load_history();
}

void EnergyStats::load_persisted_day()
{
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READONLY, &handle) != ESP_OK) return;

    int32_t day = -1, year = -1, month = -1;
    uint32_t date_key = 0;
    uint64_t yesterday_import_wh = 0, yesterday_export_wh = 0;
    int32_t day_start_import_wh = 0, day_start_export_wh = 0;
    uint8_t have_day_start = 0;
    const bool valid = nvs_get_i32(handle, "day_of_year", &day) == ESP_OK &&
        nvs_get_i32(handle, "year", &year) == ESP_OK &&
        nvs_get_i32(handle, "month", &month) == ESP_OK &&
        nvs_get_u64(handle, "y_import_wh", &yesterday_import_wh) == ESP_OK &&
        nvs_get_u64(handle, "y_export_wh", &yesterday_export_wh) == ESP_OK;
    const bool start_valid = nvs_get_i32(handle, "start_import_wh", &day_start_import_wh) == ESP_OK &&
        nvs_get_i32(handle, "start_export_wh", &day_start_export_wh) == ESP_OK &&
        nvs_get_u8(handle, "have_day_start", &have_day_start) == ESP_OK;
    nvs_get_u32(handle, "date_key", &date_key);
    uint32_t acc_date = 0, acc_pv = 0, acc_chg = 0, acc_dis = 0;
    if (nvs_get_u32(handle, "acc_date", &acc_date) == ESP_OK && acc_date == date_key && date_key != 0 &&
        nvs_get_u32(handle, "acc_pv", &acc_pv) == ESP_OK &&
        nvs_get_u32(handle, "acc_chg", &acc_chg) == ESP_OK &&
        nvs_get_u32(handle, "acc_dis", &acc_dis) == ESP_OK) {
        pv_wh_ = acc_pv;
        bat_chg_wh_ = acc_chg;
        bat_dis_wh_ = acc_dis;
    }
    nvs_close(handle);
    if (!valid) return;

    date_key_ = date_key;
    day_ = day;
    year_ = year;
    month_ = month;
    yesterday_import_kwh_ = yesterday_import_wh / 1000.0;
    yesterday_export_kwh_ = yesterday_export_wh / 1000.0;

    if (start_valid && have_day_start != 0) {
        day_start_import_wh_ = day_start_import_wh;
        day_start_export_wh_ = day_start_export_wh;
        have_day_start_ = true;
    }
}

bool EnergyStats::persist_day_snapshot() const
{
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READWRITE, &handle) != ESP_OK) {
        ESP_LOGW(TAG, "cannot open NVS for daily snapshot");
        return false;
    }

    const uint64_t import_wh = static_cast<uint64_t>(yesterday_import_kwh_ * 1000.0 + 0.5);
    const uint64_t export_wh = static_cast<uint64_t>(yesterday_export_kwh_ * 1000.0 + 0.5);
    esp_err_t err = nvs_set_i32(handle, "day_of_year", day_);
    if (err == ESP_OK) err = nvs_set_i32(handle, "year", year_);
    if (err == ESP_OK) err = nvs_set_i32(handle, "month", month_);
    if (err == ESP_OK) err = nvs_set_u64(handle, "y_import_wh", import_wh);
    if (err == ESP_OK) err = nvs_set_u64(handle, "y_export_wh", export_wh);
    if (err == ESP_OK) err = nvs_set_i32(handle, "start_import_wh", day_start_import_wh_);
    if (err == ESP_OK) err = nvs_set_i32(handle, "start_export_wh", day_start_export_wh_);
    if (err == ESP_OK) err = nvs_set_u8(handle, "have_day_start", have_day_start_ ? 1 : 0);
    if (err == ESP_OK) err = nvs_set_u32(handle, "date_key", date_key_);
    if (err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "daily snapshot save failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

void EnergyStats::load_history()
{
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READONLY, &handle) != ESP_OK) return;
    auto load = [&](const char* key, std::vector<PeriodRecord>& out, size_t max_records) {
        size_t len = 0;
        if (nvs_get_blob(handle, key, nullptr, &len) != ESP_OK || len == 0 ||
            len % sizeof(PeriodRecord) != 0 || len / sizeof(PeriodRecord) > max_records) return;
        out.resize(len / sizeof(PeriodRecord));
        if (nvs_get_blob(handle, key, out.data(), &len) != ESP_OK) out.clear();
    };
    load("hist_days", days_, kMaxDays);
    load("hist_months", months_, kMaxMonths);
    nvs_close(handle);
}

void EnergyStats::persist_accumulators() const
{
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READWRITE, &handle) != ESP_OK) return;
    esp_err_t err = nvs_set_u32(handle, "acc_date", date_key_);
    if (err == ESP_OK) err = nvs_set_u32(handle, "acc_pv", static_cast<uint32_t>(pv_wh_));
    if (err == ESP_OK) err = nvs_set_u32(handle, "acc_chg", static_cast<uint32_t>(bat_chg_wh_));
    if (err == ESP_OK) err = nvs_set_u32(handle, "acc_dis", static_cast<uint32_t>(bat_dis_wh_));
    if (err == ESP_OK) nvs_commit(handle);
    nvs_close(handle);
}

// Integrates the Home Assistant PV / battery power (W) over the time since the last call.
void EnergyStats::integrate_ha_power()
{
    const int64_t now_us = esp_timer_get_time();
    const int64_t dt_us = now_us - last_integration_us_;
    last_integration_us_ = now_us;
#if ESP_IDF_VERSION_MAJOR >= 6
    if (dt_us <= 0 || dt_us > 300LL * 1000000) return;   // first call or long gap: do not extrapolate
    const double hours = dt_us / 3.6e9;
    double w = 0;
    if (ExtData::get().role_power_w("pv", w) && w > 0) pv_wh_ += w * hours;
    if (ExtData::get().role_power_w("battery", w)) {
        if (w > 0) bat_chg_wh_ += w * hours;
        else bat_dis_wh_ += -w * hours;
    }
#else
    (void)dt_us;
#endif
}

bool EnergyStats::persist_history() const
{
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READWRITE, &handle) != ESP_OK) return false;
    esp_err_t err = ESP_OK;
    if (!days_.empty())
        err = nvs_set_blob(handle, "hist_days", days_.data(), days_.size() * sizeof(PeriodRecord));
    if (err == ESP_OK && !months_.empty())
        err = nvs_set_blob(handle, "hist_months", months_.data(), months_.size() * sizeof(PeriodRecord));
    if (err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    if (err != ESP_OK) ESP_LOGW(TAG, "history save failed: %s", esp_err_to_name(err));
    return err == ESP_OK;
}

// Closes the running day (date_key_) into the day and month history. Called under mutex_.
void EnergyStats::finalize_day()
{
    if (date_key_ == 0 || !have_day_start_) return;
    if (!days_.empty() && days_.back().key >= date_key_) return;

    int64_t imp = static_cast<int64_t>(today_import_kwh_ * 1000.0 + 0.5);
    int64_t exp = static_cast<int64_t>(today_export_kwh_ * 1000.0 + 0.5);
    const bool have_end = have_prev_ || have_late_;
    if (have_end) {
        const int64_t end_i = have_prev_ ? prev_fwd_wh_ : late_fwd_wh_;
        const int64_t end_e = have_prev_ ? prev_rev_wh_ : late_rev_wh_;
        const int64_t di = end_i - day_start_import_wh_;
        const int64_t de = end_e - day_start_export_wh_;
        if (di >= imp && di < 1000000) imp = di;
        if (de >= exp && de < 1000000) exp = de;
    }
    if (imp < 0) imp = 0;
    if (exp < 0) exp = 0;

    TariffConfig tariff;
    ConfigStore::get().load_tariff(tariff);
    const uint32_t pv = static_cast<uint32_t>(pv_wh_ + 0.5);
    PeriodRecord rec{date_key_, static_cast<uint32_t>(imp), static_cast<uint32_t>(exp),
                     cost_milli_of(static_cast<uint32_t>(imp), static_cast<uint32_t>(exp),
                                   tariff.price_import_kwh, tariff.price_export_kwh),
                     pv, static_cast<uint32_t>(bat_chg_wh_ + 0.5), static_cast<uint32_t>(bat_dis_wh_ + 0.5),
                     saving_milli_of(pv, static_cast<uint32_t>(exp), tariff.price_import_kwh)};
    pv_wh_ = bat_chg_wh_ = bat_dis_wh_ = 0;
    days_.push_back(rec);
    while (days_.size() > kMaxDays) days_.erase(days_.begin());

    const uint32_t mkey = date_key_ / 100;
    if (!months_.empty() && months_.back().key == mkey) {
        add_period(months_.back(), rec);
    } else {
        PeriodRecord month = rec;
        month.key = mkey;
        months_.push_back(month);
        while (months_.size() > kMaxMonths) months_.erase(months_.begin());
    }
    persist_history();
}

void EnergyStats::check_rollover(int day_of_year, int year, int month)
{
    if (day_ == -1) {
        day_ = day_of_year; year_ = year; month_ = month;
        have_day_start_ = false;
        return;
    }
    if (year != year_ || day_of_year != day_) {
        finalize_day();
        yesterday_import_kwh_ = today_import_kwh_;
        yesterday_export_kwh_ = today_export_kwh_;
        today_import_kwh_ = 0;
        today_export_kwh_ = 0;
        day_ = day_of_year;
        year_ = year;
        month_ = month;
        have_day_start_ = false;
    }
}

void EnergyStats::record(int32_t power_net_w, int32_t fwd_wh, int32_t rev_wh)
{
    time_t now = time(nullptr);
    // Treat anything before ~2023 as "clock not synced yet" (SNTP not done, or offline).
    const bool synced = now > 1700000000;
    struct tm tmv{};
    localtime_r(&now, &tmv);

    xSemaphoreTake(mutex_, portMAX_DELAY);

    if (synced) integrate_ha_power();
    if (synced) {
        late_fwd_wh_ = fwd_wh;
        late_rev_wh_ = rev_wh;
        have_late_ = true;
        check_rollover(tmv.tm_yday, tmv.tm_year, tmv.tm_mon);
        date_key_ = static_cast<uint32_t>(tmv.tm_year + 1900) * 10000 +
                    static_cast<uint32_t>(tmv.tm_mon + 1) * 100 + static_cast<uint32_t>(tmv.tm_mday);
        if (!have_day_start_) {
            day_start_import_wh_ = fwd_wh;
            day_start_export_wh_ = rev_wh;
            today_import_kwh_ = 0;
            today_export_kwh_ = 0;
            have_day_start_ = true;
            persist_day_snapshot();
        } else {
            const int64_t import_delta = static_cast<int64_t>(fwd_wh) - day_start_import_wh_;
            const int64_t export_delta = static_cast<int64_t>(rev_wh) - day_start_export_wh_;
            bool baseline_adjusted = false;
            if (import_delta >= 0) {
                today_import_kwh_ = import_delta / 1000.0;
            } else if (import_delta >= -kDailyBaselineToleranceWh) {
                today_import_kwh_ = 0;
            } else {
                day_start_import_wh_ = fwd_wh;
                today_import_kwh_ = 0;
                baseline_adjusted = true;
            }
            if (export_delta >= 0) {
                today_export_kwh_ = export_delta / 1000.0;
            } else if (export_delta >= -kDailyBaselineToleranceWh) {
                today_export_kwh_ = 0;
            } else {
                day_start_export_wh_ = rev_wh;
                today_export_kwh_ = 0;
                baseline_adjusted = true;
            }
            if (baseline_adjusted) {
                persist_day_snapshot();
                ESP_LOGW(TAG, "meter counter reset detected; daily baseline adjusted");
            }
        }
    }

    if (synced && esp_timer_get_time() - last_acc_persist_us_ > 3600LL * 1000000) {
        last_acc_persist_us_ = esp_timer_get_time();
        persist_accumulators();
    }
    prev_fwd_wh_ = fwd_wh;
    prev_rev_wh_ = rev_wh;
    have_prev_   = true;

    hist_.push_back(PowerSample{ synced ? static_cast<uint32_t>(now) : 0u, power_net_w });
    while (hist_.size() > kMaxSamples) hist_.pop_front();

    xSemaphoreGive(mutex_);
}

StatsSnapshot EnergyStats::snapshot() const
{
    StatsSnapshot s;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    s.today_import_kwh     = today_import_kwh_;
    s.today_export_kwh     = today_export_kwh_;
    s.yesterday_import_kwh = yesterday_import_kwh_;
    s.yesterday_export_kwh = yesterday_export_kwh_;
    uint64_t m_imp = 0, m_exp = 0, w_imp = 0, w_exp = 0;
    int64_t m_cost = 0, w_cost = 0;
    if (date_key_ != 0) {
        for (const auto& m : months_) {
            if (m.key == date_key_ / 100) { m_imp = m.import_wh; m_exp = m.export_wh; m_cost = m.cost_milli; }
        }
        const uint32_t wk = week_key(date_key_);
        for (const auto& d : days_) {
            if (week_key(d.key) == wk) { w_imp += d.import_wh; w_exp += d.export_wh; w_cost += d.cost_milli; }
        }
    }
    s.month_import_kwh     = m_imp / 1000.0 + today_import_kwh_;
    s.month_export_kwh     = m_exp / 1000.0 + today_export_kwh_;
    s.week_import_kwh      = w_imp / 1000.0 + today_import_kwh_;
    s.week_export_kwh      = w_exp / 1000.0 + today_export_kwh_;
    const double t_pv = pv_wh_, t_bc = bat_chg_wh_, t_bd = bat_dis_wh_;
    auto house_wh = [](double i, double e, double pv, double bc, double bd) { return std::max(0.0, i - e + pv - bc + bd); };
    double w_house = 0, m_house = 0; bool known = (t_pv + t_bc + t_bd) > 0;
    if (date_key_ != 0) {
        const uint32_t wk2 = week_key(date_key_);
        for (const auto& d : days_) {
            if (week_key(d.key) == wk2) w_house += house_wh(d.import_wh, d.export_wh, d.pv_wh, d.bat_chg_wh, d.bat_dis_wh);
            if (d.pv_wh || d.bat_chg_wh || d.bat_dis_wh) known = true;
        }
        for (const auto& m : months_) {
            if (m.key == date_key_ / 100) m_house = house_wh(m.import_wh, m.export_wh, m.pv_wh, m.bat_chg_wh, m.bat_dis_wh);
        }
    }
    const double today_house = house_wh(today_import_kwh_ * 1000.0, today_export_kwh_ * 1000.0, t_pv, t_bc, t_bd);
    s.today_house_kwh = today_house / 1000.0;
    s.week_house_kwh  = (w_house + today_house) / 1000.0;
    s.month_house_kwh = (m_house + today_house) / 1000.0;
    s.have_house      = known;
    s.week_cost_final      = w_cost / 1000.0;
    s.month_cost_final     = m_cost / 1000.0;
    s.time_synced          = (day_ != -1);
    xSemaphoreGive(mutex_);
    return s;
}

DailyBaselineSnapshot EnergyStats::daily_baseline() const
{
    DailyBaselineSnapshot baseline;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    baseline.import_wh = day_start_import_wh_;
    baseline.export_wh = day_start_export_wh_;
    baseline.available = have_day_start_;
    xSemaphoreGive(mutex_);
    return baseline;
}

bool EnergyStats::set_daily_baseline(int32_t import_wh, int32_t export_wh)
{
    if (import_wh <= 0 || export_wh <= 0) return false;
    const time_t now = time(nullptr);
    if (now <= 1700000000) return false;
    struct tm tmv{};
    localtime_r(&now, &tmv);

    xSemaphoreTake(mutex_, portMAX_DELAY);
    check_rollover(tmv.tm_yday, tmv.tm_year, tmv.tm_mon);
    const int64_t import_delta = static_cast<int64_t>(prev_fwd_wh_) - import_wh;
    const int64_t export_delta = static_cast<int64_t>(prev_rev_wh_) - export_wh;
    if (!have_prev_ || import_delta < -kDailyBaselineToleranceWh ||
        import_delta > kDailyBaselineToleranceWh || export_delta < -kDailyBaselineToleranceWh ||
        export_delta > kDailyBaselineToleranceWh) {
        xSemaphoreGive(mutex_);
        return false;
    }

    const int32_t old_import_wh = day_start_import_wh_;
    const int32_t old_export_wh = day_start_export_wh_;
    const double old_today_import_kwh = today_import_kwh_;
    const double old_today_export_kwh = today_export_kwh_;
    const bool old_have_day_start = have_day_start_;

    day_start_import_wh_ = import_wh;
    day_start_export_wh_ = export_wh;
    today_import_kwh_ = import_delta > 0 ? import_delta / 1000.0 : 0;
    today_export_kwh_ = export_delta > 0 ? export_delta / 1000.0 : 0;
    have_day_start_ = true;
    if (!persist_day_snapshot()) {
        day_start_import_wh_ = old_import_wh;
        day_start_export_wh_ = old_export_wh;
        today_import_kwh_ = old_today_import_kwh;
        today_export_kwh_ = old_today_export_kwh;
        have_day_start_ = old_have_day_start;
        xSemaphoreGive(mutex_);
        return false;
    }
    xSemaphoreGive(mutex_);
    return true;
}

std::string EnergyStats::history_json() const
{
    std::string out;
    out += '[';
    char buf[40];
    xSemaphoreTake(mutex_, portMAX_DELAY);
    bool first = true;
    for (const auto& s : hist_) {
        if (!first) out += ',';
        first = false;
        snprintf(buf, sizeof(buf), "{\"t\":%" PRIu32 ",\"p\":%" PRId32 "}",
                 s.t_epoch_s, s.power_w);
        out += buf;
    }
    xSemaphoreGive(mutex_);
    out += ']';
    return out;
}

std::string EnergyStats::stats_json(double price_import, double price_export, const char* currency) const
{
    std::vector<PeriodRecord> days, months;
    PeriodRecord today{0, 0, 0, 0, 0, 0, 0, 0};
    xSemaphoreTake(mutex_, portMAX_DELAY);
    days = days_;
    months = months_;
    if (date_key_ != 0 && have_day_start_) {
        const uint32_t ti = static_cast<uint32_t>(today_import_kwh_ * 1000.0 + 0.5);
        const uint32_t te = static_cast<uint32_t>(today_export_kwh_ * 1000.0 + 0.5);
        const uint32_t pv = static_cast<uint32_t>(pv_wh_ + 0.5);
        today = PeriodRecord{date_key_, ti, te, cost_milli_of(ti, te, price_import, price_export),
                             pv, static_cast<uint32_t>(bat_chg_wh_ + 0.5), static_cast<uint32_t>(bat_dis_wh_ + 0.5),
                             saving_milli_of(pv, te, price_import)};
    }
    xSemaphoreGive(mutex_);

    std::vector<PeriodRecord> weeks;
    std::vector<PeriodRecord> all = days;
    if (today.key != 0) all.push_back(today);
    for (const auto& d : all) {
        const uint32_t wk = week_key(d.key);
        if (weeks.empty() || weeks.back().key != wk) weeks.push_back(PeriodRecord{wk, 0, 0, 0, 0, 0, 0, 0});
        add_period(weeks.back(), d);
    }
    if (today.key != 0) {
        const uint32_t mk = today.key / 100;
        if (!months.empty() && months.back().key == mk) {
            add_period(months.back(), today);
        } else {
            PeriodRecord month = today;
            month.key = mk;
            months.push_back(month);
        }
    }

    std::string out = "{\"currency\":\"";
    out += currency;
    out += "\",\"today\":";
    out += std::to_string(today.key);
    char buf[224];
    auto emit = [&](const char* name, const std::vector<PeriodRecord>& v, uint32_t running_key) {
        out += ",\"";
        out += name;
        out += "\":[";
        for (size_t i = 0; i < v.size(); ++i) {
            snprintf(buf, sizeof(buf), "%s{\"k\":%" PRIu32 ",\"i\":%" PRIu32 ",\"e\":%" PRIu32 ",\"c\":%" PRId32
                     ",\"pv\":%" PRIu32 ",\"bc\":%" PRIu32 ",\"bd\":%" PRIu32 ",\"s\":%" PRId32 "%s}",
                     i ? "," : "", v[i].key, v[i].import_wh, v[i].export_wh, v[i].cost_milli,
                     v[i].pv_wh, v[i].bat_chg_wh, v[i].bat_dis_wh, v[i].sav_milli,
                     v[i].key == running_key ? ",\"p\":1" : "");
            out += buf;
        }
        out += ']';
    };
    emit("days", all, today.key);
    emit("weeks", weeks, today.key != 0 ? week_key(today.key) : 0);
    emit("months", months, today.key / 100);
    out += '}';
    return out;
}

} // namespace app
