#include "hardware_abstraction.h"
#include <cstddef>
#include <cstdlib>

#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <esp_system.h>
#include <esp_sleep.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <driver/ledc.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <soc/soc_caps.h>
#endif

namespace HardwareAbstraction {

    // Global state tracking
    static bool g_initialized = false;
    static bool g_i2c_initialized = false;
    static bool g_spi_initialized = false;
    static bool g_adc_initialized = false;
    #ifdef ARDUINO
    static nvs_handle_t g_nvs_handle = 0;
    #else
    static uint32_t g_nvs_handle = 0; // Mock handle for testing
    #endif

    // Convert result to string
    const char* resultToString(Result result) {
        switch (result) {
            case Result::SUCCESS: return "SUCCESS";
            case Result::ERROR_INIT_FAILED: return "ERROR_INIT_FAILED";
            case Result::ERROR_NOT_INITIALIZED: return "ERROR_NOT_INITIALIZED";
            case Result::ERROR_INVALID_PARAMETER: return "ERROR_INVALID_PARAMETER";
            case Result::ERROR_TIMEOUT: return "ERROR_TIMEOUT";
            case Result::ERROR_COMMUNICATION_FAILED: return "ERROR_COMMUNICATION_FAILED";
            case Result::ERROR_HARDWARE_FAULT: return "ERROR_HARDWARE_FAULT";
            default: return "UNKNOWN_ERROR";
        }
    }

    // Hardware initialization
    Result initialize() {
        if (g_initialized) {
            return Result::SUCCESS;
        }

        #ifdef ARDUINO
        // Initialize NVS
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        if (ret != ESP_OK) {
            return Result::ERROR_INIT_FAILED;
        }

        // Initialize timers
        if (Timer::initialize() != Result::SUCCESS) {
            return Result::ERROR_INIT_FAILED;
        }

        g_initialized = true;
        return Result::SUCCESS;
        #else
        // Mock implementation for testing
        g_initialized = true;
        return Result::SUCCESS;
        #endif
    }

    bool isInitialized() {
        return g_initialized;
    }

    void deinitialize() {
        if (!g_initialized) {
            return;
        }

        #ifdef ARDUINO
        if (g_nvs_handle != 0) {
            nvs_close(g_nvs_handle);
            g_nvs_handle = 0;
        }
        
        nvs_flash_deinit();
        #endif

        g_i2c_initialized = false;
        g_spi_initialized = false;
        g_adc_initialized = false;
        
        // Reset Timer subsystem
        Timer::reset();
        
        g_initialized = false;
    }

    // GPIO Implementation
    namespace GPIO {
        Result pinMode(uint8_t pin, Mode mode) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }
            
            if (pin > 48) { // ESP32-S3 max GPIO
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            switch (mode) {
                case Mode::MODE_INPUT:
                    ::pinMode(pin, INPUT);
                    break;
                case Mode::MODE_OUTPUT:
                    ::pinMode(pin, OUTPUT);
                    break;
                case Mode::MODE_INPUT_PULLUP:
                    ::pinMode(pin, INPUT_PULLUP);
                    break;
                case Mode::MODE_INPUT_PULLDOWN:
                    ::pinMode(pin, INPUT_PULLDOWN);
                    break;
                default:
                    return Result::ERROR_INVALID_PARAMETER;
            }
            #endif

            return Result::SUCCESS;
        }

        Result digitalWrite(uint8_t pin, Level level) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (pin > 48) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            ::digitalWrite(pin, static_cast<int>(level));
            #endif

            return Result::SUCCESS;
        }

        Level digitalRead(uint8_t pin) {
            if (!g_initialized || pin > 48) {
                return Level::LEVEL_LOW;
            }

            #ifdef ARDUINO
            return static_cast<Level>(::digitalRead(pin));
            #else
            return Level::LEVEL_LOW; // Mock
            #endif
        }

        Result attachInterrupt(uint8_t pin, void (*callback)(), int mode) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (pin > 48 || callback == nullptr) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            ::attachInterrupt(digitalPinToInterrupt(pin), callback, mode);
            #endif

            return Result::SUCCESS;
        }

        Result detachInterrupt(uint8_t pin) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (pin > 48) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            ::detachInterrupt(digitalPinToInterrupt(pin));
            #endif

            return Result::SUCCESS;
        }
    }

    // I2C Implementation
    namespace I2C {
        Result initialize(uint8_t sda, uint8_t scl, uint32_t frequency) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (sda > 48 || scl > 48) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            if (g_i2c_initialized) {
                Wire.end();
            }

            Wire.begin(sda, scl);
            Wire.setClock(frequency);
            g_i2c_initialized = true;
            #else
            g_i2c_initialized = true;
            #endif

            return Result::SUCCESS;
        }

        Result beginTransmission(uint8_t address) {
            if (!g_i2c_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (address > 0x7F) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            Wire.beginTransmission(address);
            #endif

            return Result::SUCCESS;
        }

        Result write(uint8_t data) {
            if (!g_i2c_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            size_t written = Wire.write(data);
            return (written == 1) ? Result::SUCCESS : Result::ERROR_COMMUNICATION_FAILED;
            #else
            return Result::SUCCESS;
            #endif
        }

        Result write(const uint8_t* data, size_t length) {
            if (!g_i2c_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (data == nullptr || length == 0) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            size_t written = Wire.write(data, length);
            return (written == length) ? Result::SUCCESS : Result::ERROR_COMMUNICATION_FAILED;
            #else
            return Result::SUCCESS;
            #endif
        }

        Result endTransmission(bool sendStop) {
            if (!g_i2c_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            uint8_t result = Wire.endTransmission(sendStop);
            switch (result) {
                case 0: return Result::SUCCESS;
                case 1: return Result::ERROR_COMMUNICATION_FAILED; // Data too long
                case 2: return Result::ERROR_COMMUNICATION_FAILED; // NACK on address
                case 3: return Result::ERROR_COMMUNICATION_FAILED; // NACK on data
                case 4: return Result::ERROR_COMMUNICATION_FAILED; // Other error
                case 5: return Result::ERROR_TIMEOUT;
                default: return Result::ERROR_HARDWARE_FAULT;
            }
            #else
            return Result::SUCCESS;
            #endif
        }

        Result requestFrom(uint8_t address, size_t length) {
            if (!g_i2c_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (address > 0x7F || length == 0) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            size_t received = Wire.requestFrom(address, length);
            return (received == length) ? Result::SUCCESS : Result::ERROR_COMMUNICATION_FAILED;
            #else
            return Result::SUCCESS;
            #endif
        }

        int available() {
            if (!g_i2c_initialized) {
                return 0;
            }

            #ifdef ARDUINO
            return Wire.available();
            #else
            return 0;
            #endif
        }

        int read() {
            if (!g_i2c_initialized) {
                return -1;
            }

            #ifdef ARDUINO
            return Wire.read();
            #else
            return -1;
            #endif
        }

        void reset() {
            if (g_i2c_initialized) {
                #ifdef ARDUINO
                Wire.end();
                #endif
                g_i2c_initialized = false;
            }
        }
    }

    // SPI Implementation
    namespace SPI {
        Result initialize() {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            if (!g_spi_initialized) {
                ::SPI.begin();
                g_spi_initialized = true;
            }
            #else
            g_spi_initialized = true;
            #endif

            return Result::SUCCESS;
        }

        Result beginTransaction(const Settings& settings) {
            if (!g_spi_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            SPISettings spiSettings(settings.frequency, settings.bitOrder, settings.dataMode);
            ::SPI.beginTransaction(spiSettings);
            #endif

            return Result::SUCCESS;
        }

        uint8_t transfer(uint8_t data) {
            if (!g_spi_initialized) {
                return 0;
            }

            #ifdef ARDUINO
            return ::SPI.transfer(data);
            #else
            return data; // Mock echo
            #endif
        }

        void transfer(uint8_t* data, size_t length) {
            if (!g_spi_initialized || data == nullptr) {
                return;
            }

            #ifdef ARDUINO
            ::SPI.transfer(data, length);
            #endif
        }

        Result endTransaction() {
            if (!g_spi_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            ::SPI.endTransaction();
            #endif

            return Result::SUCCESS;
        }
    }

    // PWM Implementation
    namespace PWM {
        static bool s_ledc_initialized = false;

        Result initialize(uint8_t pin, uint32_t frequency) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (pin > 48) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            if (!s_ledc_initialized) {
                // Configure LEDC timer
                ledc_timer_config_t ledc_timer = {};
                ledc_timer.duty_resolution = LEDC_TIMER_10_BIT;
                ledc_timer.freq_hz = frequency;
                ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
                ledc_timer.timer_num = LEDC_TIMER_0;
                ledc_timer.clk_cfg = LEDC_AUTO_CLK;

                esp_err_t ret = ledc_timer_config(&ledc_timer);
                if (ret != ESP_OK) {
                    return Result::ERROR_INIT_FAILED;
                }
                s_ledc_initialized = true;
            }

            // Configure LEDC channel
            ledc_channel_config_t ledc_channel = {};
            ledc_channel.channel = static_cast<ledc_channel_t>(pin % 8);
            ledc_channel.duty = 0;
            ledc_channel.gpio_num = pin;
            ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
            ledc_channel.timer_sel = LEDC_TIMER_0;

            esp_err_t ret = ledc_channel_config(&ledc_channel);
            return (ret == ESP_OK) ? Result::SUCCESS : Result::ERROR_INIT_FAILED;
            #else
            return Result::SUCCESS;
            #endif
        }

        Result setDutyCycle(uint8_t pin, uint16_t dutyCycle) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (pin > 48 || dutyCycle > 1023) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            ledc_channel_t channel = static_cast<ledc_channel_t>(pin % 8);
            esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, dutyCycle);
            if (ret != ESP_OK) {
                return Result::ERROR_HARDWARE_FAULT;
            }

            ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
            return (ret == ESP_OK) ? Result::SUCCESS : Result::ERROR_HARDWARE_FAULT;
            #else
            return Result::SUCCESS;
            #endif
        }

        Result stop(uint8_t pin) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (pin > 48) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            ledc_channel_t channel = static_cast<ledc_channel_t>(pin % 8);
            esp_err_t ret = ledc_stop(LEDC_LOW_SPEED_MODE, channel, 0);
            return (ret == ESP_OK) ? Result::SUCCESS : Result::ERROR_HARDWARE_FAULT;
            #else
            return Result::SUCCESS;
            #endif
        }
    }

    // ADC Implementation
    namespace ADC {
        #ifdef ARDUINO
        static esp_adc_cal_characteristics_t* s_adc_chars = nullptr;
        static const adc_atten_t ADC_ATTEN = ADC_ATTEN_DB_12; // Use non-deprecated constant
        static const adc_unit_t ADC_UNIT = ADC_UNIT_1;
        #else
        static void* s_adc_chars = nullptr; // Mock for testing
        static const int ADC_ATTEN = 3;     // Mock value
        static const int ADC_UNIT = 1;      // Mock value
        #endif

        Result initialize() {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            if (g_adc_initialized) {
                return Result::SUCCESS;
            }

            // Configure ADC
            esp_err_t ret = adc1_config_width(ADC_WIDTH_BIT_12);
            if (ret != ESP_OK) {
                return Result::ERROR_INIT_FAILED;
            }

            // Allocate memory for ADC characteristics
            s_adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
            if (s_adc_chars == nullptr) {
                return Result::ERROR_INIT_FAILED;
            }

            // Characterize ADC
            esp_adc_cal_characterize(ADC_UNIT, ADC_ATTEN, ADC_WIDTH_BIT_12, 1100, (esp_adc_cal_characteristics_t*)s_adc_chars);
            g_adc_initialized = true;
            #else
            g_adc_initialized = true;
            #endif

            return Result::SUCCESS;
        }

        Result read(uint8_t pin, uint16_t& value) {
            if (!g_adc_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            // Convert GPIO to ADC channel for ESP32-S3
            adc1_channel_t channel;
            switch (pin) {
                case 1: channel = ADC1_CHANNEL_0; break;
                case 2: channel = ADC1_CHANNEL_1; break;
                case 3: channel = ADC1_CHANNEL_2; break;
                case 4: channel = ADC1_CHANNEL_3; break;
                case 5: channel = ADC1_CHANNEL_4; break;
                case 6: channel = ADC1_CHANNEL_5; break;
                case 7: channel = ADC1_CHANNEL_6; break;
                case 8: channel = ADC1_CHANNEL_7; break;
                case 9: channel = ADC1_CHANNEL_8; break;
                case 10: channel = ADC1_CHANNEL_9; break;
                // ESP32-S3 specific mappings
                case 11: channel = ADC1_CHANNEL_0; break; // GPIO1
                case 12: channel = ADC1_CHANNEL_1; break; // GPIO2
                case 13: channel = ADC1_CHANNEL_2; break; // GPIO3
                case 14: channel = ADC1_CHANNEL_3; break; // GPIO4
                case 15: channel = ADC1_CHANNEL_4; break; // GPIO5
                case 16: channel = ADC1_CHANNEL_5; break; // GPIO6
                case 17: channel = ADC1_CHANNEL_6; break; // GPIO7
                case 18: channel = ADC1_CHANNEL_7; break; // GPIO8
                case 19: channel = ADC1_CHANNEL_8; break; // GPIO9
                case 20: channel = ADC1_CHANNEL_9; break; // GPIO10
                default: return Result::ERROR_INVALID_PARAMETER;
            }
            #else
            // Mock channel for testing - validate pin range
            if (pin > 20) {
                return Result::ERROR_INVALID_PARAMETER;
            }
            int channel = pin % 10;
            #endif

            #ifdef ARDUINO
            // Configure channel
            esp_err_t ret = adc1_config_channel_atten(channel, ADC_ATTEN);
            if (ret != ESP_OK) {
                return Result::ERROR_HARDWARE_FAULT;
            }

            // Read raw value
            int raw_value = adc1_get_raw(channel);
            if (raw_value < 0) {
                return Result::ERROR_HARDWARE_FAULT;
            }

            value = static_cast<uint16_t>(raw_value);
            #else
            value = 2048; // Mock middle value
            #endif

            return Result::SUCCESS;
        }

        Result readVoltage(uint8_t pin, float& voltage) {
            if (!g_adc_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            uint16_t raw_value;
            Result result = read(pin, raw_value);
            if (result != Result::SUCCESS) {
                return result;
            }

            #ifdef ARDUINO
            if (s_adc_chars != nullptr) {
                uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(raw_value, s_adc_chars);
                voltage = voltage_mv / 1000.0f;
            } else {
                voltage = (raw_value / 4095.0f) * 3.3f; // Fallback calculation
            }
            #else
            voltage = (raw_value / 4095.0f) * 3.3f; // Mock calculation
            #endif

            return Result::SUCCESS;
        }

        Result setResolution(uint8_t bits) {
            if (!g_adc_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (bits < 9 || bits > 12) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            // For ESP32-S3, only 12-bit is supported
            if (bits != 12) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            esp_err_t ret = adc1_config_width(ADC_WIDTH_BIT_12);
            return (ret == ESP_OK) ? Result::SUCCESS : Result::ERROR_HARDWARE_FAULT;
            #else
            return Result::SUCCESS;
            #endif
        }
    }

    // Timer Implementation
    namespace Timer {
        struct Handle {
            uint32_t interval_ms;
            bool repeating;
            TimerCallback callback;
            uint32_t last_trigger;
            bool active;
            bool delete_requested;
        };

        static constexpr size_t MAX_TIMERS = 16;
        static Handle s_timers[MAX_TIMERS];
        static bool s_timer_initialized = false;

        Result initialize() {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            if (!s_timer_initialized) {
                // Initialize all timer handles
                for (size_t i = 0; i < MAX_TIMERS; ++i) {
                    s_timers[i] = {};
                }
                s_timer_initialized = true;
            }

            return Result::SUCCESS;
        }

        uint32_t millis() {
            #ifdef ARDUINO
            return ::millis();
            #else
            // Mock implementation - return incrementing value
            static uint32_t mock_time = 0;
            return ++mock_time;
            #endif
        }

        uint32_t micros() {
            #ifdef ARDUINO
            return ::micros();
            #else
            // Mock implementation
            static uint32_t mock_time = 0;
            return ++mock_time;
            #endif
        }

        void delay(uint32_t ms) {
            #ifdef ARDUINO
            ::delay(ms);
            #endif
        }

        void delayMicroseconds(uint32_t us) {
            #ifdef ARDUINO
            ::delayMicroseconds(us);
            #endif
        }

        Handle* createTimer(uint32_t intervalMs, bool repeating, TimerCallback callback) {
            if (!s_timer_initialized || callback == nullptr) {
                return nullptr;
            }

            // Find free timer slot
            for (size_t i = 0; i < MAX_TIMERS; ++i) {
                if (s_timers[i].callback == nullptr) {
                    s_timers[i].interval_ms = intervalMs;
                    s_timers[i].repeating = repeating;
                    s_timers[i].callback = callback;
                    s_timers[i].last_trigger = millis();
                    s_timers[i].active = false;
                    s_timers[i].delete_requested = false;
                    return &s_timers[i];
                }
            }

            return nullptr; // No free slots
        }

        Result startTimer(Handle* timer) {
            if (!s_timer_initialized || timer == nullptr) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            timer->active = true;
            timer->last_trigger = millis();
            return Result::SUCCESS;
        }

        Result stopTimer(Handle* timer) {
            if (!s_timer_initialized || timer == nullptr) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            timer->active = false;
            return Result::SUCCESS;
        }

        Result deleteTimer(Handle* timer) {
            if (!s_timer_initialized || timer == nullptr) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            timer->delete_requested = true;
            return Result::SUCCESS;
        }

        // Reset timer subsystem (for testing)
        void reset() {
            s_timer_initialized = false;
            for (size_t i = 0; i < MAX_TIMERS; ++i) {
                s_timers[i] = {};
            }
        }

        // Timer processing function (should be called regularly from main loop)
        void processTimers() {
            if (!s_timer_initialized) {
                return;
            }

            uint32_t current_time = millis();

            for (size_t i = 0; i < MAX_TIMERS; ++i) {
                Handle& timer = s_timers[i];

                // Handle deletion requests
                if (timer.delete_requested) {
                    timer = {}; // Clear the timer
                    continue;
                }

                // Skip inactive or null timers
                if (!timer.active || timer.callback == nullptr) {
                    continue;
                }

                // Check if timer should trigger
                if (current_time - timer.last_trigger >= timer.interval_ms) {
                    timer.callback();
                    
                    if (timer.repeating) {
                        timer.last_trigger = current_time;
                    } else {
                        timer.active = false;
                    }
                }
            }
        }
    }

    // Power Management Implementation
    namespace Power {
        static const uint8_t VEXT_PIN = 36; // Heltec V3 Vext control pin

        Result enableVext() {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            ::pinMode(VEXT_PIN, OUTPUT);
            ::digitalWrite(VEXT_PIN, LOW); // LOW enables Vext on Heltec boards
            #endif

            return Result::SUCCESS;
        }

        Result disableVext() {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            ::pinMode(VEXT_PIN, OUTPUT);
            ::digitalWrite(VEXT_PIN, HIGH); // HIGH disables Vext on Heltec boards
            #endif

            return Result::SUCCESS;
        }

        Result sleep(Mode mode, uint32_t timeMs) {
            if (!g_initialized) {
                return Result::ERROR_NOT_INITIALIZED;
            }

            #ifdef ARDUINO
            switch (mode) {
                case Mode::LIGHT_SLEEP:
                    if (timeMs > 0) {
                        esp_sleep_enable_timer_wakeup(timeMs * 1000); // Convert to microseconds
                    }
                    esp_light_sleep_start();
                    break;

                case Mode::DEEP_SLEEP:
                    if (timeMs > 0) {
                        esp_sleep_enable_timer_wakeup(timeMs * 1000); // Convert to microseconds
                    }
                    esp_deep_sleep_start();
                    break;

                case Mode::NORMAL:
                default:
                    if (timeMs > 0) {
                        Timer::delay(timeMs);
                    }
                    break;
            }
            #endif

            return Result::SUCCESS;
        }

        Result wakeup() {
            // ESP32 wakes up automatically, nothing to do
            return Result::SUCCESS;
        }

        float getBatteryVoltage() {
            #ifdef ARDUINO
            // Heltec V3 has battery voltage divider on ADC pin
            // This is a simplified implementation - actual pin may vary
            uint16_t raw_value;
            if (ADC::read(37, raw_value) == Result::SUCCESS) {
                // Assuming 2:1 voltage divider for battery monitoring
                return (raw_value / 4095.0f) * 3.3f * 2.0f;
            }
            #endif
            return 0.0f;
        }

        uint8_t getBatteryPercent() {
            float voltage = getBatteryVoltage();
            
            // Simple linear mapping for Li-Ion battery (3.0V - 4.2V)
            if (voltage < 3.0f) return 0;
            if (voltage > 4.2f) return 100;
            
            return static_cast<uint8_t>((voltage - 3.0f) / 1.2f * 100.0f);
        }
    }

    // Memory Management Implementation
    namespace Memory {
        void* allocate(size_t size) {
            #ifdef ARDUINO
            return malloc(size);
            #else
            return malloc(size);
            #endif
        }

        void deallocate(void* ptr) {
            if (ptr != nullptr) {
                free(ptr);
            }
        }

        size_t getFreeHeap() {
            #ifdef ARDUINO
            return esp_get_free_heap_size();
            #else
            return 200000; // Mock value
            #endif
        }

        size_t getMinFreeHeap() {
            #ifdef ARDUINO
            return esp_get_minimum_free_heap_size();
            #else
            return 150000; // Mock value
            #endif
        }

        size_t getMaxAllocHeap() {
            #ifdef ARDUINO
            return heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
            #else
            return 100000; // Mock value
            #endif
        }

        Result nvs_open(const char* namespace_name) {
            if (!g_initialized || namespace_name == nullptr) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            if (g_nvs_handle != 0) {
                ::nvs_close(g_nvs_handle);
            }

            esp_err_t ret = ::nvs_open(namespace_name, NVS_READWRITE, &g_nvs_handle);
            return (ret == ESP_OK) ? Result::SUCCESS : Result::ERROR_INIT_FAILED;
            #else
            g_nvs_handle = 1; // Mock handle
            return Result::SUCCESS;
            #endif
        }

        Result nvs_get(const char* key, void* value, size_t& length) {
            if (g_nvs_handle == 0 || key == nullptr || value == nullptr) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            esp_err_t ret = ::nvs_get_blob(g_nvs_handle, key, value, &length);
            switch (ret) {
                case ESP_OK: return Result::SUCCESS;
                case ESP_ERR_NVS_NOT_FOUND: return Result::ERROR_COMMUNICATION_FAILED;
                case ESP_ERR_NVS_INVALID_LENGTH: return Result::ERROR_INVALID_PARAMETER;
                default: return Result::ERROR_HARDWARE_FAULT;
            }
            #else
            return Result::SUCCESS;
            #endif
        }

        Result nvs_set(const char* key, const void* value, size_t length) {
            if (g_nvs_handle == 0 || key == nullptr || value == nullptr) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            esp_err_t ret = ::nvs_set_blob(g_nvs_handle, key, value, length);
            return (ret == ESP_OK) ? Result::SUCCESS : Result::ERROR_HARDWARE_FAULT;
            #else
            return Result::SUCCESS;
            #endif
        }

        Result nvs_commit() {
            if (g_nvs_handle == 0) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            esp_err_t ret = ::nvs_commit(g_nvs_handle);
            return (ret == ESP_OK) ? Result::SUCCESS : Result::ERROR_HARDWARE_FAULT;
            #else
            return Result::SUCCESS;
            #endif
        }

        Result nvs_close() {
            if (g_nvs_handle == 0) {
                return Result::ERROR_INVALID_PARAMETER;
            }

            #ifdef ARDUINO
            ::nvs_close(g_nvs_handle);
            #endif
            g_nvs_handle = 0;
            return Result::SUCCESS;
        }
    }

    // System Information Implementation
    namespace System {
        void getSystemInfo(Info& info) {
            #ifdef ARDUINO
            esp_chip_info_t chip_info;
            esp_chip_info(&chip_info);

            info.chipModel = "ESP32-S3";
            info.chipRevision = chip_info.revision;
            info.chipId = ESP.getEfuseMac();
            info.flashSize = ESP.getFlashChipSize();
            info.freeHeap = esp_get_free_heap_size();
            info.uptime = millis();
            info.cpuFreq = ESP.getCpuFreqMHz();
            #else
            // Mock values for testing
            info.chipModel = "ESP32-S3-Mock";
            info.chipRevision = 1;
            info.chipId = 0x12345678;
            info.flashSize = 8388608; // 8MB
            info.freeHeap = 200000;
            info.uptime = Timer::millis();
            info.cpuFreq = 240.0f;
            #endif
        }

        void restart() {
            #ifdef ARDUINO
            ESP.restart();
            #endif
        }

        void enableWatchdog(uint32_t timeoutMs) {
            #ifdef ARDUINO
            esp_task_wdt_init(timeoutMs / 1000, true);
            esp_task_wdt_add(NULL);
            #endif
        }

        void feedWatchdog() {
            #ifdef ARDUINO
            esp_task_wdt_reset();
            #endif
        }

        void disableWatchdog() {
            #ifdef ARDUINO
            esp_task_wdt_delete(NULL);
            esp_task_wdt_deinit();
            #endif
        }
    }
}