#pragma once
#include <Arduino.h>

#define WATER_PIN 32   // ขา analog สำหรับ water sensor

class SensorWater {
public:
    SensorWater();
    void begin();
    float readWaterLevel();

private:
    int pin;
};
