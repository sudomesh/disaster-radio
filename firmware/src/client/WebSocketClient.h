#ifndef WebSocketCLIENT_H
#define WebSocketCLIENT_H

#include "../DisasterClient.h"
#include "../DisasterServer.h"

#include <AsyncWebSocket.h>

class WebSocketClient : public DisasterClient
{
    AsyncWebSocketClient *client;

public:
    WebSocketClient(AsyncWebSocketClient *c)
        : client{c} {};

    void receive(struct Datagram datagram, size_t len);

    // bool interactive() { return true; };

    void handleDisconnect();
    void handleData(void *data, size_t len);
    void handleError(uint16_t code, const char *message);

    static void startServer(AsyncWebSocket *ws, void (*callback)(WebSocketClient *));
};

#endif
