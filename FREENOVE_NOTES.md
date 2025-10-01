# FreeNove ESP32-S3 WROOM Specific Notes

![FreeNove ESP32-S3 WROOM](ESP32-S3-WROOM.jpg)

## Board Information
- **Board**: FreeNove ESP32-S3 WROOM Clone with Camera Connector
- **MCU**: ESP32-S3-WROOM-1
- **Flash**: 8MB
- **PSRAM**: Usually none on basic models

## Pin Configuration Changes Made

### SD Card SDMMC Pins (FreeNove Specific)
The firmware has been updated to use FreeNove SDMMC interface:

```
SDMMC Configuration:
- CMD (Command): GPIO 38
- CLK (Clock): GPIO 39  
- DATA0: GPIO 40
```

### SDMMC vs SPI
Your FreeNove board uses **SDMMC interface** instead of SPI:
- **Faster**: SDMMC is much faster than SPI
- **Fewer pins**: Only needs 3 pins (CMD, CLK, DATA0) for 1-bit mode
- **Auto-detection**: Firmware tries both 1-bit and 4-bit modes
- **Multiple frequencies**: Automatically tests 20MHz, 10MHz, and 5MHz

### OneWire Temperature Sensors
- **Data Pin**: GPIO 4 (unchanged)
- **Pull-up**: 4.7kΩ resistor between GPIO 4 and 3.3V

## Camera Connector Considerations
FreeNove ESP32-S3 boards with camera connectors may have some GPIO pins reserved for camera functionality. If you experience issues:

1. **Check Pin Conflicts**: Some camera pins might conflict with other peripherals
2. **Camera Module**: If using camera module, ensure pin assignments don't overlap
3. **Alternative Pins**: The firmware tries multiple pin combinations automatically

## Common Issues and Solutions

### SD Card Not Detected
If SD card initialization fails on all pin combinations:

1. **Check Physical Connection**:
   - Ensure SD card is properly inserted
   - Verify wiring matches the updated pin assignments

2. **Board Variant**: FreeNove has multiple ESP32-S3 variants:
   - Some use different pin assignments
   - Check your specific board's pinout diagram
   - The firmware auto-detects, but manual verification helps

3. **Power Issues**:
   - Ensure stable 3.3V power supply
   - Some FreeNove boards need external power for SD card operations

### Temperature Sensors
- GPIO 4 should work consistently across FreeNove variants
- If sensors not detected, verify 4.7kΩ pull-up resistor

## Verification Steps

After flashing the firmware:

1. **Open Serial Monitor** (115200 baud):
   ```bash
   pio device monitor
   ```

2. **Check Boot Messages**:
   - Look for "Board: FreeNove ESP32-S3 WROOM Clone"
   - Monitor SD card initialization attempts on different pins
   - Verify temperature sensor detection

3. **Expected Output**:
   ```
   === ESP32 Temperature Logger ===
   Board: FreeNove ESP32-S3 WROOM Clone
   Starting initialization...
   
   === SD Card Initialization ===
   Trying CS pin 21 with 4000000 Hz SPI frequency...
   SUCCESS! SD card initialized on CS pin 21
   ```

## Board Identification
To confirm you have a FreeNove ESP32-S3 WROOM:

1. **Physical Inspection**:
   - Look for "FreeNove" branding on PCB
   - Camera connector (if present)
   - ESP32-S3-WROOM module

2. **Pin Labels**: Check silkscreen pin labels on board

3. **Documentation**: Refer to FreeNove's official documentation for your specific model

## Alternative Pin Configurations
If the default configuration doesn't work, you can manually edit `src/main.cpp`:

```cpp
// Try these alternatives for FreeNove variants:
#define SD_CS_PIN 5      // Alternative CS pin
#define SD_MOSI_PIN 23   // Alternative MOSI
#define SD_MISO_PIN 19   // Alternative MISO  
#define SD_SCK_PIN 18    // Alternative SCK
```

The firmware's auto-detection should handle most cases, but manual configuration may be needed for specific board revisions.

## Support
If you continue experiencing issues:
1. Check FreeNove's official documentation for your board revision
2. Verify connections with multimeter
3. Try a different SD card
4. Enable verbose logging in platformio.ini