#include "LoRaClient.h"

bool LoRaClient::init()
{
    LL2->init(); // initialize Layer2
    LL2->setInterval(0); // set to zero to disable routing packets
    return true;
}

void LoRaClient::loop()
{
    LL2->daemon();
    struct Packet packet = LL2->readData();
    if (packet.totalLength > HEADER_LENGTH)
    {
        server->transmit(this, packet.datagram, packet.totalLength - HEADER_LENGTH);
    }
}

void LoRaClient::receive(struct Datagram datagram, size_t len)
{
    struct Datagram response = {0};
    int value;
    int ret;
    size_t msgLen;

    // forward all messages to LL2, except those of type 'i'(info)
    if(datagram.type == 'i'){
        if(memcmp(&datagram.message[0], "addr", 4) == 0){
            char localAddr[8] = {'\0'};
            LL2->getLocalAddress(localAddr);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            msgLen = sprintf((char *)response.message, "%s", localAddr);
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
        else if(memcmp(&datagram.message[0], "lora", 4) == 0){
            char r_table[256] = {'\0'}; //TODO: need to check size of routing table to allocate correct amount of memory
            LL2->getRoutingTable(r_table);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            msgLen = sprintf((char *)response.message, "%s", r_table);
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
        else if(memcmp(&datagram.message[0], "config", 4) == 0){
            char config[256] = {'\0'};
            LL2->getCurrentConfig(config);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            msgLen = sprintf((char *)response.message, "%s", config);
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
        else if(memcmp(&datagram.message[0], "txpower", 4) == 0){
            sscanf((char *)&datagram.message[8], "%d", &value);
            ret = LL2->setTxPower(value, 1);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            if(ret > 0){
                msgLen = sprintf((char *)response.message, "TxPower on LoRa%d set to %ddB\r\n", ret, value);
            }
            else{
                msgLen = sprintf((char *)response.message, "TxPower setting failed\r\n");
            }
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
    }
    else{
      LL2->writeData(datagram, len);
    }
}
