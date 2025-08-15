// Integration tests for cross-module functionality
// Tests interactions between app_logic and other components
#include <unity.h>
#include "app_logic.h"
#include <cstring>

// Integration test scenarios
struct TestScenario {
    const char* name;
    uint32_t buttonPressDuration;
    ButtonAction expectedAction;
    const char* expectedBehavior;
};

// Define test scenarios that represent real usage patterns
static const TestScenario scenarios[] = {
    {"Quick tap", 50, ButtonAction::Ignore, "Should ignore accidental touches"},
    {"Short press", 200, ButtonAction::ToggleMode, "Should toggle between sender/receiver"},
    {"Medium press", 1500, ButtonAction::CycleSF, "Should cycle through spreading factors"},
    {"Long press", 5000, ButtonAction::CycleBW, "Should cycle through bandwidths"},
    {"Boundary short", 99, ButtonAction::Ignore, "Edge case for ignore threshold"},
    {"Boundary toggle", 100, ButtonAction::ToggleMode, "Edge case for toggle threshold"},
    {"Boundary SF", 1000, ButtonAction::CycleSF, "Edge case for SF threshold"},
    {"Boundary BW", 3000, ButtonAction::CycleBW, "Edge case for BW threshold"}
};

#define NUM_SCENARIOS (sizeof(scenarios) / sizeof(scenarios[0]))

// Mock configuration for testing interactions
struct MockSystemState {
    bool isSender;
    int currentSF;
    float currentBW;
    uint32_t messageSeq;
    char lastMessage[64];
};

static MockSystemState systemState = {true, 9, 125.0, 0, ""};

// Integration helper: Simulate button press and system response
void simulateButtonPressAndResponse(uint32_t pressDuration) {
    ButtonAction action = classifyPress(pressDuration);

    switch (action) {
        case ButtonAction::Ignore:
            // No state change
            break;

        case ButtonAction::ToggleMode:
            systemState.isSender = !systemState.isSender;
            strcpy(systemState.lastMessage, systemState.isSender ? "Switched to SENDER" : "Switched to RECEIVER");
            break;

        case ButtonAction::CycleSF:
            systemState.currentSF = cycleIndex(systemState.currentSF - 7, 6) + 7; // SF 7-12
            snprintf(systemState.lastMessage, sizeof(systemState.lastMessage), "SF changed to %d", systemState.currentSF);
            break;

        case ButtonAction::CycleBW:
            // Cycle through common bandwidths: 62.5, 125, 250, 500
            int bwIndex;
            if (systemState.currentBW == 62.5) bwIndex = 0;
            else if (systemState.currentBW == 125.0) bwIndex = 1;
            else if (systemState.currentBW == 250.0) bwIndex = 2;
            else if (systemState.currentBW == 500.0) bwIndex = 3;
            else bwIndex = 1; // Default to 125

            bwIndex = cycleIndex(bwIndex, 4);

            switch (bwIndex) {
                case 0: systemState.currentBW = 62.5; break;
                case 1: systemState.currentBW = 125.0; break;
                case 2: systemState.currentBW = 250.0; break;
                case 3: systemState.currentBW = 500.0; break;
            }
            snprintf(systemState.lastMessage, sizeof(systemState.lastMessage), "BW changed to %.1f kHz", systemState.currentBW);
            break;
    }
}

// Integration helper: Simulate message transmission
void simulateMessageTransmission() {
    if (systemState.isSender) {
        std::string message = formatTxMessage(systemState.messageSeq);
        strcpy(systemState.lastMessage, message.c_str());
        systemState.messageSeq++;
    }
}

// Reset system state for tests
void resetSystemState() {
    systemState.isSender = true;
    systemState.currentSF = 9;
    systemState.currentBW = 125.0;
    systemState.messageSeq = 0;
    strcpy(systemState.lastMessage, "");
}

// Test cases
void test_button_press_scenarios() {
    for (int i = 0; i < NUM_SCENARIOS; i++) {
        const TestScenario& scenario = scenarios[i];

        // Test the classification
        ButtonAction action = classifyPress(scenario.buttonPressDuration);
        TEST_ASSERT_EQUAL_MESSAGE((int)scenario.expectedAction, (int)action, scenario.name);

        // Verify the scenario description makes sense
        TEST_ASSERT_NOT_NULL_MESSAGE(scenario.expectedBehavior, scenario.name);
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, strlen(scenario.expectedBehavior), scenario.name);
    }
}

void test_system_state_integration() {
    resetSystemState();

    // Test initial state
    TEST_ASSERT_TRUE(systemState.isSender);
    TEST_ASSERT_EQUAL(9, systemState.currentSF);
    TEST_ASSERT_EQUAL_FLOAT(125.0, systemState.currentBW);
    TEST_ASSERT_EQUAL(0, systemState.messageSeq);

    // Test mode toggle
    simulateButtonPressAndResponse(200); // ToggleMode
    TEST_ASSERT_FALSE(systemState.isSender);
    TEST_ASSERT_EQUAL_STRING("Switched to RECEIVER", systemState.lastMessage);

    // Toggle back
    simulateButtonPressAndResponse(200);
    TEST_ASSERT_TRUE(systemState.isSender);
    TEST_ASSERT_EQUAL_STRING("Switched to SENDER", systemState.lastMessage);
}

void test_sf_cycling_integration() {
    resetSystemState();

    // Test SF cycling
    int initialSF = systemState.currentSF;
    simulateButtonPressAndResponse(1500); // CycleSF
    TEST_ASSERT_NOT_EQUAL(initialSF, systemState.currentSF);
    TEST_ASSERT_GREATER_OR_EQUAL(7, systemState.currentSF);
    TEST_ASSERT_LESS_OR_EQUAL(12, systemState.currentSF);

    // Test multiple cycles
    int sf1 = systemState.currentSF;
    simulateButtonPressAndResponse(1500);
    int sf2 = systemState.currentSF;
    simulateButtonPressAndResponse(1500);
    int sf3 = systemState.currentSF;

    // All should be different (unless we wrapped around)
    TEST_ASSERT_NOT_EQUAL(sf1, sf2);
    // sf3 might equal sf1 if we cycled through all values
}

void test_bw_cycling_integration() {
    resetSystemState();

    // Test BW cycling
    float initialBW = systemState.currentBW;
    simulateButtonPressAndResponse(5000); // CycleBW
    TEST_ASSERT_NOT_EQUAL(initialBW, systemState.currentBW);

    // Test that BW is one of the expected values
    bool validBW = (systemState.currentBW == 62.5 ||
                    systemState.currentBW == 125.0 ||
                    systemState.currentBW == 250.0 ||
                    systemState.currentBW == 500.0);
    TEST_ASSERT_TRUE(validBW);
}

void test_message_transmission_integration() {
    resetSystemState();

    // Test message formatting and sequence increment
    simulateMessageTransmission();
    TEST_ASSERT_EQUAL_STRING("PING seq=0", systemState.lastMessage);
    TEST_ASSERT_EQUAL(1, systemState.messageSeq);

    simulateMessageTransmission();
    TEST_ASSERT_EQUAL_STRING("PING seq=1", systemState.lastMessage);
    TEST_ASSERT_EQUAL(2, systemState.messageSeq);

    // Switch to receiver mode and verify no transmission
    simulateButtonPressAndResponse(200); // ToggleMode
    strcpy(systemState.lastMessage, ""); // Clear message
    simulateMessageTransmission();
    TEST_ASSERT_EQUAL_STRING("", systemState.lastMessage); // Should remain empty
}

void test_ignore_action_integration() {
    resetSystemState();

    // Store initial state
    bool initialMode = systemState.isSender;
    int initialSF = systemState.currentSF;
    float initialBW = systemState.currentBW;
    uint32_t initialSeq = systemState.messageSeq;

    // Simulate ignored button press
    simulateButtonPressAndResponse(50); // Ignore

    // Verify no state changes
    TEST_ASSERT_EQUAL(initialMode, systemState.isSender);
    TEST_ASSERT_EQUAL(initialSF, systemState.currentSF);
    TEST_ASSERT_EQUAL_FLOAT(initialBW, systemState.currentBW);
    TEST_ASSERT_EQUAL(initialSeq, systemState.messageSeq);
}

void test_boundary_conditions_integration() {
    resetSystemState();

    // Test boundary values that should trigger different actions
    simulateButtonPressAndResponse(99);   // Should ignore
    simulateButtonPressAndResponse(100);  // Should toggle mode
    simulateButtonPressAndResponse(999);  // Should toggle mode
    simulateButtonPressAndResponse(1000); // Should cycle SF
    simulateButtonPressAndResponse(2999); // Should cycle SF
    simulateButtonPressAndResponse(3000); // Should cycle BW

    // Verify we ended up in a different state (mode should have toggled twice = same as start)
    TEST_ASSERT_TRUE(systemState.isSender); // Should be back to sender after two toggles
}

void test_rapid_button_presses() {
    resetSystemState();

    // Simulate rapid button presses of different durations
    uint32_t pressDurations[] = {150, 1200, 200, 4000, 80, 1800};
    int numPresses = sizeof(pressDurations) / sizeof(pressDurations[0]);

    for (int i = 0; i < numPresses; i++) {
        simulateButtonPressAndResponse(pressDurations[i]);

        // Verify system state remains consistent
        TEST_ASSERT_GREATER_OR_EQUAL(7, systemState.currentSF);
        TEST_ASSERT_LESS_OR_EQUAL(12, systemState.currentSF);

        bool validBW = (systemState.currentBW == 62.5 ||
                        systemState.currentBW == 125.0 ||
                        systemState.currentBW == 250.0 ||
                        systemState.currentBW == 500.0);
        TEST_ASSERT_TRUE(validBW);
    }
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Scenario-based tests
    RUN_TEST(test_button_press_scenarios);

    // State integration tests
    RUN_TEST(test_system_state_integration);
    RUN_TEST(test_sf_cycling_integration);
    RUN_TEST(test_bw_cycling_integration);
    RUN_TEST(test_message_transmission_integration);
    RUN_TEST(test_ignore_action_integration);

    // Edge case tests
    RUN_TEST(test_boundary_conditions_integration);
    RUN_TEST(test_rapid_button_presses);

    return UNITY_END();
}
