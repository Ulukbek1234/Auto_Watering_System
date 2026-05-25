import 'package:flutter/material.dart';
import 'dart:async';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

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



/*class PageSettings extends StatefulWidget {
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
}*/


class PageSettings extends StatefulWidget {
  const PageSettings({super.key});

  @override
  State<PageSettings> createState() => _PageSettingsState();
}

class _PageSettingsState extends State<PageSettings> {
  WebSocketChannel? channel;
  String status = "Not connected";
  String lastMessage = "";

  final TextEditingController ipController =
      TextEditingController(text: "10.150.65.208");

  void connectToEsp32() async {
    final ip = ipController.text.trim();

    try {
      setState(() {
        status = "Connecting...";
      });

      final ws = WebSocketChannel.connect(
        Uri.parse("ws://$ip:81"),
      );

      channel = ws;

      await ws.ready;

      setState(() {
        status = "Connected";
      });

      ws.stream.listen(
        (message) {
          if (!mounted) return;

          setState(() {
            lastMessage = message.toString();
          });
        },
        onError: (error) {
          if (!mounted) return;

          setState(() {
            status = "Connection error:\n$error";
          });

          channel = null;
        },
        onDone: () {
          if (!mounted) return;

          setState(() {
            status = "Disconnected";
          });

          channel = null;
        },
      );
    } catch (e) {
      setState(() {
        status = "Failed:\n$e";
      });

      channel = null;
    }
  }

  void sendCommand(String command) {
    try {
      if (channel == null) {
        setState(() {
          status = "Not connected";
        });
        return;
      }

      channel!.sink.add(command);

      setState(() {
        status = "Sent: $command";
      });
    } catch (e) {
      setState(() {
        status = "Send failed:\n$e";
      });
    }
  }

  void disconnect() {
    channel?.sink.close();
    channel = null;

    setState(() {
      status = "Disconnected";
    });
  }

  @override
  void dispose() {
    ipController.dispose();
    channel?.sink.close();
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

        Text(
          "Status: $status",
          style: const TextStyle(fontSize: 18),
        ),

        const SizedBox(height: 12),

        Text(
          "ESP32 says: $lastMessage",
          style: const TextStyle(fontSize: 18),
        ),
      ],
    );
  }
}