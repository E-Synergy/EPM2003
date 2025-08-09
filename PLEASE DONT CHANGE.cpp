// ====== Blynk Template Info ======
#define BLYNK_TEMPLATE_ID "TMPL6eoSfctzR"
#define BLYNK_TEMPLATE_NAME "IOT Agriculture"
#define BLYNK_AUTH_TOKEN "s0uaYLq3KStgyuW9zWfVMcF2iMigEtML"

// ====== Libraries ======
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>

// ====== WiFi credentials ======
char ssid[] = "BigDong";
char pass[] = "645619123";

// ====== Pin assignments ======
#define ONE_WIRE_BUS D2     // DS18B20 data pin (GPIO4)
#define RELAY_PIN D5        // Relay control pin (GPIO14)

// ====== LCD I2C settings ======
LiquidCrystal_I2C lcd(0x27, 16, 2);  // 16x2 LCD, I2C address 0x27

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
BlynkTimer timer;

// ====== States ======
bool autoMode = false;
bool relayState = false;

// ====== Timezone for Malaysia ======
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;

// ====== Blynk Controls ======

// Manual relay control (V1)
BLYNK_WRITE(V1) {
  relayState = param.asInt();
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  Serial.println(relayState ? "Relay: MANUAL ON" : "Relay: MANUAL OFF");
}

// Auto mode toggle (V2)
BLYNK_WRITE(V2) {
  autoMode = param.asInt();
  Serial.println(autoMode ? "AUTO MODE: ON" : "AUTO MODE: OFF");
}

// ====== LCD Display Function ======

void updateDisplay(float tempC, const char* relayStatus, const char* timeStr) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  lcd.print(timeStr);

  lcd.setCursor(15, 0);  // Show 'A' or 'M' at top-right
  lcd.print(autoMode ? "A" : "M");

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(tempC);
  lcd.print((char)223);  // Degree symbol
  lcd.print("C ");
  lcd.print(relayStatus);
}

// ====== Temperature Check Function ======

void checkTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // Handle sensor error
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: DS18B20 not detected!");

    // Show error on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    lcd.setCursor(0, 1);
    lcd.print("Check DS18B20");

    // Also send error flag to Blynk (optional)
    Blynk.virtualWrite(V0, -127);  // convention: -127 = error

    return;
  }

  Blynk.virtualWrite(V0, tempC);  // Send temp to Blynk gauge/chart

  // Get current time
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char timeStr[9];  // HH:MM:SS
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);

  // Auto mode logic
  if (autoMode && tempC > 30.0 && !relayState) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    Serial.println("AUTO: Relay ON due to temp > 30°C");
  } else if (autoMode && tempC <= 30.0 && relayState) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    Serial.println("AUTO: Relay OFF, temp normal");
  }

  const char* relayStatus = relayState ? "ON " : "OFF";
  updateDisplay(tempC, relayStatus, timeStr);
}

// ====== NTP Time Sync ======

void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP");
  while (time(nullptr) < 100000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTime sync complete!");
}

// ====== Setup ======

void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Start with relay OFF

  sensors.begin();

  Wire.begin(D3, D4);            // Use D3 (GPIO0) for SDA, D4 (GPIO2) for SCL
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  setupTime();

  // Sync saved states from Blynk
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);

  timer.setInterval(5000L, checkTemperature);  // every 5 seconds
}

// ====== Loop ======

void loop() {
  Blynk.run();
  timer.run();
}
