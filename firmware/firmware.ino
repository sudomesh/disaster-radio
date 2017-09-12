#include <ESP8266WiFi.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <SPI.h>
#include <LoRa.h>

// This is just a slightly modified version of the
// official ESPAsyncWebserver example
// Where the SetSpread example has been added 
// from the LoRa library examples 

// SKETCH BEGIN
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

byte mac[6];
char ssid[32] = "DisasterRadio ";
//const char* password = "*******";
const char * hostName = "esp-async";
const char* http_username = "admin";
const char* http_password = "admin";

const int csPin = 15;          // LoRa radio chip select, GPIO15 = D8 on WeMos D1 mini
const int resetPin = 5;       // LoRa radio reset ,GPIO5 = D1
const int irqPin = 4;        // interrupt pin for receive callback?, GPIO4 = D2

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress;     // assigned to last byte of mac address in setup
byte destination = 0xFF;      // destination to send to default broadcast
int interval = 2000;          // interval between sends
long lastSendTime = 0; // time of last packet send

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";                 // payload of packet

  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.printf("error: message length does not match length\r\n");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.printf("This message is not for me.\r\n");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.printf("Received from: 0x%02x/r/n", sender);
  Serial.printf("Sent to: 0x%02x/r/n", recipient);
  Serial.printf("Message ID: %d/r/n", incomingMsgId);
  Serial.printf("Message length: %d/r/n", incomingLength);
  Serial.printf("Message: %s/r/n", incoming.c_str());
  Serial.printf("RSSI: %f/r/n", LoRa.packetRssi());
  Serial.printf("Snr: %f/r/n", LoRa.packetSnr());

  char text[20] = "You got a message: ";
  char msg[100];
  incoming.toCharArray(msg, incomingLength);
  strcat(text, msg);
  // TODO send message to websocket chat
  //ws.textAll((char*)text);

}


void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\r\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect: %u\r\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\r\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\r\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }

      sendMessage(msg);
      Serial.printf("Sending %s\r\n", msg.c_str());
      LoRa.receive();

      /*
      if(info->opcode == WS_TEXT)
        client->text("Your message was broadcast");
      else
        client->binary("Your binary message was broadcast");
      */
    } 
    else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\r\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\r\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\r\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\r\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\r\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}


void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  char macaddr[14];
  WiFi.macAddress(mac);
  //this could be done more loopy, but it works
  sprintf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[5], mac[4], mac[3], mac[2], mac[1], mac [0]);
  strcat(ssid, macaddr);
  WiFi.hostname(hostName);
  //WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);

  if (SPIFFS.begin()) {
    Serial.print("ok\r\n");
    if (SPIFFS.exists("/index.html")) {
      Serial.printf("The file exists!\r\n");
      File f = SPIFFS.open("/index.html", "r");
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

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  events.onConnect([](AsyncEventSourceClient *client){
    client->send("hello!",NULL,millis(),1000);
  });
  server.addHandler(&events);

  server.addHandler(new SPIFFSEditor(http_username,http_password));

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

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
  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf("UploadStart: %s\r\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\r\n", filename.c_str(), index+len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\r\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\r\n", total);
  });
  server.begin();

  Serial.printf("LoRa Duplex - Set spreading factor\r\n");

  localAddress = mac[0];

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin); // set CS, reset, IRQ pin

  if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.printf("LoRa init failed. Check your connections.\r\n");
    while (true);                       // if failed, do nothing
  }

  LoRa.setSpreadingFactor(12);           // ranges from 6-12,default 7 see API docs
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.printf("LoRa init succeeded.\r\n");
  Serial.printf("local address: %02x\r\n", localAddress);
  Serial.printf("%s\r\n", macaddr);
}

/*int mcount = 0;
int ecount = 0;
char last = 0;

void beaconMode(){
  if (millis() - lastSendTime > interval) {
    String message = "HeLoRa World! ";   // send a message
    message += msgCount;
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
    msgCount++;
  }
}*/

void loop(){
  //everything is done in callbacks
}
