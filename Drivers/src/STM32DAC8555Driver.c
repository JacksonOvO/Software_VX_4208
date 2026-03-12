/*******************************************************************
*文件名称:  STM32AOSPIDriver.c
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
#include "stm32f30x_spi.h"
#include "stm32DAC8555driver.h"
#include "stm32f30x_gpio.h"
#include "STM32GPIODriver.h"
#include "stm32f30x_rcc.h"
#include "Ioinit.h"
#include "IOOptions.h"
#include "Iocontrol.h"
#include "stm32f30x_tim.h"
#include "MasterCommunication.h"
#include "STM32F0SPIDrive.h"
#include "SEGGER_RTT.h"
#define printf(...) SEGGER_RTT_printf(0, __VA_ARGS__)

#if FR4XX4_CODE || (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/

#if (IO_CODE_TYPE == FR4024_CODE) 
#define AO_MULTIPLIER   (52429)
#define AO_DIVIDEDER    (16)
#define AO_MAX_OUTPUT_DATA   (65535)
#endif

/* AO标定宏 */
#define AO_CALIBRATION_MULTIPLIER   (10737)
#define AO_CALIBRATION_DIVIDEDER    (30)


/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                         全局变量定义                            *
******************************************************************/
uint8_t IOLinkEventInfo[16] = {0};
// AD5755 芯片配置结构体（存储芯片工作模式参数）
AD5575_Setup AD5575_st = 
{
    0,                      // pinAD0state
    0,                      // pinAD1state
    0,                      // enablePacketErrorCheck
    1,                      // pocBit
    0,                      // statReadBit
    0,                      // shtCcLimBit
    1,                      //ewdEnable
    {0,0,0,0},           // rsetBitA, rsetBitB, rsetBitC, rsetBitD
    {0,0,0,0},           // ovrngBitA, ovrngBitB, ovrngBitC, ovrngBitD    
    0,                      // dcDcCompBit
    AD5755_PHASE_ALL_DC_DC, // dcDcPhaseBit
    AD5755_FREQ_410_HZ,     // dcDcFreqBit
    AD5755_MAX_23V          // dcDcMaxVBit
};

AD5575_Setup AD5575_st2 = 
{
    0,                      // pinAD0state
    1,                      // pinAD1state
    0,                      // enablePacketErrorCheck
    1,                      // pocBit
    0,                      // statReadBit
    0,                      // shtCcLimBit
    1,                      //ewdEnable
    {0,0,0,0},           // rsetBitA, rsetBitB, rsetBitC, rsetBitD
    {0,0,0,0},           // ovrngBitA, ovrngBitB, ovrngBitC, ovrngBitD    
    0,                      // dcDcCompBit
    AD5755_PHASE_ALL_DC_DC, // dcDcPhaseBit
    AD5755_FREQ_410_HZ,     // dcDcFreqBit
    AD5755_MAX_23V          // dcDcMaxVBit
};

    
uint8_t Rangeflags[9]={0,0,0,0,0,0,0,0,0};
uint8_t Errorflags[9]={0,0,0,0,0,0,0,0,0};

/*为使SPI能循环4通道输出并知道当前输出是哪个通道定义全局变量*/
static uint8_t gucCurrentChannel = 0;

/*组装DAC芯片4通道所需的最高8位的数据，该值是确认的，定义一个
   全局数组用于存储4通道高8位数据*/
const uint8_t DAC_HIGH_DATA[MAX_AO_CHANNEL_NUMBER] = {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E};

#if FR4XX4_CODE 
/*待发送的16位数据，全局变量声明*/
extern int16_t gusaAnalogData[MAX_AO_CHANNEL_NUMBER];
extern int16_t gusaAnalogDataMainLoop[MAX_AO_CHANNEL_NUMBER];
#endif

/* 量程定义 */
#define AI_MIN_IPUT_DATA (0x8000)
#define AI_MAX_INPUT_DATA   (0x7fff)

/*输入的码值*/
extern AODemarcate_t gtaAODemarcate[MAX_AI_CHANNEL_NUMBER];

/******************************************************************
*                          局部函数声明                            *
******************************************************************/

/******************************************************************
*                         全局函数实现                            *
******************************************************************/

void TIM7_Config(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    // 假设 APB1 定时器时钟 72MHz
    // 500ms 定时：72MHz / 7200 = 10kHz, 10kHz * 500 = 5000
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1; // 分频7200
    TIM_TimeBaseStructure.TIM_Period = 500 - 1;    // 自动重装载值
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE); // 允许更新中断
    TIM_Cmd(TIM7, ENABLE); // 启动定时器

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel =  TIM7_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM7_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

        AD5755_SetControlRegisters(AD5755_CREG_SOFT, 0, AD5755_SPI_CODE); 
    }
}


/**
 * @brief 初始化 STM32 与 DAC5755 通信
 * @details 完成 SPI 外设、GPIO 控制引脚、DAC 芯片初始化，并执行通信测试
 * @return E_STM32_AO_SPI_OK          初始化成功
 *         E_STM32_AO_SPI_TXFIFO_NOT_EMPTY  TXFIFO 非空错误
 */
STM32AOSPIDriverError_e STM32DAC5755DriverInit()
{ 


    // 初始化 SPI1 外设（配置时钟、引脚、通信模式）
    AD5755_SPI1_Init();
    
    // 初始化 DAC5755 控制引脚（LDAC、CLEAR 等 GPIO 配置）
    AD5755_GPIO_CONFIG();

    // 短延时确保 GPIO 状态稳定
    delay_us(100);
    
    // 执行 DAC5755 芯片初始化流程（复位、寄存器配置）
    AD5755_Init();
   

    return E_STM32_AO_SPI_OK;
}

/******************************************************************
*函数名称:STM32DAC8555DriverOutput
*功能描述:数据输出
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/2/10                              zhanghaifeng  we015
******************************************************************/
void STM32DAC8555DriverOutput(void)
{
    /*检查全局变量gucCurrentChannel以确保一轮4个通道都输出完成*/
    if (gucCurrentChannel != CHANNEL_INITIAL_VALUE)
    {
        return;
    }
    /*调用STM32AOSPISendData函数，用于填充TXFIFO*/
    //(void)STM32AD5755SendData(gucCurrentChannel);

    /*启动定时器，设置定时器时间为20us*/
    //DiSamplingTimerInit(AO_SAMPLING_PERIOD);    
}

/******************************************************************
*                         局部函数实现                            *
******************************************************************/

/**
 * @brief 配置 AD5755 控制引脚 GPIO
 * @details 初始化 GPIOB 引脚为推挽输出，设置 LDAC=低电平、CLEAR=高电平
 */
void AD5755_GPIO_CONFIG(void)
{

    GPIO_InitTypeDef GPIOInitStruct; // GPIO 初始化结构体

    // 使能 GPIOB 时钟（LDAC/CLEAR 引脚位于 GPIOB）
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOBEN, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOAEN, ENABLE);

    // 配置引脚模式：输出模式、推挽、50MHz 速度、无上下拉
    GPIOInitStruct.GPIO_Pin   = INDICTOR_LDAC_GPIO_PIN | INDICTOR_CLEAR_GPIO_PIN; // 组合引脚号
    GPIOInitStruct.GPIO_Mode  = GPIO_Mode_OUT;                                   // 输出模式
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;                                  // 推挽输出
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;                                // 高速模式
    GPIOInitStruct.GPIO_PuPd  = GPIO_PuPd_UP;                               // 无上下拉电阻

    // 初始化 GPIOB 相关引脚
    GPIO_Init(GPIOB, &GPIOInitStruct);

    // 设置初始电平：LDAC 低电平（禁止更新），CLEAR 低电平
    GPIO_ResetBits(GPIOB, INDICTOR_LDAC_GPIO_PIN); // LDAC = 0
    GPIO_ResetBits(GPIOB, INDICTOR_CLEAR_GPIO_PIN);  // CLEAR = 0
    
    GPIOInitStruct.GPIO_Pin   = GPIO_Pin_13;         // ALERT 引脚
    GPIOInitStruct.GPIO_Mode  = GPIO_Mode_IN;        // 输入模式
    GPIOInitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;      // 下拉（防止悬空）
    GPIO_Init(GPIOB, &GPIOInitStruct);
    
    //----------------------------------------------------------------------------//
    
        // 配置引脚模式：输出模式、推挽、50MHz 速度、无上下拉
    GPIOInitStruct.GPIO_Pin   = INDICTOR_LDAC2_GPIO_PIN | INDICTOR_CLEAR2_GPIO_PIN; // 组合引脚号
    GPIOInitStruct.GPIO_Mode  = GPIO_Mode_OUT;                                   // 输出模式
    GPIOInitStruct.GPIO_OType = GPIO_OType_PP;                                  // 推挽输出
    GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;                                // 高速模式
    GPIOInitStruct.GPIO_PuPd  = GPIO_PuPd_UP;                               // 无上下拉电阻

    // 初始化 GPIAB 相关引脚
    GPIO_Init(GPIOA, &GPIOInitStruct);

    // 设置初始电平：LDAC 低电平（禁止更新），CLEAR 低电平
    GPIO_ResetBits(GPIOA, INDICTOR_LDAC2_GPIO_PIN); // LDAC = 0
    GPIO_ResetBits(GPIOA, INDICTOR_CLEAR2_GPIO_PIN);  // CLEAR = 0
    
    GPIOInitStruct.GPIO_Pin   = GPIO_Pin_8;         // ALERT 引脚
    GPIOInitStruct.GPIO_Mode  = GPIO_Mode_IN;        // 输入模式
    GPIOInitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;      // 下拉（防止悬空）
    GPIO_Init(GPIOA, &GPIOInitStruct);
}

/******************************************************************
* 函数名称: AD5755_Init
* 功能描述: 初始化 AD5755 DAC 芯片
* 输入参数: 无
* 输出参数: 无
******************************************************************/
void AD5755_Init(void)
{
    unsigned char status  = 0;      // 状态码（未使用）
    unsigned char channel = 0;      // 通道计数器
    AD5575_Setup *pAD5575_st = &AD5575_st; // 配置结构体指针
    AD5575_Setup *pAD5575_st2 = &AD5575_st2; // 配置结构体指针

    // 1. 软件复位芯片
    AD5755_Software_Reset();
    delay_us(100);                  // 复位后延时（确保芯片稳定）

    // 2. 配置主控制寄存器
    AD5755_SetControlRegisters(AD5755_CREG_MAIN,
                              0,
                              (pAD5575_st->pocBit * AD5755_MAIN_POC) |         // 上电校准位
                              (pAD5575_st->statReadBit * AD5755_MAIN_STATREAD) | // 状态读取位
                              (pAD5575_st->ewdEnable * AD5755_MAIN_EWD) |
                               AD5755_MAIN_WD(AD5755_WD_200MS) |
                              AD5755_MAIN_SHTCCTLIM(pAD5575_st->shtCcLimBit));   // 短路电流限制位
    AD5755_SetControlRegisters2(AD5755_CREG_MAIN,
                              0,
                              (pAD5575_st2->pocBit * AD5755_MAIN_POC) |         // 上电校准位
                              (pAD5575_st2->statReadBit * AD5755_MAIN_STATREAD) | // 状态读取位
                              (pAD5575_st2->ewdEnable * AD5755_MAIN_EWD) |
                               AD5755_MAIN_WD(AD5755_WD_200MS) |
                              AD5755_MAIN_SHTCCTLIM(pAD5575_st2->shtCcLimBit));   // 短路电流限制位

    // 3. 启用数据包错误校验（若配置）
    if (AD5575_st.enablePacketErrorCheck)
    {
        AD5755_PEC_ENABLE();
    }
 
    // 4. 配置 DC-DC 转换器参数
    AD5755_SetControlRegisters(
          AD5755_CREG_DC_DC,
          0,
          (pAD5575_st->dcDcCompBit * AD5755_DC_DC_COMP) |       // DC-DC 补偿
          (AD5755_DC_DC_FREQ(pAD5575_st->dcDcFreqBit)) |         // 工作频率
          (AD5755_DC_DC_PHASE(pAD5575_st->dcDcPhaseBit)) |       // 相位模式
          (AD5755_DC_DC_MAX_V(pAD5575_st->dcDcMaxVBit)));        // 最大输出电压
    
    AD5755_SetControlRegisters2(
          AD5755_CREG_DC_DC,
          0,
          (pAD5575_st2->dcDcCompBit * AD5755_DC_DC_COMP) |       // DC-DC 补偿
          (AD5755_DC_DC_FREQ(pAD5575_st2->dcDcFreqBit)) |         // 工作频率
          (AD5755_DC_DC_PHASE(pAD5575_st2->dcDcPhaseBit)) |       // 相位模式
          (AD5755_DC_DC_MAX_V(pAD5575_st2->dcDcMaxVBit)));        // 最大输出电压

    // 5. 配置每个通道的 DAC 控制寄存器
    for (channel = AD5755_DAC_A; channel <= AD5755_DAC_D; channel++)
    {
 
        // 配置通道参数（内部参考、DC-DC 模式、输出范围等）
        AD5755_SetControlRegisters(AD5755_CREG_DAC,
                                   channel,
                                   pAD5575_st->rsetBits[channel] |       // 输出电阻设置
                                   pAD5575_st->ovrngBits[channel] |      // 过载范围设置
                                   AD5755_DAC_INT_ENABLE |               // 启用内部参考
                                   //AD5755_DAC_OVRNG |
                                   AD5755_DAC_DC_DC |                    // 启用 DC-DC 转换器
                                   AD5755_DAC_R(AD5755_R_0_12_V));      // 默认设置为 4-20mA 范围
        AD5755_SetControlRegisters2(AD5755_CREG_DAC,
                                   channel,
                                   pAD5575_st2->rsetBits[channel] |       // 输出电阻设置
                                   pAD5575_st2->ovrngBits[channel] |      // 过载范围设置
                                   AD5755_DAC_INT_ENABLE |               // 启用内部参考
                                   //AD5755_DAC_OVRNG |
                                   AD5755_DAC_DC_DC |                    // 启用 DC-DC 转换器
                                   AD5755_DAC_R(AD5755_R_0_12_V));      // 默认设置为 4-20mA 范围

        // 设置初始输出值（0x1000 = 约 8mA，对应 4-20mA 范围的中点）
        AD5755_SetRegisterValue(AD5755_DREG_WR_DAC, channel, 0x0);
        AD5755_SetRegisterValue2(AD5755_DREG_WR_DAC, channel, 0x0);
    }

    // 6. 延时确保 DC-DC 转换器稳定（至少 200us）
    delay_us(200);

    // 7. 启用所有通道输出
    for (channel = AD5755_DAC_A; channel <= AD5755_DAC_D; channel++)
    {

        AD5755_SetControlRegisters(AD5755_CREG_DAC,
                                   channel,
                                   AD5755_DAC_INT_ENABLE |   // 启用内部参考
                                   AD5755_DAC_OUTEN |        // 启用输出
                                   //AD5755_DAC_OVRNG |
                                   AD5755_DAC_DC_DC |        // 启用 DC-DC 转换器
                                   AD5755_DAC_R(AD5755_R_0_12_V)); // 输出范围为 4-20mA
        AD5755_SetControlRegisters2(AD5755_CREG_DAC,
                                   channel,
                                   AD5755_DAC_INT_ENABLE |   // 启用内部参考
                                   AD5755_DAC_OUTEN |        // 启用输出
                                   //AD5755_DAC_OVRNG |
                                   AD5755_DAC_DC_DC |        // 启用 DC-DC 转换器
                                   AD5755_DAC_R(AD5755_R_0_12_V)); // 输出范围为 4-20mA
    }
   
    
}


/**
 * @brief  电压输出码值线性补偿
 * @param  code 原码值（电压DAC码值）
 * @return 补偿后的码值
 */
int32_t CurrentLinearCompensate(int32_t dacCode, uint8_t ucCurrentChannel)
{
    if (dacCode == 0) {
        return 0;
    }
    

    double comp = 0.0;
    double corrected = (double)dacCode;
    if (dacCode > 5000 || dacCode < -5000) {  // 支持正负值
        if(AISampleType[ucCurrentChannel] == 0x01 || 
           AISampleType[ucCurrentChannel] == 0x03 || 
           AISampleType[ucCurrentChannel] == 0x05 || 
           AISampleType[ucCurrentChannel] == 0x07 )
        {  
          
          if(ucCurrentChannel < 4 ){
              comp = ((double)dacCode / 27648.0) * (13.0+gtaAODemarcate[ucCurrentChannel].lCompensateValue);
          }else{
              comp = ((double)dacCode / 27648.0) * (13.0+gtaAODemarcate[ucCurrentChannel].lCompensateValue2);
          }
          
        } else {
          double scale = 32000.0 / 27648.0;
          if(ucCurrentChannel < 4 ){
              comp = ((double)dacCode / 32000.0) *  (13.0+gtaAODemarcate[ucCurrentChannel].lCompensateValue) * scale;
          }else{
              comp = ((double)dacCode / 32000.0) *  (13.0+gtaAODemarcate[ucCurrentChannel].lCompensateValue2) * scale;
          }    
        }
    }
    
    //printf("lCompensateValue=%d\n",gtaAODemarcate[ucCurrentChannel].lCompensateValue);

    // 再加上公式里的线性补偿
    corrected += comp;

    if (corrected > 65535.0) corrected = 65535.0;
    if (corrected < -65535.0) corrected = -65535.0;

    return (int32_t)(corrected + (corrected >= 0 ? 0.5 : -0.5)); // 四舍五入，兼容负数
}

int32_t CurrentLinearCompensate_mA(int32_t dacCode,uint8_t ucCurrentChannel)
{
    if (dacCode == 0) {
        return 0;  // 零点不补偿
    }

    double comp = 0.0;
    double corrected = (double)dacCode;

    if (AISampleType[ucCurrentChannel] == 9 || AISampleType[ucCurrentChannel] == 11) {
        // 0~27648 对应 0~20mA 或 4~20mA
      if(ucCurrentChannel< 4){
        comp = ( (double)dacCode / 27648.0 ) * (16.0+gtaAODemarcate[ucCurrentChannel+4].lCompensateValue) + 13.0;  
      }else{
        comp = ( (double)dacCode / 27648.0 ) * (16.0+gtaAODemarcate[ucCurrentChannel+4].lCompensateValue2) + 13.0;  
      }
    } else {
        // 0~32000 对应 0~20mA 或 4~20mA
        double scale = 32000.0 / 27648.0;  // 基准比值 ≈ 1.157
         if(ucCurrentChannel< 4){
            comp = (( (double)dacCode / 32000.0 ) * (16.0+gtaAODemarcate[ucCurrentChannel+4].lCompensateValue) + 13.0) * scale;
         }else{
            comp = (( (double)dacCode / 32000.0 ) * (16.0+gtaAODemarcate[ucCurrentChannel+4].lCompensateValue2) + 13.0) * scale;
         }
    }

    // 如果 lCompensateValue 单位是 "码值补偿"
    //corrected += gtaAODemarcate[ucCurrentChannel+4].lCompensateValue;

    // 再加上公式里的线性补偿
    corrected += comp;

    // 限制范围（电流模式一般不会超过逻辑范围）
    if (corrected > 65535.0) corrected = 32000.0;
    if (corrected < 0.0) corrected = 0.0;

    return (int32_t)(corrected + 0.5); // 四舍五入
}


/******************************************************************
*函数名称:STM32AD5755SendData
*功能描述:填充TXFIFO
*输入参数:无
*输出参数:
*返回值:E_STM32_AO_SPI_OK                                     SPI成功
                        E_STM32_AO_SPI_TXFIFO_NOT_EMPTY        TXFIFO不为空
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/2/10                              zhanghaifeng  we015
******************************************************************/
STM32AOSPIDriverError_e STM32AD5755SendData(uint8_t ucCurrentChannel)
{
  if(RangeSwitchingFlagBit == 0)
    {
    int32_t userCode = gusaAnalogDataMainLoop[ucCurrentChannel];
//    if(gusaAnalogData[4] == 0x01){
//      //printf("AAA\n");
//      if(AIChannelSet[ucCurrentChannel] == 0x00){
//        userCode = gusaAnalogDataMainLoop[ucCurrentChannel];
//        //printf("gusaAnalogDataMainLoop[%d]=%d\n",ucCurrentChannel,gusaAnalogDataMainLoop[ucCurrentChannel]);
//      }else if(AIChannelSet[ucCurrentChannel] == 0x01){
//        userCode = 0 ;
//      }else{
//        userCode = AIValue[ucCurrentChannel];
//      }
//    }
    int32_t logicMin = 0, logicMax = 0;
    int32_t lower  = 0, Upper  = 0;
    float userMin = 0.0f, userMax = 0.0f;     // 用户定义的物理电压或电流范围
    float adcMin = 0.0f, adcMax = 0.0f;       // AD5755 实际输出电压电流范围
    float unitValue = 0.0f;
    uint16_t dacCode = 0;
    int16_t NEWCode = 0;
    AD5575_Setup *pAD5575_st = &AD5575_st; // 配置结构体指针

    
   switch (AISampleType[ucCurrentChannel])
    {
        case 0x01:  // 0~10V, 0~27648 → ADC: 0~12V
            logicMin = 0; logicMax = 27648;
            lower = 0; Upper = 32767;
            userMin = 0.0f; userMax = 10.0f;
            adcMin = 0.0f;  adcMax = 12.0f;
            if(userCode>=32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x02:  // 0~10V, 0~32000 → ADC: 0~12V
            logicMin = 0; logicMax = 32000;
            lower = 0; Upper = 32767;
            userMin = 0.0f; userMax = 10.0f;
            adcMin = 0.0f;  adcMax = 12.0f;
            if(userCode>=32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x03:  // -10V~10V, -27648~27648 → ADC: -12V~12V
            logicMin = -27648; logicMax = 27648;
            lower = -32767; Upper = 32767;
            userMin = -10.0f; userMax = 10.0f;
            adcMin = -12.0f;  adcMax = 12.0f;
            if(userCode>=32511||userCode<=-32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x04:  // -10V~10V, -32000~32000 → ADC: -12V~12V
            logicMin = -32000; logicMax = 32000;
            lower = -32767; Upper = 32767;
            userMin = -10.0f; userMax = 10.0f;
            adcMin = -12.0f;  adcMax = 12.0f;
            if(userCode>=32511||userCode<=-32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x05:  // 0V~5V, 0~27648 → ADC: 0V~6V
            logicMin = 0; logicMax = 27648;
            lower = 0; Upper = 32767;
            userMin = 0.0f; userMax = 5.0f;
            adcMin = 0.0f;  adcMax = 6.0f;
            if(userCode>=32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x06:  // 0V~5V, 0~32000 → ADC: 0V~6V
            logicMin = 0; logicMax = 32000;
            lower = 0; Upper = 32767;
            userMin = 0.0f; userMax = 5.0f;
            adcMin = 0.0f;  adcMax = 6.0f;
            if(userCode>=32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x07:  // -5V~5V, -27648~27648 → ADC: -6V~6V
            logicMin = -27648; logicMax = 27648;
            lower = -32767; Upper = 32767;
            userMin = -5.0f; userMax = 5.0f;
            adcMin = -6.0f;  adcMax = 6.0f;
            if(userCode>=32511||userCode<=-32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x08:  // -5V~5V, -32000~32000 → ADC: -6V~6V
            logicMin = -32000; logicMax = 32000;
            lower = -32767; Upper = 32767;
            userMin = -5.0f; userMax = 5.0f;
            adcMin = -6.0f;  adcMax = 6.0f;
            if(userCode>=32511||userCode<=-32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x09:  // 0~20mA, 0~27648 → ADC: 0~24mA
            logicMin = 0; logicMax = 27648;
            lower = 0; Upper = 32767;
            userMin = 0.0f; userMax = 20.0f;
            adcMin = 0.0f;  adcMax = 24.0f;
            if(userCode>=32511||userCode<=-32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x0A:  // 0~20mA, 0~32000 → ADC: 0~24mA
            logicMin = 0; logicMax = 32000;
            lower = 0; Upper = 32767;
            userMin = 0.0f; userMax = 20.0f;
            adcMin = 0.0f;  adcMax = 24.0f;
            if(userCode>=32511||userCode<=-32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x0B:  // 4~20mA, 0~27648 → ADC: 0~24mA
            logicMin = 0; logicMax = 27648;
            lower = 0; Upper = 32767;
            userMin = 4.0f; userMax = 20.0f;
            adcMin = 0.0f;  adcMax = 24.0f;
            if(userCode>=32511||userCode<=-32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        case 0x0C:  // 4~20mA, 0~32000 → ADC: 0~24mA
            logicMin = 0; logicMax = 32000;
            lower = 0; Upper = 32767;
            userMin = 4.0f; userMax = 20.0f;
            adcMin = 0.0f;  adcMax = 24.0f;
            if(userCode>=32511||userCode<=-32511){
              Rangeflags[ucCurrentChannel]=1;
            }else{
              Rangeflags[ucCurrentChannel]=0;
            }
            break;
        default:
            return E_STM32_AO_SPI_TXFIFO_NOT_EMPTY;
    }
   
   // 限制用户输入在合法范围
    if (userCode < lower) userCode = lower;
    if (userCode > Upper) userCode = Upper;

     
    if(AISampleType[ucCurrentChannel] == 9 ||AISampleType[ucCurrentChannel] == 10 ||AISampleType[ucCurrentChannel] == 11 || AISampleType[ucCurrentChannel] == 12 )
    {

      NEWCode = CurrentLinearCompensate_mA(userCode,ucCurrentChannel);
      float ratio = (float)(NEWCode - logicMin) / (logicMax - logicMin);
      unitValue = userMin + ratio * (userMax - userMin);
    }else{
      NEWCode = CurrentLinearCompensate(userCode,ucCurrentChannel);
      float ratio = (float)(NEWCode - logicMin) / (logicMax - logicMin);
      unitValue = userMin + ratio * (userMax - userMin);
      
    }
    
        // 步骤2：将目标值换算为 DAC 码值
    dacCode = (uint16_t)((((unitValue - adcMin) / (adcMax - adcMin)) * 65535.0f) + 0.5f);
    
    //printf("channel:%d,mode:%d,uservalue:%d,unitValue: %d.%02d,dacCode:%d newcode:%d\n",ucCurrentChannel,AISampleType[ucCurrentChannel],userCode,(int)unitValue,(int)((unitValue - (int)unitValue) * 100),dacCode,NEWCode);
    
    if (dacCode > 65535) dacCode = 65535;    
        
    if(ucCurrentChannel<4){
        AD5755_SetRegisterValue(AD5755_DREG_WR_DAC, ucCurrentChannel, dacCode);
    }else{
        AD5755_SetRegisterValue2(AD5755_DREG_WR_DAC, ucCurrentChannel-4, dacCode);
    }    
    
    }

    return E_STM32_AO_SPI_OK;
}


/******************************************************************
*函数名称:TIM2_IRQHandler
*功能描述:定时器中断
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/2/10                              zhanghaifeng  we015
******************************************************************/
#if FR4XX4_CODE|| (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)||(IO_CODE_TYPE == FR4104_CODE)
void TIM2_IRQHandler(void)
{        
    /* Disable timer 2 */
    TIM_Cmd(TIM2, DISABLE);
    
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        /* 清除中断 */
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        if (SPI_I2S_GetFlagStatus(DAC_SPI, SPI_I2S_FLAG_BSY) == RESET)
        {
            /*输出时配置SYNC为高，没有输出时配置为低
                    (备注:本次硬件版本是输出时SYNC为高，实际输出时SYNC应为低)*/


            /*准备进行下个通道输出，当前发送通道全局变量加1*/
            gucCurrentChannel++;
            if(AO_CYCLE_TIMES == gucCurrentChannel)
            {
                gucCurrentChannel = CHANNEL_INITIAL_VALUE;
                return;
            }
            
            /*调用STM32AOSPISendData函数实现DAC4通道输出*/
            //(void)STM32AD5755SendData(gucCurrentChannel);        
        }
    }
    /* Enable timer 2 */
    TIM_Cmd( TIM2, ENABLE );
}
#endif

/******************************************************************
* 函数名称: AD5755_SetVoltage
* 功能描述: 设置指定通道的电压输出值
* 输入参数: channel - 通道号（0~3）
*           voltage - 目标电压值（单位：V）
* 输出参数: 实际输出电压值（浮点型，带校准后结果）
******************************************************************/
float AD5755_SetVoltage(unsigned char channel, float voltage)
{
  
    unsigned long code        = 0;
    unsigned char range       = 0;
    unsigned char resolution  = 0;
    unsigned long rangeOffset = 0;
    float         vRef        = 0;
    float         realVoltage = 0;
    
    /* Get the range of the selected channel. */
    range = AD5755_GetRegisterValue(AD5755_RD_CTRL_REG(channel)) & 0x7;
    switch(range)
    {
        case AD5755_R_0_5_V:
            rangeOffset = 0;
            vRef = 5.0;
            resolution = 16;
            break;
        case AD5755_R_0_10_V:
            rangeOffset = 0;
            vRef = 10.0;
            resolution = 16;
            break;
        case AD5755_R_M5_P5_V:
            rangeOffset = 0x8000;
            vRef = 5.0;
            resolution = 15;
            break;
        case AD5755_R_M10_P10_V:
            rangeOffset = 0x8000;
            vRef = 10.0;
            resolution = 15;
            break;
        default:
            rangeOffset = 0;
            vRef = 0;
            resolution = 0;
    }
    
    /* Compute the binary code from the users voltage value. */
    code = (long)(voltage * (1l << resolution) / vRef) + rangeOffset;
    if(code > 0xFFFF)
    {
        code = 0xFFFF;
    }
    
    // 写入 DAC 数据寄存器
    AD5755_SetRegisterValue(AD5755_DREG_WR_DAC, channel, code);
    
    // 计算实际输出电压（用于返回校准后的值）
    realVoltage = ((long)(code - rangeOffset) * vRef) / (1l << resolution);
        
    return realVoltage;
}

float AD5755_SetCurrent(unsigned char channel, float mACurrent)
{
  printf("AD5755_SetCurrent\n");
    long  range       = 0;
    long  code        = 0;
    char  rangeOffset = 0;
    float iRef        = 0;
    float realCurrent = 0;
    
    /* Get the range of the selected channel. */
    range = AD5755_GetRegisterValue(AD5755_RD_CTRL_REG(channel)) & 0x7;
    switch(range)
    {
        case AD5755_R_4_20_MA:
            iRef = 16.0;        // mA
            rangeOffset = 4;    // mA
            break;
        case AD5755_R_0_20_MA:
            iRef = 20.0;        // mA
            rangeOffset = 0;    // mA
            break;
        case AD5755_R_0_24_MA:
            iRef = 24.0;        // mA
            rangeOffset = 0;    // mA
            break;
        default:
            iRef = 1;
            rangeOffset = 0;
    }
    
    /* Compute the binary code from the value(mA) provided by user. */
    code = (long)((mACurrent - rangeOffset) * (1l << 16) / iRef);
    if(code > 0xFFFF)
    {
        code = 0xFFFF;
    }

    /* Write to the Data Register of the DAC. */
    AD5755_SetRegisterValue(AD5755_DREG_WR_DAC,
                            channel,
                            code);
    
    realCurrent = (code * iRef / (float)(1l << 16)) + rangeOffset;
    
    return realCurrent;
}

/******************************************************************
* 函数名称: AD5755_SetRegisterValue
* 功能描述: 向 DAC 寄存器写入数据
* 输入参数: registerAddress - 寄存器地址
*           channel         - 目标通道号
*           registerValue   - 写入值（32位）
* 输出参数: 状态寄存器值（通信结果）
******************************************************************/
unsigned short AD5755_SetRegisterValue(unsigned char registerAddress,
                                       unsigned char channel,
                                       unsigned long registerValue)
{

    unsigned char  buff[4]   = {0, 0, 0, 0}; // 传输缓冲区
    unsigned long  command   = 0;            // 命令字
    unsigned short statusReg = 0;            // 状态寄存器值
 
    // 构建写寄存器命令（包含地址、通道、数据等信息）
    command = AD5755_ISR_WRITE | 
              AD5755_ISR_DUT_AD1(AD5575_st.pinAD1state) | 
              AD5755_ISR_DUT_AD0(AD5575_st.pinAD0state) |
              AD5755_ISR_DREG(registerAddress) | 
              AD5755_ISR_DAC_AD(channel) |
              AD5755_ISR_DATA(registerValue);

    // 拆分命令字为3字节（高位在前）
    buff[0] = (command & 0xFF0000) >> 16;
    buff[1] = (command & 0x00FF00) >> 8;
    buff[2] = (command & 0x0000FF) >> 0;    

    // 若启用 CRC 校验，计算并添加校验字节
    if (AD5575_st.enablePacketErrorCheck)
    {
        buff[3] = AD5755_CheckCrc(buff, 3);
    }

    // 通过 SPI 发送数据（包含 CRC 校验字节）
    SPI_Write(AD5755_SLAVE_ID, buff, 3 + AD5575_st.enablePacketErrorCheck);  

    return statusReg; // 返回状态寄存器（当前未实际读取，返回0）
}

/******************************************************************
* 函数名称: AD5755_SetRegisterValue
* 功能描述: 向 DAC 寄存器写入数据
* 输入参数: registerAddress - 寄存器地址
*           channel         - 目标通道号
*           registerValue   - 写入值（32位）
* 输出参数: 状态寄存器值（通信结果）
******************************************************************/
unsigned short AD5755_SetRegisterValue2(unsigned char registerAddress,
                                       unsigned char channel,
                                       unsigned long registerValue)
{

    unsigned char  buff[4]   = {0, 0, 0, 0}; // 传输缓冲区
    unsigned long  command   = 0;            // 命令字
    unsigned short statusReg = 0;            // 状态寄存器值
 
    // 构建写寄存器命令（包含地址、通道、数据等信息）
    command = AD5755_ISR_WRITE | 
              AD5755_ISR_DUT_AD1(AD5575_st2.pinAD1state) | 
              AD5755_ISR_DUT_AD0(AD5575_st2.pinAD0state) |
              AD5755_ISR_DREG(registerAddress) | 
              AD5755_ISR_DAC_AD(channel) |
              AD5755_ISR_DATA(registerValue);

    // 拆分命令字为3字节（高位在前）
    buff[0] = (command & 0xFF0000) >> 16;
    buff[1] = (command & 0x00FF00) >> 8;
    buff[2] = (command & 0x0000FF) >> 0;    

    // 若启用 CRC 校验，计算并添加校验字节
    if (AD5575_st.enablePacketErrorCheck)
    {
        buff[3] = AD5755_CheckCrc(buff, 3);
    }

    // 通过 SPI 发送数据（包含 CRC 校验字节）
    SPI_Write(AD5755_SLAVE_ID, buff, 3 + AD5575_st.enablePacketErrorCheck);

    return statusReg; // 返回状态寄存器（当前未实际读取，返回0）
}

/******************************************************************
* 函数名称: AD5755_CheckCrc
* 功能描述: 计算 CRC8 校验值
* 输入参数: data        - 数据缓冲区
*           bytesNumber - 数据字节数
* 输出参数: CRC8 校验值
******************************************************************/
unsigned char AD5755_CheckCrc(unsigned char* data, unsigned char bytesNumber)
{
    unsigned char crc  = 0x00; // 校验值初始化为0
    unsigned char byte = 0;    // 当前字节索引
    unsigned char bit  = 0;    // 当前位索引

    // CRC8 计算算法（多项式：0x07）
    for (byte = 0; byte < bytesNumber; byte++)
    {
        crc ^= (data[byte]); // 异或当前字节
        for (bit = 8; bit > 0; bit--)
        {
            if (crc & 0x80) // 最高位为1时，应用多项式除法
            {
                crc = (crc << 1) ^ AD5755_CRC_POLYNOMIAL;
            }
            else // 最高位为0时，直接左移
            {
                crc = (crc << 1);
            }
        }
    }
    return crc; // 返回计算得到的 CRC 校验值
}

/******************************************************************
* 函数名称: delay_us
* 功能描述: 微秒级延时函数（基于空操作循环）
* 输入参数: us - 延时微秒数
* 输出参数: 无
* 注意事项: 需根据实际 MCU 主频调整循环系数
******************************************************************/
void delay_us(uint32_t us)
{
    us *= 72;  // 循环系数（需根据 MCU 主频调整）
    while (us--) __NOP();  // 空操作循环实现延时
}

void delay_ms(uint32_t ms)
{
    ms *= 72000;  // 循环系数（STM32F302C8主频为72MHz）
    while (ms--) __NOP();  // 空操作循环实现延时
}


/******************************************************************
* 函数名称: AD5755_Software_Reset
* 功能描述: 软件复位 AD5755 芯片
* 输入参数: 无
* 输出参数: 无
******************************************************************/
void AD5755_Software_Reset(void)
{
    // 向软件控制寄存器写入复位命令（0xFFFF）
    AD5755_SetControlRegisters(AD5755_CREG_SOFT, 0, AD5755_RESET_CODE);
    AD5755_SetControlRegisters2(AD5755_CREG_SOFT, 0, AD5755_RESET_CODE);
}

/******************************************************************
* 函数名称: AD5755_SetControlRegisters
* 功能描述: 设置 AD5755 控制寄存器
* 输入参数: ctrlRegAddress - 控制寄存器地址
*           channel        - 目标通道
*           regValue       - 寄存器值
* 输出参数: 无
******************************************************************/
void AD5755_SetControlRegisters(unsigned char  ctrlRegAddress,
                                unsigned char  channel,
                                unsigned short regValue)
{
    // 调用寄存器写入函数，写入控制寄存器
    AD5755_SetRegisterValue(AD5755_DREG_WR_CTRL_REG, 
                            channel,
                            AD5755_CTRL_CREG(ctrlRegAddress) | regValue);
}

void AD5755_SetControlRegisters2(unsigned char  ctrlRegAddress,
                                unsigned char  channel,
                                unsigned short regValue)
{
    // 调用寄存器写入函数，写入控制寄存器
    AD5755_SetRegisterValue2(AD5755_DREG_WR_CTRL_REG, 
                            channel,
                            AD5755_CTRL_CREG(ctrlRegAddress) | regValue);
}
/******************************************************************
* 函数名称: AD5755_PEC_ENABLE
* 功能描述: 启用数据包错误校验（PEC）
* 输入参数: 无
* 输出参数: 无
******************************************************************/
void AD5755_PEC_ENABLE(void)
{
    // 向软件控制寄存器写入 PEC 启用命令（位12置1）
    AD5755_SetControlRegisters(AD5755_CREG_SOFT, 0, 1 << 12);
}

/******************************************************************
* 函数名称: AD5755_GetRegisterValue
* 功能描述: 读取 AD5755 寄存器值
* 输入参数: registerAddress - 寄存器地址
* 输出参数: 寄存器值（32位）
******************************************************************/
long AD5755_GetRegisterValue(unsigned char registerAddress)
{

    unsigned char buffer[4] = {0, 0, 0, 0};
    unsigned long command   = 0;
    unsigned long regValue  = 0;
    unsigned char crc       = 0;
    
    command = AD5755_ISR_READ | 
              AD5755_ISR_DUT_AD1(AD5575_st.pinAD1state) | 
              AD5755_ISR_DUT_AD0(AD5575_st.pinAD0state) | 
              AD5755_ISR_RD(registerAddress);
    buffer[0] = (command & 0xFF0000) >> 16;
    buffer[1] = (command & 0x00FF00) >> 8;
    buffer[2] = (command & 0x0000FF) >> 0;
    if(AD5575_st.enablePacketErrorCheck)
    {
        buffer[3] = AD5755_CheckCrc(buffer, 3);
    }
    SPI_Write(AD5755_SLAVE_ID, buffer, 3 + AD5575_st.enablePacketErrorCheck);
    command = AD5755_ISR_WRITE | 
              AD5755_ISR_DUT_AD1(AD5575_st.pinAD1state) | 
              AD5755_ISR_DUT_AD0(AD5575_st.pinAD0state) |
              AD5755_ISR_NOP;
    buffer[0] = (command & 0xFF0000) >> 16;
    buffer[1] = (command & 0x00FF00) >> 8;
    buffer[2] = (command & 0x0000FF) >> 0;
    if(AD5575_st.enablePacketErrorCheck)
    {
        buffer[3] = AD5755_CheckCrc(buffer, 3);
    }
    SPI_Read(AD5755_SLAVE_ID, buffer, 3 + AD5575_st.enablePacketErrorCheck);
    regValue = ((unsigned short)buffer[1] << 8) + buffer[2];
    /* Check the CRC. */
    if(AD5575_st.enablePacketErrorCheck)
    {
        crc = AD5755_CheckCrc(&buffer[1], 3);
        if(crc != AD5755_CRC_CHECK_CODE)
        {
            regValue = -1;
        }
    }
    
    return regValue;
}

/******************************************************************
* 函数名称: AD5755_GetRegisterValue2
* 功能描述: 读取 AD5755 寄存器值
* 输入参数: registerAddress - 寄存器地址
* 输出参数: 寄存器值（32位）
******************************************************************/
long AD5755_GetRegisterValue2(unsigned char registerAddress)
{

    unsigned char buffer[4] = {0, 0, 0, 0};
    unsigned long command   = 0;
    unsigned long regValue  = 0;
    unsigned char crc       = 0;
    
    command = AD5755_ISR_READ | 
              AD5755_ISR_DUT_AD1(AD5575_st2.pinAD1state) | 
              AD5755_ISR_DUT_AD0(AD5575_st2.pinAD0state) | 
              AD5755_ISR_RD(registerAddress);
    buffer[0] = (command & 0xFF0000) >> 16;
    buffer[1] = (command & 0x00FF00) >> 8;
    buffer[2] = (command & 0x0000FF) >> 0;
    if(AD5575_st2.enablePacketErrorCheck)
    {
        buffer[3] = AD5755_CheckCrc(buffer, 3);
    }
    SPI_Write(AD5755_SLAVE_ID, buffer, 3 + AD5575_st2.enablePacketErrorCheck);
    command = AD5755_ISR_WRITE | 
              AD5755_ISR_DUT_AD1(AD5575_st2.pinAD1state) | 
              AD5755_ISR_DUT_AD0(AD5575_st2.pinAD0state) |
              AD5755_ISR_NOP;
    buffer[0] = (command & 0xFF0000) >> 16;
    buffer[1] = (command & 0x00FF00) >> 8;
    buffer[2] = (command & 0x0000FF) >> 0;
    if(AD5575_st2.enablePacketErrorCheck)
    {
        buffer[3] = AD5755_CheckCrc(buffer, 3);
    }
    SPI_Read(AD5755_SLAVE_ID, buffer, 3 + AD5575_st2.enablePacketErrorCheck);
    regValue = ((unsigned short)buffer[1] << 8) + buffer[2];
    /* Check the CRC. */
    if(AD5575_st2.enablePacketErrorCheck)
    {
        crc = AD5755_CheckCrc(&buffer[1], 3);
        if(crc != AD5755_CRC_CHECK_CODE)
        {
            regValue = -1;
        }
    }
    
    return regValue;
}


void AD5755_SetChannelRange(unsigned char channel, unsigned char range)
{
  printf("AD5755_SetChannelRange\n");
    unsigned short outputCode = 0x0000;
    unsigned long  oldDacCtrlReg = 0;
    unsigned long  newDacCtrlReg = 0;
    
    /* Read the content of the DAC Control Register of the selected channel. */
    oldDacCtrlReg = AD5755_GetRegisterValue(AD5755_RD_CTRL_REG(channel));
    /* Clear the bits that will be modified by this function. */
    oldDacCtrlReg &= ~(AD5755_DAC_INT_ENABLE | 
                       AD5755_DAC_OUTEN | 
                       AD5755_DAC_DC_DC | 
                       AD5755_DAC_R(7));
    /* Select the output code before changing the range. */
    if((range == AD5755_R_M5_P5_V) || (range == AD5755_R_M10_P10_V))
    {
        outputCode = 0x8000;
    }
    /* Set the output code to zero or midscale. */
    AD5755_SetRegisterValue(AD5755_DREG_WR_DAC, channel, outputCode);
    /* Set range. */
    newDacCtrlReg = oldDacCtrlReg | 
                    AD5755_DAC_INT_ENABLE | 
                    AD5755_DAC_DC_DC | 
                    AD5755_DAC_R(range);
    AD5755_SetControlRegisters(AD5755_CREG_DAC, channel, newDacCtrlReg);
    /* Set the output code to zero or midscale. */
    AD5755_SetRegisterValue(AD5755_DREG_WR_DAC, channel, outputCode);
    delay_us(200);
    /* Enable the output of the channel. */
    newDacCtrlReg |= AD5755_DAC_OUTEN;
    AD5755_SetControlRegisters(AD5755_CREG_DAC, channel, newDacCtrlReg);
}

void AD5755_ChannelClearEnable(unsigned char channel, unsigned char clearEn)
{
    printf("AD5755_ChannelClearEnable\n");
    unsigned long  oldDacCtrlReg = 0;
    unsigned long  newDacCtrlReg = 0;
    
    /* Read the content of the DAC Control Register of the selected channel. */
    oldDacCtrlReg = AD5755_GetRegisterValue(AD5755_RD_CTRL_REG(channel));
    /* Clear the CLR_EN bit. */
    oldDacCtrlReg &= ~(AD5755_DAC_CLR_EN);
    newDacCtrlReg |= oldDacCtrlReg | (clearEn * AD5755_DAC_CLR_EN);
    AD5755_SetControlRegisters(AD5755_CREG_DAC, channel, newDacCtrlReg);
}

void AD5755_ParseStatusRegister(void) 
{
    // 状态寄存器地址，根据之前结论填 0x18
    const unsigned char statusRegAddr = 0x18; 
    
    uint8_t byte0 = 0;
    uint8_t byte1 = 0;
    
    long statusValue = AD5755_GetRegisterValue(statusRegAddr);
    Errorflags[0]=0;
    Errorflags[1]=0;
    Errorflags[2]=0;
    Errorflags[3]=0;
    Errorflags[4]=0;
    Errorflags[5]=0;
    Errorflags[6]=0;
    Errorflags[7]=0;
    Errorflags[8]=0;
    uint8_t faultState = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
    //printf("status raw=0x%08lX\n", statusValue);
    if (faultState == Bit_SET) {
        // 发生故障，开启灯闪烁
        Errorflags[4]=1;
        byte1 |= (1<<2);
    } else {
        // 正常，关闭闪烁或常亮
        Errorflags[4]=0;
        byte1 |= (0<<2);
    }
    
    // 逐位解析状态寄存器，根据表格定义判断故障
    // DC-DCD 故障（Bit15）
    if (statusValue & (1 << 15)) {
        printf("DC-DC D ERR\n");
        Rangeflags[3]=1;
    }
    // DC-DCC 故障（Bit14）
    if (statusValue & (1 << 14)) {
        printf("DC-DC C ERR\n");
        Rangeflags[2]=1;
    }
    // DC-DCB 故障（Bit13）
    if (statusValue & (1 << 13)) {
        printf("DC-DC B ERR\n");
        Rangeflags[1]=1;
    }
    // DC-DCA 故障（Bit12）
    if (statusValue & (1 << 12)) {
        printf("DC-DC A ERR\n");
        Rangeflags[0]=1;
    }
    // 用户切换（Bit11）
    if (statusValue & (1 << 11)) {
        
    }
    // PEC 错误（Bit10）
    if (statusValue & (1 << 10)) {
        printf("PECERR\n");
    }
    // 斜坡有效（Bit9）
    if (statusValue & (1 << 9)) {
       
    }
    // 过热（Bit8）
    if (statusValue & (1 << 8)) {
        //printf("Overheating\n");
        byte1 |= (1<<1);
    }
    // VOUT_D 故障（Bit7）
    if (statusValue & (1 << 7)) {
        //printf("VOUT_D ERROR\n");
        Rangeflags[3]=1;
        byte0 |= (1<<7);
    }
    // VOUT_C 故障（Bit6）
    if (statusValue & (1 << 6)) {
        //printf("VOUT_C ERROR\n");
        Rangeflags[2]=1;
        byte0 |= (1<<6);
    }
    // VOUT_B 故障（Bit5）
    if (statusValue & (1 << 5)) {
        //printf("VOUT_B ERROR\n");
        Rangeflags[1]=1;
        byte0 |= (1<<5);
    }
    // VOUT_A 故障（Bit4）
    if (statusValue & (1 << 4)) {
        //printf("VOUT_A ERROR\n");
        Rangeflags[0]=1;
        byte0 |= (1<<4);
    }
    // IOUT_D 故障（Bit3）
    if (statusValue & (1 << 3)) {
        //printf("IOUT_D ERR\n");
        Rangeflags[3]=1;
        byte0 |= (1<<3);
    }
    // IOUT_C 故障（Bit2）
    if (statusValue & (1 << 2)) {
        //printf("IOUT_C ERR\n");
        Rangeflags[2]=1;
        byte0 |= (1<<2);
    }
    // IOUT_B 故障（Bit1）
    if (statusValue & (1 << 1)) {
        //printf("IOUT_B ERR\n");
        Rangeflags[1]=1;
        byte0 |= (1<<1);
    }
    // IOUT_A 故障（Bit0）
    if (statusValue & (1 << 0)) {
        //printf("IOUT_A ERR\n");
        Rangeflags[0]=1;
        byte0 |= (1<<0);
    }
    
    IOLinkEventInfo[0] = byte0;
    IOLinkEventInfo[1] = byte1;
}

void AD5755_ParseStatusRegister2(void) 
{
    // 状态寄存器地址，根据之前结论填 0x18
    const unsigned char statusRegAddr = 0x18; 
    
    uint8_t byte0 = 0;
    uint8_t byte1 = 0;
    
    long statusValue = AD5755_GetRegisterValue2(statusRegAddr);
    Errorflags[0]=0;
    Errorflags[1]=0;
    Errorflags[2]=0;
    Errorflags[3]=0;
    Errorflags[4]=0;
    Errorflags[5]=0;
    Errorflags[6]=0;
    Errorflags[7]=0;
    Errorflags[8]=0;
                    
    uint8_t faultState = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8);
    //printf("status raw=0x%08lX\n", statusValue);
    if (faultState == Bit_SET) {
        // 发生故障，开启灯闪烁
        Errorflags[8]=1;
        byte1 |= (1<<2);
    } else {
        // 正常，关闭闪烁或常亮
        Errorflags[8]=0;
        byte1 |= (0<<2);
    }
    
    // 逐位解析状态寄存器，根据表格定义判断故障
    // DC-DCD 故障（Bit15）
    if (statusValue & (1 << 15)) {
        printf("DC-DC D ERR\n");
        Rangeflags[7]=1;
    }
    // DC-DCC 故障（Bit14）
    if (statusValue & (1 << 14)) {
        printf("DC-DC C ERR\n");
        Rangeflags[6]=1;
    }
    // DC-DCB 故障（Bit13）
    if (statusValue & (1 << 13)) {
        printf("DC-DC B ERR\n");
        Rangeflags[5]=1;
    }
    // DC-DCA 故障（Bit12）
    if (statusValue & (1 << 12)) {
        printf("DC-DC A ERR\n");
        Rangeflags[4]=1;
    }
    // 用户切换（Bit11）
    if (statusValue & (1 << 11)) {
        
    }
    // PEC 错误（Bit10）
    if (statusValue & (1 << 10)) {
        printf("PECERR\n");
    }
    // 斜坡有效（Bit9）
    if (statusValue & (1 << 9)) {
       
    }
    // 过热（Bit8）
    if (statusValue & (1 << 8)) {
        //printf("Overheating\n");
        byte1 |= (1<<1);
    }
    // VOUT_D 故障（Bit7）
    if (statusValue & (1 << 7)) {
        //printf("VOUT_D ERROR\n");
        Rangeflags[7]=1;
        byte0 |= (1<<7);
    }
    // VOUT_C 故障（Bit6）
    if (statusValue & (1 << 6)) {
        //printf("VOUT_C ERROR\n");
        Rangeflags[6]=1;
        byte0 |= (1<<6);
    }
    // VOUT_B 故障（Bit5）
    if (statusValue & (1 << 5)) {
        //printf("VOUT_B ERROR\n");
        Rangeflags[5]=1;
        byte0 |= (1<<5);
    }
    // VOUT_A 故障（Bit4）
    if (statusValue & (1 << 4)) {
        //printf("VOUT_A ERROR\n");
        Rangeflags[4]=1;
        byte0 |= (1<<4);
    }
    // IOUT_D 故障（Bit3）
    if (statusValue & (1 << 3)) {
        //printf("IOUT_D ERR\n");
        Rangeflags[7]=1;
        byte0 |= (1<<3);
    }
    // IOUT_C 故障（Bit2）
    if (statusValue & (1 << 2)) {
        //printf("IOUT_C ERR\n");
        Rangeflags[6]=1;
        byte0 |= (1<<2);
    }
    // IOUT_B 故障（Bit1）
    if (statusValue & (1 << 1)) {
        //printf("IOUT_B ERR\n");
        Rangeflags[5]=1;
        byte0 |= (1<<1);
    }
    // IOUT_A 故障（Bit0）
    if (statusValue & (1 << 0)) {
        //printf("IOUT_A ERR\n");
        Rangeflags[4]=1;
        byte0 |= (1<<0);
    }
    
    IOLinkEventInfo[2] = byte0;
    IOLinkEventInfo[3] = byte1;
}

void UpdateIndicatorLights() {
    for (uint8_t channel = 0; channel < 9; channel++) {
        // 根据 RangeFlags 和 ErrorFlags 的状态决定指示灯模式
         uint8_t mode;
        
        if (Errorflags[channel]) {
            mode = 2;   // 有错误：快速闪烁
            //printf("CHANNEL:%d,Errorflags[channel]:%d\n",channel,Errorflags[channel]);
        } else if (Rangeflags[channel]) {
            mode = 1;     // 正常范围：常亮
            //printf("CHANNEL:%d,Rangeflags[channel]:%d\n",channel,Rangeflags[channel]);
        } else {
            mode = 0;    // 无信号：熄灭
        }
        //printf("CHANNEL:%d,MODE:%d\n",channel,mode);
        AIAOSetIndictor(channel, mode);  // 假设通道编号从1开始
    }
}

#endif


