# Multi-Network WiFi Configuration

This project now supports multiple WiFi networks with automatic fallback and manual selection capabilities.

## Features

- **Multiple Network Support**: Configure home, work, and additional networks
- **Automatic Fallback**: Automatically tries networks in priority order
- **Manual Selection**: Force connection to specific networks
- **Persistent Storage**: Remembers your preferences across reboots
- **Smart Reconnection**: Automatically reconnects if connection is lost

## Configuration

### 1. Edit Network Credentials

Open `src/wifi_config.h` and modify the `wifiNetworks` array:

```cpp
static const WiFiNetwork wifiNetworks[] = {
  {"YourHomeSSID", "YourHomePassword", "Home", 1},      // Primary network
  {"YourWorkSSID", "YourWorkPassword", "Work", 2},      // Secondary network
  {"GuestNetwork", "GuestPassword", "Guest", 3},        // Additional network
};
```

**Parameters:**
- **SSID**: Your WiFi network name
- **Password**: Your WiFi password
- **Location**: Descriptive name (e.g., "Home", "Work", "Office")
- **Priority**: Lower number = higher priority (1 is highest)

### 2. Network Selection Modes

The system supports several modes:

- **AUTO**: Automatically tries networks in priority order (recommended)
- **MANUAL_HOME**: Force connection to home network
- **MANUAL_WORK**: Force connection to work network
- **MANUAL_CUSTOM**: Use last connected network

## Usage

### Button Controls (Receiver Mode)

- **Medium Press (1-3 seconds)**: Cycle through network modes
  - Shows current mode on display
  - Automatically reconnects with new mode
- **Long Press (3+ seconds)**: Still cycles bandwidth (existing functionality)

### Display Information

The OLED display shows:
- Current network location (e.g., "Home", "Work")
- Connection status
- IP address when connected

### Automatic Behavior

1. **Boot**: Tries to connect using saved mode
2. **Fallback**: If primary network fails, tries others in priority order
3. **Reconnection**: Automatically reconnects if connection is lost
4. **Memory**: Remembers last successful network for faster reconnection

## Example Scenarios

### Scenario 1: Home to Work Transition
1. Device connects to home network at startup
2. When you arrive at work, manually switch to "Work" mode
3. Device connects to work network
4. Mode is saved and will be used next time

### Scenario 2: Automatic Fallback
1. Device tries home network first (priority 1)
2. If home network is unavailable, tries work network (priority 2)
3. If work network fails, tries any additional networks
4. Once connected, remembers the working network

### Scenario 3: Network Outage
1. Device is connected to home network
2. Home network goes down
3. Device automatically tries work network
4. Reconnects to work network seamlessly

## Security Considerations

- **Credentials**: WiFi passwords are stored in the code (consider using external config file)
- **Network Names**: SSIDs are visible in the code
- **Recommendation**: Add `wifi_networks.h` to `.gitignore` if storing real credentials

## Troubleshooting

### Common Issues

1. **No Networks Connect**
   - Check SSID and password spelling
   - Verify network availability
   - Check signal strength

2. **Stuck on One Network**
   - Use button to cycle network modes
   - Check if other networks are available
   - Verify network priorities

3. **Connection Drops**
   - Check WiFi signal strength
   - Verify network stability
   - Check for interference

### Debug Information

The Serial Monitor shows:
- Network connection attempts
- Current network mode
- Connection status changes
- Reconnection attempts

## Adding More Networks

To add additional networks:

1. Add new entry to `wifiNetworks` array
2. Set appropriate priority (higher number = lower priority)
3. Choose descriptive location name
4. Rebuild and upload firmware

Example:
```cpp
{"OfficeWiFi", "OfficePass", "Office", 3},        // Third priority
{"CafeNetwork", "CafePass", "Cafe", 4},           // Fourth priority
```

## Advanced Configuration

### Custom Timeouts

Modify these values in `wifi_config.h`:

```cpp
#define WIFI_CONNECT_TIMEOUT_MS 10000    // 10 seconds per network
#define WIFI_RETRY_DELAY_MS 1000        // 1 second between retries
#define WIFI_MAX_RETRIES 3              // Max retries per network
```

### Custom Network Names

You can use any descriptive names for locations:

```cpp
{"Home5G", "HomePass", "Home 5GHz", 1},
{"WorkGuest", "GuestPass", "Work Guest", 2},
{"MobileHotspot", "MobilePass", "Mobile", 3},
```

## Support

If you encounter issues:
1. Check Serial Monitor for error messages
2. Verify network credentials
3. Test network availability from other devices
4. Check button press timing for mode changes
