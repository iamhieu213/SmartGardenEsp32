#include <WiFi.h>
#include <PubSubClient.h>

#include "config/wifi.h"
#include "config/mqtt.h"
#include "config/pins.h"

#include "sensors/dht_sensor.h"
#include "sensors/soil_sensor.h"
#include "sensors/water_sensor.h"
#include "sensors/light_sensor.h"

// Cấu hình bật/tắt nhanh cảm biến phụ 2
#define HAS_SOIL_2 false

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_INTERVAL 10000 // Gửi cảm biến định kỳ mỗi 5 phút (300000 ms)

// --- HÀM QUY ĐỔI ĐỘ ẨM ĐẤT SANG PHẦN TRĂM (%) ---
int getSoilMoisturePercent(int analogVal) {
    // Bạn đo thực tế cảm biến đất của bạn và điền vào đây:
    const int dryValue = 3200; // Giá trị khi cảm biến khô hoàn toàn ngoài không khí
    const int wetValue = 1200; // Giá trị khi cắm cảm biến ngập vào nước
    
    int percent = map(analogVal, dryValue, wetValue, 0, 100);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return percent;
}

// --- THIẾT LẬP KẾT NỐI WIFI ---
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// --- CALLBACK NHẬN LỆNH ĐIỀU KHIỂN TỪ WEB (MQTT SUBSCRIBER) ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Nhan lenh tu server [");
    Serial.print(topic);
    Serial.print("]: ");
    
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println(message);

    // Kiểm tra mực nước an toàn trước khi cho phép bật bơm
    int waterVal = readWaterLevel(WATER_SENSOR_PIN_1);
    if (waterVal < 200) { // Nếu bể cạn nước (< 5mm)
        Serial.println("CẢNH BÁO: Không thể bật bơm vì bể đang cạn nước!");
        digitalWrite(PUMP_PIN, LOW); // Luôn ngắt bơm để bảo vệ
        return;
    }

    // Thực hiện bật/tắt bơm từ nút nhấn trên Web gửi xuống
    if (message.indexOf("\"pump\":1") >= 0) {
        digitalWrite(PUMP_PIN, HIGH);
        Serial.println("Đã BẬT máy bơm.");
    } 
    else if (message.indexOf("\"pump\":0") >= 0) {
        digitalWrite(PUMP_PIN, LOW);
        Serial.println("Đã TẮT máy bơm.");
    }
}

// --- DUY TRÌ KẾT NỐI MQTT ---
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-" + String(random(0xffff), HEX);
        
        String mac = WiFi.macAddress();
        mac.replace(":", "");
        String statusTopic = "smartgarden/devices/" + mac + "/status";
        String controlTopic = "smartgarden/devices/" + mac + "/control";
        
        // Kết nối với LWT (Last Will & Testament)
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS, statusTopic.c_str(), 1, true, "offline")) {
            Serial.println("connected!");
            
            // Gửi tin nhắn thông báo đang online
            client.publish(statusTopic.c_str(), "online", true);
            
            client.subscribe(controlTopic.c_str());
            Serial.println("Subscribed to: " + controlTopic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

// --- LOGIC BẢO VỆ MÁY BƠM ---
// void autoIrrigationControl() {
//     int waterVal = readWaterLevel(WATER_SENSOR_PIN_1);

//     // 1. Kiểm tra an toàn mực nước (Cạn nước thì tắt ngay lập tức để bảo vệ phần cứng)
//     if (waterVal < 200) { 
//         if (digitalRead(PUMP_PIN) == HIGH) {
//             digitalWrite(PUMP_PIN, LOW);
//             Serial.println("TỰ ĐỘNG NGẮT: Bể hết nước! Ngắt bơm chống cháy.");
//         }
//         return; 
//     }
// }

void setup() {
    Serial.begin(115200);
    
    // Cấu hình chân rơ-le bơm làm ngõ ra
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW); // Mặc định tắt máy bơm khi bắt đầu chạy
    
    initDHT();
    initLightSensors();
    setup_wifi();
    
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback); // Đăng ký hàm xử lý lệnh
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Chạy logic tự động kiểm tra và điều khiển bơm liên tục
    // autoIrrigationControl();

    // Gửi dữ liệu các cảm biến lên Server định kỳ
    unsigned long now = millis();
    if (now - lastMsg > MSG_INTERVAL) {
        lastMsg = now;

        String payload = "{";
        bool firstField = true;

        // ---- DHT 1 ----
        float t1 = readTemperature(1);
        float h1 = readHumidity(1);
        if (!isnan(t1) && !isnan(h1)) {
            payload += "\"temperature1\":" + String(t1, 1) + ",\"humidity1\":" + String(h1, 1);
            firstField = false;
        }

        // ---- DHT 2 ----
        float t2 = readTemperature(2);
        float h2 = readHumidity(2);
        if (!isnan(t2) && !isnan(h2)) {
            if (!firstField) payload += ",";
            payload += "\"temperature2\":" + String(t2, 1) + ",\"humidity2\":" + String(h2, 1);
            firstField = false;
        }

        // ---- ĐẤT 1 ----
        int soil1 = readSoilMoisture(SOIL_SENSOR_PIN_1);
        if (soil1 > 0 && soil1 < 4095) {
            if (!firstField) payload += ",";
            payload += "\"soilMoisture1\":" + String(soil1);
            firstField = false;
        }

        // ---- ĐẤT 2 ----
        if (HAS_SOIL_2) {
            int soil2 = readSoilMoisture(SOIL_SENSOR_PIN_2);
            if (!firstField) payload += ",";
            payload += "\"soilMoisture2\":" + String(soil2);
            firstField = false;
        }

        // ---- NƯỚC 1 (Gửi ngầm lên để Server lưu trữ) ----
        int water1 = readWaterLevel(WATER_SENSOR_PIN_1);
        if (water1 > 0) {
            if (!firstField) payload += ",";
            payload += "\"waterLevel1\":" + String(water1);
            firstField = false;
        }

        // ---- ÁNH SÁNG 1 ----
        float light1 = readLightLevel(1);
        if (light1 >= 0) {
            if (!firstField) payload += ",";
            payload += "\"lightIntensity1\":" + String(light1, 1);
            firstField = false;
        }

        // ---- ÁNH SÁNG 2 ----
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
        Serial.println("Da gui payload: " + payload);
    }
}