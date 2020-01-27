#include "HistoryRecord.h"

void HistoryRecord::receive(String message)
{
    if (history)
    {
        history->record(message);
    }
}