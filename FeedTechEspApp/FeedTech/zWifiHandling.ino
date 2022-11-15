#include <WiFi.h>

void connectToWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);  
  int loopCount = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print("Connecting to WiFi...");
    Serial.println(WiFi.status());
    loopCount++;
    if (loopCount == 3) {
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      loopCount = 0;
    }
  }
  Serial.println(WiFi.localIP());
}