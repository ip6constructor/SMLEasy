#pragma once
#include <string>

namespace app {

/// WiFi: STA with SoftAP fallback.
/// AP SSID: "Smartmeter-XXXXXX" (last 6 MAC hex digits), WPA2 PSK.
class WifiManager {
public:
    static constexpr uint32_t kStaTimeoutMs = 30000;
    static constexpr uint32_t kApFallbackWindowMs = 20u * 60u * 1000u;

    static WifiManager& get();

    /// Load config from NVS, start WiFi. Blocks until STA connects or AP active.
    bool start();

    /// Apply new credentials, stop the current WiFi mode, and connect again.
    bool reconnect(const std::string& ssid, const std::string& password);

    [[nodiscard]] bool        is_sta_connected() const { return sta_connected_; }
    [[nodiscard]] bool        is_ap_active()     const { return ap_active_; }
    [[nodiscard]] std::string get_ip()           const { return ip_; }
    [[nodiscard]] std::string ap_ssid()          const { return ap_ssid_; }
    [[nodiscard]] std::string sta_ssid()         const { return sta_ssid_; }

private:
    WifiManager() = default;
    static void ap_timeout_task_fn(void* arg);
    void schedule_ap_timeout(uint32_t timeout_ms);
    void stop_ap();
    void start_ap(const std::string& ssid);
    bool start_sta(const std::string& ssid, const std::string& pass);

    bool        sta_connected_{false};
    bool        ap_active_{false};
    bool        have_wifi_credentials_{false};
    std::string ip_;
    std::string ap_ssid_;
    std::string sta_ssid_;
};

} // namespace app
