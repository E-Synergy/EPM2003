#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>  // For NTP time

// WiFi credentials
const char* ssid = "SunwayEdu";
const char* password = "";

// ThingSpeak settings
WiFiClient client;
unsigned long myChannelNumber = 2975330;
const char * myWriteAPIKey = "P7N4N8GGXILVWY9P";

// DS18B20 setup
#define ONE_WIRE_BUS 4  // GPIO4 (D2)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LCD setup (0x27 address, 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Relay setup
const int relayPin = 5; // GPIO5 (D1)
bool valveOpen = false;
unsigned long valveStart = 0;
const unsigned long valveDuration = 10UL * 60UL * 1000UL; // 10 minutes

// Time settings
const long gmtOffset_sec = 8 * 3600;  // GMT+8 for Malaysia
const int daylightOffset_sec = 0;

// Timing
unsigned long previousMillis = 0;
const long interval = 15000;  // ThingSpeak requires at least 15 sec

// Time trigger
const int openHour = 14;
const int openMinute = 0;

void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time");
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nNTP time set!");
}

void setup() {
  Serial.begin(9600);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Ensure relay is OFF

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected");

  sensors.begin();
  ThingSpeak.begin(client);

  setupTime();
  delay(1000);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    lcd.clear();
    lcd.setCursor(0, 0);

    if (temperatureC == DEVICE_DISCONNECTED_C) {
      Serial.println("Error: DS18B20 not found");
      lcd.print("Sensor Error");
    } else {
      Serial.print("Temperature: ");
      Serial.print(temperatureC);
      Serial.println(" \xC2\xB0C");

      lcd.print("Temp: ");
      lcd.print(temperatureC);
      lcd.print((char)223);  // Degree symbol
      lcd.print("C");

      ThingSpeak.writeField(myChannelNumber, 1, temperatureC, myWriteAPIKey);
    }

    // Show time from NTP
    time_t now = time(nullptr);
    struct tm * timeinfo = localtime(&now);
    char timeStr[9];  // HH:MM:SS
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);

    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    lcd.print(timeStr);
  }

  // Valve control based on time
  time_t now = time(nullptr);
  struct tm * curTime = localtime(&now);

  if (!valveOpen && curTime->tm_hour == openHour && curTime->tm_min == openMinute && curTime->tm_sec < 2) {
    digitalWrite(relayPin, LOW); // Turn relay ON
    valveOpen = true;
    valveStart = currentMillis;
    Serial.println("Valve Opened");
  }

  if (valveOpen && (currentMillis - valveStart >= valveDuration)) {
    digitalWrite(relayPin, HIGH); // Turn relay OFF
    valveOpen = false;
    Serial.println("Valve Closed");
  }
}
