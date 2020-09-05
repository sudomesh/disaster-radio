#include "Console.h"

/** Comment out to stop debug output */
// #define DEBUG_OUT

#include <LoRaLayer2.h>
#include "../utils/utils.h"
#include "../../config.h"
#ifndef SIM
#include "settings/settings.h"
#endif

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
  size_t len = vsnprintf((char *)response.message, sizeof(response.message), format, args);
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

void Console::usage()
{
  printf("Commands: /help /join /nick /raw /lora /get /set /restart\r\n");
}

// TODO add support for `/get` and `/set` in simulator
#ifndef SIM
void Console::getUsage()
{
  printf("OPTIONs include,\r\n");
  printf("'config' - print the current configuration of Layer1 interface and LL2 options\r\n");
}

void Console::uiUsage()
{
  printf("'ui'           - toggles between WiFi and BLE user interface\r\n\r\n");
}

void Console::txpowerUsage()
{
  printf("'txpower VAL'  - sets the transmit power of LoRa transceiver,\r\n");
  printf("                 where VAL is desired power in dB between 2 and 20\r\n\r\n");
}

void Console::lorafrqUsage()
{
  printf("'lorafrq VAL'  - sets the frequency for the LoRa transceiver,\r\n");
  printf("                 where VAL is desired frequency in MHz\r\n\r\n");
}

void Console::sfUsage()
{
  printf("'sf VAL'       - sets the spreading factor of LoRa transceiver,\r\n");
  printf("                 where VAL is desired spreading factor between 6 and 12\r\n\r\n");
}

void Console::dutyUsage()
{
  printf("'duty VAL'     - sets the duty cycle of LoRaLayer2 protocol,\r\n");
  printf("                 where VAL is a floating point decimal representing\r\n");
  printf("                 precentage of airtime alotted for this LL2 node\r\n\r\n");
}

void Console::intervalUsage()
{
  printf("'interval VAL' - sets the interval (in ms) at which LoRaLayer2\r\n");
  printf("                 routing table messages are broadcast to neighboiring nodes\r\n");
  printf("                 Allowing routing table is built/maintained `automatically`\r\n");
  printf("                 if 0 is entered as VAL, routing table messages are disabled\r\n");
  printf("                 and routing table must be built/maintained 'manually' by nodes\r\n");
  printf("                 actively transmitting messages to one another\r\n\r\n");
}

void Console::gpsUsage()
{
  printf("'gps VAL'      - sets the inverval (in ms) at which a GPS beacon messages are broadcast\r\n");
  printf("                 if 0 is entered as VAL, GPS beacon messages are disabled,\r\n");
  printf("                 but GPS coordinates may still be availble for internal use\r\n");
}

void Console::setUsage()
{
  printf("SETTINGs include,\r\n");
  uiUsage();
  txpowerUsage();
  lorafrqUsage();
  sfUsage();
  dutyUsage();
  intervalUsage();
#ifdef GPS_SERIAL
  gpsUsage();
#endif
}

void Console::get(std::vector<char *> args)
{
  struct Datagram response;
  int msgLen;

  if (strncmp(&args[1][0], "config", 6) == 0){
    memcpy(response.destination, BROADCAST, ADDR_LENGTH);
    response.type = 'i';
    msgLen = sprintf((char *)response.message, "config");
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
  }
}

void Console::uiSet()
{
  saveUI(!useBLE);
  delay(500);
  ESP.restart();
}

void Console::txpowerSet(std::vector<char *> args)
{
  struct Datagram response;
  int msgLen;
  int value;

  sscanf(&args[2][0], "%d", &value);
  if((value < 2) || (value > 20)){
    printf("Invalid value provided, type '/set txpower VAL'\r\n");
    txpowerUsage();
  }
  else{
    saveTxPower(value);
    memcpy(response.destination, BROADCAST, ADDR_LENGTH);
    response.type = 'i';
    msgLen = sprintf((char *)response.message, "txpower %d", value);
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
  }
}

void Console::lorafrqSet(std::vector<char *> args)
{
  int value;

  sscanf(&args[2][0], "%d", &value);
  saveLoraFrq(value);
  delay(500);
  ESP.restart();
}

void Console::sfSet(std::vector<char *> args)
{
  struct Datagram response;
  int msgLen;
  int value;

  sscanf(&args[2][0], "%d", &value);
  if((value < 6) || (value > 12)){
    printf("Invalid value provided, type '/set sf VAL'\r\n");
    sfUsage();
  }
  else{
    saveSpreadingFactor(value);
    memcpy(response.destination, BROADCAST, ADDR_LENGTH);
    response.type = 'i';
    msgLen = sprintf((char *)response.message, "sf %d", value);
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
  }
}

void Console::dutySet(std::vector<char *> args)
{
  struct Datagram response;
  int msgLen;
  double value;

  sscanf(&args[2][0], "%lf", &value);
  if((value <= 0) || (value > 1)){
    printf("Invalid value provided, type '/set duty VAL'\r\n");
    dutyUsage();
  }
  else{
    saveDutyCycle(value);
    memcpy(response.destination, BROADCAST, ADDR_LENGTH);
    response.type = 'i';
    msgLen = sprintf((char *)response.message, "duty %lf", value);
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
  }
}

void Console::intervalSet(std::vector<char *> args)
{
  struct Datagram response;
  int msgLen;
  long value; // this should use an unsigned long, but LL2 would need to be updated also

  sscanf(&args[2][0], "%ld", &value);
  if(value < 0 || value > 2147483647){ // i.e. max value of signed long
    printf("Invalid value provided, type '/set interval VAL'\r\n");
    intervalUsage();
  }
  else{
    saveInterval(value);
    memcpy(response.destination, BROADCAST, ADDR_LENGTH);
    response.type = 'i';
    msgLen = sprintf((char *)response.message, "interval %ld", value);
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
  }
}

void Console::gpsSet(std::vector<char *> args)
{
  struct Datagram response;
  int msgLen;
  long value; // this should use an unsigned long, but LL2 would need to be updated also

  sscanf(&args[2][0], "%ld", &value);
  if(value < 0 || value > 2147483647){ // i.e. max value of signed long
    printf("Invalid value provided, type '/set gps VAL'\r\n");
    gpsUsage();
  }
  else{
    saveGPS(value);
    memcpy(response.destination, BROADCAST, ADDR_LENGTH);
    response.type = 'i';
    msgLen = sprintf((char *)response.message, "gps %ld", value);
    server->transmit(this, response, msgLen + DATAGRAM_HEADER);
  }
}

void Console::set(std::vector<char *> args)
{
  if (strncmp(&args[1][0], "ui", 2) == 0)
  {
    uiSet();
  }
  else if ((strncmp(&args[1][0], "txpower", 7) == 0) && (args.size() > 2))
  {
    txpowerSet(args);
  }
  else if ((strncmp(&args[1][0], "txpower", 7) == 0) && (args.size() == 2))
  {
    printf("No value provided, type '/set txpower VAL'\r\n");
    txpowerUsage();
  }
  else if ((strncmp(&args[1][0], "lorafrq", 7) == 0) && (args.size() > 2))
  {
    lorafrqSet(args);
  }
  else if ((strncmp(&args[1][0], "lorafrq", 7) == 0) && (args.size() == 2))
  {
    printf("No value provided, type '/set lorafrq VAL'\r\n");
    lorafrqUsage();
  }
  else if ((strncmp(&args[1][0], "sf", 2) == 0) && (args.size() > 2))
  {
    sfSet(args);
  }
  else if ((strncmp(&args[1][0], "sf", 2) == 0) && (args.size() == 2))
  {
    printf("No value provided, type '/set sf VAL'\r\n");
    sfUsage();
  }
  else if ((strncmp(&args[1][0], "duty", 4) == 0) && (args.size() > 2))
  {
    dutySet(args);
  }
  else if ((strncmp(&args[1][0], "duty", 4) == 0) && (args.size() == 2)){
    printf("No value provided, type '/set duty VAL'\r\n");
    dutyUsage();
  }
  else if ((strncmp(&args[1][0], "interval", 8) == 0) && (args.size() > 2))
  {
    intervalSet(args);
  }
  else if ((strncmp(&args[1][0], "interval", 8) == 0) && (args.size() == 2)){
    printf("No value provided, type '/set interval VAL'\r\n");
    intervalUsage();
  }
#ifdef GPS_SERIAL
  else if ((strncmp(&args[1][0], "gps", 3) == 0) && (args.size() > 2))
  {
    gpsSet(args);
  }
  else if ((strncmp(&args[1][0], "gps", 3) == 0) && (args.size() == 2)){
    printf("No value provided, type '/set gps VAL'\r\n");
    gpsUsage();
  }
#endif
  else{
    printf("Setting provided does not exist, type '/set SETTING'\r\n");
    setUsage();
  }
}
#endif

void Console::processLine(char *message, size_t len)
{
  if(len <= 2){
    // message will always contain CR-LF
    // if length is less or equal to 2, do nothing
    return;
  }
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
      usage();
    }
    else if (strncmp(&args[0][1], "raw", 3) == 0)
    {
      disconnect(client);
      server->disconnect(this);
      server->connect(client);
    }
    #ifndef SIM
    else if ((strncmp(&args[0][1], "get", 3) == 0) && (args.size() > 1))
    {
      get(args);
    }
    else if ((strncmp(&args[0][1], "get", 3) == 0) && (args.size() == 1)){
      printf("No option provided, type '/get OPTION'\r\n");
      getUsage();
    }

    else if ((strncmp(&args[0][1], "set", 3) == 0) && (args.size() > 1))
    {
      set(args);
    }
    else if ((strncmp(&args[0][1], "set", 3) == 0) && (args.size() == 1)){
      printf("No setting provided, type '/set SETTING'\r\n");
      setUsage();
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
  struct Datagram request = {0};
  size_t msgLen;
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
  printf("Local address of your node is ");
  memcpy(request.destination, BROADCAST, ADDR_LENGTH);
  request.type = 'i';
  msgLen = sprintf((char *)request.message, "addr");
  server->transmit(this, request, msgLen + DATAGRAM_HEADER);
  printf("\r\n");
  printf("Type '/join NICKNAME' to join the chat, or '/help' for more commands.\r\n");
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
  // only print chat and info messages, add more as needed
  if(datagram.type == 'i')
  {
    client->receive(datagram, len);
  }

  // only print out additional formatting for chat messages
  if(datagram.type == 'c'){
    printf("\r\n");
    client->receive(datagram, len);
    printPrompt();
  }
}
