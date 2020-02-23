#include "GPSClient.h"

void GPSClient::loop()
{
    while (stream->available() > 0)
    {
        gps.encode(stream->read());
    }

    int elapsed = millis() - beacon_last;
    if (elapsed > beacon_period)
    {
        if (gps.location.isValid() && gps.location.age() < beacon_period)
        {
            String message = String("c|<" + username + "> ") + gps.location.lat() + ", " + gps.location.lng();

            size_t len = message.length();
            uint8_t* data; 
            message.getBytes(data, len);
            struct Datagram datagram = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
            datagram.type = 'c';
            memcpy(datagram.message, data, len);
            len = len+DATAGRAM_HEADER;

            server->transmit(this, datagram, len);
            beacon_last = millis();
        }
    }
}
