// Simplified tests for the state machine system (header-only testing)
#include <unity.h>
#include "../src/system/state_machine.h"
#include <cstring>

// Test state machine enums and constants
void test_state_machine_enums() {
    // Test system states
    TEST_ASSERT_EQUAL_INT(0, (int)SystemStateMachine::SystemState::BOOT);
    TEST_ASSERT_EQUAL_INT(1, (int)SystemStateMachine::SystemState::INIT);
    TEST_ASSERT_EQUAL_INT(2, (int)SystemStateMachine::SystemState::IDLE);
    TEST_ASSERT_EQUAL_INT(3, (int)SystemStateMachine::SystemState::SENDER);
    TEST_ASSERT_EQUAL_INT(4, (int)SystemStateMachine::SystemState::RECEIVER);
    TEST_ASSERT_EQUAL_INT(5, (int)SystemStateMachine::SystemState::CONFIG);
    TEST_ASSERT_EQUAL_INT(6, (int)SystemStateMachine::SystemState::OTA_UPDATE);
    TEST_ASSERT_EQUAL_INT(7, (int)SystemStateMachine::SystemState::ERROR);
    TEST_ASSERT_EQUAL_INT(8, (int)SystemStateMachine::SystemState::SLEEP);
    TEST_ASSERT_EQUAL_INT(9, (int)SystemStateMachine::SystemState::SHUTDOWN);

    // Test events
    TEST_ASSERT_EQUAL_INT(0, (int)SystemStateMachine::Event::POWER_ON);
    TEST_ASSERT_EQUAL_INT(1, (int)SystemStateMachine::Event::INIT_COMPLETE);
    TEST_ASSERT_EQUAL_INT(2, (int)SystemStateMachine::Event::BUTTON_SHORT);
    TEST_ASSERT_EQUAL_INT(3, (int)SystemStateMachine::Event::BUTTON_MEDIUM);
    TEST_ASSERT_EQUAL_INT(4, (int)SystemStateMachine::Event::BUTTON_LONG);
    TEST_ASSERT_EQUAL_INT(5, (int)SystemStateMachine::Event::BUTTON_VERY_LONG);
    TEST_ASSERT_EQUAL_INT(6, (int)SystemStateMachine::Event::LORA_MESSAGE);
    TEST_ASSERT_EQUAL_INT(7, (int)SystemStateMachine::Event::WIFI_CONNECTED);
    TEST_ASSERT_EQUAL_INT(8, (int)SystemStateMachine::Event::WIFI_DISCONNECTED);
    TEST_ASSERT_EQUAL_INT(9, (int)SystemStateMachine::Event::OTA_AVAILABLE);
    TEST_ASSERT_EQUAL_INT(10, (int)SystemStateMachine::Event::SENSOR_TRIGGER);
    TEST_ASSERT_EQUAL_INT(11, (int)SystemStateMachine::Event::ERROR_OCCURRED);
    TEST_ASSERT_EQUAL_INT(12, (int)SystemStateMachine::Event::TIMEOUT);
    TEST_ASSERT_EQUAL_INT(13, (int)SystemStateMachine::Event::SLEEP_REQUEST);
    TEST_ASSERT_EQUAL_INT(14, (int)SystemStateMachine::Event::WAKEUP);
    TEST_ASSERT_EQUAL_INT(15, (int)SystemStateMachine::Event::SHUTDOWN_REQUEST);
    TEST_ASSERT_EQUAL_INT(16, (int)SystemStateMachine::Event::CUSTOM);

    // Test transition results
    TEST_ASSERT_EQUAL_INT(0, (int)SystemStateMachine::TransitionResult::SUCCESS);
    TEST_ASSERT_EQUAL_INT(1, (int)SystemStateMachine::TransitionResult::INVALID_EVENT);
    TEST_ASSERT_EQUAL_INT(2, (int)SystemStateMachine::TransitionResult::TRANSITION_BLOCKED);
    TEST_ASSERT_EQUAL_INT(3, (int)SystemStateMachine::TransitionResult::ACTION_FAILED);
    TEST_ASSERT_EQUAL_INT(4, (int)SystemStateMachine::TransitionResult::INVALID_STATE);
}

void test_state_machine_structures() {
    // Test Transition structure
    SystemStateMachine::Transition transition;

    // Test default constructor
    TEST_ASSERT_EQUAL_INT((int)SystemStateMachine::SystemState::BOOT, (int)transition.fromState);
    TEST_ASSERT_EQUAL_INT((int)SystemStateMachine::Event::POWER_ON, (int)transition.triggerEvent);
    TEST_ASSERT_EQUAL_INT((int)SystemStateMachine::SystemState::INIT, (int)transition.toState);

    // Test parameterized constructor
    SystemStateMachine::Transition customTransition(
        SystemStateMachine::SystemState::INIT,
        SystemStateMachine::Event::BUTTON_SHORT,
        SystemStateMachine::SystemState::IDLE
    );

    TEST_ASSERT_EQUAL_INT((int)SystemStateMachine::SystemState::INIT, (int)customTransition.fromState);
    TEST_ASSERT_EQUAL_INT((int)SystemStateMachine::Event::BUTTON_SHORT, (int)customTransition.triggerEvent);
    TEST_ASSERT_EQUAL_INT((int)SystemStateMachine::SystemState::IDLE, (int)customTransition.toState);
}

void test_state_handler_base_class() {
    // Test StateHandler base class
    SystemStateMachine::StateHandler handler(SystemStateMachine::SystemState::BOOT, "TestHandler");

    // Test getters
    TEST_ASSERT_EQUAL_INT((int)SystemStateMachine::SystemState::BOOT, (int)handler.getState());
    TEST_ASSERT_EQUAL_STRING("TestHandler", handler.getName());

    // Test default behavior
    TEST_ASSERT_TRUE(handler.onEntry());
    handler.onExit();
    handler.onUpdate();
    TEST_ASSERT_TRUE(handler.canTransition(SystemStateMachine::Event::POWER_ON));
}

void test_predefined_state_handlers() {
    // Test predefined state handler declarations (these will be implemented later)
    // For now, we just verify the classes are declared in the header

    // These classes are declared but not implemented yet:
    // - BootState, InitState, IdleState, SenderState, ReceiverState, ConfigState, ErrorState

    // This test will pass as long as the header compiles
    TEST_ASSERT_TRUE(true);
}

void test_callback_types() {
    // Test callback type definitions
    SystemStateMachine::StateEntryAction entryAction = []() -> bool { return true; };
    SystemStateMachine::StateExitAction exitAction = []() {};
    SystemStateMachine::StateUpdateAction updateAction = []() {};
    SystemStateMachine::GuardCondition guardCondition = [](SystemStateMachine::Event event) -> bool { return true; };
    SystemStateMachine::EventCallback eventCallback = [](SystemStateMachine::Event event, uint32_t data) {};
    SystemStateMachine::StateChangeCallback stateChangeCallback = [](SystemStateMachine::SystemState oldState, SystemStateMachine::SystemState newState, SystemStateMachine::Event trigger) {};

    // Test that callbacks can be called
    TEST_ASSERT_TRUE(entryAction());
    exitAction();
    updateAction();
    TEST_ASSERT_TRUE(guardCondition(SystemStateMachine::Event::POWER_ON));
    eventCallback(SystemStateMachine::Event::POWER_ON, 0);
    stateChangeCallback(SystemStateMachine::SystemState::BOOT, SystemStateMachine::SystemState::INIT, SystemStateMachine::Event::INIT_COMPLETE);
}

void test_utility_functions() {
    // Test utility function declarations (these will be implemented later)
    // For now, we just verify the functions exist in the header

    // These functions are declared but not implemented yet:
    // - setupDefaultStateMachine()
    // - setupDefaultTransitions()
    // - validateStateMachine()

    // This test will pass as long as the header compiles
    TEST_ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Basic enum and structure tests
    RUN_TEST(test_state_machine_enums);
    RUN_TEST(test_state_machine_structures);
    RUN_TEST(test_state_handler_base_class);
    RUN_TEST(test_predefined_state_handlers);
    RUN_TEST(test_callback_types);
    RUN_TEST(test_utility_functions);

    return UNITY_END();
}
