#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "wifi_psk.h"
#include "mqtt.h"



 int humidityLevel;
 int waterLevel;

 WiFiClient espClient;

 PubSubClient client(espClient);

void mqtt_reconnect();
void wifi_connect();

char buffer[10];

void serialDump();
void water();

void setup() {
  Serial.begin(9600);
  pinMode(WATER_SENSOR_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, 1);
  wifi_connect();
  client.setServer(MQTT_ADDRESS, MQTT_PORT);
  mqtt_reconnect();
}

void wifi_connect() {
  WiFi.begin(STASSID, STAPSK);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

}


void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void loop() {
  humidityLevel = analogRead(0);
  waterLevel = 1 - digitalRead(WATER_SENSOR_PIN);

  serialDump();
  client.publish(MQTT_TOPIC_HUMIDITY,  itoa(humidityLevel,buffer, 10));
  client.publish(MQTT_TOPIC_TANK_1_LEVEL, itoa(waterLevel, buffer, 10));
  delay(5000);
  if (humidityLevel > HUMIDITY_THRESHOLD){
    if (waterLevel > 0){
      water();
    } else {
      Serial.println("Tank is empty");
    }
  }
}

void water(){
  digitalWrite(PUMP_PIN, 0);
  Serial.println("Begin watering.");
  client.publish(MQTT_PUMP_1_TOPIC, "1");
  delay(5000);
  digitalWrite(PUMP_PIN, 1);
  Serial.println("End watering");
  client.publish(MQTT_PUMP_1_TOPIC, "0");
}

void serialDump(){
  Serial.print("Water level: ");
  Serial.flush();
  Serial.println(waterLevel);
  Serial.print("Humidity level: ");
  Serial.flush();
  Serial.println(humidityLevel);
}