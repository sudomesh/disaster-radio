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
        struct Datagram datagram;
		memset(datagram.message, 0, DATAGRAM_MESSAGE);
        size_t len = packet.totalLength - HEADER_LENGTH;
        // parse out datagram
        memcpy(&datagram, packet.data, len);
        server->transmit(this, datagram, len);
    }
}

void LoRaClient::receive(struct Datagram datagram, size_t len)
{
    uint8_t buf[len] = {'\0'};
    memcpy(buf, &datagram, len);
    LL2.writeData(buf, sizeof(buf));
}
