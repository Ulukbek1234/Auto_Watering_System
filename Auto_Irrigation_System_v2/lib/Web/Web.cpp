#include "Web.h"

Web::Web(uint16_t port)
{

  bool connected = connectSavedWiFi();

  if (!connected) {
    startProvisioningPortal();
  }

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

bool Web::connectSavedWiFi()
{
  prefs.begin("wifi", true);
  String savedSsid = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  prefs.end();

  if (savedSsid == "") {
    Serial.println("No saved WiFi credentials");
    return false;
  }

  return connectWiFi(savedSsid, savedPass);
}

void Web::startProvisioningPortal()
{
  Serial.println("Starting ESP32 setup access point");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Setup", "12345678");

  Serial.print("Setup AP IP: ");
  Serial.println(WiFi.softAPIP());

  config_server = new WebServer(80);

  config_server->on("/wifi", HTTP_POST, [this]() {
    String newSsid = config_server->arg("ssid");
    String newPass = config_server->arg("pass");

    if (newSsid == "") {
      config_server->send(400, "application/json", "{\"error\":\"missing ssid\"}");
      return;
    }

    config_server->send(200, "application/json", "{\"status\":\"received\"}");

    delay(500);

    if (connectWiFi(newSsid, newPass)) {
      prefs.begin("wifi", false);
      prefs.putString("ssid", newSsid);
      prefs.putString("pass", newPass);
      prefs.end();

      Serial.println("WiFi saved. Restarting...");
      delay(1000);
      ESP.restart();
    } else {
      Serial.println("Invalid WiFi credentials");
      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP32-Setup", "12345678");
    }
  });

  config_server->on("/", HTTP_GET, [this]() {
    config_server->send(200, "text/plain", "ESP32 setup portal. POST ssid and pass to /wifi");
  });

  config_server->begin();
}

bool Web::connectWiFi(String ssid, String password)
{
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(200);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if(MDNS.begin("esp32")) {
      MDNS.addService("ws", "tcp", 80);
    }
    return true;
  }

  Serial.println("WiFi connection failed");
  return false;
}

String Web::read()
{
  if (config_server != nullptr) {
    config_server->handleClient();
  }

  web_socket->loop();

  String msg = last_message;
  last_message = "";

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


void Web::updateFirmware() {
  WiFiClientSecure client;
  client.setInsecure(); // easier, but not ideal for production

  HTTPClient http;
  http.begin(client, firmwareUrl);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP error: %d\n", httpCode);
    http.end();
    return;
  }

  int contentLength = http.getSize();

  if (!Update.begin(contentLength)) {
    Serial.println("Not enough space for OTA");
    http.end();
    return;
  }

  WiFiClient* stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);

  if (written == contentLength && Update.end()) {
    if (Update.isFinished()) {
      Serial.println("Update complete. Rebooting...");
      ESP.restart();
    }
  } else {
    Serial.printf("Update failed: %s\n", Update.errorString());
  }

  http.end();
}

void Web::checkFirmwareVersion()
{
  WiFiClientSecure client;
  client.setInsecure(); // easier, but not ideal for production

  HTTPClient http;
  http.begin(client, versionUrl);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP error: %d\n", httpCode);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  // Parse JSON
  JSONVar myObject = JSON.parse(payload);
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  // Iterate through keys
  JSONVar keys = myObject.keys();
  for (int i = 0; i < keys.length(); i++) {
    Serial.print(keys[i]);
    Serial.print(" = ");
    Serial.println(myObject[keys[i]]);
  }
}