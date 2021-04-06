
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
/* 
 *  2019/07/07
all settings made in config.h 
const char* SSID = "";
const char* PSK = "";
const char* MQTT_BROKER = "";
*/

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIN D6

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, NEOPIN, NEO_GRB + NEO_KHZ800);

long lastMsg = 0;
char msg[50];
int value = 0;
StaticJsonDocument<200> doc;

void setup() {
  
  Serial.begin(115200);

  setup_wifi();
  client.setServer(MQTT_BROKER, 8883);
  client.setCallback(callback);
  strip.begin();
  strip.setBrightness(50);
  Serial.println("dazwischen");
  strip.show(); // Initialize all pixels to 'off'
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");  
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  DeserializationError error = deserializeJson(doc, payload);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  String status = doc["status"];
  Serial.println(status);
  if (status == "open") {
    Serial.println("OFFEN!");
    colorWipe(strip.Color(255, 0, 0), 50);
  } else {
    Serial.println("ZU!");
    colorWipe(strip.Color(0, 255, 0), 50);
  }
  
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Reconnecting...");
    if (!client.connect("statusLight")) {
      Serial.print("Failed, rd=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    } else {
      client.subscribe("/home/garage/door");
    }
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //snprintf(msg, 50, "Alive since %ld milliseconds", millis());
  //Serial.print("Publish message: ");
  //Serial.println(msg);
  //client.publish("/home/garage", msg);

  /*if (digitalRead(pinStatusGarageDoor) == LOW) {
    Serial.println("Closed");
    snprintf(msg, 50, "{\"status\":\"closed\"}");
    Serial.println(msg);
    client.publish("/home/garage/door", msg);
  }
  else {
    Serial.println("open");
    snprintf(msg, 50, "{\"status\":\"open\"}");
    client.publish("/home/garage/door", msg);
    Serial.println(msg);
  }*/
  
  delay(1000);
}
