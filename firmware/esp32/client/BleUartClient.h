#ifndef BLECLIENT_H
#define BLECLIENT_H

#include "../DisasterClient.h"
#include "../DisasterServer.h"

#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <BLE2902.h>
#include <esp_bt_device.h>

#include "server/DisasterRadio.h"
#include "middleware/HistoryReplay.h"

extern DisasterRadio *radio;
extern DisasterHistory *history;
extern bool sdInitialized;
extern bool spiffsInitialized;
extern bool displayInitialized;
extern bool loraInitialized;

class BleUartClient : public DisasterClient
{
  BleUartClient *client;

public:
  void receive(struct Datagram datagram, size_t len);

  void handleData(void *data, size_t len);
  void loop();
  static void init();
  static void startServer(void (*callback)(BleUartClient *));
};

#endif
