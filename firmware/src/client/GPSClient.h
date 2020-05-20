
#ifndef GPSCLIENT_H
#define GPSCLIENT_H

#include "../DisasterClient.h"

#include "TinyGPS++.h"
#include "settings/settings.h"

#define DEFAULT_PERIOD 30000 // 10 seconds

class GPSClient : public DisasterClient
{
    TinyGPSPlus gps;
	// Stream *stream;
	HardwareSerial *stream;

    unsigned long beacon_period = DEFAULT_PERIOD;
    unsigned long beacon_last = 0;

public:
    // String username = "beacon";

	// GPSClient(Stream *s, unsigned long p = DEFAULT_PERIOD)
	// 	: stream(s), beacon_period(p){};
	GPSClient(HardwareSerial *s, unsigned long p = DEFAULT_PERIOD)
        : stream(s), beacon_period(p){};

    void loop();
	void setUsername(String username);
};

#endif
