
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
        uint8_t* data; 
        message.getBytes(data, len);
        struct Datagram datagram = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        datagram.type = 'c';
        memcpy(datagram.message, data, len);
        len = len+DATAGRAM_HEADER;
        
        server->transmit(this, datagram, len);
    }
};

void StreamClient::receive(struct Datagram datagram, size_t len)
{
    String message;
    for(size_t i=0; i < len-DATAGRAM_HEADER; i++) {
        message += (char) datagram.message[i];
    }
    stream->print(message);
};
