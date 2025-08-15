#pragma once

#include <stdint.h>

// Comprehensive error handling and recovery system
namespace ErrorHandling {

    // Error severity levels
    enum class Severity {
        INFO,       // Informational - no action needed
        WARNING,    // Warning - system can continue
        ERROR,      // Error - feature may not work
        CRITICAL    // Critical - system may need restart
    };

    // Error categories
    enum class Category {
        HARDWARE,   // Hardware initialization/communication failures
        RADIO,      // LoRa radio communication issues
        WIFI,       // WiFi connection problems
        SENSOR,     // Sensor reading failures
        ACTUATOR,   // LED/buzzer control issues
        OTA,        // Firmware update problems
        SYSTEM,     // General system errors
        CONFIG      // Configuration/storage issues
    };

    // Error codes
    enum class Code {
        // Hardware errors (100-199)
        OLED_INIT_FAILED = 100,
        I2C_COMMUNICATION_FAILED = 101,
        POWER_MANAGEMENT_FAILED = 102,

        // Radio errors (200-299)
        RADIO_INIT_FAILED = 200,
        RADIO_TX_FAILED = 201,
        RADIO_RX_FAILED = 202,
        RADIO_CONFIG_FAILED = 203,

        // WiFi errors (300-399)
        WIFI_CONNECT_FAILED = 300,
        WIFI_TIMEOUT = 301,
        WIFI_AUTH_FAILED = 302,
        WIFI_CONFIG_INVALID = 303,

        // Sensor errors (400-499)
        SENSOR_INIT_FAILED = 400,
        SENSOR_READ_FAILED = 401,
        SENSOR_CALIBRATION_FAILED = 402,

        // Actuator errors (500-599)
        LED_INIT_FAILED = 500,
        LED_UPDATE_FAILED = 501,
        BUZZER_INIT_FAILED = 502,

        // OTA errors (600-699)
        OTA_INIT_FAILED = 600,
        OTA_DOWNLOAD_FAILED = 601,
        OTA_VERIFICATION_FAILED = 602,
        OTA_STORAGE_FAILED = 603,

        // System errors (700-799)
        MEMORY_ALLOCATION_FAILED = 700,
        TASK_CREATION_FAILED = 701,
        WATCHDOG_TIMEOUT = 702,

        // Config errors (800-899)
        CONFIG_LOAD_FAILED = 800,
        CONFIG_SAVE_FAILED = 801,
        CONFIG_VALIDATION_FAILED = 802
    };

    // Error information structure
    struct ErrorInfo {
        Code code;
        Category category;
        Severity severity;
        uint32_t timestamp;
        const char* message;
        const char* module;
        uint32_t data;      // Additional context data
    };

    // Error handler callback type
    typedef void (*ErrorCallback)(const ErrorInfo& error);

    // Initialize error handling system
    void initialize();

    // Report an error
    void reportError(Code code, Category category, Severity severity,
                    const char* module, const char* message = nullptr,
                    uint32_t data = 0);

    // Register error callback
    void registerCallback(ErrorCallback callback);

    // Get last error
    bool getLastError(ErrorInfo& error);

    // Clear error history
    void clearErrors();

    // Get error count by category
    uint32_t getErrorCount(Category category = Category::SYSTEM);

    // Check if system is in error state
    bool hasErrors(Severity minSeverity = Severity::ERROR);

    // Attempt automatic recovery
    bool attemptRecovery(Code code);

    // System health check
    bool performHealthCheck();

    // Convert error to string
    const char* errorCodeToString(Code code);
    const char* categoryToString(Category category);
    const char* severityToString(Severity severity);
}
