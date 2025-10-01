# Wiring Diagram for ESP32 Simple Temperature Logger

## Board Compatibility
This project supports:
- **FreeNove ESP32-S3 WROOM Clone** (with Camera Connector) - Primary target
- Standard ESP32-S3-DevKitC-1
- Other ESP32-S3 variants

## Overview for FreeNove ESP32-S3 WROOM
```
                    ┌─────────────────────┐
                    │ FreeNove ESP32-S3   │
                    │   WROOM Clone       │
                    │                     │
 DS18B20 Sensors    │  GPIO 4 ◄───────────┼─── OneWire Data (with 4.7kΩ pullup to 3.3V)
                    │                     │
    ┌───────────────┤  3.3V               │
    │               │                     │
    │       ┌───────┤  GND                │
    │       │       │                     │
    │       │       │                     │ SD Card SDMMC Interface (FreeNove):
    │       │       │  GPIO 38 ───────────┼─── SD Card CMD (Command)
    │       │       │  GPIO 39 ───────────┼─── SD Card CLK (Clock)
    │       │       │  GPIO 40 ───────────┼─── SD Card DATA (D0)
    │       │       │                     │
    │       │       │ [Camera Connector]  │
    │       │       └─────────────────────┘
    │       │
    │       │
   VCC     GND
    │       │
    ┴───────┴───────────────┐
    │           │           │
┌───┴───┐   ┌───┴───┐   ┌───┴───┐
│DS18B20│   │DS18B20│   │DS18B20│  ...
│ #1    │   │ #2    │   │ #N    │
└───┬───┘   └───┬───┘   └───┬───┘
    │           │           │
    └───────────┴───────────┘
                │
                └─── OneWire Data Line (all connected in parallel)
```

## FreeNove ESP32-S3 WROOM SD Card SDMMC Pins
The firmware uses the SDMMC interface (not SPI) with these pins:

- **CMD (Command)**: GPIO 38
- **CLK (Clock)**: GPIO 39  
- **DATA0**: GPIO 40

**Important Notes for FreeNove SDMMC:**
- This board uses SDMMC interface, not SPI
- Much faster than SPI interface
- Supports both 1-bit and 4-bit modes
- The firmware automatically detects the best mode and frequency

## DS18B20 Pinout (TO-92 Package)
```
   Looking at flat side:
   
   ┌─────────────┐
   │   DS18B20   │
   │             │
   └──┬───┬───┬──┘
      │   │   │
      1   2   3
      │   │   │
     GND DATA VCC
      │   │   │
      │   │   └─── to 3.3V
      │   │
      │   └─────── to GPIO 4 (with 4.7kΩ pullup to 3.3V)
      │
      └─────────── to GND
```

## Pull-up Resistor Connection
```
        3.3V
         │
         ├─── 4.7kΩ Resistor
         │
    GPIO 4 ◄──┴─── All DS18B20 DATA pins
```

## Notes
- All DS18B20 sensors share the same OneWire bus (GPIO 4)
- Only ONE 4.7kΩ pull-up resistor is needed for the entire bus
- For long cables (>3m), consider using lower pull-up value (2.2kΩ)
- Parasitic power mode is NOT used - all sensors need VCC connection
- SD card connection depends on your ESP32-S3 board's SD slot design
