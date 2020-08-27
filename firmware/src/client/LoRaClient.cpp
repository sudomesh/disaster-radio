#include "LoRaClient.h"

bool LoRaClient::init()
{
    LL2->init(); // initialize Layer2
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
    double value2;
    long value3;
    long ret;
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
        else if(memcmp(&datagram.message[0], "config", 5) == 0){
            char config[256] = {'\0'};
            LL2->getCurrentConfig(config);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            msgLen = sprintf((char *)response.message, "%s", config);
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
        else if(memcmp(&datagram.message[0], "txpower", 7) == 0){
            sscanf((char *)&datagram.message[8], "%d", &value);
            ret = LL2->setTxPower(value, 1);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            if(ret > 0){
                msgLen = sprintf((char *)response.message, "TxPower on LoRa%ld set to %ddB\r\n", ret, value);
            }
            else{
                msgLen = sprintf((char *)response.message, "TxPower setting failed\r\n");
            }
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
        else if(memcmp(&datagram.message[0], "sf", 2) == 0){
            sscanf((char *)&datagram.message[3], "%d", &value);
            ret = LL2->setSpreadingFactor(value, 1);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            if(ret > 0){
                msgLen = sprintf((char *)response.message, "SpreadingFactor on LoRa%ld set to %d\r\n", ret, value);
            }
            else{
                msgLen = sprintf((char *)response.message, "SpreadingFactor setting failed\r\n");
            }
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
        else if(memcmp(&datagram.message[0], "duty", 4) == 0){
            sscanf((char *)&datagram.message[5], "%lf", &value2);
            LL2->setDutyCycle(value2);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            msgLen = sprintf((char *)response.message, "Duty Cycle of LL2 set to %lf\r\n", value2);
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
        else if(memcmp(&datagram.message[0], "interval", 8) == 0){
            sscanf((char *)&datagram.message[9], "%ld", &value3);
            ret = LL2->setInterval(value3);
            memcpy(response.destination, BROADCAST, ADDR_LENGTH);
            response.type = 'i';
            if(ret > 0){
              msgLen = sprintf((char *)response.message, "Routing mode set to `auto`, interval of routing table messages set to %ld\r\n", ret);
            }
            else if(ret == 0){
              msgLen = sprintf((char *)response.message, "Routing table messages disabled, routing mode set to `manual`\r\n");
            }
            server->transmit(this, response, msgLen + DATAGRAM_HEADER);
        }
    }
    else{
      LL2->writeData(datagram, len);
    }
}
