# GitHub Releases Setup for Firmware Distribution

This guide explains how to set up GitHub releases to automatically distribute firmware files to the web flasher.

## 📋 Release Structure

Each release should include:

1. **Firmware Binary Files** (`.bin` files)
2. **Firmware Manifest** (`firmware_manifest.json`)
3. **Release Notes** (description of changes)

## 🏗️ Release Asset Organization

### File Naming Convention
```
v1.0.0/
├── sender_firmware_v1.0.0.bin      # Transmitter firmware
├── receiver_firmware_v1.0.0.bin     # Receiver firmware
└── firmware_manifest.json           # Firmware metadata
```

### Version Tagging
- Use semantic versioning: `v1.0.0`, `v1.1.0`, `v2.0.0`
- Tag format: `v{major}.{minor}.{patch}`
- Example: `v1.2.3` for major=1, minor=2, patch=3

## 📝 Creating a Release

### 1. Build Firmware
```bash
# Build transmitter firmware
pio run -e sender -t upload

# Build receiver firmware
pio run -e receiver -t upload

# Copy .bin files from .pio/build/ directories
cp .pio/build/sender/firmware.bin sender_firmware_v1.0.0.bin
cp .pio/build/receiver/firmware.bin receiver_firmware_v1.0.0.bin
```

### 2. Create Firmware Manifest
Use the template in `firmware_manifest_example.json` and update:
- Version number
- Release date
- File sizes (actual .bin file sizes)
- SHA256 checksums
- Release notes

### 3. GitHub Release Creation
1. Go to GitHub repository → Releases → "Create a new release"
2. **Tag version**: `v1.0.0`
3. **Release title**: `v1.0.0 - Initial Release`
4. **Description**: Add release notes and changelog
5. **Assets**: Upload all three files:
   - `sender_firmware_v1.0.0.bin`
   - `receiver_firmware_v1.0.0.bin`
   - `firmware_manifest.json`

## 🔄 Automatic Firmware Updates

The web flasher automatically:
1. Fetches the latest release from GitHub API
2. Downloads the `firmware_manifest.json`
3. Maps device types to actual filenames
4. Downloads the appropriate firmware file
5. Falls back to manual upload if GitHub firmware unavailable

## 📊 Firmware Manifest Schema

```json
{
  "version": "string",           // Release version (e.g., "1.0.0")
  "release_date": "string",      // ISO date (e.g., "2025-01-14")
  "release_notes": "string",     // Human-readable release description
  "firmware_files": {
    "transmitter": {
      "filename": "string",       // Actual .bin filename
      "size": "number",          // File size in bytes
      "checksum": "string",      // SHA256 hash
      "description": "string",   // Firmware description
      "features": ["array"],     // List of features
      "target_board": "string",  // Compatible hardware
      "mcu": "string"           // Microcontroller type
    },
    "receiver": {
      // Same structure as transmitter
    }
  },
  "compatibility": {
    "min_esp_idf": "string",     // Minimum ESP-IDF version
    "min_arduino": "string",     // Minimum Arduino version
    "supported_boards": ["array"] // List of compatible boards
  },
  "installation": {
    "flash_size": "string",      // Required flash size
    "partition_scheme": "string", // Partition scheme
    "upload_speed": "string"     // Recommended upload speed
  }
}
```

## 🚀 Benefits of This Approach

### ✅ **Version Independence**
- No hardcoded filenames in code
- Automatic detection of latest firmware
- Easy to add new device types

### ✅ **Flexibility**
- Support for multiple firmware versions
- Fallback to manual uploads
- Rich metadata for each release

### ✅ **Maintainability**
- Centralized firmware management
- Automatic checksum verification
- Clear compatibility information

### ✅ **User Experience**
- Users always get the latest firmware
- No need to manually find .bin files
- Clear progress and status updates

## 🔧 Implementation Notes

- The web flasher tries GitHub releases first
- Falls back to manual file uploads if needed
- Supports both automatic and manual workflows
- Maintains backward compatibility

## 📱 Testing Releases

1. Create a test release with sample firmware
2. Verify web flasher can fetch manifest
3. Test firmware download process
4. Validate fallback to manual uploads
5. Check error handling for missing files
