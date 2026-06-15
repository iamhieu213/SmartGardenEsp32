#include <WiFi.h>
#include <PubSubClient.h>

#include "config/wifi.h"
#include "config/mqtt.h"

#include "sensors/dht_sensor.h"
#include "sensors/soil_sensor.h"
#include "sensors/water_sensor.h"
#include "sensors/light_sensor.h"

// Biến cấu hình để bật/tắt nhanh các cảm biến Analog phụ trong code 
// (vì chân Analog để trống sẽ trả về số ngẫu nhiên)
#define HAS_SOIL_2 false  // Đổi thành true khi cắm thêm cảm biến đất 2
#define HAS_WATER_2 false // Đổi thành true khi cắm thêm cảm biến nước 2

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_INTERVAL 300000 

void setup() {
    Serial.begin(115200);
    initDHT();
    initLightSensors();
    setup_wifi();
    client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
    if (!client.connected()) { reconnect(); }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > MSG_INTERVAL) {
        lastMsg = now;

        // Bắt đầu chuỗi JSON
        String payload = "{";
        bool firstField = true;

        // ---- ĐỌC & CHECK CẢM BIẾN NHIỆT ĐỘ / ĐỘ ẨM KHÍ 1 ----
        float t1 = readTemperature(1);
        float h1 = readHumidity(1);
        if (!isnan(t1) && !isnan(h1)) {
            payload += "\"temperature1\":" + String(t1, 1) + ",\"humidity1\":" + String(h1, 1);
            firstField = false;
        }

        // ---- ĐỌC & CHECK CẢM BIẾN NHIỆT ĐỘ / ĐỘ ẨM KHÍ 2 ----
        float t2 = readTemperature(2);
        float h2 = readHumidity(2);
        if (!isnan(t2) && !isnan(h2)) {
            if (!firstField) payload += ",";
            payload += "\"temperature2\":" + String(t2, 1) + ",\"humidity2\":" + String(h2, 1);
            firstField = false;
        }

        // ---- ĐỌC & CHECK CẢM BIẾN ĐẤT 1 ----
        int soil1 = readSoilMoisture(SOIL_SENSOR_PIN_1);
        if (soil1 > 0 && soil1 < 4095) { // Kiểm tra nếu cảm biến hoạt động bình thường
            if (!firstField) payload += ",";
            payload += "\"soilMoisture1\":" + String(soil1);
            firstField = false;
        }

        // ---- ĐỌC & CHECK CẢM BIẾN ĐẤT 2 ----
        if (HAS_SOIL_2) {
            int soil2 = readSoilMoisture(SOIL_SENSOR_PIN_2);
            if (!firstField) payload += ",";
            payload += "\"soilMoisture2\":" + String(soil2);
            firstField = false;
        }

        // ---- ĐỌC & CHECK CẢM BIẾN NƯỚC 1 ----
        int water1 = readWaterLevel(WATER_SENSOR_PIN_1);
        if (water1 > 0) {
            if (!firstField) payload += ",";
            payload += "\"waterLevel1\":" + String(water1);
            firstField = false;
        }

        // ---- ĐỌC & CHECK CẢM BIẾN NƯỚC 2 ----
        if (HAS_WATER_2) {
            int water2 = readWaterLevel(WATER_SENSOR_PIN_2);
            if (!firstField) payload += ",";
            payload += "\"waterLevel2\":" + String(water2);
            firstField = false;
        }

        // ---- ĐỌC & CHECK CẢM BIẾN ÁNH SÁNG 1 ----
        float light1 = readLightLevel(1);
        if (light1 >= 0) {
            if (!firstField) payload += ",";
            payload += "\"lightIntensity1\":" + String(light1, 1);
            firstField = false;
        }

        // ---- ĐỌC & CHECK CẢM BIẾN ÁNH SÁNG 2 ----
        float light2 = readLightLevel(2);
        if (light2 >= 0) {
            if (!firstField) payload += ",";
            payload += "\"lightIntensity2\":" + String(light2, 1);
            firstField = false;
        }

        payload += "}";

        String macAddress = WiFi.macAddress();
        macAddress.replace(":", ""); 
        String topic = "smartgarden/devices/" + macAddress + "/data";
        
        client.publish(topic.c_str(), payload.c_str());
    }
}