# LtngDet OTA (Over-The-Air) Update System

This project now includes a hybrid OTA update system that allows both WiFi and LoRa-based firmware updates.

## How It Works

### **Receiver (WiFi-enabled location):**
- **WiFi OTA**: Can receive updates directly via WiFi from Arduino IDE or web interface
- **LoRa Bridge**: Acts as a bridge to relay firmware updates to transmitters

### **Transmitter (Remote/no WiFi):**
- **LoRa OTA**: Receives firmware updates via LoRa from the receiver
- **Fallback**: Can still be updated via USB if needed

## Setup Instructions

### 1. Configure WiFi (Receiver only)

Edit `src/wifi_config.h` with your WiFi credentials:

```cpp
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"
#define OTA_HOSTNAME "LtngDet-Receiver"
#define OTA_PASSWORD "123456"  // Change this password
```

### 2. Build and Upload

```bash
# Build receiver with WiFi OTA
pio run -e receiver --target upload

# Build transmitter (no WiFi needed)
pio run -e sender --target upload
```

## Using WiFi OTA (Receiver)

### From Arduino IDE:
1. Connect receiver to WiFi
2. In Arduino IDE: Tools → Port → Select the WiFi IP address
3. Upload new firmware normally

### From PlatformIO:
1. Connect receiver to WiFi
2. Use the IP address for upload:
```bash
pio run -e receiver --target upload --upload-port 192.168.1.100
```

## Using LoRa OTA (Transmitter)

### Automatic Updates:
1. Receiver gets new firmware via WiFi OTA
2. Receiver automatically broadcasts firmware to all transmitters
3. Transmitters receive and flash new firmware
4. Transmitters reboot with new firmware

### Manual LoRa OTA:
1. Receiver can manually trigger LoRa OTA using the `sendLoraOtaUpdate()` function
2. Useful for updating specific transmitters or testing

## OTA Protocol

### LoRa OTA Packet Types:

- **OTA_START:size:timeout** - Initiates OTA update
- **OTA_DATA:chunk:data** - Contains firmware chunk data
- **OTA_END:** - Signals end of OTA update

### Example LoRa OTA Flow:
```
Receiver → OTA_START:32768:30000
Receiver → OTA_DATA:0:[firmware chunk 0]
Receiver → OTA_DATA:1:[firmware chunk 1]
...
Receiver → OTA_DATA:N:[firmware chunk N]
Receiver → OTA_END:
Transmitter → Flashes firmware and reboots
```

## Security Features

- **WiFi OTA**: Password-protected (configurable in `wifi_config.h`)
- **LoRa OTA**: Uses same LoRa network, no additional security
- **Firmware Validation**: ESP32 Update library validates firmware before flashing

## Troubleshooting

### WiFi Connection Issues:
- Check WiFi credentials in `wifi_config.h`
- Verify WiFi signal strength
- Check serial monitor for connection status

### LoRa OTA Issues:
- Ensure both devices are on same LoRa frequency
- Check LoRa signal strength
- Monitor serial output for OTA progress

### Firmware Update Failures:
- Check available flash space
- Verify firmware size compatibility
- Monitor serial output for error messages

## Configuration Options

### Build Flags:
- `ENABLE_WIFI_OTA`: Enables WiFi OTA for receiver builds
- `ROLE_SENDER`: Builds transmitter firmware
- `ROLE_RECEIVER`: Builds receiver firmware with WiFi OTA

### OTA Timeouts:
- WiFi OTA: No timeout (handled by ArduinoOTA)
- LoRa OTA: 30 seconds default (configurable)

## Example Usage

### Receiver Code:
```cpp
#ifdef ENABLE_WIFI_OTA
  if (!isSender) {
    initWiFi();
    if (wifiConnected) {
      initOTA();
    }
  }
#endif
```

### Transmitter Code:
```cpp
// Automatically handles LoRa OTA packets
// No additional code needed
```

## Notes

- **Firmware Size**: LoRa OTA is limited by LoRa packet size and memory
- **Reliability**: LoRa OTA includes error checking and timeout handling
- **Battery**: OTA updates consume power, ensure adequate battery for field devices
- **Backup**: Always keep a working firmware backup for USB recovery

## Future Enhancements

- [ ] Base64 encoding for LoRa OTA data
- [ ] CRC checksums for firmware validation
- [ ] Selective transmitter updates
- [ ] OTA progress reporting via LoRa
- [ ] Firmware rollback capability
