#pragma once

#include "sensor_interface.h"
#include "../config/system_config.h"

// AS3935 Lightning Sensor Implementation
namespace Sensors {

    // Lightning sensor specific data
    struct LightningData {
        bool lightningDetected;
        uint8_t distance;           // Distance in km (0 = overhead, 1-40 km, 63 = out of range)
        uint32_t energy;            // Lightning energy level
        uint8_t strikeCount;        // Number of strikes in current event
        uint32_t lastStrikeTime;    // Timestamp of last strike
        bool isDisturber;           // Whether event was classified as disturber
        uint8_t noiseLevel;         // Current noise floor level
    };

    // AS3935 registers
    enum class AS3935Register : uint8_t {
        AFE_GAIN = 0x00,
        PWD = 0x00,
        NF_LEV = 0x01,
        WDTH = 0x01,
        CL_STAT = 0x02,
        MIN_NUM_LIGH = 0x02,
        SREJ = 0x02,
        LCO_FDIV = 0x03,
        MASK_DIST = 0x03,
        DISP_LCO = 0x08,
        DISP_SRCO = 0x08,
        DISP_TRCO = 0x08,
        TUN_CAP = 0x08
    };

    // Interrupt reasons
    enum class InterruptReason : uint8_t {
        NOISE = 0x01,               // Noise level too high
        DISTURBER = 0x04,           // Disturber detected
        LIGHTNING = 0x08            // Lightning detected
    };

    class LightningSensor : public ISensor {
    public:
        LightningSensor();
        virtual ~LightningSensor() = default;

        // ISensor interface implementation
        bool initialize() override;
        bool deinitialize() override;
        State getState() const override { return state_; }
        const char* getId() const override { return "AS3935"; }
        const char* getName() const override { return "Lightning Sensor"; }
        uint16_t getCapabilities() const override;

        bool readSensor(Reading& reading) override;
        bool hasNewData() const override { return hasNewData_; }
        uint32_t getReadingCount() const override { return readingCount_; }

        bool setParameter(const char* name, const void* value, size_t size) override;
        bool getParameter(const char* name, void* value, size_t& size) const override;
        bool calibrate() override;
        bool selfTest() override;

        bool sleep() override;
        bool wakeup() override;
        bool reset() override;

        void setReadingCallback(ReadingCallback callback) override { readingCallback_ = callback; }
        void setErrorCallback(ErrorCallback callback) override { errorCallback_ = callback; }
        void setStateChangeCallback(StateChangeCallback callback) override { stateChangeCallback_ = callback; }

        void update() override;
        uint32_t getLastError() const override { return lastError_; }
        const char* getErrorString(uint32_t errorCode) const override;

        // Lightning sensor specific methods
        bool setNoiseFloor(uint8_t level);              // 0-7
        bool setWatchdogThreshold(uint8_t threshold);   // 0-15
        bool setSpikeRejection(uint8_t rejection);      // 0-15
        bool setMinimumStrikes(uint8_t strikes);        // 1, 5, 9, 16
        bool setIndoorMode(bool indoor);
        bool maskDisturbers(bool mask);

        // Calibration
        bool tuneTankCircuit();
        bool calibrateRCO();

        // Status queries
        uint8_t getNoiseFloor() const { return config_.noiseFloor; }
        uint8_t getWatchdogThreshold() const { return config_.watchdogThreshold; }
        uint8_t getSpikeRejection() const { return config_.spikeRejection; }
        uint8_t getMinimumStrikes() const { return config_.minimumStrikes; }
        bool isIndoorMode() const { return config_.indoorMode; }
        bool areDisturbersLightning() const { return config_.disturbersLightning; }

        // Statistics
        uint32_t getTotalLightningCount() const { return stats_.totalLightning; }
        uint32_t getTotalDisturberCount() const { return stats_.totalDisturbers; }
        uint32_t getTotalNoiseEvents() const { return stats_.totalNoise; }
        uint32_t getLastActivityTime() const { return stats_.lastActivity; }

        // Lightning data access
        bool getLastLightningData(LightningData& data) const;

        // Interrupt handling (called from ISR)
        static void IRAM_ATTR interruptHandler();
        void handleInterrupt();

    private:
        // Configuration structure
        struct Config {
            uint8_t noiseFloor;         // Noise floor level (0-7)
            uint8_t watchdogThreshold;  // Watchdog threshold (0-15)
            uint8_t spikeRejection;     // Spike rejection level (0-15)
            uint8_t minimumStrikes;     // Minimum number of strikes (1,5,9,16)
            bool indoorMode;            // Indoor/outdoor mode
            bool disturbersLightning;   // Treat disturbers as lightning
            uint8_t tuningCapacitor;    // Tank circuit tuning (0-15)
        };

        // Statistics structure
        struct Statistics {
            uint32_t totalLightning;
            uint32_t totalDisturbers;
            uint32_t totalNoise;
            uint32_t lastActivity;
            uint32_t communicationErrors;
            uint32_t calibrationCount;
        };

        State state_;
        Config config_;
        Statistics stats_;
        LightningData lastLightning_;

        bool hasNewData_;
        uint32_t readingCount_;
        uint32_t lastError_;

        // Callbacks
        ReadingCallback readingCallback_;
        ErrorCallback errorCallback_;
        StateChangeCallback stateChangeCallback_;

        // Interrupt handling
        volatile bool interruptPending_;
        static LightningSensor* instance_;

        // Hardware interface
        bool writeRegister(AS3935Register reg, uint8_t value);
        bool readRegister(AS3935Register reg, uint8_t& value);
        bool modifyRegister(AS3935Register reg, uint8_t mask, uint8_t value);

        // Internal operations
        bool powerUp();
        bool powerDown();
        bool clearStatistics();
        InterruptReason getInterruptReason();
        uint8_t getLightningDistance();
        uint32_t getLightningEnergy();

        // Validation
        bool validateConfig() const;
        bool isConfigurationValid(const Config& config) const;

        // State management
        void setState(State newState);
        void reportError(uint32_t errorCode, const char* message = nullptr);

        // Constants
        static constexpr uint8_t CHIP_ID = 0x3C;
        static constexpr uint32_t RESET_DELAY_MS = 2;
        static constexpr uint32_t CALIBRATION_TIMEOUT_MS = 2000;
        static constexpr uint32_t INTERRUPT_TIMEOUT_MS = 100;
    };

    // Error codes specific to lightning sensor
    namespace LightningErrorCodes {
        constexpr uint32_t CHIP_NOT_FOUND = 1001;
        constexpr uint32_t COMMUNICATION_FAILED = 1002;
        constexpr uint32_t CALIBRATION_FAILED = 1003;
        constexpr uint32_t INVALID_CONFIGURATION = 1004;
        constexpr uint32_t INTERRUPT_SETUP_FAILED = 1005;
        constexpr uint32_t TANK_TUNING_FAILED = 1006;
        constexpr uint32_t RCO_CALIBRATION_FAILED = 1007;
    }
}
