# Lightning Detection System - PlatformIO Heltec V3

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/Skeyelab/LightningDetector)
[![Tests](https://img.shields.io/badge/tests-112%20passing-brightgreen)](./run_tests.sh)
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
[![Framework](https://img.shields.io/badge/framework-Arduino-red)](https://www.arduino.cc/)

A distributed lightning detection system built for Heltec WiFi LoRa 32 V3 boards with ESP32-S3, featuring modular architecture, comprehensive hardware abstraction, and GPS tracking capabilities.

> **📘 ESP32 Template**: This project's modular architecture makes it an excellent template for general ESP32 development. See [README_TEMPLATE.md](README_TEMPLATE.md) for using this codebase as a starting point for your own ESP32 projects.

## 🚀 Features

### Core Functionality
- **Lightning Detection** - AS3935 sensor integration with noise filtering
- **LoRa Communication** - Long-range data transmission using SX1262 radio
- **GPS Tracking** - UC6580 GNSS support for precise location data
- **OLED Display** - Real-time status and data visualization
- **WiFi Connectivity** - Multi-network support with automatic fallback
- **OTA Updates** - Over-the-air firmware updates via WiFi

### Hardware Abstraction Layer (HAL)
- **Modular Design** - Clean hardware abstraction for easy portability
- **Comprehensive API** - GPIO, I2C, SPI, PWM, ADC, Timer, Power, Memory management
- **Test Coverage** - 51 unit tests ensuring reliability
- **ESP32-S3 Optimized** - Tailored for Heltec WiFi LoRa 32 V3 hardware

### Advanced Features
- **Multi-Board Support** - Sender/receiver architecture
- **Power Management** - Sleep modes and battery monitoring
- **Error Handling** - Robust error detection and recovery
- **Debugging Tools** - Serial logging and diagnostic information
- **Template Ready** - Use as a foundation for any ESP32 project (see [examples](examples/))

## 📋 Hardware Requirements

### Supported Boards
- **[Heltec WiFi LoRa 32 V3](https://heltec.org/project/wifi-lora-32-v3/)** - Main development board
- **[Heltec Wireless Tracker](https://heltec.org/project/wireless-tracker/)** - GPS-enabled transmitter (upcoming)

### Key Specifications
- **MCU**: ESP32-S3 (240MHz, dual-core, 8MB Flash, 512KB SRAM)
- **Radio**: SX1262 LoRa transceiver (915MHz)
- **Display**: 128x64 OLED (SSD1306)
- **GPS**: UC6580 GNSS (L1+L5/L2 dual frequency)
- **Connectivity**: WiFi 802.11 b/g/n

### Sensors & Peripherals
- AS3935 Lightning sensor
- Battery voltage monitoring
- External GPIO expansion
- I2C/SPI device support

## 🛠️ Quick Start

### Prerequisites
```bash
# Install PlatformIO Core
pip install platformio

# Clone the repository
git clone https://github.com/Skeyelab/LightningDetector.git
cd LightningDetector
```

### Build Options

#### Sender (Lightning Detection Node)
```bash
# Build lightning detection transmitter
pio run -e sender

# Upload to device
pio run -e sender -t upload

# Monitor serial output
pio device monitor
```

#### Receiver (Base Station)
```bash
# Build base station with WiFi/OTA
pio run -e receiver

# Upload to device
pio run -e receiver -t upload
```

## 🌐 Web Flasher

### Browser-Based Firmware Flashing
The project includes a web-based ESP32 firmware flasher that allows you to flash devices directly from your browser without installing additional software.

#### Features
- **🌐 Web Interface**: Flash ESP32 devices from any modern browser
- **📱 Device Support**: Transmitter and receiver firmware flashing
- **📁 File Upload**: Support for custom firmware files
- **🔌 Serial Port**: Direct USB connection to ESP32 devices
- **📊 Progress Tracking**: Real-time flashing progress and status updates

#### Quick Start
1. **Visit**: [Web Flasher](https://skeyelab.github.io/LightningDetector/)
2. **Connect**: Plug your ESP32 device via USB
3. **Select**: Choose device type (transmitter or receiver)
4. **Flash**: Click "Connect & Flash" and follow the prompts

#### Requirements
- **Browser**: Chrome or Edge (Web Serial API support)
- **Hardware**: ESP32 device in download mode
- **USB Cable**: Data cable (not just charging cable)

#### Manual Firmware Upload
- Upload custom `.bin` files for transmitter or receiver
- Support for both pre-built and custom firmware
- Automatic file validation and size checking

### Configuration

#### WiFi Networks
```cpp
// Copy and modify wifi_networks_example.h
cp wifi_networks_example.h src/wifi_networks.h
// Edit src/wifi_networks.h with your network credentials
```

#### Hardware Pins
```cpp
// Heltec V3 Pin Configuration (src/config/system_config.h)
#define VEXT_PIN 36        // Power control
#define OLED_RST_PIN 21    // OLED reset
#define BUTTON_PIN 0       // User button
#define LORA_NSS 8         // LoRa chip select
#define LORA_DIO1 14       // LoRa interrupt
```

## 🧪 Testing

### Run All Tests
```bash
# Execute comprehensive test suite
./run_tests.sh
```

### Individual Test Suites
```bash
# Hardware Abstraction Layer tests
pio test -e native -f test_hardware_abstraction

# Integration tests
pio test -e native -f test_integration

# WiFi functionality tests
pio test -e native -f test_wifi_manager
```

### Static Analysis
```bash
# Code quality checks
./run_static_analysis.sh

# Clang-tidy analysis
./run_tidy.sh
```

## 📁 Project Structure

```
├── src/
│   ├── hardware/              # Hardware Abstraction Layer
│   │   ├── hardware_abstraction.h
│   │   └── hardware_abstraction.cpp
│   ├── sensors/               # Sensor implementations
│   │   ├── gps_sensor.h/.cpp
│   │   ├── lightning_sensor.h
│   │   └── sensor_interface.h
│   ├── communication/         # Communication protocols
│   ├── system/               # System management
│   ├── config/               # Configuration files
│   └── main.cpp              # Application entry point
├── test/                     # Unit tests
├── docs/                     # Additional documentation
└── platformio.ini           # PlatformIO configuration
```

## 🔧 Hardware Abstraction Layer

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

### GPS Integration
```cpp
#include "sensors/gps_sensor.h"

// Initialize GPS for Wireless Tracker
GPS::Config config = GPS::getWirelessTrackerV11Config();
GPS::initializeGPS(config);

// Read GPS data
if (GPS::hasGPSFix()) {
    const GPS::Data& data = GPS::getGPSData();
    printf("Position: %.6f, %.6f\n", data.latitude, data.longitude);
}
```

## 📡 Communication Protocol

### LoRa Messaging
```cpp
// Message format
struct LightningMessage {
    uint32_t timestamp;
    float latitude;
    float longitude;
    uint16_t strike_count;
    uint8_t noise_level;
    uint16_t battery_mv;
};
```

### WiFi Multi-Network
- Automatic network selection
- Connection priority management
- Fallback mechanisms
- OTA update support

## 🔋 Power Management

### Sleep Modes
```cpp
// Deep sleep for battery conservation
Power::sleep(Power::Mode::DEEP_SLEEP, 300000); // 5 minutes

// Wake on button press or timer
```

### Battery Monitoring
```cpp
// Check battery status
float voltage = Power::getBatteryVoltage();
uint8_t percent = Power::getBatteryPercent();
```

## 📊 Monitoring & Debugging

### Serial Logging
```cpp
// Debug output levels
#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARN    2
#define LOG_ERROR   3
```

### Performance Metrics
- Memory usage tracking
- LoRa transmission statistics
- GPS fix quality monitoring
- Power consumption analysis

## 🤝 Contributing

### Development Workflow
1. **Fork** the repository
2. **Create** feature branch (`git checkout -b feature/new-sensor`)
3. **Implement** changes with tests
4. **Run** test suite (`./run_tests.sh`)
5. **Submit** pull request

### Code Standards
- Follow existing code style
- Add unit tests for new features
- Update documentation
- Pass static analysis checks

### Issue Tracking
- [Hardware Abstraction Layer](https://github.com/Skeyelab/LightningDetector/issues/7) ✅ **COMPLETE**
- [Modular Architecture](https://github.com/Skeyelab/LightningDetector/issues/14) 🚧 **IN PROGRESS**

## 📚 Documentation

- [WiFi Multi-Network Setup](./WIFI_MULTI_NETWORK_README.md)
- [OTA Update Guide](./OTA_README.md)
- [Troubleshooting Guide](./TROUBLESHOOTING.md)
- [Static Analysis Results](./STATIC_ANALYSIS.md)
- [CI Caching Optimization](./CI_CACHING_OPTIMIZATION.md)
- [Project Status](./CURSOR_PROJECT_STATUS.md)

## 🎯 Roadmap

### Phase 1: Foundation ✅
- [x] Hardware Abstraction Layer
- [x] GPS Integration (UC6580)
- [x] Comprehensive Testing
- [x] Build System Optimization

### Phase 2: Lightning Detection 🚧
- [ ] AS3935 Sensor Integration
- [ ] Noise Filtering Algorithms
- [ ] Calibration Procedures
- [ ] Real-time Processing

### Phase 3: Communication 🔮
- [ ] LoRa Network Protocol
- [ ] Message Encryption
- [ ] Multi-hop Routing
- [ ] Data Aggregation

### Phase 4: Analytics 🔮
- [ ] Storm Tracking
- [ ] Prediction Models
- [ ] Web Dashboard
- [ ] Mobile App

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Heltec Automation** - Hardware platform and documentation
- **Espressif Systems** - ESP32-S3 microcontroller and ESP-IDF
- **RadioLib** - LoRa communication library
- **U8g2** - OLED display library
- **PlatformIO** - Development platform and toolchain

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/Skeyelab/LightningDetector/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Skeyelab/LightningDetector/discussions)
- **Documentation**: [Project Wiki](https://github.com/Skeyelab/LightningDetector/wiki)

---

**Built with ❤️ for the lightning detection community**