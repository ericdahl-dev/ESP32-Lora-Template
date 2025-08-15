# Hardware Abstraction Layer (HAL) Guide

The Hardware Abstraction Layer provides a unified interface for hardware operations, making your code portable across different ESP32 boards and configurations.

## Table of Contents

1. [Overview](#overview)
2. [GPIO Operations](#gpio-operations)
3. [I2C Communication](#i2c-communication)
4. [SPI Communication](#spi-communication)
5. [ADC (Analog Input)](#adc-analog-input)
6. [PWM Output](#pwm-output)
7. [Timer Operations](#timer-operations)
8. [Power Management](#power-management)
9. [Memory Management](#memory-management)
10. [Sensor Integration Examples](#sensor-integration-examples)
11. [Actuator Integration Examples](#actuator-integration-examples)
12. [Best Practices](#best-practices)

## Overview

The HAL provides consistent APIs for:
- Digital I/O operations
- Communication protocols (I2C, SPI, UART)
- Analog operations (ADC, PWM)
- System functions (timers, interrupts, power, memory)

### Basic Usage Pattern

```cpp
#include "hardware/hardware_abstraction.h"

void setup() {
    // Initialize HAL
    HAL::init();
    
    // Use HAL functions
    HAL::GPIO::pinMode(LED_PIN, OUTPUT);
}
```

## GPIO Operations

### Basic Digital I/O

```cpp
// Set pin mode
HAL::GPIO::pinMode(pin, mode);  // mode: INPUT, OUTPUT, INPUT_PULLUP

// Digital write
HAL::GPIO::digitalWrite(pin, value);  // value: HIGH or LOW

// Digital read
int value = HAL::GPIO::digitalRead(pin);

// Toggle pin
HAL::GPIO::toggle(pin);
```

### Example: LED Control

```cpp
const int LED_PIN = 2;

void setup() {
    HAL::init();
    HAL::GPIO::pinMode(LED_PIN, OUTPUT);
}

void loop() {
    HAL::GPIO::digitalWrite(LED_PIN, HIGH);
    delay(1000);
    HAL::GPIO::digitalWrite(LED_PIN, LOW);
    delay(1000);
}
```

### Example: Button Input

```cpp
const int BUTTON_PIN = 0;
const int LED_PIN = 2;

void setup() {
    HAL::init();
    HAL::GPIO::pinMode(BUTTON_PIN, INPUT_PULLUP);
    HAL::GPIO::pinMode(LED_PIN, OUTPUT);
}

void loop() {
    if (HAL::GPIO::digitalRead(BUTTON_PIN) == LOW) {
        HAL::GPIO::digitalWrite(LED_PIN, HIGH);
    } else {
        HAL::GPIO::digitalWrite(LED_PIN, LOW);
    }
}
```

## I2C Communication

### Initialization

```cpp
// Initialize I2C with default pins
HAL::I2C::begin();

// Or specify custom pins
HAL::I2C::begin(sda_pin, scl_pin);

// Set frequency (optional)
HAL::I2C::setFrequency(400000);  // 400kHz
```

### Basic Operations

```cpp
// Write data to device
uint8_t data[] = {0x01, 0x02, 0x03};
bool success = HAL::I2C::write(device_address, data, sizeof(data));

// Read data from device
uint8_t buffer[10];
bool success = HAL::I2C::read(device_address, buffer, sizeof(buffer));

// Write then read (common pattern)
uint8_t reg = 0x00;
HAL::I2C::write(device_address, &reg, 1);
HAL::I2C::read(device_address, buffer, 4);
```

### Example: I2C Temperature Sensor (LM75)

```cpp
class LM75Sensor {
private:
    static const uint8_t LM75_ADDR = 0x48;
    static const uint8_t TEMP_REG = 0x00;
    
public:
    void begin() {
        HAL::I2C::begin();
    }
    
    float readTemperature() {
        uint8_t reg = TEMP_REG;
        uint8_t data[2];
        
        // Write register address
        HAL::I2C::write(LM75_ADDR, &reg, 1);
        
        // Read temperature data
        HAL::I2C::read(LM75_ADDR, data, 2);
        
        // Convert to temperature
        int16_t raw = (data[0] << 8) | data[1];
        return raw / 256.0;
    }
};
```

### Example: OLED Display (SSD1306)

```cpp
class OLEDDisplay {
private:
    static const uint8_t OLED_ADDR = 0x3C;
    static const uint8_t COMMAND_MODE = 0x00;
    static const uint8_t DATA_MODE = 0x40;
    
public:
    void begin() {
        HAL::I2C::begin();
        
        // Send initialization sequence
        uint8_t init_cmds[] = {
            COMMAND_MODE, 0xAE,  // Display off
            COMMAND_MODE, 0xD5,  // Set display clock
            COMMAND_MODE, 0x80,  // Clock value
            COMMAND_MODE, 0xA8,  // Set multiplex
            COMMAND_MODE, 0x3F,  // 64 lines
            COMMAND_MODE, 0x20,  // Memory mode
            COMMAND_MODE, 0x00,  // Horizontal addressing
            COMMAND_MODE, 0xAF   // Display on
        };
        
        HAL::I2C::write(OLED_ADDR, init_cmds, sizeof(init_cmds));
    }
    
    void writePixels(uint8_t* data, size_t len) {
        uint8_t cmd = DATA_MODE;
        HAL::I2C::write(OLED_ADDR, &cmd, 1);
        HAL::I2C::write(OLED_ADDR, data, len);
    }
};
```

## SPI Communication

### Initialization

```cpp
// Initialize with default pins
HAL::SPI::begin();

// Or specify custom pins
HAL::SPI::begin(mosi_pin, miso_pin, sck_pin, cs_pin);

// Configure SPI settings
HAL::SPI::setFrequency(1000000);     // 1MHz
HAL::SPI::setDataMode(SPI_MODE0);    // CPOL=0, CPHA=0
HAL::SPI::setBitOrder(MSBFIRST);
```

### Basic Operations

```cpp
// Single byte transfer
uint8_t response = HAL::SPI::transfer(0x42);

// Multi-byte transfer
uint8_t tx_data[] = {0x01, 0x02, 0x03};
uint8_t rx_data[3];
HAL::SPI::transfer(tx_data, rx_data, 3);

// Chip select management
HAL::SPI::beginTransaction();
// ... SPI operations ...
HAL::SPI::endTransaction();
```

### Example: SPI Flash Memory

```cpp
class SPIFlash {
private:
    static const uint8_t CMD_READ_ID = 0x9F;
    static const uint8_t CMD_READ = 0x03;
    static const uint8_t CMD_WRITE_ENABLE = 0x06;
    static const uint8_t CMD_PAGE_PROGRAM = 0x02;
    
public:
    void begin() {
        HAL::SPI::begin();
        HAL::SPI::setFrequency(10000000);  // 10MHz
    }
    
    uint32_t readID() {
        HAL::SPI::beginTransaction();
        
        HAL::SPI::transfer(CMD_READ_ID);
        uint32_t id = HAL::SPI::transfer(0) << 16;
        id |= HAL::SPI::transfer(0) << 8;
        id |= HAL::SPI::transfer(0);
        
        HAL::SPI::endTransaction();
        return id;
    }
    
    void readData(uint32_t address, uint8_t* buffer, size_t len) {
        HAL::SPI::beginTransaction();
        
        HAL::SPI::transfer(CMD_READ);
        HAL::SPI::transfer((address >> 16) & 0xFF);
        HAL::SPI::transfer((address >> 8) & 0xFF);
        HAL::SPI::transfer(address & 0xFF);
        
        for (size_t i = 0; i < len; i++) {
            buffer[i] = HAL::SPI::transfer(0);
        }
        
        HAL::SPI::endTransaction();
    }
};
```

## ADC (Analog Input)

### Basic ADC Reading

```cpp
// Read analog value (0-4095 for 12-bit ADC)
int raw_value = HAL::ADC::read(pin);

// Read voltage (0-3.3V)
float voltage = HAL::ADC::readVoltage(pin);

// Configure ADC resolution
HAL::ADC::setResolution(12);  // 12-bit (default)

// Configure attenuation for different voltage ranges
HAL::ADC::setAttenuation(pin, ADC_ATTEN_DB_11);  // 0-3.3V range
```

### Example: Analog Sensor Reading

```cpp
class AnalogSensor {
private:
    uint8_t _pin;
    float _scale_factor;
    
public:
    AnalogSensor(uint8_t pin, float scale) 
        : _pin(pin), _scale_factor(scale) {}
    
    void begin() {
        HAL::ADC::setAttenuation(_pin, ADC_ATTEN_DB_11);
    }
    
    float read() {
        float voltage = HAL::ADC::readVoltage(_pin);
        return voltage * _scale_factor;
    }
    
    float readAverage(int samples = 10) {
        float sum = 0;
        for (int i = 0; i < samples; i++) {
            sum += read();
            delay(10);
        }
        return sum / samples;
    }
};

// Usage: Light sensor
AnalogSensor lightSensor(34, 100.0);  // Scale to lux
```

### Example: Battery Voltage Monitor

```cpp
class BatteryMonitor {
private:
    uint8_t _pin;
    float _divider_ratio;
    
public:
    BatteryMonitor(uint8_t pin, float ratio) 
        : _pin(pin), _divider_ratio(ratio) {}
    
    void begin() {
        HAL::ADC::setAttenuation(_pin, ADC_ATTEN_DB_11);
    }
    
    float getVoltage() {
        float adc_voltage = HAL::ADC::readVoltage(_pin);
        return adc_voltage * _divider_ratio;
    }
    
    int getPercentage() {
        float voltage = getVoltage();
        // LiPo battery: 4.2V = 100%, 3.0V = 0%
        int percent = (voltage - 3.0) * 100 / (4.2 - 3.0);
        return constrain(percent, 0, 100);
    }
};
```

## PWM Output

### Basic PWM Control

```cpp
// Setup PWM channel
HAL::PWM::setupChannel(channel, frequency, resolution);

// Attach pin to channel
HAL::PWM::attachPin(pin, channel);

// Set duty cycle (0-255 for 8-bit)
HAL::PWM::setDutyCycle(channel, duty);

// Or use percentage (0-100)
HAL::PWM::setDutyCyclePercent(channel, 50);  // 50%
```

### Example: LED Dimming

```cpp
class DimmableLED {
private:
    uint8_t _pin;
    uint8_t _channel;
    
public:
    DimmableLED(uint8_t pin, uint8_t channel) 
        : _pin(pin), _channel(channel) {}
    
    void begin() {
        HAL::PWM::setupChannel(_channel, 5000, 8);  // 5kHz, 8-bit
        HAL::PWM::attachPin(_pin, _channel);
    }
    
    void setBrightness(uint8_t brightness) {
        HAL::PWM::setDutyCycle(_channel, brightness);
    }
    
    void fadeIn(int duration_ms) {
        for (int i = 0; i <= 255; i++) {
            setBrightness(i);
            delay(duration_ms / 255);
        }
    }
    
    void fadeOut(int duration_ms) {
        for (int i = 255; i >= 0; i--) {
            setBrightness(i);
            delay(duration_ms / 255);
        }
    }
};
```

### Example: Servo Control

```cpp
class ServoMotor {
private:
    uint8_t _pin;
    uint8_t _channel;
    int _min_us = 1000;  // 1ms
    int _max_us = 2000;  // 2ms
    
public:
    ServoMotor(uint8_t pin, uint8_t channel) 
        : _pin(pin), _channel(channel) {}
    
    void begin() {
        HAL::PWM::setupChannel(_channel, 50, 16);  // 50Hz, 16-bit
        HAL::PWM::attachPin(_pin, _channel);
    }
    
    void setAngle(int angle) {
        angle = constrain(angle, 0, 180);
        int pulse_us = map(angle, 0, 180, _min_us, _max_us);
        
        // Convert to duty cycle (50Hz = 20ms period)
        int duty = (pulse_us * 65535) / 20000;
        HAL::PWM::setDutyCycle(_channel, duty);
    }
    
    void sweep(int start_angle, int end_angle, int duration_ms) {
        int steps = abs(end_angle - start_angle);
        int direction = (end_angle > start_angle) ? 1 : -1;
        
        for (int i = 0; i <= steps; i++) {
            setAngle(start_angle + (i * direction));
            delay(duration_ms / steps);
        }
    }
};
```

## Timer Operations

### Basic Timer Usage

```cpp
// Attach interrupt to timer
HAL::Timer::attachInterrupt(timer_num, callback, interval_us);

// Detach interrupt
HAL::Timer::detachInterrupt(timer_num);

// Get microseconds since boot
uint64_t micros = HAL::Timer::getMicros();

// Get milliseconds since boot
uint32_t millis = HAL::Timer::getMillis();
```

### Example: Periodic Task

```cpp
volatile bool task_flag = false;

void IRAM_ATTR onTimer() {
    task_flag = true;
}

void setup() {
    HAL::init();
    
    // Setup timer for 1 second interval
    HAL::Timer::attachInterrupt(0, onTimer, 1000000);
}

void loop() {
    if (task_flag) {
        task_flag = false;
        
        // Perform periodic task
        Serial.println("Timer triggered!");
        readSensors();
        updateDisplay();
    }
}
```

### Example: Precise Timing Measurement

```cpp
class TimingMeasurement {
private:
    uint64_t _start_time;
    
public:
    void start() {
        _start_time = HAL::Timer::getMicros();
    }
    
    uint64_t elapsed() {
        return HAL::Timer::getMicros() - _start_time;
    }
    
    void measurePulse(uint8_t pin) {
        // Wait for pulse start
        while (HAL::GPIO::digitalRead(pin) == LOW);
        start();
        
        // Wait for pulse end
        while (HAL::GPIO::digitalRead(pin) == HIGH);
        
        uint64_t duration = elapsed();
        Serial.printf("Pulse duration: %llu us\n", duration);
    }
};
```

## Power Management

### Sleep Modes

```cpp
// Light sleep (WiFi/BT maintained)
HAL::Power::lightSleep(duration_ms);

// Deep sleep (lowest power, restarts on wake)
HAL::Power::deepSleep(duration_ms);

// Deep sleep with wake on GPIO
HAL::Power::deepSleepWithWakePin(pin, level, duration_ms);

// Get battery voltage
float voltage = HAL::Power::getBatteryVoltage();

// Check if woke from deep sleep
if (HAL::Power::getWakeupReason() == WAKEUP_REASON_TIMER) {
    Serial.println("Woke from timer");
}
```

### Example: Battery-Powered Sensor

```cpp
class BatteryPoweredSensor {
private:
    const int SLEEP_DURATION = 5 * 60 * 1000;  // 5 minutes
    const float LOW_BATTERY = 3.3;
    
public:
    void setup() {
        HAL::init();
        
        // Check battery on wake
        float battery = HAL::Power::getBatteryVoltage();
        if (battery < LOW_BATTERY) {
            // Blink LED and go back to sleep
            blinkError();
            HAL::Power::deepSleep(SLEEP_DURATION * 2);
        }
        
        // Normal operation
        readAndTransmit();
        
        // Go to sleep
        HAL::Power::deepSleep(SLEEP_DURATION);
    }
    
    void readAndTransmit() {
        // Read sensors
        float temp = readTemperature();
        float humidity = readHumidity();
        
        // Transmit data
        transmitData(temp, humidity);
    }
};
```

## Memory Management

### Memory Monitoring

```cpp
// Get free heap memory
uint32_t free_heap = HAL::Memory::getFreeHeap();

// Get largest free block
uint32_t largest_block = HAL::Memory::getLargestFreeBlock();

// Get heap fragmentation
float fragmentation = HAL::Memory::getFragmentation();

// Print memory stats
HAL::Memory::printStats();
```

### Example: Memory-Aware Buffer

```cpp
class DynamicBuffer {
private:
    uint8_t* _buffer = nullptr;
    size_t _size = 0;
    
public:
    bool allocate(size_t requested_size) {
        // Check available memory
        uint32_t free_heap = HAL::Memory::getFreeHeap();
        
        // Leave at least 10KB free
        if (free_heap - requested_size < 10240) {
            Serial.println("Not enough memory!");
            return false;
        }
        
        _buffer = (uint8_t*)malloc(requested_size);
        if (_buffer) {
            _size = requested_size;
            return true;
        }
        
        return false;
    }
    
    void free() {
        if (_buffer) {
            ::free(_buffer);
            _buffer = nullptr;
            _size = 0;
        }
    }
    
    ~DynamicBuffer() {
        free();
    }
};
```

## Sensor Integration Examples

### DHT Temperature/Humidity Sensor

```cpp
class DHTSensor {
private:
    uint8_t _pin;
    uint8_t _type;  // DHT11, DHT22
    
public:
    DHTSensor(uint8_t pin, uint8_t type) 
        : _pin(pin), _type(type) {}
    
    void begin() {
        HAL::GPIO::pinMode(_pin, INPUT_PULLUP);
    }
    
    bool read(float& temperature, float& humidity) {
        // DHT protocol implementation using HAL
        // ... (simplified for brevity)
        return true;
    }
};
```

### Ultrasonic Distance Sensor

```cpp
class UltrasonicSensor {
private:
    uint8_t _trigger_pin;
    uint8_t _echo_pin;
    
public:
    UltrasonicSensor(uint8_t trig, uint8_t echo) 
        : _trigger_pin(trig), _echo_pin(echo) {}
    
    void begin() {
        HAL::GPIO::pinMode(_trigger_pin, OUTPUT);
        HAL::GPIO::pinMode(_echo_pin, INPUT);
    }
    
    float readDistance() {
        // Send trigger pulse
        HAL::GPIO::digitalWrite(_trigger_pin, LOW);
        delayMicroseconds(2);
        HAL::GPIO::digitalWrite(_trigger_pin, HIGH);
        delayMicroseconds(10);
        HAL::GPIO::digitalWrite(_trigger_pin, LOW);
        
        // Measure echo pulse
        uint64_t start = HAL::Timer::getMicros();
        while (HAL::GPIO::digitalRead(_echo_pin) == LOW) {
            if (HAL::Timer::getMicros() - start > 30000) {
                return -1;  // Timeout
            }
        }
        
        start = HAL::Timer::getMicros();
        while (HAL::GPIO::digitalRead(_echo_pin) == HIGH) {
            if (HAL::Timer::getMicros() - start > 30000) {
                return -1;  // Timeout
            }
        }
        
        uint64_t duration = HAL::Timer::getMicros() - start;
        
        // Calculate distance (speed of sound = 343 m/s)
        return (duration * 0.0343) / 2;
    }
};
```

## Actuator Integration Examples

### Relay Control

```cpp
class RelayController {
private:
    uint8_t _pins[4];
    uint8_t _count;
    bool _active_low;
    
public:
    RelayController(uint8_t* pins, uint8_t count, bool active_low = false) 
        : _count(count), _active_low(active_low) {
        for (int i = 0; i < count; i++) {
            _pins[i] = pins[i];
        }
    }
    
    void begin() {
        for (int i = 0; i < _count; i++) {
            HAL::GPIO::pinMode(_pins[i], OUTPUT);
            setRelay(i, false);  // All off initially
        }
    }
    
    void setRelay(uint8_t index, bool state) {
        if (index >= _count) return;
        
        bool pin_state = _active_low ? !state : state;
        HAL::GPIO::digitalWrite(_pins[index], pin_state);
    }
    
    void toggleRelay(uint8_t index) {
        if (index >= _count) return;
        
        HAL::GPIO::toggle(_pins[index]);
    }
    
    void allOn() {
        for (int i = 0; i < _count; i++) {
            setRelay(i, true);
        }
    }
    
    void allOff() {
        for (int i = 0; i < _count; i++) {
            setRelay(i, false);
        }
    }
};
```

### Stepper Motor Control

```cpp
class StepperMotor {
private:
    uint8_t _pins[4];
    int _step_delay = 2;
    int _current_step = 0;
    
    const uint8_t STEP_SEQUENCE[8][4] = {
        {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
        {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1}
    };
    
public:
    StepperMotor(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4) {
        _pins[0] = pin1; _pins[1] = pin2;
        _pins[2] = pin3; _pins[3] = pin4;
    }
    
    void begin() {
        for (int i = 0; i < 4; i++) {
            HAL::GPIO::pinMode(_pins[i], OUTPUT);
            HAL::GPIO::digitalWrite(_pins[i], LOW);
        }
    }
    
    void setSpeed(int rpm) {
        // 4096 steps per revolution for 28BYJ-48
        _step_delay = 60000000L / (4096L * rpm);
    }
    
    void step(int steps) {
        int direction = (steps > 0) ? 1 : -1;
        steps = abs(steps);
        
        for (int i = 0; i < steps; i++) {
            _current_step = (_current_step + direction + 8) % 8;
            
            for (int pin = 0; pin < 4; pin++) {
                HAL::GPIO::digitalWrite(_pins[pin], 
                    STEP_SEQUENCE[_current_step][pin]);
            }
            
            delayMicroseconds(_step_delay);
        }
    }
    
    void rotate(float degrees) {
        // 4096 steps = 360 degrees
        int steps = (degrees * 4096) / 360;
        step(steps);
    }
};
```

## Best Practices

### 1. Error Handling

```cpp
class I2CSensor {
    bool readRegister(uint8_t reg, uint8_t& value) {
        if (!HAL::I2C::write(_address, &reg, 1)) {
            Serial.println("I2C write failed!");
            return false;
        }
        
        if (!HAL::I2C::read(_address, &value, 1)) {
            Serial.println("I2C read failed!");
            return false;
        }
        
        return true;
    }
};
```

### 2. Resource Management

```cpp
class ResourceManager {
private:
    bool _spi_initialized = false;
    bool _i2c_initialized = false;
    
public:
    void initSPI() {
        if (!_spi_initialized) {
            HAL::SPI::begin();
            _spi_initialized = true;
        }
    }
    
    void initI2C() {
        if (!_i2c_initialized) {
            HAL::I2C::begin();
            _i2c_initialized = true;
        }
    }
};
```

### 3. Pin Configuration

```cpp
// Define pins in a central location
namespace Pins {
    const uint8_t LED = 2;
    const uint8_t BUTTON = 0;
    const uint8_t I2C_SDA = 21;
    const uint8_t I2C_SCL = 22;
    const uint8_t SPI_MOSI = 23;
    const uint8_t SPI_MISO = 19;
    const uint8_t SPI_SCK = 18;
    const uint8_t SPI_CS = 5;
}
```

### 4. Debugging Support

```cpp
#define DEBUG_HAL 1

class DebugHelper {
public:
    static void printPinState(uint8_t pin) {
        #if DEBUG_HAL
        int mode = HAL::GPIO::getPinMode(pin);
        int value = HAL::GPIO::digitalRead(pin);
        Serial.printf("Pin %d: mode=%d, value=%d\n", pin, mode, value);
        #endif
    }
    
    static void printMemoryUsage(const char* location) {
        #if DEBUG_HAL
        Serial.printf("[%s] Free heap: %d, largest block: %d\n",
            location,
            HAL::Memory::getFreeHeap(),
            HAL::Memory::getLargestFreeBlock());
        #endif
    }
};
```

### 5. Platform Independence

```cpp
// Abstract sensor interface
class Sensor {
public:
    virtual void begin() = 0;
    virtual float read() = 0;
    virtual bool isReady() = 0;
};

// Concrete implementation using HAL
class AnalogSensorHAL : public Sensor {
private:
    uint8_t _pin;
    
public:
    AnalogSensorHAL(uint8_t pin) : _pin(pin) {}
    
    void begin() override {
        // HAL handles platform-specific setup
        HAL::ADC::setAttenuation(_pin, ADC_ATTEN_DB_11);
    }
    
    float read() override {
        return HAL::ADC::readVoltage(_pin);
    }
    
    bool isReady() override {
        return true;  // Analog always ready
    }
};
```

## Conclusion

The Hardware Abstraction Layer provides a consistent, portable interface for hardware operations. By using HAL:

1. Your code becomes portable across different ESP32 boards
2. Hardware changes require minimal code modifications
3. Testing becomes easier with the ability to mock HAL functions
4. Code is more readable and maintainable

Remember to always check the HAL header file (`hardware_abstraction.h`) for the complete API reference and any board-specific notes.