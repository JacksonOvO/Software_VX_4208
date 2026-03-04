/******************************************************************
*
*文件名称:  diagnose.c
*文件标识:
*内容摘要:
*其它说明:
*当前版本:
*作    者: 
*完成日期:  
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
#include "common.h"
#include "diagnose.h"
#include "warn.h"
#include "hardware.h"
#include "stm32f30x.h"
//#include "stm32f30x_syscfg.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_rcc.h"
#include "IOOptions.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_dma.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/

/******************************************************************
*                           数据类型                              *
******************************************************************/
ADCSamplechannel_e ucConvered = E_ADC_MCU_TEM_CHANNEL;
uint8_t ucADCStart = ADC_STOP;
/******************************************************************
*                         全局变量定义                            *
******************************************************************/
/* 诊断数据区，比特0表示ID为1的诊断 */
uint16_t gusaDiagnoseData[MAX_DIAGNOSE_DATA_NUM] = {0};

uint16_t            usADC1TemConvertedValue = 0;   /*温度采样值*/
uint16_t            usADC1VolConvertedValue = 0;      /*单片机电源电压采样值*/
/******************************************************************
*                         全局变量声明                            *
******************************************************************/
/* 告警数据区，比特0表示ID为1的告警*/
extern uint8_t gucaWarnData[MAX_WARN_DATA_NUM/BIT_NUM_PER_BYTE];

/******************************************************************
*                          局部函数声明                            *
******************************************************************/

/******************************************************************
*                         全局函数实现                            *
******************************************************************/

/******************************************************************
*函数名称:ADC1_Config
*功能描述:芯片内部温度实时诊断配置
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/6                             lwe004 we013
******************************************************************/
IO_InterfaceResult_e ADC1_Config( void )
{
//    uint8_t              ucTime = 0;
//    ADC_InitTypeDef      ADC_InitStructure;
//    NVIC_InitTypeDef     NVIC_InitStructure;
//    GPIO_InitTypeDef     GPIO_InitStructure;
    
    /*使能GPIOA 时钟*/
    RCC_AHBPeriphClockCmd(ADC_MCU_POWER_GPIO_RCC, ENABLE);
    
//    /*  配置电压通道采样引脚 */
//    if (IO_CODE_TYPE != FR1008_CODE)
//    {        
//        GPIO_InitStructure.GPIO_Pin = ADC_MCU_POWER_SAMPLE_PIN;
//    }
//    else
//    {
//        GPIO_InitStructure.GPIO_Pin = ADC_DI_MCU_POWER_SAMPLE_PIN;
//    }
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//    GPIO_Init(ADC_MCU_POWER_GPIO, &GPIO_InitStructure);
//
//    /* 使能ADC1 外部时钟APB2 */
//    RCC_AHBPeriphClockCmd(ADC_PERIPH_RCC, ENABLE);
//    /*ADC1时钟预分频为PCLK/4*/
//    RCC->CFGR |= ADC_TIME_PRESCALER;
//    /*选择ADC1时钟源为PCLK/4*/
//    RCC->CFGR3 |= ADC_TIME_SOURCE;
//    
//    /* 初始化ADC1外围寄存器，使它们恢复默认值 */  
//    ADC_DeInit(ADC1);
//    /*初始化化ADC 结构体 */
//    ADC_StructInit(&ADC_InitStructure);
//    
//    /*配置ADC 结构体 初始化ADC1 */
//    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
//    /*配置单次模式，由主循环轮循开启*/
//    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
//    ADC_InitStruct.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;  
//    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//    ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Backward;
//    ADC_Init(ADC1, &ADC_InitStructure); 
//
//    /* 使能ADC1 中断*/
//    NVIC_InitStructure.NVIC_IRQChannel = ADC1_COMP_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPriority = TEMPERATURE_WARN_ADC1_PRI;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    //NVIC_Init(&NVIC_InitStructure);
//    /*使能转换结束标记EOC 中断*/
//    //ADC1->IER |= ADC1_EOC_INTERRUPT_ENABLE;
//
//    /* 选择ADC1 温度转换通道和采样时间 时间为239 *1/12us,约为21.1us*/ 
//    ADC_ChannelConfig(ADC1, MCU_TEMPERATURE_AD_CHANNEL, MCU_AD_SAMPLE_CYCLE); 
//    /*选择单片机电源电压采样通道*/
//    if (IO_CODE_TYPE != FR1008_CODE)
//    {
//        ADC_ChannelConfig(ADC1, MCU_POWER_AD_CHANNEL, MCU_AD_SAMPLE_CYCLE); 
//    }
//    else
//    {
//        ADC_ChannelConfig(ADC1, MCU_DI_POWER_AD_CHANNEL, MCU_AD_SAMPLE_CYCLE);
//    }
//    /* 校准ADC1 ,校准ADC1_CALIBRATION_NUM 次*/
//    ucTime = ADC1_CALIBRATION_NUM;
//    while ( ucTime )
//    {
//         if ( ADC_GetCalibrationFactor( ADC1 ) )
//            {
//                break;
//            }
//         --ucTime;
//    }
//    if ( !ucTime )
//    {
//    }
//    /* 使能ADC1 外设*/    
//    ADC_Cmd(ADC1, ENABLE);
//    /*使能ADC1   过冲管理模式*/
//    ADC_OverrunModeCmd(ADC1, DISABLE);
//    /*使能温度传感器通道*/
//    ADC_TempSensorCmd( ENABLE );
//    /* 等待 ADC 使能标记,  等待ADC1_WAIT_START_FLAG_NUM 次 */
//    ucTime = ADC1_WAIT_START_FLAG_NUM;
//    while ( ucTime )
//    {
//         if ( SET == ADC_GetFlagStatus( ADC1, ADC_FLAG_ADEN ) )
//            {
//                break;
//            }
//         --ucTime;
//    }
//    if ( !ucTime )
//    {
//        return E_IO_ERROR_ADC1_CONFIG;
//    }
//    /* 启动ADC1 转换*/ 
//    ADC_StartOfConversion(ADC1);
//    
    return E_IO_ERROR_NONE;
}
/******************************************************************
*函数名称:ADC1_COMP_IRQHandler
*功能描述:ADC1 中断回调函数( EOC 中断)
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/6                             lwe004 we013
******************************************************************/
void ADC1_COMP_IRQHandler( void )
{
	
//    if ( ADC_GetFlagStatus( ADC1, ADC_FLAG_EOSEQ ) )
//    { 
//    	ADC_ClearFlag(ADC1, ADC_FLAG_EOSEQ);
//       switch( ucConvered )
//       {
//       case E_ADC_MCU_VOLTAGE_CHANNEL:
//       {
//            /*读取单片机电源电压采样值*/
//            usADC1VolConvertedValue = ADC_GetConversionValue(ADC1);
//            ucConvered = E_ADC_MCU_TEM_CHANNEL;
//            /*设置ADC 采样方向*/
//            ADC1->CFGR1 |= ADC_ScanDirection_Back;
//            ucADCStart = ADC_START;
//            break;
//        }
//        case E_ADC_MCU_TEM_CHANNEL:
//        {
//            /*读取温度采样转换值*/
//            usADC1TemConvertedValue =ADC_GetConversionValue(ADC1);   
//            ucConvered = E_ADC_MCU_VOLTAGE_CHANNEL;
//            /*设置ADC 采样方向*/
//            ADC1->CFGR1 &= ADC_ScanDirection_Up;
//            ucADCStart = ADC_START;
//            break;
//       }
//       default:
//       {
//        break;
//       }
//       }
//    }
}

/******************************************************************
*函数名称:ADC1GetSampleData
*功能描述:获取温度电压诊断数据
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/8/1                             gaojunfeng we035
******************************************************************/
void ADC1GetVolTmpDiagnosisData(void)
{
//    if ( ADC_GetFlagStatus( ADC1, ADC_FLAG_EOSEQ ) )
//    { 
//    	ADC_ClearFlag(ADC1, ADC_FLAG_EOSEQ);
//       switch( ucConvered )
//       {
//       case E_ADC_MCU_VOLTAGE_CHANNEL:
//       {
//            /*读取单片机电源电压采样值*/
//            usADC1VolConvertedValue = ADC_GetConversionValue(ADC1);
//            ucConvered = E_ADC_MCU_TEM_CHANNEL;
//            /*设置ADC 采样方向*/
//            ADC1->CFGR1 |= ADC_ScanDirection_Back;
//            ucADCStart = ADC_START;
//            break;
//        }
//        case E_ADC_MCU_TEM_CHANNEL:
//        {
//            /*读取温度采样转换值*/
//            usADC1TemConvertedValue =ADC_GetConversionValue(ADC1);   
//            ucConvered = E_ADC_MCU_VOLTAGE_CHANNEL;
//            /*设置ADC 采样方向*/
//            ADC1->CFGR1 &= ADC_ScanDirection_Up;
//            ucADCStart = ADC_START;
//            break;
//       }
//       default:
//       {
//        break;
//       }
//       }
//    }
}
/******************************************************************
*函数名称:ChipTempDiagnoseAndwarn
*功能描述:芯片温度告警诊断
*输入参数:无
*输出参数:
*返回值:E_IO_ERROR_ADC1_CONVERT_VALUE  ADC1 转换值非法
                    E_IO_ERROR_NONE 成功
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/5                             lwe004 we013
******************************************************************/
void ChipTempDiagnoseAndwarn( void )
{
    uint32_t            uitemperature = 0;
    uitemperature = usADC1TemConvertedValue;
        
    /* 计算采样电压值 ，为了浮点型不参与运算3.3 扩大10000倍*/
    //usADC1ConvertedVoltage = ( usADC1TemConvertedValue * ADC1_VOLTAGE_MAX ) / ADC1_CONVERT_VALUE_MAX;
    /*计算温度为了浮点型不参与运算1.43 扩大10000倍，0.0043扩大10000倍,计算出来为开尔文温度*/
    //  ustemperature = ( ADC1_V25VSENSE_VALUE - usADC1ConvertedVoltage ) / ADC1_TEMP_AVG_SLOPE + ADC1_TEMP_25 + KELVIN_TEMPERATURE;
    /* 温度计算使用移位、魔鬼数字，提高处理速度，具体公式见上面算法 */
    uitemperature = 630 - ((uitemperature * 1535) >> 13);
    gusaDiagnoseData[TEMPERATURE_DIAGNOSE_OFF] = (uint16_t )uitemperature;
    /*温度告警处理*/
    if ( ( uitemperature > KELVIN_MAX_TEMPERATURE_THRESHOLD ) || ( uitemperature < KELVIN_MIN_TEMPERATURE_THRESHOLD ) )
    {
        /*置 温度告警BIT 位*/
        ( *gucaWarnData) |= WARN_TEMPERATURE; 
    }
    else
    {
        /*清温度告警BIT 位*/
        ( *gucaWarnData) &= NOWARN_TEMPERATURE;     
    }
}
/******************************************************************
*函数名称:MCU_POWERVOLTAGE_DIAGNOSE
*功能描述:单片机电源诊断
*输入参数:无
*输出参数:无
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/14                              lwe004  we013
******************************************************************/
void MCU_POWERVOLTAGE_DIAGNOSE( void )
{
    uint32_t uiPowerValue = 0;
    uiPowerValue = usADC1VolConvertedValue;
    
    uiPowerValue = ( (uiPowerValue * 21120 ) >> 17 );
    gusaDiagnoseData[VOLTAGE_DIAGNOSE_OFF] = (uint16_t)uiPowerValue;
    
    if( ( uiPowerValue > MAX_MCU_POWER ) || ( uiPowerValue < MIN_MCU_POWER ) )
    {
         /*置 电源告警BIT 位*/
        ( *gucaWarnData) |= WARN_MCU_POWER; 
    }
    else
    {
        /*清电源告警BIT 位*/
        ( *gucaWarnData) &= NOWARN_MCU_POWER; 
    }
}
/******************************************************************
*函数名称:IODiagnose
*功能描述:IO 诊断
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/9                              lwe004  we013
******************************************************************/
void IODiagnose( void )
{
    /*芯片温度诊断告警*/
    ChipTempDiagnoseAndwarn(  );
    /*芯片电源电压诊断*/
    MCU_POWERVOLTAGE_DIAGNOSE( );
}




