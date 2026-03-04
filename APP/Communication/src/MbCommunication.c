/******************************************************************
*
*文件名称:  MbCommunication.c
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
#include "IOOptions.h"
#include "MbCommunication.h"
#include "warn.h"
#include "MasterCommunication.h"
#include "Iocontrol.h"
#include "stm32gpiodriver.h"
/*为了使用core_cm0.h 中的系统复位函数
STM32F0usart.h中包含许多头文件，直接包含core_cm0.h 
编译无法通过*/
#include "STM32F0usart.h"
#include "core_cmFunc.h"

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


/* AO数据区 */
#if FR4XX4_CODE
extern int16_t gusaAnalogData[MAX_AO_CHANNEL_NUMBER];
/* AO数据刷新标记，置位表示需要进行输出 */
extern uint8_t gucAoDataIsChanged;
#endif


/* 各类寄存器起始地址存储区 */
uint16_t gusaVariousStartAddr[VARIOUS_START_ADDR_NUM] = {0};

/* 寄存器ID和设备描述文件对应表 */
IoDeviceInfoType_e DeviceInfoAndRegisterIdMap[E_REGISTER_BUTT] = 
{
    //(IoDeviceInfoType_e)0,  /* 设备描述文件总长度*/
    E_IO_DEVICE_INFO_TYPE_BASE, /* 基本信息 */
    E_IO_DEVICE_INFO_TYPE_CONFIG,   /* 配置信息 */
    E_IO_DEVICE_INFO_TYPE_DOWN_DIGTITAL,  /* 下行数字量 */
    E_IO_DEVICE_INFO_TYPE_DOWN_ANALOG,  /* 下行模拟量 */
    E_IO_DEVICE_INFO_TYPE_UP_DIGTITAL,  /* 上行数字量 */
    E_IO_DEVICE_INFO_TYPE_UP_ANALOG,  /* 上行模拟量 */
    E_IO_DEVICE_INFO_TYPE_WARN,     /* 告警信息 */
    E_IO_DEVICE_INFO_TYPE_DIAGNOSIS,    /*诊断信息 */
    /* 其他信息不从设备描述文件获取 */
};

extern uint16_t gusaDiagnoseData[MAX_DIAGNOSE_DATA_NUM];   /* 诊断数据区，比特0表示ID为1的诊断 */
extern uint8_t gucaWarnData[MAX_WARN_DATA_NUM/BIT_NUM_PER_BYTE]; /* 告警数据区，比特0表示ID为1的告警*/
extern uint8_t gucChannelCount; /* 通道个数*/

/* 从站站号，初始化为站号分配使用的地址 */
extern uint8_t gucStationNum;

/* 定时器启用标志*/
uint8_t ucIsDisconnectCheckTimerStart = 0;

/******************************************************************
*                          局部函数声明                            *
******************************************************************/

/******************************************************************
*                         全局函数实现                            *
******************************************************************/
/******************************************************************
*函数名称:HandleDigitalData
*功能描述:线圈回调函数，处理读写线圈，处理上下行数字量数据和告警
*输入参数:
                pubRegBuffer 指向输入PDU，输出数据也填充到这里。
                usAddress 线圈起始地址
                usNRegs 线圈个数
                eRegMode Modbus协议栈定义，读MBS_REGISTER_READ或写MBS_REGISTER_WRITE
*输出参数:
*返回值:
*其它说明:只能从和网关协商的起始地址读取数字量数据
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/1                              lv we004
******************************************************************/
eMBException HandleDigitalData(UBYTE *pubRegBuffer, USHORT usAddress, USHORT usNRegs, eMBSRegisterMode eRegMode)
{
    eMBException eException = MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
    uint8_t ucByteNum = 0;      /* 字节数，1个字节8个线圈 */
    void (*UpgradeProgram)(uint8_t, uint32_t) = (void (*)(uint8_t, uint32_t))(GW_UPGRADE_AREA_START + 1); /* 升级程序指针*/

    FUNC_ENTER();

    /* 入参检查 */
    MODBUS_NULL_CHECK(pubRegBuffer);

    /* 1个字节8个线圈 */
    ucByteNum = usNRegs / BIT_NUM_PER_BYTE;
    if (NUMBER_ZERO != (usNRegs % BIT_NUM_PER_BYTE))
    {
        ucByteNum++;
    }

    switch(usAddress)
    {
    case SOFT_RESET_COIL_START_ADDRESS: /*从站软件复位*/
    {
        /*关闭中断*/
        __disable_irq( );
        /*系统复位*/
        NVIC_SystemReset( );
        break;
    }
    case UPGRADE_COIL_START_ADDRESS:     /* 从站升级指示 */
    {
        /*从站升级*/
        (*UpgradeProgram)(gucStationNum, SLAVE_STATION_DEFAULT_COMMCATION_BAUD);
        break;
    }
    case NEXT_ALLOC_PREPARE_COIL_START_ADDRESS:     /* 下一个从站站号分配准备指示 */
    {
        (void)STM32GPIOInputOutputSwitch(ADDRESS_RECOGNITION_RIGHT_GPIO, ADDRESS_RECOGNITION_RIGHT_PIN, GPIO_Mode_OUT);
        GPIO_WriteBit(ADDRESS_RECOGNITION_RIGHT_GPIO,ADDRESS_RECOGNITION_RIGHT_PIN,Bit_SET);
        break;
    }
    case NEXT_ALLOC_FINISH_COIL_START_ADDRESS:     /* 下一个从站站号分配结束指示*/
    {
        (void)STM32GPIOInputOutputSwitch(ADDRESS_RECOGNITION_RIGHT_GPIO, ADDRESS_RECOGNITION_RIGHT_PIN, GPIO_Mode_OUT);
        GPIO_WriteBit(ADDRESS_RECOGNITION_RIGHT_GPIO,ADDRESS_RECOGNITION_RIGHT_PIN,Bit_RESET);

        break;
    }

    case WARN_DIGITAL_START_ADDR:    /* 告警 */
    {
        if (usNRegs > MAX_WARN_DATA_NUM)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }

        if (MBS_REGISTER_WRITE == eRegMode)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_FUNCTION;
        }
        
        memcpy(pubRegBuffer, gucaWarnData, ucByteNum);
        break;
    }
    default:
    {
        FUNC_EXIT();
        return eException;
    }
    }

    eException = MB_PDU_EX_NONE;

    FUNC_EXIT();

    return eException;
}


/******************************************************************
*函数名称:HandleAnalogData
*功能描述:模拟量数据，诊断数据处理
*输入参数:
pubRegBuffer 指向输入PDU，输出数据也填充到这里。
usAddress 寄存器起始地址
usNRegs 寄存器个数
eRegMode Modbus协议栈定义，读MBS_REGISTER_READ或写MBS_REGISTER_WRITE
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/1                              lv we004
******************************************************************/
eMBException HandleAnalogData(UBYTE *pubRegBuffer, USHORT usAddress, USHORT usNRegs, eMBSRegisterMode eRegMode)
{
    eMBException eException = MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
    uint16_t *pusStartAddr = NULL;      /* 寄存器起始地址 */
    uint16_t i = 0;  /* 循环变量*/
    
    FUNC_ENTER();

    /* 入参检查 */
    MODBUS_NULL_CHECK(pubRegBuffer);

    if (usNRegs > MAX_REGISTER_NUM)
    {
        return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }
    
    /* 寄存器处理 */
    if (usAddress <= DIAGNOSE_ANALOG_INFO_START_ADDR)
    {
        if (MBS_REGISTER_WRITE == eRegMode)
        {
            return MB_PDU_EX_ILLEGAL_FUNCTION;
        }
        
        /* 计算起始地址 */
        eException = CalculateStartAddr(gusaVariousStartAddr, VARIOUS_START_ADDR_NUM, SLAVE_DEVICE_INFO_LENGTH_ADDR, usAddress, usNRegs, &pusStartAddr);
        if (MB_PDU_EX_NONE != eException)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }

        /* 组装数据 */
        for(i = 0 ; i <= usNRegs - 1; i++ )
        {
            *pubRegBuffer++ = ( UBYTE ) ( pusStartAddr[i] >> 8 );
            *pubRegBuffer++ = ( UBYTE ) ( pusStartAddr[i] & 0xFF );
        }

        FUNC_EXIT();

        return MB_PDU_EX_NONE;
    }

#if FR4XX4_CODE || (IO_CODE_TYPE == FR4124_CODE) || (IO_CODE_TYPE == FR4114_CODE)|| (IO_CODE_TYPE == FR4104_CODE)
    /*下行业务数据处理 */
    if ((usAddress >= DOWN_ANALOG_DATA_START_ADDR) && (usAddress < UP_ANALOG_DATA_START_ADDR))
    {
        if (MBS_REGISTER_READ == eRegMode)
        {
            return MB_PDU_EX_ILLEGAL_FUNCTION;
        }
        
         /* 计算起始地址 */
        eException = CalculateStartAddr((uint16_t *)gusaAnalogData, gucChannelCount, DOWN_ANALOG_DATA_START_ADDR, usAddress, usNRegs, &pusStartAddr);
        if (MB_PDU_EX_NONE != eException)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }

        /* 组装数据 */
        for(i = 0 ; i <= usNRegs - 1; i++ )
        {
            pusStartAddr[i] = ( USHORT ) * pubRegBuffer++ << 8;
            pusStartAddr[i] |= ( USHORT ) * pubRegBuffer++;
        }
        
        /* 更改AO数据刷新标志 */
        gucAoDataIsChanged = AO_DATA_CHANGED;

        /*启用AO与网关断连的定时器*/
        if (0  == ucIsDisconnectCheckTimerStart)
        {
            TIM_Cmd( TIM3, ENABLE );
            ucIsDisconnectCheckTimerStart = TIMER_ON;
        }

        /*重置定时器*/
        TIM_SetCounter(TIM3, 0);

        FUNC_EXIT();
        
        return MB_PDU_EX_NONE;
    }
#endif

    /* 诊断数据处理 */
    if ((usAddress >= DIAGNOSE_ANALOG_START_ADDR) && (usAddress < MAX_REGISTER_ADDR))
    {
        if (MBS_REGISTER_WRITE == eRegMode)
        {
            return MB_PDU_EX_ILLEGAL_FUNCTION;
        }
        
         /* 计算起始地址 */
        eException = CalculateStartAddr(gusaDiagnoseData, MAX_DIAGNOSE_DATA_NUM, DIAGNOSE_ANALOG_START_ADDR, usAddress, usNRegs, &pusStartAddr);
        if (MB_PDU_EX_NONE != eException)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }

         /* 组装数据 */
        for(i = 0 ; i <= usNRegs - 1; i++ )
        {
            *pubRegBuffer++ = ( UBYTE ) ( pusStartAddr[i] >> 8 );
            *pubRegBuffer++ = ( UBYTE ) ( pusStartAddr[i] & 0xFF );
        }

        FUNC_EXIT();
        
        return MB_PDU_EX_NONE;
    }

    /* 程序执行到此，说明起始地址错误 */
    FUNC_EXIT();
    return MB_PDU_EX_ILLEGAL_DATA_VALUE;
}


/******************************************************************
*函数名称:HandleReadFileRecord
*功能描述:读文件记录回调函数
*输入参数:
pubFileRecordBuffer 响应PDU 记录数据地址
arxSubRequests 子请求对应的结构体
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/1                              lv we004
******************************************************************/
eMBException HandleReadFileRecord(UBYTE *pubFileRecordBuffer, xMBSFileRecordReq_t *arxSubRequests)
{
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT; 
    UBYTE *pFileRecordAddress = NULL;
    USHORT usRecordLen = 0;   /* 读文件记录记录长度 */
    uint16_t usDeviceBaseInfoLen = 0;
    uint8_t *pucIODeviceInfo = gucaIOPartDeviceInfo;  /* 基本信息存放位置 */
    uint8_t *pucaIoProductInfo = NULL;    /* 存放版本信息的缓冲区 */
    uint16_t usProductInfoLen = 0;  /* 实际读取到的版本信息的长度 */
    uint16_t usAssemblyBaseInfoLength = 0;  /* 组装后的基本信息长度 */
    uint8_t ucRecordNumType;   /*文件记录号类型*/
    uint8_t ucDataTransmitTimeNum;  /*当前传输次数编号*/
    eMBException estatus;

    FUNC_ENTER();

    /* 入参检查 */
    MODBUS_NULL_CHECK(pubFileRecordBuffer);
    MODBUS_NULL_CHECK(arxSubRequests);

    /* 读文件记录使用文件号0  */
    if (MBM_READ_FILE_NUM != arxSubRequests->usFileNumber)
    {
       return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }

    usRecordLen = arxSubRequests->usRecordLength;
    
    /*获取当前文件记录号类型及传输次数编号*/
    estatus = ReadFileRecordAndJudgeFileRecordType(arxSubRequests,&ucRecordNumType,&ucDataTransmitTimeNum);
    if (MB_PDU_EX_NONE != estatus)
    {
       return estatus;
    } 
    
    switch(ucRecordNumType)
    {
    /* 诊断基本信息 */
    case SLAVE_BASE_INFO_RECORD_NUM:
    {     
        /*获取基本信息 */
        usDeviceBaseInfoLen = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_BASE].usContentLength;
        memcpy(pucIODeviceInfo, taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_BASE].pucContentInfo, usDeviceBaseInfoLen);

        /* 获取版本信息 */
        pucaIoProductInfo = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_PRODUCT].pucContentInfo;
        usProductInfoLen = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_PRODUCT].usContentLength;

        /* 组装基本信息和版本信息 */
        eInterfaceResult = AssemblyIoBaseInfo(pucIODeviceInfo, MAX_PART_DEVICE_INFO_LENGTH,
        usDeviceBaseInfoLen,
        pucaIoProductInfo, usProductInfoLen,
        &usAssemblyBaseInfoLength);
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }
        
        pFileRecordAddress = pucIODeviceInfo;
        break;
    }
    
     /* 诊断配置信息 */
    case SLAVE_CONFIG_INFO_RECORD_NUM:
    {
        pFileRecordAddress = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_CONFIG].pucContentInfo + MAX_LENGTH_READ_FILE*(ucDataTransmitTimeNum-1);
        break;
    }
    
    /* 诊断下行数字量 */
    case SLAVE_DOWN_DIGITAL_INFO_RECORD_NUM:
    {
        pFileRecordAddress = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_DOWN_DIGTITAL].pucContentInfo + MAX_LENGTH_READ_FILE*(ucDataTransmitTimeNum-1);
        break;
    }
    
    /* 诊断下行模拟量 */
    case SLAVE_DOWN_ANALOG_INFO_RECORD_NUM:
    {
        pFileRecordAddress = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_DOWN_ANALOG].pucContentInfo + MAX_LENGTH_READ_FILE*(ucDataTransmitTimeNum-1);
        break;
    }

    /* 诊断上行数字量 */
    case SLAVE_UP_DIGITAL_INFO_RECORD_NUM:
    {
        pFileRecordAddress = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_UP_DIGTITAL].pucContentInfo + MAX_LENGTH_READ_FILE*(ucDataTransmitTimeNum-1);
        break;
    }
    
     /* 诊断上行模拟量 */
    case SLAVE_UP_ANALOG_INFO_RECORD_NUM:
    {
        pFileRecordAddress = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_UP_ANALOG].pucContentInfo + MAX_LENGTH_READ_FILE*(ucDataTransmitTimeNum-1);
        break;
    }

    /* 诊断告警信息 */
    case SLAVE_WARN_DIGITAL_INFO_RECORD_NUM:
    {
        pFileRecordAddress = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_WARN].pucContentInfo + MAX_LENGTH_READ_FILE*(ucDataTransmitTimeNum-1);
        break;
    }

    /* 诊断诊断信息 */
    case SLAVE_DIAGNOSE_ANALOG_INFO_RECORD_NUM:
    {
        pFileRecordAddress = taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_DIAGNOSIS].pucContentInfo + MAX_LENGTH_READ_FILE*(ucDataTransmitTimeNum-1);
        break;
    }
    
    default:
    {
       return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }
    }

    while (usRecordLen)
    {
        *pubFileRecordBuffer++ = ( UBYTE )(*pFileRecordAddress++ );         
        usRecordLen--;
    }

    FUNC_EXIT();

    return MB_PDU_EX_NONE;   
}


/******************************************************************
*函数名称:HandleWriteFileRecord
*功能描述:写寄存器回调函数，根据记录号，调用对应的主站处理函数
*输入参数:
pubFileRecordBuffer 响应PDU 记录数据地址
arxSubRequests 子请求对应的结构体
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/1                              lv we004
******************************************************************/
eMBException HandleWriteFileRecord(UBYTE *pubFileRecordBuffer, xMBSFileRecordReq_t *arxSubRequests)
{
    IO_InterfaceResult_e eInterfaceResult = E_IO_ERROR_NOT_BUTT;

    FUNC_ENTER();

     /* 入参检查 */
    MODBUS_NULL_CHECK(pubFileRecordBuffer);
    MODBUS_NULL_CHECK(arxSubRequests);

    if (arxSubRequests->usFileNumber > MBM_WRITE_FILE_NUM)
    {
        FUNC_EXIT();
        return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }

    switch(arxSubRequests->usRecordNumber)
    {
    /* 诊断基本信息 */
    case SLAVE_BASE_INFO_RECORD_NUM:
    {     
        eInterfaceResult = DiagnoseIoDeviceBaseInfo(pubFileRecordBuffer, arxSubRequests->usRecordLength);
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }      
        break;
    }
    /* 诊断配置信息 */
    case SLAVE_CONFIG_INFO_RECORD_NUM:
    {
        eInterfaceResult = ConfigSlaveStation(pubFileRecordBuffer, arxSubRequests->usRecordLength);
  
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }

        /* 给配置数据区分配空间 */
        eInterfaceResult = InitConfigDataField();
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }

        break;
    }
    /* 诊断下行数字量 */
    case SLAVE_DOWN_DIGITAL_INFO_RECORD_NUM:
    {      
        eInterfaceResult = DiagnoseIoDataInfo(pubFileRecordBuffer, arxSubRequests->usRecordLength, E_IO_DEVICE_INFO_TYPE_DOWN_DIGTITAL);
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }      
        break;
    }
    /* 诊断下行模拟量 */
    case SLAVE_DOWN_ANALOG_INFO_RECORD_NUM:
    {       
        eInterfaceResult = DiagnoseIoDataInfo(pubFileRecordBuffer, arxSubRequests->usRecordLength, E_IO_DEVICE_INFO_TYPE_DOWN_ANALOG);
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }        
        break;
    }
    /* 诊断上行数字量 */
    case SLAVE_UP_DIGITAL_INFO_RECORD_NUM:
    {       
        eInterfaceResult = DiagnoseIoDataInfo(pubFileRecordBuffer, arxSubRequests->usRecordLength, E_IO_DEVICE_INFO_TYPE_UP_DIGTITAL);
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }       
        break;
    }
    /* 诊断上行模拟量 */
    case SLAVE_UP_ANALOG_INFO_RECORD_NUM:
    {      
        eInterfaceResult = DiagnoseIoDataInfo(pubFileRecordBuffer, arxSubRequests->usRecordLength, E_IO_DEVICE_INFO_TYPE_UP_ANALOG);
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }      
        break;
    }
    /* 诊断告警信息 */
    case SLAVE_WARN_DIGITAL_INFO_RECORD_NUM:
    { 
        eInterfaceResult = DiagnoseIoDataInfo(pubFileRecordBuffer, arxSubRequests->usRecordLength, E_IO_DEVICE_INFO_TYPE_WARN);
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }     
        break;
    }
    /* 诊断诊断信息 */
    case SLAVE_DIAGNOSE_ANALOG_INFO_RECORD_NUM:
    {    
        eInterfaceResult = DiagnoseIoDataInfo(pubFileRecordBuffer, arxSubRequests->usRecordLength, E_IO_DEVICE_INFO_TYPE_DIAGNOSIS);
        if (E_IO_ERROR_NONE != eInterfaceResult)
        {
            FUNC_EXIT();
            return MB_PDU_EX_ILLEGAL_DATA_VALUE;
        }       
        break;
    }
    default:
    {
        FUNC_EXIT();
        return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }
    }

    FUNC_EXIT();
    return MB_PDU_EX_NONE;
}


/******************************************************************
*函数名称:GetVariousStartAddr
*功能描述:设置各类寄存器值，其中设备描述文件
基本信息长度和产品信息长度组合起来构成基本信息长度
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/10                              lv we004
******************************************************************/
IO_InterfaceResult_e GetVariousStartAddr(void)
{
    uint8_t i = 0;  /* 循环变量 */
    IoDeviceInfoType_e eIoDeviceInfoType = E_IO_DEVICE_INFO_TYPE_BUTT;
    
    FUNC_ENTER();
    
    /* 设置设备描述文件各部分内容的长度到寄存器中 */
    for(i = E_REGISTER_SLAVE_BASE_INFO_LENGTH_ADDR; i <=E_REGISTER_DIAGNOSE_ANALOG_INFO_LENGTH_ADDR; i++)
    {
        eIoDeviceInfoType = DeviceInfoAndRegisterIdMap[i-1];
        gusaVariousStartAddr[i] = taIoDeviceContentInfo[eIoDeviceInfoType].usContentLength;
        gusaVariousStartAddr[E_REGISTER_SLAVE_DEVICE_INFO_LENGTH_ADDR] += gusaVariousStartAddr[i];        
    }
    
    /* 更新基本信息长度为-设备描述文件中的基本信息长度+产品信息长度*/
    gusaVariousStartAddr[E_REGISTER_SLAVE_BASE_INFO_LENGTH_ADDR] += taIoDeviceContentInfo[E_IO_DEVICE_INFO_TYPE_PRODUCT].usContentLength;

    /* 保存业务数据，告警，诊断的起始地址 */
    gusaVariousStartAddr[DOWN_DIGITAL_SERVICE_DATA_START_ADDR] = DOWN_DIGITAL_DATA_START_ADDR;
    gusaVariousStartAddr[DOWN_ANALOG_SERVICE_DATA_START_ADDR] = DOWN_ANALOG_DATA_START_ADDR;
    gusaVariousStartAddr[UP_DIGITAL_SERVICE_DATA_START_ADDR] = UP_DIGITAL_DATA_START_ADDR;
    gusaVariousStartAddr[UP_ANALOG_SERVICE_DATA_START_ADDR] = UP_ANALOG_DATA_START_ADDR;
    gusaVariousStartAddr[WARN_DIGITAL_INFO_START_ADDR] = WARN_DIGITAL_START_ADDR;
    gusaVariousStartAddr[DIAGNOSE_ANALOG_INFO_START_ADDR] = DIAGNOSE_ANALOG_START_ADDR;

    FUNC_EXIT();

    return E_IO_ERROR_NONE;
}

/******************************************************************
*                         局部函数实现                            *
******************************************************************/
/******************************************************************
*函数名称:CalculateStartAddr
*功能描述:计算寄存器起始地址
*输入参数:
pusStartRegisterIn:数据区起始地址
ucBufferSize:数据区大小
ucRealStartAddr:对应数据区的起始寄存器号
usAddress:要读取或写入的起始寄存器号
usNRegs:寄存器数量
ppusStartAddrOut:计算出来的数据区起始地址，从这个地址开始读写寄存器
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/1                              lv we004
******************************************************************/
eMBException CalculateStartAddr(uint16_t *pusStartRegisterIn, uint8_t ucBufferSize,
    uint16_t ucRealStartAddr, USHORT usAddress, USHORT usNRegs, uint16_t **ppusStartAddrOut)
{
    uint16_t *pusEndRegister = NULL;    /* 数据结束地址 */

    FUNC_ENTER();

     /* 入参检查 */
    MODBUS_NULL_CHECK(pusStartRegisterIn);
    MODBUS_NULL_CHECK(ppusStartAddrOut);

     /* 计算起始地址 */
    pusEndRegister = pusStartRegisterIn + ucBufferSize;
    *ppusStartAddrOut = pusStartRegisterIn + usAddress - ucRealStartAddr;

    /* 验证起始地址 */
    if ((*ppusStartAddrOut + usNRegs) > pusEndRegister)
    {
        return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }

    FUNC_EXIT();

    return MB_PDU_EX_NONE;
}

/******************************************************************
*函数名称:ReadFileRecordAndJudgeFileRecordType
*功能描述:获取当前是哪种信息记录号，并判断出是第几次传送
*输入参数:arxSubRequests:子请求对应的结构体
*输出参数:RecordNumTypeOut:文件记录号类型
                        DataTransmitTimeNumOut:第几次传输
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2018/12/8                              weiguangyu we031
******************************************************************/
eMBException ReadFileRecordAndJudgeFileRecordType(xMBSFileRecordReq_t *arxSubRequests, uint8_t *pucRecordNumTypeOut, uint8_t *pucDataTransmitTimeNumOut)
{
    uint8_t i = 0;   /*循环变量*/
    uint8_t ucInterpolation = 0;
    /* 入参检查 */
    MODBUS_NULL_CHECK(arxSubRequests);

    /*为基本信息记录号则直接返回*/
    if (0 == arxSubRequests->usRecordNumber)
    {
        /*获取文件记录号类型*/
        *pucRecordNumTypeOut = SLAVE_BASE_INFO_RECORD_NUM;
        *pucDataTransmitTimeNumOut = 0;
        return MB_PDU_EX_NONE;
    }
    
    /*其它信息获取文件记录号处理*/
    for (i = 0; i < SLAVE_DIAGNOSE_ANALOG_INFO_RECORD_NUM; i++)
    {
        /*计算当前传输次数编号*/
        ucInterpolation = arxSubRequests->usRecordNumber - SLAVE_INFO_RECORD_NUM_TRANSMIT_MAX_TIME_NUM*i;

        if (ucInterpolation > SLAVE_INFO_RECORD_NUM_TRANSMIT_MAX_TIME_NUM)
        {
            continue;
        }
     
        /*获取文件记录号类型*/
        *pucRecordNumTypeOut = i+1;
        /*当前传输次数编号*/
        *pucDataTransmitTimeNumOut = ucInterpolation;

        return MB_PDU_EX_NONE;  
    }
    
    return MB_PDU_EX_ILLEGAL_DATA_VALUE;
}
