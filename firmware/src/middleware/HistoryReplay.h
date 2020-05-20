#ifndef HISTORYREPLAY_H
#define HISTORYREPLAY_H

#include "../DisasterMiddleware.h"
#include "../DisasterHistory.h"

class HistoryReplay : public DisasterMiddleware
{
    DisasterHistory *history = NULL;

public:
    HistoryReplay(DisasterHistory *h)
        : history(h){};

    void setup();
};

#endif