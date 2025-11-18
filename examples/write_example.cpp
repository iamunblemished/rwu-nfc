/*
 * NFC Write Example
 * 
 * This example demonstrates how to write data to NFC cards using the NFCReader class.
 * Supports both Mifare Classic (with authentication) and NTAG/Ultralight cards.
 */

#include <Arduino.h>
#include "NFCReader.h"

// Configuration
#define USE_SPI  // or USE_I2C
// #define USE_IRQ_MODE  // Comment out for polling mode

// Create NFC Reader instance
#ifdef USE_I2C
  #ifdef USE_IRQ_MODE
    NFCReader nfcReader(NFCCommMode::I2C, NFCReadMode::IRQ);
  #else
    NFCReader nfcReader(NFCCommMode::I2C, NFCReadMode::POLLING);
  #endif
#elif defined(USE_SPI)
  #ifdef USE_IRQ_MODE
    NFCReader nfcReader(NFCCommMode::SPI, NFCReadMode::IRQ);
  #else
    NFCReader nfcReader(NFCCommMode::SPI, NFCReadMode::POLLING);
  #endif
#else
  #error "Please define either USE_I2C or USE_SPI"
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("===================");
  Serial.println("NFC Write Example");
  Serial.println("===================");
  
  if (!nfcReader.begin()) {
    Serial.println("Failed to initialize NFC reader!");
    while (1);
  }
  
  Serial.println("\nPlace a card on the reader to write data...\n");
}

void loop() {
  // Read card first to detect it
  NFCCardInfo cardInfo = nfcReader.readCard();
  
  if (cardInfo.detected) {
    Serial.println("\n--- Writing to card ---");
    
    // Example 1: Write a simple string (auto-detects card type)
    String message = "Hello NFC!";
    NFCWriteResult result = nfcReader.writeString(message);
    
    if (result.success) {
      Serial.println("✓ Write successful!");
      if (result.verified) {
        Serial.println("✓ Verification passed!");
      }
    } else {
      Serial.print("✗ Write failed: ");
      Serial.println(result.errorMessage);
    }
    
    // Example 2: Write specific card types
    if (cardInfo.cardType == NFCCardType::MIFARE_ULTRALIGHT || 
        cardInfo.cardType == NFCCardType::NTAG) {
      // Write to NTAG page 4 (safe user area)
      uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
      result = nfcReader.writeNTAG(4, data, 4, true);
      
      if (result.success) {
        Serial.println("✓ NTAG page 4 written");
      }
    } 
    else if (cardInfo.cardType == NFCCardType::MIFARE_CLASSIC_1K) {
      // Write to Mifare Classic block 4 (sector 1, first data block)
      // Using default key 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      String text = "Mifare data test";
      result = nfcReader.writeMifareClassicString(4, text);
      
      if (result.success) {
        Serial.println("✓ Mifare Classic block 4 written");
      } else {
        Serial.print("✗ Mifare write failed: ");
        Serial.println(result.errorMessage);
      }
    }
    
    Serial.println("--- Write complete ---\n");
    
    // Wait before allowing next write
    delay(3000);
  }
  
  #ifndef USE_IRQ_MODE
    delay(100);
  #endif
}
