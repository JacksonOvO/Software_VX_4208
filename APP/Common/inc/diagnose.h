/******************************************************************
*
*文件名称:  diagnose.h
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

#ifndef __DIAGNOSE_H__
#define __DIAGNOSE_H__

/******************************************************************
*                             头文件                              *
******************************************************************/
#include "common.h"
/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/* 除产品信息外的其他诊断数 */
#define COMMON_DIAGNOSE_NUM (2)
/* 产品信息长度 */
#define PRODUCT_INFO_NUMBER (4)
/* 诊断数据定义 */
#define MAX_DIAGNOSE_DATA_NUM   (COMMON_DIAGNOSE_NUM )     /* 最大诊断数 */

#define ADC1_VOLTAGE_MAX            (33000)  /*ADC1 转换电压最大值，3.3V扩大了10000倍*/
#define ADC1_CONVERT_VALUE_MAX      (0x0FFF)  /*ADC1 转换值最大值*/
#define ADC1_V25VSENSE_VALUE         (14300)  /*温度传感器25度时，16通道采样的电压值1.43 *10000*/
#define ADC1_TEMP_AVG_SLOPE          (43)  /*温度曲线平均斜率值0.0043*10000*/
#define ADC1_TEMP_25                  (25)  /*温度25度*/

#define ADC1_EOC_INTERRUPT_ENABLE   (0x0004) /*使能ADC  EOS和EOC转换结束标记*/

#define ADC1_CALIBRATION_NUM            (10) /*ADC1 校准次数, 最少一次*/
#define ADC1_WAIT_START_FLAG_NUM       (10) /* 等待ADC1 开始标记次数*/
#define TEMPERATURE_DIAGNOSE_OFF       (0) /*温度诊断偏移地址*/
#define VOLTAGE_DIAGNOSE_OFF            ( 1 )/*电源电压诊断偏移地址*/
#define KELVIN_TEMPERATURE              (273) /*开尔文温标273.15，约等于273*/

#define ADC_STOP               ( 0 ) /*ADC停止*/
#define ADC_START              ( 1 )/*ADC 开启*/
#define ADC_ScanDirection_Up   ( 0xFFFFFFFB )  /*ADC 通道向前扫描*/
#define ADC_ScanDirection_Back ( 0x00000004 )  /*ADC 通道向后扫描*/
/******************************************************************
*                           数据类型                              *
******************************************************************/
typedef enum
{
    E_ADC_MCU_VOLTAGE_CHANNEL = 0, /*单片机电源电压ADC 采样通道 */
    E_ADC_MCU_TEM_CHANNEL = !E_ADC_MCU_VOLTAGE_CHANNEL, /*单片机内部温度ADC 采样通道 */ 
    E_ADC_MCU_BUTT
}ADCSamplechannel_e;
/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
IO_InterfaceResult_e ADC1_Config( void );
void ChipTempDiagnoseAndwarn( void );

void IODiagnose( void );
void ADC1GetVolTmpDiagnosisData(void);
#endif


