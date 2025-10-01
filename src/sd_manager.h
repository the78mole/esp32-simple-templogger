#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>

// Pin Definitions for FreeNove ESP32-S3 WROOM Clone SDMMC interface
#define SD_CLK_PIN 39         // SD card clock pin
#define SD_CMD_PIN 38         // SD card command pin  
#define SD_DATA_PIN 40        // SD card data pin (D0)

// File paths
#define CONFIG_FILE "/config.toml"
#define LOG_FILE "/templog.csv"

class SDManager {
public:
    SDManager();
    
    // Initialization and status
    bool initializeSD();
    bool isAvailable() const;
    
    // Configuration management
    bool readConfig(unsigned long& logInterval, String& wifiSSID, String& wifiPassword, int& webServerPort, bool& wifiEnabled, String& mdnsHostname);
    bool createDefaultConfig(unsigned long defaultLogInterval);
    
    // CSV logging
    void createCSVHeader(int numberOfDevices);
    bool logTemperatureData(unsigned long timestamp, unsigned long millis, int numberOfDevices, 
                           String* addresses, float* temperatures);
    
    // Time management
    unsigned long getLastTimestamp();
    void initTimeOffset(unsigned long& timeOffset, unsigned long logInterval);
    
    // File operations
    File openLogFile(const char* mode = "r");
    bool fileExists(const char* filename);
    
private:
    bool sdCardAvailable;
    
    // Helper methods
    bool tryInitSDMMC(bool mode1bit, int frequency);
    void printSDCardInfo();
    String parseConfigValue(const String& content, const String& key, const String& defaultValue = "");
    int parseConfigInt(const String& content, const String& key, int defaultValue = 0);
};

// Global instance
extern SDManager sdManager;

#endif // SD_MANAGER_H