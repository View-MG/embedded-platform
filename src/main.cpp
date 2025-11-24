#include <Arduino.h>
#include <time.h>
#include "config_service.h"
#include "sensor_service.h"
#include "motor_control.h"
#include "log_service.h"

// Services ที่ใช้
ConfigService config;
SensorService sensor;
MotorControl motor;
LogService* logger;

// สำหรับ Log
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 30000; // 30 วินาที

void setup() {
    Serial.begin(115200);

    config.beginWiFi();
    config.beginFirebase();

    motor.begin(&config);
    sensor.begin();

    logger = new LogService(&config);
    logger->begin();

    Serial.println("[System] Ready");
}

void loop() {
    float temperature = sensor.getTemperature();
    float humidity    = sensor.getHumidity();
    float water       = sensor.getWaterLevel();
    bool tilted       = (sensor.getTiltDegree() > 0);
    bool button       = sensor.getButton();

    // ควบคุมมอเตอร์ (ใช้ข้อมูล tilt เพื่อ safety)
    motor.update(true);

    // Log ทุก 30 วินาที
    if (millis() - lastLogTime > LOG_INTERVAL) {
        logger->writeLog(temperature, humidity, water, sensor.getTiltDegree(), button);
        lastLogTime = millis();
    }

    delay(500);
}
