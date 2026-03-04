/******************************************************************
*
*文件名称:  Ioinit.h
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

#ifndef __IO_INIT_H__
#define __IO_INIT_H__


/******************************************************************
*                             头文件                              *
******************************************************************/
#include <stdio.h>
#include "common.h"
#include "mbs.h"

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
#define MASTER_9_MODE_ADDR  (0xFE)  /*主站九位模式使用地址*/
#define SLAVE_9_MODE_ADDR   (0xFD)  /* 从站九位模式使用地址 */

#define BIT16        (16)

#define REQ_RSP_TYPE_LEN  (1)  /*请求或响应类型长度1字节 */
#define HANDLE_RESULT     (1)   /*处理结果长度1字节 */
#define BUFFER_LEN        (20)  /*接收或发送响应buffer长度*/
#define SLAVE_RESP_FRAME_LEN  (13) /*从站站号分配响应的帧长度*/
#define GW_REQ_FRAME_LEN      (5) /*从站站站号分配网关请求帧长度*/

#define REQ_TYPE_LOCATION  (1)  /*请求类型在请求帧数组的第一字节，从0开始*/
#define SLAVE_NUM_LOCATION  (2) /*从站站号在请求帧数组的第二字节，从0开始*/


/*网关请求处理 结果*/
#define HANDLE_SUCCESS       (0x01)   /*网关请求处理 成功*/
#define HANDLE_VIH            (0x02)  /*网关终端电阻响应*/
#define HANDLE_GWDEVICE_ID_ERR (0x03) /*网关请求处理 失败，网关设备ID错误*/
#define HANDLE_SLAVE_NUM_ERR (0x04)  /*网关请求处理 失败，站号错误*/
#define HANDLE_REQ_TYPE_ERR  (0x05)  /*网关请求处理 失败，请求类型错误*/
#define HANDLE_MODBUS485_INIT_ERR (0x06)  /*Modbus485初始化失败*/
#define HANDLE_REQUEST_CRC_ERROR    (0x07)  /* 请求CRC校验失败 */
/******************************************************************
*                           数据类型                              *
******************************************************************/
typedef enum
{
    E_UART_REQUEST_ALLOC_SLAVE_STATION_NUMBER = 0x01,    /* 从站站号分配请求 */
    E_UART_REQUEST_BUTT
}UART_REQUEST_TYPE_e;

typedef enum
{
    E_UART_RESPONSE_ALLOC_SLAVE_STATION_NUMBER = 0x01,  /* 从站站号分配响应 */
    E_UART_RESPONSE_BUTT
}UART_REPONSE_TYPE_e;

#define delay_50ns()	__NOP(); __NOP(); __NOP(); __NOP();__NOP(); __NOP(); __NOP(); __NOP()
						
                                                       
/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
extern IO_InterfaceResult_e IoInit(xMBSHandle  *xMBSHdl);
IO_InterfaceResult_e PacketAndSendResponse(uint8_t *pucSendbufferIn,uint8_t ucResultIn);
void set_VI_value( uint8_t ch, uint32_t value);
void DAC_Start(void);
void DAC_ShiftOut24Bit(unsigned long int outdata);

#endif


