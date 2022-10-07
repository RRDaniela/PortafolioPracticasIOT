#include "WiFi.h"
#include "time.h"

#define GMT_MX -5
#define GMT_MX_OFF 0
#define HRS_IN_SECS 3600

const char* ssid = "TestWiFiNet";
const char* password = "1234567890";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = GMT_MX*HRS_IN_SECS;
const int   daylightOffset_sec = GMT_MX_OFF*HRS_IN_SECS;

void setup(){
  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Serial.println("");
  // Serial.println("WiFi connected.");
    //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.print("Conectado a internet, mi IP es: ");
  // Print the IP address
  Serial.println(WiFi.localIP());
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop(){
  // delay(10000);
  // printLocalTime();
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  // Serial.print("Day of week: ");
  // Serial.println(&timeinfo, "%A");
  // Serial.print("Month: ");
  // Serial.println(&timeinfo, "%B");
  // Serial.print("Day of Month: ");
  // Serial.println(&timeinfo, "%d");
  // Serial.print("Year: ");
  // Serial.println(&timeinfo, "%Y");
  // Serial.print("Hour: ");
  // Serial.println(&timeinfo, "%H");
  // Serial.print("Hour (12 hour format): ");
  // Serial.println(&timeinfo, "%I");
  // Serial.print("Minute: ");
  // Serial.println(&timeinfo, "%M");
  // Serial.print("Second: ");
  // Serial.println(&timeinfo, "%S");

  // Serial.println("Time variables");
  // char timeHour[3];
  // strftime(timeHour,3, "%H", &timeinfo);
  // Serial.println(timeHour);
  // char timeWeekDay[10];
  // strftime(timeWeekDay,10, "%A", &timeinfo);
  // Serial.println(timeWeekDay);
  // Serial.println();
}