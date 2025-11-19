# NFC Access Control System

> **An embedded NFC project progressing from basic tag operations to a complete access control system with advanced card cloning technology.**

[![Platform](https://img.shields.io/badge/Platform-Arduino-blue.svg)](https://www.arduino.cc/)
[![Framework](https://img.shields.io/badge/Framework-PlatformIO-orange.svg)](https://platformio.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/Docs-Sphinx-brightgreen.svg)](https://menazaa.github.io/rwu-nfc/)

## 📖 Description

The NFC Access Control System is a comprehensive embedded project developed as part of an NFC technology course. What started as two fundamental tasks—reading and writing NFC tags—has evolved into a professional-grade access control solution built on the Arduino platform with the industry-standard PN532 NFC module.

### Project Evolution

This project follows a structured learning progression:

- **📖 Task 1: Tag Reading** - Learn NFC fundamentals by reading card UIDs and identifying different card types
- **✍️ Task 2: Tag Writing** - Build on Task 1 by writing data to NFC cards with different memory structures
- **🔒 Extended Project** - Full access control system integrating both tasks with card management, persistent storage, LCD interface, and physical access control

The result combines educational value with practical functionality, suitable for both learning and real-world applications.

### What Makes This Project Unique?

Unlike traditional access control systems, this project implements **custom sector card cloning technology** that works with standard Mifare Classic cards—no expensive "magic cards" required. The system intelligently manages both physical and cloned UIDs, providing flexible access control without the limitations of conventional NFC systems.

### Key Highlights

- 🎓 **Educational**: Progressive learning from simple read/write to complete system
- 📚 **Well-Documented**: Comprehensive documentation with practical examples
- 🔒 **Secure**: Local card database with EEPROM persistence, no network dependencies
- ⚡ **Fast**: IRQ-based card detection with <100ms response time
- 🎯 **Smart**: Automatic UID selection (cloned vs. physical)
- 🖥️ **User-Friendly**: Interactive LCD menu with intuitive button navigation
- 🔧 **Flexible**: Support for up to 40 authorized cards with easy management
- 🛠️ **Professional**: Built with PlatformIO for modern embedded development

### Perfect For

- **Learning NFC Technology**: Complete Tasks 1 & 2 with provided examples
- **Embedded Systems Courses**: Demonstrate progressive system development
- **Laboratory Security**: Manage access to restricted areas in research facilities
- **Maker Spaces**: Control access to shared equipment and facilities
- **Office Access**: Small office door control with card management
- **Prototype Development**: Foundation for commercial access control products

## 🚀 Quick Start

### For Course Tasks (Beginners)

If you're completing the NFC course tasks, you can build and flash the examples directly without overwriting the main system code:

**Method 1: Using VS Code PlatformIO Extension (Easiest)**

1. Open project in VS Code with PlatformIO extension installed
2. Click on PlatformIO icon in left sidebar
3. Expand "task1_read" or "task2_write" environment
4. Click "Upload" button
5. Open Serial Monitor (baud: 115200)

**Method 2: Using Command Line**

If `pio` command is in your PATH:

**Task 1: Reading NFC Tags**

```bash
# Build and upload the read example
pio run -e task1_read --target upload

# Monitor serial output
pio device monitor --baud 115200
```

**Task 2: Writing to NFC Tags**

```bash
# Build and upload the write example
pio run -e task2_write --target upload

# Monitor serial output
pio device monitor --baud 115200
```

**Method 3: If `pio` is not in PATH**

Windows:
```powershell
# Use full path to platformio.exe
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e task1_read --target upload

# Or use Python module
python -m platformio run -e task1_read --target upload
```

Linux/macOS:
```bash
# Use full path
~/.platformio/penv/bin/platformio run -e task1_read --target upload

# Or use Python module
python3 -m platformio run -e task1_read --target upload
```

**Specifying COM Port (if needed):**

```bash
# Windows
pio run -e task1_read --target upload --upload-port COM13

# Linux/macOS
pio run -e task1_read --target upload --upload-port /dev/ttyUSB0
```

**Finding Your COM/Serial Port:**

```bash
# List available ports
pio device list

# Windows: Look for COM3, COM13, etc.
# Linux: Look for /dev/ttyUSB0, /dev/ttyACM0
# macOS: Look for /dev/tty.usbserial-*
```

> **Note:** The examples are built directly from the `examples/` directory. Your main system code in `src/main.cpp` remains untouched.

See `examples/` folder for detailed commented examples covering both tasks.

### For Full System (Advanced)

The main access control system with LCD, buttons, and full menu:

```bash
# Build and upload full system (default environment)
pio run --target upload

# Or specify the environment explicitly
pio run -e nanoatmega328 --target upload

# Monitor
pio device monitor --baud 115200
```

## 🔧 Available Build Environments

The project includes three PlatformIO environments:

| Environment | Command | Description |
|-------------|---------|-------------|
| `nanoatmega328` | `pio run --target upload` | Full access control system (default) |
| `task1_read` | `pio run -e task1_read --target upload` | Task 1: NFC tag reading example |
| `task2_write` | `pio run -e task2_write --target upload` | Task 2: NFC tag writing example |

**Why use environments?**
- No need to copy/overwrite files
- Keep main system intact while testing examples
- Easy switching between full system and examples
- Each environment includes only necessary dependencies

### Changing Default Environment

The PlatformIO upload button builds all environments by default. To change which environment is built:

1. **Edit platformio.ini** - Change the `default_envs` setting:
   ```ini
   [platformio]
   default_envs = task2_write  # Change to: nanoatmega328, task1_read, or task2_write
   ```

2. **Use VS Code Tasks** - Run specific tasks from Command Palette (Ctrl+Shift+P):
   - `Tasks: Run Task` → `PlatformIO: Upload Task 1 (Read)`
   - `Tasks: Run Task` → `PlatformIO: Upload Task 2 (Write)`
   - `Tasks: Run Task` → `PlatformIO: Upload Full System`
   - `Tasks: Run Task` → `PlatformIO: Monitor Task 1`

3. **Use Terminal** - Specify environment explicitly:
   ```bash
   # Using Python module (works without PlatformIO in PATH)
   python -m platformio run -e task2_write --target upload
   python -m platformio device monitor --baud 115200
   
   # Using pio command (if in PATH)
   pio run -e task2_write --target upload
   pio device monitor --baud 115200
   ```

### Prerequisites
- Arduino Nano or compatible board
- PN532 NFC module
- 16x2 LCD display (for full system only)
- 4 push buttons (for full system only)
- Relay module (for full system only)
- **PlatformIO IDE** (VS Code extension) or **PlatformIO Core** (CLI)

### Installing PlatformIO

**Option 1: VS Code Extension (Recommended for Beginners)**

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code
3. Go to Extensions (Ctrl+Shift+X)
4. Search for "PlatformIO IDE"
5. Click Install
6. Reload VS Code
7. Open this project folder
8. Use the PlatformIO toolbar at the bottom

**Option 2: PlatformIO Core (CLI)**

1. Install Python 3.6+ from [python.org](https://www.python.org/)
2. Install PlatformIO Core:
   
   ```bash
   pip install platformio
   ```

3. Add to PATH (if not automatic):
   
   Windows PowerShell:
   ```powershell
   # Add PlatformIO to current session
   $env:Path += ";$env:USERPROFILE\.platformio\penv\Scripts"
   
   # Or permanently (requires admin)
   [Environment]::SetEnvironmentVariable("Path", $env:Path + ";$env:USERPROFILE\.platformio\penv\Scripts", "User")
   ```
   
   Linux/macOS:
   ```bash
   # Add to ~/.bashrc or ~/.zshrc
   export PATH=$PATH:~/.platformio/penv/bin
   ```

4. Verify installation:
   ```bash
   pio --version
   ```

### First Use (Full System)

1. **Power up the system** - LCD displays "NFC Access Control"
2. **Press SELECT** to enter menu
3. **Select "Register Card"** 
4. **Scan your first card** - It's now authorized
5. **Test access** - Scan the card to unlock the relay

## 📑 Table of Contents

- [Description](#-description)
- [Quick Start](#-quick-start)
- [Learning Path](#-learning-path)
- [Examples](#-examples)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Setup](#software-setup)
- [How It Works](#how-it-works)
- [Menu System](#menu-system)
- [Configuration](#configuration)
- [Documentation](#-documentation)
- [Troubleshooting](#troubleshooting)
- [Contributing](#-contributing)
- [License & Credits](#license--credits)

## 🎓 Learning Path

### Task 1: Reading NFC Tags

**Location**: `examples/read_example.cpp`

**What you'll learn:**
- Initialize and configure the PN532 NFC reader
- Detect when a card enters the reader field
- Read card UID (Unique Identifier)
- Identify different card types (Mifare Classic, NTAG, etc.)
- Display and process card information

**Key concepts:**
- NFC communication protocols
- SPI vs I2C modes
- Polling vs IRQ reading modes
- Card UID structure

### Task 2: Writing to NFC Tags

**Location**: `examples/write_example.cpp`

**What you'll learn:**
- Write text strings to NFC cards
- Write binary data to specific addresses
- Understand block-based (Mifare Classic) vs page-based (NTAG) storage
- Verify written data
- Handle authentication for Mifare Classic

**Key concepts:**
- Memory structures and safe write areas
- Authentication with default keys
- Data verification
- Avoiding card damage

### Extended Project: Full Access Control System

**Location**: `src/`

**What you'll learn:**
- State machine design
- EEPROM persistent storage
- LCD user interface
- Button input handling
- Relay control
- Advanced card cloning

## 📝 Examples

The `examples/` directory contains standalone examples for each course task:

### read_example.cpp
- Basic NFC tag reading
- Card type detection
- UID display
- Both I2C and SPI modes
- Polling and IRQ modes

### write_example.cpp
- Writing strings and binary data
- Auto-detect card type writing
- Type-specific operations (NTAG vs Mifare)
- Write verification
- Safe memory areas

## 🛠️ Technology Stack

| Component | Technology | Purpose |
|-----------|-----------|---------|
| **Microcontroller** | ATmega328P (Arduino Nano) | Main processing unit |
| **NFC Module** | PN532 @ 13.56MHz | Card reading/writing |
| **Communication** | SPI + IRQ | Fast, non-blocking I/O |
| **Display** | HD44780 (16x2 LCD) | User interface (full system) |
| **Storage** | EEPROM (1KB) | Persistent card database |
| **Build System** | PlatformIO | Modern embedded development |
| **Language** | C++11 | Firmware programming |
| **Documentation** | Sphinx + Graphviz | Technical documentation |
| **Libraries** | Adafruit PN532, LiquidCrystal | Hardware abstraction |

## Features

### Educational Features (Course Tasks)
- **📖 Simple Examples**: Standalone read and write examples in `examples/` folder
- **📝 Well-Commented Code**: Extensive documentation within code
- **🎓 Progressive Learning**: Build from simple to complex
- **🔍 Multiple Modes**: Learn I2C vs SPI, Polling vs IRQ

### Core Functionality (Full System)
- **Multi-card Support**: Store up to 40 authorized cards in EEPROM
- **Smart Card Cloning**: Clone any Mifare Classic card to any other card using custom sector
- **Real-time Access Control**: Instant card verification with relay output
- **Interactive Menu System**: Full-featured LCD menu with button navigation
- **IRQ-based Detection**: Fast, non-blocking card detection using interrupts

### Advanced Card Management
- **Custom Sector Technology**: Uses Sector 1 (blocks 4-7) for cloned UID storage
- **Dual UID System**: 
  - Physical UID (read-only manufacturer block)
  - Cloned UID (writable custom sector)
- **Automatic UID Selection**: Uses cloned UID if present, falls back to physical UID
- **No Magic Cards Required**: Clone to any standard Mifare Classic 1K/4K card

## Hardware Requirements

### Minimal Setup (For Course Tasks 1 & 2)
- **Microcontroller**: Arduino Nano (ATmega328P)
- **NFC Module**: PN532 (SPI or I2C mode)
- **Connection**: USB cable for programming and serial monitor

### Full System Components
- All minimal setup components, plus:
- **Display**: 16x2 LCD (4-bit parallel mode)
- **Buttons**: 4 push buttons (UP, DOWN, SELECT, BACK)
- **Output**: Relay module for door lock control
- **Power**: 5V DC power supply

### Pin Configuration

#### PN532 NFC Module (SPI - Recommended)
| Pin | Arduino | Description |
|-----|---------|-------------|
| SCK | D13 | SPI Clock |
| MISO | D12 | SPI MISO |
| MOSI | D11 | SPI MOSI |
| SS | D10 | SPI Chip Select |
| IRQ | D2 | Interrupt Request (optional) |
| RST | D3 | Reset |

#### LCD Display (4-bit Parallel) - Full System Only
| Pin | Arduino | Description |
|-----|---------|-------------|
| RS | D4 | Register Select |
| EN | D5 | Enable |
| D4 | D6 | Data 4 |
| D5 | D7 | Data 5 |
| D6 | D8 | Data 6 |
| D7 | D9 | Data 7 |

#### Buttons & Output - Full System Only
| Component | Arduino | Type |
|-----------|---------|------|
| BTN_UP | A0 | Active LOW, internal pullup |
| BTN_DOWN | A1 | Active LOW, internal pullup |
| BTN_SELECT | A2 | Active LOW, internal pullup |
| BTN_BACK | A3 | Active LOW, internal pullup |
| RELAY | A6 | Configurable HIGH/LOW |

> **Note**: For course tasks (examples), only the Arduino and PN532 module are needed.

## Software Setup

### PlatformIO Installation
1. Install [VS Code](https://code.visualstudio.com/)
2. Install PlatformIO extension
3. Clone this repository
4. Open folder in VS Code
5. PlatformIO will auto-install dependencies

### Running Course Examples
```bash
# Task 1: Read NFC Tags
cp examples/read_example.cpp src/main.cpp
pio run --target upload

# Open serial monitor to view output
pio device monitor --baud 115200

# Task 2: Write to NFC Tags
cp examples/write_example.cpp src/main.cpp
pio run --target upload

# Open serial monitor to view output
pio device monitor --baud 115200
```

### Running Full System
```bash
# The main application is in src/ by default
# Just build and upload (requires all hardware components)
pio run --target upload --upload-port COM13
pio device monitor --baud 115200
```

### Build & Upload
```bash
# Build firmware (uses default environment in platformio.ini)
pio run

# Upload specific environment
pio run -e task2_write --target upload --upload-port COM13

# Open serial monitor (115200 baud required)
pio device monitor --baud 115200

# Or use Python module if pio not in PATH
python -m platformio run -e task2_write --target upload
python -m platformio device monitor --baud 115200

# All-in-one: upload and monitor
python -m platformio run -e task2_write --target upload && python -m platformio device monitor --baud 115200
```

### Dependencies
- Adafruit PN532 v1.3.4
- LiquidCrystal v1.0.7 (full system only)
- SPI (built-in)
- Wire (built-in)
- EEPROM (built-in)

## How It Works

### Basic Operation (Course Tasks)

**Task 1 - Reading:**
1. Initialize PN532 module
2. Wait for card in reader field
3. Read card UID
4. Identify card type
5. Display information via serial

**Task 2 - Writing:**
1. Read card to detect type
2. Write data to safe memory areas
3. Verify written data
4. Display success/failure via serial

### Advanced Operation (Full System)

#### Card Detection & Custom Sector
When a card is scanned:
1. **Read Physical UID** from block 0 (manufacturer, read-only)
2. **Read Custom Sector 1** (blocks 4-7) for cloned UID
3. **Use Effective UID** for access control:
   - If custom sector has cloned UID → use cloned UID
   - Otherwise → use physical manufacturer UID

#### Custom Sector Structure (Block 4)
```
Byte 0-1:   Magic bytes (0xAC, 0xDB) - Identifies initialized card
Byte 2:     UID length (0-7)
Byte 3-9:   Cloned UID (up to 7 bytes)
Byte 10-15: Reserved for future use
```

### Card Cloning Process
1. **Select Clone from Menu**
2. **Scan Source Card**
   - System reads effective UID (cloned if present, otherwise physical)
   - Display shows "Remove & scan new"
3. **Remove Source Card**
   - Automatic timeout (1 second) detects removal
   - IRQ detection restarts for next card
4. **Scan Target Card** (must be Mifare Classic 1K/4K)
   - System writes source UID to target's Sector 1
   - Verifies write operation
   - Target now acts as source card

### Access Control Logic
```cpp
// Card scanned
if (hasClonedUID) {
    checkUID = clonedUID;  // From Sector 1
} else {
    checkUID = physicalUID;  // From block 0
}

if (isAuthorized(checkUID)) {
    grantAccess();
    unlockDoor(3000ms);
}
```

## Menu System

### Main Menu (Long-press SELECT)
- **Register Card**: Add new card to authorized list
- **Delete Card**: Remove card from authorized list
- **List Cards**: Browse stored cards (UP/DOWN to navigate)
- **Clone Card**: Copy UID from one card to another
- **Settings**: System configuration
- **Clear All**: Remove all authorized cards
- **Exit Menu**: Return to scan mode

### Navigation
- **UP/DOWN**: Navigate menu items or list cards
- **SELECT**: Confirm selection
- **BACK**: Return to previous screen/exit menu
- **LONG PRESS SELECT**: Open menu from main screen

## Serial Monitor Output

### Normal Card Scan
```
Physical UID: 12 34 56 78
Access GRANTED
```

### Cloned Card Scan
```
Physical UID: AA BB CC DD
Cloned UID: 12 34 56 78 (from Sector 1)
Access GRANTED
```

### Clone Operation
```
Clone source UID: 12 34 56 78
(Using physical manufacturer UID)
Remove source card and scan target card...
Target card detected, cloning to custom sector...
Cloning UID to sector 1: 12 34 56 78
Writing cloned UID to block 4: 12 34 56 78
Clone write SUCCESS
Clone VERIFIED!
NFC reader reset, ready for next card
```

## Configuration

Edit `include/Config.h` to customize:

```cpp
// Access Control
#define RELAY_ACTIVE_HIGH  true    // Relay trigger level
#define DOOR_UNLOCK_TIME   3000    // Door unlock duration (ms)
#define MAX_STORED_CARDS   40      // Maximum authorized cards

// Timing
#define MESSAGE_DISPLAY_TIME  2000  // Status message duration
#define MENU_TIMEOUT          30000 // Auto-exit menu timeout
#define CARD_TIMEOUT          1000  // Card removal detection

// NFC Mode
#define NFC_COMM_MODE  NFC_COMM_SPI     // SPI or I2C
#define NFC_READ_MODE  NFC_READ_IRQ     // IRQ or POLLING
```

## EEPROM Storage

### Memory Map
| Address | Content | Size |
|---------|---------|------|
| 0-1 | Magic number (0xABCD) | 2 bytes |
| 2 | Card count | 1 byte |
| 4+ | Card storage | 9 bytes/card |

### Card Storage Format (per card)
```
Byte 0:   UID length
Byte 1:   Active flag
Byte 2-8: UID data (7 bytes max)
```

## Troubleshooting

### Common Issues (Course Tasks)

#### Card Not Detected (Examples)
- Check PN532 wiring (especially SPI pins: D10-D13)
- Verify power connections (3.3V or 5V depending on module)
- Check Serial Monitor for "PN532 NOT FOUND!" error
- Ensure card is within 4cm of reader
- Try different card type (Mifare Classic, NTAG)

#### Write Failed (write_example.cpp)
- **Target must be writable** (not all cards support write)
- For Mifare Classic: verify default key (FF FF FF FF FF FF)
- **Block 0 is read-only** - write attempts will fail but won't damage card
- **NEVER write to sector trailers** (blocks 3, 7, 11, 15, etc.) - will lock the sector
- Hold card steady during entire write operation

#### RAM Warning ("data size greater than maximum")
- **Expected for write example** - Uses verbose output for educational purposes
- Code will still function but with reduced buffer overflow protection
- Already optimized with `-Os` flag and reduced serial buffers in platformio.ini
- Production code would use much less RAM (fewer debug strings)
- Arduino Nano has only 2KB RAM total

#### Serial Monitor Shows Garbage
- Set baud rate to 115200
- Check correct COM port selected
- Verify USB cable supports data (not charge-only)

### Full System Issues

#### Display Issues
- Check LCD contrast (potentiometer adjustment)
- Verify 4-bit parallel wiring (D4-D9)
- Check power supply (5V, stable)
- Verify LCD is HD44780-compatible

#### Buttons Not Responding
- Verify active LOW configuration
- Check internal pullups enabled (set in code)
- Test button continuity with multimeter
- Adjust debounce time in Config.h if needed

#### Clone Not Working
- **Target must be Mifare Classic 1K or 4K**
- NTAG/Ultralight cards not supported for clone target
- Check authentication (default key: FF FF FF FF FF FF)
- Verify custom sector (Sector 1) is writable
- Don't move card during write operation

### General Troubleshooting

#### Reading Stops After First Card
- Remove card completely before scanning again
- In examples: Call `nfcReader.resetCardState()` after processing
- Wait for "Ready for next card" message
- Check Serial Monitor for card removal detection

#### Memory Issues
- Close unused serial monitors
- Reset Arduino if behavior becomes erratic
- Check for memory leaks in custom code

## Technical Details

### Non-Blocking Architecture (Full System)
- All operations use `millis()` for timing
- No `delay()` calls in main loop (except examples)
- IRQ-driven card detection for fast response
- Maintains responsive button handling

### Memory Optimization
- Flash: ~20KB / 30KB (66%)
- RAM: ~1.1KB / 2KB (55%)
- PROGMEM used for strings
- Efficient EEPROM usage

### Security Considerations
- **For educational purposes, not high-security applications**
- Default Mifare key used (FF FF FF FF FF FF)
- Physical UID remains untouched
- Custom sector can be overwritten
- Not suitable for cryptographic security requirements
- Designed for access convenience and learning

## Project Structure

```
rwu-nfc/
├── examples/              # Course task examples
│   ├── read_example.cpp   # Task 1: Reading NFC tags
│   └── write_example.cpp  # Task 2: Writing to NFC tags
├── include/
│   ├── AccessControlSystem.h  # Main system class (full system)
│   ├── NFCReader.h            # NFC read/write operations
│   └── Config.h               # Hardware configuration
├── src/
│   ├── AccessControlSystem.cpp  # Full system implementation
│   ├── NFCReader.cpp            # NFC functionality
│   └── main.cpp                 # Main application (full system)
├── docs/                  # Sphinx documentation
│   ├── source/
│   │   ├── examples.rst   # Task examples documentation
│   │   ├── api.rst        # API reference
│   │   └── ...
│   └── build/             # Generated documentation
├── platformio.ini         # Build configuration
└── README.md
```

## 📚 Documentation

Comprehensive documentation is available in the `docs/` directory and online:

**[📖 Full Documentation](https://menazaa.github.io/rwu-nfc/)**

### Key Documentation Pages

- **[Introduction](docs/source/introduction.rst)** - Project overview, architecture, and learning path
- **[Examples](docs/source/examples.rst)** - **START HERE for Tasks 1 & 2** - Detailed examples with code
- **[Hardware Setup](docs/source/hardware.rst)** - Wiring diagrams and component details
- **[Installation](docs/source/installation.rst)** - Development environment setup
- **[Usage Guide](docs/source/usage.rst)** - Full system operation and menu navigation
- **[API Reference](docs/source/api.rst)** - Class documentation and function reference
- **[Card Cloning](docs/source/card_cloning.rst)** - Advanced cloning technology details
- **[Troubleshooting](docs/source/troubleshooting.rst)** - Common issues and solutions

### Building Documentation Locally

To build the documentation from source:

```bash
cd docs
pip install -r requirements.txt

# Windows
.\make.bat html

# Linux/macOS
make html
```

Open `docs/build/html/index.html` in your browser.

For PDF output:
```bash
# Windows
.\make.bat latexpdf

# Linux/macOS
make latexpdf
```

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. **Report Bugs**: Open an issue with detailed reproduction steps
2. **Suggest Features**: Share your ideas for improvements
3. **Submit Pull Requests**: Fork, implement, and submit PRs
4. **Improve Documentation**: Fix typos, add examples, or clarify instructions
5. **Share Use Cases**: Tell us how you're using this project

### Development Guidelines

- Follow existing code style and conventions
- Test thoroughly before submitting PRs
- Update documentation for new features
- Add comments for complex logic
- Keep commits focused and descriptive

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🔒 Security Considerations

**Important**: This system is designed for educational and convenience purposes. For production deployments:

- ✅ Use encrypted card data storage
- ✅ Implement access logging and audit trails
- ✅ Add physical tamper detection
- ✅ Use secure communication protocols for remote management
- ✅ Regular security audits and updates
- ✅ Consider upgrading to DESFire or other secure card types

The default Mifare Classic key (FF FF FF FF FF FF) is used for demonstration purposes. In production, use custom keys.

## 📊 Project Status

- ✅ **Task 1**: Basic NFC tag reading (examples/read_example.cpp)
- ✅ **Task 2**: NFC tag writing (examples/write_example.cpp)
- ✅ Core access control functionality
- ✅ Card registration and deletion
- ✅ Advanced card cloning with custom sectors
- ✅ IRQ-based fast card detection
- ✅ Interactive LCD menu system
- ✅ Comprehensive Sphinx documentation with diagrams
- 🚧 Network connectivity (future enhancement)
- 🚧 Mobile app integration (future enhancement)
- 🚧 Time-based access rules (future enhancement)

## 🌟 Acknowledgments

- **RWU NFC Course** for the project foundation and learning structure
- **Adafruit** for the excellent PN532 library
- **PlatformIO** for modern embedded development tools
- **Arduino Community** for extensive resources and support
- **Sphinx** for documentation generation

## 📧 Contact & Support

- **Project Repository**: [github.com/Menazaa/rwu-nfc](https://github.com/Menazaa/rwu-nfc)
- **Documentation**: [menazaa.github.io/rwu-nfc](https://menazaa.github.io/rwu-nfc/)
- **Issues**: [github.com/Menazaa/rwu-nfc/issues](https://github.com/Menazaa/rwu-nfc/issues)

---

**Made with ❤️ for the NFC learning community**

For issues and questions:
1. Check Serial Monitor output (115200 baud)
2. Verify hardware connections against wiring diagram
3. Review [Troubleshooting Documentation](https://menazaa.github.io/rwu-nfc/troubleshooting.html)
4. Check card compatibility (Mifare Classic 1K/4K required for cloning)
5. Open an issue on GitHub with detailed information

---

**⭐ If you find this project useful, please consider giving it a star!**

*Made with ❤️ for the maker community*
