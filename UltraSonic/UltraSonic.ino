#include <EspMQTTClient.h>
#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define D5 14
#define D6 12

// define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define DIST_THRESH 30

const int trigPin = D6;
const int echoPin = D5;

// WifiManager
WiFiManager wm;

// MQTT settings
#define mqttBroker "sweetdream.iptime.org"
#define mqttClientname "22100113@SB"
#define MQTTUsername "iot"
#define MQTTPassword "csee1414"

String mqttTopic = "iot/5";

EspMQTTClient mqttClient(
  mqttBroker,     // MQTT Broker server ip
  1883,           // The MQTT port, default to 1883. this line can be omitted
  MQTTUsername,   // Can be omitted if not needed
  MQTTPassword,   // Can be omitted if not needed
  mqttClientname  // Client name that uniquely identify your device
);

// defines variables
long duration;
int distance;


void setup() {
  Serial.begin(115200);

  // init WiFi connection
  bool res;
  res = wm.autoConnect(); // auto generated AP name from chipid

  if(!res) {
      Serial.println("Failed to connect");
  } 
  else {
      Serial.println("WiFi connected...");
  }

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
}

void loop() {
  // mqtt subscribe
  mqttClient.loop();

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * SOUND_VELOCITY / 2;

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  // Check if the distance is less than 30 cm
  if (distance < DIST_THRESH) {
    JSONVar myObject;
    myObject["dist"] = (int) distance;
    String jsonString = JSON.stringify(myObject);

    const String pubTopic = mqttTopic + "/data";

    // Publish the payload to the MQTT topic
    mqttClient.publish(pubTopic, jsonString);
  }

  // Handle MQTT events
  mqttClient.loop();

  delay(2000);
}


// on mqtt connection established, read from subscribed topic
void onConnectionEstablished() {
  mqttClient.subscribe(mqttTopic, [](const String & payload) {
    Serial.println(payload);
  });

  mqttClient.publish(mqttTopic, "Greetings from NodeMCU");
}
