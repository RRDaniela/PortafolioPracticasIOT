// Max pair sequence
int paso [4][4] =
{
  {1, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 1},
  {1, 0, 0, 1}
};

void initSensors()
{
  pinMode(US_TRIGGER, OUTPUT); // Sets the US_TRIGGER as an Output
  pinMode(US_ECHO, INPUT); // Sets the US_ECHO as an Input
  pinMode(SENS_INFR, INPUT); // Sets infrarred sensor as an Input

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

double getPercentageOfFoodLeft()
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
  double distanceCm = duration * SOUND_SPEED/2;

  if (distanceCm > 20) {
    distanceCm = 20;
  }
  // Cylinder is about 20cm
  double percentageLeft = 100 - (distanceCm * 100 / 20);
  return percentageLeft;
}

int hasFoodPresentSensor()
{
  return digitalRead(SENS_INFR);
}

void feedNPortions(int n)
{
  if (n > 0)
  {    
    sendNumberOfFeeds(n);
  }
  for (int i = 0; i < n; i++)
  {
    xTaskCreatePinnedToCore(feedOnePortion, String("Feednow" + String(now())).c_str(), 2048, NULL, 5, NULL, 0);
  }
}

void feedOnePortion(void* params)
{
  // A mutex is needed because we do not want more than one thread manipulating the motor at the same time
  xSemaphoreTake(feederMutex, portMAX_DELAY);
  Serial.println("Feeding one portion...");
  for (int i = 0; i < 2050; i++)
  {
    digitalWrite(IN1, paso[i%4][0]);
    digitalWrite(IN2, paso[i%4][1]);
    digitalWrite(IN3, paso[i%4][2]);
    digitalWrite(IN4, paso[i%4][3]);
    vTaskDelay(4);
  }
  Serial.println("Feeding done! \n");
  xSemaphoreGive(feederMutex);
  vTaskDelete(NULL);
}