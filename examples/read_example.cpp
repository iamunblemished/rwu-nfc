/*
 * NFC Tag Read Example - Course Task 1
 * 
 * This example demonstrates basic NFC tag reading functionality using the NFCReader class.
 * This is the first task in the NFC course: reading tag UIDs and card information.
 * 
 * What you'll learn:
 * - How to initialize the PN532 NFC reader
 * - How to detect when a card is present
 * - How to read card UID (Unique Identifier)
 * - How to identify different card types
 * - How to display card information
 * 
 * Supported Cards:
 * - Mifare Classic 1K/4K (4-byte UID)
 * - Mifare Ultralight (7-byte UID)
 * - NTAG213/215/216 (7-byte UID)
 */

#include <Arduino.h>
#include "NFCReader.h"

// ========== CONFIGURATION ==========
// Choose communication mode: I2C or SPI
#define USE_SPI  // Comment this out and uncomment USE_I2C to use I2C mode
// #define USE_I2C

// Choose reading mode: POLLING or IRQ
// POLLING: Actively checks for cards at regular intervals
// IRQ: Uses interrupt for faster, more efficient detection
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
  Serial.print("Card Type: ");
  switch (cardType) {
    case NFCCardType::MIFARE_CLASSIC_1K:
      Serial.println("Mifare Classic 1K");
      break;
    case NFCCardType::MIFARE_CLASSIC_4K:
      Serial.println("Mifare Classic 4K");
      break;
    case NFCCardType::MIFARE_ULTRALIGHT:
      Serial.println("Mifare Ultralight");
      break;
    case NFCCardType::NTAG:
      Serial.println("NTAG (213/215/216)");
      break;
    default:
      Serial.println("Unknown");
      break;
  }
}

/**
 * Print UID in hexadecimal format
 */
void printUID(const uint8_t* uid, uint8_t length) {
  Serial.print("UID: ");
  for (uint8_t i = 0; i < length; i++) {
    if (uid[i] < 0x10) Serial.print("0");
    Serial.print(uid[i], HEX);
    if (i < length - 1) Serial.print(" ");
  }
  Serial.println();
}

/**
 * Print complete card information
 */
void printCardInfo(const NFCCardInfo& card) {
  Serial.println("┌─────────────────────────────────────┐");
  Serial.println("│       CARD DETECTED                 │");
  Serial.println("├─────────────────────────────────────┤");
  
  // Card type
  Serial.print("│ ");
  printCardType(card.cardType);
  
  // Physical UID
  Serial.print("│ Physical ");
  printUID(card.uid, card.uidLength);
  Serial.print("│ UID Length: ");
  Serial.print(card.uidLength);
  Serial.println(" bytes");
  
  // Card ID (for 4-byte UIDs)
  if (card.uidLength == 4) {
    Serial.print("│ Card ID (Decimal): ");
    Serial.println(card.cardID);
  }
  
  // Cloned UID info (if present)
  if (card.hasClonedUID) {
    Serial.println("│ ");
    Serial.println("│ ⚠️  CLONED UID DETECTED");
    Serial.print("│ Cloned ");
    printUID(card.clonedUID, card.clonedUIDLength);
    Serial.println("│ ");
    Serial.print("│ Effective ");
    printUID(card.getEffectiveUID(), card.getEffectiveUIDLength());
  }
  
  Serial.println("└─────────────────────────────────────┘");
  Serial.println();
}

// ========== MAIN PROGRAM ==========

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for serial port to connect (needed for some boards)
  
  // Print header
  Serial.println("═══════════════════════════════════════════");
  Serial.println("     NFC TAG READ EXAMPLE - TASK 1         ");
  Serial.println("═══════════════════════════════════════════");
  Serial.println();
  
  // Display configuration
  Serial.println("Configuration:");
  #ifdef USE_I2C
    Serial.println("  Communication: I2C");
  #else
    Serial.println("  Communication: SPI");
  #endif
  
  #ifdef USE_IRQ_MODE
    Serial.println("  Read Mode: IRQ (Interrupt-based)");
  #else
    Serial.println("  Read Mode: POLLING");
  #endif
  Serial.println();
  
  // Initialize NFC reader
  Serial.println("Initializing NFC reader...");
  if (!nfcReader.begin()) {
    Serial.println("❌ Failed to initialize NFC reader!");
    Serial.println("Please check:");
    Serial.println("  - PN532 module connections");
    Serial.println("  - Power supply");
    Serial.println("  - Communication mode setting");
    while (1) {
      delay(1000);  // Halt execution
    }
  }
  
  Serial.println("✓ NFC reader initialized successfully!");
  Serial.println();
  Serial.println("═══════════════════════════════════════════");
  Serial.println("Ready! Place a card near the reader...");
  Serial.println("═══════════════════════════════════════════");
  Serial.println();
}

void loop() {
  // Read card information
  NFCCardInfo cardInfo = nfcReader.readCard();
  
  // Check if a card was detected
  if (cardInfo.detected) {
    // Card found! Print all information
    printCardInfo(cardInfo);
    
    // Example: You can use the UID for access control or logging
    // For example, check if this is a specific card:
    // if (cardInfo.uidLength == 4 && 
    //     cardInfo.uid[0] == 0xAB && 
    //     cardInfo.uid[1] == 0xCD && 
    //     cardInfo.uid[2] == 0xEF && 
    //     cardInfo.uid[3] == 0x12) {
    //   Serial.println("This is my special card!");
    // }
    
    // Wait a bit before reading again to avoid repeated detections
    delay(2000);
    
    // Reset card state to allow detection of the same card again
    nfcReader.resetCardState();
    
    Serial.println("Ready for next card...");
    Serial.println();
  }
  
  // In polling mode, add a small delay to avoid excessive checking
  #ifndef USE_IRQ_MODE
    delay(100);
  #endif
}
