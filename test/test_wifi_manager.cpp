// Unit tests for WiFi Manager functionality
// These tests focus on testable logic and minimize hardware dependencies
#include <unity.h>
#include <cstring>

// Mock definitions for Arduino dependencies
class MockPreferences {
public:
    int lastMode = 0;
    int lastNetwork = -1;

    void begin(const char* name, bool readOnly) {}
    int getInt(const char* key, int defaultValue) {
        if (strcmp(key, "networkMode") == 0) return lastMode;
        if (strcmp(key, "lastNetwork") == 0) return lastNetwork;
        return defaultValue;
    }
    void putInt(const char* key, int value) {
        if (strcmp(key, "networkMode") == 0) lastMode = value;
        if (strcmp(key, "lastNetwork") == 0) lastNetwork = value;
    }
};

// Mock Serial for testing
class MockSerial {
public:
    static void printf(const char* format, ...) {}
    static void println(const char* str) {}
    static void print(const char* str) {}
};

#define Serial MockSerial

// Include WiFi config definitions
#include "wifi_config.h"

// Test helper functions that would normally be in wifi_manager.cpp
// We'll test the core logic without hardware dependencies

// Test helper: Find network by location
int findNetworkByLocation(const char* location) {
    for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
        if (strcmp(WIFI_NETWORKS[i].location, location) == 0) {
            return i;
        }
    }
    return -1;
}

// Test helper: Find network by priority
int findNetworkByPriority(int priority) {
    for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
        if (WIFI_NETWORKS[i].priority == priority) {
            return i;
        }
    }
    return -1;
}

// Test helper: Validate network index
bool isValidNetworkIndex(int index) {
    return index >= 0 && index < NUM_WIFI_NETWORKS;
}

// Test helper: Get priority order (returns array of indices sorted by priority)
void getPriorityOrder(int* orderArray, int maxSize) {
    // Initialize all to -1
    for (int i = 0; i < maxSize; i++) {
        orderArray[i] = -1;
    }

    // Fill with network indices in priority order
    for (int priority = 1; priority <= NUM_WIFI_NETWORKS; priority++) {
        for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
            if (WIFI_NETWORKS[i].priority == priority && (priority - 1) < maxSize) {
                orderArray[priority - 1] = i;
                break;
            }
        }
    }
}

// Test cases
void test_wifi_network_configuration() {
    // Test that we have valid network configuration
    TEST_ASSERT_GREATER_THAN(0, NUM_WIFI_NETWORKS);
    TEST_ASSERT_LESS_OR_EQUAL(10, NUM_WIFI_NETWORKS); // Reasonable upper limit

    // Test that each network has required fields
    for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
        TEST_ASSERT_NOT_NULL(WIFI_NETWORKS[i].ssid);
        TEST_ASSERT_NOT_NULL(WIFI_NETWORKS[i].password);
        TEST_ASSERT_NOT_NULL(WIFI_NETWORKS[i].location);
        TEST_ASSERT_GREATER_THAN(0, WIFI_NETWORKS[i].priority);
    }
}

void test_findNetworkByLocation() {
    // Test finding existing networks
    int homeIndex = findNetworkByLocation("Home");
    int workIndex = findNetworkByLocation("Work");

    // Should find at least one network (based on wifi_config.h)
    TEST_ASSERT_NOT_EQUAL(-1, homeIndex);

    // Verify the found network actually has the right location
    if (homeIndex != -1) {
        TEST_ASSERT_EQUAL_STRING("Home", WIFI_NETWORKS[homeIndex].location);
    }

    // Test non-existent location
    TEST_ASSERT_EQUAL(-1, findNetworkByLocation("NonExistent"));
    TEST_ASSERT_EQUAL(-1, findNetworkByLocation(""));
}

void test_findNetworkByPriority() {
    // Test finding by priority
    int priority1Index = findNetworkByPriority(1);
    TEST_ASSERT_NOT_EQUAL(-1, priority1Index);

    if (priority1Index != -1) {
        TEST_ASSERT_EQUAL(1, WIFI_NETWORKS[priority1Index].priority);
    }

    // Test invalid priority
    TEST_ASSERT_EQUAL(-1, findNetworkByPriority(0));
    TEST_ASSERT_EQUAL(-1, findNetworkByPriority(-1));
    TEST_ASSERT_EQUAL(-1, findNetworkByPriority(999));
}

void test_isValidNetworkIndex() {
    // Test valid indices
    TEST_ASSERT_TRUE(isValidNetworkIndex(0));
    if (NUM_WIFI_NETWORKS > 1) {
        TEST_ASSERT_TRUE(isValidNetworkIndex(1));
    }
    TEST_ASSERT_TRUE(isValidNetworkIndex(NUM_WIFI_NETWORKS - 1));

    // Test invalid indices
    TEST_ASSERT_FALSE(isValidNetworkIndex(-1));
    TEST_ASSERT_FALSE(isValidNetworkIndex(NUM_WIFI_NETWORKS));
    TEST_ASSERT_FALSE(isValidNetworkIndex(999));
}

void test_getPriorityOrder() {
    int orderArray[10];
    getPriorityOrder(orderArray, 10);

    // Check that we get valid indices for configured networks
    for (int i = 0; i < NUM_WIFI_NETWORKS && i < 10; i++) {
        if (orderArray[i] != -1) {
            TEST_ASSERT_TRUE(isValidNetworkIndex(orderArray[i]));
            // Priority should match position + 1
            TEST_ASSERT_EQUAL(i + 1, WIFI_NETWORKS[orderArray[i]].priority);
        }
    }

    // Check that priorities are unique (no duplicates)
    for (int i = 0; i < NUM_WIFI_NETWORKS && i < 10; i++) {
        if (orderArray[i] != -1) {
            for (int j = i + 1; j < NUM_WIFI_NETWORKS && j < 10; j++) {
                if (orderArray[j] != -1) {
                    TEST_ASSERT_NOT_EQUAL(orderArray[i], orderArray[j]);
                }
            }
        }
    }
}

void test_networkSelectionMode_enum() {
    // Test that enum values are reasonable
    TEST_ASSERT_EQUAL(0, (int)NetworkSelectionMode::AUTO);
    TEST_ASSERT_EQUAL(1, (int)NetworkSelectionMode::MANUAL_HOME);
    TEST_ASSERT_EQUAL(2, (int)NetworkSelectionMode::MANUAL_WORK);
    TEST_ASSERT_EQUAL(3, (int)NetworkSelectionMode::MANUAL_CUSTOM);
}

void test_wifi_config_constants() {
    // Test that configuration constants are reasonable
    TEST_ASSERT_GREATER_THAN(1000, WIFI_CONNECT_TIMEOUT_MS);
    TEST_ASSERT_LESS_THAN(60000, WIFI_CONNECT_TIMEOUT_MS);

    TEST_ASSERT_GREATER_THAN(100, WIFI_RETRY_DELAY_MS);
    TEST_ASSERT_LESS_THAN(10000, WIFI_RETRY_DELAY_MS);

    TEST_ASSERT_GREATER_THAN(0, WIFI_MAX_RETRIES);
    TEST_ASSERT_LESS_THAN(10, WIFI_MAX_RETRIES);
}

void test_network_priorities_are_sequential() {
    // Check that priorities start at 1 and are sequential
    bool foundPriorities[NUM_WIFI_NETWORKS + 1] = {false}; // index 0 unused

    for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
        int priority = WIFI_NETWORKS[i].priority;
        TEST_ASSERT_GREATER_THAN(0, priority);
        TEST_ASSERT_LESS_OR_EQUAL(NUM_WIFI_NETWORKS, priority);

        // Check no duplicate priorities
        TEST_ASSERT_FALSE(foundPriorities[priority]);
        foundPriorities[priority] = true;
    }

    // Check all priorities from 1 to NUM_WIFI_NETWORKS are present
    for (int i = 1; i <= NUM_WIFI_NETWORKS; i++) {
        TEST_ASSERT_TRUE(foundPriorities[i]);
    }
}

void test_ota_configuration() {
    // Test OTA settings are defined and reasonable
    TEST_ASSERT_NOT_NULL(OTA_HOSTNAME);
    TEST_ASSERT_NOT_NULL(OTA_PASSWORD);

    // Hostname should be reasonable length
    TEST_ASSERT_GREATER_THAN(3, strlen(OTA_HOSTNAME));
    TEST_ASSERT_LESS_THAN(32, strlen(OTA_HOSTNAME));

    // Password should exist (even if simple)
    TEST_ASSERT_GREATER_THAN(0, strlen(OTA_PASSWORD));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Configuration tests
    RUN_TEST(test_wifi_network_configuration);
    RUN_TEST(test_wifi_config_constants);
    RUN_TEST(test_ota_configuration);
    RUN_TEST(test_networkSelectionMode_enum);
    RUN_TEST(test_network_priorities_are_sequential);

    // Logic tests
    RUN_TEST(test_findNetworkByLocation);
    RUN_TEST(test_findNetworkByPriority);
    RUN_TEST(test_isValidNetworkIndex);
    RUN_TEST(test_getPriorityOrder);

    return UNITY_END();
}
