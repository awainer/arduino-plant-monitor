#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "wifi_psk.h"
#include "mqtt.h"

#define SLEEP_INTERVAL 10e06 // seconds 

 int readValue;
 WiFiClient espClient;

 PubSubClient client(espClient);

void mqtt_reconnect();
void wifi_connect();

char buffer[10];


void setup() {
 Serial.begin(9600);
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
  readValue = analogRead(0);
  Serial.println(readValue);
  client.publish(MQTT_TOPIC, itoa(readValue,buffer, 10));
  // Allow the message to be sent before going to sleep
  delay(2000);
  ESP.deepSleep(SLEEP_INTERVAL); 
}