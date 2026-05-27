import 'dart:nativewrappers/_internal/vm/lib/ffi_native_type_patch.dart';

import 'package:flutter/material.dart';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'dart:convert';
import 'package:fl_chart/fl_chart.dart';


class IrrigationUnit {
  final String name;
  final String pumpName;
  final String sensorName;
  final int pumpStatus;
  final int soilHumidity;
  final double moistureThreshold;
  final double waterFlowDaily;
  final double waterFlowTotal;
  final double waterFlowDailyMax;

  IrrigationUnit({
    required this.name,
    required this.pumpName,
    required this.sensorName,
    required this.pumpStatus,
    required this.soilHumidity,
    required this.moistureThreshold,
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
      moistureThreshold: (json["moistureThreshold" as num?]?.toDouble() ?? 0.0),
      waterFlowDaily: (json["waterFlowDaily"] as num?)?.toDouble() ?? 0.0,
      waterFlowTotal: (json["waterFlowTotal"] as num?)?.toDouble() ?? 0.0,
      waterFlowDailyMax: (json["waterFlowDailyMax"] as num?)?.toDouble() ?? 0.0,
    );
  }
}

class ChartReading {
  final DateTime time;
  final Double humidity_1;
  final Double humidity_2;
  final Double humidity_3;
  final Double humidity_4;


  ChartReading({
    required this.time,
    required this.humidity_1,
    required this.humidity_2,
    required this.humidity_3,
    required this.humidity_4,
  });
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
  final List<ChartReading> readings = [];


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
                pumpName: "16",
                sensorName: "32",
                pumpStatus: int.tryParse(parsed["current_mode_16"] ?? "0") ?? 0,
                soilHumidity: int.tryParse(parsed["moisture_percent_32"] ?? "0") ?? 0,
                moistureThreshold: double.tryParse(parsed["moisture_threshold_32"] ?? "0") ?? 0,
                waterFlowDaily: double.tryParse(parsed["daily_liter_16"] ?? "0") ?? 0.0,
                waterFlowTotal: double.tryParse(parsed["total_liter_16"] ?? "0") ?? 0.0,
                waterFlowDailyMax: double.tryParse(parsed["max_liter_16"] ?? "0") ?? 0.0,
              ),
              IrrigationUnit(
                name: "Plant 2",
                pumpName: "17",
                sensorName: "33",
                pumpStatus: int.tryParse(parsed["current_mode_17"] ?? "0") ?? 0,
                soilHumidity: int.tryParse(parsed["moisture_percent_33"] ?? "0") ?? 0,
                moistureThreshold: double.tryParse(parsed["moisture_threshold_33"] ?? "0") ?? 0,
                waterFlowDaily: double.tryParse(parsed["daily_liter_17"] ?? "0") ?? 0.0,
                waterFlowTotal: double.tryParse(parsed["total_liter_17"] ?? "0") ?? 0.0,
                waterFlowDailyMax: double.tryParse(parsed["max_liter_17"] ?? "0") ?? 0.0,
              ),
              IrrigationUnit(
                name: "Plant 3",
                pumpName: "18",
                sensorName: "34",
                pumpStatus: int.tryParse(parsed["current_mode_18"] ?? "0") ?? 0,
                soilHumidity: int.tryParse(parsed["moisture_percent_34"] ?? "0") ?? 0,
                moistureThreshold: double.tryParse(parsed["moisture_threshold_34"] ?? "0") ?? 0,
                waterFlowDaily: double.tryParse(parsed["daily_liter_18"] ?? "0") ?? 0.0,
                waterFlowTotal: double.tryParse(parsed["total_liter_18"] ?? "0") ?? 0.0,
                waterFlowDailyMax: double.tryParse(parsed["max_liter_18"] ?? "0") ?? 0.0,

              ),
              IrrigationUnit(
                name: "Plant 4",
                pumpName: "19",
                sensorName: "35",
                pumpStatus: int.tryParse(parsed["current_mode_19"] ?? "0") ?? 0,
                soilHumidity: int.tryParse(parsed["moisture_percent_35"] ?? "0") ?? 0,
                moistureThreshold: double.tryParse(parsed["moisture_threshold_35"] ?? "0") ?? 0,
                waterFlowDaily: double.tryParse(parsed["daily_liter_19"] ?? "0") ?? 0.0,
                waterFlowTotal: double.tryParse(parsed["total_liter_19"] ?? "0") ?? 0.0,
                waterFlowDailyMax: double.tryParse(parsed["max_liter_19"] ?? "0") ?? 0.0,
              ),
            ];
          
            // To
            readings.add(
              ChartReading(
                time: DateTime.now(),
                humidity_1: double.tryParse(parsed["moisture_percent_32"] ?? "0") ?? 0,
                humidity_2: double.tryParse(parsed["moisture_percent_33"] ?? "0") ?? 0,
                humidity_3: double.tryParse(parsed["moisture_percent_34"] ?? "0") ?? 0,
                humidity_4: double.tryParse(parsed["moisture_percent_35"] ?? "0") ?? 0,

              ),
            );
          
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
    final pumpStatusController = TextEditingController(text: unit.pumpStatus.toString());
    final maxDailyLitersController = TextEditingController(text: unit.waterFlowDailyMax.toString());
    final thresholdController = TextEditingController(text: unit.moistureThreshold.toString());

    showDialog(
      context: context,
      builder: (_) => AlertDialog(
        title: Text(unit.name),

        content: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                "Pump: ${unit.pumpName}\n"
                "Sensor: ${unit.sensorName}\n\n"
                "Pump status: ${unit.pumpStatus}\n"
                "Soil humidity: ${unit.soilHumidity}%\n"
                "Moisture threshold: ${unit.moistureThreshold}%\n" 
                "Water flow today: ${unit.waterFlowDaily} L\n"
                "Total water flow: ${unit.waterFlowTotal} L\n"
                "Max water flow: ${unit.waterFlowDailyMax} L"
              ),

              const SizedBox(height: 20),

              // Change pump setting
              /*
                - Set Mode
                - Daily liters
                - Moisture threshold
                - Sensor calibration
                - Manual irrigation
              */

              TextField(
                controller: pumpStatusController,
                keyboardType: TextInputType.number,
                decoration: const InputDecoration(
                  labelText: "Change Mode" ,
                  border: OutlineInputBorder(),
                ),
              ),

              const SizedBox(height: 12),


              TextField(
                controller: maxDailyLitersController,
                keyboardType: TextInputType.number,
                decoration: const InputDecoration(
                  labelText: "Max Daily (L)" ,
                  border: OutlineInputBorder(),
                ),
              ),

              const SizedBox(height: 12),

              TextField(
                controller: thresholdController,
                keyboardType: TextInputType.number,
                decoration: const InputDecoration(
                  labelText: "Humidity threshold (%)",
                  border: OutlineInputBorder(),
                ),
              ),

              // TODO CALIBRATION
              
              
            ],
          ),
        ),

        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text("Close"),
          ),

          ElevatedButton(
            onPressed: () {
              final pumpStatusNew = pumpStatusController.text;
              final maxDailyLitersNew = maxDailyLitersController.text;
              final thresholdNew = thresholdController.text;

              esp32.send(
                "cmd: config, pump: ${unit.pumpName} set_mode: $pumpStatusNew, chg_dly_ltr: $maxDailyLitersNew, chg_moi_thr: $thresholdNew",
              );

              Navigator.pop(context);
            },
            child: const Text("Send"),
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



enum ChartRange { hour, day, week, month }


class PageCharts extends StatefulWidget {
  final List<ChartReading> readings;

  const PageCharts({
    super.key,
    required this.readings,
  });

  @override
  State<PageCharts> createState() => _PageChartsState();
}

class _PageChartsState extends State<PageCharts> {
  ChartRange selectedRange = ChartRange.day;


  Duration get selectedDuration {
    switch (selectedRange) {
      case ChartRange.hour:
        return const Duration(hours: 1);
      case ChartRange.day:
        return const Duration(days: 1);
      case ChartRange.week:
        return const Duration(days: 7);
      case ChartRange.month:
        return const Duration(days: 30);
    }
  }

  List<ChartReading> get filteredReadings {
    final from = DateTime.now().subtract(selectedDuration);

    return widget.readings.where((reading) {
      return reading.time.isAfter(from);
    }).toList();
  }

  List<FlSpot> getHumiditySpots(int sensor) {
    final from = DateTime.now().subtract(selectedDuration);

    return filteredReadings.map((reading) {
      final x = reading.time.difference(from).inMinutes.toDouble();
      return FlSpot(x, reading.humidity_1);
    }).toList();
  }


  double get maxX => selectedDuration.inMinutes.toDouble();

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.all(16),
      child: Column(
        children: [
          SegmentedButton<ChartRange>(
            segments: const [
              ButtonSegment(
                value: ChartRange.hour,
                label: Text("1h"),
              ),
              ButtonSegment(
                value: ChartRange.day,
                label: Text("1d"),
              ),
              ButtonSegment(
                value: ChartRange.week,
                label: Text("1w"),
              ),
              ButtonSegment(
                value: ChartRange.month,
                label: Text("1m"),
              ),
            ],
            selected: {selectedRange},
            onSelectionChanged: (value) {
              setState(() {
                selectedRange = value.first;
              });
            },
          ),

          const SizedBox(height: 20),

          Expanded(
            child: LineChart(
              LineChartData(
                minY: 0,
                maxY: 100,
                minX: 0,
                maxX: maxX,

                gridData: const FlGridData(show: true),

                titlesData: FlTitlesData(
                  topTitles: const AxisTitles(
                    sideTitles: SideTitles(showTitles: false),
                  ),
                  rightTitles: const AxisTitles(
                    sideTitles: SideTitles(showTitles: false),
                  ),
                  leftTitles: const AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      reservedSize: 40,
                      interval: 20,
                    ),
                  ),
                  bottomTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      interval: maxX / 4,
                      getTitlesWidget: (value, meta) {
                        return Text(_bottomLabel(value));
                      },
                    ),
                  ),
                ),

                lineBarsData: [
                  LineChartBarData(
                    spots: getHumiditySpots(),
                    isCurved: true,
                    barWidth: 3,
                    dotData: const FlDotData(show: false),
                  ),
                ],
              ),
            ),
          ),

          const SizedBox(height: 12),

          const Text("Humidity and daily liters over time"),
        ],
      ),
    );
  }

  String _bottomLabel(double value) {
    switch (selectedRange) {
      case ChartRange.hour:
        return "${value.toInt()}m";
      case ChartRange.day:
        return "${(value / 60).toInt()}h";
      case ChartRange.week:
        return "${(value / 1440).toInt()}d";
      case ChartRange.month:
        return "${(value / 1440).toInt()}d";
    }
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