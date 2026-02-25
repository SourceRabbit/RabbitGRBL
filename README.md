# Rabbit GRBL 
<p>
  <img align="left" width="220" src="https://github.com/SourceRabbit/RabbitGRBL/blob/main/Images/rabbit-grbl-logo.png"alt="Rabbit GRBL Logo">
<strong>GRBL-compatible motion control firmware for ESP32</strong><br><br>
Deterministic, high-speed and stable motion control firmware,
designed for real CNC machines and long production jobs,
with predictable behavior, precise timing and reliable multi-axis control,
engineered for continuous operation, low latency, accuracy, robustness, scalability, safety.
</p>

<br clear="left"/>

[![](https://dcbadge.limes.pink/api/server/nRKETyjJ7E)](https://discord.gg/nRKETyjJ7E)

## Introduction

Rabbit GRBL is a **GRBL-compatible motion control firmware for ESP32**, originally developed as a fork of **Grbl_Esp32**.

This repository is maintained to provide a **professional-grade**, fast and predictable firmware that remains  
**100% backward compatible with the original Grbl v1.1h for AVR processors** command set and works with **all standard GRBL G-code senders**.

Rabbit GRBL is optimized for **stable timing and low jitter**, achieving **up to 120 kHz step pulse rates** on ESP32 hardware,
making it suitable for real CNC machines — not experimental setups.

## Design Goals

- Deterministic motion control
- Stable behavior under continuous load
- Full GRBL compatibility (no surprises)
- Predictable execution for production use

## Features

- Control of **up to 6 axes**
- Step rates up to **120,000 steps/sec**
- **Jitter-free** pulse generation
- **Backlash compensation**
- **Canned Cycles** [G73](https://github.com/SourceRabbit/RabbitGRBL/wiki/G%E2%80%90Code-Commands#g73--peck-drilling-chip-breaking), [G81](https://github.com/SourceRabbit/RabbitGRBL/wiki/G%E2%80%90Code-Commands#g81--drilling-canned-cycle), [G83](https://github.com/SourceRabbit/RabbitGRBL/wiki/G%E2%80%90Code-Commands#g83--peck-drilling-chip-removing)
- **User-defined homing sequences**
- **USB / Serial connectivity**
- Optimized for **ESP32 dual-core architecture**


## Compatible Hardware & Software

Rabbit GRBL runs on **ESP32-based CNC controller boards** and remains fully compatible with the standard GRBL ecosystem.

You can use it with supported ESP32 hardware as described in the official hardware compatibility list:

👉 Compatible Hardware<br>
https://github.com/SourceRabbit/RabbitGRBL/wiki/Compatible-Hardware  

On the software side, Rabbit GRBL works with **all standard GRBL G-code senders**, without requiring proprietary tools.  
You can find the full list of compatible software here:

👉 Compatible Software/G-Code Senders<br>
https://github.com/SourceRabbit/RabbitGRBL/wiki/Compatible-GCode-Senders

Because Rabbit GRBL is **100% backward compatible with the original GRBL (AVR)** command set, it integrates seamlessly with existing GRBL-based workflows.

## Build & Usage

Build instructions, configuration options and usage examples are documented in the Wiki:

👉 https://github.com/SourceRabbit/Rabbit_GRBL/wiki


## Community & Support

- Discord community  
  https://discord.gg/nRKETyjJ7E

- Official website  
  https://www.sourcerabbit.com
