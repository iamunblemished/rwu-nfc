#ifndef NFC_READER_H
#define NFC_READER_H

#include <Arduino.h>
#include <Adafruit_PN532.h>

// Communication mode enum
enum class NFCCommMode {
  I2C,
  SPI
};

// Reading mode enum
enum class NFCReadMode {
  POLLING,
  IRQ
};

// Card type enum
enum class NFCCardType {
  UNKNOWN,
  MIFARE_CLASSIC_1K,
  MIFARE_CLASSIC_4K,
  MIFARE_ULTRALIGHT,
  NTAG
};

// Custom sector configuration for card data storage
// We use Sector 1 (blocks 4-7) to store cloned UID and metadata
#define CUSTOM_SECTOR 1
#define CUSTOM_BLOCK_UID 4      // Block 4: Cloned UID (7 bytes) + flags
#define CUSTOM_BLOCK_DATA 5     // Block 5: Additional data (timestamp, etc)
#define CUSTOM_BLOCK_RESERVED 6 // Block 6: Reserved for future use
// Block 7 is the sector trailer (contains keys)

// Magic bytes to identify initialized cards
#define CARD_MAGIC_BYTE1 0xAC  // Access Control
#define CARD_MAGIC_BYTE2 0xDB  // DataBase

// Card information structure
struct NFCCardInfo {
  bool detected;
  uint8_t uid[7];          // Physical UID from manufacturer block
  uint8_t uidLength;
  NFCCardType cardType;
  uint32_t cardID;         // For 4-byte UIDs
  
  // Custom sector data
  bool hasClonedUID;       // True if card has been initialized with cloned data
  uint8_t clonedUID[7];    // UID stored in our custom sector
  uint8_t clonedUIDLength;
  
  // For access control, use clonedUID if hasClonedUID=true, otherwise use uid
  const uint8_t* getEffectiveUID() const { return hasClonedUID ? clonedUID : uid; }
  uint8_t getEffectiveUIDLength() const { return hasClonedUID ? clonedUIDLength : uidLength; }
};

// Write operation result
struct NFCWriteResult {
  bool success;
  bool verified;  // If verification was performed
  String errorMessage;
};

// Default Mifare Classic authentication key
const uint8_t DEFAULT_KEY[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

class NFCReader {
public:
  // Constructor
  NFCReader(NFCCommMode commMode, NFCReadMode readMode = NFCReadMode::POLLING);
  
  // Destructor
  ~NFCReader();
  
  // Initialization
  bool begin();
  
  // Card detection and reading
  NFCCardInfo readCard();
  bool isCardPresent();
  
  // Writing methods
  // Write to NTAG/Ultralight (page-based, 4 bytes per page)
  NFCWriteResult writeNTAG(uint8_t page, const uint8_t* data, uint8_t dataLength, bool verify = true);
  NFCWriteResult writeNTAGString(uint8_t startPage, const String& text, bool verify = true);
  
  // Write to Mifare Classic (block-based, 16 bytes per block)
  NFCWriteResult writeMifareClassic(uint8_t block, const uint8_t* data, uint8_t dataLength, 
                                     const uint8_t* key = DEFAULT_KEY, bool useKeyB = false, bool verify = true);
  NFCWriteResult writeMifareClassicString(uint8_t startBlock, const String& text, 
                                           const uint8_t* key = DEFAULT_KEY, bool useKeyB = false, bool verify = true);
  
  // Generic write that auto-detects card type
  NFCWriteResult writeData(const uint8_t* data, uint8_t dataLength, uint8_t startAddress = 0, bool verify = true);
  NFCWriteResult writeString(const String& text, uint8_t startAddress = 0, bool verify = true);
  
  // Read methods for verification
  bool readNTAGPage(uint8_t page, uint8_t* buffer);
  bool readMifareClassicBlock(uint8_t block, uint8_t* buffer, const uint8_t* key = DEFAULT_KEY, bool useKeyB = false);
  
  // Custom sector operations for card cloning
  bool readCustomSector(NFCCardInfo& info);  // Read our custom sector data
  bool writeClonedUID(const uint8_t* sourceUID, uint8_t sourceUIDLength);  // Clone UID to custom sector
  bool isCardInitialized();  // Check if card has our custom sector data
  bool initializeCard();  // Initialize blank card with empty custom sector
  
  // Get firmware version
  uint32_t getFirmwareVersion();
  
  // Get current modes
  NFCCommMode getCommMode() const { return _commMode; }
  NFCReadMode getReadMode() const { return _readMode; }
  
  // IRQ handling
  void handleIRQ();
  bool hasIRQEvent();
  void clearIRQEvent();
  
  // Card state management
  void resetCardState(); // Call after processing card to allow new detection
  bool wasCardRemoved(); // Check if card was removed
  
  // Configuration
  void setIRQPin(uint8_t pin) { _irqPin = pin; }
  void setResetPin(uint8_t pin) { _resetPin = pin; }
  void setSPIPins(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss);
  
private:
  NFCCommMode _commMode;
  NFCReadMode _readMode;
  Adafruit_PN532* _nfc;
  
  // Pin configurations
  uint8_t _irqPin;
  uint8_t _resetPin;
  uint8_t _spiSCK;
  uint8_t _spiMISO;
  uint8_t _spiMOSI;
  uint8_t _spiSS;
  
  // IRQ state
  volatile bool _cardPresent;
  unsigned long _lastIRQTime;
  
  // Polling throttle
  unsigned long _lastPollTime;
  static const unsigned long POLL_INTERVAL = 100; // ms between polls
  
  // Card presence tracking
  unsigned long _lastCardDetectedTime;
  bool _lastCardPresent;
  static const unsigned long CARD_TIMEOUT = 1000; // ms before considering card removed
  
  // Helper methods
  NFCCardType determineCardType(uint8_t uidLength);
  uint32_t calculateCardID(uint8_t* uid, uint8_t uidLength);
  void printCardInfo(const NFCCardInfo& info);
  bool authenticateMifareBlock(uint8_t block, const uint8_t* key, bool useKeyB, const uint8_t* uid, uint8_t uidLength);
  bool verifyWrite(const uint8_t* expected, const uint8_t* actual, uint8_t length);
  
  // Store last card info for write operations
  NFCCardInfo _lastCardInfo;
};

#endif // NFC_READER_H
