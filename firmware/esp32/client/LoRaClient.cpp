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
    struct Packet packet = LL2.popFromL3OutBuffer();
    if (packet.totalLength > HEADER_LENGTH)
    {
        struct Datagram datagram;
		memset(datagram.message, 0, DATAGRAM_MESSAGE);
        size_t len = packet.totalLength - HEADER_LENGTH;
        // parse out datagram
        memcpy(&datagram, packet.data, len);
        //memcpy(datagram.message, packet.data + DATAGRAM_HEADER, len - DATAGRAM_HEADER);
        server->transmit(this, datagram, len);
    }
}

void LoRaClient::receive(struct Datagram datagram, size_t len)
{
    unsigned char buf[len]; // = {'\0'};
    memcpy(buf, &datagram, sizeof(buf));

    //memcpy(buf, &datagram, DATAGRAM_HEADER);
    //memcpy(datagram.message, packet.data + DATAGRAM_HEADER, len - DATAGRAM_HEADER);

    LL2.sendToLayer2(buf, sizeof(buf));
}
