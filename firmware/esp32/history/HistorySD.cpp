#include "HistorySD.h"

#include <SD.h>
#include <list>

// FIXME: assumes messages don't have newlines

bool HistorySD::init()
{
    File log = SD.open(log_path, FILE_APPEND);
    if (log)
    {
        initialized = true;
        return true;
    }
    else
    {
        return false;
    }
}

void HistorySD::record(struct Datagram datagram, size_t len)
{
    /*
    File log = SD.open(log_path, FILE_APPEND);
    if (log)
    {
        log.print(message);
        log.write('\n');
        log.close();
    }
*/
}

#define MAX_HISTORY 10
#define BLOCK_SIZE 1024

void HistorySD::replay(DisasterClient *client)
{
    /*
    File log = SD.open(log_path, FILE_READ);
    if (log)
    {
        while (log.available())
        {
            String line = log.readStringUntil('\n');
            if (line.length() > 0)
            {
                client->receive(line);
            }
        }
        log.close();
    }
    */
}
