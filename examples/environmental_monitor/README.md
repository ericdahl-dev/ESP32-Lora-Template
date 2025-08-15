# Environmental Monitor Example

This example demonstrates how to build an environmental monitoring system using the ESP32 template.

## Features

- Temperature and humidity monitoring (DHT22)
- Air quality sensing (MQ-135)
- Light level detection (BH1750)
- OLED display for real-time data
- WiFi connectivity for remote monitoring
- Data logging to SD card
- Deep sleep for battery efficiency

## Hardware Requirements

- ESP32 board (any variant)
- DHT22 temperature/humidity sensor
- MQ-135 air quality sensor
- BH1750 light sensor
- 128x64 OLED display (SSD1306)
- SD card module (optional)
- 3.7V battery (optional)

## Wiring

```
ESP32 Pin | Component
----------|------------
GPIO 4    | DHT22 Data
GPIO 34   | MQ-135 Analog Out
GPIO 21   | I2C SDA (OLED & BH1750)
GPIO 22   | I2C SCL (OLED & BH1750)
GPIO 5    | SD Card CS
GPIO 18   | SD Card SCK
GPIO 19   | SD Card MISO
GPIO 23   | SD Card MOSI
GPIO 35   | Battery voltage divider
```

## Configuration

1. Copy `wifi_networks_example.h` to `wifi_networks.h`
2. Add your WiFi credentials
3. Build and upload:

```bash
platformio run -e environmental_monitor -t upload
```

## Usage

The device will:
1. Connect to WiFi
2. Read all sensors every 60 seconds
3. Display data on OLED
4. Log to SD card (if present)
5. Sleep between readings to save power

## Data Format

CSV logging format:
```
timestamp,temperature,humidity,air_quality,light_level,battery_voltage
2024-01-14 12:00:00,22.5,45.2,85,350,3.7
```

## Customization

- Adjust reading interval in `platformio.ini`
- Add more sensors by extending the sensor array
- Enable MQTT for cloud integration
- Add web server for local access