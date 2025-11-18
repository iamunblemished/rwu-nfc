#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========== HARDWARE CONFIGURATION ==========

// NFC PN532 Pins (SPI Mode)
#define NFC_SCK   13
#define NFC_MISO  12
#define NFC_MOSI  11
#define NFC_SS    10
#define NFC_IRQ   2
#define NFC_RST   3

// LCD Pins (4-bit Parallel Mode)
#define LCD_RS    4
#define LCD_EN    5
#define LCD_D4    6
#define LCD_D5    7
#define LCD_D6    8
#define LCD_D7    9

// Button Pins (Active LOW with internal pull-up)
#define BTN_UP     A0
#define BTN_DOWN   A1
#define BTN_SELECT A2
#define BTN_BACK   A3

// Output Pins
#define RELAY_PIN   A6

// ========== SYSTEM SETTINGS ==========

// LCD Configuration
#define LCD_COLS   16
#define LCD_ROWS   2

// Access Control Settings
#define RELAY_ACTIVE_HIGH  true   // true = relay ON when pin HIGH, false = active LOW
#define DOOR_UNLOCK_TIME   3000   // milliseconds
#define MAX_STORED_CARDS   40     // Maximum number of cards to store in EEPROM

// Button Settings
#define BUTTON_DEBOUNCE_TIME  20   // milliseconds
#define LONG_PRESS_TIME       1000 // milliseconds for long press

// EEPROM Addresses
#define EEPROM_MAGIC_ADDR      0    // Magic number to check if EEPROM is initialized
#define EEPROM_CARD_COUNT_ADDR 2    // Number of stored cards
#define EEPROM_CARDS_START     4    // Start of card storage area
#define EEPROM_MAGIC_NUMBER    0xABCD  // Magic number value

// Card UID Settings
#define MAX_UID_LENGTH     7    // Maximum UID length (Mifare Classic = 4, Ultralight = 7)

// NFC Communication Mode Shortcuts
#define NFC_COMM_I2C  NFCCommMode::I2C
#define NFC_COMM_SPI  NFCCommMode::SPI
#define NFC_READ_POLLING  NFCReadMode::POLLING
#define NFC_READ_IRQ      NFCReadMode::IRQ

// Display Timeouts
#define MESSAGE_DISPLAY_TIME  2000  // How long to show access granted/denied messages (gives time to remove card)
#define MENU_TIMEOUT          30000 // Return to main screen if no activity (30 seconds)

#endif // CONFIG_H
