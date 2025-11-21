#pragma once
#include <Arduino.h>

class MockSensor {
public:
    void begin();
    float readWaterLevel();
    float readHumidity();
    float readTiltDegree();
    bool readButton();
};
