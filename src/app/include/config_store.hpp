#pragma once
#include <cstdint>
#include <string>

namespace app {

struct MeterConfig {
    uint32_t    interval_s{30};  ///< Read interval in seconds (min 5)
    bool        uart_debug{false};  ///< true = log raw RX bytes each cycle
    bool        last_login_ok{false};  ///< Legacy field kept for UI compatibility
    std::string meter_pin{};  ///< Optional PIN used by login_cmd placeholder {PIN}
    std::string login_cmd{};  ///< Optional pre-read UART login command, supports {PIN} and escape sequences
    uint16_t    login_wait_ms{250};  ///< Delay after login command before frame read

    // Default profile: Iskraemeco MT631/MS2020 (Tasmota-like alias lists).
    // Format: comma-separated OBIS entries, e.g. "1-0:1.8.0*255,1-0:1.8.1*255".
    std::string obis_import_wh{"1-0:1.8.0*255,1-0:1.8.1*255,1-0:1.8.2*255"};
    std::string obis_export_wh{"1-0:2.8.0*255,1-0:2.8.1*255,1-0:2.8.2*255"};
    std::string obis_power_net_w{"1-0:16.7.0*255"};
    std::string obis_power_import_w{"1-0:1.7.0*255"};
    std::string obis_power_export_w{"1-0:2.7.0*255"};
    std::string obis_voltage_l1_v{"1-0:32.7.0*255"};
    std::string obis_voltage_l2_v{"1-0:52.7.0*255"};
    std::string obis_voltage_l3_v{"1-0:72.7.0*255"};
    std::string obis_current_l1_a{"1-0:31.7.0*255"};
    std::string obis_current_l2_a{"1-0:51.7.0*255"};
    std::string obis_current_l3_a{"1-0:71.7.0*255"};
    std::string obis_frequency_hz{"1-0:14.7.0*255,1-0:14.4.0*255"};
    std::string obis_pf_l1{"1-0:13.7.0*255"};
};

struct WifiConfig {
    std::string ssid{};
    std::string password{};  ///< empty = open network
};

struct WebAuthConfig {
    std::string username{"admin"};
    std::string password{"P@assword26"};
};

/// Tariff / pricing config used for cost estimation in the dashboard.
struct TariffConfig {
    double      price_import_kwh{0.30};  ///< price paid per imported kWh
    double      price_export_kwh{0.0664};  ///< feed-in compensation per exported kWh
    std::string currency{"EUR"};
};

/// Home Assistant integration config (transport: MQTT discovery).
struct HaConfig {
    bool        enabled{false};                     ///< Enable HA integration
    std::string broker_uri{"mqtt://192.168.1.1"}; ///< MQTT broker URI
    std::string username{};                         ///< empty = anonymous
    std::string password{};                         ///< broker password
    std::string device_name{"Smartmeter"};        ///< Human-readable device name
    std::string ha_prefix{"homeassistant"};       ///< HA discovery topic prefix
};

/// Persistent config stored in NVS.
class ConfigStore {
public:
    static ConfigStore& get();

    bool load_meter(MeterConfig& out);
    bool save_meter(const MeterConfig& cfg);

    bool load_wifi(WifiConfig& out);
    bool save_wifi(const WifiConfig& cfg);

    bool load_web_auth(WebAuthConfig& out);
    bool save_web_auth(const WebAuthConfig& cfg);
    bool reset_all_config();

    bool load_ha(HaConfig& out);
    bool save_ha(const HaConfig& cfg);

    bool load_tariff(TariffConfig& out);
    bool save_tariff(const TariffConfig& cfg);

private:
    ConfigStore() = default;
    static constexpr const char* kNsMeter  = "meter_cfg";
    static constexpr const char* kNsWifi   = "wifi_cfg";
    static constexpr const char* kNsAuth   = "web_auth";
    static constexpr const char* kNsHa     = "ha_cfg";
    static constexpr const char* kNsMqttLegacy = "mqtt_cfg";
    static constexpr const char* kNsTariff = "tariff_cfg";
};

} // namespace app
