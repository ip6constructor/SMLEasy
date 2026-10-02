#include "energy_stats.hpp"
#include "nvs.h"
#include "esp_log.h"
#include <ctime>
#include <cstdio>

namespace app {

static const char* TAG = "EnergyStats";

EnergyStats& EnergyStats::get() {
    static EnergyStats inst;
    return inst;
}

EnergyStats::EnergyStats() {
    mutex_ = xSemaphoreCreateMutex();
    load_persisted_day();
}

void EnergyStats::load_persisted_day()
{
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READONLY, &handle) != ESP_OK) return;

    int32_t day = -1, year = -1, month = -1;
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
    nvs_close(handle);
    if (!valid) return;

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

void EnergyStats::persist_day_snapshot() const
{
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READWRITE, &handle) != ESP_OK) {
        ESP_LOGW(TAG, "cannot open NVS for daily snapshot");
        return;
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
    if (err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    if (err != ESP_OK) ESP_LOGW(TAG, "daily snapshot save failed: %s", esp_err_to_name(err));
}

void EnergyStats::check_rollover(int day_of_year, int year, int month)
{
    if (day_ == -1) {
        day_ = day_of_year; year_ = year; month_ = month;
        have_day_start_ = false;
        return;
    }
    if (year != year_ || month != month_) {
        month_import_kwh_ = 0;
        month_export_kwh_ = 0;
        month_ = month;
        year_  = year;
    }
    if (year != year_ || day_of_year != day_) {
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

    if (synced) {
        check_rollover(tmv.tm_yday, tmv.tm_year, tmv.tm_mon);
        if (!have_day_start_) {
            day_start_import_wh_ = fwd_wh;
            day_start_export_wh_ = rev_wh;
            today_import_kwh_ = 0;
            today_export_kwh_ = 0;
            have_day_start_ = true;
            persist_day_snapshot();
        } else if (fwd_wh >= day_start_import_wh_ && rev_wh >= day_start_export_wh_) {
            today_import_kwh_ = (fwd_wh - day_start_import_wh_) / 1000.0;
            today_export_kwh_ = (rev_wh - day_start_export_wh_) / 1000.0;
        } else {
            if (fwd_wh < day_start_import_wh_) {
                day_start_import_wh_ = fwd_wh;
                today_import_kwh_ = 0;
            }
            if (rev_wh < day_start_export_wh_) {
                day_start_export_wh_ = rev_wh;
                today_export_kwh_ = 0;
            }
            persist_day_snapshot();
            ESP_LOGW(TAG, "meter counter reset detected; daily baseline adjusted");
        }
    }

    if (have_prev_) {
        const int32_t d_fwd = fwd_wh - prev_fwd_wh_;
        const int32_t d_rev = rev_wh - prev_rev_wh_;
        // Ignore non-positive or implausibly large deltas (meter reset/rollover/first read jump).
        if (d_fwd > 0 && d_fwd < 1000000) {
            month_import_kwh_ += d_fwd / 1000.0;
        }
        if (d_rev > 0 && d_rev < 1000000) {
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
