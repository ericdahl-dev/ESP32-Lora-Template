#pragma once

#include <stdint.h>
#ifdef ARDUINO
#include <Arduino.h>
#endif

// Structured logging system for debugging and monitoring
namespace Logging {

    // Log levels
    enum class Level {
        TRACE = 0,   // Detailed trace information
        DEBUG = 1,   // Debug information
        INFO = 2,    // General information
        WARN = 3,    // Warning messages
        ERROR = 4,   // Error messages
        FATAL = 5    // Fatal errors
    };

    // Log categories
    enum class Category {
        SYSTEM,      // System-wide messages
        HARDWARE,    // Hardware initialization and operations
        RADIO,       // LoRa radio communications
        WIFI,        // WiFi operations
        SENSOR,      // Sensor readings and operations
        ACTUATOR,    // LED/buzzer operations
        OTA,         // Firmware update operations
        UI,          // User interface operations
        CONFIG,      // Configuration operations
        TEST         // Test and debug messages
    };

    // Log destinations
    enum class Destination {
        SERIAL = 1,      // Serial output
        DISPLAY = 2,     // OLED display (brief messages)
        RADIO = 4,       // Send via LoRa (critical messages only)
        STORAGE = 8      // Store in flash (future feature)
    };

    // Initialize logging system
    void initialize(Level minLevel = Level::INFO,
                   uint8_t destinations = static_cast<uint8_t>(Destination::SERIAL));

    // Set minimum log level
    void setLevel(Level level);

    // Enable/disable categories
    void enableCategory(Category category, bool enabled = true);

    // Set log destinations
    void setDestinations(uint8_t destinations);

    // Core logging functions
    void log(Level level, Category category, const char* format, ...);
    void trace(Category category, const char* format, ...);
    void debug(Category category, const char* format, ...);
    void info(Category category, const char* format, ...);
    void warn(Category category, const char* format, ...);
    void error(Category category, const char* format, ...);
    void fatal(Category category, const char* format, ...);

    // Convenience macros for easier usage
    #define LOG_TRACE(cat, ...) Logger::trace(Logger::Category::cat, __VA_ARGS__)
    #define LOG_DEBUG(cat, ...) Logger::debug(Logger::Category::cat, __VA_ARGS__)
    #define LOG_INFO(cat, ...) Logger::info(Logger::Category::cat, __VA_ARGS__)
    #define LOG_WARN(cat, ...) Logger::warn(Logger::Category::cat, __VA_ARGS__)
    #define LOG_ERROR(cat, ...) Logger::error(Logger::Category::cat, __VA_ARGS__)
    #define LOG_FATAL(cat, ...) Logger::fatal(Logger::Category::cat, __VA_ARGS__)

    // Special logging functions
    void logSystemBoot();
    void logMemoryUsage();
    void logRadioStats();
    void logSensorReading(const char* sensorName, float value, const char* unit);
    void logError(const char* module, const char* error, uint32_t errorCode = 0);

    // Performance monitoring
    void startTimer(const char* name);
    void endTimer(const char* name);

    // Convert enums to strings
    const char* levelToString(Level level);
    const char* categoryToString(Category category);

    // Get log statistics
    struct LogStats {
        uint32_t totalMessages;
        uint32_t messagesByLevel[6];  // Count for each level
        uint32_t droppedMessages;
        uint32_t uptime;
    };

    void getStats(LogStats& stats);
}
