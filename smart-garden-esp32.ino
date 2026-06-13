#include <WiFi.h>
#include <PubSubClient.h>

#include "config/wifi.h"
#include "config/mqtt.h"
#include "models/SensorData.h"

#include "sensors/dht_sensor.h"
#include "sensors/soil_sensor.h"
#include "sensors/water_sensor.h"
#include "sensors/light_sensor.h"

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_INTERVAL 300000  // Tần suất gửi dữ liệu (5p gui 1 lan)

// Hàm kết nối Wi-Fi sử dụng thông tin cấu hình từ config/wifi.h
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Dang ket noi den Wi-Fi: ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Wi-Fi da ket noi thanh cong!");
    Serial.print("Dia chi IP cua ESP32: ");
    Serial.println(WiFi.localIP());
}

// Hàm kết nối và tự động kết nối lại MQTT Broker kèm tài khoản từ config/mqtt.h
void reconnect() {
    while (!client.connected()) {
        Serial.print("Dang ket noi MQTT Broker...");
        
        // 1. Lấy địa chỉ MAC của ESP32
        String mac = WiFi.macAddress();
        String clientId = "ESP32Client-" + mac;
        
        // 2. Chuyển đổi MAC thành DeviceID (loại bỏ dấu ":") để trùng với DB ở backend
        mac.replace(":", "");
        
        // 3. Tạo topic trạng thái cho thiết bị này
        String statusTopic = "smartgarden/devices/" + mac + "/status";
        
        // 4. Kết nối tới Broker kèm theo cấu hình Last Will & Testament (LWT)
        // Cú pháp connect: client.connect(clientId, username, password, willTopic, willQos, willRetain, willMessage)
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS, statusTopic.c_str(), 1, true, "offline")) {
            Serial.println("da ket noi thanh cong!");
            
            // 5. Gửi trạng thái "online" ngay sau khi kết nối thành công (thiết lập retain = true)
            client.publish(statusTopic.c_str(), "online", true);
        } else {
            Serial.print("that bai, ma loi rc=");
            Serial.print(client.state());
            Serial.println(". Thu lai sau 5 giay...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    // Khởi tạo cảm biến nhiệt độ & độ ẩm không khí DHT
    initDHT();
    
    // Kết nối Wi-Fi
    setup_wifi();
    
    // Thiết lập kết nối MQTT
    client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
    // Luôn giữ kết nối tới MQTT Broker
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > MSG_INTERVAL) {
        lastMsg = now;

        // 1. Đọc dữ liệu từ các cảm biến thực tế/giả lập
        SensorData data;
        data.temperature = readTemperature();
        data.humidity = readHumidity();
        data.soilMoisture = readSoilMoisture();
        data.waterLevel = readWaterLevel();
        data.lightLevel = readLightLevel();

        // 2. Lấy địa chỉ MAC của ESP32 làm Device ID và loại bỏ các dấu ':'
        String macAddress = WiFi.macAddress();
        macAddress.replace(":", ""); 
        
        // Topic gửi đi: smartgarden/devices/240AC48899AA/data
        String topic = "smartgarden/devices/" + macAddress + "/data";

        // 3. Đóng gói các giá trị cảm biến thành định dạng JSON
        String payload = "{";
        payload += "\"temperature\":" + String(data.temperature, 1) + ",";
        payload += "\"humidity\":" + String(data.humidity, 1) + ",";
        payload += "\"soilMoisture\":" + String(data.soilMoisture) + ",";
        payload += "\"lightLevel\":" + String(data.lightLevel);
        payload += "}";

        // 4. Gửi dữ liệu lên MQTT Broker
        Serial.print("Publish tin nhan den topic: ");
        Serial.println(topic);
        Serial.print("Payload: ");
        Serial.println(payload);

        client.publish(topic.c_str(), payload.c_str());
        Serial.println("-----------------------------");
    }
}