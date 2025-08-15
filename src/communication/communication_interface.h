#pragma once

#include <stdint.h>
#include <functional>

// Unified communication framework for LoRa, WiFi, Serial, etc.
namespace CommunicationSystem {

    // Communication channels
    enum class Channel {
        LORA_DATA,      // LoRa data channel
        LORA_CONTROL,   // LoRa control channel
        WIFI,           // WiFi communication
        SERIAL,         // Serial/USB communication
        BLUETOOTH,      // Bluetooth (future)
        USB,            // USB communication (future)
        CUSTOM          // Custom protocol
    };

    // Message types
    enum class MessageType {
        PING,           // Heartbeat/ping message
        PONG,           // Ping response
        CONFIG,         // Configuration data
        SENSOR_DATA,    // Sensor readings
        COMMAND,        // Control commands
        STATUS,         // Status updates
        ERROR,          // Error reports
        OTA_REQUEST,    // OTA update request
        OTA_DATA,       // OTA firmware data
        CUSTOM          // Custom message type
    };

    // Message priority levels
    enum class Priority {
        LOW = 0,        // Non-critical messages
        NORMAL = 1,     // Standard messages
        HIGH = 2,       // Important messages
        CRITICAL = 3    // Critical system messages
    };

    // Communication states
    enum class State {
        UNINITIALIZED,
        INITIALIZING,
        READY,
        TRANSMITTING,
        RECEIVING,
        ERROR,
        DISABLED
    };

    // Message header structure
    struct MessageHeader {
        uint32_t messageId;     // Unique message identifier
        MessageType type;       // Message type
        Priority priority;      // Message priority
        uint8_t sourceId;       // Source device ID
        uint8_t destinationId;  // Destination device ID (0xFF = broadcast)
        uint16_t sequenceNumber; // Sequence number for ordering
        uint16_t payloadSize;   // Size of payload data
        uint32_t timestamp;     // Message timestamp
        uint16_t checksum;      // Header checksum
    };

    // Complete message structure
    struct Message {
        MessageHeader header;
        uint8_t* payload;       // Message payload
        bool ownsPayload;       // Whether this message owns the payload memory

        Message() : payload(nullptr), ownsPayload(false) {}
        ~Message() { if (ownsPayload && payload) delete[] payload; }

        // Copy constructor and assignment operator
        Message(const Message& other);
        Message& operator=(const Message& other);

        // Move constructor and assignment operator
        Message(Message&& other) noexcept;
        Message& operator=(Message&& other) noexcept;
    };

    // Communication statistics
    struct Statistics {
        uint32_t messagesSent;
        uint32_t messagesReceived;
        uint32_t bytesTransmitted;
        uint32_t bytesReceived;
        uint32_t errorsTransmit;
        uint32_t errorsReceive;
        uint32_t messagesDropped;
        float averageRSSI;
        float averageSNR;
        uint32_t lastActivity;
    };

    // Communication callback types
    typedef std::function<void(const Message&)> MessageCallback;
    typedef std::function<void(Channel channel, uint32_t errorCode)> ErrorCallback;
    typedef std::function<void(Channel channel, State oldState, State newState)> StateChangeCallback;
    typedef std::function<bool(const Message&)> MessageFilterCallback;

    // Base communication interface
    class ICommunication {
    public:
        virtual ~ICommunication() = default;

        // Basic operations
        virtual bool initialize() = 0;
        virtual bool deinitialize() = 0;
        virtual State getState() const = 0;
        virtual Channel getChannel() const = 0;
        virtual const char* getName() const = 0;

        // Message operations
        virtual bool sendMessage(const Message& message) = 0;
        virtual bool receiveMessage(Message& message) = 0;
        virtual bool hasMessages() const = 0;
        virtual size_t getMessageCount() const = 0;
        virtual bool clearMessages() = 0;

        // Configuration
        virtual bool setParameter(const char* name, const void* value, size_t size) = 0;
        virtual bool getParameter(const char* name, void* value, size_t& size) const = 0;
        virtual bool reset() = 0;

        // Callbacks
        virtual void setMessageCallback(MessageCallback callback) = 0;
        virtual void setErrorCallback(ErrorCallback callback) = 0;
        virtual void setStateChangeCallback(StateChangeCallback callback) = 0;
        virtual void setMessageFilter(MessageFilterCallback filter) = 0;

        // Statistics
        virtual void getStatistics(Statistics& stats) const = 0;
        virtual void resetStatistics() = 0;

        // Power management
        virtual bool sleep() = 0;
        virtual bool wakeup() = 0;

        // Utility
        virtual void update() = 0;  // Called periodically from main loop
        virtual uint32_t getLastError() const = 0;
        virtual const char* getErrorString(uint32_t errorCode) const = 0;
    };

    // Communication manager for handling multiple channels
    class CommunicationManager {
    public:
        static CommunicationManager& getInstance();

        // Channel management
        bool registerChannel(ICommunication* comm);
        bool unregisterChannel(Channel channel);
        ICommunication* getChannel(Channel channel);

        // Global operations
        bool initializeAll();
        void updateAll();
        void deinitializeAll();

        // Message routing
        bool sendMessage(const Message& message, Channel preferredChannel = Channel::LORA_DATA);
        bool broadcastMessage(const Message& message);
        bool routeMessage(const Message& message);

        // Convenience methods
        bool sendPing(uint8_t destinationId = 0xFF);
        bool sendConfig(const void* config, size_t size, uint8_t destinationId = 0xFF);
        bool sendSensorData(const void* data, size_t size, uint8_t destinationId = 0xFF);
        bool sendCommand(uint32_t command, const void* data = nullptr, size_t size = 0, uint8_t destinationId = 0xFF);
        bool sendStatus(uint32_t status, const void* data = nullptr, size_t size = 0);

        // Global callbacks
        void setGlobalMessageCallback(MessageCallback callback);
        void setGlobalErrorCallback(ErrorCallback callback);
        void setGlobalMessageFilter(MessageFilterCallback filter);

        // Device management
        void setDeviceId(uint8_t deviceId);
        uint8_t getDeviceId() const;
        void setSequenceNumber(uint16_t seqNum);
        uint16_t getNextSequenceNumber();

        // Statistics
        void getGlobalStatistics(Statistics& stats) const;
        void resetAllStatistics();

        // Status
        size_t getChannelCount() const;
        void getChannelList(Channel* channels, size_t maxChannels, size_t& count) const;

        // Diagnostics
        void printStatus() const;
        bool performHealthCheck();

    private:
        CommunicationManager() : deviceId_(1), sequenceNumber_(0) {}
        static constexpr size_t MAX_CHANNELS = 6;

        struct ChannelEntry {
            ICommunication* comm;
            bool isActive;
            uint32_t lastActivity;
            uint32_t errorCount;
        };

        ChannelEntry channels_[MAX_CHANNELS];
        size_t channelCount_;
        uint8_t deviceId_;
        uint16_t sequenceNumber_;

        MessageCallback globalMessageCallback_;
        ErrorCallback globalErrorCallback_;
        MessageFilterCallback globalMessageFilter_;

        int findChannelIndex(Channel channel) const;
        bool routeToChannel(const Message& message, Channel channel);
    };

    // Utility functions
    Message createMessage(MessageType type, const void* payload = nullptr, size_t payloadSize = 0);
    Message createPingMessage();
    Message createConfigMessage(const void* config, size_t size);
    Message createSensorDataMessage(const void* data, size_t size);
    Message createCommandMessage(uint32_t command, const void* data = nullptr, size_t size = 0);
    Message createStatusMessage(uint32_t status, const void* data = nullptr, size_t size = 0);

    bool validateMessage(const Message& message);
    uint16_t calculateChecksum(const MessageHeader& header);

    const char* channelToString(Channel channel);
    const char* messageTypeToString(MessageType type);
    const char* priorityToString(Priority priority);
    const char* stateToString(State state);
}
