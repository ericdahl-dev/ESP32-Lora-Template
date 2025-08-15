#include <unity.h>
#include "../src/config/system_config.h"
#include "../src/system/error_handler.h"
#include "../src/system/logger.h"
#include "../src/sensors/sensor_interface.h"
#include "../src/actuators/actuator_interface.h"
#include "../src/communication/communication_interface.h"
#include "../src/system/state_machine.h"

// Test configuration constants and pin definitions
void test_system_config_pins() {
    // Test hardware pin definitions
    TEST_ASSERT_EQUAL_UINT8(36, SystemConfig::Pins::VEXT);
    TEST_ASSERT_EQUAL_UINT8(21, SystemConfig::Pins::OLED_RST);
    TEST_ASSERT_EQUAL_UINT8(0, SystemConfig::Pins::BUTTON);
    TEST_ASSERT_EQUAL_UINT8(17, SystemConfig::Pins::I2C_SDA);
    TEST_ASSERT_EQUAL_UINT8(18, SystemConfig::Pins::I2C_SCL);

    // Test LoRa pin definitions
    TEST_ASSERT_EQUAL_UINT8(8, SystemConfig::Pins::LORA_NSS);
    TEST_ASSERT_EQUAL_UINT8(14, SystemConfig::Pins::LORA_DIO1);
    TEST_ASSERT_EQUAL_UINT8(12, SystemConfig::Pins::LORA_RST);
    TEST_ASSERT_EQUAL_UINT8(13, SystemConfig::Pins::LORA_BUSY);

    // Test future expansion pins
    TEST_ASSERT_EQUAL_UINT8(4, SystemConfig::Pins::LIGHTNING_IRQ);
    TEST_ASSERT_EQUAL_UINT8(5, SystemConfig::Pins::LIGHTNING_CS);
    TEST_ASSERT_EQUAL_UINT8(2, SystemConfig::Pins::LED_DATA);
    TEST_ASSERT_EQUAL_UINT8(3, SystemConfig::Pins::BUZZER);
}

void test_system_config_lora() {
    // Test LoRa configuration defaults
    TEST_ASSERT_EQUAL_FLOAT(915.0f, SystemConfig::LoRa::DEFAULT_FREQ_MHZ);
    TEST_ASSERT_EQUAL_FLOAT(125.0f, SystemConfig::LoRa::DEFAULT_BW_KHZ);
    TEST_ASSERT_EQUAL_UINT8(9, SystemConfig::LoRa::DEFAULT_SF);
    TEST_ASSERT_EQUAL_UINT8(5, SystemConfig::LoRa::DEFAULT_CR);
    TEST_ASSERT_EQUAL_INT8(17, SystemConfig::LoRa::DEFAULT_TX_DBM);

    // Test LoRa ranges
    TEST_ASSERT_EQUAL_UINT8(7, SystemConfig::LoRa::SF_MIN);
    TEST_ASSERT_EQUAL_UINT8(12, SystemConfig::LoRa::SF_MAX);
    TEST_ASSERT_EQUAL_UINT32(4, SystemConfig::LoRa::BW_OPTIONS_COUNT);

    // Test bandwidth options
    TEST_ASSERT_EQUAL_FLOAT(62.5f, SystemConfig::LoRa::BW_OPTIONS[0]);
    TEST_ASSERT_EQUAL_FLOAT(125.0f, SystemConfig::LoRa::BW_OPTIONS[1]);
    TEST_ASSERT_EQUAL_FLOAT(250.0f, SystemConfig::LoRa::BW_OPTIONS[2]);
    TEST_ASSERT_EQUAL_FLOAT(500.0f, SystemConfig::LoRa::BW_OPTIONS[3]);
}

void test_system_config_timing() {
    // Test button timing
    TEST_ASSERT_EQUAL_UINT32(50, SystemConfig::Button::DEBOUNCE_MS);
    TEST_ASSERT_EQUAL_UINT32(100, SystemConfig::Button::SHORT_PRESS_MS);
    TEST_ASSERT_EQUAL_UINT32(1000, SystemConfig::Button::MEDIUM_PRESS_MS);
    TEST_ASSERT_EQUAL_UINT32(3000, SystemConfig::Button::LONG_PRESS_MS);

    // Test system timing
    TEST_ASSERT_EQUAL_UINT32(10, SystemConfig::Timing::MAIN_LOOP_INTERVAL_MS);
    TEST_ASSERT_EQUAL_UINT32(1000, SystemConfig::Timing::STATUS_UPDATE_INTERVAL_MS);
    TEST_ASSERT_EQUAL_UINT32(2000, SystemConfig::Timing::PING_INTERVAL_MS);
}

void test_error_handler_enums() {
    // Test error severity levels
    TEST_ASSERT_EQUAL_INT(0, (int)ErrorHandling::Severity::INFO);
    TEST_ASSERT_EQUAL_INT(1, (int)ErrorHandling::Severity::WARNING);
    TEST_ASSERT_EQUAL_INT(2, (int)ErrorHandling::Severity::ERROR);
    TEST_ASSERT_EQUAL_INT(3, (int)ErrorHandling::Severity::CRITICAL);

    // Test error categories
    TEST_ASSERT_EQUAL_INT(0, (int)ErrorHandling::Category::HARDWARE);
    TEST_ASSERT_EQUAL_INT(1, (int)ErrorHandling::Category::RADIO);
    TEST_ASSERT_EQUAL_INT(2, (int)ErrorHandling::Category::WIFI);
    TEST_ASSERT_EQUAL_INT(3, (int)ErrorHandling::Category::SENSOR);
    TEST_ASSERT_EQUAL_INT(4, (int)ErrorHandling::Category::ACTUATOR);

    // Test some error codes
    TEST_ASSERT_EQUAL_INT(100, (int)ErrorHandling::Code::OLED_INIT_FAILED);
    TEST_ASSERT_EQUAL_INT(200, (int)ErrorHandling::Code::RADIO_INIT_FAILED);
    TEST_ASSERT_EQUAL_INT(300, (int)ErrorHandling::Code::WIFI_CONNECT_FAILED);
    TEST_ASSERT_EQUAL_INT(400, (int)ErrorHandling::Code::SENSOR_INIT_FAILED);
    TEST_ASSERT_EQUAL_INT(500, (int)ErrorHandling::Code::LED_INIT_FAILED);
}

void test_logger_enums() {
    // Test log levels
    TEST_ASSERT_EQUAL_INT(0, (int)Logging::Level::TRACE);
    TEST_ASSERT_EQUAL_INT(1, (int)Logging::Level::DEBUG);
    TEST_ASSERT_EQUAL_INT(2, (int)Logging::Level::INFO);
    TEST_ASSERT_EQUAL_INT(3, (int)Logging::Level::WARN);
    TEST_ASSERT_EQUAL_INT(4, (int)Logging::Level::ERROR);
    TEST_ASSERT_EQUAL_INT(5, (int)Logging::Level::FATAL);

    // Test log categories
    TEST_ASSERT_EQUAL_INT(0, (int)Logging::Category::SYSTEM);
    TEST_ASSERT_EQUAL_INT(1, (int)Logging::Category::HARDWARE);
    TEST_ASSERT_EQUAL_INT(2, (int)Logging::Category::RADIO);

    // Test log destinations as bitmask
    TEST_ASSERT_EQUAL_INT(1, (int)Logging::Destination::SERIAL);
    TEST_ASSERT_EQUAL_INT(2, (int)Logging::Destination::DISPLAY);
    TEST_ASSERT_EQUAL_INT(4, (int)Logging::Destination::RADIO);
    TEST_ASSERT_EQUAL_INT(8, (int)Logging::Destination::STORAGE);
}

void test_sensor_interface_enums() {
    // Test sensor states
    TEST_ASSERT_EQUAL_INT(0, (int)SensorSystem::State::UNINITIALIZED);
    TEST_ASSERT_EQUAL_INT(1, (int)SensorSystem::State::INITIALIZING);
    TEST_ASSERT_EQUAL_INT(2, (int)SensorSystem::State::READY);
    TEST_ASSERT_EQUAL_INT(3, (int)SensorSystem::State::READING);
    TEST_ASSERT_EQUAL_INT(4, (int)SensorSystem::State::ERROR);
    TEST_ASSERT_EQUAL_INT(5, (int)SensorSystem::State::DISABLED);

    // Test data types
    TEST_ASSERT_EQUAL_INT(0, (int)SensorSystem::DataType::BOOLEAN);
    TEST_ASSERT_EQUAL_INT(1, (int)SensorSystem::DataType::INTEGER);
    TEST_ASSERT_EQUAL_INT(2, (int)SensorSystem::DataType::FLOAT);
    TEST_ASSERT_EQUAL_INT(3, (int)SensorSystem::DataType::STRING);
    TEST_ASSERT_EQUAL_INT(4, (int)SensorSystem::DataType::BINARY);

    // Test capabilities as bitmask
    TEST_ASSERT_EQUAL_INT(0, (int)SensorSystem::Capability::NONE);
    TEST_ASSERT_EQUAL_INT(1, (int)SensorSystem::Capability::INTERRUPT_CAPABLE);
    TEST_ASSERT_EQUAL_INT(2, (int)SensorSystem::Capability::CONFIGURABLE);
    TEST_ASSERT_EQUAL_INT(4, (int)SensorSystem::Capability::SELF_TEST);
    TEST_ASSERT_EQUAL_INT(8, (int)SensorSystem::Capability::CALIBRATION);
}

void test_actuator_interface_enums() {
    // Test actuator states
    TEST_ASSERT_EQUAL_INT(0, (int)ActuatorSystem::State::UNINITIALIZED);
    TEST_ASSERT_EQUAL_INT(1, (int)ActuatorSystem::State::INITIALIZING);
    TEST_ASSERT_EQUAL_INT(2, (int)ActuatorSystem::State::READY);
    TEST_ASSERT_EQUAL_INT(3, (int)ActuatorSystem::State::ACTIVE);
    TEST_ASSERT_EQUAL_INT(4, (int)ActuatorSystem::State::ERROR);
    TEST_ASSERT_EQUAL_INT(5, (int)ActuatorSystem::State::DISABLED);

    // Test actuator types
    TEST_ASSERT_EQUAL_INT(0, (int)ActuatorSystem::Type::LED_STRIP);
    TEST_ASSERT_EQUAL_INT(1, (int)ActuatorSystem::Type::LED_SINGLE);
    TEST_ASSERT_EQUAL_INT(2, (int)ActuatorSystem::Type::BUZZER);
    TEST_ASSERT_EQUAL_INT(3, (int)ActuatorSystem::Type::DISPLAY);

    // Test animations
    TEST_ASSERT_EQUAL_INT(0, (int)ActuatorSystem::Animation::NONE);
    TEST_ASSERT_EQUAL_INT(1, (int)ActuatorSystem::Animation::FADE);
    TEST_ASSERT_EQUAL_INT(2, (int)ActuatorSystem::Animation::BLINK);
    TEST_ASSERT_EQUAL_INT(3, (int)ActuatorSystem::Animation::PULSE);

    // Test sound patterns
    TEST_ASSERT_EQUAL_INT(0, (int)ActuatorSystem::SoundPattern::NONE);
    TEST_ASSERT_EQUAL_INT(1, (int)ActuatorSystem::SoundPattern::BEEP);
    TEST_ASSERT_EQUAL_INT(2, (int)ActuatorSystem::SoundPattern::DOUBLE_BEEP);
}

void test_color_structure() {
    // Test Color constructor
    ActuatorSystem::Color black;
    TEST_ASSERT_EQUAL_UINT8(0, black.red);
    TEST_ASSERT_EQUAL_UINT8(0, black.green);
    TEST_ASSERT_EQUAL_UINT8(0, black.blue);
    TEST_ASSERT_EQUAL_UINT8(0, black.white);

    // Test Color with values
    ActuatorSystem::Color red(255, 0, 0);
    TEST_ASSERT_EQUAL_UINT8(255, red.red);
    TEST_ASSERT_EQUAL_UINT8(0, red.green);
    TEST_ASSERT_EQUAL_UINT8(0, red.blue);

    // Test predefined colors
    TEST_ASSERT_EQUAL_UINT8(255, ActuatorSystem::Colors::WHITE.red);
    TEST_ASSERT_EQUAL_UINT8(255, ActuatorSystem::Colors::WHITE.green);
    TEST_ASSERT_EQUAL_UINT8(255, ActuatorSystem::Colors::WHITE.blue);

    TEST_ASSERT_EQUAL_UINT8(255, ActuatorSystem::Colors::RED.red);
    TEST_ASSERT_EQUAL_UINT8(0, ActuatorSystem::Colors::RED.green);
    TEST_ASSERT_EQUAL_UINT8(0, ActuatorSystem::Colors::RED.blue);
}

void test_communication_interface_enums() {
    // Test communication channels
    TEST_ASSERT_EQUAL_INT(0, (int)CommunicationSystem::Channel::LORA_DATA);
    TEST_ASSERT_EQUAL_INT(1, (int)CommunicationSystem::Channel::LORA_CONTROL);
    TEST_ASSERT_EQUAL_INT(2, (int)CommunicationSystem::Channel::WIFI);
    TEST_ASSERT_EQUAL_INT(3, (int)CommunicationSystem::Channel::SERIAL);

    // Test message types
    TEST_ASSERT_EQUAL_INT(0, (int)CommunicationSystem::MessageType::PING);
    TEST_ASSERT_EQUAL_INT(1, (int)CommunicationSystem::MessageType::PONG);
    TEST_ASSERT_EQUAL_INT(2, (int)CommunicationSystem::MessageType::CONFIG);
    TEST_ASSERT_EQUAL_INT(3, (int)CommunicationSystem::MessageType::SENSOR_DATA);

    // Test priority levels
    TEST_ASSERT_EQUAL_INT(0, (int)CommunicationSystem::Priority::LOW);
    TEST_ASSERT_EQUAL_INT(1, (int)CommunicationSystem::Priority::NORMAL);
    TEST_ASSERT_EQUAL_INT(2, (int)CommunicationSystem::Priority::HIGH);
    TEST_ASSERT_EQUAL_INT(3, (int)CommunicationSystem::Priority::CRITICAL);
}

void test_state_machine_enums() {
    // Test system states
    TEST_ASSERT_EQUAL_INT(0, (int)SystemStateMachine::SystemState::BOOT);
    TEST_ASSERT_EQUAL_INT(1, (int)SystemStateMachine::SystemState::INIT);
    TEST_ASSERT_EQUAL_INT(2, (int)SystemStateMachine::SystemState::IDLE);
    TEST_ASSERT_EQUAL_INT(3, (int)SystemStateMachine::SystemState::SENDER);
    TEST_ASSERT_EQUAL_INT(4, (int)SystemStateMachine::SystemState::RECEIVER);
    TEST_ASSERT_EQUAL_INT(5, (int)SystemStateMachine::SystemState::CONFIG);

    // Test events
    TEST_ASSERT_EQUAL_INT(0, (int)SystemStateMachine::Event::POWER_ON);
    TEST_ASSERT_EQUAL_INT(1, (int)SystemStateMachine::Event::INIT_COMPLETE);
    TEST_ASSERT_EQUAL_INT(2, (int)SystemStateMachine::Event::BUTTON_SHORT);
    TEST_ASSERT_EQUAL_INT(3, (int)SystemStateMachine::Event::BUTTON_MEDIUM);
    TEST_ASSERT_EQUAL_INT(4, (int)SystemStateMachine::Event::BUTTON_LONG);

    // Test transition results
    TEST_ASSERT_EQUAL_INT(0, (int)SystemStateMachine::TransitionResult::SUCCESS);
    TEST_ASSERT_EQUAL_INT(1, (int)SystemStateMachine::TransitionResult::INVALID_EVENT);
    TEST_ASSERT_EQUAL_INT(2, (int)SystemStateMachine::TransitionResult::TRANSITION_BLOCKED);
}

void test_message_header_structure() {
    // Test MessageHeader size and alignment
    CommunicationSystem::MessageHeader header = {};

    // Initialize header fields
    header.messageId = 0x12345678;
    header.type = CommunicationSystem::MessageType::PING;
    header.priority = CommunicationSystem::Priority::NORMAL;
    header.sourceId = 0x01;
    header.destinationId = 0xFF;
    header.sequenceNumber = 100;
    header.payloadSize = 0;
    header.timestamp = 123456789;
    header.checksum = 0xABCD;

    // Verify fields are set correctly
    TEST_ASSERT_EQUAL_UINT32(0x12345678, header.messageId);
    TEST_ASSERT_EQUAL_INT((int)CommunicationSystem::MessageType::PING, (int)header.type);
    TEST_ASSERT_EQUAL_INT((int)CommunicationSystem::Priority::NORMAL, (int)header.priority);
    TEST_ASSERT_EQUAL_UINT8(0x01, header.sourceId);
    TEST_ASSERT_EQUAL_UINT8(0xFF, header.destinationId);
    TEST_ASSERT_EQUAL_UINT16(100, header.sequenceNumber);
    TEST_ASSERT_EQUAL_UINT16(0, header.payloadSize);
    TEST_ASSERT_EQUAL_UINT32(123456789, header.timestamp);
    TEST_ASSERT_EQUAL_UINT16(0xABCD, header.checksum);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Configuration tests
    RUN_TEST(test_system_config_pins);
    RUN_TEST(test_system_config_lora);
    RUN_TEST(test_system_config_timing);

    // Error handling tests
    RUN_TEST(test_error_handler_enums);

    // Logging tests
    RUN_TEST(test_logger_enums);

    // Sensor framework tests
    RUN_TEST(test_sensor_interface_enums);

    // Actuator framework tests
    RUN_TEST(test_actuator_interface_enums);
    RUN_TEST(test_color_structure);

    // Communication framework tests
    RUN_TEST(test_communication_interface_enums);
    RUN_TEST(test_message_header_structure);

    // State machine tests
    RUN_TEST(test_state_machine_enums);

    return UNITY_END();
}
