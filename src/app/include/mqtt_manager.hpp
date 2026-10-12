#pragma once
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
// mqtt_client.h is included only in mqtt_manager.cpp to keep this header lightweight.
// Forward-declare the handle type so this header compiles without the mqtt component.
struct esp_mqtt_client;
using esp_mqtt_client_handle_t = esp_mqtt_client*;
// esp_event_base_t is needed for the event_handler signature.
// esp_event_base.h is a tiny header with no mqtt dependency.
#include "esp_event_base.h"
#include "app_state.hpp"
#include "config_store.hpp"

namespace app {

/// Home Assistant integration manager using MQTT discovery transport.
///
/// Topic layout:
///   State    : ha_meter/{device_id}/state          — JSON with all meter values
///   Avail.   : ha_meter/{device_id}/availability   — "online" / "offline" (LWT)
///   Discovery: {ha_prefix}/sensor/{device_id}/{id}/config
///
/// Call init() once at boot, then publish(md) after every successful read.
/// The client reconnects automatically; publish() is a no-op when disconnected.
class MqttManager {
public:
    static MqttManager& get();

    /// Configure and start the MQTT client.  Safe to call multiple times —
    /// stops and restarts the client with the new config.
    void init(const HaConfig& cfg);

    /// Stop the client (e.g. before reconfiguring).
    void stop();

    /// Publish all meter values and (if enabled) HA discovery configs.
    /// No-op when MQTT is disabled or not yet connected.
    void publish(const MeterData& md);

    bool is_connected() const { return _connected.load(); }

    /// Called for every message on a topic registered with set_extra_topics().
    using MessageCallback = void (*)(const std::string& topic, const std::string& payload);

    /// Subscribes to additional (read-only) topics on the HA broker, e.g. values published
    /// by Home Assistant automations. Re-subscribed automatically after reconnects.
    void set_extra_topics(const std::vector<std::string>& topics, MessageCallback callback);

    /// Topic prefix (with trailing slash) under which Home Assistant pushes values to this device.
    static std::string input_topic_base();

private:
    MqttManager() = default;
    MqttManager(const MqttManager&) = delete;

    void subscribe_extra();

    /// Build a device-ID slug from device_name (lowercase, spaces → underscores).
    static std::string make_device_id(const std::string& name);

    /// Publish a single HA discovery config topic.
    void publish_discovery_sensor(const std::string& device_id,
                                  const std::string& sensor_id,
                                  const std::string& name,
                                  const std::string& device_class,   ///< "" = none
                                  const std::string& state_class,
                                  const std::string& unit,
                                  const std::string& value_template,
                                  const std::string& state_topic,
                                  const std::string& avail_topic,
                                  const std::string& device_json);

    /// Publish HA discovery configs for all sensors.
    void publish_discovery(const std::string& device_id,
                           const std::string& state_topic,
                           const std::string& avail_topic);

    /// Build the state JSON payload from meter data.
    std::string build_state_json(const MeterData& md) const;

    static void event_handler(void*           handler_args,
                               esp_event_base_t base,
                               int32_t          event_id,
                               void*            event_data);

    esp_mqtt_client_handle_t _client{nullptr};
    HaConfig                 _cfg;
    std::atomic<bool>        _connected{false};
    bool                     _discovery_sent{false};
    std::mutex               _extra_mutex;
    std::vector<std::string> _extra_topics;
    MessageCallback          _on_message{nullptr};
};

} // namespace app
