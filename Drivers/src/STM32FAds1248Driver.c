/*******************************************************************
*文件名称:  STM32FAds1248Driver.c
*文件标识:
*内容摘要:
*其它说明:
*当前版本:
*作    者: 
*修改记录1:
*    修改日期:
*    版 本 号:
*    修 改 人:
*    修改内容:
******************************************************************/


/******************************************************************
*                             头文件                              *
******************************************************************/
#include "STM32FAds1248Driver.h"
#include "stm32f0xx_dma.h"
#include "STM32F0Ads8331Driver.h"
#include "stm32f0xx_conf.h"
#include "Iocontrol.h"
#include "STM32F0usartopt.h"

#if TEMPERATURE_MODULE_CODE

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/

/******************************************************************
*                           数据类型                              *
******************************************************************/
#define ADSCOMPENSATIONDATA  8388608
#define ADSREFERENCEVOLTAGE  2500
#define INDEXINGTABLEADDENDFACTOR   1000000000
#define INDEXINGTABLEMULTIFACTOR    1000

#define  PRECISION_MAGNIFICATION    100000
#define  PRECISION_MAGNIFICATION_1000  1000
#define  PRECISION_MAGNIFICATION_10    10

#define  MCUREFERENCEVOLTAGE  1225
#define  VOLTAGE   33000000
/*冷端补偿转换系数*/
#define  COLDENDVOLTAGE    2048//3291
#define  RESOLUTIONBITLENGTH_12   12
#define  TABLETYPELENGTH   12
#define  DMA_BUFFER_LENGTH   16


#define FSCBYTELENGTH       5
#define GAINBYTELENGTH      3

#define  TYPE_B_FSC_CMD           0x47020000
#define  TYPE_B_SYS_CMD           0x40430079


#define  TYPE_K_FSC_CMD        0x00000247  
#define  TYPE_K_SYS_CMD        0x59004340   

#define CHANNELCFGLENGTH    3

#define RDATACMD         0xffffff12     
#define RDATACMDLENGTH      4


#define DATARADYPORT       GPIOA
#define DRDAYPIN           GPIO_Pin_2


#define  WRITECMD                 0x40
#define  WRITEBYTENUM             0x00

#define  CMD_VALUE_0       0x01
#define  CMD_VALUE_2       0x25
#define  CMD_VALUE_4       0x37
#define  CMD_VALUE_6       0x13

#define FSCANDPGAVALUE_0   0x47
#define FSCANDPGAVALUE_1   0x00
#define FSCANDPGAVALUE_2   0x00
#define FSCANDPGAVALUE_3   0x48
#define FSCANDPGAVALUE_4   0x00
#define FSCANDPGAVALUE_5   0x00
#define FSCANDPGAVALUE_6   0x49
#define FSCANDPGAVALUE_7   0x00
#define FSCANDPGAVALUE_8   0x40
#define FSCANDPGAVALUE_9   0x43
#define FSCANDPGAVALUE_10  0x00
#define FSCANDPGAVALUE_32  0x55
#define FSCANDPGAVALUE_64  0x69
#define FSCANDPGAVALUE_128  0x79

#define VBIASCMD   0x41
#define VBIASVALUE  0xaa
#define FSCANDPGALENGTH   18

/*新增FR3924使用的宏*/
#if (IO_CODE_TYPE == FR3924_CODE)
#define READDATATIMES         27//8//16
#define READDATATIMES_COMPENSATION         5//16


#define REFSELT_VALUE   0x30

#define  RTU_USER_CHANNEL_1         0x37
#define  RTU_USER_CHANNEL_2         0x1a
#define  RTU_USER_CHANNEL_3         0x37
#define  RTU_USER_CHANNEL_4         0x1a

#define GPIO_MODE   0x03
#define GPIO_OUT    0x00


/*GPIO1-GPIO0: 01*/
#define  RTU_USER_GPIO_SWITCH_1    0x01
/*GPIO1-GPIO0: 10*/
#define  RTU_USER_GPIO_SWITCH_2    0x02

#define RTU_USER_INIT_PARA_LEN      30
#define RTU_USER_VBIAS_VALUE         0x84

#define  RTU_COMPENSATION_CHANNEL_1   0x05
#define  RTU_COMPENSATION_CHANNEL_2   0x0D
#define  RTU_COMPENSATION_CHANNEL_3   0x0D
#define  RTU_COMPENSATION_CHANNEL_4   0x25

#define TMEPERATURE_CHANNEL_0    0
#define COMPENSATION_CHANNEL_0   1

#define TMEPERATURE_CHANNEL_1    2
#define COMPENSATION_CHANNEL_1   3

#define TMEPERATURE_CHANNEL_2    4
#define COMPENSATION_CHANNEL_2   5

#define TMEPERATURE_CHANNEL_3    6
#define COMPENSATION_CHANNEL_3   7

#define RTU_PGA_128_RATE_160     0x77  
#define RTU_PGA_64_RATE_160      0x67  
#define RTU_PGA_32_RATE_160      0x57  
#define RTU_PGA_16_RATE_160      0x47  
#define RTU_PGA_1_RATE_160       0x07

#define RTU_CHANNEL_CONFIG_LEN     9
#define RTU_CHANNEL_CONFIG_NO_GPIO_LEN     6
#define RTU_COMPENSATION_CONFIG_LEN  6

#define RTU_SAMPLE_REFVOLTAGE     2048
#define  RESOLUTIONBITLENGTH_23  23

#define TABLE_GAIN_VALUE_10000         10000
#define TABLE_GAIN_VALUE_5000          5000

#define TEMPERATURE_INTERVAL_2          2
#endif

uint8_t sucSampleTimes = 0;    


/******************************************************************
*                         全局变量定义                            *
******************************************************************/
uint8_t gucCurrentIndex= 0;

uint16_t ADS_DMA_RXBUFFER[DMA_BUFFER_LENGTH] = {0};

extern int16_t gusaAnalogData[MAX_AI_CHANNEL_NUMBER];

int32_t giaSPIAIFilterHandleBuffer[SPI_AI_RXBUFFER_SIZE];
extern AISampleInfo_t gtaAISampleInfo[MAX_AI_CHANNEL_NUMBER];
extern  int16_t gsaVlotageSample[MCUSAMPLETIMES];
extern  uint8_t SPI_AI_RxBuffer[SPI_AI_RXBUFFER_SIZE]; 
extern  uint8_t SPI_AI_TxBuffer[SPI_AI_TXBUFFER_SIZE];
extern  uint8_t gucChannelCount;
/*分度表的起始地址*/
uint32_t guiTableAddress = 0;
uint32_t guiTableLength = 0;

#if (IO_CODE_TYPE == FR3924_CODE)

/*热敏电阻的地址*/
uint32_t guiThermistorTableAddress = 0;
uint32_t guiThermistorTablelength = 0;

TableInfo_t gtaTableInfo[TABLETYPELENGTH] = 
{
{E_THERMOCOUPLE_TYPE_K, -100, 1370,32,RTU_PGA_32_RATE_160},
{E_THERMOCOUPLE_TYPE_J, -100, 1200,16,RTU_PGA_16_RATE_160},
{E_THERMOCOUPLE_TYPE_T, -100, 400,64,RTU_PGA_64_RATE_160},
{E_THERMOCOUPLE_TYPE_E, -100, 1000,16,RTU_PGA_16_RATE_160},
{E_THERMOCOUPLE_TYPE_N, -100, 1300,32,RTU_PGA_32_RATE_160},
{E_THERMOCOUPLE_TYPE_S, 0, 1700,64,RTU_PGA_64_RATE_160},
{E_THERMOCOUPLE_TYPE_R, 0, 1700,64,RTU_PGA_64_RATE_160},
{E_THERMOCOUPLE_TYPE_B, 600, 1800,128,RTU_PGA_128_RATE_160},
{E_THERMOCOUPLE_TYPE_C, 0, 2320,32,RTU_PGA_32_RATE_160},
{E_THERMISTOR_TYPE, -10, 70,0}
};

#endif
#if  (IO_CODE_TYPE == FR3822_CODE)
#define READDATATIMES         8

#define PGA_1_DATABAUD_160     0x05
#define PGA_4_DATABAUD_160     0x25
#define PGA_8_DATABAUD_160     0x35
#define PGA_16_DATABAUD_160    0x45


#define RTD_VBIAS_VALUE      0x30    /*5,4为正*/
#define RTD_VBIAS_VALUE_NO   0x00
#define RTD_DRIVING_SOURCE_250   0x03   /*激励电源250uA*/      //     0x02   /*激励电源100uA*/
#define RTD_DRIVING_SOURCE_500   0x04
#define RTD_USER_CHANNEL_1   0x01  //0x04  
#define RTD_USER_CHANNEL_2   0x2C  //0x0D   

#define RTD_USER_CHANNEL_1_BURNOUT   0x81//0xC8
#define RTD_USER_CHANNEL_2_BURNOUT   0xAC//0xEC

#define RTD_USER_CHANNEL_SAMPLE_TIMES    2

#define RTD_USER_CHANNEL_1_REF  0x20
#define RTD_USER_CHANNEL_2_REF  0x28
#define RTD_USER_CHANNEL_1_DRIIVING_SOURCE_1  0x01 //0x04
#define RTD_USER_CHANNEL_1_DRIIVING_SOURCE_2  0x10 //0x40
#define RTD_USER_CHANNEL_2_DRIIVING_SOURCE_1  0x54 //0x15
#define RTD_USER_CHANNEL_2_DRIIVING_SOURCE_2  0x45 //0x51

#define USER_CHANNEL_0     0
#define USER_CHANNEL_1     1

#define RTD_PT100_TABLE_CHANGERULE    1000

#define RTD_PT100_RESISTANCE_REF       1370  //12000
#define RTD_PT100_FULL_SCALE            23

TableInfo_t gtaTableInfo[TABLETYPELENGTH] = 
{
    {E_THERMAL_RESISTANCE_TYPE_PT100, -200, 800,4},
    {E_THERMAL_RESISTANCE_TYPE_PT200, -200, 630,4},
    {E_THERMAL_RESISTANCE_TYPE_PT500, -200, 630,1},
    {E_THERMAL_RESISTANCE_TYPE_PT1000, -50, 300,1},
    {E_THERMAL_RESISTANCE_TYPE_NI120, -79, 309,4},
    {E_THERMAL_RESISTANCE_TYPE_RESISTANCE_MEASUREMENT, 1, 1200,1},
};

DivingSourceInfo_t gaDivingSourceInfo = {0,0,0,0};


#endif

InitStatus_e eInitCompleteFlag = E_INIT_NONE;           


/******************************************************************
*                          局部函数声明                            *
******************************************************************/
#if (IO_CODE_TYPE == FR3924_CODE)
IO_InterfaceResult_e TempConversion(TableType_e ucTypeIn,int32_t *piFilterDataIn);
IO_InterfaceResult_e MCU_TEM_Compensation(int32_t iVoltageIn, int64_t * piTemperatureOut);
void TableLinearFittingReverse(TableLinear_t* ptLinearLinearIn, uint64_t ulLinearValueIn, int32_t* piTableValueOut);
void ChangeVoltageRules(TableType_e ucTypeIn,int64_t* plVoltageInOut);
IO_InterfaceResult_e GetValueFromThermistorTable(TableType_e ucTypeIn,int64_t iCheckValueIn,TableLinear_t * ptLinearLinearOut);
IO_InterfaceResult_e GetTemperatureFromTable(TableType_e ucTypeIn,int32_t iCheckValueIn,TableLinear_t * ptLinearLinearOut);
IO_InterfaceResult_e GetTemperatureFromTable_Calibration(TableType_e ucTypeIn,int32_t iCheckValueIn,TableLinear_t * ptLinearLinearOut);
IO_InterfaceResult_e SetCalibrationValueByThermocoupleType(TableType_e tTypeIn);
void TemperatureOffsetCalc(TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut);
void TemperatureOffsetCalcForThermistor (TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut);

#endif
IO_InterfaceResult_e GetValueFromTable(TableType_e ucTypeIn,int64_t iCheckValueIn,TableLinear_t * ptLinearLinearOut);
void ADS1248_StartConvert(void);
void ADS1248_StopConvert(void);
void UpdateDataArea(int64_t* piTableValueIn);
void ADS1248_Delay(uint32_t nCount);
void TableLinearFitting(TableLinear_t* ptLinearLinearIn, int64_t iLinearValueIn, int64_t* piTableValueOut);

#if (IO_CODE_TYPE == FR3822_CODE)
void GetDataToDivingSource(DivingSourceInfo_t *ptDivingSourceOut,int32_t uiFilterDataIn);
IO_InterfaceResult_e TempConversion(TableType_e ucTypeIn, DivingSourceInfo_t *ptDivingSourceIn);
void ChangeVoltageRules(TableType_e ucTypeIn,int64_t *plResistanceInOut);
void TemperatureOffsetCalc(TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut);


#endif
/******************************************************************
*                         全局函数实现                            *
******************************************************************/

/******************************************************************
*                         局部函数实现                            *
******************************************************************/
/******************************************************************
*函数名称:STM32F0Ads1248DriverConfig
*功能描述:配置SPI DMA GPIO等参数
*输入参数:
*输出参数:STM32F0_SPI_AI_DRIVER_ERROR_e
*返回值:函数执行结果
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/23                              
******************************************************************/

STM32F0_SPI_AI_DRIVER_ERROR_e STM32F0Ads1248DriverConfig(TableType_e tTypeIn,SPI_InitTypeDef  *SPI_InitStructure, GPIO_InitTypeDef  *GPIO_InitStructure, 
                                                         GPIO_InitTypeDef  *GPIO_LOWInitStructure, DMA_InitTypeDef  *DMA_RXInitStructure, DMA_InitTypeDef  *DMA_TXInitStructure)
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
   
    /* Enable the DMA peripheral */
    RCC_AHBPeriphClockCmd(DMA_AI_CLK, ENABLE);
    
    /* Enable TIM DMA trigger clock */ 

    /* Enable SCK, MOSI, MISO and NSS GPIO clocks */
    RCC_AHBPeriphClockCmd(SPI_AI_SCK_GPIO_CLK | SPI_AI_MISO_GPIO_CLK | SPI_AI_MOSI_GPIO_CLK | SPI_AI_NSS_GPIO_CLK | TIM_AI_TRIGGER_GPIO_CLK, ENABLE);

    /*复用管脚配置*/
    GPIO_PinAFConfig(SPI_AI_SCK_GPIO_PORT, SPI_AI_SCK_SOURCE, SPI_AI_SCK_AF);
    GPIO_PinAFConfig(SPI_AI_MOSI_GPIO_PORT, SPI_AI_MOSI_SOURCE, SPI_AI_MOSI_AF);
    GPIO_PinAFConfig(SPI_AI_MISO_GPIO_PORT, SPI_AI_MISO_SOURCE, SPI_AI_MISO_AF);
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
    /*配置START引脚PB9*/
    GPIO_LOWInitStructure->GPIO_Pin = ADS_START_CONTROL_PIN;
    GPIO_Init(ADS_START_GPIO_PORT, GPIO_LOWInitStructure);

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
    GPIO_LOWInitStructure->GPIO_Pin = SPI_AI_NSS_PIN;
    GPIO_Init(SPI_AI_NSS_GPIO_PORT, GPIO_LOWInitStructure);
#if 0 //TEST引脚
    GPIO_InitStructure->GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure->GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure->GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure->GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure->GPIO_Speed = GPIO_Speed_Level_3;
    GPIO_Init(GPIOB, GPIO_InitStructure);
    /*set*/
    GPIO_SetBits(GPIOB,GPIO_Pin_14);
#endif
    /*DRDY触发引脚配置*/
    GPIO_InitStructure->GPIO_Pin = TIM_AI_TRIGGER_PIN; 
    GPIO_InitStructure->GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure->GPIO_PuPd  = GPIO_PuPd_NOPULL;    
    GPIO_Init(TIM_AI_TRIGGER_GPIO_PORT, GPIO_InitStructure);

    /*配置SPI初始化参数*/
    SPI_InitStructure->SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure->SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure->SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure->SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure->SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitStructure->SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure->SPI_CRCPolynomial = SPI_AI_CRCPolynomial;
    SPI_InitStructure->SPI_Mode = SPI_Mode_Master;

    /*配置DMA接收初始化参数*/
    DMA_RXInitStructure->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_RXInitStructure->DMA_MemoryDataSize =  DMA_MemoryDataSize_Byte;
    DMA_RXInitStructure->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_RXInitStructure->DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_RXInitStructure->DMA_Mode = DMA_Mode_Normal;
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
    DMA_TXInitStructure->DMA_Mode = DMA_Mode_Normal;
    DMA_TXInitStructure->DMA_M2M = DMA_M2M_Disable;
    DMA_TXInitStructure->DMA_BufferSize = SPI_AI_DMA_SENDSIZE;
    DMA_TXInitStructure->DMA_PeripheralBaseAddr = (uint32_t)SPI_AI_DR_ADDRESS;
    DMA_TXInitStructure->DMA_MemoryBaseAddr = (uint32_t)SPI_AI_TxBuffer;
    DMA_TXInitStructure->DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_TXInitStructure->DMA_Priority = DMA_Priority_Low;
    return E_STM32F0_AI_SPI_OK;
}


void ADS1248_Delay(uint32_t nCount)
{
    uint32_t i, j;

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < nCount; j++);
    }
}


/******************************************************************
*函数名称:STM32F0Ads1248DriverInit
*功能描述:初始化SPI DMA GPIO等结构体
*输入参数:
*输出参数:STM32F0_SPI_AI_DRIVER_ERROR_e
*返回值:函数执行结果
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/23                              
******************************************************************/
STM32F0_SPI_AI_DRIVER_ERROR_e STM32F0Ads1248DriverInit(TableType_e tTypeIn,SPI_InitTypeDef  *SPI_InitStructure, GPIO_InitTypeDef  *GPIO_InitStructure, 
                                                      GPIO_InitTypeDef  *GPIO_LOWInitStructure, DMA_InitTypeDef  *DMA_RXInitStructure, DMA_InitTypeDef  *DMA_TXInitStructure)
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

    /*拉高片选*/ 
    GPIO_SetBits(SPI_AI_NSS_GPIO_PORT,SPI_AI_NSS_PIN);

    /* Initializes the SPI communication */
    SPI_Init(SPI_AI, SPI_InitStructure);
    /*使能nss脉冲*/
    SPI_NSSPulseModeCmd(SPI_AI, ENABLE);

    /* Initialize the FIFO threshold */
    SPI_RxFIFOThresholdConfig(SPI_AI, SPI_RxFIFOThreshold_QF);
    /*DMA初始化*/
    DMA_Init(SPI_AI_RX_DMA_CHANNEL, DMA_RXInitStructure);
    DMA_Init(SPI_AI_TX_DMA_CHANNEL, DMA_TXInitStructure);

    /*使能DMA请求*/
    SPI_I2S_DMACmd(SPI_AI, SPI_I2S_DMAReq_Rx, ENABLE);
    SPI_I2S_DMACmd(SPI_AI,SPI_I2S_DMAReq_Tx,ENABLE);
    
    /*使能SPI*/
    SPI_Cmd(SPI_AI, ENABLE);

    /*拉低START*/    
    GPIO_ResetBits(ADS_START_GPIO_PORT,ADS_START_CONTROL_PIN);

    /*芯片上电到MAIN函数第一步，需要8ms   第一条语句到执行SPI发送第一个数据
          延时8.399825ms     启动到开始配置ADS时间大概  8ms + 8.399825ms*/
    ADS1248_Delay(13000);

    
    /*禁能DMA通道*/
    DMA_Cmd(SPI_AI_TX_DMA_CHANNEL,DISABLE);    
    DMA_Cmd(SPI_AI_RX_DMA_CHANNEL,DISABLE);
    
    SPI_AI_TxBuffer[0] =ADS_RESET;
    
    DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,1);   
    DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,1);

    /*使能DMA通道*/
    SendDMACmd();
    /*复位之后需要等待0.6ms*/
    ADS1248_Delay(2000);
    
    ADS1248_StartConvert();
    
    /*发送共用的参数*/
#if (IO_CODE_TYPE == FR3924_CODE)   
    /*根据传感器类型初始化通道校准值*/
    (void)SetCalibrationValueByThermocoupleType(tTypeIn);
    /*根据传感器类型初始化配置参数*/
    ConfigureDataByThermocoupleType(tTypeIn);
#endif

#if (IO_CODE_TYPE == FR3822_CODE)
    ConfigureDataByThermalResistanceType(tTypeIn);
#endif
    /*置状态为RUNING*/ 
    gtaAISampleInfo[TMEPERATURE_CHANNEL_0].eChannelStatus =  E_CHANNEL_RUNNING;
    for(uint8_t i=0;i<MAX_AI_CHANNEL_NUMBER;i++)
    {
        gtaAISampleInfo[i].ucConnectionFlag = ADS_DISCONNECTION;
    }
    return E_STM32F0_AI_SPI_OK;
}


/******************************************************************
*函数名称:void SendDMACmd(void)
*功能描述:使能DMA发送函数
*输入参数:void
*输出参数: void
*返回值: 无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/31                              
******************************************************************/
void SendDMACmd(void)
{
    uint8_t i = 0;

    /*关闭串口中断*/    
    asm ("CPSID   I");

    /*拉低片选*/
    SPI_AI_NSS_GPIO_PORT->BRR = SPI_AI_NSS_PIN;

    /*启动DMA_RX接收*/
    SPI_AI_RX_DMA_CHANNEL->CCR |= DMA_CCR_EN;
    /*启动DMA_TX发送*/
    SPI_AI_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;

    while(SET != DMA_GetFlagStatus(DMA1_FLAG_TC2))
    {
        /*清除主站与其它从站交互导致的串口中断标志位*/
         if ( (SET == USART_GetITStatus(USART1, USART_IT_RTO)))
         {
             /*未接收到字节则直接退出*/
             if (DMA_RECEIVE_BUFSIZE == DMA1_Channel5->CNDTR)
             {
                 /*  清除超时中断标志*/
                 USART1->ICR = USART_FLAG_RTO;
             }
         }
    }
    DMA_ClearFlag(DMA1_FLAG_TC2);

 
    /* 延时1.7微秒以上*/
    for (i=0; i< 15; i++);
    
    /*拉高片选*/
    SPI_AI_NSS_GPIO_PORT->BSRR = SPI_AI_NSS_PIN;

    /*打通串口中断*/    
    asm ("CPSIE   I");

}


/******************************************************************
*函数名称:void UpdateDataArea(int64_t* piTableValueIn)
*功能描述:更新用户业务数据区  并以四舍五入的方式进行精度保留
*输入参数:piTableValueIn  温度数据地址
*输出参数:无
*返回值: void
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/19                              
******************************************************************/
void UpdateDataArea(int64_t* piTableValueIn)
{
    int32_t iTemperature = *piTableValueIn; //温度被放大100000
    int8_t cDataTemp = 0;
    /*对数据进行四舍五入保留两位小数*/
    iTemperature = iTemperature/PRECISION_MAGNIFICATION_1000;
    /*取出最后一位小数并判断是否大于4*/
    cDataTemp = iTemperature%PRECISION_MAGNIFICATION_10;
    if(cDataTemp > 4)
    {
        /*对数据进行进一位处理*/
        iTemperature = iTemperature + PRECISION_MAGNIFICATION_10;
    }
    if(cDataTemp <= -5)
    {        
        /*对数据进行进一位处理*/
        iTemperature = iTemperature - PRECISION_MAGNIFICATION_10;
    }
    /*最后用户使用的数据为保留一位小数*/
    iTemperature = iTemperature/PRECISION_MAGNIFICATION_10;
    
    /*更新数据区*/
#if (IO_CODE_TYPE == FR3924_CODE)   
    gusaAnalogData[gucCurrentIndex/2] = (int16_t)(iTemperature);
#endif
#if (IO_CODE_TYPE == FR3822_CODE)
    gusaAnalogData[gucCurrentIndex/2] = (int16_t)(iTemperature);
#endif
    return;
}

/******************************************************************
*函数名称:void TableLinearFitting(TableLinear_t* ptLinearLinearIn, int64_t iLinearValueIn, int64_t* piTableValueOut)
*功能描述: 线性拟合计算
*输入参数:TableLinear_t 拟合区间 iLinearValueIn 拟合的值 piTableValueOut拟合结果地址
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/13                              
******************************************************************/
void TableLinearFitting(TableLinear_t* ptLinearLinearIn, int64_t iLinearValueIn, int64_t* piTableValueOut)
{
    int64_t lLineartemp = iLinearValueIn;//;PRECISION_MAGNIFICATION
    int64_t sTemp = 0;
    /*保存精度  放大100000*/
    sTemp = (lLineartemp - ptLinearLinearIn->usaLinearValue1)*PRECISION_MAGNIFICATION/(int32_t)((ptLinearLinearIn->usaLinearValue2 - ptLinearLinearIn->usaLinearValue1));

    *piTableValueOut = (ptLinearLinearIn->lTemperature_1)*PRECISION_MAGNIFICATION + sTemp*(ptLinearLinearIn->lTemperature_2 - ptLinearLinearIn->lTemperature_1);
    return;
}

/******************************************************************
*函数名称:void ADS1248_StartConvert(void)
*功能描述:拉高START
*输入参数:void
*输出参数:无
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/11                              
******************************************************************/
void ADS1248_StartConvert(void)
{
    /*START引脚拉高*/
    GPIO_SetBits(ADS_START_GPIO_PORT,ADS_START_CONTROL_PIN);
    return;
}

/******************************************************************
*函数名称:void ADS1248_StopConvert(void)
*功能描述:拉低START引脚
*输入参数:void
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/31                              
******************************************************************/
void ADS1248_StopConvert(void)
{
    /*START引脚拉低*/
    GPIO_ResetBits(ADS_START_GPIO_PORT,ADS_START_CONTROL_PIN);
    return;
}

/******************************************************************
*函数名称:IO_InterfaceResult_e ADS1248_lOOP(TableType_e ucTypeIn)
*功能描述:ADS1248循环工作主函数 
*输入参数:ucTypeIn   表格类型
*输出参数:IO_InterfaceResult_e 
*返回值:      E_IO_ERROR_NONE  无错误 
                         E_IO_ERROR_TABLE_OVERFLOW  数据溢出 
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/11                              
******************************************************************/
IO_InterfaceResult_e ADS1248_lOOP(TableType_e ucTypeIn)
{
    STM32F0_SPI_AI_DRIVER_ERROR_e eAiSpiDrvierReturnCode = E_STM32_AI_SPI_BUTT;
    SPI_InitTypeDef     SPI_InitStructure;
    GPIO_InitTypeDef    GPIO_InitStructure;
    GPIO_InitTypeDef    GPIO_LOWInitStructure;
    NVIC_InitTypeDef    NVIC_InitStructure;
    DMA_InitTypeDef     DMA_RXInitStructure;
    DMA_InitTypeDef     DMA_TXInitStructure;
#if (IO_CODE_TYPE == FR3924_CODE)   
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;
    int64_t iTemperature = 0;
    int32_t iVoltageTemp = 0; 
    uint64_t ullDataTemp = 0;
    TableLinear_t tTableLinear;
    int64_t liSampleSum = 0;
#endif    
    int32_t iFilterData = 0;

    if (eInitCompleteFlag == E_INIT_NONE)
    {
        return E_IO_ERROR_NONE;
    }

    if (eInitCompleteFlag == E_INIT_MCU_COMPLETE)
    {

        /* AI SPI驱动使用的结构体初始化 */
        memset ( &SPI_InitStructure, 0, sizeof ( SPI_InitTypeDef ) );
        memset ( &GPIO_InitStructure, 0, sizeof ( GPIO_InitTypeDef ) );
        memset ( &GPIO_LOWInitStructure, 0, sizeof ( GPIO_InitTypeDef ) );
        memset ( &NVIC_InitStructure, 0, sizeof ( NVIC_InitTypeDef ) );
        memset ( &DMA_RXInitStructure, 0, sizeof ( DMA_InitTypeDef ) );
        memset ( &DMA_TXInitStructure, 0, sizeof ( DMA_InitTypeDef ) );

        /*  SPI驱动初始化 */
        eAiSpiDrvierReturnCode = STM32F0Ads1248DriverConfig(ucTypeIn,&SPI_InitStructure, &GPIO_InitStructure, 
                 &GPIO_LOWInitStructure, &DMA_RXInitStructure, &DMA_TXInitStructure);
        if (E_STM32F0_AI_SPI_OK != eAiSpiDrvierReturnCode)
        {
            return E_IO_ERROR_ADS_SPI_DRIVER_CONFIG;
        }

        eAiSpiDrvierReturnCode = STM32F0Ads1248DriverInit(ucTypeIn,&SPI_InitStructure, &GPIO_InitStructure, 
                 &GPIO_LOWInitStructure,  &DMA_RXInitStructure, &DMA_TXInitStructure);
        if (E_STM32F0_AI_SPI_OK != eAiSpiDrvierReturnCode)
        {
            return E_IO_ERROR_ADS_SPI_DRIVER_INIT;
        }
   
        eInitCompleteFlag = E_INIT_DRIVER_COMPLETE;
    }


    /*判断是否需要启动当前通道*/
    if(E_CHANNEL_INITIALIZATION ==gtaAISampleInfo[gucCurrentIndex].eChannelStatus)
    {    
       /*启动ADS转换*/
        ADS1248_StartConvert();
#if (IO_CODE_TYPE == FR3924_CODE)   
        /*配置当前通道信息*/
        ConfigureDataToADS(gucCurrentIndex,ucTypeIn);
#endif
#if (IO_CODE_TYPE == FR3822_CODE)   
        /*配置当前通道信息*/
        ConfigureDataToADS(gucCurrentIndex);
#endif

        /*置当前通道为RUNING状态*/
        gtaAISampleInfo[gucCurrentIndex].eChannelStatus =  E_CHANNEL_RUNNING;
        
     }
    /*判断当前通道是不是RUNNING状态*/
    if(E_CHANNEL_RUNNING ==gtaAISampleInfo[gucCurrentIndex].eChannelStatus)
    {
        /*检测DRDY为低*/
        if(Bit_RESET == (DATARADYPORT->IDR & DRDAYPIN))
        {
            /*获取ADS数据*/
            giaSPIAIFilterHandleBuffer[sucSampleTimes] = 0;
            giaSPIAIFilterHandleBuffer[sucSampleTimes] = ((uint32_t)(SPI_AI_RxBuffer[1]) << 16) | ((uint32_t)(SPI_AI_RxBuffer[2]) << 8) | SPI_AI_RxBuffer[3];
            if (giaSPIAIFilterHandleBuffer[sucSampleTimes] & 0x00800000)
            {
                giaSPIAIFilterHandleBuffer[sucSampleTimes] = giaSPIAIFilterHandleBuffer[sucSampleTimes] | 0xFF000000;
            }
            
            /*禁能DMA*/            
            DMA_Cmd(SPI_AI_TX_DMA_CHANNEL,DISABLE); 
            DMA_Cmd(SPI_AI_RX_DMA_CHANNEL,DISABLE); 
            
            DMA_ClearFlag(DMA1_FLAG_GL2);
            DMA_ClearFlag(DMA1_FLAG_GL3);
            /*发送DMA_TXBUFFER*/
            /*组装发送数据*/
            *(uint32_t *)(SPI_AI_TxBuffer) = RDATACMD;
            /*设置DMA发送的长度*/
            DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RDATACMDLENGTH);
            DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RDATACMDLENGTH);
    
            /*使能DMA通道*/
            SendDMACmd();

            /*读数据次数增1*/
            sucSampleTimes++;
        }
#if (IO_CODE_TYPE == FR3822_CODE) 
        /*为了减少对数据更新周期影响，断连检测只采集两组数据*/
        if ( TMEPERATURE_CHANNEL_4 == gucCurrentIndex || TMEPERATURE_CHANNEL_5 ==gucCurrentIndex)
        {
            if (RTD_USER_CHANNEL_SAMPLE_TIMES == sucSampleTimes)
            {
                /*停止ADS转换*/
                ADS1248_StopConvert();
                /*次数清零*/
                sucSampleTimes = 0;
                /*置当前通道为CALC状态*/
                gtaAISampleInfo[gucCurrentIndex].eChannelStatus = E_CHANNEL_CALCATING;
            }
        }

        /*判断当前采样次数*/
        if(READDATATIMES == sucSampleTimes)
        {
            /*停止ADS转换*/
            ADS1248_StopConvert();
            /*次数清零*/
            sucSampleTimes = 0;
                                             
            /*置当前通道为CALC状态*/
            gtaAISampleInfo[gucCurrentIndex].eChannelStatus = E_CHANNEL_CALCATING;
        }
 #endif    

#if (IO_CODE_TYPE == FR3924_CODE) 

        /*对通道采样值进行如下计算*/
        if ( TMEPERATURE_CHANNEL_0 ==gucCurrentIndex || TMEPERATURE_CHANNEL_1 ==gucCurrentIndex
             || TMEPERATURE_CHANNEL_2 ==gucCurrentIndex || TMEPERATURE_CHANNEL_3 ==gucCurrentIndex )
        {
            /*判断当前采样次数*/
            if(READDATATIMES == sucSampleTimes)
            {
                /*停止ADS转换*/
                ADS1248_StopConvert();
                /*次数清零*/
                sucSampleTimes = 0;
                                                 
                /*置当前通道为CALC状态*/
                gtaAISampleInfo[gucCurrentIndex].eChannelStatus = E_CHANNEL_CALCATING;  
            }
        }
        /*冷端补偿只采样一次数据*/
        else
        {
            /*判断当前采样次数*/
            if(READDATATIMES_COMPENSATION == sucSampleTimes)
            {
                /*停止ADS转换*/
                ADS1248_StopConvert();
                /*次数清零*/
                sucSampleTimes = 0;
                                                 
                /*置当前通道为CALC状态*/
                gtaAISampleInfo[gucCurrentIndex].eChannelStatus = E_CHANNEL_CALCATING;      
            }
        }

  #endif      
    }
    /*判断当前通道是否可以处理*/
    if(E_CHANNEL_CALCATING ==gtaAISampleInfo[gucCurrentIndex].eChannelStatus)
    {    
#if (IO_CODE_TYPE == FR3822_CODE) 

        /*滤波并将滤波数据保存返回*/
        if ( TMEPERATURE_CHANNEL_4 == gucCurrentIndex || TMEPERATURE_CHANNEL_5 ==gucCurrentIndex)
        {
            /*采集两次，第一次为垃圾数据，保存第二次数据用于断连检测*/
            gtaAISampleInfo[gucCurrentIndex].ulChannelDisconnectValue = giaSPIAIFilterHandleBuffer[ADS_DISCONNECTION_VALIED_VALUE];
        }
        else
        {
            ArithmeticAverageFilterCalc(giaSPIAIFilterHandleBuffer,&iFilterData);
        }
#endif      
#if (IO_CODE_TYPE == FR3924_CODE) 
        /*判断是否需要冷端补偿*/
        if ( COMPENSATION_CHANNEL_0 ==gucCurrentIndex || COMPENSATION_CHANNEL_1 ==gucCurrentIndex
             || COMPENSATION_CHANNEL_2 ==gucCurrentIndex || COMPENSATION_CHANNEL_3 ==gucCurrentIndex )
        {
            /*采集两次，第一次为垃圾数据，保存第二次数据用于冷端补偿*/
            for (uint8_t i = 0; i < READDATATIMES_COMPENSATION - 1; i++)
            {
                liSampleSum += giaSPIAIFilterHandleBuffer[i+1];

            }
            /*5个数据 1个垃圾数据不丢弃*/
            iFilterData = liSampleSum/(READDATATIMES_COMPENSATION-1);
        }
        else
        {
            ArithmeticAverageFilterCalc(giaSPIAIFilterHandleBuffer,&iFilterData);
        }
#endif 

#if (IO_CODE_TYPE == FR3822_CODE) 
        if ( TMEPERATURE_CHANNEL_4 != gucCurrentIndex || TMEPERATURE_CHANNEL_5 !=gucCurrentIndex)
        {
            /*保存数据*/
            GetDataToDivingSource(&gaDivingSourceInfo,iFilterData);
        }
        /*在温度计算前进行断连判断*/
        
        /*温度计算 调试第一种类型表格*/
        (void)TempConversion(ucTypeIn,&gaDivingSourceInfo);
#endif

#if (IO_CODE_TYPE == FR3924_CODE) 
        /*温度计算 调试第一种类型表格*/
        (void)TempConversion(ucTypeIn,&iFilterData);
#endif        

        /*置当前通道为INIT*/
        gtaAISampleInfo[gucCurrentIndex].eChannelStatus = E_CHANNEL_INITIALIZATION;

#if (IO_CODE_TYPE == FR3822_CODE) 
        /*当前通道进行偏移 偏移的顺序0->1->2->3->0->1.....*/
        gucCurrentIndex = (gucCurrentIndex+1)%6;
        if (0 == gucCurrentIndex)
        {
            eInitCompleteFlag = E_LED_SET_COMPLETE;

            if ((ADSDISCONNECTIONVALUE ==gtaAISampleInfo[TMEPERATURE_CHANNEL_4].ulChannelDisconnectValue) 
                  || (ADSDISCONNECTIONVALUE_tvs == gtaAISampleInfo[TMEPERATURE_CHANNEL_4].ulChannelDisconnectValue))
            {
                gtaAISampleInfo[0].ucConnectionFlag = ADS_DISCONNECTION;
            }

            if ((ADSDISCONNECTIONVALUE ==gtaAISampleInfo[TMEPERATURE_CHANNEL_5].ulChannelDisconnectValue) 
                  || (ADSDISCONNECTIONVALUE_tvs == gtaAISampleInfo[TMEPERATURE_CHANNEL_5].ulChannelDisconnectValue))
            {
                gtaAISampleInfo[1].ucConnectionFlag = ADS_DISCONNECTION;
            }
        }
#endif
        /*FR3924有冷端补偿 冷端补偿由ADS处理*/
#if (IO_CODE_TYPE == FR3924_CODE) 
        /*判断是否需要冷端补偿*/
        if ( COMPENSATION_CHANNEL_0 ==gucCurrentIndex || COMPENSATION_CHANNEL_1 ==gucCurrentIndex
             || COMPENSATION_CHANNEL_2 ==gucCurrentIndex || COMPENSATION_CHANNEL_3 ==gucCurrentIndex )      
        {         
            memset(&tTableLinear,0,sizeof(tTableLinear));

            /*码值转换成电压值*/
            ullDataTemp = (uint64_t)iFilterData;
            ullDataTemp = ullDataTemp*COLDENDVOLTAGE >>RESOLUTIONBITLENGTH_23;
            iVoltageTemp = (int32_t)ullDataTemp;
            /*热敏电阻对应的温度*/
            eResult = MCU_TEM_Compensation(iVoltageTemp,&iTemperature);
            if(E_IO_ERROR_NONE != eResult)
            {
                /*当前通道进行偏移 偏移到下一个*/
                gucCurrentIndex = (gucCurrentIndex+1)%8;
                return eResult;
            }
            /*通过温度查分度表 得到对应的电压*/
            eResult = GetTemperatureFromTable(ucTypeIn,iTemperature,&tTableLinear);
            if(E_IO_ERROR_NONE != eResult)
            {
                /*当前通道进行偏移 偏移到下一个*/
                gucCurrentIndex = (gucCurrentIndex+1)%8;
                /*数据超限  不需要进行线性拟合*/
                return eResult;
            }
            /*线性拟合  查表反向  知道温度，拟合出分度表对应的电压*/
            TableLinearFittingReverse(&tTableLinear,iTemperature,&gtaAISampleInfo[gucCurrentIndex].lCompensationValue); 
            /*增加对冷端补偿的标定*/
            gtaAISampleInfo[gucCurrentIndex].lCompensationValue -= gtaAISampleInfo[gucCurrentIndex].lCalibrationValue;
            
        }
        
        /*当前通道进行偏移 偏移到下一个*/
        gucCurrentIndex = (gucCurrentIndex+1)%8;
        if (0 == gucCurrentIndex)
        {
            eInitCompleteFlag = E_LED_SET_COMPLETE;

            
        }
#endif    
    }
    return E_IO_ERROR_NONE;
}
#if (IO_CODE_TYPE == FR3924_CODE)

/******************************************************************
*函数名称:void TemperatureOffsetCalc(TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut)
*功能描述:通过表的类型 对温度进行偏移计算
*输入参数:ucTypeIn  表格类型   ptLinearLinearInOut线性区间的地址 
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/17                              
******************************************************************/
/*改变入参  只传入INDEX  */
void TemperatureOffsetCalc(TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut)
{
    /*对温度进行偏移*/  
        /*FR3924FLASH中存放的数据是以2摄氏度为间隔存放，与FR3822存入格式不相同*/
    ptLinearLinearInOut->lTemperature_1 = (ptLinearLinearInOut->lTemperature_1)*TEMPERATURE_INTERVAL_2 +gtaTableInfo[ucTypeIn].sStartTemperature;
    ptLinearLinearInOut->lTemperature_2 = (ptLinearLinearInOut->lTemperature_2)*TEMPERATURE_INTERVAL_2 +gtaTableInfo[ucTypeIn].sStartTemperature;
    return;
}

/******************************************************************
*函数名称:void TemperatureOffsetCalc(TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut)
*功能描述:通过表的类型 对温度进行偏移计算
*输入参数:ucTypeIn  表格类型   ptLinearLinearInOut线性区间的地址 
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/17                              
******************************************************************/
/*改变入参  只传入INDEX  */
void TemperatureOffsetCalcForThermistor (TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut)
{
    /*对温度进行偏移*/  
        /*FR3924FLASH中存放的数据是以2摄氏度为间隔存放，与FR3822存入格式不相同*/
    ptLinearLinearInOut->lTemperature_1 = ptLinearLinearInOut->lTemperature_1 +gtaTableInfo[ucTypeIn].sStartTemperature;
    ptLinearLinearInOut->lTemperature_2 = ptLinearLinearInOut->lTemperature_2 +gtaTableInfo[ucTypeIn].sStartTemperature;
    return;
}

/******************************************************************
*函数名称:IO_InterfaceResult_e GetValueFromTable(TableType_e ucTypeIn,int64_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
*功能描述:二分法查表
*输入参数:ucTypeIn 热电偶类型  iCheckValueIn 查表值 ptLinearLinearOut 线性区间地址
*输出参数:IO_InterfaceResult_e
*返回值:      E_IO_ERROR_TABLE_DATA  数据溢出
                         E_IO_ERROR_NONE          无错误 
*其它说明:
*修改日期    版本号   修改人    修改内容
                                                                  不将表格内容当成数据传入进来，直接从FLASH中读取 需要确定读取哪l表
*---------------------------------------------------
*2018/3/13                              
******************************************************************/
IO_InterfaceResult_e GetValueFromTable(TableType_e ucTypeIn,int64_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;
    uint16_t usLowIndex = 0;
    /*保存二分法循环变量*/
    uint16_t  usMiddleIndex = 0;
    uint16_t usHighIndex = 0;
    /*定义变量保存从FLASH中读取的数据*/
    uint32_t ulMaxValue = 0;
    uint32_t ulMinVALue = 0;
    uint16_t usCalcTemp1 = 0;
    uint16_t usCalcTemp2 = 0;
    /*比较变量*/
    uint64_t usCompareValue = 0;
    /*定义变量保存地址*/
    uint32_t uiAddressTemp = 0;

    /*缩小100000倍  需要缩小*/
    iCheckValueIn = iCheckValueIn/PRECISION_MAGNIFICATION;
    usCompareValue = iCheckValueIn;
    usHighIndex = guiTableLength/2 -1;
    memcpy(&ulMinVALue,(void *)(guiTableAddress),2);
    memcpy(&ulMaxValue,(void *)(guiTableAddress + (usHighIndex)*2),2);
    uiAddressTemp = guiTableAddress;
    
    if((usCompareValue < ulMinVALue) || (usCompareValue > ulMaxValue))
    {
        eResult = E_IO_ERROR_TABLE_DATA;
        return eResult; // 超限，不做特殊处理
    }

    while(usHighIndex >= usLowIndex)
    {
        usMiddleIndex = (usHighIndex + usLowIndex) / 2;
        
        /*mid的值有变化，重新读取对应FLASH中的地址*/
        memcpy(&usCalcTemp1,(void *)(uiAddressTemp + (usMiddleIndex)*2),2);

        if((usCompareValue >= usCalcTemp1) || (0 == usMiddleIndex))
        {
            /*计算Middle+1的值*/            
            memcpy(&usCalcTemp2,(void *)(uiAddressTemp + (usMiddleIndex+1)*2 ),2);
            if(usCompareValue <= usCalcTemp2)
            {
                ptLinearLinearOut->lTemperature_1 = usMiddleIndex;
                ptLinearLinearOut->lTemperature_2 = usMiddleIndex+1;
                ptLinearLinearOut->usaLinearValue1 = usCalcTemp1;
                ptLinearLinearOut->usaLinearValue2 = usCalcTemp2;
                /*对温度进行偏移*/
                TemperatureOffsetCalc(ucTypeIn,ptLinearLinearOut);
                return E_IO_ERROR_NONE;
            }
            else
            {
                usLowIndex = usMiddleIndex + 1;
            }
        }
        else if(usCompareValue < usCalcTemp1)
        {
            usHighIndex = usMiddleIndex - 1;
        }
    }
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:void TableLinearFittingReverse(TableLinear_t* ptLinearLinearIn, uint64_t ulLinearValueIn, uint32_t* puiTableValueOut)
*功能描述: 线性拟合计算 
*输入参数:TableLinear_t 线性拟合区间 ulLinearValueIn 拟合的值 puiTableValueOut 拟合结果值地址
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/13                              
******************************************************************/
void TableLinearFittingReverse(TableLinear_t* ptLinearLinearIn, uint64_t ulLinearValueIn, int32_t* piTableValueOut)
{
    /*温度*/
    int64_t iTemperatureTemp = ulLinearValueIn;
    int64_t sTemp = 0;
    /*计算出拟合比例的值 被放大100000倍*/
    sTemp = (iTemperatureTemp - ptLinearLinearIn->lTemperature_1*PRECISION_MAGNIFICATION)/(ptLinearLinearIn->lTemperature_2 - ptLinearLinearIn->lTemperature_1);
    /*算出  放大100000倍*/
    *piTableValueOut = (ptLinearLinearIn->usaLinearValue1)*PRECISION_MAGNIFICATION + sTemp* (ptLinearLinearIn->usaLinearValue2 - ptLinearLinearIn->usaLinearValue1);

    return;
}

/******************************************************************
*函数名称:IO_InterfaceResult_e ReadTableAddress(TableType_e eSnesorTypeIn)
*功能描述: 读取固化在FLASH中的地址及表格长度
*输入参数:eSnesorTypeIn  表格类型
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/17                              
******************************************************************/
IO_InterfaceResult_e ReadTableAddress(TableType_e eSnesorTypeIn)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NONE;
    /*读取热敏电阻起始地址*/    
    memcpy(&guiThermistorTableAddress,(void*)TABLEHEADADDRESS,FLASHADDRESSLEN);
    /*读取热敏电阻长度*/    
    memcpy(&guiThermistorTablelength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN),FLASHADDRESSLEN);
    switch(eSnesorTypeIn)
    {
    case E_THERMOCOUPLE_TYPE_K :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*2),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*3),FLASHADDRESSLEN);
        break;
    }
    case E_THERMOCOUPLE_TYPE_J :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*4),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*5),FLASHADDRESSLEN);
        break;
    }
    case E_THERMOCOUPLE_TYPE_T :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*6),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*7),FLASHADDRESSLEN);
        break;
    }
    case E_THERMOCOUPLE_TYPE_E :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*8),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*9),FLASHADDRESSLEN);
        break;
    }
    case E_THERMOCOUPLE_TYPE_N :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*10),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*11),FLASHADDRESSLEN);
        break;
    }
    case E_THERMOCOUPLE_TYPE_S :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*12),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*13),FLASHADDRESSLEN);
        break;
    }
    case E_THERMOCOUPLE_TYPE_R :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*14),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*15),FLASHADDRESSLEN);
        break;
    }
    case E_THERMOCOUPLE_TYPE_B :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*16),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*17),FLASHADDRESSLEN);
        break;
    }
    case E_THERMOCOUPLE_TYPE_C :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*18),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*19),FLASHADDRESSLEN);
        break;
    }    

    default :
    {
        eResult = E_IO_ERROR_TABLE_TYPE;
        break;
    }
    }

    /*读取标定的温度值*/

    memcpy(&gtaAISampleInfo[COMPENSATION_CHANNEL_0].lCalibrationValue,(void*)CALIBRATIONADADDRESS, FLASHADDRESSLEN );    
    memcpy(&gtaAISampleInfo[COMPENSATION_CHANNEL_1].lCalibrationValue,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN), FLASHADDRESSLEN );    
    memcpy(&gtaAISampleInfo[COMPENSATION_CHANNEL_2].lCalibrationValue,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*2), FLASHADDRESSLEN );    
    memcpy(&gtaAISampleInfo[COMPENSATION_CHANNEL_3].lCalibrationValue,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*3), FLASHADDRESSLEN );    

    /*读取标定值，上面的标定是针对温度，此处标定针对通道采样码值*/
    memcpy(&gtaAISampleInfo[TMEPERATURE_CHANNEL_0].lCalibrationChannelValue_K,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*4), FLASHADDRESSLEN ); 
    memcpy(&gtaAISampleInfo[TMEPERATURE_CHANNEL_0].lCalibrationChannelValue_B,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*5), FLASHADDRESSLEN );  
    memcpy(&gtaAISampleInfo[TMEPERATURE_CHANNEL_1].lCalibrationChannelValue_K,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*6), FLASHADDRESSLEN ); 
    memcpy(&gtaAISampleInfo[TMEPERATURE_CHANNEL_1].lCalibrationChannelValue_B,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*7), FLASHADDRESSLEN ); 
    memcpy(&gtaAISampleInfo[TMEPERATURE_CHANNEL_2].lCalibrationChannelValue_K,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*8), FLASHADDRESSLEN ); 
    memcpy(&gtaAISampleInfo[TMEPERATURE_CHANNEL_2].lCalibrationChannelValue_B,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*9), FLASHADDRESSLEN );  
    memcpy(&gtaAISampleInfo[TMEPERATURE_CHANNEL_3].lCalibrationChannelValue_K,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*10), FLASHADDRESSLEN ); 
    memcpy(&gtaAISampleInfo[TMEPERATURE_CHANNEL_3].lCalibrationChannelValue_B,(void*)(CALIBRATIONADADDRESS+FLASHADDRESSLEN*11), FLASHADDRESSLEN ); 

    /*通道赋初值*/
    for(uint8_t i = 0; i < gucChannelCount; i++)
    {
        gusaAnalogData[i] = gtaTableInfo[eSnesorTypeIn].sEndTemperature*10;
    }
    return eResult;
}

/******************************************************************
*函数名称:void ConfigureDataToADS(uint8_t ucChannelIn)
*功能描述:利用DMA的外设将配置信息发送出去
*输入参数:当前通道(差分用户通道)
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/27                              
******************************************************************/
void ConfigureDataToADS(uint8_t ucChannelIn,TableType_e ucTypeIn)
{
    /*禁能DMA*/    
    DMA_Cmd(SPI_AI_TX_DMA_CHANNEL,DISABLE);    
    DMA_Cmd(SPI_AI_RX_DMA_CHANNEL,DISABLE);

    DMA_ClearFlag(DMA1_FLAG_GL2);    
    DMA_ClearFlag(DMA1_FLAG_GL3);

    /*根据通道组装TX BUFFER*/
    switch(ucChannelIn)
    {
    /*用户1通道*/
    case TMEPERATURE_CHANNEL_0 :
    { 
         /*配置GPIO开关*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_GPIODAT;
        SPI_AI_TxBuffer[1] = WRITEBYTENUM;
        SPI_AI_TxBuffer[2] = RTU_USER_GPIO_SWITCH_1;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[4] =WRITEBYTENUM;
        SPI_AI_TxBuffer[5] =gtaTableInfo[ucTypeIn].uRegWriteValue;
        /*差分通道-sample*/
        SPI_AI_TxBuffer[6] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[7] = WRITEBYTENUM;
        SPI_AI_TxBuffer[8] = RTU_USER_CHANNEL_1; 

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_CHANNEL_CONFIG_LEN);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_CHANNEL_CONFIG_LEN);  

        /*使能DMA通道*/
        SendDMACmd();
        
        break;
    }
    /*补偿*/
    case COMPENSATION_CHANNEL_0 :
    {

        /*配置PGA1 RATE:160sps*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_SYS0;;
        SPI_AI_TxBuffer[1] =WRITEBYTENUM;
        SPI_AI_TxBuffer[2] =RTU_PGA_1_RATE_160; 
        /*差分通道-compensation */
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTU_COMPENSATION_CHANNEL_1; 

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_COMPENSATION_CONFIG_LEN);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_COMPENSATION_CONFIG_LEN);  

        /*使能DMA通道*/
        SendDMACmd();
        
        break;
    }
    /*采样*/
    case TMEPERATURE_CHANNEL_1 :
    {
        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[1] =WRITEBYTENUM;
        SPI_AI_TxBuffer[2] =gtaTableInfo[ucTypeIn].uRegWriteValue; 
         /*差分通道-sample*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTU_USER_CHANNEL_2; 

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_CHANNEL_CONFIG_NO_GPIO_LEN);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_CHANNEL_CONFIG_NO_GPIO_LEN);  

        /*使能DMA通道*/
        SendDMACmd();
        
        break;
    }
    /*补偿2*/
    case COMPENSATION_CHANNEL_1 :
    {  
        /*配置PGA1 RATE:160sps*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[1] =WRITEBYTENUM;
        SPI_AI_TxBuffer[2] =RTU_PGA_1_RATE_160; 
        /*差分通道-compensation */
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTU_COMPENSATION_CHANNEL_2; 

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_COMPENSATION_CONFIG_LEN);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_COMPENSATION_CONFIG_LEN);  

        /*使能DMA通道*/
        SendDMACmd();
        
        break;
    }
    /*用户3通道*/
    case TMEPERATURE_CHANNEL_2 :
    {
         /*配置GPIO开关*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_GPIODAT;
        SPI_AI_TxBuffer[1] = WRITEBYTENUM;
        SPI_AI_TxBuffer[2] = RTU_USER_GPIO_SWITCH_2;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_SYS0;;
        SPI_AI_TxBuffer[4] =WRITEBYTENUM;
        SPI_AI_TxBuffer[5] =gtaTableInfo[ucTypeIn].uRegWriteValue; 
         /*差分通道*/
        SPI_AI_TxBuffer[6] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[7] = WRITEBYTENUM;
        SPI_AI_TxBuffer[8] = RTU_USER_CHANNEL_3; 

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_CHANNEL_CONFIG_LEN);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_CHANNEL_CONFIG_LEN);  

        /*使能DMA通道*/
        SendDMACmd();

        break; 
    }
    /*补偿3*/
    case COMPENSATION_CHANNEL_2 :
    {
        /*配置PGA1 RATE:160sps*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_SYS0;;
        SPI_AI_TxBuffer[1] =WRITEBYTENUM;
        SPI_AI_TxBuffer[2] =RTU_PGA_1_RATE_160; 
        /*差分通道-compensation */
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTU_COMPENSATION_CHANNEL_3; 

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_COMPENSATION_CONFIG_LEN);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_COMPENSATION_CONFIG_LEN);  

        /*使能DMA通道*/
        SendDMACmd();
        
        break;
    }
    /*采样*/
    case TMEPERATURE_CHANNEL_3 :
    {
        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_SYS0;;
        SPI_AI_TxBuffer[1] =WRITEBYTENUM;
        SPI_AI_TxBuffer[2] =gtaTableInfo[ucTypeIn].uRegWriteValue; 
        /*差分通道-sample*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTU_USER_CHANNEL_4; 

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_CHANNEL_CONFIG_NO_GPIO_LEN);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_CHANNEL_CONFIG_NO_GPIO_LEN);  

        /*使能DMA通道*/
        SendDMACmd();
        
        break;
        
    }
    
    /*补偿4*/
    case COMPENSATION_CHANNEL_3 :
    {  
        /*配置PGA1 RATE:160sps*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_SYS0;;
        SPI_AI_TxBuffer[1] =WRITEBYTENUM;
        SPI_AI_TxBuffer[2] =RTU_PGA_1_RATE_160; 
        /*差分通道-compensation */
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTU_COMPENSATION_CHANNEL_4; 

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_COMPENSATION_CONFIG_LEN);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_COMPENSATION_CONFIG_LEN);  

        /*使能DMA通道*/
        SendDMACmd();
        
        break;
    }
    
    default :
    {
        break;
    }    
    }

    return;
}

/******************************************************************
*函数名称:void ConfigureDataByThermocoupleType(TableType_e tTypeIn)
*功能描述:根据热电偶类型组装全局DMA_TXBUFFER
*输入参数:tTypeIn  传感器类型
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/27                              
******************************************************************/
void ConfigureDataByThermocoupleType(TableType_e tTypeIn)
{
    /*禁能DMA*/    
    DMA_Cmd(SPI_AI_TX_DMA_CHANNEL,DISABLE);    
    DMA_Cmd(SPI_AI_RX_DMA_CHANNEL,DISABLE);
    switch(tTypeIn)
    {
    case E_THERMOCOUPLE_TYPE_T :
    {
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;
        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        
        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;    

         /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_128_RATE_160; 
        break;
    }
    case E_THERMOCOUPLE_TYPE_K :
    {
        /*FSC 0x400000h*/
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;
        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        
        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;

         /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_32_RATE_160; 
        break;
    }
    case E_THERMOCOUPLE_TYPE_S :
    {   
        /*FSC 0x400000h*/
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;
        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        
        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_64_RATE_160; 

        break;
    }
    case E_THERMOCOUPLE_TYPE_E :
    {        
        /*FSC 0x400000h*/
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;
        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        
        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_16_RATE_160; 

        break;
    }
    case E_THERMOCOUPLE_TYPE_J :
    { 
        /*FSC 0x400000h*/
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;
        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        
        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_16_RATE_160; 

        break;
    }
    case E_THERMOCOUPLE_TYPE_N :
    { 
        /*FSC 0x400000h*/
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;
        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        
        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_32_RATE_160; 

        break;
    }
    case E_THERMOCOUPLE_TYPE_R :
    { 
        /*FSC 0x400000h*/
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;
        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        
        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_64_RATE_160; 

        break;
    }
    case E_THERMOCOUPLE_TYPE_B :
    { 
        /*FSC 0x400000h*/
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;
        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        
        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_128_RATE_160; 

        break;
    }
    case E_THERMOCOUPLE_TYPE_C :
    {  
        /*FSC 0x400000h*/
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;

        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        

        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;

        /*配置PGA32 RATE:160sps*/
        SPI_AI_TxBuffer[9] = ADS_WREG | ADS_SYS0;
        SPI_AI_TxBuffer[10] =WRITEBYTENUM;
        SPI_AI_TxBuffer[11] =RTU_PGA_32_RATE_160; 

        break;
    }
    default :
    {
        break;
    }
    }


    /*N极偏置*/
    SPI_AI_TxBuffer[12] = ADS_WREG | ADS_VBIAS;
    SPI_AI_TxBuffer[13] = WRITEBYTENUM;
    SPI_AI_TxBuffer[14] = RTU_USER_VBIAS_VALUE;

    /*配置GPIO开关*/
    SPI_AI_TxBuffer[15] = ADS_WREG | ADS_GPIOCFG;
    SPI_AI_TxBuffer[16] = WRITEBYTENUM;
    SPI_AI_TxBuffer[17] = GPIO_MODE;

    /*GPIO  out mode*/
    SPI_AI_TxBuffer[18] = ADS_WREG | ADS_GPIODIR;
    SPI_AI_TxBuffer[19] = WRITEBYTENUM;
    SPI_AI_TxBuffer[20] = GPIO_OUT;

    SPI_AI_TxBuffer[21] = ADS_WREG | ADS_GPIODAT;
    SPI_AI_TxBuffer[22] = WRITEBYTENUM;
    SPI_AI_TxBuffer[23] = RTU_USER_GPIO_SWITCH_1;

    /*内置基准*/
    SPI_AI_TxBuffer[24] = ADS_WREG | ADS_MUX1;
    SPI_AI_TxBuffer[25] = WRITEBYTENUM;
    SPI_AI_TxBuffer[26] = REFSELT_VALUE;
    
    /*差分通道*/
    SPI_AI_TxBuffer[27] = ADS_WREG | ADS_MUX0;
    SPI_AI_TxBuffer[28] = WRITEBYTENUM;
    SPI_AI_TxBuffer[29] = RTU_USER_CHANNEL_1;
    /*设置DMA长度  配置信息长度30*/
    DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,RTU_USER_INIT_PARA_LEN);    
    DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,RTU_USER_INIT_PARA_LEN);

    /*使能DMA通道*/
    SendDMACmd();
    
    return;
}

/******************************************************************
*函数名称:IO_InterfaceResult_e TempConversion(TableType_e ucTypeIn,int32_t *piFilterDataIn)
*功能描述:ADS采样温度计算
*输入参数:ucTypeIn 热电偶类型  piFilterDataIn滤波码值的地址
*输出参数:IO_InterfaceResult_e  函数执行结果
*返回值:IO_InterfaceResult_e
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/11                              
******************************************************************/
IO_InterfaceResult_e TempConversion(TableType_e ucTypeIn,int32_t *piFilterDataIn)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;
    int64_t iVoltageValue = 0;
    int64_t iTemperature = 0;
    int64_t iFilterData = *piFilterDataIn;
    TableLinear_t tTableLinear;        

    /*对通道采样值进行如下计算，补偿则不能以下面方式计算*/
    if ( TMEPERATURE_CHANNEL_0 ==gucCurrentIndex || TMEPERATURE_CHANNEL_1 ==gucCurrentIndex
         || TMEPERATURE_CHANNEL_2 ==gucCurrentIndex || TMEPERATURE_CHANNEL_3 ==gucCurrentIndex )
    {
        /*对采样码值判断*/
        if((ADSDISCONNECTIONVALUE_tvs == (int32_t)iFilterData )||((ADSDISCONNECTIONVALUE == (int32_t)iFilterData )))    
        {
            /*断连  置通道状态为断连状态*/
            gtaAISampleInfo[gucCurrentIndex].ucConnectionFlag = ADS_DISCONNECTION;
            /*更新数据区返回*/
            gusaAnalogData[gucCurrentIndex/2] = gtaTableInfo[ucTypeIn].sEndTemperature*10;
            eResult = E_IO_ERROR_ADS_DATA_DISCONNECTION;
            return eResult;
        }
        /*通道状态置为连接状态*/
        gtaAISampleInfo[gucCurrentIndex].ucConnectionFlag = ADS_CONNECTION;
        memset(&tTableLinear,0,sizeof(TableLinear_t));

        /*此处对采样码值进行标定*/
        iFilterData = iFilterData*gtaAISampleInfo[gucCurrentIndex].lCalibrationChannelValue_K + gtaAISampleInfo[gucCurrentIndex].lCalibrationChannelValue_B;
        iFilterData = iFilterData/100000;

        /*码值转换成电压*/    
        iVoltageValue = ((iFilterData*RTU_SAMPLE_REFVOLTAGE)*PRECISION_MAGNIFICATION/gtaTableInfo[ucTypeIn].lPGA) >>RESOLUTIONBITLENGTH_23;
        
        /*根据热电偶类型对电压数据进行还原并放大100000倍*/
        ChangeVoltageRules(ucTypeIn,&iVoltageValue);
        /*补偿后的电压*/       
        iVoltageValue = iVoltageValue + gtaAISampleInfo[gucCurrentIndex+1].lCompensationValue;
#if 0

        /*表中数据*1000+10000的规则，但是对于 E型和J型是在原来的基础上再除以2*/
        /*Y1 = KX1+B  和Y2 = KX2+B同时相加，实际上多加了一个B，因此这里减去B值
            加入标定后，前面的计算已经将B值减去，此时不需要添加*/
        if ( E_THERMOCOUPLE_TYPE_E == ucTypeIn || E_THERMOCOUPLE_TYPE_J == ucTypeIn )
        {
            iVoltageValue = iVoltageValue + TABLE_GAIN_VALUE_5000*PRECISION_MAGNIFICATION;
        }
#endif    
        /*获取分度表   结构体命名*/
        eResult = GetValueFromTable(ucTypeIn,iVoltageValue,&tTableLinear);
        /*判断数据是否超限*/
        if(E_IO_ERROR_TABLE_DATA == eResult)
        {
            /*更新数据区返回*/
            gusaAnalogData[gucCurrentIndex/2] = gtaTableInfo[ucTypeIn].sEndTemperature*10;
            /*通道状态超出量程*/
            gtaAISampleInfo[gucCurrentIndex].ucConnectionFlag = ADS_OVERFLOW;

            return eResult;
        }
        /*线性拟合*/
        iVoltageValue = iVoltageValue/PRECISION_MAGNIFICATION;
        TableLinearFitting(&tTableLinear,iVoltageValue,&iTemperature);
        /*更新数据区  查询出来的温度直接保留一位小数显示出来*/
        UpdateDataArea(&iTemperature); 
        
    }    
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:IO_InterfaceResult_e MCU_TEM_Compensation(int32_t iVoltageIn, int64_t * piTemperatureOut)
*功能描述:冷端温度补偿
*输入参数:iVoltageIn 补偿电压   
*输出参数:piTemperatureOut 温度地址
*返回值:IO_InterfaceResult_e
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/12                              
******************************************************************/
IO_InterfaceResult_e MCU_TEM_Compensation(int32_t iVoltageIn, int64_t * piTemperatureOut)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;
    int64_t iThermistorValue = 0;
    TableLinear_t tTableLinear;
    uint64_t ullVoltageTemp = 0;

    /*清0*/
    memset(&tTableLinear,0,sizeof(TableLinear_t));

    ullVoltageTemp = (uint64_t)iVoltageIn;
    
    /*单位转换成mV  阻值是 千欧 并放大 100000倍*/
    iThermistorValue = (VOLTAGE*(MCUREFERENCEVOLTAGE-ullVoltageTemp)/ullVoltageTemp);
    /*通过热敏电阻值 求出温度  */
    eResult = GetValueFromThermistorTable(E_THERMISTOR_TYPE,iThermistorValue,&tTableLinear);
    if(E_IO_ERROR_NONE != eResult)
    {
        return eResult;
    }
    /*已经取得温度数据 并且温度被放大100000倍*/
    TableLinearFitting(&tTableLinear,iThermistorValue,piTemperatureOut);

    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:IO_InterfaceResult_e GetValueFromThermistorTable(TableType_e ucTypeIn,int64_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
*功能描述:查热敏电阻表
*输入参数:ucTypeIn 表格类型 iCheckValueIn  查表值 ptLinearLinearOut 线性拟合区间地址
*输出参数:IO_InterfaceResult_e
*返回值:      E_IO_ERROR_NONE     无错误 
                         E_IO_ERROR_TABLE_DATA  数据溢出    
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/19                              
******************************************************************/
IO_InterfaceResult_e GetValueFromThermistorTable(TableType_e ucTypeIn,int64_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NONE;
    uint16_t usLowIndex = 0;
    /*保存二分法循环变量*/
    uint16_t  usMiddleIndex = 0;
    uint16_t usHighIndex = 0;
    /*定义变量保存从FLASH中读取的数据*/
    uint32_t ulMaxValue = 0;
    uint32_t ulMinVALue = 0;
    uint32_t usCalcTemp1 = 0;
    uint32_t usCalcTemp2 = 0;
    /*定义变量保存地址*/
    uint32_t uiAddressTemp = 0;
    /*比较变量*/
    uint32_t usCompareValue = (uint32_t)(iCheckValueIn);
    /*转换成数据长度*/
    usHighIndex = guiThermistorTablelength/4 -1;
    /*热敏电阻的表是单调递减*/
    memcpy(&ulMaxValue,(void *)(guiThermistorTableAddress),4);
    memcpy(&ulMinVALue,(void *)(guiThermistorTableAddress + (usHighIndex)*4),4);

    if((usCompareValue < ulMinVALue) || (usCompareValue > ulMaxValue))
    {
        eResult = E_IO_ERROR_TABLE_DATA;
        return eResult; // 超限，不做特殊处理
    }
    uiAddressTemp = guiThermistorTableAddress;
    while(usHighIndex >= usLowIndex)
    {
        usMiddleIndex = (usHighIndex + usLowIndex) / 2;
        /*mid的值有变化，重新读取对应FLASH中的地址*/
        memcpy(&usCalcTemp1,(void *)(uiAddressTemp + (usMiddleIndex*4)),4);
        
        if((usCompareValue <= usCalcTemp1) || (0 == usMiddleIndex))
        {
            /*计算Middle+1的值*/            
            memcpy(&usCalcTemp2,(void *)(uiAddressTemp + (usMiddleIndex+1)*4 ),4);
            if(usCompareValue >= usCalcTemp2)
            {
                /*温度与实际索引相差1*/
                ptLinearLinearOut->lTemperature_1 = usMiddleIndex;
                ptLinearLinearOut->lTemperature_2 = usMiddleIndex+1;
                ptLinearLinearOut->usaLinearValue1 = usCalcTemp1;
                ptLinearLinearOut->usaLinearValue2 = usCalcTemp2;
                /*对温度进行偏移*/
                TemperatureOffsetCalcForThermistor(ucTypeIn,ptLinearLinearOut);
                return E_IO_ERROR_NONE;
            }
            else
            {
                usLowIndex = usMiddleIndex + 1;
            }
        }
        else if(usCompareValue > usCalcTemp1)
        {
            usHighIndex = usMiddleIndex - 1;
        }
    }    
    return E_IO_ERROR_NONE;

}

/******************************************************************
*函数名称:void ChangeVoltageRules(TableType_e ucTypeIn,int64_t* puiVoltageInOut)
*功能描述:表格类型(热电偶类型)      
*输入参数:puiVoltageInOut  转换值地址 
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/17                              
******************************************************************/
void ChangeVoltageRules(TableType_e ucTypeIn,int64_t* plVoltageInOut)
{
    /*定义变量保存入参的值*/
    int64_t lVoltageTemp = 0;
    
    lVoltageTemp = *plVoltageInOut;
    switch(ucTypeIn)
    {
    case E_THERMOCOUPLE_TYPE_T :
    {
        *plVoltageInOut = lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR;
        break;
    }
    case E_THERMOCOUPLE_TYPE_K :
    {
        *plVoltageInOut = lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR;
        break;
    }
    case E_THERMOCOUPLE_TYPE_S :
    {
        *plVoltageInOut = (lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR);
        break;
    }
    case E_THERMOCOUPLE_TYPE_E :
    {
        *plVoltageInOut = (lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR)>>1;
        break;
    }
    case E_THERMOCOUPLE_TYPE_J :
    {
        *plVoltageInOut = (lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR)>>1;
        break;
    }
    case E_THERMOCOUPLE_TYPE_N :
    {
        *plVoltageInOut = lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR;
        break;
    }
    case E_THERMOCOUPLE_TYPE_R :
    {
        *plVoltageInOut = lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR;
        break;
    }
    case E_THERMOCOUPLE_TYPE_B :
    {
        *plVoltageInOut = lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR;
        break;
    }
    case E_THERMOCOUPLE_TYPE_C :
    {
        *plVoltageInOut = lVoltageTemp*INDEXINGTABLEMULTIFACTOR+INDEXINGTABLEADDENDFACTOR;
        break;
    }    
    default :
    {
        break;
    }
    }
}

/******************************************************************
*函数名称:IO_InterfaceResult_e GetTemperatureFromTable(TableType_e ucTypeIn,int32_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
*功能描述:查分度表根据温度得到电压
*输入参数:ucTypeIn 表格类型  iCheckValueIn 查表值 
*输出参数:ptLinearLinearOut 线性拟合的区间 
*返回值:E_IO_ERROR_TABLE_OVERFLOW  数据溢出 
                   E_IO_ERROR_NONE         无错误 
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/16                              
******************************************************************/
IO_InterfaceResult_e GetTemperatureFromTable(TableType_e ucTypeIn,int32_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;
    int32_t iTemperatureTemp = 0;
    /*定义变量保存从FLASH中读取的数据*/
    uint16_t usMaxValue = 0;
    uint16_t usMinVALue = 0;
    uint16_t usIndexTemp1 = 0;
    uint16_t usIndexTemp2 = 0;
    /*温度值被放大了100000倍 这里缩小100000倍*/
    iTemperatureTemp = iCheckValueIn/PRECISION_MAGNIFICATION;

    if(iTemperatureTemp < gtaTableInfo[ucTypeIn].sStartTemperature || iTemperatureTemp >  gtaTableInfo[ucTypeIn].sEndTemperature )
    {
        eResult = E_IO_ERROR_TABLE_OVERFLOW;
        return eResult;
    }
    
    /*恢复成索引*/  
    /*FR3924的数据是以2摄氏度间隔存放，因此这里需要转换成INDEX,除以2*/
    usIndexTemp1 = (iTemperatureTemp -gtaTableInfo[ucTypeIn].sStartTemperature)/TEMPERATURE_INTERVAL_2;
    /*温度对应的上区间*/
    usIndexTemp2 = usIndexTemp1 + 1;
    memcpy(&usMinVALue,(void *)(guiTableAddress + (usIndexTemp1)*2),2);
    memcpy(&usMaxValue,(void *)(guiTableAddress + (usIndexTemp2)*2),2);
    /*保存温度*/
    ptLinearLinearOut->lTemperature_1 = (iTemperatureTemp/TEMPERATURE_INTERVAL_2)*TEMPERATURE_INTERVAL_2;
    /*下一个温度区间偏移2*/
    ptLinearLinearOut->lTemperature_2 = iTemperatureTemp+2;
    ptLinearLinearOut->usaLinearValue1 = usMinVALue;
    ptLinearLinearOut->usaLinearValue2 = usMaxValue;

    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:IO_InterfaceResult_e GetTemperatureFromTable_Calibration(TableType_e ucTypeIn,int32_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
*功能描述:查分度表根据温度得到电压
*输入参数:ucTypeIn 表格类型  iCheckValueIn 查表值 
*输出参数:ptLinearLinearOut 线性拟合的区间 
*返回值:E_IO_ERROR_TABLE_OVERFLOW  数据溢出 
                   E_IO_ERROR_NONE         无错误 
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/16                              
******************************************************************/
IO_InterfaceResult_e GetTemperatureFromTable_Calibration(TableType_e ucTypeIn,int32_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;
    int32_t iTemperatureTemp = 0;
    /*定义变量保存从FLASH中读取的数据*/
    uint16_t usMaxValue = 0;
    uint16_t usMinVALue = 0;
    uint16_t usIndexTemp1 = 0;
    uint16_t usIndexTemp2 = 0;
    
    /*保存T型表地址
        为提高精度，这里对所有表进行标定 这里以T型表做数据进行标定*/    
    uint32_t uiTableAddress = 0x080074a4;
    /*温度值被放大了100000倍 这里缩小100000倍*/
    iTemperatureTemp = iCheckValueIn/PRECISION_MAGNIFICATION;

    if(iTemperatureTemp < gtaTableInfo[ucTypeIn].sStartTemperature || iTemperatureTemp >  gtaTableInfo[ucTypeIn].sEndTemperature )
    {
        eResult = E_IO_ERROR_TABLE_OVERFLOW;
        return eResult;
    }
    
    /*恢复成索引*/  
    /*FR3924的数据是以2摄氏度间隔存放，因此这里需要转换成INDEX,除以2*/
    usIndexTemp1 = (iTemperatureTemp -gtaTableInfo[ucTypeIn].sStartTemperature)/TEMPERATURE_INTERVAL_2;
    /*温度对应的上区间*/
    usIndexTemp2 = usIndexTemp1 + 1;
    memcpy(&usMinVALue,(void *)(uiTableAddress + (usIndexTemp1)*2),2);
    memcpy(&usMaxValue,(void *)(uiTableAddress + (usIndexTemp2)*2),2);
    /*保存温度*/
    ptLinearLinearOut->lTemperature_1 = (iTemperatureTemp/TEMPERATURE_INTERVAL_2)*TEMPERATURE_INTERVAL_2;
    /*下一个温度区间偏移2*/
    ptLinearLinearOut->lTemperature_2 = iTemperatureTemp+2;
    ptLinearLinearOut->usaLinearValue1 = usMinVALue;
    ptLinearLinearOut->usaLinearValue2 = usMaxValue;

    return E_IO_ERROR_NONE;
}



/******************************************************************
*函数名称:void SetCalibrationValueByThermocoupleType
*功能描述:设置每个通道的校准值
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/9/1                              
******************************************************************/
IO_InterfaceResult_e SetCalibrationValueByThermocoupleType(TableType_e tTypeIn)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;
    int64_t iTemperature = 0;
    TableLinear_t tTableLinear;

    memset(&tTableLinear,0,sizeof(tTableLinear));
    for (uint8_t i = 1; i < 8; i +=2 )
    {
        /*开始标定进来的数据为温度值放大100000值*/
        iTemperature = (int64_t)gtaAISampleInfo[i].lCalibrationValue; 

        /*通过温度查分度表 得到对应的电压*/
        eResult = GetTemperatureFromTable_Calibration(E_THERMOCOUPLE_TYPE_T,iTemperature,&tTableLinear);
        if(E_IO_ERROR_NONE != eResult)
        {
            /*数据超限  不需要进行线性拟合*/
            return eResult;
        }
        /*线性拟合  查表反向  知道温度，拟合出分度表对应的电压*/
        TableLinearFittingReverse(&tTableLinear,iTemperature,&gtaAISampleInfo[i].lCalibrationValue); 
        /*判断表的存储格式是否一致*/
        if ( tTypeIn == E_THERMOCOUPLE_TYPE_E || tTypeIn == E_THERMOCOUPLE_TYPE_J)
        {
            gtaAISampleInfo[i].lCalibrationValue = gtaAISampleInfo[i].lCalibrationValue/2;
        }
    }

    return E_IO_ERROR_NONE;
}


#endif

#if (IO_CODE_TYPE == FR3822_CODE)

/******************************************************************
*函数名称:void TemperatureOffsetCalc(TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut)
*功能描述:通过表的类型 对温度进行偏移计算
*输入参数:ucTypeIn  表格类型   ptLinearLinearInOut线性区间的地址 
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/17                              
******************************************************************/
/*改变入参  只传入INDEX  */
void TemperatureOffsetCalc(TableType_e ucTypeIn,TableLinear_t * ptLinearLinearInOut)
{
    /*对温度进行偏移*/
    ptLinearLinearInOut->lTemperature_1 = ptLinearLinearInOut->lTemperature_1 +gtaTableInfo[ucTypeIn].sStartTemperature;
    ptLinearLinearInOut->lTemperature_2 = ptLinearLinearInOut->lTemperature_2 +gtaTableInfo[ucTypeIn].sStartTemperature;
    return;
}

/******************************************************************
*函数名称:IO_InterfaceResult_e ReadTableAddress(TableType_e eSnesorTypeIn)
*功能描述: 读取固化在FLASH中的地址及表格长度
*输入参数:eSnesorTypeIn  表格类型
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/17                              
******************************************************************/
IO_InterfaceResult_e ReadTableAddress(TableType_e eSnesorTypeIn)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NONE;
    switch(eSnesorTypeIn)
    {
    case E_THERMAL_RESISTANCE_TYPE_PT1000 :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*2),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*3),FLASHADDRESSLEN);
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_PT200 :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*4),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*5),FLASHADDRESSLEN);
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_PT500 :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*6),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*7),FLASHADDRESSLEN);
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_PT100 :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*8),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*9),FLASHADDRESSLEN);
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_NI120 :
    {
        /*读取Index table 的起始地址*/
        memcpy(&guiTableAddress,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*10),FLASHADDRESSLEN);
        /*读取Index table 的表格长度*/
        memcpy(&guiTableLength,(void*)(TABLEHEADADDRESS+FLASHADDRESSLEN*11),FLASHADDRESSLEN);
        break;
    }
    default :
    {
        eResult = E_IO_ERROR_TABLE_TYPE;
        break;
    }
    }

    /*通道赋初值*/
    for(uint8_t i = 0; i < gucChannelCount; i++)
    {
        gusaAnalogData[i] = gtaTableInfo[eSnesorTypeIn].sEndTemperature*10;
    }
    return eResult;
}

/******************************************************************
*函数名称:void ConfigureDataByThermalResistanceType(TableType_e tTypeIn)
*功能描述:根据热电阻类型组装全局DMA_TXBUFFER
*输入参数:tTypeIn  传感器类型
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/27                              
******************************************************************/
void ConfigureDataByThermalResistanceType(TableType_e tTypeIn)
{
    /*禁能DMA*/    
    DMA_Cmd(SPI_AI_TX_DMA_CHANNEL,DISABLE);    
    DMA_Cmd(SPI_AI_RX_DMA_CHANNEL,DISABLE);
    switch(tTypeIn)
    {
    case E_THERMAL_RESISTANCE_TYPE_PT1000 :
    {
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;

        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        

        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;
        /*增益*/        
        SPI_AI_TxBuffer[9] = FSCANDPGAVALUE_9;
        SPI_AI_TxBuffer[10] =FSCANDPGAVALUE_10;
        SPI_AI_TxBuffer[11] =PGA_1_DATABAUD_160;  

        /*N极偏置*/
        SPI_AI_TxBuffer[12] = VBIASCMD;
        SPI_AI_TxBuffer[13] = WRITEBYTENUM;
        SPI_AI_TxBuffer[14] = RTD_VBIAS_VALUE_NO;

        /*配置激励源大小*/
        SPI_AI_TxBuffer[15] = ADS_WREG | ADS_IDAC0;
        SPI_AI_TxBuffer[16] = WRITEBYTENUM;
        SPI_AI_TxBuffer[17] = RTD_DRIVING_SOURCE_250; 
        
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_PT200 :
    {             
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;

        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        

        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;
        /*增益*/        
        SPI_AI_TxBuffer[9] = FSCANDPGAVALUE_9;
        SPI_AI_TxBuffer[10] =FSCANDPGAVALUE_10;
        SPI_AI_TxBuffer[11] =PGA_4_DATABAUD_160;

         /*N极偏置*/
        SPI_AI_TxBuffer[12] = VBIASCMD;
        SPI_AI_TxBuffer[13] = WRITEBYTENUM;
        SPI_AI_TxBuffer[14] = RTD_VBIAS_VALUE_NO;

        /*配置激励源大小*/
        SPI_AI_TxBuffer[15] = ADS_WREG | ADS_IDAC0;
        SPI_AI_TxBuffer[16] = WRITEBYTENUM;
        SPI_AI_TxBuffer[17] = RTD_DRIVING_SOURCE_250; 
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_PT500 :
    {           
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;

        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        

        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;
        /*增益*/        
        SPI_AI_TxBuffer[9] = FSCANDPGAVALUE_9;
        SPI_AI_TxBuffer[10] =FSCANDPGAVALUE_10;
        SPI_AI_TxBuffer[11] =PGA_1_DATABAUD_160; 

         /*N极偏置*/
        SPI_AI_TxBuffer[12] = VBIASCMD;
        SPI_AI_TxBuffer[13] = WRITEBYTENUM;
        SPI_AI_TxBuffer[14] = RTD_VBIAS_VALUE_NO;

        /*配置激励源大小*/
        SPI_AI_TxBuffer[15] = ADS_WREG | ADS_IDAC0;
        SPI_AI_TxBuffer[16] = WRITEBYTENUM;
        SPI_AI_TxBuffer[17] = RTD_DRIVING_SOURCE_250; 
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_PT100 :
    {          
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;

        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        

        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;
        /*增益*/        
        SPI_AI_TxBuffer[9] = FSCANDPGAVALUE_9;
        SPI_AI_TxBuffer[10] =FSCANDPGAVALUE_10;
        SPI_AI_TxBuffer[11] =PGA_4_DATABAUD_160; 

         /*N极偏置*/
        SPI_AI_TxBuffer[12] = VBIASCMD;
        SPI_AI_TxBuffer[13] = WRITEBYTENUM;
        SPI_AI_TxBuffer[14] = 0x00;

        /*配置激励源大小*/
        SPI_AI_TxBuffer[15] = ADS_WREG | ADS_IDAC0;
        SPI_AI_TxBuffer[16] = WRITEBYTENUM;
        SPI_AI_TxBuffer[17] = RTD_DRIVING_SOURCE_500; 
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_NI120 :
    {   
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;

        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        

        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;
        /*增益*/        
        SPI_AI_TxBuffer[9] = FSCANDPGAVALUE_9;
        SPI_AI_TxBuffer[10] =FSCANDPGAVALUE_10;
        SPI_AI_TxBuffer[11] =PGA_4_DATABAUD_160; 

         /*N极偏置*/
        SPI_AI_TxBuffer[12] = VBIASCMD;
        SPI_AI_TxBuffer[13] = WRITEBYTENUM;
        SPI_AI_TxBuffer[14] = RTD_VBIAS_VALUE_NO;

        /*配置激励源大小*/
        SPI_AI_TxBuffer[15] = ADS_WREG | ADS_IDAC0;
        SPI_AI_TxBuffer[16] = WRITEBYTENUM;
        SPI_AI_TxBuffer[17] = RTD_DRIVING_SOURCE_500; 
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_RESISTANCE_MEASUREMENT :
    {         
        SPI_AI_TxBuffer[0] =FSCANDPGAVALUE_0;
        SPI_AI_TxBuffer[1] =FSCANDPGAVALUE_1;        
        SPI_AI_TxBuffer[2] =FSCANDPGAVALUE_2;
        SPI_AI_TxBuffer[3] =FSCANDPGAVALUE_3;

        SPI_AI_TxBuffer[4] =FSCANDPGAVALUE_4;        
        SPI_AI_TxBuffer[5] =FSCANDPGAVALUE_5;
        SPI_AI_TxBuffer[6] =FSCANDPGAVALUE_6;
        SPI_AI_TxBuffer[7] =FSCANDPGAVALUE_7;        

        SPI_AI_TxBuffer[8] =FSCANDPGAVALUE_8;
        /*增益*/        
        SPI_AI_TxBuffer[9] = FSCANDPGAVALUE_9;
        SPI_AI_TxBuffer[10] =FSCANDPGAVALUE_10;
        SPI_AI_TxBuffer[11] =PGA_1_DATABAUD_160; 

         /*N极偏置*/
        SPI_AI_TxBuffer[12] = VBIASCMD;
        SPI_AI_TxBuffer[13] = WRITEBYTENUM;
        SPI_AI_TxBuffer[14] = RTD_VBIAS_VALUE_NO;

        /*配置激励源大小*/
        SPI_AI_TxBuffer[15] = ADS_WREG | ADS_IDAC0;
        SPI_AI_TxBuffer[16] = WRITEBYTENUM;
        SPI_AI_TxBuffer[17] = RTD_DRIVING_SOURCE_250; 
        break;
        break;
    }
    default :
    {
        break;
    }
    }

    /*配置参考REF*/
    SPI_AI_TxBuffer[18] = ADS_WREG | ADS_MUX1;
    SPI_AI_TxBuffer[19] = WRITEBYTENUM;
    SPI_AI_TxBuffer[20] = RTD_USER_CHANNEL_1_REF;

    /*配置第一个通道*/
    SPI_AI_TxBuffer[21] = ADS_WREG | ADS_MUX0;
    SPI_AI_TxBuffer[22] = WRITEBYTENUM;
    SPI_AI_TxBuffer[23] = RTD_USER_CHANNEL_1;
    
    /*设置DMA长度  配置信息长度24*/
    DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,24);   
    DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,24);

    /*使能DMA通道*/
    SendDMACmd();

    return;
}


/******************************************************************
*函数名称:
*功能描述:
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/4/29                              
******************************************************************/
void ConfigureDataToADS(uint8_t ucChannelIn)
{
    /*禁能DMA*/    
    DMA_Cmd(SPI_AI_TX_DMA_CHANNEL,DISABLE);    
    DMA_Cmd(SPI_AI_RX_DMA_CHANNEL,DISABLE);

    DMA_ClearFlag(DMA1_FLAG_GL2);    
    DMA_ClearFlag(DMA1_FLAG_GL3);

    /*根据通道组装TX BUFFER*/
    switch(ucChannelIn)
    {
    case TMEPERATURE_CHANNEL_0 :
    {  
         /*配置参考REF*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_MUX1;
        SPI_AI_TxBuffer[1] = WRITEBYTENUM;
        SPI_AI_TxBuffer[2] = RTD_USER_CHANNEL_1_REF;

        /*配置1 激励源*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_IDAC1;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTD_USER_CHANNEL_1_DRIIVING_SOURCE_1;
        
        /*差分通道*/
        SPI_AI_TxBuffer[6] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[7] = WRITEBYTENUM;
        SPI_AI_TxBuffer[8] = RTD_USER_CHANNEL_1;
        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,9);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,9);       

        /*使能DMA通道*/
        SendDMACmd();
        break;
                    
    }
    case TMEPERATURE_CHANNEL_1 :
    {
        /*配置2 激励源*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_IDAC1;
        SPI_AI_TxBuffer[1] = WRITEBYTENUM;
        SPI_AI_TxBuffer[2] = RTD_USER_CHANNEL_1_DRIIVING_SOURCE_2;

        /*差分通道*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTD_USER_CHANNEL_1;

        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,6);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,6);       

        /*使能DMA通道*/
        SendDMACmd();
        break;
    }
    case TMEPERATURE_CHANNEL_2 :
    {
         /*配置参考REF*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_MUX1;
        SPI_AI_TxBuffer[1] = WRITEBYTENUM;
        SPI_AI_TxBuffer[2] = RTD_USER_CHANNEL_2_REF;

        /*配置1 激励源*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_IDAC1;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTD_USER_CHANNEL_2_DRIIVING_SOURCE_1;
        
        /*差分通道*/
        SPI_AI_TxBuffer[6] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[7] = WRITEBYTENUM;
        SPI_AI_TxBuffer[8] = RTD_USER_CHANNEL_2;
        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,9);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,9);       

        /*使能DMA通道*/
        SendDMACmd();
        break;

    }
    case TMEPERATURE_CHANNEL_3 :
    {  
        /*配置2 激励源*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_IDAC1;
        SPI_AI_TxBuffer[1] = WRITEBYTENUM;
        SPI_AI_TxBuffer[2] = RTD_USER_CHANNEL_2_DRIIVING_SOURCE_2;

         /*差分通道*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTD_USER_CHANNEL_2;
        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,6);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,6); 

        /*使能DMA通道*/
        SendDMACmd();
        break;
    }
    case TMEPERATURE_CHANNEL_4 :
    {  
         /*配置参考REF*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_MUX1;
        SPI_AI_TxBuffer[1] = WRITEBYTENUM;
        SPI_AI_TxBuffer[2] = RTD_USER_CHANNEL_1_REF;

        /*配置1 激励源*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_IDAC1;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTD_USER_CHANNEL_1_DRIIVING_SOURCE_1;
        
        /*差分通道*/
        SPI_AI_TxBuffer[6] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[7] = WRITEBYTENUM;
        SPI_AI_TxBuffer[8] = RTD_USER_CHANNEL_1_BURNOUT;
        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,9);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,9);       

        /*使能DMA通道*/
        SendDMACmd();
        break;
    }
    case TMEPERATURE_CHANNEL_5 :
    {  
         /*配置参考REF*/
        SPI_AI_TxBuffer[0] = ADS_WREG | ADS_MUX1;
        SPI_AI_TxBuffer[1] = WRITEBYTENUM;
        SPI_AI_TxBuffer[2] = RTD_USER_CHANNEL_2_REF;

        /*配置1 激励源*/
        SPI_AI_TxBuffer[3] = ADS_WREG | ADS_IDAC1;
        SPI_AI_TxBuffer[4] = WRITEBYTENUM;
        SPI_AI_TxBuffer[5] = RTD_USER_CHANNEL_2_DRIIVING_SOURCE_1;
        
        /*差分通道*/
        SPI_AI_TxBuffer[6] = ADS_WREG | ADS_MUX0;
        SPI_AI_TxBuffer[7] = WRITEBYTENUM;
        SPI_AI_TxBuffer[8] = RTD_USER_CHANNEL_2_BURNOUT;
        /*设置DMA长度  */
        DMA_SetCurrDataCounter(SPI_AI_TX_DMA_CHANNEL,9);
        DMA_SetCurrDataCounter(SPI_AI_RX_DMA_CHANNEL,9);       

        /*使能DMA通道*/
        SendDMACmd();
        break;

    }
    default :
    {
        break;
    }    
    }
    
    return;
}


/******************************************************************
*函数名称:
*功能描述:
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/4/29                              
******************************************************************/
IO_InterfaceResult_e TempConversion(TableType_e ucTypeIn, DivingSourceInfo_t *ptDivingSourceIn)
{
    /*用于保存计算RTD电阻系数*/
    int64_t llRtdResistanceCoefficient = 0;
    /*线性拟合区间*/
    TableLinear_t tTableLinear;
    /*保存计算温度值*/
    int64_t iTemperature = 0;
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;

    /*用户未断连*/
    if (ADSDISCONNECTIONVALUE !=gtaAISampleInfo[TMEPERATURE_CHANNEL_4].ulChannelDisconnectValue && ADSDISCONNECTIONVALUE_tvs != gtaAISampleInfo[TMEPERATURE_CHANNEL_4].ulChannelDisconnectValue)
    {
        /*用户1通道数据更新完成，可以更新业务数据区*/
        if(TMEPERATURE_CHANNEL_1 ==gucCurrentIndex)
        {   
            /*计算出系数并放大100000倍*/
            llRtdResistanceCoefficient = (ptDivingSourceIn->sUserChannelDivingSource_1_1 + ptDivingSourceIn->sUserChannelDivingSource_1_2)
                                          *PRECISION_MAGNIFICATION * RTD_PT100_RESISTANCE_REF>> RTD_PT100_FULL_SCALE;
            /*除以增益系数*/
            llRtdResistanceCoefficient = llRtdResistanceCoefficient/gtaTableInfo[ucTypeIn].lPGA;
            /*对传感器类型进行判断    如果是电阻类型传感器直接将电阻值显示给用户*/
            if(E_THERMAL_RESISTANCE_TYPE_RESISTANCE_MEASUREMENT == ucTypeIn)
            {
                /*判断是否超出范围*/
                if(llRtdResistanceCoefficient < gtaTableInfo[ucTypeIn].sStartTemperature*PRECISION_MAGNIFICATION || llRtdResistanceCoefficient > gtaTableInfo[ucTypeIn].sEndTemperature*PRECISION_MAGNIFICATION)
                {
                    /*更新数据区返回*/
                    gusaAnalogData[gucCurrentIndex/2] = gtaTableInfo[ucTypeIn].sEndTemperature*10;
                    /*通道数据超出范围标记*/
                    gtaAISampleInfo[gucCurrentIndex/2].ucConnectionFlag = ADS_OVERFLOW;
                    return E_IO_ERROR_NONE;
                }

                /*通道状态置为连接状态*/
                gtaAISampleInfo[gucCurrentIndex/2].ucConnectionFlag = ADS_CONNECTION;
                /*更新业务数据区*/
                UpdateDataArea(&llRtdResistanceCoefficient);
                return E_IO_ERROR_NONE;
            }
            
            /*对阻值转换*/
            ChangeVoltageRules(ucTypeIn,&llRtdResistanceCoefficient);
            /*查表取数据*/        
            eResult = GetValueFromTable(ucTypeIn,llRtdResistanceCoefficient,&tTableLinear);
            if (E_IO_ERROR_TABLE_DATA == eResult)
            {
                /*更新数据区返回*/
                gusaAnalogData[gucCurrentIndex/2] = gtaTableInfo[ucTypeIn].sEndTemperature*10;
                /*通道数据超出范围标记*/
                gtaAISampleInfo[gucCurrentIndex/2].ucConnectionFlag = ADS_OVERFLOW;
                return eResult;
            }
            
            /*线性拟合*/
            llRtdResistanceCoefficient = llRtdResistanceCoefficient/PRECISION_MAGNIFICATION;
            TableLinearFitting(&tTableLinear,llRtdResistanceCoefficient,&iTemperature);
            
            /*更新数据区  查询出来的温度直接保留一位小数显示出来*/
            UpdateDataArea(&iTemperature);  
            
            /*通道状态置为连接状态*/
            gtaAISampleInfo[gucCurrentIndex/2].ucConnectionFlag = ADS_CONNECTION;
            return E_IO_ERROR_NONE;
        }
    }
    else  if ( TMEPERATURE_CHANNEL_1 ==gucCurrentIndex)
    {
        /*更新数据区返回*/
        gusaAnalogData[gucCurrentIndex/2] = gtaTableInfo[ucTypeIn].sEndTemperature*10;
        /*断连  置通道状态为断连状态*/
        gtaAISampleInfo[USER_CHANNEL_0].ucConnectionFlag = ADS_DISCONNECTION;
    }
    
    /*用户未断连*/
    if (ADSDISCONNECTIONVALUE !=gtaAISampleInfo[TMEPERATURE_CHANNEL_5].ulChannelDisconnectValue && ADSDISCONNECTIONVALUE_tvs != gtaAISampleInfo[TMEPERATURE_CHANNEL_5].ulChannelDisconnectValue)
    {
        /*用户2通道数据更新完成，可以更新业务数据区*/
        if(TMEPERATURE_CHANNEL_3 ==gucCurrentIndex)
        {
            /*计算出系数并放大100000倍*/
            llRtdResistanceCoefficient = (ptDivingSourceIn->sUserChannelDivingSource_2_1 + ptDivingSourceIn->sUserChannelDivingSource_2_2)
                                          *PRECISION_MAGNIFICATION * RTD_PT100_RESISTANCE_REF>> RTD_PT100_FULL_SCALE;
            /*除以增益系数*/
            llRtdResistanceCoefficient = llRtdResistanceCoefficient/gtaTableInfo[ucTypeIn].lPGA;

            /*对传感器类型进行判断    如果是电阻类型传感器直接将电阻值显示给用户*/
            if(E_THERMAL_RESISTANCE_TYPE_RESISTANCE_MEASUREMENT == ucTypeIn)
            {
                /*判断是否超出范围*/
                if(llRtdResistanceCoefficient < gtaTableInfo[ucTypeIn].sStartTemperature*PRECISION_MAGNIFICATION || llRtdResistanceCoefficient > gtaTableInfo[ucTypeIn].sEndTemperature*PRECISION_MAGNIFICATION)
                {
                    /*更新数据区返回*/
                    gusaAnalogData[gucCurrentIndex/2] = gtaTableInfo[ucTypeIn].sEndTemperature*10;
                    /*通道数据超出范围标记*/
                    gtaAISampleInfo[gucCurrentIndex/2].ucConnectionFlag = ADS_OVERFLOW;
                    return E_IO_ERROR_NONE;
                }

                /*通道状态置为连接状态*/
                gtaAISampleInfo[gucCurrentIndex/2].ucConnectionFlag = ADS_CONNECTION;
                /*更新业务数据区*/
                UpdateDataArea(&llRtdResistanceCoefficient);
                return E_IO_ERROR_NONE;
            }
            
            /*对阻值转换*/
            ChangeVoltageRules(ucTypeIn,&llRtdResistanceCoefficient);
            /*查表取数据*/        
            eResult = GetValueFromTable(ucTypeIn,llRtdResistanceCoefficient,&tTableLinear);
            if(E_IO_ERROR_TABLE_DATA == eResult)
            {
                /*更新数据区返回*/
                gusaAnalogData[gucCurrentIndex/2] = gtaTableInfo[ucTypeIn].sEndTemperature*10;
                /*通道数据超出范围标记*/
                gtaAISampleInfo[gucCurrentIndex/2].ucConnectionFlag = ADS_OVERFLOW;
                return eResult;
            }
            /*线性拟合*/
            llRtdResistanceCoefficient = llRtdResistanceCoefficient/PRECISION_MAGNIFICATION;
            TableLinearFitting(&tTableLinear,llRtdResistanceCoefficient,&iTemperature);
            /*更新数据区  查询出来的温度直接保留一位小数显示出来*/
            UpdateDataArea(&iTemperature);  

            /*通道状态置为连接状态*/
            gtaAISampleInfo[gucCurrentIndex/2].ucConnectionFlag = ADS_CONNECTION;
            return E_IO_ERROR_NONE;
        }
    }
    else  if ( TMEPERATURE_CHANNEL_3 ==gucCurrentIndex)/*通道断连*/
    {
        /*更新数据区返回*/
        gusaAnalogData[gucCurrentIndex/2] = gtaTableInfo[ucTypeIn].sEndTemperature*10;
        /*断连  置通道状态为断连状态*/
        gtaAISampleInfo[USER_CHANNEL_1].ucConnectionFlag = ADS_DISCONNECTION;
    }

    return E_IO_ERROR_NONE;

}
/******************************************************************
*函数名称:void ChangeVoltageRules(TableType_e ucTypeIn,int64_t* puiVoltageInOut)
*功能描述:表格类型(热电阻类型)      
*输入参数:puiVoltageInOut  转换值地址 
*输出参数:void
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/3/17                              
******************************************************************/
void ChangeVoltageRules(TableType_e ucTypeIn,int64_t *plResistanceInOut)
{
    /*定义变量保存入参的值*/
    int64_t lResistanceTemp = 0;
    
    lResistanceTemp = *plResistanceInOut;
    switch(ucTypeIn)
    {
    case E_THERMAL_RESISTANCE_TYPE_PT1000 :
    {   
        /*根据规则将数据转换表格存储形式*/
        *plResistanceInOut = lResistanceTemp*RTD_PT100_TABLE_CHANGERULE;
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_PT200 :
    {
        /*根据规则将数据转换表格存储形式*/
        *plResistanceInOut = lResistanceTemp*RTD_PT100_TABLE_CHANGERULE;
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_PT500 :
    {   
        /*根据规则将数据转换表格存储形式*/
        *plResistanceInOut = lResistanceTemp*RTD_PT100_TABLE_CHANGERULE;
        break;
    }    
    case E_THERMAL_RESISTANCE_TYPE_PT100 :
    {        
        /*根据规则将数据转换表格存储形式*/
        *plResistanceInOut = lResistanceTemp*RTD_PT100_TABLE_CHANGERULE;
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_NI120 :
    {        
        /*根据规则将数据转换表格存储形式*/
        *plResistanceInOut = lResistanceTemp*RTD_PT100_TABLE_CHANGERULE;
        break;
    }
    case E_THERMAL_RESISTANCE_TYPE_RESISTANCE_MEASUREMENT :
    {
        /*根据规则将数据转换表格存储形式*/
        *plResistanceInOut = lResistanceTemp*RTD_PT100_TABLE_CHANGERULE;
        break;
    }

    default :
    {
        break;
    }
        }
}


/******************************************************************
*函数名称:
*功能描述:
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/4/29                              
******************************************************************/
void GetDataToDivingSource(DivingSourceInfo_t *ptDivingSourceOut,int32_t uiFilterDataIn)
{
    switch(gucCurrentIndex)
    {
    case TMEPERATURE_CHANNEL_0 :
    {  
        ptDivingSourceOut->sUserChannelDivingSource_1_1 = uiFilterDataIn;
        break;
                    
    }
    case TMEPERATURE_CHANNEL_1 :
    {
        ptDivingSourceOut->sUserChannelDivingSource_1_2 = uiFilterDataIn;
        break;
    }
    case TMEPERATURE_CHANNEL_2 :
    {
        ptDivingSourceOut->sUserChannelDivingSource_2_1 = uiFilterDataIn;
        break;
    }
    case TMEPERATURE_CHANNEL_3 :
    {  
        ptDivingSourceOut->sUserChannelDivingSource_2_2 = uiFilterDataIn;
        break;
    }
    default :
    {
        break;
    }    
    }    

}

/******************************************************************
*函数名称:IO_InterfaceResult_e GetValueFromTable(TableType_e ucTypeIn,int64_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
*功能描述:二分法查表
*输入参数:ucTypeIn 热电阻类型  iCheckValueIn 查表值 ptLinearLinearOut 线性区间地址
*输出参数:IO_InterfaceResult_e
*返回值:      E_IO_ERROR_TABLE_DATA  数据溢出
                         E_IO_ERROR_NONE          无错误 
*其它说明:
*修改日期    版本号   修改人    修改内容
                                                                  不将表格内容当成数据传入进来，直接从FLASH中读取 需要确定读取哪l表
*---------------------------------------------------
*2018/3/13                              
******************************************************************/
IO_InterfaceResult_e GetValueFromTable(TableType_e ucTypeIn,int64_t iCheckValueIn,TableLinear_t * ptLinearLinearOut)
{
    IO_InterfaceResult_e  eResult = E_IO_ERROR_NOT_BUTT;
    uint16_t usLowIndex = 0;
    /*保存二分法循环变量*/
    uint16_t  usMiddleIndex = 0;
    uint16_t usHighIndex = 0;
    /*定义变量保存从FLASH中读取的数据*/
    uint64_t ulMaxValue = 0;
    uint64_t ulMinVALue = 0;
    uint64_t usCalcTemp1 = 0;
    uint64_t usCalcTemp2 = 0;
    /*比较变量*/
    uint64_t usCompareValue = 0;
    /*定义变量保存地址*/
    uint32_t uiAddressTemp = 0;

    /*缩小100000倍  需要缩小*/
    iCheckValueIn = iCheckValueIn/PRECISION_MAGNIFICATION;
    usCompareValue = iCheckValueIn;
    usHighIndex = guiTableLength/FLASHADDRESSLEN -1;
    memcpy(&ulMinVALue,(void *)(guiTableAddress),FLASHADDRESSLEN);
    memcpy(&ulMaxValue,(void *)(guiTableAddress + (usHighIndex)*FLASHADDRESSLEN),FLASHADDRESSLEN);
    uiAddressTemp = guiTableAddress;
    
    if((usCompareValue < ulMinVALue) || (usCompareValue > ulMaxValue))
    {
        eResult = E_IO_ERROR_TABLE_DATA;
        return eResult; // 超限，不做特殊处理
    }

    while(usHighIndex >= usLowIndex)
    {
        usMiddleIndex = (usHighIndex + usLowIndex) / 2;
        
        /*mid的值有变化，重新读取对应FLASH中的地址*/
        memcpy(&usCalcTemp1,(void *)(uiAddressTemp + (usMiddleIndex)*FLASHADDRESSLEN),FLASHADDRESSLEN);

        if((usCompareValue >= usCalcTemp1) || (0 == usMiddleIndex))
        {
            /*计算Middle+1的值*/            
            memcpy(&usCalcTemp2,(void *)(uiAddressTemp + (usMiddleIndex+1)*FLASHADDRESSLEN),FLASHADDRESSLEN);
            if(usCompareValue <= usCalcTemp2)
            {
                ptLinearLinearOut->lTemperature_1 = usMiddleIndex;
                ptLinearLinearOut->lTemperature_2 = usMiddleIndex+1;
                ptLinearLinearOut->usaLinearValue1 = usCalcTemp1;
                ptLinearLinearOut->usaLinearValue2 = usCalcTemp2;
                /*对温度进行偏移*/
                TemperatureOffsetCalc(ucTypeIn,ptLinearLinearOut);
                return E_IO_ERROR_NONE;
            }
            else
            {
                usLowIndex = usMiddleIndex + 1;
            }
        }
        else if(usCompareValue < usCalcTemp1)
        {
            usHighIndex = usMiddleIndex - 1;
        }
    }
    return E_IO_ERROR_NONE;
}


#endif
#endif

