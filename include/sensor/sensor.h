#pragma once
#include <Arduino.h>
#include <math.h>
#include <DHT.h>
#include <Firebase_ESP_Client.h>
#include "constant.h"

class EnvSensorService {
private:
    DHT dht;
    bool  ready        = false;
    float curTemp      = 0.0f;
    float curHum       = 0.0f;

    float lastTempSent = 0.0f;
    float lastHumSent  = 0.0f;
    bool  pushedOnce   = false;

    unsigned long lastRead = 0;
    unsigned long lastPush = 0;

public:
    EnvSensorService() : dht(DHT_PIN, DHT_TYPE) {}

    void begin() {
        dht.begin();
        Serial.println("[Env] DHT11 init");
    }

    void update(FirebaseData* fb) {
        // อ่านค่าทุก ENV_POLL_MS
        if (millis() - lastRead < ENV_POLL_MS) return;
        lastRead = millis();

        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
            Serial.println("[Env] DHT read failed");
            return;
        }

        ready   = true;
        curHum  = h;
        curTemp = t;

        bool push = false;
        if (!pushedOnce) {
            push = true;
        } else {
            if (fabsf(h - lastHumSent) >= 1.0f ||
                fabsf(t - lastTempSent) >= 0.5f) {
                push = true;
            } else if (millis() - lastPush > SENSOR_PUSH_MS) {
                push = true;
            }
        }

        if (push && fb != nullptr) {
            Firebase.RTDB.setFloat(fb, PATH_SENSOR_TEMP, curTemp);
            Firebase.RTDB.setFloat(fb, PATH_SENSOR_HUMID, curHum);

            lastTempSent = curTemp;
            lastHumSent  = curHum;
            lastPush     = millis();
            pushedOnce   = true;

            Serial.printf("[Env]  T=%.1f°C H=%.1f%%\n", curTemp, curHum);
        }
    }

    bool  isReady()     const { return ready;     }
    float getTemp()     const { return curTemp;   }
    float getHumidity() const { return curHum;    }
};
