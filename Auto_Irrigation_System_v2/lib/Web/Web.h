#ifndef WEB_H
#define WEB_H
#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <Arduino_JSON.h>

class Web {
private:
    const char* ssid = "InternetHandy";
    const char* password = "thisisuluk";
    const String versionUrl = "https://raw.githubusercontent.com/Ulukbek1234/Auto_Watering_System/ESP/Auto_Irrigation_System_v2/releases/version.json";
    String firmwareUrl = "";
    String firmwareVersion = "0.0.1";

    WebServer* config_server = nullptr;
    WebSocketsServer *web_socket = nullptr;
    String last_message = "";
    uint8_t local_client;
    Preferences prefs;
public:
    Web(uint16_t port = 81);

    void begin();
    void startProvisioningPortal();
    bool connectSavedWiFi();
    bool connectWiFi(String ssid, String password);
    
    String read();
    void onWebSocketEvent(  uint8_t client,
                            WStype_t type,
                            uint8_t * payload,
                            size_t length);
    void write(String output);
    void updateFirmware();
    void checkFirmwareVersion();
};

#endif
