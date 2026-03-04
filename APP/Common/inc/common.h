/******************************************************************
*
*文件名称:  common.h
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

#ifndef ____COMMON_H__
#define ____COMMON_H__
/******************************************************************
*                             头文件                              *
******************************************************************/
#include <stdint.h> 
#include <string.h>
#include "mbport.h"
#include "IOOptions.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/*定义从站与网关交互波特率*/
#ifndef SLAVE_STATION_DEFAULT_COMMCATION_BAUD
#define SLAVE_STATION_DEFAULT_COMMCATION_BAUD     (6000000)  //6000000
#endif

#define MIN_STATION_NUMBER  (1)   /* 最小从站站号为1 */
#define MAX_STATION_NUMBER  (247) /* 最大从站站号为247 ，Modbus 485(RTU)的站号0用于广播看， 248-255保留 */

/* BEGIN: Added by   PN:IO-288,重新定义所有中断优先级 */
/* 中断优先级定义 */
#define USART_RECEIVE_USART1_PRI           (1)     /* 串口接收DMA中断优先级 */
#define TEMPERATURE_WARN_ADC1_PRI          (3)     /* 温度告警使用的ADC1中断优先级 */
#define DI_VOLTAGE_WARN_EXTI_2_3_PRI     (3)     /* DI公共端电压异常告警使用的外部2-3中断优先级 */
#define DO_DRIVER_CHIP_EXCEPT_EXTI_14_15_PRI   (3)  /* DO驱动芯片异常告警使用的外部14-15中断优先级 */
#define DI_SAMPLING_TIME2_PRI               (2)     /* DI采集定时器中断优先级 */
#define TIME3_PRI                            (3)     /* 与网关连接断开定时器中断优先级 */
#define TIME15_PRI                       (3)        /*AIAO指示灯定时器中断优先级*/
#define DI_SAMPLING_SPI_PRI                 (0)     /* DI采集SPI中断优先级 */
#define SPI_AI_DMAIRQSPriority          (0)       /* AI SPI驱动采集使用DMA中断优先级 */
/* END:   */

/*函数入参判断宏定义*/
#define  NULL_CHECK(parameter)  if (NULL == parameter) \
                                   { \
                                        return E_IO_ERROR_NULL_PARAMETER; \
                                    }
/*
#define  NULL_CHECK(parameter)  if (NULL == parameter) \
                                   { \
                                        return E_IO_ERROR_NULL_PARAMETER; \
                                    }
*/

/*函数进出日志打印宏定义*/
#ifndef FUNC_ENTER
#define FUNC_ENTER( )   //IOLogOutput(E_IO_LOG_LEVEL_5_INFO, "%s %s %d  Enter\n", __FILE__, __FUNCTION__, __LINE__)
#endif
#ifndef FUNC_EXIT
#define FUNC_EXIT( )   //IOLogOutput(E_IO_LOG_LEVEL_5_INFO, "%s %s %d  Exit\n", __FILE__, __FUNCTION__, __LINE__)
#endif

/* 求数组大小 */
#define SIZE_OF_ARRAY(x)    (sizeof(x)/sizeof((x)[0]))

#define BIT_NUM_PER_BYTE    (8)     /* 每字节比特数 */
#define BIT_NUM_PER_BYTE_OFF   (3)     /* 数字8bit偏移量 */
#define NUMBER_ZERO (0) /* 数字0 */
#define DIGITAL_DATA_8_BIT        (8)     /* 每个数字量占1个bit，一个字节8个bit */
#define DIVIDE_NUM_EIGTH  (3)  /* 右移3位相当于除以8 */

/* 置位清零宏 */
#define INT_BIT       (8)
#define BITMASK(y)   (1<<((y)%INT_BIT))
#define SET_BITY(x,y) ((x)|= (1<<(y)))                                  /*置y位为1*/
#define CLE_BIT(x,y) ((x)&=(~(BITMASK(y))))                           /*清零y位*/

/* 获取字符X的比特位Y的值*/
#define IO_GET_BIT_VALUE(x,y) (((x)& (1<<(y)))>>(y))

#define DO_SAMPLE_DATA_HIGH_LEVEL  (1) /* 高电平 */
#define DO_SAMPLE_DATA_LOW_LEVEL   (0) /* 低电平 */

#define GET_DO_DATA__BIT_0   (0x01)
#define GET_DO_DATA__BIT_1   (0x02)
#define GET_DO_DATA__BIT_2   (0x04)
#define GET_DO_DATA__BIT_3   (0x08)
#define GET_DO_DATA__BIT_4   (0x10)
#define GET_DO_DATA__BIT_5   (0x20)
#define GET_DO_DATA__BIT_6   (0x40)
#define GET_DO_DATA__BIT_7   (0x80)

#define GET_FR1118_DATA__BIT_0   (0x04)  /*PB2*/
#define GET_FR1118_DATA__BIT_1   (0x02)  /*PB1*/
#define GET_FR1118_DATA__BIT_2   (0x01)  /*PB0*/
#define GET_FR1118_DATA__BIT_3   (0x400)  /*PB10*/
#define GET_FR1118_DATA__BIT_4   (0x20)  /*PB5*/
#define GET_FR1118_DATA__BIT_5   (0x08)  /*PB3*/
#define GET_FR1118_DATA__BIT_6   (0x10)/*PB4*/
#define GET_FR1118_DATA__BIT_7   (0x40)/*PB6*/

/* 打开日志开关，打印到FLAH ，系统异常时打印CSTACK到FLASH */
//#define TEST_DEBUG_LOG        /* 目前关闭 */

/* 日志级别 */
#define DEBUG_LEVEL E_IO_LOG_LEVEL_0_NONE

/* IO模块状态机 */
typedef enum
{
    IO_STATE_NOT_INIT = 0,
    IO_STATE_INITIALIZED,
    IO_STATE_BUTT
}IO_STATE_e;

/******************************************************************
*                           数据类型                              *
******************************************************************/

/* 设备描述文件类型 */
typedef enum
{
    E_IO_DEVICE_INFO_TYPE_BASE = 0, /* 基本信息 */
    E_IO_DEVICE_INFO_TYPE_CONFIG,   /* 配置信息 */
    E_IO_DEVICE_INFO_TYPE_DOWN_DIGTITAL,  /* 下行数字量 */
    E_IO_DEVICE_INFO_TYPE_DOWN_ANALOG,  /* 下行模拟量 */
    E_IO_DEVICE_INFO_TYPE_UP_DIGTITAL,  /* 上行数字量 */
    E_IO_DEVICE_INFO_TYPE_UP_ANALOG,  /* 上行模拟量 */
    E_IO_DEVICE_INFO_TYPE_DIAGNOSIS,    /*诊断信息 */
    E_IO_DEVICE_INFO_TYPE_WARN,     /* 告警信息 */
    E_IO_DEVICE_INFO_TYPE_PRODUCT,
    E_IO_DEVICE_INFO_TYPE_BUTT
}IoDeviceInfoType_e;

/* 各个模块接口返回值定义 */
typedef enum
{
    /* 公用部分 */
    E_IO_ERROR_NONE = 0,    /* 无错误，正常返回 */
    E_IO_ERROR_NULL_PARAMETER,  /* 接口的一个或多个入参为空 */
    /* 初始化部分  */
    E_IO_ERROR_IO_USART = 100,    /* 初始化串口驱动失败 */
    E_IO_ERROR_IO_FLASH,    /* 初始化FLASH驱动失败 */
    E_IO_ERROR_NOT_START_INIT,    /* 没有开始初始化，初始化开始指示为假 */
    E_IO_ERROR_SET_STATION_NUMBER,  /* 设置站号失败 */
    E_IO_ERROR_START_IAP,       /* 启动IAP失败 */
    E_IO_ERROR_SET_STATION_NUMBER_REQUEST,    /* 从站分配的请求数据错误 */ 
    E_IO_ERROR_M0DBUS485_INIT,  /* 初始化modbus485协议栈失败 */
    E_IO_ERROR_DI_SPI_DRIVER_CONFIG, /* 配置DI SPI驱动失败 */
    E_IO_ERROR_DI_SPI_DRIVER_INIT, /* 初始化DI SPI驱动失败 */
    E_IO_ERROR_AI_SPI_DRIVER_CONFIG, /* 配置AI SPI驱动失败 */
    E_IO_ERROR_AI_SPI_DRIVER_INIT, /* 初始化AI SPI驱动失败 */
    E_IO_ERROR_AO_SPI_DRIVER_CONFIG, /* 配置AO SPI驱动失败 */
    E_IO_ERROR_AO_SPI_DRIVER_INIT, /* 初始化AO SPI驱动失败 */
    E_IO_ERROR_REGIST_COIL_CB,  /* 注册线圈回调函数失败 */
    E_IO_ERROR_REGIST_HOLDING_REGISTER_CB,  /* 注册保持寄存器回调函数失败 */
    E_IO_ERROR_REGIST_READ_FILE_RECORD_CB,  /* 注册读文件记录回调函数失败 */
    E_IO_ERROR_REGIST_WRITE_FILE_RECORD_CB,  /* 注册写文件记录回调函数失败 */
    E_IO_ERROR_GW_DEVICE_ID, /* 网关设备ID错误 */
    E_IO_ERROR_GET_SLAVE_DEVICE_ID, /* 获取从站设备ID失败 */
    E_IO_ERROR_QUERY_START_INIT,/*没有开始初始化或这没有开始升级 */
    E_IO_ERROR_REQ_TYPE_ERROR,/*请求类型错误*/
    E_IO_ERROR_FRAME_LENGTH_ILLEGAL,/*帧长度非法*/
    E_IO_ERROR_DEVICE_CONTENT_COUNT,/*设备描述文件内容个数错误*/
    E_IO_ERROR_ALLOC_DATA_FIELD,       /* 分配数据区错误 */
    E_IO_ERROR_CRC16,                   /*CRC16校验错误*/

    /* 设备描述文件读取 */
    E_IO_ERROR_IO_DEVICE_FLASH_BLANK, /* 存储IO设置描述文件缓冲区为空*/
    E_IO_ERROR_IO_DEVICE_BUFFER_TOO_SHORT, /* IO设备描述文件缓冲区过短 */
    E_IO_ERROR_READ_FLASH_IO_DEVICE_INFO, /* 从FLASH中，读取IO设备描述文件失败 */
    E_IO_ERROR_READ_FLASH_INVALID_FLAG, /* 读取标志错误 */
    E_IO_ERROR_SET_VARIOUS_START_ADDR,  /* 设置寄存器地址错误 */
    E_IO_ERROR_VENDOR_ID_TOO_LONG,  /* 厂商ID过长 */
    E_IO_ERROR_PRODUCT_ID_TOO_LONG, /* 产品ID过长 */
    E_IO_ERROR_PRODUCT_CONTENT_LENGTH, /* 产品信息长度错误*/
    E_IO_ERROR_ONE_PRODUCT_CONTENT_TOO_LONG, /* 产品信息内容过长*/
    
    /* IO诊断配置 */
    E_IO_ERROR_DIAGNOSIS_IO_BASE_INFO, /* IO基本信息非法 */
    E_IO_ERROR_BASE_INFO_CONTENT_TOO_LONG, /* 某个基本信息的内容过长 */
    E_IO_ERROR_BASE_INFO_ID_NOT_EXIST, /* 某个基本信息的ID不存在*/
    E_IO_ERROR_CONFIG_INFO_ID_NOT_EXIST, /* 某个配置信息的ID不存在*/
    E_IO_ERROR_DATA_INFO_ID_NOT_EXIST, /* 某个数据信息的ID不存在*/
    E_IO_ERROR_CONFIG_INFO_VALUE_INVALID, /* 某个配置信息的值非法*/
    E_IO_ERROR_DATA_TYPE_INVALID, /* 数据类型非法*/
    E_IO_ERROR_DIAGNOSIS_IO_UP_DIGITAL_INFO,    /* IO上行数字量非法 */
    E_IO_ERROR_DIAGNOSIS_IO_UP_ANALOG_INFO,     /* IO上行模拟量非法 */
    E_IO_ERROR_DIAGNOSIS_IO_DOWN_DIGITAL_INFO,  /* IO下行数字量非法 */
    E_IO_ERROR_DIAGNOSIS_IO_DOWN_ANALOG_INFO,   /* IO下行模拟量非法 */
    E_IO_ERROR_DIAGNOSIS_IO_CONFIG_INFO, /* IO配置信息非法 */
    E_IO_ERROR_DIAGNOSIS_IO_WARN_INFO, /* IO告警信息非法 */
    E_IO_ERROR_DIAGNOSIS_IO_DIAGNOSIS_INFO, /* IO诊断信息非法 */
    /* 设备描述文件获取接口 */
    E_IO_ERROR_RECORD,  /* 文件记录号非法 */
    /* 输入输出控制 */
    E_IO_ERROR_GET_DI_DATA, /* 读取数字量失败 */
    E_IO_ERROR_SET_DI_DATA, /* 设置数字量失败 */
    E_IO_ERROR_GET_AI_DATA, /* 读取模拟量失败 */
    E_IO_ERROR_SET_AI_DATA, /* 设置模拟量失败 */

    /*ADC1 初始化*/
    E_IO_ERROR_ADC1_CONFIG, /*芯片内部温度告警配置失败*/
    E_IO_ERROR_ADC1_CONVERT_VALUE, /*ADC1 采集温度传感器值非法*/
 
    E_IO_ERROR_NOT_BUTT
}IO_InterfaceResult_e;

/* 从站日志级别定义 */
typedef enum
{
    E_IO_LOG_LEVEL_0_NONE = 0, /* 不输出日志 */
    E_IO_LOG_LEVEL_1_CRITICAL, /* 严重错误日志：程序出现严重错误 */
    E_IO_LOG_LEVEL_2_ERROR, /* 错误日志：程序出错，无法运行时使用 */
    E_IO_LOG_LEVEL_3_WARN, /* 告警日志：程序出错，但还可以正常运行时使用 */
    E_IO_LOG_LEVEL_4_DEBUG, /* 调试日志：解决问题时使用 */
    E_IO_LOG_LEVEL_5_INFO, /* 运行日志：网关的所有运行信息，当DEBUG无法满足要求时使用 */
    E_IO_LOG_LEVEL_BUTT
} IOLogLevel_e;

/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
void SystemTestFaildhandle(void);
#endif

