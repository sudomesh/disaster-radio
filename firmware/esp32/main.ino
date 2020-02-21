#include <Arduino.h>

// required for LoRa and/or AsyncSDServer libraries?
#include <FS.h>
#include <SPI.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <AsyncSDServer.h>
#include <SD.h>
#include <SPIFFS.h>
#include "SSD1306Wire.h"

#include "config.h"

// server
#include "server/DisasterRadio.h"
// client
#include "client/OLEDClient.h"
#include "client/HistoryRecord.h"
#include "client/LoRaClient.h"
#include "client/StreamClient.h"
#include "client/TCPClient.h"
#include "client/WebSocketClient.h"
#include "client/GPSClient.h"
#ifdef USE_BLE
#include "client/BleUartClient.h"
#endif
// middleware
#include "middleware/Console.h"
#include "middleware/HistoryReplay.h"
// history
#include "history/HistorySD.h"
#include "history/HistoryMemory.h"

#include "DisasterHistory.h"
#include "DisasterMiddleware.h"

#define MDNS_NAME_PREFIX "disaster"
#define HOST_NAME "disaster.radio"
#define WIFI_AP_SSID_PREFIX "disaster.radio "

#define INDEX_FILE "index.htm"

SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

SPIClass sd_card(HSPI);

AsyncServer tcp_server(23);
AsyncWebServer http_server(80);
AsyncWebSocket ws_server("/ws");

#ifdef USE_BLE
BleUartClient ble_client;
#endif

DisasterRadio *radio = new DisasterRadio();
DisasterHistory *history = NULL;

int internal_clients = 0;
int clients = 0;
int routes = 0;

#define WIFI_POLL_DELAY 500
#define WIFI_POLL_TRIES 10

char macaddr[12 + 1] = {'\0'};
char ssid[100] = {'\0'};
IPAddress ip;

bool sdInitialized = false;
bool spiffsInitialized = false;
bool displayInitialized = false;
bool loraInitialized = false;

void setupWiFi()
{
  Serial.println("* Initializing WiFi...");

  // get the WiFi MAC address
  uint8_t mac[6];
  WiFi.macAddress(mac);

  // format WiFi MAC address to string
  sprintf(macaddr, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

#ifdef WIFI_SSID
  Serial.printf(" --> Connecting to WiFi \"%s\"\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int i = 0; i < WIFI_POLL_TRIES && WiFi.status() != WL_CONNECTED; i++)
  {
    delay(WIFI_POLL_DELAY);
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    ip = WiFi.localIP();
    snprintf(ssid, sizeof(ssid), "%s", WIFI_SSID);

    Serial.print(" --> WiFi connected, IP address: ");
    Serial.println(ip);
  }
  else
  {
    Serial.println(" --> Unable to connect to WiFi");
  }
#endif

  if (WiFi.status() != WL_CONNECTED)
  {
    // format full SSID with MAC address suffix
    snprintf(ssid, sizeof(ssid), "%s%s", WIFI_AP_SSID_PREFIX, macaddr);

    // start the AP
    WiFi.mode(WIFI_AP);
    WiFi.setHostname(HOST_NAME);
    WiFi.softAP(ssid);

    ip = WiFi.localIP();
    Serial.printf(" --> Started WiFi AP \"%s\", IP address: ", ssid);
    Serial.println(ip);
  }
}

void setupMDNS()
{
  Serial.println("* Initializing mDNS responder...");
  if (MDNS.begin(MDNS_NAME_PREFIX))
  {
    Serial.println(" --> Advertising http://" MDNS_NAME_PREFIX ".local");
  }
  else
  {
    Serial.println(" --> mDNS responder failed");
  }
}

void setupSD()
{
#ifdef SD_SCK
  Serial.println("* Initializing SD card...");
  sd_card.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
  if (SD.begin(13, sd_card))
  {
    sdInitialized = true;
  }
  else
  {
    Serial.println(" --> SD card not found");
  }
#endif
}

void setupSPIFFS()
{
  Serial.println("* Initializing SPIFFS...");
  if (SPIFFS.begin())
  {
    spiffsInitialized = true;
  }
  else
  {
    Serial.println(" --> SPIFFS initializtion failed");
  }
}

bool checkFileIsReadable(FS &fs, const char *path)
{
  if (fs.exists(path))
  {
    return !!fs.open(path, "r");
  }
  return false;
}

void setupHTTPSever()
{
  Serial.println("* Initializing HTTP server...");

  if (sdInitialized && checkFileIsReadable(SD, "/" INDEX_FILE))
  {
    Serial.println(" --> Serving web application from SD card");
    http_server.addHandler(new AsyncStaticSDWebHandler("/", SD, "/", "max-age=604800"));
  }
  else if (spiffsInitialized && checkFileIsReadable(SPIFFS, "/" INDEX_FILE))
  {
    Serial.println(" --> Serving web application from SPIFFS");
    http_server.serveStatic("/", SPIFFS, "/").setDefaultFile(INDEX_FILE);
  }
  else
  {
    Serial.println(" --> No web application found! Did you add it to the SD card or SPIFFs?");
  }

  http_server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  http_server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  });

  http_server.begin();

  MDNS.addService("http", "tcp", 80);
}

void setupHistory()
{
  Serial.println("* Initializing history...");

  if (sdInitialized)
  {
    HistorySD *history_sd_client = new HistorySD(&sd_card);
    if (history_sd_client->init())
    {
      Serial.println(" --> Storing history on SD card");
      history = history_sd_client;
    }
  }
  if (!history)
  {
    Serial.println(" --> Storing (limited) history in memory");
    history = new HistoryMemory();
  }

  radio->connect(new HistoryRecord(history));
}

class WelcomeMessage : public DisasterMiddleware
{
public:
  void setup()
  {
    #ifdef USE_BLE
    if (client == NULL)
    {
      client = &ble_client;
      Serial.println("No client!!!!!!!!!!!!");
    }
    #endif
    client->receive(String("c|Welcome to DISASTER RADIO"));
    if (!sdInitialized)
    {
      client->receive(String("c|WARNING: SD card not found, functionality may be limited"));
    }
    if (!loraInitialized)
    {
      client->receive(String("c|WARNING: LoRa radio not found, functionality may be limited"));
    }
    client->setup();
  }
};

void setupSerial()
{
  Serial.println("* Initializing serial...");

  radio->connect(new WelcomeMessage())
      ->connect(new HistoryReplay(history))
      ->connect(new Console())
      ->connect(new StreamClient(&Serial));
}

void setupTelnet()
{
  Serial.println("* Initializing telnet server...");

  TCPClient::startServer(&tcp_server, [](TCPClient *tcp_client) {
    radio->connect(new WelcomeMessage())
        ->connect(new HistoryReplay(history))
        ->connect(new Console())
        ->connect(tcp_client);
  });
}

void setupWebSocket()
{
  Serial.println("* Initializing WebSocket server...");

  http_server.addHandler(&ws_server);

  WebSocketClient::startServer(&ws_server, [](WebSocketClient *ws_client) {
    radio->connect(new WelcomeMessage())
        ->connect(new HistoryReplay(history))
        ->connect(ws_client);
  });
}

void setupLoRa()
{
#ifdef LORA_CS
  Serial.println("* Initializing LoRa...");

  Layer1.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  Layer1.setLocalAddress(macaddr);
  if (Layer1.init())
  {
    LoRaClient *lora_client = new LoRaClient();
    if (lora_client->init())
    {
      Serial.printf(" --> LoRa address: %s\n", macaddr);
      radio->connect(lora_client);
      loraInitialized = true;
      return;
    }
  }
  Serial.println(" --> Failed to initialize LoRa");
#endif
}

#define STATUS_BAR_HEIGHT 12
#define MARGIN 3

void drawStatusBar()
{
  if (!displayInitialized)
  {
    return;
  }

  display.setFont(ArialMT_Plain_10);

  // clear the status bar
  display.setColor(BLACK);
  display.fillRect(0, 0, OLED_WIDTH, STATUS_BAR_HEIGHT);

  // draw MAC
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, macaddr);

  String clientsString = String(clients) + "," + routes;
  String ipString = ip.toString();

  // measure client and IP width
  int widthClients = (int)display.getStringWidth(clientsString) + 1;
  int widthIP = display.getStringWidth(ipString);

  // clear space behind IP address in case the SSID was too long
  display.setColor(BLACK);
  display.fillRect(OLED_WIDTH - widthClients - MARGIN - widthIP - MARGIN, 0, OLED_WIDTH, STATUS_BAR_HEIGHT);

  // draw client count
  display.setColor(WHITE);
  display.fillRect(OLED_WIDTH - widthClients, 2, widthClients, STATUS_BAR_HEIGHT - 3);
  display.setColor(BLACK);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(OLED_WIDTH, 0, clientsString);

  // draw IP address
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(OLED_WIDTH - widthClients - MARGIN, 0, ipString);

  // draw divider line
  display.drawLine(0, 12, 128, 12);
  display.display();
}

void setupDisplay()
{
#ifdef OLED_SDA
  Serial.println("* Initializing display...");
  if (OLED_RST != NOT_A_PIN)
  {
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW); // set GPIO16 low to reset OLED
    delay(50);
    digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high
  }
  if (display.init())
  {
    displayInitialized = true;

    display.flipScreenVertically();
    display.clear();

    radio->connect(new OLEDClient(&display, 0, STATUS_BAR_HEIGHT + 1));

    drawStatusBar();
  }
  else
  {
    Serial.println(" --> Failed to initialize the display");
  }

#endif
}

void setupGPS()
{
#ifdef GPS_SERIAL
  Serial.println("* Initializing GPS...");
  Serial1.begin(GPS_SERIAL);
  radio->connect(new GPSClient(&Serial1));
#endif
}

void setupBLE(void)
{
#ifdef USE_BLE
  Serial.println("* Initializing BLE...");

  uint64_t uniqueId = ESP.getEfuseMac();

  sprintf(macaddr, "%02x%02x%02x%02x%02x%02x",
          (uint8_t)(uniqueId), (uint8_t)(uniqueId >> 8), (uint8_t)(uniqueId >> 16),
          (uint8_t)(uniqueId >> 24), (uint8_t)(uniqueId >> 32), (uint8_t)(uniqueId >> 40));

  /// \todo Callback. Not working, as even BLE server is ready, there is no client yet
  ble_client.startServer([](BleUartClient *ble_client) {
  	radio->connect(new WelcomeMessage())
  		->connect(new HistoryReplay(history))
  		->connect(ble_client);
  });
#endif
}

void setup()
{
  Serial.begin(115200);
  delay(200);
  
  setCpuFrequencyMhz(CLOCK_SPEED); //Set CPU clock in config.h
  getCpuFrequencyMhz(); //Get CPU clock

#ifndef USE_BLE
  setupWiFi();
  setupMDNS();
#endif
  setupSD();
  setupSPIFFS();
#ifndef USE_BLE
  setupHTTPSever();
#endif

  setupHistory();
  setupSerial();
#ifndef USE_BLE
  setupTelnet();
  setupWebSocket();
#else
  setupBLE();
#endif
  setupLoRa();
  setupDisplay();
  setupGPS();

  internal_clients = radio->clients.size();

  radio->setup();
}

void loop()
{
  radio->loop();

  int new_clients = radio->clients.size() - internal_clients;
  int new_routes = LL2.getRouteEntry();
  if (clients != new_clients || routes != new_routes)
  {
    clients = new_clients;
    routes = new_routes;
    Serial.printf("--> clients=%d routes=%d\n", clients, routes);
    drawStatusBar();
  }
}
