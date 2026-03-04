/******************************************************************
*
*文件名称:  STM32F0Ads8331Driver.h
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

#ifndef __STM32F0_SPI_AI_H
#define __STM32F0_SPI_AI_H

/******************************************************************
*                             头文件                              *
******************************************************************/
#include "stm32f30x.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_spi.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_dma.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_syscfg.h"
#include "IOOptions.h"  

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/* Communication boards SPIx Interface */
#define SPI_AI                             SPI1
#define SPI_AI_CLK                         RCC_APB2Periph_SPI1


#define SPI_AI_DR_ADDRESS                  0x4001300C   

#define DMA_AI_CLK                         RCC_AHBPeriph_DMA1

#if !TEMPERATURE_MODULE_CODE
#define SPI_AI_RX_DMA_CHANNEL              DMA1_Channel2
#define SPI_AI_TX_DMA_CHANNEL              DMA1_Channel1
#endif
#if TEMPERATURE_MODULE_CODE
#define SPI_AI_RX_DMA_CHANNEL              DMA1_Channel2
#define SPI_AI_TX_DMA_CHANNEL              DMA1_Channel3
#endif
#define SPI_AI_SCK_PIN                     GPIO_Pin_3
#define SPI_AI_SCK_GPIO_PORT               GPIOB
#define SPI_AI_SCK_GPIO_CLK                RCC_AHBPeriph_GPIOB
#define SPI_AI_SCK_SOURCE                  GPIO_PinSource3
#define SPI_AI_SCK_AF                      GPIO_AF_0
#if TEMPERATURE_MODULE_CODE
#define  ADS_START_CONTROL_PIN                   GPIO_Pin_9
#define  ADS_START_GPIO_PORT                GPIOB
#endif
#define SPI_AI_MISO_PIN                    GPIO_Pin_4
#define SPI_AI_MISO_GPIO_PORT              GPIOB
#define SPI_AI_MISO_GPIO_CLK               RCC_AHBPeriph_GPIOB
#define SPI_AI_MISO_SOURCE                 GPIO_PinSource4
#define SPI_AI_MISO_AF                     GPIO_AF_0
    
#define SPI_AI_MOSI_PIN                    GPIO_Pin_5
#define SPI_AI_MOSI_GPIO_PORT              GPIOB
#define SPI_AI_MOSI_GPIO_CLK               RCC_AHBPeriph_GPIOB
#define SPI_AI_MOSI_SOURCE                 GPIO_PinSource5
#define SPI_AI_MOSI_AF                       GPIO_AF_0
   
#define SPI_AI_NSS_PIN                    GPIO_Pin_15
#define SPI_AI_NSS_GPIO_PORT              GPIOA
#define SPI_AI_NSS_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define SPI_AI_NSS_SOURCE                 GPIO_PinSource15
#define SPI_AI_NSS_AF                     GPIO_AF_0

#define SPI_AI_CONVST_PIN                    GPIO_Pin_1
#define SPI_AI_CONVST_GPIO_PORT              GPIOA
#define SPI_AI_CONVST_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define SPI_AI_CONVST_SOURCE                 GPIO_PinSource1

#define SPI_AI_RESET_PIN                    GPIO_Pin_0
#define SPI_AI_RESET_GPIO_PORT              GPIOA
#define SPI_AI_RESET_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define SPI_AI_RESET_SOURCE                 GPIO_PinSource0

#define TIM_AI                             TIM2
#define TIM_AI_CLK                         RCC_APB1Periph_TIM2
#define TIM_AI_DMA_CHANNEL                 TIM_DMA_CC3

#define TIM_AI_TRIGGER_PIN                 GPIO_Pin_2
#define TIM_AI_TRIGGER_GPIO_PORT           GPIOA
#define TIM_AI_TRIGGER_GPIO_CLK            RCC_AHBPeriph_GPIOA
#define TIM_AI_TRIGGER_SOURCE              GPIO_PinSource2
#define TIM_AI_TRIGGER_AF                  GPIO_AF_2
#define TIM_ARR                          ((uint16_t)0x2EDF)
#define TIM_CCR                          ((uint16_t)0x1770)
#define TIM_AI_Prescaler                     (0x00)
#define TIM_AI_ClockDivision                (0x00)
#define TIM_AI_RepetitionCounter              (0x00)
#define TIM_AI_ICFilter                     (0)
#define TIM_AI_ICPrescaler                    (0)

#define SPI_AI_CRCPolynomial                     (7)

#define RESET_PIN_ACTIVE         (1 << 0)




#define MANUALCHANNEL                  (0x0000)
#define AUTOCHANNEL                    (0x0001 << 11)

#define USESCLK                         (0x0000)
#define USEOSC                          (0x0001 << 10)

#define AUTOTRIGGER                    (0x0000)
#define MANUALTRIGGER                  (0x0001 << 9)

#define RATE500K                        (0x0000)
#define RATE250K                      (0x0001 << 8)

#define EOCINTHIGH                     (0x0000)
#define EOCINTLOW                      (0x0001 << 7)

#define PIN10ASINT                    (0x0000)
#define PIN10ASEOC                  (0x0001 << 6)

#define PIN10ASCDI                    (0x0000)
#define PIN10ASINTEOC                  (0x0001 << 5)

#define AUTONAPENABLED                    (0x0000)
#define AUTONAPDISABLED                  (0x0001 << 4)

#define NAPENABLED                    (0x0000)
#define NAPDISABLED                   (0x0001 << 3)

#define DEEPENABLED                    (0x0000)
#define DEEPDISABLED                   (0x0001 << 2)

#define TAGDISABLED                    (0x0000)
#define TAGENABLED                  (0x0001 << 1)

#define SYSRESET                    (0x0000)
#define SYSOPERATION                  (0x0001 << 0)

#define ADSCMF_CONFIG  (W_CFR           \
                         |AUTOCHANNEL    \
                         |USEOSC         \
                         |AUTOTRIGGER    \
                         |RATE250K       \
                         |EOCINTLOW      \
                         |PIN10ASEOC       \
                         |PIN10ASINTEOC    \
                         |AUTONAPDISABLED  \
                         |NAPDISABLED      \
                         |DEEPDISABLED     \
                         |TAGDISABLED      \
                         |SYSOPERATION )

/******************************************************************
*                           数据类型                              *
******************************************************************/
typedef enum
{
    E_STM32F0_AI_SPI_OK = 0,                               /*运行成功*/
    E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR,          /*配置参数错误*/
    E_STM32_AI_SPI_BUTT
}STM32F0_SPI_AI_DRIVER_ERROR_e;

/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
STM32F0_SPI_AI_DRIVER_ERROR_e STM32F0Ads8331DriverConfig(SPI_InitTypeDef  *SPI_InitStructure, GPIO_InitTypeDef  *GPIO_InitStructure, 
                                                     GPIO_InitTypeDef  *GPIO_LOWInitStructure, NVIC_InitTypeDef *NVIC_InitStructure,
                                                     DMA_InitTypeDef  *DMA_RXInitStructure, DMA_InitTypeDef  *DMA_TXInitStructure);
STM32F0_SPI_AI_DRIVER_ERROR_e STM32F0Ads8331DriverInit(SPI_InitTypeDef  *SPI_InitStructure, GPIO_InitTypeDef  *GPIO_InitStructure, 
                                                 GPIO_InitTypeDef  *GPIO_LOWInitStructure, NVIC_InitTypeDef *NVIC_InitStructure,
                                                 DMA_InitTypeDef  *DMA_RXInitStructure, DMA_InitTypeDef  *DMA_TXInitStructure);
void STM32F0Ads8331DriverStart(void);
void SPIFilterHandle(uint16_t *pusSPISampleDataBufferIn);
#endif

