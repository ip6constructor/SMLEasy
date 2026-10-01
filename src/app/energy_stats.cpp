#include "energy_stats.hpp"
#include <ctime>
#include <cstdio>

namespace app {

EnergyStats& EnergyStats::get() {
    static EnergyStats inst;
    return inst;
}

EnergyStats::EnergyStats() {
    mutex_ = xSemaphoreCreateMutex();
}

void EnergyStats::check_rollover(int day_of_year, int year, int month)
{
    if (day_ == -1) {
        day_ = day_of_year; year_ = year; month_ = month;
        return;
    }
    if (year != year_ || month != month_) {
        month_import_kwh_ = 0;
        month_export_kwh_ = 0;
        month_ = month;
        year_  = year;
    }
    if (day_of_year != day_) {
        yesterday_import_kwh_ = today_import_kwh_;
        yesterday_export_kwh_ = today_export_kwh_;
        today_import_kwh_ = 0;
        today_export_kwh_ = 0;
        day_ = day_of_year;
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

    if (synced) {
        check_rollover(tmv.tm_yday, tmv.tm_year, tmv.tm_mon);
    }

    if (have_prev_) {
        const int32_t d_fwd = fwd_wh - prev_fwd_wh_;
        const int32_t d_rev = rev_wh - prev_rev_wh_;
        // Ignore non-positive or implausibly large deltas (meter reset/rollover/first read jump).
        if (d_fwd > 0 && d_fwd < 1000000) {
            today_import_kwh_ += d_fwd / 1000.0;
            month_import_kwh_ += d_fwd / 1000.0;
        }
        if (d_rev > 0 && d_rev < 1000000) {
            today_export_kwh_ += d_rev / 1000.0;
            month_export_kwh_ += d_rev / 1000.0;
        }
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
    s.month_import_kwh     = month_import_kwh_;
    s.month_export_kwh     = month_export_kwh_;
    s.time_synced          = (day_ != -1);
    xSemaphoreGive(mutex_);
    return s;
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
        snprintf(buf, sizeof(buf), "{\"t\":%u,\"p\":%d}", s.t_epoch_s, s.power_w);
        out += buf;
    }
    xSemaphoreGive(mutex_);
    out += ']';
    return out;
}

} // namespace app
