#!/bin/bash

# SBT PIO Heltec V3 OLED Release Script
# This script creates a new release tag and pushes it to trigger the release workflow

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 <version> [options]"
    echo ""
    echo "Arguments:"
    echo "  version         Version number (e.g., 1.0.0, 2.1.3)"
    echo ""
    echo "Options:"
    echo "  -h, --help      Show this help message"
    echo "  -d, --dry-run   Show what would be done without executing"
    echo "  -f, --force     Force tag creation even if it already exists"
    echo ""
    echo "Examples:"
    echo "  $0 1.0.0              # Create release v1.0.0"
    echo "  $0 2.1.0 --dry-run   # Show what would be done"
    echo "  $0 1.5.2 --force     # Force recreate tag v1.5.2"
    echo ""
    echo "Version format should follow semantic versioning (e.g., 1.0.0, 2.1.3)"
}

# Function to validate version format
validate_version() {
    local version=$1
    if [[ ! $version =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9.-]+)?(\+[a-zA-Z0-9.-]+)?$ ]]; then
        print_error "Invalid version format: $version"
        echo "Version should follow semantic versioning (e.g., 1.0.0, 2.1.3-beta.1)"
        exit 1
    fi
}

# Function to check if tag exists
tag_exists() {
    local version=$1
    git tag -l "v$version" | grep -q "v$version"
}

# Function to check git status
check_git_status() {
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        print_error "Not in a git repository"
        exit 1
    fi

    # Check if we're on main branch
    local current_branch=$(git branch --show-current)
    if [[ "$current_branch" != "main" ]]; then
        print_warning "Current branch is '$current_branch', not 'main'"
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_error "Aborted"
            exit 1
        fi
    fi

    # Check for uncommitted changes
    if ! git diff-index --quiet HEAD --; then
        print_warning "You have uncommitted changes"
        git status --short
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_error "Aborted"
            exit 1
        fi
    fi
}

# Function to create and push tag
create_release_tag() {
    local version=$1
    local force=$2
    local dry_run=$3

    local tag_name="v$version"

    if [[ "$dry_run" == "true" ]]; then
        print_status "DRY RUN: Would create tag '$tag_name'"
        print_status "DRY RUN: Would push tag '$tag_name' to origin"
        print_status "DRY RUN: This would trigger the release workflow"
        return
    fi

    # Create tag
    if [[ "$force" == "true" ]]; then
        if tag_exists "$version"; then
            print_warning "Tag '$tag_name' already exists, deleting it..."
            git tag -d "$tag_name"
            git push origin ":refs/tags/$tag_name" 2>/dev/null || true
        fi
    else
        if tag_exists "$version"; then
            print_error "Tag '$tag_name' already exists. Use --force to recreate it."
            exit 1
        fi
    fi

    print_status "Creating tag '$tag_name'..."
    git tag -a "$tag_name" -m "Release $tag_name"

    print_status "Pushing tag '$tag_name' to origin..."
    git push origin "$tag_name"

    print_success "Release tag '$tag_name' created and pushed successfully!"
    print_status "The release workflow will now start automatically"
    print_status "You can monitor progress at: https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^/]*\/[^/]*\)\.git.*/\1/')/actions"
}

# Main script
main() {
    local version=""
    local dry_run=false
    local force=false

    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_usage
                exit 0
                ;;
            -d|--dry-run)
                dry_run=true
                shift
                ;;
            -f|--force)
                force=true
                shift
                ;;
            -*)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
            *)
                if [[ -z "$version" ]]; then
                    version=$1
                else
                    print_error "Multiple versions specified: $version and $1"
                    exit 1
                fi
                shift
                ;;
        esac
    done

    # Check if version is provided
    if [[ -z "$version" ]]; then
        print_error "Version number is required"
        show_usage
        exit 1
    fi

    # Validate version format
    validate_version "$version"

    # Check git status
    check_git_status

    # Create and push tag
    create_release_tag "$version" "$force" "$dry_run"

    if [[ "$dry_run" == "false" ]]; then
        echo ""
        print_status "Next steps:"
        print_status "1. Monitor the release workflow at GitHub Actions"
        print_status "2. Once complete, the release will be available at:"
        print_status "   https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^/]*\/[^/]*\)\.git.*/\1/')/releases/tag/v$version"
        print_status "3. Users can download firmware and use the web flasher HTML files"
    fi
}

# Run main function with all arguments
main "$@"
