/*
 * MODBUS Library: ARM STM32 Port
 * Copyright (c) Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbportserial.c,v 1.1 2011/08/06 20:17:01 embedded-solutions.cwalter Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "stm32f30x_conf.h"
#include "stm32f30x_misc.h"
#include "stm32f30x.h"
#include "stm32f30x_usart.h"
#include "stm32f0usart.h"
#include "stm32f0usartopt.h"
/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"
#include "mbs.h"
#include "mbsi.h"
#include "mbsiconfig.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbportlayer.h"
#include "common/mbframe.h"
#include "common/mbutils.h"

/* ----------------------- Defines ------------------------------------------*/
#define IDX_INVALID             ( 255 )
#define UART_BAUDRATE_MIN       ( 300 )
#define UART_BAUDRATE_MAX		( 115200 )
/* BEGIN: Added by lwe004, 2017/9/24   PN:MODBUS 帧最大长度 */
#define MODBUS_ADU_LENGTH_MAX   ( 256 )
/* END:   Added by lwe004, 2017/9/24 */

#define UART_1_ENABLED          ( 0 )   /*!< Set this to 1 to enable USART1 */
#define UART_2_ENABLED          ( 1 )   /*!< Set this to 1 to enable USART2 */

#if ( UART_1_ENABLED == 1 ) && ( UART_2_ENABLED == 1 )
#define UART_1_PORT             ( MB_UART_1 )
#define UART_2_PORT             ( MB_UART_2 )
/* BEGIN: Modified by lwe004, 2017/9/25 PN:根据初始化获取串口号 */
/*
#define UART_1_IDX              ( 0 )
#define UART_2_IDX              ( 1 )
#define NUARTS                     ( 2  ) 
*/
#define UART_1_IDX              ( 1 )
#define UART_2_IDX              ( 2 )
#define NUARTS                   ( 3 ) 

/* END:   Modified by lwe004, 2017/9/25    */
#elif ( UART_1_ENABLED == 1 )
#define UART_1_PORT             ( MB_UART_1 )
/* BEGIN: Modified by lwe004, 2017/9/25 PN:根据初始化获取串口号 */
/*
#define UART_1_IDX              ( 0 )
#define NUARTS                  ( 1 ) 
*/
#define UART_1_IDX              ( 1 )
#define NUARTS                  ( 2 ) 

/* END:   Modified by lwe004, 2017/9/25    */
#elif ( UART_2_ENABLED == 1 )
#define UART_2_PORT             ( MB_UART_2 )
/* BEGIN: Modified by lwe004, 2017/9/25 PN:根据初始化获取串口号 */
/*
#define UART_2_IDX              ( 0 )
#define NUARTS                  ( 1 ) 
*/
#define UART_2_IDX              ( 2 )
#define NUARTS                  ( 3 )

/* END:   Modified by lwe004, 2017/9/25    */
#else
#define NUARTS                  ( 0 )
#endif

#define RS_485_UART_1_INIT(  )	\
do { \
} while( 0 )

#define RS_485_UART_1_ENABLE_TX(  )	\
do {\
   GPIO_WriteBit( GPIOA, GPIO_Pin_8, Bit_SET ); \
   GPIO_WriteBit( GPIOA, GPIO_Pin_11, Bit_SET ); \
} while( 0 )

#define RS_485_UART_1_DISABLE_TX(  ) \
do { \
   GPIO_WriteBit( GPIOA, GPIO_Pin_8, Bit_RESET ); \
   GPIO_WriteBit( GPIOA, GPIO_Pin_11, Bit_RESET ); \
} while( 0 )

#define RS_485_UART_2_INIT(  )\
do { \
    /* not implemented yet */ \
} while( 0 )

#define RS_485_UART_2_ENABLE_TX(  )	\
do { \
    /* not implemented yet */ \
} while( 0 )

#define RS_485_UART_2_DISABLE_TX(  ) \
do { \
    /* not implemented yet */ \
} while( 0 )

/* ----------------------- Defines ------------------------------------------*/
/* ----------------------- Defines (Internal - Don't change) ----------------*/
#define HDL_RESET( x ) do { \
    ( x )->ubIdx = IDX_INVALID; \
    ( x )->pbMBSTransmitterEmptyFN = NULL; \
    ( x )->pvMBSReceiveFN = NULL; \
    ( x )->xMBSHdl = MB_HDL_INVALID; \
} while( 0 );
/*停止位适配*/
#define STOP_BIT_1                ( 1 )  /*停止位为1*/
#define STOP_BIT_2               ( 2 )  /*停止位为1*/

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE           ubIdx;
    pbMBPSerialTransmitterEmptyAPIV2CB pbMBSTransmitterEmptyFN;
    pvMBPSerialReceiverAPIV2CB pvMBSReceiveFN;
    xMBHandle       xMBSHdl;
} xSerialHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC xSerialHandle xSerialHdls[NUARTS];
STATIC BOOL     bIsInitalized = FALSE;
/*声明回调函数*/
STATIC void eMBPSerialReceiveCB ( Port_Num_e  ucPort);
/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
/******************************************************************
*函数名称:eMBPSerialInit
*功能描述:初始化一个新的串行端口，并返回它的句柄
*输入参数:pxSerialHdl  指向串行句柄的指针
                         ucPort  以区分不同的串行接口的依赖 移植层的编号
                         ulBaudRate  波特率
                         ucDataBits  数据位的数量7或者8
                         eParity  校验方式
                         ucStopBits  停止位
                         xMBMHdl  Modbus协议栈句柄
*输出参数:无
*返回值:  MB_ENOERR  新的串行端口实例创建完成
                      MB_EINVAL  pxSerialHd为空或有参数无效
                      MB_EPORTERR  所有其它错误 
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/9/25                              lwe004
******************************************************************/
eMBErrorCode
eMBPSerialInit( xMBPSerialHandle * pxSerialHdl, UCHAR ucPort, ULONG ulBaudRate,
                eMBSerialParity eParity, UCHAR ucStopBits, xMBHandle xMBSHdl )
{
    USART_InitTypeDef USART_InitStructure; /*串口初始化结构体*/
    DMA_InitTypeDef DMA_TXInitStructure; /*DMA发送初始化结构体*/
    DMA_InitTypeDef DMA_RXInitStructure; /*DMA接收初始化结构体*/
    GPIO_InitTypeDef GPIO_TXInitStructure;/*GPIO发送初始化结构体*/
    GPIO_InitTypeDef GPIO_RXInitStructure;/*GPIO接收初始化结构体*/
    GPIO_InitTypeDef GPIO_RTSInitStructure;/*RTS初始化结构体*/
    eMBErrorCode    eStatus = MB_ENOERR;
    UBYTE           ubIdx;

    memset ( &USART_InitStructure, 0, sizeof ( USART_InitTypeDef ) );
    memset ( &DMA_TXInitStructure, 0, sizeof ( DMA_InitTypeDef ) );
    memset ( &DMA_RXInitStructure, 0, sizeof ( DMA_InitTypeDef ) );
    memset ( &GPIO_TXInitStructure, 0, sizeof ( GPIO_InitTypeDef ) );
    memset ( &GPIO_RXInitStructure, 0, sizeof ( GPIO_InitTypeDef ) );
    memset ( &GPIO_RTSInitStructure, 0, sizeof ( GPIO_InitTypeDef ) );

    MBP_ENTER_CRITICAL_SECTION(  );	
    if( !bIsInitalized )
    {
        for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
        {
            HDL_RESET( &xSerialHdls[ubIdx] );
        }
        bIsInitalized = TRUE;
    }
    /*串口号做数组元素下标,待考虑*/
    if( IDX_INVALID == xSerialHdls[ucPort].ubIdx )
    {
        HDL_RESET( &xSerialHdls[ucPort] );

       xSerialHdls[ucPort].xMBSHdl = xMBSHdl; 

       xSerialHdls[ucPort].ubIdx = ucPort;

       if ( E_STM32F0_USART_DRIVER_OK != STM32F0USARTDriverConfig( ( Port_Num_e ) ucPort, &USART_InitStructure, &DMA_TXInitStructure,
                                                                        &DMA_RXInitStructure, &GPIO_TXInitStructure, &GPIO_RXInitStructure, 
                                                                        &GPIO_RTSInitStructure ) )
        {
            return MB_EPORTERR;
        }
       
        USART_InitStructure.USART_BaudRate = ( uint32_t ) ulBaudRate;  /*串口波特率*/
        switch ( eParity )
        {
        case MB_PAR_NONE :
        {       
            USART_InitStructure.USART_Parity =  USART_Parity_No; /*校验方式*/
            break;
        }
        case MB_PAR_ODD :
        {       
            USART_InitStructure.USART_Parity =  USART_Parity_Odd; /*校验方式*/
            break;
        }
        case MB_PAR_EVEN :
        {       
            USART_InitStructure.USART_Parity =  USART_Parity_Even; /*校验方式*/
            break;
        }
        default :
        {
            break;
        }
        }

        switch ( ucStopBits )
        {
        case STOP_BIT_1:
        {
            USART_InitStructure.USART_StopBits = USART_StopBits_1;  /*停止位*/
            break;
        }
        case STOP_BIT_2:
        {
            USART_InitStructure.USART_StopBits = USART_CR2_STOP_1;  /*停止位*/
            break;
        }
        default :
        {
            break;
        }
        }
        
      /*初始化串口驱动,注册接收回调函数*/
      if ( E_STM32F0_USART_DRIVER_OK != STM32F0USARTDriverInit( ( Port_Num_e ) ucPort, &USART_InitStructure, &DMA_TXInitStructure,
                                                                   &DMA_RXInitStructure, &GPIO_TXInitStructure, &GPIO_RXInitStructure, 
                                                                   &GPIO_RTSInitStructure, eMBPSerialReceiveCB) )
      {
        eStatus = MB_EPORTERR;
        HDL_RESET( &xSerialHdls[ucPort] );
      }
      else
      {
        *pxSerialHdl = &xSerialHdls[ucPort];
         eStatus = MB_ENOERR;
      }

   }
   else
   {
     eStatus = MB_ENORES;
   }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

/******************************************************************
*函数名称:eMBPSerialClose
*功能描述:关闭串行端口
*输入参数:xSerialHdl  一个串口的有效句柄
*输出参数:无
*返回值:  MB_ENOERR  端口已被释放
                      MB_EAGAIN  现在不能关机，再次回调函数
                      MB_EINVAL 句柄无效
                      MB_EPORTERR  所有其它错误
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/9/25                              lwe004
******************************************************************/
eMBErrorCode
eMBPSerialClose( xMBPSerialHandle xSerialHdl )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
    //  if( ( NULL == pxSerialIntHdl->pbMBSTransmitterEmptyFN ) && ( NULL == pxSerialIntHdl->pvMBSReceiveFN ) )
      {
       /* Close USART 1 */
        if (E_STM32F0_USART_DRIVER_OK != STM32F0USARTDriverClose( ( Port_Num_e ) pxSerialIntHdl->ubIdx ) )
        {
           return MB_EIO;
        }
       /* Reset handle */
        HDL_RESET( pxSerialIntHdl );
       /* No error */
        eStatus = MB_ENOERR;
       }
     //  else
       {
    //    eStatus = MB_EIO;
       }
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

/******************************************************************
*函数名称:eMBPSerialTxEnable
*功能描述:使能或禁能协议栈发送器和注册一回调函数
*输入参数:xSerialHdl  一个串口的有效句柄
                         pbMBPTransmitterEmptyFN  指向回调函数的指针或发送器应该被禁用时为空
*输出参数:无
*返回值: MB_ENOERR 发送器已启用
                     MB_EINVAL   句柄无效
                     MB_EPORTERR  所有其它错误
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/9/25                              lwe004
******************************************************************/
eMBErrorCode
eMBPSerialTxEnable( xMBPSerialHandle xSerialHdl, pbMBPSerialTransmitterEmptyCB pbMBSTransmitterEmptyFN )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        eStatus = MB_ENOERR;

        if( NULL == pbMBSTransmitterEmptyFN )
        {
            /*禁能协议栈发送*/
            pxSerialIntHdl->pbMBSTransmitterEmptyFN = NULL;
        }

        /*注册回调函数*/
        pxSerialIntHdl->pbMBSTransmitterEmptyFN = pbMBSTransmitterEmptyFN;  
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

/******************************************************************
*函数名称:eMBPSerialRxEnable
*功能描述:使能或禁能协议栈接收器和注册一回调函数
*输入参数:xSerialHdl  一个串口的有效句柄
                         pbMBPTransmitterEmptyFN  指向回调函数的指针或接收器应该被禁用时为空
*输出参数:无
*返回值: MB_ENOERR 发送器已启用
                     MB_EINVAL   句柄无效
                     MB_EPORTERR  所有其它错误

*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/9/25                              lwe004
******************************************************************/
eMBErrorCode
eMBPSerialRxEnable( xMBPSerialHandle xSerialHdl, pvMBPSerialReceiverCB pvMBSReceiveFN )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );

    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        eStatus = MB_ENOERR;

        if( NULL == pvMBSReceiveFN )
        {
         /*禁能协议栈接收*/
         pxSerialIntHdl->pvMBSReceiveFN = NULL;
        }

        /*注册回调函数*/
        pxSerialIntHdl->pvMBSReceiveFN = pvMBSReceiveFN;

    }

    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}
/******************************************************************
*函数名称:eMBPSerialSend
*功能描述:调用串口驱动发送数据
*输入参数:xSerialHdl  一个串口的有效句柄
*输出参数:无
*返回值:MB_ENOERR 发送器已启用
                     MB_EINVAL   句柄无效
                     MB_EPORTERR  所有其它错误
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/9/24                              lwe004
******************************************************************/
eMBErrorCode eMBPSerialSend( xMBPSerialHandle xSerialHdl)
{
    //UBYTE *ubTxByte = NULL;
    UBYTE  ucSendBuffer[256];/*发送缓存*/
    USHORT usSendLength = 0; /*发送数据长度*/
    USHORT usBufferMax = MODBUS_ADU_LENGTH_MAX;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );

    memset ( ucSendBuffer, 0, sizeof ( ucSendBuffer ) );
    if ( !MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
      return MB_EINVAL;
    }

    if (IDX_INVALID == xSerialHdls[ pxSerialIntHdl->ubIdx].ubIdx)
    {
      return MB_EINVAL;
    }

    if ( NULL == xSerialHdls[pxSerialIntHdl->ubIdx].pbMBSTransmitterEmptyFN )
    {
      return MB_EINVAL;
    }

    /*从协议栈获取数据*/
    if (!xSerialHdls[pxSerialIntHdl->ubIdx].pbMBSTransmitterEmptyFN( pxSerialIntHdl->xMBSHdl, 
                                                                         ucSendBuffer, usBufferMax, &usSendLength ) )
    {
        return MB_EPORTERR;
    }

    /* 串口置为接收 */
    //(void)STM32F0UDMASendComplete( ( Port_Num_e )pxSerialIntHdl->ubIdx );
    /*调用驱动发送数据*/
    if ( E_STM32F0_USART_DRIVER_OK != STM32F0USARTDriverSend( ( Port_Num_e )pxSerialIntHdl->ubIdx, 
                                                                  ucSendBuffer, usSendLength ) )  
    {
        return MB_EPORTERR;
    }
    //for (int i;i<50;i++);
    MBP_EXIT_CRITICAL_SECTION(  );
    return MB_ENOERR;
}


/******************************************************************
*函数名称:eMBPSerialReceive
*功能描述:调用串口驱动接收数据
*输入参数:xSerialHdl  一个串口的有效句柄
*输出参数:无
*返回值:MB_ENOERR 发送器已启用
                     MB_EINVAL   句柄无效
                     MB_EPORTERR  所有其它错误
*其它说明:
*修改日期    版本号   修改人    修改内容

*---------------------------------------------------
*2017/9/17                              lwe004
******************************************************************/
eMBErrorCode eMBPSerialReceive( xMBPSerialHandle xSerialHdl)
{
    xMBSInternalHandle *pxIntHdl = NULL;
    xSerialHandle  *pxSerialIntHdl =  xSerialHdl;
    xMBPEventType   eEvent;

    uint16_t usReceiveLength = 0;  /*接收数据长度*/
    uint8_t *pucReceiveBuffer = NULL; /*接收数据缓存*/
    
    MBP_ENTER_CRITICAL_SECTION(  );
    if( !MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        return MB_EINVAL;
    }

    MBP_ENTER_CRITICAL_SECTION(  );

    /*传给协议栈内部句柄*/
    pxIntHdl = (xMBSInternalHandle *)pxSerialIntHdl->xMBSHdl;

    /*判断是否接收到消息*/
    if ( !bMBPEventGet( pxIntHdl->xSerialEventHdl, &eEvent ) )
    {
      return MB_EPORTERR;
    }

    /*判断消息类型是否匹配*/
    if ( MBS_EV_RECEIVED != ( eMBSEvent ) eEvent )
    { 
      return MB_EPORTERR;
    }

     /*判断句柄是否有用*/
    if (IDX_INVALID == xSerialHdls[ pxSerialIntHdl->ubIdx].ubIdx)
    {
        return MB_EPORTERR;
    }
    
    /*ST串口驱动接收接口（返回指针）*/
    if ( E_STM32F0_USART_DRIVER_OK != STM32F0USARTDriverReceivePointer( ( Port_Num_e ) pxSerialIntHdl->ubIdx, 
                                                                            &pucReceiveBuffer, &usReceiveLength ) )
    {
      return MB_EPORTERR;
    }
     
    //eMBPSerialRxEnable( xSerialHdls[pxSerialIntHdl->ubIdx], vMBPSerialReceiverAPIV2CB );
   // xSerialHdls[pxSerialIntHdl->ubIdx].pvMBSReceiveFN == vMBPSerialReceiverAPIV2CB;
    if( NULL == xSerialHdls[pxSerialIntHdl->ubIdx].pvMBSReceiveFN )
    {
      return MB_EINVAL;
    }
    /*发送数据到协议栈*/

    xSerialHdls[pxSerialIntHdl->ubIdx].pvMBSReceiveFN (  pxSerialIntHdl->xMBSHdl, ( UBYTE * )pucReceiveBuffer, usReceiveLength );
    MBP_EXIT_CRITICAL_SECTION(  );
    return MB_ENOERR;

}

/******************************************************************
*函数名称:eMBPSerialReceiveCB
*功能描述:中断回调函数
*输入参数:发送信号量
*输出参数:ePortNum 驱动使用端口号
*返回值:  MB_ENOERR:表示信号量发送成功
                      其余表示失败
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/9/17                              lwe004
******************************************************************/
STATIC void eMBPSerialReceiveCB ( Port_Num_e  ucPort)
{
    xMBSInternalHandle *pxIntHdl = NULL;

    if (NULL != xSerialHdls[ucPort].xMBSHdl )
    {
      pxIntHdl = ( xMBSInternalHandle *)xSerialHdls[ucPort].xMBSHdl;
      /*发送信号量*/
      if (MB_ENOERR != (eMBPEventPost( pxIntHdl->xSerialEventHdl, ( xMBPEventType ) MBS_EV_RECEIVED ) ) )
      {
        MBSLAVE_DEBUG("Post semaphore failed");
      }
    }
}



