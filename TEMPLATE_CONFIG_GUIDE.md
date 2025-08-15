# ESP32 Template Configuration Guide

This guide helps you configure the ESP32 template for various use cases and project types.

## 📋 Table of Contents

1. [Basic Configuration](#basic-configuration)
2. [Project Types](#project-types)
3. [Hardware Profiles](#hardware-profiles)
4. [Feature Flags](#feature-flags)
5. [Build Configurations](#build-configurations)
6. [Custom Configurations](#custom-configurations)

## 🔧 Basic Configuration

### Step 1: Choose Your Board

Edit `platformio.ini` and select your ESP32 board:

```ini
[env:your_device]
platform = espressif32
framework = arduino

; Choose one of these popular boards:
board = esp32dev                    ; Generic ESP32
; board = esp32-s3-devkitc-1       ; ESP32-S3 DevKit
; board = heltec_wifi_lora_32_V3   ; Heltec V3
; board = ttgo-lora32-v21          ; TTGO LoRa32
; board = esp32cam                 ; ESP32-CAM
```

### Step 2: Set Build Flags

Configure features via build flags:

```ini
build_flags = 
    -D ENABLE_WIFI=1
    -D ENABLE_BLUETOOTH=0
    -D ENABLE_OTA=1
    -D DEBUG_LEVEL=2
    -D DEVICE_NAME="\"MyDevice\""
```

### Step 3: Configure Libraries

Add libraries based on your needs:

```ini
lib_deps = 
    ; Core libraries (always included)
    SPI
    Wire
    
    ; Optional - uncomment as needed
    ; adafruit/Adafruit SSD1306       ; OLED display
    ; sandeepmistry/LoRa               ; LoRa communication
    ; knolleary/PubSubClient           ; MQTT
    ; bblanchon/ArduinoJson            ; JSON parsing
    ; me-no-dev/AsyncTCP               ; Async networking
```

## 🎯 Project Types

### IoT Sensor Node

```ini
[env:sensor_node]
extends = env:base
build_flags = 
    ${env:base.build_flags}
    -D PROJECT_TYPE=SENSOR_NODE
    -D ENABLE_DEEP_SLEEP=1
    -D SENSOR_INTERVAL=60000  ; 1 minute
    -D ENABLE_BATTERY_MONITOR=1

lib_deps = 
    ${env:base.lib_deps}
    adafruit/DHT sensor library
    adafruit/Adafruit BMP280 Library
```

### Home Automation Controller

```ini
[env:home_automation]
extends = env:base
build_flags = 
    ${env:base.build_flags}
    -D PROJECT_TYPE=HOME_AUTOMATION
    -D ENABLE_MQTT=1
    -D ENABLE_WEB_SERVER=1
    -D MAX_DEVICES=32

lib_deps = 
    ${env:base.lib_deps}
    knolleary/PubSubClient
    me-no-dev/ESP Async WebServer
    bblanchon/ArduinoJson
```

### Data Logger

```ini
[env:data_logger]
extends = env:base
build_flags = 
    ${env:base.build_flags}
    -D PROJECT_TYPE=DATA_LOGGER
    -D ENABLE_SD_CARD=1
    -D ENABLE_RTC=1
    -D LOG_INTERVAL=1000

lib_deps = 
    ${env:base.lib_deps}
    arduino-libraries/SD
    adafruit/RTClib
```

### Gateway/Bridge

```ini
[env:gateway]
extends = env:base
build_flags = 
    ${env:base.build_flags}
    -D PROJECT_TYPE=GATEWAY
    -D ENABLE_LORA=1
    -D ENABLE_WIFI=1
    -D ENABLE_ETHERNET=0
    -D BRIDGE_MODE=1

lib_deps = 
    ${env:base.lib_deps}
    sandeepmistry/LoRa
    bblanchon/ArduinoJson
```

## 🔌 Hardware Profiles

### Display Options

```ini
; OLED Display (I2C)
-D ENABLE_DISPLAY=1
-D DISPLAY_TYPE=SSD1306
-D DISPLAY_WIDTH=128
-D DISPLAY_HEIGHT=64
-D DISPLAY_I2C_ADDR=0x3C

; TFT Display (SPI)
-D ENABLE_DISPLAY=1
-D DISPLAY_TYPE=ILI9341
-D DISPLAY_WIDTH=320
-D DISPLAY_HEIGHT=240
```

### Communication Options

```ini
; WiFi Configuration
-D ENABLE_WIFI=1
-D WIFI_MODE=STA              ; STA, AP, or STA_AP
-D WIFI_HOSTNAME="esp32-device"

; LoRa Configuration
-D ENABLE_LORA=1
-D LORA_FREQUENCY=915E6       ; 915MHz (US)
-D LORA_SPREADING_FACTOR=7
-D LORA_BANDWIDTH=125E3

; Bluetooth Configuration
-D ENABLE_BLUETOOTH=1
-D BT_DEVICE_NAME="ESP32-BT"
-D ENABLE_BLE=1
```

### Sensor Configurations

```ini
; Environmental Sensors
-D ENABLE_TEMP_SENSOR=1
-D TEMP_SENSOR_TYPE=DHT22     ; DHT22, DS18B20, BME280
-D TEMP_SENSOR_PIN=4

; Motion Detection
-D ENABLE_MOTION_SENSOR=1
-D MOTION_SENSOR_PIN=5
-D MOTION_SENSOR_TYPE=PIR     ; PIR, RADAR, ULTRASONIC

; Light Sensors
-D ENABLE_LIGHT_SENSOR=1
-D LIGHT_SENSOR_TYPE=BH1750   ; BH1750, TSL2561, ANALOG
```

## 🚦 Feature Flags

### Core Features

```ini
; Debugging
-D DEBUG_LEVEL=2              ; 0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG
-D ENABLE_SERIAL_DEBUG=1
-D DEBUG_BAUD_RATE=115200

; Power Management
-D ENABLE_DEEP_SLEEP=1
-D SLEEP_DURATION=3600        ; 1 hour in seconds
-D ENABLE_BATTERY_MONITOR=1
-D BATTERY_PIN=35
-D BATTERY_DIVIDER_RATIO=2.0

; Storage
-D ENABLE_SPIFFS=1
-D ENABLE_SD_CARD=0
-D ENABLE_EEPROM=0

; Time Management
-D ENABLE_NTP=1
-D NTP_SERVER="pool.ntp.org"
-D TIMEZONE_OFFSET=-8         ; PST
```

### Security Features

```ini
; Authentication
-D ENABLE_AUTH=1
-D AUTH_USERNAME="admin"
-D AUTH_PASSWORD="changeme"

; Encryption
-D ENABLE_SSL=1
-D SSL_CERT_VALIDATION=0

; OTA Security
-D OTA_PASSWORD="ota_secret"
-D OTA_PORT=3232
```

## 🏗️ Build Configurations

### Development Build

```ini
[env:dev]
extends = env:base
build_type = debug
build_flags = 
    ${env:base.build_flags}
    -D DEVELOPMENT=1
    -D DEBUG_LEVEL=4
    -D ENABLE_TELNET_DEBUG=1
    -O0  ; No optimization
```

### Production Build

```ini
[env:prod]
extends = env:base
build_type = release
build_flags = 
    ${env:base.build_flags}
    -D PRODUCTION=1
    -D DEBUG_LEVEL=1
    -D ENABLE_WATCHDOG=1
    -Os  ; Size optimization
```

### Testing Build

```ini
[env:test]
extends = env:base
build_flags = 
    ${env:base.build_flags}
    -D TESTING=1
    -D ENABLE_TEST_MODE=1
    -D MOCK_SENSORS=1
```

## 🛠️ Custom Configurations

### Creating Your Own Configuration

1. **Define Your Requirements**
   ```ini
   ; myproject_config.h
   #define PROJECT_NAME "CustomProject"
   #define CUSTOM_FEATURE_1 1
   #define CUSTOM_FEATURE_2 0
   ```

2. **Create Environment**
   ```ini
   [env:custom]
   extends = env:base
   build_flags = 
       ${env:base.build_flags}
       -D CUSTOM_CONFIG=1
       -include myproject_config.h
   ```

3. **Conditional Compilation**
   ```cpp
   // In your code
   #ifdef CUSTOM_FEATURE_1
       // Feature 1 implementation
   #endif
   ```

### Configuration Templates

Save these as starting points:

**Minimal Configuration**
```ini
[env:minimal]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

**Full Featured**
```ini
[env:full]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

build_flags = 
    -D ENABLE_WIFI=1
    -D ENABLE_BLUETOOTH=1
    -D ENABLE_DISPLAY=1
    -D ENABLE_OTA=1
    -D DEBUG_LEVEL=3

lib_deps = 
    SPI
    Wire
    WiFi
    adafruit/Adafruit SSD1306
    bblanchon/ArduinoJson
    knolleary/PubSubClient
```

### Environment Variables

Use environment variables for sensitive data:

```ini
build_flags = 
    -D WIFI_SSID="${sysenv.WIFI_SSID}"
    -D WIFI_PASSWORD="${sysenv.WIFI_PASSWORD}"
    -D API_KEY="${sysenv.API_KEY}"
```

Set them before building:
```bash
export WIFI_SSID="MyNetwork"
export WIFI_PASSWORD="MyPassword"
export API_KEY="secret_key"
platformio run
```

## 📝 Configuration Best Practices

1. **Use Base Configurations**
   - Create a base environment with common settings
   - Extend it for specific configurations

2. **Organize by Feature**
   - Group related flags together
   - Use clear naming conventions

3. **Document Your Flags**
   - Add comments explaining each flag
   - Include valid values and defaults

4. **Version Control**
   - Keep sensitive data out of version control
   - Use `.env` files for local configurations

5. **Test Configurations**
   - Create test builds for each configuration
   - Verify feature combinations work correctly

## 🔍 Troubleshooting

### Common Issues

1. **Conflicting Features**
   ```ini
   ; Don't enable both simultaneously
   -D ENABLE_DEEP_SLEEP=1
   -D ENABLE_WEB_SERVER=1  ; Requires constant connection
   ```

2. **Memory Constraints**
   ```ini
   ; For memory-constrained boards
   -D REDUCE_MEMORY_USAGE=1
   -D DISABLE_DEBUG_STRINGS=1
   -D USE_PROGMEM=1
   ```

3. **Pin Conflicts**
   - Check pin assignments for conflicts
   - Some pins have special functions (boot, flash)
   - Use pin mapping documentation for your board

### Validation Script

Create a configuration validator:

```python
# validate_config.py
import configparser

def validate_platformio_config(filename):
    config = configparser.ConfigParser()
    config.read(filename)
    
    # Check for required sections
    assert any(s.startswith('env:') for s in config.sections()), "No environment defined"
    
    # Validate each environment
    for section in config.sections():
        if section.startswith('env:'):
            # Check required fields
            assert 'platform' in config[section], f"{section}: Missing platform"
            assert 'board' in config[section], f"{section}: Missing board"
            assert 'framework' in config[section], f"{section}: Missing framework"
    
    print("Configuration valid!")

if __name__ == "__main__":
    validate_platformio_config("platformio.ini")
```

## 🚀 Next Steps

1. Choose your project type from the templates above
2. Customize the configuration for your hardware
3. Add any additional libraries needed
4. Test your configuration with a simple sketch
5. Iterate and refine based on your requirements

Remember: Start simple and add features incrementally!