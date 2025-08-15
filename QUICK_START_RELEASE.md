# Quick Start: Creating a Release

This guide shows you how to quickly create a release for your firmware.

## Prerequisites

- You have push access to the repository
- Your changes are committed and pushed to the main branch
- You're ready to release a new version

## Quick Release Steps

### 1. Create a Release Tag

```bash
# Navigate to the scripts directory
cd scripts

# Create release v1.0.0 (replace with your version)
./create_release.sh 1.0.0
```

### 2. Monitor the Workflow

The script will automatically:
- Create a git tag (e.g., `v1.0.0`)
- Push the tag to GitHub
- Trigger the release workflow

Monitor progress at: `https://github.com/<owner>/<repo>/actions`

### 3. Access Your Release

Once complete, your release will be available at:
`https://github.com/<owner>/<repo>/releases/tag/v1.0.0`

## What Gets Created

- **Firmware Files**: `sender_firmware.bin`, `receiver_firmware.bin`
- **Web Flasher Pages**: HTML files for easy flashing
- **Manifest Files**: JSON files for ESP32 web flasher tools
- **GitHub Release**: Organized release page with all assets

## Testing Before Release

```bash
# Test the script without creating tags
./create_release.sh 1.0.0 --dry-run

# Test the script functionality
./test_release_script.sh
```

## Common Commands

```bash
# Help
./create_release.sh --help

# Dry run (see what would happen)
./create_release.sh 1.0.0 --dry-run

# Force recreate existing tag
./create_release.sh 1.0.0 --force

# Create different version types
./create_release.sh 1.0.0          # Stable release
./create_release.sh 1.1.0-beta.1   # Beta release
./create_release.sh 2.0.0-alpha.2  # Alpha release
```

## Version Format

Use semantic versioning:
- **MAJOR.MINOR.PATCH** (e.g., 1.0.0, 2.1.3)
- **Pre-release**: 1.0.0-beta.1, 2.1.0-rc.1
- **Build metadata**: 1.0.0+build.123

## Need Help?

- Check [RELEASE_README.md](RELEASE_README.md) for detailed documentation
- Review [CI_CACHING_OPTIMIZATION.md](CI_CACHING_OPTIMIZATION.md) for CI/CD details
- Open an issue for problems or questions

---

**That's it!** Your release will be built automatically and available for users to download and flash.
