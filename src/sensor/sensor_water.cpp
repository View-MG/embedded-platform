#include "sensor_water.h"

SensorWater::SensorWater()
    : pin(WATER_PIN)
{}

void SensorWater::begin() {
    pinMode(pin, INPUT);
}

float SensorWater::readWaterLevel() {
    int raw = analogRead(pin);      // 0–4095
    float percent = (raw / 4095.0f) * 100.0f;
    return percent;
}