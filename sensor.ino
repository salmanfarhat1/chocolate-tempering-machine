#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =========================
// PINS
// =========================

#define SENSOR_PIN 4

// Pump
const int PUMP_RELAY_PIN = 13;
const int PUMP_LED_PIN = 12;
const int PUMP_BUTTON_PIN = 27;

// Heater
const int HEATER_RELAY_PIN = 14;
const int HEATER_BUTTON_PIN = 26;
const int HEATER_LED_PIN = 25;

// Peltier
const int PELTIER_RELAY_PIN = 33;
const int PELTIER_BUTTON_PIN = 32;
const int PELTIER_LED_PIN = 23;

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

const unsigned long conversionDelay = 750;

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
// PELTIER BUTTON
// =========================

bool peltierState = false;
bool peltierLastReading = HIGH;
bool peltierButtonState = HIGH;
unsigned long peltierLastDebounceTime = 0;

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

  // Temperature sensor
  sensors.begin();
  sensors.setWaitForConversion(false);

  // =========================
  // PUMP
  // =========================

  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(PUMP_LED_PIN, OUTPUT);
  pinMode(PUMP_BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(PUMP_RELAY_PIN, HIGH);
  digitalWrite(PUMP_LED_PIN, LOW);

  // =========================
  // HEATER
  // =========================

  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(HEATER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(HEATER_LED_PIN, OUTPUT);

  digitalWrite(HEATER_RELAY_PIN, HIGH);
  digitalWrite(HEATER_LED_PIN, LOW);

  // =========================
  // PELTIER
  // =========================

  pinMode(PELTIER_RELAY_PIN, OUTPUT);
  pinMode(PELTIER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PELTIER_LED_PIN, OUTPUT);

  digitalWrite(PELTIER_RELAY_PIN, HIGH);
  digitalWrite(PELTIER_LED_PIN, LOW);

  // =========================
  // OLED
  // =========================

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

  // Temperature
  handleTemperature();

  // Pump button
  handleButton(
    PUMP_BUTTON_PIN,
    lastReading,
    buttonState,
    lastDebounceTime,
    pumpState,
    PUMP_RELAY_PIN,
    PUMP_LED_PIN,
    "PUMP"
  );

  // Heater button
  handleButton(
    HEATER_BUTTON_PIN,
    heaterLastReading,
    heaterButtonState,
    heaterLastDebounceTime,
    heaterState,
    HEATER_RELAY_PIN,
    HEATER_LED_PIN,
    "HEATER"
  );

  // Peltier button
  handleButton(
    PELTIER_BUTTON_PIN,
    peltierLastReading,
    peltierButtonState,
    peltierLastDebounceTime,
    peltierState,
    PELTIER_RELAY_PIN,
    PELTIER_LED_PIN,
    "PELTIER"
  );
}

// =========================
// TEMPERATURE
// =========================

void handleTemperature() {

  // Start temperature conversion
  if (!tempConversionInProgress &&
      millis() - lastTempTime >= 1000) {

    lastTempTime = millis();

    sensors.requestTemperatures();

    conversionStartTime = millis();
    tempConversionInProgress = true;
  }

  // Read temperature after conversion
  if (tempConversionInProgress &&
      millis() - conversionStartTime >= conversionDelay) {

    tempC = sensors.getTempCByIndex(0);

    tempConversionInProgress = false;

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" C");

    updateDisplay();
  }
}

// =========================
// GENERIC BUTTON HANDLER
// =========================

void handleButton(
  int pin,
  bool &lastReading,
  bool &buttonState,
  unsigned long &lastDebounceTime,
  bool &state,
  int relayPin,
  int ledPin,
  const char* label
) {

  bool reading = digitalRead(pin);

  // Button changed
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  // Debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {

      buttonState = reading;

      // Button pressed
      if (buttonState == LOW) {

        state = !state;

        // Relay is active LOW
        digitalWrite(
          relayPin,
          state ? LOW : HIGH
        );

        // Only control LED if there is one
        if (ledPin != -1) {
          digitalWrite(
            ledPin,
            state ? HIGH : LOW
          );
        }

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

  // Title
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("TEMPERING MACHINE");

  // Temperature
  display.setTextSize(2);
  display.setCursor(0, 16);

  if (tempC == DEVICE_DISCONNECTED_C) {

    display.println("Sensor ERR");

  } else {

    display.print(tempC, 1);
    display.println(" C");
  }

  // Status
  display.setTextSize(1);

  display.setCursor(0, 38);

  display.print("P:");
  display.print(pumpState ? "ON " : "OFF");

  display.print(" H:");
  display.print(heaterState ? "ON " : "OFF");

  display.print(" C:");
  display.println(peltierState ? "ON" : "OFF");

  display.display();
}
