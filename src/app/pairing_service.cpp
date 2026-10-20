#include "pairing_service.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include "app_tokens.hpp"
#include "config_store.hpp"
#include "key_provider.hpp"
#include "tls_certs.hpp"
#include "wifi_manager.hpp"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_tls.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "mbedtls/platform_util.h"
#include "psa/crypto.h"
#include "mdns.h"

namespace app::pairing {
namespace {
constexpr int64_t kWindowUs = 300LL * 1000000;
constexpr size_t kMaxFrame = 2048;
const char* TAG = "Pairing";
std::mutex mutex;
std::atomic<int64_t> until{0};
bool running = false;
int attempts = 0;
bool (*peer_filter)(int) = nullptr;
Status state;
int decision = 0;

std::string random_hex(size_t count)
{
    unsigned char bytes[32];
    esp_fill_random(bytes, count);
    std::string text;
    for (size_t i = 0; i < count; ++i) {
        char hex[3];
        std::snprintf(hex, sizeof(hex), "%02x", bytes[i]);
        text += hex;
    }
    mbedtls_platform_zeroize(bytes, sizeof(bytes));
    return text;
}

bool active()
{
    return until.load() > esp_timer_get_time();
}

void fail(const char* message)
{
    ESP_LOGW(TAG, "%s", message);
    std::lock_guard<std::mutex> lock(mutex);
    state.error = message;
}

// Both peers hash the observed TLS certificate, client nonce and request ID.
std::string confirmation_code(const std::string& fingerprint, const std::string& nonce,
                              const std::string& id)
{
    const std::string input = "smleasy-pair-v1:" + fingerprint + ":" + nonce + ":" + id;
    unsigned char hash[32];
    size_t hash_size = 0;
    if (psa_hash_compute(PSA_ALG_SHA_256, reinterpret_cast<const unsigned char*>(input.data()), input.size(),
                         hash, sizeof(hash), &hash_size) != PSA_SUCCESS || hash_size != sizeof(hash))
        return {};
    const uint32_t number = (uint32_t(hash[0]) << 24) | (uint32_t(hash[1]) << 16) |
                            (uint32_t(hash[2]) << 8) | hash[3];
    char code[7];
    std::snprintf(code, sizeof(code), "%06lu", static_cast<unsigned long>(number % 1000000));
    return code;
}

bool transfer(esp_tls_t* tls, unsigned char* bytes, size_t size, bool writing)
{
    const int64_t deadline = std::min(until.load(), esp_timer_get_time() + 5000000);
    size_t offset = 0;
    while (offset < size && active() && esp_timer_get_time() < deadline) {
        const ssize_t n = writing ? esp_tls_conn_write(tls, bytes + offset, size - offset)
                                  : esp_tls_conn_read(tls, bytes + offset, size - offset);
        if (n == ESP_TLS_ERR_SSL_WANT_READ || n == ESP_TLS_ERR_SSL_WANT_WRITE) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (n <= 0) return false;
        offset += static_cast<size_t>(n);
    }
    return offset == size;
}

bool send(esp_tls_t* tls, const std::string& text)
{
    unsigned char length[] = {static_cast<unsigned char>(text.size() >> 8),
                              static_cast<unsigned char>(text.size())};
    return text.size() <= kMaxFrame && transfer(tls, length, 2, true) &&
        transfer(tls, reinterpret_cast<unsigned char*>(const_cast<char*>(text.data())), text.size(), true);
}

std::string receive(esp_tls_t* tls)
{
    unsigned char length[2];
    if (!transfer(tls, length, 2, false)) return {};
    const size_t size = (size_t(length[0]) << 8) | length[1];
    if (size == 0 || size > kMaxFrame) return {};
    std::string text(size, '\0');
    if (!transfer(tls, reinterpret_cast<unsigned char*>(&text[0]), size, false)) return {};
    return text;
}

std::string field(cJSON* json, const char* key)
{
    const cJSON* value = cJSON_GetObjectItemCaseSensitive(json, key);
    return cJSON_IsString(value) && value->valuestring ? value->valuestring : "";
}

bool hex_nonce(const std::string& text)
{
    return text.size() == 64 && text.find_first_not_of("0123456789abcdef") == std::string::npos;
}

bool reply(esp_tls_t* tls, cJSON* json)
{
    if (!json) return false;
    char* text = cJSON_PrintUnformatted(json);
    const bool ok = text && send(tls, text);
    if (text) {
        mbedtls_platform_zeroize(text, std::strlen(text));
        cJSON_free(text);
    }
    cJSON_Delete(json);
    return ok;
}

void session(esp_tls_t* tls, const std::string& fingerprint)
{
    const std::string raw = receive(tls);
    cJSON* request = cJSON_ParseWithLength(raw.data(), raw.size());
    const std::string nonce = field(request, "nonce");
    std::string name = field(request, "name");
    const bool valid = field(request, "op") == "pair" && hex_nonce(nonce) &&
                       !name.empty() && name.size() <= 40;
    cJSON_Delete(request);
    if (!valid) {
        send(tls, "{\"status\":\"error\",\"error\":\"Invalid pairing request\"}");
        return;
    }
    for (char& c : name) if (static_cast<unsigned char>(c) < 32) c = ' ';
    const std::string id = random_hex(16);
    const std::string code = confirmation_code(fingerprint, nonce, id);
    if (code.empty()) { fail("Confirmation hash failed"); return; }
    {
        std::lock_guard<std::mutex> lock(mutex);
        state.request_id = id;
        state.name = name;
        state.code = code;
        decision = 0;
    }
    cJSON* pending = cJSON_CreateObject();
    cJSON_AddStringToObject(pending, "status", "pending");
    cJSON_AddStringToObject(pending, "request_id", id.c_str());
    if (!reply(tls, pending)) return;
    while (active()) {
        const std::string poll = receive(tls);
        cJSON* json = cJSON_ParseWithLength(poll.data(), poll.size());
        const bool valid_poll = field(json, "op") == "poll" && field(json, "request_id") == id;
        cJSON_Delete(json);
        if (!valid_poll) return;
        int choice;
        {
            std::lock_guard<std::mutex> lock(mutex);
            choice = decision;
        }
        if (choice < 0) { send(tls, "{\"status\":\"rejected\"}"); return; }
        if (choice > 0) {
            std::string token;
            if (!tokens::issue(name, token)) {
                fail("App token could not be saved (storage error or all 4 slots used)");
                send(tls, "{\"status\":\"error\",\"error\":\"App token could not be saved; check free app slots\"}");
                return;
            }
            TlsConfig cfg;
            ConfigStore::get().load_tls(cfg);
            cJSON* result = cJSON_CreateObject();
            cJSON_AddStringToObject(result, "status", "approved");
            cJSON_AddStringToObject(result, "token", token.c_str());
            cJSON_AddStringToObject(result, "fingerprint", fingerprint.c_str());
            cJSON_AddStringToObject(result, "device_id", keys::device_id_hex().c_str());
            cJSON_AddStringToObject(result, "fqdn", cfg.fqdn.c_str());
            cJSON_AddNumberToObject(result, "https_port", 443);
            const std::string ipv4 = WifiManager::get().get_ip();
            cJSON_AddStringToObject(result, "ipv4", ipv4.c_str());
            cJSON* addresses = cJSON_AddArrayToObject(result, "ipv6");
            for (const auto& address : WifiManager::get().get_ip6_addresses()) {
                // Link-local needs the phone's interface scope, supplied by NSD instead.
                if (address.rfind("fe80", 0) != 0)
                    cJSON_AddItemToArray(addresses, cJSON_CreateString(address.c_str()));
            }
            if (!reply(tls, result)) fail("Pairing response lost; revoke the unused app and retry");
            mbedtls_platform_zeroize(&token[0], token.size());
            until.store(0);
            return;
        }
        if (!send(tls, "{\"status\":\"waiting\"}")) return;
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void task(void*)
{
    std::string certificate, key;
    int listener = -1;
    bool advertised = false;
    do {
        if (!tls_certs::load_identity(certificate, key)) { fail("Pairing TLS identity unavailable"); break; }
        const std::string fingerprint = tls_certs::fingerprint(certificate);
        if (fingerprint.empty()) { fail("Pairing certificate fingerprint unavailable"); break; }
#if CONFIG_LWIP_IPV6
        listener = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        int v6only = 0;
        if (listener >= 0 && setsockopt(listener, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only)) != 0) {
            fail("Pairing dual-stack socket failed"); break;
        }
        sockaddr_in6 address{};
        address.sin6_family = AF_INET6;
        address.sin6_port = htons(7384);
#else
        listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(7384);
        address.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
        int reuse = 1;
        if (listener < 0 || setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0 ||
            bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
            listen(listener, 1) != 0) { fail("Pairing port 7384 unavailable"); break; }
        if (mdns_service_add(nullptr, "_smleasy-pair", "_tcp", 7384, nullptr, 0) != ESP_OK) {
            fail("Pairing discovery could not be advertised"); break;
        }
        advertised = true;
        while (active()) {
            fd_set set;
            FD_ZERO(&set);
            FD_SET(listener, &set);
            timeval wait{0, 200000};
            const int ready = select(listener + 1, &set, nullptr, nullptr, &wait);
            if (ready < 0) { fail("Pairing listener failed"); break; }
            if (ready == 0) continue;
            const int client = accept(listener, nullptr, nullptr);
            if (client < 0) { fail("Pairing accept failed"); continue; }
            if (!active() || !peer_filter(client)) { ::close(client); continue; }
            if (++attempts > 10) {
                ::close(client);
                fail("Pairing attempt limit reached; reopen pairing mode");
                break;
            }
            timeval timeout{1, 0};
            if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
                setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0) {
                ::close(client); fail("Pairing socket timeout setup failed"); continue;
            }
            esp_tls_cfg_server_t cfg{};
            cfg.servercert_buf = reinterpret_cast<const unsigned char*>(certificate.c_str());
            cfg.servercert_bytes = certificate.size() + 1;
            cfg.serverkey_buf = reinterpret_cast<const unsigned char*>(key.c_str());
            cfg.serverkey_bytes = key.size() + 1;
            cfg.tls_handshake_timeout_ms = 5000;
            esp_tls_t* tls = esp_tls_init();
            if (!tls) {
                ::close(client);
                fail("Pairing TLS allocation failed");
                break;
            }
            const int flags = fcntl(client, F_GETFL, 0);
            int handshake = -1;
            if (flags >= 0 && fcntl(client, F_SETFL, flags | O_NONBLOCK) == 0 &&
                esp_tls_server_session_init(&cfg, client, tls) == ESP_OK) {
                const int64_t deadline = std::min(until.load(), esp_timer_get_time() + 5000000);
                do {
                    handshake = esp_tls_server_session_continue_async(tls);
                    if (handshake != ESP_TLS_ERR_SSL_WANT_READ && handshake != ESP_TLS_ERR_SSL_WANT_WRITE) break;
                    vTaskDelay(pdMS_TO_TICKS(10));
                } while (active() && esp_timer_get_time() < deadline);
            }
            if (handshake == 0 && active()) session(tls, fingerprint);
            else if (active()) fail("Pairing TLS handshake failed");
            esp_tls_server_session_delete(tls);
            // The server-session API frees TLS state, but the caller owns the socket.
            ::close(client);
            std::lock_guard<std::mutex> lock(mutex);
            state.request_id.clear();
            state.name.clear();
            state.code.clear();
            decision = 0;
        }
    } while (false);
    until.store(0);
    if (advertised && mdns_service_remove("_smleasy-pair", "_tcp") != ESP_OK)
        fail("Pairing discovery cleanup failed");
    if (listener >= 0) ::close(listener);
    if (!key.empty()) mbedtls_platform_zeroize(&key[0], key.size());
    {
        std::lock_guard<std::mutex> lock(mutex);
        state.request_id.clear();
        state.name.clear();
        state.code.clear();
        running = false;
    }
    vTaskDelete(nullptr);
}
} // namespace

esp_err_t open(bool (*allow_peer)(int))
{
    // A closed window may still be winding down; wait briefly, but never start a second mode.
    for (int attempt = 0; attempt < 30; ++attempt) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!running) {
                state = {};
                decision = 0;
                attempts = 0;
                peer_filter = allow_peer;
                until.store(esp_timer_get_time() + kWindowUs);
                running = true;
                if (xTaskCreate(task, "pairing", 10240, nullptr, 3, nullptr) != pdPASS) {
                    running = false;
                    until.store(0);
                    state.error = "Pairing task allocation failed";
                    ESP_LOGE(TAG, "%s", state.error.c_str());
                    return ESP_ERR_NO_MEM;
                }
                return ESP_OK;
            }
            if (active()) return ESP_ERR_INVALID_STATE;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return ESP_ERR_TIMEOUT;
}

void close()
{
    until.store(0);
}

Status status()
{
    std::lock_guard<std::mutex> lock(mutex);
    Status result = state;
    const int64_t remaining = until.load() - esp_timer_get_time();
    result.open = remaining > 0;
    result.remaining_s = result.open ? static_cast<int>((remaining + 999999) / 1000000) : 0;
    if (!result.open) { result.request_id.clear(); result.name.clear(); result.code.clear(); }
    return result;
}

bool decide(const std::string& request_id, bool approve)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!active() || request_id.empty() || request_id != state.request_id || decision != 0) return false;
    decision = approve ? 1 : -1;
    return true;
}
} // namespace app::pairing
