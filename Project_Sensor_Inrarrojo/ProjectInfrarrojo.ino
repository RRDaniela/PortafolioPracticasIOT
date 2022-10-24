#define SENS_INFR GPIO_NUM_13
#define DETECTED_INFR LOW

int isFoodPresent = 0;

void setup() {
  Serial.begin(115200);
  // Initialize Infrarred Sensor
  pinMode(SENS_INFR, INPUT);
  Serial.println("Sensor Infrarrojo Inicializado");
  Serial.println("------------------------------");
}

void loop() 
{
  isThereFood = digitalRead(SENS_INFR);
  if(DETECTED_INFR == isThereFood)
  {
    Serial.println("Comida detectada");
  }
  delay(1000);
}
