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
    struct Packet packet = LL2.popFromWSBuffer();
    size_t len = packet.totalLength - HEADER_LENGTH;
    if (packet.totalLength > HEADER_LENGTH && len > 0)
    {
        uint16_t msg_id;
        char msg[len - 2 + 1] = {'\0'};

        // parse out message and message id
        memcpy(&msg_id, packet.data, 2);
        memcpy(msg, packet.data + 2, len - 2);

        server->transmit(this, String(msg));
    }
}

void LoRaClient::receive(String message)
{
    // TODO: msg id? defaulting to 0 for now
    uint16_t msg_id = 0;

    unsigned char buf[2 + message.length() + 1] = {'\0'};
    memcpy(buf, &msg_id, 2);
    message.getBytes(buf + 2, message.length() + 1);

    uint8_t destination[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t msg_type = 'c';
    LL2.sendToLayer2(destination, msg_type, buf, sizeof(buf));
}
