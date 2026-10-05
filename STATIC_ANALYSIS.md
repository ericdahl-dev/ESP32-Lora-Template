# Static Analysis for ESP32 Template

This ESP32 template uses comprehensive static analysis tools to maintain high code quality and catch potential issues early in development.

## 🔧 Tools Configured

### 1. **cppcheck** (Memory Safety & Logic Analysis)
- **Purpose**: Detects memory leaks, buffer overflows, null pointer dereferences
- **Configuration**: Built into PlatformIO check environments
- **Environments**: `template-check`, `example-check`

### 2. **clang-tidy** (Comprehensive Code Quality)
- **Purpose**: Comprehensive C++ static analysis, modernization, and style checking
- **Configuration**: `.clang-tidy` file with embedded-friendly rules
- **Environments**: `template-tidy`, `example-tidy`

## 🚀 Quick Start

### Local Development
```bash
# Run all static analysis (cppcheck + clang-tidy)
./run_static_analysis.sh

# Run only clang-tidy
./run_tidy.sh

# Run specific environment
pio check -e template-tidy
pio check -e example-tidy
pio check -e template-check
pio check -e example-check
```

### Requirements
```bash
# macOS
brew install llvm

# Ubuntu/Debian
sudo apt-get install clang-tidy

# Windows (via MSYS2)
pacman -S clang-tools-extra
```

## 📋 Clang-Tidy Configuration

Our `.clang-tidy` configuration is optimized for embedded C++ development:

### ✅ **Enabled Checks**
- **Bugprone**: Potential bugs and suspicious code patterns
- **Performance**: Performance anti-patterns and optimizations
- **Readability**: Code clarity and maintainability
- **Modernize**: C++11/14/17 modernization suggestions
- **Cert**: CERT C++ security guidelines
- **Misc**: Miscellaneous useful checks
- **Concurrency**: Thread safety issues
- **CPPCoreGuidelines**: Core Guidelines compliance

### ❌ **Disabled Checks**
- Platform-specific checks (Android, Darwin, Fuchsia, etc.)
- Style guides we don't follow (Google, LLVM, HICPP)
- Overly strict embedded-unfriendly rules
- Magic number warnings (common in embedded)
- Trailing return types (not our style)

### 🎯 **Naming Conventions**
- **Namespaces**: `PascalCase` (e.g., `Sensors`, `HAL`)
- **Classes/Structs**: `PascalCase` (e.g., `LightningSensor`)
- **Functions**: `camelBack` (e.g., `initializeHardware`)
- **Variables**: `camelBack` (e.g., `sensorReading`)
- **Private Members**: `camelBack_` (e.g., `isInitialized_`)
- **Constants**: `UPPER_CASE` (e.g., `MAX_SENSORS`)
- **Macros**: `UPPER_CASE` (e.g., `ENABLE_WIFI_OTA`)

## ⚙️ PlatformIO Environments

### Cppcheck Environments
```ini
[env:template-check]
extends = env:template
check_tool = cppcheck
check_flags = cppcheck: --enable=all --suppress=missingIncludeSystem

[env:example-check]
extends = env:environmental_monitor
check_tool = cppcheck
check_flags = cppcheck: --enable=all --suppress=missingIncludeSystem
```

### Clang-Tidy Environments
```ini
[env:template-tidy]
extends = env:template
check_tool = clangtidy
check_flags = clangtidy: --config-file=.clang-tidy

[env:example-tidy]
extends = env:environmental_monitor
check_tool = clangtidy
check_flags = clangtidy: --config-file=.clang-tidy
```

## 🎯 CI/CD Integration

GitHub Actions automatically runs all static analysis:

```yaml
- name: Run PlatformIO Check (cppcheck)
  run: |
    pio check -e template-check
    pio check -e example-check

- name: Run PlatformIO Check (clang-tidy)
  run: |
    pio check -e template-tidy
    pio check -e example-tidy
```

## 🔧 Customization

### Adding New Checks
Edit `.clang-tidy` to enable additional checks:
```yaml
Checks: >
  *,
  -disabled-check-*,
  +new-check-category-*
```

### Suppressing Specific Issues
For false positives or intentional patterns:

```cpp
// NOLINT(specific-check-name)
void problematicFunction() {
    // Code that triggers false positive
}

// NOLINTNEXTLINE(specific-check-name)
int* rawPointer = malloc(sizeof(int));
```

### Environment-Specific Suppressions
```ini
check_flags =
    clangtidy: --config-file=.clang-tidy --checks=-specific-check
```

## 📊 Common Issues & Solutions

### 1. **Arduino Library Warnings**
```
check_skip_packages = yes  # Skip analysis of third-party libraries
```

### 2. **False Positives in ISRs**
```cpp
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void IRAM_ATTR interruptHandler() {
    // ISR code
}
```

### 3. **Embedded-Specific Patterns**
Our configuration already disables many embedded-unfriendly checks like:
- Magic number warnings (sensor thresholds, pin numbers)
- Array-to-pointer decay (common with Arduino APIs)
- C-style casts (sometimes needed for hardware registers)

## 🏆 Benefits

### **Code Quality**
- Catch bugs before they reach hardware
- Ensure consistent coding style
- Improve code maintainability
- Reduce technical debt

### **Security**
- Detect buffer overflows and memory issues
- Identify potential security vulnerabilities
- Enforce safe coding practices

### **Performance**
- Identify performance anti-patterns
- Suggest modern C++ optimizations
- Catch inefficient algorithms

### **Maintainability**
- Enforce naming conventions
- Improve code readability
- Reduce complexity
- Standardize patterns across codebase

## 📚 Resources

- [Clang-Tidy Documentation](https://clang.llvm.org/extra/clang-tidy/)
- [PlatformIO Check Documentation](https://docs.platformio.org/en/latest/core/userguide/cmd_check.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [CERT C++ Guidelines](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682)

---

*This static analysis setup ensures your ESP32 projects maintain professional code quality while being embedded-system friendly.*
