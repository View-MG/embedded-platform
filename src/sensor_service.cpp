#include "sensor_service.h"

SensorService::SensorService()
    : humidity() {}

void SensorService::begin() {
    humidity.begin();
    water.begin();
    tilt.begin();
    button.begin();
}

float SensorService::getTemperature() {
    return humidity.readTemperature();
}   

float SensorService::getHumidity() {
    return humidity.readHumidity();
}

bool SensorService::getButton() {
    return button.readButton();
}

float SensorService::getTiltDegree() {
    return tilt.isTilted() ? 90.0f : 0.0f;   // KY-020 ให้เพียงเอียง/ไม่เอียง
}

float SensorService::getWaterLevel() {
    return water.readWaterLevel();
}
