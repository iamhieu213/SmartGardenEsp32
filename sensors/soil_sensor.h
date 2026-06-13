#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include "../config/pins.h"
#include <Arduino.h>

int readSoilMoisture() {
    long sum = 0;
    const int numSamples = 20;
    for (int i = 0; i < numSamples; i++) {
        sum += analogRead(SOIL_SENSOR_PIN);
        delay(5); // Đợi 5ms giữa các lần đọc để lấy mẫu đều hơn
    }
    return sum / numSamples;
}

#endif