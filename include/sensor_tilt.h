#pragma once
#include <Arduino.h>

#define TILT_PIN  27   // digital pin

class SensorTilt {
public:
    SensorTilt();
    void begin();
    bool isTilted();   // true = เอียง

private:
    int pin;
};
