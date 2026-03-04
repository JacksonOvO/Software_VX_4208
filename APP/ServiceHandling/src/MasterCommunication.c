/******************************************************************
*
*文件名称:  MasterCommunication.c
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
#include "MasterCommunication.h"
#include "Iocontrol.h"
#include "MbCommunication.h"
#include "stm32DAC8555driver.h"
#include <stdio.h>
#include "SEGGER_RTT.h"

/******************************************************************
*                            常量                                 *
******************************************************************/
int8_t RangeSwitchingFlagBit = 0;
/******************************************************************
*                           宏定义                                *
******************************************************************/
#define printf(...) SEGGER_RTT_printf(0, __VA_ARGS__)

/* 某个基本信息内容的最大长度*/
#define MAX_ONE_BASE_CONTENT_LENGTH (16)

/* 一个配置信息的长度*/
#define CONFIG_INFO_VALUE_LENGTH (4)

/* 数据ID所占的长度*/
#define DATA_ID_SPACE_LENGTH (1) 

/* 数据长度所占的长度*/
#define DATA_LENGTH_SPACE_LENGTH (1)

/* PLC地址所占的长度*/
#define DATA_PLC_ADDR_SPACE_LENGTH (1)

/* 开始B IT位所占的长度*/
#define DATA_START_BIT_SPACE_LENGTH (1)

/* 名称长度所占的字节*/
#define NAME_LENGTH_BIT_SPACE_LENGTH (1)

/* 设备描述文件中的一个数据长度*/
#define ONE_DATA_LENGTH_IN_DEVICE_INFO (DATA_ID_SPACE_LENGTH+DATA_LENGTH_SPACE_LENGTH)

/* 配置文件中的一个数据长度*/
#define ONE_DATA_LENGTH_IN_CONFIG_INFO (DATA_ID_SPACE_LENGTH+DATA_LENGTH_SPACE_LENGTH+DATA_PLC_ADDR_SPACE_LENGTH+DATA_START_BIT_SPACE_LENGTH)

/* FLASH中空地址长度标识*/
#define FLASH_LENGTH_BLANK_FLAG 0xFFFF

/******************************************************************
*                           数据类型                              *
******************************************************************/
/* 基本信息ID定义*/
typedef enum
{
    E_DEVICE_BASE_INFO_ID_VENDOR = 1, /* 厂商ID*/
    E_DEVICE_BASE_INFO_ID_PRODUCT = 5, /* 产品型号*/
    E_DEVICE_BASE_INFO_ID_CHANNEL_COUNT = 101, /* 通道个数*/
    E_DEVICE_BASE_INFO_ID_BUTT
} DeviceBaseInfoID_e;



/* 配置信息ID定义*/
typedef enum
{
    E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_1=1,  /* 通道1模式配置*/
    E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_2, /* 通道2模式配置*/
    E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_3, /* 通道3模式配置*/
    E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_4, /* 通道4模式配置*/
    E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_5,  /* 通道1模式配置*/
    E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_6, /* 通道2模式配置*/
    E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_7, /* 通道3模式配置*/
    E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_8, /* 通道4模式配置*/
    E_AI_DEVICE_CONFIG_INFO_ID_BUTT 
} AIDeviceConfigInfoID_e;


/* 基本信息中需要比较的数据类型*/
typedef enum
{
    E_BASE_INFO_COMPARE_VENDOR = 0, /* 厂商ID内容*/
    E_BASE_INFO_COMPARE_PRODUCT, /* 产品型号内容*/
    E_BASE_INFO_COMPARE_BUTT
} BaseInfoCompareType_e;

/******************************************************************
*                         全局变量定义                            *
******************************************************************/
/* 设备描述文件部分信息缓冲区*/
uint8_t gucaIOPartDeviceInfo[MAX_PART_DEVICE_INFO_LENGTH];

/* BEGIN: Added by we004, 2017/12/17   PN:从站诊断、配置时，从站处理时间过长，导致Modbus 485通信超时 */
/* 设备描述文件所有信息缓冲区*/
uint8_t gucaIoWholeDeviceInfo[MAX_WHOLE_DEVICE_INFO_LENGTH];
IODeviceContentInfo_t taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_BUTT];
/* END:   Added by we004, 2017/12/17 */


/*从站设备ID*/
uint8_t gucaSlaveDeviceID[DEVICE_ID_LENGTH] = {0};


/* 通道个数*/
extern uint8_t gucChannelCount;

#define MAX_GAIN_VALUE_COUNT   (16)

/*标定值固化*/
__root const int32_t gllaGainValue[MAX_GAIN_VALUE_COUNT]@".gllaGainValue" = 
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};






/* AO设备描述文件定义*/
__root const uint8_t gucaIODeviceFile[MAX_IO_DEVICE_INFO_LENGTH]@".IODeviceInfo" = 
{
    /* 总长度*/
    0x27, 0x03,  //0xDA, 0x00,
    /* 基本信息*/
    0x2D, 0x00,                                          /* 基本信息长度*/
    /*ID(1字节)，长度(1字节)，内容(可变)*/
    0x01, 0x01,0x17,                                     /* 厂商ID*/
    0x02, 0x02,'C','T',                               /* 厂商名称*/
    0x04, 0x02,'A','O',                                  /* 产品类型*/
    0x05, 0x06,'V','X','4','2','0','8',               /* 产品型号*/
    0x06, 0x05,'2','8','5','m','A',                          /* 功耗*/
    0x35, 0x02,0x00,0x01,                                /* 软件版本号1*/
    0x36, 0x02,0x00,0x01,                               /* 软件版本号2*/
    0x65, 0x01, 0x38,                                       /* 通道数 */
    0x66, 0x02, '1','6',                                    /* 步进值 */
    0xFF,0x02,0x42,0x00,                                 /*代码类型*/
    /* 配置信息*/
    0x18, 0x01,                     /* 配置信息长度*/
    /*ID(1字节)，最小值(4字节)，最大值(4字节)，默认值(4字节)，名称长度(1字节)，名称(可变)*/
    0x01, 0x00,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0X15, 'C','h','1',':','M','e','a','s','u','r','e','m','e','n','t',' ','r','a','n','g','e', //采集配置模式 
    0x02, 0x00,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0X15, 'C','h','2',':','M','e','a','s','u','r','e','m','e','n','t',' ','r','a','n','g','e', //采集配置模式 默认0 -10-10V，1为0-10V 2为0-5V             
    0x03, 0x00,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0X15, 'C','h','3',':','M','e','a','s','u','r','e','m','e','n','t',' ','r','a','n','g','e',
    0x04, 0x00,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0X15, 'C','h','4',':','M','e','a','s','u','r','e','m','e','n','t',' ','r','a','n','g','e',
    0x05, 0x00,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0X15, 'C','h','5',':','M','e','a','s','u','r','e','m','e','n','t',' ','r','a','n','g','e', //采集配置模式 
    0x06, 0x00,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0X15, 'C','h','6',':','M','e','a','s','u','r','e','m','e','n','t',' ','r','a','n','g','e', //采集配置模式 默认0 -10-10V，1为0-10V 2为0-5V             
    0x07, 0x00,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0X15, 'C','h','7',':','M','e','a','s','u','r','e','m','e','n','t',' ','r','a','n','g','e',
    0x08, 0x00,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0X15, 'C','h','8',':','M','e','a','s','u','r','e','m','e','n','t',' ','r','a','n','g','e',
    
    /* 下行数字量业务数据 */
    0x00, 0x00,                                          /* 业务数据长度*/
    /* 下行模拟量业务数据 */
    0x60, 0x00,                                          /* 业务数据长度*/
    /*ID(1字节)，长度(1字节)，名称长度(1字节)，名称(可变)*/
    0x01, 0x10, 0x09, 'C','h','a','n','n','e','l',' ','1',    /* 通道1 */
    0x02, 0x10, 0x09, 'C','h','a','n','n','e','l',' ','2',    /* 通道2 */
    0x03, 0x10, 0x09, 'C','h','a','n','n','e','l',' ','3',    /* 通道3 */
    0x04, 0x10, 0x09, 'C','h','a','n','n','e','l',' ','4',    /* 通道4 */   
    0x05, 0x10, 0x09, 'C','h','a','n','n','e','l',' ','5',    /* 通道1 */
    0x06, 0x10, 0x09, 'C','h','a','n','n','e','l',' ','6',    /* 通道2 */
    0x07, 0x10, 0x09, 'C','h','a','n','n','e','l',' ','7',    /* 通道3 */
    0x08, 0x10, 0x09, 'C','h','a','n','n','e','l',' ','8',    /* 通道4 */ 
    
    /* 上行数字量业务数据 */
    0x00, 0x00,                                          /* 业务数据长度*/
    /* 上行模拟量业务数据 */
    0x1A, 0x00,                                          /* 业务数据长度*/
    0x01, 0x10, 0x0A, 'E','r','r','o','r','C','o','d','e','1',    /* 通道1 */
    0x02, 0x10, 0x0A, 'E','r','r','o','r','C','o','d','e','2',    /* 通道1 */
    /*ID(1字节)，长度(1字节)*/
    /* 诊断信息*/
    0x18, 0x00,                                          /* 诊断信息长度*/
    /*ID(1字节)，诊断信息内容长度(1字节)，名称长度(1字节)，名称(可变)*/
    0x01, 0x10, 0x0B, 'T','e','m','p','e','r','a','t','u','r','e',     /* 温度*/
    0x02, 0x10, 0x07, 'V','o','l','t','a','g','e',    /* 5V电源电压*/
    /* 告警数据*/
    0x24, 0x00,                                          /* 告警信息长度*/
    /*ID(1字节)，告警数据内容长度(1字节)，名称长度(1字节)，名称(可变)*/
    0x01, 0x01, 0x11, 'T','e','m','p','e','r','a','t','u','r','e',' ','A','l','a','r','m',        /* 温度异常告警 */
    0x02, 0x01, 0x0D, 'V','o','l','t','a','g','e',' ','A','l','a','r','m'         /*电源 电压异常告警 */
};





/* 产品信息定义*/
__root const uint8_t gucaIOVersionInfo[MAX_IO_PRODUCT_INFO_LENGTH]@".IOVersionInfo" = 
{
    /* 总长度 */
    0x18, 0x00, 
    /*  ID(1字节), 长度(1字节)，内容(可变) */
    0x33, 0x02, 0x00,0x01,                                                 /*从站硬件版本号1*/
    0x34, 0x02, 0x00,0x00,                                                 /* 从站硬件版本号2*/
    0x37, 0x02, 0x00,0x01,                                                 /*产品批次1*/
    0x38, 0x02, 0x00,0x01,                                                 /* 产品批次2*/
    0x03, 0x06,'0','6','4','0','0','8',                                      /* 产品ID*/
};


/* 升级信息定义 */
__root const uint8_t gucaIOUpgradeInfo[MAX_IO_UPGRADE_INFO_LENGTH]@".IOUpgradeInfo" = 
{
    /* 软件类型,1字节 */
    IO_CODE_TYPE, 
    /* 软件支持的硬件版本号 */


    0x01,           /* 支持的硬件版本个数 */
    0x00,0x00,0x00,0x01,    /* 硬件版本号 1.0.0.0 */

};



/******************************************************************
*                          局部函数声明                            *
******************************************************************/
static IO_InterfaceResult_e GetBaseInfoContent(uint8_t ucBaseInfoIDIn, uint16_t usBaseInfoLengthIn, uint8_t *pucBaseInfoIn,  
            uint8_t ucContentBuffLengthIn, uint8_t *pucContentBuffOut, uint8_t *pucContentLengthOut);
static IO_InterfaceResult_e GetDeviceConfigInfoRange(uint8_t ucConfigInfoIDIn, uint16_t usConfigInfoLengthIn, uint8_t *pucConfigInfoIn,
                                                                    uint32_t *puiMinValueOut, uint32_t *puiMaxValueOut);
static void SetConfigValue(uint8_t ucConfigIDIn, uint32_t uiConfigValueIn);

static IO_InterfaceResult_e GetIoDataErrorValue(IoDeviceInfoType_e eDiagnoseTypeIn);
static IO_InterfaceResult_e GetDeviceDataLength(uint8_t ucConfigDataIDIn, uint16_t usDeviceDataInfoLengthIn, uint8_t *pucDeviceDataInfoIn,
                                                                    uint8_t *pucDeviceDataLengthOut);
static IO_InterfaceResult_e ReadFlashIoDeviceInfo(uint16_t usIoDeviceInfoLengthIn, IoDeviceInfoType_e eReadFlagIn, 
                                                        uint8_t *pucIoDeviceInfoOut, uint16_t *pusReadLengthOut);


/******************************************************************
*                         全局函数实现                            *
******************************************************************/
/* BEGIN: Added by we004, 2017/12/17   PN:从站诊断、配置时，从站处理时间过长，导致Modbus 485通信超时 */
/******************************************************************
*函数名称:InitIoDeviceContentInfo
*功能描述:初始化设备描述文件内容信息
*输入参数:taIODevcieContentInfoIn 设备描述文件内容信息数据组的首地址
                        ucIODeviceContentCountIn 设备描述文件内容类型个数
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/17                              we004
******************************************************************/
void Change_CH_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOBEN, ENABLE);

    GPIO_InitStructure.GPIO_Pin = CH1_CTRL_GPIO_PIN|CH2_CTRL_GPIO_PIN|CH3_CTRL_GPIO_PIN|CH4_CTRL_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
    
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

IO_InterfaceResult_e GetIoDeviceContentInfo(void)
{
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT;
    uint8_t i = 0; /* 循环临时变量*/
    uint16_t usUsedBuffLength = 0;/* 设备描述文件已经使用的缓冲区长度*/
    

    FUNC_ENTER();

    /* 获取各部分内容信息以及长度*/    
    memset(gucaIoWholeDeviceInfo, 0, MAX_WHOLE_DEVICE_INFO_LENGTH);
    memset(taIoDeviceContentInfo, 0, sizeof(IODeviceContentInfo_t)*E_IO_DEVICE_INFO_TYPE_BUTT);
    usUsedBuffLength = 0;
    for (i = 0; i < E_IO_DEVICE_INFO_TYPE_BUTT; i++)
    {
        taIoDeviceContentInfo[i].pucContentInfo = gucaIoWholeDeviceInfo + usUsedBuffLength;
        if (i == E_IO_DEVICE_INFO_TYPE_PRODUCT)
        {
            /* 获取产品信息的内容以及长度*/
            eInterfaceResult = ReadFlashProductInfo((MAX_WHOLE_DEVICE_INFO_LENGTH - usUsedBuffLength), 
                                                    taIoDeviceContentInfo[i].pucContentInfo, 
                                                    &(taIoDeviceContentInfo[i].usContentLength));
            if (E_IO_ERROR_NONE != eInterfaceResult)
            {
                return eInterfaceResult;
            }
        }
        else
        {
            /* 获取基本信息、业务数据、告警、诊断信息内容以及长度*/
            eInterfaceResult = ReadFlashIoDeviceInfo((MAX_WHOLE_DEVICE_INFO_LENGTH - usUsedBuffLength), (IoDeviceInfoType_e)i, taIoDeviceContentInfo[i].pucContentInfo, 
                                                        &(taIoDeviceContentInfo[i].usContentLength));
            if (eInterfaceResult != E_IO_ERROR_NONE)
            {
                return eInterfaceResult;
            }
        }

        /* 重新计算已经使用的缓冲区长度*/
        usUsedBuffLength = usUsedBuffLength + taIoDeviceContentInfo[i].usContentLength;

        if (usUsedBuffLength > MAX_WHOLE_DEVICE_INFO_LENGTH)
        {
            return eInterfaceResult;            
        }
    }
    
    FUNC_EXIT();

    return E_IO_ERROR_NONE;
}
/* END:   Added by we004, 2017/12/17 */

/******************************************************************
*函数名称:DiagnoseIoDeviceBaseInfo
*功能描述:以设备描述文件的基本信息为基准，诊断网关发送的配置文件基本信息是否正确
*输入参数:pucConfigBaseInfoIn 指向网关发送的配置文件基本信息
                        usConfigBaseLengthIn 网关发送的配置文件基本信息长度
*输出参数:无
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/28                              we004
******************************************************************/
IO_InterfaceResult_e DiagnoseIoDeviceBaseInfo(uint8_t *pucConfigBaseInfoIn, uint16_t usConfigBaseLengthIn)
{
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT;
    uint16_t usDeviceBaseInfoLength = 0;
    uint8_t ucDeviceContentLength = 0;
    uint8_t ucConfigContentLength = 0;
    uint8_t ucDeviceContent[MAX_ONE_BASE_CONTENT_LENGTH];
    uint8_t ucConfigContent[MAX_ONE_BASE_CONTENT_LENGTH]; /* */
    uint8_t ucaCompareIDs[E_BASE_INFO_COMPARE_BUTT]; /* 需要比较的基本信息ID*/
    uint8_t i = 0; /* 循环临时变量*/
    uint8_t *pucIoPartDeviceInfo = NULL;
        

    FUNC_ENTER();
    
    /* 入参判断*/
    NULL_CHECK(pucConfigBaseInfoIn);


    /* 获取设备描述文件基本信息*/
    pucIoPartDeviceInfo = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_BASE].pucContentInfo;
    usDeviceBaseInfoLength = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_BASE].usContentLength;
    
    /* 初始化需要比较的基本信息ID*/
    memset(ucaCompareIDs, 0, E_BASE_INFO_COMPARE_BUTT);
    ucaCompareIDs[E_BASE_INFO_COMPARE_VENDOR] = E_DEVICE_BASE_INFO_ID_VENDOR;
    ucaCompareIDs[E_BASE_INFO_COMPARE_PRODUCT] = E_DEVICE_BASE_INFO_ID_PRODUCT;

    /* 比较基本信息，并获取相关内容*/
    for (i=0; i<E_BASE_INFO_COMPARE_BUTT; i++)
    {
        /* 从设备描述文件中获取内容长度以及内容*/
        memset(ucDeviceContent, 0, MAX_ONE_BASE_CONTENT_LENGTH);
        eInterfaceResult = GetBaseInfoContent(ucaCompareIDs[i], usDeviceBaseInfoLength, pucIoPartDeviceInfo,  
                                    MAX_ONE_BASE_CONTENT_LENGTH, ucDeviceContent, &ucDeviceContentLength);
        if (eInterfaceResult != E_IO_ERROR_NONE)
        {
            return eInterfaceResult;
        }

        /* 从配置信息中内容长度以及内容*/
        memset(ucConfigContent, 0, MAX_ONE_BASE_CONTENT_LENGTH);
        eInterfaceResult = GetBaseInfoContent(ucaCompareIDs[i], usConfigBaseLengthIn, pucConfigBaseInfoIn,  
                                    MAX_ONE_BASE_CONTENT_LENGTH, ucConfigContent, &ucConfigContentLength);
        if (eInterfaceResult != E_IO_ERROR_NONE)
        {
            return eInterfaceResult;
        }

        if (ucDeviceContentLength != ucConfigContentLength)
        {
            return E_IO_ERROR_DIAGNOSIS_IO_BASE_INFO;
        }

        if (memcmp(ucDeviceContent, ucConfigContent, ucDeviceContentLength))
        {
            return E_IO_ERROR_DIAGNOSIS_IO_BASE_INFO;
        }    
    }

    memset(ucDeviceContent, 0, MAX_ONE_BASE_CONTENT_LENGTH);
     /* 获取通道个数*/
    eInterfaceResult = GetBaseInfoContent(E_DEVICE_BASE_INFO_ID_CHANNEL_COUNT, usDeviceBaseInfoLength, pucIoPartDeviceInfo,  
                                    MAX_ONE_BASE_CONTENT_LENGTH, ucDeviceContent, &ucDeviceContentLength);
    if (eInterfaceResult != E_IO_ERROR_NONE)
    {
        return eInterfaceResult;
    }
    gucChannelCount = (*(ucDeviceContent) - '0');
    
    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:ConfigSlaveStation
*功能描述:诊断配置信息，如果合法则配置从站
*输入参数:pucConfigConfigInfoIn 指向配置信息的指针
                        ucConfigConfigLengthIn 配置信息长度
*输出参数:无
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/29                              we004
******************************************************************/
IO_InterfaceResult_e ConfigSlaveStation(uint8_t *pucConfigConfigInfoIn, uint16_t ucConfigConfigLengthIn)
{
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT;
    uint16_t usDeviceConfigInfoLength = 0;
    uint16_t usTempLength = 0;
    uint8_t ucConfigInfoID = 0;
    uint32_t uiConfigInfoValue = 0;
    int32_t uiMinValue = 0;
    int32_t uiMaxValue = 0;
    uint8_t *pucIoPartDeviceInfo = NULL;


    FUNC_ENTER();
    
    /* 入参检查*/
    NULL_CHECK(pucConfigConfigInfoIn);

    
    /* 获取设备描述文件配置信息*/
    pucIoPartDeviceInfo = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_CONFIG].pucContentInfo;
    usDeviceConfigInfoLength = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_CONFIG].usContentLength;   
    RangeSwitchingFlagBit = 1;
    usTempLength = 0;
    while (usTempLength < ucConfigConfigLengthIn)
    {
        /*获取当前配置信息的ID 以及值*/
        ucConfigInfoID = pucConfigConfigInfoIn[usTempLength++];
        memcpy(&uiConfigInfoValue, (pucConfigConfigInfoIn + usTempLength), CONFIG_INFO_VALUE_LENGTH);
        
        /* 获取配置信息范围*/
        uiMinValue = 0;
        uiMaxValue = 0;
        eInterfaceResult = GetDeviceConfigInfoRange(ucConfigInfoID, usDeviceConfigInfoLength, pucIoPartDeviceInfo, &uiMinValue, &uiMaxValue);
        if (eInterfaceResult != E_IO_ERROR_NONE)
        {
            return eInterfaceResult;
        }

        /* 检查是否合法*/
        if (uiConfigInfoValue < uiMinValue || uiConfigInfoValue > uiMaxValue)
        {
            return E_IO_ERROR_CONFIG_INFO_VALUE_INVALID;
        }
        
        /* 设置配置信息值*/
        SetConfigValue(ucConfigInfoID, uiConfigInfoValue);
        
        /* 偏移到下一个配置信息*/
        usTempLength = usTempLength + CONFIG_INFO_VALUE_LENGTH;
    }    
    RangeSwitchingFlagBit = 0;
    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:DiagnoseIoDataInfo
*功能描述:诊断业务数据、告警数据、诊断信息是否合法
*输入参数:pucIoConfigFileInfoIn网关发送的配置文件信息
                        IoConfigFileInfoLenIn网关发送的配置文件信息长度
                        eDiagnoseTypeIn 数据类型
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/29                              we004
******************************************************************/
IO_InterfaceResult_e DiagnoseIoDataInfo(uint8_t *pucIoConfigFileInfoIn, uint8_t IoConfigFileInfoLengthIn, IoDeviceInfoType_e eDiagnoseTypeIn)
{
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT;
    uint16_t usDeviceDataLength = 0; /* 设备描述文件中的数据长度*/
    uint16_t usTempLength = 0; /* 临时变量，存放数据长度*/
    uint8_t ucDataID = 0; /* 数据ID*/
    uint8_t ucConfigDataLength = 0; /* 配置信息中的数据长度*/
    uint8_t ucDeviceDataLength = 0; /* 设备描述文件中的数据长度*/
    uint8_t *pucIoPartDeviceInfo = NULL;
    

    FUNC_ENTER();
    
    /* 入参判断*/
    NULL_CHECK(pucIoConfigFileInfoIn);
    if (eDiagnoseTypeIn >= E_IO_DEVICE_INFO_TYPE_BUTT || eDiagnoseTypeIn <= E_IO_DEVICE_INFO_TYPE_CONFIG)
    {
        return E_IO_ERROR_DATA_TYPE_INVALID;
    }
    
    /* 获取设备描述文件中对应的内容*/    
    pucIoPartDeviceInfo = taIoDeviceContentInfo[eDiagnoseTypeIn].pucContentInfo;
    usDeviceDataLength = taIoDeviceContentInfo[eDiagnoseTypeIn].usContentLength;   

    /* 检查数据ID以及长度是否合法*/
    usTempLength = 0;
    while (usTempLength < IoConfigFileInfoLengthIn)
    {
        /*获取当前数据的ID 以及长度*/
        ucDataID = pucIoConfigFileInfoIn[usTempLength];
        ucConfigDataLength = pucIoConfigFileInfoIn[usTempLength+DATA_ID_SPACE_LENGTH];

        /* 获取设备描述文件中的数据长度*/
        ucDeviceDataLength = 0;
        eInterfaceResult = GetDeviceDataLength(ucDataID,usDeviceDataLength,pucIoPartDeviceInfo,&ucDeviceDataLength);
        if (eInterfaceResult != E_IO_ERROR_NONE)
        {
            return eInterfaceResult;
        }

        /* 检查是否合法*/
        if (ucDeviceDataLength != ucConfigDataLength)
        {
            return GetIoDataErrorValue(eDiagnoseTypeIn);
        }

        /* 偏移到下一个配置信息*/
        usTempLength = usTempLength + ONE_DATA_LENGTH_IN_CONFIG_INFO;
    }    

    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;    
}

/******************************************************************
*函数名称:ReadFlashProductInfo
*功能描述:从固定的FLASH中读取版本、序列号等信息
*输入参数:
usIoDeviceInfoLengthIn: 缓冲区长度
pucIoDeviceInfoOut:缓冲区指针
pusReadLengthOut:实际读取的长度
*输出参数:无
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/6                              lv we004
******************************************************************/
IO_InterfaceResult_e ReadFlashProductInfo(uint16_t usIoDeviceInfoLengthIn, 
                                uint8_t *pucIoDeviceInfoOut, uint16_t *pusReadLengthOut)
{
    uint32_t uiReadStartAddr = FLASH_PRODUCT_INFO_START_ADDR; /* 读取开始位置*/
    uint16_t usReadLength = 0; /* 读取长度*/
    

    FUNC_ENTER();
    
    /* 入参判断*/
    NULL_CHECK(pucIoDeviceInfoOut);
    NULL_CHECK(pusReadLengthOut);

    memcpy(&usReadLength, (uint8_t *)uiReadStartAddr, FLASH_DEVICE_INFO_LENGTH_COUNT);
    
    /* 判断产品信息内容是否合法*/
    if (usReadLength == FLASH_LENGTH_BLANK_FLAG)
    {
        return E_IO_ERROR_IO_DEVICE_FLASH_BLANK;
    }

    /* 判断缓冲区长度是否合法*/
    if (usReadLength > usIoDeviceInfoLengthIn)
    {
        return E_IO_ERROR_IO_DEVICE_BUFFER_TOO_SHORT;
    }    

    /*复制到缓冲区中*/
    memcpy(pucIoDeviceInfoOut, (void *)(uiReadStartAddr+FLASH_DEVICE_INFO_LENGTH_COUNT), usReadLength);
    (*pusReadLengthOut) = usReadLength;

    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:AssemblyIoBaseInfo
*功能描述:把设备描述文件基本信息和版本信息组装成新的基本信息
*输入参数:
pucIoDeviceBaseInfoIn:存放组装后的基本信息的缓存，已经存放了从
FLASH中读取的基本信息，长度为usIoDeviceBaseInfoLengthIn
usIoDeviceBaseInfoLengthIn:存放组装后的基本信息的长度
usIoDeviceInfoLengthIn:从FLASH中读取的基本信息的长度
pucIoVersionInfoIn从FLASH中读取的版本信息
usIoVersionInfoLengthIn:从FLASH中读取的版本信息的长度
pusBaseInfoLengthOut:组装后的基本信息长度
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/6                              lv we004
******************************************************************/
IO_InterfaceResult_e AssemblyIoBaseInfo(uint8_t *pucIoDeviceBaseInfoIn, uint16_t usIoDeviceBaseInfoLengthIn,
        uint16_t usIoDeviceInfoLengthIn,
        uint8_t *pucIoVersionInfoIn, uint16_t usIoVersionInfoLengthIn,
        uint16_t *pusBaseInfoLengthOut)
{
    FUNC_ENTER();
    
    /* 入参判断*/
    NULL_CHECK(pucIoDeviceBaseInfoIn);
    NULL_CHECK(pucIoVersionInfoIn);
    NULL_CHECK(pusBaseInfoLengthOut);

    if (usIoDeviceBaseInfoLengthIn < (usIoDeviceInfoLengthIn + usIoVersionInfoLengthIn))
    {
        return E_IO_ERROR_IO_DEVICE_BUFFER_TOO_SHORT;
    }

    /*复制到缓冲区中*/
    memcpy(pucIoDeviceBaseInfoIn+usIoDeviceInfoLengthIn, pucIoVersionInfoIn, usIoVersionInfoLengthIn);
    (*pusBaseInfoLengthOut) = usIoDeviceInfoLengthIn + usIoVersionInfoLengthIn;

    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;    
}

/******************************************************************
*                         局部函数实现                            *
******************************************************************/
/******************************************************************
*函数名称:GetBaseInfoContent
*功能描述:根据基本信息ID获取基本信息内容长度以及基本信息内容
*输入参数:ucBaseInfoIDIn 基本信息ID
                        ucBaseInfoIDIn 指向所有基本信息内容的指针
                        ucContentBuffLengthIn 存储某个基本信息内容的缓冲区长度
*输出参数:pucContentLengthOut 指向某个基本信息内容长度的指针
                        pucContentBuffOut存储某个基本信息内容的缓冲区
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/29                              we004
******************************************************************/
static IO_InterfaceResult_e GetBaseInfoContent(uint8_t ucBaseInfoIDIn, uint16_t usBaseInfoLengthIn, uint8_t *pucBaseInfoIn,  
                                uint8_t ucContentBuffLengthIn, uint8_t *pucContentBuffOut, uint8_t *pucContentLengthOut)
{
    uint16_t usTempBaseLength = 0; /* 临时循环变量*/
    uint8_t ucBaseID = 0;
    uint8_t ucBaseContentLength = 0;


    FUNC_ENTER();
    
    /* 入参判断*/
    NULL_CHECK(pucBaseInfoIn);
    NULL_CHECK(pucContentLengthOut);
    NULL_CHECK(pucContentBuffOut);
    
    usTempBaseLength = 0;
    while (usTempBaseLength < usBaseInfoLengthIn)
    {
        /* 获取当前基本信息ID、基本信息内容长度*/
        ucBaseID = pucBaseInfoIn[usTempBaseLength++];
        ucBaseContentLength = pucBaseInfoIn[usTempBaseLength++];

        /* 获取基本信息内容以及长度*/
        if (ucBaseID == ucBaseInfoIDIn)
        {
            /* 基本信息内容长度大于存放的缓冲区长度*/
            if (ucBaseContentLength > ucContentBuffLengthIn)
            {
                return E_IO_ERROR_BASE_INFO_CONTENT_TOO_LONG;
            }
            (*pucContentLengthOut) = ucBaseContentLength;
            memcpy(pucContentBuffOut, (pucBaseInfoIn+usTempBaseLength), ucBaseContentLength);
            break;
        }

        /* 偏移到下一个基本信息*/
        usTempBaseLength = usTempBaseLength + ucBaseContentLength;
    }

    if (usTempBaseLength >= usBaseInfoLengthIn)
    {
        return E_IO_ERROR_BASE_INFO_ID_NOT_EXIST;
    }

    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:GetDeviceConfigInfoRange
*功能描述:获取设备描述文件中某个配置信息ID的最大值 与最小值
*输入参数:ucConfigInfoIDIn 配置信息ID
                        usConfigInfoLengthIn 配置信息的缓存区长度
                        pucConfigInfoIn 配置信息内容
*输出参数:puiMinValueOut 配置信息最小值
                        puiMaxValueOut配置信息最大值
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/29                              we004
******************************************************************/
static IO_InterfaceResult_e GetDeviceConfigInfoRange(uint8_t ucConfigInfoIDIn, uint16_t usConfigInfoLengthIn, uint8_t *pucConfigInfoIn,
                                                                    uint32_t *puiMinValueOut, uint32_t *puiMaxValueOut)
{
    uint16_t usTempConfigLength = 0; /* 临时循环变量*/
    uint8_t ucConfigInfoID = 0;


    FUNC_ENTER();
    
    /* 入参检查*/
    NULL_CHECK(pucConfigInfoIn);
    NULL_CHECK(puiMinValueOut);
    NULL_CHECK(puiMaxValueOut);

    usTempConfigLength = 0;
    while (usTempConfigLength < usConfigInfoLengthIn)
    {
        /* 获取当前配置信息ID */
        ucConfigInfoID = pucConfigInfoIn[usTempConfigLength++];

        /* 获取配置信息最小值以及最大值*/
        if (ucConfigInfoID == ucConfigInfoIDIn)
        {
            memcpy(puiMinValueOut, (pucConfigInfoIn+usTempConfigLength), CONFIG_INFO_VALUE_LENGTH);
            memcpy(puiMaxValueOut, (pucConfigInfoIn+usTempConfigLength+CONFIG_INFO_VALUE_LENGTH), CONFIG_INFO_VALUE_LENGTH);
            break;
        }

        /* 偏移到下一个基本信息*/
        usTempConfigLength = usTempConfigLength + CONFIG_INFO_VALUE_LENGTH*3;
        usTempConfigLength = usTempConfigLength + pucConfigInfoIn[usTempConfigLength] + NAME_LENGTH_BIT_SPACE_LENGTH;
    }

    if (usTempConfigLength >= usConfigInfoLengthIn)
    {
        return E_IO_ERROR_CONFIG_INFO_ID_NOT_EXIST;
    }

    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;
}
    
/******************************************************************
*函数名称:SetConfigValue
*功能描述:根据配置信息ID 以及值进行设置
*输入参数:ucConfigIDIn 配置信息ID
                        uiConfigValueIn 配置信息值
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/29                              we004
******************************************************************/
static void SetConfigValue(uint8_t ucConfigIDIn, uint32_t uiConfigValueIn)
{
    uint32_t mode = uiConfigValueIn;
    
    if((uint8_t)uiConfigValueIn==0||(uint8_t)uiConfigValueIn==1||(uint8_t)uiConfigValueIn==2)
    {
      mode = AD5755_R_0_12_V;
    }else if ((uint8_t)uiConfigValueIn==3||(uint8_t)uiConfigValueIn==4)
    {
      mode = AD5755_R_M12_P12_V;
    }else if ((uint8_t)uiConfigValueIn==5||(uint8_t)uiConfigValueIn==6)
    {
      mode = AD5755_R_0_6_V;
    }else if ((uint8_t)uiConfigValueIn==7||(uint8_t)uiConfigValueIn==8)
    {
      mode = AD5755_R_M6_P6_V;
    }else if ((uint8_t)uiConfigValueIn==9||(uint8_t)uiConfigValueIn==10)
    {
      mode = AD5755_R_0_24_MA;
    }else{
      mode = AD5755_R_0_24_MA;
    }
    
    printf("channel:%d,AISampleType:%d,mode:%d\n",ucConfigIDIn,(uint8_t)uiConfigValueIn,mode);
    
  
    switch (ucConfigIDIn)
    {
   
    case E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_1:
    
       AISampleType[E_CHANNEL_TYPE_CHANNEL_1]= (uint8_t)uiConfigValueIn;
        break;

    case E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_2:
  
      AISampleType[E_CHANNEL_TYPE_CHANNEL_2]= (uint8_t)uiConfigValueIn;
       break;

    case E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_3:
     
      AISampleType[E_CHANNEL_TYPE_CHANNEL_3]= (uint8_t)uiConfigValueIn;
       break;

    case E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_4:
    
      AISampleType[E_CHANNEL_TYPE_CHANNEL_4]= (uint8_t)uiConfigValueIn;    
       break;
       
    case E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_5:
    
       AISampleType[E_CHANNEL_TYPE_CHANNEL_5]= (uint8_t)uiConfigValueIn;
        break;

    case E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_6:
  
      AISampleType[E_CHANNEL_TYPE_CHANNEL_6]= (uint8_t)uiConfigValueIn;
       break;

    case E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_7:
     
      AISampleType[E_CHANNEL_TYPE_CHANNEL_7]= (uint8_t)uiConfigValueIn;
       break;

    case E_AI_DEVICE_CONFIG_INFO_ID_MODE_CONFIG_CHANNEL_8:
    
      AISampleType[E_CHANNEL_TYPE_CHANNEL_8]= (uint8_t)uiConfigValueIn;    
       break;

    default:
    {
    }
    }   
    
    if(ucConfigIDIn <= 4){
            
              AD5755_SetRegisterValue(AD5755_DREG_WR_DAC, ucConfigIDIn-1, 0x0);
              // 配置通道参数（内部参考、DC-DC 模式、输出范围等）
              AD5755_SetControlRegisters(AD5755_CREG_DAC,
                                         ucConfigIDIn-1,
                                         AD5575_st.rsetBits[ucConfigIDIn-1] |       // 输出电阻设置
                                         AD5575_st.ovrngBits[ucConfigIDIn-1] |      // 过载范围设置
                                         AD5755_DAC_INT_ENABLE |               // 启用内部参考
                                         AD5755_DAC_NOUTEN |
                                         AD5755_DAC_DC_DC |                    // 启用 DC-DC 转换器
                                         AD5755_DAC_R(mode));      // 默认设置为 4-20mA 范围

              // 设置初始输出值（0x1000 = 约 8mA，对应 4-20mA 范围的中点）
              AD5755_SetRegisterValue(AD5755_DREG_WR_DAC, ucConfigIDIn-1, 0x0);
             
                              
             //  LDAC_LOW() ; 	
              // delay_us(10);
             // LDAC_HIGH() ;
               if(uiConfigValueIn == 0)
               {
              AD5755_SetControlRegisters(AD5755_CREG_DAC,
                                         ucConfigIDIn-1,
                                         AD5755_DAC_INT_ENABLE |   // 启用内部参考
                                         AD5755_DAC_NOUTEN |        // 启用输出
                                         AD5755_DAC_DC_DC |        // 启用 DC-DC 转换器
                                         AD5755_DAC_R(mode)); // 输出范围为 4-20mA
               }else{
               AD5755_SetControlRegisters(AD5755_CREG_DAC,
                                         ucConfigIDIn-1,
                                         AD5755_DAC_INT_ENABLE |   // 启用内部参考
                                         AD5755_DAC_OUTEN |        // 启用输出
                                         AD5755_DAC_DC_DC |        // 启用 DC-DC 转换器
                                         AD5755_DAC_R(mode)); // 输出范围为 4-20mA
               }
      
    }else{
              AD5755_SetRegisterValue2(AD5755_DREG_WR_DAC, ucConfigIDIn-5, 0x0);
              // 配置通道参数（内部参考、DC-DC 模式、输出范围等）
              AD5755_SetControlRegisters2(AD5755_CREG_DAC,
                                         ucConfigIDIn-5,
                                         AD5575_st.rsetBits[ucConfigIDIn-5] |       // 输出电阻设置
                                         AD5575_st.ovrngBits[ucConfigIDIn-5] |      // 过载范围设置
                                         AD5755_DAC_INT_ENABLE |               // 启用内部参考
                                         AD5755_DAC_NOUTEN |
                                         AD5755_DAC_DC_DC |                    // 启用 DC-DC 转换器
                                         AD5755_DAC_R(mode));      // 默认设置为 4-20mA 范围

              // 设置初始输出值（0x1000 = 约 8mA，对应 4-20mA 范围的中点）
              AD5755_SetRegisterValue2(AD5755_DREG_WR_DAC, ucConfigIDIn-5, 0x0);
             
                              
             //  LDAC_LOW() ; 	
              // delay_us(10);
             // LDAC_HIGH() ;
               if(uiConfigValueIn == 0)
               {
              AD5755_SetControlRegisters2(AD5755_CREG_DAC,
                                         ucConfigIDIn-5,
                                         AD5755_DAC_INT_ENABLE |   // 启用内部参考
                                         AD5755_DAC_NOUTEN |        // 启用输出
                                         AD5755_DAC_DC_DC |        // 启用 DC-DC 转换器
                                         AD5755_DAC_R(mode)); // 输出范围为 4-20mA
               }else{
               AD5755_SetControlRegisters2(AD5755_CREG_DAC,
                                         ucConfigIDIn-5,
                                         AD5755_DAC_INT_ENABLE |   // 启用内部参考
                                         AD5755_DAC_OUTEN |        // 启用输出
                                         AD5755_DAC_DC_DC |        // 启用 DC-DC 转换器
                                         AD5755_DAC_R(mode)); // 输出范围为 4-20mA
               }
    }
  
    return;
}

/******************************************************************
*函数名称:GetIoDataErrorValue
*功能描述:根据数据类型获取错误的返回值
*输入参数:eDiagnoseTypeIn 数据类型
*输出参数:无
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/29                              we004
******************************************************************/
static IO_InterfaceResult_e GetIoDataErrorValue(IoDeviceInfoType_e eDiagnoseTypeIn)
{
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT;


    switch (eDiagnoseTypeIn)
    {
    case E_IO_DEVICE_INFO_TYPE_DOWN_DIGTITAL:
        eInterfaceResult = E_IO_ERROR_DIAGNOSIS_IO_DOWN_DIGITAL_INFO;
        break;
    
    case E_IO_DEVICE_INFO_TYPE_DOWN_ANALOG:
        eInterfaceResult = E_IO_ERROR_DIAGNOSIS_IO_DOWN_ANALOG_INFO;
        break;
        
    case E_IO_DEVICE_INFO_TYPE_UP_DIGTITAL:
        eInterfaceResult = E_IO_ERROR_DIAGNOSIS_IO_UP_DIGITAL_INFO;
        break;

    case E_IO_DEVICE_INFO_TYPE_UP_ANALOG:
        eInterfaceResult = E_IO_ERROR_DIAGNOSIS_IO_UP_ANALOG_INFO;
        break;
    
    case E_IO_DEVICE_INFO_TYPE_WARN:
        eInterfaceResult = E_IO_ERROR_DIAGNOSIS_IO_WARN_INFO;
        break;
        
    case E_IO_DEVICE_INFO_TYPE_DIAGNOSIS:
        eInterfaceResult = E_IO_ERROR_DIAGNOSIS_IO_DIAGNOSIS_INFO;
        break;
    default:
        break;
    }

    return eInterfaceResult;
}

/******************************************************************
*函数名称:GetDeviceDataLength
*功能描述:根据配置信息中的数据ID，获取设备描述文件中该ID对应的数据长度
*输入参数:ucConfigDataIDIn 配置信息中的数据ID
                        usDeviceDataInfoLengthIn 设备描述文件中的数据长度
                        pucDeviceDataInfoIn设备描述文件中的数据内容
*输出参数:pucDeviceDataLengthOut 设备描述文件中该ID对应的数据长度
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/29                              we004
******************************************************************/
static IO_InterfaceResult_e GetDeviceDataLength(uint8_t ucConfigDataIDIn, uint16_t usDeviceDataInfoLengthIn, uint8_t *pucDeviceDataInfoIn,
                                                                    uint8_t *pucDeviceDataLengthOut)
{
    uint16_t usTempDataLength = 0; /* 临时循环变量*/
    uint8_t ucDataInfoID = 0;


    FUNC_ENTER();
    
    /* 入参检查*/
    NULL_CHECK(pucDeviceDataInfoIn);
    NULL_CHECK(pucDeviceDataLengthOut);

    usTempDataLength = 0;
    while (usTempDataLength < usDeviceDataInfoLengthIn)
    {
        /* 获取当前配置信息ID */
        ucDataInfoID = pucDeviceDataInfoIn[usTempDataLength];

        /* 获取配置信息最小值以及最大值*/
        if (ucDataInfoID == ucConfigDataIDIn)
        {
            (*pucDeviceDataLengthOut) = pucDeviceDataInfoIn[usTempDataLength+DATA_ID_SPACE_LENGTH];
            break;
        }

        /* 偏移到下一个基本信息*/
        usTempDataLength = usTempDataLength + ONE_DATA_LENGTH_IN_DEVICE_INFO;
        usTempDataLength = usTempDataLength + pucDeviceDataInfoIn[usTempDataLength] + NAME_LENGTH_BIT_SPACE_LENGTH;
    }

    if (usTempDataLength >= usDeviceDataInfoLengthIn)
    {
        return E_IO_ERROR_DATA_INFO_ID_NOT_EXIST;
    }

    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:ReadFlashIoDeviceInfo
*功能描述:从FLASH中读取从站设备描述文件的部分信息
*输入参数:usIoDeviceInfoLengthIn保存设备描述文件缓冲区长度
                        eReadFlagIn 设备描述文件读取标志
*输出参数:pucIoDeviceInfoOut保存设备描述文件的缓冲区
                        pusReadLengthOut 读取的设备描述文件长度 
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/28                              we004
******************************************************************/
static IO_InterfaceResult_e ReadFlashIoDeviceInfo(uint16_t usIoDeviceInfoLengthIn, IoDeviceInfoType_e eReadFlagIn, 
                                                        uint8_t *pucIoDeviceInfoOut, uint16_t *pusReadLengthOut)
{
    uint32_t uiReadStartAddr = FLASH_DEVICE_INFO_START_ADDR; /* 读取开始位置*/
    uint16_t usReadLength = 0; /* 读取长度*/
    uint16_t usCurLength = 0; /* 当前偏移长度*/
    uint8_t ucTypeSequence = 0; /* 读取的信息类型在设备描述文件中的顺序*/
    uint8_t i = 0; /* 循环变量*/
    

    FUNC_ENTER();
    
    /* 入参判断*/
    NULL_CHECK(pucIoDeviceInfoOut);
    NULL_CHECK(pusReadLengthOut);
    if (eReadFlagIn >= E_IO_DEVICE_INFO_TYPE_BUTT)
    {
        return E_IO_ERROR_READ_FLASH_INVALID_FLAG;
    }

    /* 获取需要读取的长度以及开始地址*/
    uiReadStartAddr = uiReadStartAddr + FLASH_DEVICE_INFO_LENGTH_COUNT;
    usCurLength = 0;
    ucTypeSequence = (uint8_t)(eReadFlagIn + 1);
    for (i=0; i<ucTypeSequence; i++)
    {
        /* 获取当前信息的起始地址以及长度*/
        uiReadStartAddr = uiReadStartAddr + usCurLength;
        memcpy(&usReadLength, (uint8_t *)uiReadStartAddr, FLASH_DEVICE_INFO_LENGTH_COUNT);

        /* 计算偏移到下一个信息的偏移长度*/
        usCurLength = usReadLength + FLASH_DEVICE_INFO_LENGTH_COUNT;
    }
    
    /* 判断缓冲区长度是否合法*/
    if (usReadLength == FLASH_LENGTH_BLANK_FLAG)
    {
        return E_IO_ERROR_IO_DEVICE_FLASH_BLANK;
    }

    /* 判断缓冲区长度是否合法*/
    if (usReadLength > usIoDeviceInfoLengthIn)
    {
        return E_IO_ERROR_IO_DEVICE_BUFFER_TOO_SHORT;        
    }

    /*复制到缓冲区中*/
    memcpy(pucIoDeviceInfoOut, (void *)(uiReadStartAddr+FLASH_DEVICE_INFO_LENGTH_COUNT), usReadLength);
    (*pusReadLengthOut) = usReadLength;

    FUNC_EXIT();
    
    return E_IO_ERROR_NONE;
}


/******************************************************************
*函数名称:SetConfigDefaultValue
*功能描述:初始化时设置配置信息默认值
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/12/5                              weiguangyu we031
******************************************************************/
void SetConfigDefaultValue(void)
{
       AISampleType[E_CHANNEL_TYPE_CHANNEL_1]= 1;
       AISampleType[E_CHANNEL_TYPE_CHANNEL_2]= 1;
       AISampleType[E_CHANNEL_TYPE_CHANNEL_3]= 1;
       AISampleType[E_CHANNEL_TYPE_CHANNEL_4]= 1;
       AISampleType[E_CHANNEL_TYPE_CHANNEL_5]= 1;
       AISampleType[E_CHANNEL_TYPE_CHANNEL_6]= 1;
       AISampleType[E_CHANNEL_TYPE_CHANNEL_7]= 1;
       AISampleType[E_CHANNEL_TYPE_CHANNEL_8]= 1;
  
  return;
}
