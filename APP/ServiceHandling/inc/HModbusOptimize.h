/******************************************************************
*文件名称:  HModbusOptimize.h
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

#ifndef __HMODBUSOPTIMIZE_H__
#define __HMODBUSOPTIMIZE_H__


/******************************************************************
*                             头文件                              *
******************************************************************/
#include "common.h"
#include "IOOptions.h"

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/

/*写线圈功能码*/
#define WRITE_COIL_FUNC_CODE ((uint8_t)0x10)
/*写线圈起始地址*/
#define COIL_START_ADDRESS ((uint16_t)0xF501)
/*响应帧长度定义*/
#define RESPONSE_FRAME_LENGTH (8)
/*请求帧长度定义*/
#define UP_RESPONSE_FRAME_LENGTH (IO_LINK_UP_BYTE_COUNT+5)
#define REQUEST_FRAME_LENGTH (25)
/*AO通道数*/
#define CHANNEL_COUNT (8)
/*AO数据字节数*/
#define AO_DATA_BYTE_COUNT (CHANNEL_COUNT*2)

#define IO_LINK_UP_BYTE_COUNT  (4)
#define  MODBUS_OPTIMIZE_CRC16_NOERR    (0)

#define FUNC_CODE (0x03)

/*DI、AI模块SPI中断所采集数据处理状态机*/
typedef enum
{
    E_SPI_SAMPLE_DATA_INIT = 0,/*SPI初始化完毕，还未采集数据*/
    E_SPI_SAMPLE_DATA_UPDATED,/*数据更新态*/
    E_SPI_SAMPLE_DATA_HANDING,/*数据处理态*/
    E_SPI_SAMPLE_DATA_HOLD,/*数据保持态*/

    E_SPI_SAMPLE_DATA_BUTT
}SpiSampleDataStatus_e;

typedef enum
{
    E_DATA_SYNC_STATUS_INIT = 0,/*SPI初始化完毕，还未采集数据*/
    E_DATA_SYNC_STATUS_UPDATED,/*数据更新态*/
    E_DATA_SYNC_STATUS_MAINLOOP_HANDLING,/*数据处理态*/
    E_DATA_SYNC_STATUS_HOLD,/*数据保持态*/

    E_DATA_SYNC_STATUS_BUTT
}DataSyncStatus_e;

/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
void ServiceDataFramePackage(void);
void BigLittleEdianTransfer(void);
void MainLoopServDataFraPackage(void);
#endif

