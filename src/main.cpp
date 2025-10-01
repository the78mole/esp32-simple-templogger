#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include "sd_manager.h"
#include "web_server.h"

// Pin Definitions for FreeNove ESP32-S3 WROOM Clone
#define ONE_WIRE_BUS 21       // GPIO pin for OneWire bus

// Configuration defaults
#define DEFAULT_LOG_INTERVAL 15000  // 15 seconds in milliseconds

// OneWire and DallasTemperature instances
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Global variables
unsigned long logInterval = DEFAULT_LOG_INTERVAL;
unsigned long lastLogTime = 0;
int numberOfDevices = 0;
DeviceAddress deviceAddresses[10];  // Support up to 10 sensors
unsigned long timeOffset = 0;        // Offset to continue from last timestamp

// WiFi and WebServer variables
String wifiSSID = "";
String wifiPassword = "";
int webServerPort = 80;
bool wifiEnabled = false;
String mdnsHostname = "templogger";

// Function prototypes
void initSensors();
void logTemperatures();
void logToSerial(unsigned long timestamp);
String getDeviceAddressString(DeviceAddress deviceAddress);

void setup() {
  // FIRST: Disable WiFi immediately to prevent automatic connections
  WiFi.mode(WIFI_OFF);
  WiFi.disconnect(true);
  
  // Initialize Serial for traditional UART (not USB CDC)
  Serial.begin(115200);
  delay(2000);
  
  Serial.println();
  Serial.println("===============================================");
  Serial.println("===     ESP32 Temperature Logger START     ===");
  Serial.println("===============================================");
  Serial.println("Board: FreeNove ESP32-S3 WROOM Clone");
  Serial.println("DEBUG: Serial communication working!");
  Serial.println("Starting initialization...");
  Serial.println();
  
  // Initialize SD card using SD Manager
  sdManager.initializeSD();
  
  // Read configuration using SD Manager
  if (sdManager.isAvailable() && !sdManager.readConfig(logInterval, wifiSSID, wifiPassword, webServerPort, wifiEnabled, mdnsHostname)) {
    Serial.println("Using default configuration");
  } else if (!sdManager.isAvailable()) {
    Serial.println("SD card not available - using default configuration");
  }
  
  // Initialize temperature sensors
  initSensors();
  
  // Create CSV file with header if it doesn't exist (only if SD card is available)
  if (sdManager.isAvailable()) {
    sdManager.createCSVHeader(numberOfDevices);
    // Initialize time offset to continue from last logged timestamp
    sdManager.initTimeOffset(timeOffset, logInterval);
  } else {
    Serial.println("SD card not available - will log to Serial only");
  }
  
  Serial.println("Initialization complete!");
  Serial.printf("Logging interval: %lu seconds\n", logInterval / 1000);
  Serial.printf("Number of sensors: %d\n", numberOfDevices);
  Serial.printf("SD card available: %s\n", sdManager.isAvailable() ? "YES" : "NO");
  
  // Initialize WiFi and WebServer if enabled
  if (wifiEnabled && wifiSSID.length() > 0 && wifiPassword.length() > 0) {
    Serial.println("WiFi configuration found - enabling WiFi...");
    if (webServer.initWiFi(wifiSSID, wifiPassword, mdnsHostname)) {
      Serial.println("WiFi connected successfully - starting WebServer...");
      if (webServer.initWebServer(webServerPort)) {
        Serial.printf("WebServer started successfully on port %d\n", webServerPort);
        // Set sensor data for web server
        webServer.setSensorData(&sensors, numberOfDevices, deviceAddresses, timeOffset);
        Serial.printf("Access the web interface at: http://%s.local or http://%s\n", 
                     mdnsHostname.c_str(), WiFi.localIP().toString().c_str());
      } else {
        Serial.println("Failed to start WebServer");
      }
    } else {
      Serial.println("WiFi connection failed - WebServer not started");
    }
  } else {
    // Keep WiFi disabled (already disabled at startup)
    Serial.println("WiFi remains disabled - configure WiFi credentials in config.toml to enable web interface");
  }
  
  Serial.println("===========================================\n");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Handle web server requests
  if (webServer.isServerRunning() && webServer.isWiFiConnected()) {
    webServer.handleClient();
  }
  
  // Check if it's time to log
  if (currentTime - lastLogTime >= logInterval) {
    lastLogTime = currentTime;
    logTemperatures();
  }
  
  delay(100);  // Small delay to prevent tight looping
}

void initSensors() {
  Serial.println("\n=== Temperature Sensors Initialization ===");
  
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  
  Serial.printf("Found %d device(s) on OneWire bus (GPIO %d)\n", numberOfDevices, ONE_WIRE_BUS);
  
  if (numberOfDevices == 0) {
    Serial.println("\n*** WARNING: No temperature sensors found! ***");
    Serial.println("Please check:");
    Serial.println("- DS18B20 sensors are connected");
    Serial.println("- 4.7k pull-up resistor is installed between DATA and 3.3V");
    Serial.println("- OneWire bus pin is correct (GPIO " + String(ONE_WIRE_BUS) + ")");
    Serial.println("- Sensor power connections (VCC to 3.3V, GND to GND)");
    Serial.println("- Cable connections and lengths (try shorter cables)");
    Serial.println("\nSystem will continue but no temperature data will be available\n");
    return;
  }
  
  // Store device addresses and configure sensors
  for (int i = 0; i < numberOfDevices && i < 10; i++) {
    if (sensors.getAddress(deviceAddresses[i], i)) {
      Serial.printf("Sensor %d address: %s", i, getDeviceAddressString(deviceAddresses[i]).c_str());
      
      // Set to 12-bit resolution for highest accuracy
      sensors.setResolution(deviceAddresses[i], 12);
      
      // Test sensor communication
      sensors.requestTemperaturesByAddress(deviceAddresses[i]);
      delay(750); // Wait for conversion (12-bit takes ~750ms)
      float tempC = sensors.getTempC(deviceAddresses[i]);
      
      if (tempC == DEVICE_DISCONNECTED_C) {
        Serial.println(" - ERROR: Cannot read temperature!");
      } else {
        Serial.printf(" - Current temp: %.2f°C\n", tempC);
      }
    }
  }
  
  Serial.println("Temperature sensors initialization complete\n");
}

void logTemperatures() {
  Serial.println("--- Logging temperatures ---");
  
  // Request temperatures from all sensors
  sensors.requestTemperatures();
  
  // Get timestamp (continuous across reboots)
  unsigned long timestamp = (millis() / 1000) + timeOffset;
  
  // Always log to Serial
  logToSerial(timestamp);
  
  // Prepare data for SD card logging
  String addresses[10];
  float temperatures[10];
  
  for (int i = 0; i < numberOfDevices; i++) {
    addresses[i] = getDeviceAddressString(deviceAddresses[i]);
    temperatures[i] = sensors.getTempC(deviceAddresses[i]);
  }
  
  // Log to SD card if available
  if (sdManager.isAvailable()) {
    if (sdManager.logTemperatureData(timestamp, millis(), numberOfDevices, addresses, temperatures)) {
      Serial.println("Data logged to SD card successfully");
    } else {
      Serial.println("Failed to log data to SD card");
    }
  } else {
    Serial.println("Data logged to Serial only (SD card not available)");
  }
  
  // Update sensor data for web server
  if (webServer.isServerRunning()) {
    webServer.setSensorData(&sensors, numberOfDevices, deviceAddresses, timeOffset);
  }
}

void logToSerial(unsigned long timestamp) {
  Serial.printf("Time: %lu s | ", timestamp);
  
  // Read and display each sensor
  for (int i = 0; i < numberOfDevices; i++) {
    float tempC = sensors.getTempC(deviceAddresses[i]);
    
    if (tempC == DEVICE_DISCONNECTED_C) {
      Serial.printf("Sensor%d: ERROR ", i);
    } else {
      Serial.printf("Sensor%d: %.2f°C ", i, tempC);
    }
  }
  
  Serial.println();
}

String getDeviceAddressString(DeviceAddress deviceAddress) {
  String addr = "";
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) addr += "0";
    addr += String(deviceAddress[i], HEX);
  }
  addr.toUpperCase();
  return addr;
}