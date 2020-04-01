
#include "StreamClient.h"

#define STREAM_ECHO

void StreamClient::setup()
{
    stream->setTimeout(0);
}

void StreamClient::loop()
{
    if (stream->available() > 0)
    {
        String message = stream->readString();
        size_t len = message.length();
        uint8_t data[len];
        memcpy(&data, message.c_str(), len);
        struct Datagram datagram = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        memset(datagram.message, 0, 233);
        datagram.type = 'c';
        memcpy(datagram.message, data, len);
        len = len + DATAGRAM_HEADER;

        server->transmit(this, datagram, len);
    }
};

void StreamClient::receive(struct Datagram datagram, size_t len)
{
    stream->write(datagram.message, len - DATAGRAM_HEADER);
};
