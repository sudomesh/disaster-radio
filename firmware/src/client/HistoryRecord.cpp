#include "HistoryRecord.h"

void HistoryRecord::receive(struct Datagram datagram, size_t len)
{
    if (history)
    {
        history->record(datagram, len);
    }
}
