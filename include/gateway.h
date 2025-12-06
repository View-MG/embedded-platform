#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "constant.h"
#include <time.h>

extern SensorPacket currentSensorData;
extern bool isSensorDataNew;

class GatewayNetwork{
private:
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    CommandPacket cmd;

    static void onRecv(const uint8_t * mac,const uint8_t*incoming,int len){
        if(len==sizeof(SensorPacket)){
            memcpy(&currentSensorData,incoming,sizeof(SensorPacket));
            isSensorDataNew=true;
        }
    }

public:
    void begin(){
        WiFi.mode(WIFI_AP_STA);
        WiFi.setSleep(false);
        WiFi.begin(WIFI_SSID,WIFI_PASSWORD);

        Serial.print("[WiFi] Connecting");
        while(WiFi.status()!=WL_CONNECTED){
            Serial.print(".");
            delay(300);
        }
        Serial.println("\n[WiFi] Connected ✔");

        configTime(7*3600,0,"pool.ntp.org");
        while(time(nullptr) < 1000000000){
            Serial.print(".");
            delay(300);
        }
        Serial.println("\n[Time] Synced");

        config.api_key = FIREBASE_API_KEY;
        config.database_url = FIREBASE_DATABASE_URL;
        config.token_status_callback = tokenStatusCallback;

        Firebase.signUp(&config,&auth,"","");
        Firebase.begin(&config,&auth);
        Firebase.reconnectWiFi(true);

        if(esp_now_init()!=ESP_OK){
            Serial.println("ESP-NOW Init Failed!");
            return;
        }

        esp_now_register_recv_cb(onRecv);

        esp_now_peer_info_t peer={};
        memcpy(peer.peer_addr,SENSOR_NODE_MAC,6);
        peer.channel=0; peer.encrypt=false;
        esp_now_add_peer(&peer);

        Serial.println("[Network] Ready");
    }

    void send(uint8_t type,bool state){
        cmd.deviceType=type;
        cmd.active=state;
        esp_now_send(SENSOR_NODE_MAC,(uint8_t*)&cmd,sizeof(cmd));
    }

    FirebaseData* get(){ return &fbdo; }
    bool ok(){ return Firebase.ready(); }

    void log(String s){
        if(ok()) Firebase.RTDB.pushString(&fbdo, PATH_ERROR_LOG, s);
    }
};
