#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include "routing.h"

#ifdef LORA
#include <LoRa.h>
#endif

#ifdef SIM
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#endif

uint8_t _localAddress[ADDR_LENGTH];
uint8_t _messageCount;

// metric weights
float packetSuccessWeight = .8;
float randomMetricWeight = .2;

// tables and buffers
struct Packet buffer[8];
int bufferEntry = 0;

uint8_t hashTable[256][SHA1_LENGTH];
uint8_t hashEntry = 0;

struct NeighborTableEntry neighborTable[255];
int neighborEntry = 0;

struct RoutingTableEntry routeTable[255];
int routeEntry = 0;

// timeout intervals
int _helloInterval = 10;
int _routeInterval = 10;
int _messageInterval = 5;
int _discoveryTimeout = 30;
int _learningTimeout = 400;
int _maxRandomDelay = 20;
int timeDistortion = 1;

uint8_t messageCount(){
    return _messageCount;
}

int simulationTime(int realTime) {
    return realTime * timeDistortion;
}
int helloInterval() {
    return simulationTime(_helloInterval);
}
int routeInterval() {
    return simulationTime(_routeInterval);
}
int messageInterval() {
    return simulationTime(_messageInterval);
}
int discoveryTimeout() {
    return simulationTime(_discoveryTimeout);
}
int learningTimeout() {
    return simulationTime(_learningTimeout);
}
int maxRandomDelay() {
    return simulationTime(_maxRandomDelay);
}

uint8_t hex_digit(char ch){
  if(( '0' <= ch ) && ( ch <= '9' )){
    ch -= '0';
  }else{
    if(( 'a' <= ch ) && ( ch <= 'f' )){
      ch += 10 - 'a';
    }else{
      if(( 'A' <= ch ) && ( ch <= 'F' ) ){
        ch += 10 - 'A';
      }else{
        ch = 16;
      }
    }
  }
  return ch;
}

int setLocalAddress(char* macString){
  for( int i = 0; i < sizeof(_localAddress)/sizeof(_localAddress[0]); ++i ){
    _localAddress[i]  = hex_digit( macString[2*i] ) << 4;
    _localAddress[i] |= hex_digit( macString[2*i+1] );
  }
  if(_localAddress){
    return 1;
  }else{
    return 0;
  }
}

uint8_t* localAddress(){
    return _localAddress;
}

#ifdef LORA
int getTime(){
    return millis();
}

int isHashNew(char incoming[SHA1_LENGTH]){
    int hashNew = 1;
    for( int i = 0 ; i <= hashEntry ; i++){
        if(strcmp(incoming, (char*) hashTable[i]) == 0){
            hashNew = 0; 
        }
    }
    if( hashNew ){
        Serial.printf("New message received");
        Serial.printf("\r\n");
        for( int i = 0 ; i < SHA1_LENGTH ; i++){
            hashTable[hashEntry][i] = incoming[i];
        }
        hashEntry++;
    }
    return hashNew;
}

int send_packet(char* data, int len){

    Serial.printf("Sending: ");
    if(LoRa.beginPacket()){
        for( int i = 0 ; i < len ; i++){
            LoRa.write(data[i]);
            Serial.printf("%02x", data[i]);
        }
        Serial.printf("\r\n");
        LoRa.endPacket(1);
        LoRa.receive();
    }
}
#endif

#ifdef SIM
int getTime(){
    return time(NULL);
}

int isHashNew(uint8_t hash[SHA1_LENGTH]){

    int hashNew = 1;
    Serial.printf("hash is %x\n", hash);
    for( int i = 0 ; i <= hashEntry ; i++){
        if(strcmp(hash, hashTable[i]) == 0){
            hashNew = 0; 
            Serial.printf("Not new!\n");
        }
    }
    if(hashNew){
        // add to hash table
        Serial.printf("New message received");
        Serial.printf("\r\n");
        for( int i = 0 ; i < SHA1_LENGTH ; i++){
            hashTable[hashEntry][i] = hash[i];
        }
        hashEntry++;
    }
    return hashNew;
}

int send_packet(char* data, uint8_t len) {

    char packet[258];
    ssize_t written = 0;
    ssize_t ret;
    if(!len) {
        len = strlen(data);
    }
    if(len > 256) {
        fprintf(stderr, "Attempted to send packet larger than 256 bytes\n");
        return -1;
    }
    packet[0] = len;
    memcpy(packet+1, data, len);
    while(written < len) {
        ret = write(STDOUT, (void*) packet, len+1);
        if(ret < 0) {
            return ret;
        }
        written += ret;
    }
    printf("\n");
    fflush(stdout);
    return 0;
}
#endif

int debug_printf(const char* format, ...) {

    if(DEBUG){
        int ret;
        va_list args;
        va_start(args, format);
        ret = vfprintf(stderr, format, args);
        va_end(args);
        fflush(stderr);
        return ret;
    }else{
        return 0;
    }
}

int sendPacket(struct Packet packet) {

    uint8_t* sending = (uint8_t*) malloc(sizeof(packet));
    memcpy(sending, &packet, sizeof(packet));
    /*
    int send = 1;
    if(hashingEnabled){
        // do not send message if already transmitted once
        //uint8_t hash[SHA1_LENGTH];
        //SHA1(sending, packet.totalLength, hash);
        //if(isHashNew(hash)){
        //  send = 0;
        //}
    }
    */
    send_packet((char*) sending, packet.totalLength);
    _messageCount++;
    return _messageCount;
}

void pushToBuffer(struct Packet packet){

    if(bufferEntry > 7){
        bufferEntry = 0;
    }

    memset(&buffer[bufferEntry], 0, sizeof(buffer[bufferEntry]));
    memcpy(&buffer[bufferEntry], &packet, sizeof(buffer[bufferEntry]));
    bufferEntry++;
}

struct Packet popFromBuffer(){

    bufferEntry--;
    struct Packet pop;
    memcpy(&pop, &buffer[bufferEntry], sizeof(pop));
    return pop; 
}

void checkBuffer(){

    if (bufferEntry > 0){
        struct Packet packet = popFromBuffer();
        sendPacket(packet);
    }
    //else buffer is empty;
}

struct Packet buildPacket( uint8_t ttl, uint8_t src[6], uint8_t dest[6], uint8_t sequence, uint8_t type, uint8_t data[240], uint8_t dataLength){

    uint8_t packetLength = HEADER_LENGTH + dataLength;
    uint8_t* buffer = (uint8_t*)  malloc(dataLength);
    buffer = (uint8_t*) data;
    struct Packet packet = {
        ttl,
        packetLength,
        src[0], src[1], src[2], src[3], src[4], src[5],
        dest[0], dest[1], dest[2], dest[3], dest[4], dest[5],
        sequence,
        type 
    };
    memcpy(&packet.data, buffer, packet.totalLength);
    return packet;
}

void printMetadata(struct Metadata metadata){
    Serial.printf("RSSI: %x\n", metadata.rssi);
    Serial.printf("SNR: %x\n", metadata.snr);
}

void printPacketInfo(struct Packet packet){

    Serial.printf("\r\n");
    Serial.printf("ttl: %d\r\n", packet.ttl);
    Serial.printf("length: %d\r\n", packet.totalLength);
    Serial.printf("source: ");
    for(int i = 0 ; i < ADDR_LENGTH ; i++){
        Serial.printf("%x", packet.source[i]);
    }
    Serial.printf("\r\n");
    Serial.printf("destination: ");
    for(int i = 0 ; i < ADDR_LENGTH ; i++){
        Serial.printf("%x", packet.destination[i]);
    }
    Serial.printf("\r\n");
    Serial.printf("sequence: %02x\r\n", packet.sequence);
    Serial.printf("type: %c\r\n", packet.type);
    Serial.printf("data: ");
    for(int i = 0 ; i < packet.totalLength-HEADER_LENGTH ; i++){
        Serial.printf("%02x", packet.data[i]);
    }
    Serial.printf("\r\n");
}

void printNeighborTable(){

    Serial.printf("\n");
    Serial.printf("Neighbor Table:\n");
    for( int i = 0 ; i < neighborEntry ; i++){
        for(int j = 0 ; j < ADDR_LENGTH ; j++){
            Serial.printf("%02x", neighborTable[i].address[j]);
        }
        Serial.printf(" %3d ", neighborTable[i].metric);
        Serial.printf("\n");
    }
    Serial.printf("\n");
}

void printRoutingTable(){

    Serial.printf("\n");
    Serial.printf("Routing Table: total routes %d\n", routeEntry);
    for( int i = 0 ; i < routeEntry ; i++){
        Serial.printf("%d hops from ", routeTable[i].distance);
        for(int j = 0 ; j < ADDR_LENGTH ; j++){
            Serial.printf("%02x", routeTable[i].destination[j]);
        }
        Serial.printf(" via ");
        for(int j = 0 ; j < ADDR_LENGTH ; j++){
            Serial.printf("%02x", routeTable[i].nextHop[j]);
        }
        Serial.printf(" metric %3d ", routeTable[i].metric);
        Serial.printf("\n");
    }
    Serial.printf("\n\n");
}

void printAddress(uint8_t address[ADDR_LENGTH]){
    for( int i = 0 ; i < ADDR_LENGTH; i++){
        Serial.printf("%02x", address[i]);
    }
}

uint8_t calculatePacketLoss(int entry, uint8_t sequence){

    uint8_t packet_loss;
    uint8_t sequence_diff = sequence - neighborTable[entry].lastReceived;
    if(sequence_diff == 0){
        // this is first packet received from neighbor
        // assume perfect packet success
        neighborTable[entry].packet_success = 0xFF; 
        packet_loss = 0x00; 
    }else if(sequence_diff == 1){
        // do not decrease packet success rate
        packet_loss = 0x00; 
    }else if(sequence_diff > 1 && sequence_diff < 16){
        // decrease packet success rate by difference
        packet_loss = 0x10 * sequence_diff; 
    }else if(sequence_diff > 16){
        // no packet received recently
        // assume complete pakcet loss
        packet_loss = 0xFF; 
    }
    return packet_loss;
}

uint8_t calculateMetric(int entry, uint8_t sequence, struct Metadata metadata){

    float weightedPacketSuccess =  ((float) neighborTable[entry].packet_success)*packetSuccessWeight;
    float weightedRandomness =  ((float) metadata.randomness)*randomMetricWeight;
    //float weightedRSSI =  ((float) metadata.rssi)*RSSIWeight;
    //float weightedSNR =  ((float) metadata.snr)*SNRWeight;
    uint8_t metric = weightedPacketSuccess+weightedRandomness;
    debug_printf("weighted packet success: %3f\n", weightedPacketSuccess);
    debug_printf("weighted randomness: %3f\n", weightedRandomness);
    //debug_printf("weighted RSSI: %3f\n", weightedRSSI);
    //debug_printf("weighted SNR: %3f\n", weightedSNR);
    debug_printf("metric calculated: %3d\n", metric);
    return metric;
}

int checkNeighborTable(struct NeighborTableEntry neighbor){

    int entry = routeEntry;
    for( int i = 0 ; i < neighborEntry ; i++){
        //had to use memcmp instead of strcmp?
        if(memcmp(neighbor.address, neighborTable[i].address, sizeof(neighbor.address)) == 0){
            entry = i; 
        }
    }
    return entry;
}

int checkRoutingTable(struct RoutingTableEntry route){

    int entry = routeEntry; // assume this is a new route
    for( int i = 0 ; i < routeEntry ; i++){
        if(memcmp(route.destination, _localAddress, sizeof(route.destination)) == 0){
            //this is me don't add to routing table 
            //debug_printf("this route is my local address\n");
            entry = -1;
            return entry;
        }else 
        if(memcmp(route.destination, routeTable[i].destination, sizeof(route.destination)) == 0){
            if(memcmp(route.nextHop, routeTable[i].nextHop, sizeof(route.nextHop)) == 0){
                // already have this exact route, update metric
                entry = i; 
                return entry;
            }else{
                // already have this destination, but via a different neighbor
                if(route.distance < routeTable[i].distance){
                    // replace route if distance is better 
                    entry = i;
                }else 
                if(route.distance == routeTable[i].distance){
                    if(route.metric > routeTable[i].metric){
                    // replace route if distance is equal and metric is better 
                        entry = i;
                    }else{
                        entry = -1;
                    }
                }else{
                    // ignore route if distance and metric are worse
                    entry = -1;
                }
                return entry;
            }
        } 
    }
    return entry;
}

int updateNeighborTable(struct NeighborTableEntry neighbor, int entry){

    memset(&neighborTable[entry], 0, sizeof(neighborTable[entry]));
    memcpy(&neighborTable[entry], &neighbor, sizeof(neighborTable[entry]));
    if(entry == neighborEntry){
        neighborEntry++;
        debug_printf("new neighbor found: ");
    }else{
        debug_printf("neighbor updated! ");
    }
    return entry;
}

int updateRouteTable(struct RoutingTableEntry route, int entry){

    memset(&routeTable[entry], 0, sizeof(routeTable[entry]));
    memcpy(&routeTable[entry], &route, sizeof(routeTable[entry]));
    if(entry == routeEntry){
        routeEntry++;
        debug_printf("new route found! ");
    }else{
        debug_printf("route updated! ");
    }
    printAddress(routeTable[entry].destination);
    debug_printf("\n");
    return entry;
}

int selectRoute(struct Packet packet){

    int entry = -1;
    for( int i = 0 ; i < routeEntry ; i++){
        if(memcmp(packet.destination, routeTable[i].destination, sizeof(packet.destination)) == 0){
            entry = i;
        }
    }
    return entry;
}

void retransmitRoutedPacket(struct Packet packet, struct RoutingTableEntry route){

    // decrement ttl
    packet.ttl--;
    Serial.printf("retransmitting\n");
    uint8_t data[240];
    int dataLength = 0;
    for( int i = 0 ; i < ADDR_LENGTH ; i++){
        data[dataLength] = route.nextHop[i];
        dataLength++;
    }
    struct Packet newMessage = buildPacket(packet.ttl, packet.source, packet.destination, packet.sequence, packet.type, data, dataLength); 

    // queue packet to be transmitted
    pushToBuffer(newMessage);
}

int parseHelloPacket(struct Packet packet, struct Metadata metadata){

    struct NeighborTableEntry neighbor;
    memcpy(neighbor.address, packet.source, sizeof(neighbor.address));
    int n_entry = checkNeighborTable(neighbor);
    neighbor.lastReceived = packet.sequence;
    uint8_t packet_loss = calculatePacketLoss(n_entry, packet.sequence);
    neighbor.packet_success = neighborTable[n_entry].packet_success - packet_loss;
    uint8_t metric = calculateMetric(n_entry, packet.sequence, metadata); 
    neighbor.metric = metric;
    updateNeighborTable(neighbor, n_entry);  

    struct RoutingTableEntry route;
    memcpy(route.destination, packet.source, ADDR_LENGTH);
    memcpy(route.nextHop, packet.source, ADDR_LENGTH);
    route.distance = 1;
    route.metric = neighborTable[n_entry].metric;
    int r_entry = checkRoutingTable(route);
    if(r_entry == -1){
        debug_printf("do nothing, already have better route to ");
        printAddress(route.destination);
        debug_printf("\n");
    }else{
        //if(routeEntry <= 30){
        updateRouteTable(route, r_entry);
        //}
    }
    return n_entry;
}

int parseRoutingPacket(struct Packet packet, struct Metadata metadata){
    int numberOfRoutes = (packet.totalLength - HEADER_LENGTH) / (ADDR_LENGTH+2);
    debug_printf("routes in packet: %d\n", numberOfRoutes);

    int n_entry = parseHelloPacket(packet, metadata);

    for( int i = 0 ; i < numberOfRoutes ; i++){
        struct RoutingTableEntry route; 
        memcpy(route.destination, packet.data + (ADDR_LENGTH+2)*i, ADDR_LENGTH);
        memcpy(route.nextHop, packet.source, ADDR_LENGTH);
        route.distance = packet.data[(ADDR_LENGTH+2)*i + ADDR_LENGTH]; 
        route.distance++; // add a hop to distance
        float metric = (float) packet.data[(ADDR_LENGTH+2)*i + ADDR_LENGTH+1];

        int entry = checkRoutingTable(route);
        if(entry == -1){
            debug_printf("do nothing, already have route to ");
            printAddress(route.destination);
            debug_printf("\n");
        }else{
            // average neighbor metric with rest of route metric
            float hopRatio = 1/((float)route.distance);
            metric = ((float) neighborTable[n_entry].metric)*(hopRatio) + ((float)route.metric)*(1-hopRatio);
            route.metric = (uint8_t) metric;
            //if(routeEntry <= 30){
            updateRouteTable(route, entry);
            //}
        }
    }
    return numberOfRoutes;
}

void parseChatPacket(struct Packet packet){
    
    if(memcmp(packet.destination, _localAddress, sizeof(packet.destination)) == 0){
        Serial.printf("this message is for me\n");
        return;
    }
    uint8_t nextHop[ADDR_LENGTH];
    memcpy(nextHop, packet.data, sizeof(nextHop));
    if(memcmp(nextHop, _localAddress, sizeof(nextHop)) == 0){
        Serial.printf("I am the next hop ");
        int entry = selectRoute(packet);
        if(entry == -1){
            Serial.printf(" but I don't have a route\n");
        }else{
            Serial.printf(" and I have a route RETRANSMIT\n");
            retransmitRoutedPacket(packet, routeTable[entry]);
        }
    }else{
        Serial.printf("I am not the next hop, packet dropped\n");
    }
}
    
struct Packet packetreceived(char* data, size_t len) {

    data[len] = '\0';

    // convert ASCII data to pure bytes
    uint8_t* byteData = ( uint8_t* ) data;
    
    // randomly generate RSSI and SNR values 
    // see https://github.com/sudomesh/disaster-radio-simulator/issues/3
    //uint8_t packet_rssi = rand() % (256 - 128) + 128;
    //uint8_t packet_snr = rand() % (256 - 128) + 128;
    // articial packet loss
    //uint8_t packet_randomness = rand() % (256 - 128) + 128;

    
    struct Metadata metadata;
    struct Packet packet = {
        byteData[0],
        byteData[1], 
        byteData[2], byteData[3], byteData[4], byteData[5], byteData[6], byteData[7],
        byteData[8], byteData[9], byteData[10], byteData[11], byteData[12], byteData[13],
        byteData[14],
        byteData[15],
    };
    memcpy(packet.data, byteData + HEADER_LENGTH, packet.totalLength-HEADER_LENGTH);

    //printPacketInfo(packet);
    
    switch(packet.type){
        case 'h' :
            // hello packet;
            parseHelloPacket(packet, metadata);
            //printNeighborTable();
            break;
        case 'r':
            // routing packet;
            //parseHelloPacket(packet, metadata);
            parseRoutingPacket(packet, metadata);
            //printRoutingTable();
            break;
        case 'c' :
            // chat packet
            parseChatPacket(packet);
            //Serial.printf("this is a chat message\n");
            break;
        case 'm' :
            Serial.printf("this is a map message\n");
            break;
        default :
            printPacketInfo(packet);
            Serial.printf("message type not found\n");
    }
    return packet;
}

long transmitHello(long interval, long lastTime){

    long newLastTime = 0;
    if (getTime() - lastTime > interval) {
        uint8_t data[240] = "Hola";
        int dataLength = 4;
        //TODO: add randomness to message to avoid hashisng issues
        uint8_t destination[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
        struct Packet helloMessage = buildPacket(1, _localAddress, destination, _messageCount, 'h', data, dataLength); 
        sendPacket(helloMessage);
        newLastTime = getTime();
    }
    return newLastTime;
}

long transmitRoutes(long interval, long lastTime){

    long newLastTime = 0;
    if (getTime() - lastTime > interval) {
        uint8_t data[240];
        int dataLength = 0;
        Serial.printf("transmitting routes\r\n");
        debug_printf("number of routes before transmit: %d\n", routeEntry);
        int routesPerPacket = routeEntry;
        if (routeEntry >= MAX_ROUTES_PER_PACKET-1){
            routesPerPacket = MAX_ROUTES_PER_PACKET-1;
        }
        // random select without replacement of routes
        for( int i = 0 ; i < routesPerPacket ; i++){
            for( int j = 0 ; j < ADDR_LENGTH ; j++){
                data[dataLength] = routeTable[i].destination[j];
                dataLength++;
            }
            data[dataLength] = routeTable[i].distance; //distance
            dataLength++;
            data[dataLength] = routeTable[i].metric;
            dataLength++;
        }
        debug_printf("Sending data: ");
        for(int i = 0 ; i < dataLength ; i++){
            debug_printf("%02x ", data[i]);
        }
        debug_printf("\n");
        uint8_t destination[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
        struct Packet routeMessage = buildPacket(1, _localAddress, destination, _messageCount, 'r', data, dataLength); 
        printPacketInfo(routeMessage);
        sendPacket(routeMessage);
        newLastTime = getTime();
    }
    return newLastTime;
}

long transmitToRandomRoute(long interval, long lastTime){
    long newLastTime = 0;
    if (getTime() - lastTime > interval) {
        if (routeEntry == 0){
            Serial.printf("trying to send but I have no routes ");
            newLastTime = getTime();
            return newLastTime;
        }
        int choose = rand()%routeEntry;
        uint8_t destination[ADDR_LENGTH];
        memcpy(destination, &routeTable[choose].destination, sizeof(destination));

        Serial.printf("trying to send a random message to ");
        for( int j = 0 ; j < ADDR_LENGTH ; j++){
            Serial.printf("%02x", routeTable[choose].destination[j]);
        }
        Serial.printf(" via ");
        for( int j = 0 ; j < ADDR_LENGTH ; j++){
            Serial.printf("%02x", routeTable[choose].nextHop[j]);
        }
        Serial.printf("\n");

        uint8_t data[240];
        int dataLength = 0;
        for( int i = 0 ; i < ADDR_LENGTH ; i++){
            data[dataLength] = routeTable[choose].nextHop[i];
            dataLength++;
        }
        struct Packet randomMessage = buildPacket(32, _localAddress, destination, _messageCount, 'c', data, dataLength); 
        sendPacket(randomMessage);
        _messageCount++;
        newLastTime = getTime();
    }
    return newLastTime;
}

