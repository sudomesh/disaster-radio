#include "HistoryMemory.h"
#include "LoRaLayer2.h"

void HistoryMemory::record(struct Datagram datagram, size_t len)
{
  if (datagram.type != 'c')
  {
    // Serial.println("Not a chat message, skip");
    return;
  }

  char tempBuf[len - DATAGRAM_HEADER + 1] = {0};
  memcpy(tempBuf, datagram.message, len);
  String newEntry = String(tempBuf);

  //   Serial.printf("HistoryMemory::record New History entry >%s<\n", newEntry.c_str());

  buffer.push_back(newEntry);
  while (buffer.size() > limit)
  {
    buffer.pop_front();
  }

  // HistoryEntry newEntry;
  // memcpy((void *)&newEntry.data, (void *)&datagram, len);

  // buffer.push_back(newEntry);
  // while (buffer.size() > limit)
  // {
  //   buffer.pop_front();
  // }
}

void HistoryMemory::replay(DisasterClient *client)
{
  for (String message : buffer)
  {
    // Serial.printf("HistoryMemory::replay Replay >%s<\n", message.c_str());
    Datagram newHistory = {0};
    memcpy(newHistory.destination, BROADCAST, ADDR_LENGTH);
    newHistory.type = 'c';
    size_t msgLen = sprintf((char *)newHistory.message, message.c_str());

    // Serial.println("HistoryMemory::replay Datagram RAW");
    // char *dbgBuf = (char *)&newHistory;
    // for (int idx = 0; idx < DATAGRAM_HEADER; idx++)
    // {
    //   Serial.printf("%02X ", dbgBuf[idx]);
    // }
    // for (int idx = DATAGRAM_HEADER; idx < msgLen + DATAGRAM_HEADER; idx++)
    // {
    //   Serial.printf("%c", dbgBuf[idx]);
    // }
    // Serial.println("");

    client->receive(newHistory, msgLen + DATAGRAM_HEADER);
    // Give others a chance to process the message
    delay(200);
  }

  // for (HistoryEntry message : buffer)
  // {
  //   client->receive(message.data, message.len);
  //   // Give others a chance to process the message
  //   delay(200);
  // }
}
