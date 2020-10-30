#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
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
  if (!MDNS.begin(MDNS_NAME)) {
    Serial.println("Error setting up MDNS responder!");
  } else
  {
      Serial.println("mDNS responder started");
  }
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin(); 
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
  MDNS.update();
  ArduinoOTA.handle();
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