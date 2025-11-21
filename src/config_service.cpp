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

    auth.user.email = "";
    auth.user.password = "";

    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("[Firebase] sign-up OK");
    } else {
        Serial.print("[Firebase] sign-up FAILED: ");
        Serial.println(config.signer.signupError.message.c_str());
    }

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("[Firebase] Ready");
}


void ConfigService::sendFloat(const String& path, float value) {
    Firebase.RTDB.setFloat(&fbdo, path.c_str(), value);
}

float ConfigService::readFloat(const String& path) {
    if (Firebase.RTDB.getFloat(&fbdo, path.c_str()))
        return fbdo.floatData();
    return NAN;
}

void ConfigService::sendBool(const String& path, bool value) {
    Firebase.RTDB.setBool(&fbdo, path.c_str(), value);
}

bool ConfigService::readBool(const String& path) {
    if (Firebase.RTDB.getBool(&fbdo, path.c_str()))
        return fbdo.boolData();
    return false;
}