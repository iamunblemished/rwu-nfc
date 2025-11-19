.. _nfc-reading-report:

=================================================
Report on Data Reading Operations
=================================================

Introduction
------------
Data reading is the fundamental operation of the NFC Access Control System. It serves two primary purposes: **Access Verification** (scanning a card to grant entry) and **Source Identification** (scanning a card to be cloned). The system utilizes a sophisticated "Dual UID" logic, prioritizing data read from a custom sector over the card's physical manufacturer ID.

Hardware Used
-------------
The reading operation relies on the following components:

* **PN532 NFC Module**: Configured in SPI mode for high-speed communication.
* **Arduino Nano (ATmega328P)**: Processes the data and handles the interrupt logic.

Pin Configuration
^^^^^^^^^^^^^^^^^

Default pin assignments are defined in ``include/Config.h``. Modify if needed::

    // PN532 NFC Module pins
    #define PN532_SCK  (13)
    #define PN532_MISO (12)
    #define PN532_MOSI (11)
    #define PN532_SS   (10)
    #define PN532_IRQ  (2)
    #define PN532_RST  (3)

    // LCD Display pins
    #define LCD_RS (4)
    #define LCD_EN (5)
    #define LCD_D4 (6)
    #define LCD_D5 (7)
    #define LCD_D6 (8)
    #define LCD_D7 (9)

    // Button pins
    #define BTN_UP     (A0)
    #define BTN_DOWN   (A1)
    #define BTN_SELECT (A2)
    #define BTN_BACK   (A3)

    // Relay pin
    #define RELAY_PIN (A4)

Software Used
-------------
* **Library**: ``Adafruit_PN532`` for low-level communication.
* **Class**: ``NFCReader`` (defined in ``include/NFCReader.h``) encapsulates the reading logic.

Key Methods
^^^^^^^^^^^

* ``NFCReader::readCard()``: The main orchestrator for detecting and reading data.
* ``NFCReader::readCustomSector()``: Authenticates and reads Sector 1 to check for cloned credentials.
* ``NFCReader::readMifareClassicBlock()``: Authenticates a Mifare Classic Block before reading data

Wiring Diagram
-----------------------
.. graphviz::
   :caption: Hardware Connection Diagram

   digraph hardware_connections {
       rankdir=LR;
       node [shape=record];

       // Arduino
       arduino [label="{Arduino Nano|{<d2>D2 (IRQ)|<d3>D3 (RST)|<d4>D4|<d5>D5|<d6>D6|<d7>D7|<d8>D8|<d9>D9|<d10>D10 (SS)|<d11>D11 (MOSI)|<d12>D12 (MISO)|<d13>D13 (SCK)|<a0>A0|<a1>A1|<a2>A2|<a3>A3|<a4>A4|<5v>5V|<gnd>GND}}", fillcolor=lightgreen, style=filled];

       // PN532
       pn532 [label="{PN532 NFC Module|{<irq>IRQ|<rst>RST|<ss>SS|<mosi>MOSI|<miso>MISO|<sck>SCK|<vcc>VCC|<gnd>GND}}", fillcolor=lightyellow, style=filled];

       // LCD
       lcd [label="{16x2 LCD Display|{<rs>RS|<e>E|<d4>D4|<d5>D5|<d6>D6|<d7>D7|<vdd>VDD|<vss>VSS|<v0>V0}}", fillcolor=lightblue, style=filled];

       // Buttons
       btn_up [label="Button UP", shape=box, fillcolor=orange, style=filled];
       btn_down [label="Button DOWN", shape=box, fillcolor=orange, style=filled];
       btn_select [label="Button SELECT", shape=box, fillcolor=orange, style=filled];
       btn_back [label="Button BACK", shape=box, fillcolor=orange, style=filled];

       // Relay
       relay [label="{Relay Module|{<in>IN|<vcc>VCC|<gnd>GND}}", fillcolor=pink, style=filled];

       // Potentiometer
       pot [label="10kΩ POT\n(Contrast)", shape=ellipse, fillcolor=lightgray, style=filled];

       // Power
       power_5v [label="5V", shape=ellipse, fillcolor=red, style=filled];
       power_gnd [label="GND", shape=ellipse, fillcolor=black, style=filled, fontcolor=white];

       // Connections
       arduino:d2 -> pn532:irq [label="IRQ"];
       arduino:d3 -> pn532:rst [label="RST"];
       arduino:d10 -> pn532:ss [label="SS"];
       arduino:d11 -> pn532:mosi [label="MOSI"];
       arduino:d12 -> pn532:miso [label="MISO"];
       arduino:d13 -> pn532:sck [label="SCK"];

       arduino:d4 -> lcd:rs [label="RS"];
       arduino:d5 -> lcd:e [label="E"];
       arduino:d6 -> lcd:d4 [label="D4"];
       arduino:d7 -> lcd:d5 [label="D5"];
       arduino:d8 -> lcd:d6 [label="D6"];
       arduino:d9 -> lcd:d7 [label="D7"];

       arduino:a0 -> btn_up [label="UP"];
       arduino:a1 -> btn_down [label="DOWN"];
       arduino:a2 -> btn_select [label="SELECT"];
       arduino:a3 -> btn_back [label="BACK"];

       arduino:a4 -> relay:in [label="Control"];

       arduino:5v -> power_5v;
       arduino:gnd -> power_gnd;

       power_5v -> pn532:vcc;
       power_5v -> lcd:vdd;
       power_5v -> relay:vcc;

       power_gnd -> pn532:gnd;
       power_gnd -> lcd:vss;
       power_gnd -> relay:gnd;

       pot -> lcd:v0 [label="Contrast"];
   }


Code Snippets
-------------
The system implements an intelligent ``readCard`` method that automatically prioritizes cloned data.

Reading Strategy (NFCReader.cpp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    // From src/NFCReader.cpp
    NFCCardInfo NFCReader::readCard() {
      // ... (IRQ/Polling logic) ...
      
      // 1. Read Physical UID first
      success = _nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
      
      if (success) {
        // ... (Store Physical UID) ...
        
        // 2. Attempt to read Custom Sector (Sector 1)
        if (info.cardType == NFCCardType::MIFARE_CLASSIC_1K) {
           readCustomSector(info); // Updates info.hasClonedUID if found
        }
      }
      return info;
    }

Custom Sector Reading (NFCReader.cpp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    bool NFCReader::readCustomSector(NFCCardInfo& info) {
      // Authenticate to Block 4 using Default Key A
      if (!authenticateMifareBlock(CUSTOM_BLOCK_UID, DEFAULT_KEY, ...)) {
        return false; 
      }
      // Read data
      if (!_nfc->mifareclassic_ReadDataBlock(CUSTOM_BLOCK_UID, blockData)) {
        return false;
      }
      // Check for Magic Bytes to verify this is a valid clone
      if (blockData[0] == CARD_MAGIC_BYTE1 && blockData[1] == CARD_MAGIC_BYTE2) {
        info.hasClonedUID = true;
        // Extract the "virtual" UID
        memcpy(info.clonedUID, &blockData[3], info.clonedUIDLength);
        return true;
      }
      return false;
    }

Mifare Classic Reading (NFCReader.cpp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    bool NFCReader::readMifareClassicBlock(uint8_t block, uint8_t* buffer, const uint8_t* key, bool useKeyB) {
      if (!_nfc || !_lastCardInfo.detected) return false;
      
      // Authenticate first
      if (!authenticateMifareBlock(block, key, useKeyB, _lastCardInfo.uid, _lastCardInfo.uidLength)) {
        return false;
      }
      
      // Read block
      return _nfc->mifareclassic_ReadDataBlock(block, buffer);
    }

The NFCReader class can be instantiated and used as follows:

.. code-block:: cpp

    #include <NFCReader.h>
    
    NFCReader reader(NFCCommMode::SPI, NFCReadMode::IRQ);
    uint8_t uid[7];
    uint8_t uidLength;
    
    void setup() {
        Serial.begin(115200);
        if (!reader.begin()) {
            Serial.println("NFC reader initialization failed!");
            while(1);
        }
    }
    
    void loop() {
        if (reader.readCard(uid, &uidLength)) {
            Serial.print("Card UID: ");
            for (uint8_t i = 0; i < uidLength; i++) {
                Serial.print(uid[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
        delay(100);
    }

Results
-------
When a card is presented:

1. **Detection**: The system detects the card within 100ms using IRQ mode.
2. **Identification**: 
   
   * If the card is a standard tag, the system returns the **Physical UID** (e.g., ``DE AD BE EF``).
   * If the card has been written to by this system previously, it returns the **Cloned UID** (e.g., ``AB 12 CD 34``) stored in Sector 1.

3. **Access**: The retrieved UID is compared against the EEPROM database to grant or deny access.

Limitations
-----------
* **Distance**: The card must be within 4 cm of the reader.
* **Card Types**: The "Dual UID" reading feature (reading cloned data from Sector 1) only works on **Mifare Classic** cards. NTAG/Ultralight cards will only yield their physical UID.
* **Security**: The system reads Sector 1 using default keys (``0xFF...``). If a card uses custom encryption keys for Sector 1, the "Cloned UID" read will fail, and the system will default to the Physical UID.

Conclusion
----------
The reading mechanism is robust and non-blocking, suitable for real-time access control. By implementing a priority read logic (Custom Sector > Manufacturer Block), the system allows inexpensive standard cards to function as "clones" of other credentials without needing special hardware.
