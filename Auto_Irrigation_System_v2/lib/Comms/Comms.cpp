#include "Comms.h"

Comms::Comms() : serial_comms(), web()
{
    active_types[0] = USB_SERIAL_COMMS;
    active_types[1] = WEB_SOCKET_COMMS;
}

String Comms::read(COMMS_TYPE comms_type)
{
    switch (comms_type)
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

