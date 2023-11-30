#include <EspMQTTClient.h>
#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define D5 14
#define D6 12

// define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define DIST_THRESH 70

const int trigPin = D6;
const int echoPin = D5;

// OLED display dimensions
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // display data

// WifiManager
WiFiManager wm;

// MQTT settings
#define mqttBroker "sweetdream.iptime.org"
#define mqttClientname "22100113@SB"
#define MQTTUsername "iot"
#define MQTTPassword "csee1414"

String mqttTopic = "iot/5";
String carMqttTopic = "iot/5/car";

String carCount = "Car Count: 0";

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

  // init OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  display.clearDisplay();

  display_sensors(carCount); // init display to 0 cars

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
  mqttClient.subscribe(carMqttTopic, [](const String & payload) {
    Serial.println(payload);

    carCount = payload;

    display_sensors(carCount);
  });

  mqttClient.publish(mqttTopic, "Greetings from NodeMCU");
}

// display car count to OLED display
void display_sensors(String payload) {
  // setup display
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // write to display buffer
  display.println(payload);

  display.display();
}
