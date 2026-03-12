/******************************************************************
*
*文件名称:  STM32F0Ads8331Driver.c
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
#include "STM32F0Ads8331Driver.h"
#include "Iocontrol.h"
#include "common.h"


#ifdef SCAN_HMODBUS_OPTIMIZE
#include"HModbusOptimize.h"
#endif
#if FR3XX4_CODE 
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
*                           全局变量声明                          *
******************************************************************/
#if FR3XX4_CODE && (IO_CODE_TYPE != FR3924_CODE)&& (IO_CODE_TYPE != FR3822_CODE)

uint16_t SPI_AI_RxBuffer[SPI_AI_RXBUFFER_SIZE] = {0};   
uint16_t SPI_AI_TxBuffer[SPI_AI_TXBUFFER_SIZE] = {R_DATA,R_DATA};
#endif

#if TEMPERATURE_MODULE_CODE
uint8_t SPI_AI_RxBuffer[SPI_AI_RXBUFFER_SIZE] = {0};   
uint8_t SPI_AI_TxBuffer[SPI_AI_TXBUFFER_SIZE] = {R_DATA,R_DATA};
#endif
/* 通道个数*/
extern uint8_t gucChannelCount;

/* AI原始采集数据*/
extern uint16_t gusAIOriginalSampleData[MAX_AI_CHANNEL_NUMBER * DMA_BUFFER_SIZE_PER_SAMPLE];

/* AI原始采集数据,大小为DMA_BUFFER_SIZE_PER_SAMPLE * gucChannelCount */
extern uint16_t gusAIOriginalSampleCount;

#ifdef SCAN_HMODBUS_OPTIMIZE
#if (FR3XX4_CODE ||(IO_CODE_TYPE == FR1008_CODE))&& (IO_CODE_TYPE != FR3924_CODE)&& (IO_CODE_TYPE != FR3822_CODE)
extern SpiSampleDataStatus_e geSpiSampleDataStatus;
extern int32_t giaSPIAIFilterHandleBuffer[SPI_AI_RXBUFFER_SIZE];
#endif
#if FR3XX4_CODE && (IO_CODE_TYPE != FR3924_CODE) && (IO_CODE_TYPE != FR3822_CODE)
extern uint16_t gusaSPIAIFilterHandleBuffer[SPI_AI_RXBUFFER_SIZE];   
#endif

#endif
/******************************************************************
*                         全局函数声明                            *
******************************************************************/

#if FR3XX4_CODE && (IO_CODE_TYPE != FR3924_CODE)&& (IO_CODE_TYPE != FR3822_CODE)
/******************************************************************
*函数名称:TIM_Config
*功能描述:进行TIM初始化，使能输入捕获模式，
                         触发DMA发送
*输入参数:无
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/01/05                            yangwenfan  we014
******************************************************************/
static void TIM_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = TIM_ARR;
    TIM_TimeBaseStructure.TIM_Prescaler = TIM_AI_Prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_AI_ClockDivision;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = TIM_AI_RepetitionCounter;
    TIM_TimeBaseInit(TIM_AI, &TIM_TimeBaseStructure);
    /* Time ICInit configuration */
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3 ;
    TIM_ICInitStructure.TIM_ICFilter =TIM_AI_ICFilter ;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling ;
    TIM_ICInitStructure.TIM_ICPrescaler= TIM_AI_ICPrescaler ;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInit(TIM_AI, &TIM_ICInitStructure );
}



/******************************************************************
*函数名称:STM32F0Ads8331DriverConfig
*功能描述:配置SPI初始化参数
*输入参数: SPI_InitStructure              SPI初始化结构体
                         GPIO_InitStructure             GPIO初始化结构体
                         GPIO_MOSIInitStructure     MOSI初始化结构体
                         NVIC_InitStructure             中断控制初始化结构体
                         DMA_RXInitStructure          DMA接收初始化结构体
                         DMA_TXInitStructure          DMA发送初始化结构体
*输出参数:
*返回值: E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR           配置参数错误
                    E_STM32F0_AI_SPI_OK                                                     运行成功
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/01/04                            yangwenfan we014
******************************************************************/
STM32F0_SPI_AI_DRIVER_ERROR_e STM32F0Ads8331DriverConfig(SPI_InitTypeDef  *SPI_InitStructure, GPIO_InitTypeDef  *GPIO_InitStructure, 
                                                         GPIO_InitTypeDef  *GPIO_LOWInitStructure, NVIC_InitTypeDef *NVIC_InitStructure,
                                                         DMA_InitTypeDef  *DMA_RXInitStructure, DMA_InitTypeDef  *DMA_TXInitStructure)
{
     /*判断SPI初始化指针是否为空*/
    if (NULL == SPI_InitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
     /*判断GPIO初始化指针是否为空*/
    if (NULL == GPIO_InitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断MOSI初始化指针是否为空*/
    if (NULL == GPIO_LOWInitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断NVIC初始化指针是否为空*/
    if (NULL == NVIC_InitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断DMA接收结构体指针是否为空*/
    if (NULL == DMA_RXInitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断DMA发送结构体指针是否为空*/
    if (NULL == DMA_TXInitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }

    /* Enable the SPI periph */
    RCC_APB2PeriphClockCmd(SPI_AI_CLK, ENABLE);
    
    /* Enable the TIM peripheral */
    RCC_APB1PeriphClockCmd(TIM_AI_CLK, ENABLE);
   
    /* Enable the DMA peripheral */
    RCC_AHBPeriphClockCmd(DMA_AI_CLK, ENABLE);
    
    /* Enable TIM DMA trigger clock */ 
    RCC_AHBPeriphClockCmd(TIM_AI_TRIGGER_GPIO_CLK, ENABLE);

    /* Enable SCK, MOSI, MISO and NSS GPIO clocks */
    RCC_AHBPeriphClockCmd(SPI_AI_SCK_GPIO_CLK | SPI_AI_MISO_GPIO_CLK | SPI_AI_MOSI_GPIO_CLK | SPI_AI_NSS_GPIO_CLK | TIM_AI_TRIGGER_GPIO_CLK, ENABLE);

    /*复用管脚配置*/
    GPIO_PinAFConfig(SPI_AI_SCK_GPIO_PORT, SPI_AI_SCK_SOURCE, SPI_AI_SCK_AF);
    GPIO_PinAFConfig(SPI_AI_MOSI_GPIO_PORT, SPI_AI_MOSI_SOURCE, SPI_AI_MOSI_AF);
    GPIO_PinAFConfig(SPI_AI_MISO_GPIO_PORT, SPI_AI_MISO_SOURCE, SPI_AI_MISO_AF);
    GPIO_PinAFConfig(SPI_AI_NSS_GPIO_PORT, SPI_AI_NSS_SOURCE, SPI_AI_NSS_AF);
    GPIO_PinAFConfig(TIM_AI_TRIGGER_GPIO_PORT, TIM_AI_TRIGGER_SOURCE, GPIO_AF_2);

    /*GPIO初始化参数配置*/
    GPIO_InitStructure->GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure->GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure->GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure->GPIO_Speed = GPIO_Speed_Level_3;

    /*MOSI初始化参数配置*/
    GPIO_LOWInitStructure->GPIO_Mode = GPIO_Mode_OUT;
    GPIO_LOWInitStructure->GPIO_OType = GPIO_OType_PP;
    GPIO_LOWInitStructure->GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_LOWInitStructure->GPIO_Speed = GPIO_Speed_Level_3;

    /* SPI SCK pin configuration */
    GPIO_InitStructure->GPIO_Pin = SPI_AI_SCK_PIN;
    GPIO_Init(SPI_AI_SCK_GPIO_PORT, GPIO_InitStructure);

    /* SPI  MOSI pin configuration */
    GPIO_InitStructure->GPIO_Pin = SPI_AI_MOSI_PIN;
    GPIO_Init(SPI_AI_MOSI_GPIO_PORT, GPIO_InitStructure);

    /* SPI MISO pin configuration */
    GPIO_InitStructure->GPIO_Pin = SPI_AI_MISO_PIN;
    GPIO_Init(SPI_AI_MISO_GPIO_PORT, GPIO_InitStructure);

    /* SPI NSS pin configuration */
    GPIO_InitStructure->GPIO_Pin = SPI_AI_NSS_PIN;
    GPIO_Init(SPI_AI_NSS_GPIO_PORT, GPIO_InitStructure);

    /*EOC触发引脚配置*/
    GPIO_InitStructure->GPIO_Pin = TIM_AI_TRIGGER_PIN;   
    GPIO_InitStructure->GPIO_PuPd  = GPIO_PuPd_NOPULL;    
    GPIO_Init(TIM_AI_TRIGGER_GPIO_PORT, GPIO_InitStructure);

    /*DMA中断初始化*/
    NVIC_InitStructure->NVIC_IRQChannel = DMA1_Channel2_3_IRQn;
    NVIC_InitStructure->NVIC_IRQChannelPriority = SPI_AI_DMAIRQSPriority;
    NVIC_InitStructure->NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(NVIC_InitStructure);

    /*配置SPI初始化参数*/
    SPI_InitStructure->SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure->SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure->SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure->SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure->SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStructure->SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure->SPI_CRCPolynomial = SPI_AI_CRCPolynomial;
    SPI_InitStructure->SPI_Mode = SPI_Mode_Master;

    /*配置DMA接收初始化参数*/
    DMA_RXInitStructure->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_RXInitStructure->DMA_MemoryDataSize =  DMA_MemoryDataSize_Byte;
    DMA_RXInitStructure->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_RXInitStructure->DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_RXInitStructure->DMA_Mode = DMA_Mode_Circular;
    DMA_RXInitStructure->DMA_M2M = DMA_M2M_Disable;
    DMA_RXInitStructure->DMA_BufferSize = SPI_AI_DMA_RECEIVESIZE;
    DMA_RXInitStructure->DMA_PeripheralBaseAddr = (uint32_t)SPI_AI_DR_ADDRESS;
    DMA_RXInitStructure->DMA_MemoryBaseAddr = (uint32_t)SPI_AI_RxBuffer;
    DMA_RXInitStructure->DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_RXInitStructure->DMA_Priority = DMA_Priority_High;

    /*配置DMA发送初始化参数*/
    DMA_TXInitStructure->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_TXInitStructure->DMA_MemoryDataSize =  DMA_MemoryDataSize_Byte;
    DMA_TXInitStructure->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_TXInitStructure->DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_TXInitStructure->DMA_Mode = DMA_Mode_Circular;
    DMA_TXInitStructure->DMA_M2M = DMA_M2M_Disable;
    DMA_TXInitStructure->DMA_BufferSize = SPI_AI_DMA_SENDSIZE;
    DMA_TXInitStructure->DMA_PeripheralBaseAddr = (uint32_t)SPI_AI_DR_ADDRESS;
    DMA_TXInitStructure->DMA_MemoryBaseAddr = (uint32_t)SPI_AI_TxBuffer;
    DMA_TXInitStructure->DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_TXInitStructure->DMA_Priority = DMA_Priority_Low;

    return E_STM32F0_AI_SPI_OK;
}

/******************************************************************
*函数名称:STM32F0Ads8331DriverInit
*功能描述:进行SPI初始化
*输入参数:SPI_InitStructure              SPI初始化结构体
                         GPIO_InitStructure             GPIO初始化结构体
                         GPIO_MOSIInitStructure     MOSI初始化结构体
                         NVIC_InitStructure             中断控制初始化结构体
                         DMA_RXInitStructure          DMA接收初始化结构体
                         DMA_TXInitStructure          DMA发送初始化结构体
*输出参数:
*返回值:E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR           参数配置错误
                   E_STM32F0_AI_SPI_OK                                                     运行成功
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/01/04                            yangwenfan  we014
******************************************************************/
STM32F0_SPI_AI_DRIVER_ERROR_e STM32F0Ads8331DriverInit(SPI_InitTypeDef  *SPI_InitStructure, GPIO_InitTypeDef  *GPIO_InitStructure, 
                                                      GPIO_InitTypeDef  *GPIO_LOWInitStructure, NVIC_InitTypeDef *NVIC_InitStructure,
                                                      DMA_InitTypeDef  *DMA_RXInitStructure, DMA_InitTypeDef  *DMA_TXInitStructure)
{
     /*判断SPI初始化指针是否为空*/
    if (NULL == SPI_InitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断GPIO初始化指针是否为空*/
    if (NULL == GPIO_InitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断MOSI初始化指针是否为空*/
    if (NULL == GPIO_LOWInitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断NVIC初始化指针是否为空*/
    if (NULL == NVIC_InitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断DMA接收结构体指针是否为空*/
    if (NULL == DMA_RXInitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }
    /*判断DMA发送结构体指针是否为空*/
    if (NULL == DMA_TXInitStructure)
    {
        return E_STM32F0_AI_SPI_CONFIGURATION_PARA_ERROR;
    }

    /*Deinitializes the SPIx peripheral registers to their default reset values*/
    SPI_I2S_DeInit(SPI_AI);

    /* Initializes the SPI communication */
    SPI_Init(SPI_AI, SPI_InitStructure);

    /*使能nss脉冲*/
    SPI_NSSPulseModeCmd(SPI_AI, ENABLE);

    /* Initialize the FIFO threshold */
    SPI_RxFIFOThresholdConfig(SPI_AI, SPI_RxFIFOThreshold_QF);

    /*Deinitializes theTIM2 peripheral registers to their default reset values*/
    TIM_DeInit(TIM_AI);

    /*定时器配置初始化*/
    TIM_Config(); 

    /*使能SPI*/
    SPI_Cmd(SPI_AI, ENABLE);

    /*对ADS芯片进行复位操作,从发出复位命令到下一条对ADS指令，
        延时需要达到2us，根据目前代码，实际延时为5.8us*/
    SPI_I2S_SendData16(SPI_AI, RESET_CFR);

    /*使能DMA接收中断*/
    DMA_ITConfig(SPI_AI_RX_DMA_CHANNEL, DMA_IT_TE | DMA_IT_TC | DMA_IT_HT, ENABLE); 

    /*DMA初始化*/
    DMA_Init(SPI_AI_RX_DMA_CHANNEL, DMA_RXInitStructure);
    DMA_Init(SPI_AI_TX_DMA_CHANNEL, DMA_TXInitStructure);

    /*使能DMA请求*/
    SPI_I2S_DMACmd(SPI_AI, SPI_I2S_DMAReq_Rx, ENABLE);
    TIM_DMACmd(TIM_AI, TIM_AI_DMA_CHANNEL, ENABLE);

    
    /*使能DMA通道*/
    DMA_Cmd(SPI_AI_RX_DMA_CHANNEL, ENABLE);       
    DMA_Cmd(SPI_AI_TX_DMA_CHANNEL, ENABLE);

    /*读取ADS芯片CFR寄存器*/
    SPI_I2S_SendData16(SPI_AI, R_CFR);

    /*使能TIM*/
    TIM_Cmd(TIM_AI, ENABLE);
#ifdef SCAN_HMODBUS_OPTIMIZE
#if (FR3XX4_CODE ||(IO_CODE_TYPE == FR1008_CODE))&& (IO_CODE_TYPE != FR3924_CODE)&& (IO_CODE_TYPE != FR3822_CODE)
    geSpiSampleDataStatus = E_SPI_SAMPLE_DATA_INIT;
#endif
#endif
    return E_STM32F0_AI_SPI_OK;
}

/******************************************************************
*函数名称:STM32F0Ads8331DriverStart
*功能描述:启动AI SPI采集
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/8                              lv we004
******************************************************************/
void STM32F0Ads8331DriverStart(void)
{
    /*发送ADS芯片配置信息*/
    SPI_I2S_SendData16(SPI_AI, ADSCMF_CONFIG);
}

/******************************************************************
*函数名称:DMA1_Channel2_3_IRQHandler
*功能描述:dma接收中断
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/01/04                            yangwenfan  we014
*2018/08/04                            gaojunfeng  we035
******************************************************************/
void DMA1_Channel2_3_IRQHandler(void)
{
    uint16_t *pusOriginalSampleData = SPI_AI_RxBuffer;
    uint8_t ucCopyCount;
#ifdef SCAN_HMODBUS_OPTIMIZE
    uint32_t *uiDestBuffer;/*目的buffer指针*/
    uint32_t *uiSourceBuffer;/*源buffer指针*/
#endif
    if (DMA_GetITStatus(DMA1_IT_TC2))
    {   
        /*清除全局中断*/
        DMA_ClearFlag(DMA1_IT_GL2);

        /* 保存采集的原始数据 */
        pusOriginalSampleData += gusAIOriginalSampleCount;
    }   
    else if (DMA_GetITStatus(DMA1_IT_HT2))
    {   
        /*清除接收half中断*/
        DMA_ClearFlag(DMA1_IT_HT2);
    }
    else
    {
        /*异常中断返回*/
        return;
    }
    
#ifdef SCAN_HMODBUS_OPTIMIZE
    uiDestBuffer = (uint32_t *)gusaSPIAIFilterHandleBuffer;
    uiSourceBuffer = (uint32_t *)pusOriginalSampleData;
    if (E_SPI_SAMPLE_DATA_HANDING != geSpiSampleDataStatus)
    {
        for (ucCopyCount = 0;ucCopyCount < SPI_AI_RXBUFFER_SIZE/4; ++ucCopyCount)
        {
            *(uiDestBuffer+ucCopyCount) = *(uiSourceBuffer+ucCopyCount);
        }
        geSpiSampleDataStatus = E_SPI_SAMPLE_DATA_UPDATED;
    }
#endif
}
#endif

/******************************************************************
*函数名称:SPIFilterHandle
*功能描述:SPI采集数据滤波处理
*输入参数:pusSPISampleDataBufferIn SPI采集数据缓存指针
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/08/04                            gaojunfeng  we035
******************************************************************/
void SPIFilterHandle(uint16_t *pusSPISampleDataBufferIn)
{
    switch(gucChannelCount)
    {
    case AI_CHANNEL_NUMBER_2:
    {
        ArithmeticAverageFilter2Channel(pusSPISampleDataBufferIn);
        break;
    }
    case AI_CHANNEL_NUMBER_4:
    {
        ArithmeticAverageFilter4Channel(pusSPISampleDataBufferIn);
        break;
    }
    case AI_CHANNEL_NUMBER_8:
    {
        ArithmeticAverageFilter8Channel(pusSPISampleDataBufferIn);
        break;
    }
    default:
    {
        break;
    }
    }
}

#endif  //#if  FR3XX4_CODE


