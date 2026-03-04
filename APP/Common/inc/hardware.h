/******************************************************************
*
*文件名称:  hardware.h
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

#ifndef __HARD_WARE_H__
#define __HARD_WARE_H__

/******************************************************************
*                             头文件                              *
******************************************************************/
#include "IOOptions.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/


#if FR4XX4_CODE || (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
#define ADDRESS_RECOGNITION_LEFT_GPIO   (GPIOA)         /* 地址识别电路左侧引脚模块 */
#define ADDRESS_RECOGNITION_LEFT_PIN    (GPIO_Pin_4)  /* 地址识别电路左侧引脚 */
#define ADDRESS_RECOGNITION_RIGHT_GPIO   (GPIOA)     /* 地址识别电路右侧引脚模块 */
#define ADDRESS_RECOGNITION_RIGHT_PIN    (GPIO_Pin_5)  /* 地址识别电路右侧引脚 */
#endif


#define INDICTOR_CHANNEL1_GPIO                      (GPIOA)        
#define INDICTOR_CHANNEL1_GPIO_PIN              (GPIO_Pin_7)     
#define INDICTOR_CHANNEL2_GPIO                      (GPIOB)          
#define INDICTOR_CHANNEL2_GPIO_PIN              (GPIO_Pin_0)
#define INDICTOR_CHANNEL3_GPIO                      (GPIOB)         
#define INDICTOR_CHANNEL3_GPIO_PIN              (GPIO_Pin_1)   
#define INDICTOR_CHANNEL4_GPIO                      (GPIOB)       
#define INDICTOR_CHANNEL4_GPIO_PIN              (GPIO_Pin_2)   
#define INDICTOR_CHANNEL5_GPIO                      (GPIOB)       
#define INDICTOR_CHANNEL5_GPIO_PIN              (GPIO_Pin_6)   
#define INDICTOR_CHANNEL6_GPIO                      (GPIOB)       
#define INDICTOR_CHANNEL6_GPIO_PIN              (GPIO_Pin_7) 
#define INDICTOR_CHANNEL7_GPIO                      (GPIOB)       
#define INDICTOR_CHANNEL7_GPIO_PIN              (GPIO_Pin_8) 
#define INDICTOR_CHANNEL8_GPIO                      (GPIOB)       
#define INDICTOR_CHANNEL8_GPIO_PIN              (GPIO_Pin_9) 
#define INDICTOR_ERROR_GPIO                      (GPIOA)       
#define INDICTOR_ERROR_GPIO_PIN              (GPIO_Pin_0) 

#define INDICTOR_LDAC_GPIO                      (GPIOB)         
#define INDICTOR_LDAC_GPIO_PIN              (GPIO_Pin_12)   
#define INDICTOR_CLEAR_GPIO                      (GPIOB)       
#define INDICTOR_CLEAR_GPIO_PIN              (GPIO_Pin_14) 

#define INDICTOR_LDAC2_GPIO                      (GPIOA)         
#define INDICTOR_LDAC2_GPIO_PIN              (GPIO_Pin_10)   
#define INDICTOR_CLEAR2_GPIO                      (GPIOA)       
#define INDICTOR_CLEAR2_GPIO_PIN              (GPIO_Pin_9) 

#if FR4XX4_CODE || (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
#define WATCH_DOG_GPIO  (GPIOA)
#define WATCH_DOG_PIN   (GPIO_Pin_2)
#endif


#if FR4XX4_CODE || (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
#define ADC_MCU_POWER_GPIO        ( GPIOB ) /*单片机电压采样通道引脚模块*/
#define ADC_MCU_POWER_SAMPLE_PIN     ( GPIO_Pin_1 ) /*DO,AO,AI电压采样通道引脚*/
#define MCU_POWER_AD_CHANNEL ( ADC_Channel_9 )  /*DO,AI,AO单片机电源电压采样通道*/
#endif

#define ADC_MCU_POWER_GPIO_RCC ( RCC_AHBPeriph_GPIOA ) /*单片机电压采样GPIO 时钟*/
#define ADC_PERIPH_RCC           ( RCC_APB2Periph_ADC1 )/*ADC1 时钟*/
#define ADC_DI_MCU_POWER_SAMPLE_PIN     ( GPIO_Pin_6 ) /*DI电压采样通道引脚*/
#define MCU_TEMPERATURE_AD_CHANNEL  ( ADC_Channel_16 )   /*AD 采集温度传感器通道*/
#define MCU_DI_POWER_AD_CHANNEL ( ADC_Channel_6 )  /*DI单片机电源电压采样通道*/
#define MCU_AD_SAMPLE_CYCLE  ( ADC_SampleTime_239_5Cycles )/*AD 采样周期*/
#define ADC_TIME_PRESCALER ( 0x00004000 ) /*AD时钟预分频*/
#define ADC_TIME_SOURCE     ( 0x00000100 ) /*AD 时钟源*/


/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/

#endif


