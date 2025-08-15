#pragma once

#include <stdint.h>
#include <cstddef>

// Hardware Abstraction Layer (HAL)
// Provides a clean interface to hardware components for better testability
namespace HardwareAbstraction {

    // Initialize result codes
    enum class Result {
        SUCCESS = 0,
        ERROR_INIT_FAILED,
        ERROR_NOT_INITIALIZED,
        ERROR_INVALID_PARAMETER,
        ERROR_TIMEOUT,
        ERROR_COMMUNICATION_FAILED,
        ERROR_HARDWARE_FAULT
    };

    // GPIO abstraction
    namespace GPIO {
        enum class Mode {
            MODE_INPUT,
            MODE_OUTPUT,
            MODE_INPUT_PULLUP,
            MODE_INPUT_PULLDOWN
        };

        enum class Level {
            LEVEL_LOW = 0,
            LEVEL_HIGH = 1
        };

        Result pinMode(uint8_t pin, Mode mode);
        Result digitalWrite(uint8_t pin, Level level);
        Level digitalRead(uint8_t pin);
        Result attachInterrupt(uint8_t pin, void (*callback)(), int mode);
        Result detachInterrupt(uint8_t pin);
    }

    // I2C abstraction
    namespace I2C {
        Result initialize(uint8_t sda, uint8_t scl, uint32_t frequency = 100000);
        Result beginTransmission(uint8_t address);
        Result write(uint8_t data);
        Result write(const uint8_t* data, size_t length);
        Result endTransmission(bool sendStop = true);
        Result requestFrom(uint8_t address, size_t length);
        int available();
        int read();
        void reset();
    }

    // SPI abstraction
    namespace SPI {
        struct Settings {
            uint32_t frequency;
            uint8_t bitOrder;
            uint8_t dataMode;
        };

        Result initialize();
        Result beginTransaction(const Settings& settings);
        uint8_t transfer(uint8_t data);
        void transfer(uint8_t* data, size_t length);
        Result endTransaction();
    }

    // PWM abstraction
    namespace PWM {
        Result initialize(uint8_t pin, uint32_t frequency = 1000);
        Result setDutyCycle(uint8_t pin, uint16_t dutyCycle); // 0-1023
        Result stop(uint8_t pin);
    }

    // ADC abstraction
    namespace ADC {
        Result initialize();
        Result read(uint8_t pin, uint16_t& value);
        Result readVoltage(uint8_t pin, float& voltage);
        Result setResolution(uint8_t bits);
    }

    // Timer abstraction
    namespace Timer {
        typedef void (*TimerCallback)();

        Result initialize();
        uint32_t millis();
        uint32_t micros();
        void delay(uint32_t ms);
        void delayMicroseconds(uint32_t us);

        // Software timers
        struct Handle;
        Handle* createTimer(uint32_t intervalMs, bool repeating, TimerCallback callback);
        Result startTimer(Handle* timer);
        Result stopTimer(Handle* timer);
        Result deleteTimer(Handle* timer);
        void reset(); // Reset timer subsystem (for testing)
    }

    // Power management
    namespace Power {
        enum class Mode {
            NORMAL,
            LIGHT_SLEEP,
            DEEP_SLEEP
        };

        Result enableVext();      // Enable external power rail
        Result disableVext();     // Disable external power rail
        Result sleep(Mode mode, uint32_t timeMs = 0);
        Result wakeup();
        float getBatteryVoltage();
        uint8_t getBatteryPercent();
    }

    // Memory management
    namespace Memory {
        void* allocate(size_t size);
        void deallocate(void* ptr);
        size_t getFreeHeap();
        size_t getMinFreeHeap();
        size_t getMaxAllocHeap();

        // Non-volatile storage
        Result nvs_open(const char* namespace_name);
        Result nvs_get(const char* key, void* value, size_t& length);
        Result nvs_set(const char* key, const void* value, size_t length);
        Result nvs_commit();
        Result nvs_close();
    }

    // System information
    namespace System {
        struct Info {
            const char* chipModel;
            uint8_t chipRevision;
            uint32_t chipId;
            uint32_t flashSize;
            uint32_t freeHeap;
            uint32_t uptime;
            float cpuFreq;
        };

        void getSystemInfo(Info& info);
        void restart();
        void enableWatchdog(uint32_t timeoutMs);
        void feedWatchdog();
        void disableWatchdog();
    }

    // Hardware initialization
    Result initialize();
    bool isInitialized();
    void deinitialize();

    // Convert result to string
    const char* resultToString(Result result);
}
