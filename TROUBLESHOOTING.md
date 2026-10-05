# ESP32 Template Troubleshooting Guide

## Common Issues and Solutions

This guide helps resolve common issues when using the ESP32 template for your projects.

### Hardware Abstraction Layer Issues
- **GPIO Configuration**: Ensure proper pin modes and pull-up/pull-down settings
- **I2C Communication**: Check SDA/SCL pin assignments and device addresses
- **SPI Communication**: Verify MOSI/MISO/SCK/CS pin connections
- **Power Management**: Ensure proper voltage levels and power sequencing

## Button Interface Troubleshooting

### Issue: Button Not Responding
**Symptoms**: Button presses don't trigger expected actions
**Possible Causes**:
- Button pin not properly configured
- Pull-up resistor not enabled
- Main loop delays blocking button detection

**Solutions**:
1. **Check Button Configuration using HAL**:
```cpp
using namespace HardwareAbstraction;
GPIO::pinMode(BUTTON_PIN, GPIO::Mode::MODE_INPUT_PULLUP);
```

2. **Verify Button Pin Assignment**:
```cpp
#define BUTTON_PIN 0  // GPIO0 (BOOT button) or your chosen pin
```

3. **Use Non-blocking Code**:
```cpp
// WRONG - blocks button detection
delay(2000);

// CORRECT - non-blocking timing
if (millis() - lastUpdate >= 2000) {
  // do something
  lastUpdate = millis();
}
```

### Issue: Button Too Sensitive/Not Sensitive Enough
**Symptoms**: Button triggers on slight touch or requires very long press
**Solutions**:
1. **Implement Debouncing**:
```cpp
class Button {
private:
    uint8_t pin;
    unsigned long lastPress = 0;
    const unsigned long debounceDelay = 50;
public:
    bool isPressed() {
        if (GPIO::digitalRead(pin) == LOW &&
            millis() - lastPress > debounceDelay) {
            lastPress = millis();
            return true;
        }
        return false;
    }
};
```

2. **Adjust Sensitivity**:
```cpp
const unsigned long SHORT_PRESS = 100;
const unsigned long LONG_PRESS = 1000;

if (pressDuration > SHORT_PRESS && pressDuration < LONG_PRESS) {
    // Short press action
} else if (pressDuration >= LONG_PRESS) {
    // Long press action
}
```

## Communication Issues

### Issue: WiFi Connection Problems
**Symptoms**: Device cannot connect to WiFi network
**Root Cause**: Network configuration or signal issues

**Solutions**:
1. **Check Network Configuration**:
```cpp
// Verify WiFi credentials in wifi_networks.h
const char* WIFI_SSIDS[] = {"YourNetwork"};
const char* WIFI_PASSWORDS[] = {"YourPassword"};
```

2. **Debug Connection Process**:
```cpp
Serial.printf("Connecting to %s...\n", WIFI_SSID);
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
Serial.println("Connected!");
```

3. **Check Signal Strength**:
```cpp
int rssi = WiFi.RSSI();
Serial.printf("Signal strength: %d dBm\n", rssi);
```

### Issue: Sensor Readings Inconsistent
**Symptoms**: Sensor values fluctuate wildly or return invalid data
**Possible Causes**:
- Improper sensor initialization
- Power supply issues
- Timing problems

**Solutions**:
1. **Proper Sensor Initialization**:
```cpp
void TemperatureSensor::initialize() {
    GPIO::pinMode(sensorPin, GPIO::Mode::MODE_INPUT);
    delay(100);  // Allow sensor to stabilize
}
```

2. **Add Averaging for Stability**:
```cpp
float TemperatureSensor::readAverage(int samples) {
    float sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += read();
        delay(10);
    }
    return sum / samples;
}
```

3. **Check Power Supply**:
```cpp
float voltage = Power::getBatteryVoltage();
if (voltage < 3.0) {
    Serial.println("Warning: Low battery voltage!");
}
```

## Display Issues

### Issue: OLED Display Not Working
**Symptoms**: Display remains blank or shows garbage
**Solutions**:

**Check I2C Configuration using HAL**:
```cpp
using namespace HardwareAbstraction;

// Initialize I2C
I2C::initialize(SDA_PIN, SCL_PIN, 400000);

// Test I2C communication
uint8_t address = 0x3C;  // Common OLED address
if (I2C::beginTransmission(address) && I2C::endTransmission()) {
    Serial.println("OLED found on I2C bus");
} else {
    Serial.println("OLED not responding");
}
```

**Verify Power Management**:
```cpp
// Ensure display power is enabled
GPIO::pinMode(VEXT_PIN, GPIO::Mode::MODE_OUTPUT);
GPIO::digitalWrite(VEXT_PIN, GPIO::Level::LEVEL_LOW);  // Enable power
delay(100);
```

### Issue: Display Shows Garbled Text
**Symptoms**: Text appears but is unreadable or corrupted
**Solutions**:
1. **Check Display Initialization**:
```cpp
class OLEDDisplay {
public:
    void initialize() {
        I2C::initialize(SDA_PIN, SCL_PIN);
        // Send initialization sequence
        sendCommand(0xAE);  // Display off
        sendCommand(0xD5);  // Set clock
        // ... other init commands
        sendCommand(0xAF);  // Display on
    }

    void clear() {
        // Clear display buffer
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = 0;
        }
        sendBuffer();
    }
};
```

## Build and Configuration Issues

### Check Board Definition
Ensure the correct board is selected:
```ini
[env:my_device]
board = esp32dev  # Or your specific board
platform = espressif32
framework = arduino
```

### Check Dependencies
Template working configuration:
```ini
lib_deps =
    SPI
    Wire
    # Add libraries as needed for your project
    # adafruit/Adafruit SSD1306  ; For OLED displays
    # bblanchon/ArduinoJson     ; For JSON parsing
```

### Build Flags
Configure features for your project:
```ini
build_flags =
    -D ENABLE_WIFI=1
    -D ENABLE_OTA=1
    -D DEBUG_LEVEL=2
    -D DEVICE_NAME="\"MyDevice\""
    # Add your specific pin definitions
    -D SDA_PIN=21
    -D SCL_PIN=22
```

## Hardware Checks

### USB Cable and Connection
- Use high-quality data cable (not power-only)
- Try different USB ports
- Check if board shows up in device manager
- Verify drivers are installed

### Power Supply
- Ensure stable power supply (3.3V for most sensors)
- Check for voltage drops during high current operations
- Monitor battery levels if using battery power
- Verify USB power is sufficient for your peripherals

### Pin Connections
- Double-check pin assignments in your configuration
- Verify no pin conflicts between different peripherals
- Ensure proper pull-up/pull-down resistors where needed
- Check for short circuits or loose connections
- Use HAL pin definitions for consistency:
```cpp
// Define pins in a central location
namespace Pins {
    const uint8_t LED = 2;
    const uint8_t BUTTON = 0;
    const uint8_t SDA = 21;
    const uint8_t SCL = 22;
}
```

## Debug Steps

### Step 1: Verify Basic Operation
Check that the system boots and initializes properly:
1. Upload firmware
2. Open serial monitor (115200 baud)
3. Verify boot messages and HAL initialization
4. Check all peripherals are detected

### Step 2: Test Hardware Abstraction Layer
1. **GPIO Test**: Toggle LEDs or read button states
2. **I2C Test**: Scan for devices and test communication
3. **SPI Test**: Verify SPI peripherals respond
4. **Power Test**: Check voltage levels and power management

### Step 3: Test Application Logic
1. Verify sensors read valid data
2. Test actuators respond correctly
3. Check communication protocols work
4. Validate data logging and storage

### Step 4: Integration Testing
1. Test multiple systems working together
2. Verify no conflicts between different components
3. Check system performance under load
4. Test error handling and recovery

## Common Error Codes

### HAL Errors
- `HAL_ERROR_INVALID_PIN`: Pin number out of range or invalid
- `HAL_ERROR_INIT_FAILED`: Hardware initialization failed
- `HAL_ERROR_TIMEOUT`: Operation timed out
- `HAL_ERROR_NO_DEVICE`: Device not found on bus

### WiFi Errors
- `WL_NO_SSID_AVAIL`: Network not found
- `WL_CONNECT_FAILED`: Authentication failed
- `WL_CONNECTION_LOST`: Connection dropped

### General ESP32 Errors
- Boot loops: Check for memory issues or infinite loops
- Brownout: Insufficient power supply
- Guru meditation: Stack overflow or memory corruption

## Getting Help

1. **Check Serial Output**: Most issues show error messages in serial monitor
2. **Review Documentation**: Check the specific guides for your components
3. **Test Incrementally**: Add one feature at a time to isolate issues
4. **Use HAL Debug**: Enable HAL debugging for detailed hardware status

```cpp
#define HAL_DEBUG_LEVEL 3  // Enable verbose HAL debugging
```

## Resources
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Arduino ESP32 Core](https://github.com/espressif/arduino-esp32)
- [PlatformIO ESP32](https://docs.platformio.org/en/latest/platforms/espressif32.html)
- [Hardware Abstraction Layer Guide](docs/HAL_GUIDE.md)
- [Template Configuration Guide](TEMPLATE_CONFIG_GUIDE.md)

---
*This troubleshooting guide covers common issues when using the ESP32 Modular Device Template*
*For project-specific issues, check the documentation in your chosen example directory*
