#include "Console.h"

/** Comment out to stop debug output */
// #define DEBUG_OUT

#include <LoRaLayer2.h>
#include "../utils/utils.h"
#ifndef SIM
#include "settings/settings.h"
#endif

#include <vector>

#ifdef SIM
Console::Console(){};
#endif

void Console::printf(const char* format, ...)
{

  va_list args;
  va_start(args, format);
  struct Datagram response;
  memcpy(response.destination, LOOPBACK, ADDR_LENGTH);
  response.type = 'i';
  size_t len = vsprintf((char *)response.message, format, args);
  client->receive(response, len + DATAGRAM_HEADER);
  va_end(args);
}

void Console::setup()
{
  #ifndef SIM
  DisasterMiddleware::setup();
  if (history)
  {
    history->replay(this);
  }
  #endif
  sessionConnected = 0;
}

void Console::processLine(char *message, size_t len)
{
  if(len <= 2){
    // message will always contain CR-LF
    // if length is less or equal to 2, do nothing
    return;
  }
  struct Datagram response;
  memset(response.message, 0, DATAGRAM_MESSAGE);
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
      printf("Commands: /help /join /nick /raw /lora /set /restart\r\n");
    }
    else if (strncmp(&args[0][1], "raw", 3) == 0)
    {
      disconnect(client);
      server->disconnect(this);
      server->connect(client);
    }
    #ifndef SIM
    else if ((strncmp(&args[0][1], "set", 3) == 0) && (args.size() > 1))
    {
      if (strncmp(&args[1][0], "ui", 2) == 0){
        saveUI(!useBLE);
        delay(500);
        ESP.restart();
      }
    }
    else if ((strncmp(&args[0][1], "set", 3) == 0) && (args.size() == 1)){
      printf("No setting provided, type '/set SETTING'\r\n");
      printf("SETTINGs include,\r\n");
      printf("'ui' - toggles between WiFi and BLE user interface\r\n");
    }

    else if (((strncmp(&args[0][1], "join", 4) == 0) || (strncmp(&args[0][1], "nick", 4) == 0)) && (args.size() > 1))
    {
      strtok(args[1], "\r"); // remove CR-LF from username
      strtok(args[1], "\n"); // remove CR-LF from username

      if (username.length() > 0)
      {
        msgLen = sprintf((char *)response.message, "00c|~ %s is now known as %s\r\n", username.c_str(), args[1]);
      }
      else
      {
        msgLen = sprintf((char *)response.message, "00c|~ %s joined the channel\r\n", args[1]);
      }

      memcpy(response.destination, BROADCAST, ADDR_LENGTH);
      response.type = 'c';
      server->transmit(this, response, msgLen + DATAGRAM_HEADER);

      memcpy(response.message, &response.message[4], msgLen - 4);
      response.message[msgLen - 4] = '\n';
      client->receive(response, msgLen - 4 + DATAGRAM_HEADER);

      username = String(args[1]);
      saveUsername(username);

    }
    else if (((strncmp(&args[0][1], "join", 4) == 0) || (strncmp(&args[0][1], "nick", 4) == 0)) && (args.size() == 1)){
      printf("No NICKNAME provided, type '/join NICKNAME' to join the chat\r\n");
    }
    else if ((strncmp(&args[0][1], "restart", 7) == 0))
    {
      ESP.restart();
    }
    #endif
    else if ((strncmp(&args[0][1], "lora", 4) == 0))
    {
      // send messsage to LoRaClient, wait for response from LL2
      memcpy(response.destination, BROADCAST, ADDR_LENGTH);
      response.type = 'i';
      msgLen = sprintf((char *)response.message, "lora");
      server->transmit(this, response, msgLen + DATAGRAM_HEADER);
    }
    else
    {
      msgLen = sprintf((char *)response.message, "Unknown command '%s'\r\n", msgBuff);
      client->receive(response, msgLen + DATAGRAM_HEADER);
    }
  }
  #ifndef SIM
  else if (username.length() > 0)
  {
    msgLen = sprintf((char *)response.message, "00c|<%s>%s", username.c_str(), msgBuff);
    memcpy(response.destination, BROADCAST, ADDR_LENGTH);
    response.type = 'c';
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
    memcpy(response.message, &response.message[4], msgLen - 4);
    response.message[msgLen - 4] = '\n';
  }
  #endif
  else if(msgBuff[0] == '@')
  {
    // "direct"/routed message
    uint8_t destination[ADDR_LENGTH];
    charToHex(destination, msgBuff+1, ADDR_LENGTH);
    msgLen = sprintf((char *)response.message, "%s", msgBuff);
    memcpy(response.destination, destination, ADDR_LENGTH);
    response.type = 'c';
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
  }
  else
  {
    // broadcast message
    msgLen = sprintf((char *)response.message, "%s", msgBuff);
    memcpy(response.destination, BROADCAST, ADDR_LENGTH);
    response.type = 'c';
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
    #ifdef DEBUG_OUT
    //memcpy(response.message, &response.message, msgLen);
    //response.message[msgLen] = '\n';
    //Serial.printf("Console message =>%s<\r\n", &response.message[4]);
    #endif
  }
}

void Console::printBanner()
{
  printf("     ___              __                            ___    \r\n");
  printf(" ___/ (_)__ ___ ____ / /____ ____      _______ ____/ (_)__ \r\n");
  printf("/ _  / (_-</ _ `(_-</ __/ -_) __/ _   / __/ _ `/ _  / / _ \\\r\n");
  printf("\\_,_/_/___/\\_,_/___/\\__/\\__/_/   (_) /_/  \\_,_/\\_,_/_/\\___/\r\n");
  printf("v1.0.0-rc.2\r\n");
  #ifndef SIM
  /*
  if(Layer1.loraInitialized()){
    printf("LoRa transceiver connected\r\n");
  }else{
    printf("WARNING: LoRa transceiver not found!\r\n");
  }
  */
  #endif
  char *str = (char*)malloc(ADDR_LENGTH*2 + 1);// = {'\0'};
  //hexToChar(str, LL2Class::localAddress(), ADDR_LENGTH);
  printf("Local address of your node is %s\r\n", str);
  printf("Type '/join NICKNAME' to join the chat, or '/help' for more commands.\r\n");
  free(str);
}

void Console::printPrompt()
{
  #ifndef SIM
  if (username.length() > 0)
  {
    printf("<%s> ", username.c_str());
  }
  else
  {
    printf("< > ");
  }
  #else
  printf("< > ");
  #endif
}

void Console::transmit(DisasterClient *client, struct Datagram datagram, size_t len)
{
  // TODO: set sessionConnected back to zero on disconnection?
  if(sessionConnected == 0){
    printBanner();
    printPrompt();
    sessionConnected = 1;
  }
  else if(sessionConnected == 1){
    // Console receives one line at a time
    processLine((char *) datagram.message, len - DATAGRAM_HEADER);
    printPrompt();
  }
}

void Console::receive(struct Datagram datagram, size_t len)
{
  printf("\r\n");
  client->receive(datagram, len);
  printPrompt();
}
