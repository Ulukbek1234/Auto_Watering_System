import 'package:flutter/material.dart';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'dart:convert';

class IrrigationUnit {
  final String name;
  final String pumpName;
  final String sensorName;
  final int pumpStatus;
  final int soilHumidity;
  final double waterFlowDaily;
  final double waterFlowTotal;
  final double waterFlowDailyMax;

  IrrigationUnit({
    required this.name,
    required this.pumpName,
    required this.sensorName,
    required this.pumpStatus,
    required this.soilHumidity,
    required this.waterFlowDaily,
    required this.waterFlowTotal,
    required this.waterFlowDailyMax,
  });

  factory IrrigationUnit.fromJson(Map<String, dynamic> json) {
    return IrrigationUnit(
      name: json["name"] ?? "Unknown Unit",
      pumpName: json["pumpName"] ?? "Unknown Pump",
      sensorName: json["sensorName"] ?? "Unknown Sensor",
      pumpStatus: json["pumpStatus"] ?? "Unknown",
      soilHumidity: (json["soilHumidity"] as num?)?.toInt() ?? 0,
      waterFlowDaily: (json["waterFlowDaily"] as num?)?.toDouble() ?? 0.0,
      waterFlowTotal: (json["waterFlowTotal"] as num?)?.toDouble() ?? 0.0,
      waterFlowDailyMax: (json["waterFlowDailyMax"] as num?)?.toDouble() ?? 0.0,
    );
  }
}

Map<String, String> parseTelemetryText(String text) {
  final Map<String, String> data = {};

  final parts = text.split(',');

  for (final part in parts) {
    final keyValue = part.trim().split(':');

    if (keyValue.length == 2) {
      data[keyValue[0].trim()] = keyValue[1].trim();
    }
  }

  return data;
}

void main() => runApp(const BottomNavigationBarExampleApp());

class Esp32Service {
  Esp32Service._();
  static final Esp32Service instance = Esp32Service._();

  WebSocketChannel? _channel;
  final ValueNotifier<List<IrrigationUnit>> units = ValueNotifier([]);

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
          final text = data.toString();
          message.value = text;

          final parsed = parseTelemetryText(text);

          if (parsed.isNotEmpty) {
            units.value = [
              IrrigationUnit(
                name: "Plant 1",
                pumpName: "Pump 16",
                sensorName: "Sensor 32",
                pumpStatus: int.tryParse(parsed["current_mode_16"] ?? "0") ?? 0,
                soilHumidity: int.tryParse(parsed["moisture_percent_32"] ?? "0") ?? 0,
                waterFlowDaily: double.tryParse(parsed["daily_liter_16"] ?? "0") ?? 0.0,
                waterFlowTotal: double.tryParse(parsed["total_liter_16"] ?? "0") ?? 0.0,
                waterFlowDailyMax: double.tryParse(parsed["max_liter_16"] ?? "0") ?? 0.0,
              ),
              IrrigationUnit(
                name: "Plant 2",
                pumpName: "Pump 17",
                sensorName: "Sensor 33",
                pumpStatus: int.tryParse(parsed["current_mode_17"] ?? "0") ?? 0,
                soilHumidity: int.tryParse(parsed["moisture_percent_33"] ?? "0") ?? 0,
                waterFlowDaily: double.tryParse(parsed["daily_liter_17"] ?? "0") ?? 0.0,
                waterFlowTotal: double.tryParse(parsed["total_liter_17"] ?? "0") ?? 0.0,
                waterFlowDailyMax: double.tryParse(parsed["max_liter_17"] ?? "0") ?? 0.0,
              ),
              IrrigationUnit(
                name: "Plant 3",
                pumpName: "Pump 18",
                sensorName: "Sensor 34",
                pumpStatus: int.tryParse(parsed["current_mode_18"] ?? "0") ?? 0,
                soilHumidity: int.tryParse(parsed["moisture_percent_34"] ?? "0") ?? 0,
                waterFlowDaily: double.tryParse(parsed["daily_liter_18"] ?? "0") ?? 0.0,
                waterFlowTotal: double.tryParse(parsed["total_liter_18"] ?? "0") ?? 0.0,
                waterFlowDailyMax: double.tryParse(parsed["max_liter_18"] ?? "0") ?? 0.0,

              ),
              IrrigationUnit(
                name: "Plant 4",
                pumpName: "Pump 19",
                sensorName: "Sensor 35",
                pumpStatus: int.tryParse(parsed["current_mode_19"] ?? "0") ?? 0,
                soilHumidity: int.tryParse(parsed["moisture_percent_35"] ?? "0") ?? 0,
                waterFlowDaily: double.tryParse(parsed["daily_liter_19"] ?? "0") ?? 0.0,
                waterFlowTotal: double.tryParse(parsed["total_liter_19"] ?? "0") ?? 0.0,
                waterFlowDailyMax: double.tryParse(parsed["max_liter_19"] ?? "0") ?? 0.0,
              ),
            ];
          }
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

  void openDetails(BuildContext context, IrrigationUnit unit) {
    showDialog(
      context: context,
      builder: (_) => AlertDialog(
        title: Text(unit.name),
        content: Text(
          "Pump: ${unit.pumpName}\n"
          "Sensor: ${unit.sensorName}\n\n"
          "Pump status: ${unit.pumpStatus}\n"
          "Soil humidity: ${unit.soilHumidity}%\n"
          "Water flow: ${unit.waterFlowDaily} L/Day\n"
          "Water flow: ${unit.waterFlowTotal} L/Total",
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

        ElevatedButton(
          onPressed: () {
            esp32.send("cmd: telem");
          },
          child: const Text("Refresh Telemetry"),
        ),

        const SizedBox(height: 16),

        ValueListenableBuilder<List<IrrigationUnit>>(
          valueListenable: esp32.units,
          builder: (_, units, __) {
            if (units.isEmpty) {
              return const Text(
                "No telemetry yet. Connect in Settings, then press Refresh Telemetry.",
                style: TextStyle(fontSize: 16),
              );
            }

            return Column(
              children: units.map((unit) {
                return Card(
                  margin: const EdgeInsets.only(bottom: 14),
                  child: ListTile(
                    leading: const Icon(Icons.water_drop),
                    title: Text(
                      unit.name,
                      style: const TextStyle(fontWeight: FontWeight.bold),
                    ),
                    subtitle: Text(
                      "${unit.pumpName}: ${unit.pumpStatus}\n"
                      "${unit.sensorName}: ${unit.soilHumidity}%\n"
                      "Flow: ${unit.waterFlowDaily} L/Day",
                    ),
                    isThreeLine: true,
                    trailing: const Icon(Icons.chevron_right),
                    onTap: () => openDetails(context, unit),
                  ),
                );
              }).toList(),
            );
          },
        ),

        const SizedBox(height: 16),

        ValueListenableBuilder(
          valueListenable: esp32.message,
          builder: (_, value, __) {
            return Text(
              "Last ESP32 message: $value",
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