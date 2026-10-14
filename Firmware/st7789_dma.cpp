#include "st7789_dma.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>

static const char * TAG = "st7789_dma";

bool ST7789DMA::init(const Pins& pins, spi_host_device_t host) {
    pins_ = pins;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pins_.dc) | (1ULL << pins_.rst) | (1ULL << pins_.bl),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    gpio_set_level(pins_.rst, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(pins_.rst, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    spi_bus_config_t buscfg = {
        .mosi_io_num = pins_.mosi,
        .miso_io_num = -1,
        .sclk_io_num = pins_.sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SCREEN_WIDTH * TITLE_HEIGHT * sizeof(uint16_t) + 16,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO));

    bufferA = static_cast<uint16_t*>(heap_caps_malloc(SCREEN_WIDTH * TILE_HEIGHT * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    bufferB = static_cast<uint16_t*>(heap_caps_malloc(SCREEN_WIDTH * TILE_HEIGHT * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    assert(bufferA && bufferB);

    sendCommand(0x01); // Software reset
    vTaskDelay(pdMS_TO_TICKS(150));
    sendCommand(0x11); // Sleep out
    vTaskDelay(pdMS_TO_TICKS(120));

    uint8_t colorMode[] = {0x55}; // 16-bit color
    sendCommand(0x3A); // COLMOD: Pixel Format Set
    sendData(colorMode, 1);

    uint8_t madctl[] = {0x00}; //rgb order
    sendCommand(0x36); // MADCTL: Memory Data Access Control
    sendData(madctl, 1);

    sendCommand(0x21); // Inversion on
    sendCommand(0x29); // Display on

    gpio_set_level(pins_.bl, 1); // Turn on backlight
    ESP_LOGI(TAG, "ST7789 initialized successfully");
    return true;
}

void ST7789DMA::sendCommand(uint8_t cmd) {
    gpio_set_level(pins_.dc, 0);
    spi_transaction_t t{};
    t.length = 8;
    t.txbuffer = &cmd;
    spi_device_polling_transmit(spi_, &t);
}

void ST7789DMA::sendData(uint8_t* data, size_t length) {
    gpio_set_level(pins_.dc, 1);
    spi_transaction_t t{};
    t.length = length * 8;
    t.txbuffer = data;
    spi_device_polling_transmit(spi_, &t);

    void ST7789DMA::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
        uint8_t ca[4] = {static_cast<uint8_t>(x0 >> 8), static_cast<uint8_t>(x0 & 0xFF), static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1 & 0xFF)};
        uint8_t ra[4] = {static_cast<uint8_t>(y0 >> 8), static_cast<uint8_t>(y0 0xFF), static_cast<uint8_t(y1 >> 8), static_cast<uint8_t>(y1 & 0xFF)};

        sendCommand(0x2A); // Column Address Set
        sendData(ca, 4);
        sendCommand(0x2B); // Row Address Set
        sendData(ra, 4);
        sendCommand(0x2C); // Memory Write
    }

    void ST7789DMA::flushTileAsync(const uint16_t* buffer, int numPixels) {
        gpio_set_level(pins_.dc, 1);
        std::memset(&transaction_, 0, sizeof(transaction_));
        trans_.tx_buffer = buffer;
        ESP_ERROR_CHECK(spi-device_queue_trans(spi_, &transaction_, portMAX_DELAY));
    }

    void ST7789DMA::waitTransferComplete() {
        spi_transaction_t* completedTransaction;
        ESP_ERROR_CHECK(spi_device_get_trans_result(spi_, &completedTransaction, portMAX_DELAY));
    }
}