#include "SerialComms.h"

SerialComms::SerialComms()
{
    Serial.begin(115200);
    Serial.println("Finally running");
}

String SerialComms::read()
{
    String input;
    input = serial->readStringUntil('\n');
    return input;
}

void SerialComms::write(String output)
{
    serial->println(output.c_str());
    serial->flush();
}

HardwareSerial* SerialComms::getSource()
{
    return serial;
}