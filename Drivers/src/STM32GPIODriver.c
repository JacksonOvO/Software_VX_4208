/******************************************************************
*
*文件名称:  STM32GPIODriver.c
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
#include <string.h>
#include "stm32f30x_rcc.h"
#include "stm32gpiodriver.h"

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
*函数名称:STM32AddressRecognition
*功能描述:地址识别函数
*输入参数:无
*输出参数:
*返回值:无
*其它说明:DI,DO模块地址设别管脚配置
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/20                              zhanghaifeng  we015
******************************************************************/
void STM32AddressRecognition(void)
{
    GPIO_InitTypeDef GPIOInitStruct;

    /*GPIO模块时钟配置 */
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOAEN, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOBEN, ENABLE);

    /*左侧地址识别电路初始化*/
    GPIOInitStruct.GPIO_Pin  = ADDRESS_RECOGNITION_LEFT_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    /*初始化GPIOInitStruct结构体*/
    GPIO_Init(ADDRESS_RECOGNITION_LEFT_GPIO, &GPIOInitStruct);

    /*右侧地址识别电路初始化*/
    GPIOInitStruct.GPIO_Pin  = ADDRESS_RECOGNITION_RIGHT_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    /*初始化GPIOInitStruct结构体*/
    GPIO_Init(ADDRESS_RECOGNITION_RIGHT_GPIO, &GPIOInitStruct);
}

/******************************************************************
*函数名称:STM32GPIOInputOutputSwitch
*功能描述:输入输出转换
*输入参数: GPIOx                    GPIO模块
                            eGPIOMode            GPIO模式
*输出参数:
*返回值:E_STM32_DO_GPIO_OK        成功
                     E_STM32_DO_GPIO_INPUT_OUTPUT_SWITCH_PARA_ERROR     入参错误
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/27                              zhanghaifeng  we015
******************************************************************/
STM32GPIODriverError_e STM32GPIOInputOutputSwitch (GPIO_TypeDef *GPIOx, uint16_t usGPIOPinNum, GPIOMode_TypeDef eGPIOMode)
{ 
    GPIO_InitTypeDef GPIOInitStruct;
    
    /*入参判断*/
    if (NULL == GPIOx)
    {
        return E_STM32_DO_GPIO_INPUT_OUTPUT_SWITCH_PARA_ERROR;
    }
    if (ZERO == usGPIOPinNum)
    {
        return E_STM32_DO_GPIO_INPUT_OUTPUT_SWITCH_PARA_ERROR;
    }
    
    /*GPIO结构体配置*/
    GPIOInitStruct.GPIO_Pin  = usGPIOPinNum;
    GPIOInitStruct.GPIO_Mode = eGPIOMode;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    
    /*初始化GPIOInitStruct结构体实现端口模式转换*/
    GPIO_Init(GPIOx, &GPIOInitStruct);

    return E_STM32_DO_GPIO_OK;
}

/*GPIO驱动除提供GPIO初始化函数和模式转换接口外，
    本驱动还提供GPIO输出输入接口供业务层调用，
    具体参见:输出函数GPIO_WriteBit,  输入函数GPIO_ReadInputDataBit*/

/******************************************************************
*                         局部函数实现                            *
******************************************************************/

