#include "DHT.h"
#include "ThingSpeak.h"
#include "WiFi.h"
#define pin1 13

// Conexión a Internet
const char *ssid = "SSID";         // Nombre de su internet
const char *password = "PASSWORD"; // Contraseña de su internet

// Información para poder subir datos a ThingSpeak
unsigned long channelID = 111111;      // ID del canal de UDS
const char *WriteAPIKey = "FFFFFFFFF"; // Write API Key del canal  que creaste
WiFiClient cliente;                    // Objeto Cliente del WiFi

DHT dht1(pin1, DHT11); // El azul

void setup()
{
  Serial.begin(115200);
  Serial.println("Test de sensores:");

  // Conexión a internet
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi conectado!");

  ThingSpeak.begin(cliente); // Inicializar el servidor de ThingSpeak

  dht1.begin();
}

void loop()
{

  delay(2000);
  leerdht1();

  // Mandar los datos a ThingSpeak
  ThingSpeak.writeFields(channelID, WriteAPIKey);
  Serial.println("Datos enviados a ThingSpeak!");
  delay(5000);
}

void leerdht1()
{

  float t1 = dht1.readTemperature();
  float h1 = dht1.readHumidity();

  while (isnan(t1) || isnan(h1))
  {
    Serial.println("Lectura fallida en el sensor DHT11, repitiendo lectura...");
    delay(2000);
    t1 = dht1.readTemperature();
    h1 = dht1.readHumidity();
  }

  Serial.print("Temperatura DHT11: ");
  Serial.print(t1);
  Serial.println(" ºC.");

  Serial.print("Humedad DHT11: ");
  Serial.print(h1);
  Serial.println(" %.");

  Serial.println("-----------------------");

  // Se ponen los campos definidos en el canal ThingSpeak con las variables de temperatura y humedad
  ThingSpeak.setField(1, t1); // En el campo 1 se le asigna la temperatura (t1)
  ThingSpeak.setField(2, h1); // En el campo 2 se le asigna la humedad (h1)
}