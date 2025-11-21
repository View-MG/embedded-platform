#include "log_service.h"
#include "config.h"
#include <time.h>

LogService::LogService(ConfigService* c)
    : config(c) {}

void LogService::begin() {
    Serial.println("[LogService] Firestore Ready");
}

String LogService::getTimestamp() {
    time_t now = time(nullptr);
    char buf[40];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return String(buf);
}

void LogService::writeLog(float temperature,
                          float humidity,
                          float water,
                          float tilt,
                          bool button)
{
    String ts = getTimestamp();
    String documentPath = "logs/";
    documentPath += ts;

    FirebaseJson content;
    content.set("fields/temp/doubleValue", temperature);
    content.set("fields/humidity/doubleValue", humidity);
    content.set("fields/water/doubleValue", water);
    content.set("fields/tilt/doubleValue", tilt);
    content.set("fields/button/booleanValue", button);
    content.set("fields/created_at/timestampValue", ts);

    if (Firebase.Firestore.createDocument(&config->fbdo,
                                          FIREBASE_PROJECT_ID,
                                          "",
                                          documentPath.c_str(),
                                          content.raw()))
    {
        Serial.printf("[Firestore] Log saved: %s\n", ts.c_str());
    }
    else {
        Serial.printf("[Firestore] ERROR: %s\n", config->fbdo.errorReason().c_str());
    }
}
