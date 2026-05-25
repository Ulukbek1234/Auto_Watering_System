#include "Web.h"

Web::Web(uint16_t port)
{
  WiFi.begin("InternetHandy", "thisisuluk");

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.println(".");
  }

  Serial.println("WiFi Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  web_socket = new WebSocketsServer(port);
  web_socket->begin();

  web_socket->onEvent([this](
    uint8_t client,
    WStype_t type,
    uint8_t * payload,
    size_t length
  ) {
    this->onWebSocketEvent(client, type, payload, length);
  });
}

String Web::read()
{
  web_socket->loop();

  String msg = last_message;
  last_message = ""; // optional: clear after reading

  return msg;
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
    local_client = client;
  }

  if (type == WStype_TEXT) {
    String msg = String((char*)payload);

    Serial.print("Received: ");
    Serial.println(msg);

    last_message = msg;

  }
  
  if (type == WStype_DISCONNECTED) {
    Serial.println("Phone disconnected");
  }
}

void Web::write(String output) {
  web_socket->sendTXT(local_client, output);
  
}