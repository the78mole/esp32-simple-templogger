# Code Refactoring: Modularisierung

## Übersicht
Die monolithische `main.cpp` wurde in separate Module aufgeteilt, um eine bessere Code-Organisation, Wartbarkeit und Testbarkeit zu erreichen.

## Neue Dateistruktur

### 📂 src/
```
src/
├── main.cpp              # Hauptprogramm (vereinfacht)
├── sd_manager.h          # SD-Karten Header
├── sd_manager.cpp        # SD-Karten Implementation
├── web_server.h          # WebServer Header
└── web_server.cpp        # WebServer Implementation
```

## Module im Detail

### 🗃️ SD Manager (`sd_manager.h/.cpp`)

**Zweck**: Komplette Verwaltung aller SD-Karten-Operationen

**Hauptfunktionen**:
```cpp
class SDManager {
public:
    // Initialisierung
    bool initializeSD();
    bool isAvailable() const;
    
    // Konfiguration
    bool readConfig(...);
    bool createDefaultConfig(...);
    
    // CSV-Logging
    void createCSVHeader(int numberOfDevices);
    bool logTemperatureData(...);
    
    // Zeitverwaltung
    unsigned long getLastTimestamp();
    void initTimeOffset(...);
    
    // Dateioperationen
    File openLogFile(const char* mode);
    bool fileExists(const char* filename);
};
```

**Globale Instanz**: `extern SDManager sdManager;`

### 🌐 WebServer (`web_server.h/.cpp`)

**Zweck**: Komplette WiFi- und WebServer-Funktionalität

**Hauptfunktionen**:
```cpp
class TemperatureWebServer {
public:
    // WiFi Management
    bool initWiFi(const String& ssid, const String& password);
    bool isWiFiConnected() const;
    String getIPAddress() const;
    
    // WebServer Management
    bool initWebServer(int port);
    void handleClient();
    void stopServer();
    
    // Sensor-Daten
    void setSensorData(DallasTemperature* sensors, ...);
};
```

**Globale Instanz**: `extern TemperatureWebServer webServer;`

### 🎯 Vereinfachte main.cpp

**Neue Länge**: ~220 Zeilen (vorher: ~780 Zeilen)

**Vereinfachte Struktur**:
```cpp
#include "sd_manager.h"
#include "web_server.h"

void setup() {
    // 1. Serielle Initialisierung
    // 2. SD-Manager
    sdManager.initializeSD();
    sdManager.readConfig(...);
    
    // 3. Sensoren initialisieren
    initSensors();
    
    // 4. CSV und Zeitoffset
    sdManager.createCSVHeader(...);
    sdManager.initTimeOffset(...);
    
    // 5. WiFi und WebServer (falls aktiviert)
    webServer.initWiFi(...);
    webServer.initWebServer(...);
}

void loop() {
    // 1. WebServer-Anfragen bearbeiten
    webServer.handleClient();
    
    // 2. Temperatur-Logging
    if (Zeit für Log) {
        logTemperatures();
    }
}
```

## Vorteile der Modularisierung

### ✅ Wartbarkeit
- **Trennung der Verantwortlichkeiten**: Jedes Modul hat eine klare Aufgabe
- **Kleinere Dateien**: Leichter zu verstehen und zu bearbeiten
- **Isolierte Änderungen**: Änderungen in einem Modul beeinträchtigen andere nicht

### ✅ Wiederverwendbarkeit
- **SD Manager**: Kann in anderen ESP32-Projekten verwendet werden
- **WebServer**: Generisch für Temperatur-Monitoring-Projekte
- **Klar definierte APIs**: Einfache Integration in neue Projekte

### ✅ Testbarkeit
- **Unit Tests**: Jedes Modul kann separat getestet werden
- **Mock-Objekte**: Einfacheres Testen durch definierte Schnittstellen
- **Isolierte Fehlersuche**: Probleme können gezielt lokalisiert werden

### ✅ Code-Qualität
- **Reduzierte Komplexität**: Jede Datei hat einen klaren Fokus
- **Bessere Lesbarkeit**: Logik ist in passende Bereiche gruppiert
- **Konsistente APIs**: Einheitliche Namenskonventionen und Funktionsdesign

## Migrationspfad

### Vorher (Monolitisch)
```cpp
main.cpp (780 Zeilen)
├── SD-Karten Funktionen (200+ Zeilen)
├── WebServer Funktionen (300+ Zeilen)
├── WiFi Management (100+ Zeilen)
├── Sensor Logic (100+ Zeilen)
└── Setup/Loop (80+ Zeilen)
```

### Nachher (Modular)
```cpp
main.cpp (220 Zeilen)        # Orchestrierung
sd_manager.cpp (280 Zeilen)  # SD-Operationen
web_server.cpp (310 Zeilen)  # Web-Interface
```

## Speicherverbrauch
- **RAM**: 14.5% (47,444 Bytes) - nahezu unverändert
- **Flash**: 66.7% (873,821 Bytes) - minimal größer (+1,756 Bytes)

Die minimale Vergrößerung kommt durch:
- Zusätzliche Klassen-Overhead
- Virtuelle Funktionen (falls verwendet)
- Template-Instanziierungen

## API-Beispiele

### SD Manager verwenden
```cpp
// Initialisierung
if (sdManager.initializeSD()) {
    // Konfiguration lesen
    sdManager.readConfig(logInterval, wifiSSID, ...);
    
    // CSV erstellen
    sdManager.createCSVHeader(sensorCount);
    
    // Daten loggen
    sdManager.logTemperatureData(timestamp, millis(), 
                                sensorCount, addresses, temps);
}
```

### WebServer verwenden
```cpp
// WiFi verbinden
if (webServer.initWiFi(ssid, password)) {
    // Server starten
    if (webServer.initWebServer(80)) {
        // Sensor-Daten setzen
        webServer.setSensorData(&sensors, deviceCount, 
                               addresses, timeOffset);
    }
}

// In loop()
webServer.handleClient();
```

## Kompatibilität
- **Bestehende Hardware**: Voll kompatibel
- **Konfigurationsdateien**: Unverändert
- **CSV-Format**: Identisch
- **Web-Interface**: Gleiche Funktionalität
- **Verhalten**: Identisch zur vorherigen Version

## Zukünftige Erweiterungen

### Mögliche neue Module
- **sensor_manager.h/.cpp**: Sensor-spezifische Logik
- **config_manager.h/.cpp**: Erweiterte Konfigurationsverwaltung
- **network_manager.h/.cpp**: Erweiterte Netzwerk-Features
- **data_logger.h/.cpp**: Verschiedene Logging-Backends

### Einfache Integration
- **MQTT**: Neues Modul `mqtt_client.h/.cpp`
- **InfluxDB**: Integration via `database_client.h/.cpp`
- **OTA Updates**: Modul `ota_manager.h/.cpp`

## Entwicklernutzen

### Parallele Entwicklung
- **Teams**: Verschiedene Entwickler können an verschiedenen Modulen arbeiten
- **Features**: Neue Features können isoliert entwickelt werden
- **Testing**: Module können unabhängig getestet werden

### Debugging
- **Isolierte Probleme**: Fehler sind einfacher zu lokalisieren
- **Modulare Logs**: Logging kann pro Modul aktiviert/deaktiviert werden
- **Focused Development**: Weniger Code zum Durchsuchen

## Fazit
Das Refactoring hat den Code erheblich strukturierter und wartbarer gemacht, ohne die Funktionalität zu beeinträchtigen. Die neuen Module sind wiederverwendbar und können als Basis für zukünftige ESP32-Projekte dienen.