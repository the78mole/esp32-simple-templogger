#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>

// Forward declarations
typedef uint8_t DeviceAddress[8];

class TemperatureWebServer {
public:
    TemperatureWebServer();
    ~TemperatureWebServer();
    
    // WiFi management
    bool initWiFi(const String& ssid, const String& password, const String& mdnsHostname = "templogger");
    bool isWiFiConnected() const;
    String getIPAddress() const;
    int getWiFiRSSI() const;
    String getMDNSHostname() const;
    
    // WebServer management
    bool initWebServer(int port);
    void handleClient();
    void stopServer();
    
    // Set sensor data for web interface
    void setSensorData(DallasTemperature* sensors, int deviceCount, DeviceAddress* addresses, unsigned long timeOffset);
    
    // Server status
    bool isServerRunning() const;
    int getServerPort() const;
    
private:
    WebServer* server;
    bool wifiConnected;
    bool serverRunning;
    int serverPort;
    String mdnsHostname;
    
    // Sensor data references
    DallasTemperature* temperatureSensors;
    int numberOfDevices;
    DeviceAddress* deviceAddresses;
    unsigned long currentTimeOffset;
    
    // Route handlers
    void handleRoot();
    void handleCurrentData();
    void handleCSVDownload();
    void handleNotFound();
    
    // Helper methods
    String generateHTML();
    String getDeviceAddressString(const DeviceAddress& deviceAddress);
    void setupRoutes();
    bool initMDNS();
};

// Global instance
extern TemperatureWebServer webServer;

#endif // WEB_SERVER_H