
#ifndef OLEDCLIENT_H
#define OLEDCLIENT_H

#include "../DisasterClient.h"
#include "../DisasterServer.h"
#include "../config.h"

#ifdef OLED_SDA
#include "SSD1306Wire.h"
#include <list>

class OLEDClient : public DisasterClient
{
  SSD1306Wire *display;
  std::list<String> buffer;

  int left;
  int top;
  int width = OLED_WIDTH;
  int height = OLED_HEIGHT;

  bool needs_display = false;

public:
  OLEDClient(SSD1306Wire *d, int l = 0, int t = 0)
      : display{d}, left{l}, top{t} {};

  void loop();
  void receive(struct Datagram datagram, size_t len);
};

#endif
#endif
