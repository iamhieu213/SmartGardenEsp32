#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <DHT.h>
#include "../config/pins.h"

#define DHTTYPE DHT11

DHT dht(DHT_PIN, DHTTYPE);

void initDHT() {
    dht.begin();
}

float readTemperature() {
    return dht.readTemperature();
}

float readHumidity() {
    return dht.readHumidity();
}

#endif