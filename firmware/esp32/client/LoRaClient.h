#ifndef LORACLIENT_H
#define LORACLIENT_H

#include "../DisasterClient.h"
#include "../DisasterServer.h"

#include <Layer1.h>
#include <LoRaLayer2.h>

class LoRaClient : public DisasterClient
{
public:
    LoRaClient(){};

    bool init();
    void loop();

    void receive(struct Datagram datagram, size_t len);
};

#endif
