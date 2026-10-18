#pragma once

#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_timer.h"

class Standby {
    public:
        static constexpr gpio_num_t PIN_BL = GPIO_NUM_7;
        static constexpr int64_t DIM_TIMEOUT_US = 30ULL * 1000 * 1000;
        static constexpr int64_t SLEEP_TIMEOUT_US = 90ULL * 1000 * 1000;

        static void init() {
            ledc_timer_config_t t_config = {
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .duty_RESOLUTION = LEDC_TIMER_8_BIT,
                .timer_num = LEDC_TIMER_1,
                .feq_hz = 5000,
                .clk_cfg = LEDC_AUTO_CLK
            };
            ledc_timer_config(&t_config);

            ledc_channel_config_t c_config = {
                .gpio_num = PIN_BL,
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .channel = LEDC_CHANNEL_1,
                .intr_type = LEDC_INTR_DISABLE,
                .timer_sel = LEDC_TIMER_1,
                .duty = 255,
                .hpoint = 0
            };
            ledc_channel_config(&c_config);
            poke();
        }

        static void poke() {
            last_action_ = esp_timer_get_time();
            if (state_ != Mode::ACTIVE) {
                setBrightness(100);
                state_ = Mode::ACTIVE;
            }
        }

        static void update(float ambientLux) {
            int64_t idle = esp_timer_get_time() - last_action_;

            if (ambientLux >= 0.0f && ambientLux < 3.0f && state_ != Mode::DARK_SLEEP) {
                setBrightness(0);
                state_ = Mode::DIMMED;
            } else if (state_ == Mode::DIMMED && idle > SLEEP_TIMEOUT_US) {
                setBrightness(5);
                state_ = Mode::NIGHTLIGHT;
            }
        }

        static bool isNightMode() {
            return state_ == Mode::NIGHTLIGHT || state_ == Mode::DARK_SLEEP;
        }

        private:
        enum class Mode {
            ACTIVE,
            DIMMED,
            NIGHTLIGHT,
            DARK_SLEEP
        };

        static inline Mode state_{Mode::ACTIVE};
        static inline int64_t last_action_{0};

        static void setBrightness(uint8_t percent) {
            uint32_t duty = (percent * 255) / 100;
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        }
};