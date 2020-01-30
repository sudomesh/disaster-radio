
#include "TCPClient.h"

void TCPClient::receive(String message)
{
    client->add(message.c_str(), message.length());
    client->send();
}

void TCPClient::handleData(void *data, size_t len)
{
    char buf[len + 1];
    memcpy(buf, data, len);
    buf[len] = 0;
    String str = String(buf);
    Serial.printf("1:[%d][%s]\n", str.length(), str.c_str());
    server->transmit(this, str);
}

void TCPClient::handleDisconnect()
{
    server->disconnect(this);
}

void TCPClient::handleError(int8_t error)
{
    server->disconnect(this);
}

void TCPClient::handleTimeout(uint32_t time)
{
    server->disconnect(this);
}

void TCPClient::startServer(AsyncServer *server, void (*callback)(TCPClient *))
{
    server->onClient([callback](void *arg, AsyncClient *client) {
        // add to clients list
        TCPClient *tcp_client = new TCPClient(client);

        // register events
        client->onData([tcp_client](void *arg, AsyncClient *client, void *data, size_t len) {
            tcp_client->handleData(data, len);
        });
        client->onDisconnect([tcp_client](void *arg, AsyncClient *client) {
            tcp_client->handleDisconnect();
        });
        client->onError([tcp_client](void *arg, AsyncClient *client, int8_t error) {
            tcp_client->handleError(error);
        });
        client->onTimeout([tcp_client](void *arg, AsyncClient *client, uint32_t time) {
            tcp_client->handleTimeout(time);
        });

        callback(tcp_client);
    },
                     NULL);
    server->begin();
}
