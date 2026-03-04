/******************************************************************
*
*文件名称:  Iocontrol.h
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

#ifndef __IO_CONTROL_H__
#define __IO_CONTROL_H__


/******************************************************************
*                             头文件                              *
******************************************************************/
#include "common.h"
#include "IOOptions.h"
#include "stm32f30x.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/* 以下为DO使用的宏定义 */
#define DO_DATA_NOT_CHANGED     (0)     /* DO数据未刷新 */
#define DO_DATA_CHANGED     (1)     /* DO数据已刷新，需要重新输出 */

/* 以下为DI使用的宏定义 */
#define SAMPLING_PERIOD (100)   /* 采样周期，0.1ms */
#define MAX_DEBOUNCE_TIME   (10000)     /* 最大去抖动时间 10ms */
#define MIN_DEBOUNCE_TIME   (1000)       /* 最小去抖动时间1ms */
#define MAX_DI_SAMPLE_COUNT (MAX_DEBOUNCE_TIME/SAMPLING_PERIOD)    /* 数字量采集数据存储区大小 */
 /* 组态软件配置的去抖动时间以毫秒为单位代码里以微妙为单位 */
#define DEBOUNC_TIME_SWITCH_FROM_MS_TO_US   (1000) 
/* 以上为DI使用的宏定义 */

/* 以下为AI使用的宏定义 */
#define AI_CHANNEL_NUMBER_2   (2)         /* 2通道AI */
#define AI_CHANNEL_NUMBER_4   (4)         /* 4通道AI */
#define AI_CHANNEL_NUMBER_8   (8)         /* 8通道AI */

#define ADS_SAMPLING_PERIOD (4)     /* AD芯片采样率为250ksps,即4us采集一个数据 */
#define MAX_FILTER_WIDTH    (10000)  /* 最小滤波宽度，1ms */
#define MIN_FILTER_WIDTH    (1000) /* 最大滤波宽度，10ms */


/* 以上两个宏需要同步修改 */

 /* 组态软件配置的滤波宽度以毫秒为单位代码里以微妙为单位 */
#define FILTER_WIDTH_SWITCH_FROM_MS_TO_US   (1000) 
/* 滤波宽度为1ms时二阶滤波数据区大小 */
/* 采样率为250ksps时，1ms采集250个数据 */
/* 一阶滤波处理16个数据,二阶滤波约处理4个数据 */
/* 2ms约处理8个数据 */
#define SENCOND_ORDER_SAMPLE_DATA_FIELD_SIZE    (4)

/* 以上为AI使用的宏定义 */

/*以下为AO使用的宏定义 */
#define AO_DATA_NOT_CHANGED     (0)     /* AO数据未刷新 */
#define AO_DATA_CHANGED     (1)     /* AO数据已刷新，需要重新输出 */


/******************************************************************
*                           数据类型                              *
******************************************************************/
/* DI采集数据结构 */
typedef struct
{
    uint8_t ucaSampleData[MAX_DI_SAMPLE_COUNT]; /* 采集数据存储区 */
    uint8_t ucConfigSampleCount;                /* 实际使用的采集数据区大小 */
    uint8_t ucCurrentSampleOffset;              /* 指示当前采集数据存放位置 */
    uint8_t ucSumSample;                        /* 采集到的所有数据累加和 */
    uint8_t ucReserved;                         /* 对齐 */
    uint16_t usDebounceTime;                     /* 去抖动时间，由组态软件配置 */
} DISampleInfo_t;

/* AI数据结构 */
/* 可配置的滤波宽度 */
typedef enum
{
    e_CONFIG_FILTER_WIDTH_1 = 1,    /* 滤波宽度为1 */
    e_CONFIG_FILTER_WIDTH_2 = 2,    /* 滤波宽度为2 */
    e_CONFIG_FILTER_WIDTH_3 = 3,    /* 滤波宽度为3 */
    e_CONFIG_FILTER_WIDTH_4 = 4,    /* 滤波宽度为4 */
    e_CONFIG_FILTER_WIDTH_5 = 5,    /* 滤波宽度为5 */
    e_CONFIG_FILTER_WIDTH_6 = 6,    /* 滤波宽度为6 */
    e_CONFIG_FILTER_WIDTH_7 = 7,    /* 滤波宽度为7 */
    e_CONFIG_FILTER_WIDTH_8 = 8,    /* 滤波宽度为8 */
    e_CONFIG_FILTER_WIDTH_9 = 9,    /* 滤波宽度为9 */
    e_CONFIG_FILTER_WIDTH_10 = 10,    /* 滤波宽度为10 */
}CONFIG_FILTER_WIDTH_e;

/* 实际计算可使用的滤波宽度 */
typedef enum
{
    e_CALC_FILTER_WIDTH_1 = 1,      /* 滤波宽度为1 */
    e_CALC_FILTER_WIDTH_2 = 2,      /* 滤波宽度为2 */
    e_CALC_FILTER_WIDTH_4 = 4,      /* 滤波宽度为4 */
    e_CALC_FILTER_WIDTH_8 = 8,      /* 滤波宽度为8 */
    e_CALC_FILTER_WIDTH_BUTT
}CALC_FILTER_WIDTH_e;

/* AI除数因子 */
typedef enum
{
    e_AI_DIVIDED_0 = 0,     /* 除以1 */
    e_AI_DIVIDED_1 = 1,     /* 除以2 */
    e_AI_DIVIDED_2 = 2,     /* 除以4 */
    e_AI_DIVIDED_3 = 3,     /* 除以8 */
    e_AI_DIVIDED_4 = 4,     /* 除以16 */
    e_AI_DIVIDED_5 = 5,     /* 除以32 */
    e_AI_DIVIDED_BUTT
}AI_DIVIDED_e;

/* AI可能的通道数 */
typedef enum
{
    e_AI_CHANNEL_NUM_2 = 2,     /* 2通道AI */
    e_AI_CHANNEL_NUM_4 = 4,     /* 4通道AI */
    e_AI_CHANNEL_NUM_8 = 8,     /* 8通道AI */
    e_AI_CHANNEL_NUM_BUTT
}AI_CHANNEL_NUM_e;

/* 滤波算法选择 */
typedef enum
{
    E_FILTER_SELECT_ALGORITHM = 0, /* 算术平均法 */
    E_FILTER_SELEECT_BUTT
}FilterSelect_e;

typedef enum
{
    E_CHANNEL_INITIALIZATION = 0,      /*当前通道为初始化状态*/
    E_CHANNEL_RUNNING,                  /*当前通道为正在运行状态*/  
    E_CHANNEL_CALCATING,                /*当前通道的数据可以计算*/     
    E_CHANNEL_BUTT
}CHANNELSTATUS_e;



/*AIAO指示灯状态*/
typedef enum
{
    E_AI_AO_INDICATOR_OFF = 0,
    E_AI_AO_INDICTOR_FLASH,
    E_AI_AO_INDICTOR_ON,
    E_AI_AO_INDICTOR_BUTT
}AIAOIndictor_e;

typedef struct
{
    uint8_t ucAIAOChennel; /*AIAO通道*/
    AIAOIndictor_e eAIAOChennelStatus;/*AIAO通道状态*/
    GPIO_TypeDef *GPIOx;/*AIAO通道GPIO*/
    uint32_t ulAIAOChennelPIN;/*AIAO通道管脚*/
}AIAOIndictor_t;

/*AO通道标定结构体*/
typedef struct
{
    int32_t lGainValue;                             /*通道补偿系数*/
    int32_t lCompensateValue;                      /*通道1234补偿常数*/
    int32_t lCompensateValue2;                      /*通道补偿常数*/
}AODemarcate_t;

typedef struct
{
    uint8_t  Enable;
    uint16_t ID;                /* 实际使用的采集数据区大小 */
    uint16_t DIData;
    uint8_t  InputData[32];
    uint8_t  OutputData[32];                     /* 采集到的所有数据累加和 */
    uint8_t  InputLen;
    uint8_t  OutputLen;
} ModuleInfo_t;
/******************************************************************
*                           数据类型                              *
******************************************************************/
/* AI,AO指示灯状态 */
typedef enum
{
    e_FLASH_OFF = 0,    /* 指示灯灭 */
    e_FLASH_ON,         /* 指示灯亮 */
    e_FLASH_BUTT
}FlsahMode_e;
extern uint8_t AISampleType[MAX_AI_CHANNEL_NUMBER];
extern uint8_t AIChannelSet[MAX_AI_CHANNEL_NUMBER];
extern int16_t AIValue[MAX_AI_CHANNEL_NUMBER];


/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
void initChannelOutput();
void ZeroChannelOutput();
void HandleIoData(void);
IO_InterfaceResult_e InitConfigDataField(void);
IO_InterfaceResult_e SetDoData(uint8_t *pucDigitalData);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
IO_InterfaceResult_e GetDiData(uint8_t *pucDigitalData);
void DiSamplingTimerInit(uint32_t ustimeoutUS);
void DisConnectWithGWTimerInit(uint32_t ustimeoutUS);
void ArithmeticAverageFilter(uint16_t *pusSampleDataBufferIn, uint16_t usSampleDataCountIn, uint16_t *pusAnalogDataOut);
IO_InterfaceResult_e GetAiData(uint16_t *pusAnalogData);
IO_InterfaceResult_e SetAoData(uint16_t *pusAnalogData);
void ArithmeticAverageFilter2Channel(uint16_t *pusSampleDataBufferIn);
void ArithmeticAverageFilter8Channel(uint16_t *pusSampleDataBufferIn);
void ArithmeticAverageFilter4Channel(uint16_t *pusSampleDataBufferIn);
void AO_AIIndicatorTimerInit(uint32_t ustimeoutUS);
void AIAOIndictorGPIOInit(void);
void AIAOSetIndictor(uint8_t ucAIAOChannelNum, FlsahMode_e eFlsahMode);
void TIM6_Config(void);
void TIM6_DAC_IRQHandler(void);
#endif

