#ifndef LORACLIENT_H
#define LORACLIENT_H

#include "../DisasterClient.h"
#include "../DisasterServer.h"

#include <LoRaLayer2.h>

class LoRaClient : public DisasterClient
{
public:
    LoRaClient(LL2Class *Layer2)
      : LL2{Layer2} {};

    bool init();
    void loop();

    void receive(struct Datagram datagram, size_t len);

private:
    LL2Class *LL2;
};

#endif
