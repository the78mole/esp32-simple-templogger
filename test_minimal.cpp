#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== MINIMAL TEST ===");
  Serial.println("Board is working!");
}

void loop() {
  static unsigned long lastPrint = 0;
  
  if (millis() - lastPrint >= 5000) {
    lastPrint = millis();
    Serial.println("Heartbeat: " + String(millis()));
  }
  
  delay(100);
}