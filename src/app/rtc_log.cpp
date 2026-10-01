/// rtc_log.cpp — Crash-persistenter Log-Ringpuffer im RTC SLOW RAM.
/// Überlebt Soft-Resets (Panic, Task-Watchdog, Stack-Overflow) und ist
/// nach dem Neustart intern für Diagnosezwecke verfügbar.

#include "esp_attr.h"   // RTC_DATA_ATTR, RTC_NOINIT_ATTR
#include "esp_system.h"
#include <cstring>
#include <cstdio>
#include <string>

// ── RTC-RAM Ringpuffer ────────────────────────────────────────────────────────
// RTC_NOINIT_ATTR: section survives reboot WITHOUT being zeroed by startup code.
// We use a magic sentinel to distinguish a fresh power-on from a crash-reboot.

static constexpr uint32_t MAGIC       = 0xDEAD1234u;
static constexpr int      N_LINES     = 12;
static constexpr int      LINE_LEN    = 120;

struct RtcLogBuf {
    uint32_t magic;
    int      head;                    ///< next write index
    int      count;                   ///< how many lines valid (0..N_LINES)
    char     lines[N_LINES][LINE_LEN];
};

RTC_NOINIT_ATTR static RtcLogBuf s_buf;

static void ensure_init() {
    if (s_buf.magic != MAGIC) {
        memset(&s_buf, 0, sizeof(s_buf));
        s_buf.magic = MAGIC;
    }
}

extern "C" void rtc_log_push(const char* line) {
    ensure_init();
    strncpy(s_buf.lines[s_buf.head], line, LINE_LEN - 1);
    s_buf.lines[s_buf.head][LINE_LEN - 1] = '\0';
    s_buf.head = (s_buf.head + 1) % N_LINES;
    if (s_buf.count < N_LINES) ++s_buf.count;
}

// ── Public API ────────────────────────────────────────────────────────────────

namespace app {

/// Returns a multiline string with the last N_LINES log entries from before
/// the last reboot, oldest first.
std::string rtc_log_get_last() {
    ensure_init();
    std::string out;
    if (s_buf.count == 0) return "(keine Log-Einträge vor dem Neustart)";
    // Oldest entry is at (head - count + N_LINES) % N_LINES
    int start = (s_buf.head - s_buf.count + N_LINES * 2) % N_LINES;
    for (int i = 0; i < s_buf.count; ++i) {
        int idx = (start + i) % N_LINES;
        out += s_buf.lines[idx];
        out += '\n';
    }
    return out;
}

} // namespace app
