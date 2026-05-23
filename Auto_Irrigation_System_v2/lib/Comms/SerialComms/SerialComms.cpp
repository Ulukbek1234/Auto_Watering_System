#include "SerialComms.h"

SerialComms::SerialComms()
{

}

void SerialComms::initComms(CommsType init_serial_type)
{
    // Construct SerialComms
    switch (init_serial_type)
    {
    case USB_SERIAL_COMMS:
        Serial.begin(9600); // TODO make first to initialize (to remove Serial in setup function)
        serial_type = USB_SERIAL_COMMS;
        //serial = &Serial;
        break;
    case ESP32_SERIAL_COMMS:
        Serial.begin(115200);
        serial_type = ESP32_SERIAL_COMMS;
        //serial = &Serial;
        break;
    default:
        break;
    }
}

String SerialComms::read()
{
    String input;
    //input = serial->readStringUntil('\n');
    return input;
}

void SerialComms::write(String output)
{
    //serial->println(output.c_str());
    //serial->flush();
}

HardwareSerial* SerialComms::getSource()
{
    //return serial;
}