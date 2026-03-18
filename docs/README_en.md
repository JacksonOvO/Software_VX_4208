# VX-4204/VX-4208 8-Channel Analog Output Module - Technical Documentation

> **Project**: Software_VX_4208-master  
> **MCU**: STM32F302CB (72MHz)  
> **Last Updated**: 2026-03-18  
> **Analyzer**: 小瓦 (Xiao Wa)

---

[![English](https://img.shields.io/badge-English-blue)](README_en.md)
[![中文](https://img.shields.io/badge-中文-green)](README_cn.md)

---

## 📋 Chapter 1: Project Overview

### 1.1 What Is This Product?

This is an **industrial-grade 8-channel analog output module** based on STM32F302CB microcontroller:

| Core Function | Description |
|---------------|-------------|
| **8-Channel AO** | Via AD5755 DAC chip |
| **Output Types** | Voltage (0-5V, 0-10V, ±5V, ±10V) / Current (4-20mA, 0-20mA) |
| **Communication** | Modbus RTU (RS-485), address assignable |
| **Resolution** | 16-bit |
| **Protection** | Hardware watchdog, LED status, over-temp/voltage protection |

### 1.2 Why This Design?

```
┌─────────────────────────────────────────────────────────────────┐
│                         Design Goals                            │
├─────────────────────────────────────────────────────────────────┤
│  1. Industrial Reliability → HW Watchdog + Wide Temp Range     │
│  2. Flexible Configuration → Modbus-configurable output type  │
│  3. Easy Maintenance → LED status, fault diagnosis            │
│  4. Field Upgrade → IAP bootloader support                     │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📂 Chapter 2: Directory Structure Analysis

```
Software_VX_4208-master/
├── APP/                          # Application Layer
│   ├── main.c                    # Main entry point
│   ├── Common/                   # Common modules
│   │   ├── inc/                 # Headers
│   │   │   ├── common.h         # Common definitions
│   │   │   ├── diagnose.h       # Diagnosis interface
│   │   │   ├── hardware.h       # Pin definitions
│   │   │   ├── warn.h           # Alarm definitions
│   │   │   └── watchdog.h       # Watchdog interface
│   │   ├── opt/                # Optional config
│   │   │   └── IOOptions.h      # IO type codes, Flash addresses
│   │   └── src/                # Implementations
│   │       ├── common.c
│   │       ├── diagnose.c       # MCU temp/voltage monitoring
│   │       ├── warn.c
│   │       └── watchdog.c       # HW watchdog driver
│   │
│   ├── Communication/           # Communication module
│   │   ├── inc/
│   │   │   └── MbCommunication.h
│   │   └── src/
│   │       └── MbCommunication.c    # Modbus register callbacks
│   │
│   ├── Init/                    # System initialization
│   │   ├── inc/
│   │   │   └── Ioinit.h
│   │   └── src/
│   │       └── IoInit.c         # Core initialization logic ⭐
│   │
│   └── ServiceHandling/         # Business logic
│       ├── inc/
│       │   ├── Iocontrol.h
│       │   ├── MasterCommunication.h
│       │   └── HModbusOptimize.h
│       └── src/
│           ├── Iocontrol.c          # AO data processing, LED control
│           ├── MasterCommunication.c
│           └── HModbusOptimize.c
│
├── Drivers/                     # Peripheral Driver Layer
│   ├── inc/
│   │   ├── STM32F0usart.h      # UART driver
│   │   ├── STM32F0SPIDrive.h   # SPI driver
│   │   ├── STM32DAC8555driver.h # DAC driver (AD5755)
│   │   ├── STM32F0Ads8331Driver.h
│   │   └── STM32GPIODriver.h   # GPIO driver
│   └── src/
│       ├── STM32F0usart.c       # UART+DMA implementation
│       ├── STM32F0SPIDrive.c    # SPI+DMA implementation
│       ├── STM32DAC8555Driver.c # AD5755 complete driver ⭐
│       └── STM32GPIODriver.c
│
├── BSP/                        # Board Support Package
│   ├── CMSIS/                  # ARM Cortex-M interface
│   ├── ICF/                    # IAR linker config
│   └── stm32f0xx_conf.h
│
├── Protocol/                   # Third-party Protocol Stack
│   └── MBslave/               # Modbus RTU Slave Stack
│       ├── mbs.c
│       ├── rtu/
│       │   ├── mbsrtu.c        # Frame parsing
│       │   └── mbscrc.c        # CRC check
│       ├── functions/
│       │   ├── mbsfuncholding.c  # 0x03/0x10 Holding registers
│       │   ├── mbsfunccoils.c    # 0x01/0x05 Coils
│       │   └── ...
│       └── port/
│           ├── mbportserial.c   # Serial port adapter
│           ├── mbporttimer.c   # Timer adapter
│           └── mbportevent.c   # Event mechanism
│
├── PRJ/                        # IAR Project Files
│   └── IAR/
│       ├── VX4204.ewp
│       ├── VX4204.ewd
│       └── Debug/
│           └── Exe/
│               └── VX-4204-V2.0.x-*.bin
│
└── RTTViewer/                  # Segger RTT Debug Tool
    ├── RTT/
    │   ├── SEGGER_RTT.c
    │   └── SEGGER_RTT.h
    └── SEGGER_RTT_Conf.h
```

---

## 🏗️ Chapter 3: System Architecture

### 3.1 Layered Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application Layer (APP)                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐ │
│  │   main.c     │  │ Communication│  │   ServiceHandling    │ │
│  │  Main Loop   │  │ Modbus Proc  │  │ IO Control + Data   │ │
│  └──────────────┘  └──────────────┘  └──────────────────────┘ │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐ │
│  │   Common     │  │     Init      │  │      Common          │ │
│  │Diag/Warn/WD │  │ IO Init       │  │                      │ │
│  └──────────────┘  └──────────────┘  └──────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│                      Driver Layer (Drivers)                     │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────────────┐ │
│  │  UART   │ │   SPI   │ │  GPIO   │ │       DAC           │ │
│  │ RS485   │ │ AD5755  │ │  LED    │ │     AD5755          │ │
│  └─────────┘ └─────────┘ └─────────┘ └─────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│                    Protocol Layer (Protocol)                    │
│              Modbus RTU Slave Stack (MBslave)                   │
├─────────────────────────────────────────────────────────────────┤
│                      BSP / CMSIS                                 │
│           STM32F3 Standard Peripherals Library                   │
├─────────────────────────────────────────────────────────────────┤
│                        Hardware                                  │
│              STM32F302CB + AD5755 × 2                           │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 Data Flow Architecture

```
                    Modbus RTU (RS-485)
                          │
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                        USART2 + DMA                            │
│                    (Address Detection Mode)                     │
└─────────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Modbus Stack                                │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐      │
│  │ CRC Chk  │→│  Frame   │→│ Function  │→│ Register │      │
│  │ mbscrc  │  │ mbsrtu   │  │ Handler  │  │ Callback │      │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘      │
└─────────────────────────────────────────────────────────────────┘
                          │
          ┌───────────────┼───────────────┐
          ▼               ▼               ▼
┌─────────────────┐ ┌─────────────┐ ┌─────────────────┐
│  Coil (DO)      │ │ Holding Reg │ │ File Record     │
│  0x/1x          │ │ 4x          │ │ Device Config   │
└─────────────────┘ └─────────────┘ └─────────────────┘
          │               │               │
          └───────────────┼───────────────┘
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                     HandleIoData()                              │
│  1. Check if AO data updated                                   │
│  2. Copy data to main loop buffer                               │
│  3. Send to AD5755 via SPI                                     │
│  4. Update LED status                                          │
└─────────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                    AD5755 DAC × 2                              │
│  ┌────────────────────┐    ┌────────────────────┐            │
│  │ AD5755 (Ch 1-4)    │    │ AD5755 (Ch 5-8)   │            │
│  │ SPI1 + AD0=0       │    │ SPI1 + AD0=1       │            │
│  └────────────────────┘    └────────────────────┘            │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🚀 Chapter 4: Startup Flow Analysis

### 4.1 Complete Startup Flow

```
Power On
  │
  ▼
┌──────────────────────┐
│ System Reset         │
│ (Hardware Reset)     │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ IoSystemInit()      │
│ - SYSCFG_DeInit()   │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ ReadDataFromFlash() │
│ - Read cal params   │
│ - Gain/Offset       │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ AIAOIndictorGPIOInit│
│ - LED GPIO Init     │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ STM32DAC5755DriverInit│
│ - SPI1 Init         │
│ - AD5755 Chip Init  │
│ - Configure ranges  │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ GetIoDeviceContentInfo│
│ - Read device config│
│   from Flash        │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ GetVariousStartAddr │
│ - Calculate reg map │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ USART2 + DMA Init   │
│ - Baudrate config   │
│ - 9-bit addr mode   │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ STM32AddressRecognition│
│ - Read addr pins    │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ TIM6/TIM3 Init     │
│ - Timer IRQ config  │
└──────────┬───────────┘
           │
           ▼
    ╔═══════════════════════╗
    ║   Wait for Address   ║
    ║   Assignment         ║
    ║   QueryStartInit()   ║
    ╚═══════════════════════╝
           │
           ├─→ No assign → Wait in loop...
           │
           ▼ (Received assign frame)
┌──────────────────────┐
│ SetStationNumber()  │
│ - Set Modbus addr  │
│ - Enable 9-bit mode │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ eMBSSerialInit()    │
│ - Init Modbus stack │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ Register Callbacks   │
│ - Coil callback     │
│ - Holding Reg cb    │
│ - File Record cb    │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ Main Loop main()    │
│ FOREVER             │
└──────────┬───────────┘
```

### 4.2 Address Assignment Mechanism (Key!)

This design is clever: **The product has no preset Modbus address, assigned by external tool**.

```c
// Key logic in IoInit.c
while (1) {
    // 1. Wait for address recognition pin HIGH
    ucBitStatus = GPIO_ReadInputDataBit(ADDRESS_RECOGNITION_LEFT_GPIO, ...);
    if (Bit_SET == ucBitStatus) {
        USART_Cmd(USART2, ENABLE);  // Enable UART RX
        break;
    }
    FeedTheWatchDog();  // Keep watchdog happy
}

// 2. Wait for assignment frame
while (1) {
    if (STM32F0_USART_RECEIVE_COMPLETE_SWITCH_ON == ucIsReceivedData) {
        break;
    }
}

// 3. Parse frame and set address
SetStationNumber(ucContent);  // Set slave address

// 4. Switch to 9-bit address mode
USART_AddressDetectionConfig(USART2, USART_AddressLength_7b);
USART_SetAddress(USART2, gucStationNum);
USART_MuteModeCmd(USART2, ENABLE);  // Only accept addr-matched frames
```

**Why this design?**
- Multiple devices can be paralleled on same RS-485 bus
- No manual dial switch needed for factory settings
- Host can assign addresses in batch - easy for deployment

---

## 📊 Chapter 5: Core Module Analysis

### 5.1 main.c - Main Loop Architecture

```c
int main(void)
{
    xMBSHandle xMBSHdl = NULL;
    
    // 1. Init RTT debug
    SEGGER_RTT_Init();
    
    // 2. Core init (returns handle to Modbus stack)
    eResults = IoInit(&xMBSHdl);
    
    // 3. Enter main loop
    for(;;)
    {
        // 【Key】Feed watchdog - prevent system hang
        FeedTheWatchDog();
        
        // Handle Modbus receive
        if (ucIsReceivedData == 1) {
            RTUFrameReceive(xMBSHdl);   // Receive frame
            eMBSPoll(xMBSHdl);           // Process request
            ucIsReceivedData = 0;
        }
        
        // Handle IO data (AO output)
        HandleIoData();
        
        // ADC diagnosis data acquisition
        ADC1GetVolTmpDiagnosisData();
        
        // IO diagnosis
        IODiagnose();
    }
}
```

### 5.2 HandleIoData - AO Output Processing Flow

```c
void HandleIoData(void)
{
    // 1. Disconnection handling
    if(gucDisConnectionFlag) {
        ZeroChannelOutput();  // Output 0 on disconnect
        gucDisConnectionFlag = 0;
    }
    
    // 2. Check if data updated
    if(gucAoDataIsChanged != AO_DATA_CHANGED) {
        return;  // No update, skip
    }
    
    // 3. Copy data to main loop buffer
    if (gucDownServDataAnalogUpdateOK == E_DATA_SYNC_STATUS_UPDATED) {
        gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_MAINLOOP_HANDLING;
        memcpy(gusaAnalogDataMainLoop, gusaAnalogData, MAX_AO_CHANNEL_NUMBER);
        gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_HOLD;
    }
    
    // 4. Send to DAC channel by channel
    for(uiChannelNum = 0; uiChannelNum < 8; uiChannelNum++) {
        if (SPI_I2S_GetFlagStatus(DAC_SPI, SPI_I2S_FLAG_BSY) == RESET) {
            (void)STM32AD5755SendData(uiChannelNum);  // SPI send
        }
    }
    
    // 5. DAC status read & LED update
    AD5755_SetControlRegisters(...);   // Read status
    AD5755_ParseStatusRegister();       // Parse error flags
    UpdateIndicatorLights();            // Update LEDs
}
```

### 5.3 Modbus Register Mapping

This is key to understanding the entire system!

| Address | Function | Access | Description |
|---------|----------|--------|-------------|
| **0x0000 - 0x0007** | AO_CH1 - AO_CH8 | RW | Analog output value (16-bit) |
| **0x0010** | STATUS | R | Module status |
| **0x0011** | VERSION | R | Firmware version |
| **0x1000 - 0x1FFF** | Diagnosis data | R | Temperature, voltage, etc |
| **0x2000 - 0x2FFF** | Config params | RW | Output type, calibration |
| **File Record 0** | Device info | R/W | Vendor, product info |
| **File Record 1** | Config info | R/W | Channel config |
| **Coil 0x0000** | Soft reset | WO | Write 1 to reset |
| **Coil 0x0001** | Upgrade flag | WO | FW upgrade trigger |
| **Coil 0x0002** | Assign prep | WO | Address assign prep |
| **Coil 0x0003** | Assign done | WO | Address assign done |

### 5.4 AD5755 Driver Core

**Two AD5755 chips cascaded for 8 channels**:

```c
// Chip select via AD0 pin
AD5575_st  = { pinAD0state = 0 };  // Chip 1 (Ch 1-4)
AD5575_st2 = { pinAD1state = 1 };  // Chip 2 (Ch 5-8)

// SPI command format (32-bit)
command = AD5755_ISR_WRITE |     // Write operation
           AD5755_ISR_DUT_AD1(...) | // Chip select
           AD5755_ISR_DREG(reg) |    // Target register
           AD5755_ISR_DAC_AD(ch) |  // Channel select
           AD5755_ISR_DATA(value);  // Data value
```

**Supported Output Types** (configured via `AISampleType[]`):

| Type Code | Range | Description |
|-----------|-------|-------------|
| 0x01 | 0-10V (0-27648) | Voltage |
| 0x02 | 0-10V (0-32000) | Voltage |
| 0x03 | ±10V (-27648~27648) | Bipolar voltage |
| 0x04 | ±10V (-32000~32000) | Bipolar voltage |
| 0x05 | 0-5V (0-27648) | Voltage |
| 0x07 | ±5V (-27648~27648) | Bipolar voltage |
| 0x09 | 0-20mA (0-27648) | Current |
| 0x0B | 4-20mA (0-27648) | Current ⭐Common |
| 0x0C | 4-20mA (0-32000) | Current |

---

## 📝 Chapter 6: Key Data Structures

### 6.1 AO Data Structure

```c
// Analog output data (written by Modbus)
int16_t gusaAnalogData[MAX_AO_CHANNEL_NUMBER];  // [8] Ch 1-8

// Main loop data copy
int16_t gusaAnalogDataMainLoop[MAX_AO_CHANNEL_NUMBER];

// AO channel calibration params (Flash stored)
typedef struct {
    int32_t lCompensateValue;   // Comp value (Ch 1-4)
    int32_t lCompensateValue2;  // Comp value (Ch 5-8)
} AODemarcate_t;

AODemarcate_t gtaAODemarcate[MAX_AI_CHANNEL_NUMBER];  // [8]
```

### 6.2 Device Config Structure

```c
typedef struct {
    uint8_t  ucContentInfo[MAX_PART_DEVICE_INFO_LENGTH];  // Config content
    uint16_t usContentLength;   // Content length
} IoDeviceContentInfo_t;

IoDeviceContentInfo_t taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_BUTT];
```

### 6.3 LED Indicator Structure

```c
typedef struct {
    uint8_t   uchannel;           // Channel number
    uint8_t   eAIAOChennelStatus; // Status
    GPIO_TypeDef* GPIOx;           // GPIO port
    uint16_t  ulAIAOChennelPIN;    // Pin
} AIAOIndictor_t;

AIAOIndictor_t gtaIndictor[9] = {
    {0, ..., GPIOA, GPIO_Pin_7},   // CH1
    {1, ..., GPIOB, GPIO_Pin_0},   // CH2
    // ...
    {8, ..., GPIOA, GPIO_Pin_0},   // ERROR
};
```

---

## 🔄 Chapter 7: Key Flow Timing

### 7.1 Modbus Communication Timing

```
Host sends request frame:
┌────┬────┬─────┬────┬────┬────┐
│ADDR│FUNC│DATA │... │CRC │CRC │
└────┴────┴─────┴────┴────┴────┘
 1B   1B   nB            1B   1B

  │
  ▼ (USART2 DMA receive)

┌─────────────────────────────────────────┐
│ ucIsReceivedData = 1                    │
└─────────────────────────────────────────┘
  │
  ▼ (Detected in main loop)

┌─────────────────────────────────────────┐
│ RTUFrameReceive()                       │
│ - Frame parsing + CRC check             │
└─────────────────────────────────────────┘
  │
  ▼

┌─────────────────────────────────────────┐
│ eMBSPoll()                              │
│ - Call corresponding callback by func code│
│   - 0x03/0x10 → HandleAnalogData()     │
│   - 0x01/0x05 → HandleDigitalData()    │
│   - 0x14/0x15 → HandleRead/WriteFileRecord()│
└─────────────────────────────────────────┘
  │
  ▼

┌─────────────────────────────────────────┐
│ HandleAnalogData()                      │
│ - Write to gusaAnalogData[]             │
│ - Set gucAoDataIsChanged = 1           │
│ - Reset disconnection timer             │
└─────────────────────────────────────────┘
```

### 7.2 AO Output Timing

```
Modbus writes AO value
       │
       ▼
HandleAnalogData()
  │
  ├─→ gusaAnalogData[ch] = value
  │
  └─→ gucAoDataIsChanged = AO_DATA_CHANGED
                    │
                    ▼ (Main loop)
              HandleIoData()
                    │
                    ├─→ memcpy(gusaAnalogDataMainLoop, gusaAnalogData)
                    │
                    ├─→ for 8 channels:
                    │     SPI_Send(DAC value)
                    │
                    ├─→ AD5755_SetControlRegisters()  // Read status
                    │
                    ├─→ AD5755_ParseStatusRegister() // Parse errors
                    │
                    └─→ UpdateIndicatorLights()      // LED indication
```

### 7.3 Disconnection Detection Timing

```
Host writes AO data
       │
       ▼
TIM3 timer reset
       │
       ▼ (Timeout - no new data received)
  TIM3_IRQHandler()
       │
       ├─→ gucAoDataIsChanged = AO_DATA_CHANGED
       │
       └─→ gucDisConnectionFlag = 1
                    │
                    ▼ (Main loop)
              HandleIoData()
                    │
                    └─→ ZeroChannelOutput()  // Output to zero
```

---

## ⚠️ Chapter 8: Potential Issues / Risk Points

### 8.1 Hardware Related

| Issue | Risk | Description |
|-------|------|-------------|
| **SPI no timeout** | 🔴 High | If SPI hangs, all channels blocked |
| **DAC no watchdog** | 🟡 Medium | AD5755 internal watchdog 200ms, may desync with main MCU |
| **Addr pin noise** | 🟡 Medium | PA4/PA5 may be interfered in industrial env |
| **No TVS protection** | 🟡 Medium | RS-485 should add TVS diodes |

### 8.2 Software Related

| Issue | Risk | Description |
|-------|------|-------------|
| **Modbus frame buffer race** | 🔴 High | `gusaAnalogData` vs `gusaAnalogDataMainLoop` may desync |
| **Interrupt nesting risk** | 🟡 Medium | Check SPI/USART interrupt priority carefully |
| **Flash wear** | 🟡 Medium | Frequent calibration writes wear Flash |
| **Memory overflow** | 🟢 Low | Static arrays, no dynamic allocation |

### 8.3 Communication Related

| Issue | Risk | Description |
|-------|------|-------------|
| **9-bit addr mode compatibility** | 🟡 Medium | Some hosts don't support |
| **CRC calculation perf** | 🟢 Low | Table lookup method, good performance |
| **Broadcast address support** | 🟢 Low | Slave doesn't respond to broadcast, spec compliant |

---

## 💡 Chapter 9: Optimization Suggestions

### 9.1 Engineering-Level Optimization

#### 1. Add RTOS (Strongly Recommended ⭐⭐⭐)
```c
// Current: Bare-metal polling
for(;;) {
    HandleIoData();
    // ... other processing
}

// Recommended: FreeRTOS task division
TaskHandle_t xModbusTask;
TaskHandle_t xIoTask;
TaskHandle_t xDiagTask;

void vModbusTask(void *pvParameters) {
    for(;;) {
        xSemaphoreTake(xModbusSem, portMAX_DELAY);
        eMBSPoll(xMBSHdl);
    }
}

void vIoTask(void *pvParameters) {
    for(;;) {
        HandleIoData();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
```

#### 2. SPI DMA Transfer Optimization
```c
// Current: Blocking send
for(uiChannelNum = 0; uiChannelNum < 8; uiChannelNum++) {
    while (SPI_I2S_GetFlagStatus(DAC_SPI, SPI_I2S_FLAG_BSY) == SET);
    SPI_SendData(...);
}

// Recommended: DMA double buffer
typedef struct {
    uint8_t txBuf[8][4];
    uint8_t rxBuf[8][4];
    uint8_t currentChannel;
} DAC_HandleTypeDef;
```

#### 3. Watchdog Optimization
```c
// Current: GPIO simulated square wave
void FeedTheWatchDog(void) {
    static uint8_t count = 0;
    if (count == 0) GPIO_ResetBits(...);
    else GPIO_SetBits(...);
    count = !count;
}

// Recommended: Use STM32 built-in IWDG
void Watchdog_Init(void) {
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);  // 32kHz/256 ≈ 8s
    IWDG_SetReload(0xFFF);  // Max timeout
    IWDG_ReloadCounter();
}
```

### 9.2 Code Quality Optimization

#### 1. Add Assertions
```c
// Add at key entry points
#define ASSERT(expr)  ((expr) ? (void)0 : assert_failed(__FILE__, __LINE__))
void assert_failed(uint8_t* file, uint32_t line);
```

#### 2. Decouple Modules
```c
// Current: Direct global variables
extern int16_t gusaAnalogData[];

// Recommended: Information hiding
typedef struct {
    int16_t (*get)(uint8_t channel);
    void    (*set)(uint8_t channel, int16_t value);
} AO_Interface_t;

extern const AO_Interface_t g_AO;
```

#### 3. Config Verification
```c
// Add startup integrity check
IO_InterfaceResult_e IoInit_Verify(void) {
    // Verify Flash data CRC
    // Verify DAC chip communication
    // Verify critical GPIOs
}
```

### 9.3 Testing Recommendations

| Test Item | Method |
|-----------|--------|
| Communication stress | 10000+ continuous read/write |
| Disconnect recovery | RS-485 plug/unplug multiple times |
| Long duration run | 72 hours continuous output |
| Temperature test | -40°C ~ 85°C |
| EMI test | Burst, surge |

---

## 📌 Appendix

### A. Hardware Pin Quick Reference

```
STM32F302CB                    Function
─────────────────────────────────
PA5 (SPI1_SCK)    ────────────  AD5755 SCK
PA6 (SPI1_MISO)   ────────────  AD5755 MISO  
PA7 (SPI1_MOSI)   ────────────  AD5755 MOSI
PA9 (USART1_TX)   ────────────  RS-485 TX
PA10(USART1_RX)   ────────────  RS-485 RX
PA8              ────────────  RS-485 DE

PA4               ────────────  Address recognition (L)
PA5               ────────────  Address recognition (R)

PB0-PB2, PB6-9    ────────────  LED CH1-CH8
PA0               ────────────  LED ERROR
```

### B. Register Address Quick Reference

```c
// Core address definitions (common.h)
#define DOWN_ANALOG_DATA_START_ADDR    0x0000  // AO values
#define UP_ANALOG_DATA_START_ADDR      0x0100  // AI values (reserved)
#define WARN_DIGITAL_START_ADDR        0x0500  // Alarms
#define DIAGNOSE_ANALOG_START_ADDR     0x1000  // Diagnosis
#define CONFIG_INFO_START_ADDR         0x2000  // Configuration
```

### C. Build Instructions

```bash
# IAR Embedded Workbench 9.x
# Chip: STM32F302CB
# Build: Debug or Release
# Download: J-Link / ST-Link
```

---

*Document generated: 2026-03-18*
*Generated from code analysis*
