// Faya-Nugget exercise draft  (MQTT_ThingSpeak_DHT11.ino)

// Description: Every 15 seconds, faya brickNano will get two data from faya Humidity and Temperature Sensor 
//              and then send to ThingSpeak channel through faya Wi-Fi by following MQTT API protocol.

// Wiring: Arduino Pin ==> faya Module Pin
//                  D4 ==>  TxD   (Wi-Fi)
//                  D5 ==>  RxD
//-----------------------------------------------------------------
//                  D9 ==>  Data  (Humidity & Temperature Sensor)

// --- include WiFiEsp library ---
#include "WiFiEsp.h"

#include<SoftwareSerial.h>
SoftwareSerial fayaWiFi(4, 5);     // create instance and set the
                                   // simulated RxD and TxD pins
#define ssid "Ahmed"      // define Wi-Fi station name
#define password "ahmed#123"   // define Wi-Fi station password

WiFiEspClient espClient;           // Initialize the Ethernet client object
int status = WL_IDLE_STATUS;       // the Wi-Fi radio's status

// --- include MQTT library ----
#include <PubSubClient.h>
PubSubClient client(espClient);     // create instance with espClient
 //21d8ec27fa@emailvb.pro                                   // network client = espClient
//21d8ec27faA
//**********************************************************************
// ThingSpeak MQTT Information:
//**********************************************************************
const char* mqtt_server = "mqtt.thingspeak.com";    // MQTT Server address
int mqtt_port = 1883;                         // MQTT Port

const char* Channel_ID = "2778442";              // ThingSpeak Channel ID
const char* Username = "mwa0000036213139";                // username, can be any string
const char* MQTT_apiKey = "G918EMCSD85HMOXW";          // MQTT API Key
const char* Write_apiKey = "AJ1H9RP05DMCQYYD";         // ThingSpeak Write API Key

// cascade publish Topic
String Topic = "channels/" + String(Channel_ID) + "/publish/" + String(Write_apiKey);

// concert string to a pointer to the C-style version of the involving string
const char* publishRawTopic = Topic.c_str();

// define message
const char* publishRawPayload = "field1=0&field2=0&field3=0&field4=0&field5=0&field6=0&field7=0&field8=0";

unsigned long lastConnectionTime = 0;              // last connection time
// resend data every 15 seconds
const unsigned long postingInterval = 15L * 1000L; // 15000ms = 15 second
//**********************************************************************

// --- include library and define connection pin for DHT11 ---
#include <dht.h>        // include dht11 library
#define dataPin 9       // sensor data port connects to Arduino D9 pin

// --- create instance of dht class---
dht faya_dht11;         // instance name = faya_dht11

void setup() {
  Serial.begin(9600);   // initialize baud rate = 9600bps
  setup_wifi();         // run setup_wifi() subroutine

  // set server details (broker IP address and port)
  client.setServer(mqtt_server, mqtt_port);
}

// Initialize and connect to Wi-Fi
void setup_wifi() {
  fayaWiFi.begin(9600);  // software baud rate = 9600 bps
  WiFi.init(&fayaWiFi);  // initialize ESP module

  // check for the presence of the Wi-Fi shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue
    while (true);
  }

  // attempt to connect to Wi-Fi network
  int retryCount = 0;
  while (status != WL_CONNECTED && retryCount < 10) {
    Serial.print(F("Attempting to connect to WPA : "));
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
    retryCount++;
    delay(5000); // wait 5 seconds before retrying
  }

  if (status == WL_CONNECTED) {
    // you're connected now, so print out connected IP address
    Serial.print(F("This ESP8266 IP address is: "));
    Serial.println(WiFi.localIP());
    Serial.println("");
  } else {
    Serial.println(F("Failed to connect to Wi-Fi"));
    // don't continue
    while (true);
  }
}

// --- reconnect subroutine:  ---
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    Serial.println("");
    // connects the client to broker
    if (client.connect(Channel_ID, Username, MQTT_apiKey)) {
      Serial.println(F("connected"));
    }
    else {
      // if connection fails, print out following message
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      delay(5000);    // wait 5 second and try connect again
    }
  }
}

char msg[10];         // char array to store publishing message

void loop() {
  if (client.loop()) {                       // if the client is still connected
    int chk = faya_dht11.read11(dataPin);    // read sensor data
    int read_temp = faya_dht11.temperature;  // get temperature value
    int read_humd = faya_dht11.humidity;     // get humidity value

    //-----------------  Publish to ThingSpeak MQTT  -----------------

    // publish to MQTT broker every 15 seconds
    if (millis() - lastConnectionTime > postingInterval) {
      
      // format string before print out
      sprintf(publishRawPayload, "field1=%d&field2=%d", read_temp, read_humd);
      
      // publish to MQTT
      client.publish(publishRawTopic, publishRawPayload);

      Serial.print("Publish Raw Payload:");  // print out string
      Serial.println(publishRawPayload);     // print published message
      lastConnectionTime = millis();         // update last connection time
    }
  }

  // if disconnected
  else {
    if (status != WL_CONNECTED) {            // if wi-fi disconnected
      setup_wifi();                          // reconnect Wi-Fi
      delay(1000);                           // delay 1 second before next try
    }
    else {                                   // or MQTT disconnected
      reconnect();                           // reconnect MQTT
    }
    delay(1000);                             // delay 1 second before next try
  }
}