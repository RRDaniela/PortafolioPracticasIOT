
#include <DHT.h>
#include <DHT_U.h>

#define pin1 15

DHT dht1(pin1, DHT11);

#define PIN_LED 02
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char DEVICE_LOGIN_NAME[]  = "23accfdc-a0e0-4963-86bc-f5a7ef8c0dc8";

const char SSID[]               = "";    // Network SSID (name)
const char PASS[]               = "";    // Network password (use for WPA, or use as key for WEP)
const char DEVICE_KEY[]  = "";    // Secret device password

void onLedLightChange();

CloudLight ledLight;
CloudTemperatureSensor temperaturaCuarto;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(ledLight, READWRITE, ON_CHANGE, onLedLightChange);
  ArduinoCloud.addProperty(temperaturaCuarto, READ, ON_CHANGE, NULL);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);



void setup() {
 // Initialize serial and wait for port to open:
 Serial.begin(9600);
 // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
 delay(1500);
 // Defined in thingProperties.h
 initProperties();
 // Connect to Arduino IoT Cloud
 ArduinoCloud.begin(ArduinoIoTPreferredConnection);
/*
 The following function allows you to obtain more information
 related to the state of network and IoT Cloud connection and errors
 the higher number the more granular information you’ll get.
 The default is 0 (only errors).
 Maximum is 4
 */
 setDebugMessageLevel(2);
 ArduinoCloud.printDebugInfo();

 pinMode(PIN_LED,OUTPUT); //DEFINIR LED COMO OUTPUT
 dht1.begin(); //INICIAR DHT
}
void loop() {
 ArduinoCloud.update();
 // Your code here

 temperaturaCuarto = dht1.readTemperature(); //ASIGNAR LA LECTURA DE LA TEMPERATURA A LA VARIABLE "temp" GENERADA EN THING "SMARTHOME"
 Serial.println("Esta es la temperatura");
 Serial.println(temperaturaCuarto); //IMPRIMIR EN EL MONITOR SERIAL LA TEMPERATURA

 delay(1);
}
/*
 Since LedLight is READ_WRITE variable, onLedLightChange() is
 executed every time a new value is received from IoT Cloud.
*/
void onLedLightChange() {
 // Add your code here to act upon LedLight change
 digitalWrite(PIN_LED, ledLight); //ASIGNAR EL ESTADO DEL LED A LA VARIABLE "ledLight"
}
