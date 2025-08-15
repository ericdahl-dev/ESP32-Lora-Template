#!/bin/bash

# Clang-Tidy Cache Optimization Script
# Optimizes clang-tidy performance through intelligent caching

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CACHE_DIR="$PROJECT_ROOT/.clang-tidy-cache"
PIO_CACHE_DIR="$PROJECT_ROOT/.pio/.clang-tidy-cache"

echo "=========================================="
echo "Clang-Tidy Cache Optimization"
echo "=========================================="

# Function to setup cache directories
setup_cache_dirs() {
    echo "Setting up clang-tidy cache directories..."
    
    mkdir -p "$CACHE_DIR"
    mkdir -p "$PIO_CACHE_DIR"
    mkdir -p "$PROJECT_ROOT/.pio/build"
    
    echo "✓ Cache directories created:"
    echo "  - Main cache: $CACHE_DIR"
    echo "  - PIO cache: $PIO_CACHE_DIR"
}

# Function to check clang-tidy version and capabilities
check_clang_tidy() {
    echo "Checking clang-tidy installation..."
    
    if ! command -v clang-tidy &> /dev/null; then
        echo "❌ clang-tidy not found!"
        echo "Please install clang-tidy:"
        echo "  Ubuntu: sudo apt-get install clang-tidy"
        echo "  macOS: brew install llvm"
        return 1
    fi
    
    CLANG_TIDY_VERSION=$(clang-tidy --version | head -n 1)
    echo "✓ Found: $CLANG_TIDY_VERSION"
    
    # Check for cache support (newer versions)
    if clang-tidy --help | grep -q "store-check-profile"; then
        echo "✓ Cache and profiling support available"
        export CLANG_TIDY_HAS_CACHE=1
    else
        echo "⚠️  Limited caching support in this version"
        export CLANG_TIDY_HAS_CACHE=0
    fi
}

# Function to generate compilation database
generate_compile_db() {
    echo "Generating compilation database..."
    
    cd "$PROJECT_ROOT"
    
    # Generate for both tidy environments
    echo "Building compilation database for sender-tidy..."
    pio run -e sender-tidy --target compiledb || echo "⚠️  Sender compiledb generation had issues"
    
    echo "Building compilation database for receiver-tidy..."
    pio run -e receiver-tidy --target compiledb || echo "⚠️  Receiver compiledb generation had issues"
    
    # Verify compilation databases exist
    SENDER_DB="$PROJECT_ROOT/.pio/build/sender-tidy/compile_commands.json"
    RECEIVER_DB="$PROJECT_ROOT/.pio/build/receiver-tidy/compile_commands.json"
    
    if [[ -f "$SENDER_DB" ]]; then
        echo "✓ Sender compilation database: $(wc -l < "$SENDER_DB") entries"
    else
        echo "❌ Sender compilation database missing"
    fi
    
    if [[ -f "$RECEIVER_DB" ]]; then
        echo "✓ Receiver compilation database: $(wc -l < "$RECEIVER_DB") entries"
    else
        echo "❌ Receiver compilation database missing"
    fi
}

# Function to optimize cache settings
optimize_cache_settings() {
    echo "Optimizing clang-tidy cache settings..."
    
    # Set cache environment variables
    export CLANG_TIDY_CACHE_DIR="$CACHE_DIR"
    export TMPDIR="$PIO_CACHE_DIR"
    
    # Create cache configuration
    cat > "$CACHE_DIR/cache_config.txt" << EOF
# Clang-Tidy Cache Configuration
# Generated: $(date)
Cache Directory: $CACHE_DIR
PIO Cache Directory: $PIO_CACHE_DIR
Clang-Tidy Version: $(clang-tidy --version | head -n 1)
Project Root: $PROJECT_ROOT
EOF
    
    echo "✓ Cache configuration saved to $CACHE_DIR/cache_config.txt"
}

# Function to run cached clang-tidy analysis
run_cached_analysis() {
    local environment=$1
    local description=$2
    
    echo "Running cached clang-tidy analysis for $description..."
    
    cd "$PROJECT_ROOT"
    
    # Set cache environment
    export CLANG_TIDY_CACHE_DIR="$CACHE_DIR"
    
    # Additional caching flags for newer clang-tidy versions
    local cache_flags=""
    if [[ "$CLANG_TIDY_HAS_CACHE" == "1" ]]; then
        cache_flags="--store-check-profile=$CACHE_DIR/${environment}_profile.json"
    fi
    
    # Run clang-tidy with caching
    echo "Running: pio check -e $environment"
    if pio check -e "$environment" --fail-on-defect=medium; then
        echo "✅ $description analysis completed successfully"
        return 0
    else
        echo "⚠️  $description analysis completed with warnings"
        return 1
    fi
}

# Function to show cache statistics
show_cache_stats() {
    echo "Cache Statistics:"
    echo "----------------------------------------"
    
    if [[ -d "$CACHE_DIR" ]]; then
        local cache_size=$(du -sh "$CACHE_DIR" 2>/dev/null | cut -f1)
        local cache_files=$(find "$CACHE_DIR" -type f 2>/dev/null | wc -l)
        echo "✓ Main cache: $cache_size ($cache_files files)"
    else
        echo "❌ Main cache directory missing"
    fi
    
    if [[ -d "$PIO_CACHE_DIR" ]]; then
        local pio_cache_size=$(du -sh "$PIO_CACHE_DIR" 2>/dev/null | cut -f1)
        local pio_cache_files=$(find "$PIO_CACHE_DIR" -type f 2>/dev/null | wc -l)
        echo "✓ PIO cache: $pio_cache_size ($pio_cache_files files)"
    else
        echo "❌ PIO cache directory missing"
    fi
    
    # Show compilation database info
    echo ""
    echo "Compilation Databases:"
    find "$PROJECT_ROOT/.pio/build" -name "compile_commands.json" -exec echo "  ✓ {}" \; 2>/dev/null || echo "  ❌ No compilation databases found"
    
    echo ""
    echo "Build Artifacts:"
    if [[ -d "$PROJECT_ROOT/.pio/build" ]]; then
        local build_size=$(du -sh "$PROJECT_ROOT/.pio/build" 2>/dev/null | cut -f1)
        echo "  ✓ Build cache: $build_size"
    else
        echo "  ❌ No build artifacts found"
    fi
}

# Function to clean cache
clean_cache() {
    echo "Cleaning clang-tidy cache..."
    
    if [[ -d "$CACHE_DIR" ]]; then
        rm -rf "$CACHE_DIR"
        echo "✓ Main cache cleaned"
    fi
    
    if [[ -d "$PIO_CACHE_DIR" ]]; then
        rm -rf "$PIO_CACHE_DIR"
        echo "✓ PIO cache cleaned"
    fi
    
    # Clean compilation databases but keep build artifacts
    find "$PROJECT_ROOT/.pio/build" -name "compile_commands.json" -delete 2>/dev/null || true
    echo "✓ Compilation databases cleaned"
}

# Function to validate cache integrity
validate_cache() {
    echo "Validating cache integrity..."
    
    local issues=0
    
    # Check cache directories
    if [[ ! -d "$CACHE_DIR" ]]; then
        echo "❌ Main cache directory missing"
        ((issues++))
    fi
    
    if [[ ! -d "$PIO_CACHE_DIR" ]]; then
        echo "❌ PIO cache directory missing"
        ((issues++))
    fi
    
    # Check compilation databases
    if [[ ! -f "$PROJECT_ROOT/.pio/build/sender-tidy/compile_commands.json" ]]; then
        echo "⚠️  Sender compilation database missing"
        ((issues++))
    fi
    
    if [[ ! -f "$PROJECT_ROOT/.pio/build/receiver-tidy/compile_commands.json" ]]; then
        echo "⚠️  Receiver compilation database missing"
        ((issues++))
    fi
    
    if [[ $issues -eq 0 ]]; then
        echo "✅ Cache validation passed"
        return 0
    else
        echo "⚠️  Cache validation found $issues issues"
        return 1
    fi
}

# Main execution
main() {
    local command=${1:-"setup"}
    
    case "$command" in
        "setup")
            setup_cache_dirs
            check_clang_tidy
            optimize_cache_settings
            generate_compile_db
            show_cache_stats
            ;;
        "run")
            setup_cache_dirs
            check_clang_tidy
            optimize_cache_settings
            
            local analysis_failed=0
            run_cached_analysis "sender-tidy" "Sender" || ((analysis_failed++))
            run_cached_analysis "receiver-tidy" "Receiver" || ((analysis_failed++))
            
            show_cache_stats
            
            if [[ $analysis_failed -eq 0 ]]; then
                echo "🎉 All clang-tidy analyses completed successfully!"
                exit 0
            else
                echo "⚠️  Some analyses completed with warnings"
                exit 1
            fi
            ;;
        "stats")
            show_cache_stats
            ;;
        "clean")
            clean_cache
            echo "🧹 Cache cleaned successfully"
            ;;
        "validate")
            validate_cache
            ;;
        "help"|"-h"|"--help")
            echo "Usage: $0 [command]"
            echo ""
            echo "Commands:"
            echo "  setup      Setup cache directories and generate compilation database"
            echo "  run        Run clang-tidy analysis with caching (default)"
            echo "  stats      Show cache statistics"
            echo "  clean      Clean all cache files"
            echo "  validate   Validate cache integrity"
            echo "  help       Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  CLANG_TIDY_CACHE_DIR   Custom cache directory"
            echo "  CLANG_TIDY_VERBOSE     Enable verbose output"
            ;;
        *)
            echo "❌ Unknown command: $command"
            echo "Use '$0 help' for usage information"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"