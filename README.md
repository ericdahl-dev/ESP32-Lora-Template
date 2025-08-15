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
git clone https://github.com/ericdahl.dev/ESP32-Lora-Template.git my-iot-device
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
ESP32-Lora-Template/
├── src/                    # Source code
│   ├── hardware/          # Hardware Abstraction Layer
│   │   ├── hardware_abstraction.h
│   │   └── hardware_abstraction.cpp
│   ├── sensors/           # Sensor implementations
│   │   ├── gps_sensor.h/.cpp
│   │   ├── lightning_sensor.h
│   │   └── sensor_interface.h
│   ├── actuators/         # Actuator implementations
│   │   └── actuator_interface.h
│   ├── communication/     # Communication protocols
│   │   └── communication_interface.h
│   ├── system/           # System utilities
│   │   ├── error_handler.h
│   │   ├── logger.h
│   │   └── state_machine.h
│   ├── config/           # Configuration files
│   │   └── system_config.h
│   ├── app_logic.cpp/.h  # Main application logic
│   ├── wifi_manager.cpp/.h # WiFi management
│   └── main.cpp          # Entry point
├── test/                  # Unit tests
├── examples/             # Example implementations
│   └── environmental_monitor/
├── docs/                 # Documentation
│   └── HAL_GUIDE.md
├── web-flasher/          # Browser-based flasher
└── platformio.ini        # Build configuration
```

## 🛠️ Hardware Abstraction Layer (HAL)

The HAL provides a clean interface to hardware functionality:

### GPIO Operations
```cpp
#include "hardware/hardware_abstraction.h"
using namespace HardwareAbstraction;

// Initialize HAL
initialize();

// Configure GPIO
GPIO::pinMode(2, GPIO::Mode::MODE_OUTPUT);
GPIO::digitalWrite(2, GPIO::Level::LEVEL_HIGH);
```

### I2C Communication
```cpp
// Initialize I2C for OLED
I2C::initialize(21, 22, 400000); // SDA, SCL, frequency

// Write to device
uint8_t data[] = {0x00, 0xFF};
I2C::beginTransmission(0x3C);
I2C::write(data, sizeof(data));
I2C::endTransmission();
```

### Power Management
```cpp
// Deep sleep for battery conservation
Power::sleep(Power::Mode::DEEP_SLEEP, 300000); // 5 minutes

// Check battery status
float voltage = Power::getBatteryVoltage();
```

### Memory Management
```cpp
// Get free heap memory
uint32_t free_heap = Memory::getFreeHeap();
uint32_t largest_block = Memory::getLargestFreeBlock();
```

## 🔧 Adding New Features

### Adding a Sensor

1. Create sensor file in `src/sensors/`:
```cpp
// src/sensors/temperature_sensor.h
#pragma once
#include "hardware/hardware_abstraction.h"
#include "sensor_interface.h"

class TemperatureSensor : public SensorInterface {
private:
    uint8_t _pin;
    
public:
    TemperatureSensor(uint8_t pin) : _pin(pin) {}
    void initialize() override;
    SensorData read() override;
    bool isReady() const override;
};
```

2. Implement the sensor:
```cpp
// src/sensors/temperature_sensor.cpp
#include "temperature_sensor.h"
using namespace HardwareAbstraction;

void TemperatureSensor::initialize() {
    GPIO::pinMode(_pin, GPIO::Mode::MODE_INPUT);
}

SensorData TemperatureSensor::read() {
    // Read analog value and convert to temperature
    int rawValue = analogRead(_pin);
    float celsius = (rawValue * 3.3 / 4095.0) * 100;
    
    SensorData data;
    data.value = celsius;
    data.unit = "°C";
    data.timestamp = millis();
    return data;
}
```

3. Use in your application:
```cpp
// src/app_logic.cpp
#include "sensors/temperature_sensor.h"

TemperatureSensor tempSensor(34); // GPIO34

void AppLogic::setup() {
    HardwareAbstraction::initialize();
    tempSensor.initialize();
}

void AppLogic::loop() {
    if (tempSensor.isReady()) {
        SensorData data = tempSensor.read();
        Serial.printf("Temperature: %.2f%s\n", data.value, data.unit.c_str());
    }
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

For more examples and detailed implementations, see the [Examples README](examples/README.md).

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
- [Template Configuration Guide](TEMPLATE_CONFIG_GUIDE.md)
- [WiFi Multi-Network Setup](WIFI_MULTI_NETWORK_README.md)
- [OTA Update Guide](OTA_README.md)
- [Troubleshooting Guide](TROUBLESHOOTING.md)
- [Static Analysis Results](STATIC_ANALYSIS.md)

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
