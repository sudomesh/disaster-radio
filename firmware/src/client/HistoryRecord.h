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

    void receive(struct Datagram datagram, size_t len);
};

#endif
