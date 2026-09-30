# Chocolate Tempering Machine

DIY chocolate tempering controller based on ESP32.

## Features

- DS18B20 temperature sensor
- SSD1306 OLED display
- Pump relay control
- Heater relay control
- Physical buttons with debounce
- Non-blocking temperature reading

## Hardware

- ESP32
- DS18B20 temperature sensor
- 128x64 OLED display
- 12V pump
- Heater
- Relay modules
- Push buttons

## Pin Configuration

| Component | ESP32 Pin |
|---|---|
| Temperature Sensor | GPIO 4 |
| Pump Relay | GPIO 13 |
| Pump LED | GPIO 12 |
| Pump Button | GPIO 27 |
| Heater Relay | GPIO 14 |
| Heater LED | GPIO 25 |
| Heater Button | GPIO 26 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |

## Temperature Process

- Heating phase: ~50°C
- Cooling phase: ~27°C
- Working phase: ~31-32°C

## Firmware

Main file:

