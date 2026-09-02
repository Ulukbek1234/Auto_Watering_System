import 'dart:async';

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:multicast_dns/multicast_dns.dart';
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

/// Device commands are kept in one place to preserve the firmware protocol.
class Esp32Commands {
  Esp32Commands._();

  static const telemetry = 'cmd: telem';
  static const updateFirmware = 'cmd: updt_firm';
  static const saveSettings = 'cmd: sav_eep';
  static const resetSettings = 'cmd: rst_eep';
  static const restart = 'cmd: rstrt';

  static String calibrate(String sensorPin, String type) =>
      'cmd: cali_snsr, soil_pin: $sensorPin, cali_type: $type';

  static String irrigate(String pumpPin, String amount) =>
      'cmd: man_irr, pump: $pumpPin, amount: ${amount.trim()}, ';

  static String configure({
    required String pumpPin,
    required String mode,
    required String maxDailyLiters,
    required String moistureThreshold,
  }) =>
      'cmd: config, pump: $pumpPin, '
      'set_mode: ${mode.trim()}, '
      'chg_dly_ltr: ${maxDailyLiters.trim()}, '
      'chg_moi_thr: ${moistureThreshold.trim()}';
}

class WifiProvisioningService {
  const WifiProvisioningService();

  Future<String> provision({
    required String ssid,
    required String password,
  }) async {
    final response = await http.post(
      Uri.parse('http://192.168.4.1/wifi'),
      body: {'ssid': ssid.trim(), 'pass': password},
    ).timeout(const Duration(seconds: 10));

    return response.statusCode == 200
        ? 'Credentials sent. ESP32 will connect to your home Wi-Fi and restart.'
        : 'Failed: ${response.body}';
  }
}

class Esp32Service {
  Esp32Service._();
  static final Esp32Service instance = Esp32Service._();

  static const _pumpPins = ['16', '17', '18', '19'];
  static const telemetryInterval = Duration(minutes: 1);
  static const maxHistoryItems = 1000;

  static const _sensorPins = ['32', '33', '34', '35'];

  WebSocketChannel? _channel;
  Timer? _telemetryTimer;

  final ValueNotifier<List<IrrigationUnit>> units = ValueNotifier([]);
  final ValueNotifier<List<ChartReading>> readings = ValueNotifier([]);
  final ValueNotifier<String> status = ValueNotifier('Disconnected');
  final ValueNotifier<String> message = ValueNotifier('');
  final ValueNotifier<String?> firmwareVersion = ValueNotifier('0.0.0');
  final ValueNotifier<String> time = ValueNotifier('');

  bool get connected => _channel != null;

  Future<String?> _resolveEsp32Host(String host) async {
    final client = MDnsClient();
    await client.start();

    try {
      await for (final record in client.lookup<IPAddressResourceRecord>(
        ResourceRecordQuery.addressIPv4(host),
      )) {
        return record.address.address;
      }
    } finally {
      client.stop();
    }

    return null;
  }

  Future<void> connect(String ip) async {
    var trimmedIp = ip.trim();

    if (trimmedIp.isEmpty) {
      trimmedIp = 'esp32.local';
    }

    if (_channel != null) return;

    try {
      status.value = 'Resolving $trimmedIp...';

      if (trimmedIp.endsWith('.local')) {
        final resolvedIp = await _resolveEsp32Host(trimmedIp);

        if (resolvedIp == null) {
          status.value = 'Could not find $trimmedIp';
          return;
        }

        trimmedIp = resolvedIp;
      }

      status.value = 'Connecting...';

      final ws = WebSocketChannel.connect(
        Uri.parse('ws://$trimmedIp:81/'),
      );

      await ws.ready;

      _channel = ws;
      status.value = 'Connected to $trimmedIp';

      send(Esp32Commands.telemetry);
      _startTelemetryPolling();

      ws.stream.listen(
        _handleMessage,
        onError: (Object error) {
          _channel = null;
          _stopTelemetryPolling();
          status.value = 'Error: $error';
        },
        onDone: () {
          _channel = null;
          _stopTelemetryPolling();
          status.value = 'Disconnected';
        },
      );
    } catch (error) {
      _channel = null;
      _stopTelemetryPolling();
      status.value = 'Failed: $error';
    }
  }

  void _startTelemetryPolling() {
    _telemetryTimer?.cancel();

    _telemetryTimer = Timer.periodic(
      telemetryInterval,
      (_) {
        if (_channel != null) {
          send(Esp32Commands.telemetry);
        }
      },
    );
  }

  void _stopTelemetryPolling() {
    _telemetryTimer?.cancel();
    _telemetryTimer = null;
  }

  void _handleMessage(dynamic data) {
    final text = data.toString();
    message.value = text;

    final parsed = parseTelemetryText(text);
    if (parsed.isEmpty) return;

    if (parsed.containsKey('firmware_version')) {
      firmwareVersion.value = parsed['firmware_version'];
    }

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
      time: parseTime(parsed['time']),
      humidities: _sensorPins
          .map((pin) => _doubleValue(parsed, 'moisture_percent_$pin'))
          .toList(),
    );

    final history = [...readings.value, nextReading];
    readings.value = history.length > maxHistoryItems
        ? history.sublist(history.length - maxHistoryItems)
        : history;
  }

  DateTime parseTime(String? inputTime) {
    if (inputTime != null && inputTime.isNotEmpty) {
      time.value = inputTime.replaceAll(';', ':');
    }
    return DateTime.parse(time.value);
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
    _stopTelemetryPolling();
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
          NavigationDestination(icon: Icon(Icons.home), label: 'Home'),
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

  void _openCalibrate(BuildContext context, IrrigationUnit unit) {
    showDialog<void>(
      context: context,
      builder: (dialogContext) => AlertDialog(
        title: Text(unit.name),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(dialogContext).pop(),
            child: const Text('Close'),
          ),
          for (final type in ['air', 'water'])
            TextButton(
              onPressed: () {
                esp32.send(Esp32Commands.calibrate(unit.sensorName, type));
                Navigator.of(dialogContext).pop();
              },
              child: Text(type == 'air' ? 'Air' : 'Water'),
            ),
        ],
      ),
    );
  }

  void _openSplash(BuildContext context, IrrigationUnit unit) {
    showDialog<void>(
      context: context,
      builder: (_) => _SplashDialog(unit: unit, service: esp32),
    );
  }

  void _openConfig(BuildContext context, IrrigationUnit unit) {
    showDialog<void>(
      context: context,
      builder: (_) => _ConfigDialog(unit: unit, service: esp32),
    );
  }


  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Text('Irrigation Units', style: Theme.of(context).textTheme.headlineSmall),
        const SizedBox(height: 16),
        FilledButton.icon(
          onPressed: () => esp32.send(Esp32Commands.telemetry),
          icon: const Icon(Icons.refresh),
          label: const Text('Refresh telemetry'),
        ),
        const SizedBox(height: 16),
        ValueListenableBuilder<List<IrrigationUnit>>(
          valueListenable: esp32.units,
          builder: (_, units, __) {
            final visibleUnits = units.isEmpty
                ? const [
                    IrrigationUnit(
                      name: 'Pump -1',
                      pumpName: '-1',
                      sensorName: '-1',
                      pumpStatus: -1,
                      soilHumidity: 0,
                      moistureThreshold: 0,
                      waterFlowDaily: 0,
                      waterFlowDailyMax: 0,
                      waterFlowTotal: 0,
                    ),
                  ]
                : units;

            return Column(
              children: visibleUnits.map((unit) {
                return Card(
                  margin: const EdgeInsets.only(bottom: 12),
                  child: ListTile(
                    leading: Icon(
                      unit.pumpStatus == 3
                          ? Icons.water_drop
                          : Icons.water_drop_outlined,
                    ),
                    title: Text(
                      unit.name,
                      style: const TextStyle(fontWeight: FontWeight.bold),
                    ),
                    subtitle: Text(
                      'Pump ${unit.pumpName}: mode ${unit.pumpStatus}\n'
                      'Sensor ${unit.sensorName}: ${unit.soilHumidity.toStringAsFixed(1)}%\n'
                      'Flow today: ${unit.waterFlowDaily.toStringAsFixed(2)} L',
                    ),
                    isThreeLine: true,
                    trailing: PopupMenuButton<String>(
                      onSelected: (cmd) {
                        switch (cmd) {
                          case 'config':
                            _openConfig(context, unit);
                            break;
                          case 'splash':
                            _openSplash(context, unit);
                            break;
                          case 'calibrate':
                            _openCalibrate(context, unit);
                            break;
                        }
                      },
                      itemBuilder: (_) => const [
                        PopupMenuItem(value: 'config', child: Text('Config')),
                        PopupMenuItem(value: 'splash', child: Text('Splash')),
                        PopupMenuItem(value: 'calibrate', child: Text('Calibrate')),
                      ],
                    ),
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

class _SplashDialog extends StatefulWidget {
  const _SplashDialog({required this.unit, required this.service});

  final IrrigationUnit unit;
  final Esp32Service service;

  @override
  State<_SplashDialog> createState() => _SplashDialogState();
}

class _SplashDialogState extends State<_SplashDialog> {
  late final TextEditingController amountController;

  @override
  void initState() {
    super.initState();
    amountController = TextEditingController(text: '0.1');
  }

  @override
  void dispose() {
    amountController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text(widget.unit.name),
      content: SingleChildScrollView(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            _DetailRow(label: 'Pump', value: widget.unit.pumpName),
            _DetailRow(label: 'Today', value: '${widget.unit.waterFlowDaily.toStringAsFixed(2)} L'),
            _DetailRow(label: 'Total', value: '${widget.unit.waterFlowTotal.toStringAsFixed(2)} L'),
            const SizedBox(height: 20),
            _NumberField(controller: amountController, label: 'Splash Amount'),
            const SizedBox(height: 12),
          ],
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.of(context).pop(),
          child: const Text('Close'),
        ),
        FilledButton(
          onPressed: () {
            FocusScope.of(context).unfocus();
            widget.service.send(
              Esp32Commands.irrigate(widget.unit.pumpName, amountController.text),
            );
          },
          child: const Text('Send'),
        ),
      ],
    );
  }
}

class _ConfigDialog extends StatefulWidget {
  const _ConfigDialog({required this.unit, required this.service});

  final IrrigationUnit unit;
  final Esp32Service service;

  @override
  State<_ConfigDialog> createState() => _ConfigDialogState();
}

class _ConfigDialogState extends State<_ConfigDialog> {
  late final TextEditingController modeController;
  late final TextEditingController maxLitersController;
  late final TextEditingController thresholdController;

  @override
  void initState() {
    super.initState();
    modeController = TextEditingController(text: widget.unit.pumpStatus.toString());
    maxLitersController = TextEditingController(text: widget.unit.waterFlowDailyMax.toString());
    thresholdController = TextEditingController(text: widget.unit.moistureThreshold.toString());
  }

  @override
  void dispose() {
    modeController.dispose();
    maxLitersController.dispose();
    thresholdController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text(widget.unit.name),
      content: SingleChildScrollView(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            _DetailRow(label: 'Pump', value: widget.unit.pumpName),
            _DetailRow(label: 'Sensor', value: widget.unit.sensorName),
            _DetailRow(label: 'Mode', value: '${widget.unit.pumpStatus}'),
            _DetailRow(label: 'Humidity', value: '${widget.unit.soilHumidity.toStringAsFixed(1)}%'),
            _DetailRow(label: 'Threshold', value: '${widget.unit.moistureThreshold.toStringAsFixed(1)}%'),
            _DetailRow(label: 'Today', value: '${widget.unit.waterFlowDaily.toStringAsFixed(2)} L'),
            _DetailRow(label: 'Total', value: '${widget.unit.waterFlowTotal.toStringAsFixed(2)} L'),
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
          onPressed: () => Navigator.of(context).pop(),
          child: const Text('Close'),
        ),
        FilledButton(
          onPressed: () {
            FocusScope.of(context).unfocus();
            widget.service.send(
              Esp32Commands.configure(
                pumpPin: widget.unit.pumpName,
                mode: modeController.text,
                maxDailyLiters: maxLitersController.text,
                moistureThreshold: thresholdController.text,
              ),
            );
  
            Navigator.of(context).pop();
          },
          child: const Text('Send'),
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

enum ChartRange {
  hour(Duration(hours: 1)),
  day(Duration(days: 1)),
  week(Duration(days: 7)),
  month(Duration(days: 30));

  const ChartRange(this.duration);

  final Duration duration;
}

class PageCharts extends StatefulWidget {
  const PageCharts({super.key});

  @override
  State<PageCharts> createState() => _PageChartsState();
}

class _PageChartsState extends State<PageCharts> {
  ChartRange selectedRange = ChartRange.day;

  Duration get selectedDuration => selectedRange.duration;

  double get maxX => selectedDuration.inMinutes.toDouble();

  List<ChartReading> _filteredReadings(
    List<ChartReading> readings,
    DateTime from,
  ) {
    return readings.where((reading) => reading.time.isAfter(from)).toList();
  }

  List<FlSpot> _humiditySpots(
    List<ChartReading> readings,
    int sensorIndex,
    DateTime from,
  ) {
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
                final from = DateTime.now().subtract(selectedDuration);
                final readings = _filteredReadings(allReadings, from);

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
                        spots: _humiditySpots(readings, index, from),
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

  final ipController = TextEditingController(text: 'esp32.local');

  final wifiSsidController = TextEditingController();
  final wifiPasswordController = TextEditingController();

  final _wifiProvisioning = const WifiProvisioningService();

  bool provisioning = false;
  String provisioningStatus = '';

  @override
  void initState() {
    super.initState();

    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (mounted) {
        esp32.connect(ipController.text.trim());
      }
    });
  }

  @override
  void dispose() {
    ipController.dispose();
    wifiSsidController.dispose();
    wifiPasswordController.dispose();
    super.dispose();
  }

  Future<void> provisionWifi() async {
    if (provisioning) return;

    setState(() {
      provisioning = true;
      provisioningStatus = 'Sending Wi-Fi credentials...';
    });

    String result;
    try {
      result = await _wifiProvisioning.provision(
        ssid: wifiSsidController.text,
        password: wifiPasswordController.text,
      );
    } catch (_) {
      result =
          'Could not reach ESP32. Connect your phone to ESP32-Setup Wi-Fi first.';
    }

    if (!mounted) return;
    setState(() {
      provisioningStatus = result;
      provisioning = false;
    });
  }

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Text(
          'Wi-Fi ESP32 Connection',
          style: Theme.of(context).textTheme.headlineSmall,
        ),

        ValueListenableBuilder<String?>(
          valueListenable: esp32.firmwareVersion,
          builder: (_, value, __) => Text('Firmware Version: $value'),
        ),

        const SizedBox(height: 16),

        Card(
          child: Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                Text(
                  'Setup ESP32 Wi-Fi',
                  style: Theme.of(context).textTheme.titleMedium,
                ),
                const SizedBox(height: 8),
                const Text(
                  'First connect your phone to the ESP32-Setup Wi-Fi network, then enter your home Wi-Fi credentials below.',
                ),
                const SizedBox(height: 16),
                TextField(
                  controller: wifiSsidController,
                  decoration: const InputDecoration(
                    labelText: 'Home Wi-Fi SSID',
                    border: OutlineInputBorder(),
                  ),
                ),
                const SizedBox(height: 12),
                TextField(
                  controller: wifiPasswordController,
                  obscureText: true,
                  decoration: const InputDecoration(
                    labelText: 'Home Wi-Fi password',
                    border: OutlineInputBorder(),
                  ),
                ),
                const SizedBox(height: 12),
                FilledButton.icon(
                  onPressed: provisioning ? null : provisionWifi,
                  icon: provisioning
                      ? const SizedBox(
                          width: 18,
                          height: 18,
                          child: CircularProgressIndicator(strokeWidth: 2),
                        )
                      : const Icon(Icons.settings_input_antenna),
                  label: const Text('Send Wi-Fi to ESP32'),
                ),
                if (provisioningStatus.isNotEmpty) ...[
                  const SizedBox(height: 12),
                  Text(provisioningStatus),
                ],
              ],
            ),
          ),
        ),

        const SizedBox(height: 24),

        TextField(
          controller: ipController,
          decoration: const InputDecoration(
            labelText: 'ESP32 IP address',
            border: OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 16),
        FilledButton.icon(
          onPressed: () => esp32.connect(ipController.text.trim()),
          icon: const Icon(Icons.wifi),
          label: const Text('Reconnect'),
        ),
        const SizedBox(height: 12),
        OutlinedButton(
          onPressed: esp32.disconnect,
          child: const Text('Disconnect'),
        ),
        const SizedBox(height: 12),
        for (final action in (const <String, String>{
          'Request telemetry': Esp32Commands.telemetry,
          'Update Firmware': Esp32Commands.updateFirmware,
          'Save to EEPROM': Esp32Commands.saveSettings,
          'Reset EEPROM': Esp32Commands.resetSettings,
          'Restart': Esp32Commands.restart,
        }).entries) ...[
          OutlinedButton(
            onPressed: () => esp32.send(action.value),
            child: Text(action.key),
          ),
          const SizedBox(height: 12),
        ],

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
