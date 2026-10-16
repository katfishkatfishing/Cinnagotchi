#pragma once

#include <cstdint>
#include "driver/gpio.h"
#include "driver/ledc.h"

class Buzzer {
    public:
        static constexpr gpio_num_t BUZZER_PIN = GPIO_NUM_5;
        static constexpr ledc_timer_t TIMER_NUM = LEDC_TIMER_0;
        static constexpr ledc_channel_t CHANNEL = LEDC_CHANNEL_0;

        static void init();
        static void playTone(uint32_t frequency, uint32_t duration);
        static void chirpHappy();
        static void chirpSad();
        static void click();
};