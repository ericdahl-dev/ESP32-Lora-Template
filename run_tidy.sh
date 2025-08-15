#!/bin/bash

# Clang-Tidy runner script for Lightning Detection System
# Runs comprehensive static analysis on C++ code

set -e

echo "=========================================="
echo "Running Clang-Tidy Analysis"
echo "=========================================="

# Function to run clang-tidy for a specific environment
run_tidy() {
    local env_name=$1
    local description=$2

    echo ""
    echo "Running $description analysis..."
    echo "----------------------------------------"

    # Check if clang-tidy is available
    if ! command -v clang-tidy &> /dev/null; then
        echo "⚠️  clang-tidy not found. Please install with:"
        echo "   macOS: brew install llvm"
        echo "   Ubuntu: sudo apt-get install clang-tidy"
        echo "   Other: Check your package manager"
        return 1
    fi

    # Run PlatformIO check with clang-tidy
    echo "Running: pio check -e $env_name"
    if pio check -e "$env_name"; then
        echo "✅ $description analysis PASSED"
        return 0
    else
        echo "❌ $description analysis FAILED"
        return 1
    fi
}

# Initialize results tracking
total_checks=0
passed_checks=0
failed_checks=0

# Check environments to analyze
check_envs=(
    "sender-tidy:Sender Environment"
    "receiver-tidy:Receiver Environment"
)

for env in "${check_envs[@]}"; do
    IFS=':' read -r env_name description <<< "$env"
    total_checks=$((total_checks + 1))

    if run_tidy "$env_name" "$description"; then
        passed_checks=$((passed_checks + 1))
    else
        failed_checks=$((failed_checks + 1))
    fi
done

# Summary
echo ""
echo "=========================================="
echo "Clang-Tidy Analysis Summary"
echo "=========================================="
echo "Total environments checked: $total_checks"
echo "Passed: $passed_checks"
echo "Failed: $failed_checks"

if [ $failed_checks -eq 0 ]; then
    echo ""
    echo "🎉 All clang-tidy checks passed! Code quality looks good."
    exit 0
else
    echo ""
    echo "💥 Some clang-tidy checks failed. Please review and fix the issues above."
    echo ""
    echo "Tip: Run individual checks with:"
    echo "  pio check -e sender-tidy    # For sender code"
    echo "  pio check -e receiver-tidy  # For receiver code"
    exit 1
fi
