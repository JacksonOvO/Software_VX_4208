/******************************************************************
*文件名称:  FR1118Driver.c
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

/******************************************************************
*                             头文件                              *
******************************************************************/
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "FR1118GPIODriver.h"
#include "hardware.h"
/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/

/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                         全局变量定义                            *
******************************************************************/

/******************************************************************
*                          局部函数声明                            *
******************************************************************/

/******************************************************************
*                         全局函数实现                            *
******************************************************************/
/******************************************************************
*函数名称:FR1118GPIOInit
*功能描述:初始化GPIO管脚
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/11                              we004 we014
******************************************************************/
void FR1118GPIOInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
    /*初始化采样管脚*/
    GPIO_InitStructure.GPIO_Pin = FR1118_DI_CHANNEL_0_GPIO_PIN;
    GPIO_Init(FR1118_DI_CHANNEL_0_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = FR1118_DI_CHANNEL_1_GPIO_PIN;
    GPIO_Init(FR1118_DI_CHANNEL_1_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = FR1118_DI_CHANNEL_2_GPIO_PIN;
    GPIO_Init(FR1118_DI_CHANNEL_2_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = FR1118_DI_CHANNEL_3_GPIO_PIN;
    GPIO_Init(FR1118_DI_CHANNEL_3_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = FR1118_DI_CHANNEL_4_GPIO_PIN;
    GPIO_Init(FR1118_DI_CHANNEL_4_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = FR1118_DI_CHANNEL_5_GPIO_PIN;
    GPIO_Init(FR1118_DI_CHANNEL_5_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = FR1118_DI_CHANNEL_6_GPIO_PIN;
    GPIO_Init(FR1118_DI_CHANNEL_6_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = FR1118_DI_CHANNEL_7_GPIO_PIN;
    GPIO_Init(FR1118_DI_CHANNEL_7_GPIO, &GPIO_InitStructure);
    /*初始化LED灯管脚*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
    
    GPIO_InitStructure.GPIO_Pin = FR1118_LED_CHANNEL_0_GPIO_PIN|FR1118_LED_CHANNEL_1_GPIO_PIN|FR1118_LED_CHANNEL_2_GPIO_PIN|FR1118_LED_CHANNEL_3_GPIO_PIN;
    GPIO_Init(FR1118_LED_CHANNEL_0_GPIO, &GPIO_InitStructure);
    GPIO_SetBits(FR1118_LED_CHANNEL_0_GPIO, FR1118_LED_CHANNEL_0_GPIO_PIN|FR1118_LED_CHANNEL_1_GPIO_PIN|FR1118_LED_CHANNEL_2_GPIO_PIN|FR1118_LED_CHANNEL_3_GPIO_PIN);

    GPIO_InitStructure.GPIO_Pin = FR1118_LED_CHANNEL_4_GPIO_PIN|FR1118_LED_CHANNEL_6_GPIO_PIN;
    GPIO_Init(FR1118_LED_CHANNEL_4_GPIO, &GPIO_InitStructure);
    GPIO_SetBits(FR1118_LED_CHANNEL_4_GPIO, FR1118_LED_CHANNEL_4_GPIO_PIN|FR1118_LED_CHANNEL_6_GPIO_PIN);

    GPIO_InitStructure.GPIO_Pin = FR1118_LED_CHANNEL_5_GPIO_PIN|FR1118_LED_CHANNEL_7_GPIO_PIN;
    GPIO_Init(FR1118_LED_CHANNEL_5_GPIO, &GPIO_InitStructure);
    GPIO_SetBits(FR1118_LED_CHANNEL_5_GPIO, FR1118_LED_CHANNEL_5_GPIO_PIN|FR1118_LED_CHANNEL_7_GPIO_PIN);
}

/******************************************************************
*函数名称:SetFR1118ChannelLed
*功能描述:点亮相应通道的LED
*输入参数:ucSampleChannelIn       输入的通道值
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/11                              we004 we014
******************************************************************/
void SetFR1118ChannelLed(uint8_t ucSampleChannelIn)
{
    switch (ucSampleChannelIn)
    {
    case FR1118_DI_CHANNEL_0:
    {
        GPIO_ResetBits(FR1118_LED_CHANNEL_0_GPIO, FR1118_LED_CHANNEL_0_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_1:
    {
        GPIO_ResetBits(FR1118_LED_CHANNEL_1_GPIO, FR1118_LED_CHANNEL_1_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_2:
    {
        GPIO_ResetBits(FR1118_LED_CHANNEL_2_GPIO, FR1118_LED_CHANNEL_2_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_3:
    {
        GPIO_ResetBits(FR1118_LED_CHANNEL_3_GPIO, FR1118_LED_CHANNEL_3_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_4:
    {
        GPIO_ResetBits(FR1118_LED_CHANNEL_4_GPIO, FR1118_LED_CHANNEL_4_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_5:
    {
        GPIO_ResetBits(FR1118_LED_CHANNEL_5_GPIO, FR1118_LED_CHANNEL_5_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_6:
    {
        GPIO_ResetBits(FR1118_LED_CHANNEL_6_GPIO, FR1118_LED_CHANNEL_6_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_7:
    {
        GPIO_ResetBits(FR1118_LED_CHANNEL_7_GPIO, FR1118_LED_CHANNEL_7_GPIO_PIN);
        break;
    }
    default:
    {
        break;
    }
    }
}

/******************************************************************
*函数名称:ReSetFR1118ChannelLed
*功能描述:熄灭相应通道LED
*输入参数:ucSampleChannelIn      输入的通道值
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/12                              we004 we014
******************************************************************/
void ReSetFR1118ChannelLed(uint8_t ucSampleChannelIn)
{
    switch (ucSampleChannelIn)
    {
    case FR1118_DI_CHANNEL_0:
    {
        GPIO_SetBits(FR1118_LED_CHANNEL_0_GPIO, FR1118_LED_CHANNEL_0_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_1:
    {
        GPIO_SetBits(FR1118_LED_CHANNEL_1_GPIO, FR1118_LED_CHANNEL_1_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_2:
    {
        GPIO_SetBits(FR1118_LED_CHANNEL_2_GPIO, FR1118_LED_CHANNEL_2_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_3:
    {
        GPIO_SetBits(FR1118_LED_CHANNEL_3_GPIO, FR1118_LED_CHANNEL_3_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_4:
    {
        GPIO_SetBits(FR1118_LED_CHANNEL_4_GPIO, FR1118_LED_CHANNEL_4_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_5:
    {
        GPIO_SetBits(FR1118_LED_CHANNEL_5_GPIO, FR1118_LED_CHANNEL_5_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_6:
    {
        GPIO_SetBits(FR1118_LED_CHANNEL_6_GPIO, FR1118_LED_CHANNEL_6_GPIO_PIN);
        break;
    }
    case FR1118_DI_CHANNEL_7:
    {
        GPIO_SetBits(FR1118_LED_CHANNEL_7_GPIO, FR1118_LED_CHANNEL_7_GPIO_PIN);
        break;
    }
    default:
    {
        break;
    }
    }
}

/******************************************************************
*                         局部函数实现                            *
******************************************************************/


