#include "buzzer.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void Buzzer::init() {
    ledc_timer_config_t timer_config = {
        .spped_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = BUZZER_TIMER,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = buzzer_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = BUZZER_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config);
}

void Buzzer::playTone(uint32_t frequency, uint32_t duration) {
    if (frequency == 0) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
    } else {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, BUZZER_TIMER, frequency);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 512); // 50% duty cycle
        ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
    }
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
}

void Buzzer::chirpHappy() {
    play_tone(1046, 60) // C6
    play_tone(1318, 60) // E6
    play_tone(1567, 60) // G6
    play_tone(2093, 60) // C7
}

void Buzzer::chirpSad() {
    playTone(880, 80); // A5
    playTone(784, 80); // G5
    playTone(659, 120); // E5
}

void Buzzer::click() {
    playTone(2400, 15);
}


