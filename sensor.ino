#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =========================
// PINS
// =========================

#define SENSOR_PIN 4

const int PUMP_RELAY_PIN = 13;      // Pump relay
const int PUMP_LED_PIN = 12;        // Pump LED
const int PUMP_BUTTON_PIN = 27;     // Pump button

const int HEATER_RELAY_PIN = 14;   // Heater relay (K4)
const int HEATER_BUTTON_PIN = 26;  // Heater button
const int HEATER_LED_PIN = 25;     // Heater LED

// =========================
// OLED
// =========================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// =========================
// TEMPERATURE SENSOR
// =========================

OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);

bool tempConversionInProgress = false;
unsigned long conversionStartTime = 0;
const unsigned long conversionDelay = 750; // ms, matches 12-bit DS18B20

// =========================
// PUMP BUTTON
// =========================

bool pumpState = false;
bool lastReading = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;

// =========================
// HEATER BUTTON
// =========================

bool heaterState = false;
bool heaterLastReading = HIGH;
bool heaterButtonState = HIGH;
unsigned long heaterLastDebounceTime = 0;

// =========================
// TIMING
// =========================

unsigned long lastTempTime = 0;
const unsigned long debounceDelay = 50;

float tempC = 0.0;

// =========================
// SETUP
// =========================

void setup() {

  Serial.begin(115200);

  sensors.begin();
  sensors.setWaitForConversion(false); // don't block while sensor converts

  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(PUMP_LED_PIN, OUTPUT);
  pinMode(PUMP_BUTTON_PIN, INPUT_PULLUP);

  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(HEATER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(HEATER_LED_PIN, OUTPUT);

  digitalWrite(PUMP_RELAY_PIN, HIGH);
  digitalWrite(PUMP_LED_PIN, LOW);

  digitalWrite(HEATER_RELAY_PIN, HIGH);
  digitalWrite(HEATER_LED_PIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Tempering Machine");
  display.display();

  delay(1000);
}

// =========================
// LOOP
// =========================

void loop() {

  handleTemperature();
  handleButton(PUMP_BUTTON_PIN, lastReading, buttonState, lastDebounceTime, pumpState, PUMP_RELAY_PIN, PUMP_LED_PIN, "PUMP");
  handleButton(HEATER_BUTTON_PIN, heaterLastReading, heaterButtonState, heaterLastDebounceTime, heaterState, HEATER_RELAY_PIN, HEATER_LED_PIN, "HEATER");
}

// =========================
// TEMPERATURE (non-blocking)
// =========================

void handleTemperature() {

  if (!tempConversionInProgress && millis() - lastTempTime >= 1000) {
    lastTempTime = millis();
    sensors.requestTemperatures();
    conversionStartTime = millis();
    tempConversionInProgress = true;
  }

  if (tempConversionInProgress && millis() - conversionStartTime >= conversionDelay) {
    tempC = sensors.getTempCByIndex(0);
    tempConversionInProgress = false;

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" C");

    updateDisplay();
  }
}

// =========================
// GENERIC DEBOUNCED BUTTON HANDLER
// =========================

void handleButton(int pin, bool &lastReading, bool &buttonState,
                   unsigned long &lastDebounceTime, bool &state,
                   int relayPin, int ledPin, const char* label) {

  bool reading = digitalRead(pin);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        state = !state;

        digitalWrite(relayPin, state ? LOW : HIGH);
        digitalWrite(ledPin, state ? HIGH : LOW);

        Serial.print(label);
        Serial.println(state ? " ON" : " OFF");

        updateDisplay();
      }
    }
  }

  lastReading = reading;
}

// =========================
// OLED DISPLAY
// =========================

void updateDisplay() {

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("TEMPERING MACHINE");

  display.setTextSize(2);
  display.setCursor(0, 18);

  if (tempC == DEVICE_DISCONNECTED_C) {
    display.println("Sensor ERR");
  } else {
    display.print(tempC, 1);
    display.println(" C");
  }

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Pump: ");
  display.println(pumpState ? "ON" : "OFF");

  display.setCursor(0, 52);
  display.print("Heater: ");
  display.println(heaterState ? "ON" : "OFF");

  display.display();
}