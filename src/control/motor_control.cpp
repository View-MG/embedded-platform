#include "motor_control.h"

MotorControl::MotorControl()
    : pin(MOTOR_PIN),
      _state(false),
      timeReady(false),
      lastPrintedMinute(-1),
      lastConfigFetch(0),
      config(nullptr)
{
    cfg.mode = "off";
    cfg.manualState = false;
    cfg.schedStart = 0;
    cfg.schedEnd   = 0;
}

void MotorControl::begin(ConfigService* c) {
    config = c;

    pinMode(pin, OUTPUT);
    off();

    initTime();
    Serial.println("[Motor] Ready");
}

// -----------------------------
// NTP / Time init
// -----------------------------
void MotorControl::initTime() {
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    delay(2000);

    Serial.println("[Motor] Waiting for NTP sync...");
    for (int i = 0; i < 20; i++) {
        time_t now = time(nullptr);
        if (now > 1700000000L) { // เวลาประมาณปลายปี 2023
            timeReady = true;
            Serial.println("[Motor] Time synced");
            return;
        }
        delay(1000); // delay() จะ yield ให้ระบบแล้ว
    }
    Serial.println("[Motor] Failed to sync time");
    timeReady = false;
}

// -----------------------------
// ดึง config จาก Firebase
// -----------------------------
void MotorControl::fetchConfig() {
    if (!config) return;
    if (!Firebase.ready()) return;

    // อ่านทีละตัว แต่แทรก yield() เพื่อไม่ให้ block นานเกินไป
    String modeStr = config->readString("/motor/control/mode");
    yield();

    if (modeStr.length() == 0) modeStr = "off";
    cfg.mode = modeStr;

    cfg.manualState = config->readBool("/motor/control/manual_state");
    yield();

    cfg.schedStart = config->readString("/motor/control/sched_start").toInt();
    yield();

    cfg.schedEnd   = config->readString("/motor/control/sched_end").toInt();
    yield();

    // ถ้า end < start และไม่ใช่ 0 ให้สลับ
    if (cfg.schedEnd != 0 && cfg.schedStart != 0 && cfg.schedEnd < cfg.schedStart) {
        long t = cfg.schedStart;
        cfg.schedStart = cfg.schedEnd;
        cfg.schedEnd = t;
    }

    // Debug เล็ก ๆ (เปิดได้ถ้าต้องการ)
    // Serial.printf("[Motor] cfg: mode=%s manual=%d start=%ld end=%ld\n",
    //               cfg.mode.c_str(), cfg.manualState, cfg.schedStart, cfg.schedEnd);
}

// -----------------------------
// ฟังก์ชันหลักที่ main เรียกทุก loop
// -----------------------------
void MotorControl::update(bool tilted) {
    yield(); // ให้โอกาส WiFi / RTOS

    time_t now = time(nullptr);

    // ดึง config จาก Firebase ทุก CONFIG_FETCH_INTERVAL ms
    unsigned long nowMs = millis();
    if (nowMs - lastConfigFetch >= CONFIG_FETCH_INTERVAL) {
        lastConfigFetch = nowMs;
        fetchConfig();
        yield();
    }

    bool hasNext = false;
    bool nextIsOn = false;
    long nextTS = 0;

    bool targetOn = computeMotorState(now, tilted, hasNext, nextIsOn, nextTS);

    if (targetOn) on();
    else off();

    printCountdown(targetOn, hasNext, nextIsOn, now, nextTS);
}

// -----------------------------
// ตัดสินใจว่า Motor ควร On/Off
// -----------------------------
bool MotorControl::computeMotorState(
    time_t now,
    bool tilted,
    bool &hasNext,
    bool &nextIsOn,
    long &nextTS
) {
    hasNext = false;
    nextTS = 0;

    // Tilted → ปิดทันที
    //if (tilted) return false;

    // ถ้ายังไม่ sync เวลา → ปิด
    if (!timeReady) return false;

    if (cfg.mode == "off") {
        return false;
    }

    if (cfg.mode == "manual") {
        // manualState = true → เปิด, false → ปิด
        Serial.println("[Motor] Open! (Manual Mode)");
        return cfg.manualState;
    }

    if (cfg.mode == "auto") {
        long S = cfg.schedStart;
        long E = cfg.schedEnd;

        if (S == 0 || E == 0) {
            return false;
        }

        if (now < S) {
            hasNext = true;
            nextIsOn = true; // ต่อไปจะ "เปิด"
            nextTS = S;
            return false;
        }
        else if (now >= S && now < E) {
            hasNext = true;
            nextIsOn = false; // ต่อไปจะ "ปิด"
            nextTS = E;
            return true;
        } else {
            // เลยช่วงแล้ว → ปิด
            return false;
        }
    }

    // mode ไม่ตรงอะไรเลย → ปิด
    return false;
}

// -----------------------------
// แสดง countdown บน Serial
// -----------------------------
void MotorControl::printCountdown(
    bool motorOn,
    bool hasNext,
    bool nextIsOn,
    long now,
    long nextTS
){
    if (!hasNext) return;

    long diff = nextTS - now;
    if (diff <= 0) return;

    long minutes = diff / 60;
    if (minutes == lastPrintedMinute) return;

    lastPrintedMinute = minutes;

    if (motorOn) {
        Serial.printf("[Motor] ON  → will OFF in %ld minutes\n", minutes);
    } else {
        if (nextIsOn)
            Serial.printf("[Motor] OFF → will ON in %ld minutes\n", minutes);
        else
            Serial.printf("[Motor] OFF → next change in %ld minutes\n", minutes);
    }
}

// -----------------------------
// ควบคุมขา GPIO จริง ๆ
// -----------------------------
void MotorControl::on() {
    digitalWrite(pin, HIGH);
    _state = true;
    if (config) {
        config->sendBool("/motor/status", true);
    }
}

void MotorControl::off() {
    digitalWrite(pin, LOW);
    _state = false;
    if (config) {
        config->sendBool("/motor/status", false);
    }
}

bool MotorControl::isOn() {
    return _state;
}
