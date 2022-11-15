#include <BluetoothSerial.h>
#include <Preferences.h>

Preferences preferences;

bool hasFeederData()
{
  preferences.begin("FeederIot", false);
  feederId = preferences.getString("feederId", "");
  userId = preferences.getString("userId", "");
  refreshToken = preferences.getString("refreshToken", "");
  preferences.end();
  return feederId != "" && userId != "";
}

void maybeRequestUserDataFromBT()
{
  if (!hasFeederData()) {
    getDataFromApp();
    preferences.begin("FeederIot", false);
    preferences.putString("feederId",feederId);
    preferences.putString("userId",userId);
    preferences.end();
  } else {
    Serial.println("Recovered feederId " + feederId);
    Serial.println("Recovered userId " + userId);
  }
}

void getDataFromApp()
{
  BluetoothSerial SerialBT;
  uint64_t macAddress = ESP.getEfuseMac();                             // The chip ID is essentially its MAC address(length: 6 bytes).
  unsigned long highMacAddress = (unsigned long)((macAddress & 0xFFFFFF000000) >> 24);
  unsigned long lowMacAddress = (unsigned long)((macAddress & 0x000000FFFFFF));
  String strMacAddress = String(highMacAddress, HEX) + String(lowMacAddress, HEX);
  strMacAddress.toUpperCase();
  String feederName = String("FEEDER-") +
    strMacAddress.substring(10,12) +
    strMacAddress.substring(8,10) +
    strMacAddress.substring(6,8) +
    strMacAddress.substring(4,6) +
    strMacAddress.substring(2,4) +
    strMacAddress.substring(0,2);
  Serial.printf("Exposing ESP BT as %s\n", feederName.c_str());
  SerialBT.begin(feederName.c_str()); // Bluetooth device name
  while (SerialBT.hasClient() == false) {
    delay(350);
  }
  SerialBT.setTimeout(10000);
  // Send this feeders name
  String feederNameArg = "feederName:" + feederName + "\n";
  SerialBT.write((uint8_t*) feederNameArg.c_str(), feederNameArg.length());
  // Get the feeders firebase id
  feederId = SerialBT.readStringUntil('\n');
  Serial.println("feederId " + feederId);
  // Get the users uid
  userId = SerialBT.readStringUntil('\n');
  Serial.println("Closing BT connection...");
  SerialBT.end();
  SerialBT.disconnect();
}

void eraseLocalFeederData()
{
  preferences.begin("FeederIot", false);
  preferences.clear();
  preferences.end();
}

void setRefreshTokenPreference(String token)
{
  preferences.begin("FeederIot", false);
  preferences.putString("refreshToken",refreshToken);
  preferences.end();
}
