#include "sensor_button.h"

SensorButton::SensorButton()
    : pin(BUTTON_PIN)
{}

void SensorButton::begin() {
    pinMode(pin, INPUT_PULLUP);  // ใช้ pull-up
}

bool SensorButton::readButton() {
    return digitalRead(pin) == LOW;   // LOW = กด
}
