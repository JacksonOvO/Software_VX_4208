/******************************************************************
*
*文件名称:  STM32F0IAPDriver.c
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
#include "STM32F0IAPDriveropt.h"
#include "STM32F0IAPDriver.h"
#include "STM32F0usartopt.h"


/******************************************************************
*                           数据类型                              *
******************************************************************/
/* 看门狗宏定义 */
/* watchdog宏定义 */

#if (IAP_IO_CODE_TYPE == IAP_FR4004_CODE) ||(IAP_IO_CODE_TYPE == IAP_FR4014_CODE)||(IAP_IO_CODE_TYPE == IAP_FR4024_CODE)||(IAP_IO_CODE_TYPE == IAP_FR4504_CODE)||(IAP_IO_CODE_TYPE == IAP_FR4514_CODE)||(IAP_IO_CODE_TYPE == IAP_FR4524_CODE)||(IAP_IO_CODE_TYPE == IAP_FR4124_CODE)||(IAP_IO_CODE_TYPE == IAP_FR4114_CODE)||(IAP_IO_CODE_TYPE == IAP_FR4104_CODE)
#define IAP_WATCH_DOG_GPIO  (IAP_GPIOA)
#define IAP_WATCH_DOG_PIN   (IAP_GPIO_Pin_2)
#endif
/* 看门狗宏定义 */

/* CRC */
#define IAP_CRC16_USETABLE          ( 0 )


/******************************************************************
*                           全局变量声明                          *
******************************************************************/ 
/*接收DMA BUFFER*/
IAP_uint8_t gucIAPUpgradeRebuffer[RECEIVE_SIZE]@0x20000000;

/*定义DMA发送缓存*/
IAP_uint8_t gucIAPUpgradeTxbuffer[RECEIVE_SIZE]@0x20000100;
IAP_uint16_t gusIAPUpgradeTxbuffer[RECEIVE_SIZE]@0x20000200;

/* 静态变量固化 */
IAP_uint32_t ucIAPWatchdogCount@0x20000400;
IAP_uint32_t uiNextCodePackageSN@0x20000404;
IAP_uint32_t uiNextDeviceInfoPackageSN@0x20000408;
IAP_uint32_t uiNextVersionPackageSN@0x2000040c;


/* 数组固化 */
__root const IAP_uint8_t IAPAPBAHBPrescTable[MESSGAE_SIZE]@".IAPAPBAHBPrescTable" = 
{0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
extern IAP_uint16_t gusUSART1DMATx[DMA_SEND_BUFSIZE];

/******************************************************************
*                        局部函数声明                            *
******************************************************************/
static IAPUSHORT usIapUtlCRC16GetTab( IAPUBYTE ubIdx );
static IAPUSHORT usIapMBSCRC16( const IAPUBYTE *pucFrame, IAPUSHORT usLen );
static void IAP_GPIO_SetBits(IAP_GPIO_TypeDef* GPIOx, IAP_uint16_t GPIO_Pin);
static void IAP_GPIO_ResetBits(IAP_GPIO_TypeDef* GPIOx, IAP_uint16_t GPIO_Pin);
static void IAPFeedTheWatchDog(void);
static void STM32F0IAPReceive(IAP_uint32_t uiReceiveLength);
static void STM32F0IAPSend(IAP_uint8_t *pucSendBuffer, IAP_uint16_t usSendLength);
static void IAP_FLASH_Unlock(void);
static IAP_FLASH_Status IAP_FLASH_GetStatus(void);
static IAP_FLASH_Status IAP_FLASH_WaitForLastOperation(IAP_uint32_t Timeout);
static IAP_FLASH_Status IAP_FLASH_ErasePage(IAP_uint32_t Page_Address);
static IAP_FLASH_Status IAP_FLASH_ProgramWord(IAP_uint32_t Address, IAP_uint32_t Data);
static void STM32F0IAPMemset(IAP_uint8_t ucBuffer[RECEIVE_SIZE], IAP_uint16_t uiLength);
static void STM32F0IAPMemcpy(IAP_uint8_t *pucDst, IAP_uint8_t *pucSrc, IAP_uint32_t uiLength);
static void STM32FOIAPTypeIndicate(IAP_uint8_t ucFrameType, IAP_uint8_t ucSlaveStationNumber);
static void STM32F0TypeIndicateResponse(IAP_uint8_t ucSlaveStationNum,
                IAP_uint8_t ucFrameType, IAP_uint8_t ucUpgradeType, IAP_uint8_t ucHandleResult);
static void STM32FOIAPContent(void);
static void STM32F0ContentIndicateResponse(IAP_uint8_t *pucReceiveBuff, IAP_uint8_t ucHandleResult);
static IAP_Result_e STM32F0IAPRAMToFlash(IAP_uint8_t *pucReceiveBuff, IAP_uint8_t ucPacketretransmission, 
            IAP_uint32_t uiUpgradePackageSN, IAP_uint16_t usUpgradePackagelen, IAP_uint32_t uiNextPackageSN, IAP_uint32_t uiFlashDestination);
static IAP_Result_e STM32IAPContentFrameCheck(IAP_uint8_t *pucReceiveBuff, IAP_uint32_t uiUpgradePackageSN, 
                                           IAP_uint8_t ucPacketretransmission, IAP_uint16_t usUpgradePackagelen, IAP_uint32_t uiNextPackageSN);
static void STM32FOIAPComplete(IAP_uint8_t ucFrameType, IAP_uint8_t ucSlaveStationNumber);
static void STM32F0Reset(void);
static void STM32F0IAPCompleteIndicateResponse(IAP_uint8_t ucSlaveStationNum, IAP_uint8_t ucFrameType,IAP_uint8_t ucHandleResult);

/******************************************************************
*函数名称:usIapMBSCRC16
*功能描述:CRC校验
*输入参数:pucFrame       BUFFER指针
                            usLen             长度
*输出参数:
*返回值:
*其它说明:添加CRC校验，以确保传输数据正确
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/2/27                              zhanghaifeng  we015
******************************************************************/

#if IAP_CRC16_USETABLE == 0
static         IAPUSHORT
usIapUtlCRC16GetTab( IAPUBYTE ubIdx )
{
    IAPUSHORT          usCRC16;
    IAPUSHORT          usC;
    IAPUBYTE           i;

    usCRC16 = 0;
    usC = ( IAPUSHORT ) ubIdx;

    for( i = 0; i < 8; i++ )
    {

        if( ( usCRC16 ^ usC ) & 0x0001 )
        {
            usCRC16 = ( usCRC16 >> 1 ) ^ ( IAPUSHORT )0xA001U;
        }
        else
        {
            usCRC16 = usCRC16 >> 1;
        }
        usC = usC >> 1;
    }
    return usCRC16;
}
#endif

static IAPUSHORT
usIapMBSCRC16( const IAPUBYTE *pucFrame, IAPUSHORT usLen )
{
    IAPUBYTE           ubCRCHi = 0xFF;
    IAPUBYTE           ubCRCLo = 0xFF;
    IAPUBYTE           ubIndex;

#if IAP_CRC16_USETABLE == 0
    IAPUSHORT          usCRCTableValue;
#endif

    while( usLen-- )
    {
        ubIndex = ubCRCLo ^ *( pucFrame++ );
#if IAP_CRC16_USETABLE == 1
        ubCRCLo = ubCRCHi ^ ( IAPUBYTE ) IapaucCRCHi[ubIndex];
        ubCRCHi = ( IAPUBYTE ) IapaucCRCLo[ubIndex];
#else
        usCRCTableValue = usIapUtlCRC16GetTab( ubIndex );
        ubCRCLo = ubCRCHi ^ ( IAPUBYTE ) ( usCRCTableValue & 0xFF );
        ubCRCHi = ( IAPUBYTE ) ( usCRCTableValue >> 8U );
#endif

    }
    /* Additional casts a for PIC MCC18 compiler to fix a bug when -Oi is not used. 
     * This is required because it does not enforce ANSI c integer promotion
     * rules.
     */
    return ( IAPUSHORT )( ( IAPUSHORT )ubCRCHi << 8 | ( IAPUSHORT)ubCRCLo );
}

/******************************************************************
*函数名称:IAP_GPIO_SetBits
*功能描述:
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/3/7                              lv we004
******************************************************************/
static void IAP_GPIO_SetBits(IAP_GPIO_TypeDef* GPIOx, IAP_uint16_t GPIO_Pin)
{
  GPIOx->BSRR = GPIO_Pin;
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
*2017/3/7                              lv we004
******************************************************************/
static void IAP_GPIO_ResetBits(IAP_GPIO_TypeDef* GPIOx, IAP_uint16_t GPIO_Pin)
{
  GPIOx->BRR = GPIO_Pin;
}

/******************************************************************
*函数名称:IAPFeedTheWatchDog
*功能描述:升级程序喂狗
*输入参数:
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/3/7                              lv we004
******************************************************************/
static void IAPFeedTheWatchDog(void)
{
 //   static IAP_uint32_t ucIAPWatchdogCount = 0;
    
    if (0 == ucIAPWatchdogCount)
    {
        IAP_GPIO_ResetBits(IAP_WATCH_DOG_GPIO, IAP_WATCH_DOG_PIN);
        ucIAPWatchdogCount = 1;
    }
    else
    {
        IAP_GPIO_SetBits(IAP_WATCH_DOG_GPIO, IAP_WATCH_DOG_PIN);
        ucIAPWatchdogCount = 0;
    }
}

/******************************************************************
*函数名称:STM32F0IAPReceive
*功能描述:接收函数，接收更新包
*输入参数:
*输出参数:
*返回值: 
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/04                            yangwenfan we014
******************************************************************/
static void STM32F0IAPReceive(IAP_uint32_t uiReceiveLength)
{
    IAP_uint16_t usPacketlength = 0;

    IAP_DMA1_Channel5->CCR &= (IAP_uint16_t)(~IAP_DMA_CCR_EN);    
    IAP_DMA1_Channel5->CNDTR = uiReceiveLength;
    IAP_DMA1_Channel5->CMAR = (IAP_uint32_t)gucIAPUpgradeRebuffer;
    IAP_DMA1_Channel5->CCR |= IAP_DMA_CCR_EN;
    do
    {
        usPacketlength = ((IAP_uint16_t)(IAP_DMA1_Channel5->CNDTR));
    }while(RECEIVE_COMPLETE != usPacketlength);
}

/******************************************************************
*函数名称:STM32F0IAPSend
*功能描述:发送数据
*输入参数:pucSendBuffer                发送buffer
                            usSendLength                发送长度
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/14                              zhanghaifeng  we015
******************************************************************/
static void STM32F0IAPSend(IAP_uint8_t *pucSendBuffer, IAP_uint16_t usSendLength)
{
    IAP_uint8_t i = 0;

    /*多发送一个字节:主站站号0XFE*/
    usSendLength++;
    
    /*发送DMA使能，发送使用通道2*/
    IAP_DMA1_Channel4->CCR &= (IAP_uint16_t)(~IAP_DMA_CCR_EN);

    /*设置发送数据长度*/
    IAP_DMA1_Channel4->CNDTR = (usSendLength & 0xFFFF);

    /*重新发送buffer*/
    IAP_DMA1_Channel4->CMAR = (IAP_uint32_t)gusIAPUpgradeTxbuffer;
    
    /*拷贝待发送数据到DMA缓存*/
    for (i = 0; i <= usSendLength; i++)
    {
        gucIAPUpgradeTxbuffer[i] = *pucSendBuffer++;
    }

     /*9位模式下发送数据为2字节发送*/
    gusIAPUpgradeTxbuffer[0] = 0XFE|0x100;
    for (i = 0; i < usSendLength; i++)
    {
        gusIAPUpgradeTxbuffer[i+1] = gucIAPUpgradeTxbuffer[i];
    }
   // memcpy(gucIAPUpgradeTxbuffer, pucSendBuffer, usSendLength);
   /*备注:使用MEMCPY会使出现程序不知跑到哪儿的错误，
         以前碰到过，问题没有解决，先使用FOR循环使用*/

    /*使能DMA通道*/
    IAP_DMA1_Channel4->CCR |= IAP_DMA_CCR_EN;

}


/******************************************************************
*函数名称:IAP_FLASH_Unlock
*功能描述:解锁flash空间
*输入参数:
*输出参数:
*返回值: 
*其它说明:库函数FLASH_Unlock
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/04                            yangwenfan we014
******************************************************************/
static void IAP_FLASH_Unlock(void)
{
    if((IAP_FLASH->CR & IAP_FLASH_CR_LOCK) != 0)
    {
        /* Unlocking the program memory access */
        IAP_FLASH->KEYR = IAP_FLASH_FKEY1;
        IAP_FLASH->KEYR = IAP_FLASH_FKEY2;
    }
}
/******************************************************************
*函数名称:IAP_FLASH_GetStatus
*功能描述:获取flash操作状态
*输入参数:
*输出参数:
*返回值: 
*其它说明:库函数FLASH_GetStatus
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/04                            yangwenfan we014
******************************************************************/
static IAP_FLASH_Status IAP_FLASH_GetStatus(void)
{
    IAP_FLASH_Status FLASHstatus = IAP_FLASH_COMPLETE;

    if((IAP_FLASH->SR & IAP_FLASH_FLAG_BSY) == IAP_FLASH_FLAG_BSY) 
    {
        FLASHstatus = IAP_FLASH_BUSY;
    }
    else 
    {  
        if((IAP_FLASH->SR & (IAP_uint32_t)IAP_FLASH_FLAG_WRPERR)!= (IAP_uint32_t)0x00)
        { 
            FLASHstatus = IAP_FLASH_ERROR_WRP;
        }
        else 
        {
            if((IAP_FLASH->SR & (IAP_uint32_t)(IAP_FLASH_FLAG_PGERR)) != (IAP_uint32_t)0x00)
            {
                FLASHstatus = IAP_FLASH_ERROR_PROGRAM; 
            }
            else
            {
                FLASHstatus = IAP_FLASH_COMPLETE;
            }
        }
    }
    /* Return the FLASH Status */
    return FLASHstatus;
}

/******************************************************************
*函数名称:IAP_FLASH_WaitForLastOperation
*功能描述:上一次flash操作返回值
*输入参数:Timeout
*输出参数:
*返回值: 
*其它说明:库函数FLASH_WaitForLastOperation
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/04                            yangwenfan we014
******************************************************************/
static IAP_FLASH_Status IAP_FLASH_WaitForLastOperation(IAP_uint32_t Timeout)
{ 
    IAP_FLASH_Status status = IAP_FLASH_COMPLETE;

    /* Check for the FLASH Status */
    status = IAP_FLASH_GetStatus();

    /* Wait for a FLASH operation to complete or a TIMEOUT to occur */
    while((status == IAP_FLASH_BUSY) && (Timeout != 0x00))
    {
        status = IAP_FLASH_GetStatus();
        Timeout--;
    }

    if(Timeout == 0x00 )
    {
        status = IAP_FLASH_TIMEOUT;
    }
    /* Return the operation status */
    return status;
}

/******************************************************************
*函数名称:IAP_FLASH_ErasePage
*功能描述:擦除扇区某一页
*输入参数:Page_Address
*输出参数:
*返回值: 
*其它说明:库函数FLASH_ErasePage
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/04                            yangwenfan we014
******************************************************************/
static IAP_FLASH_Status IAP_FLASH_ErasePage(IAP_uint32_t Page_Address)
{
    IAP_FLASH_Status status = IAP_FLASH_COMPLETE;

    /* Wait for last operation to be completed */
    status = IAP_FLASH_WaitForLastOperation(IAP_FLASH_ER_PRG_TIMEOUT);

    if (IAP_FLASH_COMPLETE == status)
    { 
        /* If the previous operation is completed, proceed to erase the page */
        IAP_FLASH->CR |= IAP_FLASH_CR_PER;
        IAP_FLASH->AR  = Page_Address;
        IAP_FLASH->CR |= IAP_FLASH_CR_STRT;

        /* Wait for last operation to be completed */
        status = IAP_FLASH_WaitForLastOperation(IAP_FLASH_ER_PRG_TIMEOUT);

        /* Disable the PER Bit */
        IAP_FLASH->CR &= ~IAP_FLASH_CR_PER;
    }

    /* Return the Erase Status */
    return status;
}
/******************************************************************
*函数名称:IAP_FLASH_ProgramWord
*功能描述:flash编程
*输入参数:Address
                         Data
*输出参数:
*返回值: 
*其它说明:库函数FLASH_ProgramWord
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/12/04                            yangwenfan we014
******************************************************************/
static IAP_FLASH_Status IAP_FLASH_ProgramWord(IAP_uint32_t Address, IAP_uint32_t Data)
{
    IAP_FLASH_Status status = IAP_FLASH_COMPLETE;
    __IAP_IO IAP_uint32_t tmp = 0;

    /* Wait for last operation to be completed */
    status = IAP_FLASH_WaitForLastOperation(IAP_FLASH_ER_PRG_TIMEOUT);

    if(status == IAP_FLASH_COMPLETE)
    {
        /* If the previous operation is completed, proceed to program the new first 
                     half word */
        IAP_FLASH->CR |= IAP_FLASH_CR_PG;

        *(__IAP_IO IAP_uint16_t*)Address = (IAP_uint16_t)Data;

        /* Wait for last operation to be completed */
        status = IAP_FLASH_WaitForLastOperation(IAP_FLASH_ER_PRG_TIMEOUT);

        if(status == IAP_FLASH_COMPLETE)
        {
            /* If the previous operation is completed, proceed to program the new second 
                             half word */
            tmp = Address + 2;

            *(__IAP_IO IAP_uint16_t*) tmp = Data >> 16;

            /* Wait for last operation to be completed */
            status = IAP_FLASH_WaitForLastOperation(IAP_FLASH_ER_PRG_TIMEOUT);

            /* Disable the PG Bit */
            IAP_FLASH->CR &= ~IAP_FLASH_CR_PG;
        }
        else
        {
            /* Disable the PG Bit */
            IAP_FLASH->CR &= ~IAP_FLASH_CR_PG;
        }
    }

    /* Return the Program Status */
    return status;
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
*2017/1/18                              zhanghaifeng  we015
******************************************************************/
static void STM32F0IAPMemset(IAP_uint8_t ucBuffer[RECEIVE_SIZE], IAP_uint16_t uiLength)
{
    IAP_uint16_t i = 0;
        
    for(i = 0; i < uiLength; i++)
    {
        ucBuffer[i] = 0;
    }
}

/******************************************************************
*函数名称:STM32F0IAPMemcpy
*功能描述:pucDst         目标地址
                            pucSrc         源地址
                            uiLength      长度
*输入参数:
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/18                              zhanghaifeng  we015
******************************************************************/
static void STM32F0IAPMemcpy(IAP_uint8_t *pucDst, IAP_uint8_t *pucSrc, IAP_uint32_t uiLength)
{
    IAP_uint32_t i = 0;
    
    for(i = 0; i < uiLength; i++)
    {
       *pucDst++ = *pucSrc++;
    }
}
/******************************************************************
*函数名称:STM32FOIAPTypeIndicate
*功能描述:升级类型
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/15                              zhanghaifeng  we015
******************************************************************/
static void STM32FOIAPTypeIndicate(IAP_uint8_t ucFrameType, IAP_uint8_t ucSlaveStationNumber)
{
    IAP_uint8_t *pucReceiveBuff = IAP_NULL;   /*指向接收消息内容的首地址*/
    IAP_uint32_t uiFromGWProductBatch = 0; /*生产批次*/
    IAP_uint32_t uiProductBatch = 0; /*生产批次*/
    IAP_uint16_t usProductBatch1 = 0; /*生产批次高两个字节*/
    IAP_uint16_t usProductBatch2 = 0; /*生产批次低两个字节*/
    IAP_uint8_t ucHandleRsults = 0; /*处理结果*/
    IAP_uint8_t ucUpgradeType = 0;/*升级类型*/
    IAP_uint32_t uiFlashDestination = 0;/*待拷贝FLASH地址*/
    IAP_uint32_t uiHardWareVersionNUM = 0;/*获取从站硬件版本号*/
    IAP_uint16_t usHardWareVersionNUM1 = 0;/*获取从站硬件版本号1*/
    IAP_uint16_t usHardWareVersionNUM2 = 0;/*获取从站硬件版本号2*/
    IAP_uint32_t uiFromHardWareVersionNUM = 0;/*从网关获取从站硬件版本号*/
    IAP_uint8_t uiFromGWUpgradePackageType = 0; /*从网关获取升级包类型*/
    IAP_uint8_t ucFromGWHardWareVersionAnzahl = 0;/*从网关获取的升级包支持硬件版本个数*/
    IAP_uint8_t i = 0;
    IAP_uint8_t z = 0;
    IAP_FLASH_Status RAMToFlashstatus = IAP_FLASH_TIMEOUT;
    IAP_uint32_t uiProductInfoAddr = IAP_FLASH_DEVICE_INFO_START_ADDR;        /* 产品信息地址 */
    IAP_uint32_t uiUpgradeInfoAddr = IAP_FLASH_UPGRADE_INFO_START_ADDR;         /* 升级信息地址 */
    IAP_uint8_t ucBoardType = 0;        /* 板类型 */
        
    /*解析网关请求帧*/
    pucReceiveBuff = gucIAPUpgradeRebuffer;
    /*从网关获取从站生产批次*/
    STM32F0IAPMemcpy((IAP_uint8_t *)&uiFromGWProductBatch, (IAP_uint8_t *)&gucIAPUpgradeRebuffer[IAP_PRODUCT_BATCH_LOCATION], IAP_PRODUCT_BATCH_SIZE);
    /*获取升级类型*/
    pucReceiveBuff = pucReceiveBuff + IAP_UPGRADE_TYPE_LOCATE;
    STM32F0IAPMemcpy((IAP_uint8_t *)&ucUpgradeType, (IAP_uint8_t *)pucReceiveBuff, IAP_UPGRADE_TYPE_SIZE);
    

    /*获取从站硬件版本号1*/
  //  STM32F0IAPMemcpy((IAP_uint8_t *)&usHardWareVersionNUM1, (IAP_uint8_t *)&gusaDiagnoseData[IAP_HARDWARE_VERSION1_LOCATION], IAP_HARDWARE_VERSION_NUM1_SIZE);
    uiProductInfoAddr += IAP_PRODUCT_INFO_HEAD_LENGTH + IAP_PRODUCT_INFO_CONTENT_HEAD_LENGTH;
    STM32F0IAPMemcpy((IAP_uint8_t *)&usHardWareVersionNUM1, (IAP_uint8_t *)uiProductInfoAddr, IAP_PRODUCT_INFO_CONTENT_LENGTH);
    /*获取从站硬件版本号2*/
   // STM32F0IAPMemcpy((IAP_uint8_t *)&usHardWareVersionNUM2, (IAP_uint8_t *)&gusaDiagnoseData[IAP_HARDWARE_VERSION2_LOCATION], IAP_HARDWARE_VERSION_NUM2_SIZE);
    uiProductInfoAddr += IAP_PRODUCT_INFO_CONTENT_LENGTH + IAP_PRODUCT_INFO_CONTENT_HEAD_LENGTH;
    STM32F0IAPMemcpy((IAP_uint8_t *)&usHardWareVersionNUM2, (IAP_uint8_t *)uiProductInfoAddr, IAP_PRODUCT_INFO_CONTENT_LENGTH);
    /*组装从站硬件版本号*/
    uiHardWareVersionNUM = ( (usHardWareVersionNUM1 << BIT_16) | usHardWareVersionNUM2 );

    /*获取从站产品批次1*/
    //STM32F0IAPMemcpy((IAP_uint8_t *)&usProductBatch1, (IAP_uint8_t *)&gusaDiagnoseData[IAP_PRODUCT_BATCH1_LOCATION], IAP_PRODUCT_BATCH1_SIZE);
    uiProductInfoAddr += IAP_PRODUCT_INFO_CONTENT_LENGTH + IAP_PRODUCT_INFO_CONTENT_HEAD_LENGTH;
    STM32F0IAPMemcpy((IAP_uint8_t *)&usProductBatch1, (IAP_uint8_t *)uiProductInfoAddr, IAP_PRODUCT_INFO_CONTENT_LENGTH);
    /*获取从站产品批次2*/
 //   STM32F0IAPMemcpy((IAP_uint8_t *)&usProductBatch2, (IAP_uint8_t *)&gusaDiagnoseData[IAP_PRODUCT_BATCH2_LOCATION], IAP_PRODUCT_BATCH2_SIZE);
    uiProductInfoAddr += IAP_PRODUCT_INFO_CONTENT_LENGTH + IAP_PRODUCT_INFO_CONTENT_HEAD_LENGTH;
    STM32F0IAPMemcpy((IAP_uint8_t *)&usProductBatch2, (IAP_uint8_t *)uiProductInfoAddr, IAP_PRODUCT_INFO_CONTENT_LENGTH);
    /*组装从站产品批次*/
    uiProductBatch = ( (usProductBatch1 << BIT_16) | usProductBatch2 );
    /*判断从站产品批次是否合法*/
    if (uiProductBatch != uiFromGWProductBatch)
    {
        ucHandleRsults = IAP_PRODUCT_BATCHES_ERROR;
        
        /*响应产品批次错误帧*/
        (void)STM32F0TypeIndicateResponse(ucSlaveStationNumber, ucFrameType, ucUpgradeType, ucHandleRsults);
        return;
    }

    /*从网关获取升级包类型*/
    pucReceiveBuff = pucReceiveBuff + IAP_UPGRADE_PACKAGE_TYPE_LOCATE;
    STM32F0IAPMemcpy((IAP_uint8_t *)&uiFromGWUpgradePackageType, (IAP_uint8_t *)pucReceiveBuff, IAP_UPGRADE_PACKAGE_TYPE_SIZE);
    
    /*判断升级包类型是否合法*/
    //if(uiFromGWUpgradePackageType != gucaIOUpgradeInfo[IAP_SLAVE_UPGRADE_PACKAGE_TYPE_LOCATE])
    STM32F0IAPMemcpy(&ucBoardType, (IAP_uint8_t *)uiUpgradeInfoAddr, IAP_UPGRADE_BOARD_TYPE_LENGTH);
    if(uiFromGWUpgradePackageType != ucBoardType)
    {
        ucHandleRsults = IAP_UPGRADE_PACKAGE_TYPE_ERROR;
        STM32F0TypeIndicateResponse(ucSlaveStationNumber, ucFrameType, ucUpgradeType, ucHandleRsults);
        return;
    }

    /*从网关获取升级包支持的硬件版本个数*/
    pucReceiveBuff = pucReceiveBuff + IAP_HARDWARE_VERSION_ANZAHL_LOCATE;
    STM32F0IAPMemcpy((IAP_uint8_t *)&ucFromGWHardWareVersionAnzahl, (IAP_uint8_t *)pucReceiveBuff, IAP_HARDWARE_VERSION_ANZAHL_SIZE);

    /*检测硬件版本号是否合法*/
    /*指向硬件版本号*/
    pucReceiveBuff = pucReceiveBuff + IAP_HARDWARE_VERSION_ANZAHL_SIZE;
    for(i = 0; i < ucFromGWHardWareVersionAnzahl; i++)
    {
        STM32F0IAPMemcpy((IAP_uint8_t *)&uiFromHardWareVersionNUM, (IAP_uint8_t *)pucReceiveBuff, IAP_HARDWARE_VERSION_NUM_SIZE);
        if(uiFromHardWareVersionNUM == uiHardWareVersionNUM)
        {
            break;
        }
        pucReceiveBuff = pucReceiveBuff + IAP_HARDWARE_VERSION_NUM_SIZE;
    }
    if(i == ucFromGWHardWareVersionAnzahl)
    {
        ucHandleRsults = IAP_HARDWARE_VERSION_NUM_ERROR;
        STM32F0TypeIndicateResponse(ucSlaveStationNumber, ucFrameType, ucUpgradeType, ucHandleRsults);
        return;
    }
    
    /*升级类型*/
    switch(ucUpgradeType)
    {
    case IAP_PROGRAM_UPGRADE :
    {
        uiFlashDestination = IAP_UPGRADE_CODE;
        
        /*擦除代码扇区*/
        for(i = 0;i < IAP_CODE_SECTOR_NUM;i++)
        {
            for(z = 0; z < IAP_SECTOR_ERASE_TIMES; z++)
            {
                /* BEGIN: Added by lv we004, 2017/3/8   PN:watchdog */
                IAPFeedTheWatchDog();
                /* END:   Added by lv we004, 2017/3/8 */
                RAMToFlashstatus = IAP_FLASH_ErasePage(uiFlashDestination + (PAGE_SIZE * z));
                if(RAMToFlashstatus != IAP_FLASH_COMPLETE)
                {
                    ucHandleRsults = IAP_FLASH_ERASER_ERROR;
                    STM32F0TypeIndicateResponse(ucSlaveStationNumber, ucFrameType, ucUpgradeType, ucHandleRsults);                
                } 
            }           
            uiFlashDestination += (PAGE_SIZE * z);
        }
        break;
    }
    case IAP_DEVICE_FILE :
    {
        uiFlashDestination = IAP_UPGRADE_DEVICE;

        /*擦除设备描述文件扇区*/
        for(z = 0; z < IAP_SECTOR_ERASE_TIMES; z++)
        {
            /* BEGIN: Added by lv we004, 2017/3/8   PN:watchdog */
            IAPFeedTheWatchDog();
            /* END:   Added by lv we004, 2017/3/8 */
            
            RAMToFlashstatus = IAP_FLASH_ErasePage(uiFlashDestination + (PAGE_SIZE * z));
            if(RAMToFlashstatus != IAP_FLASH_COMPLETE)
            {
                ucHandleRsults = IAP_FLASH_ERASER_ERROR;
                STM32F0TypeIndicateResponse(ucSlaveStationNumber, ucFrameType, ucUpgradeType, ucHandleRsults);                
            }  
        }
        break;
    }
    case IAP_VERSION_DATA :
    {
        uiFlashDestination = IAP_UPGRADE_VERSION;

        /*擦除杂项数据扇区*/
        for(z = 0; z < IAP_SECTOR_ERASE_TIMES; z++)
        {
            /* BEGIN: Added by lv we004, 2017/3/8   PN:watchdog */
            IAPFeedTheWatchDog();
            /* END:   Added by lv we004, 2017/3/8 */
            
            RAMToFlashstatus = IAP_FLASH_ErasePage(uiFlashDestination + (PAGE_SIZE * z));
            if(RAMToFlashstatus != IAP_FLASH_COMPLETE)
            {            
                ucHandleRsults = IAP_FLASH_ERASER_ERROR;
                STM32F0TypeIndicateResponse(ucSlaveStationNumber, ucFrameType, ucUpgradeType, ucHandleRsults);                
            }   
        }

        break;
    }
    default :
    {
        ucHandleRsults = IAP_UPGRADE_TYPE_ERROR;
        
        /*响应升级类型错误帧*/
        (void)STM32F0TypeIndicateResponse(ucSlaveStationNumber, ucFrameType, ucUpgradeType, ucHandleRsults);
        return;
    }
    }
    
    /*组装并相应成功*/
    ucHandleRsults = SYSTEM_UPGRADE_REQ_HANDLE_SUCCESS;
    
    /*响应成功帧*/
    (void)STM32F0TypeIndicateResponse(ucSlaveStationNumber, ucFrameType, ucUpgradeType, ucHandleRsults);

    return;
}

/******************************************************************
*函数名称:STM32F0TypeIndicateResponse
*功能描述:升级类型响应
*输入参数:
                            ucSlaveStationNum      从站站号
                            ucFrameType               帧类型
                            ucUpgradeType            升级类型
                            ucHandleResult             处理结果
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/15                              zhanghaifeng  we015
******************************************************************/
static void STM32F0TypeIndicateResponse(IAP_uint8_t ucSlaveStationNum, 
                    IAP_uint8_t ucFrameType, IAP_uint8_t ucUpgradeType, IAP_uint8_t ucHandleResult)
{    
    IAP_uint16_t usIapSendCrc16Value = 0;
    
    /*组装升级类型错误响应帧*/
    /*备注:使用MEMCPY，MEMSET会使出现程序不知跑到哪儿的错误，
          以前碰到过，问题没有解决，先使用注释掉*/
    STM32F0IAPMemset(gucIAPUpgradeTxbuffer, RECEIVE_SIZE);
    gucIAPUpgradeTxbuffer[GW_STATION_LOCATION] = GW_STATION_NUM;
    gucIAPUpgradeTxbuffer[SLAVE_STATION_LOCATION] = ucSlaveStationNum;
    gucIAPUpgradeTxbuffer[FRAME_TYPE_LOCATION] = ucFrameType;
    gucIAPUpgradeTxbuffer[UPGRADE_TYPE_LOCATION] = ucUpgradeType;
    gucIAPUpgradeTxbuffer[IAP_TYPE__HANDLE_RESULT_LOCATION] = ucHandleResult;
    
    
    /*计算CRC16校验值*/
    usIapSendCrc16Value = usIapMBSCRC16(( const IAPUBYTE * )&gucIAPUpgradeTxbuffer[0], IAP_TYPE_RESPONSE_FRAME_LEN);
    
    /*CRC校验值添加到发送数据后面两位*/   
    STM32F0IAPMemcpy((IAP_uint8_t *)&gucIAPUpgradeTxbuffer[IAP_TYPE_RESPONSE_FRAME_LEN], (IAP_uint8_t *)&usIapSendCrc16Value, IAP_CRC16_VALUE_LENGTH);

    /*发送响应*/    
    /*回复网关响应帧*/
    (void)STM32F0IAPSend(gucIAPUpgradeTxbuffer, IAP_TYPE_RESPONSE_FRAME_LEN + IAP_CRC16_VALUE_LENGTH);   
}
/******************************************************************
*函数名称:STM32FOIAPContent
*功能描述:升级内容
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/15                              zhanghaifeng  we015
******************************************************************/
static void STM32FOIAPContent(void)
{
    IAP_uint8_t *pucReceiveBuff = IAP_NULL;   /*指向接收消息内容的首地址*/
    IAP_uint8_t ucUpgradeType = 0;
    IAP_uint16_t usUpgradePackagelen = 0;/*升级包长度*/
    IAP_uint8_t ucHandleResult = 0; /*处理结果*/
    IAP_uint32_t uiUpgradePackageSN = 0;/*升级包序号*/
    IAP_uint32_t uiFlashDestination = 0;/*待拷贝FLASH地址*/
    IAP_uint8_t ucPacketretransmission = 0;/*包重传标志*/
    /*定义静态变量，由于判断，包序号是否连续*/
    //static IAP_uint32_t uiNextCodePackageSN = 0;
    //static IAP_uint32_t uiNextDeviceInfoPackageSN = 0;                                                                                                                                                                                                                                                                                                          
    //static IAP_uint32_t uiNextVersionPackageSN = 0;
    IAP_Result_e eResult = E_IAP_BUTT;
    
    /*解析网关请求帧，获取升级类型，升级包序号*/
    pucReceiveBuff = gucIAPUpgradeRebuffer;
    /*获取升级类型*/
    pucReceiveBuff = pucReceiveBuff + IAP_UPGRADE_TYPE_LOCATION;
    STM32F0IAPMemcpy((IAP_uint8_t *)&ucUpgradeType, (IAP_uint8_t *)pucReceiveBuff, IAP_UPGRADE_TYPE_SIZE);
    /*获取包重传标志*/
    pucReceiveBuff = pucReceiveBuff + IAP_UPGRADE_TYPE_SIZE;
    STM32F0IAPMemcpy((IAP_uint8_t *)&ucPacketretransmission, (IAP_uint8_t *)pucReceiveBuff, IAP_PACKAGE_RETRANSMISSION_SIZE);
    /*获取升级包顺序号*/
    pucReceiveBuff = pucReceiveBuff + IAP_PACKAGE_RETRANSMISSION_SIZE;
    STM32F0IAPMemcpy((IAP_uint8_t *)&uiUpgradePackageSN, (IAP_uint8_t *)pucReceiveBuff, IAP_UPGRADE_PACKAGE_SN_SIZE);
    /*获取升级包内容长度*/
    pucReceiveBuff = pucReceiveBuff + IAP_UPGRADE_PACKAGE_SN_SIZE;
    STM32F0IAPMemcpy((IAP_uint8_t *)&usUpgradePackagelen, (IAP_uint8_t *)pucReceiveBuff, IAP_UPGRADE_PACKAGE_SIZE);

    switch(ucUpgradeType)
    {
    case IAP_PROGRAM_UPGRADE :
    {
        uiFlashDestination = IAP_UPGRADE_CODE;  
        
        /*拷贝RAM数据到响应的FLASH中*/      
        eResult = STM32F0IAPRAMToFlash(gucIAPUpgradeRebuffer,ucPacketretransmission,uiUpgradePackageSN,
                     usUpgradePackagelen, uiNextCodePackageSN,
                     uiFlashDestination + (uiNextCodePackageSN * IAP_FLASH_SECTOR_SIZE));
        if(eResult != E_IAP_OK)
        {
            return;

        }
        /*下一个连续的序列号*/
        uiNextCodePackageSN = uiUpgradePackageSN + IAP_TO_NEXT_SN;
        break;
    }
    case IAP_DEVICE_FILE :
    {
        uiFlashDestination = IAP_UPGRADE_DEVICE;
        
        /*拷贝RAM数据到响应的FLASH中*/
        eResult = STM32F0IAPRAMToFlash(gucIAPUpgradeRebuffer,ucPacketretransmission,uiUpgradePackageSN,
                    usUpgradePackagelen,uiNextDeviceInfoPackageSN,
                    uiFlashDestination + (uiNextDeviceInfoPackageSN * IAP_FLASH_SECTOR_SIZE));
        if(eResult != E_IAP_OK)
        {
            return;

        }
        /*下一个连续的序列号*/
        uiNextDeviceInfoPackageSN = uiUpgradePackageSN + IAP_TO_NEXT_SN;
        break;
    }
    case IAP_VERSION_DATA :
    {
        uiFlashDestination = IAP_UPGRADE_VERSION;
        
        /*拷贝RAM数据到响应的FLASH中*/
        eResult = STM32F0IAPRAMToFlash(gucIAPUpgradeRebuffer,ucPacketretransmission,uiUpgradePackageSN,
                    usUpgradePackagelen,uiNextVersionPackageSN,
                    uiFlashDestination + (uiNextVersionPackageSN * IAP_FLASH_SECTOR_SIZE));
        if(eResult != E_IAP_OK)
        {
            return;
        }
        /*下一个连续的序列号*/
        uiNextVersionPackageSN = uiUpgradePackageSN + IAP_TO_NEXT_SN;
        break;
    }
    default :
    {
        ucHandleResult = IAP_UPGRADE_TYPE_ERROR;
        
        /*响应升级类型错误帧*/
        STM32F0ContentIndicateResponse(gucIAPUpgradeRebuffer,ucHandleResult);
        return;
    }
    }
    /*组装并相应成功*/
    ucHandleResult = SYSTEM_UPGRADE_REQ_HANDLE_SUCCESS;

    /*响应成功帧*/
    STM32F0ContentIndicateResponse(gucIAPUpgradeRebuffer, ucHandleResult);
    
    return;
}
/******************************************************************
*函数名称:STM32F0ContentIndicateResponse
*功能描述:升级内容响应
*输入参数:
                            pucReceiveBuff             接收BUFFER
                            ucSlaveStationNum      从站站号
                            ucFrameType               帧类型
                            ucUpgradeType            升级类型
                            ucHandleResult             处理结果
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/15                              zhanghaifeng  we015
******************************************************************/
static void STM32F0ContentIndicateResponse(IAP_uint8_t *pucReceiveBuff, IAP_uint8_t ucHandleResult)
{
    IAP_uint8_t *pucFrameBufferstatus = IAP_NULL;
    IAP_uint16_t usIapSendCrc16Value = 0;
    /*组装从站升级内容相应帧*/
    pucFrameBufferstatus = gucIAPUpgradeTxbuffer;
    STM32F0IAPMemset(gucIAPUpgradeTxbuffer, RECEIVE_SIZE);

    /*网关站号组装*/
    *pucFrameBufferstatus++ = GW_STATION_NUM; 

    /*从站站号，帧类型，升级类型，包顺序号。处理结果组装*/
    STM32F0IAPMemcpy(pucFrameBufferstatus, pucReceiveBuff, IAP_CONTENT_RESPONSE_SIZE);
    pucFrameBufferstatus = pucFrameBufferstatus + IAP_CONTENT_RESPONSE_SIZE;
    *pucFrameBufferstatus = ucHandleResult;
 
    /*计算CRC16校验值*/
    usIapSendCrc16Value = usIapMBSCRC16(( const IAPUBYTE * )&gucIAPUpgradeTxbuffer[0], IAP_UPGRADE_CONTENT_REQ_FRAME_SIZE);
    
    /*CRC校验值添加到发送数据后面两位*/   
    STM32F0IAPMemcpy((IAP_uint8_t *)&gucIAPUpgradeTxbuffer[IAP_UPGRADE_CONTENT_REQ_FRAME_SIZE], (IAP_uint8_t *)&usIapSendCrc16Value, IAP_CRC16_VALUE_LENGTH);

    /*回复响应帧给网关*/
    STM32F0IAPSend(gucIAPUpgradeTxbuffer, IAP_UPGRADE_CONTENT_REQ_FRAME_SIZE + IAP_CRC16_VALUE_LENGTH); 
}

/******************************************************************
*函数名称:STM32F0IAPRAMToFlash
*功能描述:拷贝数据从RAM至FLASH
*输入参数:      
                            pucReceiveBuff             接收BUFFER
                            uiUpgradePackageSN    升级包顺序号
                            usUpgradePackagelen   升级包内容长度
                            uiNextPackageSN          下个升级包顺序号
                            uiFlashDestination         FLASH拷贝起始地址
*输出参数:
*返回值:    E_IAP_CONTENT__FRAME_CHECK_ERROR     帧检查错误
                          E_IAP_RAM_TO_FLASH_ERROR,         从RAM向FLASH拷贝错误

*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/15                              zhanghaifeng  we015
******************************************************************/
static IAP_Result_e STM32F0IAPRAMToFlash(IAP_uint8_t *pucReceiveBuff, IAP_uint8_t ucPacketretransmission, 
            IAP_uint32_t uiUpgradePackageSN, IAP_uint16_t usUpgradePackagelen, IAP_uint32_t uiNextPackageSN, IAP_uint32_t uiFlashDestination)
{
    IAP_uint8_t i = 0;
    IAP_uint32_t RamSource = 0;
    IAP_uint8_t ucHandleResult = 0; 
    IAP_Result_e eResult = E_IAP_BUTT;
    IAP_FLASH_Status eFlashResult =  IAP_FLASH_TIMEOUT;
    IAP_uint8_t *pucUpgradeOffset = IAP_NULL;

    /*升级包顺序号，长度，序列号检查*/
    eResult = STM32IAPContentFrameCheck(pucReceiveBuff, uiUpgradePackageSN, ucPacketretransmission, usUpgradePackagelen, uiNextPackageSN);
    if (E_IAP_OK != eResult)
    {
        return E_IAP_CONTENT__FRAME_CHECK_ERROR;
    }
    pucUpgradeOffset = &gucIAPUpgradeRebuffer[IAP_UPGRADE_PACKAGE_CONTENT_LOCATION];

    /*单个包复制循环*/
    for(i = 0;i < PAGE_PROGRAMME_TIME;i++)
    {
        STM32F0IAPMemcpy((IAP_uint8_t *)&RamSource, pucUpgradeOffset, PROGRAMME_LENGTH);
        eFlashResult = IAP_FLASH_ProgramWord(uiFlashDestination,RamSource);
        if (eFlashResult != IAP_FLASH_COMPLETE)
        {
            ucHandleResult = IAP_FLASH_WRITE_ERROR;
            
            /*响应未知错误*/
            STM32F0ContentIndicateResponse(pucReceiveBuff, ucHandleResult);
            return E_IAP_RAM_TO_FLASH_ERROR;
        }
        uiFlashDestination += PROGRAMME_LENGTH;
        pucUpgradeOffset = pucUpgradeOffset + PROGRAMME_LENGTH;
    }        
    return E_IAP_OK;
}

/******************************************************************
*函数名称:STM32IAPContentFrameCheck
*功能描述:
*输入参数:
                        pucReceiveBuff             接收BUFFER
                        uiUpgradePackageSN    升级包顺序号
                        usUpgradePackagelen   升级包内容长度
                        uiNextPackageSN          下个升级包顺序号
*输出参数:
*返回值:    E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_SN_ERROR,   升级内容,升级包顺序号错误
                         E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_SIZE_ERROR,                      升级包大小错误
                         E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_NUM_ERROR,                      升级包序列号错误
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/18                              zhanghaifeng  we015
******************************************************************/
static IAP_Result_e STM32IAPContentFrameCheck(IAP_uint8_t *pucReceiveBuff, IAP_uint32_t uiUpgradePackageSN, 
                                           IAP_uint8_t ucPacketretransmission, IAP_uint16_t usUpgradePackagelen, IAP_uint32_t uiNextPackageSN)
{
    IAP_uint8_t ucHandleResult = 0; 

    /*判断升级包顺序号是否合法*/
    if (uiUpgradePackageSN >= IAP_MAX_PROGRAM_UPGRADE_PACKAGE_COUNT)
    {
        ucHandleResult = IAP_UPGRADE_PACKAGE_NUM_ERROR;

        /*响应升级包顺序号非法错误帧*/
        STM32F0ContentIndicateResponse(pucReceiveBuff,ucHandleResult);
        return E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_SN_ERROR;
    }

    /*判断升级包长度是否为128*/
    if (IAP_SLAVE_UPGRADE_PACKAGE_SIZE != usUpgradePackagelen)
    {
        ucHandleResult = IAP_UPGRADE_PACKAGE_LEN_ERROR;

        /*响应升级包长度错误帧*/
        STM32F0ContentIndicateResponse(pucReceiveBuff,ucHandleResult);
        return E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_SIZE_ERROR;
    }

    if(ucPacketretransmission == IAP_NOT_PACKAGE_RETRANSMISSION)
    {
        /*判断序列号是否连续*/
        if (uiUpgradePackageSN != 0)
        {
            if (uiUpgradePackageSN != uiNextPackageSN)
            {
                ucHandleResult = IAP_UPGRADE_PACKAGE_NUM_ERROR;

                /*响应包序列号不连续错误帧*/
                STM32F0ContentIndicateResponse(pucReceiveBuff,ucHandleResult);
                return E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_NUM_ERROR;
            }
        }
    }
    if(ucPacketretransmission > IAP_PACKAGE_RETRANSMISSION)
    {
        ucHandleResult = IAP_PACKAGE_RETRANSMISSION_ERROR;
        STM32F0ContentIndicateResponse(gucIAPUpgradeRebuffer, ucHandleResult);
    }
    return E_IAP_OK;
}

/******************************************************************
*函数名称:STM32FOIAPComplete
*功能描述:升级完成
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/15                              zhanghaifeng  we015
******************************************************************/
static void STM32FOIAPComplete(IAP_uint8_t ucFrameType, IAP_uint8_t ucSlaveStationNumber)
{
    IAP_uint8_t *pucReceiveBuff = IAP_NULL;   /*指向接收消息内容的首地址*/
    IAP_uint32_t uiFromGWProductBatch = 0;
    IAP_uint32_t uiFromGWProductSerialNum = 0;
    IAP_uint8_t ucHandleRsults = 0; /*处理结果*/
    IAP_uint32_t uiProductBatch = 0; /*生产批次*/
    IAP_uint16_t usProductBatch1 = 0; /*生产批次高两个字节*/
    IAP_uint16_t usProductBatch2 = 0; /*生产批次低两个字节*/
    IAP_uint32_t uiProductInfoAddr = IAP_FLASH_DEVICE_INFO_START_ADDR;        /* 产品信息地址 */
    
    /*解析网关请求帧*/
    pucReceiveBuff = gucIAPUpgradeRebuffer;
    /*从网关获取从站生产批次*/
    pucReceiveBuff = pucReceiveBuff + IAP_PRODUCT_BATCH_LOCATION;
    STM32F0IAPMemcpy((IAP_uint8_t *)&uiFromGWProductBatch, (IAP_uint8_t *)pucReceiveBuff, IAP_PRODUCT_BATCH_SIZE);
    /*从网关获取从站序列号*/
    pucReceiveBuff = pucReceiveBuff + IAP_PRODUCT_BATCH_SIZE;
    STM32F0IAPMemcpy((IAP_uint8_t *)&uiFromGWProductSerialNum, (IAP_uint8_t *)pucReceiveBuff, IAP_PRODUCT_SERIAL_NUM_SIZE);
      
    /*获取从站产品批次1*/
    //STM32F0IAPMemcpy((IAP_uint8_t *)&usProductBatch1, (IAP_uint8_t *)&gusaDiagnoseData[IAP_PRODUCT_BATCH1_LOCATION], IAP_PRODUCT_BATCH1_SIZE);
    uiProductInfoAddr += IAP_PRODUCT_INFO_CONTENT_LENGTH + IAP_PRODUCT_INFO_CONTENT_HEAD_LENGTH + IAP_PRODUCT_BATCH_LOCATION_OFFSET;
    STM32F0IAPMemcpy((IAP_uint8_t *)&usProductBatch1, (IAP_uint8_t *)uiProductInfoAddr, IAP_PRODUCT_INFO_CONTENT_LENGTH);
    /*获取从站产品批次2*/
    //STM32F0IAPMemcpy((IAP_uint8_t *)&usProductBatch2, (IAP_uint8_t *)&gusaDiagnoseData[IAP_PRODUCT_BATCH2_LOCATION], IAP_PRODUCT_BATCH2_SIZE);
    uiProductInfoAddr += IAP_PRODUCT_INFO_CONTENT_LENGTH + IAP_PRODUCT_INFO_CONTENT_HEAD_LENGTH;
    STM32F0IAPMemcpy((IAP_uint8_t *)&usProductBatch2, (IAP_uint8_t *)uiProductInfoAddr, IAP_PRODUCT_INFO_CONTENT_LENGTH);
    /*组装从站产品批次*/
    uiProductBatch = ( (usProductBatch1 << BIT_16) | usProductBatch2 );
    /*判断从站产品批次是否合法*/
    if (uiProductBatch != uiFromGWProductBatch)
    {
        ucHandleRsults = IAP_PRODUCT_BATCHES_ERROR;

        /*  响应从站产品批次非法帧*/
        STM32F0IAPCompleteIndicateResponse(ucSlaveStationNumber, ucFrameType,ucHandleRsults);
        return;
    }
    
    /*从站软复位*/
    /*系统复位*/
    STM32F0Reset();
    
    return;
}
/******************************************************************
*函数名称:STM32F0Reset
*功能描述:软复位
*输入参数:无
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/19                              zhanghaifeng  we015
******************************************************************/
static void STM32F0Reset(void)
{
    asm ("DSB");
    IAP_SCB->AIRCR = IAP_RESET; 
    asm ("DSB");
    while(1);                                                    /* wait until reset */
}
/******************************************************************
*函数名称:STM32F0IAPCompleteIndicateResponse
*功能描述:升级完成响应
*输入参数:
                            ucSlaveStationNum      从站站号
                            ucFrameType               帧类型
                            ucHandleResult             处理结果
*输出参数:
*返回值:
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/15                              zhanghaifeng  we015
******************************************************************/
static void STM32F0IAPCompleteIndicateResponse(IAP_uint8_t ucSlaveStationNum, IAP_uint8_t ucFrameType,IAP_uint8_t ucHandleResult)
{
    IAP_uint16_t usIapSendCrc16Value = 0;
    /*组装升级类型错误响应帧*/
    /*备注:使用MEMCPY，MEMSET会使出现程序不知跑到哪儿的错误，
        以前碰到过，问题没有解决，先使用注释掉*/
    //memset(gucIAPUpgradeTxbuffer,0,sizeof(gucIAPUpgradeTxbuffer));
    STM32F0IAPMemset(gucIAPUpgradeTxbuffer, RECEIVE_SIZE);
    gucIAPUpgradeTxbuffer[GW_STATION_LOCATION] = GW_STATION_NUM;
    gucIAPUpgradeTxbuffer[SLAVE_STATION_LOCATION] = ucSlaveStationNum;
    gucIAPUpgradeTxbuffer[FRAME_TYPE_LOCATION] = ucFrameType;
    gucIAPUpgradeTxbuffer[IAP_COMPLETE_HANDLE_RESULT_LOCATION] = ucHandleResult;
    
    /*计算CRC16校验值*/
    usIapSendCrc16Value = usIapMBSCRC16(( const IAPUBYTE * )&gucIAPUpgradeTxbuffer[0], IAP_COMPLETE_RESPONSE_FRAME_LEN);
    
    /*CRC校验值添加到发送数据后面两位*/   
    STM32F0IAPMemcpy((IAP_uint8_t *)&gucIAPUpgradeTxbuffer[IAP_COMPLETE_RESPONSE_FRAME_LEN], (IAP_uint8_t *)&usIapSendCrc16Value, IAP_CRC16_VALUE_LENGTH);

    /*回复网关响应帧*/
    STM32F0IAPSend(gucIAPUpgradeTxbuffer, IAP_COMPLETE_RESPONSE_FRAME_LEN + IAP_CRC16_VALUE_LENGTH);
}

/******************************************************************
*函数名称:STM32F0IAPUpgrade
*功能描述:升级接口
*输入参数:ucSlaveStationNumber       从站站号
*输出参数:
*返回值:无
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/1/15                              zhanghaifeng  we015
******************************************************************/
__root static void STM32F0IAPUpgrade(IAP_uint8_t ucSlaveStationNumber, IAP_uint32_t uiSlavestationBaud) @".STM32F0IAPUpgrade"
{
    /*帧类型*/
    IAP_uint8_t ucFrameType = 0;
    IAP_uint16_t i = 0;
    IAP_uint16_t usIapRecvCrc16Value = 0;
    
    /*关闭所有中断 使用的汇编语言*/
    asm ("CPSID   I");
    
    /*串口使能*/
    IAP_USART1->CR1 |= IAP_USART_CR1_UE;

    /*解锁flash*/
    IAP_FLASH_Unlock();

    while(1)
    {
        /* BEGIN: Added by lv we004, 2017/3/8   PN:watchdog */
        IAPFeedTheWatchDog();
        /* END:   Added by lv we004, 2017/3/8 */

        /*调用接收函数，接收网关请求帧*/
        for(i = 0; i < RECEIVE_SIZE; i++)
        {
            gucIAPUpgradeRebuffer[i] = 0;
        }

        STM32F0IAPReceive(FROM_GW_REQUEST_FRAME_LEN + IAP_CRC16_VALUE_LENGTH );
        usIapRecvCrc16Value = usIapMBSCRC16(( const IAPUBYTE * )&gucIAPUpgradeRebuffer[0], FROM_GW_REQUEST_FRAME_LEN + IAP_CRC16_VALUE_LENGTH);
        if(IAP_CRC16_VALUE_NOERR != usIapRecvCrc16Value)
        {
            continue;
        }

        /*检查从站站号是否合法*/
        if (gucIAPUpgradeRebuffer[FROM_GW_SLAVE_STATION_LOCATION] != ucSlaveStationNumber)
        {
            continue;
        }
        
        /*帧类型解析*/
        ucFrameType = gucIAPUpgradeRebuffer[FROM_GW_FRAME_TYPE_LOCATION];

        /*帧类型*/
        switch(ucFrameType)
        {
        case IAP_TYPE_INDICATE_REQ :
        {
            STM32FOIAPTypeIndicate(ucFrameType, ucSlaveStationNumber);
            break;
        }
        case IAP_CONTENT_REQ :
        {          
            STM32FOIAPContent();
            break;
        }
        case IAP_COMPLETE_REQ :
        {
            STM32FOIAPComplete(ucFrameType, ucSlaveStationNumber);
            break;
        }
        default :
        {
        }
        }
    }
}


