#include <NewPing.h>
#include <DHT.h>
#include "WiFi.h"
#include "time.h"
 
#define TRIGGER_PIN GPIO_NUM_12
#define ECHO_PIN GPIO_NUM_14 
#define MAX_DISTANCE 200

#define DHTPIN GPIO_NUM_13
#define DHTTYPE DHT11

#define GMT_MX -5
#define GMT_MX_OFF 0
#define HRS_IN_SECS 3600

const char* ssid = "TestWiFiNet";
const char* password = "1234567890";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = GMT_MX*HRS_IN_SECS;
const int   daylightOffset_sec = GMT_MX_OFF*HRS_IN_SECS;

NewPing Ultrasonic(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 
DHT DHT_HumSens(DHTPIN, DHTTYPE);

void setup() 
{
  Serial.begin(115200);
  Serial.println("Test de sensores:");
  DHT_HumSens.begin();

    // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
    //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.print("Conectado a internet, mi IP es: ");
  // Print the IP address
  Serial.println(WiFi.localIP());
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // printLocalTime();

  //disconnect WiFi as it's no longer needed
  // WiFi.disconnect(true);
  // WiFi.mode(WIFI_OFF);
}

void loop() 
{
  delay(5000); // lectura cada 5 segundos
  printLocalTime();
  readSensors();
}

void readSensors() 
{
  float t1 = DHT_HumSens.readTemperature();
  float h1 = DHT_HumSens.readHumidity();

  while (isnan(t1) || isnan(h1)) 
  {
    Serial.println("Lectura fallida en el sensor DHT_HumSens1, repitiendo lectura...");
    delay(3000); // esperar 3 seg
    t1 = DHT_HumSens.readTemperature();
    h1 = DHT_HumSens.readHumidity();
  }

  Serial.print("Distance: ");
  Serial.print(Ultrasonic.ping_cm());
  Serial.println(" cm.");

  float h = DHT_HumSens.readHumidity();
  float t = DHT_HumSens.readTemperature();
  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
  }

  float hic = DHT_HumSens.computeHeatIndex(t,h,false);
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%; Temperature: ");
  Serial.print(t);
  Serial.print("°C");
  Serial.print("; Heat index: ");
  Serial.print(hic);
  Serial.println("°C.");
  Serial.println();
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}