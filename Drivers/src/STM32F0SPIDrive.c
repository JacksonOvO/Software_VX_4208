/******************************************************************
*
*文件名称:  STM32F0SPIDrive.c
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
#include <string.h>
#include "STM32F0SPIDrive.h"
#include "IOOptions.h"
#include "common.h"
#include "Iocontrol.h"
#include <stdio.h>
#ifdef SCAN_HMODBUS_OPTIMIZE
#include "HModbusOptimize.h"
#endif

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/* DI采集通道映射宏,采集到的数据需要循环左移3位 */
#define GET_HIGH_THREE_BIT  (0xE0)
#define SHIFT_TO_THE_LEFT_THREE_BIT  (3)
#define SHIFT_TO_THE_RIGHT_FIVE_BIT  (5)
/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                           全局变量声明                          *
******************************************************************/
uint8_t RxBuffer[RXBUFFSIZE] = {0};                                            /*中断接收全局buffer*/

/* 通道个数*/
extern uint8_t gucChannelCount;
/*实际使用的数字量采集数据存储区大小 */
//extern uint8_t gucaActualSamplingDataFieldSize[MAX_DI_CHANNEL_NUMBER];
/* 数字量采集存储区 */
//extern uint8_t ucSamplingData[MAX_DI_CHANNEL_NUMBER][SAMPLING_DATA_FIELD_SIZE];
//extern uint8_t gucaSamplingDataPos[MAX_DI_CHANNEL_NUMBER];  /* 指示当前采集数据存放位置 */
//extern uint8_t gucaSamplingSum[MAX_DI_CHANNEL_NUMBER];   /* 通道采样值累加和 */
/* DI采集数据保存数据区 */
extern DISampleInfo_t gtaDISampleInfo[MAX_DI_CHANNEL_NUMBER];



extern uint8_t gucaDigitalData[MAX_DI_CHANNEL_NUMBER/DIGITAL_DATA_8_BIT];


/******************************************************************
*                         全局函数声明                            *
******************************************************************/
/******************************************************************
* 函数名称: AD5755_SPI1_Init
* 功能描述: 初始化 SPI1 外设
* 输入参数: 无
* 输出参数: 无
******************************************************************/
void AD5755_SPI1_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct; // GPIO 初始化结构体
    SPI_InitTypeDef  SPI_InitStruct;  // SPI 初始化结构体

    // 1. 使能相关时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);  // PA15（NSS）时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);  // PB3-5（SCK/MISO/MOSI）时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); // SPI1 外设时钟


    // 2. 配置 GPIO 复用功能
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3,  GPIO_AF_5); // PB3 复用为 SPI1_SCK
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4,  GPIO_AF_5); // PB4 复用为 SPI1_MISO
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5,  GPIO_AF_5); // PB5 复用为 SPI1_MOSI

    // 3. 初始化 SPI 引脚为复用功能模式
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;    // 复用功能模式
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   // 推挽输出
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL; // 无上下拉
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // 50MHz 高速

    // 配置 SPI1 引脚：PB3（SCK）、PB4（MISO）、PB5（MOSI）
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 4. 配置 NSS 引脚（软件控制模式）
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;       // PA15 作为 NSS1
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;    // 输出模式
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   // 推挽输出
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;     // 上拉
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // 高速
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_SetBits(GPIOA, GPIO_Pin_15);             // 默认拉高

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;       // PA11 作为 NSS2
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;    // 输出模式
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;   // 推挽输出
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;     // 上拉
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // 高速
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_SetBits(GPIOA, GPIO_Pin_11);             // 默认拉高
    
    // 5. 配置 SPI1 工作模式
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 全双工模式
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;                        // 主模式
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;                    // 8位数据帧
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;                           // 时钟极性：空闲高电平
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;                         // 时钟相位：第二个边沿采样
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;                            // 软件控制 NSS
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // 波特率预分频（系统时钟/16）
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;                   // 高位在前
    SPI_InitStruct.SPI_CRCPolynomial = 0x0;                              // CRC 多项式（未使用）
    SPI_Init(SPI1, &SPI_InitStruct);
    
    SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);

    // 使能 SPI1
    SPI_Cmd(SPI1, ENABLE);
}
    
/******************************************************************
* 函数名称: SPI_Write
* 功能描述: 通过 SPI 发送数据
* 输入参数: slaveDeviceId - 从设备ID（未使用，保留）
*           data          - 发送数据缓冲区
*           bytesNumber   - 发送字节数
******************************************************************/
void SPI_Write(uint8_t slaveDeviceId, uint8_t* data, uint8_t bytesNumber)
{
    //printf("SPI_Write\n");
    uint8_t i;

    SET_SYNC_LOW(); // 拉低 SYNC 信号，开始传输

    for (i = 0; i < bytesNumber; i++)
    {
        //delay_us(50);
        // 等待发送缓冲区为空
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

        // 发送一个字节数据
        SPI_SendData8(SPI1, data[i]);
  
        //delay_us(50);
        // 等待接收缓冲区非空（全双工模式，dummy read）
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

        // 读取接收缓冲区（忽略数据，仅维持通信时序）
        (void)SPI_ReceiveData8(SPI1);
    }
    SET_SYNC_HIGH(); // 拉高 SYNC 信号，结束传输
    
    // 调试用：打印发送的字节数据
//    printf("SPI_Write: ");
//    for (int i = 0; i < bytesNumber; i++) {
//        printf("%02X ", data[i]);
//    }
//    printf("\r\n");

}

/******************************************************************
* 函数名称: SPI_Write2
* 功能描述: 通过 SPI 发送数据
* 输入参数: slaveDeviceId - 从设备ID（未使用，保留）
*           data          - 发送数据缓冲区
*           bytesNumber   - 发送字节数
******************************************************************/
void SPI_Write2(uint8_t slaveDeviceId, uint8_t* data, uint8_t bytesNumber)
{
    //printf("SPI_Write\n");
    uint8_t i;

    SET_SYNC2_LOW(); // 拉低 SYNC 信号，开始传输

    for (i = 0; i < bytesNumber; i++)
    {
        //delay_us(50);
        // 等待发送缓冲区为空
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

        // 发送一个字节数据
        SPI_SendData8(SPI1, data[i]);
  
        //delay_us(50);
        // 等待接收缓冲区非空（全双工模式，dummy read）
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

        // 读取接收缓冲区（忽略数据，仅维持通信时序）
        (void)SPI_ReceiveData8(SPI1);
    }
    SET_SYNC2_HIGH(); // 拉高 SYNC 信号，结束传输
    
    // 调试用：打印发送的字节数据
//    printf("SPI_Write: ");
//    for (int i = 0; i < bytesNumber; i++) {
//        printf("%02X ", data[i]);
//    }
//    printf("\r\n");

}

/******************************************************************
* 函数名称: SPI_Read
* 功能描述: 通过 SPI 读取数据
* 输入参数: slaveDeviceId - 从设备ID（未使用，保留）
*           readBuffer    - 接收数据缓冲区
*           bytesNumber   - 读取字节数
******************************************************************/
void SPI_Read(uint8_t slaveDeviceId, uint8_t* readBuffer, uint8_t bytesNumber)
{
    uint8_t i;

    SET_SYNC_LOW(); // 拉低 SYNC 信号，开始传输

    for (i = 0; i < bytesNumber; i++)
    {
        // 等待发送缓冲区为空
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

        // 发送 dummy 数据（触发从设备返回数据）
        SPI_SendData8(SPI1, readBuffer[i]);

        // 等待接收缓冲区非空
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

        // 读取接收数据并存储到缓冲区
        readBuffer[i] = SPI_ReceiveData8(SPI1);
    }

    SET_SYNC_HIGH(); // 拉高 SYNC 信号，结束传输

}

void SPI_Read2(uint8_t slaveDeviceId, uint8_t* readBuffer, uint8_t bytesNumber)
{
    uint8_t i;

    SET_SYNC2_LOW(); // 拉低 SYNC 信号，开始传输

    for (i = 0; i < bytesNumber; i++)
    {
        // 等待发送缓冲区为空
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

        // 发送 dummy 数据（触发从设备返回数据）
        SPI_SendData8(SPI1, readBuffer[i]);

        // 等待接收缓冲区非空
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

        // 读取接收数据并存储到缓冲区
        readBuffer[i] = SPI_ReceiveData8(SPI1);
    }

    SET_SYNC2_HIGH(); // 拉高 SYNC 信号，结束传输

}


