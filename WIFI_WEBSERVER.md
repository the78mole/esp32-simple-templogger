# WiFi und WebServer Integration

## Übersicht
Der ESP32 Temperature Logger wurde um WiFi-Konnektivität und einen eingebauten WebServer erweitert. Dies ermöglicht den ferngesteuerten Zugriff auf aktuelle Temperaturdaten und den Download der CSV-Logs über eine benutzerfreundliche Web-Oberfläche.

## Neue Funktionen

### 1. WiFi-Konfiguration
Die `config.toml` wurde um WiFi-Parameter erweitert:

```toml
# WiFi Configuration
# Set your WiFi credentials to enable web interface
# Leave as defaults to disable WiFi
wifi_ssid = "YOUR_WIFI_SSID"
wifi_password = "YOUR_WIFI_PASSWORD"

# Web Server Configuration
# Port for the web interface (default: 80)
webserver_port = 80
```

### 2. WebServer-Routen

#### Hauptseite (`/`)
- **URL**: `http://[ESP32-IP]/` oder `http://esp32-templogger.local/`
- **Funktionen**:
  - Echtzeitanzeige aller Temperatursensoren
  - Automatische Aktualisierung alle 5 Sekunden
  - Systemstatus (Uptime, WiFi-Signal, SD-Karte)
  - Responsive Design für mobile Geräte

#### JSON-API (`/data`)
- **URL**: `http://[ESP32-IP]/data`
- **Format**: JSON
- **Inhalt**:
  ```json
  {
    "timestamp": 1580,
    "uptime_ms": 125000,
    "wifi_rssi": -45,
    "sensor_count": 2,
    "sensors": [
      {
        "id": 0,
        "address": "28FF641E8315043C",
        "temperature_c": 24.56,
        "status": "ok"
      }
    ]
  }
  ```

#### CSV-Download (`/download`)
- **URL**: `http://[ESP32-IP]/download`
- **Funktion**: Direkter Download der kompletten `templog.csv`
- **Format**: CSV-Datei mit allen geloggten Temperaturdaten

### 3. mDNS-Support
- **Hostname**: `esp32-templogger.local`
- **Zugriff**: `http://esp32-templogger.local/` (ohne IP-Adresse)

## Konfiguration

### Schritt 1: WiFi-Einstellungen
1. Bearbeite die `config.toml` auf der SD-Karte
2. Trage deine WiFi-Zugangsdaten ein:
   ```toml
   wifi_ssid = "MeinWiFi"
   wifi_password = "MeinPasswort"
   ```
3. Optional: Ändere den WebServer-Port (Standard: 80)

### Schritt 2: ESP32 neustarten
Nach dem Ändern der Konfiguration den ESP32 neu starten.

### Schritt 3: IP-Adresse finden
Die IP-Adresse wird beim Start über die serielle Konsole ausgegeben:
```
WiFi connected successfully!
IP address: 192.168.1.100
WebServer started successfully!
Access web interface at: http://192.168.1.100:80
Or use: http://esp32-templogger.local:80
```

## Web-Interface Features

### Hauptansicht
- **Header**: Titel und ESP32-Logo
- **System Status**: 
  - Aktuelle Zeit (kontinuierlicher Zeitstempel)
  - System-Uptime
  - WiFi-Signalstärke
  - Anzahl gefundener Sensoren
  - Log-Intervall
  - SD-Karten-Status

### Sensor-Anzeige
- **Live-Temperaturen**: Alle angeschlossenen DS18B20-Sensoren
- **Sensor-Details**: ID, Adresse, aktuelle Temperatur
- **Fehler-Anzeige**: Deutliche Markierung defekter Sensoren
- **Auto-Refresh**: Automatische Aktualisierung alle 5 Sekunden

### Aktionsbuttons
- **📊 JSON Data**: Rohdaten im JSON-Format
- **📥 Download CSV**: Komplettes CSV-Log herunterladen
- **🔄 Refresh**: Manuelle Aktualisierung

## Technische Details

### Speicherverbrauch
- **RAM**: 14.5% (47,420 Bytes)
- **Flash**: 66.5% (872,065 Bytes)

### Neue Bibliotheken
- `WiFi.h` - WiFi-Konnektivität
- `WebServer.h` - HTTP-Server
- `ESPmDNS.h` - mDNS-Resolver

### Erweiterte Funktionen
```cpp
void initWiFi()           // WiFi-Verbindung aufbauen
void initWebServer()      // WebServer konfigurieren und starten
void handleRoot()         // Hauptseite anzeigen
void handleCurrentData()  // JSON-API für aktuelle Daten
void handleCSVDownload()  // CSV-Datei zum Download bereitstellen
String generateHTML()     // HTML-Interface generieren
```

## Sicherheitshinweise

### WiFi-Sicherheit
- Verwende starke WiFi-Passwörter
- Der WebServer läuft im lokalen Netzwerk
- Keine Verschlüsselung der HTTP-Verbindung

### Zugriffskontrolle
- Aktuell keine Benutzerauthentifizierung
- Jeder im Netzwerk kann auf die Daten zugreifen
- Für Produktionsumgebungen Authentifizierung empfohlen

## Fehlerbehebung

### WiFi-Verbindung schlägt fehl
1. Überprüfe SSID und Passwort in `config.toml`
2. Stelle sicher, dass das WiFi-Netzwerk verfügbar ist
3. Prüfe die serielle Ausgabe für Fehlermeldungen
4. Versuche WiFi-Reset durch Neustart

### WebServer nicht erreichbar
1. Überprüfe die IP-Adresse in der seriellen Ausgabe
2. Ping-Test: `ping 192.168.1.100`
3. Port-Konflikt: Ändere `webserver_port` in der Konfiguration
4. Firewall-Einstellungen prüfen

### mDNS funktioniert nicht
1. Stelle sicher, dass mDNS im Netzwerk unterstützt wird
2. Verwende die direkte IP-Adresse als Alternative
3. Auf Windows: Bonjour-Service installieren

## Kompatibilität
- **Browser**: Alle modernen Browser (Chrome, Firefox, Safari, Edge)
- **Mobile Geräte**: Responsive Design für Smartphones/Tablets  
- **Netzwerk**: 2.4 GHz WiFi (ESP32-Standard)
- **Router**: Standard-WiFi-Router mit mDNS-Support

## Beispiel-Nutzung

### Remote-Monitoring
```bash
# Aktuelle Temperaturen abrufen (JSON)
curl http://esp32-templogger.local/data

# CSV-Daten herunterladen
wget http://esp32-templogger.local/download -O temperatures.csv
```

### Integration in andere Systeme
Das JSON-API kann einfach in Smart-Home-Systeme, Datenlogger oder Monitoring-Tools integriert werden.

## Ausblick
Mögliche Erweiterungen:
- HTTPS-Verschlüsselung
- Benutzerauthentifizierung
- WebSocket für Echtzeitdaten
- Grafische Darstellung der Temperaturverläufe
- Export in verschiedene Formate
- Email-Benachrichtigungen bei Grenzwerten