#define SENS_INFR GPIO_NUM_13
#define DETECTED_INFR LOW
#define SERVO GPIO_NUM_14

#include "ESP32Servo.h"

Servo Servomotor;

int isThereFood = 0;
int servoPosition = 0;

void setup() {
  Serial.begin(115200);

  /*Initialize Infrarred*/
  pinMode(SENS_INFR, INPUT);
  Serial.println("Sensor Infrarrojo Inicializado");
  Serial.println("------------------------------");

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
  Servomotor.setPeriodHertz(50);    // standard 50 hz servo
  Servomotor.attach(SERVO, 500, 2400);  // attaches the servo on pin 18 to the servo object
	// using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep

  Servomotor.write(servoPosition);
  delay(2000);
}

void loop() 
{
  isThereFood = digitalRead(SENS_INFR);
  if(DETECTED_INFR == isThereFood)
  {
    servoPosition += 20;
    Serial.println("Comida detectada");
    Servomotor.write(servoPosition);
  }
  delay(1000);
}
