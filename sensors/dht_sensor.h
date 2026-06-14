#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <DHT.h>
#include "../config/pins.h"

#define DHTTYPE DHT11

DHT dht1(DHT_PIN_1, DHTTYPE);
DHT dht2(DHT_PIN_2, DHTTYPE);

void initDHT() {
    dht1.begin();
    dht2.begin();
}

float readTemperature(int sensorIndex) {
    if (sensorIndex == 1) return dht1.readTemperature();
    return dht2.readTemperature();
}

float readHumidity(int sensorIndex) {
    if (sensorIndex == 1) return dht1.readHumidity();
    return dht2.readHumidity();
}

#endif