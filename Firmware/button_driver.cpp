#include "button_driver.hpp"
#include "esp_timer.h"
#include "esp_log.h"

static const char* TAG = "BTN";

QueueHandle_t ButtonDriver::raw_gpio_queue_ = nullptr;
QueueHandle_t ButtonDriver::app_event_queue_ = nullptr;

struct RawEdgeEvent {
    ButtonId id;
    bool isPressed;
    int64_t timestamp;
};

void IRAM_ATTR ButtonDriver::isrHandler(void* arg) {
    auto id = static_cast<ButtonId>(reinterpret_cast<uintptr_t>(arg));
    gpio_num_t pin = (id == ButtonId::BTN_SCROLL) ? BTN_SCROLL_PIN : (id == ButtonId::BTN_SELECT) ? BTN_SELECT_PIN : BTN_BACK_PIN;

    RawEdgeEvent edge = {
        .id = id,
        .pressed = (gpio_get_level(pin) == 0),
        .timestamp = esp_timer_get_time()
    };

    BaseType_t higherPriorityTaskWoken = pdFALSE;

    xQueueSendFromISR(raw_gpio_queue_, &edge, &higherPriorityTaskWoken);

    if (higherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void ButtonDriver::debounceTask(void* arg) {
    RawEdgeEvent edge;
    int64_t press_start[3] = {0, 0, 0};
    constexpr int64_t DEBOUNCE_US = 35 * 1000; // 35 ms
    constexpr int64_t LONG_PRESS_US = 600 * 1000; // 600 ms

    while (true) {
        if (xQueueReceive(raw_gpio_queue_, edge, portMAX_DELAY)) {
            uint8_t id_index = static_cast<uint8_t>(edge.id);
            if (edge.isPressed) {
                if (press_start[id_index] == 0) {
                    press_start[id_index] = edge.timestamp;
                }
            } else {
                if (press_start[id_index] != 0) {
                    int64_t duration = edge.timestamp - press_start[id_index];
                    press_start[id_index] = 0;

                    if (duration >= DEBOUNCE_US) {
                        ButtonMessage msg = {
                            .id = edge.id,
                            .event = (duration >= LONG_PRESS_US) ? ButtonEvent::LONG_PRESS : ButtonEvent::SHORT_PRESS
                        };
                        xQueueSend(app_event_queue_, &msg, 0);
                        ESP_LOGD(TAG, "Btn %d %s (%lld ms)", id_index, msg.event == ButtonEvent::LONG_PRESS ? "LONG" : "SHORT", duration / 1000);
                    }
                }
            }
        }
    }
}
