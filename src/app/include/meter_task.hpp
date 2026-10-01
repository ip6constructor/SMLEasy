#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <atomic>
#include <cstdint>
#include <string>

namespace app {

/// FreeRTOS task for passive SML readout over optical UART.
class MeterTask {
public:
    static MeterTask& get();

    void init(int tx_pin, int rx_pin, int uart_num = 1); ///< Install UART driver (call once at boot)
    void start();            ///< Single-shot read: open port → read → close → stop
    void start_continuous(); ///< Start repeating reads at configured interval
    void stop();             ///< Signal stop after current cycle
    void trigger();          ///< Wake up sleeping continuous cycle immediately

    [[nodiscard]] bool is_running() const;
    [[nodiscard]] int  tx_pin()     const { return tx_pin_; }
    [[nodiscard]] int  rx_pin()     const { return rx_pin_; }

private:
    MeterTask() = default;
    static void task_fn(void* arg);
    void        run();

    bool         uart_installed_{false};
    int          tx_pin_{1};   ///< UART TX GPIO (TX=1, RX=3 confirmed working)
    int          rx_pin_{3};
    TaskHandle_t          task_handle_{nullptr};
    std::atomic<bool>     task_running_{false}; ///< true from xTaskCreate until just before vTaskDelete
    volatile bool stop_requested_{false};
};

} // namespace app
