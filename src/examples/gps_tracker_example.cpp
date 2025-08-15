#include "../hardware/hardware_abstraction.h"
#include "../sensors/gps_sensor.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

// Example usage of GPS sensor with Wireless Tracker
class GPSTrackerExample {
public:
    GPSTrackerExample() : m_last_fix_time(0), m_fix_lost_time(0) {}

    bool initialize() {
        // Initialize hardware abstraction layer
        auto result = HardwareAbstraction::initialize();
        if (result != HardwareAbstraction::Result::SUCCESS) {
            #ifdef ARDUINO
            Serial.printf("HAL initialization failed: %s\n", 
                         HardwareAbstraction::resultToString(result));
            #endif
            return false;
        }

        // Initialize GPS with Wireless Tracker V1.1 configuration
        GPS::Config gps_config = GPS::getWirelessTrackerV11Config();
        result = GPS::initializeGPS(gps_config);
        if (result != HardwareAbstraction::Result::SUCCESS) {
            #ifdef ARDUINO
            Serial.printf("GPS initialization failed: %s\n", 
                         HardwareAbstraction::resultToString(result));
            #endif
            return false;
        }

        #ifdef ARDUINO
        Serial.println("GPS Tracker initialized successfully!");
        Serial.println("Waiting for GPS fix...");
        #endif

        return true;
    }

    void update() {
        // Update GPS data
        GPS::g_gps.update();
        
        const GPS::Data& data = GPS::getGPSData();
        bool has_fix = GPS::hasGPSFix();
        
        uint32_t current_time = HardwareAbstraction::Timer::millis();

        if (has_fix) {
            if (m_last_fix_time == 0) {
                // First fix acquired
                #ifdef ARDUINO
                Serial.println("\n*** GPS FIX ACQUIRED ***");
                printGPSInfo(data);
                #endif
            }
            m_last_fix_time = current_time;
            m_fix_lost_time = 0;
        } else {
            if (m_last_fix_time > 0 && m_fix_lost_time == 0) {
                // Fix lost
                m_fix_lost_time = current_time;
                #ifdef ARDUINO
                Serial.println("GPS fix lost, searching...");
                #endif
            }
        }

        // Print status every 10 seconds
        static uint32_t last_status_print = 0;
        if (current_time - last_status_print > 10000) {
            printStatus(data, has_fix);
            last_status_print = current_time;
        }

        // Print detailed info every 30 seconds when we have a fix
        static uint32_t last_detail_print = 0;
        if (has_fix && current_time - last_detail_print > 30000) {
            printDetailedInfo(data);
            last_detail_print = current_time;
        }
    }

private:
    uint32_t m_last_fix_time;
    uint32_t m_fix_lost_time;

    void printStatus(const GPS::Data& data, bool has_fix) {
        #ifdef ARDUINO
        Serial.printf("[%lu] Status: %s | Sats: %d | HDOP: %.2f | ",
                     HardwareAbstraction::Timer::millis(),
                     has_fix ? "FIX" : "NO_FIX",
                     data.satellites,
                     data.hdop);

        if (has_fix) {
            Serial.printf("Pos: %.6f,%.6f | Alt: %.1fm | Speed: %.1f km/h\n",
                         data.latitude, data.longitude, data.altitude, data.speed_kmh);
        } else {
            Serial.println("Searching for satellites...");
        }
        #endif
    }

    void printGPSInfo(const GPS::Data& data) {
        #ifdef ARDUINO
        Serial.printf("Position: %.6f, %.6f\n", data.latitude, data.longitude);
        Serial.printf("Altitude: %.2f meters\n", data.altitude);
        Serial.printf("Speed: %.2f km/h\n", data.speed_kmh);
        Serial.printf("Course: %.2f degrees\n", data.course_deg);
        Serial.printf("Satellites: %d\n", data.satellites);
        Serial.printf("HDOP: %.2f\n", data.hdop);
        Serial.printf("Fix Type: %s\n", 
                     data.fix_type == GPS::FixType::FIX_3D ? "3D" :
                     data.fix_type == GPS::FixType::FIX_2D ? "2D" : "NO_FIX");
        Serial.printf("Time: %02d:%02d:%02d UTC\n", data.hour, data.minute, data.second);
        Serial.printf("Date: %04d-%02d-%02d\n", data.year, data.month, data.day);
        Serial.println();
        #endif
    }

    void printDetailedInfo(const GPS::Data& data) {
        #ifdef ARDUINO
        Serial.println("=== Detailed GPS Information ===");
        printGPSInfo(data);
        
        // Print diagnostics
        GPS::g_gps.printDiagnostics();
        
        // Show data freshness
        Serial.printf("Data age: %lu ms\n", 
                     HardwareAbstraction::Timer::millis() - data.timestamp);
        Serial.printf("Data fresh: %s\n", 
                     GPS::g_gps.isDataFresh() ? "Yes" : "No");
        Serial.println("================================\n");
        #endif
    }
};

// Global tracker instance
GPSTrackerExample g_tracker;

#ifdef ARDUINO
void setup() {
    Serial.begin(115200);
    delay(2000); // Allow time for serial to initialize

    Serial.println("Wireless Tracker GPS Example");
    Serial.println("============================");
    
    if (!g_tracker.initialize()) {
        Serial.println("Failed to initialize GPS tracker!");
        while (1) {
            delay(1000);
        }
    }
}

void loop() {
    g_tracker.update();
    delay(1000); // Update every second
}
#endif

// For unit testing
#ifndef ARDUINO
int main() {
    printf("GPS Tracker Example (Test Mode)\n");
    
    if (!g_tracker.initialize()) {
        printf("Failed to initialize GPS tracker!\n");
        return 1;
    }

    // Simulate updates
    for (int i = 0; i < 10; i++) {
        g_tracker.update();
        HardwareAbstraction::Timer::delay(1000);
    }

    return 0;
}
#endif