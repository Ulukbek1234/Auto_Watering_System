import 'package:flutter/material.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

void main() => runApp(const BottomNavigationBarExampleApp());

class Esp32Service {
  Esp32Service._();
  static final Esp32Service instance = Esp32Service._();

  WebSocketChannel? _channel;

  final ValueNotifier<String> status = ValueNotifier("Disconnected");
  final ValueNotifier<String> message = ValueNotifier("");

  bool get connected => _channel != null;

  Future<void> connect(String ip) async {
    if (_channel != null) return;

    try {
      status.value = "Connecting...";

      final ws = WebSocketChannel.connect(
        Uri.parse("ws://$ip:81/"),
      );

      await ws.ready;

      _channel = ws;
      status.value = "Connected to $ip";

      ws.stream.listen(
        (data) {
          message.value = data.toString();
        },
        onError: (error) {
          _channel = null;
          status.value = "Error: $error";
        },
        onDone: () {
          _channel = null;
          status.value = "Disconnected";
        },
      );
    } catch (e) {
      _channel = null;
      status.value = "Failed: $e";
    }
  }

  void send(String command) {
    if (_channel == null) {
      status.value = "Connect first";
      return;
    }

    _channel!.sink.add(command);
    status.value = "Sent: $command";
  }

  void disconnect() {
    _channel?.sink.close();
    _channel = null;
    status.value = "Disconnected";
  }
}

class BottomNavigationBarExampleApp extends StatelessWidget {
  const BottomNavigationBarExampleApp({super.key});

  @override
  Widget build(BuildContext context) {
    return const MaterialApp(
      home: BottomNavigationBarExample(),
    );
  }
}

class BottomNavigationBarExample extends StatefulWidget {
  const BottomNavigationBarExample({super.key});

  @override
  State<BottomNavigationBarExample> createState() =>
      _BottomNavigationBarExampleState();
}

class _BottomNavigationBarExampleState
    extends State<BottomNavigationBarExample> {
  int _selectedIndex = 0;

  static final List<Widget> _pages = <Widget>[
    PageHome(),
    PageCharts(),
    PageCamera(),
    PageSettings(),
  ];

  void _onItemTapped(int index) {
    setState(() {
      _selectedIndex = index;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Automatic Irrigation System'),
      ),
      body: _pages[_selectedIndex],
      bottomNavigationBar: BottomNavigationBar(
        currentIndex: _selectedIndex,
        selectedItemColor: Colors.amber[800],
        onTap: _onItemTapped,
        items: const [
          BottomNavigationBarItem(
            icon: Icon(Icons.home),
            label: 'Home',
            backgroundColor: Colors.red,
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.bar_chart),
            label: 'Charts',
            backgroundColor: Colors.yellow,
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.camera),
            label: 'Camera',
            backgroundColor: Colors.green,
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.settings),
            label: 'Settings',
            backgroundColor: Colors.purple,
          ),
        ],
      ),
    );
  }
}


class PageHome extends StatelessWidget {
  PageHome({super.key});

  final Esp32Service esp32 = Esp32Service.instance;

  final List<Map<String, dynamic>> units = const [
    {
      "name": "Plant A",
      "pumpName": "pump_pin_16",
      "sensorName": "moisture_percent_32",
      "pumpStatus": "Active",
      "soilHumidity": "42%",
      "waterFlow": "2.4 L/min",
      "waterLevel": "78%",
      "temperature": "23°C",
      "mode": "Automatic",
    },
    {
      "name": "Plant B",
      "pumpName": "pump_pin_17",
      "sensorName": "moisture_percent_33",
      "pumpStatus": "Off",
      "soilHumidity": "61%",
      "waterFlow": "0.0 L/min",
      "waterLevel": "78%",
      "temperature": "22°C",
      "mode": "Manual",
    },
    {
      "name": "Plant C",
      "pumpName": "pump_pin_18",
      "sensorName": "moisture_percent_34",
      "pumpStatus": "Active",
      "soilHumidity": "35%",
      "waterFlow": "2.1 L/min",
      "waterLevel": "78%",
      "temperature": "24°C",
      "mode": "Automatic",
    },
    {
      "name": "Plant D",
      "pumpName": "pump_pin_19",
      "sensorName": "moisture_percent_35",
      "pumpStatus": "Active",
      "soilHumidity": "35%",
      "waterFlow": "2.1 L/min",
      "waterLevel": "78%",
      "temperature": "24°C",
      "mode": "Automatic",
    },
  ];

  void openDetails(BuildContext context, Map<String, dynamic> unit) {
    showDialog(
      context: context,
      builder: (_) => AlertDialog(
        title: Text(unit["name"]),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text("Pump: ${unit["pumpName"]}"),
            Text("Sensor: ${unit["sensorName"]}"),
            const SizedBox(height: 12),
            Text("Pump status: ${unit["pumpStatus"]}"),
            Text("Soil humidity: ${unit["soilHumidity"]}"),
            Text("Water flow: ${unit["waterFlow"]}"),
            Text("Water level: ${unit["waterLevel"]}"),
            Text("Temperature: ${unit["temperature"]}"),
            Text("Mode: ${unit["mode"]}"),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text("Close"),
          ),
          ElevatedButton(
            onPressed: () {
              esp32.send("cmd: telem");
            },
            child: const Text("Refresh"),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        const Text(
          "Irrigation Units",
          style: TextStyle(fontSize: 26, fontWeight: FontWeight.bold),
        ),

        const SizedBox(height: 16),

        ...units.map((unit) {
          return Card(
            margin: const EdgeInsets.only(bottom: 14),
            child: InkWell(
              onTap: () => openDetails(context, unit),
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      unit["name"],
                      style: const TextStyle(
                        fontSize: 20,
                        fontWeight: FontWeight.bold,
                      ),
                    ),

                    const SizedBox(height: 8),

                    Row(
                      children: [
                        const Icon(Icons.water_drop, size: 20),
                        const SizedBox(width: 8),
                        Text("${unit["pumpName"]}: ${unit["pumpStatus"]}"),
                      ],
                    ),

                    const SizedBox(height: 6),

                    Row(
                      children: [
                        const Icon(Icons.sensors, size: 20),
                        const SizedBox(width: 8),
                        Text("${unit["sensorName"]}: ${unit["soilHumidity"]}"),
                      ],
                    ),

                    const SizedBox(height: 10),

                    Text("Flow: ${unit["waterFlow"]}"),
                    Text("Temperature: ${unit["temperature"]}"),

                    const Align(
                      alignment: Alignment.centerRight,
                      child: Icon(Icons.chevron_right),
                    ),
                  ],
                ),
              ),
            ),
          );
        }),

        const SizedBox(height: 20),

        ElevatedButton(
          onPressed: () {
            esp32.send("cmd: telem");
          },
          child: const Text("Refresh All Telemetry"),
        ),

        const SizedBox(height: 16),

        ValueListenableBuilder(
          valueListenable: esp32.message,
          builder: (_, value, __) {
            
            return Text(
              "ESP32 says: $value",
              style: const TextStyle(fontSize: 16),
            );
          },
        ),

        
      ],
    );
  }
}

class PageCharts extends StatelessWidget {
  const PageCharts({super.key});

  @override
  Widget build(BuildContext context) {
    return const Center(
      child: Text(
        'Page Charts',
        style: TextStyle(fontSize: 30),
      ),
    );
  }
}

class PageCamera extends StatelessWidget {
  const PageCamera({super.key});

  @override
  Widget build(BuildContext context) {
    return const Center(
      child: Text(
        'Camera Page',
        style: TextStyle(fontSize: 30),
      ),
    );
  }
}

class PageSettings extends StatefulWidget {
  const PageSettings({super.key});

  @override
  State<PageSettings> createState() => _PageSettingsState();
}

class _PageSettingsState extends State<PageSettings> {
  final Esp32Service esp32 = Esp32Service.instance;

  final TextEditingController ipController =
      TextEditingController(text: "10.150.65.208");

  void connectToEsp32() {
    esp32.connect(ipController.text.trim());
  }

  void sendCommand(String command) {
    esp32.send(command);
  }

  void disconnect() {
    esp32.disconnect();
  }

  @override
  void dispose() {
    ipController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        const Text(
          "Wi-Fi ESP32 Connection",
          style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
        ),

        const SizedBox(height: 16),

        TextField(
          controller: ipController,
          decoration: const InputDecoration(
            labelText: "ESP32 IP Address",
            border: OutlineInputBorder(),
          ),
        ),

        const SizedBox(height: 16),

        ElevatedButton.icon(
          onPressed: connectToEsp32,
          icon: const Icon(Icons.wifi),
          label: const Text("Connect"),
        ),

        const SizedBox(height: 12),

        ElevatedButton(
          onPressed: () => sendCommand("cmd: telem"),
          child: const Text("Send get_telem"),
        ),

        const SizedBox(height: 12),

        ElevatedButton(
          onPressed: () => sendCommand("LED_OFF"),
          child: const Text("Send LED_OFF"),
        ),

        const SizedBox(height: 12),

        ElevatedButton(
          onPressed: disconnect,
          child: const Text("Disconnect"),
        ),

        const SizedBox(height: 24),

        ValueListenableBuilder(
          valueListenable: esp32.status,
          builder: (_, value, __) {
            return Text(
              "Status: $value",
              style: const TextStyle(fontSize: 18),
            );
          },
        ),

        const SizedBox(height: 12),

        ValueListenableBuilder(
          valueListenable: esp32.message,
          builder: (_, value, __) {
            return Text(
              "ESP32 says: $value",
              style: const TextStyle(fontSize: 18),
            );
          },
        ),
      ],
    );
  }
}