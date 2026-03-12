# Software_VX_4208

8路模拟量输出模块 (AO Module)

## 产品简介

基于 STM32F302CB 单片机的工业控制模块，支持 8 通道模拟量输出，通过 Modbus RTU 通信协议控制。

## 技术参数

| 参数 | 规格 |
|------|------|
| 通道数 | 8 路 |
| 输出类型 | 电流 / 电压 |
| 电流范围 | 4-20mA, 0-20mA |
| 电压范围 | 0-5V, 0-10V, ±5V, ±10V |
| 分辨率 | 16 位 |
| 通信协议 | Modbus RTU (RS485) |
| 主芯片 | STM32F302CB |

## 目录结构

```
Software_VX_4208/
├── APP/                    # 应用层代码
│   ├── Common/            # 公共模块
│   ├── Communication/     # 通信模块
│   ├── Init/             # 初始化模块
│   └── ServiceHandling/  # 服务处理
├── BSP/                   # 板级支持包
├── Drivers/               # 外设驱动
│   └── StdPeriph/       # 标准外设库
├── PRJ/                   # 工程文件 (IAR)
├── Protocol/              # 通信协议 (Modbus)
└── RTTViewer/            # RTT调试工具
```

## 功能特性

- 8 路模拟量输出 (AO)
- Modbus RTU 通信
- 看门狗保护
- LED 状态指示
- SPI DAC 控制 (AD5755)

## 编译工具

- IAR Embedded Workbench
- STM32F3 Standard Peripherals Library

## 版本历史

### v1.0.0
- 日期: 2026-03-02
- 修改人: Jackson
- 完成内容:
  - 实现基础功能
  - 8路AO输出
  - Modbus通信
  - 看门狗保护
