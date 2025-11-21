#pragma once
#include <Arduino.h>
#include <driver/i2s.h>

#define SAMPLE_RATE     16000
#define SAMPLE_BITS     32
#define I2S_PORT        I2S_NUM_0

#define I2S_WS   15
#define I2S_SCK  14
#define I2S_SD   32

class AudioService {
public:
    void begin();
    size_t readSamples(int32_t* buffer, size_t samples);

private:
    void initI2S();
};
