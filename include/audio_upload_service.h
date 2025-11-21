#pragma once
#include <Arduino.h>
#include "audio_service.h"
#include "config_service.h"

class AudioUploadService {
public:
    AudioUploadService(ConfigService* config, AudioService* audio);

    void begin();
    void startRecording();
    void process();
    void attemptUpload();

private:
    ConfigService* config;
    AudioService* audio;

    bool recording = false;
    int32_t *recordBuffer;
    int recordIndex = 0;
};
