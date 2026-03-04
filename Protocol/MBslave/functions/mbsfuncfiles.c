/*******************************************************************
*文件名称:  mbsfuncfiles.c
*文件标识:
*内容摘要:实现modbus 从站读写文件功能码设计
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
/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
#include <string.h>
	
/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"
	
/* ----------------------- Modbus includes ----------------------------------*/
#include <stdlib.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mbs.h"
#include "mbframe.h"
#include "internal/mbsiframe.h"
#include "internal/mbsi.h"
#include "mbsfunctions.h"

#include "warn.h"


/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
/*读写文件公用数据区*/
#define MBS_PDU_FUNC_FILE_REFERENCE_TYPE                 ( 6 )
#define MBS_PDU_FUNC_FILE_NUMBER_MIN                      (0x0000)//( 0x0001 )zhangy
#define MBS_PDU_FUNC_FILE_NUMBER_MAX                      ( 0xFFFF )
#define MBS_PDU_FUNC_RECORD_NUMBER_MIN                    ( 0x0000 )
#define MBS_PDU_FUNC_RECORD_NUMBER_MAX                    ( 0x270F )
#define MBS_PDU_FUNC_FILE_RECORD_SUBREQUESTS_MIN            ( 1 )

#define MBS_PDU_FUNC_RD_FILE_MAX_SUBREQUESTS                   ( 35 )
#define MBS_PDU_FUNC_RD_FILE_RECORD_RESPONSE_DATA_LENGTH_MIN ( 0x07 )
#define MBS_PDU_FUNC_RD_FILE_RECORD_RESPONSE_DATA_LENGTH_MAX ( 0xF5 )
/*最少一个子请求数据长度*/
#define MB_PDU_FUNC_RD_FILE_RECORDREQ_SIZE_MIN                   ( 8 )
/*请求PDU 字节数偏移地址*/
#define MB_PDU_FUNC_RD_FILEBYTE_NUM_OFF   ( 1 )

/*写文件请求PDU最大长度*/
#define MBS_PDU_FUNC_WR_FILE_PDULEN_MAX                  ( 253 )
/*请求PDU 数据长度偏移地址*/
#define MB_PDU_FUNC_WR_NUMLEN_OFF                         ( 1 )
/*写文件第一个记录数据偏移地址*/
#define MB_PDU_FUNC_WR_FIRST_RECORDDATA_OFF              ( 8 )
/*每个子请求携带信息(参考类型，记录号，文件号，记录长度)所占字节数*/
#define MB_PDU_FUNC_WR_SIGNAL_REQ_INF_SIZE               ( 7 )
/******************************************************************
*                           数据类型                              *
******************************************************************/

/******************************************************************
*                         全局变量定义                            *
******************************************************************/

/******************************************************************
*                          局部函数声明                            *
******************************************************************/

/******************************************************************
*                         全局函数实现                            *
******************************************************************
*函数名称:eMBSFuncReadFileRecord
*功能描述:读文件记录，实现0x14功能码
*输入参数:pubMBPDU:请求PDU 地址
                         pusMBPDULen 响应PDU长度
                         pxMBSRegisterCB :0x14功能码回调函数
*输出参数:无
*返回值:  MB_PDU_EX_ILLEGAL_DATA_ADDRESS 非法数据地址
                      MB_PDU_EX_ILLEGAL_DATA_VALUE 非法数据
                      MB_PDU_EX_NONE  正常响应
*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/9/18                              lwe004
******************************************************************/
/*******************************************************************/
#if MBS_FUNC_RD_FILES_ENABLED == 1
eMBException 
eMBSFuncReadFileRecord ( UBYTE * pubMBPDU, USHORT * pusMBPDULen, const xMBSRegisterCB * pxMBSRegisterCB )
MB_CDECL_SUFFIX
{
	xMBSFileRecordReq_t arxSubRequests[MBS_FUNC_RD_FILES_REQ_NUM]; /*存储每个子请求信息*/
        
	UBYTE	*pubFrameCur = NULL;
	USHORT  usi = 0;
	USHORT  usMBPDULen = 0;		/*存储请求PDU 数据长度*/
	USHORT  usIndex = 0;
	USHORT  usRecordLengthCount = 0;	/*多个文件号记录长度和*/
	USHORT  usResponseDataSize = 0;  /*响应PDU中响应数据长度*/
	
	eMBException    eStatus = MB_PDU_EX_ILLEGAL_DATA_ADDRESS;

	if (NULL == pubMBPDU )
	{
	  return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
	}
	
	if (NULL == pusMBPDULen )
	{
	  return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
	}
		
	if (NULL == pxMBSRegisterCB )
	{
      return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
	}
	
    /*判断用户回调函数是否合法*/
	if( NULL == pxMBSRegisterCB->peMBSReFileRecordCB )
    {
	  return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
    }
	
	/*存储请求PDU 数据长度*/
	usMBPDULen = *pusMBPDULen;
	
	memset(arxSubRequests, 0, sizeof(arxSubRequests ));
	
	/*判断是否超出最大请求个数*/
	if (MBS_FUNC_RD_FILES_REQ_NUM > MBS_PDU_FUNC_RD_FILE_MAX_SUBREQUESTS )
	{
	  return MB_PDU_EX_ILLEGAL_DATA_VALUE;
	}

	/*判断最小请求个数*/
	if ( MBS_FUNC_RD_FILES_REQ_NUM < MBS_PDU_FUNC_FILE_RECORD_SUBREQUESTS_MIN )
	{
	  return MB_PDU_EX_ILLEGAL_DATA_VALUE;
	}


	/*字节数偏移地址*/
	usIndex = MB_PDU_FUNC_RD_FILEBYTE_NUM_OFF;
	/*解析请求PDU*/
	for ( usi = 0; usi < MBS_FUNC_RD_FILES_REQ_NUM; ++usi )
	{
	  /*记录子请求参考类型*/
	  arxSubRequests[usi].ucReferenceType = ( UCHAR ) pubMBPDU[++usIndex] ;
	  /*记录子请求文件号*/
	  arxSubRequests[usi].usFileNumber = ( ( USHORT ) pubMBPDU[++usIndex] << 8U );
	  arxSubRequests[usi].usFileNumber |= ( USHORT ) ( pubMBPDU[++usIndex] );
	  /*记录子请求记录号*/
	  arxSubRequests[usi].usRecordNumber = ( ( USHORT ) pubMBPDU[++usIndex] << 8U );
	  arxSubRequests[usi].usRecordNumber |= ( USHORT ) ( pubMBPDU[++usIndex] );
	  /*记录子请求记录长度*/
	  arxSubRequests[usi].usRecordLength =  ( ( USHORT ) pubMBPDU[++usIndex] << 8U );
	  arxSubRequests[usi].usRecordLength |= ( USHORT ) ( pubMBPDU[++usIndex] );
	}
	
    /*计算记录数据整长度*/
	for ( usi = 0; usi < MBS_FUNC_RD_FILES_REQ_NUM; ++usi )
	{
	  usRecordLengthCount = usRecordLengthCount + arxSubRequests[usi].usRecordLength;
	}
	/*计算响应PDU中响应数据长度*/
	usResponseDataSize= (usRecordLengthCount * MB_SIGNAL_RECORD_DATA_SIZE + 
		                 MBS_FUNC_RD_FILES_REQ_NUM * (MB_REFER_TYPE_SIZE + MB_FILE_RESPONSE_DATA_SIZE));
	
	if ( usResponseDataSize > MBS_PDU_FUNC_RD_FILE_RECORD_RESPONSE_DATA_LENGTH_MAX )	
	{
	  return MB_PDU_EX_ILLEGAL_DATA_VALUE;
	}

	if ( usResponseDataSize < MBS_PDU_FUNC_RD_FILE_RECORD_RESPONSE_DATA_LENGTH_MIN )	
	{
	  return MB_PDU_EX_ILLEGAL_DATA_VALUE;
	}
	
    /*计算响应PDU 数据长度*/
	*pusMBPDULen = MB_PDU_FUNC_OFF;
	/*功能码长度*/
	*pusMBPDULen += ( USHORT ) MB_FUNCTION_CODE_SIZE;
	/*响应数据长度*/
	*pusMBPDULen += ( USHORT ) MB_RESPONSE_DATA_SIZE;
	/*响应PDU 整体长度*/
	*pusMBPDULen += usResponseDataSize;

	/*判断请求PDU 数据长度是否合法*/
    if ( usMBPDULen < ( MB_PDU_FUNC_RD_FILE_RECORDREQ_SIZE_MIN + MB_PDU_SIZE_MIN ) )
    {
	  return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }

	/*判断请求PDU 字节数大小是否合法*/
	if ( pubMBPDU[MB_PDU_FUNC_RD_FILEBYTE_NUM_OFF] > MBS_PDU_FUNC_RD_FILE_RECORD_RESPONSE_DATA_LENGTH_MAX )	
	{
	  return MB_PDU_EX_ILLEGAL_DATA_VALUE;
	}

	if ( pubMBPDU[MB_PDU_FUNC_RD_FILEBYTE_NUM_OFF] < MBS_PDU_FUNC_RD_FILE_RECORD_RESPONSE_DATA_LENGTH_MIN )	
	{
	  return MB_PDU_EX_ILLEGAL_DATA_VALUE;
	}

	/*判断文件号记录号是否合法*/
	for (  usi = 0; usi < MBS_FUNC_RD_FILES_REQ_NUM; ++usi )
	{
	  /*判断参考类型是否匹配*/
	  if ( MBS_PDU_FUNC_FILE_REFERENCE_TYPE != arxSubRequests[usi].ucReferenceType)
	  {
	    return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
	  }

	  /*判断文件号是否合法*/
      /* BEGIN: Deleted by lv we004, 2017/2/5 */
      /* 由于主站的文件号为0也是合法，所以这里的判断不需要 */
      /* 为消除编译告警，删除 */
        /*
      if ( arxSubRequests[usi].usFileNumber < MBS_PDU_FUNC_FILE_NUMBER_MIN )
	  {
		return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
	  }	
      */
      /* END:   Deleted by lv we004, 2017/2/5  */
       

	  /*判断记录号是否合法*/
	  if( arxSubRequests[usi].usRecordNumber > MBS_PDU_FUNC_RECORD_NUMBER_MAX )  
	  {
		return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
	  }
	}

	/*填响应PDU功能码*/
    pubFrameCur = &pubMBPDU[MB_PDU_FUNC_OFF];
    
/* BEGIN: Modified by l  we010 , 2017/10/8 PN: 如果有告警，响应PDU中的功能码变成0x54*/
    if( FALSE == Get_WarnStatus() )
    {
        *pubFrameCur++ = MBS_FUNC_RD_FILE_RECORD;
    }
    else
    {
        *pubFrameCur++ = ( MBS_FUNC_RD_FILE_RECORD + WARN_CODE );
    }
/* END:   Modified by l  we010 , 2017/10/8    */
	
    /*填响应PDU 响应数据长度*/
	*pubFrameCur++ = usResponseDataSize;
	
	/*填每个子请求数据*/
	for ( usi = 0; usi < MBS_FUNC_RD_FILES_REQ_NUM; ++usi )
	{
	  /*写文件响应长度*/
	  *pubFrameCur++ = arxSubRequests[usi].usRecordLength * MB_SIGNAL_RECORD_DATA_SIZE + MB_REFER_TYPE_SIZE;
	  
	  /*写参考类型*/
	  *pubFrameCur++ = MBS_PDU_FUNC_FILE_REFERENCE_TYPE;
	  	 
	   /*如果回调函数发生错误，错误状态需要返回*/
	   eStatus = pxMBSRegisterCB->peMBSReFileRecordCB( pubFrameCur, &arxSubRequests[usi] );

	   if ( MB_PDU_EX_NONE != eStatus )
	   {
	   	 return eStatus;
	   }

	   /*计算下一个子请求在PDU 中所占的地址*/
	   pubFrameCur = pubFrameCur + arxSubRequests[usi].usRecordLength * MB_SIGNAL_RECORD_DATA_SIZE;
	   pubFrameCur++;	   
	}

	return MB_PDU_EX_NONE;
}
#endif

/******************************************************************
*函数名称:eMBSFuncWriteFileRecord
*功能描述:读文件记录，实现0x14功能码
*输入参数:pubMBPDU:请求PDU 地址
                         pusMBPDULen 响应PDU长度
                         pxMBSRegisterCB :0x14功能码回调函数
*输出参数:无
*返回值:  MB_PDU_EX_ILLEGAL_DATA_ADDRESS 非法数据地址
                      MB_PDU_EX_ILLEGAL_DATA_VALUE 非法数据
                      MB_PDU_EX_NONE  正常响应

*其它说明:
*修改日期    版本号   修改人    修改内容
*---------------------------------------------------
*2017/10/17                              lwe004
******************************************************************/
#if MBS_FUNC_WR_FILES_ENABLED == 1
eMBException 
eMBSFuncWriteFileRecord ( UBYTE * pubMBPDU, USHORT * pusMBPDULen, const xMBSRegisterCB * pxMBSRegisterCB )
MB_CDECL_SUFFIX
{
    xMBSFileRecordReq_t arxSubRequests[MBS_FUNC_WR_FILES_REQ_NUM]; /*存储每个子请求信息*/   
    USHORT  usi = 0;
    USHORT  usIndex = 0;
    eMBException    eStatus = MB_PDU_EX_ILLEGAL_DATA_ADDRESS;

    if (NULL == pubMBPDU )
    {
        return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
    }   
    if (NULL == pusMBPDULen )
    {
        return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
    }   
    if (NULL == pxMBSRegisterCB )
    {
        return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
    }
    /*判断用户回调函数是否合法*/
    if( NULL == pxMBSRegisterCB->peMBSWrFileRecordCB )
    {
        return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
    }

    /*判断最小请求个数，无最大请求数目，请求数目与请求数据长度有关*/
    if ( MBS_FUNC_WR_FILES_REQ_NUM < MBS_PDU_FUNC_FILE_RECORD_SUBREQUESTS_MIN )
    {
        return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }

    /*判断请求PDU 数据长度是否合法*/
    if ( *pusMBPDULen > MBS_PDU_FUNC_WR_FILE_PDULEN_MAX )
    {
        return MB_PDU_EX_ILLEGAL_DATA_VALUE;
    }

    /*数据长度偏移地址*/
    usIndex = MB_PDU_FUNC_WR_NUMLEN_OFF;
    /*解析请求PDU*/
    for ( usi = 0; usi < MBS_FUNC_WR_FILES_REQ_NUM; ++usi )
    {
        /*记录子请求参考类型*/
        arxSubRequests[usi].ucReferenceType = ( UCHAR ) pubMBPDU[++usIndex] ;
        /*记录子请求文件号*/
        arxSubRequests[usi].usFileNumber = ( ( USHORT ) pubMBPDU[++usIndex] << 8U );
        arxSubRequests[usi].usFileNumber |= ( USHORT ) ( pubMBPDU[++usIndex] );
        /*记录子请求记录号*/
        arxSubRequests[usi].usRecordNumber = ( ( USHORT ) pubMBPDU[++usIndex] << 8U );
        arxSubRequests[usi].usRecordNumber |= ( USHORT ) ( pubMBPDU[++usIndex] );
        /*记录子请求记录长度*/
        arxSubRequests[usi].usRecordLength =  ( ( USHORT ) pubMBPDU[++usIndex] << 8U );
        arxSubRequests[usi].usRecordLength |= ( USHORT ) ( pubMBPDU[++usIndex] );	
    
        usIndex = usIndex +arxSubRequests[usi].usRecordLength * MB_SIGNAL_RECORD_DATA_SIZE;
    }

   /*判断文件号记录号是否合法*/
   for (  usi = 0; usi < MBS_FUNC_RD_FILES_REQ_NUM; ++usi )
   {
        /*判断参考类型是否匹配*/
        if ( MBS_PDU_FUNC_FILE_REFERENCE_TYPE != arxSubRequests[usi].ucReferenceType)
        {
           return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
        }

        /*判断文件号是否合法*/
        /* BEGIN: Deleted by lv we004, 2017/2/5 */
        /* 由于主站的文件号为0也是合法，所以这里的判断不需要 */
      /* 为消除编译告警，删除 */
        /*
        if ( arxSubRequests[usi].usFileNumber < MBS_PDU_FUNC_FILE_NUMBER_MIN )
        {
           return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
        }   
        */
        /* END:   Deleted by lv we004, 2017/2/5   */
         

        /*判断记录号是否合法*/
        if( arxSubRequests[usi].usRecordNumber > MBS_PDU_FUNC_RECORD_NUMBER_MAX )  
        {
           return MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
        }
   }
   
   /*判断是否有告警信息*/
   if( FALSE == Get_WarnStatus() )
   {
        pubMBPDU[MB_PDU_FUNC_OFF] = MBS_FUNC_WR_FILE_RECORD;
   }
   else
   {
       pubMBPDU[MB_PDU_FUNC_OFF] = ( MBS_FUNC_WR_FILE_RECORD + WARN_CODE );
   }   

   /*调用用户回调函数实现相关功能*/
   /*获取第一个子请求记录数据偏移地址*/
   usIndex = MB_PDU_FUNC_WR_FIRST_RECORDDATA_OFF;
   for (  usi = 0; usi < MBS_FUNC_RD_FILES_REQ_NUM; ++usi )
   { 
        /*调用用户回调函数*/
       eStatus = pxMBSRegisterCB->peMBSWrFileRecordCB( &pubMBPDU[++usIndex], &arxSubRequests[usi] );
        /*检测调用结果*/
        if ( MB_PDU_EX_NONE != eStatus )
        {
            return eStatus;
        }
   
        /*记录下一个子请求记录数据偏移地址*/
        usIndex = usIndex + arxSubRequests[usi].usRecordLength * MB_SIGNAL_RECORD_DATA_SIZE + 
                    MB_PDU_FUNC_WR_SIGNAL_REQ_INF_SIZE;
   }

   return MB_PDU_EX_NONE;
}
#endif



