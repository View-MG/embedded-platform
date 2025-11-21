#include "sensor_tilt.h"

SensorTilt::SensorTilt()
    : pin(TILT_PIN)
{}

void SensorTilt::begin() {
    pinMode(pin, INPUT);
}

bool SensorTilt::isTilted() {
    int val = digitalRead(pin);
    return val == HIGH;
}
