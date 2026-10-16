#pragma once

#include <cstdint>
#include "driver/gpio.h"
#include "freeRtos/FreeRTOS.h"
#include "freertos/queue.h"

enum class ButtonId : uint8_t {
    BTN_SCROLL = 0,
    BTN_SELECT = 1,
    BTN_BACK = 2,
};

enum class ButtonEvent : uint8_t {
    SHORT_PRESS,
    LONG_PRESS,
};

struct ButtonMessage {
    ButtonId id;
    ButtonEvent event;
};

class ButtonDriver {
    public:
        static constexpr gpio_num_t BTN_SCROLL_PIN = GPIO_NUM_12;
        static constexpr gpio_num_t BTN_SELECT_PIN = GPIO_NUM_13;
        static constexpr gpio_num_t BTN_BACK_PIN = GPIO_NUM_14;
    
    static bool init(QueueHandle_t outEventQueue);

    private:
        static void IRAM_ATTR isrHandler(void* arg);
        static QueueHandle_t raw_qpio_queue_;
        static QueueHandle_t app_event_queue_;
};