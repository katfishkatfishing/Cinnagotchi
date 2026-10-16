#include "sensor_hub.hpp"
#include <cmath>
#include "esp_log.h"

static const char* TAG = "Sensors";
float SensorHub::last_accel_norm_ = 1.0f;

esp_err_t SensorHub::writeRegister(uint8_t deviceAddress, uint8_t regAddress, uint8_t data) {
    uint8_t packet[2] = {regAddress, data};
    return i2c_master_write_to_device(I2C_PORT, deviceAddress, packet, sizeof(packet), pdMS_TO_TICKS(1000));
}

esp_err_t SensorHub::readRegisters(uint8_t deviceAddress, uint8_t regAddress, uint8_t* outData, size_t length) {
    return i2c_master_write_read_device(I2C_PORT, deviceAddress, &regAddress, 1, outData, length, pdMS_TO_TICKS(1000));
}

bool SensorHub::init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = { .clk_speed = 400000 },
        .clk_flags = 0
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));
    
    if (writeRegister(ADDR_MPU6050, 0x6B, 0x01) != ESP_OK) {
        ESP_LOGE(TAG, "MPU-6050 initialisation failed");
        return false;
    }

    writeRegister(ADDR_MPU6050, 0x1C, 0x08); // Set accelerometer range to ±4g

    uint8_t bh1750_init_cmd = 0x10; // Continuously H-Resolution Mode
    i2c_master_write_to_device(I2C_PORT, ADDR_BH1750, &bh1750_init_cmd, 1, pdMS_TO_TICKS(50));
    bh_cmd = 0x10; // Continuously H-Resolution Mode
    i2c_master_write_to_device(I2C_PORT, ADDR_BH1750, &bh_cmd, 1, pdMS_TO_TICKS(50));

    ESP_LOGI(TAG, "Sensor Hub I2C bus initialised at 400 kHz");
    return true;
}

float SensorHub::readAmbientLux() {
    uint8_t raw[2];
    if (i2c_master_read_from_device(I2C_PORT, ADDR_BH1750, raw, 2, pdMS_TO_TICKS(30)) == ESP_OK) {
        uint16_t rawLux = (raw[0] << 8) | raw[1];
        return static_cast<float>(rawLux) / 1.2f; // Convert to lux
    }
    return -1.0f;
}

SensorHub::MotionData SensorHub::readMotionData() {
    uint8_t buffer[6];
    MotionData out = {0, 0, 0, 0, false};

    if (readRegisters(ADDR_MPU6050, 0x38, buffer, 6) == ESP_OK) {
        int16_t rawAccelX = (buffer[0] << 8) | buffer[1];
        int16_t rawAccelY = (buffer[2] << 8) | buffer[3];
        int16_t rawAccelZ = (buffer[4] << 8) | buffer[5];

        // Convert raw accelerometer data to g's
        out.accelX = static_cast<float>(rawAccelX) / 8192.0f; // ±4g range
        out.accelY = static_cast<float>(rawAccelY) / 8192.0f;
        out.accelZ = static_cast<float>(rawAccelZ) / 8192.0f;

        float accel_norm = std::sqrt(out.accelX * out.accelX + out.accelY * out.accelY + out.accelZ * out.accelZ);
        out.shakeMagnitude = std::abs(accel_norm - last_accel_norm_);
        last_accel_norm_ = accel_norm;

        if (out.shakeMagnitude > 0.65f) {
            out.tapDetected = true;
        }
    }
    return out;
}

