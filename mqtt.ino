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

// --- include library and define connection pin for DHT11 ---
#include <dht.h>        // include dht11 library
#define dataPin 9       // sensor data port connects to Arduino D9 pin

// --- create instance of dht class---
dht faya_dht11;         // instance name = faya_dht11

const char* writeApiKey = "SZ4TPTW9GJUP9MA7"; // ThingSpeak Write API Key
const char* server = "api.thingspeak.com";    // ThingSpeak server

unsigned long lastConnectionTime = 0;              // last connection time
const unsigned long postingInterval = 15L * 1000L; // 15000ms = 15 second

void setup() {
  Serial.begin(9600);   // initialize baud rate = 9600bps
  setup_wifi();         // run setup_wifi() subroutine
}

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

void loop() {
  if (WiFi.status() == WL_CONNECTED) { // if Wi-Fi is connected
    int chk = faya_dht11.read11(dataPin);    // read sensor data
    int read_temp = faya_dht11.temperature;  // get temperature value
    int read_humd = faya_dht11.humidity;     // get humidity value

    if (millis() - lastConnectionTime > postingInterval) {
      sendDataToThingSpeak(read_temp, read_humd);
      lastConnectionTime = millis();         // update last connection time
    }
  } else {
    setup_wifi(); // reconnect Wi-Fi
    delay(1000);  // delay 1 second before next try
  }
}

void sendDataToThingSpeak(int temperature, int humidity) {
  if (espClient.connect(server, 80)) {
    String url = "/update?api_key=";
    url += writeApiKey;
    url += "&field1=";
    url += temperature;
    url += "&field2=";
    url += humidity;

    espClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                    "Host: " + server + "\r\n" +
                    "Connection: close\r\n\r\n");

    Serial.print("Requesting URL: ");
    Serial.println(url);
    espClient.stop();
  } else {
    Serial.println("Connection to ThingSpeak failed");
  }
}
