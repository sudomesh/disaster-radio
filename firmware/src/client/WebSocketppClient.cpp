#ifdef SIM
#include "WebSocketppClient.h"
#include "Layer1_Sim.h"

#include <map>

typedef std::set<connection_hdl, std::owner_less<connection_hdl>> connection_map;
connection_map client_map;

WebSocketppClient::WebSocketppClient(){}//WebSocketppServer *wss)
//  : ws_server{wss} {};

void WebSocketppClient::receive(struct Datagram datagram, size_t len)
{
    unsigned char buf[len-DATAGRAM_HEADER];
    memcpy(buf, &datagram.message, sizeof(buf));

    // Read datagram message into outgoing value
    std::stringstream val;
    for(int i = 0; i < len-DATAGRAM_HEADER ; i++){
      val << buf[i];
    }

    // iterate over client map to send message to all connected clients
    connection_map::iterator it;
    for (it = client_map.begin(); it != client_map.end(); ++it) {
      ws.send(*it, val.str(), websocketpp::frame::opcode::text);
    }
}

void WebSocketppClient::on_message(connection_hdl hdl, WebSocketppServer::message_ptr msg)
{
  // Upgrade connection handle to full connection pointer
  WebSocketppServer::connection_ptr con = ws.get_con_from_hdl(hdl);

  std::string message = msg->get_payload(); 

  // Send ack to ws client
  std::stringstream val;
  val << message.substr(0,2) << "!";
  ws.send(con, val.str(), websocketpp::frame::opcode::text);

  // Send message to other connected layer 3 clients
  struct Datagram datagram = {0xff, 0xff, 0xff, 0xff};
  datagram.type = 'c';
  memcpy(&datagram.message, message.c_str(), message.length());
  server->transmit(this, datagram, DATAGRAM_HEADER+message.length());
}

void WebSocketppClient::on_http(connection_hdl hdl, std::string root)
{
    // folder where static website is stored
    //std::string docroot = "./static/";
    WebSocketppServer::connection_ptr con = ws.get_con_from_hdl(hdl);
    std::ifstream file;
    std::string filename = con->get_resource();
    std::string response;

    // Open index.htm when "/" is requested
    if (filename == "/") {
        filename = root+"index.htm";
    } else {
        filename = root+filename.substr(1);
    }
    
    //Open requested file
    file.open(filename.c_str(), std::ios::in);
    if (!file) {
        // 404 error
        std::stringstream ss;
    
        ss << "<!doctype html><html><head>"
           << "<title>Error 404 (Resource not found)</title><body>"
           << "<h1>Error 404</h1>"
           << "<p>The requested URL " << filename << " was not found on this server.</p>"
           << "</body></head></html>";
    
        con->set_body(ss.str());
        con->set_status(websocketpp::http::status_code::not_found);
        return;
    }

    // Parse file into response and send
    file.seekg(0, std::ios::end);
    response.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    response.assign((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    con->set_body(response);
    con->set_status(websocketpp::http::status_code::ok);
}

void WebSocketppClient::loop()
{
  ws.poll();
}

void WebSocketppClient::startServer(uint16_t port, std::string root)
{
    ws.clear_access_channels(websocketpp::log::alevel::all);
    ws.set_reuse_addr(true);
    ws.set_open_handler(
        [](connection_hdl hdl){
            //WebSocketppClient *ws_client = new WebSocketppClient(ws);
            client_map.insert(hdl);// = ws_client;
            //callback(ws_client);
        });
    ws.set_close_handler(
        [](connection_hdl hdl){
            //WebSocketppClient *ws_client = client_map[hdl];
            //ws_client->handleDisconnect();
            client_map.erase(hdl);
        });
    ws.set_http_handler(
        [this, root](connection_hdl hdl){
            //WebSocketppClient *ws_client = client_map[hdl];
            //WebSocketppServer::connection_ptr con = ws->get_con_from_hdl(hdl);
            on_http(hdl, root);
        });
    ws.set_message_handler(
        [this](connection_hdl hdl, WebSocketppServer::message_ptr msg){
            //WebSocketppClient *ws_client = client_map[hdl];
            on_message(hdl, msg);
        });
    ws.set_fail_handler(
        [](connection_hdl hdl){
            std::cout << "fail: " << hdl.lock() << std::endl;
        });

    boost::asio::io_service ios;
    ws.init_asio(&ios);
    // listen on specified port
    ws.listen(port);
    // Start the server accept loop
    ws.start_accept();
    // Start the ASIO io_service run loop
    //ws.run(); // is blocking, CANNOT USE.
}
#endif
