#include "NFCReader.h"
#include <Wire.h>
#include <SPI.h>

// Static instance pointer for IRQ callback
static NFCReader* _irqInstance = nullptr;

// Static IRQ handler
static void staticIRQHandler() {
  if (_irqInstance) {
    _irqInstance->handleIRQ();
  }
}

NFCReader::NFCReader(NFCCommMode commMode, NFCReadMode readMode)
  : _commMode(commMode),
    _readMode(readMode),
    _nfc(nullptr),
    _irqPin(2),
    _resetPin(3),
    _spiSCK(13),
    _spiMISO(12),
    _spiMOSI(11),
    _spiSS(10),
    _cardPresent(false),
    _lastIRQTime(0),
    _lastPollTime(0),
    _lastCardDetectedTime(0),
    _lastCardPresent(false)
{
  _irqInstance = this;
  // Initialize last card info
  _lastCardInfo.detected = false;
  _lastCardInfo.hasClonedUID = false;
  _lastCardInfo.clonedUIDLength = 0;
}

NFCReader::~NFCReader() {
  if (_nfc) {
    delete _nfc;
  }
  _irqInstance = nullptr;
}

void NFCReader::setSPIPins(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss) {
  _spiSCK = sck;
  _spiMISO = miso;
  _spiMOSI = mosi;
  _spiSS = ss;
}

bool NFCReader::begin() {
  // Create PN532 instance based on communication mode
  if (_commMode == NFCCommMode::I2C) {
    _nfc = new Adafruit_PN532(_irqPin, _resetPin);
    Serial.print(F("PN532 I2C mode"));
  } else {
    _nfc = new Adafruit_PN532(_spiSCK, _spiMISO, _spiMOSI, _spiSS);
    Serial.print(F("PN532 SPI mode"));
  }
  
  if (_readMode == NFCReadMode::IRQ) {
    Serial.println(F(" + IRQ"));
  } else {
    Serial.println(F(" + Poll"));
  }
  
  // Initialize the PN532
  _nfc->begin();
  
  // Check firmware version
  uint32_t versiondata = _nfc->getFirmwareVersion();
  if (!versiondata) {
    Serial.println(F("PN532 NOT FOUND!"));
    return false;
  }
  
  Serial.print(F("PN5"));
  Serial.print((versiondata >> 24) & 0xFF, HEX);
  Serial.print(F(" v"));
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print(F("."));
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  
  // Configure SAM (Security Access Module)
  _nfc->SAMConfig();
  
  // Setup IRQ mode if enabled
  if (_readMode == NFCReadMode::IRQ) {
    pinMode(_irqPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_irqPin), staticIRQHandler, FALLING);
    _nfc->startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
    Serial.println(F("Ready (IRQ)"));
  } else {
    Serial.println(F("Ready (Poll)"));
  }
  
  return true;
}

uint32_t NFCReader::getFirmwareVersion() {
  if (_nfc) {
    return _nfc->getFirmwareVersion();
  }
  return 0;
}

void NFCReader::handleIRQ() {
  unsigned long now = millis();
  // Debounce: ignore interrupts within 500ms of last one
  if (now - _lastIRQTime > 500) {
    _cardPresent = true;
    _lastIRQTime = now;
  }
}

bool NFCReader::hasIRQEvent() {
  return _cardPresent;
}

void NFCReader::clearIRQEvent() {
  _cardPresent = false;
}

void NFCReader::resetCardState() {
  _lastCardPresent = false;
  _lastCardDetectedTime = 0;
  // Restart detection in IRQ mode
  if (_readMode == NFCReadMode::IRQ && _nfc) {
    _nfc->startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
  }
}

bool NFCReader::wasCardRemoved() {
  if (!_lastCardPresent) {
    return true; // No card was present
  }
  unsigned long now = millis();
  return (now - _lastCardDetectedTime > CARD_TIMEOUT);
}

NFCCardType NFCReader::determineCardType(uint8_t uidLength) {
  if (uidLength == 4) {
    // Could be Classic 1K or 4K - default to 1K
    return NFCCardType::MIFARE_CLASSIC_1K;
  } else if (uidLength == 7) {
    // Could be Ultralight or NTAG
    return NFCCardType::MIFARE_ULTRALIGHT;
  }
  return NFCCardType::UNKNOWN;
}

uint32_t NFCReader::calculateCardID(uint8_t* uid, uint8_t uidLength) {
  if (uidLength != 4) {
    return 0;
  }
  
  uint32_t cardid = uid[0];
  cardid <<= 8;
  cardid |= uid[1];
  cardid <<= 8;
  cardid |= uid[2];
  cardid <<= 8;
  cardid |= uid[3];
  
  return cardid;
}

void NFCReader::printCardInfo(const NFCCardInfo& info) {
  // Disabled to save RAM on Arduino Nano
  // Uncomment for debugging if needed
  (void)info;  // Suppress unused parameter warning
}

NFCCardInfo NFCReader::readCard() {
  NFCCardInfo info;
  info.detected = false;
  info.uidLength = 0;
  info.cardType = NFCCardType::UNKNOWN;
  info.cardID = 0;
  
  uint8_t success;
  uint8_t uid[7] = {0};
  uint8_t uidLength;
  
  if (_readMode == NFCReadMode::IRQ) {
    // IRQ mode - check if card was detected by interrupt
    if (!_cardPresent) {
      // Check if previous card timed out (removed)
      unsigned long now = millis();
      if (_lastCardPresent && (now - _lastCardDetectedTime > CARD_TIMEOUT)) {
        _lastCardPresent = false;
        // Card removed - restart detection
        _nfc->startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
        Serial.println(F("NFC: Card removed, restarting detection"));
      }
      return info;
    }
    
    // Only process if enough time passed since last read (debounce)
    unsigned long now = millis();
    if (_lastCardPresent && (now - _lastCardDetectedTime < 1000)) {
      // Same card still present, don't read again
      _cardPresent = false; // Clear IRQ flag
      return info;
    }
    
    // Try to read the card
    success = _nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
    
    if (success) {
      // Copy UID
      memcpy(info.uid, uid, uidLength);
      info.uidLength = uidLength;
      info.detected = true;
      info.cardType = determineCardType(uidLength);
      info.cardID = calculateCardID(uid, uidLength);
      
      // Initialize cloned UID fields
      info.hasClonedUID = false;
      info.clonedUIDLength = 0;
      memset(info.clonedUID, 0, 7);
      
      // Store for potential write operations
      _lastCardInfo = info;
      _lastCardDetectedTime = millis();
      _lastCardPresent = true;
      
      // Try to read custom sector data (for cloned UIDs)
      // Only for Mifare Classic cards
      if (info.cardType == NFCCardType::MIFARE_CLASSIC_1K || 
          info.cardType == NFCCardType::MIFARE_CLASSIC_4K) {
        readCustomSector(info);
        _lastCardInfo = info; // Update with cloned UID info
      }
      
      printCardInfo(info);
    }
    
    // Clear the IRQ flag
    _cardPresent = false;
    
    // In IRQ mode, after reading (especially with authentication), we need to 
    // restart passive target detection when the card is removed
    // The timeout mechanism in the beginning of this function handles the restart
    
  } else {
    // Polling mode - actively check for card (throttled)
    unsigned long now = millis();
    if (now - _lastPollTime < POLL_INTERVAL) {
      return info; // Too soon, skip this poll
    }
    _lastPollTime = now;
    
    success = _nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);
    
    if (success) {
      // Check if this is the same card (debounce)
      if (_lastCardPresent && (now - _lastCardDetectedTime < 1000)) {
        // Same card still present, don't report again
        return info;
      }
      
      // Copy UID
      memcpy(info.uid, uid, uidLength);
      info.uidLength = uidLength;
      info.detected = true;
      info.cardType = determineCardType(uidLength);
      info.cardID = calculateCardID(uid, uidLength);
      
      // Initialize cloned UID fields
      info.hasClonedUID = false;
      info.clonedUIDLength = 0;
      memset(info.clonedUID, 0, 7);
      
      // Store for potential write operations
      _lastCardInfo = info;
      _lastCardDetectedTime = millis();
      _lastCardPresent = true;
      
      // Try to read custom sector data (for cloned UIDs)
      // Only for Mifare Classic cards
      if (info.cardType == NFCCardType::MIFARE_CLASSIC_1K || 
          info.cardType == NFCCardType::MIFARE_CLASSIC_4K) {
        readCustomSector(info);
        _lastCardInfo = info; // Update with cloned UID info
      }
      
      printCardInfo(info);
    } else {
      // No card detected - check if card was removed
      if (_lastCardPresent && (now - _lastCardDetectedTime > CARD_TIMEOUT)) {
        _lastCardPresent = false;
        Serial.println(F("NFC: Card removed"));
      }
    }
  }
  
  return info;
}

bool NFCReader::isCardPresent() {
  if (_readMode == NFCReadMode::IRQ) {
    return _cardPresent;
  } else {
    // In polling mode, we'd need to actually check
    // This is a simplified version
    uint8_t uid[7];
    uint8_t uidLength;
    return _nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);
  }
}

// ========== WRITE METHODS ==========

// Helper: Authenticate to Mifare Classic block
bool NFCReader::authenticateMifareBlock(uint8_t block, const uint8_t* key, bool useKeyB, const uint8_t* uid, uint8_t uidLength) {
  uint8_t keyType = useKeyB ? 1 : 0;  // 0 = Key A, 1 = Key B
  // Library expects non-const pointers, but doesn't modify the data
  return _nfc->mifareclassic_AuthenticateBlock((uint8_t*)uid, uidLength, block, keyType, (uint8_t*)key);
}

// Helper: Verify write by comparing data
bool NFCReader::verifyWrite(const uint8_t* expected, const uint8_t* actual, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    if (expected[i] != actual[i]) {
      return false;
    }
  }
  return true;
}

// Read NTAG/Ultralight page (4 bytes)
bool NFCReader::readNTAGPage(uint8_t page, uint8_t* buffer) {
  if (!_nfc) return false;
  
  // NTAG read returns 16 bytes (4 pages), we need first 4 bytes
  uint8_t data[32];
  if (_nfc->mifareultralight_ReadPage(page, data)) {
    memcpy(buffer, data, 4);
    return true;
  }
  return false;
}

// Read Mifare Classic block (16 bytes)
bool NFCReader::readMifareClassicBlock(uint8_t block, uint8_t* buffer, const uint8_t* key, bool useKeyB) {
  if (!_nfc || !_lastCardInfo.detected) return false;
  
  // Authenticate first
  if (!authenticateMifareBlock(block, key, useKeyB, _lastCardInfo.uid, _lastCardInfo.uidLength)) {
    return false;
  }
  
  // Read block
  return _nfc->mifareclassic_ReadDataBlock(block, buffer);
}

// Write to NTAG/Ultralight page
NFCWriteResult NFCReader::writeNTAG(uint8_t page, const uint8_t* data, uint8_t dataLength, bool verify) {
  NFCWriteResult result;
  result.success = false;
  result.verified = false;
  
  if (!_nfc) {
    result.errorMessage = "NFC not initialized";
    return result;
  }
  
  // Allow pages 0-1 write for magic cards (will fail on regular cards)
  // Pages 0-1 contain UID - only works on special writable UID cards
  
  if (dataLength > 4) {
    result.errorMessage = "NTAG pages are 4 bytes max";
    return result;
  }
  
  // Prepare 4-byte page data
  uint8_t pageData[4] = {0};
  memcpy(pageData, data, dataLength);
  
  // Write to page
  if (_nfc->mifareultralight_WritePage(page, pageData)) {
    result.success = true;
    
    // Verify if requested
    if (verify) {
      uint8_t readBack[4];
      if (readNTAGPage(page, readBack)) {
        result.verified = verifyWrite(pageData, readBack, 4);
        if (!result.verified) {
          result.errorMessage = "Write succeeded but verification failed";
        }
      } else {
        result.errorMessage = "Write succeeded but couldn't read back for verification";
      }
    }
  } else {
    result.errorMessage = "Write operation failed";
  }
  
  return result;
}

// Write string to NTAG/Ultralight
NFCWriteResult NFCReader::writeNTAGString(uint8_t startPage, const String& text, bool verify) {
  NFCWriteResult result;
  result.success = true;
  result.verified = true;
  
  uint8_t textLength = text.length();
  uint8_t page = startPage;
  
  for (uint8_t i = 0; i < textLength; i += 4) {
    uint8_t chunkSize = min(4, textLength - i);
    NFCWriteResult pageResult = writeNTAG(page, (const uint8_t*)(text.c_str() + i), chunkSize, verify);
    
    if (!pageResult.success) {
      result.success = false;
      result.errorMessage = "Failed at page " + String(page) + ": " + pageResult.errorMessage;
      return result;
    }
    
    if (verify && !pageResult.verified) {
      result.verified = false;
    }
    
    page++;
  }
  
  return result;
}

// Write to Mifare Classic block
NFCWriteResult NFCReader::writeMifareClassic(uint8_t block, const uint8_t* data, uint8_t dataLength, 
                                              const uint8_t* key, bool useKeyB, bool verify) {
  NFCWriteResult result;
  result.success = false;
  result.verified = false;
  
  if (!_nfc || !_lastCardInfo.detected) {
    result.errorMessage = "No card detected";
    return result;
  }
  
  // Allow block 0 write for magic cards (will fail on regular cards)
  // Block 0 contains UID - only works on special writable UID cards
  
  // Check if this is a trailer block (last block of sector) - still dangerous
  if (block != 0 && (block + 1) % 4 == 0) {
    result.errorMessage = "Block " + String(block) + " is a sector trailer (contains keys). Writing to trailers is dangerous!";
    return result;
  }
  
  if (dataLength > 16) {
    result.errorMessage = "Mifare Classic blocks are 16 bytes max";
    return result;
  }
  
  // Prepare 16-byte block data
  uint8_t blockData[16] = {0};
  memcpy(blockData, data, dataLength);
  
  // Authenticate
  if (!authenticateMifareBlock(block, key, useKeyB, _lastCardInfo.uid, _lastCardInfo.uidLength)) {
    result.errorMessage = "Authentication failed for block " + String(block);
    return result;
  }
  
  // Write block
  if (_nfc->mifareclassic_WriteDataBlock(block, blockData)) {
    result.success = true;
    
    // Verify if requested
    if (verify) {
      // Re-authenticate for reading
      if (authenticateMifareBlock(block, key, useKeyB, _lastCardInfo.uid, _lastCardInfo.uidLength)) {
        uint8_t readBack[16];
        if (_nfc->mifareclassic_ReadDataBlock(block, readBack)) {
          result.verified = verifyWrite(blockData, readBack, 16);
          if (!result.verified) {
            result.errorMessage = "Write succeeded but verification failed";
          }
        } else {
          result.errorMessage = "Write succeeded but couldn't read back for verification";
        }
      } else {
        result.errorMessage = "Write succeeded but couldn't re-authenticate for verification";
      }
    }
  } else {
    result.errorMessage = "Write operation failed for block " + String(block);
  }
  
  return result;
}

// Write string to Mifare Classic
NFCWriteResult NFCReader::writeMifareClassicString(uint8_t startBlock, const String& text, 
                                                     const uint8_t* key, bool useKeyB, bool verify) {
  NFCWriteResult result;
  result.success = true;
  result.verified = true;
  
  uint8_t textLength = text.length();
  uint8_t block = startBlock;
  
  for (uint8_t i = 0; i < textLength; i += 16) {
    // Skip trailer blocks
    if ((block + 1) % 4 == 0) {
      block++;
    }
    
    uint8_t chunkSize = min(16, textLength - i);
    NFCWriteResult blockResult = writeMifareClassic(block, (const uint8_t*)(text.c_str() + i), 
                                                     chunkSize, key, useKeyB, verify);
    
    if (!blockResult.success) {
      result.success = false;
      result.errorMessage = "Failed at block " + String(block) + ": " + blockResult.errorMessage;
      return result;
    }
    
    if (verify && !blockResult.verified) {
      result.verified = false;
    }
    
    block++;
  }
  
  return result;
}

// Generic write that auto-detects card type
NFCWriteResult NFCReader::writeData(const uint8_t* data, uint8_t dataLength, uint8_t startAddress, bool verify) {
  NFCWriteResult result;
  
  if (!_lastCardInfo.detected) {
    result.success = false;
    result.errorMessage = "No card detected. Call readCard() first.";
    return result;
  }
  
  if (_lastCardInfo.cardType == NFCCardType::MIFARE_CLASSIC_1K || 
      _lastCardInfo.cardType == NFCCardType::MIFARE_CLASSIC_4K) {
    // Mifare Classic - startAddress is block number
    uint8_t block = startAddress == 0 ? 4 : startAddress; // Default to block 4 (first data block in sector 1)
    
    // Write in 16-byte chunks
    uint8_t numBlocks = (dataLength + 15) / 16;
    for (uint8_t i = 0; i < numBlocks; i++) {
      uint8_t currentBlock = block + i;
      
      // Skip trailer blocks
      if ((currentBlock + 1) % 4 == 0) {
        currentBlock++;
      }
      
      uint8_t offset = i * 16;
      uint8_t chunkSize = min(16, dataLength - offset);
      
      result = writeMifareClassic(currentBlock, data + offset, chunkSize, DEFAULT_KEY, false, verify);
      if (!result.success) {
        return result;
      }
    }
  } else {
    // NTAG/Ultralight - startAddress is page number
    uint8_t page = startAddress == 0 ? 4 : startAddress; // Default to page 4 (safe user area)
    
    // Write in 4-byte chunks
    uint8_t numPages = (dataLength + 3) / 4;
    for (uint8_t i = 0; i < numPages; i++) {
      uint8_t offset = i * 4;
      uint8_t chunkSize = min(4, dataLength - offset);
      
      result = writeNTAG(page + i, data + offset, chunkSize, verify);
      if (!result.success) {
        return result;
      }
    }
  }
  
  return result;
}

// Generic write string
NFCWriteResult NFCReader::writeString(const String& text, uint8_t startAddress, bool verify) {
  if (!_lastCardInfo.detected) {
    NFCWriteResult result;
    result.success = false;
    result.errorMessage = "No card detected. Call readCard() first.";
    return result;
  }
  
  if (_lastCardInfo.cardType == NFCCardType::MIFARE_CLASSIC_1K || 
      _lastCardInfo.cardType == NFCCardType::MIFARE_CLASSIC_4K) {
    uint8_t block = startAddress == 0 ? 4 : startAddress;
    return writeMifareClassicString(block, text, DEFAULT_KEY, false, verify);
  } else {
    uint8_t page = startAddress == 0 ? 4 : startAddress;
    return writeNTAGString(page, text, verify);
  }
}

// ========== CUSTOM SECTOR OPERATIONS ==========

// Read custom sector data (Sector 1, blocks 4-6)
bool NFCReader::readCustomSector(NFCCardInfo& info) {
  if (!_nfc || !info.detected) {
    return false;
  }
  
  // Only works for Mifare Classic
  if (info.cardType != NFCCardType::MIFARE_CLASSIC_1K && 
      info.cardType != NFCCardType::MIFARE_CLASSIC_4K) {
    return false;
  }
  
  // Authenticate to block 4 (first block of sector 1)
  if (!authenticateMifareBlock(CUSTOM_BLOCK_UID, DEFAULT_KEY, false, info.uid, info.uidLength)) {
    Serial.println(F("Custom sector auth failed"));
    return false;
  }
  
  // Read block 4 (UID block)
  uint8_t blockData[16];
  if (!_nfc->mifareclassic_ReadDataBlock(CUSTOM_BLOCK_UID, blockData)) {
    Serial.println(F("Custom sector read failed"));
    return false;
  }
  
  // Check magic bytes
  if (blockData[0] == CARD_MAGIC_BYTE1 && blockData[1] == CARD_MAGIC_BYTE2) {
    // Card is initialized with our custom data
    info.hasClonedUID = true;
    info.clonedUIDLength = blockData[2]; // UID length stored in byte 2
    
    if (info.clonedUIDLength > 0 && info.clonedUIDLength <= 7) {
      // Copy cloned UID (bytes 3-9)
      memcpy(info.clonedUID, &blockData[3], info.clonedUIDLength);
      
      Serial.print(F("Found cloned UID: "));
      for (uint8_t i = 0; i < info.clonedUIDLength; i++) {
        if (info.clonedUID[i] < 0x10) Serial.print(F("0"));
        Serial.print(info.clonedUID[i], HEX);
        if (i < info.clonedUIDLength - 1) Serial.print(F(" "));
      }
      Serial.println();
      
      return true;
    }
  }
  
  // No valid custom data found
  info.hasClonedUID = false;
  return false;
}

// Write cloned UID to custom sector
bool NFCReader::writeClonedUID(const uint8_t* sourceUID, uint8_t sourceUIDLength) {
  if (!_nfc || !_lastCardInfo.detected) {
    Serial.println(F("No card for clone write"));
    return false;
  }
  
  // Only works for Mifare Classic
  if (_lastCardInfo.cardType != NFCCardType::MIFARE_CLASSIC_1K && 
      _lastCardInfo.cardType != NFCCardType::MIFARE_CLASSIC_4K) {
    Serial.println(F("Clone only for Mifare Classic"));
    return false;
  }
  
  if (sourceUIDLength > 7) {
    Serial.println(F("UID too long"));
    return false;
  }
  
  Serial.print(F("Writing cloned UID to block "));
  Serial.print(CUSTOM_BLOCK_UID);
  Serial.print(F(": "));
  for (uint8_t i = 0; i < sourceUIDLength; i++) {
    if (sourceUID[i] < 0x10) Serial.print(F("0"));
    Serial.print(sourceUID[i], HEX);
    if (i < sourceUIDLength - 1) Serial.print(F(" "));
  }
  Serial.println();
  
  // Prepare block data
  // Format: [Magic1][Magic2][UIDLen][UID0-6][Reserved bytes]
  uint8_t blockData[16] = {0};
  blockData[0] = CARD_MAGIC_BYTE1;
  blockData[1] = CARD_MAGIC_BYTE2;
  blockData[2] = sourceUIDLength;
  memcpy(&blockData[3], sourceUID, sourceUIDLength);
  // Bytes 10-15 reserved for future use
  
  // Authenticate to block 4
  if (!authenticateMifareBlock(CUSTOM_BLOCK_UID, DEFAULT_KEY, false, 
                                _lastCardInfo.uid, _lastCardInfo.uidLength)) {
    Serial.println(F("Auth failed for clone write"));
    return false;
  }
  
  // Write to block 4
  if (!_nfc->mifareclassic_WriteDataBlock(CUSTOM_BLOCK_UID, blockData)) {
    Serial.println(F("Clone write failed"));
    return false;
  }
  
  Serial.println(F("Clone write SUCCESS"));
  
  // Verify by reading back
  if (authenticateMifareBlock(CUSTOM_BLOCK_UID, DEFAULT_KEY, false, 
                              _lastCardInfo.uid, _lastCardInfo.uidLength)) {
    uint8_t readBack[16];
    if (_nfc->mifareclassic_ReadDataBlock(CUSTOM_BLOCK_UID, readBack)) {
      // Check if data matches
      bool verified = true;
      for (uint8_t i = 0; i < 10; i++) { // Check first 10 bytes (magic + len + UID)
        if (readBack[i] != blockData[i]) {
          verified = false;
          break;
        }
      }
      
      if (verified) {
        Serial.println(F("Clone VERIFIED!"));
        return true;
      } else {
        Serial.println(F("Clone verify MISMATCH"));
      }
    }
  }
  
  return false;
}

// Check if card is initialized with our custom data
bool NFCReader::isCardInitialized() {
  if (!_lastCardInfo.detected) {
    return false;
  }
  
  NFCCardInfo tempInfo = _lastCardInfo;
  return readCustomSector(tempInfo);
}

// Initialize blank card with empty custom sector
bool NFCReader::initializeCard() {
  if (!_nfc || !_lastCardInfo.detected) {
    return false;
  }
  
  // Only works for Mifare Classic
  if (_lastCardInfo.cardType != NFCCardType::MIFARE_CLASSIC_1K && 
      _lastCardInfo.cardType != NFCCardType::MIFARE_CLASSIC_4K) {
    return false;
  }
  
  Serial.println(F("Initializing card..."));
  
  // Prepare empty block with magic bytes but no UID
  uint8_t blockData[16] = {0};
  blockData[0] = CARD_MAGIC_BYTE1;
  blockData[1] = CARD_MAGIC_BYTE2;
  blockData[2] = 0; // No UID yet
  
  // Authenticate to block 4
  if (!authenticateMifareBlock(CUSTOM_BLOCK_UID, DEFAULT_KEY, false, 
                                _lastCardInfo.uid, _lastCardInfo.uidLength)) {
    Serial.println(F("Auth failed for init"));
    return false;
  }
  
  // Write to block 4
  if (!_nfc->mifareclassic_WriteDataBlock(CUSTOM_BLOCK_UID, blockData)) {
    Serial.println(F("Init write failed"));
    return false;
  }
  
  Serial.println(F("Card initialized"));
  return true;
}
