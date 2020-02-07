#ifdef USE_BLE
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

class BleDrClient : public DisasterClient
{
  BleDrClient *client;

public:
  String buffer = "";

  void receive(String message);

  void handleData(void *data, size_t len);
  void loop();
  void init();
  /// \todo Callback. Not working, as even BLE server is ready, there is no client yet
  void startServer(void (*callback)(BleDrClient *));
};

#endif
#endif // #ifdef USE_BLE
