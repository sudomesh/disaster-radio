#ifndef DISASTERRADIO_H
#define DISASTERRADIO_H

#ifndef SIM
#include <Arduino.h>
#endif
#include <set>
#include <list>
#include <cstddef>

#include "../DisasterClient.h"
#include "../DisasterServer.h"
#include "../DisasterMiddleware.h"

class DisasterRadio : public DisasterServer
{
    std::list<DisasterClient *> connected;

public:
    std::set<DisasterClient *> clients;

    // NOTE: multimethod so we can return the correct parameter type to chain on
    DisasterMiddleware *connect(DisasterMiddleware *client);
    DisasterClient *connect(DisasterClient *client);

    void disconnect(DisasterClient *client);

    void setup();
    void loop();

    void transmit(DisasterClient *client, struct Datagram datagram, size_t len);
};

#endif
