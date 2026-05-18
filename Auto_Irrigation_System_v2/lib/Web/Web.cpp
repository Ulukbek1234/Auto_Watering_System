#include "Web.h"

// Constructor
Web::Web(uint16_t port)
  : server(port) {
    // Wifi & server start 
    WiFi.begin("InternetUluk", "thisisuluk");
    while(WiFi.status() != WL_CONNECTED) {
      delay(200);
      Serial.println(".");
    }
    Serial.println("WiFi Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// Start server and define routes
void Web::begin() {

  // Root page
  server.on("/", [this]() {
    this->handleRoot();
  });

  // Commands
  server.on("/on", [this]() {
    this->handleOn();
  });

  server.on("/off", [this]() {
    this->handleOff();
  });

  server.begin();
}

// Handle client requests
void Web::loop() {
  server.handleClient();
}

// ===== Route Handlers =====

void Web::handleRoot() {
  server.send(200, "text/html", getHTML());
}

void Web::handleOn() {
  digitalWrite(2, HIGH); // LED ON (GPIO 2)
  server.send(200, "text/plain", "LED ON");
}

void Web::handleOff() {
  digitalWrite(2, LOW); // LED OFF
  server.send(200, "text/plain", "LED OFF");
}

// ===== HTML Page =====

String Web::getHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Control</title>
  <style>
    body { text-align: center; font-family: Arial; }
    button {
      padding: 20px;
      font-size: 20px;
      margin: 10px;
      width: 200px;
    }
  </style>
</head>
<body>
  <h1>ESP32 Control Panel</h1>

  <button onclick="sendCmd('on')">LED ON</button>
  <button onclick="sendCmd('off')">LED OFF</button>

  <script>
    function sendCmd(cmd) {
      fetch('/' + cmd)
        .then(res => console.log(cmd))
        .catch(err => console.error(err));
    }
  </script>
</body>
</html>
)rawliteral";
}

// void Web::handleRoot() {
//   String html = "<!DOCTYPE html><html>";
//   html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
//   html += "<link rel=\"icon\" href=\"data:,\">";
//   html += "<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}";
//   html += "table { border-collapse: collapse; width:60%; margin-left:auto; margin-right:auto; }";
//   html += "th { padding: 10px; background-color: #0043af; color: white; }";
//   html += "tr { border: 1px solid #ddd; padding: 10px; }";
//   html += "tr:hover { background-color: #bcbcbc; }";
//   html += "td { border: none; padding: 8px; }";
//   html += ".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }</style></head>";
//   html += "<body><h1>Auto Irrigation Grow Dashboard</h1>";
//   html += "<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>";
//   html += "<tr><td>Pump 1 Total</td><td><span class=\"sensor\">";
//   html += String(1);
//   html += " *C</span></td></tr>";
//   html += "<tr><td>Pump 2 Total</td><td><span class=\"sensor\">";
//   html += String(2);
//   html += " *F</span></td></tr>";
//   html += "<tr><td>Pump 3 Total</td><td><span class=\"sensor\">";
//   html += String(3);
//   html += " hPa</span></td></tr>";
//   html += "<tr><td>Pump 4 Total</td><td><span class=\"sensor\">";
//   html += String(4);
//   html += " m</span></td></tr>";
//   html += " %</span></td></tr></table></body></html>";

//   // Send the response to the client
//   server.send(200, "text/html", html);
// }

// void Web::loop() {
//   // server.handleClient();
//   Serial.println("Web loop");
// }

/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/esp32-web-server-beginners-guide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/



// // Assign output variables to GPIO pins
// const int output26 = 26;
// const int output27 = 27;
// String output26State = "off";
// String output27State = "off";

// // Create a web server object

// // Function to handle turning GPIO 26 on
// void handleGPIO26On() {
//   output26State = "on";
//   digitalWrite(output26, HIGH);
//   handleRoot();
// }

// // Function to handle turning GPIO 26 off
// void handleGPIO26Off() {
//   output26State = "off";
//   digitalWrite(output26, LOW);
//   handleRoot();
// }

// // Function to handle turning GPIO 27 on
// void handleGPIO27On() {
//   output27State = "on";
//   digitalWrite(output27, HIGH);
//   handleRoot();
// }

// // Function to handle turning GPIO 27 off
// void handleGPIO27Off() {
//   output27State = "off";
//   digitalWrite(output27, LOW);
//   handleRoot();
// }

// // Function to handle the root URL and show the current states
// void handleRoot() {
//   String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
//   html += "<link rel=\"icon\" href=\"data:,\">";
//   html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
//   html += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
//   html += ".button2 { background-color: #555555; }</style></head>";
//   html += "<body><h1>ESP32 Web Server</h1>";

//   // Display GPIO 26 controls
//   html += "<p>GPIO 26 - State " + output26State + "</p>";
//   if (output26State == "off") {
//     html += "<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>";
//   } else {
//     html += "<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>";
//   }

//   // Display GPIO 27 controls
//   html += "<p>GPIO 27 - State " + output27State + "</p>";
//   if (output27State == "off") {
//     html += "<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>";
//   } else {
//     html += "<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>";
//   }

//   html += "</body></html>";
//   server.send(200, "text/html", html);
// }

// void setup() {
//   Serial.begin(115200);

//   // Initialize the output variables as outputs
//   pinMode(output26, OUTPUT);
//   pinMode(output27, OUTPUT);
//   // Set outputs to LOW
//   digitalWrite(output26, LOW);
//   digitalWrite(output27, LOW);



//   // Set up the web server to handle different routes
//   server.on("/", handleRoot);
//   server.on("/26/on", handleGPIO26On);
//   server.on("/26/off", handleGPIO26Off);
//   server.on("/27/on", handleGPIO27On);
//   server.on("/27/off", handleGPIO27Off);

//   // Start the web server
//   server.begin();
//   Serial.println("HTTP server started");
// }

// void loop() {
//   // Handle incoming client requests
//   server.handleClient();
// }