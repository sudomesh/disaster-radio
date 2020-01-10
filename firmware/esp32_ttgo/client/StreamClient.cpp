
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
        server->transmit(this, stream->readString());
    }
};

void StreamClient::receive(String message)
{
    stream->print(message);
};
