/******************************************************************
*文件名称:  HModbusOptimize.c
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
#include "HModbusOptimize.h"
#include "STM32F0usart.h"
#include "mbscrc.h"
#include "warn.h"
#include "Mbs.h"
#include "STM32F0Ads8331Driver.h"

#ifdef SCAN_HMODBUS_OPTIMIZE
/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/

/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                         全局变量定义                            *
******************************************************************/
extern uint8_t gucStationNum;

uint8_t gucaUpServiceDataRespFrameMainLoop[UP_RESPONSE_FRAME_LENGTH] = {0}; 
uint16_t gusaUpServiceDataRespFrameMainLoop[UP_RESPONSE_FRAME_LENGTH+1] = {0}; 

uint8_t gucaUpServiceDataRespFrameUSART[UP_RESPONSE_FRAME_LENGTH] = {0};
uint16_t gusaUpServiceDataRespFrameUSART[UP_RESPONSE_FRAME_LENGTH+1] = {0};

#if FR4XX4_CODE
/*读取从站业务数据请求帧(无告警)*/
uint8_t gucaServiceDataRespFrame[RESPONSE_FRAME_LENGTH] = {0}; 
uint16_t gusaServiceDataRespFrame[RESPONSE_FRAME_LENGTH+1] = {0}; 

/*读取从站业务数据请求帧(有告警)*/
uint8_t gucaServiceDataRespFrameWarn[RESPONSE_FRAME_LENGTH] = {0}; 
uint16_t gusaServiceDataRespFrameWarn[RESPONSE_FRAME_LENGTH+1] = {0};

uint8_t gucaUpServiceDataRequFrame[RESPONSE_FRAME_LENGTH] = {0};

extern int16_t gusaAnalogData[MAX_AO_CHANNEL_NUMBER];
extern int8_t gusaIOLinkOutputDataMainLoop[MAX_AO_CHANNEL_NUMBER];
uint8_t gucaLittleToBigEdian[AO_DATA_BYTE_COUNT] = {0};
/*下行业务数据(模拟量)更新完成标记完成标记*/
uint8_t gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_BUTT;
uint8_t gucDisConnectionFlag = 0;
#endif

uint8_t gucServDataFraPackageOK = E_DATA_SYNC_STATUS_BUTT;




/******************************************************************
*                          局部函数声明                            *
******************************************************************/

/******************************************************************
*                         全局函数实现                            *
******************************************************************/


/******************************************************************
*函数名称:ServiceDataFramePackage
*功能描述:系统初始完成后，组装业务数据响应帧
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/6/24                              gaojunfeng we035
******************************************************************/
void ServiceDataFramePackage(void)
{
    uint16_t usSendCrc16Value = 0;
    
        /*上行业务数据响应帧组装*/
    gucaUpServiceDataRespFrameMainLoop[0] = gucStationNum;    /*从站站号*/
    gucaUpServiceDataRespFrameMainLoop[1] = 0x03; /*功能码*/
    gucaUpServiceDataRespFrameMainLoop[2] = 0x04; /*响应字节数*/
    
    /*置业务数据响应帧(主循环)未组装完成*/
    gucServDataFraPackageOK = E_DATA_SYNC_STATUS_INIT;
    
    gucaUpServiceDataRespFrameUSART[0] = gucStationNum;    /*从站站号*/
    gucaUpServiceDataRespFrameUSART[1] = 0x03; /*功能码*/
    gucaUpServiceDataRespFrameUSART[2] = 0x04; /*响应字节数*/
    memset(gucaUpServiceDataRespFrameUSART+3,0,IO_LINK_UP_BYTE_COUNT);   /*响应帧(中断)的业务数据初始化为0*/

    usSendCrc16Value = usMBSCRC16(( const uint8_t * )gucaUpServiceDataRespFrameUSART, UP_RESPONSE_FRAME_LENGTH - CRC16_VALUE_LENGTH);
    memcpy(gucaUpServiceDataRespFrameUSART+IO_LINK_UP_BYTE_COUNT+3,&usSendCrc16Value,CRC16_VALUE_LENGTH);
    
    gucaUpServiceDataRequFrame[0] = gucStationNum;    /*从站站号*/
    gucaUpServiceDataRequFrame[1] = 0x03; /*功能码*/
    gucaUpServiceDataRequFrame[2] = 0x03; /*保持寄存器地址高字节*/
    gucaUpServiceDataRequFrame[3] = 0xE9; /*保持寄存器地址低字节*/
    gucaUpServiceDataRequFrame[4] = 0x0;  /*寄存器个数高字节*/
    gucaUpServiceDataRequFrame[5] = 0x02;  /*寄存器个数高字节*/
    
    usSendCrc16Value = usMBSCRC16(( const uint8_t * )gucaUpServiceDataRequFrame, RESPONSE_FRAME_LENGTH - CRC16_VALUE_LENGTH);
    *((uint16_t *)(gucaUpServiceDataRequFrame+6))=usSendCrc16Value;
    
    /*9位模式下发送数据为2字节发送*/
    gusaUpServiceDataRespFrameMainLoop[0] = 0xFE|0x100;
    for (uint8_t i = 0; i < UP_RESPONSE_FRAME_LENGTH; i++)
    {
        gusaUpServiceDataRespFrameMainLoop[i+1] = gucaUpServiceDataRespFrameMainLoop[i];
    }

    gusaUpServiceDataRespFrameUSART[0] = 0xFE|0x100;
    for (uint8_t i = 0; i < UP_RESPONSE_FRAME_LENGTH; i++)
    {
        gusaUpServiceDataRespFrameUSART[i+1] = gucaUpServiceDataRespFrameUSART[i];
    }
    
    
    /*下行业务数据响应帧组装*/
#if FR4XX4_CODE || (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
    /*无告警响应帧组装*/
    gucaServiceDataRespFrame[0] = gucStationNum;    /*从站站号*/
    gucaServiceDataRespFrame[1] = 0x10; /*功能码*/
    gucaServiceDataRespFrame[2] = 0x01; /*寄存器地址高字节*/
    gucaServiceDataRespFrame[3] = 0xF5; /*寄存器地址低字节*/
    gucaServiceDataRespFrame[4] = 0x0;  /*寄存器总数高字节*/
    gucaServiceDataRespFrame[5] = 0x08;  /*寄存器总数低字节*/
    usSendCrc16Value = usMBSCRC16(( const uint8_t * )gucaServiceDataRespFrame, RESPONSE_FRAME_LENGTH - CRC16_VALUE_LENGTH);
    *((uint16_t *)(gucaServiceDataRespFrame+6)) = usSendCrc16Value;
    /*有告警响应帧组装*/
    gucaServiceDataRespFrameWarn[0] = gucStationNum;    /*从站站号*/
    gucaServiceDataRespFrameWarn[1] = 0x50; /*功能码*/
    gucaServiceDataRespFrameWarn[2] = 0x01; /*寄存器地址高字节*/
    gucaServiceDataRespFrameWarn[3] = 0xF5; /*寄存器地址低字节*/
    gucaServiceDataRespFrameWarn[4] = 0x0;  /*寄存器总数高字节*/
    gucaServiceDataRespFrameWarn[5] = 0x08;  /*寄存器总数低字节*/
    usSendCrc16Value = usMBSCRC16(( const uint8_t * )gucaServiceDataRespFrameWarn, RESPONSE_FRAME_LENGTH - CRC16_VALUE_LENGTH);
    *((uint16_t *)(gucaServiceDataRespFrameWarn+6)) = usSendCrc16Value;

    /*9位模式下发送数据为2字节发送*/
    gusaServiceDataRespFrame[0] = 0xFE|0x100;
    for (uint8_t i = 0; i < RESPONSE_FRAME_LENGTH; i++)
    {
       gusaServiceDataRespFrame[i+1] = gucaServiceDataRespFrame[i];
    }

    gusaServiceDataRespFrameWarn[0] = 0xFE|0x100;
    for (uint8_t i = 0; i < RESPONSE_FRAME_LENGTH; i++)
    {
       gusaServiceDataRespFrameWarn[i+1] = gucaServiceDataRespFrameWarn[i];
    }
    gucDownServDataAnalogUpdateOK = E_DATA_SYNC_STATUS_INIT;

#endif
}

/******************************************************************
*函数名称:BigLittleEdianTransfer
*功能描述:业务数据响应帧(主循环)组装函数
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/7/4                              gaojunfeng we035
******************************************************************/
void MainLoopServDataFraPackage(void)
{
    uint16_t usSendCrc16Value = 0;
    
    //swapEndian(gusaIOLinkOutputDataMainLoop, 64);
    //swapEndian(ModuleInfo[ucChannelNum - 1].InputData, 16);
    gucServDataFraPackageOK = E_DATA_SYNC_STATUS_MAINLOOP_HANDLING;
    

    
//    /*判断是否存在告警*/
//    if(IO_NO_WARN != gucaWarnData[0])
//    {
//        gucaUpServiceDataRespFrameMainLoop[1] = FUNC_CODE + WARN_CODE;
//    }
//    else
//    {
//        gucaUpServiceDataRespFrameMainLoop[1] = FUNC_CODE;
//    }

    
    /*置业务数据响应帧(主循环)组装完成*/
    gucServDataFraPackageOK = E_DATA_SYNC_STATUS_UPDATED;
}

#endif

