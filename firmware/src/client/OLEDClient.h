
#ifndef OLEDCLIENT_H
#define OLEDCLIENT_H

#include "../DisasterClient.h"
#include "../DisasterServer.h"

#ifdef HAS_OLED
#include "SSD1306Wire.h"
#include <list>

class OLEDClient : public DisasterClient
{
  SSD1306Wire *display;
  std::list<String> buffer;

  int left;
  int top;
  int width;
  int height;

  bool needs_display = false;

public:
  OLEDClient(SSD1306Wire *d, int l = 0, int t = 0, int w = 128, int h = 64)
      : display{d}, left{l}, top{t}, width{w}, height{h} {};
  virtual ~OLEDClient();

  void loop();
  void receive(struct Datagram datagram, size_t len);
};

#endif
#endif
