#pragma once

#include "../hardware/hardware_abstraction.h"
#include <stdint.h>

namespace GPS {
    
    // GPS fix types
    enum class FixType {
        NO_FIX = 0,
        FIX_2D = 2,
        FIX_3D = 3
    };

    // GPS data structure
    struct Data {
        // Position
        double latitude;        // Degrees
        double longitude;       // Degrees
        float altitude;         // Meters above sea level
        
        // Accuracy and quality
        float hdop;            // Horizontal dilution of precision
        float vdop;            // Vertical dilution of precision
        uint8_t satellites;    // Number of satellites in use
        FixType fix_type;      // Type of GPS fix
        
        // Time
        uint8_t hour;          // UTC hour (0-23)
        uint8_t minute;        // UTC minute (0-59)
        uint8_t second;        // UTC second (0-59)
        uint16_t year;         // UTC year
        uint8_t month;         // UTC month (1-12)
        uint8_t day;           // UTC day (1-31)
        
        // Speed and course
        float speed_kmh;       // Speed in km/h
        float course_deg;      // Course in degrees (0-360)
        
        // Status
        bool valid;            // True if fix is valid
        uint32_t timestamp;    // System timestamp of last update
    };

    // Configuration options
    struct Config {
        uint32_t baud_rate;    // UART baud rate (default: 9600)
        uint8_t uart_num;      // UART number (default: 1)
        uint8_t tx_pin;        // UART TX pin
        uint8_t rx_pin;        // UART RX pin
        uint8_t pps_pin;       // Pulse-per-second pin (optional)
        uint8_t enable_pin;    // GPS enable/power pin
        bool auto_power_on;    // Automatically power on GPS at init
        uint32_t update_rate;  // Update rate in Hz (1-10)
    };

    // GPS sensor class for UC6580 GNSS chip
    class UC6580 {
    public:
        UC6580();
        ~UC6580();

        // Initialization and configuration
        HardwareAbstraction::Result initialize(const Config& config);
        HardwareAbstraction::Result deinitialize();
        bool isInitialized() const;

        // Power management
        HardwareAbstraction::Result powerOn();
        HardwareAbstraction::Result powerOff();
        bool isPowered() const;

        // Configuration
        HardwareAbstraction::Result setBaudRate(uint32_t baud_rate);
        HardwareAbstraction::Result setUpdateRate(uint32_t rate_hz);
        HardwareAbstraction::Result enableGNSSSystems(bool gps, bool glonass, bool beidou, bool galileo);

        // Data reading
        HardwareAbstraction::Result update();                    // Update GPS data from UART
        const Data& getData() const;                             // Get latest GPS data
        bool hasValidFix() const;                                // Check if GPS has valid fix
        bool isDataFresh(uint32_t max_age_ms = 5000) const;      // Check if data is fresh

        // Utility functions
        float distanceTo(double lat, double lon) const;          // Distance to coordinates (km)
        float bearingTo(double lat, double lon) const;           // Bearing to coordinates (degrees)
        HardwareAbstraction::Result factoryReset();              // Reset GPS to factory defaults
        
        // Diagnostic functions
        void printDiagnostics() const;                           // Print GPS status info
        uint32_t getMessagesReceived() const;                    // Number of valid NMEA messages
        uint32_t getParseErrors() const;                         // Number of parse errors

    private:
        Config m_config;
        Data m_data;
        bool m_initialized;
        bool m_powered;
        
        // Statistics
        uint32_t m_messages_received;
        uint32_t m_parse_errors;
        uint32_t m_last_update;
        
        // Internal methods
        HardwareAbstraction::Result parseNMEA(const char* sentence);
        HardwareAbstraction::Result parseGGA(const char* fields[], int field_count);
        HardwareAbstraction::Result parseRMC(const char* fields[], int field_count);
        HardwareAbstraction::Result parseGSA(const char* fields[], int field_count);
        HardwareAbstraction::Result parseGSV(const char* fields[], int field_count);
        
        bool validateChecksum(const char* sentence) const;
        int splitNMEA(const char* sentence, const char* fields[], int max_fields) const;
        double nmeaToDecimal(const char* nmea_coord, char direction) const;
        float knots_to_kmh(float knots) const;
        
        // Hardware interface helpers
        HardwareAbstraction::Result configureUART();
        HardwareAbstraction::Result sendCommand(const char* command);
        int readLine(char* buffer, int max_length, uint32_t timeout_ms = 1000);
    };

    // Global GPS instance (singleton pattern for simplicity)
    extern UC6580 g_gps;
    
    // Configuration functions  
    Config getDefaultConfig();                                   // Get default config for Wireless Tracker
    Config getWirelessTrackerV11Config();                        // Specific config for V1.1 hardware
    
    // Convenience functions for common operations
    HardwareAbstraction::Result initializeGPS(const Config& config = getDefaultConfig());
    const Data& getGPSData();
    bool hasGPSFix();
}