#include <Arduino.h>
#include <time.h>
#include "config_service.h"
#include "sensor_service.h"
#include "motor_control.h"
#include "log_service.h"
#include "audio_service.h"

// Services
ConfigService config;
SensorService sensor;
MotorControl motor;
LogService* logger;
AudioService audio;

// --- ตัวแปรสำหรับจับเวลา (Non-blocking) ---
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 30000; // 30 วินาที (Log Firebase)

unsigned long lastSensorTime = 0;
const unsigned long SENSOR_INTERVAL = 500; // 0.5 วินาที (อ่าน Sensor/คุม Motor)

void setup() {
    Serial.begin(115200);

    // 1. Init System
    config.beginWiFi();
    config.beginFirebase();

    motor.begin(&config);
    sensor.begin();

    // 2. Init Audio (ถ้าเปิด)
    if (ENABLE_AUDIO_STREAM) {
        audio.begin();
    }

    // 3. Init Logger
    logger = new LogService(&config);
    logger->begin();

    Serial.println("[System] Ready");
}

void loop() {
    // -----------------------------------------------------------
    // PRIORITY 1: Audio Service (ต้องทำงานทุกรอบ ห้ามรอ!)
    // -----------------------------------------------------------
    if (ENABLE_AUDIO_STREAM) {
        audio.loop(); 
    }

    // -----------------------------------------------------------
    // PRIORITY 2: Sensor & Motor Control (ทำทุกๆ 500ms)
    // -----------------------------------------------------------
    if (millis() - lastSensorTime > SENSOR_INTERVAL) {
        lastSensorTime = millis(); // รีเซ็ตเวลา

        // อ่านค่า Sensor
        float temperature = sensor.getTemperature();
        float humidity    = sensor.getHumidity();
        float water       = sensor.getWaterLevel();
        bool tilted       = (sensor.getTiltDegree() > 0);
        bool button       = sensor.getButton();

        // ควบคุม Motor
        motor.update(true); 

        // (Optional) ปริ้นดูสถานะได้ตรงนี้ ถ้าอยาก Debug
        // Serial.printf("T:%.1f H:%.1f W:%.1f Tilt:%d\n", temperature, humidity, water, tilted);
    }

    // -----------------------------------------------------------
    // PRIORITY 3: Logging (ทำทุกๆ 30 วินาที)
    // -----------------------------------------------------------
    if (millis() - lastLogTime > LOG_INTERVAL) {
        lastLogTime = millis();
        
        // อ่านค่าล่าสุดอีกทีเพื่อให้ Log ตรงปัจจุบันที่สุด
        logger->writeLog(
            sensor.getTemperature(), 
            sensor.getHumidity(), 
            sensor.getWaterLevel(), 
            sensor.getTiltDegree(), 
            sensor.getButton()
        );
        Serial.println("[System] Log Sent to Firebase");
    }

    // ❌ ห้ามใส่ delay() ตรงนี้เด็ดขาด! 
    // ถ้าใส่ delay(500) เสียงจะกระตุกและ WS จะหลุดทันที
}