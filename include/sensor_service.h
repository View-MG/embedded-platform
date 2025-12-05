#pragma once
#include <Arduino.h>
#include "sensor_humidity.h"
#include "sensor_humidity.h"
#include "sensor_water.h"
#include "sensor_tilt.h"
#include "sensor_button.h"

class SensorService {
public:
    SensorService();
    void begin();

    float getTemperature();
    float getHumidity();
    bool getButton();
    float getTiltDegree();
    float getWaterLevel();

private:
    SensorHumidity humidity;
    SensorWater water;
    SensorTilt tilt;
    SensorButton button;
};
