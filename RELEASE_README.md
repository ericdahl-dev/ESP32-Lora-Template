# Release Process

This document explains how to create releases for the SBT PIO Heltec V3 OLED project.

## Overview

The project uses GitHub Actions to automatically build and release firmware when a tag is pushed. Each release includes:

- **Sender Firmware**: For devices that send LoRa messages
- **Receiver Firmware**: For devices that receive LoRa messages and support OTA updates
- **Web Flasher Pages**: HTML files for easy web-based firmware flashing
- **Manifest Files**: JSON files compatible with ESP32 web flasher tools

## Creating a Release

### Prerequisites

- You must have push access to the repository
- Your local repository should be up to date with the main branch
- All changes should be committed and pushed

### Method 1: Using the Release Script (Recommended)

1. **Navigate to the scripts directory:**
   ```bash
   cd scripts
   ```

2. **Run the release script:**
   ```bash
   ./create_release.sh <version>
   ```

   **Examples:**
   ```bash
   # Create release v1.0.0
   ./create_release.sh 1.0.0

   # Preview what would happen (dry run)
   ./create_release.sh 1.0.0 --dry-run

   # Force recreate an existing tag
   ./create_release.sh 1.0.0 --force
   ```

3. **Monitor the workflow:**
   - The script will automatically create and push a tag
   - This triggers the GitHub Actions release workflow
   - Monitor progress at: `https://github.com/<owner>/<repo>/actions`

### Method 2: Manual Tag Creation

1. **Create a local tag:**
   ```bash
   git tag -a v1.0.0 -m "Release v1.0.0"
   ```

2. **Push the tag:**
   ```bash
   git push origin v1.0.0
   ```

## Release Workflow

When a tag is pushed, the following happens automatically:

1. **Build Phase**: Both sender and receiver firmware are built
2. **Artifact Creation**: Web flasher files and manifests are generated
3. **Release Creation**: A GitHub release is created with all artifacts
4. **Asset Upload**: Firmware binaries and web flasher files are attached

## Release Artifacts

Each release includes the following files:

### Firmware Binaries
- `sender_firmware.bin` - Sender role firmware
- `receiver_firmware.bin` - Receiver role firmware

### Web Flasher Files
- `sender_web_flasher.html` - Web-based flasher for sender devices
- `receiver_web_flasher.html` - Web-based flasher for receiver devices

### Manifest Files
- `sender_manifest.json` - ESP32 web flasher manifest for sender
- `receiver_manifest.json` - ESP32 web flasher manifest for receiver

## Using the Web Flasher

### For Users

1. **Download the appropriate web flasher HTML file** from the release
2. **Open the HTML file** in a modern web browser
3. **Connect your Heltec V3 board** via USB
4. **Put the board in download mode** (hold BOOT, press RESET, release RESET, release BOOT)
5. **Click "Flash Firmware"** and wait for completion
6. **Reset the board** after flashing

### For Developers

The web flasher integrates with the [ESP32 Web Flasher](https://github.com/espressif/esp-web-flasher) library and provides:

- Progress tracking during flashing
- Error handling and user feedback
- Responsive design for mobile and desktop
- Automatic firmware detection and configuration

## Version Numbering

Follow [Semantic Versioning](https://semver.org/) (SemVer):

- **MAJOR.MINOR.PATCH** (e.g., 1.0.0, 2.1.3)
- **Pre-release versions** supported (e.g., 1.0.0-beta.1)
- **Build metadata** supported (e.g., 1.0.0+build.123)

## Troubleshooting

### Common Issues

1. **Tag already exists:**
   ```bash
   # Use --force to recreate
   ./create_release.sh 1.0.0 --force
   ```

2. **Workflow not triggered:**
   - Ensure the tag format is `v*` (e.g., v1.0.0)
   - Check that the tag was pushed to the remote repository
   - Verify GitHub Actions are enabled for the repository

3. **Build failures:**
   - Check the GitHub Actions logs for specific errors
   - Ensure all dependencies are properly specified in `platformio.ini`
   - Verify the build environment configuration

### Getting Help

- Check the [GitHub Actions logs](https://github.com/<owner>/<repo>/actions)
- Review the [CI/CD documentation](CI_CACHING_OPTIMIZATION.md)
- Open an issue for persistent problems

## Security Notes

- **Never commit secrets** or sensitive configuration
- **Verify firmware integrity** before flashing to production devices
- **Use HTTPS** when hosting web flasher files
- **Validate firmware sources** before downloading

## Future Enhancements

Planned improvements to the release process:

- [ ] Automated changelog generation
- [ ] Firmware signature verification
- [ ] Multi-board support
- [ ] OTA update server integration
- [ ] Release notes automation
- [ ] Docker-based build environments

## Contributing

To improve the release process:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This release process is part of the SBT PIO Heltec V3 OLED project and follows the same license terms.
