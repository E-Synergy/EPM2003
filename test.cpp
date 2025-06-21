// How to connect to ThingSpeak: This project connects to ThingSpeak using the ESP8266 WiFi module.

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>

const char* ssid = "Your_SSID";       // Your Network SSID
const char* password = "Your_PASSWORD";     // Your Network Password

WiFiClient client;

unsigned long myChannelNumber = 2975330; 
const char * myWriteAPIKey = "P7N4N8GGXILVWY9P"; 

int val;

void setup() 
{
    series.begin(9600);
    WiFi.begin(ssid, passwor);
}

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // Seed random number generator using analog noise
    randomSeed(analogRead(A0));

    ThingSpeak.begin(client);
}

