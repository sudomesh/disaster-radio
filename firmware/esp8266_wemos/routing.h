#include <unistd.h>
#include <stdint.h>

#define LORA
#define DEBUG 0
#define HEADER_LENGTH 16
#define SHA1_LENGTH 40
#define ADDR_LENGTH 6 
#define MAX_ROUTES_PER_PACKET 30
#define ASYNC_TX 1;

int getTime();

int simulationTime(int realTime);
int helloInterval();
int routeInterval();
int messageInterval();
int discoveryTimeout();
int learningTimeout();
int maxRandomDelay();

struct Metadata {
    uint8_t rssi;
    uint8_t snr;
    uint8_t randomness;
};

struct Packet {
    uint8_t ttl;
    uint8_t totalLength;
    uint8_t source[ADDR_LENGTH];
    uint8_t destination[ADDR_LENGTH];
    uint8_t sequence;
    uint8_t type;
    uint8_t data[240];
};

struct RoutedMessage {
    uint8_t nextHop[6];
    uint8_t data[234];
};

struct NeighborTableEntry{
    uint8_t address[ADDR_LENGTH];
    uint8_t lastReceived;
    uint8_t packet_success;
    uint8_t metric;
};

struct RoutingTableEntry{
    uint8_t destination[ADDR_LENGTH];
    uint8_t nextHop[ADDR_LENGTH];
    uint8_t distance;
    uint8_t lastReceived;
    uint8_t metric;
};

uint8_t messageCount();
int setLocalAddress(char* macString);
uint8_t* localAddress();

int isHashNew(char incoming[SHA1_LENGTH]);

int send_packet(char* data, int len);

int debug_printf(const char* format, ...);

int sendPacket(struct Packet packet);

void pushToBuffer(struct Packet packet);

struct Packet popFromBuffer();

void checkBuffer();

struct Packet buildPacket( uint8_t ttl, uint8_t src[6], uint8_t dest[6], uint8_t sequence, uint8_t type, uint8_t data[240], uint8_t dataLength);

void printMetadata(struct Metadata metadata);

void printPacketInfo(struct Packet packet);

void printNeighborTable();

void printRoutingTable();

void printAddress(uint8_t address[ADDR_LENGTH]);

//uint8_t calculatePacketLoss(int entry, uint8_t sequence);

//uint8_t calculateMetric(int entry, uint8_t sequence, struct Metadata metadata);

//int checkNeighborTable(struct NeighborTableEntry neighbor);

//int checkRoutingTable(struct RoutingTableEntry route);

//int updateNeighborTable(struct NeighborTableEntry neighbor, int entry);

//int updateRouteTable(struct RoutingTableEntry route, int entry);

//int selectRoute(struct Packet packet);

//void retransmitRoutedPacket(struct Packet packet, struct RoutingTableEntry route);

//int parseHelloPacket(struct Packet packet, struct Metadata metadata);

//int parseRoutingPacket(struct Packet packet, struct Metadata metadata);

struct Packet packetreceived(char* data, size_t len);

long transmitHello(long interval, long lastTime);

long transmitRoutes(long interval, long lastTime);

long transmitToRandomRoute(long interval, long lastTime);



