<p>
  <img align="left" width="220" 
       src="https://github.com/SourceRabbit/RabbitGRBL/blob/main/Images/rabbit-grbl-logo.png"
       alt="Rabbit GRBL Logo">

  <h1>Rabbit GRBL</h1>
  <strong>GRBL-compatible motion control firmware for ESP32</strong><br><br>
  Deterministic, high-speed and stable motion control firmware,
  designed for real CNC machines and long production jobs.
</p>

<br clear="left"/>


[![](https://dcbadge.limes.pink/api/server/nRKETyjJ7E)](https://discord.gg/nRKETyjJ7E)

## Introduction

Rabbit GRBL is a **GRBL-compatible motion control firmware for ESP32**, originally based on **Grbl_Esp32**.

This repository is maintained to provide a **professional-grade**, fast and predictable firmware that remains  
**100% backward compatible with the original GRBL (AVR)** command set and works with **all standard GRBL G-code senders**.

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
- **User-defined homing sequences**
- **USB / Serial connectivity**
- Optimized for **ESP32 dual-core architecture**


## Compatible Hardware & Software

The following products are **100% compatible** with Rabbit GRBL:

| 4-Axis CNC Motherboard | 4-Axis CNC Motherboard Compact | Focus – PC-Based CNC Control |
|-----------------------|--------------------------------|------------------------------|
| <a href="https://www.sourcerabbit.com/Shop/pr-i-86-t-4-axis-cnc-motherboard.htm"><img src="https://github.com/SourceRabbit/GRBL_ESP32/blob/main/Images/SourceRabbit-4Axis-CNC-Motherboard.png" alt="4-Axis CNC Motherboard"></a> | <a href="https://www.sourcerabbit.com/Shop/pr-i-97-t-4-axis-cnc-motherboard-compact.htm"><img src="https://github.com/SourceRabbit/GRBL_ESP32/blob/main/Images/SourceRabbit-4Axis-CNC-Motherboard-Compact.jpg" alt="4-Axis CNC Motherboard Compact"></a> | <a href="https://www.sourcerabbit.com/Shop/pr-i-91-t-focus-cnc-control-software.htm"><img src="https://github.com/SourceRabbit/GRBL_ESP32/blob/main/Images/FocusGitHub.png" alt="Focus CNC Control Software"></a> |

> Rabbit GRBL is not tied to proprietary hardware and can be adapted to custom ESP32-based CNC controller designs.


## Build & Usage

Build instructions, configuration options and usage examples are documented in the Wiki:

👉 https://github.com/SourceRabbit/Rabbit_GRBL/wiki


## Community & Support

- Discord community  
  https://discord.gg/nRKETyjJ7E

- Official website  
  https://www.sourcerabbit.com
