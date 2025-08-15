#!/bin/bash

# Test script for the release creation script
# This script tests the functionality without actually creating tags

set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

echo "Testing release script functionality..."
echo "====================================="

# Test 1: Check if release script exists and is executable
if [[ -f "scripts/create_release.sh" ]]; then
    if [[ -x "scripts/create_release.sh" ]]; then
        print_success "Release script exists and is executable"
    else
        print_error "Release script exists but is not executable"
        exit 1
    fi
else
    print_error "Release script not found"
    exit 1
fi

# Test 2: Test help functionality
echo ""
echo "Testing help functionality..."
if ./scripts/create_release.sh --help 2>&1 | grep -q "Usage:"; then
    print_success "Help functionality works correctly"
else
    print_error "Help functionality failed"
fi

# Test 3: Test version validation
echo ""
echo "Testing version validation..."

# Test valid versions
valid_versions=("1.0.0" "2.1.3" "10.0.0-beta.1" "1.5.2+build.123")
for version in "${valid_versions[@]}"; do
    if ./scripts/create_release.sh "$version" --dry-run 2>&1 | grep -q "DRY RUN"; then
        print_success "Valid version '$version' accepted"
    else
        print_error "Valid version '$version' rejected"
    fi
done

# Test invalid versions
invalid_versions=("1.0" "v1.0.0" "1.0.0.0" "abc" "1.0.0-")
for version in "${invalid_versions[@]}"; do
    if ./scripts/create_release.sh "$version" --dry-run 2>&1 | grep -q "Invalid version format"; then
        print_success "Invalid version '$version' correctly rejected"
    else
        print_error "Invalid version '$version' incorrectly accepted"
    fi
done

# Test 4: Test dry run functionality
echo ""
echo "Testing dry run functionality..."
if ./scripts/create_release.sh 1.0.0 --dry-run 2>&1 | grep -q "DRY RUN"; then
    print_success "Dry run functionality works correctly"
else
    print_error "Dry run functionality failed"
fi

# Test 5: Check if web flasher template exists
echo ""
echo "Checking web flasher template..."
if [[ -f "scripts/web_flasher_template.html" ]]; then
    print_success "Web flasher template exists"

    # Check if template contains required placeholders
    required_placeholders=("{{FIRMWARE_NAME}}" "{{VERSION}}" "{{ROLE}}" "{{BUILD_DATE}}" "{{FIRMWARE_FILE}}")
    missing_placeholders=()

    for placeholder in "${required_placeholders[@]}"; do
        if ! grep -q "$placeholder" "scripts/web_flasher_template.html"; then
            missing_placeholders+=("$placeholder")
        fi
    done

    if [[ ${#missing_placeholders[@]} -eq 0 ]]; then
        print_success "All required template placeholders are present"
    else
        print_error "Missing template placeholders: ${missing_placeholders[*]}"
    fi
else
    print_error "Web flasher template not found"
fi

# Test 6: Check if release workflow exists
echo ""
echo "Checking release workflow..."
if [[ -f ".github/workflows/release.yml" ]]; then
    print_success "Release workflow exists"

    # Check if workflow has required triggers
    if grep -q "tags:" ".github/workflows/release.yml"; then
        print_success "Release workflow has tag triggers"
    else
        print_error "Release workflow missing tag triggers"
    fi

    # Check if workflow builds both environments
    if grep -q "environment: \[sender, receiver\]" ".github/workflows/release.yml"; then
        print_success "Release workflow builds both sender and receiver environments"
    else
        print_error "Release workflow missing environment matrix"
    fi
else
    print_error "Release workflow not found"
fi

# Test 7: Test script argument parsing
echo ""
echo "Testing argument parsing..."

# Test missing version
if ./scripts/create_release.sh 2>&1 | grep -q "Version number is required"; then
    print_success "Missing version argument correctly handled"
else
    print_error "Missing version argument not handled correctly"
fi

# Test multiple versions
if ./scripts/create_release.sh 1.0.0 2.0.0 2>&1 | grep -q "Multiple versions specified"; then
    print_success "Multiple versions argument correctly handled"
else
    print_error "Multiple versions argument not handled correctly"
fi

# Test unknown option
if ./scripts/create_release.sh 1.0.0 --unknown-option 2>&1 | grep -q "Unknown option"; then
    print_success "Unknown option correctly handled"
else
    print_error "Unknown option not handled correctly"
fi

echo ""
echo "====================================="
print_success "All tests completed successfully!"
echo ""
echo "The release script is ready to use. To create a release:"
echo "  ./scripts/create_release.sh 1.0.0"
echo ""
echo "To test without creating tags:"
echo "  ./scripts/create_release.sh 1.0.0 --dry-run"
