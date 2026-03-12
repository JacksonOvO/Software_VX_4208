/******************************************************************
*
*文件名称:  IoInit.c
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
#include "Ioinit.h"
#include "warn.h"
#include "stm32f0usart.h"
#include "STM32GPIODriver.h"
#include "STM32F0SPIDrive.h"
#include "MbCommunication.h"
#include "MasterCommunication.h"
#include "Iocontrol.h"
#include "STM32F0Ads8331Driver.h"
#include "stm32DAC8555driver.h"
#include "mbscrc.h"
#include "watchdog.h"
#include "stm32f0usartopt.h"
#include "SEGGER_RTT.h"
#define printf(...) SEGGER_RTT_printf(0, __VA_ARGS__)
/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
#define MBS_PORT                ( 2 )
#define MBS_EPARITY            ( MB_PAR_NONE )


#define FLASH_GAINK1_ADD           0x0800DA00
#define FALSH_COMPENSATEB1_ADD     0x0800DA04

#define FLASH_GAINK2_ADD           0x0800DA08
#define FALSH_COMPENSATEB2_ADD     0x0800DA0c

#define FLASH_GAINK3_ADD           0x0800DA10
#define FALSH_COMPENSATEB3_ADD     0x0800DA14

#define FLASH_GAINK4_ADD           0x0800DA18
#define FALSH_COMPENSATEB4_ADD     0x0800DA1c

#define FLASH_GAINK5_ADD           0x0800DA20
#define FALSH_COMPENSATEB5_ADD     0x0800DA24

#define FLASH_GAINK6_ADD           0x0800DA28
#define FALSH_COMPENSATEB6_ADD     0x0800DA2c

#define FLASH_GAINK7_ADD           0x0800DA30
#define FALSH_COMPENSATEB7_ADD     0x0800DA34

#define FLASH_GAINK8_ADD           0x0800DA38
#define FALSH_COMPENSATEB8_ADD     0x0800DA3c


#define FLASH_GAIN_LEN     4

#define StartValue          10502*100000

/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                         全局变量定义                            *
******************************************************************/
/* 从站站号，初始化为站号分配使用的地址 */
uint8_t gucStationNum = SLAVE_9_MODE_ADDR;

/*指示是否接收到一个完整的Modbus帧*/
extern uint8_t ucIsReceivedData;

/* 诊断数据区，比特0表示ID为1的诊断 */
extern uint16_t gusaDiagnoseData[MAX_DIAGNOSE_DATA_NUM];

extern AODemarcate_t gtaAODemarcate[MAX_AI_CHANNEL_NUMBER];

/******************************************************************
*                          局部函数声明                            *
******************************************************************/
static IO_InterfaceResult_e QueryStartInit(UART_REQUEST_TYPE_e *peUARTRequestType, uint8_t *pucRequest);
static IO_InterfaceResult_e SetStationNumer (uint8_t ucStationNumber);
static void ReadDataFromFlash(void);

void STM32F0USARTDriverISRCB2(Port_Num_e ucPort );
static void IoSystemInit(void);

/******************************************************************
*                         全局函数实现                            *
******************************************************************/
/******************************************************************
*函数名称:IoInit
*功能描述:从站初始化
*输入参数:无
*输出参数:无
*返回值:E_IO_ERROR_NONE      无错误，正常返回
                    E_IO_ERROR_IO_USART     初始化串口驱动失败 
                    E_IO_ERROR_IO_FLASH     初始化FLASH驱动失败 
                    E_IO_ERROR_NOT_START_INIT    没有开始初始化，初始化开始指示为假
                    E_IO_ERROR_SET_STATION_NUMBER   设置站号失败 
                    E_IO_ERROR_START_IAP        启动IAP失败 
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/27                              l  we010 
******************************************************************/
IO_InterfaceResult_e  IoInit(xMBSHandle  *xMBSHdl)
{
    STM32F0_USART_DRIVER_ERROR_CODE_e  estatus = E_STM32F0_USART_DRIVER_BUTT;
    IO_InterfaceResult_e  eResults = E_IO_ERROR_NOT_BUTT;
    UART_REQUEST_TYPE_e  eUARTRequestType = E_UART_REQUEST_BUTT;
    Port_Num_e  ePortNum = E_STM32F0_USART_PORT_BUTT;
    USART_InitTypeDef  USART_InitStruct;
    DMA_InitTypeDef  DMA_TXInitStruct;
    DMA_InitTypeDef  DMA_RXInitStruct;
    GPIO_InitTypeDef GPIO_TXInitStruct;
    GPIO_InitTypeDef GPIO_RXInitStruct;
    GPIO_InitTypeDef GPIO_RTSInitStruct;
    eMBErrorCode  estate = MB_EIO;
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT;

    
    uint8_t  ucContent = 0;
    uint8_t  ucSendBuffer[BUFFER_LEN];
    uint8_t ucBitStatus = Bit_SET;

    FUNC_ENTER( );

    /* 初始化系统 */
    IoSystemInit();

    ReadDataFromFlash();
    AIAOIndictorGPIOInit();
    
    STM32DAC5755DriverInit();
    //TIM7_Config();

	//initChannelOutput();
    /* BEGIN: Added by we004, 2017/12/17   PN:从站诊断、配置时，从站处理时间过长，导致Modbus 485通信超时 */
    /* 获取设备描述文件中各部分内容的信息以及长度*/
    eResults = GetIoDeviceContentInfo();
    if (E_IO_ERROR_NONE != eResults)
    {
        return eResults;
    }
  
    /* 设置各类寄存器起始地址 */
    /* 考虑到接收网关请求后需要尽快回响应，
    以防网关串口接收超时，另一方面，回响应后网关
    可能很快发出Modbus请求，IO来不及准备寄存器值，
    将寄存器初始化放在初始化一开始，后续测量IO处理时间，
    可以考虑放入下面的初始化分支 */
    eResults = GetVariousStartAddr();
    if (E_IO_ERROR_NONE != eResults)
    {
        return eResults;
    }
    
    /*新增读FLASH代码*/
        


    memset ( &USART_InitStruct, 0, sizeof ( USART_InitTypeDef ) );
    memset ( &DMA_RXInitStruct, 0, sizeof ( DMA_InitTypeDef ) );
    memset ( &DMA_TXInitStruct, 0, sizeof ( DMA_InitTypeDef ) );
    memset ( &GPIO_TXInitStruct, 0, sizeof ( GPIO_InitTypeDef ) );
    memset ( &GPIO_RTSInitStruct, 0, sizeof ( GPIO_InitTypeDef ) );
    memset ( &GPIO_RXInitStruct, 0, sizeof ( GPIO_InitTypeDef ) );
    memset ( &GPIO_RXInitStruct, 0, sizeof ( GPIO_InitTypeDef ) );
    
    /* 看门狗GPIO初始化 */
    //WatchDogGPIOInit();
    
    /*串口驱动初始化*/
    ePortNum = E_STM32F0_USART_PORT_TWO;
    estatus = STM32F0USARTDriverConfig(ePortNum, &USART_InitStruct, &DMA_RXInitStruct,
                                          &DMA_TXInitStruct, &GPIO_TXInitStruct,
                                          &GPIO_RTSInitStruct, &GPIO_RXInitStruct);
    if (E_STM32F0_USART_DRIVER_OK != estatus)
    {
        return E_IO_ERROR_IO_USART;
    }
    
    estatus = STM32F0USARTDriverInit(ePortNum, &USART_InitStruct, &DMA_RXInitStruct,
                                &DMA_TXInitStruct, &GPIO_TXInitStruct,&GPIO_RTSInitStruct,
                                &GPIO_RXInitStruct,STM32F0USARTDriverISRCB2);    
    if (E_STM32F0_USART_DRIVER_OK != estatus)
    {
        return E_IO_ERROR_IO_USART;
    }
    
    /*关闭串口接收 */ 
   USART_Cmd(USART2, DISABLE);
    /* BEGIN: Modified by lv we004, 2017/12/20 PN: 地址识别电路和
    DO通道初始化导致GPIO通道复用*/    
        
    /* 地址识别电路初始化 */
    STM32AddressRecognition();                                    
    
    //Change_CH_Config();

    DisConnectWithGWTimerInit(DISCONNECT_TIMEOUT);
    
    TIM6_Config();

    /* 接收串口数据，进行站号分配*/
    while (1)
    {
        /*获取是否开始初始化*/
        eResults = QueryStartInit(&eUARTRequestType,&ucContent);
        if (E_IO_ERROR_NONE == eResults)
        {
            break;
        }

        if (E_IO_ERROR_CRC16 == eResults)
        {
            memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
            eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_REQUEST_CRC_ERROR);
            if (E_IO_ERROR_NONE != eResults)
            {
                return eResults;
            }

            /* 置为未接收 */
            ucIsReceivedData = STM32F0_USART_RECEIVE_COMPLETE_SWITCH_OFF;
            /* 串口置为接收 */
            (void)STM32F0UDMASendComplete(E_STM32F0_USART_PORT_TWO);
        }
    }

    /*如果请求类型非法，响应从站站号分配错误响应*/
    if (E_UART_REQUEST_ALLOC_SLAVE_STATION_NUMBER != eUARTRequestType)
    {
        memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_REQ_TYPE_ERR);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
        return E_IO_ERROR_REQ_TYPE_ERROR;
    }
    
    /**如果是站号分配请求，设置站号*/
    eResults = SetStationNumer(ucContent); 
    if (E_IO_ERROR_NONE != eResults)
    {
        memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_SLAVE_NUM_ERR);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
        return E_IO_ERROR_SET_STATION_NUMBER;
    }
    /*初始化modbus485协议栈*/
    estate = eMBSSerialInit( xMBSHdl, MB_RTU, gucStationNum,MBS_PORT, SLAVE_STATION_DEFAULT_COMMCATION_BAUD, MBS_EPARITY);
    if (MB_ENOERR != estate)
    {
        memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_MODBUS485_INIT_ERR);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
        ( void )eMBSClose( *xMBSHdl );
        return E_IO_ERROR_M0DBUS485_INIT;
    }

    /*注册线圈对应的回调函数*/
    estate = eMBSRegisterCoilCB( *xMBSHdl, HandleDigitalData );
    if(MB_ENOERR != estate)
    {
        memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_MODBUS485_INIT_ERR);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
        /*回调函数注册不成功则关闭协议栈*/
        ( void )eMBSClose( *xMBSHdl );
        return E_IO_ERROR_REGIST_COIL_CB;
    }

    /*注册保持寄存器对应的回调函数*/
    estate = eMBSRegisterHoldingCB( *xMBSHdl, HandleAnalogData );
    if(MB_ENOERR != estate)
    {
        memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_MODBUS485_INIT_ERR);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
        /*回调函数注册不成功则关闭协议栈*/
        ( void )eMBSClose( *xMBSHdl );
        return E_IO_ERROR_REGIST_HOLDING_REGISTER_CB;
    }

    /*注册读文件记录对应的回调函数*/
    estate = eMBSReadFileRecordCB( *xMBSHdl, HandleReadFileRecord );
    if(MB_ENOERR != estate)
    {
        memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_MODBUS485_INIT_ERR);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
        /*回调函数注册不成功则关闭协议栈*/
        ( void )eMBSClose( *xMBSHdl );
        return E_IO_ERROR_REGIST_READ_FILE_RECORD_CB;
    }

    /*注册写文件记录对应的回调函数*/
    estate = eMBSWriteFileRecordCB( *xMBSHdl, HandleWriteFileRecord );
    if(MB_ENOERR != estate)
    {
        memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_MODBUS485_INIT_ERR);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
        /*回调函数注册不成功则关闭协议栈*/
        ( void )eMBSClose( *xMBSHdl );
        return E_IO_ERROR_REGIST_WRITE_FILE_RECORD_CB;
    }

    /*如果初始化成功，先先地址识别电路右侧电平*/
    ucBitStatus = GPIO_ReadInputDataBit(ADDRESS_RECOGNITION_RIGHT_GPIO,ADDRESS_RECOGNITION_RIGHT_PIN);

    memset(ucSendBuffer, 0, sizeof(ucSendBuffer));
    /*如果电平为高，组装终端电阻响应*/
    if (Bit_SET == ucBitStatus)
    {
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_VIH);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
    }
    else
    {
        /* 向网关发送成功响应 */
        eResults = PacketAndSendResponse(ucSendBuffer,HANDLE_SUCCESS);
        if (E_IO_ERROR_NONE != eResults)
        {
            return eResults;
        }
    }
    
        /*设置配置信息默认值*/
    SetConfigDefaultValue();
    
    /* 给配置数据区分配空间 */
    eInterfaceResult = InitConfigDataField();
    if (E_IO_ERROR_NONE != eInterfaceResult)
    {
        FUNC_EXIT();
        return eInterfaceResult;
    }

    FUNC_EXIT( );
    return E_IO_ERROR_NONE;
}


/******************************************************************
*函数名称:PacketAndSendResponse
*功能描述:组装并发送响应
*输入参数:ucResultIn :处理结果
                         pucSendbuffIn :发送缓冲区
*输出参数:无
*返回值:E_IO_ERROR_NONE :无错误，正常返回
                    其他:发送失败
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/27                              l  we010 
******************************************************************/
IO_InterfaceResult_e PacketAndSendResponse(uint8_t *pucSendbufferIn,uint8_t ucResultIn)
{
    STM32F0_USART_DRIVER_ERROR_CODE_e estatus = E_STM32F0_USART_DRIVER_BUTT;
    uint8_t *pucsendbuffer = NULL;
    uint16_t usSendCrc16Value = 0;

    FUNC_ENTER( );

    NULL_CHECK(pucSendbufferIn);

    pucsendbuffer = pucSendbufferIn;
    /*组装响应*/
    *pucsendbuffer++ = MASTER_9_MODE_ADDR;/*主站九位模式使用地址*/
    *pucsendbuffer++ = E_UART_RESPONSE_ALLOC_SLAVE_STATION_NUMBER; /*响应类型*/
    *pucsendbuffer++ = ucResultIn; /*处理结果*/

    /*计算CRC16校验值*/
    usSendCrc16Value = usMBSCRC16(( const UBYTE * )&pucSendbufferIn[0], SLAVE_RESP_FRAME_LEN - CRC16_VALUE_LENGTH);
    
    /*CRC校验值添加到发送数据后面两位*/   
    memcpy(&pucSendbufferIn[SLAVE_RESP_FRAME_LEN - CRC16_VALUE_LENGTH], (uint16_t *)&usSendCrc16Value, CRC16_VALUE_LENGTH);

    /*发送响应*/    
    estatus = STM32F0USARTDriverSend(E_STM32F0_USART_PORT_TWO,pucSendbufferIn,SLAVE_RESP_FRAME_LEN);
    if (E_STM32F0_USART_DRIVER_OK != estatus)
    {
        return E_IO_ERROR_NOT_START_INIT;
    }
    
    FUNC_EXIT( );
    return E_IO_ERROR_NONE;
}

/******************************************************************
*                         局部函数实现                            *
******************************************************************/
/******************************************************************
*函数名称:QueryStartInit
*功能描述:获取是否开始初始化
*输入参数: peUARTRequestType请求类型，开始初始化还是开始升级程序
                          pucRequest 请求内容，开始初始化则返回站号，开始升级程序内容待定义
*输出参数:
*返回值:E_IO_ERROR_NONE = 0  无错误，正常返回 
                   E_IO_ERROR_QUERY_START_INIT   没有开始初始化或这没有开始升级 
                   E_IO_ERROR_GW_DEVICE_ID  网关设备ID错误 
                   E_IO_ERROR_REQ_TYPE_ERROR 请求类型错误
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/27                              l  we010 
******************************************************************/
static IO_InterfaceResult_e QueryStartInit(UART_REQUEST_TYPE_e *peUARTRequestType, uint8_t *pucRequest)
{
    uint8_t ucBitStatus = Bit_SET;
    STM32F0_USART_DRIVER_ERROR_CODE_e  estatus = E_STM32F0_USART_DRIVER_BUTT;
    Port_Num_e  ePortNum = E_STM32F0_USART_PORT_BUTT;
    UART_REQUEST_TYPE_e  eUARTRequestType = E_UART_REQUEST_BUTT;
    uint8_t ucRecvBuffer[BUFFER_LEN];
    uint16_t usRecvBufferlen = 0;
    uint16_t usRecvCrc16Value = 0;

    FUNC_ENTER( );
    
    ePortNum = E_STM32F0_USART_PORT_TWO;
    /*等待高电平*/
    while(1)
    {
        /* BEGIN: Added by lv we004, 2017/3/8   PN:watchdog */
        FeedTheWatchDog();
        /* END:   Added by lv we004, 2017/3/8 */
        
        /*获取地址识别电路左侧电平*/
        ucBitStatus = GPIO_ReadInputDataBit(ADDRESS_RECOGNITION_LEFT_GPIO,ADDRESS_RECOGNITION_LEFT_PIN);

#ifdef ST_IAP_TEST
        /*  IAP调试代码 ，其他人不要使用 */
        ucBitStatus = Bit_SET; //l海峰，实际情况由硬件置高，测试时先用软件置高
#endif
        if (Bit_SET == ucBitStatus)
        {   
            /*使能串口接收*/
            USART_Cmd(USART2, ENABLE);
            break;
        }
    }
    /*如果电平为高，读取串口数据*/
    while(1)
    {
        /* BEGIN: Added by lv we004, 2017/3/8   PN:watchdog */
     //   FeedTheWatchDog();
        /* END:   Added by lv we004, 2017/3/8 */

        if (STM32F0_USART_RECEIVE_COMPLETE_SWITCH_ON == ucIsReceivedData)
        {
            break;
        }
    }
    memset(ucRecvBuffer, 0, sizeof(ucRecvBuffer));
    estatus = STM32F0USARTDriverReceive(ePortNum,ucRecvBuffer,&usRecvBufferlen);
    if (E_STM32F0_USART_DRIVER_OK != estatus)
    {
        return E_IO_ERROR_QUERY_START_INIT;
    }
    
    /*计算串口接收数据的CRC16校验值*/
    usRecvCrc16Value = usMBSCRC16(( const UBYTE * )&ucRecvBuffer[0], GW_REQ_FRAME_LEN);
    if (UART_DRIVER_CRC16_NOERR != usRecvCrc16Value)
    {
       return E_IO_ERROR_CRC16;
    }

    /*请求类型*/
    eUARTRequestType = (UART_REQUEST_TYPE_e)ucRecvBuffer[REQ_TYPE_LOCATION];
    *peUARTRequestType = eUARTRequestType;
    /*请求内容*/
    *pucRequest = ucRecvBuffer[SLAVE_NUM_LOCATION];
    
    FUNC_EXIT();
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:SetStationNumer
*功能描述:设置从站站号
*输入参数:ucStationNumber 从站站号
*输出参数:无
*返回值:E_IO_ERROR_NONE = 0    无错误，正常返回 
                   E_IO_ERROR_SET_STATION_NUMBER_REQUEST  从站分配的请求数据错误 
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/27                              l  we010 
******************************************************************/
static IO_InterfaceResult_e SetStationNumer (uint8_t ucStationNumber)
{
    FUNC_ENTER( );
    
    /*判断站号是否合法*/
    if ((ucStationNumber < MIN_STATION_NUMBER) || (ucStationNumber > MAX_STATION_NUMBER))
    {
        return E_IO_ERROR_SET_STATION_NUMBER_REQUEST;
    }                  
    /*如果站号合法，设置从站站号*/
    gucStationNum = ucStationNumber;

    /*站号分配成功后重新设置从站9位模式地址*/
    /*使能串口*/
    USART_Cmd(USART2, DISABLE);
    /*9位模式*/
    USART_AddressDetectionConfig(USART2,USART_AddressLength_7b);
    /*设置从站地址*/
    USART_SetAddress(USART2,gucStationNum);

    USART_MuteModeCmd(USART2, ENABLE);
    USART_MuteModeWakeUpConfig(USART2,USART_WakeUp_AddressMark);
    
     /*使能串口*/
    USART_Cmd(USART2, ENABLE);
     
    FUNC_EXIT( );
    return E_IO_ERROR_NONE;
}

/******************************************************************
*函数名称:
*功能描述:
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/11/28                              l  we010 
******************************************************************/
void STM32F0USARTDriverISRCB2(Port_Num_e ucPort )
{
}

/******************************************************************
*函数名称:IoSystemInit
*功能描述:系统上电后需要统一初始化的部分在这里初始化
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/25                              lv we004
******************************************************************/
static void IoSystemInit(void)
{
    /* 重置SYSCFG registers寄存器 */
    SYSCFG_DeInit();
}

/******************************************************************
*函数名称:void ReadDataFromFlash(void)
*功能描述:从FLASH中读取增益补偿值
*输入参数:void
*输出参数:void
*返回值:     无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/11/20                              panyong we033
******************************************************************/
static void ReadDataFromFlash(void)
{    

/*拷贝数据到用户缓存*/
    memcpy(&gtaAODemarcate[0].lCompensateValue2,(void*)FLASH_GAINK1_ADD ,FLASH_GAIN_LEN );    
    memcpy(&gtaAODemarcate[0].lCompensateValue,(void*)FALSH_COMPENSATEB1_ADD ,FLASH_GAIN_LEN ); //1通道电压补偿
    
    memcpy(&gtaAODemarcate[1].lCompensateValue2,(void*)FLASH_GAINK2_ADD,FLASH_GAIN_LEN );    
    memcpy(&gtaAODemarcate[1].lCompensateValue,(void*)FALSH_COMPENSATEB2_ADD ,FLASH_GAIN_LEN );//2通道电压补偿
    
    memcpy(&gtaAODemarcate[2].lCompensateValue2,(void*)FLASH_GAINK3_ADD,FLASH_GAIN_LEN );    
    memcpy(&gtaAODemarcate[2].lCompensateValue,(void*)FALSH_COMPENSATEB3_ADD ,FLASH_GAIN_LEN );//3通道电压补偿
    
    memcpy(&gtaAODemarcate[3].lCompensateValue2,(void*)FLASH_GAINK4_ADD,FLASH_GAIN_LEN );    
    memcpy(&gtaAODemarcate[3].lCompensateValue,(void*)FALSH_COMPENSATEB4_ADD ,FLASH_GAIN_LEN );//4通道电压补偿
   
    memcpy(&gtaAODemarcate[4].lCompensateValue2,(void*)FLASH_GAINK5_ADD ,FLASH_GAIN_LEN );    
    memcpy(&gtaAODemarcate[4].lCompensateValue,(void*)FALSH_COMPENSATEB5_ADD ,FLASH_GAIN_LEN );//1通道电流补偿
    
    memcpy(&gtaAODemarcate[5].lCompensateValue2,(void*)FLASH_GAINK6_ADD,FLASH_GAIN_LEN );    
    memcpy(&gtaAODemarcate[5].lCompensateValue,(void*)FALSH_COMPENSATEB6_ADD ,FLASH_GAIN_LEN );//2通道电流补偿
    
    memcpy(&gtaAODemarcate[6].lCompensateValue2,(void*)FLASH_GAINK7_ADD,FLASH_GAIN_LEN );    
    memcpy(&gtaAODemarcate[6].lCompensateValue,(void*)FALSH_COMPENSATEB7_ADD ,FLASH_GAIN_LEN );//3通道电流补偿
    
    memcpy(&gtaAODemarcate[7].lCompensateValue2,(void*)FLASH_GAINK8_ADD,FLASH_GAIN_LEN );    
    memcpy(&gtaAODemarcate[7].lCompensateValue,(void*)FALSH_COMPENSATEB8_ADD ,FLASH_GAIN_LEN );//4通道电流补偿
    
        /* 打印数据（每个值4字节，十六进制） */
//    for (int ch = 0; ch < 8; ch++) {
//        printf("CH%d GainValue: ", ch);
//        uint8_t *pGain = (uint8_t*)&gtaAODemarcate[ch].lGainValue;
//        for (int i = 0; i < 4; i++) {
//            printf("%02X ", pGain[i]);
//        }
//        printf("\n");
//
//        printf("CH%d CompensateValue: ", ch);
//        uint8_t *pComp = (uint8_t*)&gtaAODemarcate[ch].lCompensateValue;
//        for (int i = 0; i < 4; i++) {
//            printf("%02X ", pComp[i]);
//        }
//        printf("\n");
//    }

    
//    gtaAODemarcate[0].lGainValue= 0x0001516A;
//    gtaAODemarcate[0].lCompensateValue=0x7FE8;
//    gtaAODemarcate[1].lGainValue= 0x000151B8;
//    gtaAODemarcate[1].lCompensateValue=0x8021;
//    gtaAODemarcate[2].lGainValue= 0x000151B8;
//    gtaAODemarcate[2].lCompensateValue=0x8021;
//    gtaAODemarcate[3].lGainValue= 0x000151B8;
//    gtaAODemarcate[3].lCompensateValue=0x8021;
//    
//    gtaAODemarcate[4].lGainValue= 0x000151B8;
//    gtaAODemarcate[4].lCompensateValue=0x8021;
//    gtaAODemarcate[5].lGainValue= 0x000151B8;
//    gtaAODemarcate[5].lCompensateValue=0x8021;
//    gtaAODemarcate[6].lGainValue= 0x000151B8;
//    gtaAODemarcate[6].lCompensateValue=0x8021;
//    gtaAODemarcate[7].lGainValue= 0x000151B8;
//    gtaAODemarcate[7].lCompensateValue=0x8021;
//    
    
    return;

}



