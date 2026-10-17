#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"
#include "st7789_dma.hpp"
#include "ui.hpp"
#include "vitals.hpp"
#include "pet_fsm.hpp"
#include "buzzer.hpp"
#include "button_driver.hpp"
#include "sensor_hub.hpp"
#include "storage.hpp"

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
    static constexpr uint16_t COLOR_WHITE = 0xFFFF;
    static constexpr uint16_t COLOR_AMBER = (0xFBE0 << 8) | (0xFBE0 >> 8);
    static constexpr uint16_t COLOR_BAR_BG = (0x4208 << 8) | (0x4208 >> 8);
    static constexpr uint16_t COLOR_BAR_FILL = (0x2DE0 << 8) | (0x2DE0 >> 8);

    void drawUIOverly(uint16_t* buffer, int tileY, const UINavigator& nav, const Vitals& vitals) {
        ViewState view = nav.getCurrentView();

        if (view == ViewState::VIEW_MAIN_MENU) {
            constexpr int BAR_Y = 10;
            constexpr int ICON_SIZE = 24;
            constexpr int SPACING = 55;

            for (int i = 0; i < UINavigator::MENU_ITEMS_COUNT; ++1) {
                int iconX = 20 + i * SPACING;
                uint16_t frameColor = (i == nav.getMenuCursor()) ? COLOR_AMBER : COLOR_WHITE;

                for (int y = 0; y < ICON_SIZE; ++y) {
                    int globalY = BAR_Y + y;

                    if (globalY >= tileY && globalY < tileY + ST7789DMA::TILE_HEIGHT) {
                        int localY = globalY - tileY;
                        tileBuffer[localY * ST7789DMA::SCREEN_WIDTH + iconX] = frameColor;
                        tileBuffer[localY * ST7789DMA::SCREEN_WIDTH + iconX + ICON_SIZE - 1] = frameColor;

                        if (y == 0 || y == ICON_SIZE - 1) {
                            for (int x = 0; x < ICON_SIZE; ++x) {
                                tileBuffer[localY * ST7789DMA::SCREEN_WIDTH + iconX + x] = framColor;
                            }
                        }    
                    }
                }
            }
        }
    

        if (view == ViewState::VIEW_STATS_CARD) {
            constexpr int BAR_START_Y = 60;
            constexpr int BAR_HEIGHT = 12;
            constexpr int BAR_MMAX_WIDTH = 160;
            constexpr int BAR_x = 40;

            uint8_t stats[3] = {vitals.hunger, vitals.happiness, vitals.energy};

            for (int s = 0; s < 3; ++s) {
                int baseY = BAR_START_Y + s * (BAR_HEIGHT + 10);
                int fillWidth = (stats[s] * BAR_MAX_WIDTH) / 100;

                for (int y = 0; y < BAR_HEIGHT; ++y) {
                    int globalY = baseY + y;
                    if (globalY >= tileY && globalY < tileY + ST7789DMA::TILE_HEIGHT) {
                        int localY = globalY - tileY;

                        for (int x = 0; x < BAR_MAX_WIDTH; ++x) {
                            uint16_t color = (x < fillWidth) ? COLOR_BAR_FILL : COLOR_BAR_BG;
                            tileBuffer[localY * ST7789DMA::SCREEN_WIDTH + BAR_X + x] = color;
                        }
                    }
                }
            }
        }
    }

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

static QueueHandle_t buttonEventQueue;
static UINavigator navigator;

void gameTask(void* pvParameters) {
    ButtonMessage btn;

    while (true) {

        while (xQueueReceive(buttonEventQueue, &btn, 0) == pdTRUE) {
            navigator.handleInput(btn, vitals);
        }

        // Check ambient light levels
        float lux = SnesorHub::readAmbientLux();
        if (lux >= 0.0f && lux < 5.0f && navigator.getCurrentView() == ViewState::VIEW_HOME) {
            petFsm.forceState(PetState::sleeping);
        }

        petFsm.update(vitals);
        ESP_LOG(TAG, "Pet State: %d, | Hunger: %d, Happiness: %d", static_cast<int>(petFsm.getState()), vitals.hunger, vitals.happiness);
        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz update rate
    }
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "Booting Cinnagotchi...");

    xTaskCreatePinnedToCore(renderTask, "Render Task", 4096, nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(gameTask, "Game Task", 4096, nullptr, 2, nullptr, 0);
}