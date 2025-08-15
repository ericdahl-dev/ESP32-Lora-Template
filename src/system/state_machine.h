#pragma once

#include <stdint.h>
#include <functional>

// State machine framework for better control flow and mode management
namespace SystemStateMachine {

    // System states
    enum class SystemState {
        BOOT,           // System is booting up
        INIT,           // Initializing components
        IDLE,           // System is idle, waiting for input
        SENDER,         // Acting as LoRa sender
        RECEIVER,       // Acting as LoRa receiver
        CONFIG,         // Configuration mode
        OTA_UPDATE,     // Firmware update mode
        ERROR,          // Error state
        SLEEP,          // Low power sleep mode
        SHUTDOWN        // Shutting down
    };

    // System events
    enum class Event {
        POWER_ON,           // System powered on
        INIT_COMPLETE,      // Initialization completed
        BUTTON_SHORT,       // Short button press
        BUTTON_MEDIUM,      // Medium button press
        BUTTON_LONG,        // Long button press
        BUTTON_VERY_LONG,   // Very long button press (>5s)
        LORA_MESSAGE,       // LoRa message received
        WIFI_CONNECTED,     // WiFi connection established
        WIFI_DISCONNECTED,  // WiFi connection lost
        OTA_AVAILABLE,      // OTA update available
        SENSOR_TRIGGER,     // Sensor triggered (lightning, etc.)
        ERROR_OCCURRED,     // Error condition detected
        TIMEOUT,            // Timeout expired
        SLEEP_REQUEST,      // Sleep mode requested
        WAKEUP,             // Wake up from sleep
        SHUTDOWN_REQUEST,   // Shutdown requested
        CUSTOM              // Custom application event
    };

    // State transition result
    enum class TransitionResult {
        SUCCESS,            // Transition successful
        INVALID_EVENT,      // Event not valid for current state
        TRANSITION_BLOCKED, // Transition blocked by guard condition
        ACTION_FAILED,      // State action failed
        INVALID_STATE       // Invalid target state
    };

    // Forward declarations
    class StateHandler;
    class StateMachineEngine;

    // State action callbacks
    typedef std::function<bool()> StateEntryAction;    // Called when entering state
    typedef std::function<void()> StateExitAction;     // Called when exiting state
    typedef std::function<void()> StateUpdateAction;   // Called periodically while in state
    typedef std::function<bool(Event)> GuardCondition; // Guard condition for transitions

    // Event callback
    typedef std::function<void(Event, uint32_t data)> EventCallback;

    // State change callback
    typedef std::function<void(SystemState oldState, SystemState newState, Event trigger)> StateChangeCallback;

        // State transition definition
    struct Transition {
        SystemState fromState;
        Event triggerEvent;
        SystemState toState;
        GuardCondition guard;       // Optional guard condition
        StateEntryAction action;    // Optional transition action

        Transition() : fromState(SystemState::BOOT), triggerEvent(Event::POWER_ON),
                      toState(SystemState::INIT) {}

        Transition(SystemState from, Event event, SystemState to,
                  GuardCondition guardFn = nullptr, StateEntryAction actionFn = nullptr)
            : fromState(from), triggerEvent(event), toState(to), guard(guardFn), action(actionFn) {}
    };

    // State handler class
    class StateHandler {
    public:
        StateHandler(SystemState state, const char* name)
            : state_(state), name_(name) {}

        virtual ~StateHandler() = default;

        // State lifecycle methods
        virtual bool onEntry() { return true; }    // Override for entry actions
        virtual void onExit() {}                   // Override for exit actions
        virtual void onUpdate() {}                 // Override for periodic updates
        virtual bool canTransition(Event event) { return true; } // Override for guards

        // Getters
        SystemState getState() const { return state_; }
        const char* getName() const { return name_; }

        // Action setters
        void setEntryAction(StateEntryAction action) { entryAction_ = action; }
        void setExitAction(StateExitAction action) { exitAction_ = action; }
        void setUpdateAction(StateUpdateAction action) { updateAction_ = action; }

    protected:
        SystemState state_;
        const char* name_;
        StateEntryAction entryAction_;
        StateExitAction exitAction_;
        StateUpdateAction updateAction_;

        friend class StateMachineEngine;
    };

    // State machine engine
    class StateMachineEngine {
    public:
        static StateMachineEngine& getInstance();

        // State machine management
        bool initialize();
        void deinitialize();
        bool isInitialized() const { return initialized_; }

        // State management
        bool registerState(StateHandler* handler);
        bool unregisterState(SystemState state);
        StateHandler* getStateHandler(SystemState state);

        // Transition management
        bool addTransition(const Transition& transition);
        bool removeTransition(SystemState fromState, Event event);
        bool canTransition(SystemState fromState, Event event) const;

        // State machine operations
        bool start(SystemState initialState = SystemState::BOOT);
        bool stop();
        TransitionResult processEvent(Event event, uint32_t data = 0);
        void update();  // Call periodically from main loop

        // State queries
        SystemState getCurrentState() const { return currentState_; }
        SystemState getPreviousState() const { return previousState_; }
        const char* getCurrentStateName() const;
        uint32_t getTimeInCurrentState() const;
        uint32_t getStateChangeCount() const { return stateChangeCount_; }

        // Event handling
        bool postEvent(Event event, uint32_t data = 0);
        bool postDelayedEvent(Event event, uint32_t delayMs, uint32_t data = 0);
        void clearDelayedEvents();

        // Callbacks
        void setEventCallback(EventCallback callback) { eventCallback_ = callback; }
        void setStateChangeCallback(StateChangeCallback callback) { stateChangeCallback_ = callback; }

        // Timeouts
        bool setTimeout(uint32_t timeoutMs);
        void clearTimeout();
        bool hasTimeout() const { return timeoutSet_; }

        // Diagnostics
        void printCurrentState() const;
        void printTransitionTable() const;
        void printStatistics() const;

        // Utility
        const char* stateToString(SystemState state) const;
        const char* eventToString(Event event) const;
        const char* resultToString(TransitionResult result) const;

    private:
        StateMachineEngine() : initialized_(false), running_(false), currentState_(SystemState::BOOT) {}

        static constexpr size_t MAX_STATES = 16;
        static constexpr size_t MAX_TRANSITIONS = 32;
        static constexpr size_t MAX_DELAYED_EVENTS = 8;

        // State storage
        struct StateEntry {
            StateHandler* handler;
            bool isActive;
        };

        StateEntry states_[MAX_STATES];
        size_t stateCount_;

        // Transition storage
        Transition transitions_[MAX_TRANSITIONS];
        size_t transitionCount_;

        // Delayed event storage
        struct DelayedEvent {
            Event event;
            uint32_t data;
            uint32_t triggerTime;
            bool active;
        };

        DelayedEvent delayedEvents_[MAX_DELAYED_EVENTS];

        // State machine state
        bool initialized_;
        bool running_;
        SystemState currentState_;
        SystemState previousState_;
        uint32_t stateEntryTime_;
        uint32_t stateChangeCount_;

        // Timeout management
        bool timeoutSet_;
        uint32_t timeoutTime_;

        // Callbacks
        EventCallback eventCallback_;
        StateChangeCallback stateChangeCallback_;

        // Helper methods
        int findStateIndex(SystemState state) const;
        int findTransitionIndex(SystemState fromState, Event event) const;
        int findDelayedEventSlot() const;
        void processDelayedEvents();
        TransitionResult executeTransition(const Transition& transition, uint32_t eventData);
    };

    // Predefined state handlers
    class BootState : public StateHandler {
    public:
        BootState() : StateHandler(SystemState::BOOT, "Boot") {}
        bool onEntry() override;
        void onUpdate() override;
    };

    class InitState : public StateHandler {
    public:
        InitState() : StateHandler(SystemState::INIT, "Initialize") {}
        bool onEntry() override;
        void onUpdate() override;
    };

    class IdleState : public StateHandler {
    public:
        IdleState() : StateHandler(SystemState::IDLE, "Idle") {}
        bool onEntry() override;
        void onUpdate() override;
    };

    class SenderState : public StateHandler {
    public:
        SenderState() : StateHandler(SystemState::SENDER, "Sender") {}
        bool onEntry() override;
        void onExit() override;
        void onUpdate() override;
    };

    class ReceiverState : public StateHandler {
    public:
        ReceiverState() : StateHandler(SystemState::RECEIVER, "Receiver") {}
        bool onEntry() override;
        void onExit() override;
        void onUpdate() override;
    };

    class ConfigState : public StateHandler {
    public:
        ConfigState() : StateHandler(SystemState::CONFIG, "Config") {}
        bool onEntry() override;
        void onUpdate() override;
    };

    class ErrorState : public StateHandler {
    public:
        ErrorState() : StateHandler(SystemState::ERROR, "Error") {}
        bool onEntry() override;
        void onUpdate() override;
        bool canTransition(Event event) override;
    };

    // Utility functions
    void setupDefaultStateMachine();
    void setupDefaultTransitions();
    bool validateStateMachine();
}
