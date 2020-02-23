#include "HistoryMemory.h"

void HistoryMemory::record(struct Datagram datagram, size_t len)
{
  /*
  buffer.push_back(datagram);
  while (buffer.size() > limit)
  {
    buffer.pop_front();
  }
  */
}

void HistoryMemory::replay(DisasterClient *client)
{
  /*
  for (String message : buffer)
  {
    client->receive(message);
  }
  */
}
