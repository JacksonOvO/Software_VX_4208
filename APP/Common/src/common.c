/******************************************************************
*
*文件名称:  common.c
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
#include <stdarg.h>
#include <stdio.h>
#include "stm32f30x.h"
#include "stm32f30x_flash.h"
#include "common.h"
#include "IOOptions.h"
#include "core_cmFunc.h"
#include "MasterCommunication.h"
#include "STM32F0usartopt.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/

#define MAX_LOG_BUFFER_LENGTH 512
#define MAX_LOG_LEVEL_STR_LENGTH 16

/* 每个扇区的页数 */
#define PAGE_NUMBER_PER_SECTOR  (4)

/* 每页的字节数 */
#define BYTE_NUMBER_PER_PAGE    (1024)

/* 日志打印扇区 */
#define FLASH_LOG_OUTPUT_BEGIN  (0x0800A000)
#define FLASH_LOG_OUTPUT_END  (0x0800AFFF)

/* CSTACK快照 */
#define FLASH_CSTACK_SNAP_OUTPUT_BEGIN  (0x0800B000)
#define FLASH_CSTACK_SNAP_OUTPUT_END  (0x0800BFFF)

#define DEBUG_FLASH_LOCKED      (0)     /* FLASH未解锁 */
#define DEBUG_FLASH_UNLOCKED    (1)     /* FLASH已解锁*/
#define DEBUG_FLASH_4_BYTE      (4)     /* 写入FLASH需要时4字节的整数倍 */
#define DEBUG_FLASH_16_BYTE     (16)    /* 写入FLASH时不足16字节补齐16字节，便于观察数据 */
#define DEBUG_FLASH_EACH_ROW_LENGTH (16)    /* 读取FLASH时每行字节数 */
#define DEBUG_FLASH_INTERVAL_NUMBER (4)     /* FLASH打印间隔行数 */
#define CSTACK_SIZE_4_BYTE_COUNT    (CSTACK_SIZE/DEBUG_FLASH_4_BYTE)    /* CSTACK按4字节来数的大小 */

/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                         全局变量定义                            *
******************************************************************/
/* 日志缓存*/
#ifdef TEST_DEBUG_LOG
static char cLogBuffer[MAX_LOG_BUFFER_LENGTH];
#endif

/* 主函数堆栈指针 */
unsigned long gulMainCstackPointer = 0;

/* 设备描述文件所有信息缓冲区*/
/* 当进入错误中断打印日志到FLASH时，使用此缓存保存临时数据 */
extern uint8_t gucaIoWholeDeviceInfo[MAX_WHOLE_DEVICE_INFO_LENGTH];


/* AO数据区 */
#if FR4XX4_CODE
extern int16_t gusaAnalogData[MAX_AO_CHANNEL_NUMBER];
#endif

#if STM32F0_UART1_DMA_ENABLE
/*定义UASRT1DMA发送缓存*/
extern uint8_t gucUSART1DMATx[DMA_SEND_BUFSIZE];   

/*定义USART1DMA接收缓存*/
extern uint8_t gucUSART1DMARx[DMA_RECEIVE_BUFSIZE];
#endif

#if STM32F0_UART2_DMA_ENABLE
/*定义UASRT1DMA发送缓存*/
extern uint8_t gucUSART2DMATx[DMA_SEND_BUFSIZE];   

/*定义USART1DMA接收缓存*/
extern uint8_t gucUSART2DMARx[DMA_RECEIVE_BUFSIZE];
#endif


/******************************************************************
*                          局部函数声明                            *
******************************************************************/
static uint32_t FLASH_Write(uint32_t uiFLASHAddr, uint8_t *pucWriteContent, uint16_t usWriteLength);

/******************************************************************
*                         全局函数实现                            *
******************************************************************/

/******************************************************************
*函数名称:SysTick_Handler
*功能描述:
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
******************************************************************/
void SysTick_Handler(void)
{
    return;
}

void HardFault_Handler(void)
{
 //   GPIO_WriteBit(GPIOA, GPIO_Pin_2, 1)
//    GPIOB->BSRR = GPIO_Pin_12;
    uint32_t uiFlashAddr = FLASH_DEVICE_INFO_START_ADDR; /* 设备描述文件起始地址*/
    uint32_t ulCstackTop = 0;  /* CSTACK栈底地址 */
    uint16_t usWriteLength = 0;     /* 写入FLASH的数据长度 */
    uint8_t i = 0;

    /* 保存堆栈指针 */
    ulCstackTop = __get_MSP();

    /* 保存设备描述文件到缓存 */
    memset(gucaIoWholeDeviceInfo, 0, MAX_WHOLE_DEVICE_INFO_LENGTH);
    memcpy(gucaIoWholeDeviceInfo, (uint8_t *)uiFlashAddr, MAX_WHOLE_DEVICE_INFO_LENGTH);

    /*关闭所有中断 */
    asm ("CPSID   I");

    /* 解锁FLASH */
    FLASH_Unlock();

    /* 擦除扇区 */
    for (i = 0; i < PAGE_NUMBER_PER_SECTOR; i++)
    {
        (void)FLASH_ErasePage(uiFlashAddr + i * BYTE_NUMBER_PER_PAGE);
    }

    /* 设备描述文件重新写入FLASH */
    uiFlashAddr = FLASH_Write(uiFlashAddr, gucaIoWholeDeviceInfo, MAX_WHOLE_DEVICE_INFO_LENGTH);

    /* CSTACK内容写入FLASH */
    usWriteLength = gulMainCstackPointer - ulCstackTop;
    uiFlashAddr = FLASH_Write(uiFlashAddr, (uint8_t *)ulCstackTop, usWriteLength);


    /* 保存数据区 */
    memset(gucaIoWholeDeviceInfo, 0, MAX_WHOLE_DEVICE_INFO_LENGTH);

    usWriteLength = MAX_AI_CHANNEL_NUMBER * sizeof(gusaAnalogData[0]);
    memcpy(gucaIoWholeDeviceInfo, gusaAnalogData, usWriteLength);

    uiFlashAddr = FLASH_Write(uiFlashAddr, gucaIoWholeDeviceInfo, usWriteLength);

    /* 保存接收DMA缓存 */
    usWriteLength = DMA_RECEIVE_BUFSIZE;
    uiFlashAddr = FLASH_Write(uiFlashAddr, gucUSART2DMARx, usWriteLength);

    /* 保存发送DMA缓存 */
    usWriteLength = DMA_SEND_BUFSIZE;
    uiFlashAddr = FLASH_Write(uiFlashAddr, gucUSART2DMATx, usWriteLength);


    while(1)
    {
    }
}

/******************************************************************
*                         局部函数实现                            *
******************************************************************/
/******************************************************************
*函数名称:DEBUG_FLASH_Write
*功能描述:异常中断时调用的写FLASH函数
*输入参数:uiFLASHAddr:写入起始FLASH地址
pucWriteContent:需要写入的内容
usWriteLength:需要写入的长度
*输出参数:
*返回值:返回当前可写入的FLASH起始地址，下一次写入调用此参数
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
******************************************************************/
static uint32_t FLASH_Write(uint32_t uiFLASHAddr, uint8_t *pucWriteContent, uint16_t usWriteLength)
{
    uint16_t usWriteSize = 0;   /* 写入内容按4字节算的个数 */
    uint8_t ucOtherRowCount = 0;    /* 写入FLASH时最后一行不足16字节的字节数 */
    uint8_t ucOtherRowLength = 0;   /* 不足16字节时多写的长度 */
    uint32_t uiReserved = 0xFFFFFFFF;    /* 填充字节 内容 */
    uint16_t usIntervalLength = DEBUG_FLASH_EACH_ROW_LENGTH * DEBUG_FLASH_INTERVAL_NUMBER;  /* 间隔字节数 */
    uint8_t ucOtherByteCount = 0;   /* 写入FLASH时不能按4字节的的整数倍写入的字节个数 */
    uint16_t i = 0;

    /* 打印内容 */
    usWriteSize = usWriteLength / DEBUG_FLASH_4_BYTE;
    ucOtherByteCount = usWriteLength % DEBUG_FLASH_4_BYTE;
    if (0 != ucOtherByteCount)
    {
        usWriteSize++;
        usWriteLength = usWriteLength - ucOtherByteCount+ DEBUG_FLASH_4_BYTE;
    }
    
    for (i = 0; i < usWriteSize; i++)
    {
        (void)FLASH_ProgramWord(uiFLASHAddr+i*DEBUG_FLASH_4_BYTE, *(uint32_t *)(pucWriteContent+i*DEBUG_FLASH_4_BYTE));
    }

    uiFLASHAddr = uiFLASHAddr + usWriteLength;

    /* 最后一行填充满16字节 */
    ucOtherRowCount = usWriteLength % DEBUG_FLASH_16_BYTE;
    if (0 != ucOtherRowCount)
    {
        ucOtherRowLength = DEBUG_FLASH_16_BYTE - ucOtherRowCount;
        usWriteSize = ucOtherRowLength / DEBUG_FLASH_4_BYTE;

        for (i = 0; i < usWriteSize; i++)
        {
            (void)FLASH_ProgramWord(uiFLASHAddr+i*DEBUG_FLASH_4_BYTE, uiReserved);
        }

        uiFLASHAddr = uiFLASHAddr + ucOtherRowLength;
    }

    /* 填充间隔，空16*4字节 */
    usWriteSize = usIntervalLength / DEBUG_FLASH_4_BYTE;
    for (i = 0; i < usWriteSize; i++)
    {
        (void)FLASH_ProgramWord(uiFLASHAddr+i*DEBUG_FLASH_4_BYTE, uiReserved);
    }

    uiFLASHAddr = uiFLASHAddr + usIntervalLength;

    return uiFLASHAddr;
}
