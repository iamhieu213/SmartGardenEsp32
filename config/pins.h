#ifndef PINS_H
#define PINS_H

// 2 Cảm biến Nhiệt độ / Độ ẩm DHT
#define DHT_PIN_1 4
#define DHT_PIN_2 18 

// 2 Cảm biến Độ ẩm đất (Analog ADC1)
#define SOIL_SENSOR_PIN_1 34
#define SOIL_SENSOR_PIN_2 33 

// 1 Cảm biến Mực nước (Analog ADC1)
#define WATER_SENSOR_PIN_1 35 

// Chân I2C cho cảm biến ánh sáng BH1750
#define I2C_SDA 21
#define I2C_SCL 22

#define PUMP_PIN 25 // Chân GPIO 25 trên ESP32 để kích Relay bật/tắt bơm


#endif