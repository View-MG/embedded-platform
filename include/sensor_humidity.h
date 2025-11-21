#pragma once
#include <Arduino.h>
#include <DHT.h>

#define DHT_PIN   14
#define DHT_TYPE  DHT11

class SensorHumidity {
public:
    SensorHumidity();
    void begin();
    float readTemperature();
    float readHumidity();

private:
    DHT dht;
};
