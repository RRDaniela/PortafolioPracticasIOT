#define SERVO GPIO_NUM_14

#include "ESP32Servo.h"

Servo Servomotor;

int servoPosition = 0;
int rotationDirection = 1;

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando servomotor");
  Servomotor.setPeriodHertz(50);    // standard 50 hz servo
  Servomotor.attach(SERVO);  // attaches the servo on pin 14 to the servo object
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep

  Servomotor.write(servoPosition);
  delay(2000);
}

void loop() 
{
  if (rotationDirection > 0) {
    servoPosition += 15;
  } else {
    servoPosition -= 15;
  }
  Servomotor.write(servoPosition);
  if (servoPosition <= 0) {
    rotationDirection = 1;
  } else if (servoPosition >= 180) {
    rotationDirection = -1;
  }
  delay(1000);
}
