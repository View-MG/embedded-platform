#include "audio_service.h"

void AudioService::begin() {
    initI2S();
}

void AudioService::initI2S() {
    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 256
    };

    i2s_pin_config_t pin_cfg = {
        .bck_io_num   = I2S_SCK,
        .ws_io_num    = I2S_WS,
        .data_out_num = -1,
        .data_in_num  = I2S_SD
    };

    i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_cfg);
}

size_t AudioService::readSamples(int32_t* buffer, size_t samples) {
    size_t bytesRead = 0;
    i2s_read(I2S_PORT, buffer, samples * sizeof(int32_t), &bytesRead, portMAX_DELAY);
    return bytesRead / sizeof(int32_t);
}
