#ifndef CONSOLE_H
#define CONSOLE_H

#include "../DisasterMiddleware.h"
#include "../DisasterClient.h"
#ifndef SIM
#include "../DisasterHistory.h"
#endif

class Console : public DisasterMiddleware
{
#ifndef SIM
  String username = "";
  String buffer = "";
  DisasterHistory *history = NULL;
#endif

public:
#ifdef SIM
    Console();
#else
    Console(DisasterHistory *h = NULL)
        : DisasterMiddleware(), history(h){};
#endif
    void printf(const char* format, ...);

    void setup();

    void transmit(DisasterClient *client, struct Datagram datagram, size_t len);
    void receive(struct Datagram datagram, size_t len);

private:
    void processLine(char *message, size_t len);
    void printBanner();
    void printPrompt();

    int sessionConnected;
};

#endif
