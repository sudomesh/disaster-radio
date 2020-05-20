#include "LoRaClient.h"

bool LoRaClient::init()
{
    LL2.init();             // initialize Layer2
    LL2.setInterval(10000); // set to zero to disable routing packets
    return true;
}

void LoRaClient::loop()
{
    LL2.daemon();
    struct Packet packet = LL2.readData();
    if (packet.totalLength > HEADER_LENGTH)
    {
        server->transmit(this, packet.datagram, packet.totalLength - HEADER_LENGTH);
    }
}

void LoRaClient::receive(struct Datagram datagram, size_t len)
{
    LL2.writeData(datagram, len);
}
