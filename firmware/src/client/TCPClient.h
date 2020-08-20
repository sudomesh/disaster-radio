#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "../DisasterClient.h"
#include "../DisasterServer.h"

#include <AsyncTCP.h>

class TCPClient : public DisasterClient
{
    AsyncClient *client;

public:
    String buffer = "";

    TCPClient(AsyncClient *c)
        : client{c} {};

    // bool interactive() { return true; };

    void receive(struct Datagram datagram, size_t len);

    void handleData(void *data, size_t len);
    void handleDisconnect();
    void handleError(int8_t error);
    void handleTimeout(uint32_t time);

    static void startServer(AsyncServer *server, void (*callback)(TCPClient *));
};

#endif
