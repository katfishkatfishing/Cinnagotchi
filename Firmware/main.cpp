#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

static const char * TAG = "main";

static constexpr ST7789DMA::Pins LCD_PINS = {
    .mosi =
    .sclk =
    .cs = 
    .dc =
    .rst =
    .bl =
};

static void blitSprite(uint16_t* )