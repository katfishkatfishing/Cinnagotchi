#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"
#include "st7789_dma.hpp"
#include "vitals.hpp"
#include "pet_fsm.hpp"

static const char * TAG = "main";

static constexpr ST7789DMA::Pins LCD_PINS = {
    .mosi =
    .sclk =
    .cs = 
    .dc =
    .rst =
    .bl =
};

static ST7789DMA display;
static Vitals vitals;
static PetFSM petFsm;

static void blitSprite(uint16_t* destTile, int tileY, int sprX, int sprY, int sprW, int sprH, const uint16_t* sprData) {
    for (int y = 0; y < sprH; ++y) {
        int globalY = sprY + y;
        if (globalY >= tileY && globalY < (tileY + ST7789DMA::TILE_HEIGHT)) {
            int localY = globalY - tileY;
            for (int x = 0; x < sprW; ++x) {
                int globalX = sprX + x;
                if (globalX >= 0 && globalX < ST7789DMA::SCREEN_WIDTH) {
                    uint16_t pixel = sprData[y * sprW + x];
                    if (pixel != ST7789DMA::COLOR_KEY) {
                        destTile[localY * ST7789DMA::SCREEN_WIDTH + globalX] = pixel;
                    }
                }
            }
        }
    }
}

void renderTask(void* pvParameters) {
    display.init(LCD_PINS);
    uint16_t* bufferA = display.getBufferA();
    uint16_t* bufferB = display.getBufferB();

    const uint16_t* BG_COLOR = (0xF75B << 8) | (0xF75B >> 8); // Example background color

    int animationFrame = 0;

    while (true) {
        display.setAddressWindow(0, 0, ST7789DMA::SCREEN_WIDTH - 1, ST7789DMA::TILE_HEIGHT - 1);

        for (int tile = 0; tile < (ST7789DMA::SCREEN_WIDTH - 1, ST7789DMA::TILE_HEIGHT - 1); ++tile) {
            int tileY = tile * ST7789DMA::TILE_HEIGHT;

            for (int i = 0; i < ST7789DMA::SCREEN_WIDTH * ST7789DMA::TILE_HEIGHT; ++i) {
                bufferB[i] = BG_COLOR;
            }

            int bounceY = (petFsm.getState() == PetState::happy) ? ((animationFrame % 20 < 10)? -4 : 0) : 0;

            static const uint16_t BODY_WHITE = (0xFFFF << 8) | (0xFFF >> 8);
            
            for (int y = 0; y <40; ++y) {
                int globalY = 100 + bounceY + y;
                if (globalY >= tileY && globalY < tileY + ST7789DMA::TILE_HEIGHT) {
                    int localY = globalY - tileY;
                    for (int x = 80; x < 160; ++x) {
                        bufferB[localY * ST7789DMA::SCREEN_WIDTH + x] = BODY_WHITE;
                    }
                }
            }

            if (tile > 0) {
                display.waitTransferComplete();
            }
            display.flushTileAsync(bufferB, ST7789DMA::SCREEN_WIDTH * ST7789DMA::TILE_HEIGHT);

            std::swap(bufferA, bufferB);
        }
        display.waitTransferComplete();
        animationFrame++;
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

void gameTask(void* pvParameters) {
    while (true) {
        petFsm.update(vitals);
        ESP_LOG(TAG, "Pet State: %d, | Hunger: %d, Happiness: %d", static_cast<int>(petFsm.getState()), vitals.hunger, vitals.happiness);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "Booting Cinnagotchi...");

    xTaskCreatePinnedToCore(renderTask, "Render Task", 4096, nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(gameTask, "Game Task", 4096, nullptr, 2, nullptr, 0);
}