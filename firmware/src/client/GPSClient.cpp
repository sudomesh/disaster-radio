#include "GPSClient.h"

extern char *nodeAddress; //TODO: should not use extern

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
			// 04m|<user>{"pos":[48.75608,2.302038]}
            struct Datagram datagram = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
			memset(datagram.message, 0, 233);
			datagram.type = 'm';
			size_t len;
			if (username.length() > 0)
			{
				len = snprintf((char *)datagram.message, 233, "m|<%s>{\"pos\":[%.3f,%.3f]}", username.c_str(), gps.location.lat(), gps.location.lng());
			}
			else
			{
				len = snprintf((char *)datagram.message, 233, "m|<%s>{\"pos\":[%.3f,%.3f]}", nodeAddress, gps.location.lat(), gps.location.lng());
			}
            len = len+DATAGRAM_HEADER;

			Serial.printf("Sending GPS %s\n", (char *)datagram.message);
            server->transmit(this, datagram, len);
            beacon_last = millis();
        }
    }
}

void GPSClient::setUsername(String newname)
{
	username = newname;
}
