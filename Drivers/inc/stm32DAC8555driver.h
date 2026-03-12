/*******************************************************************
*文件名称:  STM32AOSPIDriver.h
*文件标识:
*内容摘要:
*其它说明:
*当前版本:
*
*修改记录1:
*    修改日期:
*    版 本 号:
*    修 改 人:
*    修改内容:
******************************************************************/

#ifndef __STM32DAC8555DRIVER_H
#define __STM32DAC8555DRIVER_H

/******************************************************************
*                             头文件                              *
******************************************************************/
#include "stm32f30x.h"
#include "stm32f30x_spi.h"

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
#define DAC_CONTROL_BASE_VALUE             0x10
#define DAC_CHANNELA                       0x0
#define DAC_CHANNELB                       0x2
#define DAC_CHANNELC                       0x4
#define DAC_CHANNELD                       0x6
#define DAC_DATA_CONFIG                    8
#define A_BYTE_DATA                       0xFF
#define CRC_POLYNOMIAL                     7 
#define AO_SAMPLING_PERIOD                 20    /*定时器时间10us*/
#define AO_CYCLE_TIMES                     8
#define CHANNEL_INITIAL_VALUE              0

/*关于GPIO相应的DAC芯片管脚*/


#define DAC_SPI_CLK                              RCC_APB2Periph_SPI1
#define DAC_SPI                                  SPI1

#define INDICTOR_LDAC_GPIO                      (GPIOB)         
#define INDICTOR_LDAC_GPIO_PIN              (GPIO_Pin_12)   
#define INDICTOR_CLEAR_GPIO                      (GPIOB)       
#define INDICTOR_CLEAR_GPIO_PIN              (GPIO_Pin_14) 
#define INDICTOR_LDAC2_GPIO                      (GPIOA)         
#define INDICTOR_LDAC2_GPIO_PIN              (GPIO_Pin_10)   
#define INDICTOR_CLEAR2_GPIO                      (GPIOA)       
#define INDICTOR_CLEAR2_GPIO_PIN              (GPIO_Pin_9)                   

#define  LDAC_HIGH()  				 GPIO_SetBits(GPIOB, GPIO_Pin_12)
					
#define  LDAC_LOW()  				 GPIO_ResetBits(GPIOB, GPIO_Pin_12) 

/* SPI slave device ID */
#define AD5755_SLAVE_ID         1

/* Input Shift Register Contents for a Write Operation. */
#define AD5755_ISR_WRITE            (0ul << 23)
#define AD5755_ISR_DUT_AD1(x)       (((long)(x) & 0x1) << 22)
#define AD5755_ISR_DUT_AD0(x)       (((long)(x) & 0x1) << 21)
#define AD5755_ISR_DREG(x)          (((long)(x) & 0x7) << 18)
#define AD5755_ISR_DAC_AD(x)        (((long)(x) & 0x3) << 16)
#define AD5755_ISR_DATA(x)          ((long)(x) & 0xFFFF)

/* Nop operation code. */
#define AD5755_ISR_NOP              0x1CE000

/* AD5755_ISR_DREG(x) options. */
#define AD5755_DREG_WR_DAC          0
#define AD5755_DREG_WR_GAIN         2
#define AD5755_DREG_WR_GAIN_ALL     3
#define AD5755_DREG_WR_OFFSET       4
#define AD5755_DREG_WR_OFFSET_ALL   5
#define AD5755_DREG_WR_CLR_CODE     6
#define AD5755_DREG_WR_CTRL_REG     7

/* AD5755_ISR_DAC_AD(x) options. */
#define AD5755_DAC_A            0
#define AD5755_DAC_B            1
#define AD5755_DAC_C            2
#define AD5755_DAC_D            3

/* Gain register definition. */
#define AD5755_GAIN_ADJUSTMENT(x)       ((long)(x) & 0xFFFF)

/* Offset register definition. */
#define AD5755_OFFSET_ADJUSTMENT(x)     ((long)(x) & 0xFFFF)

/* Clear Code Register definition. */
#define AD5755_CLEAR_CODE(x)            ((long)(x) & 0xFFFF)

/* Control Register definition. */
#define AD5755_CTRL_CREG(x)             (((x) & 0x7) << 13)
#define AD5755_CTRL_DATA(x)             ((x) & 0xFFF)

/* AD5755_CTRL_CREG(x) options. */
#define AD5755_CREG_SLEW        0 // Slew rate control register(one per channel)
#define AD5755_CREG_MAIN        1 // Main control register
#define AD5755_CREG_DAC         2 // DAC control register(one per channel)
#define AD5755_CREG_DC_DC       3 // DC-to-dc control register
#define AD5755_CREG_SOFT        4 // Software register

/* Slew Rate Control Register definition. */
#define AD5755_SLEW_SREN            (1 << 12)
#define AD5755_SLEW_SR_CLOCK(x)     (((x) & 0xF) << 3)
#define AD5755_SLEW_SR_STEP(x)      (((x) & 0x7) << 0)

/* AD5755_SLEW_SR_CLOCK(x) options. */
#define AD5755_SR_CLK_64K       0
#define AD5755_SR_CLK_32k       1
#define AD5755_SR_CLK_16k       2
#define AD5755_SR_CLK_8K        3
#define AD5755_SR_CLK_4K        4
#define AD5755_SR_CLK_2K        5
#define AD5755_SR_CLK_1K        6
#define AD5755_SR_CLK_500       7
#define AD5755_SR_CLK_250       8
#define AD5755_SR_CLK_125       9
#define AD5755_SR_CLK_64        10
#define AD5755_SR_CLK_32        11
#define AD5755_SR_CLK_16        12
#define AD5755_SR_CLK_8         13
#define AD5755_SR_CLK_4         14
#define AD5755_SR_CLK_0_5       15

/* AD5755_SLEW_SR_STEP(x) options. */
#define AD5755_STEP_1       0
#define AD5755_STEP_2       1
#define AD5755_STEP_4       2
#define AD5755_STEP_16      3
#define AD5755_STEP_32      4
#define AD5755_STEP_64      5
#define AD5755_STEP_128     6
#define AD5755_STEP_256     7

/* Main Control Register definition. */
#define AD5755_MAIN_POC             (1 << 12)
#define AD5755_MAIN_STATREAD        (1 << 11)
#define AD5755_MAIN_EWD             (1 <<10)
#define AD5755_MAIN_WD(x)           (((x) & 0x3) << 8)
#define AD5755_MAIN_SHTCCTLIM(x)    (((x) & 0x1) << 6)
#define AD5755_MAIN_OUTEN_ALL       (1 << 5)
#define AD5755_MAIN_DCDC_ALL        (1 << 4)

/* AD5755_MAIN_WD(x) options. */
#define AD5755_WD_5MS               0 // 5 ms timeout period
#define AD5755_WD_10MS              1 // 10 ms timeout period
#define AD5755_WD_100MS             2 // 100 ms timeout period
#define AD5755_WD_200MS             3 // 200 ms timeout period

/* AD5755_MAIN_SHTCCTLIM(x) options. */
#define AD5755_LIMIT_16_MA          0 // 16 mA (default)
#define AD5755_LIMIT_8_MA           1 // 8 mA

/* DAC Control Register definition. */
#define AD5755_DAC_INT_ENABLE       (1 << 8)
#define AD5755_DAC_CLR_EN           (1 << 7)
#define AD5755_DAC_OUTEN            (1 << 6)
#define AD5755_DAC_NOUTEN            (0 << 6)
#define AD5755_DAC_RSET             (1 << 5)
#define AD5755_DAC_DC_DC            (1 << 4)
#define AD5755_DAC_OVRNG            (1 << 3)
#define AD5755_DAC_R(x)             ((x) & 0xf)

/* AD5755_DAC_R(x) options. */
#define AD5755_R_0_5_V              0 // 0 V to 5 V voltage range (default)
#define AD5755_R_0_10_V             1 // 0 V to 10 V voltage range
#define AD5755_R_M5_P5_V            2 // -5 V to +5 V voltage range
#define AD5755_R_M10_P10_V          3 // -10 V to 10 V voltage range
#define AD5755_R_4_20_MA            4 // 4 mA to 20 mA current range
#define AD5755_R_0_20_MA            5 // 0 mA to 20 mA current range
#define AD5755_R_0_24_MA            6 // 0 mA to 24 mA current range
#define AD5755_R_0_6_V              8 // 0 V to 6 V voltage range (default)
#define AD5755_R_0_12_V             9 // 0 V to 12 V voltage range
#define AD5755_R_M6_P6_V           10 // -6 V to +6 V voltage range
#define AD5755_R_M12_P12_V         11 // -12 V to 12 V voltage range

/* DC-to-DC Control Register definition. */
#define AD5755_DC_DC_COMP           (1 << 6)
#define AD5755_DC_DC_PHASE(x)       (((x) & 0x3) << 4) 
#define AD5755_DC_DC_FREQ(x)        (((x) & 0x3) << 2)   
#define AD5755_DC_DC_MAX_V(x)       (((x) & 0x3) << 0)

/* AD5755_DC_DC_PHASE(x) options. */
#define AD5755_PHASE_ALL_DC_DC      0 // all dc-dc converters clock on same edge
#define AD5755_PHASE_AB_CD          1 // Ch A,B clk same edge, C,D opposite edge
#define AD5755_PHASE_AC_BD          2 // Ch A,C clk same edge, B,D opposite edge
#define AD5755_PHASE_A_B_C_D_90     3 // A,B,C,D clock 90 degree out of phase

/* AD5755_DC_DC_FREQ(x) options. */
#define AD5755_FREQ_250_HZ          0 // 250 +/- 10% kHz
#define AD5755_FREQ_410_HZ          1 // 410 +/- 10% kHz
#define AD5755_FREQ_650_HZ          2 // 650 +/- 10% kHz

#define AD5755_DAC_RSET_10K  (0 << 4)  // 参考你芯片文档或驱动头文件
#define AD5755_DAC_OVRNG_4_20mA (1 << 6)

/* AD5755_DC_DC_MAX_V(x) options. */
#define AD5755_MAX_23V          0 // 23 V + 1 V/-1.5 V (default)
#define AD5755_MAX_24_5V        1 // 24.5 V +/- 1 V
#define AD5755_MAX_27V          2 // 27 V +/- 1 V
#define AD5755_MAX_29_5V        3 // 29.5 V +/- 1V

/* Software Register definition. */
#define AD5755_SOFT_USER_BIT        (1 << 12)
#define AD5755_SOFT_RESET_CODE(x)   ((x) & 0xFFF)

/* AD5755_SOFT_RESET_CODE(x) options. */
#define AD5755_RESET_CODE       0x555 // Performs a reset of the AD5755.
#define AD5755_SPI_CODE         0x195 // If watchdog is enabled, 0x195 must be
                                      // written to the software register within
                                      // the programmed timeout period.
                                      
/* Input Shift Register Contents for a Read Operation. */                                      
#define AD5755_ISR_READ             (1ul << 23)
/* Same as Input Shift Register Contents for a Write Operation. */
/*
#define AD5755_ISR_DUT_AD1(x)       (((long)(x) & 0x1) << 22)
#define AD5755_ISR_DUT_AD0(x)       (((long)(x) & 0x1) << 21)
*/
#define AD5755_ISR_RD(x)            (((long)(x) & 0x1F) << 16)

/* AD5755_ISR_RD(x) options. */
#define AD5755_RD_DATA_REG(x)           (((x) & 0x3) + 0)
#define AD5755_RD_CTRL_REG(x)           (((x) & 0x3) + 4)
#define AD5755_RD_GAIN_REG(x)           (((x) & 0x3) + 8)
#define AD5755_RD_OFFSET_REG(x)         (((x) & 0x3) + 12)
#define AD5755_RD_CODE_REG(x)           (((x) & 0x3) + 16)
#define AD5755_RD_SR_CTRL_REG(x)        (((x) & 0x3) + 20)
#define AD5755_RD_STATUS_REG            24
#define AD5755_RD_MAIN_CTRL_REG         25
#define AD5755_RD_Dc_DC_CTRL_REG        26

/* Status Register definition. */
#define AD5755_STATUS_DC_DC_D           (1 << 15)
#define AD5755_STATUS_DC_DC_C           (1 << 14)
#define AD5755_STATUS_DC_DC_B           (1 << 13)
#define AD5755_STATUS_DC_DC_A           (1 << 12)
#define AD5755_STATUS_USER_BIT          (1 << 11)
#define AD5755_STATUS_PEC_ERROR         (1 << 10)
#define AD5755_STATUS_RAMP_ACTIVE       (1 << 9)
#define AD5755_STATUS_OVER_TEMP         (1 << 8)
#define AD5755_STATUS_VOUT_D_FAULT      (1 << 7)
#define AD5755_STATUS_VOUT_C_FAULT      (1 << 6)
#define AD5755_STATUS_VOUT_B_FAULT      (1 << 5)
#define AD5755_STATUS_VOUT_A_FAULT      (1 << 4)
#define AD5755_STATUS_IOUT_D_FAULT      (1 << 3)
#define AD5755_STATUS_IOUT_C_FAULT      (1 << 2)
#define AD5755_STATUS_IOUT_B_FAULT      (1 << 1)
#define AD5755_STATUS_IOUT_A_FAULT      (1 << 0)

#define AD5755_CRC_POLYNOMIAL   0x07	// P(x)=x^8+x^2+x^1+1 = 100000111
#define AD5755_CRC_CHECK_CODE   0x00

#define FLASHADDRESSLEN        4
#define CALIBRATIONADADDRESS   0x0800DA00 

typedef struct AD5755_InitialSettings
{
    unsigned char pinAD0state;
    unsigned char pinAD1state;
    unsigned char enablePacketErrorCheck;
    
    /* Main Control Register bits */
    unsigned char pocBit;
    unsigned char statReadBit;
    unsigned char shtCcLimBit;
    unsigned char ewdEnable;
    
    /* DAC Control Registers: A, B, C and D */
    unsigned char rsetBits[4];
    unsigned char ovrngBits[4];
    
    /* DC-to-DC Control Register */
    unsigned char dcDcCompBit;
    unsigned char dcDcPhaseBit;
    unsigned char dcDcFreqBit;
    unsigned char dcDcMaxVBit;
    
    
} AD5575_Setup;

extern AD5575_Setup AD5575_st;
extern int8_t RangeSwitchingFlagBit;

/******************************************************************
*                           数据类型                              *
******************************************************************/
typedef enum
{
    E_STM32_AO_SPI_OK = 0,                       /*AO模块SPI驱动成功*/
    E_STM32_AO_SPI_DRIVER_PARA_ERROR,            /*函数入参错误*/
    E_STM32_AO_SPI_TXFIFO_NOT_EMPTY,             /*发送BUFFER不为空*/
    E_STM32_AO_SPI_BSY_ERROR,                    /*BSY没有置位*/
    E_STM32_AO_SPI_DRIVER_ERROR_BUTT
}STM32AOSPIDriverError_e;

typedef enum
{
    E_STM32_AO_SPI_A = 0,/*初值是根据DAC输入数据格式确定*/
    E_STM32_AO_SPI_B,
    E_STM32_AO_SPI_C,
    E_STM32_AO_SPI_D,
    E_STM32_AO_SPI_CHANNEL_BUTT
}STM32AOSPIChannel_e;

/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
STM32AOSPIDriverError_e STM32DAC5755DriverInit();
void STM32DAC8555DriverOutput(void);
void AD5755_GPIO_CONFIG(void);
void AD5755_Init(void);
STM32AOSPIDriverError_e STM32AD5755SendData(uint8_t ucCurrentChannel);
void TIM2_IRQHandler(void);
float AD5755_SetVoltage(unsigned char channel, float voltage);
float AD5755_SetCurrent(unsigned char channel, float mACurrent);
unsigned short AD5755_SetRegisterValue(unsigned char registerAddress,
                                       unsigned char channel,
                                       unsigned long registerValue);
unsigned short AD5755_SetRegisterValue2(unsigned char registerAddress,
                                       unsigned char channel,
                                       unsigned long registerValue);
unsigned char AD5755_CheckCrc(unsigned char* data, unsigned char bytesNumber);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void AD5755_Software_Reset(void);
void AD5755_SetControlRegisters(unsigned char  ctrlRegAddress,
                                unsigned char  channel,
                                unsigned short regValue);
void AD5755_SetControlRegisters2(unsigned char  ctrlRegAddress,
                                unsigned char  channel,
                                unsigned short regValue);
void AD5755_PEC_ENABLE(void);
long AD5755_GetRegisterValue(unsigned char registerAddress);
long AD5755_GetRegisterValue2(unsigned char registerAddress);
void AD5755_SetChannelRange(unsigned char channel, unsigned char range);
void AD5755_ChannelClearEnable(unsigned char channel, unsigned char clearEn);
void AD5755_ParseStatusRegister(void);
void AD5755_ParseStatusRegister2(void);
void UpdateIndicatorLights();
void TIM7_Config(void);
void TIM7_IRQHandler(void);
#endif


