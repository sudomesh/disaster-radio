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
            server->transmit(this, String("c|<" + username + "> ") + gps.location.lat() + ", " + gps.location.lng());
            beacon_last = millis();
        }
    }
}