#include "AccessControlSystem.h"

// Store strings in flash memory to save RAM
const char STR_SYSTEM_READY[] PROGMEM = "  System Ready  ";
const char STR_SCAN_CARD[] PROGMEM = "  Scan Card...  ";
const char STR_ACCESS_GRANTED[] PROGMEM = " Access Granted ";
const char STR_WELCOME[] PROGMEM = "   Welcome!     ";
const char STR_ACCESS_DENIED[] PROGMEM = " Access Denied  ";
const char STR_UNKNOWN_CARD[] PROGMEM = " Unknown Card   ";
const char STR_REGISTER[] PROGMEM = "Register Card";
const char STR_DELETE[] PROGMEM = "Delete Card";
const char STR_LIST[] PROGMEM = "List Cards";
const char STR_CLONE[] PROGMEM = "Clone Card";
const char STR_SETTINGS[] PROGMEM = "Settings";
const char STR_CLEAR_ALL[] PROGMEM = "Clear All";
const char STR_EXIT[] PROGMEM = "Exit Menu";

AccessControlSystem::AccessControlSystem(NFCReader& nfcReader)
  : _nfc(nfcReader),
    _lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7),
    _currentState(SystemState::IDLE),
    _lastDisplayState(SystemState::IDLE),
    _currentMenuItem(MenuItem::REGISTER_CARD),
    _lastDisplayMenuItem(MenuItem::REGISTER_CARD),
    _menuIndex(0),
    _stateChangeTime(0),
    _lastActivityTime(0),
    _relayActivationTime(0),
    _lastDisplayUpdate(0),
    _relayActive(false),
    _displayNeedsUpdate(true),
    _cachedCardCount(0),
    _cardCountCacheValid(false),
    _listCardIndex(0),
    _btnUpPressed(false),
    _btnDownPressed(false),
    _btnSelectPressed(false),
    _btnBackPressed(false),
    _btnSelectPressTime(0),
    _btnUpLastChange(0),
    _btnDownLastChange(0),
    _btnSelectLastChange(0),
    _btnBackLastChange(0),
    _btnUpLastState(HIGH),
    _btnDownLastState(HIGH),
    _btnSelectLastState(HIGH),
    _btnBackLastState(HIGH),
    _btnUpStable(false),
    _btnDownStable(false),
    _btnSelectStable(false),
    _btnBackStable(false)
{
}

bool AccessControlSystem::begin() {
  Serial.println(F("\n=== Access Control System ==="));
  Serial.println(F("Initializing hardware...\n"));
  
  // Initialize LCD
  Serial.print(F("LCD: "));
  _lcd.begin(LCD_COLS, LCD_ROWS);
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print(F("Access Control"));
  _lcd.setCursor(0, 1);
  _lcd.print(F("Initializing..."));
  Serial.println(F("OK"));
  
  // Initialize NFC Reader
  Serial.print(F("NFC: "));
  if (!_nfc.begin()) {
    Serial.println(F("FAILED!"));
    _lcd.clear();
    _lcd.print(F("NFC ERROR!"));
    _lcd.setCursor(0, 1);
    _lcd.print(F("Check wiring"));
    return false;
  }
  Serial.println(F("OK"));
  
  // Initialize hardware
  Serial.print(F("Buttons: "));
  initButtons();
  Serial.println(F("OK"));
  
  Serial.print(F("Relay: "));
  initRelay();
  Serial.println(F("OK"));
  
  Serial.print(F("EEPROM: "));
  initEEPROM();
  Serial.println(F("OK"));
  
  setState(SystemState::IDLE);
  updateDisplay(); // Force initial display update
  
  Serial.println(F("\n=== System Ready ==="));
  Serial.println(F("Scan card or long-press SELECT for menu\n"));
  return true;
}

void AccessControlSystem::initButtons() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
}

void AccessControlSystem::initRelay() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? LOW : HIGH); // Start locked
}

void AccessControlSystem::initEEPROM() {
  // Check if EEPROM is initialized
  uint16_t magic = (EEPROM.read(EEPROM_MAGIC_ADDR) << 8) | EEPROM.read(EEPROM_MAGIC_ADDR + 1);
  
  if (magic != EEPROM_MAGIC_NUMBER) {
    // First time - initialize EEPROM
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_NUMBER >> 8);
    EEPROM.write(EEPROM_MAGIC_ADDR + 1, EEPROM_MAGIC_NUMBER & 0xFF);
    saveCardCount(0);
  }
  
  // Load card count into cache
  _cachedCardCount = EEPROM.read(EEPROM_CARD_COUNT_ADDR);
  _cardCountCacheValid = true;
}

// ========== MAIN UPDATE LOOP ==========

void AccessControlSystem::update() {
  updateButtons();
  updateRelay();
  checkStateTimeout();
  
  // Read NFC card
  NFCCardInfo cardInfo = _nfc.readCard();
  
  // Handle card based on current state
  switch (_currentState) {
    case SystemState::IDLE:
      if (cardInfo.detected) {
        // Show physical UID
        Serial.print(F("Physical UID: "));
        for (uint8_t i = 0; i < cardInfo.uidLength; i++) {
          if (cardInfo.uid[i] < 0x10) Serial.print(F("0"));
          Serial.print(cardInfo.uid[i], HEX);
          if (i < cardInfo.uidLength - 1) Serial.print(F(" "));
        }
        Serial.println();
        
        // Show cloned UID if present
        if (cardInfo.hasClonedUID) {
          Serial.print(F("Cloned UID: "));
          for (uint8_t i = 0; i < cardInfo.clonedUIDLength; i++) {
            if (cardInfo.clonedUID[i] < 0x10) Serial.print(F("0"));
            Serial.print(cardInfo.clonedUID[i], HEX);
            if (i < cardInfo.clonedUIDLength - 1) Serial.print(F(" "));
          }
          Serial.println(F(" (from Sector 1)"));
        }
        
        if (isCardAuthorized(cardInfo)) {
          Serial.println(F("Access GRANTED"));
          grantAccess();
        } else {
          Serial.println(F("Access DENIED"));
          denyAccess();
        }
      }
      break;
      
    case SystemState::REGISTERING:
      if (cardInfo.detected) {
        if (addCard(cardInfo)) {
          displayMessage("Card Added!", "Successfully");
        } else {
          displayMessage("Error!", "Card exists/full");
        }
        setState(SystemState::IDLE);
      }
      break;
      
    case SystemState::DELETING:
      if (cardInfo.detected) {
        if (deleteCard(cardInfo)) {
          displayMessage("Card Deleted!", "Successfully");
        } else {
          displayMessage("Error!", "Card not found");
        }
        setState(SystemState::IDLE);
      }
      break;
      
    case SystemState::CLONING_SOURCE:
      if (cardInfo.detected) {
        _cloneSourceCard = cardInfo;
        setState(SystemState::CLONING_TARGET);
        
        // Use effective UID for cloning (cloned if present, otherwise physical)
        const uint8_t* effectiveUID = cardInfo.getEffectiveUID();
        uint8_t effectiveLength = cardInfo.getEffectiveUIDLength();
        
        // Display source UID
        char line1[17] = "Src: ";
        char uidStr[12];
        for (uint8_t i = 0; i < min(effectiveLength, (uint8_t)4); i++) {
          sprintf(uidStr + (i*2), "%02X", effectiveUID[i]);
        }
        strcat(line1, uidStr);
        displayMessage(line1, "Remove & scan new");
        
        Serial.print(F("Clone source UID: "));
        for (uint8_t i = 0; i < effectiveLength; i++) {
          if (effectiveUID[i] < 0x10) Serial.print(F("0"));
          Serial.print(effectiveUID[i], HEX);
          Serial.print(F(" "));
        }
        Serial.println();
        
        if (cardInfo.hasClonedUID) {
          Serial.println(F("(Using cloned UID from custom sector)"));
        } else {
          Serial.println(F("(Using physical manufacturer UID)"));
        }
        
        Serial.println(F("Remove source card and scan target card..."));
        // Don't call resetCardState here - let the automatic timeout handle it
        // when the source card is removed
      }
      break;
      
    case SystemState::CLONING_TARGET:
      if (cardInfo.detected) {
        Serial.println(F("Target card detected, cloning to custom sector..."));
        
        // Check if target card is different from source (compare physical UIDs)
        bool sameCard = true;
        if (cardInfo.uidLength == _cloneSourceCard.uidLength) {
          for (uint8_t i = 0; i < cardInfo.uidLength; i++) {
            if (cardInfo.uid[i] != _cloneSourceCard.uid[i]) {
              sameCard = false;
              break;
            }
          }
        } else {
          sameCard = false;
        }
        
        if (sameCard) {
          Serial.println(F("Error: Same card scanned twice"));
          displayMessage("Error!", "Same card");
          setState(SystemState::IDLE);
          break;
        }
        
        // Check if target is Mifare Classic
        if (cardInfo.cardType != NFCCardType::MIFARE_CLASSIC_1K && 
            cardInfo.cardType != NFCCardType::MIFARE_CLASSIC_4K) {
          Serial.println(F("Error: Target must be Mifare Classic 1K/4K"));
          displayMessage("Error!", "Need Classic 1K");
          delay(2000);
          setState(SystemState::IDLE);
          break;
        }
        
        // Get effective UID from source (cloned if present, otherwise physical)
        const uint8_t* sourceUID = _cloneSourceCard.getEffectiveUID();
        uint8_t sourceLength = _cloneSourceCard.getEffectiveUIDLength();
        
        // Display what we're cloning
        Serial.print(F("Cloning UID to sector 1: "));
        for (uint8_t i = 0; i < sourceLength; i++) {
          if (sourceUID[i] < 0x10) Serial.print(F("0"));
          Serial.print(sourceUID[i], HEX);
          Serial.print(F(" "));
        }
        Serial.println();
        
        displayMessage("Cloning to", "Sector 1...");
        delay(100); // Brief delay for display
        
        // Write cloned UID to custom sector (works on ANY Mifare Classic card)
        bool success = _nfc.writeClonedUID(sourceUID, sourceLength);
        
        if (success) {
          Serial.println(F("SUCCESS: Cloned UID written to custom sector!"));
          displayMessage("Clone SUCCESS!", "Sector 1 OK");
        } else {
          Serial.println(F("FAILED: Could not write to custom sector"));
          displayMessage("Clone Failed!", "Write error");
        }
        
        delay(2000); // Give time to read the message
        
        // Reset NFC reader state after delay to allow new card detection
        _nfc.resetCardState();
        Serial.println(F("NFC reader reset, ready for next card"));
        
        setState(SystemState::IDLE);
      }
      break;
      
    default:
      break;
  }
  
  updateDisplay();
}

void AccessControlSystem::updateButtons() {
  bool upNow = readButtonDebounced(BTN_UP, _btnUpLastState, _btnUpLastChange);
  bool downNow = readButtonDebounced(BTN_DOWN, _btnDownLastState, _btnDownLastChange);
  bool selectNow = readButtonDebounced(BTN_SELECT, _btnSelectLastState, _btnSelectLastChange);
  bool backNow = readButtonDebounced(BTN_BACK, _btnBackLastState, _btnBackLastChange);
  
  _lastActivityTime = millis();
  
  if (upNow && !_btnUpPressed) {
    Serial.println(F("BTN: UP"));
    if (_currentState == SystemState::LISTING_CARDS) {
      listCardsUp();
    } else {
      menuUp();
    }
  }
  
  if (downNow && !_btnDownPressed) {
    Serial.println(F("BTN: DOWN"));
    if (_currentState == SystemState::LISTING_CARDS) {
      listCardsDown();
    } else {
      menuDown();
    }
  }
  
  if (selectNow && !_btnSelectPressed) {
    _btnSelectPressTime = millis();
  }
  
  if (!selectNow && _btnSelectPressed) {
    unsigned long pressDuration = millis() - _btnSelectPressTime;
    if (pressDuration >= LONG_PRESS_TIME) {
      // Long press - enter/exit menu
      Serial.println(F("BTN: SELECT (LONG)"));
      if (_currentState == SystemState::IDLE) {
        enterMenu();
      }
    } else {
      // Short press - select current item
      Serial.println(F("BTN: SELECT"));
      menuSelect();
    }
  }
  
  if (backNow && !_btnBackPressed) {
    Serial.println(F("BTN: BACK"));
    menuBack();
  }
  
  _btnUpPressed = upNow;
  _btnDownPressed = downNow;
  _btnSelectPressed = selectNow;
  _btnBackPressed = backNow;
}

bool AccessControlSystem::readButtonDebounced(uint8_t pin, bool& lastState, unsigned long& lastChangeTime) {
  bool reading = !digitalRead(pin); // Active LOW - true when pressed
  unsigned long now = millis();
  
  // If state changed
  if (reading != lastState) {
    // If enough time passed since last change (debounce)
    if ((now - lastChangeTime) >= BUTTON_DEBOUNCE_TIME) {
      lastState = reading;
      lastChangeTime = now;
      return reading; // Return new stable state immediately
    }
  }
  
  // No change or too soon - return current stable state
  return lastState;
}

// ========== MENU NAVIGATION ==========

void AccessControlSystem::enterMenu() {
  setState(SystemState::MENU);
  _menuIndex = 0;
  _currentMenuItem = MenuItem::REGISTER_CARD;
}

void AccessControlSystem::exitMenu() {
  setState(SystemState::IDLE);
}

void AccessControlSystem::menuUp() {
  if (_currentState == SystemState::MENU) {
    if (_menuIndex > 0) {
      _menuIndex--;
    } else {
      _menuIndex = (uint8_t)MenuItem::MENU_COUNT - 1;
    }
    _currentMenuItem = (MenuItem)_menuIndex;
    _displayNeedsUpdate = true;
  }
}

void AccessControlSystem::menuDown() {
  if (_currentState == SystemState::MENU) {
    _menuIndex++;
    if (_menuIndex >= (uint8_t)MenuItem::MENU_COUNT) {
      _menuIndex = 0;
    }
    _currentMenuItem = (MenuItem)_menuIndex;
    _displayNeedsUpdate = true;
  }
}

void AccessControlSystem::menuSelect() {
  if (_currentState != SystemState::MENU) return;
  
  switch (_currentMenuItem) {
    case MenuItem::REGISTER_CARD:
      setState(SystemState::REGISTERING);
      break;
      
    case MenuItem::DELETE_CARD:
      setState(SystemState::DELETING);
      break;
      
    case MenuItem::LIST_CARDS:
      {
        uint8_t count = getStoredCardCount();
        if (count == 0) {
          displayMessage("No Cards", "Stored");
          setState(SystemState::IDLE);
        } else {
          _listCardIndex = 0;
          setState(SystemState::LISTING_CARDS);
        }
      }
      break;
      
    case MenuItem::CLONE_CARD:
      setState(SystemState::CLONING_SOURCE);
      break;
      
    case MenuItem::SETTINGS:
      displayMessage("Settings", "Not implemented");
      setState(SystemState::IDLE);
      break;
      
    case MenuItem::CLEAR_ALL:
      clearAllCards();
      displayMessage("All Cards", "Cleared!");
      setState(SystemState::IDLE);
      break;
      
    case MenuItem::EXIT_MENU:
      exitMenu();
      break;
      
    default:
      break;
  }
}

void AccessControlSystem::menuBack() {
  if (_currentState == SystemState::MENU) {
    exitMenu();
  } else if (_currentState == SystemState::LISTING_CARDS) {
    exitMenu();
  } else if (_currentState != SystemState::IDLE && _currentState != SystemState::ACCESS_GRANTED && _currentState != SystemState::ACCESS_DENIED) {
    exitMenu();
  }
}

const char* AccessControlSystem::getMenuItemName(MenuItem item) {
  switch (item) {
    case MenuItem::REGISTER_CARD: return (const char*)STR_REGISTER;
    case MenuItem::DELETE_CARD:   return (const char*)STR_DELETE;
    case MenuItem::LIST_CARDS:    return (const char*)STR_LIST;
    case MenuItem::CLONE_CARD:    return (const char*)STR_CLONE;
    case MenuItem::SETTINGS:      return (const char*)STR_SETTINGS;
    case MenuItem::CLEAR_ALL:     return (const char*)STR_CLEAR_ALL;
    case MenuItem::EXIT_MENU:     return (const char*)STR_EXIT;
    default:                      return "?";
  }
}

// ========== DISPLAY METHODS ==========

void AccessControlSystem::updateDisplay() {
  // Check if we need to update
  bool needsUpdate = (_currentState != _lastDisplayState) || 
                     (_currentMenuItem != _lastDisplayMenuItem) || 
                     _displayNeedsUpdate;
  
  if (!needsUpdate) {
    return; // No update needed
  }
  
  Serial.print(F("Display update: State="));
  Serial.print((int)_currentState);
  Serial.print(F(" Menu="));
  Serial.print((int)_currentMenuItem);
  Serial.print(F(" Flag="));
  Serial.println(_displayNeedsUpdate);
  
  // Update tracking variables AFTER we've checked them
  _lastDisplayState = _currentState;
  _lastDisplayMenuItem = _currentMenuItem;
  _displayNeedsUpdate = false;
  _lastDisplayUpdate = millis();
  
  // Update display based on current state
  switch (_currentState) {
    case SystemState::IDLE:
      displayIdle();
      break;
    case SystemState::MENU:
      displayMenu();
      break;
    case SystemState::REGISTERING:
      displayRegistering();
      break;
    case SystemState::DELETING:
      displayDeleting();
      break;
    case SystemState::LISTING_CARDS:
      displayListingCards();
      break;
    case SystemState::CLONING_SOURCE:
    case SystemState::CLONING_TARGET:
      displayCloning();
      break;
    case SystemState::ACCESS_GRANTED:
      displayAccessGranted();
      break;
    case SystemState::ACCESS_DENIED:
      displayAccessDenied();
      break;
    default:
      break;
  }
}

void AccessControlSystem::displayIdle() {
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print((__FlashStringHelper*)STR_SYSTEM_READY);
  _lcd.setCursor(0, 1);
  _lcd.print((__FlashStringHelper*)STR_SCAN_CARD);
}

void AccessControlSystem::displayAccessGranted() {
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print((__FlashStringHelper*)STR_ACCESS_GRANTED);
  _lcd.setCursor(0, 1);
  _lcd.print((__FlashStringHelper*)STR_WELCOME);
}

void AccessControlSystem::displayAccessDenied() {
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print((__FlashStringHelper*)STR_ACCESS_DENIED);
  _lcd.setCursor(0, 1);
  _lcd.print((__FlashStringHelper*)STR_UNKNOWN_CARD);
}

void AccessControlSystem::displayMenu() {
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print(F(">"));
  _lcd.print((__FlashStringHelper*)getMenuItemName(_currentMenuItem));
  
  // Show next item on second line
  MenuItem nextItem = (MenuItem)((_menuIndex + 1) % (uint8_t)MenuItem::MENU_COUNT);
  _lcd.setCursor(0, 1);
  _lcd.print(F(" "));
  _lcd.print((__FlashStringHelper*)getMenuItemName(nextItem));
}

void AccessControlSystem::displayRegistering() {
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print("Register Card");
  _lcd.setCursor(0, 1);
  _lcd.print("Scan new card...");
}

void AccessControlSystem::displayDeleting() {
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print("Delete Card");
  _lcd.setCursor(0, 1);
  _lcd.print("Scan to delete..");
}

void AccessControlSystem::displayCloning() {
  _lcd.clear();
  _lcd.setCursor(0, 0);
  if (_currentState == SystemState::CLONING_SOURCE) {
    _lcd.print("Clone: Source");
    _lcd.setCursor(0, 1);
    _lcd.print("Scan source card");
  } else {
    _lcd.print("Clone: Target");
    _lcd.setCursor(0, 1);
    _lcd.print("Scan magic card");
  }
}

void AccessControlSystem::displayMessage(const char* line1, const char* line2) {
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print(line1);
  _lcd.setCursor(0, 1);
  _lcd.print(line2);
}

void AccessControlSystem::clearDisplay() {
  _lcd.clear();
}

// ========== STATE MANAGEMENT ==========

void AccessControlSystem::setState(SystemState newState) {
  if (_currentState != newState) {
    Serial.print(F("State: "));
    Serial.print((int)_currentState);
    Serial.print(F(" -> "));
    Serial.println((int)newState);
  }
  _currentState = newState;
  _stateChangeTime = millis();
  _lastActivityTime = millis();
  _displayNeedsUpdate = true;
}

void AccessControlSystem::checkStateTimeout() {
  unsigned long now = millis();
  
  // Auto-return from access granted/denied
  if ((_currentState == SystemState::ACCESS_GRANTED || _currentState == SystemState::ACCESS_DENIED) &&
      (now - _stateChangeTime >= MESSAGE_DISPLAY_TIME)) {
    Serial.print(F("Timeout: "));
    Serial.print(now - _stateChangeTime);
    Serial.println(F("ms"));
    setState(SystemState::IDLE);
  }
  
  // Menu timeout
  if (_currentState == SystemState::MENU && (now - _lastActivityTime >= MENU_TIMEOUT)) {
    exitMenu();
  }
}

// ========== CARD MANAGEMENT ==========

bool AccessControlSystem::isCardAuthorized(const NFCCardInfo& cardInfo) {
  return findCardInEEPROM(cardInfo) >= 0;
}

bool AccessControlSystem::addCard(const NFCCardInfo& cardInfo) {
  // Check if card already exists
  if (findCardInEEPROM(cardInfo) >= 0) {
    return false; // Card already registered
  }
  
  uint8_t count = _cardCountCacheValid ? _cachedCardCount : loadCardCount();
  if (count >= MAX_STORED_CARDS) {
    return false; // Storage full
  }
  
  // Add new card - use effective UID (cloned if present, otherwise physical)
  StoredCard card;
  const uint8_t* effectiveUID = cardInfo.getEffectiveUID();
  uint8_t effectiveLength = cardInfo.getEffectiveUIDLength();
  
  memcpy(card.uid, effectiveUID, effectiveLength);
  card.uidLength = effectiveLength;
  card.active = true;
  
  saveCardToEEPROM(card, count);
  saveCardCount(count + 1);
  
  return true;
}

bool AccessControlSystem::deleteCard(const NFCCardInfo& cardInfo) {
  int index = findCardInEEPROM(cardInfo);
  if (index < 0) {
    return false; // Card not found
  }
  
  // Shift all cards after this one
  uint8_t count = _cardCountCacheValid ? _cachedCardCount : loadCardCount();
  for (int i = index; i < count - 1; i++) {
    StoredCard card;
    loadCardFromEEPROM(card, i + 1);
    saveCardToEEPROM(card, i);
  }
  
  saveCardCount(count - 1);
  return true;
}

void AccessControlSystem::clearAllCards() {
  saveCardCount(0);
}

uint8_t AccessControlSystem::getStoredCardCount() {
  return _cardCountCacheValid ? _cachedCardCount : loadCardCount();
}

int AccessControlSystem::findCardInEEPROM(const NFCCardInfo& cardInfo) {
  uint8_t count = _cardCountCacheValid ? _cachedCardCount : loadCardCount();
  
  // Use effective UID (cloned if present, otherwise physical)
  const uint8_t* effectiveUID = cardInfo.getEffectiveUID();
  uint8_t effectiveLength = cardInfo.getEffectiveUIDLength();
  
  for (uint8_t i = 0; i < count; i++) {
    StoredCard card;
    if (loadCardFromEEPROM(card, i)) {
      if (card.active && card.uidLength == effectiveLength) {
        if (compareUIDs(card.uid, effectiveUID, effectiveLength)) {
          return i;
        }
      }
    }
  }
  
  return -1;
}

bool AccessControlSystem::compareUIDs(const uint8_t* uid1, const uint8_t* uid2, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

// ========== EEPROM OPERATIONS ==========

void AccessControlSystem::saveCardToEEPROM(const StoredCard& card, uint8_t index) {
  uint16_t addr = EEPROM_CARDS_START + (index * (MAX_UID_LENGTH + 2));
  
  EEPROM.write(addr++, card.uidLength);
  EEPROM.write(addr++, card.active ? 1 : 0);
  
  for (uint8_t i = 0; i < MAX_UID_LENGTH; i++) {
    EEPROM.write(addr++, card.uid[i]);
  }
}

bool AccessControlSystem::loadCardFromEEPROM(StoredCard& card, uint8_t index) {
  uint16_t addr = EEPROM_CARDS_START + (index * (MAX_UID_LENGTH + 2));
  
  card.uidLength = EEPROM.read(addr++);
  card.active = EEPROM.read(addr++) == 1;
  
  for (uint8_t i = 0; i < MAX_UID_LENGTH; i++) {
    card.uid[i] = EEPROM.read(addr++);
  }
  
  return card.uidLength > 0 && card.uidLength <= MAX_UID_LENGTH;
}

void AccessControlSystem::saveCardCount(uint8_t count) {
  EEPROM.write(EEPROM_CARD_COUNT_ADDR, count);
  _cachedCardCount = count;
  _cardCountCacheValid = true;
}

uint8_t AccessControlSystem::loadCardCount() {
  return EEPROM.read(EEPROM_CARD_COUNT_ADDR);
}

// ========== ACCESS CONTROL ==========

void AccessControlSystem::grantAccess() {
  setState(SystemState::ACCESS_GRANTED);
  displayAccessGranted();
  unlockDoor();
}

void AccessControlSystem::denyAccess() {
  setState(SystemState::ACCESS_DENIED);
  displayAccessDenied();
}

void AccessControlSystem::unlockDoor() {
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? HIGH : LOW);
  _relayActive = true;
  _relayActivationTime = millis();
}

void AccessControlSystem::updateRelay() {
  if (_relayActive && (millis() - _relayActivationTime >= DOOR_UNLOCK_TIME)) {
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? LOW : HIGH); // Lock door
    _relayActive = false;
  }
}

// ========== LIST CARDS NAVIGATION ==========

void AccessControlSystem::listCardsUp() {
  if (_listCardIndex > 0) {
    _listCardIndex--;
    _displayNeedsUpdate = true;
  }
}

void AccessControlSystem::listCardsDown() {
  uint8_t count = getStoredCardCount();
  if (_listCardIndex < count - 1) {
    _listCardIndex++;
    _displayNeedsUpdate = true;
  }
}

void AccessControlSystem::displayListingCards() {
  uint8_t count = getStoredCardCount();
  StoredCard card;
  
  _lcd.clear();
  
  if (loadCardFromEEPROM(card, _listCardIndex)) {
    // Line 1: Card number and indicator
    _lcd.setCursor(0, 0);
    _lcd.print("#");
    _lcd.print(_listCardIndex + 1);
    _lcd.print("/");
    _lcd.print(count);
    _lcd.print(" [Cloned]");
    
    // Line 2: UID from custom sector (truncated to fit)
    _lcd.setCursor(0, 1);
    for (uint8_t i = 0; i < min(card.uidLength, (uint8_t)7); i++) {
      if (card.uid[i] < 0x10) _lcd.print("0");
      _lcd.print(card.uid[i], HEX);
      if (i < min(card.uidLength, (uint8_t)7) - 1) _lcd.print(" ");
    }
  } else {
    _lcd.setCursor(0, 0);
    _lcd.print("Error reading");
    _lcd.setCursor(0, 1);
    _lcd.print("card data");
  }
}
