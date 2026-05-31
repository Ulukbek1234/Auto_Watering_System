import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

void main() => runApp(const IrrigationApp());

class IrrigationApp extends StatelessWidget {
  const IrrigationApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.green),
        useMaterial3: true,
      ),
      home: const MainScreen(),
    );
  }
}

class IrrigationUnit {
  final String name;
  final String pumpName;
  final String sensorName;
  final int pumpStatus;
  final double soilHumidity;
  final double moistureThreshold;
  final double waterFlowDaily;
  final double waterFlowTotal;
  final double waterFlowDailyMax;

  const IrrigationUnit({
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

  factory IrrigationUnit.fromTelemetry({
    required Map<String, String> data,
    required int index,
    required String pumpPin,
    required String sensorPin,
  }) {
    return IrrigationUnit(
      name: 'Plant $index',
      pumpName: pumpPin,
      sensorName: sensorPin,
      pumpStatus: _intValue(data, 'current_mode_$pumpPin'),
      soilHumidity: _doubleValue(data, 'moisture_percent_$sensorPin'),
      moistureThreshold: _doubleValue(data, 'moisture_threshold_$sensorPin'),
      waterFlowDaily: _doubleValue(data, 'daily_liter_$pumpPin'),
      waterFlowTotal: _doubleValue(data, 'total_liter_$pumpPin'),
      waterFlowDailyMax: _doubleValue(data, 'max_liter_$pumpPin'),
    );
  }
}

class ChartReading {
  final DateTime time;
  final List<double> humidities;

  const ChartReading({
    required this.time,
    required this.humidities,
  });
}

int _intValue(Map<String, String> data, String key) {
  return int.tryParse(data[key] ?? '') ?? 0;
}

double _doubleValue(Map<String, String> data, String key) {
  return double.tryParse(data[key] ?? '') ?? 0.0;
}

Map<String, String> parseTelemetryText(String text) {
  final result = <String, String>{};

  for (final part in text.split(',')) {
    final separatorIndex = part.indexOf(':');
    if (separatorIndex == -1) continue;

    final key = part.substring(0, separatorIndex).trim();
    final value = part.substring(separatorIndex + 1).trim();

    if (key.isNotEmpty) {
      result[key] = value;
    }
  }

  return result;
}

class Esp32Service {
  Esp32Service._();
  static final Esp32Service instance = Esp32Service._();

  static const _pumpPins = ['16', '17', '18', '19'];
  static const _sensorPins = ['32', '33', '34', '35'];

  WebSocketChannel? _channel;

  final ValueNotifier<List<IrrigationUnit>> units = ValueNotifier([]);
  final ValueNotifier<List<ChartReading>> readings = ValueNotifier([]);
  final ValueNotifier<String> status = ValueNotifier('Disconnected');
  final ValueNotifier<String> message = ValueNotifier('');

  bool get connected => _channel != null;

  Future<void> connect(String ip) async {
    final trimmedIp = ip.trim();
    if (trimmedIp.isEmpty) {
      status.value = 'Enter an ESP32 IP address';
      return;
    }

    if (_channel != null) return;

    try {
      status.value = 'Connecting...';
      final ws = WebSocketChannel.connect(Uri.parse('ws://$trimmedIp:81/'));
      await ws.ready;

      _channel = ws;
      status.value = 'Connected to $trimmedIp';

      ws.stream.listen(
        _handleMessage,
        onError: (Object error) {
          _channel = null;
          status.value = 'Error: $error';
        },
        onDone: () {
          _channel = null;
          status.value = 'Disconnected';
        },
      );
    } catch (error) {
      _channel = null;
      status.value = 'Failed: $error';
    }
  }

  void _handleMessage(dynamic data) {
    final text = data.toString();
    message.value = text;

    final parsed = parseTelemetryText(text);
    if (parsed.isEmpty) return;

    final nextUnits = List<IrrigationUnit>.generate(_pumpPins.length, (i) {
      return IrrigationUnit.fromTelemetry(
        data: parsed,
        index: i + 1,
        pumpPin: _pumpPins[i],
        sensorPin: _sensorPins[i],
      );
    });

    units.value = nextUnits;

    final nextReading = ChartReading(
      time: DateTime.now(),
      humidities: _sensorPins
          .map((pin) => _doubleValue(parsed, 'moisture_percent_$pin'))
          .toList(),
    );

    final history = [...readings.value, nextReading];
    const maxHistoryItems = 1000;
    readings.value = history.length > maxHistoryItems
        ? history.sublist(history.length - maxHistoryItems)
        : history;
  }

  void send(String command) {
    final channel = _channel;
    if (channel == null) {
      status.value = 'Connect first';
      return;
    }

    channel.sink.add(command);
    status.value = 'Sent: $command';
  }

  void disconnect() {
    _channel?.sink.close();
    _channel = null;
    status.value = 'Disconnected';
  }
}

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  int _selectedIndex = 0;

  final _pages = const [
    PageHome(),
    PageCharts(),
    PageCamera(),
    PageSettings(),
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Automatic Irrigation System')),
      body: _pages[_selectedIndex],
      bottomNavigationBar: NavigationBar(
        selectedIndex: _selectedIndex,
        onDestinationSelected: (index) => setState(() => _selectedIndex = index),
        destinations: const [
          NavigationDestination(
              icon: Icon(Icons.home), 
              label: 'Home'),
          NavigationDestination(icon: Icon(Icons.show_chart), label: 'Charts'),
          NavigationDestination(icon: Icon(Icons.camera_alt), label: 'Camera'),
          NavigationDestination(icon: Icon(Icons.settings), label: 'Settings'),
        ],
      ),
    );
  }
}

class PageHome extends StatelessWidget {
  const PageHome({super.key});

  Esp32Service get esp32 => Esp32Service.instance;

  void _openDetails(BuildContext context, IrrigationUnit unit) {
    final modeController = TextEditingController(text: unit.pumpStatus.toString());
    final maxLitersController = TextEditingController(text: unit.waterFlowDailyMax.toString());
    final thresholdController = TextEditingController(text: unit.moistureThreshold.toString());

    showDialog<void>(
      context: context,
      builder: (_) => AlertDialog(
        title: Text(unit.name),
        content: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              _DetailRow(label: 'Pump', value: unit.pumpName),
              _DetailRow(label: 'Sensor', value: unit.sensorName),
              _DetailRow(label: 'Mode', value: '${unit.pumpStatus}'),
              _DetailRow(label: 'Humidity', value: '${unit.soilHumidity.toStringAsFixed(1)}%'),
              _DetailRow(label: 'Threshold', value: '${unit.moistureThreshold.toStringAsFixed(1)}%'),
              _DetailRow(label: 'Today', value: '${unit.waterFlowDaily.toStringAsFixed(2)} L'),
              _DetailRow(label: 'Total', value: '${unit.waterFlowTotal.toStringAsFixed(2)} L'),
              const SizedBox(height: 20),
              _NumberField(controller: modeController, label: 'Change mode'),
              const SizedBox(height: 12),
              _NumberField(controller: maxLitersController, label: 'Max daily liters'),
              const SizedBox(height: 12),
              _NumberField(controller: thresholdController, label: 'Humidity threshold (%)'),
            ],
          ),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Close'),
          ),
          TextButton(
            onPressed: () => esp32.send('cmd: telem'),
            child: const Text('Refresh'),
          ),
          FilledButton(
            onPressed: () {
              esp32.send(
                'cmd: config, pump: ${unit.pumpName}, '
                'set_mode: ${modeController.text.trim()}, '
                'chg_dly_ltr: ${maxLitersController.text.trim()}, '
                'chg_moi_thr: ${thresholdController.text.trim()}',
              );
              Navigator.pop(context);
            },
            child: const Text('Send'),
          ),
        ],
      ),
    ).whenComplete(() {
      modeController.dispose();
      maxLitersController.dispose();
      thresholdController.dispose();
    });
  }

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Text('Irrigation Units', style: Theme.of(context).textTheme.headlineSmall),
        const SizedBox(height: 16),
        FilledButton.icon(
          onPressed: () => esp32.send('cmd: telem'),
          icon: const Icon(Icons.refresh),
          label: const Text('Refresh telemetry'),
        ),
        const SizedBox(height: 16),
        ValueListenableBuilder<List<IrrigationUnit>>(
          valueListenable: esp32.units,
          builder: (_, units, __) {
            if (units.isEmpty) {
              return const Text('No telemetry yet. Connect in Settings, then press Refresh telemetry.');
            }

            return Column(
              children: units.map((unit) {
                return Card(
                  margin: const EdgeInsets.only(bottom: 12),
                  child: ListTile(
                    leading: Icon(
                      unit.pumpStatus == 1 ? Icons.water_drop : Icons.water_drop_outlined,
                    ),
                    title: Text(unit.name, style: const TextStyle(fontWeight: FontWeight.bold)),
                    subtitle: Text(
                      'Pump ${unit.pumpName}: mode ${unit.pumpStatus}\n'
                      'Sensor ${unit.sensorName}: ${unit.soilHumidity.toStringAsFixed(1)}%\n'
                      'Flow today: ${unit.waterFlowDaily.toStringAsFixed(2)} L',
                    ),
                    isThreeLine: true,
                    trailing: const Icon(Icons.chevron_right),
                    onTap: () => _openDetails(context, unit),
                  ),
                );
              }).toList(),
            );
          },
        ),
        const SizedBox(height: 16),
        ValueListenableBuilder<String>(
          valueListenable: esp32.message,
          builder: (_, value, __) => Text('Last ESP32 message: $value'),
        ),
      ],
    );
  }
}

class _DetailRow extends StatelessWidget {
  final String label;
  final String value;

  const _DetailRow({required this.label, required this.value});

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 4),
      child: Text('$label: $value'),
    );
  }
}

class _NumberField extends StatelessWidget {
  final TextEditingController controller;
  final String label;

  const _NumberField({required this.controller, required this.label});

  @override
  Widget build(BuildContext context) {
    return TextField(
      controller: controller,
      keyboardType: const TextInputType.numberWithOptions(decimal: true),
      decoration: InputDecoration(
        labelText: label,
        border: const OutlineInputBorder(),
      ),
    );
  }
}

enum ChartRange { hour, day, week, month }

class PageCharts extends StatefulWidget {
  const PageCharts({super.key});

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

  double get maxX => selectedDuration.inMinutes.toDouble();

  List<ChartReading> _filteredReadings(List<ChartReading> readings) {
    final from = DateTime.now().subtract(selectedDuration);
    return readings.where((reading) => reading.time.isAfter(from)).toList();
  }

  List<FlSpot> _humiditySpots(List<ChartReading> readings, int sensorIndex) {
    final from = DateTime.now().subtract(selectedDuration);

    return readings.where((reading) => reading.humidities.length > sensorIndex).map((reading) {
      final x = reading.time.difference(from).inMinutes.toDouble();
      return FlSpot(x, reading.humidities[sensorIndex]);
    }).toList();
  }

  @override
  Widget build(BuildContext context) {
    final esp32 = Esp32Service.instance;

    return Padding(
      padding: const EdgeInsets.all(16),
      child: Column(
        children: [
          SegmentedButton<ChartRange>(
            segments: const [
              ButtonSegment(value: ChartRange.hour, label: Text('1h')),
              ButtonSegment(value: ChartRange.day, label: Text('1d')),
              ButtonSegment(value: ChartRange.week, label: Text('1w')),
              ButtonSegment(value: ChartRange.month, label: Text('1m')),
            ],
            selected: {selectedRange},
            onSelectionChanged: (value) => setState(() => selectedRange = value.first),
          ),
          const SizedBox(height: 20),
          Expanded(
            child: ValueListenableBuilder<List<ChartReading>>(
              valueListenable: esp32.readings,
              builder: (_, allReadings, __) {
                final readings = _filteredReadings(allReadings);

                if (readings.isEmpty) {
                  return const Center(child: Text('No chart data yet. Refresh telemetry first.'));
                }

                return LineChart(
                  LineChartData(
                    minY: 0,
                    maxY: 100,
                    minX: 0,
                    maxX: maxX,
                    gridData: const FlGridData(show: true),
                    titlesData: FlTitlesData(
                      topTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
                      rightTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
                      leftTitles: const AxisTitles(
                        sideTitles: SideTitles(showTitles: true, reservedSize: 40, interval: 20),
                      ),
                      bottomTitles: AxisTitles(
                        sideTitles: SideTitles(
                          showTitles: true,
                          reservedSize: 32,
                          interval: maxX / 4,
                          getTitlesWidget: (value, _) => Text(_bottomLabel(value)),
                        ),
                      ),
                    ),
                    lineBarsData: List.generate(4, (index) {
                      return LineChartBarData(
                        spots: _humiditySpots(readings, index),
                        isCurved: true,
                        barWidth: 3,
                        dotData: const FlDotData(show: false),
                      );
                    }),
                  ),
                );
              },
            ),
          ),
          const SizedBox(height: 12),
          const Text('Soil humidity over time'),
        ],
      ),
    );
  }

  String _bottomLabel(double value) {
    switch (selectedRange) {
      case ChartRange.hour:
        return '${value.toInt()}m';
      case ChartRange.day:
        return '${(value / 60).toInt()}h';
      case ChartRange.week:
      case ChartRange.month:
        return '${(value / 1440).toInt()}d';
    }
  }
}

class PageCamera extends StatelessWidget {
  const PageCamera({super.key});

  @override
  Widget build(BuildContext context) {
    return const Center(
      child: Text('Camera Page', style: TextStyle(fontSize: 30)),
    );
  }
}

class PageSettings extends StatefulWidget {
  const PageSettings({super.key});

  @override
  State<PageSettings> createState() => _PageSettingsState();
}

class _PageSettingsState extends State<PageSettings> {
  final esp32 = Esp32Service.instance;
  final ipController = TextEditingController(text: '10.69.106.208');

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
        Text('Wi-Fi ESP32 Connection', style: Theme.of(context).textTheme.headlineSmall),
        const SizedBox(height: 16),
        TextField(
          controller: ipController,
          decoration: const InputDecoration(
            labelText: 'ESP32 IP address',
            border: OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 16),
        FilledButton.icon(
          onPressed: () => esp32.connect(ipController.text),
          icon: const Icon(Icons.wifi),
          label: const Text('Connect'),
        ),
        const SizedBox(height: 12),
        OutlinedButton(
          onPressed: () => esp32.send('cmd: telem'),
          child: const Text('Request telemetry'),
        ),
        const SizedBox(height: 12),
        OutlinedButton(
          onPressed: esp32.disconnect,
          child: const Text('Disconnect'),
        ),
        const SizedBox(height: 24),
        ValueListenableBuilder<String>(
          valueListenable: esp32.status,
          builder: (_, value, __) => Text('Status: $value'),
        ),
        const SizedBox(height: 12),
        ValueListenableBuilder<String>(
          valueListenable: esp32.message,
          builder: (_, value, __) => Text('ESP32 says: $value'),
        ),
      ],
    );
  }
}