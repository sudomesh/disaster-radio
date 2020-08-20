#include "settings.h"

Preferences preferences;

String username = "";
bool useBLE = true;
int txPower = 17;

void getSettings(void)
{
  if (!preferences.begin("dr", false))
  {
    Serial.println("Error opening preferences");
    nvs_flash_init();
    return;
  }

  username = preferences.getString("un", "");

  if (username.isEmpty())
  {
    Serial.println("No name saved");
  }

  Serial.printf("Got username %s\n", username.c_str());

  // Try to get UI settings, defaults to WiFi!
  useBLE = preferences.getBool("ble", false);
  Serial.printf("Got IF setting %s\n", useBLE ? "BLE" : "WiFi");
  txPower = preferences.getInt("txPwr", false);
  Serial.printf("Got txPwr setting %d\n", txPower);


  preferences.end();
  return;
}

void saveUsername(String newUserName)
{
  if (!preferences.begin("dr", false))
  {
    Serial.println("Error opening preferences");
    return;
  }

  if (newUserName.isEmpty())
  {
    preferences.remove("un");
  }
  else
  {
    preferences.putString("un", newUserName);
    //Serial.printf("Saved username '%s'\n", preferences.getString("un", "ERROR SAVING").c_str());
  }
  preferences.end();
}

void saveUI(bool useBLE)
{
  if (!preferences.begin("dr", false))
  {
    Serial.println("Error opening preferences");
    return;
  }

  preferences.putBool("ble", useBLE);
  preferences.end();
}

void saveTxPower(int txPower)
{
  if (!preferences.begin("dr", false))
  {
    Serial.println("Error opening preferences");
    return;
  }

  preferences.putInt("txPwr", txPower);
  preferences.end();
}

/*
void saveLRFQ(int lrfq)
{
  if (!preferences.begin("dr", false))
  {
    Serial.println("Error opening preferences");
    return;
  }

  preferences.putInt("lrfq", lrfq);
  preferences.end();
}
*/
