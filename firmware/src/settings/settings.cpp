#include "settings.h"

Preferences preferences;

String username = "";
bool useBLE = true;
int txPower = 17;
int loraFrq = 915;
int spreadingFactor = 9;

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

  // Try to get settings, if key does not exist, use default values
  useBLE = preferences.getBool("ble", false);
  Serial.printf("Got IF setting %s\n", useBLE ? "BLE" : "WiFi");
  txPower = preferences.getInt("txPwr", txPower);
  Serial.printf("Got txPwr setting %d\n", txPower);
  loraFrq = preferences.getInt("loraFrq", loraFrq);
  Serial.printf("Got loraFrq setting %d\n", loraFrq);
  spreadingFactor = preferences.getInt("sf", spreadingFactor);
  Serial.printf("Got sf setting %d\n", spreadingFactor);

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

void saveLoraFrq(int loraFrq)
{
  if (!preferences.begin("dr", false))
  {
    Serial.println("Error opening preferences");
    return;
  }

  preferences.putInt("loraFrq", loraFrq);
  preferences.end();
}

void saveSpreadingFactor(int spreadFactor)
{
  if (!preferences.begin("dr", false))
  {
    Serial.println("Error opening preferences");
    return;
  }

  preferences.putInt("sf", spreadFactor);
  preferences.end();
}
