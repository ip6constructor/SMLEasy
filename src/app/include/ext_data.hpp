#pragma once
#include <string>

namespace app {

/// Values from Home Assistant shown on the dashboard (battery SoC, PV power, wallbox, ...).
///
/// Two transports, chosen per slot:
///   rest : GET {ha_url}/api/states/{entity_id} with a long-lived access token (polled)
///   mqtt : a topic on the Home Assistant broker (pushed by an HA automation)
class ExtData {
public:
    static ExtData& get();

    /// Creates the polling task (once). Safe to call at boot after Wi-Fi is up.
    void start();

    /// Reloads the configuration: refreshes MQTT subscriptions and polls REST values soon.
    void apply_config();

    /// Polls the REST values now instead of waiting for the next interval.
    void refresh();

    /// {"enabled":..,"items":[{"label","value","unit","age_s","state","error","source"}]}
    std::string values_json() const;

    /// Sum of the fresh values of all slots with this role, converted to watts
    /// (kW values are scaled). "battery_neg" slots are added with inverted sign to "battery".
    /// Returns false when no slot with that role has a fresh value.
    bool role_power_w(const char* role, double& watts) const;

private:
    ExtData() = default;
    static void poll_task(void*);
    static void on_mqtt_message(const std::string& topic, const std::string& payload);
    struct Impl;
    Impl* impl();
    const Impl* impl() const;
};

} // namespace app
