#ifndef ACCESS_CONTROL_SYSTEM_H
#define ACCESS_CONTROL_SYSTEM_H

#include <Arduino.h>
#include <LiquidCrystal.h>

// Suppress unused variable warning from EEPROM library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <EEPROM.h>
#pragma GCC diagnostic pop

#include "Config.h"
#include "NFCReader.h"

// System states
enum class SystemState {
  IDLE,           // Ready to scan card
  ACCESS_GRANTED, // Card authorized
  ACCESS_DENIED,  // Card not authorized
  MENU,           // In menu system
  REGISTERING,    // Registering new card
  DELETING,       // Deleting card
  LISTING_CARDS,  // Displaying stored cards
  CLONING_SOURCE, // Waiting for source card to clone
  CLONING_TARGET  // Waiting for target card to write
};

// Menu items
enum class MenuItem {
  REGISTER_CARD,
  DELETE_CARD,
  LIST_CARDS,
  CLONE_CARD,
  SETTINGS,
  CLEAR_ALL,
  EXIT_MENU,
  MENU_COUNT  // Total number of menu items
};

// Stored card structure
struct StoredCard {
  uint8_t uid[MAX_UID_LENGTH];
  uint8_t uidLength;
  bool active;  // For soft delete
};

class AccessControlSystem {
public:
  AccessControlSystem(NFCReader& nfcReader);
  
  // Initialization
  bool begin();
  
  // Main loop - call this repeatedly
  void update();
  
  // Card management
  bool isCardAuthorized(const NFCCardInfo& cardInfo);
  bool addCard(const NFCCardInfo& cardInfo);
  bool deleteCard(const NFCCardInfo& cardInfo);
  void clearAllCards();
  uint8_t getStoredCardCount();
  
  // System control
  void grantAccess();
  void denyAccess();
  void unlockDoor();
  
private:
  NFCReader& _nfc;
  LiquidCrystal _lcd;
  
  SystemState _currentState;
  SystemState _lastDisplayState;
  MenuItem _currentMenuItem;
  MenuItem _lastDisplayMenuItem;
  uint8_t _menuIndex;
  
  unsigned long _stateChangeTime;
  unsigned long _lastActivityTime;
  unsigned long _relayActivationTime;
  unsigned long _lastDisplayUpdate;
  bool _relayActive;
  bool _displayNeedsUpdate;
  
  // EEPROM cache
  uint8_t _cachedCardCount;
  bool _cardCountCacheValid;
  
  // List cards state
  uint8_t _listCardIndex;
  
  // Button states
  bool _btnUpPressed;
  bool _btnDownPressed;
  bool _btnSelectPressed;
  bool _btnBackPressed;
  unsigned long _btnSelectPressTime;
  
  // Button debounce timing
  unsigned long _btnUpLastChange;
  unsigned long _btnDownLastChange;
  unsigned long _btnSelectLastChange;
  unsigned long _btnBackLastChange;
  bool _btnUpLastState;
  bool _btnDownLastState;
  bool _btnSelectLastState;
  bool _btnBackLastState;
  bool _btnUpStable;
  bool _btnDownStable;
  bool _btnSelectStable;
  bool _btnBackStable;
  
  // Clone operation
  NFCCardInfo _cloneSourceCard;
  
  // Initialization
  void initEEPROM();
  void initButtons();
  void initRelay();
  
  // Display methods
  void updateDisplay();
  void displayIdle();
  void displayAccessGranted();
  void displayAccessDenied();
  void displayMenu();
  void displayRegistering();
  void displayDeleting();
  void displayCloning();
  void displayMessage(const char* line1, const char* line2);
  void clearDisplay();
  
  // Button handling
  void updateButtons();
  bool isButtonPressed(uint8_t pin);
  bool readButtonDebounced(uint8_t pin, bool& lastState, unsigned long& lastChangeTime);
  
  // Menu navigation
  void menuUp();
  void menuDown();
  void menuSelect();
  void menuBack();
  void enterMenu();
  void exitMenu();
  const char* getMenuItemName(MenuItem item);
  
  // List cards navigation
  void listCardsUp();
  void listCardsDown();
  void displayListingCards();
  
  // State management
  void setState(SystemState newState);
  void checkStateTimeout();
  
  // EEPROM operations
  void saveCardToEEPROM(const StoredCard& card, uint8_t index);
  bool loadCardFromEEPROM(StoredCard& card, uint8_t index);
  void saveCardCount(uint8_t count);
  uint8_t loadCardCount();
  int findCardInEEPROM(const NFCCardInfo& cardInfo);
  
  // Relay control
  void updateRelay();
  
  // Card comparison
  bool compareUIDs(const uint8_t* uid1, const uint8_t* uid2, uint8_t length);
};

#endif // ACCESS_CONTROL_SYSTEM_H
