// Blynk Template info
#define BLYNK_TEMPLATE_ID "TMPL6eoSfctzR"
#define BLYNK_TEMPLATE_NAME "IOT Agriculture"
#define BLYNK_AUTH_TOKEN "s0uaYLq3KStgyuW9zWfVMcF2iMigEtML"

// Libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi credentials
char ssid[] = "SunwayEdu";     // Your WiFi name
char pass[] = "";              // Leave empty if no password

// DS18B20 temperature sensor on GPIO4 (D2)
#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Relay (solenoid) connected to GPIO14 (D5)
#define RELAY_PIN D5

// Blynk timer
BlynkTimer timer;

// Read and send temperature to Blynk V0
void sendTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.println(" °C");

  if (tempC != DEVICE_DISCONNECTED_C) {
    Blynk.virtualWrite(V0, tempC);  // Update V0 for Gauge + Chart
  } else {
    Serial.println("Error: DS18B20 not detected!");
  }
}

// Manual solenoid control via button on V1
BLYNK_WRITE(V1) {
  int state = param.asInt();  // 1 = ON, 0 = OFF
  digitalWrite(RELAY_PIN, state);
  Serial.println(state ? "Solenoid ON" : "Solenoid OFF");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting system...");

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Solenoid OFF by default

  sensors.begin();

  // Connect to WiFi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Send temperature every 2 seconds
  timer.setInterval(2000L, sendTemperature);
}

void loop() {
  Blynk.run();
  timer.run();
}