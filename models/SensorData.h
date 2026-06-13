#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

struct SensorData {
    float temperature;
    float humidity;

    int soilMoisture;
    int waterLevel;
    int lightLevel;
};

#endif