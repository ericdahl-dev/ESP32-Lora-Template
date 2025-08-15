# ESP32 Modular Device Template

[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
[![Framework](https://img.shields.io/badge/framework-Arduino-red)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/build-PlatformIO-orange)](https://platformio.org/)
[![HAL Tests](https://img.shields.io/badge/HAL_tests-51_passing-brightgreen)](./run_tests.sh)

A production-ready, modular template for ESP32 projects featuring a comprehensive Hardware Abstraction Layer (HAL), extensive test coverage, and a clean architecture that makes it easy to build any IoT device.

## 🎯 Why Use This Template?

This template provides a solid foundation for ESP32 projects with:

- **Modular Architecture** - Clean separation of concerns with hardware abstraction
- **Production Ready** - Comprehensive error handling, logging, and testing
- **Easy to Extend** - Simple patterns for adding new sensors, actuators, and features
- **Well Tested** - 51+ unit tests covering critical functionality
- **Professional CI/CD** - GitHub Actions with optimized caching
- **OTA Updates** - Built-in over-the-air update capability
- **Web Flasher** - Browser-based firmware upload tool

## 🚀 Quick Start

### 1. Clone and Setup

```bash
# Clone the template
git clone https://github.com/yourusername/esp32-template.git my-iot-device
cd my-iot-device

# Remove the original git history
rm -rf .git
git init

# Install PlatformIO
pip install platformio
```

### 2. Configure Your Hardware

Edit `platformio.ini` to match your board:

```ini
[env:my_device]
platform = espressif32
board = heltec_wifi_lora_32_V3  ; Change to your board
framework = arduino
monitor_speed = 115200
```

### 3. Implement Your Logic

Replace the example application logic in `src/app_logic.cpp`:

```cpp
#include "app_logic.h"
#include "hardware/hardware_abstraction.h"

void AppLogic::setup() {
    // Initialize your sensors/actuators
    HAL::GPIO::pinMode(LED_PIN, OUTPUT);
    
    // Setup your custom hardware
    myServo.attach(SERVO_PIN);
    mySensor.begin();
}

void AppLogic::loop() {
    // Your main application logic
    float reading = mySensor.read();
    
    if (reading > THRESHOLD) {
        HAL::GPIO::digitalWrite(LED_PIN, HIGH);
        myServo.write(90);
    }
}
```

## 📁 Project Structure

```
esp32-template/
├── src/                    # Source code
│   ├── hardware/          # Hardware Abstraction Layer
│   │   ├── hardware_abstraction.h
│   │   └── hardware_abstraction.cpp
│   ├── sensors/           # Sensor implementations
│   │   └── [your_sensors_here]
│   ├── actuators/         # Actuator implementations
│   │   └── [your_actuators_here]
│   ├── communication/     # WiFi, LoRa, Bluetooth modules
│   ├── system/           # System utilities
│   ├── config/           # Configuration files
│   ├── app_logic.cpp     # Main application logic
│   └── main.cpp          # Entry point
├── test/                  # Unit tests
├── examples/             # Example implementations
├── web-flasher/          # Browser-based flasher
└── platformio.ini        # Build configuration
```

## 🛠️ Hardware Abstraction Layer (HAL)

The HAL provides a clean interface to hardware functionality:

### GPIO Operations
```cpp
HAL::GPIO::pinMode(pin, mode);
HAL::GPIO::digitalWrite(pin, value);
int value = HAL::GPIO::digitalRead(pin);
```

### I2C Communication
```cpp
HAL::I2C::begin(sda, scl);
HAL::I2C::write(address, data, length);
HAL::I2C::read(address, buffer, length);
```

### SPI Communication
```cpp
HAL::SPI::begin(mosi, miso, sck, cs);
HAL::SPI::transfer(data);
```

### Timer Operations
```cpp
HAL::Timer::attachInterrupt(timer, callback, interval_us);
HAL::Timer::detachInterrupt(timer);
```

### Power Management
```cpp
HAL::Power::sleep(duration_ms);
float voltage = HAL::Power::getBatteryVoltage();
```

### Memory Management
```cpp
uint32_t free = HAL::Memory::getFreeHeap();
uint32_t largest = HAL::Memory::getLargestFreeBlock();
```

## 🔧 Adding New Features

### Adding a Sensor

1. Create sensor file in `src/sensors/`:
```cpp
// src/sensors/temperature_sensor.h
#pragma once
#include "hardware/hardware_abstraction.h"

class TemperatureSensor {
private:
    uint8_t _pin;
    
public:
    TemperatureSensor(uint8_t pin) : _pin(pin) {}
    void begin();
    float readCelsius();
};
```

2. Implement the sensor:
```cpp
// src/sensors/temperature_sensor.cpp
#include "temperature_sensor.h"

void TemperatureSensor::begin() {
    HAL::GPIO::pinMode(_pin, INPUT);
}

float TemperatureSensor::readCelsius() {
    int rawValue = HAL::ADC::read(_pin);
    return (rawValue * 3.3 / 4095.0) * 100; // Example conversion
}
```

3. Use in your application:
```cpp
// src/app_logic.cpp
#include "sensors/temperature_sensor.h"

TemperatureSensor tempSensor(34); // GPIO34

void AppLogic::setup() {
    tempSensor.begin();
}

void AppLogic::loop() {
    float temp = tempSensor.readCelsius();
    Serial.printf("Temperature: %.2f°C\n", temp);
}
```

### Adding an Actuator

Similar pattern for actuators in `src/actuators/`.

## 📱 WiFi Configuration

The template includes multi-network WiFi support:

```cpp
// wifi_networks_example.h
const char* WIFI_SSIDS[] = {"Network1", "Network2", "Network3"};
const char* WIFI_PASSWORDS[] = {"password1", "password2", "password3"};
const int WIFI_NETWORK_COUNT = 3;
```

## 🔄 OTA Updates

Enable over-the-air updates:

```cpp
// In your setup()
WiFiManager::enableOTA("device-name", "ota-password");
```

## 🧪 Testing

Run the comprehensive test suite:

```bash
./run_tests.sh
```

Add your own tests in `test/`:
```cpp
// test/test_my_sensor.cpp
#include <unity.h>
#include "sensors/my_sensor.h"

void test_sensor_initialization() {
    MySensor sensor(GPIO_NUM_34);
    sensor.begin();
    TEST_ASSERT_TRUE(sensor.isReady());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_sensor_initialization);
    UNITY_END();
}
```

## 📦 Example Projects

The `examples/` directory contains ready-to-use implementations:

### Environmental Monitor
```
examples/environmental_monitor/
├── README.md
├── platformio.ini
└── src/
    └── main.cpp  # Temperature, humidity, air quality monitoring
```

### Smart Home Controller
```
examples/smart_home/
├── README.md
├── platformio.ini
└── src/
    └── main.cpp  # Relay control, sensor integration, MQTT
```

### Data Logger
```
examples/data_logger/
├── README.md
├── platformio.ini
└── src/
    └── main.cpp  # SD card logging, RTC, sensor data
```

## 🌐 Web Flasher

Flash firmware directly from a web browser:

1. Build the flasher:
```bash
cd web-flasher
npm install
npm run build
```

2. Serve locally:
```bash
npm start
```

3. Or deploy to GitHub Pages for easy distribution.

## 🔌 Supported Hardware

### Tested Boards
- Heltec WiFi LoRa 32 V3
- ESP32-S3 DevKit
- ESP32-WROOM-32
- Add your board here!

### Common Peripherals
- OLED Displays (SSD1306, SH1106)
- LoRa Modules (SX1262, SX1276)
- GPS Modules (NEO-6M, NEO-8M)
- Various I2C/SPI sensors

## 📚 Documentation

- [Hardware Abstraction Layer Guide](docs/HAL_GUIDE.md)
- [Adding Custom Sensors](docs/CUSTOM_SENSORS.md)
- [Network Configuration](docs/NETWORK_CONFIG.md)
- [Power Management](docs/POWER_MANAGEMENT.md)
- [Troubleshooting](TROUBLESHOOTING.md)

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## 📄 License

This template is open source and available under the MIT License.

## 🙏 Acknowledgments

This template was originally based on a lightning detection system but has been generalized for broader ESP32 development use cases.