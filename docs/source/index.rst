.. NFC Access Control System documentation master file

NFC Access Control System Documentation
========================================

Welcome to the documentation for the NFC Access Control System, a comprehensive 
embedded project developed as part of an NFC technology course. Starting with 
fundamental tag reading and writing tasks, this project has evolved into a 
professional-grade access control solution using the PN532 NFC reader with 
advanced card management and cloning capabilities.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   introduction
   examples
   hardware
   installation
   usage
   diagrams
   api
   eeprom
   card_cloning
   troubleshooting
   quickstart

Overview
--------

The NFC Access Control System demonstrates a progressive learning approach to NFC 
technology, from basic operations to a complete embedded system. Built on Arduino 
Nano with the PN532 NFC module, it provides both educational value and practical 
functionality.

Learning Path
^^^^^^^^^^^^^

**Task 1: Reading NFC Tags**
  Master the fundamentals of NFC communication by detecting and reading card UIDs. 
  This forms the foundation for all NFC applications.

**Task 2: Writing to NFC Tags**
  Learn to write data to different card types, understanding memory structures 
  and authentication requirements.

**Extended Project: Full Access Control**
  Integrate reading and writing into a complete system with persistent storage, 
  user interface, and physical access control.

Key Features
------------

Core Functionality
^^^^^^^^^^^^^^^^^^

* **Multi-card Support**: Store up to 40 authorized cards in EEPROM
* **Smart Card Cloning**: Clone any Mifare Classic card using custom sector technology
* **Real-time Access Control**: Instant card verification with relay output
* **Interactive Menu System**: Full-featured LCD menu with button navigation
* **IRQ-based Detection**: Fast, non-blocking card detection using interrupts

Educational Features
^^^^^^^^^^^^^^^^^^^^

* **Progressive Examples**: Simple read/write examples for course tasks
* **Well-Documented Code**: Extensive comments explaining every operation
* **Practical Application**: Real-world system demonstrating concepts
* **Modular Design**: Easy to understand and extend

Advanced Card Management
^^^^^^^^^^^^^^^^^^^^^^^^

* **Custom Sector Technology**: Uses Sector 1 (blocks 4-7) for cloned UID storage
* **Dual UID System**: Supports both physical and cloned UIDs
* **Automatic UID Selection**: Intelligently selects appropriate UID
* **No Magic Cards Required**: Clone to any standard Mifare Classic 1K/4K card

Quick Start
-----------

**For Course Tasks (Beginners)**

1. Start with :doc:`examples` to complete Task 1 (reading) and Task 2 (writing)
2. See :doc:`hardware` for wiring the PN532 module
3. Follow :doc:`installation` to set up PlatformIO
4. Upload and test the basic examples

**For Full System (Advanced)**

1. Review :doc:`introduction` to understand system architecture
2. Complete hardware setup following :doc:`hardware`
3. Build and upload the full application from ``src/``
4. Explore :doc:`usage` for operating the complete system
5. Learn about :doc:`card_cloning` for advanced features

What You'll Learn
-----------------

Hardware Skills
^^^^^^^^^^^^^^^

* Arduino Nano programming and configuration
* SPI and I2C communication protocols
* Interfacing with NFC modules
* LCD display control
* Button input handling
* Relay control for physical outputs

Software Skills
^^^^^^^^^^^^^^^

* Embedded C++ programming
* State machine design
* Non-blocking code patterns
* EEPROM persistent storage
* Menu system implementation
* Error handling and recovery

NFC Technology
^^^^^^^^^^^^^^

* NFC/RFID fundamentals at 13.56 MHz
* Card UID reading and verification
* Memory structures (blocks vs pages)
* Authentication for Mifare Classic
* Writing and verifying data
* Card cloning techniques

Documentation Structure
-----------------------

This documentation is organized to support both learning and reference:

:doc:`introduction`
  Project overview, architecture, and design philosophy. Start here to understand 
  the system's purpose and structure.

:doc:`examples`
  Practical examples for Task 1 (reading) and Task 2 (writing). Essential for 
  course participants and beginners.

:doc:`hardware`
  Component requirements, wiring diagrams, and pin configurations. Follow this 
  for physical setup.

:doc:`installation`
  Software setup, PlatformIO configuration, and build instructions.

:doc:`usage`
  Complete guide to operating the full access control system, including menu 
  navigation and card management.

:doc:`api`
  Technical reference for all classes, methods, and data structures.

:doc:`card_cloning`
  Advanced feature: clone card UIDs without magic cards using custom sector 
  technology.

:doc:`eeprom`
  Deep dive into EEPROM storage structure and card database management.

:doc:`diagrams`
  Visual representations of system architecture, state machines, and workflows.

:doc:`troubleshooting`
  Common issues and solutions for hardware and software problems.

:doc:`quickstart`
  Quick reference for building documentation (for contributors).

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
