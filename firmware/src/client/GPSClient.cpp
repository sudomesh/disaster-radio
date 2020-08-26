#include "GPSClient.h"

extern char *nodeAddress; //TODO: should not use extern

void GPSClient::loop()
{
    while (stream->available() > 0)
    {
        gps.encode(stream->read());
    }

    long elapsed = millis() - beacon_last;
    if (elapsed > beacon_period && beacon_period > 0)
    {
        struct Datagram datagram = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        memset(datagram.message, 0, 233);
		size_t len;
        if (gps.location.isValid() && gps.location.age() < beacon_period)
        {
			// 04m|<user>{"pos":[48.75608,2.302038]}
			datagram.type = 'm';
			if (username.length() > 0)
			{
				len = snprintf((char *)datagram.message, 233, "m|<%s>{\"pos\":[%.3f,%.3f]}", username.c_str(), gps.location.lat(), gps.location.lng());
			}
			else
			{
				len = snprintf((char *)datagram.message, 233, "m|<%s>{\"pos\":[%.3f,%.3f]}", nodeAddress, gps.location.lat(), gps.location.lng());
			}
            // Send datagram with lat-long info
            server->transmit(this, datagram, len);
            // if debug is enabled, print coordinates to console
            datagram.type = 'i';
			len = snprintf((char *)datagram.message, 233, "Sent GPS reading [%.3f,%.3f]\r\n", gps.location.lat(), gps.location.lng());
            len = len+DATAGRAM_HEADER;
            server->transmit(this, datagram, len);
        }
        else
        {
            datagram.type = 'i';
	        len = snprintf((char *)datagram.message, 233, "Failed to get GPS reading\r\n");
            len = len+DATAGRAM_HEADER;
            server->transmit(this, datagram, len);
        }
        beacon_last = millis();
    }
}

void GPSClient::setUsername(String newname)
{
	username = newname;
}

void GPSClient::setBeaconPeriod(long new_period)
{
	beacon_period = new_period;
}

void GPSClient::receive(struct Datagram datagram, size_t len)
{
  Datagram response;
  int msgLen;
  long value;
  if(datagram.type == 'i'){
    if(memcmp(&datagram.message[0], "gps", 3) == 0){
      sscanf((char *)&datagram.message[4], "%ld", &value);
      setBeaconPeriod(value);
      memcpy(response.destination, BROADCAST, ADDR_LENGTH);
      response.type = 'i';
      if(value > 0){
        msgLen = sprintf((char *)response.message, "GPS beacons enabled, interval of GPS beacon messages set to %ldms\r\n", value);
      }
      else if(value == 0){
        msgLen = sprintf((char *)response.message, "GPS beacons disabled\r\n");
      }
      server->transmit(this, response, msgLen + DATAGRAM_HEADER);
    }
  }
}
