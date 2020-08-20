#ifndef DISASTERCLIENT_H
#define DISASTERCLIENT_H

#ifndef SIM
#include <Arduino.h>
#endif
#include <cstddef>

#include "DisasterServer.h"

class DisasterServer;

class DisasterClient
{
public:
    DisasterServer *server = NULL;

    virtual void setup(){};
    virtual void loop(){};
    virtual void receive(struct Datagram datagram, size_t len){};
};

#endif
