#include "sd_manager.h"

// Global instance
SDManager sdManager;

SDManager::SDManager() : sdCardAvailable(false) {
}

bool SDManager::initializeSD() {
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
        printSDCardInfo();
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
        sdCardAvailable = false;
    }
    
    return success;
}

bool SDManager::tryInitSDMMC(bool mode1bit, int frequency) {
    // End any existing SD session
    SD_MMC.end();
    delay(200);
    
    // Configure SDMMC pins explicitly for FreeNove board
    Serial.printf("Setting SDMMC pins: CLK=%d, CMD=%d, DATA0=%d\n", SD_CLK_PIN, SD_CMD_PIN, SD_DATA_PIN);
    SD_MMC.setPins(SD_CLK_PIN, SD_CMD_PIN, SD_DATA_PIN);
    
    // Try to initialize SDMMC with specific parameters
    Serial.printf("Attempting SDMMC.begin with mode1bit=%s, freq=%d\n", mode1bit ? "true" : "false", frequency);
    bool result = SD_MMC.begin("/sdcard", mode1bit, false, frequency);
    Serial.printf("SDMMC.begin result: %s\n", result ? "SUCCESS" : "FAILED");
    return result;
}

void SDManager::printSDCardInfo() {
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD card size: %llu MB\n", cardSize);
    Serial.printf("Card type: ");
    switch (SD_MMC.cardType()) {
        case CARD_MMC: Serial.println("MMC"); break;
        case CARD_SD: Serial.println("SDSC"); break;
        case CARD_SDHC: Serial.println("SDHC"); break;
        default: Serial.println("UNKNOWN"); break;
    }
}

bool SDManager::isAvailable() const {
    return sdCardAvailable;
}

bool SDManager::readConfig(unsigned long& logInterval, String& wifiSSID, String& wifiPassword, int& webServerPort, bool& wifiEnabled, String& mdnsHostname) {
    if (!sdCardAvailable) {
        Serial.println("SD card not available - cannot read config file");
        return false;
    }
    
    Serial.print("Reading configuration from SD card...");
    
    File configFile = SD_MMC.open(CONFIG_FILE);
    if (!configFile) {
        Serial.println("NOT FOUND");
        Serial.println("Creating default configuration file...");
        
        if (createDefaultConfig(logInterval)) {
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
    
    // Parse configuration
    int seconds = parseConfigInt(content, "log_interval_seconds", logInterval / 1000);
    if (seconds > 0) {
        logInterval = seconds * 1000;
        Serial.printf("Log interval set to: %d seconds\n", seconds);
    }
    
    // Parse WiFi settings
    String ssid = parseConfigValue(content, "wifi_ssid", "YOUR_WIFI_SSID");
    if (ssid.length() > 0 && ssid != "YOUR_WIFI_SSID") {
        wifiSSID = ssid;
        wifiEnabled = true;
        Serial.printf("WiFi SSID: %s\n", wifiSSID.c_str());
    }
    
    String password = parseConfigValue(content, "wifi_password", "YOUR_WIFI_PASSWORD");
    if (password.length() > 0 && password != "YOUR_WIFI_PASSWORD") {
        wifiPassword = password;
        Serial.println("WiFi password: ***");
    }
    
    int port = parseConfigInt(content, "webserver_port", 80);
    if (port > 0 && port < 65536) {
        webServerPort = port;
        Serial.printf("WebServer port: %d\n", webServerPort);
    }
    
    // Parse mDNS hostname
    String hostname = parseConfigValue(content, "mdns_hostname", "templogger");
    if (hostname.length() > 0 && hostname != "YOUR_HOSTNAME") {
        mdnsHostname = hostname;
        Serial.printf("mDNS hostname: %s.local\n", mdnsHostname.c_str());
    } else {
        mdnsHostname = "templogger"; // Default fallback
    }
    
    return true;
}

String SDManager::parseConfigValue(const String& content, const String& key, const String& defaultValue) {
    int startPos = content.indexOf(key);
    if (startPos >= 0) {
        int equalPos = content.indexOf('=', startPos);
        if (equalPos >= 0) {
            int endPos = content.indexOf('\n', equalPos);
            if (endPos < 0) endPos = content.length();
            
            String valueStr = content.substring(equalPos + 1, endPos);
            valueStr.trim();
            valueStr.replace("\"", ""); // Remove quotes
            return valueStr;
        }
    }
    return defaultValue;
}

int SDManager::parseConfigInt(const String& content, const String& key, int defaultValue) {
    String valueStr = parseConfigValue(content, key, String(defaultValue));
    int value = valueStr.toInt();
    return (value > 0) ? value : defaultValue;
}

bool SDManager::createDefaultConfig(unsigned long defaultLogInterval) {
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
    configFile.printf("log_interval_seconds = %lu\n", defaultLogInterval / 1000);
    configFile.println();
    configFile.println("# WiFi Configuration");
    configFile.println("# Set your WiFi credentials to enable web interface");
    configFile.println("# Leave as defaults to disable WiFi");
    configFile.println("wifi_ssid = \"YOUR_WIFI_SSID\"");
    configFile.println("wifi_password = \"YOUR_WIFI_PASSWORD\"");
    configFile.println();
    configFile.println("# Web Server Configuration");
    configFile.println("# Port for the web interface (default: 80)");
    configFile.println("webserver_port = 80");
    configFile.println();
    configFile.println("# mDNS Configuration");
    configFile.println("# Hostname for mDNS discovery (device will be available as <hostname>.local)");
    configFile.println("# Default: templogger (accessible as templogger.local)");
    configFile.println("mdns_hostname = \"templogger\"");
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

void SDManager::createCSVHeader(int numberOfDevices) {
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

bool SDManager::logTemperatureData(unsigned long timestamp, unsigned long milliseconds, int numberOfDevices, 
                                  String* addresses, float* temperatures) {
    if (!sdCardAvailable) {
        return false;
    }
    
    File logFile = SD_MMC.open(LOG_FILE, FILE_APPEND);
    if (!logFile) {
        Serial.println("ERROR: Failed to open log file!");
        Serial.println("SD card may have been removed or corrupted");
        sdCardAvailable = false;  // Mark SD card as unavailable
        return false;
    }
    
    // Write timestamp
    logFile.printf("%lu,%lu", timestamp, milliseconds);
    
    // Write sensor data
    for (int i = 0; i < numberOfDevices; i++) {
        logFile.printf(",%s,%.2f", addresses[i].c_str(), temperatures[i]);
    }
    
    logFile.println();
    logFile.close();
    
    return true;
}

unsigned long SDManager::getLastTimestamp() {
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

void SDManager::initTimeOffset(unsigned long& timeOffset, unsigned long logInterval) {
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

File SDManager::openLogFile(const char* mode) {
    if (!sdCardAvailable) {
        return File();
    }
    
    if (strcmp(mode, "r") == 0) {
        return SD_MMC.open(LOG_FILE);
    } else {
        return SD_MMC.open(LOG_FILE, FILE_APPEND);
    }
}

bool SDManager::fileExists(const char* filename) {
    if (!sdCardAvailable) {
        return false;
    }
    return SD_MMC.exists(filename);
}