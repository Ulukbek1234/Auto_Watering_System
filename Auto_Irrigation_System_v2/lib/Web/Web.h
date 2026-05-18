#ifndef WEB_H
#define WEB_H
#pragma once

#include <WiFi.h>
#include <WebServer.h>

class Web {
private:
    const char* ssid = "InternetUluk";
    const char* password = "thisisuluk";
    
public:
    Web(uint16_t port = 5000);

    void begin();
    void loop();

private:
    WebServer server;

    // Route handlers
    void handleRoot();
    void handleOn();
    void handleOff();

    // HTML generator
    String getHTML();
};

#endif
