#ifndef DISASTERSERVER_H
#define DISASTERSERVER_H

#include <Arduino.h>
#include <cstddef>

#include "DisasterClient.h"

class DisasterClient;

class DisasterServer
{
public:
    virtual DisasterClient* connect(DisasterClient *client) = 0;
    virtual void disconnect(DisasterClient *client) = 0;

    virtual void setup() = 0;
    virtual void loop() = 0;

    virtual void transmit(DisasterClient *client, String message) = 0;
};

#endif