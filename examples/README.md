# ESP32 Template Examples

This directory contains ready-to-use example projects built with the ESP32 template. Each example demonstrates different use cases and features of the template.

## 📦 Available Examples

### 1. Environmental Monitor
A complete environmental monitoring system with multiple sensors.

**Features:**
- Temperature & humidity sensing (DHT22)
- Air quality monitoring
- Light level detection
- OLED display
- SD card logging
- Deep sleep for battery operation

[View Example →](./environmental_monitor/)

### 2. Smart Home Controller
Home automation hub with relay control and sensor integration.

**Features:**
- 4-channel relay control
- Motion detection
- Door/window sensors
- MQTT integration
- Web interface
- Alexa/Google Home support

[View Example →](./smart_home/)

### 3. Data Logger
High-performance data logging system with multiple storage options.

**Features:**
- Multi-sensor support
- SD card storage
- Real-time clock (RTC)
- CSV/JSON export
- USB mass storage mode
- Battery backup

[View Example →](./data_logger/)

### 4. IoT Gateway
Bridge between local sensors and cloud services.

**Features:**
- LoRa to WiFi bridge
- MQTT broker
- Protocol translation
- Data filtering/aggregation
- OTA updates
- Remote management

[View Example →](./iot_gateway/)

## 🚀 Using the Examples

### Quick Start

1. **Choose an example** that matches your use case
2. **Copy the example** to your project directory:
   ```bash
   cp -r examples/environmental_monitor/* .
   ```
3. **Install dependencies**:
   ```bash
   platformio lib install
   ```
4. **Configure WiFi** (if needed):
   ```bash
   cp wifi_networks_example.h wifi_networks.h
   # Edit wifi_networks.h with your credentials
   ```
5. **Build and upload**:
   ```bash
   platformio run -t upload
   ```

### Customizing Examples

Each example is designed to be easily customizable:

1. **Hardware Configuration**: Edit `platformio.ini` to match your hardware
2. **Feature Flags**: Enable/disable features via build flags
3. **Pin Assignments**: Modify pin definitions in the configuration
4. **Sensor Types**: Swap sensors by changing the sensor class

## 📋 Example Structure

Each example follows the same structure:

```
example_name/
├── README.md           # Example documentation
├── platformio.ini      # Build configuration
├── src/
│   └── main.cpp       # Main application code
├── lib/               # Example-specific libraries (optional)
└── data/              # Web files, certificates (optional)
```

## 🔧 Common Modifications

### Adding a New Sensor

```cpp
// 1. Include the sensor header
#include "sensors/new_sensor.h"

// 2. Create sensor instance
NewSensor mySensor(SENSOR_PIN);

// 3. Initialize in setup()
mySensor.begin();

// 4. Read in loop()
float value = mySensor.read();
```

### Changing Communication Protocol

```ini
; In platformio.ini
; From WiFi to LoRa:
-D ENABLE_WIFI=0
-D ENABLE_LORA=1
-D LORA_FREQUENCY=915E6
```

### Adjusting Power Settings

```ini
; Enable deep sleep
-D ENABLE_DEEP_SLEEP=1
-D SLEEP_DURATION=300  ; 5 minutes

; Or disable for always-on operation
-D ENABLE_DEEP_SLEEP=0
```

## 📚 Learning Path

We recommend exploring the examples in this order:

1. **Environmental Monitor** - Learn sensor integration and basic features
2. **Data Logger** - Understand data storage and time management
3. **Smart Home** - Explore actuator control and web interfaces
4. **IoT Gateway** - Master advanced communication and bridging

## 🤝 Contributing Examples

Have you built something cool with the template? We'd love to include it!

1. Create a new directory under `examples/`
2. Follow the standard structure
3. Include a comprehensive README
4. Add your example to this list
5. Submit a pull request

## 📖 Additional Resources

- [Template Documentation](../README_TEMPLATE.md)
- [Configuration Guide](../TEMPLATE_CONFIG_GUIDE.md)
- [Hardware Abstraction Layer](../docs/HAL_GUIDE.md)
- [Troubleshooting Guide](../TROUBLESHOOTING.md)

## ❓ Need Help?

- Check the example's README for specific instructions
- Review the main template documentation
- Look at the test files for usage examples
- Open an issue for bugs or questions

Happy building! 🚀