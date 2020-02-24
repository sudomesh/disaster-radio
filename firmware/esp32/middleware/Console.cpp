#include "Console.h"

/** Comment out to stop debug output */
// #define DEBUG_OUT

#include <Layer1.h>
#include <LoRaLayer2.h>

#include <vector>

#define START_MESSAGE "Type '/join NICKNAME' to join the chat, or '/help' for more commands."

void Console::setup()
{
    DisasterMiddleware::setup();
  // if (history)
  // {
  // 	history->replay(this);
  // }
  struct Datagram response;
  int msgLen = sprintf((char *)response.message, "Type '/join NICKNAME' to join the chat, or '/help' for more commands.\n");
  client->receive(response, msgLen + DATAGRAM_HEADER);
}

void Console::processLine(char *message, size_t len)
{
  struct Datagram response;
  int msgLen;

  // message might not be NULL ended
  char msgBuff[len + 2] = {0};
  memcpy(msgBuff, message, len);

  if (msgBuff[0] == '/')
    {
    std::vector<char *> args;

    char *p;

    p = strtok(msgBuff, " ");
    while (p)
        {
      args.push_back(p);
      p = strtok(NULL, " ");
        }

    p = args[0];

    if (strncmp(&args[0][1], "help", 4) == 0)
        {
      msgLen = sprintf((char *)response.message, "Commands: /help /join /nick /raw /lora /restart\n");
      client->receive(response, msgLen + DATAGRAM_HEADER);
#ifdef DEBUG_OUT
      Serial.printf("Console::processLine help result %s\n", (char *)response.message);
#endif
    }
    else if (strncmp(&args[0][1], "raw", 3) == 0)
    {
#ifdef DEBUG_OUT
      Serial.printf("Console::processLine switching to RAW\n");
#endif
            disconnect(client);
            server->disconnect(this);
            server->connect(client);
        }
    else if (((strncmp(&args[0][1], "join",4) == 0) || (strncmp(&args[0][1], "nick",4) == 0)) && (args.size() > 1))
        {
      if (username.length() > 0)
      {
        msgLen = sprintf((char *)response.message, "00c|~ %s is now known as %s\n", username.c_str(), args[1]);
        }
      else
        {
        msgLen = sprintf((char *)response.message, "00c|~ %s joined the channel\n", args[1]);
      }
      memcpy(response.destination, LL2.broadcastAddr(), ADDR_LENGTH);
      response.type = 'c';
      server->transmit(this, response, msgLen + DATAGRAM_HEADER);
      memcpy(response.message, &response.message[4], msgLen - 4);
      response.message[msgLen - 4] = '\n';
      client->receive(response, msgLen - 4 + DATAGRAM_HEADER);
      username = String(args[1]);
#ifdef DEBUG_OUT
      Serial.printf("Console::processLine join/nick result %s\n", (char *)response.message);
      Serial.printf("Console::processLine new username is %s\n", username.c_str());
#endif
    }
    else if ((strncmp(&args[0][1], "restart", 7) == 0))
    {
#ifdef DEBUG_OUT
      Serial.printf("Console::processLine restarting\n");
      delay(500);
#endif
            ESP.restart();
        }
    else if ((strncmp(&args[0][1], "lora", 4) == 0))
        {
            // TODO: print to client instead of serial
            Serial.printf("Address: ");
      LL2.printAddress(LL2.localAddress());
            LL2.printRoutingTable();
        }
        else
        {
      msgLen = sprintf((char *)response.message, "Unknown command '%s'\n", msgBuff);
      client->receive(response, msgLen + DATAGRAM_HEADER);
        }
    }
    else if (username.length() > 0)
    {
    msgLen = sprintf((char *)response.message, "00c|<%s>%s", username.c_str(), msgBuff);
    memcpy(response.destination, LL2.broadcastAddr(), ADDR_LENGTH);
    response.type = 'c';
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
    memcpy(response.message, &response.message[4], msgLen - 4);
    response.message[msgLen - 4] = '\n';
    client->receive(response, msgLen - 3 + DATAGRAM_HEADER);
    }
    else
    {
    msgLen = sprintf((char *)response.message, "00c|%s", msgBuff);
    memcpy(response.destination, LL2.broadcastAddr(), ADDR_LENGTH);
    response.type = 'c';
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
    memcpy(response.message, &response.message[4], msgLen - 4);
    response.message[msgLen - 4] = '\n';
    client->receive(response, msgLen - 3 + DATAGRAM_HEADER);
    }
}

void Console::transmit(DisasterClient *client, struct Datagram datagram, size_t len)
{
#ifdef DEBUG_OUT
  Serial.printf("CONSOLE::transmit raw data with len %d type %c\n", len, datagram.type);
  for (int idx = 0; idx < len - DATAGRAM_HEADER; idx++)
  {
    Serial.printf("%02X ", datagram.message[idx]);
  }
  Serial.println("\n" + String((char *)datagram.message));
#endif

  // Split in case it is multi line
  /// \todo handle CR-LF combination
  char *p;
  char *next;
  int processLen = len - DATAGRAM_HEADER;

  p = strtok((char *)datagram.message, "\n");

  while (p)
    {
    next = p;
    p = strtok(NULL, "\n");
    if (p)
    {
      processLine(next, p - next - 1);
      processLen -= p - next;
    }
    else
    {
      processLine(next, processLen - 1);
    }
    }
};

void Console::receive(struct Datagram datagram, size_t len)
{
  int msgSize = len - DATAGRAM_HEADER;
#ifdef DEBUG_OUT
  Serial.printf("CONSOLE::receive raw data with len %d type %c\n", msgSize, datagram.type);
  for (int idx = 0; idx < msgSize; idx++)
  {
    Serial.printf("%02X ", datagram.message[idx]);
  }
  Serial.println("");
#endif

  if ((datagram.message[2] == 'c') && (datagram.message[3] == '|'))
  {
    memcpy(datagram.message, &datagram.message[4], msgSize - 4);
    msgSize -= 4;
    datagram.message[msgSize] = '\n';
    msgSize += 1;
    client->receive(datagram, msgSize + DATAGRAM_HEADER);
    }
};
