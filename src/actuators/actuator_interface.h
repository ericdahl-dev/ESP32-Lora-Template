#pragma once

#include <stdint.h>
#include <functional>

// Extensible actuator framework for LEDs, buzzers, displays, etc.
namespace ActuatorSystem {

    // Actuator states
    enum class State {
        UNINITIALIZED,
        INITIALIZING,
        READY,
        ACTIVE,
        ERROR,
        DISABLED
    };

    // Actuator types
    enum class Type {
        LED_STRIP,      // Addressable LED strips (WS2812, etc.)
        LED_SINGLE,     // Single LED
        BUZZER,         // Piezo buzzer or speaker
        DISPLAY,        // OLED/LCD displays
        MOTOR,          // Motors/servos
        RELAY,          // Relays/switches
        CUSTOM          // Custom actuator types
    };

        // Color structure for LEDs
    struct Color {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t white;      // For RGBW LEDs

        constexpr Color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t w = 0)
            : red(r), green(g), blue(b), white(w) {}

        static Color fromHSV(uint16_t hue, uint8_t saturation, uint8_t value);
        static Color fromHex(uint32_t hex);
        uint32_t toHex() const;
    };

    // Predefined colors
    namespace Colors {
        constexpr Color BLACK(0, 0, 0);
        constexpr Color WHITE(255, 255, 255);
        constexpr Color RED(255, 0, 0);
        constexpr Color GREEN(0, 255, 0);
        constexpr Color BLUE(0, 0, 255);
        constexpr Color YELLOW(255, 255, 0);
        constexpr Color CYAN(0, 255, 255);
        constexpr Color MAGENTA(255, 0, 255);
        constexpr Color ORANGE(255, 165, 0);
        constexpr Color PURPLE(128, 0, 128);
    }

    // Animation types
    enum class Animation {
        NONE,           // Static state
        FADE,           // Fade in/out
        BLINK,          // On/off blinking
        PULSE,          // Smooth pulsing
        RAINBOW,        // Rainbow color cycle
        CHASE,          // Chase pattern
        SPARKLE,        // Random sparkle effect
        LIGHTNING,      // Lightning flash effect
        CUSTOM          // Custom animation
    };

    // Sound patterns for buzzer
    enum class SoundPattern {
        NONE,           // Silent
        BEEP,           // Single beep
        DOUBLE_BEEP,    // Two quick beeps
        TRIPLE_BEEP,    // Three quick beeps
        LONG_BEEP,      // Extended beep
        ALARM,          // Alarm pattern
        MUSICAL_NOTE,   // Musical tone
        CUSTOM          // Custom pattern
    };

    // Actuator command structure
    struct Command {
        Type type;
        uint32_t timestamp;
        uint32_t duration;      // Command duration in ms (0 = indefinite)
        uint8_t priority;       // Command priority (0-255)

        union {
            // LED commands
            struct {
                uint16_t startIndex;
                uint16_t count;
                Color color;
                Animation animation;
                uint16_t animationSpeed;
                uint8_t brightness;
            } led;

            // Buzzer commands
            struct {
                SoundPattern pattern;
                uint16_t frequency;
                uint8_t volume;
                uint16_t duration;
            } buzzer;

            // Display commands
            struct {
                uint8_t x, y;
                const char* text;
                uint8_t font;
                bool clear;
            } display;

            // Generic commands
            struct {
                uint32_t parameter1;
                uint32_t parameter2;
                const void* data;
                size_t dataSize;
            } generic;
        };
    };

    // Actuator callback types
    typedef std::function<void(const char* actuatorId)> CompletionCallback;
    typedef std::function<void(const char* actuatorId, uint32_t errorCode)> ErrorCallback;
    typedef std::function<void(const char* actuatorId, State oldState, State newState)> StateChangeCallback;

    // Base actuator interface
    class IActuator {
    public:
        virtual ~IActuator() = default;

        // Basic actuator operations
        virtual bool initialize() = 0;
        virtual bool deinitialize() = 0;
        virtual State getState() const = 0;
        virtual Type getType() const = 0;
        virtual const char* getId() const = 0;
        virtual const char* getName() const = 0;

        // Command operations
        virtual bool executeCommand(const Command& command) = 0;
        virtual bool stopCommand() = 0;
        virtual bool isCommandActive() const = 0;
        virtual uint32_t getRemainingTime() const = 0;

        // State operations
        virtual bool setState(bool enabled) = 0;
        virtual bool getState(void* stateData, size_t& size) const = 0;
        virtual bool reset() = 0;

        // Configuration
        virtual bool setParameter(const char* name, const void* value, size_t size) = 0;
        virtual bool getParameter(const char* name, void* value, size_t& size) const = 0;
        virtual bool selfTest() = 0;

        // Callbacks
        virtual void setCompletionCallback(CompletionCallback callback) = 0;
        virtual void setErrorCallback(ErrorCallback callback) = 0;
        virtual void setStateChangeCallback(StateChangeCallback callback) = 0;

        // Utility
        virtual void update() = 0;  // Called periodically from main loop
        virtual uint32_t getLastError() const = 0;
        virtual const char* getErrorString(uint32_t errorCode) const = 0;
    };

    // Actuator manager for coordinating multiple actuators
    class ActuatorManager {
    public:
        static ActuatorManager& getInstance();

        // Actuator management
        bool registerActuator(IActuator* actuator);
        bool unregisterActuator(const char* actuatorId);
        IActuator* getActuator(const char* actuatorId);

        // Global operations
        bool initializeAll();
        void updateAll();
        void deinitializeAll();
        void stopAll();

        // Command operations
        bool executeCommand(const char* actuatorId, const Command& command);
        bool executeCommands(const Command* commands, size_t count);
        bool stopActuator(const char* actuatorId);

        // Convenience methods
        bool setLED(const char* ledId, const Color& color, uint8_t brightness = 255);
        bool setLEDStrip(const char* stripId, const Color& color, uint8_t brightness = 255);
        bool animateLEDs(const char* ledId, Animation animation, uint16_t speed = 100);
        bool playSound(const char* buzzerId, SoundPattern pattern, uint16_t frequency = 1000);
        bool displayText(const char* displayId, const char* text, uint8_t x = 0, uint8_t y = 0);

        // Callbacks
        void setGlobalCompletionCallback(CompletionCallback callback);
        void setGlobalErrorCallback(ErrorCallback callback);

        // Status
        size_t getActuatorCount() const;
        void getActuatorList(const char** actuatorIds, size_t maxActuators, size_t& count) const;

        // Diagnostics
        void printStatus() const;
        bool performHealthCheck();

    private:
        ActuatorManager() = default;
        static constexpr size_t MAX_ACTUATORS = 8;

        struct ActuatorEntry {
            IActuator* actuator;
            bool isActive;
            uint32_t lastUpdate;
            uint32_t errorCount;
        };

        ActuatorEntry actuators_[MAX_ACTUATORS];
        size_t actuatorCount_;
        CompletionCallback globalCompletionCallback_;
        ErrorCallback globalErrorCallback_;

        int findActuatorIndex(const char* actuatorId) const;
    };

    // Utility functions
    Command createLEDCommand(const Color& color, uint8_t brightness = 255,
                           Animation animation = Animation::NONE,
                           uint32_t duration = 0);

    Command createBuzzerCommand(SoundPattern pattern, uint16_t frequency = 1000,
                              uint8_t volume = 128, uint32_t duration = 1000);

    Command createDisplayCommand(const char* text, uint8_t x = 0, uint8_t y = 0,
                               bool clear = false, uint32_t duration = 0);

    const char* stateToString(State state);
    const char* typeToString(Type type);
    const char* animationToString(Animation animation);
    const char* soundPatternToString(SoundPattern pattern);
}
