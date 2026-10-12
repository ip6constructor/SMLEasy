#include "ext_data.hpp"
#include "config_store.hpp"
#include "mqtt_manager.hpp"
#include "wifi_manager.hpp"

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <array>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <vector>

static const char* TAG = "ExtData";

namespace app {

namespace {

struct SlotState {
    bool    has_value{false};
    double  value{0};
    std::string unit;
    std::string name;     // friendly_name reported by Home Assistant
    std::string error;
    int64_t updated_us{0};
};

struct HttpBody {
    std::string data;
    static constexpr size_t kLimit = 16384;
};

esp_err_t on_http_event(esp_http_client_event_t* event)
{
    auto* body = static_cast<HttpBody*>(event->user_data);
    if (body && event->event_id == HTTP_EVENT_ON_DATA && event->data && event->data_len > 0 &&
        body->data.size() + static_cast<size_t>(event->data_len) <= HttpBody::kLimit) {
        body->data.append(static_cast<const char*>(event->data), static_cast<size_t>(event->data_len));
    }
    return ESP_OK;
}

// Parses a plain number ("12.5", " 12,5 W" is not accepted on purpose).
bool parse_number(const std::string& text, double& out)
{
    const char* begin = text.c_str();
    while (*begin == ' ' || *begin == '\t' || *begin == '\r' || *begin == '\n') ++begin;
    if (*begin == '\0') return false;
    char* end = nullptr;
    const double value = strtod(begin, &end);
    if (end == begin) return false;
    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') ++end;
    if (*end != '\0') return false;
    out = value;
    return true;
}

// Walks a dotted path ("a.b.c") through JSON objects and reads a number or numeric string.
bool number_at_path(const cJSON* root, const std::string& path, double& out)
{
    const cJSON* node = root;
    size_t start = 0;
    while (node && start <= path.size()) {
        size_t dot = path.find('.', start);
        if (dot == std::string::npos) dot = path.size();
        const std::string part = path.substr(start, dot - start);
        if (part.empty()) return false;
        node = cJSON_GetObjectItemCaseSensitive(node, part.c_str());
        start = dot + 1;
        if (dot == path.size()) break;
    }
    if (!node) return false;
    if (cJSON_IsNumber(node)) {
        out = node->valuedouble;
        return true;
    }
    return cJSON_IsString(node) && node->valuestring && parse_number(node->valuestring, out);
}

} // namespace

struct ExtData::Impl {
    mutable std::mutex mutex;
    ExtDataConfig cfg;
    std::array<SlotState, ExtDataConfig::kMaxSlots> state;
    TaskHandle_t task{nullptr};
    bool started{false};
    std::string push_base;   // topic prefix for values pushed by Home Assistant
};

ExtData::Impl* ExtData::impl()
{
    static Impl instance;
    return &instance;
}

const ExtData::Impl* ExtData::impl() const
{
    return const_cast<ExtData*>(this)->impl();
}

ExtData& ExtData::get()
{
    static ExtData inst;
    return inst;
}

void ExtData::start()
{
    Impl* self = impl();
    {
        std::lock_guard<std::mutex> lock(self->mutex);
        if (self->started) return;
        self->started = true;
    }
    apply_config();
    if (xTaskCreate(poll_task, "ext_poll", 8192, nullptr, 3, &self->task) != pdPASS) {
        ESP_LOGE(TAG, "poll task could not be created");
    }
}

void ExtData::apply_config()
{
    Impl* self = impl();
    ExtDataConfig fresh;
    ConfigStore::get().load_ext(fresh);

    std::vector<std::string> topics;
    const std::string push_base = MqttManager::input_topic_base();
    {
        std::lock_guard<std::mutex> lock(self->mutex);
        self->push_base = push_base;
        // Keep known values when a slot still points at the same source.
        for (size_t i = 0; i < self->state.size(); ++i) {
            const bool same = i < fresh.slots.size() && i < self->cfg.slots.size() &&
                fresh.slots[i].source == self->cfg.slots[i].source &&
                fresh.slots[i].ref == self->cfg.slots[i].ref &&
                fresh.slots[i].key == self->cfg.slots[i].key;
            if (!same) self->state[i] = SlotState{};
        }
        self->cfg = fresh;
        if (fresh.enabled) {
            for (const auto& slot : fresh.slots)
                if (slot.source == "mqtt" && !slot.ref.empty()) topics.push_back(slot.ref);
                else if (slot.source == "push" && !slot.ref.empty()) topics.push_back(push_base + slot.ref);
        }
    }
    MqttManager::get().set_extra_topics(topics, on_mqtt_message);
    refresh();
}

void ExtData::refresh()
{
    Impl* self = impl();
    if (self->task) xTaskNotifyGive(self->task);
}

void ExtData::on_mqtt_message(const std::string& topic, const std::string& payload)
{
    Impl* self = ExtData::get().impl();
    std::lock_guard<std::mutex> lock(self->mutex);
    for (size_t i = 0; i < self->cfg.slots.size() && i < self->state.size(); ++i) {
        const ExtSlot& slot = self->cfg.slots[i];
        const bool matches = slot.source == "mqtt" ? slot.ref == topic
            : slot.source == "push" && topic == self->push_base + slot.ref;
        if (!matches) continue;
        double value = 0;
        bool ok = false;
        if (slot.key.empty()) {
            ok = parse_number(payload, value);
        } else if (cJSON* json = cJSON_ParseWithLength(payload.c_str(), payload.size())) {
            ok = number_at_path(json, slot.key, value);
            cJSON_Delete(json);
        }
        SlotState& state = self->state[i];
        if (ok) {
            state.has_value = true;
            state.value = value;
            state.error.clear();
            state.updated_us = esp_timer_get_time();
        } else {
            state.error = slot.key.empty() ? "payload is not a number" : "key not found or not numeric";
        }
    }
}

void ExtData::poll_task(void*)
{
    Impl* self = ExtData::get().impl();
    // First poll shortly after boot, then every poll_s seconds (or earlier on refresh()).
    uint32_t wait_ms = 5000;
    for (;;) {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(wait_ms));

        ExtDataConfig cfg;
        {
            std::lock_guard<std::mutex> lock(self->mutex);
            cfg = self->cfg;
        }
        wait_ms = static_cast<uint32_t>(cfg.poll_s < 10 ? 10 : cfg.poll_s) * 1000u;
        if (!cfg.enabled || cfg.ha_url.empty() || cfg.ha_token.empty() ||
            !WifiManager::get().is_sta_connected()) continue;

        bool any_rest = false;
        for (const auto& slot : cfg.slots) any_rest = any_rest || (slot.source == "rest" && !slot.ref.empty());
        if (!any_rest) continue;

        HttpBody body;
        esp_http_client_config_t http{};
        const std::string first_url = cfg.ha_url + "/api/states/x";
        http.url = first_url.c_str();
        http.timeout_ms = 6000;
        http.buffer_size = 2048;
        http.keep_alive_enable = true;
        http.event_handler = on_http_event;
        http.user_data = &body;
        if (cfg.ha_url.rfind("https://", 0) == 0) http.crt_bundle_attach = esp_crt_bundle_attach;
        esp_http_client_handle_t client = esp_http_client_init(&http);
        if (!client) {
            ESP_LOGW(TAG, "HTTP client could not be created");
            continue;
        }
        const std::string bearer = "Bearer " + cfg.ha_token;
        esp_http_client_set_header(client, "Authorization", bearer.c_str());
        esp_http_client_set_header(client, "Content-Type", "application/json");

        for (size_t i = 0; i < cfg.slots.size() && i < self->state.size(); ++i) {
            const ExtSlot& slot = cfg.slots[i];
            if (slot.source != "rest" || slot.ref.empty()) continue;
            body.data.clear();
            const std::string url = cfg.ha_url + "/api/states/" + slot.ref;
            esp_http_client_set_url(client, url.c_str());
            const esp_err_t err = esp_http_client_perform(client);
            const int status = esp_http_client_get_status_code(client);

            std::string error;
            double value = 0;
            std::string unit, name;
            bool ok = false;
            if (err != ESP_OK) {
                error = std::string("connection failed: ") + esp_err_to_name(err);
            } else if (status == 401 || status == 403) {
                error = "access token rejected (HTTP " + std::to_string(status) + ")";
            } else if (status == 404) {
                error = "entity not found (HTTP 404)";
            } else if (status != 200) {
                error = "HTTP " + std::to_string(status);
            } else if (cJSON* json = cJSON_ParseWithLength(body.data.c_str(), body.data.size())) {
                const cJSON* state = cJSON_GetObjectItemCaseSensitive(json, "state");
                const cJSON* attributes = cJSON_GetObjectItemCaseSensitive(json, "attributes");
                const cJSON* unit_item = attributes ? cJSON_GetObjectItemCaseSensitive(attributes, "unit_of_measurement") : nullptr;
                const cJSON* name_item = attributes ? cJSON_GetObjectItemCaseSensitive(attributes, "friendly_name") : nullptr;
                if (cJSON_IsString(unit_item) && unit_item->valuestring) unit = unit_item->valuestring;
                if (cJSON_IsString(name_item) && name_item->valuestring) name = name_item->valuestring;
                if (cJSON_IsString(state) && state->valuestring && parse_number(state->valuestring, value)) {
                    ok = true;
                } else {
                    error = std::string("state is not numeric: ") +
                            (cJSON_IsString(state) && state->valuestring ? state->valuestring : "?");
                }
                cJSON_Delete(json);
            } else {
                error = "unexpected response";
            }

            std::lock_guard<std::mutex> lock(self->mutex);
            SlotState& target = self->state[i];
            if (ok) {
                target.has_value = true;
                target.value = value;
                target.unit = unit;
                target.name = name;
                target.error.clear();
                target.updated_us = esp_timer_get_time();
            } else {
                target.error = error;
            }
        }
        esp_http_client_cleanup(client);
    }
}

bool ExtData::role_power_w(const char* role, double& watts) const
{
    const Impl* self = impl();
    std::lock_guard<std::mutex> lock(self->mutex);
    if (!self->cfg.enabled) return false;
    const int64_t now = esp_timer_get_time();
    const int stale_after_s = std::max<int>(120, self->cfg.poll_s * 3);
    const std::string want = role;
    const bool battery = want == "battery";
    double sum = 0;
    bool found = false;
    for (size_t i = 0; i < self->cfg.slots.size() && i < self->state.size(); ++i) {
        const ExtSlot& slot = self->cfg.slots[i];
        const SlotState& state = self->state[i];
        const bool match = slot.role == want || (battery && slot.role == "battery_neg");
        if (!match || !state.has_value || !state.error.empty()) continue;
        if ((now - state.updated_us) / 1000000 > stale_after_s) continue;
        std::string unit = !slot.unit.empty() ? slot.unit : state.unit;
        double value = state.value;
        if (unit == "kW" || unit == "kw") value *= 1000.0;
        sum += slot.role == "battery_neg" ? -value : value;
        found = true;
    }
    if (found) watts = sum;
    return found;
}

std::string ExtData::values_json() const
{
    const Impl* self = impl();
    std::lock_guard<std::mutex> lock(self->mutex);
    const int64_t now = esp_timer_get_time();
    const int stale_after_s = std::max<int>(120, self->cfg.poll_s * 3);

    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "enabled", self->cfg.enabled);
    cJSON* items = cJSON_AddArrayToObject(root, "items");
    if (self->cfg.enabled) {
        for (size_t i = 0; i < self->cfg.slots.size() && i < self->state.size(); ++i) {
            const ExtSlot& slot = self->cfg.slots[i];
            const SlotState& state = self->state[i];
            if (slot.ref.empty()) continue;
            cJSON* item = cJSON_CreateObject();
            const std::string label = !slot.label.empty() ? slot.label
                : !state.name.empty() ? state.name : slot.ref;
            const int age = state.has_value ? static_cast<int>((now - state.updated_us) / 1000000) : -1;
            const char* status = !state.error.empty() ? "error"
                : !state.has_value ? "waiting"
                : age > stale_after_s ? "stale" : "ok";
            cJSON_AddStringToObject(item, "label", label.c_str());
            cJSON_AddStringToObject(item, "source", slot.source.c_str());
            cJSON_AddStringToObject(item, "role", slot.role.c_str());
            cJSON_AddStringToObject(item, "state", status);
            if (state.has_value) cJSON_AddNumberToObject(item, "value", state.value);
            cJSON_AddStringToObject(item, "unit", !slot.unit.empty() ? slot.unit.c_str() : state.unit.c_str());
            cJSON_AddNumberToObject(item, "age_s", age);
            cJSON_AddStringToObject(item, "error", state.error.c_str());
            cJSON_AddItemToArray(items, item);
        }
    }
    char* text = cJSON_PrintUnformatted(root);
    std::string out(text ? text : "{\"enabled\":false,\"items\":[]}");
    cJSON_free(text);
    cJSON_Delete(root);
    return out;
}

} // namespace app
