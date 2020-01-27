#ifndef DISASTERCLIENT_H
#define DISASTERCLIENT_H

#include <Arduino.h>
#include <cstddef>

#include "DisasterServer.h"

class DisasterServer;

class DisasterClient
{
public:
    DisasterServer *server = NULL;

    virtual void setup(){};
    virtual void loop(){};
    virtual void receive(String message){};
};

#endif