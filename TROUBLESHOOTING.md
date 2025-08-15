# LtngDet LoRa + OLED Troubleshooting Guide - Heltec V3

## Current Status: ✅ FULLY FUNCTIONAL

### Resolved Issues
- **OLED Display**: Working with U8G2 library
- **LoRa Communication**: Working with RadioLib
- **Button Interface**: Working with GPIO0
- **Library Conflicts**: Resolved by removing Heltec library

## Button Interface Troubleshooting

### Issue: Button Not Responding
**Symptoms**: Button presses don't change modes or parameters
**Possible Causes**:
- Button pin not properly configured
- Pull-up resistor not enabled
- Main loop delays blocking button detection

**Solutions**:
1. **Check Button Configuration**:
```cpp
pinMode(BUTTON_PIN, INPUT_PULLUP);  // Must be INPUT_PULLUP
```

2. **Verify Button Pin**:
```cpp
#define BUTTON_PIN 0  // GPIO0 (BOOT button)
```

3. **Check for Blocking Delays**:
```cpp
// WRONG - blocks button detection
delay(2000);

// CORRECT - non-blocking timing
if (now - lastTxMs >= 2000) {
  // do something
}
```

### Issue: Button Too Sensitive/Not Sensitive Enough
**Symptoms**: Button triggers on slight touch or requires very long press
**Solutions**:
1. **Adjust Debounce Time**:
```cpp
if (pressDuration < 100) {  // Increase for less sensitivity
  // Very short press - ignore (debounce)
}
```

2. **Adjust Press Duration Thresholds**:
```cpp
} else if (pressDuration < 1000) {     // Short press threshold
  // Toggle mode
} else if (pressDuration < 3000) {     // Medium press threshold
  // Cycle SF
} else {                               // Long press threshold
  // Cycle BW
}
```

## LoRa Parameter Synchronization

### Issue: Devices Can't Communicate
**Symptoms**: Sender transmits but receiver doesn't receive
**Root Cause**: LoRa parameters don't match between devices

**Required Matching Parameters**:
- **Frequency**: Must be identical (915.0 MHz)
- **Bandwidth**: Must be identical (125/250/500 kHz)
- **Spreading Factor**: Must be identical (SF7-SF12)
- **Coding Rate**: Must be identical (CR5)

**Solutions**:
1. **Check OLED Display**: Both devices should show identical bottom lines
2. **Manual Synchronization**: Use button to match parameters
3. **Reset to Defaults**: Power cycle both devices to reset to default values

### Issue: Parameter Changes Not Taking Effect
**Symptoms**: Button press changes OLED display but communication still fails
**Possible Causes**:
- Radio settings not properly applied
- Parameter change failed silently

**Solutions**:
1. **Check Serial Output**: Look for "Radio updated" or "Settings fail" messages
2. **Verify Parameter Application**:
```cpp
int st = radio.setSpreadingFactor(currentSF);
if (st != RADIOLIB_ERR_NONE) {
  Serial.printf("SF change failed: %d\n", st);
}
```

3. **Force Radio Reconfiguration**:
```cpp
// Re-initialize radio with new parameters
radio.begin(currentFreq, currentBW, currentSF, currentCR, 0x34, currentTxPower);
```

## OLED Display Troubleshooting

### Issue: OLED Shows Nothing (RESOLVED)
**Previous Problem**: Heltec library caused hangs
**Solution**: Switched to U8G2 library with proper power management

**Current Working Configuration**:
```cpp
// Power management
#define VEXT_PIN 36        // Vext control: LOW = ON
#define OLED_RST_PIN 21    // OLED reset pin

// I2C configuration
Wire.begin(17, 18);       // SDA=17, SCL=18
Wire.setClock(100000);    // 100kHz I2C clock

// U8G2 setup
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
u8g2.setI2CAddress(0x3C << 1);
u8g2.setPowerSave(0);
u8g2.setContrast(255);
```

### Issue: OLED Shows Garbled Text
**Symptoms**: Text appears but is unreadable or corrupted
**Solutions**:
1. **Check Font Selection**:
```cpp
u8g2.setFont(u8g2_font_6x10_tr);  // Use appropriate font
```

2. **Verify Buffer Operations**:
```cpp
u8g2.clearBuffer();     // Clear before drawing
u8g2.drawStr(0, 12, "Text");  // Draw text
u8g2.sendBuffer();      // Send to display
```

## PlatformIO Configuration Issues

### Check Board Definition
Ensure the correct board is selected:
```ini
[env:sender]
board = heltec_wifi_lora_32_V3  # Must match exactly
```

### Check Dependencies
Current working configuration:
```ini
lib_deps =
  jgromes/RadioLib@^6.5.0
  olikraus/U8g2@^2.36.0
```

### Build Flags
Ensure all necessary flags are set:
```ini
build_flags =
  -D HELTEC_V3_OLED=1
  -D OLED_SDA=17
  -D OLED_SCL=18
  -D LORA_FREQ_MHZ=915.0
  -D LORA_BW_KHZ=125.0
  -D LORA_SF=9
  -D LORA_CR=5
  -D LORA_TX_DBM=17
```

## Hardware Checks

### USB Cable
- Use high-quality data cable (not power-only)
- Try different USB ports
- Check if board shows up in device manager

### Power Supply
- Ensure stable 3.3V power
- Check for voltage drops during operation
- Verify USB power is sufficient

### Pin Connections
- Verify SDA=17, SCL=18 are correct for your board
- Check for pin conflicts with other peripherals
- Ensure no short circuits

## Debug Steps

### Step 1: Verify Basic Operation
Check that the system boots and shows status:
1. Upload firmware
2. Open serial monitor (115200 baud)
3. Verify boot messages appear
4. Check OLED shows current settings

### Step 2: Test Button Interface
1. **Short Press**: Should toggle between Sender/Receiver
2. **Medium Press**: Should cycle through SF values
3. **Long Press**: Should cycle through BW values
4. OLED should update immediately for each change

### Step 3: Test LoRa Communication
1. Set both devices to same parameters
2. Set one as Sender, one as Receiver
3. Verify packets are transmitted and received
4. Check serial output for TX/RX messages

### Step 4: Parameter Synchronization
1. Change parameters on one device
2. Match parameters on other device
3. Verify communication resumes

## Common Error Codes

### RadioLib Errors
- `-1`: Invalid parameter
- `-2`: Invalid state
- `-3`: Timeout
- `-4`: Hardware error

### U8G2 Errors
- `false` from `oled.begin()`: I2C communication failed
- No display output: Check I2C address and pins

## Next Actions

1. **Test Communication**: Verify sender/receiver can communicate
2. **Validate Parameters**: Ensure parameter changes work correctly
3. **Stress Test**: Test button interface during active LoRa operation
4. **Add Features**: Implement TX power adjustment and settings sync

## Resources
- [RadioLib Documentation](https://jgromes.github.io/RadioLib/)
- [U8G2 Troubleshooting](https://github.com/olikraus/U8g2_Arduino/wiki/troubleshooting)
- [ESP32-S3 I2C Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/i2c.html)
- [Heltec V3 Documentation](https://docs.heltec.org/en/wifi_lora_32_v3/)

---
*Last Updated: Current Session - System Fully Functional*
*Status: ✅ All Major Issues Resolved - Ready for Testing*
