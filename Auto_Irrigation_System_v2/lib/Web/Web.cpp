#include "Web.h"

// Constructor
Web::Web(uint16_t port)
{
    // Wifi & server start 
    WiFi.begin("InternetHandy", "thisisuluk");
    while(WiFi.status() != WL_CONNECTED) {
      delay(200);
      Serial.println(".");
    }
    Serial.println("WiFi Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    web_socket = new WebSocketsServer(81);
    web_socket->begin();
}

String Web::read()
{
  web_socket->loop();
  return last_message;
}

void Web::onWebSocketEvent(
  uint8_t client,
  WStype_t type,
  uint8_t * payload,
  size_t length
) {
  if (type == WStype_CONNECTED) {
    Serial.println("Phone connected");
    web_socket->sendTXT(client, "Hello from ESP32");
  }

  if (type == WStype_TEXT) {
    String msg = String((char*)payload);

    Serial.print("Received: ");
    Serial.println(msg);
    last_message = msg;

    web_socket->sendTXT(client, "ESP32 got: " + msg);
  }

  if (type == WStype_DISCONNECTED) {
    Serial.println("Phone disconnected");
  }
}