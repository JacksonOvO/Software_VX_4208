# Software_VX_4208

8-Channel Analog Output Module

## Overview

Industrial control module based on STM32F302CB microcontroller, featuring 8-channel analog output with Modbus RTU communication interface.

## Specifications

| Parameter | Specification |
|-----------|---------------|
| Channels | 8 channels |
| Output Type | Current / Voltage |
| Current Range | 4-20mA, 0-20mA |
| Voltage Range | 0-5V, 0-10V, ±5V, ±10V |
| Resolution | 16-bit |
| Communication | Modbus RTU (RS485) |
| MCU | STM32F302CB |
| Clock | 72MHz |

## System Architecture

```
┌─────────────────────────────────────────┐
│              Application Layer           │
│  ┌──────────┐ ┌──────────┐ ┌────────┐ │
│  │ Common   │ │ Commu-   │ │ Service│ │
│  │          │ │ nication  │ │Handling│ │
│  └──────────┘ └──────────┘ └────────┘ │
├─────────────────────────────────────────┤
│           Board Support Layer (BSP)     │
├─────────────────────────────────────────┤
│    Drivers (SPI, UART, GPIO, Timer)    │
├─────────────────────────────────────────┤
│     STM32F3 Standard Peripherals Lib    │
└─────────────────────────────────────────┘
```

## Directory Structure

```
Software_VX_4208/
├── APP/                    # Application layer
│   ├── Common/            # Common utilities
│   ├── Communication/     # Modbus communication
│   ├── Init/              # System initialization
│   └── ServiceHandling/  # Business logic
├── BSP/                   # Board support package
├── Drivers/               # Peripheral drivers
│   └── StdPeriph/       # STM32F3 HAL
├── PRJ/                   # IAR project files
├── Protocol/              # Modbus RTU stack
└── RTTViewer/            # SEGGER RTT debugger
```

## Key Features

- **8-Channel DAC Output**: AD5755 SPI DAC, 16-bit resolution
- **Modbus RTU**: Standard industrial communication protocol
- **Hardware Watchdog**: System reliability protection
- **LED Status Indication**: Channel status visualization
- **SPI Interface**: High-speed DAC communication

## Hardware Design

### Pin Configuration

| Function | Pin | Description |
|----------|-----|-------------|
| DAC SPI | PA5/PA6/PA7 | SPI1: SCK/MISO/MOSI |
| RS485 TX | PA9 | USART1 TX |
| RS485 RX | PA10 | USART1 RX |
| RS485 DE | PA8 | Driver Enable |
| Channel 1-8 LED | PB0-PB2, PB6-PB9 | Status indicators |
| Error LED | PA0 | Error indicator |

### AD5755 DAC

The module uses AD5755 from Analog Devices as the DAC chip:
- 16-bit resolution
- Programmable output range: 0-5V, 0-10V, ±5V, ±10V, 4-20mA
- Integrated diagnostic functions
- SPI interface up to 50MHz

## Software Design

### Modbus Registers

| Address | Register | Access | Description |
|---------|----------|--------|-------------|
| 0x0000 | AO_CH1 | RW | Analog Output Channel 1 |
| 0x0001 | AO_CH2 | RW | Analog Output Channel 2 |
| ... | ... | ... | ... |
| 0x0007 | AO_CH8 | RW | Analog Output Channel 8 |
| 0x0010 | STATUS | R | Module Status |
| 0x0011 | VERSION | R | Firmware Version |

### Main Flow

```
┌────────────┐
│   Power    │
│    On      │
└─────┬──────┘
      ▼
┌────────────┐
│  System    │
│  Init      │ → Clock, GPIO, SPI, UART, Timer
└─────┬──────┘
      ▼
┌────────────┐
│   Main     │
│   Loop     │ → Modbus → Process → DAC Output
└────────────┘
```

## Build Environment

- **IDE**: IAR Embedded Workbench 9.x
- **Toolchain**: ARM Compiler 6.x
- **StdLib**: STM32F3 Standard Peripherals Library
- **Debugger**: J-Link / ST-Link

## Quick Start

1. Clone the repository
2. Open `PRJ/IAR/Software_VX_4208.eww` in IAR
3. Build and download to target
4. Configure RS485 communication parameters:
   - Baud: 9600/19200/38400/57600/115200
   - Data bits: 8
   - Parity: None/Even/Odd
   - Stop bits: 1/2

## Version History

### v1.0.0
- Date: 2026-03-02
- Author: Jackson
- Features:
  - 8-channel analog output
  - Modbus RTU communication
  - Hardware watchdog protection
  - LED status indication

---

*Project maintained by Jackson*
test 2026-03-13 16:53:22
