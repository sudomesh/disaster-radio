#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>

void getSettings(void);
void saveUsername(String newUserName);
void saveUI(bool useBLE);

extern String username;
extern bool useBLE;