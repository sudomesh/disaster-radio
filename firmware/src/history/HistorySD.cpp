#include "HistorySD.h"

#include <SD.h>
#include <ArduinoJson.h>
#include <list>

// Capacity calculated with https://arduinojson.org/v6/assistant/ to match max msg size of 233
const size_t capacity = JSON_OBJECT_SIZE(8) + 512;
DynamicJsonDocument doc(capacity);

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
  if (datagram.type != 'c')
  {
    // Serial.println("Not a chat message, skip");
    return;
  }

  // Parse datagram entries into char[] that ArduinoJSON can handle (needs NULL terminated)
  char charMsg[len - DATAGRAM_HEADER + 1];
  memcpy(charMsg, (const char *)datagram.message, len - DATAGRAM_HEADER);
  charMsg[len - DATAGRAM_HEADER] = 0;
  char dest[7] = {0};
  memcpy(dest, datagram.destination, 6);

  // Create the JSON
  doc["d"] = dest;
  doc["t"] = datagram.type;
  doc["m"] = charMsg;
  doc["l"] = len - DATAGRAM_HEADER;

    File log = SD.open(log_path, FILE_APPEND);
    if (log)
    {
    serializeJson(doc, log);
        log.write('\n');
    log.flush();
        log.close();
    }
}

#define MAX_HISTORY 10
#define BLOCK_SIZE 1024

void HistorySD::replay(DisasterClient *client)
{
    File log = SD.open(log_path, FILE_READ);
    if (log)
    {
    char msg[233];
        while (log.available())
        {
      size_t len = log.readBytesUntil('\n', (uint8_t *)msg, 233);
      if (len > 0)
      {
        DeserializationError error = deserializeJson(doc, msg);
        if (error)
            {
          continue;
        }
        // Convert JSON to a Datagram
        struct Datagram gotEntry = {0};
        const char *dest = doc["d"];
        memcpy(gotEntry.destination, dest, 6);
        gotEntry.type = doc["t"];
        const char *jsonMsg = doc["m"];
        size_t msgLen = doc["l"];
        memcpy(gotEntry.message, jsonMsg, msgLen);

        client->receive(gotEntry, msgLen + DATAGRAM_HEADER);
        delay(100);
            }
        }
        log.close();
    }
}
