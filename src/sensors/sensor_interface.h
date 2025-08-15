#pragma once

#include <stdint.h>
#include <functional>

// Extensible sensor framework for all sensor types
namespace SensorSystem {

    // Sensor states
    enum class State {
        UNINITIALIZED,
        INITIALIZING,
        READY,
        READING,
        ERROR,
        DISABLED
    };

    // Sensor data types
    enum class DataType {
        BOOLEAN,        // True/false values
        INTEGER,        // Integer values
        FLOAT,          // Floating point values
        STRING,         // Text values
        BINARY          // Raw binary data
    };

    // Sensor reading structure
    struct Reading {
        uint32_t timestamp;     // When the reading was taken
        DataType type;          // Type of data
        const char* name;       // Sensor/channel name
        const char* unit;       // Unit of measurement
        union {
            bool boolValue;
            int32_t intValue;
            float floatValue;
            const char* stringValue;
            struct {
                const uint8_t* data;
                size_t length;
            } binaryValue;
        } value;
        bool isValid;           // Whether reading is valid
        uint32_t errorCode;     // Error code if invalid
    };

    // Sensor capability flags
    enum class Capability : uint16_t {
        NONE = 0,
        INTERRUPT_CAPABLE = 1,      // Can generate interrupts
        CONFIGURABLE = 2,           // Has configurable parameters
        SELF_TEST = 4,              // Supports self-test
        CALIBRATION = 8,            // Supports calibration
        MULTI_CHANNEL = 16,         // Multiple data channels
        POWER_MANAGEMENT = 32,      // Supports power modes
        THRESHOLD_DETECTION = 64,   // Supports threshold alerts
        DATA_LOGGING = 128          // Can log data internally
    };

    // Sensor callback types
    typedef std::function<void(const Reading&)> ReadingCallback;
    typedef std::function<void(const char* sensorId, uint32_t errorCode)> ErrorCallback;
    typedef std::function<void(const char* sensorId, State state)> StateChangeCallback;

    // Base sensor interface
    class ISensor {
    public:
        virtual ~ISensor() = default;

        // Basic sensor operations
        virtual bool initialize() = 0;
        virtual bool deinitialize() = 0;
        virtual State getState() const = 0;
        virtual const char* getId() const = 0;
        virtual const char* getName() const = 0;
        virtual uint16_t getCapabilities() const = 0;

        // Data operations
        virtual bool readSensor(Reading& reading) = 0;
        virtual bool hasNewData() const = 0;
        virtual uint32_t getReadingCount() const = 0;

        // Configuration
        virtual bool setParameter(const char* name, const void* value, size_t size) = 0;
        virtual bool getParameter(const char* name, void* value, size_t& size) const = 0;
        virtual bool calibrate() = 0;
        virtual bool selfTest() = 0;

        // Power management
        virtual bool sleep() = 0;
        virtual bool wakeup() = 0;
        virtual bool reset() = 0;

        // Callbacks
        virtual void setReadingCallback(ReadingCallback callback) = 0;
        virtual void setErrorCallback(ErrorCallback callback) = 0;
        virtual void setStateChangeCallback(StateChangeCallback callback) = 0;

        // Utility
        virtual void update() = 0;  // Called periodically from main loop
        virtual uint32_t getLastError() const = 0;
        virtual const char* getErrorString(uint32_t errorCode) const = 0;
    };

    // Sensor manager for handling multiple sensors
    class SensorManager {
    public:
        static SensorManager& getInstance();

        // Sensor management
        bool registerSensor(ISensor* sensor);
        bool unregisterSensor(const char* sensorId);
        ISensor* getSensor(const char* sensorId);

        // Global operations
        bool initializeAll();
        void updateAll();
        void deinitializeAll();

        // Data access
        bool getReading(const char* sensorId, Reading& reading);
        bool getReadings(Reading* readings, size_t maxReadings, size_t& count);

        // Callbacks
        void setGlobalReadingCallback(ReadingCallback callback);
        void setGlobalErrorCallback(ErrorCallback callback);

        // Status
        size_t getSensorCount() const;
        void getSensorList(const char** sensorIds, size_t maxSensors, size_t& count) const;

        // Diagnostics
        void printStatus() const;
        bool performHealthCheck();

    private:
        SensorManager() = default;
        static constexpr size_t MAX_SENSORS = 8;

        struct SensorEntry {
            ISensor* sensor;
            bool isActive;
            uint32_t lastUpdate;
            uint32_t errorCount;
        };

        SensorEntry sensors_[MAX_SENSORS];
        size_t sensorCount_;
        ReadingCallback globalReadingCallback_;
        ErrorCallback globalErrorCallback_;

        int findSensorIndex(const char* sensorId) const;
    };

    // Utility functions
    Reading createBoolReading(const char* name, bool value, const char* unit = nullptr);
    Reading createIntReading(const char* name, int32_t value, const char* unit = nullptr);
    Reading createFloatReading(const char* name, float value, const char* unit = nullptr);
    Reading createStringReading(const char* name, const char* value);
    Reading createErrorReading(const char* name, uint32_t errorCode);

    const char* stateToString(State state);
    const char* dataTypeToString(DataType type);
    bool hasCapability(uint16_t capabilities, Capability cap);
}
