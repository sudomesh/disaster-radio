
#include "OLEDClient.h"

#ifdef HAS_OLED
#define LINE_HEIGHT 10

OLEDClient::~OLEDClient() {}

void OLEDClient::loop()
{
  if (needs_display)
  {
    needs_display = false;

    display->setColor(BLACK);
    display->fillRect(left, top, width, height);

    display->setFont(ArialMT_Plain_10);
    display->setColor(WHITE);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    int i = 0;
    for (String line : buffer)
    {
      display->drawString(0, i * LINE_HEIGHT + top, line);
      i++;
    }
    display->display();
  }
}

void OLEDClient::receive(struct Datagram datagram, size_t len)
{
  String message = "";
  if (datagram.type == 'c')
  {
    for( int i = 0 ; i < len-DATAGRAM_HEADER ; i ++){
      message = message + (char)datagram.message[i];
    }
    uint16_t maxLines = (height - top) / LINE_HEIGHT;
    buffer.push_back(message.substring(4));
    while (buffer.size() > maxLines)
    {
      buffer.pop_front();
    }
    needs_display = true;
  }
}
#endif
