/*******************************************************************
*文件名称:  MasterCommunication.h
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

#ifndef __MASTER_COMMUNICATION_H__
#define __MASTER_COMMUNICATION_H__


/******************************************************************
*                             头文件                              *
******************************************************************/

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/* 存放设备描述文件部分信息缓冲区的最大长度*/
#define MAX_PART_DEVICE_INFO_LENGTH (256)
/* BEGIN: Added by we004, 2017/12/17   PN:从站诊断、配置时，从站处理时间过长，导致Modbus 485通信超时 */
/* 存放设备描述文件所有信息缓冲区的最大长度*/
#define MAX_WHOLE_DEVICE_INFO_LENGTH (1024)
/* END:   Added by we004, 2017/12/17 */
/*  存放版本信息的缓冲区长度 */
#define PRODUCT_INFO_LENGTH (20)

/* 设备描述文件最大长度 */
#define MAX_IO_DEVICE_INFO_LENGTH (4096)

/* 版本信息最大长度 */
#define MAX_IO_PRODUCT_INFO_LENGTH  (1024)

/* 升级程序最大长度 */
#define MAX_IAP_PROGRAM_LENGTH  (8192 - 256)

/*升级信息最大长度 */
#define MAX_IO_UPGRADE_INFO_LENGTH  (256)


/* 设备描述文件中，各部分长度所占的字节数*/
#define FLASH_DEVICE_INFO_LENGTH_COUNT (2)

/* 产品信息总长度所占的字节数 */
#define FLASH_PRODUCT_INFO_TOTAL_LENGTH_COUNT   (2)

/* 设备ID长度 */
#define DEVICE_ID_LENGTH    (16)


/* 通道类型定义*/
typedef enum
{
    E_CHANNEL_TYPE_CHANNEL_1 = 0, /* 通道1*/
    E_CHANNEL_TYPE_CHANNEL_2, /* 通道2*/
    E_CHANNEL_TYPE_CHANNEL_3, /* 通道3*/
    E_CHANNEL_TYPE_CHANNEL_4, /* 通道4*/
    E_CHANNEL_TYPE_CHANNEL_5, /* 通道5*/
    E_CHANNEL_TYPE_CHANNEL_6, /* 通道6*/
    E_CHANNEL_TYPE_CHANNEL_7, /* 通道7*/
    E_CHANNEL_TYPE_CHANNEL_8, /* 通道8*/
    E_CHANNEL_TYPE_CHANNEL_BUTT
} ChannelType_e;

/*配置信息默认值定义*/
#define FILTER_ALGORITHM_SELECT_DEFAULT_VALUE   (0)     /*滤波算法选择默认值*/
#define FILTER_WIDTH_DEFAULT_VALUE   (3)     /*滤波宽度默认值*/
#define DEBOUNCE_ACCURACY_DEFAULT_VALUE   (90)     /*去抖动精度 默认值*/
#define DEBOUNCE_TIME_DEFAULT_VALUE   (3)     /*去抖动时间默认值*/

#define CH1_CTRL_GPIO_PIN             (GPIO_Pin_12)
#define CH2_CTRL_GPIO_PIN             (GPIO_Pin_13)
#define CH3_CTRL_GPIO_PIN             (GPIO_Pin_14)
#define CH4_CTRL_GPIO_PIN             (GPIO_Pin_15)

#define CH1_CTRL_VOL		GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define CH1_CTRL_CUT	        GPIO_ResetBits(GPIOB, GPIO_Pin_12)                                                                        
#define CH2_CTRL_VOL		GPIO_SetBits(GPIOB, GPIO_Pin_13)
#define CH2_CTRL_CUT	        GPIO_ResetBits(GPIOB, GPIO_Pin_13)
#define CH3_CTRL_VOL		GPIO_SetBits(GPIOB, GPIO_Pin_14)
#define CH3_CTRL_CUT	        GPIO_ResetBits(GPIOB, GPIO_Pin_14)
#define CH4_CTRL_VOL		GPIO_SetBits(GPIOB, GPIO_Pin_15)
#define CH4_CTRL_CUT	        GPIO_ResetBits(GPIOB, GPIO_Pin_15)


/******************************************************************
*                           数据类型                              *
******************************************************************/
/* BEGIN: Added by we004, 2017/12/17   PN:从站诊断、配置时，从站处理时间过长，导致Modbus 485通信超时 */
/* 设备描述文件各部分内容信息*/
typedef struct
{
    uint16_t usContentLength; /* 内容长度*/
    uint8_t ucReserve[2]; /* 保留*/
    uint8_t *pucContentInfo; /* 内容信息*/
} IODeviceContentInfo_t;
/* END:   Added by we004, 2017/12/17 */    

/******************************************************************
*                           全局变量声明                          *
******************************************************************/
extern uint8_t gucaIOPartDeviceInfo[MAX_PART_DEVICE_INFO_LENGTH];
extern IODeviceContentInfo_t taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_BUTT];
/******************************************************************
*                         全局函数声明                            *
******************************************************************/
IO_InterfaceResult_e GetIoDeviceContentInfo(void);
IO_InterfaceResult_e DiagnoseIoDeviceBaseInfo(uint8_t *pucConfigBaseInfoIn, uint16_t usConfigBaseLengthIn);
IO_InterfaceResult_e ConfigSlaveStation(uint8_t *pucConfigConfigInfoIn, uint16_t ucConfigConfigLengthIn);
IO_InterfaceResult_e DiagnoseIoDataInfo(uint8_t *pucIoConfigFileInfoIn, uint8_t IoConfigFileInfoLengthIn, IoDeviceInfoType_e eDiagnoseTypeIn);
IO_InterfaceResult_e ReadFlashProductInfo(uint16_t usIoDeviceInfoLengthIn, 
                                uint8_t *pucIoDeviceInfoOut, uint16_t *pusReadLengthOut);
IO_InterfaceResult_e AssemblyIoBaseInfo(uint8_t *pucIoDeviceBaseInfoIn, uint16_t usIoDeviceBaseInfoLengthIn,
        uint16_t usIoDeviceInfoLengthIn,
        uint8_t *pucIoVersionInfoIn, uint16_t usIoVersionInfoLengthIn,
        uint16_t *pusBaseInfoLengthOut);
#endif


