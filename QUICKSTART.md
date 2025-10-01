# Quick Start Guide

## Schnellstart in 5 Schritten

### 1. Hardware vorbereiten
- ESP32-S3-WROOM Board
- DS18B20 Sensor(en) mit 4.7kΩ Pull-up Widerstand
- SD-Karte (FAT32 formatiert)

### 2. Verkabelung
```
DS18B20:
- VCC → 3.3V
- GND → GND  
- DATA → GPIO 4 (+ 4.7kΩ Pull-up zu 3.3V)
```

Siehe [WIRING.md](WIRING.md) für Details.

### 3. SD-Karte vorbereiten
1. SD-Karte formatieren (FAT32)
2. Datei `config.toml` erstellen:
```toml
log_interval_seconds = 60
```

### 4. Firmware flashen
```bash
# PlatformIO installieren
pip install platformio

# Repository klonen
git clone https://github.com/the78mole/esp32-simple-templogger.git
cd esp32-simple-templogger

# Kompilieren und hochladen
pio run --target upload
```

### 5. Betrieb
- SD-Karte einlegen
- ESP32 mit Strom versorgen
- Logger läuft automatisch
- Daten werden in `templog.csv` gespeichert

## Daten auslesen

1. ESP32 ausschalten
2. SD-Karte entnehmen
3. `templog.csv` auf Computer kopieren
4. Mit Excel, LibreOffice Calc oder Python auswerten

## Beispiel CSV-Daten
```csv
Timestamp,Millis,Sensor0_Addr,Sensor0_TempC,Sensor1_Addr,Sensor1_TempC
60,60000,28FF1234567890AB,45.25,28FF0987654321CD,32.50
120,120000,28FF1234567890AB,45.31,28FF0987654321CD,32.48
```

## Monitoring (optional)
```bash
# Seriellen Monitor öffnen
pio device monitor

# Ausgabe:
# === ESP32 Temperature Logger ===
# Starting initialization...
# Initializing SD card...OK
# Found 2 device(s)
# --- Logging temperatures ---
# Time: 60 s, Sensor0: 45.25°C Sensor1: 32.50°C
```

## Anpassungen

### Anderer GPIO-Pin für OneWire
In `src/main.cpp`:
```cpp
#define ONE_WIRE_BUS 4  // Auf gewünschten Pin ändern
```

### Anderes Log-Intervall
In `config.toml` auf SD-Karte:
```toml
log_interval_seconds = 30  # Alle 30 Sekunden
```

## Problemlösung

| Problem | Lösung |
|---------|--------|
| SD card initialization failed | SD-Karte prüfen, FAT32 formatieren |
| No temperature sensors found | 4.7kΩ Pull-up prüfen, Verkabelung prüfen |
| Sensor zeigt -127°C | Sensor defekt oder nicht richtig verbunden |
| Log-Datei wird nicht erstellt | SD-Karte Schreibschutz prüfen |

## Support

Bei Problemen bitte Issue auf GitHub erstellen:
https://github.com/the78mole/esp32-simple-templogger/issues
