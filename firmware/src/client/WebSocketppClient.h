#ifndef WebSocketCLIENT_H
#define WebSocketCLIENT_H
#ifdef SIM

#include "../DisasterClient.h"
#include "../DisasterServer.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <thread>
#include <fstream>
#include <set>
#include <streambuf>
#include <string>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> WebSocketppServer;
typedef websocketpp::connection_hdl connection_hdl;

class WebSocketppClient : public DisasterClient
{


public:
    WebSocketppClient();
    
    void receive(struct Datagram datagram, size_t len);
    void on_message(connection_hdl hdl, WebSocketppServer::message_ptr msg);
    void on_http(connection_hdl hdl, std::string root);
    void loop();

    void startServer(uint16_t port, std::string root);
  
private:
    WebSocketppServer ws;

};

#endif
#endif
