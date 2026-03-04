/*******************************************************************
*文件名称:  STM32F0usart.h
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
#ifndef __USART_H
#define __USART_H


/******************************************************************
*                             头文件                              *
******************************************************************/
#include "stm32f30x_usart.h"
#include "stm32f30x_dma.h"
#include "stm32f30x_gpio.h"

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
#ifndef NULL
#define NULL    ((void *)0)
#endif

#define STM32F0_USART_DRIVER_TIMEOUT   11

/*DMA发送完全标志开关*/
#define STM32F0_USART_DMA_SEND_COMPLETE_SWITCH_OFF      0
#define STM32F0_USART_DMA_SEND_COMPLETE_SWITCH_ON       1

/*接收完成开关*/
#define STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF       0
#define STM32F0_USART_RECEIVE_COMPLETE_SWITCH_ON        1

#if STM32F0_UART1_DMA_ENABLE
/*DMA发送使用通道2*/
#define USARTx_TX_DMA_CHANNEL            DMA1_Channel2
#define USARTx_TX_DMA_FLAG_TC            DMA1_FLAG_TC2
#define USARTx_TX_DMA_FLAG_GL            DMA1_FLAG_GL2

/*DMA接收使用通道3*/
#define USARTx_RX_DMA_CHANNEL            DMA1_Channel3
#define USARTx_RX_DMA_FLAG_TC            DMA1_FLAG_TC3
#define USARTx_RX_DMA_FLAG_GL            DMA1_FLAG_GL3
#endif

#if STM32F0_UART2_DMA_ENABLE
/*DMA发送使用通道4*/
#define USARTx_TX_DMA_CHANNEL            DMA1_Channel4
#define USARTx_TX_DMA_FLAG_TC            DMA1_FLAG_TC4
#define USARTx_TX_DMA_FLAG_GL            DMA1_FLAG_GL4

/*DMA接收使用通道5*/
#define USARTx_RX_DMA_CHANNEL            DMA1_Channel5
#define USARTx_RX_DMA_FLAG_TC            DMA1_FLAG_TC5
#define USARTx_RX_DMA_FLAG_GL            DMA1_FLAG_GL5
#endif

/* Communication boards USART Interface */
#define USART1_TDR_ADDRESS                0x40013828
#define USART1_RDR_ADDRESS                0x40013824
#define USART2_TDR_ADDRESS                0x40004428
#define USART2_RDR_ADDRESS                0x40004424

/*静默模式是否被唤醒*/  
#define USART_NOT_WAKEUP                ((uint32_t)0x00000000)
#define USART_DMA_RECEIVE_BUFFER        (0x101 & 0xFFFF)


#define  DMA_CCR_DISEN                          ((uint16_t)0xFFFE)        /*!< Channel disable                      */
#define  USART_CR1_DISUE                        ((uint32_t)0xFFFFFFFE)            /*!< USART Disable */

/*系统时钟*/
#define STM32_SYSTEM_CLOCK                12000000

#define CR1_RWU_Set               ((uint16_t)0x80000)  /*!< USART mute mode Enable Mask */
#define CR1_RWU_Reset             ((uint16_t)0xFFFD)  /*!< USART mute mode Enable Mask */

#define CRC16_VALUE_LENGTH          (2)
#define UART_DRIVER_CRC16_NOERR    (0)

#define USART1_DEAT_VALUE      (31)

/******************************************************************
*                           数据类型                              *
******************************************************************/
typedef enum
{
    E_STM32F0_USART_DRIVER_OK = 0,    /* 返回成功 */
    E_STM32F0_USART_DRIVER_INIT_ERR,      /* 初始化失败 */
    E_STM32F0_USART_DRIVER_INIT_PARA_ERR, /*初始化入参错误*/
    E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR,/*初始化入参错误*/
    E_STM32F0_USART_DRIVER_RECEIVE_ERR,   /* 接收失败 */
    E_STM32F0_USART_DRIVER_RECEIVE_PARA_ERR,/*接收函数入参错误*/
    E_STM32F0_USART_DRIVER_RECEIVE_POINTER_ERR,/*接收指针函数失败*/
    E_STM32F0_USART_DRIVER_RECEIVE_POINTER_PARA_ERR,
    E_STM32F0_USART_DRIVER_SEND_ERR,      /* 发送失败 */
    E_STM32F0_USART_DRIVER_SEND_PARA_ERR, /*发送入参错误*/
    E_STM32F0_USART_DRIVER_SEND_COMPLETE_ERR,    /* 发送完成失败 */
    E_STM32F0_USART_DRIVER_SEND_COMPLETE_PARA_ERR,/*发送完成函数入参错误*/
    E_STM32F0_USART_DRIVER_CLOSE_PARA_ERR,  /*关闭串口入参错误*/
    E_STM32F0_USART_DRIVER_CLOSE_ERR,       /*关闭串口失败*/
    E_STM32F0_USART_DRIVER_CHANGE_BAUD_PARA_ERR,
    E_STM32F0_USART_DRIVER_BUTT
}STM32F0_USART_DRIVER_ERROR_CODE_e;

typedef enum
{
    E_STM32F0_USART_PORT_ONE = 1,
    E_STM32F0_USART_PORT_TWO,    
    E_STM32F0_USART_PORT_BUTT
}Port_Num_e;

/* BEGIN: Added by lwe004, 2017/9/27   PN:定义接收回调函数指针 */
/*定义接收回调函数指针*/
typedef void ( *STM32F0USARTDriverISRCB )(Port_Num_e ucPort );
/* END:   Added by lwe004, 2017/9/27 */


/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
/*STM32配置*/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverConfig(Port_Num_e ePortNum, 
                    USART_InitTypeDef *USART_InitStruct, DMA_InitTypeDef *DMA_RXInitStruct, 
                    DMA_InitTypeDef *DMA_TXInitStruct,GPIO_InitTypeDef *GPIO_TXInitStruct,
                    GPIO_InitTypeDef *GPIO_RTSInitStruct,GPIO_InitTypeDef *GPIO_RXInitStruct);

/*初始化*/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverInit(Port_Num_e ePortNum, 
                    USART_InitTypeDef *USART_InitStruct, DMA_InitTypeDef *DMA_RXInitStruct, 
                    DMA_InitTypeDef *DMA_TXInitStruct,GPIO_InitTypeDef *GPIO_TXInitStruct,
                    GPIO_InitTypeDef *GPIO_RTSInitStruct,GPIO_InitTypeDef *GPIO_RXInitStruct, 
                    STM32F0USARTDriverISRCB USARTxDriverCB);

/*ST串口驱动接收接口（拷贝内容）*/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverReceive(Port_Num_e ePortNum, 
                                            uint8_t *pucReceiveBuffer, uint16_t *pusReceiveLength);

/*ST串口驱动接收接口（返回指针）*/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverReceivePointer(Port_Num_e ePortNum, 
                                            uint8_t **pucReceiveBuffer, uint16_t *pusReceiveLength);

/*ST DMA中断*/
void DMA1_Channel1_IRQHandler(void);

void DMA1_Channel4_5_IRQHandler(void);

/*ST串口驱动发送*/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverSend(Port_Num_e ePortNum, 
                                uint8_t *pucSendBuffer, uint16_t usSendLength);

/*ST串口驱动发送完成接口*/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0UDMASendComplete(Port_Num_e ePortNum);

/*关闭串口接口*/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverClose(Port_Num_e ePortNum);


/*ST串口中断接口*/
void STM32F0USART1DriverISR(void);

STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDMASend(Port_Num_e ePortNum, 
                                                    uint8_t *pucSendBuffer, uint16_t usSendLength);
void STM32F0USARTEnableDE (USART_TypeDef *USARTx);
void STM32F0TempIAPEarse(void);
uint8_t STM320TempIAPWrite(uint32_t uiWriteAddr, uint8_t *pucWriteBuff, uint16_t usWriteLength);
void swapEndian(uint8_t *array, int length);
#endif 

