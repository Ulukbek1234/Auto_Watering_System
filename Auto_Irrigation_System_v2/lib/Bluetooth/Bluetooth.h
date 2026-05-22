#ifndef BLUETOOTH_H
#define BLUETOOTH_H
#pragma once
#include "Arduino.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


class Bluetooth
{
private:
    BluetoothSerial SerialBT;
public:
    Bluetooth();
    String read();


};

#endif