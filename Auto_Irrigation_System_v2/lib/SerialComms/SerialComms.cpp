#include "SerialComms.h"

SerialComms::SerialComms()
{
    Serial.begin(115200);
    Serial.println("Finally running");
}

String SerialComms::read()
{
    String input = "";
    if(Serial.available())
    {
        input = Serial.readStringUntil('\n');
    }
    return input;
}

void SerialComms::write(String output)
{
    Serial.println(output.c_str());
    Serial.flush();
}

HardwareSerial* SerialComms::getSource()
{
    return &Serial;
}