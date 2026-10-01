#pragma once
#include <string>

/// Push a log line into the RTC RAM ring buffer (called from main.cpp log hook).
/// extern "C" so it can be called without namespace qualification.
extern "C" void rtc_log_push(const char* line);

namespace app {

/// Returns the last N log lines from before the last reboot (RTC RAM).
std::string rtc_log_get_last();

} // namespace app
