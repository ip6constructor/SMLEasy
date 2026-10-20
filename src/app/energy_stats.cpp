#include "energy_stats.hpp"
#include "nvs.h"
#include "config_store.hpp"
#include "esp_idf_version.h"
#include "esp_timer.h"
#if ESP_IDF_VERSION_MAJOR >= 6
#include "ext_data.hpp"
#endif
#include "esp_log.h"
#include "esp_partition.h"
#include <cstring>
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
    a.wb_wh += b.wb_wh;
    a.wb_milli += b.wb_milli;
}

static int32_t wb_milli_of(uint32_t wb_wh, double price_import, double refund)
{
    const double v = wb_wh * (refund - price_import);
    return static_cast<int32_t>(v >= 0 ? v + 0.5 : v - 0.5);
}

// Layout before the wallbox fields were added; stored history is converted on load.
struct LegacyPeriodRecord {
    uint32_t key, import_wh, export_wh;
    int32_t  cost_milli;
    uint32_t pv_wh, bat_chg_wh, bat_dis_wh;
    int32_t  sav_milli;
};

static int64_t epoch2020() { return days_from_civil(2020, 1, 1); }

static uint16_t day_index(uint32_t key)
{
    const int64_t d = days_from_civil(static_cast<int>(key / 10000), (key / 100) % 100, key % 100) - epoch2020();
    return static_cast<uint16_t>(d < 0 ? 0 : (d > 16383 ? 16383 : d));
}

static const int kScale[4] = {1, 10, 100, 1000};
static unsigned rec_kind(const DayRec& r) { return r.day >> 14; }
static int rec_idx(const DayRec& r) { return r.day & 0x3FFF; }

static uint16_t enc16(uint32_t v, int scale)
{
    const uint32_t x = (v + scale / 2) / scale;
    return x > 65535 ? 65535 : static_cast<uint16_t>(x);
}
static int16_t encs16(int32_t milli, int scale)
{
    const int32_t unit = 10 * scale;  // milli -> 0.01 * scale
    const int32_t c = milli >= 0 ? (milli + unit / 2) / unit : -((-milli + unit / 2) / unit);
    return static_cast<int16_t>(c > 32767 ? 32767 : (c < -32768 ? -32768 : c));
}

static DayRec to_rec(const PeriodRecord& p, unsigned kind = 0)
{
    const int s = kScale[kind];
    return DayRec{static_cast<uint16_t>(day_index(p.key) | (kind << 14)), enc16(p.import_wh, s), enc16(p.export_wh, s),
                  enc16(p.pv_wh, s), enc16(p.bat_chg_wh, s), enc16(p.bat_dis_wh, s), enc16(p.wb_wh, s),
                  encs16(p.cost_milli, s), encs16(p.sav_milli, s), encs16(p.wb_milli, s)};
}

static PeriodRecord to_period(const DayRec& r)
{
    const int s = kScale[rec_kind(r)];
    return PeriodRecord{civil_key_from_days(static_cast<int64_t>(rec_idx(r)) + epoch2020()),
                        static_cast<uint32_t>(r.imp) * s, static_cast<uint32_t>(r.exp) * s,
                        r.cost_c * 10 * s, static_cast<uint32_t>(r.pv) * s, static_cast<uint32_t>(r.chg) * s,
                        static_cast<uint32_t>(r.dis) * s, r.sav_c * 10 * s, static_cast<uint32_t>(r.wb) * s,
                        r.wbres_c * 10 * s};
}

static uint32_t crc32_of(const uint8_t* d, size_t n)
{
    uint32_t c = 0xFFFFFFFFu;
    while (n--) {
        c ^= *d++;
        for (int i = 0; i < 8; ++i) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return ~c;
}

// History partition: two 64 KB slots written alternately; header is written last (commit).
static constexpr uint32_t kSlotSize = 0x10000;
static constexpr uint32_t kHistMagic = 0x53484953;
struct HistHeader { uint32_t magic, seq, count, crc; };
static constexpr size_t kNvsFallbackDays = 250;

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
        uint32_t acc_wb = 0;
        if (nvs_get_u32(handle, "acc_wb", &acc_wb) == ESP_OK) wb_wh_ = acc_wb;
        bat_chg_wh_ = acc_chg;
        bat_dis_wh_ = acc_dis;
    }
    static const char* const kCntKeys[4] = {"cnt_pv", "cnt_chg", "cnt_dis", "cnt_wb"};
    static const char* const kStartKeys[4] = {"st_pv", "st_chg", "st_dis", "st_wb"};
    for (int i = 0; i < 4; ++i) {
        uint32_t wh = 0;
        if (nvs_get_u32(handle, kCntKeys[i], &wh) == ESP_OK) { last_cnt_wh_[i] = wh; have_cnt_[i] = true; }
        int64_t st = 0;
        if (acc_date == date_key && date_key != 0 && nvs_get_i64(handle, kStartKeys[i], &st) == ESP_OK) {
            day_start_cnt_wh_[i] = static_cast<double>(st);
            have_start_cnt_[i] = true;
        }
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
    days_.clear();
    const esp_partition_t* p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "history");
    part_ = p;
    if (p) {
        HistHeader hdr[2]{};
        for (int s = 0; s < 2; ++s) {
            if (esp_partition_read(p, s * kSlotSize, &hdr[s], sizeof(HistHeader)) != ESP_OK ||
                hdr[s].magic != kHistMagic || hdr[s].count > (kSlotSize - sizeof(HistHeader)) / sizeof(DayRec))
                hdr[s].magic = 0;
        }
        int order[2] = {0, 1};
        if (hdr[1].magic == kHistMagic && (hdr[0].magic != kHistMagic || hdr[1].seq > hdr[0].seq)) { order[0] = 1; order[1] = 0; }
        for (int k = 0; k < 2 && days_.empty() && hist_seq_ == 0; ++k) {
            const int s = order[k];
            if (hdr[s].magic != kHistMagic) continue;
            std::vector<DayRec> v(hdr[s].count);
            if (hdr[s].count && esp_partition_read(p, s * kSlotSize + sizeof(HistHeader), v.data(), v.size() * sizeof(DayRec)) != ESP_OK) continue;
            if (crc32_of(reinterpret_cast<const uint8_t*>(v.data()), v.size() * sizeof(DayRec)) != hdr[s].crc) continue;
            days_.swap(v);
            hist_seq_ = hdr[s].seq;
        }
    }

    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READONLY, &handle) == ESP_OK) {
        size_t len = 0;
        if (days_.empty() && nvs_get_blob(handle, "hist_c", nullptr, &len) == ESP_OK && len && len % sizeof(DayRec) == 0) {
            days_.resize(len / sizeof(DayRec));
            if (nvs_get_blob(handle, "hist_c", days_.data(), &len) != ESP_OK) days_.clear();
        }
        // Migration of the earlier 60-day records.
        len = 0;
        if (days_.empty() && nvs_get_blob(handle, "hist_days", nullptr, &len) == ESP_OK && len) {
            std::vector<uint8_t> raw(len);
            if (nvs_get_blob(handle, "hist_days", raw.data(), &len) == ESP_OK) {
                if (len % sizeof(PeriodRecord) == 0) {
                    for (size_t o = 0; o < len; o += sizeof(PeriodRecord)) {
                        PeriodRecord r; memcpy(&r, raw.data() + o, sizeof(r)); days_.push_back(to_rec(r));
                    }
                } else if (len % sizeof(LegacyPeriodRecord) == 0) {
                    for (size_t o = 0; o < len; o += sizeof(LegacyPeriodRecord)) {
                        LegacyPeriodRecord l; memcpy(&l, raw.data() + o, sizeof(l));
                        days_.push_back(to_rec(PeriodRecord{l.key, l.import_wh, l.export_wh, l.cost_milli, l.pv_wh,
                                                            l.bat_chg_wh, l.bat_dis_wh, l.sav_milli, 0, 0}));
                    }
                }
            }
        }
        nvs_close(handle);
    }
    while (days_.size() > kMaxRecords) days_.erase(days_.begin());
    // Migration to the tiered layout (days / weeks / months / years): fold old records once and persist.
    const size_t before = days_.size();
    const uint32_t crc_before = crc32_of(reinterpret_cast<const uint8_t*>(days_.data()), before * sizeof(DayRec));
    compact_history();
    if (days_.size() != before ||
        crc32_of(reinterpret_cast<const uint8_t*>(days_.data()), days_.size() * sizeof(DayRec)) != crc_before) {
        hist_dirty_ = true;
        ESP_LOGI(TAG, "history migrated: %u -> %u records", static_cast<unsigned>(before), static_cast<unsigned>(days_.size()));
    }
    ESP_LOGI(TAG, "history: %u days (%s)", static_cast<unsigned>(days_.size()), part_ ? "partition" : "nvs fallback");
}

void EnergyStats::persist_accumulators() const
{
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READWRITE, &handle) != ESP_OK) return;
    esp_err_t err = nvs_set_u32(handle, "acc_date", date_key_);
    if (err == ESP_OK) err = nvs_set_u32(handle, "acc_pv", static_cast<uint32_t>(pv_wh_));
    if (err == ESP_OK) err = nvs_set_u32(handle, "acc_chg", static_cast<uint32_t>(bat_chg_wh_));
    if (err == ESP_OK) err = nvs_set_u32(handle, "acc_dis", static_cast<uint32_t>(bat_dis_wh_));
    if (err == ESP_OK) err = nvs_set_u32(handle, "acc_wb", static_cast<uint32_t>(wb_wh_));
    static const char* const kCntKeys[4] = {"cnt_pv", "cnt_chg", "cnt_dis", "cnt_wb"};
    static const char* const kStartKeys[4] = {"st_pv", "st_chg", "st_dis", "st_wb"};
    for (int i = 0; i < 4 && err == ESP_OK; ++i) {
        if (have_cnt_[i]) err = nvs_set_u32(handle, kCntKeys[i], static_cast<uint32_t>(last_cnt_wh_[i]));
        if (err == ESP_OK && have_start_cnt_[i])
            err = nvs_set_i64(handle, kStartKeys[i], static_cast<int64_t>(day_start_cnt_wh_[i]));
    }
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
    // Preferred: cumulative kWh counters from Home Assistant, counted as differences.
    static const char* const kRoles[4] = {"pv_kwh", "bat_chg_kwh", "bat_dis_kwh", "wb_kwh"};
    double* const targets[4] = {&pv_wh_, &bat_chg_wh_, &bat_dis_wh_, &wb_wh_};
    bool counter_used[4] = {false, false, false, false};
    for (int i = 0; i < 4; ++i) {
        double wh = 0;
        if (!ExtData::get().role_energy_wh(kRoles[i], wh)) continue;
        counter_used[i] = true;
        if (wh <= 0) continue;
        // A lower value is first treated as a glitch; if it persists for 2 minutes the counter was reset
        // (e.g. a daily energy sensor at midnight): the start is moved so that today's value is kept.
        static int64_t lower_since_us[4] = {0, 0, 0, 0};
        if (have_cnt_[i] && wh < last_cnt_wh_[i]) {
            if (lower_since_us[i] == 0) lower_since_us[i] = now_us;
            if (now_us - lower_since_us[i] < 120LL * 1000000) continue;
            day_start_cnt_wh_[i] = wh - *targets[i];
            have_start_cnt_[i] = true;
            last_cnt_wh_[i] = wh;
            lower_since_us[i] = 0;
            continue;
        }
        lower_since_us[i] = 0;
        if (!have_start_cnt_[i]) {
            // First reading of the day: bridge the gap since the last persisted counter.
            day_start_cnt_wh_[i] = (have_cnt_[i] ? last_cnt_wh_[i] : wh) - *targets[i];
            have_start_cnt_[i] = true;
        }
        // Implausible jump (> 1000 kWh, e.g. sensor exchange) is not counted as energy.
        if (have_cnt_[i] && wh - last_cnt_wh_[i] >= 1.0e6) day_start_cnt_wh_[i] += wh - last_cnt_wh_[i];
        *targets[i] = std::max(0.0, wh - day_start_cnt_wh_[i]);
        last_cnt_wh_[i] = wh;
        have_cnt_[i] = true;
    }
    // Fallback: integrate instantaneous power (W) when no counter is configured.
    if (dt_us <= 0 || dt_us > 300LL * 1000000) return;   // first call or long gap: do not extrapolate
    const double hours = dt_us / 3.6e9;
    double w = 0;
    if (!counter_used[0] && ExtData::get().role_power_w("pv", w) && w > 0) pv_wh_ += w * hours;
    if (!counter_used[1] && !counter_used[2] && ExtData::get().role_power_w("battery", w)) {
        if (w > 0) bat_chg_wh_ += w * hours;
        else bat_dis_wh_ += -w * hours;
    }
#else
    (void)dt_us;
#endif
}

bool EnergyStats::write_history(const std::vector<DayRec>& records, uint32_t seq) const
{
    if (const esp_partition_t* p = static_cast<const esp_partition_t*>(part_)) {
        const uint32_t off = (seq % 2) * kSlotSize;
        const size_t bytes = records.size() * sizeof(DayRec);
        HistHeader h{kHistMagic, seq, static_cast<uint32_t>(records.size()),
                     crc32_of(reinterpret_cast<const uint8_t*>(records.data()), bytes)};
        esp_err_t err = esp_partition_erase_range(p, off, kSlotSize);
        if (err == ESP_OK && bytes) err = esp_partition_write(p, off + sizeof(HistHeader), records.data(), bytes);
        if (err == ESP_OK) err = esp_partition_write(p, off, &h, sizeof(h));
        if (err != ESP_OK) { ESP_LOGW(TAG, "history save failed: %s", esp_err_to_name(err)); return false; }
        return true;
    }
    nvs_handle_t handle;
    if (nvs_open("energy_stats", NVS_READWRITE, &handle) != ESP_OK) return false;
    esp_err_t err = ESP_OK;
    if (!records.empty()) {
        const size_t n = std::min(records.size(), kNvsFallbackDays);
        err = nvs_set_blob(handle, "hist_c", records.data() + (records.size() - n), n * sizeof(DayRec));
    }
    if (err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    if (err != ESP_OK) ESP_LOGW(TAG, "history save failed: %s", esp_err_to_name(err));
    return err == ESP_OK;
}

void EnergyStats::flush_history()
{
    xSemaphoreTake(mutex_, portMAX_DELAY);
    if (!hist_dirty_) {
        xSemaphoreGive(mutex_);
        return;
    }
    hist_dirty_ = false;
    const std::vector<DayRec> copy = days_;
    const uint32_t seq = hist_seq_ + 1;
    xSemaphoreGive(mutex_);

    const bool ok = write_history(copy, seq);

    xSemaphoreTake(mutex_, portMAX_DELAY);
    if (ok) hist_seq_ = seq;
    else hist_dirty_ = true;  // retry with the next reading
    xSemaphoreGive(mutex_);
}

// Day records older than the current month's length become week sums, week sums older than 6 years month sums.
void EnergyStats::compact_history()
{
    if (days_.empty()) return;
    const int today = rec_idx(days_.back());
    auto first_of = [&](unsigned kind) {
        size_t i = 0;
        while (i < days_.size() && rec_kind(days_[i]) > kind) ++i;
        return i;
    };
    auto fold = [&](unsigned from, unsigned to, int limit) {
        for (;;) {
            const size_t pos = first_of(from);
            if (pos >= days_.size() || rec_kind(days_[pos]) != from || rec_idx(days_[pos]) >= today - limit) break;
            PeriodRecord p = to_period(days_[pos]);
            if (to == 1) p.key = week_key(p.key);
            else if (to == 2) p.key = p.key / 100 * 100 + 1;
            else p.key = p.key / 10000 * 10000 + 101;
            if (pos > 0 && rec_kind(days_[pos - 1]) == to) {
                PeriodRecord prev = to_period(days_[pos - 1]);
                if (prev.key == p.key) {
                    add_period(prev, p);
                    days_[pos - 1] = to_rec(prev, to);
                    days_.erase(days_.begin() + pos);
                    continue;
                }
            }
            days_[pos] = to_rec(p, to);
        }
    };
    // Days are kept for as many days as the current month has (28-31), older ones become week sums.
    const int key = to_period(days_.back()).key;
    const int y = key / 10000, m = key / 100 % 100;
    static const int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int dim = (m >= 1 && m <= 12) ? mdays[m - 1] : 31;
    if (m == 2 && (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))) dim = 29;
    fold(0, 1, dim);
    fold(1, 2, kWeekRetention);
    fold(2, 3, kMonthRetention);
}

// Closes the running day (date_key_) into the day and month history. Called under mutex_.
void EnergyStats::finalize_day()
{
    if (date_key_ == 0 || !have_day_start_) return;
    if (!days_.empty() && rec_idx(days_.back()) >= day_index(date_key_)) return;

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
                     saving_milli_of(pv, static_cast<uint32_t>(exp), tariff.price_import_kwh),
                     static_cast<uint32_t>(wb_wh_ + 0.5),
                     wb_milli_of(static_cast<uint32_t>(wb_wh_ + 0.5), tariff.price_import_kwh, tariff.wallbox_refund_kwh)};
    pv_wh_ = bat_chg_wh_ = bat_dis_wh_ = wb_wh_ = 0;
    for (int i = 0; i < 4; ++i) {
        have_start_cnt_[i] = have_cnt_[i];
        day_start_cnt_wh_[i] = last_cnt_wh_[i];
    }
    days_.push_back(to_rec(rec));
    compact_history();
    while (days_.size() > kMaxRecords) days_.erase(days_.begin());
    hist_dirty_ = true;
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
            persist_accumulators();
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

    if (synced && static_cast<int64_t>(now) / 3600 != last_acc_hour_) {
        last_acc_hour_ = static_cast<int64_t>(now) / 3600;
        persist_accumulators();
    }
    prev_fwd_wh_ = fwd_wh;
    prev_rev_wh_ = rev_wh;
    have_prev_   = true;

    hist_.push_back(PowerSample{ synced ? static_cast<uint32_t>(now) : 0u, power_net_w });
    while (hist_.size() > kMaxSamples) hist_.pop_front();

    xSemaphoreGive(mutex_);
    flush_history();
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
    double w_house = 0, m_house = 0;
    double w_pv = 0, w_chg = 0, w_dis = 0, w_wb = 0, m_pv = 0, m_chg = 0, m_dis = 0, m_wb = 0;
    int64_t w_wm = 0, m_wm = 0, w_sav = 0, m_sav = 0;
    const double t_pv = pv_wh_, t_bc = bat_chg_wh_, t_bd = bat_dis_wh_;
    auto house_wh = [](double i, double e, double pv, double bc, double bd) { return std::max(0.0, i - e + pv - bc + bd); };
    bool known = (t_pv + t_bc + t_bd) > 0;
    if (date_key_ != 0) {
        const uint32_t wk = week_key(date_key_), mk = date_key_ / 100;
        const int cutoff = static_cast<int>(day_index(date_key_)) - 40;
        for (auto it = days_.rbegin(); it != days_.rend() && rec_idx(*it) >= cutoff && rec_kind(*it) == 0; ++it) {
            const PeriodRecord d = to_period(*it);
            const double h = house_wh(d.import_wh, d.export_wh, d.pv_wh, d.bat_chg_wh, d.bat_dis_wh);
            if (d.key / 100 == mk) { m_imp += d.import_wh; m_exp += d.export_wh; m_cost += d.cost_milli; m_house += h;
                m_pv += d.pv_wh; m_chg += d.bat_chg_wh; m_dis += d.bat_dis_wh; m_wb += d.wb_wh; m_wm += d.wb_milli; m_sav += d.sav_milli; }
            if (week_key(d.key) == wk) { w_imp += d.import_wh; w_exp += d.export_wh; w_cost += d.cost_milli; w_house += h;
                w_pv += d.pv_wh; w_chg += d.bat_chg_wh; w_dis += d.bat_dis_wh; w_wb += d.wb_wh; w_wm += d.wb_milli; w_sav += d.sav_milli; }
            if (d.pv_wh || d.bat_chg_wh || d.bat_dis_wh) known = true;
        }
    }
    s.month_import_kwh     = m_imp / 1000.0 + today_import_kwh_;
    s.month_export_kwh     = m_exp / 1000.0 + today_export_kwh_;
    s.week_import_kwh      = w_imp / 1000.0 + today_import_kwh_;
    s.week_export_kwh      = w_exp / 1000.0 + today_export_kwh_;
    const double today_house = house_wh(today_import_kwh_ * 1000.0, today_export_kwh_ * 1000.0, t_pv, t_bc, t_bd);
    s.today_house_kwh = today_house / 1000.0;
    s.week_house_kwh  = (w_house + today_house) / 1000.0;
    s.month_house_kwh = (m_house + today_house) / 1000.0;
    {
        TariffConfig tc;
        ConfigStore::get().load_tariff(tc);
        const uint32_t exp_wh = static_cast<uint32_t>(today_export_kwh_ * 1000.0 + 0.5);
        const double t_sav = saving_milli_of(static_cast<uint32_t>(t_pv + 0.5), exp_wh, tc.price_import_kwh);
        const double t_wm = wb_milli_of(static_cast<uint32_t>(wb_wh_ + 0.5), tc.price_import_kwh, tc.wallbox_refund_kwh);
        const double tp[4] = {t_pv, t_bc, t_bd, wb_wh_};
        const double wk[4] = {w_pv, w_chg, w_dis, w_wb};
        const double mo[4] = {m_pv, m_chg, m_dis, m_wb};
        double* const out[4] = {s.pv_kwh, s.chg_kwh, s.dis_kwh, s.wb_kwh};
        for (int i = 0; i < 4; ++i) {
            out[i][0] = tp[i] / 1000.0;
            out[i][1] = (wk[i] + tp[i]) / 1000.0;
            out[i][2] = (mo[i] + tp[i]) / 1000.0;
        }
        s.wb_result[0] = t_wm / 1000.0; s.wb_result[1] = (w_wm + t_wm) / 1000.0; s.wb_result[2] = (m_wm + t_wm) / 1000.0;
        s.saving[0] = t_sav / 1000.0;   s.saving[1] = (w_sav + t_sav) / 1000.0;   s.saving[2] = (m_sav + t_sav) / 1000.0;
    }
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

bool EnergyStats::set_pv_wh(uint32_t key, uint32_t pv_wh, double price_import)
{
    if (pv_wh > 65535) return false;
    bool ok = false;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    if (key == date_key_) {
        pv_wh_ = pv_wh;
        if (have_cnt_[0]) {
            day_start_cnt_wh_[0] = last_cnt_wh_[0] - pv_wh;
            have_start_cnt_[0] = true;
        }
        persist_accumulators();
        ok = persist_day_snapshot();
    } else {
        const int idx = day_index(key);
        for (auto& r : days_) {
            if (rec_kind(r) != 0 || rec_idx(r) != idx) continue;
            r.pv = static_cast<uint16_t>(pv_wh);
            r.sav_c = encs16(saving_milli_of(pv_wh, r.exp, price_import), 1);
            hist_dirty_ = true;
            ok = true;
            break;
        }
    }
    xSemaphoreGive(mutex_);
    return ok;
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
    static constexpr size_t kJsonDays = 62, kJsonWeeks = 53, kJsonMonths = 72, kJsonYears = 12;
    TariffConfig tariff_cfg;
    ConfigStore::get().load_tariff(tariff_cfg);
    std::vector<PeriodRecord> days, weeks, months, years;
    PeriodRecord today{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    auto feed = [&](const PeriodRecord& d, unsigned kind) {
        const uint32_t wk = week_key(d.key), mk = d.key / 100;
        if (kind < 2) {
            if (weeks.empty() || weeks.back().key != wk) weeks.push_back(PeriodRecord{wk, 0, 0, 0, 0, 0, 0, 0, 0, 0});
            add_period(weeks.back(), d);
        }
        if (kind < 3) {
            if (months.empty() || months.back().key != mk) months.push_back(PeriodRecord{mk, 0, 0, 0, 0, 0, 0, 0, 0, 0});
            add_period(months.back(), d);
        }
        const uint32_t yk = d.key / 10000;
        if (years.empty() || years.back().key != yk) years.push_back(PeriodRecord{yk, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        add_period(years.back(), d);
    };
    xSemaphoreTake(mutex_, portMAX_DELAY);
    const size_t n = days_.size();
    const bool has_today = date_key_ != 0 && have_day_start_;
    size_t n_days = 0;
    for (const auto& r : days_) if (rec_kind(r) == 0) ++n_days;
    const size_t keep = kJsonDays - (has_today ? 1 : 0);
    size_t seen = 0;
    for (size_t i = 0; i < n; ++i) {
        const unsigned kind = rec_kind(days_[i]);
        const PeriodRecord d = to_period(days_[i]);
        feed(d, kind);
        if (kind == 0 && seen++ + keep >= n_days) days.push_back(d);
    }
    if (date_key_ != 0 && have_day_start_) {
        const uint32_t ti = static_cast<uint32_t>(today_import_kwh_ * 1000.0 + 0.5);
        const uint32_t te = static_cast<uint32_t>(today_export_kwh_ * 1000.0 + 0.5);
        const uint32_t pv = static_cast<uint32_t>(pv_wh_ + 0.5);
        const uint32_t wb = static_cast<uint32_t>(wb_wh_ + 0.5);
        today = PeriodRecord{date_key_, ti, te, cost_milli_of(ti, te, price_import, price_export),
                             pv, static_cast<uint32_t>(bat_chg_wh_ + 0.5), static_cast<uint32_t>(bat_dis_wh_ + 0.5),
                             saving_milli_of(pv, te, price_import), wb,
                             wb_milli_of(wb, price_import, tariff_cfg.wallbox_refund_kwh)};
        feed(today, 0);
        days.push_back(today);
    }
    xSemaphoreGive(mutex_);
    if (weeks.size() > kJsonWeeks) weeks.erase(weeks.begin(), weeks.end() - kJsonWeeks);
    if (months.size() > kJsonMonths) months.erase(months.begin(), months.end() - kJsonMonths);
    if (years.size() > kJsonYears) years.erase(years.begin(), years.end() - kJsonYears);

    std::string out;
    out.reserve((days.size() + weeks.size() + months.size()) * 120 + 128);
    out = "{\"currency\":\"";
    out += currency;
    out += "\",\"today\":";
    out += std::to_string(today.key);
    char buf[288];
    // "q" is a stable sequence number per level (day/week/month/year since 2020) that clients can cache by.
    auto seq_of = [](unsigned kind, uint32_t key) -> int32_t {
        switch (kind) {
            case 0: return day_index(key);
            case 1: return (day_index(key) + 2) / 7;
            case 2: return static_cast<int32_t>((key / 100 - 2020) * 12 + key % 100 - 1);
            default: return static_cast<int32_t>(key) - 2020;
        }
    };
    auto emit = [&](const char* name, unsigned kind, const std::vector<PeriodRecord>& v, uint32_t running_key) {
        out += ",\"";
        out += name;
        out += "\":[";
        for (size_t i = 0; i < v.size(); ++i) {
            snprintf(buf, sizeof(buf), "%s{\"k\":%" PRIu32 ",\"q\":%" PRId32 ",\"i\":%" PRIu32 ",\"e\":%" PRIu32 ",\"c\":%" PRId32
                     ",\"pv\":%" PRIu32 ",\"bc\":%" PRIu32 ",\"bd\":%" PRIu32 ",\"s\":%" PRId32 ",\"wb\":%" PRIu32 ",\"wm\":%" PRId32 "%s}",
                     i ? "," : "", v[i].key, seq_of(kind, v[i].key), v[i].import_wh, v[i].export_wh, v[i].cost_milli,
                     v[i].pv_wh, v[i].bat_chg_wh, v[i].bat_dis_wh, v[i].sav_milli, v[i].wb_wh, v[i].wb_milli,
                     v[i].key == running_key ? ",\"p\":1" : "");
            out += buf;
        }
        out += ']';
    };
    emit("days", 0, days, today.key);
    emit("weeks", 1, weeks, today.key != 0 ? week_key(today.key) : 0);
    emit("months", 2, months, today.key / 100);
    emit("years", 3, years, today.key / 10000);
    out += '}';
    return out;
}

} // namespace app
