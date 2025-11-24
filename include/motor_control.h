#pragma once
#include <Arduino.h>
#include <time.h>
#include "config_service.h"

#define MOTOR_PIN 25
#define CONFIG_FETCH_INTERVAL 5000UL  // อ่าน config ทุก 5 วินาที

// โครงสร้างที่ดึงข้อมูลมาจาก Firebase
struct MotorRemoteConfig {
    String mode;        // "auto", "manual", "off"
    bool   manualState; // สำหรับ Manual override
    long   schedStart;  // UTC timestamp (วินาที)
    long   schedEnd;
};

class MotorControl {
public:
    MotorControl();
    void begin(ConfigService* c);
    void update(bool tilted);

    void on();
    void off();
    bool isOn();

private:
    int pin;
    bool _state;

    bool timeReady;

    MotorRemoteConfig cfg;

    long lastPrintedMinute;
    unsigned long lastConfigFetch;

    ConfigService* config;

    void initTime();
    void fetchConfig();

    bool computeMotorState(
        time_t now,
        bool tilted,
        bool& hasNext,
        bool& nextIsOn,
        long& nextTS
    );

    void printCountdown(
        bool motorOn,
        bool hasNext,
        bool nextIsOn,
        long now,
        long nextTS
    );
};
