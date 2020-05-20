#ifndef SOCATCLIENT_H
#define SOCATCLIENT_H
#ifdef SIM

#include <stdio.h>
#include <errno.h>
#include <fcntl.h> 
#include <string>
#include <cstring>
#include <termios.h>
#include <unistd.h>

#include "../DisasterClient.h"
#include "../DisasterServer.h"

class SocatClient : public DisasterClient
{
  public:
    SocatClient(std::string name) 
    : portname{name}{};

    int set_interface_attribs(int speed, int parity);

    bool init();
    void loop();

    void receive(struct Datagram datagram, size_t len);

  private:
    std::string portname;
    int tty_file;
    bool serialInitialized;
};
#endif
#endif
