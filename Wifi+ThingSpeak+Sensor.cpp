#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "SunwayEdu";         // Replace with your WiFi SSID
const char* password = ""; // Replace with your WiFi password

WiFiClient client;

// ThingSpeak settings
unsigned long myChannelNumber = 2975330; 
const char * myWriteAPIKey = "P7N4N8GGXILVWY9P"; 

// DS18B20 setup
#define ONE_WIRE_BUS 4  // GPIO4 (D2)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LCD setup (0x27 is common address, 16x2 display)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  sensors.begin();

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("DS18B20 + WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");

  if (temperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data");
    lcd.print("Error         ");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");

    lcd.print(temperatureC);
    lcd.print((char)223); // Degree symbol
    lcd.print("C       "); // Clear trailing chars

    // Send temperature to ThingSpeak
    ThingSpeak.writeField(myChannelNumber, 1, temperatureC, myWriteAPIKey);
  }

  delay(15000);  // ThingSpeak requires minimum 15 seconds between writes
}
