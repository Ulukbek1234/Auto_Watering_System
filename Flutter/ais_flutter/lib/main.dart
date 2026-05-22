import 'package:flutter/material.dart';
import 'dart:async';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

void main() => runApp(const BottomNavigationBarExampleApp());

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
  /*
    ToDo
    - General information
      - Environmental
      - See active pumps
    - Add pumps
    - Pumps, when pressed 
      - see current telemetry
      - pump configurations
  */
  @override
  Widget build(BuildContext context) {
    return const Center(
      child: Text(
        'Page Home',
        style: TextStyle(fontSize: 30),
      ),
    );
  }
}

class PageCharts extends StatelessWidget {
  /*
    ToDo
    - See humidity 
    - See water flow
    - Different time interval
    - y from 0 - 100
   */
  
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
  /*
    ToDo
    - live feed
    - timelapse?
  */
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
  /* ToDo 
    - BLE connection 
    - WiFi credential input 
    - WiFi connection 
    - User profile 
    - Delete Data 
    - Other settings stuff 
  */
  const PageSettings({super.key});

  @override
  State<PageSettings> createState() => _PageSettingsState();
}

class _PageSettingsState extends State<PageSettings> {
  final List<ScanResult> _devices = [];
  BluetoothDevice? _connectedDevice;
  bool _isScanning = false;

  StreamSubscription<List<ScanResult>>? _scanSubscription;

  @override
  void initState() {
    super.initState();
    _requestPermissions();
    _listenToScanResults();
  }

  Future<void> _requestPermissions() async {
    await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.location,
    ].request();
  }

  void _listenToScanResults() {
    _scanSubscription = FlutterBluePlus.scanResults.listen((results) {
      if (!mounted) return;

      setState(() {
        _devices
          ..clear()
          ..addAll(results);
      });
    });
  }

  Future<void> _startScan() async {
    setState(() {
      _devices.clear();
      _isScanning = true;
    });

    try {
      await FlutterBluePlus.startScan(timeout: const Duration(seconds: 8));
    } catch (e) {
      _showMessage('Scan failed: $e');
    }

    if (mounted) {
      setState(() => _isScanning = false);
    }
  }

  Future<void> _stopScan() async {
    await FlutterBluePlus.stopScan();

    if (mounted) {
      setState(() => _isScanning = false);
    }
  }

  Future<void> _connectToDevice(BluetoothDevice device) async {
    try {
      await FlutterBluePlus.stopScan();

      await device.connect(
        timeout: const Duration(seconds: 10),
        autoConnect: false,
      );

      if (!mounted) return;

      setState(() {
        _connectedDevice = device;
        _isScanning = false;
      });

      _showMessage('Connected to ${_deviceName(device)}');
    } catch (e) {
      _showMessage('Connection failed: $e');
    }
  }

  Future<void> _disconnectDevice() async {
    final device = _connectedDevice;
    if (device == null) return;

    await device.disconnect();

    if (mounted) {
      setState(() => _connectedDevice = null);
    }

    _showMessage('Disconnected');
  }

  String _deviceName(BluetoothDevice device) {
    return device.platformName.isNotEmpty
        ? device.platformName
        : 'Unknown Device';
  }

  void _showMessage(String message) {
    if (!mounted) return;

    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message)),
    );
  }

  @override
  void dispose() {
    _scanSubscription?.cancel();
    FlutterBluePlus.stopScan();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final connectedDevice = _connectedDevice;

    return Scaffold(
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          const Text(
            'Bluetooth Connection',
            style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
          ),

          const SizedBox(height: 16),

          if (connectedDevice != null)
            Card(
              child: ListTile(
                leading: const Icon(Icons.bluetooth_connected),
                title: Text('Connected: ${_deviceName(connectedDevice)}'),
                subtitle: Text(connectedDevice.remoteId.str),
                trailing: TextButton(
                  onPressed: _disconnectDevice,
                  child: const Text('Disconnect'),
                ),
              ),
            ),

          const SizedBox(height: 12),

          ElevatedButton.icon(
            icon: Icon(_isScanning ? Icons.stop : Icons.search),
            label: Text(_isScanning ? 'Stop Scan' : 'Scan Devices'),
            onPressed: _isScanning ? _stopScan : _startScan,
          ),

          const SizedBox(height: 16),

          if (_devices.isEmpty)
            const Center(
              child: Padding(
                padding: EdgeInsets.all(24),
                child: Text('No Bluetooth devices found'),
              ),
            )
          else
            ..._devices.map((result) {
              final device = result.device;

              return Card(
                child: ListTile(
                  leading: const Icon(Icons.bluetooth),
                  title: Text(_deviceName(device)),
                  subtitle: Text(
                    '${device.remoteId.str}\nRSSI: ${result.rssi}',
                  ),
                  isThreeLine: true,
                  trailing: ElevatedButton(
                    onPressed: () => _connectToDevice(device),
                    child: const Text('Connect'),
                  ),
                ),
              );
            }),
        ],
      ),
    );
  }
}