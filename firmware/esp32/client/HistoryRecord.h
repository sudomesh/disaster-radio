#ifndef HISTORYRECORD_H
#define HISTORYRECORD_H

#include "../DisasterClient.h"
#include "../DisasterHistory.h"

class HistoryRecord : public DisasterClient
{
    DisasterHistory *history = NULL;

public:
    HistoryRecord(DisasterHistory *h)
        : history(h){};

    void receive(String message);
};

#endif