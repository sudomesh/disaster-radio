
#ifndef GPSCLIENT_H
#define GPSCLIENT_H

#include "../DisasterClient.h"

#include "TinyGPS++.h"
#include "LoRaLayer2.h"
#include "settings/settings.h"

class GPSClient : public DisasterClient
{
    TinyGPSPlus gps;
	// Stream *stream;
	HardwareSerial *stream;

    long beacon_period;
    long beacon_last = 0;

public:
    // String username = "beacon";

	// GPSClient(Stream *s, unsigned long p = DEFAULT_PERIOD)
	// 	: stream(s), beacon_period(p){};
	GPSClient(HardwareSerial *s, long p)
        : stream(s), beacon_period(p){};

    void loop();
	void setUsername(String username);
    void setBeaconPeriod(long new_period);
    void receive(struct Datagram datagram, size_t len);
};

#endif
