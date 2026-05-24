#ifndef INTERFACE_H
#define INTERFACE_H
#pragma once

#include "Comms/CommsInterface.h"

class Interface 
{
private:
    CommsInterface comms;
public:
    Interface();
};

#endif