#ifndef STREAMCLIENT_H
#define STREAMCLIENT_H

#include "../DisasterClient.h"
#include "../DisasterServer.h"

class StreamClient : public DisasterClient
{
  Stream *stream;
  String buffer = "";

public:
  StreamClient(Stream *s)
      : stream{s} {};

  void setup();
  void loop();

  void receive(struct Datagram datagram, size_t len);

private:
  char input[DATAGRAM_MESSAGE];
  int inputLength;

};

#endif
