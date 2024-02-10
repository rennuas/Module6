#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BH1750.h>

#define RELAY_PIN D0
#define SCL D1
#define SDA D2

// Update these with your Wi-Fi and MQTT broker details
const char* ssid = "MIFI-005";
const char* password = "Batam#05";
const char* mqtt_server = "6315efe699464371a7a7ca7423114c9a.s2.eu.hivemq.cloud";
const char* mqtt_username = "iotkit";
const char* mqtt_password = "Pns12345";
const int mqtt_port = 8883;

unsigned int lux;
char light[16];

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

LiquidCrystal_I2C lcd(0x27, 16, 2);
BH1750 lightMeter(0x23);

JsonDocument data;

void setupConnection();
void mqttReconnect();
void callback(char *topic, byte *payload, unsigned int length);
void publishMessage(const char* topic, String payload , boolean retained);

// PIN LED, Buzzer dan relay #define LED_PIN	D0 #define BUZZ_PIN D1 #define RELAY_PIN D2
void setup() {
    // Serial Monitor pada baudrate 115200
    Serial.begin(115200);

    Wire.begin();
    lightMeter.begin();

    lcd.init();
    lcd.backlight();
    
    // Mengatur PIN LED, Buzzer dan Relay sebagai OUTPUT 
    pinMode(RELAY_PIN, OUTPUT);
    
    setupConnection();
}

void setupConnection(){
    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        lcd.setCursor(0, 0);
        lcd.print("Wifi Connecting");
        Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("\nWiFi connected\nIP address: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wifi Connected");

    delay(100);

    espClient.setInsecure();

    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
}

void loop() {

    if (!mqttClient.connected()) mqttReconnect(); // check if client is connected
    mqttClient.loop();

    lux = lightMeter.readLightLevel();

    if(lux < 100){
        digitalWrite(RELAY_PIN, HIGH);
    }else {
        digitalWrite(RELAY_PIN, LOW);
    }

    sprintf(light, "LIGHT: %5d lx", lux);
    lcd.setCursor(0, 0);
    lcd.print("Light meter (lx)");
    lcd.setCursor(0, 1);
    lcd.print(light);

    publishMessage("module_6/ls", light, true);

}

void mqttReconnect(){
     // Loop until we're reconnected
  while (!mqttClient.connected()) {
    lcd.setCursor(0, 0);
    lcd.print("Connect to MQTT");
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("connected");
        lcd.setCursor(0, 0);
        lcd.print("connected to MQTT");

        lcd.clear();

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      lcd.setCursor(0, 0);
      lcd.print("Connection failed");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length){
    String incommingMessage = "";
    for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

    Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);
}

void publishMessage(const char* topic, String payload , boolean retained){
  if (mqttClient.publish(topic, payload.c_str(), true))
      Serial.println("Message publised ["+String(topic)+"]: "+payload);
}
