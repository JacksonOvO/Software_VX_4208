/******************************************************************
*
*文件名称:  main.c
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
#include "mbs.h"
#include "mbsi.h"
#include "mbsrtu.h"
#include "Ioinit.h"
#include "MbCommunication.h"
#include "Iocontrol.h"
#include "warn.h"
#include "STM32F0usart.h"
#include "IOOptions.h"
#include "stm32f30x_adc.h"
#include "watchdog.h"
#include "HModbusOptimize.h"
#include "stm32f30x_rcc.h"
#include "STM32F0Ads8331Driver.h"
#include "stm32f30x.h"
#include <stdio.h>
#include <yfuns.h>
#include "SEGGER_RTT.h"
/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/* 目前进入main函数后需要偏移16个字节才能偏移到栈顶
这个偏移量跟main函数里定义的变量个数有关系 */
#define MSP_OFFSET  (16)

/******************************************************************
*                           数据类型                              *
******************************************************************/
#define printf(...) SEGGER_RTT_printf(0, __VA_ARGS__)

/******************************************************************
*                         全局变量定义                            *
******************************************************************/
/* 主函数堆栈指针 */
extern unsigned long gulMainCstackPointer;

/*指示是否接收到一个完整的Modbus帧，1为接收到，
    在串口中断RTO分支里置位,0为未收到，在接收函数STM32F0USARTDriverReceive里复位*/
extern uint8_t ucIsReceivedData;

/*ADC 开启全局标记*/
extern uint8_t ucADCStart;

/* IO状态机 */
IO_STATE_e geInitState = IO_STATE_BUTT;

/******************************************************************
*                          局部函数声明                            *
******************************************************************/

/******************************************************************
*                         全局函数实现                            *
******************************************************************/

//void USART3_Init(void)
//{
//    GPIO_InitTypeDef GPIO_InitStruct;
//    USART_InitTypeDef USART_InitStruct;
//
//    // 打开 GPIOB 和 USART3 的时钟 
//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
//
//    //配置 PB10 为 USART3_TX，PB11 为 USART3_RX 
//    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_7);
//    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_7);
//
//    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
//    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
//    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
//    GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//    //初始化 USART3 
//    USART_InitStruct.USART_BaudRate = 115200;
//    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
//    USART_InitStruct.USART_StopBits = USART_StopBits_1;
//    USART_InitStruct.USART_Parity = USART_Parity_No;
//    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_Init(USART3, &USART_InitStruct);
//
//    //使能 USART3 
//    USART_Cmd(USART3, ENABLE);
//}
//
//#pragma module_name = "?__write"
//
////这个结构体来自 IAR stdio 库，必须有 
//size_t __write(int handle, const unsigned char * buffer, size_t size)
//{
//    size_t n;
//
//    // 只处理 stdout (handle==1) 和 stderr (handle==2)
//    if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR)
//        return _LLIO_ERROR;
//
//    for (n = 0; n < size; n++)
//    {
//        USART_SendData(USART3, buffer[n]);
//        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
//    }
//
//    return size;
//}


/******************************************************************
*                         局部函数实现                            *
******************************************************************/


/******************************************************************
*函数名称:main
*功能描述:IO主函数，调度从站功能模块，接收发送数据，
进行网关交互，IO处理，诊断处理。
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/9                              lv we004
******************************************************************/
int main(void)
{
    eMBErrorCode    eStatus = MB_EIO;
    xMBSHandle      xMBSHdl = NULL;    /*modbus 从站句柄*/
    IO_InterfaceResult_e  eResults = E_IO_ERROR_NOT_BUTT;
   // unsigned long int outputdata=0x05a0ff;
    /* 保存main函数cstack指针 */
    //gulMainCstackPointer = __get_MSP() + MSP_OFFSET;
   // FUNC_ENTER();

    /* 置为未初始化 */
    geInitState = IO_STATE_NOT_INIT;
    SEGGER_RTT_Init();
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    
    //USART3_Init();

    //printf("USART3 初始化完成\n");
    
    eResults = IoInit(&xMBSHdl);
    if (E_IO_ERROR_NONE  != eResults)
    {
       return eResults;
    }
 
    //printf("init end\n");
    #ifdef SCAN_HMODBUS_OPTIMIZE
    /*业务数据响应帧组装*/
    ServiceDataFramePackage();
    #endif
    /* 置为已初始化 */
   geInitState = IO_STATE_INITIALIZED;

    /* 置为未接收 */
    ucIsReceivedData = STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF;
   
    //initChannelOutput();
    /* 串口置为接收 */
   (void)STM32F0UDMASendComplete(E_STM32F0_USART_PORT_TWO);
   
    /* IO主循环 */
    for(;;)
    {
   
        /* BEGIN: Added by lv we004, 2017/3/8   PN:watchdog */
      //  FeedTheWatchDog();
        /* END:   Added by lv we004, 2017/3/8 */
        
        /* 串口接收 */
        if (STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF !=ucIsReceivedData)
        {
            eStatus = RTUFrameReceive( xMBSHdl );
            if ( MB_ENOERR == eStatus )
            {
                /* 进行Modbus处理 */
                eStatus = eMBSPoll( xMBSHdl );
                if (MB_ENOERR != eStatus)
                {
                    //(void)STM32F0UDMASendComplete(E_STM32F0_USART_PORT_ONE);
                    #ifdef SYSTEM_TEST_FAILD_HANDLE
                    SystemTestFaildhandle();
                    #endif
                }
            }
            ucIsReceivedData= STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF;
            (void)STM32F0UDMASendComplete(E_STM32F0_USART_PORT_TWO);
        }
        /* IO处理 */
        HandleIoData();

        ADC1GetVolTmpDiagnosisData();
            
        /* 诊断处理 */
        IODiagnose();
        if ( ADC_START == ucADCStart )
        {
            ucADCStart = ADC_STOP;
            /*启动ADC转换*/
         //    ADC_StartOfConversion(ADC1);
        }
    }
}


