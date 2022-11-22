#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
#include <ezTime.h>
#include <addons/TokenHelper.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// WiFi data
#define WIFI_SSID "WIFI_SSDI"
#define WIFI_PASS "PASSWORD"

// Firebase data
#define FIREBASE_PROJECT_ID "FIREBASE_PROJ_ID"
#define FIREBASE_API_KEY "FIREBASE_API_KEY"
#define RTDB_URL "FIREBASE_RTDB_URL"

// Sensors GPIOs
//   Motor
#define IN1 GPIO_NUM_13
#define IN2 GPIO_NUM_14
#define IN3 GPIO_NUM_27
#define IN4 GPIO_NUM_26
//   Ultrasonic sensor
#define US_TRIGGER 23
#define US_ECHO 22
#define SOUND_SPEED 0.0343 // define sound speed in cm/uS
//   Infrared sensor
#define SENS_INFR 21
#define DETECTED_INFR LOW

FirebaseData firebaseRTDBDataAndConn;
FirebaseData firebaseDataAndConn;

FirebaseAuth auth;
FirebaseConfig config;

String feederId;
String userId;
String refreshToken;

time_t feedTimesInt[MAX_EVENTS];
int feedPortions[MAX_EVENTS];

String shouldFeedPath;
static SemaphoreHandle_t firebaseMutex;
static SemaphoreHandle_t feedTimesMutex;
static SemaphoreHandle_t feederMutex;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  firebaseMutex = xSemaphoreCreateMutex();
  feedTimesMutex = xSemaphoreCreateMutex();
  feederMutex = xSemaphoreCreateMutex();
  maybeRequestUserDataFromBT();
  connectToWifi();
  initSensors();
  waitForSync(); // timezone sync

  Serial.println(dateTime(RFC3339));
  Serial.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  startFirebaseConnection();

  if (!Firebase.RTDB.beginStream(&firebaseRTDBDataAndConn, "feeders/" + feederId))
  {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseRTDBDataAndConn.errorReason());
    Serial.println();
  }
  Firebase.RTDB.setStreamCallback(&firebaseRTDBDataAndConn, onFeederRTDBChange, streamTimeoutCallback, 10024);
}

void loop()
{
  checkSensors();
  checkEvents();
  delay(200);
}

void scheduleFeedTimes(String times)
{
  Serial.println("Scheduling " + times);
  xSemaphoreTake(feedTimesMutex, portMAX_DELAY);
  for (int i = 0; i < MAX_EVENTS; i++)
  {
    feedTimesInt[i] = 0;
    feedPortions[i] = 0;
    // handlers start at 1
    deleteEvent(i + 1);
  }
  Serial.println("Scheduling events now");
  xSemaphoreGive(feedTimesMutex);
  char feedTime[17];
  int currentFeedTime = 0;
  for (int i = 0, j = 0; i <= times.length(); i++)
  {
    if (i == times.length() || times.charAt(i) == ':')
    {
      feedTime[j] = '\0';
      j = 0;
      String feedTimeAndPortions = String(feedTime);
      if (feedTimeAndPortions.length() < 12 || feedTimeAndPortions.charAt(10) != ';')
      {
        Serial.println("Ignored badly formated feed schedule > " + feedTimeAndPortions);
        continue;
      }
      String feedTimeStr = feedTimeAndPortions.substring(0, 10);
      String feedPortionsStr = feedTimeAndPortions.substring(11);
      time_t time = atol(feedTimeStr.c_str());
      int portions = atoi(feedPortionsStr.c_str());
      time_t now1 = UTC.now();
      while (time < now1)
      {
        time = time + SECS_PER_DAY;
      }
      feedTimesInt[currentFeedTime] = time;
      feedPortions[currentFeedTime] = portions;
      currentFeedTime++;
    }
    else if (j < 15)
    {
      feedTime[j++] = times.charAt(i);
    }
    else
    {
      feedTime[16] = '\0';
    }
  }
  xSemaphoreTake(feedTimesMutex, portMAX_DELAY);
  for (int i = 0; i < MAX_EVENTS; i++)
  {
    if (feedTimesInt[i] != 0)
    {
      Serial.println("Scheduling event for: " + dateTime(feedTimesInt[i]) + " with " + feedPortions[i] + " portions");
      setEvent(feedAndRescheduleFeedInOneDay, feedTimesInt[i], UTC_TIME);
    }
  }
  xSemaphoreGive(feedTimesMutex);
}

void updateShouldFeed(String path, bool value)
{
  xSemaphoreTake(firebaseMutex, portMAX_DELAY);
  if (!Firebase.RTDB.setBool(&firebaseDataAndConn, path, value))
  {
    Serial.println("Updating shouldFeed failed...");
    Serial.println(firebaseDataAndConn.errorReason());
  }
  firebaseDataAndConn.getWiFiClient()->stop();
  xSemaphoreGive(firebaseMutex);
}

void updateResetFeederFlag(String path, bool value)
{
  xSemaphoreTake(firebaseMutex, portMAX_DELAY);
  if (!Firebase.RTDB.setBool(&firebaseDataAndConn, path, value))
  {
    Serial.println("Updating resetFeederFlag failed...");
    Serial.println(firebaseDataAndConn.errorReason());
  }
  firebaseDataAndConn.getWiFiClient()->stop();
  xSemaphoreGive(firebaseMutex);
}

void onFeederRTDBChange(FirebaseStream data)
{
  Serial.println("StreamPath: " + data.streamPath());
  Serial.println("DataPath: " + data.dataPath());
  // The entire object changed or a new property was added (check which one)
  if (data.dataPath() == "/" && data.dataType() == "json")
  {
    FirebaseJson json = data.jsonObject();
    FirebaseJsonData dataFromDB;
    if (json.get(dataFromDB, "feedTimes"))
    {
      String feedTimes = dataFromDB.stringValue;
      scheduleFeedTimes(feedTimes);
    }
    if (json.get(dataFromDB, "shouldFeed"))
    {
      bool shouldFeed = dataFromDB.boolValue;
      if (shouldFeed == true)
      {
        feedNPortions(1);
        updateShouldFeed("feeders/" + feederId + "/" + "shouldFeed", false);
      }
    }
    if (json.get(dataFromDB, "resetFeeder"))
    {
      bool shouldResetFeeder = dataFromDB.boolValue;
      if (shouldResetFeeder == true)
      {
        eraseLocalFeederData();
        updateResetFeederFlag("feeders/" + feederId + "/" + "resetFeeder", false);
        ESP.restart();
      }
    }
  }
  // The shouldFeed property was updated
  else if (data.dataPath() == "/shouldFeed" && data.dataType() == "boolean")
  {
    Serial.println(data.boolData());
    Serial.println("Shouldfeed changed to " + String(data.boolData() == true ? "True" : "False"));
    if (data.boolData() == true)
    {
      feedNPortions(1);
      updateShouldFeed(data.streamPath() + data.dataPath(), false);
    }
  }
  else if (data.dataPath() == "/feedTimes" && data.dataType() == "string")
  {
    String feedTimes = data.stringData();
    scheduleFeedTimes(feedTimes);
  }
  else if (data.dataPath() == "/resetFeeder" && data.dataType() == "boolean")
  {
    bool shouldReset = data.boolData();
    eraseLocalFeederData();
    updateResetFeederFlag(data.streamPath() + data.dataPath(), false);
    ESP.restart();
  }
}

void feedAndRescheduleFeedInOneDay()
{
  time_t newFeedTime = 0;
  int portionsToGive = 0;
  time_t timeNow = now();
  // See which event fired
  for (int i = 0; i < MAX_EVENTS; i++)
  {
    if (feedTimesInt[i] != 0 && feedTimesInt[i] <= timeNow)
    {
      feedTimesInt[i] = feedTimesInt[i] + SECS_PER_DAY;
      newFeedTime = feedTimesInt[i];
      portionsToGive = feedPortions[i];
      break;
    }
  }
  feedNPortions(portionsToGive);
  Serial.println("Next feed time: " + dateTime(newFeedTime) + " with " + portionsToGive + " portions");
  // The event then sets a new event for the next time
  setEvent(feedAndRescheduleFeedInOneDay, newFeedTime, UTC_TIME);
}

void checkEvents()
{
  xSemaphoreTake(feedTimesMutex, portMAX_DELAY);
  events();
  xSemaphoreGive(feedTimesMutex);
}