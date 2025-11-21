#pragma once
#include <Arduino.h>

#define BUTTON_PIN  26   // digital input

class SensorButton {
public:
    SensorButton();
    void begin();
    bool readButton();   // true = กด

private:
    int pin;
};
