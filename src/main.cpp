#include <Arduino.h>
#include "config_service.h"
#include "sensor_service.h" 
#include "motor_control.h"
#include "audio_service.h"
#include "log_service.h"
#include <time.h>
#include "audio_upload_service.h"

ConfigService config;
SensorService sensor;
MotorControl motor;
AudioService audio;
AudioUploadService* audioUploader;
LogService* logger;

#define CHUNK_SAMPLES 1024
int32_t buffer[CHUNK_SAMPLES];

unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 30000; // 30 วินาที

void initTime() {
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    delay(2000);
}

void setup() {
    Serial.begin(115225);
    config.beginWiFi();
    config.beginFirebase();

    initTime();

    sensor.begin();
    motor.begin();
    audio.begin();

    logger = new LogService(&config);
    logger->begin();

    // audioUploader = new AudioUploadService(&config, &audio);
    // audioUploader->begin();
    // audioUploader->startRecording();

    Serial.println("[System] Ready");
}


void loop() {
    bool button = sensor.getButton();
    float tilt  = sensor.getTiltDegree();
    float water = sensor.getWaterLevel();
    float humidity = sensor.getHumidity();
    float temperature = sensor.getTemperature();

    // ส่งขึ้น Firebase Realtime Database
    config.sendBool("/sensor/button", button);
    config.sendFloat("/sensor/tilt", tilt);
    config.sendFloat("/sensor/water", water);
    config.sendFloat("/sensor/humidity", humidity);
    config.sendFloat("/sensor/temp", temperature);

    // Motor Logic
    motor.handleLogic(water, tilt > 0, button);

    // Log to Firestore every 30 seconds
    if (millis() - lastLogTime >= LOG_INTERVAL) {
        logger->writeLog(temperature, humidity, water, tilt, button);
        lastLogTime = millis();
    }

    delay(500);
}
