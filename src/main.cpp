#include <Arduino.h>
#include <time.h>
#include "constant.h"
#include "gateway.h"
#include "control/fan.h"
#include "control/steam.h"
#include "audio.h" 

GatewayNetwork network;
AudioService audio;
FanLogic fanLogic(&network);
SteamLogic steamLogic(&network);

SensorPacket currentSensorData;
bool isSensorDataNew = false;

TaskHandle_t AudioTaskHandle;

// AUDIO TASK (CORE 0)
void AudioTask(void * parameter) {
    Serial.println("[Audio] Task Running on CORE 0");
    while(true){
        audio.loop();          // Non-blocking already
        vTaskDelay(1);         // WDT Safe
    }
}

void setup() {
    Serial.begin(115200);

    network.begin();           // WiFi + Firebase + ESP-NOW

    if(ENABLE_AUDIO_STREAM){
        audio.begin();         // Init I2S + WebSocket

        xTaskCreatePinnedToCore(
            AudioTask, "AudioTask", 10000,
            NULL, 1, &AudioTaskHandle, 0          // ← CORE 0
        );
    }

    configTime(7*3600, 0,"pool.ntp.org");
    Serial.println("\n[System] Boot Completed");
}

void loop() {
    static unsigned long lastLogic=0;

    if(millis()-lastLogic > LOGIC_INTERVAL_MS){
        lastLogic = millis();

        time_t now = time(nullptr);
        fanLogic.update(now,currentSensorData);
        steamLogic.update(now,currentSensorData);
    }
}
