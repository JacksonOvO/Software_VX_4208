/******************************************************************
*
*文件名称:  STM32FAds1248Driver.h
*文件标识:
*内容摘要:
*其它说明:
*当前版本:
*作    者: 
*完成日期:  2018/3/25
*
*修改记录1:
*    修改日期:
*    版 本 号:
*    修 改 人:
*    修改内容:
******************************************************************/

#ifndef __STM32FADSDRIVER_H
#define  __STM32FADSDRIVER_H

#pragma once 
/******************************************************************
*                             头文件                              *
******************************************************************/
#include <string.h>
#include "common.h"
#include "STM32F0Ads8331Driver.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/

/******************************************************************
*                           数据类型                              *
******************************************************************/

#define  ADS_CONNECTION          0       
#define  ADS_DISCONNECTION       1
#define  ADS_OVERFLOW             2

typedef enum
{
    E_INIT_NONE = 0,
    E_INIT_MCU_COMPLETE,
    E_INIT_DRIVER_COMPLETE,
    E_LED_SET_COMPLETE,
    E_INIT_BUTT
}InitStatus_e;

typedef struct
{
    uint32_t usaLinearValue1; //相当于y
    uint32_t usaLinearValue2;
    int32_t lTemperature_1;   //相当于x 
    int32_t lTemperature_2;
} TableLinear_t;

       
#define FLASHADDRESSLEN        4
#define TABLEHEADADDRESS       0x08006800
#define CALIBRATIONADADDRESS   0x0800DA00 

    
#define MCUSAMPLETIMES   10
    
/*  ads1248 register map                                                */
/************************************************************************/
#define ADS_MUX0            0X00
#define ADS_VBIAS           0X01
#define ADS_MUX1            0X02
#define ADS_SYS0            0X03
#define ADS_OFC0            0X04
#define ADS_OFC1            0X05
#define ADS_OFC2            0X06
#define ADS_FSC0            0X07
#define ADS_FSC1            0X08
#define ADS_FSC2            0X09
#define ADS_IDAC0           0X0A
#define ADS_IDAC1           0X0B
#define ADS_GPIOCFG         0X0C
#define ADS_GPIODIR         0X0D
#define ADS_GPIODAT         0X0E

/************************************************************************/
/* ads1248 SPI commands                                                 */
/************************************************************************/
#define ADS_WAKEUP          0x00
#define ADS_SLEEP           0x02
#define ADS_SYNC            0x04
#define ADS_RESET           0x06
#define ADS_NOP             0xFF
#define ADS_RDATA           0x12
#define ADS_RDATAC          0x14
#define ADS_SDATAC          0x16
#define ADS_RREG            0x20
#define ADS_WREG            0x40
#define ADS_SYSOCAL         0x60
#define ADS_SYSGCAL         0x61
#define ADS_SELFOCAL        0x62

/************************************************************************/
/* ads1248 macroinstruction                                             */
/************************************************************************/
// about MUX0: Multiplexer Control Register 0
#define  P_AIN0             0x00
#define  P_AIN1             0x08
#define  P_AIN2             0x10
#define  P_AIN3             0x18
#define  P_AIN4             0x20
#define  P_AIN5             0x28
#define  P_AIN6             0x30
#define  P_AIN7             0x38
#define  N_AIN0             0x00
#define  N_AIN1             0x01
#define  N_AIN2             0x02
#define  N_AIN3             0x03
#define  N_AIN4             0x04
#define  N_AIN5             0x05
#define  N_AIN6             0x06
#define  N_AIN7             0x07
#define  CURRENT_OFF        0x00
#define  CURRENT_05uA       0x40
#define  CURRENT_2uA        0x80
#define  CURRENT_10uA       0xc0

// about MUX1: Multiplexer Control Register 1
#define  CLK_Inter              0x00
#define  CLK_Exter              0x80
#define  REF_Inter_AlwaysOn     0x20  // selecte internal reference and always open  
#define  REF_Inter_AlwaysOff    0x00  // selecte internal reference and always off 
#define  SELT_REF0              0x00                
#define  SELT_REF1              0x08
#define  SELT_Inter             0x10
#define  SELT_Inter_REF0        0x18

// about SYS0 : System Control Register 0
#define  PGAGain_1          0x00
#define  PGAGain_2          0x10
#define  PGAGain_4          0x20
#define  PGAGain_8          0x30
#define  PGAGain_16         0x40
#define  PGAGain_32         0x50
#define  PGAGain_64         0x60
#define  PGAGain_128        0x70
#define  DataRate_5         0x00
#define  DataRate_10        0x01
#define  DataRate_20        0x02
#define  DataRate_40        0x03
#define  DataRate_80        0x04
#define  DataRate_160       0x05
#define  DataRate_320       0x06
#define  DataRate_640       0x07
#define  DataRate_1000      0x08
#define  DataRate_2000      0x09

// about IDAC0: IDAC Control Register 0
#define  Drdy_Mode_EN       0x08
#define  Drdy_Mode_DIS      0x00

// the magnitude of the excitation current.
// The IDACs require the internal reference to be on.
#define  IMAG_Off           0x00
#define  IMAG_50            0x01
#define  IMAG_100           0x02
#define  IMAG_250           0x03
#define  IMAG_500           0x04
#define  IMAG_750           0x05
#define  IMAG_1000          0x06
#define  IMAG_1500          0x07

// about IDAC1: IDAC Control Register 1
// select the output pin for the first current source DAC.
#define  IDAC1_AIN0         0x00
#define  IDAC1_AIN1         0x10
#define  IDAC1_AIN2         0x20
#define  IDAC1_AIN3         0x30
#define  IDAC1_AIN4         0x40
#define  IDAC1_AIN5         0x50
#define  IDAC1_AIN6         0x60
#define  IDAC1_AIN7         0x70
#define  IDAC1_IEXT1        0x80
#define  IDAC1_IEXT2        0x90
#define  IDAC1_OFF          0xC0

// select the output pin for the second current source DAC.
#define  IDAC2_AIN0         0x00
#define  IDAC2_AIN1         0x01
#define  IDAC2_AIN2         0x02
#define  IDAC2_AIN3         0x03
#define  IDAC2_AIN4         0x04
#define  IDAC2_AIN5         0x05
#define  IDAC2_AIN6         0x06
#define  IDAC2_AIN7         0x07
#define  IDAC2_IEXT1        0x08
#define  IDAC2_IEXT2        0x09
#define  IDAC2_OFF          0x0C
    
#define ADS1248_SPI         SPI1

/******************************************************************
*                           全局变量声明                          *
******************************************************************/
extern InitStatus_e eInitCompleteFlag;

/******************************************************************
*                         全局函数声明                            *
******************************************************************/


#endif

