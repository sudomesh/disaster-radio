#ifndef HISTORYSDCLIENT_CPP
#define HISTORYSDCLIENT_CPP

#include "../DisasterClient.h"
#include "../DisasterHistory.h"

#include <SPI.h>

class HistorySD : public DisasterHistory
{
    bool initialized = false;
    const char *log_path = "/log.txt";
    SPIClass *sd;

public:
    HistorySD(SPIClass *s)
        : sd{s} {};

    bool init();
    void record(String message);
    void replay(DisasterClient *client);
};

#endif
