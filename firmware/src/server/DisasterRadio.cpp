
#include "DisasterRadio.h"

// NOTE: multimethod so we can return the correct parameter type to chain on
DisasterMiddleware *DisasterRadio::connect(DisasterMiddleware *client)
{
    client->server = this;
    clients.insert(client);
    connected.push_back(client);
    return client;
}
DisasterClient *DisasterRadio::connect(DisasterClient *client)
{
    client->server = this;
    clients.insert(client);
    connected.push_back(client);
    return client;
}
void DisasterRadio::disconnect(DisasterClient *client)
{
    clients.erase(client);
    delete client;
}

void DisasterRadio::transmit(DisasterClient *client, struct Datagram datagram, size_t len)
{
    for (DisasterClient *other_client : clients)
    {
        if (client != other_client)
        {
            other_client->receive(datagram, len);
        }
    }
}

void DisasterRadio::setup()
{
}

void DisasterRadio::loop()
{
    for (DisasterClient *client : connected)
    {
        client->setup();
    }
    connected.clear();

    for (DisasterClient *client : clients)
    {
        client->loop();
    }
}
