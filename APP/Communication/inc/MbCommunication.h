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

#ifndef __MB_COMMUNICATION_H__
#define __MB_COMMUNICATION_H__


/******************************************************************
*                             头文件                              *
******************************************************************/
#include "common.h"
#include "mbs.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/*Modbus函数入参判断宏定义*/
#define MODBUS_NULL_CHECK(parameter)  if (NULL == parameter) \
                                       { \
                                            return MB_PDU_EX_ILLEGAL_DATA_VALUE; \
                                        }

/* 一个Modbus帧里的元素占2个字节 */
#define MODBUS_ELEMENT_LENGTH   (2)

/* 断连超时时间 为1s*/
#define DISCONNECT_TIMEOUT   (2)

/*AIAO指示灯间隔时间*/
#define AIAO_INDICTOR_TIMEOUT  (1)

/*启用定时器标志 */
#define TIMER_ON    (1)

/*写文件子请求个数*/
#define MBM_READ_FILE_SUB_REQ    (1)
/* 写文件记录文件号 */
#define MBM_READ_FILE_NUM   (0)

/* 读文件记录子请求个数 */
#define MBM_WRITE_FILE_SUB_REQ  (1)
/* 读文件记录文件号 */
#define MBM_WRITE_FILE_NUM  (0)

/*从站信息文件号记录号定义*/
#define SLAVE_INFO_FILE_NUM     ( 0 )
#define SLAVE_BASE_INFO_RECORD_NUM     ( 0 )
#define SLAVE_CONFIG_INFO_RECORD_NUM   ( 1 )
#define SLAVE_DOWN_DIGITAL_INFO_RECORD_NUM   ( 2 )
#define SLAVE_DOWN_ANALOG_INFO_RECORD_NUM   ( 3 )
#define SLAVE_UP_DIGITAL_INFO_RECORD_NUM   ( 4 )
#define SLAVE_UP_ANALOG_INFO_RECORD_NUM   ( 5 )
#define SLAVE_WARN_DIGITAL_INFO_RECORD_NUM   ( 6 )
#define SLAVE_DIAGNOSE_ANALOG_INFO_RECORD_NUM   ( 7 )

/*读文件记录号定义*/
#define READ_SLAVE_INFO_FILE_NUM     ( 0 )     
/*基本信息记录号*/
#define READ_SLAVE_BASE_INFO_RECORD_NUM     ( 0 )           
/*配置信息记录号*/
#define READ_SLAVE_CONFIG_INFO_RECORD_NUM_START   ( 1 )     
#define READ_SLAVE_CONFIG_INFO_RECORD_NUM_END     ( 10 )
/*下行数字量信息记录号*/
#define READ_SLAVE_DOWN_DIGITAL_INFO_RECORD_NUM_START   ( 11 ) 
#define READ_SLAVE_DOWN_DIGITAL_INFO_RECORD_NUM_END   ( 20 )
/*下行模拟量信息记录号*/
#define READ_SLAVE_DOWN_ANALOG_INFO_RECORD_NUM_START   ( 21 )
#define READ_SLAVE_DOWN_ANALOG_INFO_RECORD_NUM_END   ( 30 )
/*上行数字量信息记录号*/
#define READ_SLAVE_UP_DIGITAL_INFO_RECORD_NUM_START   ( 31 ) 
#define READ_SLAVE_UP_DIGITAL_INFO_RECORD_NUM_END   ( 40 )
/*上行模拟量信息记录号*/
#define READ_SLAVE_UP_ANALOG_INFO_RECORD_NUM_START   ( 41 ) 
#define READ_SLAVE_UP_ANALOG_INFO_RECORD_NUM_END   ( 50 )
/*告警信息记录号*/
#define READ_SLAVE_WARN_DIGITAL_INFO_RECORD_NUM_START   ( 51 )  
#define READ_SLAVE_WARN_DIGITAL_INFO_RECORD_NUM_END   ( 60 )
/*诊断信息记录号*/
#define READ_SLAVE_DIAGNOSE_ANALOG_INFO_RECORD_NUM_START   ( 61 ) 
#define READ_SLAVE_DIAGNOSE_ANALOG_INFO_RECORD_NUM_END   ( 70 )

/*每次读取文件大小*/
#define MAX_LENGTH_READ_FILE   (112)
/*从站信息文件号记录号传输次数编号最大值*/
#define SLAVE_INFO_RECORD_NUM_TRANSMIT_MAX_TIME_NUM   ( 10 )


/* 寄存器起始地址定义 */
#define VARIOUS_START_ADDR_NUM  (15)    /* 各类寄存器起始地址个数 */
#define SLAVE_DEVICE_INFO_LENGTH_ADDR   (0)     /* 从站设备描述文件的总长度 */
#define SLAVE_BASE_INFO_LENGTH_ADDR (1)     /* 从站基本信息长度 */
#define SLAVE_CONFIG_INFO_LENGTH_ADDR   (2)     /* 从站配置信息长度 */
#define DOWN_DIGITAL_DATA_LENGTH_ADDR (3)     /* 下行业务数据(数字量)的长度 */
#define DOWN_ANALOG_DATA_LENGTH_ADDR    (4)     /* 下行业务数据(模拟量)的长度 */
#define UP_DIGITAL_DATA_LENGTH_ADDR (5) /* 上行业务数据(数字量)的长度 */
#define UP_ANALOG_DATA_LENGTH_ADDR  (6) /* 上行业务数据 (模拟量)的长度 */
#define WARN_DIGITAL_INFO_LENGTH_ADDR    (7) /* 告警数据(数字量)的长度 */
#define DIAGNOSE_ANALOG_INFO_LENGTH_ADDR    (8) /* 诊断数据(模拟量)的长度 */
//#define DATA_REG_START_ADDR (9)     /* 业务数据，告警数据，诊断数据寄存器起始地址，按顺序排列，网关一次性读出，共6个字节 */
#define DOWN_DIGITAL_SERVICE_DATA_START_ADDR  (9) /* 下行数字量业务起始地址 */
#define DOWN_ANALOG_SERVICE_DATA_START_ADDR   (10)    /* 下行模拟量业务数据起始地址 */
#define UP_DIGITAL_SERVICE_DATA_START_ADDR    (11)    /* 上行数字量业务数据起始地址 */
#define UP_ANALOG_SERVICE_DATA_START_ADDR     (12)    /* 上行模拟量业务数据起始地址 */
#define WARN_DIGITAL_INFO_START_ADDR    (13)    /* 告警数据起始地址 */
#define DIAGNOSE_ANALOG_INFO_START_ADDR     (14)    /* 诊断数据起始地址 */

/* 产品信息起始地址定义 */
#define HARDWARE_VERSION_ADDR   (51)    /* 硬件版本号 */
#define SOFTWARE_VERSION_ADDR   (52)    /* 软件版本号 */
#define PRODUCT_BATCH_ADDR  (53)    /* 产品批次 */
#define PRODUCT_SEQUENCE_NUMBER_ADDR    (54)    /* 产品序列号 */

/* 指示内容写线圈起始地址*/
#define SOFT_RESET_COIL_START_ADDRESS   ( 0 )             /* 从站软复位指示 线圈起始地址*/
#define UPGRADE_COIL_START_ADDRESS      ( 1 )             /* 从站升级指示 线圈起始地址*/
#define NEXT_ALLOC_PREPARE_COIL_START_ADDRESS ( 2 )     /* 下一个从站站号分配准备指示 线圈起始地址*/
#define NEXT_ALLOC_FINISH_COIL_START_ADDRESS ( 3 )      /* 下一个从站站号分配结束指示 线圈起始地址*/

/* 数字业务数据，告警起始地址  */
#define UP_DIGITAL_DATA_START_ADDR  (501)   /* 上行数字量起始地址 */
#define DOWN_DIGITAL_DATA_START_ADDR    (1001)  /* 下行数字量起始地址*/
#define WARN_DIGITAL_START_ADDR (1501)  /* 数字量告警起始地址 */
#define MAX_COIL_ADDR   (2000)  /* 线圈最大起始地址 */
#define MAX_COIL_NUM    (8)   /* 最大线圈数量 */

/* 模拟业务数据,诊断起始地址 */
#define DOWN_ANALOG_DATA_START_ADDR (501)   /* 下行模拟量起始地址 */
#define UP_ANALOG_DATA_START_ADDR   (1001)  /* 上行模拟量起始地址 */
#define DIAGNOSE_ANALOG_START_ADDR  (1501)      /* 模拟量诊断起始地址 */
#define MAX_REGISTER_ADDR   (2000)  /* 寄存器最大起始地址 */
#define MAX_REGISTER_NUM    (200)   /* 最大寄存器个数 */


/* 产品信息和ID和长度所占的字节数 */
#define FLASH_PRODUCT_INFO_LENGTH_COUNT (1)

/* 产品信息内容所占的字节数，现在固定为2 */
#define FLASH_PRODUCT_INFO_CONTENT_LENGTH   (2)

/* 产品信息内容长度一字节*/
#define PRODUCT_CONTENT_INFO_ONE_BYTE (1)

/* 产品信息内容长度两字节*/
#define PRODUCT_CONTENT_INFO_TWO_BYTES (2)

/*  升级程序IAP在FLASH中的地址*/
#define GW_UPGRADE_AREA_START (0x0800E000)

/* 寄存器定义 */
typedef enum
{
    E_REGISTER_SLAVE_DEVICE_INFO_LENGTH_ADDR = 0, /* 设备描述文件总长度*/
    E_REGISTER_SLAVE_BASE_INFO_LENGTH_ADDR,      /* 从站基本信息长度 */
    E_REGISTER_SLAVE_CONFIG_INFO_LENGTH_ADDR,        /* 从站配置信息长度 */
    E_REGISTER_DOWN_DIGITAL_DATA_LENGTH_ADDR,      /* 下行业务数据(数字量)的长度 */
    E_REGISTER_DOWN_ANALOG_DATA_LENGTH_ADDR,         /* 下行业务数据(模拟量)的长度 */
    E_REGISTER_UP_DIGITAL_DATA_LENGTH_ADDR,  /* 上行业务数据(数字量)的长度 */
    E_REGISTER_UP_ANALOG_DATA_LENGTH_ADDR,   /* 上行业务数据 (模拟量)的长度 */
    E_REGISTER_WARN_DIGITAL_INFO_LENGTH_ADDR,     /* 告警数据(数字量)的长度 */
    E_REGISTER_DIAGNOSE_ANALOG_INFO_LENGTH_ADDR,     /* 诊断数据(模拟量)的长度 */
    E_REGISTER_DOWN_DIGITAL_SERVICE_DATA_START_ADDR,   /* 下行数字量业务起始地址 */
    E_REGISTER_DOWN_ANALOG_SERVICE_DATA_START_ADDR,       /* 下行模拟量业务数据起始地址 */
    E_REGISTER_UP_DIGITAL_SERVICE_DATA_START_ADDR,        /* 上行数字量业务数据起始地址 */
    E_REGISTER_UP_ANALOG_SERVICE_DATA_START_ADDR,         /* 上行模拟量业务数据起始地址 */
    E_REGISTER_WARN_DIGITAL_INFO_START_ADDR,        /* 告警数据起始地址 */
    E_REGISTER_DIAGNOSE_ANALOG_INFO_START_ADDR,         /* 诊断数据起始地址 */
    E_REGISTER_BUTT
}Register_Type_e;



/******************************************************************
*                           数据类型                              *
******************************************************************/


/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
eMBException HandleDigitalData(UBYTE *pubRegBuffer, USHORT usAddress, USHORT usNRegs, eMBSRegisterMode eRegMode);
eMBException HandleAnalogData(UBYTE *pubRegBuffer, USHORT usAddress, USHORT usNRegs, eMBSRegisterMode eRegMode);
eMBException HandleReadFileRecord(UBYTE *pubFileRecordBuffer, xMBSFileRecordReq_t *arxSubRequests);
eMBException HandleWriteFileRecord(UBYTE *pubFileRecordBuffer, xMBSFileRecordReq_t *arxSubRequests);
eMBException CalculateStartAddr(uint16_t *pusStartRegisterIn, uint8_t ucBufferSize,
    uint16_t ucRealStartAddr, USHORT usAddress, USHORT usNRegs, uint16_t **ppusStartAddrOut);
IO_InterfaceResult_e GetVariousStartAddr(void);
eMBException ReadFileRecordAndJudgeFileRecordType(xMBSFileRecordReq_t *arxSubRequests, uint8_t *pucRecordNumTypeOut,
    uint8_t *pucDataTransmitTimeNumOut);

#endif


