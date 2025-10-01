# Troubleshooting Guide

## SD Card Issues

### Error Messages:
```
[W][sd_diskio.cpp:104] sdWait(): Wait Failed
[E][sd_diskio.cpp:806] sdcard_mount(): f_mount failed: (3) The physical drive cannot work
[E][sd_diskio.cpp:126] sdSelectCard(): Select Failed
```

### Solutions:

1. **Check Hardware Connections**
   - Verify all SPI pins are correctly connected (see WIRING.md)
   - Ensure proper power supply (3.3V) to SD card
   - Check for loose connections or poor solder joints

2. **SD Card Issues**
   - Try a different SD card (some cards are incompatible)
   - Format SD card as FAT32
   - Ensure SD card is not corrupted
   - Use a high-quality SD card (avoid cheap/counterfeit cards)
   - Try a smaller capacity card (≤32GB works best)

3. **Pin Configuration**
   - The firmware automatically tries multiple CS pins: 10, 5, 21, 15, 2
   - Check your ESP32-S3 board's pinout diagram
   - Some development boards have different default SPI pins

4. **SPI Speed Issues**
   - The firmware automatically tries multiple SPI speeds: 4MHz, 1MHz, 400kHz
   - Lower speeds are more reliable for longer cables or noisy environments

5. **Power Issues**
   - Ensure stable 3.3V power supply
   - SD cards can draw significant current during write operations
   - Try powering from external 3.3V regulator if using USB power

### Fallback Mode
If SD card initialization fails completely, the system will continue operating in Serial-only mode:
- Temperature data will be displayed on Serial Monitor
- Configuration will use default values
- No data will be saved to SD card

## Temperature Sensor Issues

### No Sensors Found
```
WARNING: No temperature sensors found!
```

### Solutions:

1. **Check Wiring**
   - Verify DS18B20 VCC connected to 3.3V
   - Verify DS18B20 GND connected to GND
   - Verify DS18B20 Data connected to GPIO 4

2. **Pull-up Resistor**
   - Ensure 4.7kΩ resistor between Data line and 3.3V
   - For long cables (>3m), try 2.2kΩ resistor
   - Only one pull-up resistor needed for entire bus

3. **Sensor Issues**
   - Try with just one sensor first
   - Verify sensor is not damaged
   - Check sensor pinout (see WIRING.md)

4. **Cable Length**
   - Keep cables as short as possible for testing
   - For long runs, use shielded cables
   - Consider lower pull-up resistance for long cables

### Temperature Reading Errors
```
Sensor0: ERROR
```

### Solutions:

1. **Intermittent Connection**
   - Check for loose wires
   - Verify solder joints
   - Check for cable damage

2. **Power Issues**
   - Ensure stable power supply to sensors
   - Check voltage levels (should be close to 3.3V)

3. **Bus Conflicts**
   - If multiple sensors, try with one at a time
   - Check for duplicate addresses (very rare)

## General Tips

1. **Serial Monitor**
   - Use 115200 baud rate
   - Enable timestamps to track initialization progress

2. **Power Cycling**
   - Sometimes a power cycle resolves temporary issues
   - Disconnect and reconnect power completely

3. **Firmware Update**
   - Ensure you're using the latest firmware version
   - Check for updates to ESP32 Arduino core

4. **Hardware Testing**
   - Test components individually
   - Use a multimeter to verify connections
   - Check continuity of all wires

## Getting Help

If issues persist after trying these solutions:

1. Enable detailed debug output (already enabled in platformio.ini)
2. Copy the complete Serial Monitor output during startup
3. Document your exact hardware setup and connections
4. Include photos of your wiring if possible
5. Create an issue in the project repository with this information