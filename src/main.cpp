#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <ArduinoJson.h>

// Pin Definitions
#define ONE_WIRE_BUS 4        // GPIO pin for OneWire bus
#define SD_CS_PIN 10          // SD card chip select pin (adjust for your board)

// Configuration defaults
#define DEFAULT_LOG_INTERVAL 60000  // 60 seconds in milliseconds
#define CONFIG_FILE "/config.toml"
#define LOG_FILE "/templog.csv"

// OneWire and DallasTemperature instances
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Global variables
unsigned long logInterval = DEFAULT_LOG_INTERVAL;
unsigned long lastLogTime = 0;
int numberOfDevices = 0;
DeviceAddress deviceAddresses[10];  // Support up to 10 sensors

// Function prototypes
void initSD();
void initSensors();
bool readConfig();
void logTemperatures();
String getDeviceAddressString(DeviceAddress deviceAddress);
void createCSVHeader();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32 Temperature Logger ===");
  Serial.println("Starting initialization...");
  
  // Initialize SD card
  initSD();
  
  // Read configuration
  if (!readConfig()) {
    Serial.println("Using default configuration");
  }
  
  // Initialize temperature sensors
  initSensors();
  
  // Create CSV file with header if it doesn't exist
  createCSVHeader();
  
  Serial.println("Initialization complete!");
  Serial.printf("Logging interval: %lu seconds\n", logInterval / 1000);
  Serial.printf("Number of sensors: %d\n", numberOfDevices);
  Serial.println();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check if it's time to log
  if (currentTime - lastLogTime >= logInterval) {
    lastLogTime = currentTime;
    logTemperatures();
  }
  
  delay(100);  // Small delay to prevent tight looping
}

void initSD() {
  Serial.print("Initializing SD card...");
  
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("FAILED!");
    Serial.println("SD card initialization failed. Please check:");
    Serial.println("- SD card is inserted");
    Serial.println("- SD card is formatted (FAT32)");
    Serial.println("- Wiring is correct");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("OK");
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD card size: %llu MB\n", cardSize);
}

void initSensors() {
  Serial.print("Initializing temperature sensors...");
  
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  
  Serial.printf("Found %d device(s)\n", numberOfDevices);
  
  if (numberOfDevices == 0) {
    Serial.println("WARNING: No temperature sensors found!");
    Serial.println("Please check:");
    Serial.println("- DS18B20 sensors are connected");
    Serial.println("- 4.7k pull-up resistor is installed");
    Serial.println("- OneWire bus pin is correct (GPIO " + String(ONE_WIRE_BUS) + ")");
  }
  
  // Store device addresses
  for (int i = 0; i < numberOfDevices && i < 10; i++) {
    if (sensors.getAddress(deviceAddresses[i], i)) {
      Serial.printf("Sensor %d address: %s\n", i, getDeviceAddressString(deviceAddresses[i]).c_str());
      sensors.setResolution(deviceAddresses[i], 12);  // Set to 12-bit resolution
    }
  }
}

bool readConfig() {
  Serial.print("Reading configuration from SD card...");
  
  File configFile = SD.open(CONFIG_FILE);
  if (!configFile) {
    Serial.println("NOT FOUND");
    return false;
  }
  
  String content = "";
  while (configFile.available()) {
    content += (char)configFile.read();
  }
  configFile.close();
  
  Serial.println("OK");
  
  // Simple TOML parsing (looking for log_interval_seconds)
  int startPos = content.indexOf("log_interval_seconds");
  if (startPos >= 0) {
    int equalPos = content.indexOf('=', startPos);
    if (equalPos >= 0) {
      int endPos = content.indexOf('\n', equalPos);
      if (endPos < 0) endPos = content.length();
      
      String valueStr = content.substring(equalPos + 1, endPos);
      valueStr.trim();
      
      int seconds = valueStr.toInt();
      if (seconds > 0) {
        logInterval = seconds * 1000;
        Serial.printf("Log interval set to: %d seconds\n", seconds);
      }
    }
  }
  
  return true;
}

void createCSVHeader() {
  // Check if file already exists
  if (SD.exists(LOG_FILE)) {
    Serial.println("Log file already exists, appending to it");
    return;
  }
  
  Serial.print("Creating CSV log file...");
  
  File logFile = SD.open(LOG_FILE, FILE_WRITE);
  if (!logFile) {
    Serial.println("FAILED to create log file!");
    return;
  }
  
  // Write CSV header
  logFile.print("Timestamp,Millis");
  for (int i = 0; i < numberOfDevices; i++) {
    logFile.printf(",Sensor%d_Addr,Sensor%d_TempC", i, i);
  }
  logFile.println();
  
  logFile.close();
  Serial.println("OK");
}

void logTemperatures() {
  Serial.println("--- Logging temperatures ---");
  
  // Request temperatures from all sensors
  sensors.requestTemperatures();
  
  // Open log file
  File logFile = SD.open(LOG_FILE, FILE_APPEND);
  if (!logFile) {
    Serial.println("ERROR: Failed to open log file!");
    return;
  }
  
  // Get timestamp
  unsigned long timestamp = millis() / 1000;  // Convert to seconds
  
  // Write timestamp
  logFile.printf("%lu,%lu", timestamp, millis());
  Serial.printf("Time: %lu s, ", timestamp);
  
  // Read and log each sensor
  for (int i = 0; i < numberOfDevices; i++) {
    float tempC = sensors.getTempC(deviceAddresses[i]);
    String addr = getDeviceAddressString(deviceAddresses[i]);
    
    logFile.printf(",%s,%.2f", addr.c_str(), tempC);
    Serial.printf("Sensor%d: %.2f°C ", i, tempC);
  }
  
  logFile.println();
  logFile.close();
  
  Serial.println();
  Serial.println("Data logged successfully");
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
