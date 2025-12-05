#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <driver/i2s.h>
#include "config.h" // หรือ config.h ตามที่คุณตั้ง

// config.h ควรมี define พวกนี้:
// #define ENABLE_AUDIO_STREAM true
// #define I2S_READ_LEN 512
// #define SAMPLE_RATE 16000
// #define I2S_SCK 32
// #define I2S_WS 33
// #define I2S_SD 34
// #define WS_HOST "172.20.10.6"
// #define WS_PORT 4000
// #define WS_PATH "/"

class AudioService {
public:
    WebSocketsClient ws;
    int32_t i2s_buffer[I2S_READ_LEN]; 
    int16_t pcm16[I2S_READ_LEN / 2];  // หาร 2 เพราะเราจะยุบ Stereo เป็น Mono

    void begin() {
        Serial.println("[Audio] Init...");
        initI2S();
        connectWS();
    }

    void loop() {
        ws.loop(); // ต้องทำงานถี่ที่สุดเท่าที่เป็นไปได้

        // ถ้า I2S ยังไม่พร้อม หรือไม่ได้เปิดฟีเจอร์ ให้จบเลย
        if (!ENABLE_AUDIO_STREAM) return;

        size_t bytes_read = 0;
        
        // 🔴 KEY POINT: timeout = 0 (Non-blocking)
        // อ่านข้อมูลมาใส่ Buffer แต่ถ้าไม่มีของ ให้ผ่านเลย ไม่ต้องรอ
        esp_err_t err = i2s_read(
            I2S_NUM_0,
            (void*)i2s_buffer,
            sizeof(i2s_buffer),
            &bytes_read,
            0 
        );

        // ถ้าไม่มีข้อมูล หรือ error ให้จบ loop รอบนี้
        if (err != ESP_OK || bytes_read == 0) {
            return;
        }

        int samples = bytes_read / 4;     // จำนวน Sample รวม L+R
        int frames = samples / 2;         // จำนวน Frame (คู่ L/R)

        // แปลงข้อมูล (Logic เดียวกับที่เทสผ่านแล้ว)
        for (int i = 0; i < frames; i++) {
            // เลือก Channel Left (Index คู่)
            int32_t val = i2s_buffer[i * 2]; 

            val = val >> 14; 
            if (val > 32767) val = 32767;
            if (val < -32768) val = -32768;

            pcm16[i] = (int16_t)val;
        }

        if (ws.isConnected()) {
            ws.sendBIN((uint8_t*)pcm16, frames * 2);
        }
    }

private:
    void initI2S() {
        i2s_config_t cfg = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // รับคู่เพื่อความชัวร์
            .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,
            .dma_buf_len = 64,
            .use_apll = false,
            .tx_desc_auto_clear = false,
            .fixed_mclk = 0
        };

        i2s_pin_config_t pin = {
            .bck_io_num = I2S_SCK,
            .ws_io_num = I2S_WS,
            .data_out_num = -1,
            .data_in_num = I2S_SD
        };
        pin.mck_io_num = I2S_PIN_NO_CHANGE;

        i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
        i2s_set_pin(I2S_NUM_0, &pin);
        i2s_zero_dma_buffer(I2S_NUM_0);
        i2s_start(I2S_NUM_0);
    }

    void connectWS() {
        ws.begin(WS_HOST, WS_PORT, WS_PATH);
        ws.setReconnectInterval(2000);
        // ws.enableHeartbeat(...) // แนะนำให้ปิดไปก่อน ถ้า Server Node.js ไม่ได้รับ Ping
        
        ws.onEvent([](WStype_t type, uint8_t*, size_t) {
            if (type == WStype_CONNECTED) Serial.println("🟢 WS Connected");
            else if (type == WStype_DISCONNECTED) Serial.println("🔴 WS Disconnected");
        });
    }
};