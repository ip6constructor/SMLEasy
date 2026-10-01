#include "app_state.hpp"
#include "esp_log.h"
#include <cstring>
#include <sstream>

namespace app {

AppState& AppState::get() {
    static AppState inst;
    return inst;
}

AppState::AppState() {
    mutex_ = xSemaphoreCreateMutex();
    configASSERT(mutex_);
}

// ── Meter data ────────────────────────────────────────────────────────────────

MeterData AppState::get_meter_data() const {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    MeterData copy = meter_data_;
    xSemaphoreGive(mutex_);
    return copy;
}

void AppState::set_meter_data(const MeterData& d) {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    meter_data_ = d;
    xSemaphoreGive(mutex_);
}

// ── Job state ─────────────────────────────────────────────────────────────────

JobState AppState::job_state() const {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    JobState s = job_state_;
    xSemaphoreGive(mutex_);
    return s;
}

void AppState::set_job_state(JobState s) {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    job_state_ = s;
    xSemaphoreGive(mutex_);
}

std::string AppState::job_error() const {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    std::string e = job_error_;
    xSemaphoreGive(mutex_);
    return e;
}

void AppState::set_job_error(const std::string& e) {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    job_error_ = e;
    xSemaphoreGive(mutex_);
}

// ── Log ring buffer ───────────────────────────────────────────────────────────

void AppState::push_log(const char* level, const char* tag, const char* msg) {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    if (log_lines_.size() >= kMaxLogLines) {
        log_lines_.pop_front();
    }
    log_lines_.push_back({ ++log_seq_, level, tag, msg });
    xSemaphoreGive(mutex_);
}

uint32_t AppState::log_seq() const {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    uint32_t s = log_seq_;
    xSemaphoreGive(mutex_);
    return s;
}

// Escape a string for JSON (minimal: only backslash, quote, control chars).
static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else                out.push_back(c);
    }
    return out;
}

std::string AppState::log_json(uint32_t after_seq) const {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    std::string out = "[";
    bool first = true;
    for (const auto& l : log_lines_) {
        if (l.seq <= after_seq) continue;
        if (!first) out += ',';
        first = false;
        out += "{\"seq\":";
        out += std::to_string(l.seq);
        out += ",\"level\":\""; out += json_escape(l.level);
        out += "\",\"tag\":\"";  out += json_escape(l.tag);
        out += "\",\"msg\":\"";  out += json_escape(l.msg);
        out += "\"}";
    }
    out += ']';
    xSemaphoreGive(mutex_);
    return out;
}

} // namespace app
