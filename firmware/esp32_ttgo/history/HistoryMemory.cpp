#include "HistoryMemory.h"

void HistoryMemory::record(String message)
{
  buffer.push_back(message);
  while (buffer.size() > limit)
  {
    buffer.pop_front();
  }
}

void HistoryMemory::replay(DisasterClient *client)
{
  for (String message : buffer)
  {
    client->receive(message);
  }
}
