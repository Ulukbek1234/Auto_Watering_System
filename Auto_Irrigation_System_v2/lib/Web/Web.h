#ifndef WEB_H
#define WEB_H
#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>

class Web {
private:
    const char* ssid = "InternetHandy";
    const char* password = "thisisuluk";
    const char* firmwareUrl =
    "https://github.com/Ulukbek1234/Auto_Watering_System/releases/download/v0.0.1/firmware.bin";

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

};

#endif
