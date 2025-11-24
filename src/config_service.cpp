#include "config_service.h"
#include "config.h"

void ConfigService::beginWiFi() {
    Serial.println("\n[WiFi] Connecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(400);
    }
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
}

void ConfigService::beginFirebase() {
    Serial.println("[Firebase] Initializing...");

    config.api_key = FIREBASE_API_KEY;
    config.database_url = FIREBASE_DATABASE_URL;

    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("[Firebase] sign-up OK");
    } else {
        Serial.println("[Firebase] sign-up FAILED");
    }

    delay(800);

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("[Firebase] Ready");
}

void ConfigService::sendFloat(const String& path, float value) {
    if (Firebase.ready())
        Firebase.RTDB.setFloat(&fbdo, path.c_str(), value);
}

float ConfigService::readFloat(const String& path) {
    if (Firebase.ready() && Firebase.RTDB.getFloat(&fbdo, path.c_str()))
        return fbdo.floatData();
    return NAN;
}

void ConfigService::sendBool(const String& path, bool value) {
    if (Firebase.ready())
        Firebase.RTDB.setBool(&fbdo, path.c_str(), value);
}

bool ConfigService::readBool(const String& path) {
    if (Firebase.ready() && Firebase.RTDB.getBool(&fbdo, path.c_str()))
        return fbdo.boolData();
    return false;
}

void ConfigService::sendString(const String &path, const String &value) {
    if (Firebase.ready())
        Firebase.RTDB.setString(&fbdo, path.c_str(), value);
}

String ConfigService::readString(const String& path) {
    if (!Firebase.ready()) return "";

    if (Firebase.RTDB.getString(&fbdo, path.c_str())) {
        String value = fbdo.stringData();
        value.trim();
        if (value.startsWith("\"") && value.endsWith("\""))
            value = value.substring(1, value.length() - 1);
        return value;
    }
    return "";
}
