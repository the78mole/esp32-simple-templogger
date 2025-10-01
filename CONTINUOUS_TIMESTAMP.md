# Kontinuierliche Zeitstempel nach Neustart

## Problem
Nach jedem Neustart des ESP32 begann der Zeitstempel in der CSV-Datei wieder bei 60 Sekunden, anstatt die Zeit vom letzten Eintrag fortzusetzen. Dies führte zu inkonsistenten Zeitachsen in den Logdaten.

## Lösung
Implementierung eines Systems zur automatischen Fortsetzung der Zeitstempel basierend auf dem letzten CSV-Eintrag.

### Hinzugefügte Funktionen

#### 1. getLastTimestamp()
```cpp
unsigned long getLastTimestamp()
```
- Liest die CSV-Datei zeilenweise
- Findet den höchsten Zeitstempel
- Ignoriert Header-Zeilen und ungültige Einträge
- Gibt 0 zurück, wenn keine gültigen Einträge gefunden werden

#### 2. initTimeOffset()
```cpp
void initTimeOffset()
```
- Wird während der Initialisierung aufgerufen
- Berechnet den Zeitoffset basierend auf dem letzten Zeitstempel
- Setzt den nächsten Zeitstempel auf: `letzter_timestamp + log_interval`
- Bei neuen Dateien startet er bei 60 Sekunden

### Geänderte Komponenten

#### 1. Globale Variablen
```cpp
unsigned long timeOffset = 0;  // Offset für kontinuierliche Zeitstempel
```

#### 2. setup() Funktion
- Aufruf von `initTimeOffset()` nach CSV-Header-Erstellung
- Funktioniert nur wenn SD-Karte verfügbar ist

#### 3. logTemperatures() Funktion
```cpp
unsigned long timestamp = (millis() / 1000) + timeOffset;
```
- Berechnet kontinuierliche Zeitstempel durch Addition des Offsets

## Verhalten

### Beim ersten Start (neue CSV-Datei)
1. CSV-Header wird erstellt
2. `getLastTimestamp()` findet keine Einträge → gibt 0 zurück
3. `timeOffset` wird auf 60 gesetzt
4. Erste Zeitstempel beginnen bei 60 Sekunden

### Bei Neustart (existierende CSV-Datei)
1. CSV-Datei existiert bereits
2. `getLastTimestamp()` findet den höchsten Zeitstempel (z.B. 1200s)
3. `timeOffset` wird auf 1260s gesetzt (1200 + 60s Intervall)
4. Nächste Zeitstempel beginnen bei 1260 Sekunden

### Beispiel-Zeitstempel-Folge
```
Erste Session: 60, 120, 180, 240 [Neustart]
Zweite Session: 300, 360, 420, 480 [Neustart]  
Dritte Session: 540, 600, 660, 720 ...
```

## Debug-Ausgaben
```
Initializing time offset from last CSV entry...OK
Last timestamp found in CSV: 1200 seconds
Next timestamp will be: 1260 seconds (continuing from 1200)
```

## Vorteile
- ✅ Kontinuierliche Zeitachse über Neustarts hinweg
- ✅ Automatische Erkennung der letzten Zeit
- ✅ Kompatibel mit existierenden CSV-Dateien
- ✅ Fallback für neue Installationen
- ✅ Einfache Analyse der Temperaturdaten über lange Zeiträume

## Kompilierung
```bash
pio run                    # Kompilieren
pio run --target upload    # Upload (ESP32 muss verbunden sein)
```

Der Code kompiliert erfolgreich und verwendet:
- RAM: 6.0% (19,560 Bytes)
- Flash: 28.4% (372,245 Bytes)