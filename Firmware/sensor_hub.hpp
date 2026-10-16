#pragma once

#include <cstdint>
#include "driver/i2c.h"

class SensorHub {
public:
    static constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
    static constexpr gpio_num_t SDA_PIN = GPIO_NUM_21;
    static constexpr gpio_num_t SCL_PIN = GPIO_NUM_22;

    static constexpr uint8_t ADDR_BH1750 = 0x23;
    static constexpr uint8_t ADDR_MPU6050 = 0x68;

    struct MotionData {
        float accelX, accelY, accelZ;
        float shakeMagnitude;
        bool tapDetected;
    };

    static bool init();
    static float readAmbientLux();
    static MotionData readMotionData();

private:
    static esp_err_t writeRegister(uint8_t deviceAddress, uint8_t regAddress, uint8_t data);
    static esp_err_t readRegisters(uint8_t deviceAddress, uint8_t regAddress, uint8_t* outData, size_t length);

    static float last_accel_norm_;
};