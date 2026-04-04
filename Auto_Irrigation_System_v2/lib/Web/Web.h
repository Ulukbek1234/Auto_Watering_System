#ifndef WEB_H
#define WEB_H
#pragma once

#include <WiFi.h>
#include <WebServer.h>

class Web 
{
private:
    // Replace with your network credentials
    const char* ssid = "InternetUluk";
    const char* password = "thisisuluk";
public:
    Web();
};

#endif
