time_t lastFoodPresenceDocTs = 0;
time_t lastPercentageDocTs = 0;

void checkSensors()
{
  if (Firebase.ready() && (now() - lastFoodPresenceDocTs) > SECS_PER_MIN * 10)
  {
    sendFoodPresenceDataToFirebase();
  }
  if (Firebase.ready() && ((now() - lastPercentageDocTs) > SECS_PER_MIN) * 1.5)
  {
    sendAmountOfFoodLeftDataToFirebase();
  }
}

void sendFoodPresenceDataToFirebase()
{
  // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
  FirebaseJson content;
  String collectionId = "foodPresent";
  int hasFoodPresent = hasFoodPresentSensor();
  Serial.printf("Comida presente %s\n", hasFoodPresent != 0 ? "SI" : "NO");
  // boolean
  content.set("fields/hasFoodPresent/booleanValue", !hasFoodPresent);
  // timestamp
  content.set("fields/timestamp/timestampValue", dateTime(RFC3339)); // RFC3339 UTC "Zulu" format
  sendFirebaseRequest(collectionId, content);
  lastFoodPresenceDocTs = now();
}

void sendAmountOfFoodLeftDataToFirebase()
{
  double percentageLeft = getPercentageOfFoodLeft();  
  // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
  FirebaseJson content;
  String collectionId = "foodLeft";
  // double
  content.set("fields/percentageOfFoodLeft/doubleValue", percentageLeft);
  
  String doc_path = "projects/";
  doc_path += FIREBASE_PROJECT_ID; // Firebase project id (found on settings)
  doc_path += "/databases/(default)/documents/feeders/"; // coll_id and doc_id are your collection id and document id
  doc_path += feederId; // coll_id and doc_id are your collection id and document id

  // reference
  content.set("fields/feeder/referenceValue", doc_path.c_str());

  // timestamp
  content.set("fields/timestamp/timestampValue", dateTime(RFC3339)); // RFC3339 UTC "Zulu" format
  sendFirebaseRequest(collectionId, content);
  lastPercentageDocTs = now();
}

void sendNumberOfFeeds(int numberOfFeeds)
{
  // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
  FirebaseJson content;
  String collectionId = "foodGiven";
  // double
  content.set("fields/numberOfPortions/integerValue", numberOfFeeds);
  
  String doc_path = "projects/";
  doc_path += FIREBASE_PROJECT_ID; // Firebase project id (found on settings)
  doc_path += "/databases/(default)/documents/feeders/"; // coll_id and doc_id are your collection id and document id
  doc_path += feederId; // coll_id and doc_id are your collection id and document id

  // reference
  content.set("fields/feeder/referenceValue", doc_path.c_str());

  // timestamp
  content.set("fields/timestamp/timestampValue", dateTime(RFC3339)); // RFC3339 UTC "Zulu" format
  sendFirebaseRequest(collectionId, content);
}

void sendFirebaseRequest(String collectionId, FirebaseJson content)
{
  xSemaphoreTake(firebaseMutex, portMAX_DELAY);
  if (Firebase.Firestore.createDocument(&firebaseDataAndConn, FIREBASE_PROJECT_ID, "", collectionId.c_str(), content.raw()))
  {
    Serial.println("Document in collection " + collectionId + " created");
  }
  else
    Serial.println(firebaseDataAndConn.errorReason());
  firebaseDataAndConn.getWiFiClient()->stop();
  xSemaphoreGive(firebaseMutex);
}

void startFirebaseConnection()
{
  /* Assign the api key (required) */
  config.api_key = FIREBASE_API_KEY; // Firebase API_KEY Here
  config.database_url = RTDB_URL; // Firebase RTDB link here
  Firebase.reconnectWiFi(true);
  if (refreshToken == "") {
    Serial.println("Singin up");
    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", ""))
    {
      refreshToken = Firebase.getRefreshToken();
      setRefreshTokenPreference(refreshToken);
    }
    else 
    {
      Serial.printf("Error singin up: %s\n", config.signer.signupError.message.c_str());
    }
  }
  if (refreshToken.length() > 0)
  {
    Serial.println("Refresh token is set!");
  }
  else
  {
    Serial.println("Refresh token is missing!");
    throw "No token";
  }
  
  Firebase.setIdToken(&config, "", 3500 , refreshToken);
  
  Firebase.begin(&config, &auth);
  // For sending payload callback
  config.cfs.upload_callback = fcsUploadCallback;
  // /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h  
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

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
  {
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
}