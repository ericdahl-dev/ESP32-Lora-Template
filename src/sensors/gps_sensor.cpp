#include "gps_sensor.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cstdio>

#ifdef ARDUINO
#include <HardwareSerial.h>
#endif

namespace GPS {

    // Global GPS instance
    UC6580 g_gps;

    // Default configuration for Wireless Tracker
    Config getDefaultConfig() {
        Config config = {};
        config.baud_rate = 9600;
        config.uart_num = 1;
        config.tx_pin = 43;     // Default UART1 TX for ESP32-S3
        config.rx_pin = 44;     // Default UART1 RX for ESP32-S3
        config.pps_pin = 255;   // Not connected by default
        config.enable_pin = 3;  // GPIO 3 for V1.1 hardware
        config.auto_power_on = true;
        config.update_rate = 1; // 1 Hz
        return config;
    }

    // Specific configuration for Wireless Tracker V1.1
    Config getWirelessTrackerV11Config() {
        Config config = getDefaultConfig();
        config.enable_pin = 3;  // GPIO 3 is the power control pin for V1.1
        return config;
    }

    // Convenience functions
    HardwareAbstraction::Result initializeGPS(const Config& config) {
        return g_gps.initialize(config);
    }

    const Data& getGPSData() {
        return g_gps.getData();
    }

    bool hasGPSFix() {
        return g_gps.hasValidFix();
    }

    // UC6580 class implementation
    UC6580::UC6580() 
        : m_initialized(false)
        , m_powered(false)
        , m_messages_received(0)
        , m_parse_errors(0)
        , m_last_update(0)
    {
        // Initialize data structure
        memset(&m_data, 0, sizeof(m_data));
        memset(&m_config, 0, sizeof(m_config));
    }

    UC6580::~UC6580() {
        deinitialize();
    }

    HardwareAbstraction::Result UC6580::initialize(const Config& config) {
        if (m_initialized) {
            return HardwareAbstraction::Result::SUCCESS;
        }

        m_config = config;

        // Configure GPS enable pin
        if (m_config.enable_pin != 255) {
            auto result = HardwareAbstraction::GPIO::pinMode(m_config.enable_pin, 
                                                           HardwareAbstraction::GPIO::Mode::MODE_OUTPUT);
            if (result != HardwareAbstraction::Result::SUCCESS) {
                return result;
            }
        }

        // Configure UART
        auto result = configureUART();
        if (result != HardwareAbstraction::Result::SUCCESS) {
            return result;
        }

        // Configure PPS pin if specified
        if (m_config.pps_pin != 255) {
            result = HardwareAbstraction::GPIO::pinMode(m_config.pps_pin, 
                                                      HardwareAbstraction::GPIO::Mode::MODE_INPUT);
            if (result != HardwareAbstraction::Result::SUCCESS) {
                return result;
            }
        }

        m_initialized = true;

        // Auto power on if requested
        if (m_config.auto_power_on) {
            result = powerOn();
            if (result != HardwareAbstraction::Result::SUCCESS) {
                m_initialized = false;
                return result;
            }
        }

        return HardwareAbstraction::Result::SUCCESS;
    }

    HardwareAbstraction::Result UC6580::deinitialize() {
        if (!m_initialized) {
            return HardwareAbstraction::Result::SUCCESS;
        }

        powerOff();
        m_initialized = false;
        m_powered = false;

        return HardwareAbstraction::Result::SUCCESS;
    }

    bool UC6580::isInitialized() const {
        return m_initialized;
    }

    HardwareAbstraction::Result UC6580::powerOn() {
        if (!m_initialized) {
            return HardwareAbstraction::Result::ERROR_NOT_INITIALIZED;
        }

        if (m_config.enable_pin != 255) {
            // For Wireless Tracker V1.1: GPIO 3 HIGH = GPS ON
            auto result = HardwareAbstraction::GPIO::digitalWrite(m_config.enable_pin, 
                                                                HardwareAbstraction::GPIO::Level::LEVEL_HIGH);
            if (result != HardwareAbstraction::Result::SUCCESS) {
                return result;
            }
        }

        m_powered = true;

        // Wait for GPS to start up
        HardwareAbstraction::Timer::delay(1000);

        return HardwareAbstraction::Result::SUCCESS;
    }

    HardwareAbstraction::Result UC6580::powerOff() {
        if (!m_initialized) {
            return HardwareAbstraction::Result::ERROR_NOT_INITIALIZED;
        }

        if (m_config.enable_pin != 255) {
            // For Wireless Tracker V1.1: GPIO 3 LOW = GPS OFF
            auto result = HardwareAbstraction::GPIO::digitalWrite(m_config.enable_pin, 
                                                                HardwareAbstraction::GPIO::Level::LEVEL_LOW);
            if (result != HardwareAbstraction::Result::SUCCESS) {
                return result;
            }
        }

        m_powered = false;
        m_data.valid = false;

        return HardwareAbstraction::Result::SUCCESS;
    }

    bool UC6580::isPowered() const {
        return m_powered;
    }

    HardwareAbstraction::Result UC6580::setBaudRate(uint32_t baud_rate) {
        if (!m_initialized) {
            return HardwareAbstraction::Result::ERROR_NOT_INITIALIZED;
        }

        // Send NMEA command to change baud rate
        char command[64];
        snprintf(command, sizeof(command), "$PCAS01,%u*", baud_rate);
        
        // Calculate and append checksum
        uint8_t checksum = 0;
        for (int i = 1; command[i] != '*'; i++) {
            checksum ^= command[i];
        }
        snprintf(command + strlen(command), 8, "%02X\r\n", checksum);

        auto result = sendCommand(command);
        if (result == HardwareAbstraction::Result::SUCCESS) {
            m_config.baud_rate = baud_rate;
            HardwareAbstraction::Timer::delay(100); // Wait for change to take effect
            configureUART(); // Reconfigure UART with new baud rate
        }

        return result;
    }

    HardwareAbstraction::Result UC6580::setUpdateRate(uint32_t rate_hz) {
        if (!m_initialized) {
            return HardwareAbstraction::Result::ERROR_NOT_INITIALIZED;
        }

        if (rate_hz < 1 || rate_hz > 10) {
            return HardwareAbstraction::Result::ERROR_INVALID_PARAMETER;
        }

        // Send NMEA command to change update rate
        char command[64];
        snprintf(command, sizeof(command), "$PCAS02,%u*", 1000 / rate_hz); // Period in ms
        
        // Calculate and append checksum
        uint8_t checksum = 0;
        for (int i = 1; command[i] != '*'; i++) {
            checksum ^= command[i];
        }
        snprintf(command + strlen(command), 8, "%02X\r\n", checksum);

        auto result = sendCommand(command);
        if (result == HardwareAbstraction::Result::SUCCESS) {
            m_config.update_rate = rate_hz;
        }

        return result;
    }

    HardwareAbstraction::Result UC6580::enableGNSSSystems(bool gps, bool glonass, bool beidou, bool galileo) {
        if (!m_initialized) {
            return HardwareAbstraction::Result::ERROR_NOT_INITIALIZED;
        }

        // UC6580 supports multiple GNSS systems
        // This would require specific commands for the UC6580 chip
        // For now, return success as the chip typically enables all systems by default
        
        return HardwareAbstraction::Result::SUCCESS;
    }

    HardwareAbstraction::Result UC6580::update() {
        if (!m_initialized || !m_powered) {
            return HardwareAbstraction::Result::ERROR_NOT_INITIALIZED;
        }

        char buffer[256];
        int bytes_read = readLine(buffer, sizeof(buffer), 100); // 100ms timeout

        if (bytes_read > 0) {
            return parseNMEA(buffer);
        }

        return HardwareAbstraction::Result::ERROR_TIMEOUT;
    }

    const Data& UC6580::getData() const {
        return m_data;
    }

    bool UC6580::hasValidFix() const {
        return m_data.valid && (m_data.fix_type == FixType::FIX_2D || m_data.fix_type == FixType::FIX_3D);
    }

    bool UC6580::isDataFresh(uint32_t max_age_ms) const {
        if (!m_data.valid) {
            return false;
        }

        uint32_t current_time = HardwareAbstraction::Timer::millis();
        return (current_time - m_data.timestamp) <= max_age_ms;
    }

    float UC6580::distanceTo(double lat, double lon) const {
        if (!hasValidFix()) {
            return -1.0f;
        }

        // Haversine formula for distance calculation
        const double R = 6371.0; // Earth's radius in km
        
        double lat1_rad = m_data.latitude * M_PI / 180.0;
        double lat2_rad = lat * M_PI / 180.0;
        double dlat = (lat - m_data.latitude) * M_PI / 180.0;
        double dlon = (lon - m_data.longitude) * M_PI / 180.0;

        double a = sin(dlat/2) * sin(dlat/2) + 
                   cos(lat1_rad) * cos(lat2_rad) * 
                   sin(dlon/2) * sin(dlon/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));

        return static_cast<float>(R * c);
    }

    float UC6580::bearingTo(double lat, double lon) const {
        if (!hasValidFix()) {
            return -1.0f;
        }

        double lat1_rad = m_data.latitude * M_PI / 180.0;
        double lat2_rad = lat * M_PI / 180.0;
        double dlon = (lon - m_data.longitude) * M_PI / 180.0;

        double y = sin(dlon) * cos(lat2_rad);
        double x = cos(lat1_rad) * sin(lat2_rad) - 
                   sin(lat1_rad) * cos(lat2_rad) * cos(dlon);

        double bearing_rad = atan2(y, x);
        double bearing_deg = bearing_rad * 180.0 / M_PI;

        // Normalize to 0-360 degrees
        if (bearing_deg < 0) {
            bearing_deg += 360.0;
        }

        return static_cast<float>(bearing_deg);
    }

    HardwareAbstraction::Result UC6580::factoryReset() {
        if (!m_initialized) {
            return HardwareAbstraction::Result::ERROR_NOT_INITIALIZED;
        }

        // Send factory reset command
        const char* command = "$PCAS10,3*1E\r\n"; // UC6580 factory reset
        return sendCommand(command);
    }

    void UC6580::printDiagnostics() const {
        #ifdef ARDUINO
        Serial.println("=== GPS Diagnostics ===");
        Serial.printf("Initialized: %s\n", m_initialized ? "Yes" : "No");
        Serial.printf("Powered: %s\n", m_powered ? "Yes" : "No");
        Serial.printf("Valid Fix: %s\n", hasValidFix() ? "Yes" : "No");
        Serial.printf("Fix Type: %d\n", static_cast<int>(m_data.fix_type));
        Serial.printf("Satellites: %d\n", m_data.satellites);
        Serial.printf("HDOP: %.2f\n", m_data.hdop);
        Serial.printf("Messages Received: %lu\n", m_messages_received);
        Serial.printf("Parse Errors: %lu\n", m_parse_errors);
        Serial.printf("Last Update: %lu ms ago\n", 
                     HardwareAbstraction::Timer::millis() - m_data.timestamp);
        
        if (hasValidFix()) {
            Serial.printf("Position: %.6f, %.6f\n", m_data.latitude, m_data.longitude);
            Serial.printf("Altitude: %.2f m\n", m_data.altitude);
            Serial.printf("Speed: %.2f km/h\n", m_data.speed_kmh);
            Serial.printf("Course: %.2f degrees\n", m_data.course_deg);
        }
        Serial.println("======================");
        #endif
    }

    uint32_t UC6580::getMessagesReceived() const {
        return m_messages_received;
    }

    uint32_t UC6580::getParseErrors() const {
        return m_parse_errors;
    }

    // Private implementation methods
    HardwareAbstraction::Result UC6580::configureUART() {
        #ifdef ARDUINO
        // Configure UART for GPS communication
        HardwareSerial* serial = nullptr;
        
        switch (m_config.uart_num) {
            case 0:
                serial = &Serial;
                break;
            case 1:
                serial = &Serial1;
                break;
            case 2:
                serial = &Serial2;
                break;
            default:
                return HardwareAbstraction::Result::ERROR_INVALID_PARAMETER;
        }

        serial->begin(m_config.baud_rate, SERIAL_8N1, m_config.rx_pin, m_config.tx_pin);
        
        // Wait for UART to be ready
        HardwareAbstraction::Timer::delay(100);
        #endif

        return HardwareAbstraction::Result::SUCCESS;
    }

    HardwareAbstraction::Result UC6580::sendCommand(const char* command) {
        if (!m_initialized || !m_powered) {
            return HardwareAbstraction::Result::ERROR_NOT_INITIALIZED;
        }

        #ifdef ARDUINO
        HardwareSerial* serial = nullptr;
        
        switch (m_config.uart_num) {
            case 0: serial = &Serial; break;
            case 1: serial = &Serial1; break;
            case 2: serial = &Serial2; break;
            default: return HardwareAbstraction::Result::ERROR_INVALID_PARAMETER;
        }

        serial->print(command);
        serial->flush();
        #endif

        return HardwareAbstraction::Result::SUCCESS;
    }

    int UC6580::readLine(char* buffer, int max_length, uint32_t timeout_ms) {
        if (!m_initialized || !m_powered) {
            return -1;
        }

        #ifdef ARDUINO
        HardwareSerial* serial = nullptr;
        
        switch (m_config.uart_num) {
            case 0: serial = &Serial; break;
            case 1: serial = &Serial1; break;
            case 2: serial = &Serial2; break;
            default: return -1;
        }

        int index = 0;
        uint32_t start_time = HardwareAbstraction::Timer::millis();

        while (index < max_length - 1) {
            if (serial->available()) {
                char c = serial->read();
                
                if (c == '\n') {
                    buffer[index] = '\0';
                    return index;
                }
                
                if (c != '\r') {
                    buffer[index++] = c;
                }
            }

            if (HardwareAbstraction::Timer::millis() - start_time > timeout_ms) {
                break;
            }

            HardwareAbstraction::Timer::delay(1);
        }

        buffer[index] = '\0';
        return index;
        #else
        // Mock implementation for testing
        if (max_length > 20) {
            strcpy(buffer, "$GPGGA,123456.00,0000.0000,N,00000.0000,E,1,04,1.0,0.0,M,0.0,M,,*");
            return strlen(buffer);
        }
        return 0;
        #endif
    }

    HardwareAbstraction::Result UC6580::parseNMEA(const char* sentence) {
        if (!sentence || strlen(sentence) < 6) {
            m_parse_errors++;
            return HardwareAbstraction::Result::ERROR_INVALID_PARAMETER;
        }

        // Validate checksum
        if (!validateChecksum(sentence)) {
            m_parse_errors++;
            return HardwareAbstraction::Result::ERROR_COMMUNICATION_FAILED;
        }

        // Split NMEA sentence into fields
        const char* fields[32];
        int field_count = splitNMEA(sentence, fields, 32);

        if (field_count < 1) {
            m_parse_errors++;
            return HardwareAbstraction::Result::ERROR_COMMUNICATION_FAILED;
        }

        m_messages_received++;
        HardwareAbstraction::Result result = HardwareAbstraction::Result::SUCCESS;

        // Parse different NMEA sentence types
        if (strncmp(fields[0], "$GPGGA", 6) == 0 || strncmp(fields[0], "$GNGGA", 6) == 0) {
            result = parseGGA(fields, field_count);
        }
        else if (strncmp(fields[0], "$GPRMC", 6) == 0 || strncmp(fields[0], "$GNRMC", 6) == 0) {
            result = parseRMC(fields, field_count);
        }
        else if (strncmp(fields[0], "$GPGSA", 6) == 0 || strncmp(fields[0], "$GNGSA", 6) == 0) {
            result = parseGSA(fields, field_count);
        }
        else if (strncmp(fields[0], "$GPGSV", 6) == 0 || strncmp(fields[0], "$GNGSV", 6) == 0) {
            result = parseGSV(fields, field_count);
        }

        if (result == HardwareAbstraction::Result::SUCCESS) {
            m_data.timestamp = HardwareAbstraction::Timer::millis();
        }

        return result;
    }

    HardwareAbstraction::Result UC6580::parseGGA(const char* fields[], int field_count) {
        // $GPGGA,hhmmss.ss,ddmm.mmmm,a,dddmm.mmmm,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
        
        if (field_count < 15) {
            return HardwareAbstraction::Result::ERROR_COMMUNICATION_FAILED;
        }

        // Fix quality
        if (strlen(fields[6]) > 0) {
            int quality = atoi(fields[6]);
            m_data.fix_type = (quality > 0) ? FixType::FIX_3D : FixType::NO_FIX;
            m_data.valid = (quality > 0);
        }

        // Number of satellites
        if (strlen(fields[7]) > 0) {
            m_data.satellites = atoi(fields[7]);
        }

        // Horizontal dilution of precision
        if (strlen(fields[8]) > 0) {
            m_data.hdop = atof(fields[8]);
        }

        // Position
        if (strlen(fields[2]) > 0 && strlen(fields[3]) > 0) {
            m_data.latitude = nmeaToDecimal(fields[2], fields[3][0]);
        }
        
        if (strlen(fields[4]) > 0 && strlen(fields[5]) > 0) {
            m_data.longitude = nmeaToDecimal(fields[4], fields[5][0]);
        }

        // Altitude
        if (strlen(fields[9]) > 0) {
            m_data.altitude = atof(fields[9]);
        }

        return HardwareAbstraction::Result::SUCCESS;
    }

    HardwareAbstraction::Result UC6580::parseRMC(const char* fields[], int field_count) {
        // $GPRMC,hhmmss.ss,A,ddmm.mmmm,a,dddmm.mmmm,a,x.x,x.x,ddmmyy,x.x,a*hh
        
        if (field_count < 12) {
            return HardwareAbstraction::Result::ERROR_COMMUNICATION_FAILED;
        }

        // Status
        if (strlen(fields[2]) > 0) {
            m_data.valid = (fields[2][0] == 'A');
        }

        // Speed (knots to km/h)
        if (strlen(fields[7]) > 0) {
            float speed_knots = atof(fields[7]);
            m_data.speed_kmh = knots_to_kmh(speed_knots);
        }

        // Course
        if (strlen(fields[8]) > 0) {
            m_data.course_deg = atof(fields[8]);
        }

        // Date
        if (strlen(fields[9]) > 0 && strlen(fields[9]) >= 6) {
            int date = atoi(fields[9]);
            m_data.day = date / 10000;
            m_data.month = (date / 100) % 100;
            m_data.year = 2000 + (date % 100);
        }

        // Time
        if (strlen(fields[1]) > 0 && strlen(fields[1]) >= 6) {
            int time = atoi(fields[1]);
            m_data.hour = time / 10000;
            m_data.minute = (time / 100) % 100;
            m_data.second = time % 100;
        }

        return HardwareAbstraction::Result::SUCCESS;
    }

    HardwareAbstraction::Result UC6580::parseGSA(const char* fields[], int field_count) {
        // $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
        
        if (field_count < 18) {
            return HardwareAbstraction::Result::ERROR_COMMUNICATION_FAILED;
        }

        // Fix type
        if (strlen(fields[2]) > 0) {
            int fix = atoi(fields[2]);
            switch (fix) {
                case 1: m_data.fix_type = FixType::NO_FIX; break;
                case 2: m_data.fix_type = FixType::FIX_2D; break;
                case 3: m_data.fix_type = FixType::FIX_3D; break;
                default: m_data.fix_type = FixType::NO_FIX; break;
            }
        }

        // PDOP, HDOP, VDOP
        if (strlen(fields[16]) > 0) {
            m_data.hdop = atof(fields[16]);
        }
        
        if (strlen(fields[17]) > 0) {
            m_data.vdop = atof(fields[17]);
        }

        return HardwareAbstraction::Result::SUCCESS;
    }

    HardwareAbstraction::Result UC6580::parseGSV(const char* fields[], int field_count) {
        // GSV sentences provide satellite information
        // For now, we'll just acknowledge the sentence
        return HardwareAbstraction::Result::SUCCESS;
    }

    bool UC6580::validateChecksum(const char* sentence) const {
        const char* asterisk = strchr(sentence, '*');
        if (!asterisk || asterisk - sentence < 1) {
            return false;
        }

        // Calculate checksum
        uint8_t checksum = 0;
        for (const char* p = sentence + 1; p < asterisk; p++) {
            checksum ^= *p;
        }

        // Parse expected checksum
        char expected_hex[3] = {asterisk[1], asterisk[2], '\0'};
        uint8_t expected = strtol(expected_hex, nullptr, 16);

        return checksum == expected;
    }

    int UC6580::splitNMEA(const char* sentence, const char* fields[], int max_fields) const {
        static char buffer[256];
        strncpy(buffer, sentence, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        // Find the asterisk (checksum delimiter)
        char* asterisk = strchr(buffer, '*');
        if (asterisk) {
            *asterisk = '\0';
        }

        int field_count = 0;
        char* token = strtok(buffer, ",");
        
        while (token && field_count < max_fields) {
            fields[field_count++] = token;
            token = strtok(nullptr, ",");
        }

        return field_count;
    }

    double UC6580::nmeaToDecimal(const char* nmea_coord, char direction) const {
        if (!nmea_coord || strlen(nmea_coord) < 4) {
            return 0.0;
        }

        // Find decimal point
        const char* dot = strchr(nmea_coord, '.');
        if (!dot) {
            return 0.0;
        }

        // Extract degrees (everything before last 2 digits before decimal)
        int deg_len = (dot - nmea_coord) - 2;
        if (deg_len <= 0) {
            return 0.0;
        }

        char deg_str[16];
        strncpy(deg_str, nmea_coord, deg_len);
        deg_str[deg_len] = '\0';
        
        double degrees = atof(deg_str);
        double minutes = atof(nmea_coord + deg_len);

        double decimal = degrees + (minutes / 60.0);

        // Apply direction
        if (direction == 'S' || direction == 'W') {
            decimal = -decimal;
        }

        return decimal;
    }

    float UC6580::knots_to_kmh(float knots) const {
        return knots * 1.852f;
    }

} // namespace GPS