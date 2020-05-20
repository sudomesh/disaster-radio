#include "HistoryReplay.h"

void HistoryReplay::setup()
{
    DisasterMiddleware::setup();
    if (history && client)
    {
        history->replay(client);
    }
}