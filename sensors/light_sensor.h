#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Wire.h>
#include <BH1750.h>
#include "../config/pins.h"

// Khởi tạo 2 cảm biến BH1750 với 2 địa chỉ I2C khác nhau
BH1750 lightMeter1(0x23); // Chân ADDR nối GND (hoặc để trống)
BH1750 lightMeter2(0x5C); // Chân ADDR nối nguồn 3.3V

void initLightSensors() {
    Wire.begin(I2C_SDA, I2C_SCL); // Khởi động I2C trên 2 chân cấu hình
    lightMeter1.begin();
    lightMeter2.begin();
}

// Trả về mức Lux hoặc trả về -1 nếu cảm biến không phản hồi
float readLightLevel(int sensorIndex) {
    if (sensorIndex == 1) {
        float lux = lightMeter1.readLightLevel();
        return (lux >= 0) ? lux : -1;
    } else {
        float lux = lightMeter2.readLightLevel();
        return (lux >= 0) ? lux : -1;
    }
}

#endif