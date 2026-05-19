import 'package:flutter/material.dart';

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
        title: const Text('BottomNavigationBar Sample'),
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

class PageSettings extends StatelessWidget {
  /*
    ToDo
    - BLE connection
    - WiFi credential input
    - WiFi connection
    - User profile
    - Delete Data
    - Other settings stuff
  */
  @override
  Widget build(BuildContext context) {
    return const Center(
      child: Text(
        'Settings Page',
        style: TextStyle(fontSize: 30),
      ),
    );
  }
}