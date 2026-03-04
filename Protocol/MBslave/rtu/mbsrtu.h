/* 
 * MODBUS Slave Library: A portable MODBUS slave for MODBUS ASCII/RTU/TCP.
 * Copyright (c) 2007 Christian Walter <wolti@sil.at>
 * All rights reserved.
 *
 * $Id: mbsrtu.h,v 1.3 2017/01/08 19:58:31 embedded-solutions.cwalter Exp $
 */

#ifdef __cplusplus
//PR_BEGIN_EXTERN_C
#endif

#if MBS_RTU_ENABLED == 1

/*! 
 * \if INTERNAL_DOCS
 * \addtogroup mbs_rtu_int
 * @{
 * \endif
 */

/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Function prototypes ------------------------------*/
/*! \brief Configure a MODBUS slave RTU instances.
 * \internal
 *
 * \param pxIntHdl An internal handle.
 * \param ubPort The port. This value is passed through to the porting layer.
 * \param ulBaudRate Baudrate.
 * \param eParity Parity.
 * \param ucStopBits Number of stop bits.
 *
 * \return eMBErrorCode::MB_ENOERR if a new instance has been created. In 
 *   this case the members pxFrameHdl, pFrameSendFN, pFrameRecvFN,
 *   pFrameCloseFN and pFrameManagementFN in the handle are updated to point 
 *   to this RTU instance. In case of an invalid handle or baudrate it returns
 *   eMBErrorCode::MB_EINVAL. In case of a porting layer error it returns
 *   eMBErrorCode::MB_EPORTERR.
 */
eMBErrorCode
eMBSSerialRTUInit( xMBSInternalHandle * pxIntHdl, UBYTE ubPort, ULONG ulBaudRate, eMBSerialParity eParity, UCHAR ucStopBits );
/* BEGIN: Added by lwe004, 2017/9/26   PN:协议栈接收数据 */
/*协议栈接收数据函数*/
eMBErrorCode RTUFrameReceive( xMBSHandle xHdl);

/* END:   Added by lwe004, 2017/9/26 */
/*!
 * \if INTERNAL_DOCS
 * @}
 * \endif
 */

#endif

#ifdef __cplusplus
//PR_END_EXTERN_C
#endif
