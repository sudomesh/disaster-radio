
#include "StreamClient.h"
#include <LoRaLayer2.h>

#define STREAM_ECHO

void StreamClient::setup()
{
  stream->setTimeout(0);
  inputLength = 0;
}

void StreamClient::loop()
{
  if (stream->available() > 0)
    {
      String message = stream->readString();
      size_t len = message.length();
      uint8_t data[len];
      memcpy(&data, message.c_str(), len);

      char p = (char)data[0];

      stream->write(p);

      if(p == '\b') // backspace
      {
        if(inputLength > 0){
          input[inputLength] = '\0';
          inputLength--;
          stream->write(" \b");
        }
      }
      else if(p == 0x7f ) // delete
      {
        if(inputLength > 0){
          input[inputLength] = '\0';
          inputLength--;
          stream->write("\b \b");
        }
      }
      else if(p == '\r') // enter key
      {
        stream->write("\n");
        input[inputLength] = '\r';
        inputLength++;
        input[inputLength] = '\n';
        inputLength++;

        struct Datagram datagram;
        memcpy(datagram.destination, BROADCAST, ADDR_LENGTH);
        datagram.type = 'c';
        memcpy(datagram.message, input, inputLength);
        server->transmit(this, datagram, inputLength + DATAGRAM_HEADER);

        memset(input, 0, DATAGRAM_MESSAGE);
        inputLength = 0;
      }
      else
      {
        input[inputLength] = p;
        inputLength++;
      }
    }
};

void StreamClient::receive(struct Datagram datagram, size_t len)
{
  stream->write(datagram.message, len - DATAGRAM_HEADER);
};
