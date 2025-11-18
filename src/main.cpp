#include <Arduino.h>
#include "Config.h"
#include "NFCReader.h"
#include "AccessControlSystem.h"

NFCReader nfcReader(NFC_COMM_SPI, NFC_READ_IRQ);
AccessControlSystem accessControl(nfcReader);

void setup() {
  Serial.begin(115200);
  
  // Give serial time to initialize (non-blocking approach)
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime < 100)) {
    // Wait briefly for serial on boards that need it
  }
  
  Serial.println(F("\n=== Access Control System ==="));
  Serial.println(F("Initializing...\n"));
  
  if (!accessControl.begin()) {
    Serial.println(F("FATAL: System initialization failed!"));
    // Non-blocking error state - continue running but display error
    while(1) {
      accessControl.update(); // Still update display to show error
    }
  }
  
  Serial.println(F("System ready."));
  Serial.println(F("Scan a card or press SELECT for menu.\n"));
}

void loop() {
  accessControl.update();
}
