#pragma once 
#include <cstdint>
#include "driver/spi_master.h"
#include "driver/gpio.h"

class ST7789DMA {
    public:
        static constexpr int SCREEN_WIDTH = 240;
        static constexpr int SCREEN_HEIGHT = 240;
        static constexpr int TILE_HEIGHT = 20;
        static constexpr uint16_t COLOR_KEY = 0x07E0;

        struct Pins {
            gpio_num_t mosi;
            gpio_num_t sclk;
            gpio_num_t cs;
            gpio_num_t dc;
            gpio_num_t rst;
            gpio_num_t bl;
        };

        bool init(const Pins& pins, spi_host_device_t host = SPI2_HOST);
        void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
        void flushTileAsync(uint16_t* buffer, int numPixels);
        void waitTransferComplete();

        uint16_t* getBufferA() { return bufferA;}
        uint16_t* getBufferB() { return bufferB;}

    private:
        void sendCommand(uint8_t cmd);
        void sendData(uint8_t* data, size_t length);

        spi_device_handle_t spi_{nullptr};
        Pins pins_{};
        spi_transaction_t transaction_{};

        uint16_t* bufferA{nullptr};
        uint16_t* bufferB{nullptr};
};

