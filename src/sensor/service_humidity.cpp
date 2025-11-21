#include "sensor_humidity.h"

SensorHumidity::SensorHumidity()
    : dht(DHT_PIN, DHT_TYPE)   // KY-015 = DHT11
{}

void SensorHumidity::begin() {
    dht.begin();
}

float SensorHumidity::readTemperature() {
    float t = dht.readTemperature();
    return isnan(t) ? -1 : t;
}

float SensorHumidity::readHumidity() {
    float h = dht.readHumidity();
    return isnan(h) ? -1 : h;
}
