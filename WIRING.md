# Wiring Diagram for ESP32 Simple Temperature Logger

## Overview
```
                    ┌─────────────────────┐
                    │   ESP32-S3-WROOM    │
                    │                     │
                    │                     │
 DS18B20 Sensors    │  GPIO 4 ◄───────────┼─── OneWire Data (with 4.7kΩ pullup to 3.3V)
                    │                     │
    ┌───────────────┤  3.3V               │
    │               │                     │
    │       ┌───────┤  GND                │
    │       │       │                     │
    │       │       │  GPIO 10 ───────────┼─── SD Card CS (if using SPI)
    │       │       │                     │
    │       │       │  [SD Card Slot]     │
    │       │       │                     │
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
