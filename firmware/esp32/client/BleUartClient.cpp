#ifdef USE_BLE
#include "BleUartClient.h"

/** Comment out to stop debug output */
// #define DEBUG_OUT

/** Service UUID for Uart */
#define UART_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
/** Characteristic UUID for receiver */
#define RX_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
/** Characteristic UUID for transmitter */
#define TX_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/** Characteristic for BLE-UART TX */
BLECharacteristic *pCharacteristicUartTX;
/** Characteristic for BLE-UART RX */
BLECharacteristic *pCharacteristicUartRX;
/** Descriptor for the BLE-UART TX characteristic */
BLEDescriptor *txDescriptor;
/** BLE Advertiser */
BLEAdvertising *pAdvertising;
/** BLE Service for WiFi*/
BLEService *wifiService;
/** BLE Service for Uart*/
BLEService *uartService;
/** BLE Server */
BLEServer *pServer;

/** Flag if device is connected */
bool deviceConnected = false;

/** Flag if we already registered on the server */
bool connectedToServer = false;

/** Unique device name */
char apName[] = "DR-xxxxxxxxxxxx";

uint8_t txData[512] = {0};
uint8_t rxData[512] = {0};
size_t rxLen = 0;
bool dataRcvd = false;

extern BleUartClient ble_client;
void (*connectCallback)(BleUartClient *);

void BleUartClient::receive(struct Datagram datagram, size_t len)
{
  if (deviceConnected)
  {
#ifdef DEBUG_OUT
    Serial.printf("BLE:receive %s length %d\n", (char *)datagram.message, len - DATAGRAM_HEADER);
    Serial.println("BLE::receive raw data");
    for (int idx = 0; idx < len - DATAGRAM_HEADER; idx++)
    {
      Serial.printf("%02X ", datagram.message[idx]);
    }
    Serial.println("");
#endif

    unsigned char buf[len - DATAGRAM_HEADER] = {'\0'};
    memcpy(buf, &datagram.message, len-DATAGRAM_HEADER);

#ifdef DEBUG_OUT
    Serial.printf("BLE: Sending raw data with len %d\n", sizeof(buf));
    for (int idx = 0; idx < sizeof(buf); idx++)
    {
      Serial.printf("%02X ", buf[idx]);
    }
    Serial.println("");

    Serial.printf("BLE::receive %s len %d type %c\n", buf, sizeof(buf), datagram.type);
#endif

    pCharacteristicUartTX->setValue(buf, sizeof(buf));
    pCharacteristicUartTX->notify();

    // Give BLE time to get the data out
    delay(100);
  }
}

void BleUartClient::handleData(void *data, size_t len)
{
#ifdef DEBUG_OUT
  char debug[len] = {'\0'};
  memcpy(debug, data, len);
  Serial.println("BLE: Received raw data");
  for (int idx = 0; idx < len; idx++)
  {
  	Serial.printf("%02X ", debug[idx]);
  }
  Serial.println("");
#endif

  char *msg = (char *)data;
  struct Datagram datagram = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  datagram.type = msg[2];
  memcpy(&datagram.message, data, len);

#ifdef DEBUG_OUT
  Serial.printf("BLE::handleData %s len %d type %c\n", msg, len, datagram.type);
#endif

  server->transmit(this, datagram, len + DATAGRAM_HEADER);
}

/**
 * Callbacks for client connection and disconnection
 */
class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
#ifdef DEBUG_OUT
    Serial.println("BLE client connected");
#endif
    pServer->updatePeerMTU(pServer->getConnId(), 260);
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
#ifdef DEBUG_OUT
    Serial.println("BLE client disconnected");
#endif
    deviceConnected = false;
    pAdvertising->start();
  }
};

/**
 * Callbacks for BLE client read/write requests
 * on BLE UART characteristic
 */
class UartTxCbHandler : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();
    rxLen = rxValue.size();

    if ((rxLen > 0) && (dataRcvd == false))
    {
      strncpy((char *)rxData, rxValue.c_str(), 512);

#ifdef DEBUG_OUT
      Serial.printf("BLE:onWrite write callback received %s length %d\n", (char *)rxData, rxLen);
      Serial.println("BLE:onWrite raw data");
      for (int idx = 0; idx < rxLen; idx++)
      {
        Serial.printf("%02X ", rxData[idx]);
      }
      Serial.println("");
#endif
      dataRcvd = true;
    }
  };
};

class DescriptorCallbacks : public BLEDescriptorCallbacks
{
  void onWrite(BLEDescriptor *pDescriptor)
  {
    uint8_t *descrValue;
    descrValue = pDescriptor->getValue();
    if (descrValue[0] & (1 << 0))
    {
#ifdef DEBUG_OUT
      Serial.println("Notifications enabled");
#endif
      if (connectCallback)
      {
        if (!connectedToServer)
        {
          connectCallback(&ble_client);
          connectedToServer = true;
        }
        else
        {
        //   if (history)
        //   {
        //     history->replay(&ble_client);
        //   }
        }
      }
    }
  };
};

bool oldStatus = false;

void BleUartClient::loop(void)
{
  if (dataRcvd)
  {
    handleData(rxData, rxLen);
    dataRcvd = false;
  }
}

void BleUartClient::startServer(void (*callback)(BleUartClient *))
{
  connectCallback = callback;

  init();
}

void BleUartClient::init()
{
  uint64_t uniqueId = ESP.getEfuseMac();
  // Using ESP32 MAC (48 bytes only, so upper 2 bytes will be 0)
  sprintf(apName, "DR-%02X%02X%02X%02X%02X%02X",
          (uint8_t)(uniqueId), (uint8_t)(uniqueId >> 8),
          (uint8_t)(uniqueId >> 16), (uint8_t)(uniqueId >> 24),
          (uint8_t)(uniqueId >> 32), (uint8_t)(uniqueId >> 40));
#ifdef DEBUG_OUT
  Serial.printf("Device name: %s\n", apName);
#endif

  // Initialize BLE and set output power
  BLEDevice::init(apName);
  BLEDevice::setMTU(260);
  BLEDevice::setPower(ESP_PWR_LVL_P7);

  BLEAddress thisAddress = BLEDevice::getAddress();

#ifdef DEBUG_OUT
  Serial.printf("BLE address: %s\n", thisAddress.toString().c_str());
#endif

  // Create BLE Server
  pServer = BLEDevice::createServer();

  // Set server callbacks
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the UART BLE Service
  uartService = pServer->createService(UART_UUID);

  // Create a BLE Characteristic
  pCharacteristicUartTX = uartService->createCharacteristic(
      TX_UUID,
      BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_READ);

  pCharacteristicUartTX->addDescriptor(new BLE2902());

  // Register callback for notification enabled
  pCharacteristicUartTX->setNotifyProperty(true);

  pCharacteristicUartRX = uartService->createCharacteristic(
      RX_UUID,
      BLECharacteristic::PROPERTY_WRITE);

  pCharacteristicUartRX->setCallbacks(new UartTxCbHandler());

  txDescriptor = pCharacteristicUartTX->getDescriptorByUUID("2902");
  if (txDescriptor != NULL)
  {
    txDescriptor->setCallbacks(new DescriptorCallbacks());
  }

  // Start the service
  uartService->start();

  // Start advertising
  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(UART_UUID);
  pAdvertising->start();
}
#endif // #ifdef USE_BLE
