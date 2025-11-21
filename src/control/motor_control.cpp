#include "motor_control.h"

MotorControl::MotorControl()
    : pin(MOTOR_PIN)
{}

void MotorControl::begin() {
    pinMode(pin, OUTPUT);
    off();
}

void MotorControl::on() {
    digitalWrite(pin, HIGH);
}

void MotorControl::off() {
    digitalWrite(pin, LOW);
}

/*
 ตัวอย่างเงื่อนไขง่าย ๆ:
 - ถ้าน้ำ > 70% → เปิด motor
 - ถ้ามีการเอียง → ปิด motor เพื่อความปลอดภัย
 - กดปุ่ม → toggle motor
*/
void MotorControl::handleLogic(float water, bool tilted, bool button) {

    if (tilted) {
        off();
        return;
    }

    if (water > 70.0f) {
        on();
    } else {
        off();
    }

    if (button) {
        on();   // หรือ toggle ตามที่คุณต้องการ
    }
}
