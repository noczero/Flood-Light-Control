#include <Arduino.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <ezTime.h>
#include <SchedTask.h>

// define PIN
#define RELAY_PIN_1 D1
#define RELAY_PIN_2 D5
#define RELAY_PIN_3 D6
#define RELAY_PIN_4 D7

// configure firebase
#define FIREBASE_HOST "https://flood-light-control-default-rtdb.asia-southeast1.firebasedatabase.app/" // url
#define FIREBASE_AUTH "Ghj4itXa5eD91jxif2jnVHah05M3mnjiD5j5jWJn"                                       // settings/service account/database secret
String NODE_PATH = "/device1";
String LIGHT_1_PATH = NODE_PATH + "/light_1";
String LIGHT_2_PATH = NODE_PATH + "/light_2";
String LIGHT_3_PATH = NODE_PATH + "/light_3";
String LIGHT_4_PATH = NODE_PATH + "/light_4";

// FirebaseESP8266 instance
FirebaseData firebaseData;
FirebaseJson json;

// wifi configuration
#define ssid "JAKARTA 77"
#define password "sy130077"

// datetime for automation
Timezone IDTime;
String dateTimeNow = "";

// Scheduler
void sendDataToFirebase();
void getDataFromFirebase();
SchedTask sendDataToFirebaseTask(0, 1500, sendDataToFirebase);   // create task for send data to firebase every 1s
SchedTask getDataFromFirebaseTask(0, 1000, getDataFromFirebase); // create task for receiving data from firebase every 1s

// Variable Light declaration as array
String lightCommandsArray[4];
String lightScheduledTimeStartArray[4];
String lightScheduledTimeEndArray[4];
String lightStatusArray[4];
String lightFirebasePathArray[4] = {LIGHT_1_PATH, LIGHT_2_PATH, LIGHT_3_PATH, LIGHT_4_PATH};
byte lightRelayPINArray[4] = {RELAY_PIN_1, RELAY_PIN_2, RELAY_PIN_3, RELAY_PIN_4};

// setup relay
void setupRelay()
{
  // set relay pin to OUPUT
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);

  // write to HIGH for OFF
  digitalWrite(RELAY_PIN_1, HIGH);
  digitalWrite(RELAY_PIN_2, HIGH);
  digitalWrite(RELAY_PIN_3, HIGH);
  digitalWrite(RELAY_PIN_4, HIGH);
}

// demo relay ON OFF on interval
void demoRelay()
{
  // write to HIGH for OFF
  digitalWrite(RELAY_PIN_1, HIGH);
  digitalWrite(RELAY_PIN_2, HIGH);
  digitalWrite(RELAY_PIN_3, HIGH);
  digitalWrite(RELAY_PIN_4, HIGH);
  delay(1000);
  // write to LOW for ON
  digitalWrite(RELAY_PIN_1, LOW);
  digitalWrite(RELAY_PIN_2, LOW);
  digitalWrite(RELAY_PIN_3, LOW);
  digitalWrite(RELAY_PIN_4, LOW);
}

// read relay status and convert to OFF or ON
String readRelayStatus(int relayPin)
{
  byte status = digitalRead(relayPin);
  if (status == LOW)
  {
    return "ON";
  }
  else
  {
    return "OFF";
  }
}

// setup firebase
void setupFirebase()
{
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

// setup for wifi
void setupWiFI()
{
  // Let us connect to WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println(".......");
  Serial.println("WiFi Connected....IP Address:");
  Serial.println(WiFi.localIP());
}

// setup time
void setupTime()
{
  Serial.println("Setup NTP Time...");

  events(); // date
  waitForSync();
  IDTime.setLocation("Asia/Jakarta");
  dateTimeNow = IDTime.dateTime();
  Serial.println(dateTimeNow);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("-- Flood Light Control v1 --");

  setupRelay();    // start relay setup
  setupWiFI();     // start WiFi setup
  setupFirebase(); // start Firebase setup
  setupTime(); // start setup time
}

void loop()
{
  // put your main code here, to run repeatedly:
  // demoRelay(); // relay demo
  events(); // eztime event loop
  SchedBase::dispatcher(); // dispatch any tasks due
  delay(100);
}

// send data to firebase
void sendDataToFirebase()
{
  for (int i = 0; i < 4; i++)
  {
    Serial.println("\nLight-" + String(i+1));
    // update light 1 status
    json.clear();
    String relayStatus = readRelayStatus(lightRelayPINArray[i]);
    json.add("status", relayStatus); // set status value
    boolean send_resp = Firebase.updateNode(firebaseData, lightFirebasePathArray[i], json);

    // check response
    if (send_resp)
    {
      Serial.println("Success, update status to " + relayStatus + " on Light-" + String(i+1));
    }
  }
}

// get data from firebase
void getDataFromFirebase()
{
  
  for (int i = 0; i < 4; i++)
  {
    Serial.println("\nLight-" + String(i+1));
    // read command  
    if (Firebase.getString(firebaseData, lightFirebasePathArray[i] + "/command"))
    {
      if (firebaseData.dataType() == "string")
      {
        lightCommandsArray[i] = firebaseData.stringData();
        Serial.println("Receive light command : " + lightCommandsArray[i]);
      }
    }


    // read scheduled time start
    if (Firebase.getString(firebaseData, lightFirebasePathArray[i] + "/scheduled_time/start"))
    {
      if (firebaseData.dataType() == "string")
      {
        lightScheduledTimeStartArray[i] = firebaseData.stringData();
        Serial.println("Receive scheduled time start : " + lightScheduledTimeStartArray[i]);
      }
    }

    // read scheduled time end
    if (Firebase.getString(firebaseData, lightFirebasePathArray[i] + "/scheduled_time/end"))
    {
      if (firebaseData.dataType() == "string")
      {
        lightScheduledTimeEndArray[i] = firebaseData.stringData();
        Serial.println("Receive scheduled time end : " + lightScheduledTimeEndArray[i]);
      }
    }
      /* code */
  }

}