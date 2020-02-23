#ifndef CONSOLE_H
#define CONSOLE_H

#include "../DisasterMiddleware.h"
#include "../DisasterClient.h"
#include "../DisasterHistory.h"

class Console : public DisasterMiddleware
{
    String username = "";
    String buffer = "";
    DisasterHistory *history = NULL;

public:
    Console(DisasterHistory *h = NULL)
        : DisasterMiddleware(), history(h){};

    void setup();

    void transmit(DisasterClient *client, struct Datagram datagram, size_t len);
    void receive(struct Datagram datagram, size_t len);

private:
    void processLine(char *message, size_t len);
};

#endif
