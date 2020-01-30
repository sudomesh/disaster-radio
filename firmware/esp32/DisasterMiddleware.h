#ifndef DISASTERMIDDLEWARE_H
#define DISASTERMIDDLEWARE_H

#include "DisasterClient.h"
#include "DisasterServer.h"

class DisasterMiddleware : public DisasterClient, public DisasterServer
{
public:
    DisasterClient *client = NULL;

    // NOTE: multimethod so we can return the correct parameter type to chain on
    virtual DisasterMiddleware *connect(DisasterMiddleware *client);
    virtual DisasterClient *connect(DisasterClient *client);

    virtual void disconnect(DisasterClient *client);

    virtual void setup();
    virtual void loop();

    virtual void receive(String message);
    virtual void transmit(DisasterClient *client, String message);
};

#endif