#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#include <Arduino.h>

int readWaterLevel(int pin) {
    long sum = 0;
    const int numSamples = 20;
    for (int i = 0; i < numSamples; i++) {
        sum += analogRead(pin);
        delay(5);
    }
    return sum / numSamples;
}

#endif