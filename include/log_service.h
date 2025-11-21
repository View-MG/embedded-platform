#pragma once
#include <Arduino.h>
#include "config_service.h"

class LogService {
public:
    LogService(ConfigService* config);
    void begin();

    void writeLog(float temperature,
                  float humidity,
                  float water,
                  float tilt,
                  bool button);

private:
    ConfigService* config;
    String getTimestamp();
};
