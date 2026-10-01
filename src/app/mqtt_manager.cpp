// © 2024–2026 Michael Kreutzer — Freeware, no warranty
#include "mqtt_manager.hpp"
#include "mqtt_client.h"   // included here (not in header) so header compiles without mqtt component
#include "wifi_manager.hpp"
#include "app_state.hpp"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "cJSON.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>

static const char* TAG = "HaIntegration";

namespace app {

MqttManager& MqttManager::get()
{
    static MqttManager inst;
    return inst;
}

// ── Static event handler ──────────────────────────────────────────────────────

void MqttManager::event_handler(void*            handler_args,
                                 esp_event_base_t /*base*/,
                                 int32_t          event_id,
                                 void*            event_data)
{
    auto* self = static_cast<MqttManager*>(handler_args);
    auto* ev   = static_cast<esp_mqtt_event_handle_t>(event_data);
    (void)ev;

    switch (static_cast<esp_mqtt_event_id_t>(event_id)) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "HA transport connected to %s", self->_cfg.broker_uri.c_str());
            self->_connected.store(true);
            self->_discovery_sent = false; // re-send discovery on reconnect
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "HA transport disconnected — will auto-reconnect");
            self->_connected.store(false);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "HA transport error (type=%d)",
                     ev->error_handle ? ev->error_handle->error_type : -1);
            break;
        default:
            break;
    }
}

// ── init / stop ───────────────────────────────────────────────────────────────

void MqttManager::init(const HaConfig& cfg)
{
    _cfg = cfg;
    if (!cfg.enabled) {
        ESP_LOGI(TAG, "HA integration disabled — skipping init");
        return;
    }

    stop(); // clean up any previous client

    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.uri = _cfg.broker_uri.c_str();
    if (!_cfg.username.empty())
        mqtt_cfg.username = _cfg.username.c_str();
    if (!_cfg.password.empty())
        mqtt_cfg.password = _cfg.password.c_str();

    // Last Will & Testament so HA shows "offline" when the device disconnects
    const std::string device_id  = make_device_id(_cfg.device_name);
    const std::string base_topic = std::string("ha_meter/") + device_id;
    const std::string avail_topic = base_topic + "/availability";
    mqtt_cfg.lwt_topic   = avail_topic.c_str();
    mqtt_cfg.lwt_msg     = "offline";
    mqtt_cfg.lwt_msg_len = 7;
    mqtt_cfg.lwt_qos     = 1;
    mqtt_cfg.lwt_retain  = 1;

    _client = esp_mqtt_client_init(&mqtt_cfg);
    if (!_client) {
        ESP_LOGE(TAG, "esp_mqtt_client_init failed");
        return;
    }
    esp_mqtt_client_register_event(_client, MQTT_EVENT_ANY,
                                   event_handler, this);
    esp_mqtt_client_start(_client);
    ESP_LOGI(TAG, "HA transport started → %s (device_id=%s)",
             _cfg.broker_uri.c_str(), device_id.c_str());
}

void MqttManager::stop()
{
    if (_client) {
        esp_mqtt_client_stop(_client);
        esp_mqtt_client_destroy(_client);
        _client = nullptr;
        _connected.store(false);
        _discovery_sent = false;
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

std::string MqttManager::make_device_id(const std::string& name)
{
    std::string id;
    id.reserve(name.size());
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)))
            id += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        else
            id += '_';
    }
    return id;
}

std::string MqttManager::build_state_json(const MeterData& md) const
{
    cJSON* root = cJSON_CreateObject();
    // Device info
    cJSON_AddStringToObject(root, "manufacturer",   md.manufacturer.c_str());
    cJSON_AddStringToObject(root, "model",          md.model.c_str());
    cJSON_AddStringToObject(root, "fw_version",     md.fw_version.c_str());
    cJSON_AddStringToObject(root, "serial_bcd",         md.serial_bcd.c_str());
    cJSON_AddStringToObject(root, "utility_serial", md.utility_serial.c_str());
    cJSON_AddStringToObject(root, "meter_time",     md.meter_time.c_str());
    cJSON_AddStringToObject(root, "ip",
        WifiManager::get().get_ip().c_str());
    auto js = [](JobState s) -> const char* {
        switch (s) {
            case JobState::Running: return "running";
            case JobState::Done: return "done";
            case JobState::Error: return "error";
            default: return "idle";
        }
    };
    cJSON_AddStringToObject(root, "system_state", js(AppState::get().job_state()));
    cJSON_AddNumberToObject(root, "uptime_s", (double)(esp_timer_get_time() / 1000000LL));
    // Energy counters (Wh — already in integer Wh, no scaling)
    cJSON_AddNumberToObject(root, "fwd_active_wh",     md.fwd_active_wh);
    cJSON_AddNumberToObject(root, "rev_active_wh",     md.rev_active_wh);
    cJSON_AddNumberToObject(root, "import_react_varh", md.import_react_varh);
    cJSON_AddNumberToObject(root, "export_react_varh", md.export_react_varh);
    // Power (W)
    cJSON_AddNumberToObject(root, "fwd_w", md.fwd_w);
    cJSON_AddNumberToObject(root, "rev_w", md.rev_w);
    // Voltage (V, scaled from mV)
    cJSON_AddNumberToObject(root, "v_l1", md.v_l1_mv / 1000.0);
    cJSON_AddNumberToObject(root, "v_l2", md.v_l2_mv / 1000.0);
    cJSON_AddNumberToObject(root, "v_l3", md.v_l3_mv / 1000.0);
    // Current (A, scaled from mA)
    cJSON_AddNumberToObject(root, "i_l1", md.i_l1_ma / 1000.0);
    cJSON_AddNumberToObject(root, "i_l2", md.i_l2_ma / 1000.0);
    cJSON_AddNumberToObject(root, "i_l3", md.i_l3_ma / 1000.0);
    // Frequency (Hz, scaled from mHz)
    cJSON_AddNumberToObject(root, "freq", md.freq_mhz / 1000.0);
    // Power factor (dimensionless, scaled from milli)
    cJSON_AddNumberToObject(root, "pf_l1", md.pf_l1 / 1000.0);
    cJSON_AddNumberToObject(root, "pf_l2", md.pf_l2 / 1000.0);
    cJSON_AddNumberToObject(root, "pf_l3", md.pf_l3 / 1000.0);
    // Alarms
    cJSON_AddBoolToObject  (root, "has_alarms", md.has_alarms);
    cJSON_AddStringToObject(root, "alarm_list", md.alarm_list.c_str());

    char* s = cJSON_PrintUnformatted(root);
    std::string result(s);
    cJSON_free(s);
    cJSON_Delete(root);
    return result;
}

// ── HA Discovery ──────────────────────────────────────────────────────────────

void MqttManager::publish_discovery_sensor(const std::string& device_id,
                                            const std::string& sensor_id,
                                            const std::string& name,
                                            const std::string& device_class,
                                            const std::string& state_class,
                                            const std::string& unit,
                                            const std::string& value_template,
                                            const std::string& state_topic,
                                            const std::string& avail_topic,
                                            const std::string& device_json)
{
    const std::string unique_id    = std::string("ha_meter_") + device_id + "_" + sensor_id;
    const std::string config_topic = _cfg.ha_prefix + "/sensor/" + device_id
                                     + "/" + sensor_id + "/config";

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name",           name.c_str());
    cJSON_AddStringToObject(root, "unique_id",      unique_id.c_str());
    cJSON_AddStringToObject(root, "state_topic",    state_topic.c_str());
    cJSON_AddStringToObject(root, "value_template", value_template.c_str());
    cJSON_AddStringToObject(root, "availability_topic", avail_topic.c_str());
    cJSON_AddStringToObject(root, "payload_available",    "online");
    cJSON_AddStringToObject(root, "payload_not_available","offline");
    if (!state_class.empty())
        cJSON_AddStringToObject(root, "state_class", state_class.c_str());
    if (!device_class.empty())
        cJSON_AddStringToObject(root, "device_class", device_class.c_str());
    if (!unit.empty())
        cJSON_AddStringToObject(root, "unit_of_measurement", unit.c_str());

    // Device node (parse the pre-built JSON string into a cJSON object)
    cJSON* dev = cJSON_Parse(device_json.c_str());
    if (dev) cJSON_AddItemToObject(root, "device", dev);

    char* s = cJSON_PrintUnformatted(root);
    esp_mqtt_client_publish(_client, config_topic.c_str(), s,
                            static_cast<int>(strlen(s)),
                            /*qos*/1, /*retain*/1);
    cJSON_free(s);
    cJSON_Delete(root);
}

void MqttManager::publish_discovery(const std::string& device_id,
                                     const std::string& state_topic,
                                     const std::string& avail_topic)
{
    // Build device JSON once for all sensors
    cJSON* dev = cJSON_CreateObject();
    cJSON* ids = cJSON_CreateArray();
    cJSON_AddItemToArray(ids, cJSON_CreateString((std::string("ha_meter_") + device_id).c_str()));
    cJSON_AddItemToObject(dev, "identifiers",  ids);
    cJSON_AddStringToObject(dev, "name",         _cfg.device_name.c_str());
    cJSON_AddStringToObject(dev, "manufacturer", "Michael Kreutzer");
    cJSON_AddStringToObject(dev, "model",        "ESP32 Optical Meter Reader");
    cJSON_AddStringToObject(dev, "sw_version",   esp_ota_get_app_description()->version);
    const std::string cfg_url = "http://" + WifiManager::get().get_ip();
    cJSON_AddStringToObject(dev, "configuration_url", cfg_url.c_str());
    char* dev_s = cJSON_PrintUnformatted(dev);
    std::string dev_json(dev_s);
    cJSON_free(dev_s);
    cJSON_Delete(dev);

    // Sensor table:  { id, name, device_class, state_class, unit, value_template }
    struct SensorDef {
        const char* id;
        const char* name;
        const char* device_class;
        const char* state_class;
        const char* unit;
        const char* tpl;
    };
    static const SensorDef sensors[] = {
        { "fwd_active_wh",     "Vorwärts Wirkenergie",   "energy",       "total_increasing", "Wh",   "{{ value_json.fwd_active_wh }}" },
        { "rev_active_wh",     "Rückwärts Wirkenergie",  "energy",       "total_increasing", "Wh",   "{{ value_json.rev_active_wh }}" },
        { "import_react_varh", "Import Blindenergie",    "",             "total_increasing", "varh", "{{ value_json.import_react_varh }}" },
        { "export_react_varh", "Export Blindenergie",    "",             "total_increasing", "varh", "{{ value_json.export_react_varh }}" },
        { "fwd_w",             "Wirkleistung (+)",       "power",        "measurement",      "W",    "{{ value_json.fwd_w }}" },
        { "rev_w",             "Wirkleistung (−)",       "power",        "measurement",      "W",    "{{ value_json.rev_w }}" },
        { "v_l1",              "Spannung L1",            "voltage",      "measurement",      "V",    "{{ value_json.v_l1 | round(3) }}" },
        { "v_l2",              "Spannung L2",            "voltage",      "measurement",      "V",    "{{ value_json.v_l2 | round(3) }}" },
        { "v_l3",              "Spannung L3",            "voltage",      "measurement",      "V",    "{{ value_json.v_l3 | round(3) }}" },
        { "i_l1",              "Strom L1",               "current",      "measurement",      "A",    "{{ value_json.i_l1 | round(3) }}" },
        { "i_l2",              "Strom L2",               "current",      "measurement",      "A",    "{{ value_json.i_l2 | round(3) }}" },
        { "i_l3",              "Strom L3",               "current",      "measurement",      "A",    "{{ value_json.i_l3 | round(3) }}" },
        { "freq",              "Frequenz",               "frequency",    "measurement",      "Hz",   "{{ value_json.freq | round(2) }}" },
        { "pf_l1",             "Leistungsfaktor L1",     "power_factor", "measurement",      "",     "{{ value_json.pf_l1 | round(3) }}" },
        { "system_state",      "Systemstatus",           "",             "",                 "",     "{{ value_json.system_state }}" },
        { "uptime_s",          "System Uptime",          "duration",     "measurement",      "s",    "{{ value_json.uptime_s | round(0) }}" },
    };

    for (const auto& s : sensors) {
        publish_discovery_sensor(device_id, s.id, s.name,
                                 s.device_class, s.state_class, s.unit, s.tpl,
                                 state_topic, avail_topic, dev_json);
    }
    ESP_LOGI(TAG, "HA discovery published (%zu sensors)", sizeof(sensors)/sizeof(sensors[0]));
}

// ── publish ───────────────────────────────────────────────────────────────────

void MqttManager::publish(const MeterData& md)
{
    if (!_cfg.enabled || !_client || !_connected.load()) return;

    const std::string device_id   = make_device_id(_cfg.device_name);
    const std::string base_topic  = std::string("ha_meter/") + device_id;
    const std::string state_topic = base_topic + "/state";
    const std::string avail_topic = base_topic + "/availability";

    // Send HA discovery configs once per connection (retained, so HA survives restarts)
    if (!_discovery_sent) {
        publish_discovery(device_id, state_topic, avail_topic);
        _discovery_sent = true;
    }

    // Mark online
    esp_mqtt_client_publish(_client, avail_topic.c_str(),
                            "online", 6, /*qos*/1, /*retain*/1);

    // Publish state
    const std::string payload = build_state_json(md);
    int msg_id = esp_mqtt_client_publish(_client, state_topic.c_str(),
                                          payload.c_str(),
                                          static_cast<int>(payload.size()),
                                          /*qos*/1, /*retain*/1);
    if (msg_id < 0)
        ESP_LOGW(TAG, "HA publish failed (not connected?)");
    else
        ESP_LOGI(TAG, "HA state published → %s (msg_id=%d)", state_topic.c_str(), msg_id);
}

} // namespace app
