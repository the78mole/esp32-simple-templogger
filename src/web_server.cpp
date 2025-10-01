#include "web_server.h"
#include "sd_manager.h"

// Global instance
TemperatureWebServer webServer;

TemperatureWebServer::TemperatureWebServer() 
    : server(nullptr), wifiConnected(false), serverRunning(false), serverPort(80), mdnsHostname("templogger"),
      temperatureSensors(nullptr), numberOfDevices(0), deviceAddresses(nullptr), currentTimeOffset(0) {
}

TemperatureWebServer::~TemperatureWebServer() {
    stopServer();
}

bool TemperatureWebServer::initWiFi(const String& ssid, const String& password, const String& mdnsHostname) {
    this->mdnsHostname = mdnsHostname;
    
    Serial.println("\n=== WiFi Initialization ===");
    Serial.printf("Connecting to WiFi SSID: %s\n", ssid.c_str());
    Serial.printf("Password length: %d characters\n", password.length());
    
    // Simple WiFi setup - just disconnect any existing connection
    WiFi.disconnect();
    delay(500);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    // Set WiFi configuration for better compatibility
    //WiFi.setAutoReconnect(true);
    WiFi.persistent(false);  // Don't save to flash to avoid old credentials
    WiFi.setSleep(false);    // Keep WiFi active for web access
    
    // Begin WiFi connection
    Serial.println("Starting WiFi connection...");
    Serial.printf("SSID: %s\n", ssid.c_str());
    Serial.printf("Pass: %s\n", password.c_str());

    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Simple wait and check approach
    Serial.println("Waiting for WiFi connection...");
    for (int i = 0; i < 30; i++) {  // Wait up to 30 seconds
        delay(1000);
        Serial.printf("Attempt %d: ", i + 1);
        
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println("SUCCESS!");
            Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
            Serial.printf("MAC address: %s\n", WiFi.macAddress().c_str());
            
            // Initialize mDNS
            initMDNS();
            
            return true;
        } else {
            Serial.printf("Status = %d\n", WiFi.status());
        }
    }
    
    // Connection failed
    wifiConnected = false;
    Serial.println("\n*** WiFi connection timeout! ***");
    Serial.printf("Final WiFi status: %d\n", WiFi.status());
    Serial.println("Please check your WiFi credentials and network availability");
    
    return false;
}

bool TemperatureWebServer::initMDNS() {
    if (MDNS.begin(mdnsHostname.c_str())) {
        Serial.printf("mDNS responder started: %s.local\n", mdnsHostname.c_str());
        return true;
    } else {
        Serial.printf("Failed to start mDNS responder for %s.local\n", mdnsHostname.c_str());
        return false;
    }
}

bool TemperatureWebServer::initWebServer(int port) {
    if (!wifiConnected) {
        Serial.println("WiFi not connected - cannot start web server");
        return false;
    }
    
    serverPort = port;
    Serial.printf("\n=== WebServer Initialization (Port %d) ===\n", serverPort);
    
    // Create server instance with configured port
    if (server) {
        delete server;
    }
    server = new WebServer(serverPort);
    
    // Configure routes
    setupRoutes();
    
    // Start server
    server->begin();
    serverRunning = true;
    
    Serial.printf("WebServer started successfully!\n");
    Serial.printf("Access web interface at: http://%s:%d\n", WiFi.localIP().toString().c_str(), serverPort);
    Serial.printf("Or use: http://esp32-templogger.local:%d\n", serverPort);
    
    return true;
}

void TemperatureWebServer::setupRoutes() {
    if (!server) return;
    
    server->on("/", [this]() { handleRoot(); });
    server->on("/data", [this]() { handleCurrentData(); });
    server->on("/download", [this]() { handleCSVDownload(); });
    server->onNotFound([this]() { handleNotFound(); });
}

void TemperatureWebServer::handleClient() {
    if (server && serverRunning && wifiConnected) {
        server->handleClient();
    }
}

void TemperatureWebServer::stopServer() {
    if (server) {
        server->stop();
        delete server;
        server = nullptr;
    }
    serverRunning = false;
}

bool TemperatureWebServer::isWiFiConnected() const {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

String TemperatureWebServer::getIPAddress() const {
    if (wifiConnected) {
        return WiFi.localIP().toString();
    }
    return "";
}

int TemperatureWebServer::getWiFiRSSI() const {
    if (wifiConnected) {
        return WiFi.RSSI();
    }
    return 0;
}

bool TemperatureWebServer::isServerRunning() const {
    return serverRunning;
}

int TemperatureWebServer::getServerPort() const {
    return serverPort;
}

void TemperatureWebServer::setSensorData(DallasTemperature* sensors, int deviceCount, DeviceAddress* addresses, unsigned long timeOffset) {
    temperatureSensors = sensors;
    numberOfDevices = deviceCount;
    deviceAddresses = addresses;
    currentTimeOffset = timeOffset;
}

void TemperatureWebServer::handleRoot() {
    if (!server) return;
    
    String html = generateHTML();
    server->send(200, "text/html", html);
}

void TemperatureWebServer::handleCurrentData() {
    if (!server || !temperatureSensors) return;
    
    // Request fresh temperature readings
    temperatureSensors->requestTemperatures();
    delay(750); // Wait for conversion
    
    // Create JSON response
    JsonDocument doc;
    doc["timestamp"] = (millis() / 1000) + currentTimeOffset;
    doc["uptime_ms"] = millis();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["sensor_count"] = numberOfDevices;
    
    JsonArray sensorsArray = doc["sensors"].to<JsonArray>();
    
    for (int i = 0; i < numberOfDevices; i++) {
        float tempC = temperatureSensors->getTempC(deviceAddresses[i]);
        String addr = getDeviceAddressString(deviceAddresses[i]);
        
        JsonObject sensor = sensorsArray.add<JsonObject>();
        sensor["id"] = i;
        sensor["address"] = addr;
        if (tempC == DEVICE_DISCONNECTED_C) {
            sensor["temperature_c"] = nullptr;
        } else {
            sensor["temperature_c"] = tempC;
        }
        sensor["status"] = (tempC == DEVICE_DISCONNECTED_C) ? "error" : "ok";
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    server->send(200, "application/json", jsonString);
}

void TemperatureWebServer::handleCSVDownload() {
    if (!server) return;
    
    if (!sdManager.isAvailable()) {
        server->send(404, "text/plain", "SD card not available");
        return;
    }
    
    File logFile = sdManager.openLogFile("r");
    if (!logFile) {
        server->send(404, "text/plain", "Log file not found");
        return;
    }
    
    server->sendHeader("Content-Disposition", "attachment; filename=templog.csv");
    server->streamFile(logFile, "text/csv");
    logFile.close();
}

void TemperatureWebServer::handleNotFound() {
    if (!server) return;
    
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server->uri();
    message += "\nMethod: ";
    message += (server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server->args();
    message += "\n";
    
    for (uint8_t i = 0; i < server->args(); i++) {
        message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
    }
    
    server->send(404, "text/plain", message);
}

String TemperatureWebServer::getDeviceAddressString(const DeviceAddress& deviceAddress) {
    String addr = "";
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) addr += "0";
        addr += String(deviceAddress[i], HEX);
    }
    addr.toUpperCase();
    return addr;
}

String TemperatureWebServer::generateHTML() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>ESP32 Temperature Logger</title>";
    html += "<meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }";
    html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
    html += ".header { text-align: center; color: #333; border-bottom: 2px solid #007acc; padding-bottom: 10px; }";
    html += ".status { background: #e7f3ff; padding: 15px; border-radius: 5px; margin: 20px 0; }";
    html += ".sensors { margin: 20px 0; }";
    html += ".sensor { background: #f9f9f9; padding: 10px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #007acc; }";
    html += ".temperature { font-size: 1.2em; font-weight: bold; color: #007acc; }";
    html += ".error { color: #cc0000; }";
    html += ".buttons { text-align: center; margin: 30px 0; }";
    html += ".button { background: #007acc; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; margin: 0 10px; display: inline-block; }";
    html += ".button:hover { background: #005c99; }";
    html += ".refresh { font-size: 0.9em; color: #666; margin-top: 20px; text-align: center; }";
    html += "</style>";
    html += "<script>";
    html += "function refreshData() {";
    html += "  fetch('/data').then(response => response.json()).then(data => {";
    html += "    document.getElementById('timestamp').innerText = new Date(data.timestamp * 1000).toLocaleString();";
    html += "    document.getElementById('uptime').innerText = Math.floor(data.uptime_ms / 1000) + ' seconds';";
    html += "    document.getElementById('rssi').innerText = data.wifi_rssi + ' dBm';";
    html += "    const sensorsDiv = document.getElementById('sensors');";
    html += "    sensorsDiv.innerHTML = '';";
    html += "    data.sensors.forEach(sensor => {";
    html += "      const div = document.createElement('div');";
    html += "      div.className = 'sensor';";
    html += "      if (sensor.status === 'ok') {";
    html += "        div.innerHTML = `<strong>Sensor ${sensor.id}</strong><br>Address: ${sensor.address}<br><span class='temperature'>${sensor.temperature_c.toFixed(2)}°C</span>`;";
    html += "      } else {";
    html += "        div.innerHTML = `<strong>Sensor ${sensor.id}</strong><br>Address: ${sensor.address}<br><span class='error'>Error reading sensor</span>`;";
    html += "      }";
    html += "      sensorsDiv.appendChild(div);";
    html += "    });";
    html += "  }).catch(err => console.error('Error:', err));";
    html += "}";
    html += "setInterval(refreshData, 5000);"; // Auto refresh every 5 seconds
    html += "</script>";
    html += "</head><body onload='refreshData()'>";
    
    html += "<div class='container'>";
    html += "<h1 class='header'>🌡️ ESP32 Temperature Logger</h1>";
    
    html += "<div class='status'>";
    html += "<h3>System Status</h3>";
    html += "<p><strong>Current Time:</strong> <span id='timestamp'>Loading...</span></p>";
    html += "<p><strong>Uptime:</strong> <span id='uptime'>Loading...</span></p>";
    html += "<p><strong>WiFi Signal:</strong> <span id='rssi'>Loading...</span></p>";
    html += "<p><strong>Sensors Found:</strong> " + String(numberOfDevices) + "</p>";
    html += "<p><strong>SD Card:</strong> " + String(sdManager.isAvailable() ? "Available" : "Not Available") + "</p>";
    html += "</div>";
    
    html += "<div class='sensors'>";
    html += "<h3>Current Temperatures</h3>";
    html += "<div id='sensors'>Loading sensor data...</div>";
    html += "</div>";
    
    html += "<div class='buttons'>";
    html += "<a href='/data' class='button'>📊 JSON Data</a>";
    if (sdManager.isAvailable()) {
        html += "<a href='/download' class='button'>📥 Download CSV</a>";
    }
    html += "<a href='javascript:refreshData()' class='button'>🔄 Refresh</a>";
    html += "</div>";
    
    html += "<div class='refresh'>";
    html += "Data refreshes automatically every 5 seconds";
    html += "</div>";
    
    html += "</div>";
    html += "</body></html>";
    
    return html;
}