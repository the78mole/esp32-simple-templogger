#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD_MMC.h>
#include <FS.h>
#include <ArduinoJson.h>

// Pin Definitions for FreeNove ESP32-S3 WROOM Clone
#define ONE_WIRE_BUS 4        // GPIO pin for OneWire bus
// SD Card pins for FreeNove ESP32-S3 WROOM (SDMMC interface)
#define SD_CLK_PIN 39         // SD card clock pin
#define SD_CMD_PIN 38         // SD card command pin  
#define SD_DATA_PIN 40        // SD card data pin (D0)

// Configuration defaults
#define DEFAULT_LOG_INTERVAL 15000  // 15 seconds in milliseconds
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
bool sdCardAvailable = false;        // Track SD card status
unsigned long timeOffset = 0;        // Offset to continue from last timestamp

// Function prototypes
void initSD();
bool tryInitSDMMC(bool mode1bit, int frequency);
void initSensors();
bool readConfig();
bool createDefaultConfig();
void logTemperatures();
void logToSerial(unsigned long timestamp);
String getDeviceAddressString(DeviceAddress deviceAddress);
void createCSVHeader();
unsigned long getLastTimestamp();
void initTimeOffset();

void setup() {
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
  
  // Initialize SD card
  initSD();
  
  // Read configuration (only if SD card is available)
  if (sdCardAvailable && !readConfig()) {
    Serial.println("Using default configuration");
  } else if (!sdCardAvailable) {
    Serial.println("SD card not available - using default configuration");
  }
  
  // Initialize temperature sensors
  initSensors();
  
  // Create CSV file with header if it doesn't exist (only if SD card is available)
  if (sdCardAvailable) {
    createCSVHeader();
    // Initialize time offset to continue from last logged timestamp
    initTimeOffset();
  } else {
    Serial.println("SD card not available - will log to Serial only");
  }
  
  Serial.println("Initialization complete!");
  Serial.printf("Logging interval: %lu seconds\n", logInterval / 1000);
  Serial.printf("Number of sensors: %d\n", numberOfDevices);
  Serial.printf("SD card available: %s\n", sdCardAvailable ? "YES" : "NO");
  Serial.println("===========================================\n");
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
  Serial.println();
  Serial.println("=== SD Card Initialization (SDMMC) ===");
  Serial.printf("Using SDMMC pins - CLK:%d, CMD:%d, DATA:%d\n", SD_CLK_PIN, SD_CMD_PIN, SD_DATA_PIN);
  Serial.println();
  
  // Try different SDMMC configurations
  bool success = false;
  
  // First try 1-bit mode with different frequencies
  int frequencies[] = {20000, 10000, 5000}; // 20MHz, 10MHz, 5MHz
  
  Serial.println("Trying 1-bit SDMMC mode...");
  for (int i = 0; i < 3 && !success; i++) {
    Serial.printf("\nAttempt %d: 1-bit SDMMC mode with %d kHz...\n", i+1, frequencies[i]);
    if (tryInitSDMMC(true, frequencies[i])) {
      success = true;
      sdCardAvailable = true;
      Serial.printf("*** SUCCESS! SD card initialized in 1-bit mode at %d kHz ***\n", frequencies[i]);
    } else {
      Serial.printf("Failed with %d kHz\n", frequencies[i]);
    }
    delay(1000);
  }
  
  // If 1-bit failed, try 4-bit mode (some cards prefer this)
  if (!success) {
    for (int i = 0; i < 3 && !success; i++) {
      Serial.printf("Trying 4-bit SDMMC mode with %d kHz...\n", frequencies[i]);
      if (tryInitSDMMC(false, frequencies[i])) {
        success = true;
        sdCardAvailable = true;
        Serial.printf("SUCCESS! SD card initialized in 4-bit mode at %d kHz\n", frequencies[i]);
      }
      delay(500);
    }
  }
  
  if (success) {
    // Display card info
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD card size: %llu MB\n", cardSize);
    Serial.printf("Card type: ");
    switch (SD_MMC.cardType()) {
      case CARD_MMC: Serial.println("MMC"); break;
      case CARD_SD: Serial.println("SDSC"); break;
      case CARD_SDHC: Serial.println("SDHC"); break;
      default: Serial.println("UNKNOWN"); break;
    }
  } else {
    Serial.println("\n*** SD card initialization failed on all attempts ***");
    Serial.println("Please check for FreeNove ESP32-S3 WROOM:");
    Serial.println("- SD card is inserted and properly seated");
    Serial.println("- SD card is formatted (FAT32 recommended)");
    Serial.println("- SDMMC wiring: CLK=39, CMD=38, DATA=40");
    Serial.println("- SD card is compatible with SDMMC interface");
    Serial.println("- SD card is not corrupted");
    Serial.println("- Try a different SD card");
    Serial.println("\nContinuing without SD card - data will be logged to Serial only\n");
  }
}

bool tryInitSDMMC(bool mode1bit, int frequency) {
  // End any existing SD session
  SD_MMC.end();
  delay(200);
  
  // Configure SDMMC pins explicitly for FreeNove board
  Serial.printf("Setting SDMMC pins: CLK=%d, CMD=%d, DATA0=%d\n", SD_CLK_PIN, SD_CMD_PIN, SD_DATA_PIN);
  SD_MMC.setPins(SD_CLK_PIN, SD_CMD_PIN, SD_DATA_PIN);
  
  // Try to initialize SDMMC with specific parameters
  // mode1bit: true = 1-bit mode, false = 4-bit mode
  // For 1-bit mode, we only need CLK, CMD, and DATA0
  Serial.printf("Attempting SDMMC.begin with mode1bit=%s, freq=%d\n", mode1bit ? "true" : "false", frequency);
  bool result = SD_MMC.begin("/sdcard", mode1bit, false, frequency);
  Serial.printf("SDMMC.begin result: %s\n", result ? "SUCCESS" : "FAILED");
  return result;
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

bool readConfig() {
  if (!sdCardAvailable) {
    Serial.println("SD card not available - cannot read config file");
    return false;
  }
  
  Serial.print("Reading configuration from SD card...");
  
  File configFile = SD_MMC.open(CONFIG_FILE);
  if (!configFile) {
    Serial.println("NOT FOUND");
    Serial.println("Creating default configuration file...");
    
    if (createDefaultConfig()) {
      Serial.println("Default config.toml created successfully");
      Serial.print("Re-reading configuration from SD card...");
      
      // Try to read the newly created config file
      configFile = SD_MMC.open(CONFIG_FILE);
      if (!configFile) {
        Serial.println("FAILED to read newly created config file");
        return false;
      }
    } else {
      Serial.println("Failed to create default config file");
      return false;
    }
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

bool createDefaultConfig() {
  if (!sdCardAvailable) {
    Serial.println("SD card not available - cannot create config file");
    return false;
  }
  
  Serial.print("Creating default config.toml...");
  
  File configFile = SD_MMC.open(CONFIG_FILE, FILE_WRITE);
  if (!configFile) {
    Serial.println("FAILED to create config file!");
    return false;
  }
  
  // Write default configuration
  configFile.println("# ESP32 Temperature Logger Configuration");
  configFile.println("# This file is automatically created with default values");
  configFile.println("# You can modify these values and restart the device");
  configFile.println();
  configFile.println("# Logging interval in seconds");
  configFile.println("# Default: 15 seconds");
  configFile.println("# Minimum recommended: 10 seconds");
  configFile.println("# Maximum recommended: 3600 seconds (1 hour)");
  configFile.printf("log_interval_seconds = %lu\n", DEFAULT_LOG_INTERVAL / 1000);
  configFile.println();
  configFile.println("# Temperature sensor settings");
  configFile.println("# sensor_resolution = 12  # 9, 10, 11, or 12 bits");
  configFile.println();
  configFile.println("# Debug mode (future use)");
  configFile.println("# debug_mode = false");
  configFile.println();
  configFile.println("# End of configuration");
  
  configFile.close();
  Serial.println("OK");
  
  return true;
}

void createCSVHeader() {
  if (!sdCardAvailable) {
    Serial.println("SD card not available - cannot create CSV file");
    return;
  }
  
  // Check if file already exists
  if (SD_MMC.exists(LOG_FILE)) {
    Serial.println("Log file already exists, appending to it");
    return;
  }
  
  Serial.print("Creating CSV log file...");
  
  File logFile = SD_MMC.open(LOG_FILE, FILE_WRITE);
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
  
  // Get timestamp (continuous across reboots)
  unsigned long timestamp = (millis() / 1000) + timeOffset;
  
  // Always log to Serial
  logToSerial(timestamp);
  
  // Log to SD card if available
  if (sdCardAvailable) {
    File logFile = SD_MMC.open(LOG_FILE, FILE_APPEND);
    if (!logFile) {
      Serial.println("ERROR: Failed to open log file!");
      Serial.println("SD card may have been removed or corrupted");
      sdCardAvailable = false;  // Mark SD card as unavailable
      return;
    }
    
    // Write timestamp
    logFile.printf("%lu,%lu", timestamp, millis());
    
    // Read and log each sensor
    for (int i = 0; i < numberOfDevices; i++) {
      float tempC = sensors.getTempC(deviceAddresses[i]);
      String addr = getDeviceAddressString(deviceAddresses[i]);
      
      logFile.printf(",%s,%.2f", addr.c_str(), tempC);
    }
    
    logFile.println();
    logFile.close();
    
    Serial.println("Data logged to SD card successfully");
  } else {
    Serial.println("Data logged to Serial only (SD card not available)");
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

unsigned long getLastTimestamp() {
  if (!sdCardAvailable) {
    return 0;
  }
  
  File logFile = SD_MMC.open(LOG_FILE);
  if (!logFile) {
    Serial.println("No existing log file found - starting from timestamp 0");
    return 0;
  }
  
  unsigned long lastTimestamp = 0;
  String line;
  
  // Read file line by line to find the last timestamp
  while (logFile.available()) {
    line = logFile.readStringUntil('\n');
    line.trim();
    
    // Skip header line and empty lines
    if (line.length() > 0 && !line.startsWith("Timestamp")) {
      int commaPos = line.indexOf(',');
      if (commaPos > 0) {
        String timestampStr = line.substring(0, commaPos);
        unsigned long timestamp = timestampStr.toInt();
        if (timestamp > lastTimestamp) {
          lastTimestamp = timestamp;
        }
      }
    }
  }
  
  logFile.close();
  
  Serial.printf("Last timestamp found in CSV: %lu seconds\n", lastTimestamp);
  return lastTimestamp;
}

void initTimeOffset() {
  Serial.print("Initializing time offset from last CSV entry...");
  
  unsigned long lastTimestamp = getLastTimestamp();
  
  if (lastTimestamp > 0) {
    // Continue from the last timestamp plus one log interval
    timeOffset = lastTimestamp + (logInterval / 1000);
    Serial.printf("OK\nNext timestamp will be: %lu seconds (continuing from %lu)\n", 
                  timeOffset, lastTimestamp);
  } else {
    // Start fresh
    timeOffset = 60; // Start at 60 seconds as before, but only for new files
    Serial.printf("OK\nStarting fresh with timestamp: %lu seconds\n", timeOffset);
  }
}
