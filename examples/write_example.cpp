/*
 * NFC Tag Write Example - Course Task 2
 * 
 * This example demonstrates how to write data to NFC cards using the NFCReader class.
 * This is the second task in the NFC course: writing data to NFC tags.
 * 
 * What you'll learn:
 * - How to write data to different types of NFC cards
 * - How to write text strings to tags
 * - How to write binary data
 * - How to verify written data
 * - Differences between Mifare Classic and NTAG/Ultralight writing
 * 
 * Supported Cards:
 * - Mifare Classic 1K/4K (block-based, requires authentication)
 * - Mifare Ultralight (page-based, 4 bytes per page)
 * - NTAG213/215/216 (page-based, 4 bytes per page)
 * 
 * IMPORTANT NOTES:
 * - Block 0 (manufacturer block) is READ-ONLY and CANNOT be written to
 * - For Mifare Classic, NEVER write to sector trailers (blocks 3, 7, 11, etc.) - will lock sector
 * - For NTAG/Ultralight, avoid lock bytes and OTP areas
 * - Always use safe user data areas: Block 4+ for Mifare, Page 4+ for NTAG
 * - Default authentication key for Mifare Classic: FF FF FF FF FF FF
 */

#include <Arduino.h>
#include "NFCReader.h"

// ========== CONFIGURATION ==========
// Choose communication mode: I2C or SPI
#define USE_SPI  // Comment this out and uncomment USE_I2C to use I2C mode
// #define USE_I2C

// Choose reading mode: POLLING or IRQ
#define USE_IRQ_MODE  // Comment out for polling mode

// ========== CREATE NFC READER INSTANCE ==========
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

// ========== HELPER FUNCTIONS ==========

/**
 * Print card type as human-readable string
 */
void printCardType(NFCCardType cardType) {
  Serial.print(F("Card Type: "));
  switch (cardType) {
    case NFCCardType::MIFARE_CLASSIC_1K:
      Serial.println(F("Mifare Classic 1K"));
      break;
    case NFCCardType::MIFARE_CLASSIC_4K:
      Serial.println(F("Mifare Classic 4K"));
      break;
    case NFCCardType::MIFARE_ULTRALIGHT:
      Serial.println(F("Mifare Ultralight"));
      break;
    case NFCCardType::NTAG:
      Serial.println(F("NTAG (213/215/216)"));
      break;
    default:
      Serial.println(F("Unknown"));
      break;
  }
}

/**
 * Print write result with appropriate formatting
 */
void printWriteResult(const NFCWriteResult& result, const __FlashStringHelper* operation) {
  Serial.print(F("  "));
  Serial.print(operation);
  Serial.print(F(": "));
  
  if (result.success) {
    Serial.print(F("✓ SUCCESS"));
    if (result.verified) {
      Serial.println(F(" (Verified)"));
    } else {
      Serial.println();
    }
  } else {
    Serial.print(F("✗ FAILED - "));
    Serial.println(result.errorMessage);
  }
}

// ========== MAIN PROGRAM ==========

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for serial port to connect
  
  // Print header
  Serial.println(F("═══════════════════════════════════════════"));
  Serial.println(F("    NFC TAG WRITE EXAMPLE - TASK 2         "));
  Serial.println(F("═══════════════════════════════════════════"));
  Serial.println();
  
  // Display configuration
  Serial.println(F("Configuration:"));
  #ifdef USE_I2C
    Serial.println(F("  Communication: I2C"));
  #else
    Serial.println(F("  Communication: SPI"));
  #endif
  
  #ifdef USE_IRQ_MODE
    Serial.println(F("  Read Mode: IRQ (Interrupt-based)"));
  #else
    Serial.println(F("  Read Mode: POLLING"));
  #endif
  Serial.println();
  
  // Initialize NFC reader
  Serial.println(F("Initializing NFC reader..."));
  if (!nfcReader.begin()) {
    Serial.println(F("❌ Failed to initialize NFC reader!"));
    Serial.println(F("Please check:"));
    Serial.println(F("  - PN532 module connections"));
    Serial.println(F("  - Power supply"));
    Serial.println(F("  - Communication mode setting"));
    while (1) {
      delay(1000);  // Halt execution
    }
  }
  
  Serial.println(F("✓ NFC reader initialized successfully!"));
  Serial.println();
  Serial.println(F("═══════════════════════════════════════════"));
  Serial.println(F("Ready! Place a card near the reader..."));
  Serial.println(F("═══════════════════════════════════════════"));
  Serial.println();
}

void loop() {
  // Read card first to detect it
  NFCCardInfo cardInfo = nfcReader.readCard();
  
  if (cardInfo.detected) {
    Serial.println(F("┌──────────────────────────────────────────┐"));
    Serial.println(F("│      CARD DETECTED - WRITING DATA       │"));
    Serial.println(F("├──────────────────────────────────────────┤"));
    Serial.print(F("│ "));
    printCardType(cardInfo.cardType);
    Serial.println(F("└──────────────────────────────────────────┘"));
    Serial.println();
    
    NFCWriteResult result;
    
    // ========== EXAMPLE 1: Simple String Write (Auto-detect card type) ==========
    Serial.println(F("Example 1: Writing string using auto-detect"));
    String message = "Hello NFC!";
    result = nfcReader.writeString(message, 4, true);  // Start at safe address, verify
    printWriteResult(result, F("String write"));
    Serial.println();
    
    // ========== EXAMPLE 2: Card Type-Specific Writing ==========
    if (cardInfo.cardType == NFCCardType::MIFARE_ULTRALIGHT || 
        cardInfo.cardType == NFCCardType::NTAG) {
      
      Serial.println(F("Example 2: NTAG/Ultralight specific operations"));
      Serial.println(F("  Writing to page 4 (safe user area)..."));
      
      // Write 4 bytes of data to page 4
      uint8_t pageData[] = {0x01, 0x02, 0x03, 0x04};
      result = nfcReader.writeNTAG(4, pageData, 4, true);
      printWriteResult(result, F("Page 4 write"));
      
      // Write a longer string across multiple pages
      String longText = "NFC Course Task 2: Write";
      result = nfcReader.writeNTAGString(5, longText, true);
      printWriteResult(result, F("Multi-page string"));
      
    } else if (cardInfo.cardType == NFCCardType::MIFARE_CLASSIC_1K ||
               cardInfo.cardType == NFCCardType::MIFARE_CLASSIC_4K) {
      
      Serial.println(F("Example 2: Mifare Classic specific operations"));
      Serial.println(F("  Using default key: FF FF FF FF FF FF"));
      Serial.println(F("  Writing to block 4 (Sector 1, safe area)..."));
      
      // Write 16 bytes to block 4 (first user block in Sector 1)
      uint8_t blockData[16];
      for (int i = 0; i < 16; i++) {
        blockData[i] = i + 1;  // Data: 01 02 03 ... 10
      }
      result = nfcReader.writeMifareClassic(4, blockData, 16, DEFAULT_KEY, false, true);
      printWriteResult(result, F("Block 4 write"));
      
      // Write a string to block 5
      String text = "Mifare Test Data";
      result = nfcReader.writeMifareClassicString(5, text, DEFAULT_KEY, false, true);
      printWriteResult(result, F("Block 5 string"));
      
    } else {
      Serial.println(F("⚠️  Unknown card type - skipping type-specific examples"));
    }
    
    Serial.println();
    
    // ========== EXAMPLE 3: Writing Binary Data ==========
    Serial.println(F("Example 3: Writing custom binary data"));
    uint8_t binaryData[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
    
    if (cardInfo.cardType == NFCCardType::NTAG || 
        cardInfo.cardType == NFCCardType::MIFARE_ULTRALIGHT) {
      // For NTAG: Write to page 8
      result = nfcReader.writeNTAG(8, binaryData, 4, true);  // Only 4 bytes per page
      printWriteResult(result, F("Binary data (page 8)"));
    } else if (cardInfo.cardType == NFCCardType::MIFARE_CLASSIC_1K) {
      // For Mifare Classic: Write to block 6
      result = nfcReader.writeMifareClassic(6, binaryData, 8, DEFAULT_KEY, false, true);
      printWriteResult(result, F("Binary data (block 6)"));
    }
    
    Serial.println();
    Serial.println(F("═══════════════════════════════════════════"));
    Serial.println(F("Write operations complete!"));
    Serial.println(F("═══════════════════════════════════════════"));
    Serial.println();
    
    // Wait before allowing next write (prevents accidental repeated writes)
    Serial.println(F("Waiting 3 seconds..."));
    delay(3000);
    
    // Reset card state to allow re-detection
    nfcReader.resetCardState();
    
    Serial.println(F("Ready for next card..."));
    Serial.println();
  }
  
  // In polling mode, add a small delay to avoid excessive checking
  #ifndef USE_IRQ_MODE
    delay(100);
  #endif
}
