#include "Comms.h"

Comms::Comms() : serial_comms(), web()
{
    active_types[0] = USB_SERIAL_COMMS;
    active_types[1] = WEB_SOCKET_COMMS;
}

String Comms::read(COMMS_TYPE type)
{
    switch (type)
    {
    case USB_SERIAL_COMMS:
        return serial_comms.read();
    case BLUETOOTH_COMMS:
        return bluetooth->read();
    case WEB_SOCKET_COMMS:
        return web.read();
    default:
        return "";
        break;
    }
}

void Comms::write(String output, COMMS_TYPE type)
{
    switch (type)
    {
    case USB_SERIAL_COMMS:
        serial_comms.write(output);
        break;    
    case BLUETOOTH_COMMS:
        // bluetooth->write();
        break;    
    case WEB_SOCKET_COMMS:
        web.write(output);
        break;    
    default:
        DEBUG_PRINTLN("ERROR: wrong output type");
        break;
    }
}

void Comms::updateFirmware()
{
    web.checkFirmwareVersion();
    web.updateFirmware();
}