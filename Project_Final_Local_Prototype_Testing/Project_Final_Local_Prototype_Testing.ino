/*******************************************/
//     Local Prototype tests program       //
/*******************************************/

// Motor pins
#define IN1  GPIO_NUM_13
#define IN2  GPIO_NUM_14
#define IN3  GPIO_NUM_27
#define IN4  GPIO_NUM_26
// Infrared pins
#define SENS_INFR GPIO_NUM_32
#define DETECTED_INFR LOW
// Ultrasonic pins
#define US_TRIGGER GPIO_NUM_25
#define US_ECHO GPIO_NUM_33
// define sound speed in cm/uS
#define SOUND_SPEED 0.0343
// Container height
#define CONT_HEIGHT_CM  20
 
// Secuencia de pasos de motor (par máximo)
int motor_steps [4][4] =
{
  {1, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 1},
  {1, 0, 0, 1}
};

/* Functions defs*/
void moveMotor(int laps);
double getDistanceInCm(void);

void setup()
{
  /* Begin Serial Port */
  Serial.begin(115200);
  /* Begin Motor pins */
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  /* Begin Infrared pins*/
  pinMode(SENS_INFR, INPUT);
  /* Begin Ultrasonic pins */
  pinMode(US_TRIGGER, OUTPUT);
  pinMode(US_ECHO, INPUT);

  delay(3000);  // ! Begin delay just for tests, not necessary
}

void loop()
{ 
  /* Read Food amount */
  double distance = getDistanceInCm();
  if (distance > CONT_HEIGHT_CM)
  {
    distance = CONT_HEIGHT_CM;
  }
  double percentageLeft = 100 - (distance * 100 / CONT_HEIGHT_CM);
  Serial.print("Percentage left: ");
  Serial.print(floor(percentageLeft));
  Serial.println("%");

  /* Read if there's food to eat on the plate */
  int hasFoodPresent = digitalRead(SENS_INFR);
  Serial.printf("There %s food on the plate\n", hasFoodPresent == DETECTED_INFR ? "is" : "isn't");
  Serial.println();

  /* Move motor to decrease food amount on container */
  if(hasFoodPresent != DETECTED_INFR)
  {
    Serial.println("Giving food...");
    Serial.println();
    moveMotor(1);
  }

  delay(1000);
}

void moveMotor(int laps)
{
  // Moves motor one or more laps, defined by laps parameter (2050 is for one complete 360° lap)
  for (int i = 0; i < 2050*laps; i++)
  {
    digitalWrite(IN1, motor_steps[i%4][0]);
    digitalWrite(IN2, motor_steps[i%4][1]);
    digitalWrite(IN3, motor_steps[i%4][2]);
    digitalWrite(IN4, motor_steps[i%4][3]);
    delay(4);
  }
}

double getDistanceInCm()
{
  // Clears the trigPin
  digitalWrite(US_TRIGGER, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(US_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIGGER, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(US_ECHO, HIGH);

  // Calculate the distance
  double distanceCm = duration * SOUND_SPEED / 2;

  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  return distanceCm;
}