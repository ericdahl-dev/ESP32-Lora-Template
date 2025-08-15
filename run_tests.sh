#!/bin/bash

# Test runner script for LtngDet PIO Heltec V3 OLED project
# Runs all unit tests individually to avoid symbol conflicts

echo "=========================================="
echo "Running Unit Tests for LtngDet PIO Project"
echo "=========================================="

# Function to run a single test file
run_single_test() {
    local test_name=$1
    local test_file=$2

    echo ""
    echo "Running $test_name tests..."
    echo "----------------------------------------"

    # Move other test files temporarily
    for file in test/test_*.cpp; do
        if [ "$file" != "$test_file" ]; then
            mv "$file" "${file}.bak" 2>/dev/null || true
        fi
    done

    # Run the test
    pio test -e native
    test_result=$?

    # Restore other test files
    for file in test/test_*.cpp.bak; do
        if [ -f "$file" ]; then
            mv "$file" "${file%.bak}" 2>/dev/null || true
        fi
    done

    if [ $test_result -eq 0 ]; then
        echo "✅ $test_name tests PASSED"
        return 0
    else
        echo "❌ $test_name tests FAILED"
        return 1
    fi
}

# Initialize test results
total_tests=0
passed_tests=0
failed_tests=0

# Run all test suites
test_suites=(
    "Hardware Abstraction:test/test_hardware_abstraction.cpp"
    "App Logic:test/test_app_logic.cpp"
    "WiFi Manager:test/test_wifi_manager.cpp"
    "WiFi Logic:test/test_wifi_logic.cpp"
    "Integration:test/test_integration.cpp"
    "Modular Architecture:test/test_modular_architecture.cpp"
    "State Machine:test/test_state_machine.cpp"
    "Error Handler:test/test_error_handler.cpp"
    "Sensor Framework:test/test_sensor_framework.cpp"
)

for suite in "${test_suites[@]}"; do
    IFS=':' read -r name file <<< "$suite"
    if [ -f "$file" ]; then
        total_tests=$((total_tests + 1))
        if run_single_test "$name" "$file"; then
            passed_tests=$((passed_tests + 1))
        else
            failed_tests=$((failed_tests + 1))
        fi
    else
        echo "⚠️  Test file $file not found, skipping $name tests"
    fi
done

# Summary
echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Total test suites: $total_tests"
echo "Passed: $passed_tests"
echo "Failed: $failed_tests"

if [ $failed_tests -eq 0 ]; then
    echo ""
    echo "🎉 All tests passed! Ready for refactoring."
    exit 0
else
    echo ""
    echo "💥 Some tests failed. Please fix before refactoring."
    exit 1
fi
