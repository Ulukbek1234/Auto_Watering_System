#ifndef WEB_H
#define WEB_H
#pragma once

#include <WiFi.h>
#include <WebSocketsServer.h>

class Web {
private:
    const char* ssid = "InternetUluk";
    const char* password = "thisisuluk";
    WebSocketsServer *web_socket;
    String last_message = "";
public:
    Web(uint16_t port = 81);

    void begin();
    String read();
    void onWebSocketEvent(  uint8_t client,
                            WStype_t type,
                            uint8_t * payload,
                            size_t length);
    

};

#endif
