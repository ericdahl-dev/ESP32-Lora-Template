# ESP32 Template OTA (Over-The-Air) Update System

This template includes a comprehensive OTA update system that supports WiFi-based firmware updates with optional LoRa relay capabilities.

## How It Works

### **WiFi OTA (Primary Method):**
- **Direct Updates**: Devices can receive updates directly via WiFi from Arduino IDE or web interface
- **Web Interface**: Built-in web server for firmware uploads
- **Password Protection**: Secure OTA updates with configurable passwords

### **USB Fallback:**
- **Development**: Standard USB upload during development
- **Recovery**: Emergency firmware recovery if OTA fails

## Setup Instructions

### 1. Configure WiFi (Receiver only)

Edit your WiFi configuration file with your credentials:

```cpp
// In wifi_networks.h or wifi_config.h
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"
#define OTA_HOSTNAME "ESP32-Template"
#define OTA_PASSWORD "secure_password"  // Change this password
```

### 2. Build and Upload

```bash
# Build template with WiFi OTA enabled
pio run -e template_ota --target upload

# Build example project with OTA
pio run -e environmental_monitor --target upload
```

## Using WiFi OTA (Receiver)

### From Arduino IDE:
1. Connect receiver to WiFi
2. In Arduino IDE: Tools → Port → Select the WiFi IP address
3. Upload new firmware normally

### From PlatformIO:
1. Connect device to WiFi
2. Use the IP address for upload:
```bash
pio run -e template_ota --target upload --upload-port 192.168.1.100
```

## Web-Based OTA Updates

### Built-in Web Interface:
1. Device creates web server when connected to WiFi
2. Navigate to device IP address in browser
3. Upload firmware file (.bin) through web interface
4. Device automatically installs and reboots

### Manual Web OTA:
1. Build firmware: `pio run -e your_env`
2. Locate firmware: `.pio/build/your_env/firmware.bin`
3. Upload via web interface
4. Monitor progress and wait for reboot

## Advanced OTA Features

### Automatic Update Check:
- Device can check for updates from a configured server
- Downloads and installs updates automatically
- Configurable update intervals and channels

### Rollback Protection:
- Previous firmware is preserved during update
- Automatic rollback if new firmware fails to boot
- Manual rollback trigger via button or web interface

## Security Features

- **WiFi OTA**: Password-protected (configurable in `wifi_config.h`)
- **Web OTA**: HTTPS support with certificate validation
- **Firmware Validation**: ESP32 Update library validates firmware before flashing

## Troubleshooting

### WiFi Connection Issues:
- Check WiFi credentials in `wifi_config.h`
- Verify WiFi signal strength
- Check serial monitor for connection status

### Web OTA Issues:
- Ensure device is connected to WiFi
- Check web server is accessible (try device IP)
- Verify firmware file is valid (.bin format)

### Firmware Update Failures:
- Check available flash space
- Verify firmware size compatibility
- Monitor serial output for error messages

## Configuration Options

### Build Flags:
- `ENABLE_WIFI_OTA`: Enables WiFi OTA functionality
- `ENABLE_WEB_OTA`: Enables web-based OTA interface
- `OTA_AUTO_UPDATE`: Enables automatic update checking

### OTA Timeouts:
- WiFi OTA: No timeout (handled by ArduinoOTA)
- Web OTA: 60 seconds default (configurable)
- Auto Update: 24 hours default check interval

## Example Usage

### Basic OTA Setup:
```cpp
#ifdef ENABLE_WIFI_OTA
  initWiFi();
  if (wifiConnected) {
    initOTA();
    setupWebOTA();  // Optional web interface
  }
#endif
```

### Custom OTA Handler:
```cpp
void onOTAStart() {
  display.clear();
  display.print("OTA Update...");
}

void onOTAProgress(unsigned int progress, unsigned int total) {
  int percent = (progress * 100) / total;
  display.printf("Progress: %d%%", percent);
}
```

## Notes

- **Firmware Size**: Web OTA supports full-size firmware files
- **Reliability**: WiFi OTA includes checksums and integrity verification
- **Battery**: OTA updates consume power, ensure adequate battery for field devices
- **Backup**: Always keep a working firmware backup for USB recovery

## Future Enhancements

- [ ] Encrypted firmware updates
- [ ] Signed firmware validation
- [ ] Delta updates (only changed portions)
- [ ] Multi-stage updates with verification
- [ ] Remote diagnostics and monitoring
