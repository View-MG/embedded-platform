#include <Arduino.h>
#include <time.h>
#include "constant.h"
#include "gateway.h"
#include "control/control.h"
#include "audio.h"

// shared กับ gateway.h
SensorPacket currentSensorData;
bool isSensorDataNew = false;

GatewayNetwork    network;
AudioService      audio;
EnvSensorService  env;                // <- ใหม่
ControlLogic      control(&network, &env);

TaskHandle_t AudioTaskHandle;

void AudioTask(void * parameter) {
    Serial.println("[Audio] Task Running on CORE 0");
    while (true) {
        audio.loop();
        vTaskDelay(1);  
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    network.begin();
    control.begin();      // ภายในจะเรียก env.begin()

    if (ENABLE_AUDIO_STREAM) {
        audio.begin();
        xTaskCreatePinnedToCore(
            AudioTask, "AudioTask", 10000,
            NULL, 1, &AudioTaskHandle, 0
        );
    }

    configTime(7*3600, 0, "pool.ntp.org");
    Serial.println("\n[System] Boot Completed");
}

void loop() {
    static unsigned long lastLogic = 0;

    if (millis() - lastLogic > LOGIC_INTERVAL_MS) {
        lastLogic = millis();

        time_t now = time(nullptr);
        control.update(now, currentSensorData);
    }
}
