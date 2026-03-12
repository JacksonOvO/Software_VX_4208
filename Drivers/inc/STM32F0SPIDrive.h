/*******************************************************************
*文件名称:  STM32F0SPIDrive.h
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

#ifndef __STM32F0_SPI_H
#define __STM32F0_SPI_H

/******************************************************************
*                             头文件                              *
******************************************************************/
#include "stm32f30x.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_spi.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_rcc.h"
/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
#define DAC_MCU_LDAC_PIN                         GPIO_Pin_12
#define DAC_LDAC_GPIO_PORT                       GPIOB
#define DAC_LDAC_GPIO_CLK                        RCC_AHBPeriph_GPIOB

#define DAC_MCU_CLEAR_PIN                        GPIO_Pin_14
#define DAC_CLEAR_GPIO_PORT                     GPIOB
#define DAC_CLEAR_GPIO_CLK                      RCC_AHBPeriph_GPIOB
                       
#define DAC_MCU_SYNC_PIN                         GPIO_Pin_15
#define DAC_SYNC_GPIO_PORT                       GPIOA   
#define DAC_SYNC_GPIO_CLK                        RCC_AHBPeriph_GPIOA

#define DAC_MCU_SYNC2_PIN                         GPIO_Pin_11
#define DAC_SYNC2_GPIO_PORT                       GPIOA   
#define DAC_SYNC2_GPIO_CLK                        RCC_AHBPeriph_GPIOA

#define DAC_MCU_SPI_MOSI_PIN                     GPIO_Pin_5
#define DAC_MOSI_GPIO_PORT                       GPIOB
#define DAC_MOSI_GPIO_CLK                        RCC_AHBPeriph_GPIOB
#define DAC_MOSI_SOURCE                          GPIO_PinSource5
#define DAC_MOSI_AF                              GPIO_AF_0    

#define DAC_MCU_SPI_SCK_PIN                      GPIO_Pin_3
#define DAC_SCK_GPIO_PORT                        GPIOB
#define DAC_SCK_GPIO_CLK                         RCC_AHBPeriph_GPIOB
#define DAC_SCK_SOURCE                           GPIO_PinSource3    
#define DAC_SCK_AF                               GPIO_AF_0 

/*DAC芯片的SYNC管脚*/
/*置DAC1芯片的SYNC管脚为高*/
#define SET_SYNC_HIGH()                          GPIO_SetBits(DAC_SYNC_GPIO_PORT, DAC_MCU_SYNC_PIN)
/*置DAC1芯片的SYNC管脚为低*/
#define SET_SYNC_LOW()                           GPIO_ResetBits(DAC_SYNC_GPIO_PORT, DAC_MCU_SYNC_PIN)

/*置DAC2芯片的SYNC管脚为高*/
#define SET_SYNC2_HIGH()                          GPIO_SetBits(DAC_SYNC2_GPIO_PORT, DAC_MCU_SYNC2_PIN)
/*置DAC2芯片的SYNC管脚为低*/
#define SET_SYNC2_LOW()                           GPIO_ResetBits(DAC_SYNC2_GPIO_PORT, DAC_MCU_SYNC2_PIN)

#define MOSIPIN                           (1 << 5)
#define CRCPolynomial                     (7)
#define RXSIZE                             (8)
#define KEEPLOWEIGHT                      (0xFF)
#define SYSTICKS                          (1.7 *SystemCoreClock / 10000)

#define RXBUFFSIZE                        (1)
#define IRQSYSPriority                   (0x0)
#define TURE                               (1)

/******************************************************************
*                           数据类型                              *
******************************************************************/
typedef enum
{
    E_STM32F0_DI_SPI_OK = 0,                               /*运行成功*/
    E_STM32F0_DI_SPI_CONFIGURATION_PARA_ERROR,          /*配置参数错误*/
    E_STM32_DI_SPI_DATA_BUFF_NULL_ERROR,                 /*缓存buffer为空*/
    E_STM32_DI_SPI_BUTT
}STM32F0_SPI_DRIVER_ERROR_e;

/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
void AD5755_SPI1_Init(void);
void SPI_Write(uint8_t slaveDeviceId, uint8_t* data, uint8_t bytesNumber);
void SPI_Read(uint8_t slaveDeviceId, uint8_t* readBuffer, uint8_t bytesNumber);
void SPI_Write2(uint8_t slaveDeviceId, uint8_t* data, uint8_t bytesNumber);
void SPI_Read2(uint8_t slaveDeviceId, uint8_t* readBuffer, uint8_t bytesNumber);

#endif

