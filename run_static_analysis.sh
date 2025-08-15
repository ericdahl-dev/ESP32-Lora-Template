#!/bin/bash

# Comprehensive Static Analysis Runner for Lightning Detection System
# Runs both cppcheck and clang-tidy analysis on all environments

set -e

echo "=========================================="
echo "Running Comprehensive Static Analysis"
echo "=========================================="
echo "Tools: cppcheck + clang-tidy"
echo "Environments: sender + receiver"
echo ""

# Function to run static analysis for a specific tool and environment
run_analysis() {
    local env_name=$1
    local tool_name=$2
    local description=$3

    echo "Running $description..."
    echo "----------------------------------------"

    # Check tool availability
    if [[ "$tool_name" == "clang-tidy" ]] && ! command -v clang-tidy &> /dev/null; then
        echo "⚠️  clang-tidy not found. Please install with:"
        echo "   macOS: brew install llvm"
        echo "   Ubuntu: sudo apt-get install clang-tidy"
        echo "   Other: Check your package manager"
        return 1
    fi

    # Run PlatformIO check
    echo "Running: pio check -e $env_name"
    if pio check -e "$env_name"; then
        echo "✅ $description PASSED"
        return 0
    else
        echo "❌ $description FAILED"
        return 1
    fi
}

# Initialize results tracking
total_checks=0
passed_checks=0
failed_checks=0

# Define all analysis environments
analysis_envs=(
    "sender-check:cppcheck:Sender Environment (cppcheck)"
    "receiver-check:cppcheck:Receiver Environment (cppcheck)"
    "sender-tidy:clang-tidy:Sender Environment (clang-tidy)"
    "receiver-tidy:clang-tidy:Receiver Environment (clang-tidy)"
)

# Run all analyses
for env in "${analysis_envs[@]}"; do
    IFS=':' read -r env_name tool_name description <<< "$env"
    total_checks=$((total_checks + 1))

    echo ""
    if run_analysis "$env_name" "$tool_name" "$description"; then
        passed_checks=$((passed_checks + 1))
    else
        failed_checks=$((failed_checks + 1))
    fi
done

# Summary
echo ""
echo "=========================================="
echo "Static Analysis Summary"
echo "=========================================="
echo "Total checks run: $total_checks"
echo "Passed: $passed_checks"
echo "Failed: $failed_checks"

# Detailed breakdown
echo ""
echo "Breakdown by tool:"
echo "  cppcheck:    2 environments"
echo "  clang-tidy:  2 environments"
echo ""
echo "Breakdown by environment:"
echo "  sender:      2 tools"
echo "  receiver:    2 tools"

if [ $failed_checks -eq 0 ]; then
    echo ""
    echo "🎉 All static analysis checks passed!"
    echo "Your code meets our quality standards. ⭐"
    exit 0
else
    echo ""
    echo "💥 Some static analysis checks failed."
    echo ""
    echo "To fix issues:"
    echo "  1. Review the error messages above"
    echo "  2. Fix the reported issues in your code"
    echo "  3. Re-run analysis to verify fixes"
    echo ""
    echo "Individual tool commands:"
    echo "  ./run_tidy.sh                    # Run only clang-tidy"
    echo "  pio check -e sender-check        # Run only sender cppcheck"
    echo "  pio check -e receiver-check      # Run only receiver cppcheck"
    echo "  pio check -e sender-tidy         # Run only sender clang-tidy"
    echo "  pio check -e receiver-tidy       # Run only receiver clang-tidy"
    echo ""
    echo "Documentation: STATIC_ANALYSIS.md"
    exit 1
fi
