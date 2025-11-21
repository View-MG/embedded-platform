#include "audio_upload_service.h"
#include <time.h>
#include <Firebase_ESP_Client.h>
#include "config.h"
#include <LittleFS.h> 

#define RECORD_SECONDS  2
#define TARGET_SAMPLES  (RECORD_SECONDS * SAMPLE_RATE)
#define CHUNK_SAMPLES   1024

int32_t audioBuffer[CHUNK_SAMPLES];

AudioUploadService::AudioUploadService(ConfigService* c, AudioService* a)
    : config(c), audio(a) {}

void AudioUploadService::begin() {
    if(!LittleFS.begin(true)){ 
        Serial.println("[AudioUpload] LittleFS Mount Failed");
        return;
    }

    recordBuffer = (int32_t*) malloc(TARGET_SAMPLES * sizeof(int32_t));
    Serial.println("[AudioUpload] Ready");
}

void AudioUploadService::startRecording() {
    recordIndex = 0;
    recording = true;
    Serial.println("[AudioUpload] Start recording...");
}

void AudioUploadService::process() {
    if (!recording) return;

    size_t n = audio->readSamples(audioBuffer, CHUNK_SAMPLES);

    for (size_t i = 0; i < n; i++) {
        if (recordIndex < TARGET_SAMPLES) {
            recordBuffer[recordIndex++] = audioBuffer[i];
        }
    }

    if (recordIndex >= TARGET_SAMPLES) {
        recording = false;
        Serial.println("[AudioUpload] Recording finished.");
        attemptUpload();
    }
}

String getTimestampFilename() {
    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "audio_%Y-%m-%dT%H-%M-%S.raw", gmtime(&now));
    return String(buf);
}

void AudioUploadService::attemptUpload() {
    // 1. Define a temporary local file path
    String tempFileName = "/temp_audio.raw";

    // 2. Open file in Write mode
    File file = LittleFS.open(tempFileName, "w");
    if (!file) {
        Serial.println("[AudioUpload] Failed to open file for writing");
        return;
    }

    // 3. Write the buffer from RAM to the File System
    // We cast int32_t* to uint8_t* and calculate total bytes
    file.write((uint8_t*)recordBuffer, TARGET_SAMPLES * sizeof(int32_t));
    file.close();

    // 4. Prepare the cloud path
    String cloudPath = "audio/";
    cloudPath += getTimestampFilename();

    Serial.printf("[AudioUpload] Uploading %s ...\n", cloudPath.c_str());

    // 5. Use the correct overload for File Upload
    // Params: &fbdo, bucketID, localPath, storageType, remotePath, mimeType
    bool ok = Firebase.Storage.upload(
        &config->fbdo,
        STORAGE_BUCKET_ID,
        tempFileName.c_str(),    // Local file path
        mem_storage_type_flash,  // Tell it to look in Flash (LittleFS/SPIFFS)
        cloudPath.c_str(),       // Remote file name
        "application/octet-stream"
    );

    if (ok) {
        Serial.println("[AudioUpload] Upload OK!");
    } else {
        Serial.printf("[AudioUpload] Upload FAILED: %s\n",
                      config->fbdo.errorReason().c_str());
    }
}