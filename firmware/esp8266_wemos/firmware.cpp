#define FS_NO_GLOBALS

#include <ESP8266WiFi.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <SPIFFSEditor.h>
#include <SPI.h>
#include <LoRa.h>
#include <SD.h>
#include "AsyncSDServer.ino"
#include "routing.h"

#define HEADERSIZE 4 
#define BUFFERSIZE 252

uint8_t mac[ADDR_LENGTH];
char macaddr[ADDR_LENGTH*2];
int _loraInitialized = 0;

char ssid[32] = "disaster.radio ";
const char * hostName = "disaster-node";

IPAddress local_IP(192, 162, 4, 1);
IPAddress gateway(0, 0, 0, 0);
IPAddress netmask(255, 255, 255, 0);
const char * url = "chat.disaster.radio";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

int sdInitialized = 0; // has the LoRa radio been initialized?

int retransmitEnabled = 0;
int pollingEnabled = 0;
int hashingEnabled = 1;

int beaconModeEnabled = 0;
int beaconInterval = 30000;

// for portable node (wemos d1 mini) use these settings:
const int loraChipSelect = 15; // LoRa radio chip select, GPIO15 = D8 on WeMos D1 mini
const int resetPin = 5;       // LoRa radio reset, GPIO0 = D3 
const int irqPin = 4;        // interrupt pin for receive callback?, GPIO2 = D4

// for solar-powered module use these settings:
/*
const int csPin = 2;          // LoRa radio chip select, GPIO2
const int resetPin = 5;       // LoRa radio reset (hooked to LED, unused)
const int irqPin = 16;        // interrupt pin for receive callback?, GPIO16
*/

const int SDChipSelect = 2;

//TODO: switch to volatile byte for interrupt

byte destination = 0xFF;      // destination to send to default broadcast

bool echo_on = false;

struct webSocketMessage {
    uint8_t id;
    uint8_t type;
    uint8_t delimiter;
    uint8_t data;
};

/*
  FORWARD-DEFINED FUNCTIONS
*/

void SPIenable(int opt){
    switch (opt){
        case 0: // select SD
            Serial.printf("enabling SD card");
            Serial.printf("\r\n");
            digitalWrite(loraChipSelect, HIGH);
            digitalWrite(SDChipSelect, LOW); 
            break;
        case 1: // select LORA 
            Serial.printf("enabling LoRa radio");
            Serial.printf("\r\n");
            digitalWrite(loraChipSelect, LOW);
            digitalWrite(SDChipSelect, HIGH); // select SD card first, to initialize
            break;
    }
}

void printToWS(char message[252], int messageLength){

    char msg[256] = "99c|";  
    int length = messageLength + HEADERSIZE;
    for (int i = 0 ; i < messageLength ; i++){
        msg[i+HEADERSIZE] = message[i]; 
    }
    ws.binaryAll(msg, length);
}

void storeMessage(char* message, int messageLength) {
    //store full message in log file
    File log = SD.open("/log.txt", FILE_WRITE);
    if(!log){
        Serial.printf("file open failed");
        Serial.printf("\r\n");
    }
    for(int i = 0 ; i <= messageLength ; i++){
        log.printf("%c", message[i]);
    }
    log.printf("\n");
    log.close();
}

String dumpLog() {
    String dump = "";
    File log = SD.open("/log.txt", FILE_READ);
    if(!log){
        Serial.printf("file open failed");
        Serial.printf("\r\n");
    }  
    Serial.print("reading log file: \r\n");
    while (log.available()){
        //TODO replace string with char array
        String s = log.readStringUntil('\n');
        dump += s;
        dump += "\r\n";
    }
    return dump;
}

void clearLog() {
    File log = SD.open("/log.txt", FILE_WRITE);
    log.close();
}

void printCharArray(char *buf, int len){
    for(int i = 0 ; i < len ; i++){
        Serial.printf("%c", buf[i]);
    }
    Serial.printf("\r\n");
}

void handleMessage(uint8_t message[240], uint8_t length){

    ws.binaryAll(message, length);
}

/*
  CALLBACK FUNCTIONS
*/
void onReceive(int packetSize) {

    if (packetSize == 0) return;          // if there's no packet, return

    char incoming[BUFFERSIZE];                 // payload of packet

    int incomingLength = 0;
    while (LoRa.available()) { 
        incoming[incomingLength] = (char)LoRa.read(); 
        incomingLength++;
    }

    struct Packet packet = packetreceived(incoming, incomingLength);

    //storeMessage(incoming, incomingLength);
    
    switch(packet.type){
        case 'c':
            Serial.printf("received chat message");
            Serial.printf("\r\n");
            handleMessage(packet.data, packet.totalLength-HEADER_LENGTH);
            break;
        case 'm':
            Serial.printf("received map message");
            Serial.printf("\r\n");
            handleMessage(packet.data, packet.totalLength-HEADER_LENGTH);
            break;
        case 'r':
            Serial.printf("received routing message");
            Serial.printf("\r\n");
            break;
        default:
            Serial.printf("Error: Unknown message type");
            Serial.printf("\r\n");
    }
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
        Serial.printf("ws[%s][%u] connect\r\n", server->url(), client->id());
        printToWS("Welcome to DISASTER RADIO", 26);
        if(echo_on){
            printToWS("echo enabled, to turn off, enter '$' after logging in", 54);
        }
        if(!sdInitialized){
            printToWS("WARNING: SD card not found, functionality may be limited", 57);
        }
        if(!_loraInitialized){
            printToWS("WARNING: LoRa radio not found, functionality may be limited", 60);
        }
        client->ping();
    } else if(type == WS_EVT_DISCONNECT){
        Serial.printf("ws[%s][%u] disconnect: %u\r\n", server->url(), client->id());
    } else if(type == WS_EVT_ERROR){
        Serial.printf("ws[%s][%u] error(%u): %s\r\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else if(type == WS_EVT_PONG){
        Serial.printf("ws[%s][%u] pong[%u]: %s\r\n", server->url(), client->id(), len, (len)?(char*)data:"");
    } else if(type == WS_EVT_DATA){

        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        char msg_id[4];
        char usr_id[32];
        char msg[256];
        int msg_length;
        int usr_id_length = 0;
        int usr_id_stop = 0;

        if(info->final && info->index == 0 && info->len == len){
            //the whole message is in a single frame and we got all of it's data

            Serial.printf("ws[%s][%u] %s-message[%llu]: \r\n", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
            //cast data to char array
            for(size_t i=0; i < info->len; i++) {
                //TODO check if info length is bigger than allocated memory
                msg[i] = (char) data[i];
                msg_length = i; 
                    
                if(msg[i] == '$'){
                    echo_on = !echo_on;
                }

                // check for stop char of usr_id
                if(msg[i] == '>'){
                    usr_id_stop = i;  
                }
            }
            msg_length++;
            msg[msg_length] = '\0';

            //parse message id 
            memcpy( msg_id, msg, 2 );
            msg_id[2] = '!';
            msg_id[3] = '\0';   

            //parse username
            for( int i = 5 ; i < usr_id_stop ; i++){
                usr_id[i-5] = msg[i];
            }
            usr_id_length = usr_id_stop - 5;

            //print message info to serial
            Serial.printf("Message Length: %d\r\n", msg_length);
            Serial.printf("Message ID: %02d%02d %c\r\n", msg_id[0], msg_id[1], msg_id[2]);
            Serial.printf("Message:");
            for( int i = 0 ; i <= msg_length ; i++){
                Serial.printf("%c", msg[i]);
            }
            Serial.printf("\r\n");

            //TODO delay ack based on estimated transmit time
            //send ack to websocket
            ws.binary(client->id(), msg_id, 3);

            //transmit message over LoRa
            //uint8_t data[240] = "Hola";
            //int dataLength = 4;
            uint8_t destination[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
            struct Packet packet = buildPacket(1, mac, destination, messageCount(), 'c', data, msg_length); 
            pushToBuffer(packet);

            //echoing message to ws
            if(echo_on){
                /*
                char echo[256]; 
                char prepend[7] = "<echo>";
                int prepend_length= 6;
                memcpy(echo, msg, 4);
                for( int i = 0 ; i < prepend_length ; i++){
                    echo[4+i] = prepend[i];
                }
                for( int i = 0 ; i < msg_length-usr_id_stop ; i++){
                    echo[4+prepend_length+i] = msg[i+usr_id_stop+1];
                }
                int echo_length = prepend_length - usr_id_length + msg_length - 1;
                */
                ws.binaryAll(msg, msg_length);
            }
        } 
        else {
            //TODO message is comprised of multiple frames or the frame is split into multiple packets

        }
    }
}

/*
  SETUP FUNCTIONS
*/
void wifiSetup(){
    WiFi.macAddress(mac);
    sprintf(macaddr, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac [5]);
    setLocalAddress(macaddr);
    strcat(ssid, macaddr);
    WiFi.hostname(hostName);
    WiFi.mode(WIFI_AP);
    //WiFi.softAPConfig(local_IP, gateway, netmask);
    WiFi.softAP(ssid);
}

void mdnsSetup(){
    if(!MDNS.begin("disaster")){
        Serial.printf("Error setting up mDNS\r\n");
        while(1) {
            delay(1000);
        }
    }
    Serial.printf("mDNS responder started\r\n");

    MDNS.addService("http", "tcp", 80);
}

void sdCardSetup(){
    Serial.print("\r\nWaiting for SD card to initialise...");
    if (SD.begin(SDChipSelect, 32000000)) { // CS is D8 in this example
        Serial.print("SD Card initialized");
        Serial.print("\r\n");
        sdInitialized = 1;
        File entry;
        File dir = SD.open("/");
        dir.rewindDirectory();
        Serial.print("ROOT DIRECTORY:");
        Serial.print("\r\n");
        while(true){
            entry = dir.openNextFile();
            if (!entry) break;
            Serial.printf("%s, type: %s, size: %ld", entry.name(), (entry.isDirectory())?"dir":"file", entry.size());
            entry.close();
            Serial.print("\r\n");
        }
        dir.close();
    } else{
        Serial.print("SD Card Not Found!");
        Serial.print("\r\n");
    }
}

void spiffsSetup(){
    if (SPIFFS.begin()) {
        Serial.print("ok\r\n");
        if (SPIFFS.exists("/index.htm")) {
            Serial.printf("The file exists!\r\n");
            fs::File f = SPIFFS.open("/index.htm", "r");
            if (!f) {
                Serial.printf("Some thing went wrong trying to open the file...\r\n");
            }
            else {
                int s = f.size();
                Serial.printf("Size=%d\r\n", s);
                String data = f.readString();
                Serial.printf("%s\r\n", data.c_str());
                f.close();
            }
        }
        else {
            Serial.printf("No such file found.\r\n");
        }
    }
}


void webServerSetup(){

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    events.onConnect([](AsyncEventSourceClient *client){
        client->send("hello!",NULL,millis(),1000);
    });

    server.addHandler(&events);

    if(sdInitialized){
        server.addHandler(new AsyncStaticSDWebHandler("/", SD, "/", "max-age=604800"));
    }else{
        server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");
    }
    
    server.onNotFound([](AsyncWebServerRequest *request){
        Serial.printf("NOT_FOUND: ");
        if(request->method() == HTTP_GET)
        Serial.printf("GET");
        else if(request->method() == HTTP_POST)
        Serial.printf("POST");
        else if(request->method() == HTTP_DELETE)
        Serial.printf("DELETE");
        else if(request->method() == HTTP_PUT)
        Serial.printf("PUT");
        else if(request->method() == HTTP_PATCH)
        Serial.printf("PATCH");
        else if(request->method() == HTTP_HEAD)
        Serial.printf("HEAD");
        else if(request->method() == HTTP_OPTIONS)
        Serial.printf("OPTIONS");
        else
        Serial.printf("UNKNOWN");
        Serial.printf(" http://%s%s\r\n", request->host().c_str(), request->url().c_str());

        if(request->contentLength()){
            Serial.printf("_CONTENT_TYPE: %s\r\n", request->contentType().c_str());
            Serial.printf("_CONTENT_LENGTH: %u\r\n", request->contentLength());
        }

        int headers = request->headers();
        int i;
        for(i=0;i<headers;i++){
            AsyncWebHeader* h = request->getHeader(i);
            Serial.printf("_HEADER[%s]: %s\r\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for(i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                Serial.printf("_FILE[%s]: %s, size: %u\r\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                Serial.printf("_POST[%s]: %s\r\n", p->name().c_str(), p->value().c_str());
            } else {
                Serial.printf("_GET[%s]: %s\r\n", p->name().c_str(), p->value().c_str());
            }
        }

        request->send(404);
    });

    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        if(!index)
        Serial.printf("BodyStart: %u\r\n", total);
        Serial.printf("%s", (const char*)data);
        if(index + len == total)
        Serial.printf("BodyEnd: %u\r\n", total);
    });

    server.begin();

}

int loraInitialized(){
    if(_loraInitialized){
        return 1;
    }else{
        return 0;
    }
}

void loraSetup(){

    // override the default CS, reset, and IRQ pins (optional)
    LoRa.setPins(loraChipSelect, resetPin, irqPin); // set CS, reset, IRQ pin

    if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
        Serial.printf("LoRa init failed. Check your connections.\r\n");
        return;
    }

    LoRa.setSPIFrequency(100E3);
    LoRa.setSpreadingFactor(9);           // ranges from 6-12,default 7 see API docs
    LoRa.onReceive(onReceive);
    LoRa.receive();

    _loraInitialized = 1;

    Serial.printf("LoRa init succeeded.\r\n");
}

/*
  START MAIN
*/
long startTime;
long lastRoutingTime; // time of last packet send
int routingInterval = 10000 + random(5000);    // 5-15 seconds

void setup(){
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    pinMode(loraChipSelect, OUTPUT);
    pinMode(SDChipSelect, OUTPUT);
    pinMode(irqPin, INPUT);

    wifiSetup();
    mdnsSetup();
    SPIenable(0); //SD
    sdCardSetup();
    if(!sdInitialized){
        spiffsSetup();
    }    
    webServerSetup();
    loraSetup();

    uint8_t* myAddress = localAddress();
    Serial.printf("local address: ");
    printAddress(myAddress);
    Serial.printf("\n");

    startTime = getTime();
    lastRoutingTime = startTime;
}

void loop(){

        //Serial.printf("learning... %d\r", getTime() - startTime);
        checkBuffer(); 
        long timestamp = transmitRoutes(routingInterval, lastRoutingTime);
        if(timestamp){
            Serial.print("routes transmitted");
            lastRoutingTime = timestamp;
        }
}
