
#include <FB_Const.h>
#include <FB_Error.h>
#include <FB_Network.h>
#include <FB_Utils.h>
#include <Firebase_ESP_Client.h>
#include <Firebase.h>
#include <FirebaseFS.h>
#include <MB_File.h>
#include <ezTime.h>

/****************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <HTTPClient.h>
#include <BluetoothSerial.h>
#include <Firebase_ESP_Client.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Sensor data
#define US_TRIGGER 23
#define US_ECHO 22
//define sound speed in cm/uS
#define SOUND_SPEED 0.0343
// Infrared sensor
#define SENS_INFR 21
#define DETECTED_INFR LOW

// Firebase data
#define API_KEY "API_KEY"
#define FIREBASE_PROJECT_ID "PROJ_ID"
#define USER_EMAIL "MAIL"
#define USER_PASSWORD "PASSWORD"

BluetoothSerial SerialBT;

const char *ssid = "SSID";
const char *password = "PASSWORD";

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 1;

void setup()
{
  Serial.begin(115200);
  while (!Serial);	
  SerialBT.begin("ESP32_CLASSIC_BT"); // Bluetooth device name
  SerialBT.end();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  pinMode(US_TRIGGER, OUTPUT); // Sets the US_TRIGGER as an Output
  pinMode(US_ECHO, INPUT); // Sets the US_ECHO as an Input
  pinMode(SENS_INFR, INPUT); // Sets infrarred sensor as an Input

  waitForSync(); // timezon sync
  
  Serial.println(WiFi.localIP());
  Serial.println(dateTime(RFC3339));

  // uint64_t chipid;

  // chipid = ESP.getEfuseMac();                                      // The chip ID is essentially its MAC address(length: 6 bytes).
  // Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32)); // print High 2 bytes
  // Serial.printf("%08X\n", (uint32_t)chipid);                       // print Low 4bytes.
  // Serial.printf("%X\n", chipid);

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  fbdo.setResponseSize(2048);
  Serial.println("Begin");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  // For sending payload callback
  config.cfs.upload_callback = fcsUploadCallback;
  /* Assign the callback function for the long running token generation task */
  // config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
}

void loop()
{
  // if (Serial.available())
  // {
  //   while (Serial.available())
  //   {
  //     SerialBT.write(Serial.read());
  //   }
  //   SerialBT.write('\r');
  // }
  // while (SerialBT.available())
  // {
  //   Serial.write(SerialBT.read());
  // }

  // Firebase.ready() should be called repeatedly to handle authentication tasks.
  if (Firebase.ready() && (millis() - dataMillis > 1800000 || dataMillis == 0))
  {
    dataMillis = millis();
    sendFoodPresenceDataToFirebase();
    delay(5000);
    sendAmountOfFoodLeftDataToFirebase();
  }
  delay(500);
}

// The Firestore payload upload callback function
void fcsUploadCallback(CFS_UploadStatusInfo info)
{
  if (info.status == fb_esp_cfs_upload_status_init)
  {
    Serial.printf("\nUploading data (%d)...\n", info.size);
  }
  else if (info.status == fb_esp_cfs_upload_status_upload)
  {
    Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
  }
  else if (info.status == fb_esp_cfs_upload_status_complete)
  {
    Serial.println("Upload completed ");
  }
  else if (info.status == fb_esp_cfs_upload_status_process_response)
  {
    Serial.print("Processing the response... ");
  }
  else if (info.status == fb_esp_cfs_upload_status_error)
  {
    Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
  }
}

void sendFoodPresenceDataToFirebase() {
  // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
  FirebaseJson content;
  String collectionId = "foodPresent";
  int hasFoodPresent = digitalRead(SENS_INFR);
  Serial.printf("Comida presente %s\n", hasFoodPresent != 0 ? "SI" : "NO");
  // boolean
  content.set("fields/hasFoodPresent/booleanValue", !!hasFoodPresent);
  // timestamp
  content.set("fields/timestamp/timestampValue", dateTime(RFC3339)); // RFC3339 UTC "Zulu" format
  Serial.print("Create a document... ");
  sendFirebaseRequest(collectionId, content);
}

void sendAmountOfFoodLeftDataToFirebase() {
  double distance = getDistanceInCm();
  if (distance > 20) {
    distance = 20;
  }
  // Cylinder is about 20cm
  double percentageLeft = 100 - (distance * 100 / 20);
  // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
  FirebaseJson content;
  String collectionId = "foodLeft";
  // double
  content.set("fields/percentageOfFoodLeft/doubleValue", percentageLeft);
  // timestamp
  content.set("fields/timestamp/timestampValue", dateTime(RFC3339)); // RFC3339 UTC "Zulu" format
  Serial.print("Create a document... ");
  sendFirebaseRequest(collectionId, content);
}

void sendFirebaseRequest(String collectionId, FirebaseJson content) {
  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, collectionId.c_str(), content.raw()))
  {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  }
  else
    Serial.println(fbdo.errorReason());
}

double getDistanceInCm() {
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
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  return distanceCm;
}
