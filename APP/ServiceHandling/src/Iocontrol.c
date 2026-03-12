/******************************************************************
*
*文件名称:  Iocontrol.c
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
#include "common.h"
#include "IOOptions.h"
#include "Iocontrol.h"
#include "hardware.h"
#include "STM32GPIODriver.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_tim.h"
#include "stm32DAC8555driver.h"
#include "MbCommunication.h"
#include "warn.h"

#include "stdio.h"
#include "HModbusOptimize.h"
#include "STM32F0Ads8331Driver.h"
#include "STM32F0SPIDrive.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/* 通道0位置 */
#define DI_START_POSITION   (0)

/* 数据区起始位置 */
#define DATA_FIELD_START_POSITON    (0)

/* 百分比计算 */
#define PERCENT_100 (100)

/* 去0比特数值 */
#define GET_0_BIT_VALUE (0x01)

/* 定时器计数值,99表示100us一次中断 */
#define TIMER_PRESCALER1                 ( 99U )

#define TIMER_PRESCALER2                 ( 9999U )

#define TIMER_PRESCALER15                 ( 9999U )


/* 硬件主频 */
#define TIMER_XCLK                       ( 72000000U )
 
#define TIMER_MS2TICK( xTimeOut )      ( ( TIMER_XCLK * (long long)( xTimeOut ) ) /  (( TIMER_PRESCALER1 + 1) * 1000000U ) )

#define TIMER_MS2TICK2( xTimeOut )      ( ( TIMER_XCLK * (long long)( xTimeOut ) ) /  ( TIMER_PRESCALER2 + 1 ))

#define TIMER_TICK15( xTimeOut )      ( ( TIMER_XCLK * (long long)( xTimeOut ) ) /  ( TIMER_PRESCALER15 + 1 ))


/* AI SPI DMA使用传输一半中断 */
#define HARF_LENGTH (2)

/* AI通道定义 */
#define AI_CHANNEL_0    (0)
#define AI_CHANNEL_1    (1)
#define AI_CHANNEL_2    (2)
#define AI_CHANNEL_3    (3)
#define AI_CHANNEL_4    (4)
#define AI_CHANNEL_5    (5)
#define AI_CHANNEL_6    (6)
#define AI_CHANNEL_7    (7)


/* 以下宏为模拟量输入模块使用 */
/* 第一部分宏 : 标定 */
#define AI_CALIBRATION_MULTIPLIER   (10737)
#define AI_CALIBRATION_DIVIDEDER    (30)
   


#if IO_CODE_TYPE == FR4024_CODE
#define AO_CHANNEL_VOLTAGE_ZERO   (0)
#define AO_CHANNEL_VOLTAGE_TEN   (65535)
#endif


#define AI_INDICTOR_ON           (1)
#define AI_INDICTOR_OFF          (0)


/******************************************************************
*                         全局变量定义                            *
******************************************************************/
/*定时器启动标志*/
extern uint8_t ucIsDisconnectCheckTimerStart;

/* 通道个数*/
uint8_t gucChannelCount = 0;
extern uint8_t gucCurrentChannel;
extern uint8_t gucDisConnectionFlag;

/* 预设去抖动精度，预设为90%。采样周期为0.1ms，
去抖动时间为5ms，这样会采集到50个数据，
如果1的个数大于90%，则认为本次采样结果为1，
如果0的个数大于90%，则认为本次采样结果为0. */
uint8_t gucDebounceAccuracy  = 90;

/* AO数据区 */
#if FR4XX4_CODE 
int16_t gusaAnalogData[MAX_AO_CHANNEL_NUMBER] = {0};
int16_t gusaAnalogDataMainLoop[MAX_AO_CHANNEL_NUMBER] = {0};
int16_t test[MAX_AO_CHANNEL_NUMBER] = {0};
/* AO数据刷新标记，置位表示需要进行输出 */
uint8_t gucAoDataIsChanged = AO_DATA_NOT_CHANGED;
/*AO通道标定状态*/
AODemarcate_t gtaAODemarcate[MAX_AI_CHANNEL_NUMBER] = {0};

/* AI通道模式配置 */
uint8_t AISampleType[MAX_AI_CHANNEL_NUMBER] = {0};
uint8_t AIChannelSet[MAX_AI_CHANNEL_NUMBER] = {0};
int16_t AIValue[MAX_AI_CHANNEL_NUMBER] = {0};
uint8_t indicatorFlashState [5] = {0,0,0,0,0};

#ifdef SCAN_HMODBUS_OPTIMIZE
extern uint8_t gucDownServDataAnalogUpdateOK;
#endif
#endif

uint8_t locTEST = 0;
/* 告警数据区，比特0表示ID为1的告警*/
extern uint8_t gucaWarnData[MAX_WARN_DATA_NUM/BIT_NUM_PER_BYTE];

AIAOIndictor_t gtaIndictor[9] = 
{
    {0, E_AI_AO_INDICTOR_BUTT, INDICTOR_CHANNEL1_GPIO, INDICTOR_CHANNEL1_GPIO_PIN},
    {1, E_AI_AO_INDICTOR_BUTT, INDICTOR_CHANNEL2_GPIO, INDICTOR_CHANNEL2_GPIO_PIN},
    {2, E_AI_AO_INDICTOR_BUTT, INDICTOR_CHANNEL3_GPIO, INDICTOR_CHANNEL3_GPIO_PIN},
    {3, E_AI_AO_INDICTOR_BUTT, INDICTOR_CHANNEL4_GPIO, INDICTOR_CHANNEL4_GPIO_PIN},
    {4, E_AI_AO_INDICTOR_BUTT, INDICTOR_CHANNEL5_GPIO, INDICTOR_CHANNEL5_GPIO_PIN},
    {5, E_AI_AO_INDICTOR_BUTT, INDICTOR_CHANNEL6_GPIO, INDICTOR_CHANNEL6_GPIO_PIN},
    {6, E_AI_AO_INDICTOR_BUTT, INDICTOR_CHANNEL7_GPIO, INDICTOR_CHANNEL7_GPIO_PIN},
    {7, E_AI_AO_INDICTOR_BUTT, INDICTOR_CHANNEL8_GPIO, INDICTOR_CHANNEL8_GPIO_PIN},
    {8, E_AI_AO_INDICTOR_BUTT, INDICTOR_ERROR_GPIO, INDICTOR_ERROR_GPIO_PIN},
         
};

extern uint8_t gucaSpiRecvBuffer[100];

ModuleInfo_t ModuleInfo[4];
//uint8_t IOLinkEventInfo[16] = {0};
/******************************************************************
*                          局部函数声明                            *
******************************************************************/
void NVIC_Configuration(void);
static void SetAIAOLed(uint16_t *pusAnalogData);
/******************************************************************
*                         全局函数实现                            *
******************************************************************/
//static inline void GPIO_ToggleBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
//{
//    GPIOx->ODR ^= GPIO_Pin;  // 直接按位异或翻转
//}
//
//void IndicatorFlashHandler(void) { // 500ms调用
//    for (uint8_t ch = 0; ch < 5; ch++) {
//        if (indicatorFlashState[ch]) {
//            GPIO_ToggleBits(gtaIndictor[ch].GPIOx,
//                            gtaIndictor[ch].ulAIAOChennelPIN);
//        }
//    }
//}

void TIM6_Config(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    // 假设 APB1 定时器时钟 72MHz
    // 500ms 定时：72MHz / 7200 = 10kHz, 10kHz * 500 = 5000
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1; // 分频7200
    TIM_TimeBaseStructure.TIM_Period = 5000 - 1;    // 自动重装载值
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE); // 允许更新中断
    TIM_Cmd(TIM6, ENABLE); // 启动定时器

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM6_DAC_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

        // 闪烁处理：直接翻转 ODR
        for (uint8_t ch = 0; ch < 9; ch++)
        {
            if (indicatorFlashState[ch])
            {
                gtaIndictor[ch].GPIOx->ODR ^= gtaIndictor[ch].ulAIAOChennelPIN;
            }
        }
    }
}
/******************************************************************
*函数名称:initChannelOutput
*功能描述:为组态之前让输出电压/电流实际值为0
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
******************************************************************/

void initChannelOutput()
{
  /*
    memset(gusaAnalogDataMainLoop,0,sizeof(gusaAnalogDataMainLoop));
    for(uint8_t uiChannelNum = CHANNEL_INITIAL_VALUE; uiChannelNum < 4;uiChannelNum++)
    {
        if (SPI_I2S_GetFlagStatus(DAC_SPI, SPI_I2S_FLAG_BSY) == RESET)
        {


            (void)STM32AD5755SendData(uiChannelNum);	
        }
    }

    (void)STM32AD5755SendData(0);	
    */
}

void ZeroChannelOutput()
{
  
	for(uint8_t uiChannelNum = CHANNEL_INITIAL_VALUE; uiChannelNum < 4;uiChannelNum++)
	{
                gusaAnalogDataMainLoop[uiChannelNum] = 0 ;
		while (SPI_I2S_GetFlagStatus(DAC_SPI, SPI_I2S_FLAG_BSY) != RESET) //SPI正忙，停等
	    {
		}

		(void)STM32AD5755SendData(uiChannelNum);	
	}
   
}


/******************************************************************/
/*---------------------------------------------------
*函数名称:HandleIoData
*功能描述:
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
******************************************************************/
void HandleIoData(void)
{
    static uint16_t cycleCounter = 0; // 添加一个静态计数器来记录循环次数
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT;
    uint16_t gusaAnalogTempBuffer[MAX_AO_CHANNEL_NUMBER] = {0};

    FUNC_ENTER();

    if(gucDisConnectionFlag)
    {
        ZeroChannelOutput();
        gucDisConnectionFlag = 0;
    }

    if(AO_DATA_CHANGED != gucAoDataIsChanged)
    {
        return;
    }

    /* 如果AO数据未刷新则不进行GPIO输出 */
    if (E_DATA_SYNC_STATUS_UPDATED == gucDownServDataAnalogUpdateOK)
    {
        /*置状态机为处理态*/
        gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_MAINLOOP_HANDLING;

        /* 如果已刷新则更改刷新标记 */
        gucAoDataIsChanged = AO_DATA_NOT_CHANGED;

        memcpy(gusaAnalogDataMainLoop, gusaAnalogData, MAX_AO_CHANNEL_NUMBER);
        gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_HOLD;
        //eInterfaceResult = SetAoData(gusaAnalogTempBuffer);      
    }
    
    
    for(uint8_t uiChannelNum = CHANNEL_INITIAL_VALUE; uiChannelNum < 8;uiChannelNum++)
    {
        if (SPI_I2S_GetFlagStatus(DAC_SPI, SPI_I2S_FLAG_BSY) == RESET)
        {

            (void)STM32AD5755SendData(uiChannelNum);	
        }
    }
    
    AD5755_SetControlRegisters(AD5755_CREG_SOFT, 0, AD5755_SPI_CODE); 
    AD5755_SetControlRegisters2(AD5755_CREG_SOFT, 0, AD5755_SPI_CODE); 
    AD5755_ParseStatusRegister();
    AD5755_ParseStatusRegister2();
    UpdateIndicatorLights();
    
	
}

/******************************************************************
*函数名称:InitConfigDataField
*功能描述:给配置数据区分配空间
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
******************************************************************/
IO_InterfaceResult_e InitConfigDataField(void)
{
    return E_IO_ERROR_NONE;
}


/******************************************************************
*函数名称:DiSamplingTimerInit
*功能描述:初始化DI采样定时器，启动定时器
*输入参数:ustimeoutUS，定时器超时时长，单位为us
*输出参数:无
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/9                              lv we004
******************************************************************/
#if ((IO_CODE_TYPE == FR1008_CODE) || (FR4XX4_CODE ) || (IO_CODE_TYPE == FR1118_CODE) || (IO_CODE_TYPE == FR1108_CODE)|| (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE))
void DiSamplingTimerInit(uint32_t ustimeoutUS)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    memset(&TIM_TimeBaseStructure,0,sizeof(TIM_TimeBaseInitTypeDef));
    
    /* 使能定时器时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* Enable the TIM2 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
 //   NVIC_InitStructure.NVIC_IRQChannelPriority = DI_SAMPLING_TIME2_PRI;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* Time base configuration */
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructure.TIM_Prescaler = TIMER_PRESCALER1;
    TIM_TimeBaseStructure.TIM_Period = TIMER_MS2TICK(ustimeoutUS) - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

    TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );

    /* Enable timer 2 */
    TIM_Cmd( TIM2, ENABLE );
}
#endif
/******************************************************************
*函数名称:DisConnectWithGWTimerInit
*功能描述:判断与网关是否断连定时器，初始化定时器
*输入参数:ustimeoutUS，定时器超时时长，单位为s
*输出参数:无
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/2/25                              l we010
******************************************************************/
#if (FR2XX8_CODE || FR4XX4_CODE )||(IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
void DisConnectWithGWTimerInit(uint32_t ustimeoutUS)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    memset(&TIM_TimeBaseStructure,0,sizeof(TIM_TimeBaseInitTypeDef));
    
    /* 使能定时器时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* Enable the TIM3 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Time base configuration */
    TIM_DeInit(TIM3);
    TIM_TimeBaseStructure.TIM_Prescaler = TIMER_PRESCALER2;
    TIM_TimeBaseStructure.TIM_Period = TIMER_MS2TICK2(ustimeoutUS) - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);

    TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE );

    /* 使能定时器3 */
    /* 定时器在收到第一帧Modbus后使能 */
}
#endif

/******************************************************************
*函数名称:AO_AIIndicatorTimerInit
*功能描述:AI,AO指示灯,初始化定时器
*输入参数:ustimeoutUS，定时器超时时长，单位为s
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/3/11                              zhanghaifeng  we015
******************************************************************/
#if FR3XX4_CODE || FR4XX4_CODE || (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
void AO_AIIndicatorTimerInit(uint32_t ustimeoutUS)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    memset(&TIM_TimeBaseStructure,0,sizeof(TIM_TimeBaseInitTypeDef));

    /* 使能定时器时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);

    /* Time base configuration */
    TIM_DeInit(TIM15);
    TIM_TimeBaseStructure.TIM_Prescaler = TIMER_PRESCALER15;
    TIM_TimeBaseStructure.TIM_Period = TIMER_TICK15(ustimeoutUS) - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);

    TIM_SelectOutputTrigger(TIM15, TIM_TRGOSource_Update);

    TIM_Cmd(TIM15, ENABLE);
}
#endif

/******************************************************************
*函数名称:SetAoData
*功能描述:模拟量输出
*输入参数:pusAnalogData:模拟量数据区
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/6                              lv we004
******************************************************************/
#if FR4XX4_CODE || (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
IO_InterfaceResult_e SetAoData(uint16_t *pusAnalogData)
{        
    STM32DAC8555DriverOutput();

    /* 指示灯处理 */
   // SetAIAOLed(pusAnalogData);
    
    return E_IO_ERROR_NONE;
}
#endif


/******************************************************************
*函数名称:TIM3_IRQHandler
*功能描述:通用定时器3中断处理函数，数据全部复位
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/2/25                              l we010
******************************************************************/
#if (FR2XX8_CODE || FR4XX4_CODE)|| (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
#if FR4XX4_CODE|| (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
#ifdef SCAN_HMODBUS_OPTIMIZE 
        /*置状态机为处理态*/
        gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_MAINLOOP_HANDLING;
#endif
        memset(gusaAnalogData, 0, MAX_AO_CHANNEL_NUMBER);
        /* 更改AO数据刷新标志 */
        gucAoDataIsChanged = AO_DATA_CHANGED;

        //gucDisConnectionFlag = 1;
#ifdef SCAN_HMODBUS_OPTIMIZE        
        gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_UPDATED;
#endif    //SCAN_HMODBUS_OPTIMIZE
#endif
        /* 清除中断 */
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        /*禁能定时器*/
        TIM_Cmd(TIM3, DISABLE);
        if (indicatorFlashState[4] == 0)
            {
                GPIO_ResetBits(GPIOA,GPIO_Pin_0);
            }
        ucIsDisconnectCheckTimerStart = 0;
    }
}
#endif
#if (FR3XX4_CODE || FR4XX4_CODE|| (IO_CODE_TYPE == FR4124_CODE)|| (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)) && (IO_CODE_TYPE != FR3924_CODE)&& (IO_CODE_TYPE != FR3822_CODE)
/******************************************************************
*函数名称:SetAIAOLed
*功能描述:设置AIAO指示灯
*输入参数:无
*输出参数:无
*返回值:无
*其它说明:主循环调用
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/12/23                              lv we004
******************************************************************/
static void SetAIAOLed(uint16_t *pusAnalogData)
{
    uint8_t ucAIAOChannelNum = 0;
    static FlsahMode_e eFlsahMode = e_FLASH_OFF;

    
    /* 检测定时器是否超时 */
    if (TIM_GetFlagStatus(TIM15, TIM_FLAG_Update) != SET)
    {
        return;
    }

    /* 清除中断 */
    TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
    

#if FR4XX4_CODE
        /*根据当前gusaAnalogData值确认指示灯状态  gucChannelCount*/
        for(ucAIAOChannelNum = 0; ucAIAOChannelNum < gucChannelCount; ucAIAOChannelNum++)
        {
            if(AO_CHANNEL_VOLTAGE_ZERO == pusAnalogData[ucAIAOChannelNum])
            {
                gtaIndictor[ucAIAOChannelNum].eAIAOChennelStatus = E_AI_AO_INDICATOR_OFF;
            }
            else if((pusAnalogData[ucAIAOChannelNum] > AO_CHANNEL_VOLTAGE_ZERO)
#if ((IO_CODE_TYPE == FR4004_CODE) || (IO_CODE_TYPE == FR4014_CODE) || (IO_CODE_TYPE == FR4504_CODE) || (IO_CODE_TYPE == FR4514_CODE))                
                && (pusAnalogData[ucAIAOChannelNum] <= AO_CHANNEL_VOLTAGE_TEN)
#endif
                )
            {
                gtaIndictor[ucAIAOChannelNum].eAIAOChennelStatus = E_AI_AO_INDICTOR_FLASH;
            }
            else
            {
                gtaIndictor[ucAIAOChannelNum].eAIAOChennelStatus = E_AI_AO_INDICTOR_ON;
            }
            AIAOSetIndictor(ucAIAOChannelNum, eFlsahMode);
        }
#endif

    if (eFlsahMode == e_FLASH_OFF)
    {
        eFlsahMode = e_FLASH_ON;
    }
    else
    {
        eFlsahMode = e_FLASH_OFF;
    }
}
#endif


/******************************************************************
*函数名称:AIAOSetIndictor
*功能描述:根据状态置管脚灯
*输入参数:AIAOChannelNum   管脚号
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/3/11                              zhanghaifeng  we015
******************************************************************/
 void AIAOSetIndictor(uint8_t ucAIAOChannelNum, FlsahMode_e eFlsahMode)
{
    if (ucAIAOChannelNum >= 8) return; // 防止越界

    switch (eFlsahMode) {
        case 1:
            if(ucAIAOChannelNum != 8){
            GPIO_ResetBits(gtaIndictor[ucAIAOChannelNum].GPIOx,
                           gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN);
          }
            indicatorFlashState[ucAIAOChannelNum] = 0;
            break;

        case 0:
            if(ucAIAOChannelNum != 8){
            GPIO_SetBits(gtaIndictor[ucAIAOChannelNum].GPIOx,
                         gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN);
            }
            indicatorFlashState[ucAIAOChannelNum] = 0;
            break;

        case 2:
            // 闪烁逻辑交给定时器里切换状态
            // 这里只是标记为闪烁模式
            // 例如可以用一个数组记录状态
            indicatorFlashState[ucAIAOChannelNum] = 1;
            break;

        default:
            break;
    }
  
  
  
//    switch(ucAIAOChannelNum)
//    {
//     
//    case 0:
//    {
//        if (e_FLASH_OFF == eFlsahMode)
//         {
//           GPIO_SetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN); 
//         }
//         else
//         {
//           GPIO_ResetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN);
//           
//         }
//        break;
//    }  
//    case 1:
//    {
//         if (e_FLASH_OFF == eFlsahMode)
//         {
//           GPIO_SetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN); 
//         }
//         else
//        {
//           GPIO_ResetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN);
//           
//        }
//        break;
//    }
//    case 2:
//    {
//         if (e_FLASH_OFF == eFlsahMode)
//         {
//           GPIO_SetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN); 
//         }
//         else
//        {
//           GPIO_ResetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN);
//           
//        }
//        break;
//    }
//      
//    case 3:
//    {
//        if (e_FLASH_OFF == eFlsahMode)
//         {
//           GPIO_SetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN); 
//         }
//         else
//        {
//           GPIO_ResetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN);
//           
//        }
//        break;
//    }
//    case 4:
//    {
//         if (e_FLASH_OFF == eFlsahMode)
//         {
//           GPIO_SetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN); 
//         }
//         else
//         {
//           GPIO_ResetBits(gtaIndictor[ucAIAOChannelNum].GPIOx, gtaIndictor[ucAIAOChannelNum].ulAIAOChennelPIN);
//           
//         }
//        break;
//    }   
//
//    default:
//    {
//        return;
//    }
//    }
}



/******************************************************************
*函数名称:AIAOIndictorGPIOInit
*功能描述:AIAO指示灯管脚初始化
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/3/11                              zhanghaifeng  we015
******************************************************************/

void AIAOIndictorGPIOInit(void)
{
    GPIO_InitTypeDef GPIOInitStruct;

    /*GPIO模块时钟配置 */
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOAEN, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOBEN, ENABLE);
   

    /*初始化PA7管脚,指示灯通道1*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_CHANNEL1_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_CHANNEL1_GPIO, &GPIOInitStruct);

    /*初始化PB0管脚,指示灯通道2*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_CHANNEL2_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_CHANNEL2_GPIO, &GPIOInitStruct);

    /*初始化PB1管脚,指示灯通道3*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_CHANNEL3_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_CHANNEL3_GPIO, &GPIOInitStruct);

    /*初始化PB2管脚,指示灯通道4*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_CHANNEL4_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_CHANNEL4_GPIO, &GPIOInitStruct);
    
        /*初始化PB6管脚,指示灯通道5*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_CHANNEL5_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_CHANNEL5_GPIO, &GPIOInitStruct);
    
        /*初始化PB7管脚,指示灯通道6*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_CHANNEL6_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_CHANNEL6_GPIO, &GPIOInitStruct);
    
        /*初始化PB8管脚,指示灯通道7*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_CHANNEL7_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_CHANNEL7_GPIO, &GPIOInitStruct);
    
        /*初始化PB9管脚,指示灯通道8*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_CHANNEL8_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_CHANNEL8_GPIO, &GPIOInitStruct);
    
    /*初始化PA0管脚,ERR灯*/
    GPIOInitStruct.GPIO_Pin  = INDICTOR_ERROR_GPIO_PIN;
    GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT ;
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(INDICTOR_ERROR_GPIO, &GPIOInitStruct);

    
    /*配置*/
    GPIO_SetBits(INDICTOR_CHANNEL1_GPIO, INDICTOR_CHANNEL1_GPIO_PIN);
    GPIO_SetBits(INDICTOR_CHANNEL2_GPIO, INDICTOR_CHANNEL2_GPIO_PIN);
    GPIO_SetBits(INDICTOR_CHANNEL3_GPIO, INDICTOR_CHANNEL3_GPIO_PIN);
    GPIO_SetBits(INDICTOR_CHANNEL4_GPIO, INDICTOR_CHANNEL4_GPIO_PIN);
    GPIO_SetBits(INDICTOR_CHANNEL5_GPIO, INDICTOR_CHANNEL5_GPIO_PIN);
    GPIO_SetBits(INDICTOR_CHANNEL6_GPIO, INDICTOR_CHANNEL6_GPIO_PIN);
    GPIO_SetBits(INDICTOR_CHANNEL7_GPIO, INDICTOR_CHANNEL7_GPIO_PIN);
    GPIO_SetBits(INDICTOR_CHANNEL8_GPIO, INDICTOR_CHANNEL8_GPIO_PIN);
    GPIO_ResetBits(INDICTOR_ERROR_GPIO, INDICTOR_ERROR_GPIO_PIN);
}

//void IOLinkEventIdentification(uint8_t offset)
//{
//      if(gucaSpiRecvBuffer[offset]==0&&gucaSpiRecvBuffer[2+offset]==0xFF&&gucaSpiRecvBuffer[3+offset]==0xF0&&gucaSpiRecvBuffer[4+offset]==0&&gucaSpiRecvBuffer[5+offset]==5&&gucaSpiRecvBuffer[6+offset]==0xA0)
//      {
//          if(gucaSpiRecvBuffer[7+offset]==0)
//          {
//              if(gucaSpiRecvBuffer[1+offset]==1)
//              {
//                  IOLinkEventInfo[0] = gucaSpiRecvBuffer[9+offset];
//                  IOLinkEventInfo[1] = gucaSpiRecvBuffer[10+offset];
//              }
//              
//              if(gucaSpiRecvBuffer[1+offset]==2)
//              {
//                  IOLinkEventInfo[4] = gucaSpiRecvBuffer[9+offset];
//                  IOLinkEventInfo[5] = gucaSpiRecvBuffer[10+offset];
//              }
//              
//              if(gucaSpiRecvBuffer[1+offset]==3)
//              {
//                  IOLinkEventInfo[8] = gucaSpiRecvBuffer[9+offset];
//                  IOLinkEventInfo[9] = gucaSpiRecvBuffer[10+offset];
//              }
//              
//              if(gucaSpiRecvBuffer[1+offset]==4)
//              {
//                  IOLinkEventInfo[12] = gucaSpiRecvBuffer[9+offset];
//                  IOLinkEventInfo[13] = gucaSpiRecvBuffer[10+offset];
//              }
//          }
//          else if(gucaSpiRecvBuffer[7+offset]==1)
//          {
//              if(gucaSpiRecvBuffer[1+offset]==1)
//              {
//                  IOLinkEventInfo[2] = gucaSpiRecvBuffer[9+offset];
//                  IOLinkEventInfo[3] = gucaSpiRecvBuffer[10+offset];
//              }
//              
//              if(gucaSpiRecvBuffer[1+offset]==2)
//              {
//                  IOLinkEventInfo[6] = gucaSpiRecvBuffer[9+offset];
//                  IOLinkEventInfo[7] = gucaSpiRecvBuffer[10+offset];
//              }
//              
//              if(gucaSpiRecvBuffer[1+offset]==3)
//              {
//                  IOLinkEventInfo[10] = gucaSpiRecvBuffer[9+offset];
//                  IOLinkEventInfo[11] = gucaSpiRecvBuffer[10+offset];
//              }
//              
//              if(gucaSpiRecvBuffer[1+offset]==4)
//              {
//                  IOLinkEventInfo[14] = gucaSpiRecvBuffer[9+offset];
//                  IOLinkEventInfo[15] = gucaSpiRecvBuffer[10+offset];
//              }  
//          }
//      }
//}
   
