#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "../config/pins.h"
#include <Arduino.h>

int readLightLevel() {
    long sum = 0;
    const int numSamples = 20;
    for (int i = 0; i < numSamples; i++) {
        sum += analogRead(LIGHT_SENSOR_PIN);
        delay(5); // Đợi 5ms giữa các lần đọc để lấy mẫu đều hơn
    }
    return sum / numSamples;
}

#endif