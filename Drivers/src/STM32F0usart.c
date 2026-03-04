/*******************************************************************
*ÎÄ¼þÃû³Æ:  usart.c
*ÎÄ¼þ±êÊ¶:
*ÄÚÈÝÕªÒª:
*ÆäËüËµÃ÷:
*µ±Ç°°æ±¾:
*
*ÐÞ¸Ä¼ÇÂ¼1:
*    ÐÞ¸ÄÈÕÆÚ:
*    °æ ±¾ ºÅ:
*    ÐÞ ¸Ä ÈË:
*    ÐÞ¸ÄÄÚÈÝ:
******************************************************************/


/******************************************************************
*                             Í·ÎÄ¼þ                              *
******************************************************************/
#include <string.h>
#include "stm32f30x_usart.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_dma.h"
#include "stm32f0usart.h"
#include "stm32f0usartopt.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_conf.h"
#include "common.h"
#include "STM32GPIODriver.h"
#include "Ioinit.h"
#include "stm32f30x_flash.h"
#include "HModbusOptimize.h"

#include "MbCommunication.h"
#include "Iocontrol.h"
#include "warn.h"
#include "Mbscrc.h"

/******************************************************************
*                            ³£Á¿                                 *
******************************************************************/

/******************************************************************
*                           ºê¶¨Òå                                *
******************************************************************/

/******************************************************************
*                           Êý¾ÝÀàÐÍ                              *
******************************************************************/

/******************************************************************
*                           È«¾Ö±äÁ¿ÉùÃ÷                          *
******************************************************************/
/* BEGIN: Added by lwe004, 2017/9/27	 PN:°²×°»Øµ÷º¯Êý */
/* USART driver call-back function pointer data */
static STM32F0USARTDriverISRCB pfnIntCbDat[MAX_PORT_NUM];
/* END:   Added by lwe004, 2017/9/27 */

#if STM32F0_UART1_DMA_ENABLE
/*¶¨ÒåUASRT1DMA·¢ËÍ»º´æ*/
uint8_t gucUSART1DMATx[DMA_SEND_BUFSIZE] ={0};   
uint16_t gusUSART1DMATx[DMA_SEND_BUFSIZE] ={0}; 

/*¶¨ÒåUSART1DMA½ÓÊÕ»º´æ*/
uint8_t gucUSART1DMARx[DMA_RECEIVE_BUFSIZE] ={0};
#endif

#if STM32F0_UART2_DMA_ENABLE
/*¶¨ÒåUSART2DMA·¢ËÍ»º´æ*/
uint8_t gucUSART2DMATx[DMA_SEND_BUFSIZE] ={0};   
uint16_t gusUSART2DMATx[DMA_SEND_BUFSIZE] ={0}; 

/*??USART2DMA????*/
uint8_t gucUSART2DMARx[DMA_RECEIVE_BUFSIZE] ={0};
#endif

/*Ö¸Ê¾ÊÇ·ñ½ÓÊÕµ½Ò»¸öÍêÕûµÄModbusÖ¡£¬1Îª½ÓÊÕµ½£¬
    ÔÚ´®¿ÚÖÐ¶ÏRTO·ÖÖ§ÀïÖÃÎ»,0ÎªÎ´ÊÕµ½£¬ÔÚ½ÓÊÕº¯ÊýSTM32F0USARTDriverReceiveÀï¸´Î»*/
uint8_t ucIsReceivedData = STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF;

/* BEGIN: Added by zhanghaifeng  we015, 2017/10/12   PN: */
/*ÖÐ¶ÏÏòÁ¿¿ØÖÆÆ÷£¬ÓÃÓÚÊ¹ÄÜ´®¿ÚÖÐ¶ÏºÍDMAÍ¨µÀÖÐ¶Ï*/
NVIC_InitTypeDef NVIC_InitStructure;
/* END:   Added by zhanghaifeng  we015, 2017/10/12 */

/* ´ÓÕ¾Õ¾ºÅ*/
extern uint8_t gucStationNum;

/* IO×´Ì¬»ú */
extern IO_STATE_e geInitState;

#ifdef SCAN_HMODBUS_OPTIMIZE


#if FR4XX4_CODE|| (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)||(IO_CODE_TYPE == FR4104_CODE)
extern int16_t gusaAnalogData[MAX_AO_CHANNEL_NUMBER];
/*¶ÁÈ¡´ÓÕ¾ÒµÎñÊý¾ÝÇëÇóÖ¡(ÎÞ¸æ¾¯)*/
extern uint8_t gucaServiceDataRespFrame[RESPONSE_FRAME_LENGTH]; 
extern uint16_t gusaServiceDataRespFrame[RESPONSE_FRAME_LENGTH+1]; 

/*¶ÁÈ¡´ÓÕ¾ÒµÎñÊý¾ÝÇëÇóÖ¡(ÓÐ¸æ¾¯)*/
extern uint8_t gucaServiceDataRespFrameWarn[RESPONSE_FRAME_LENGTH];
extern uint16_t gusaServiceDataRespFrameWarn[RESPONSE_FRAME_LENGTH+1];

extern uint8_t  gucaUpServiceDataRequFrame[RESPONSE_FRAME_LENGTH];

extern uint8_t gucaUpServiceDataRespFrameMainLoop[UP_RESPONSE_FRAME_LENGTH];
extern uint16_t gusaUpServiceDataRespFrameUSART[UP_RESPONSE_FRAME_LENGTH+1];

extern uint8_t gucAoDataIsChanged;
extern uint8_t gucDisConnectionFlag;
extern uint8_t ucIsDisconnectCheckTimerStart;
extern uint8_t gucaWarnData[MAX_WARN_DATA_NUM/BIT_NUM_PER_BYTE];
extern uint8_t gucDownServDataAnalogUpdateOK;

extern uint8_t gucServDataFraPackageOK;
extern ModuleInfo_t ModuleInfo[4];
extern uint8_t IOLinkEventInfo[16];
#endif
#endif



/******************************************************************
*                         È«¾Öº¯ÊýÉùÃ÷                            *
******************************************************************/
/* BEGIN: Added by lwe004, 2017/9/27   PN:°²×°»Øµ÷º¯Êý */
/*ÉùÃ÷ST ´®¿ÚÇý¶¯»Øµ÷º¯Êý*/
static void STM32F0USART1DriverInstallIntCallBack( Port_Num_e ePortNum, 
                                                      STM32F0USARTDriverISRCB DriverISRCB);
/* END:   Added by lwe004, 2017/9/27 */

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USARTDriverConfig
*¹¦ÄÜÃèÊö:³õÊ¼»¯º¯Êýµ÷ÓÃµÄÒ»Ð©½á¹¹ÌåÅäÖÃ
*ÊäÈë²ÎÊý:
                            eProtNum   ´®¿ÚºÅ
                            USART_InitStruct   ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            DMA_TXInitStruct   ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            DMA_RXInitStruct  ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            GPIO_TXInitStruct  ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            GPIO_RXInitStruct  ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            GPIO_RTSInitStruct  ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
*Êä³ö²ÎÊý:
*·µ»ØÖµ:
                            E_STM32F0_USART_DRIVER_OK  ³É¹¦
                            E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR   Èë²Î´íÎó
                            E_STM32F0_USART_DRIVER_INIT_ERR ³õÊ¼»¯Ê§°Ü
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/22                              zhanghaifeng  we015
******************************************************************/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverConfig(Port_Num_e ePortNum, 
                    USART_InitTypeDef *USART_InitStruct, DMA_InitTypeDef *DMA_RXInitStruct, 
                    DMA_InitTypeDef *DMA_TXInitStruct,GPIO_InitTypeDef *GPIO_TXInitStruct,
                    GPIO_InitTypeDef *GPIO_RTSInitStruct,GPIO_InitTypeDef *GPIO_RXInitStruct)
{
    /*ÅÐ¶ÏUSART_InitStructÊÇ·ñ´íÎó*/
    if (NULL == USART_InitStruct)
    {
        return E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR;
    }
    
    /*ÅÐ¶ÏDMA_InitStructÊÇ·ñ´íÎó*/
    if (NULL == DMA_TXInitStruct)
    {
        return E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR;
    }
    
    if (NULL == DMA_RXInitStruct)
    {
        return E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR;
    }
    
    if (NULL == GPIO_TXInitStruct)
    {
        return E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR;
    }
    
    if (NULL == GPIO_RXInitStruct)
    {
        return E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR;
    }
    
    if (NULL == GPIO_RTSInitStruct)
    {
        return E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR;
    }
    
    switch(ePortNum)
    {
    case E_STM32F0_USART_PORT_ONE:
    {
#if STM32F0_UART1_DMA_ENABLE 
        /*GPIOA£¬USART1£¬DMAµÄÊ±ÖÓÅäÖÃ*/
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOAEN, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART1EN, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /*¹Ü½Å¸´ÓÃ¹¦ÄÜÅäÖÃ*/
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

        /*GPIO½á¹¹ÌåÅäÖÃ*/
        GPIO_TXInitStruct->GPIO_Pin = GPIO_Pin_9;
        
        /*´®¿Ú·¢ËÍÊ¹ÓÃ¸´ÓÃÄ£Ê½´«ÊäËÙ¶ÈÉèÖÃÎª¸ßËÙ£¬
                ÍÆÍìÊä³ö*/
        GPIO_TXInitStruct->GPIO_Mode = GPIO_Mode_AF;
        GPIO_TXInitStruct->GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_TXInitStruct->GPIO_OType = GPIO_OType_PP;
        GPIO_TXInitStruct->GPIO_PuPd = GPIO_PuPd_UP;
        
        /*GPIO½á¹¹Ìå½ÓÊläÖÃ*/
        GPIO_RXInitStruct->GPIO_Pin = GPIO_Pin_10;

        /*´®¿Ú½ÓÊÕÊ¹ÓÃ¸´ÓÃÄ£Ê½´«ÊäËÙ¶ÈÉèÖÃÎª¸ßËÙ£¬
                ÍÆÍìÊä³ö*/
        GPIO_RXInitStruct->GPIO_Mode = GPIO_Mode_AF;
        GPIO_RXInitStruct->GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_RXInitStruct->GPIO_OType = GPIO_OType_PP;
        GPIO_RXInitStruct->GPIO_PuPd = GPIO_PuPd_UP;

        /*GPIO½á¹¹ÌåRTSÅäÖÃ*/
        GPIO_RTSInitStruct->GPIO_Pin = GPIO_Pin_12;

        /*DEÊ¹ÓÃ¸´ÓÃÄ£Ê½´«ÊäËÙ¶ÈÉèÖÃÎª¸ßËÙ£¬
                ÍÆÍìÊä³ö*/
        GPIO_RTSInitStruct->GPIO_Mode = GPIO_Mode_AF;
        GPIO_RTSInitStruct->GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_RTSInitStruct->GPIO_OType = GPIO_OType_PP;
        GPIO_RTSInitStruct->GPIO_PuPd = GPIO_PuPd_NOPULL;     

        /*DMA½á¹¹ÌåÅäÖÃ*/
        DMA_TXInitStruct->DMA_PeripheralBaseAddr = USART1_TDR_ADDRESS;
        DMA_TXInitStruct->DMA_MemoryBaseAddr = (uint32_t)gucUSART1DMATx;
        DMA_TXInitStruct->DMA_DIR = DMA_DIR_PeripheralDST;
        DMA_TXInitStruct->DMA_BufferSize = DMA_SEND_BUFSIZE;
        DMA_TXInitStruct->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_TXInitStruct->DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_TXInitStruct->DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        DMA_TXInitStruct->DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
        DMA_TXInitStruct->DMA_Mode = DMA_Mode_Normal;
        DMA_TXInitStruct->DMA_Priority = DMA_Priority_Low;
        DMA_TXInitStruct->DMA_M2M = DMA_M2M_Disable;
        
        /*DMA½á¹¹ÌåÅäÖÃ*/
        DMA_RXInitStruct->DMA_PeripheralBaseAddr = USART1_RDR_ADDRESS;
        DMA_RXInitStruct->DMA_MemoryBaseAddr = (uint32_t)gucUSART1DMARx;
        DMA_RXInitStruct->DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_RXInitStruct->DMA_BufferSize = DMA_RECEIVE_BUFSIZE;
        DMA_RXInitStruct->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_RXInitStruct->DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_RXInitStruct->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_RXInitStruct->DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_RXInitStruct->DMA_Mode = DMA_Mode_Normal;
        DMA_RXInitStruct->DMA_Priority = DMA_Priority_Low;
        DMA_RXInitStruct->DMA_M2M = DMA_M2M_Disable;
        
        /* Enable the USART IRQ channel */
        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= USART_RECEIVE_USART1_PRI;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority  = USART_RECEIVE_USART1_PRI;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        /*USART½á¹¹ÌåÅäÖÃ*/
        USART_InitStruct->USART_BaudRate = SLAVE_STATION_DEFAULT_COMMCATION_BAUD;
        USART_InitStruct->USART_WordLength = USART_WordLength_9b;
        USART_InitStruct->USART_StopBits = USART_StopBits_1;
        USART_InitStruct->USART_Parity = USART_Parity_No;
        
        /*RTSÓ²¼þÁ÷¿ØÖÆ£¬ÓÃÓÚ¸úLPC1788µÄ°ëË«¹¤´®¿ÚÍ¨Ñ¶*/
        USART_InitStruct->USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStruct->USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
#endif
        break;  
    }
    case E_STM32F0_USART_PORT_TWO:
    {
#if STM32F0_UART2_DMA_ENABLE    
        /*GPIOA,USART2,DMA?????*/
       
        RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART2EN, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOAEN, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /*????????*/
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_7);

        /*GPIO½á¹¹Ìå·¢ËÍÅäÖÃ*/
        GPIO_TXInitStruct->GPIO_Pin = GPIO_Pin_2;

        /*´®¿Ú·¢ËÍÊ¹ÓÃ¸´ÓÃÄ£Ê½´«ÊäËÙ¶ÈÉèÖÃÎª¸ßËÙ£¬
                ÍÆÍìÊä³ö*/
        GPIO_TXInitStruct->GPIO_Mode = GPIO_Mode_AF;
        GPIO_TXInitStruct->GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_TXInitStruct->GPIO_OType = GPIO_OType_PP;
        GPIO_TXInitStruct->GPIO_PuPd = GPIO_PuPd_UP;
        
        /*GPIO½á¹¹Ìå½ÓÊläÖÃ*/
        GPIO_RXInitStruct->GPIO_Pin = GPIO_Pin_3;

        /*´®¿Ú½ÓÊÕÊ¹ÓÃ¸´ÓÃÄ£Ê½´«ÊäËÙ¶ÈÉèÖÃÎª¸ßËÙ£¬
                ÍÆÍìÊä³ö*/
        GPIO_RXInitStruct->GPIO_Mode = GPIO_Mode_AF;
        GPIO_RXInitStruct->GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_RXInitStruct->GPIO_OType = GPIO_OType_PP;
        GPIO_RXInitStruct->GPIO_PuPd = GPIO_PuPd_UP;

        /*GPIO½á¹¹ÌåRTSÅäÖÃ*/
        GPIO_RTSInitStruct->GPIO_Pin = GPIO_Pin_1;

        /*DEÊ¹ÓÃ¸´ÓÃÄ£Ê½´«ÊäËÙ¶ÈÉèÖÃÎª¸ßËÙ£¬
                ÍÆÍìÊä³ö*/
        GPIO_RTSInitStruct->GPIO_Mode = GPIO_Mode_AF;
        GPIO_RTSInitStruct->GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_RTSInitStruct->GPIO_OType = GPIO_OType_PP;
        GPIO_RTSInitStruct->GPIO_PuPd = GPIO_PuPd_NOPULL;     

        /*DMA½á¹¹ÌåÅäÖÃ*/
        DMA_TXInitStruct->DMA_PeripheralBaseAddr = USART2_TDR_ADDRESS;
        DMA_TXInitStruct->DMA_MemoryBaseAddr = (uint32_t)gucUSART2DMATx;
        DMA_TXInitStruct->DMA_DIR = DMA_DIR_PeripheralDST;
        DMA_TXInitStruct->DMA_BufferSize = DMA_SEND_BUFSIZE;
        DMA_TXInitStruct->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_TXInitStruct->DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_TXInitStruct->DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        DMA_TXInitStruct->DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
        DMA_TXInitStruct->DMA_Mode = DMA_Mode_Normal;
        DMA_TXInitStruct->DMA_Priority = DMA_Priority_Low;
        DMA_TXInitStruct->DMA_M2M = DMA_M2M_Disable;

        /*DMA½á¹¹ÌåÅäÖÃ*/
        DMA_RXInitStruct->DMA_PeripheralBaseAddr = USART2_RDR_ADDRESS;
        DMA_RXInitStruct->DMA_MemoryBaseAddr = (uint32_t)gucUSART2DMARx;
        DMA_RXInitStruct->DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_RXInitStruct->DMA_BufferSize = DMA_RECEIVE_BUFSIZE;
        DMA_RXInitStruct->DMA_PeripheralInc = DMA_PeripheralInc_Disable;//----DMA_PeripheralInc_Disable
        DMA_RXInitStruct->DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_RXInitStruct->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_RXInitStruct->DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_RXInitStruct->DMA_Mode = DMA_Mode_Normal;
        DMA_RXInitStruct->DMA_Priority = DMA_Priority_Low;
        DMA_RXInitStruct->DMA_M2M = DMA_M2M_Disable;                        
            
             /* Enable the USART IRQ channel */
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;   
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority  = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        
        NVIC_Init(&NVIC_InitStructure);           
                /*USART?????*/
        USART_InitStruct->USART_BaudRate = SLAVE_STATION_DEFAULT_COMMCATION_BAUD;
        USART_InitStruct->USART_WordLength = USART_WordLength_9b;
        USART_InitStruct->USART_StopBits = USART_StopBits_1;
        USART_InitStruct->USART_Parity = USART_Parity_No;
        
        /*RTSÓ²¼þÁ÷¿ØÖÆ£¬ÓÃÓÚ¸úLPC1788µÄ°ëË«¹¤´®¿ÚÍ¨Ñ¶*/
        USART_InitStruct->USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStruct->USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
#endif
        break;        
    }
    default:
    {
        return E_STM32F0_USART_DRIVER_CONFIG_PARA_ERR;
    }
    }
    
    return E_STM32F0_USART_DRIVER_OK;
}

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USARTDriverInit
*¹¦ÄÜÃèÊö:³õÊ¼»¯
*ÊäÈë²ÎÊý:
                            eProtNum   ´®¿ÚºÅ
                            USART_InitStruct   ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            DMA_TXInitStruct   ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            DMA_RXInitStruct  ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            GPIO_TXInitStruct  ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            GPIO_RXInitStruct  ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            GPIO_RTSInitStruct  ST¶¨ÒåµÄ³õÊ¼»¯½á¹¹ÌåÖ¸Õë
                            USARTxDriverCB   »Øµ÷º¯ÊýÖ¸Õë
*Êä³ö²ÎÊý:                          
*·µ»ØÖµ:
                            E_STM32F0_USART_DRIVER_OK  ³É¹¦
                            E_STM32F0_USART_DRIVER_INIT_PARA_ERR   Èë²Î´íÎó
                            E_STM32F0_USART_DRIVER_INIT_ERR ³õÊ¼»¯Ê§°Ü
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/24                              zhanghaifeng  we015
******************************************************************/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverInit(Port_Num_e ePortNum, 
                    USART_InitTypeDef *USART_InitStruct, DMA_InitTypeDef *DMA_RXInitStruct, 
                    DMA_InitTypeDef *DMA_TXInitStruct,GPIO_InitTypeDef *GPIO_TXInitStruct,
                    GPIO_InitTypeDef *GPIO_RTSInitStruct,GPIO_InitTypeDef *GPIO_RXInitStruct, 
                    STM32F0USARTDriverISRCB USARTxDriverCB)
{
    USART_TypeDef *USARTx = NULL;
    
    assert_param(IS_USART_BAUDRATE(USART_InitStruct->USART_BaudRate));
    
    /*ÅÐ¶ÏUSART_InitStructÊÇ·ñ´íÎó*/
    if (NULL == USART_InitStruct)
    {
        return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }
    
    /*Èë²ÎÅÐ¶Ï*/
    if (NULL == DMA_TXInitStruct)
    {
        return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }
    
    if (NULL == DMA_RXInitStruct)
    {
        return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }
    
    if (NULL == GPIO_TXInitStruct)
    {
        return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }
    
    if (NULL == GPIO_RXInitStruct)
    {
        return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }
    
    if (NULL == GPIO_RTSInitStruct)
    {
        return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }
    
    /*ÅÐ¶Ï»Øµ÷º¯ÊýÈë²ÎÊÇ·ñºÏ·¨*/
    if ( NULL == USARTxDriverCB )
    {
      return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }

    if (E_STM32F0_USART_PORT_ONE == ePortNum) 
    {                               
        USARTx = USART1;            
    } 
    else if (E_STM32F0_USART_PORT_TWO == ePortNum)    
    {                           
        USARTx = USART2;       
    }
    else
    {
        return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }

    /* BEGIN: Added by zhanghaifeng  we015, 2017/1/7   PN: */
    /*ÓÉÓÚ´®¿Ú1Ê¹ÓÃDMAÍ¨µÀ2ºÍ3£¬¶ø(AI)SPIÇý¶¯±ØÐëÊ¹ÓÃDAM
           Í¨µÀ2½ÓÊÕ£¬ËùÒÔÐèÒªÐÞ¸Ä´®¿ÚÇý¶¯Ê¹ÓÃDMAÍ¨µÀ4ºÍ5
           ÊÕ·¢ ¶øÊ¹ÓÃÍ¨µÀ4ºÍ5ÊÕ·¢ÐèÒª¸´ÓÃDMAÍ¨µÀ*/
    //SYSCFG_DeInit();
  //  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
//    SYSCFG_DMAChannelRemapConfig(SYSCFG_CFGR1_USART1RX_DMA_RMP, ENABLE);//-----------
//    SYSCFG_DMAChannelRemapConfig(SYSCFG_CFGR1_USART1TX_DMA_RMP, ENABLE);
    /* END:   Added by zhanghaifeng  we015, 2017/1/7 */
    
    switch(ePortNum)
    {
    case E_STM32F0_USART_PORT_ONE:
    {
        /*GPIOAÄ£¿éºÍRTS³õÊ¼»¯*/        
        GPIO_Init(GPIOA, GPIO_RTSInitStruct);
        GPIO_Init(GPIOA, GPIO_TXInitStruct);
        GPIO_Init(GPIOA, GPIO_RXInitStruct);
        
        /*³õÊ¼»¯DMAÍ¨µÀ£¬Í¨µÀ4ÓÃ×ö·¢ËÍ*/
        DMA_DeInit(DMA1_Channel4);
        DMA_Init(DMA1_Channel4, DMA_TXInitStruct);

        /*³õÊ¼»¯DMAÍ¨µÀ£¬Í¨µÀ5ÓÃ×ö½ÓÊÕ*/
        DMA_DeInit(DMA1_Channel5);
        DMA_Init(DMA1_Channel5, DMA_RXInitStruct);
            
        /*Ê¹ÄÜ´®¿ÚDMA½ÓÊÕºÍ·¢ËÍ*/
        USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
        USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
        /*Ê¹ÄÜ´®¿Ú½ÓÊÕÍ¨µÀ£¬Í¨µÀ5ÓÃ×ö½ÓÊÕ*/
        DMA_Cmd(DMA1_Channel5, ENABLE);
        break;
    }
    case E_STM32F0_USART_PORT_TWO:
    {
        /*GPIOAÄ£¿éºÍRTS³õÊ¼»¯*/
        GPIO_Init(GPIOA, GPIO_TXInitStruct);
        GPIO_Init(GPIOA, GPIO_RXInitStruct);
        GPIO_Init(GPIOA, GPIO_RTSInitStruct);

          
        DMA_DeInit(DMA1_Channel7);
        DMA_Init(DMA1_Channel7, DMA_TXInitStruct);
        
         
        DMA_DeInit(DMA1_Channel6);
        DMA_Init(DMA1_Channel6, DMA_RXInitStruct);
             
        USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
        USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
        break;
    }
    default:
    {
        return E_STM32F0_USART_DRIVER_INIT_PARA_ERR;
    }
    }
    
    USART_OverSampling8Cmd(USARTx,ENABLE);   
    /*??USART2???,?????72MHz*/
    RCC_USARTCLKConfig(RCC_USART2CLK_SYSCLK);  
    
   /* ?????*/
    USART_Init(USARTx, USART_InitStruct);
    
    /*9Î»Ä£Ê½*/
    USART_AddressDetectionConfig(USARTx,USART_AddressLength_7b);
    /*ÉèÖÃ´ÓÕ¾µØÖ·*/
    USART_SetAddress(USARTx,gucStationNum);

    USART_MuteModeCmd(USARTx, ENABLE);
    USART_MuteModeWakeUpConfig(USARTx,USART_WakeUp_AddressMark);
    
    /*??DE*/
   // STM32F0USARTEnableDE(USARTx);
    USART_DECmd(USARTx, ENABLE);
    /*??DE????*/
    USART_SetDEAssertionTime(USARTx, USART1_DEAT_VALUE);

    /*³õÊ¼»¯MODBUSÖ§³ÖµÄÌØ¶¨º¯Êý*/
    USART_SetReceiverTimeOut(USARTx, STM32F0_USART_DRIVER_TIMEOUT);
    USART_ReceiverTimeOutCmd(USARTx, ENABLE);
    USART_ITConfig(USARTx, USART_IT_RTO, ENABLE);
    USART_RequestCmd(USARTx, USART_Request_MMRQ, ENABLE);
    /*Ê¹ÄÜ´®¿Ú*/
    USART_Cmd(USARTx, ENABLE);

    /*°²×°´®¿ÚÇý¶¯»Øµ÷º¯Êý*/
    STM32F0USART1DriverInstallIntCallBack(ePortNum, USARTxDriverCB);

    return E_STM32F0_USART_DRIVER_OK;
}

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USARTDriverSend
*¹¦ÄÜÃèÊö:
*ÊäÈë²ÎÊý:
                             ePortNum  ´®¿ÚºÅ
                             pucSendBuffer   ·¢ËÍ»º´æ£¬ÓÉÒµÎñ·ÖÅä
                             usSendLength   ·¢ËÍ³¤¶È
*Êä³ö²ÎÊý:
*·µ»ØÖµ:
                            E_STM32F0_USART_DRIVER_OK  ³É¹¦
                            E_STM32F0_USART_DRIVER_SEND_PARA_ERR    Èë²Î´íÎó
                            E_STM32F0_USART_DRIVER_SEND_ERR ½ÓÊÕÊ§°Ü
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/25                              zhanghaifeng  we015
******************************************************************/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverSend(Port_Num_e ePortNum, 
                                                uint8_t *pucSendBuffer, uint16_t usSendLength)
{
    USART_TypeDef *USARTx = NULL;
    STM32F0_USART_DRIVER_ERROR_CODE_e status = E_STM32F0_USART_DRIVER_SEND_ERR;

    /*¶à·¢ËÍÒ»¸ö×Ö½Ú:Ö÷Õ¾Õ¾ºÅ0XFE*/
    usSendLength++;
    /*ÅÐ¶Ï·¢ËÍ»º´æÇøµØÖ·Îª¿Õ*/
    if (NULL == pucSendBuffer)
    {
        return E_STM32F0_USART_DRIVER_SEND_PARA_ERR;
    }
    
    if (E_STM32F0_USART_PORT_ONE == ePortNum) 
    {                               
        USARTx = USART1;            
    }     
    else if (E_STM32F0_USART_PORT_TWO == ePortNum)    
    {                           
        USARTx = USART2;       
    }
    else
    {
        return E_STM32F0_USART_DRIVER_SEND_PARA_ERR;
    }
    
    /*Ê¹ÄÜ´®¿Ú*/
    USART_Cmd(USARTx, ENABLE);
#if STM32F0_UART1_DMA_ENABLE
    /*¹Ø±ÕÍ¨µÀ£¬·¢ËÍÊ¹ÓÃÍ¨µÀ4*/
    DMA_Cmd(DMA1_Channel4, DISABLE);

    /*ÉèÖÃDMA·¢ËÍ»º´æ´óÐ¡*/ 
    DMA_SetCurrDataCounter(DMA1_Channel4, (usSendLength & 0xFFFF));

    /*ÖØÐÂ·¢ËÍbuffer*/
    DMA1_Channel4->CMAR = (uint32_t)gusUSART1DMATx;

    /*¿½±´´ý·¢ËÍÊý¾Ýµ½DMA»º´æ*/
    memcpy(gucUSART1DMATx, pucSendBuffer, usSendLength);

    /*9Î»Ä£Ê½ÏÂ·¢ËÍÊý¾ÝÎª2×Ö½Ú·¢ËÍ*/
    gusUSART1DMATx[0] = 0XFE|0x100;
    for (uint8_t i = 0; i < usSendLength; i++)
    {
        gusUSART1DMATx[i+1] = gucUSART1DMATx[i];
    }
    
    /*´ò¿ª·¢ËÍÖÐ¶Ï*/
    //DMA_ITConfig(DMA1_Channel4, DMA_IT_TC | DMA_IT_TE, ENABLE); 
#endif

#if STM32F0_UART2_DMA_ENABLE
    /*????,??????4*/
    DMA_Cmd(DMA1_Channel7, DISABLE);

    /*??DMA??????*/
    DMA_SetCurrDataCounter(DMA1_Channel7, (usSendLength  & 0xFFFF));

    /*????buffer*/
    DMA1_Channel7->CMAR = (uint32_t)gusUSART2DMATx;
    
    /*????????DMA??*/
    memcpy(gucUSART2DMATx, pucSendBuffer, usSendLength);

        /*9?????????2????*/
    gusUSART2DMATx[0] = 0XFE|0x100;
    for (uint8_t i = 0; i < usSendLength; i++)
    {
        gusUSART2DMATx[i+1] = gucUSART2DMATx[i];
    }
    /*??????*/
   // DMA_ITConfig(DMA1_Channel4, DMA_IT_TC | DMA_IT_TE, ENABLE); 
#endif

    /*´ò¿ªDMA·¢ËÍ*/
    USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);
#if STM32F0_UART1_DMA_ENABLE    
    /* Ê¹ÄÜ DMA Í¨µÀ4*/
    DMA_Cmd(DMA1_Channel4, ENABLE);  
#endif

#if STM32F0_UART2_DMA_ENABLE
    /* ?? DMA ????4*/
    DMA_Cmd(DMA1_Channel7, ENABLE); 
#endif
    status = E_STM32F0_USART_DRIVER_OK;

    return status;    
}

/******************************************************************
*º¯ÊýÃû³Æ:DMA1_Channel2_3_IRQHandler
*¹¦ÄÜÃèÊö:´¦ÀíDMA·¢ËÍÖÐ¶Ï
*ÊäÈë²ÎÊý:ÎÞ
*Êä³ö²ÎÊý:
*·µ»ØÖµ:ÎÞ
*ÆäËüËµÃ÷:DMAchannel2_3 ÖÐ¶Ï´¦Àí£¬´®¿Ú1Ê¹ÓÃchannel2ºÍ3
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/25                              zhanghaifeng  we015
******************************************************************/
#if STM32F0_UART2_DMA_ENABLE
//void DMA1_Channel2_3_IRQHandler(void)
//{ 
//
//}
void DMA1_Channel6_IRQHandler(void)
{ 
}
void DMA1_Channel7_IRQHandler(void)
{ 
}
#endif

/******************************************************************
*º¯ÊýÃû³Æ:DMA1_Channel4_5_IRQHandler
*¹¦ÄÜÃèÊö:´¦ÀíDMA·¢ËÍÖÐ¶Ï
*ÊäÈë²ÎÊý:ÎÞ
*Êä³ö²ÎÊý:
*·µ»ØÖµ:ÎÞ
*ÆäËüËµÃ÷:DMAchannel4_4 ÖÐ¶Ï´¦Àí£¬´®¿Ú1Ê¹ÓÃchannel4ºÍ5
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/25                              zhanghaifeng  we015
******************************************************************/
#if STM32F0_UART1_DMA_ENABLE
void DMA1_Channel4_5_IRQHandler(void)
{ 
}
#endif

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0UDMASendComplete
*¹¦ÄÜÃèÊö:DMA·¢ËÍÍê³Éºó´ò¿ªDMA½ÓÊÕ¹¦ÄÜ
*ÊäÈë²ÎÊý:ePortNum  ´®¿ÚºÅ
*Êä³ö²ÎÊý:
*·µ»ØÖµ:
E_STM32F0_USART_DRIVER_OK    ³É¹¦DMAÒÑ½«Êý¾Ý·¢ËÍÍê±Ï£¬²¢¿ªÊ¼´ò¿ª½ÓÊÕÄ£Ê½
E_STM32F0_USART_DRIVER_SEND_COMPLETE_PARA_ERR    Èë²Î´íÎó
E_STM32F0_USART_DRIVER_SEND_COMPLETE_ERR ·¢ËÍÍê³ÉÊ§°Ü£¬´®¿Ú»¹Î´Íê³É·¢ËÍ
*ÆäËüËµÃ÷:¸Ãº¯ÊýÐèÒªÖ÷º¯ÊýÖ÷¶¯µ÷ÓÃ£¬Ö÷ÒªÊÇÎªÁË´ò¿ª½ÓÊÕÍ¨µÀ
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/25                              zhanghaifeng  we015
******************************************************************/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0UDMASendComplete(Port_Num_e ePortNum)
{ 
    USART_TypeDef *USARTx = NULL;
   
    if (E_STM32F0_USART_PORT_ONE == ePortNum) 
    {                               
        USARTx = USART1;            
    } 
    else if (E_STM32F0_USART_PORT_TWO == ePortNum)    
    {                           
        USARTx = USART2;       
    }
    else
    {
         return E_STM32F0_USART_DRIVER_SEND_COMPLETE_PARA_ERR;
    }
    
#if STM32F0_UART1_DMA_ENABLE      
    /*ÉèÖÃDMA½ÓÊÕ»º´æ*/
    DMA_Cmd(DMA1_Channel5, DISABLE);
    /*¸øCNDTR¼Ä´æÆ÷¸³Öµ*/
    DMA_SetCurrDataCounter(DMA1_Channel5, DMA_RECEIVE_BUFSIZE); 
    //DMA1_Channel5->CMAR = (uint32_t)gucUSART1DMARx;
    DMA_Cmd(DMA1_Channel5, ENABLE);
#endif

#if STM32F0_UART2_DMA_ENABLE   
    /*??DMA????*/
    DMA_Cmd(DMA1_Channel6, DISABLE);

    /*?CNDTR?????*/
    DMA_SetCurrDataCounter(DMA1_Channel6, DMA_RECEIVE_BUFSIZE); 
    //DMA1_Channel5->CMAR = (uint32_t)gucUSART2DMARx;
    DMA_Cmd(DMA1_Channel6, ENABLE);
#endif

#if STM32F0_UART1_DMA_ENABLE  
    /*Ê¹ÄÜ´®¿Ú½ÓÊÕÍ¨µÀ*/
    DMA_Cmd(DMA1_Channel5, ENABLE);
#endif

#if STM32F0_UART2_DMA_ENABLE  
    /*????????*/
    DMA_Cmd(DMA1_Channel6, ENABLE);
#endif
    /*Ê¹ÄÜ´®¿Ú*/
    USART_Cmd(USARTx, ENABLE);

    return E_STM32F0_USART_DRIVER_OK;
}

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USARTDriverReceive
*¹¦ÄÜÃèÊö:½ÓÊÕ´®¿ÚÊý¾Ýµ½ÉÏ²ã
*ÊäÈë²ÎÊý:
         ePortNum  ´®¿ÚºÅ
         pucReceiveBuffer   ½ÓÊÕ»º´æ£¬ÓÉÒµÎñ·ÖÅä
         pusReceiveLength   ½ÓÊÕ³¤¶ÈÖ¸Õë
*Êä³ö²ÎÊý:
*·µ»ØÖµ:
            E_STM32F0_USART_DRIVER_OK  ³É¹¦
            E_STM32F0_USART_DRIVER_RECEIVE_PARA_ERR ²ÎÊý´íÎó
            E_STM32F0_USART_DRIVER_RECEIVE_ERR ½ÓÊÕÊ§°Ü

*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/26                              zhanghaifeng  we015
******************************************************************/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverReceive(Port_Num_e ePortNum, 
                                        uint8_t *pucReceiveBuffer, uint16_t *pusReceiveLength)
{
    uint16_t NunmberCndtr = 0;
    
    /*ÅÐ¶ÏePortNumÊÇ·ñ´íÎó*/
    if (ePortNum > E_STM32F0_USART_PORT_TWO)
    {
        return E_STM32F0_USART_DRIVER_RECEIVE_PARA_ERR;
    } 
    
    /*ÅÐ¶Ï·¢ËÍ»º´æÇøµØÖ·Îª¿Õ*/
    if (NULL == pucReceiveBuffer)
    {
        return E_STM32F0_USART_DRIVER_RECEIVE_PARA_ERR;
    }
    
    /*ÅÐ¶Ï½ÓÊÕÊý¾Ý³¤¶ÈÊÇ·ñÎª¿Õ*/
    if (NULL == pusReceiveLength)
    {
        return E_STM32F0_USART_DRIVER_RECEIVE_PARA_ERR;
    }
    
    /*ÅÐ¶ÏÈ«¾Ö±äÁ¿ucIsReceivedDataÊÇ·ñ±»ÖÃÎ»(ÖÃÎ»Ôò½øÐÐÏÂÃæÊý¾Ý½ÓÊÕ)*/
    if (STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF == ucIsReceivedData)
    {
        return E_STM32F0_USART_DRIVER_RECEIVE_ERR;
    }
    
#if STM32F0_UART1_DMA_ENABLE
    /*»ñÈ¡£Ä£Í£Á½ÓÊÕ×Ö½Ú*/
    NunmberCndtr = DMA_GetCurrDataCounter(DMA1_Channel5);
    *pusReceiveLength = DMA_RECEIVE_BUFSIZE - NunmberCndtr;

    /*¿½±´Êý¾Ýµ½pucReceiveBuffer*/
    memcpy(pucReceiveBuffer, gucUSART1DMARx, *pusReceiveLength);
#endif

#if STM32F0_UART2_DMA_ENABLE
    /*??DMA????*/
    NunmberCndtr = DMA_GetCurrDataCounter(DMA1_Channel6);
    *pusReceiveLength = DMA_RECEIVE_BUFSIZE - NunmberCndtr;

    /*¿½±´Êý¾Ýµ½pucReceiveBuffer*/
    memcpy(pucReceiveBuffer, gucUSART2DMARx, *pusReceiveLength);
#endif    
    /*ÖÃÎ»Ê¹ucIsReceivedDataÎª0*/
//    ucIsReceivedData = STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF;

    return E_STM32F0_USART_DRIVER_OK;
}

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USARTDriverReceivePointer
*¹¦ÄÜÃèÊö:½ÓÊÕ´®¿ÚÊý¾Ýµ½ÉÏ²ã
*ÊäÈë²ÎÊý:
                            ePortNum  ´®¿ÚºÅ
                            pucReceiveBuffer   ½ÓÊÕ»º´æÖ¸Õë
                            pusReceiveLength   ½ÓÊÕ³¤¶ÈÖ¸Õë
*Êä³ö²ÎÊý:
*·µ»ØÖµ:
                            E_STM32F0_USART_DRIVER_OK  ³É¹¦
                            E_STM32F0_USART_DRIVER_RECEIVE_POINTER_PARA_ERR ²ÎÊý´íÎó
                            E_STM32F0_USART_DRIVER_RECEIVE_POINTER_ERR ½ÓÊÕÖ¸ÕëÊ§°Ü
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/26                              zhanghaifeng  we015
******************************************************************/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverReceivePointer(Port_Num_e ePortNum, 
                                        uint8_t **pucReceiveBuffer, uint16_t *pusReceiveLength)
{
    uint16_t NunmberCndtr = 0;   

    /*ÅÐ¶Ï½ÓÊÕ»º´æÇøµØÖ·Îª¿Õ*/
    if (NULL == pucReceiveBuffer)
    {
        return E_STM32F0_USART_DRIVER_RECEIVE_POINTER_PARA_ERR;
    }

    /*ÅÐ¶Ï½ÓÊÕÊý¾Ý³¤¶ÈÊÇ·ñÎª¿Õ*/
    if (NULL == pusReceiveLength)
    {
        return E_STM32F0_USART_DRIVER_RECEIVE_POINTER_PARA_ERR;
    }
    
    /*ÅÐ¶ÏePortNumÊÇ·ñ´íÎó*/
    if (ePortNum > E_STM32F0_USART_PORT_TWO)
    {
        return E_STM32F0_USART_DRIVER_RECEIVE_POINTER_PARA_ERR;
    }  
    
    /*ÅÐ¶ÏÈ«¾Ö±äÁ¿ucIsReceivedDataÊÇ·ñ±»ÖÃÎ»(ÖÃÎ»Ôò½øÐÐÏÂÃæÊý¾Ý½ÓÊÕ)*/    
    if (STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF == ucIsReceivedData)
    {
        return E_STM32F0_USART_DRIVER_RECEIVE_POINTER_ERR;
    }
    
#if STM32F0_UART1_DMA_ENABLE 
    /*»ñÈ¡£Ä£Í£Á½ÓÊÕ×Ö½Ú*/
    NunmberCndtr = DMA_GetCurrDataCounter(DMA1_Channel5);
    *pusReceiveLength = DMA_RECEIVE_BUFSIZE - NunmberCndtr;

    /*Ö¸Õë¸³¸øpucReceiveBuffer*/
    *pucReceiveBuffer = gucUSART1DMARx;
#endif

#if STM32F0_UART2_DMA_ENABLE  
    /*??DMA????*/
    NunmberCndtr = DMA_GetCurrDataCounter(DMA1_Channel6);
    *pusReceiveLength = DMA_RECEIVE_BUFSIZE - NunmberCndtr;

    /*Ö¸Õë¸³¸øpucReceiveBuffer*/
    *pucReceiveBuffer = gucUSART2DMARx;
#endif

    /*ÖÃÎ»Ê¹ucIsReceivedDataÎª0*/
    //ucIsReceivedData = STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF;

    return E_STM32F0_USART_DRIVER_OK;
}

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USARTDriverClose
*¹¦ÄÜÃèÊö:¹Ø±ÕÖ¸¶¨´®¿Ú
*ÊäÈë²ÎÊý:ePortNum  ´®¿ÚºÅ
*Êä³ö²ÎÊý:
*·µ»ØÖµ:
                            E_STM32F0_USART_DRIVER_OK  ³É¹¦
                            E_STM32F0_USART_DRIVER_CLOSE_PARA_ERR ²ÎÊý´íÎó
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/26                              zhanghaifeng  we015
******************************************************************/
STM32F0_USART_DRIVER_ERROR_CODE_e STM32F0USARTDriverClose(Port_Num_e ePortNum)
{ 
    USART_TypeDef *USARTx = NULL;
    
    if (E_STM32F0_USART_PORT_ONE == ePortNum) 
    {                               
        USARTx = USART1;            
    }     
    else if (E_STM32F0_USART_PORT_TWO == ePortNum)    
    {                           
        USARTx = USART2;       
    }
    else
    {
         return E_STM32F0_USART_DRIVER_CLOSE_PARA_ERR;
    }   
    
    /*¹Ø±Õ´®¿Ú*/
    USART_Cmd(USARTx, DISABLE); 
    
    return E_STM32F0_USART_DRIVER_OK;
}

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USART1DriverISR
*¹¦ÄÜÃèÊö:´¦Àí´®¿ÚÖÐ¶Ï
*ÊäÈë²ÎÊý:ÎÞ
*Êä³ö²ÎÊý:
*·µ»ØÖµ:ÎÞ
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/26                              zhanghaifeng  we015
******************************************************************/
extern uint8_t indicatorFlashState [5];
void USART2_IRQHandler(void)
{
      uint16_t usSendCrc16Value = 0;
    
#ifdef SCAN_HMODBUS_OPTIMIZE
#if FR4XX4_CODE|| (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)||(IO_CODE_TYPE == FR4104_CODE)
    uint8_t ucCountInd;
    uint16_t usTemp;/*AO´óÐ¡¶Ë×ª»»ÁÙÊ±Êý¾Ý*/
#endif
#endif
    if ( (SET == USART_GetITStatus(USART2, USART_IT_RTO)))
    {
        /*???????????*/
        if (DMA_RECEIVE_BUFSIZE == DMA1_Channel6->CNDTR)
        {
            /*  ????????*/
            USART2->ICR = USART_FLAG_RTO;
            return;
        }
         
         DMA1_Channel6->CCR &= DMA_CCR_DISEN;
        USART2->CR1 &= USART_CR1_DISUE;
        USART2->ICR = USART_FLAG_RTO;
        
//        /*??????*/        
//        DMA_Cmd(DMA1_Channel6, DISABLE);
//        USART_Cmd(USART2, DISABLE);
//        /*  ????????*/
//        USART_ClearFlag(USART2, USART_FLAG_RTO);

        /* ?????????????????,???? */
        if ((gucStationNum != gucUSART2DMARx[0])
            && (SLAVE_9_MODE_ADDR != gucUSART2DMARx[0]))
        {
            (void)STM32F0UDMASendComplete(E_STM32F0_USART_PORT_TWO);
           // DMA_Cmd(DMA1_Channel6, ENABLE);
           // USART_Cmd(USART2, ENABLE);
            return;
        }

        /* ???????????? */
        if (SLAVE_9_MODE_ADDR == gucUSART2DMARx[0])
        {
            if (IO_STATE_INITIALIZED == geInitState)
            {
                (void)STM32F0UDMASendComplete(E_STM32F0_USART_PORT_TWO);
                /* ?????????,??? */    
                if (gucStationNum != gucUSART2DMARx[2])
                {
                    return;
                }
                /*????,??????4*/
                DMA_Cmd(DMA1_Channel7, DISABLE);
                /*???DMA??*/
                DMA1_Channel7->CMAR=(uint32_t)gusUSART2DMATx;
                /*  ???????,???????????,???????? */
                uint8_t  ucSendBuffer[BUFFER_LEN];
                memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
                (void)PacketAndSendResponse(ucSendBuffer,HANDLE_SUCCESS);

                return;
            }
        }
        /*H-ModbusÒµÎñÊý¾ÝÏìÓ¦Ö¡ÓÅ»¯*/
#ifdef SCAN_HMODBUS_OPTIMIZE

         /*ÉÏÐÐÒµÎñÊý¾Ý´¦Àí*/
        if( gucUSART2DMARx[1] == gucaUpServiceDataRequFrame[1] /*¹¦ÄÜÂë¼ìÑé*/
            && (*((uint16_t *)(gucUSART2DMARx+2))) == (*((uint16_t *)(gucaUpServiceDataRequFrame+2))))
        {
            /*ÏòÖ÷Õ¾·¢ËÍÔ¤ÏÈ×é×°ºÃµÄÏìÓ¦Ö¡*/
            /*ÉèÖÃDMA½ÓÊÕbuffer´óÐ¡*/
            DMA1_Channel6->CCR &= DMA_CCR_DISEN;
            DMA1_Channel6->CNDTR = DMA_RECEIVE_BUFSIZE;
            DMA1_Channel6->CCR |= DMA_CCR_EN;
            /*Ê¹ÄÜ´®¿Ú*/
            USART2->CR1 |= USART_CR1_UE;
            /*disable DMA*/
            DMA1_Channel7->CCR &= DMA_CCR_DISEN;
            /*ÒµÎñÊý¾ÝÏìÓ¦Ö¡(Ö÷Ñ­»·)×é×°Íê³É,½«ÆäÄÚÈÝ¿½±´ÖÁÏìÓ¦Ö¡(ÖÐ¶Ï)*/
                //memcpy(gusaUpServiceDataRespFrameUSART,gusaUpServiceDataRespFrameMainLoop,(UP_RESPONSE_FRAME_LENGTH+1)*2);
                gucaUpServiceDataRespFrameMainLoop[3] = IOLinkEventInfo[0];
                gucaUpServiceDataRespFrameMainLoop[4] = IOLinkEventInfo[1];
                gucaUpServiceDataRespFrameMainLoop[5] = IOLinkEventInfo[2];
                gucaUpServiceDataRespFrameMainLoop[6] = IOLinkEventInfo[3];
                
//                printf("gucaUpServiceDataRespFrameMainLoop[3] = %d\n", gucaUpServiceDataRespFrameMainLoop[3]);
//                printf("gucaUpServiceDataRespFrameMainLoop[4] = %d\n", gucaUpServiceDataRespFrameMainLoop[4]);
//                printf("gucaUpServiceDataRespFrameMainLoop[5] = %d\n", gucaUpServiceDataRespFrameMainLoop[5]);
//                printf("gucaUpServiceDataRespFrameMainLoop[6] = %d\n", gucaUpServiceDataRespFrameMainLoop[6]);
   
                swapEndian(gucaUpServiceDataRespFrameMainLoop+3, IO_LINK_UP_BYTE_COUNT);
              
                /*¼ÆËãCRCÐ£Ñé*/
                usSendCrc16Value = usMBSCRC16(( const uint8_t * )gucaUpServiceDataRespFrameMainLoop,UP_RESPONSE_FRAME_LENGTH - CRC16_VALUE_LENGTH);
                memcpy(gucaUpServiceDataRespFrameMainLoop+3+IO_LINK_UP_BYTE_COUNT,&usSendCrc16Value,CRC16_VALUE_LENGTH);
                
                gusaUpServiceDataRespFrameUSART[0] = 0XFE|0x100;
                for (uint8_t i = 0; i < UP_RESPONSE_FRAME_LENGTH; i++)
                {
                  gusaUpServiceDataRespFrameUSART[i+1] = gucaUpServiceDataRespFrameMainLoop[i];
                }

   
            gucServDataFraPackageOK = E_DATA_SYNC_STATUS_HOLD;
            
            /*ÉèÖÃDMA ·¢ËÍbuffer*/
            /*ÒµÎñÊý¾ÝÏìÓ¦Ö¡(Ö÷Ñ­»·)Î´×é×°Íê³É*/
            DMA1_Channel7->CMAR=(uint32_t)gusaUpServiceDataRespFrameUSART;
            /*ÉèÖÃDMA·¢ËÍ»º´æ´óÐ¡*/ 
            DMA1_Channel7->CNDTR = UP_RESPONSE_FRAME_LENGTH+1;
            DMA_ClearFlag(DMA1_FLAG_GL6);
            
            DMA_ClearFlag(DMA1_FLAG_GL7);
            /* ¨º1?¨¹ DMA ¨ª¡§¦Ì¨¤4*/
            DMA1_Channel7->CCR |= DMA_CCR_EN;
        }       
        /*ÏÂÐÐÒµÎñÊý¾Ý´¦Àí*/
        else if(MODBUS_OPTIMIZE_CRC16_NOERR ==  usMBSCRC16(( const uint8_t * )gucUSART2DMARx,REQUEST_FRAME_LENGTH)/*CRC¼ìÑé*/
            && WRITE_COIL_FUNC_CODE == gucUSART2DMARx[1]    /*¹¦ÄÜÂë¼ìÑé*/
            && COIL_START_ADDRESS == *((uint16_t *)(gucUSART2DMARx+2))) /*ÆðÊ¼µØÖ·¸ß×Ö½Ú¼ìÑé*/
        {
            /*ÏòÖ÷Õ¾·¢ËÍÔ¤ÏÈ×é×°ºÃµÄÏìÓ¦Ö¡*/
            /*ÉèÖÃDMA½ÓÊÕbuffer´óÐ¡*/
            DMA1_Channel6->CCR &= DMA_CCR_DISEN;
            DMA1_Channel6->CNDTR = DMA_RECEIVE_BUFSIZE;
            DMA1_Channel6->CCR |= DMA_CCR_EN;
            /*????*/
            USART2->CR1 |= USART_CR1_UE;
            /*disable DMA*/
            DMA1_Channel7->CCR &= DMA_CCR_DISEN;
            /*ÅÐ¶ÏÊÇ·ñ´æÔÚ¸æ¾¯*/
            if(IO_NO_WARN != gucaWarnData[0])
            {
                /*ÓÐ¸æ¾¯£¬ÉèÖÃDMA ·¢ËÍbuffer*/
                DMA1_Channel7->CMAR=(uint32_t)gusaServiceDataRespFrameWarn;
            }
            else
            {
                /*ÎÞ¸æ¾¯£¬ÉèÖÃDMA ·¢ËÍbuffer*/
                DMA1_Channel7->CMAR=(uint32_t)gusaServiceDataRespFrame;  
            }
            /*ÉèÖÃDMA·¢ËÍ»º´æ´óÐ¡*/
            DMA1_Channel7->CNDTR = RESPONSE_FRAME_LENGTH+1;
            /* Ê¹ÄÜ DMA Í¨µÀ4*/
            DMA1_Channel7->CCR |= DMA_CCR_EN;
            if (E_DATA_SYNC_STATUS_MAINLOOP_HANDLING !=gucDownServDataAnalogUpdateOK)
            {
                memcpy(gusaAnalogData,gucUSART2DMARx+7,AO_DATA_BYTE_COUNT);
                    /*´óÐ¡¶Ë×ª»»*/
                for(ucCountInd = 0; ucCountInd < CHANNEL_COUNT; ++ucCountInd )
                {
                    usTemp = gusaAnalogData[ucCountInd];
                    *((int8_t *)gusaAnalogData +ucCountInd*2) = (uint8_t)(usTemp >> 8);
                    *((int8_t *)gusaAnalogData +ucCountInd*2 + 1) = (uint8_t)(usTemp & 0x00FF);
                }
                
//                for (uint8_t i = 0; i < 8; i++)
//                {
//                    printf("gusaAnalogData[%d] = %d\r\n", i, gusaAnalogData[i]);
//                }
                
                //printf("Signal = 0x%04X\n", gusaAnalogData[4]);
                
                /* ¸ü¸ÄAOÊý¾ÝË¢ÐÂ±êÖ¾ */
                gucAoDataIsChanged = AO_DATA_CHANGED;
                /*ÖÃÎª¸üÐÂÌ¬*/
                gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_UPDATED;
            }
            
            gucDisConnectionFlag = 0;
            /*ÆôÓÃAOÓëÍø¹Ø¶ÏÁ¬µÄ¶¨Ê±Æ÷*/
            if (0  == ucIsDisconnectCheckTimerStart)
            {
                TIM_Cmd( TIM3, ENABLE );
                ucIsDisconnectCheckTimerStart = TIMER_ON;
            }
            /*ÖØÖÃ¶¨Ê±Æ÷*/
            TIM_SetCounter(TIM3, 0);
                    if (indicatorFlashState[4] == 0)
            {
               GPIO_SetBits(GPIOA,GPIO_Pin_0);
            }
        }

        else
        {
          
            DMA_Cmd(DMA1_Channel7, DISABLE);          
            DMA1_Channel7->CMAR=(uint32_t)gusUSART2DMATx;          
            ucIsReceivedData = STM32F0_USART_RECEIVE_COMPLETE_SWITCH_ON;           
            pfnIntCbDat [E_STM32F0_USART_PORT_TWO] (E_STM32F0_USART_PORT_TWO);
        }
#else 
        /*ÖÃ´®¿Ú½ÓÊÕ±êÖ¾´ò¿ª*/
        ucIsReceivedData = STM32F0_USART_RECEIVE_COMPLETE_SWITCH_ON;
        /*µ÷ÓÃ»Øµ÷º¯Êý*/
        pfnIntCbDat [E_STM32F0_USART_PORT_ONE] (E_STM32F0_USART_PORT_ONE);
#endif
    }
    if ( USART_GetITStatus(USART2, USART_IT_ORE))
    {
        /*Ìí¼ÓÒì³£´¦Àí*/
    }
    if ( USART_GetITStatus(USART2, USART_IT_PE))
    {
        /*Ìí¼ÓÒì³£´¦Àí*/
    }
    if(USART_GetITStatus(USART2, USART_IT_ERR))
    {
        USART_ClearFlag(USART2, USART_IT_ERR);     
    }   
}

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USART1DriverInstallIntCallBack
*¹¦ÄÜÃèÊö:°²×°ST ´®¿ÚÇý¶¯»Øµ÷º¯Êý
*ÊäÈë²ÎÊý:
                            ePortNum    ´®¿ÚºÅ
                            DriverISRCB »Øµ÷º¯ÊýÖ¸Õë
*Êä³ö²ÎÊý:ÎÞ
*·µ»ØÖµ:ÎÞ
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/27                              lwe004
******************************************************************/
static void STM32F0USART1DriverInstallIntCallBack(Port_Num_e ePortNum, 
                                                      STM32F0USARTDriverISRCB DriverISRCB)
{
    /*°²×°»Øµ÷º¯Êý*/
    pfnIntCbDat[ePortNum] = DriverISRCB;
}

/******************************************************************
*º¯ÊýÃû³Æ:STM32F0USARTEnableDE
*¹¦ÄÜÃèÊö:Ê¹ÄÜDEÎ»£¬Ä¿µÄÎªÁËÊ¹RTSÁ÷¹¤×÷
*ÊäÈë²ÎÊý:USARTx     ´®¿Ú
*Êä³ö²ÎÊý:ÎÞ
*·µ»ØÖµ:ÎÞ
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/11/7                              zhanghaifeng  we015
******************************************************************/
void STM32F0USARTEnableDE (USART_TypeDef *USARTx)
{    
    /*Ê¹ÄÜDE*/
    USARTx->CR3 |= 0x4000;
}

/******************************************************************
*º¯ÊýÃû³Æ:
*¹¦ÄÜÃèÊö:
*ÊäÈë²ÎÊý:
*Êä³ö²ÎÊý:
*·µ»ØÖµ:
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/4                              we004 we014
******************************************************************/
void STM32F0TempIAPEarse(void)
{
    uint8_t i = 0;
    asm ("CPSID   I");

    FLASH_Unlock();
    for(i=0;i<8;i++)
    {
        FLASH_ErasePage(0x0800E000 + i * 1024);
    }
    FLASH_Lock();
    FLASH->SR |= 0x20;
   /* ´ò¿ªÖÐ¶Ï */
    asm ("CPSIE   I");
}

/******************************************************************
*º¯ÊýÃû³Æ:
*¹¦ÄÜÃèÊö:
*ÊäÈë²ÎÊý:
*Êä³ö²ÎÊý:
*·µ»ØÖµ:
*ÆäËüËµÃ÷:
*ÐÞ¸ÄÈÕÆÚ    °æ±¾ºÅ   ÐÞ¸ÄÈË    ÐÞ¸ÄÄÚÈÝ
*---------------------------------------------------
*2017/9/4                              we004 we014
******************************************************************/
uint8_t ucWriteBufftemp[64] = {0};
uint8_t STM320TempIAPWrite(uint32_t uiWriteAddr, uint8_t *pucWriteBuff, uint16_t usWriteLength)
{
    uint8_t i = 0;
    uint32_t *puiWriteData = NULL;
    FLASH_Status status = FLASH_COMPLETE;
    uint8_t *pucbuffer = ucWriteBufftemp;

    memcpy(ucWriteBufftemp,pucWriteBuff,sizeof(ucWriteBufftemp));    
    puiWriteData = (uint32_t *)pucbuffer;
    
    asm ("CPSID   I");

    FLASH_Unlock();
    for (i=0;i<16;i++)
    {
        status = FLASH_ProgramWord(uiWriteAddr, *puiWriteData);
        if (status != FLASH_COMPLETE)
        {
            return 1;
        }
        uiWriteAddr = uiWriteAddr + 4;
        puiWriteData = puiWriteData + 1;
    }
    FLASH_Lock();
    /* ´ò¿ªÖÐ¶Ï */
    asm ("CPSIE   I");

    return 0;
}

/*±¸×¢:USART2ÔÝ²»Ê¹ÓÃ*/

void swapEndian(uint8_t *array, int length) 
{
    uint8_t temp = 0;
  
    // È·±£Êý×é³¤¶ÈÖÁÉÙÎª2£¬²¢ÇÒÎªÅ¼Êý
    if (length < 2 || length % 2 != 0) {
        return;
    }

    for (int i = 0; i < length; i += 2) 
    {
        // ½»»»Î»ÖÃ i ºÍ i + 1 µÄÔªËØ
        temp = array[i];
        array[i] = array[i + 1];
        array[i + 1] = temp;
    }
}
