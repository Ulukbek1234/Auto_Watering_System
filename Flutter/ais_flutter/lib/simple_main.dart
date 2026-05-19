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
    Page1(),
    Page2(),
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

class Page1 extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return const Center(
      child: Text(
        'Page 1',
        style: TextStyle(fontSize: 30),
      ),
    );
  }
}

class Page2 extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return const Center(
      child: Text(
        'Page 2',
        style: TextStyle(fontSize: 30),
      ),
    );
  }
}

class PageCamera extends StatelessWidget {
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