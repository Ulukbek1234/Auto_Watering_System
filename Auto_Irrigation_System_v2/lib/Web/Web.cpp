#include "Web.h"

Web::Web()
{
  initConnection();
}

bool Web::connectSavedWiFi()
{
  prefs.begin("wifi", true);
  String savedSsid = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  firmwareUrl = prefs.getString("url", "");
  firmwareVersion = prefs.getString("version", "0.0.1");
  prefs.end();

  if (savedSsid == "") {
    DEBUG_PRINTLN("No saved WiFi credentials");
    return false;
  }

  return connectWiFi(savedSsid, savedPass);
}

void Web::startProvisioningPortal()
{
  DEBUG_PRINTLN("Starting ESP32 setup access point");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Setup", "12345678");

  DEBUG_PRINT("Setup AP IP: ");
  DEBUG_PRINTLN(WiFi.softAPIP());

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

      DEBUG_PRINTLN("WiFi saved. Restarting...");
      delay(1000);
      ESP.restart();
    } else {
      DEBUG_PRINTLN("Invalid WiFi credentials");
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
  DEBUG_PRINT("Connecting to WiFi: ");
  DEBUG_PRINTLN(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(200);
    DEBUG_PRINT(".");
  }

  DEBUG_PRINTLN();

  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN("WiFi Connected");
    DEBUG_PRINT("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());

    if(MDNS.begin("esp32")) {
      MDNS.addService("ws", "tcp", 80);
    }
    return true;
  }

  DEBUG_PRINTLN("WiFi connection failed");
  return false;
}

String Web::read()
{
  // Provisioning has to be finished, otherwise this wouldnt run
  if (WiFi.status() != WL_CONNECTED) {
    connectSavedWiFi();
  }

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
    DEBUG_PRINTLN("Phone connected");
    web_socket->sendTXT(client, "Hello from ESP32");
    local_client = client;
  }

  if (type == WStype_TEXT) {
    String msg = String((char*)payload);

    DEBUG_PRINT("Received: ");
    DEBUG_PRINTLN(msg);

    last_message = msg;

  }
  
  if (type == WStype_DISCONNECTED) {
    DEBUG_PRINTLN("Phone disconnected");
  }
}

void Web::write(String output) {
  web_socket->sendTXT(local_client, output);
  
}


void Web::updateFirmware() {
  HTTPClient http;
  DEBUG_PRINTLN("Updating firmware");
  newFirmwareUrl.replace("\"", "");
  http.begin(newFirmwareUrl);
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP error: %d\n", httpCode);
    http.end();
    return;
  }

  int contentLength = http.getSize();

  if (!Update.begin(contentLength)) {
    DEBUG_PRINTLN("Not enough space for OTA");
    http.end();
    return;
  }

  WiFiClient* stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);

  if (written == contentLength && Update.end()) {
    if (Update.isFinished()) {
      DEBUG_PRINTLN("Update complete. Rebooting...");
      prefs.begin("wifi");
      prefs.putString("version", newVersion);
      prefs.putString("url", newFirmwareUrl);
      prefs.end();

      firmwareVersion = newVersion;
      firmwareUrl = newFirmwareUrl;
      http.end();
      ESP.restart();
    }
  } else {
    Serial.printf("Update failed: %s\n", Update.errorString());
  }

  http.end();
}

bool Web::checkFirmwareVersion()
{
  HTTPClient http;
  http.begin(versionUrl);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // Parse JSON
  JSONVar myObject = JSON.parse(payload);
  if (JSON.typeof(myObject) == "undefined") {
    DEBUG_PRINTLN("Parsing input failed!");
    http.end();
    return false;
  }

  // Iterate through keys
  JSONVar keys = myObject.keys();
  for (int i = 0; i < keys.length(); i++) {
    DEBUG_PRINT(keys[i]);
    DEBUG_PRINT(" = ");
    DEBUG_PRINTLN(myObject[keys[i]]);
  }

  if(JSON.stringify(myObject["version"]) == firmwareVersion){
    // Same version
    DEBUG_PRINTLN("Same version, returning");
    http.end();
    return false;
  }

  // different version, gotta update 
  // TODO check if older version?
  DEBUG_PRINTLN("New version found");
  newVersion = JSON.stringify(myObject["version"]);
  newFirmwareUrl = JSON.stringify(myObject["url"]);
  DEBUG_PRINTLN(newVersion);
  DEBUG_PRINTLN(newFirmwareUrl);

  http.end();
  return true;
}

void Web::initConnection() {
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