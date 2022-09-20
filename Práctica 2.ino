#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define echoPin 22 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 23
#define pin1 4
long duration; // variable for the duration of sound wave travel
int distance;
DHT dht1(pin1, DHT11);

void setup() {
 Serial.begin(115200);
 Serial.println("Test de sensores:");
 dht1.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
}
void loop() {
 delay(5000); // lectura cada 5 segundos
 leerdht1();
   // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}
void leerdht1() {
 float t1 = dht1.readTemperature();
 float h1 = dht1.readHumidity();
 while (isnan(t1) || isnan(h1)) {
 Serial.println("Lectura fallida en el sensor DHT11, repitiendo lectura...");
 delay(3000); // esperar 3 seg
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
}
