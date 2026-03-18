# VX-4204/VX-4208 8-Channel Analog Output Module

> Industrial Modbus RTU Analog Output Module based on STM32F302CB

[![English](https://img.shields.io/badge-English-blue)](docs/README_en.md)
[![中文](https://img.shields.io/badge-中文-green)](docs/README_cn.md)

---

## Quick Overview

| Parameter | Specification |
|-----------|---------------|
| Channels | 8 channels |
| Output Type | Current / Voltage |
| Current Range | 4-20mA, 0-20mA |
| Voltage Range | 0-5V, 0-10V, ±5V, ±10V |
| Resolution | 16-bit |
| Communication | Modbus RTU (RS-485) |
| MCU | STM32F302CB |
| Clock | 72MHz |

## Documentation

- **[中文详细技术文档](docs/README_cn.md)** - 架构分析、启动流程、核心模块、寄存器映射
- **[English Technical Documentation](docs/README_en.md)** - Architecture analysis, startup flow, core modules, register mapping

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

## Key Features

- **8-Channel DAC Output**: AD5755 SPI DAC, 16-bit resolution
- **Modbus RTU**: Standard industrial communication protocol
- **Hardware Watchdog**: System reliability protection
- **LED Status Indication**: Channel status visualization
- **IAP Upgrade**: Firmware update via Modbus

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

## Build Environment

- **IDE**: IAR Embedded Workbench 9.x
- **Toolchain**: ARM Compiler 6.x
- **StdLib**: STM32F3 Standard Peripherals Library
- **Debugger**: J-Link / ST-Link

## Version History

### v2.0.2
- Date: 2025-12-09
- Features: Bug fixes and optimization

### v2.0.1
- Date: 2025-12-06
- Features: Enhanced diagnostics

### v2.0.0
- Date: 2025-12-04
- Features: Initial release

---

*For detailed technical documentation, see [docs/](docs/) folder*
