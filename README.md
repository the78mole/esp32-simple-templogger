# esp32-simple-templogger
Ein einfacher Temperaturlogger zur Analyse von Heizungsanlagen

## Beschreibung

Dieses Projekt implementiert einen einfachen Temperaturlogger für die Analyse von Heizungsanlagen. Es verwendet einen ESP32-S3-WROOM Mikrocontroller mit SD-Kartenslot und ein oder mehrere DS18B20 OneWire-Temperatursensoren. Die Messwerte werden in einer CSV-Datei auf der SD-Karte gespeichert.

## Features

- ✅ Unterstützung für mehrere DS18B20 Temperatursensoren (bis zu 10)
- ✅ Speicherung der Messwerte als CSV-Datei auf SD-Karte
- ✅ Konfigurierbare Log-Intervalle über config.toml
- ✅ Automatische Sensor-Erkennung
- ✅ Sensor-Adressen werden mitgeloggt für eindeutige Zuordnung
- ✅ Timestamps für jede Messung

## Hardware-Anforderungen

### Komponenten
- ESP32-S3-WROOM Development Board mit SD-Kartenslot
- 1-10x DS18B20 Temperatursensoren
- 1x 4.7kΩ Pull-up Widerstand für den OneWire-Bus
- SD-Karte (FAT32 formatiert)
- Verbindungskabel

### Verkabelung

#### DS18B20 Sensoren (OneWire-Bus)
- VCC → 3.3V
- GND → GND
- DATA → GPIO 4 (mit 4.7kΩ Pull-up zu 3.3V)

**Wichtig:** Alle DS18B20 Sensoren werden parallel am selben OneWire-Bus (GPIO 4) angeschlossen.

#### SD-Karte
Die SD-Karte wird über den eingebauten SD-Kartenslot des ESP32-S3 Boards verbunden.
- CS → GPIO 10 (kann in main.cpp angepasst werden)

## Software-Installation

### Voraussetzungen
- [PlatformIO](https://platformio.org/) installiert (als VS Code Extension oder CLI)
- USB-Treiber für ESP32-S3

### Projekt kompilieren und hochladen

1. Repository klonen:
```bash
git clone https://github.com/the78mole/esp32-simple-templogger.git
cd esp32-simple-templogger
```

2. Projekt mit PlatformIO kompilieren:
```bash
pio run
```

3. Firmware auf ESP32 hochladen:
```bash
pio run --target upload
```

4. Seriellen Monitor öffnen (optional, für Debugging):
```bash
pio device monitor
```

## Konfiguration

### config.toml auf SD-Karte

1. SD-Karte am Computer formatieren (FAT32)
2. Datei `config.toml` im Root-Verzeichnis der SD-Karte erstellen
3. Beispielkonfiguration:

```toml
# ESP32 Temperature Logger Configuration
# Logging interval in seconds
log_interval_seconds = 60
```

**Hinweis:** Die Datei `data/config.toml` in diesem Repository dient als Vorlage.

### Anpassbare Parameter

| Parameter | Beschreibung | Standard | Empfohlener Bereich |
|-----------|--------------|----------|---------------------|
| `log_interval_seconds` | Intervall zwischen Messungen in Sekunden | 60 | 10-3600 |

## Verwendung

1. Hardware aufbauen gemäß Verkabelungsplan
2. SD-Karte mit `config.toml` vorbereiten
3. SD-Karte in ESP32 einlegen
4. ESP32 mit Strom versorgen
5. Logger startet automatisch und speichert Daten in `templog.csv`

### CSV-Dateiformat

Die Datei `templog.csv` wird automatisch auf der SD-Karte erstellt und hat folgendes Format:

```csv
Timestamp,Millis,Sensor0_Addr,Sensor0_TempC,Sensor1_Addr,Sensor1_TempC,...
60,60000,28FF1234567890AB,45.25,28FF0987654321CD,32.50,...
120,120000,28FF1234567890AB,45.31,28FF0987654321CD,32.48,...
```

**Spalten:**
- `Timestamp`: Zeit seit Start in Sekunden
- `Millis`: Zeit seit Start in Millisekunden
- `SensorX_Addr`: Eindeutige 64-bit Adresse des Sensors (Hex)
- `SensorX_TempC`: Temperatur in Grad Celsius

## Debugging

Der Logger gibt detaillierte Informationen über die serielle Schnittstelle aus (115200 Baud):

```
=== ESP32 Temperature Logger ===
Starting initialization...
Initializing SD card...OK
SD card size: 7580 MB
Reading configuration from SD card...OK
Log interval set to: 60 seconds
Initializing temperature sensors...Found 2 device(s)
Sensor 0 address: 28FF1234567890AB
Sensor 1 address: 28FF0987654321CD
Creating CSV log file...OK
Initialization complete!
Logging interval: 60 seconds
Number of sensors: 2

--- Logging temperatures ---
Time: 60 s, Sensor0: 45.25°C Sensor1: 32.50°C
Data logged successfully
```

### Häufige Probleme

**Problem:** "SD card initialization failed"
- Lösung: SD-Karte überprüfen, FAT32 formatieren, Verkabelung prüfen

**Problem:** "No temperature sensors found"
- Lösung: 4.7kΩ Pull-up Widerstand prüfen, Verkabelung prüfen, Sensor-Stromversorgung prüfen

**Problem:** Log-Intervall wird nicht aus config.toml gelesen
- Lösung: Dateiname prüfen (muss exakt `config.toml` sein), Syntax in der Datei prüfen

## Pin-Konfiguration anpassen

Falls andere GPIO-Pins verwendet werden sollen, können diese in `src/main.cpp` angepasst werden:

```cpp
#define ONE_WIRE_BUS 4        // GPIO pin für OneWire-Bus
#define SD_CS_PIN 10          // SD-Karte Chip Select Pin
```

## Lizenz

MIT License - siehe [LICENSE](LICENSE) Datei für Details.

## Autor

Daniel Glaser (the78mole)

## Beiträge

Contributions sind willkommen! Bitte erstelle einen Pull Request oder Issue für Verbesserungsvorschläge.
