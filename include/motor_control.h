#pragma once
#include <Arduino.h>

#define MOTOR_PIN  25

class MotorControl {
public:
    MotorControl();
    void begin();
    void on();
    void off();
    void handleLogic(float water, bool tilted, bool button);
private:
    int pin;
};
