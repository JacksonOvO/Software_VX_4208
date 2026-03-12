/* 
 * MODBUS Slave Library: A portable MODBUS slave for MODBUS ASCII/RTU/TCP.
 * Copyright (c) 2007 Christian Walter <wolti@sil.at>
 * All rights reserved.
 *
 * $Id: mbutils.c,v 1.2 2008/03/06 22:01:48 cwalter Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbframe.h"
#include "common/mbutils.h"

/* ----------------------- Platform includes --------------------------------*/

/* ----------------------- Modbus includes ----------------------------------*/

/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Static variables ---------------------------------*/

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/

eMBException
eMBErrorcodeToException( eMBErrorCode eCode )
{
    eMBException    eException;

    switch ( eCode )
    {
    case MB_EX_ILLEGAL_FUNCTION:
        eException = MB_PDU_EX_ILLEGAL_FUNCTION;
        break;
    case MB_EX_ILLEGAL_DATA_ADDRESS:
        eException = MB_PDU_EX_ILLEGAL_DATA_ADDRESS;
        break;
    case MB_EX_ILLEGAL_DATA_VALUE:
        eException = MB_PDU_EX_ILLEGAL_DATA_VALUE;
        break;
    case MB_EX_SLAVE_DEVICE_FAILURE:
        eException = MB_PDU_EX_SLAVE_DEVICE_FAILURE;
        break;
    case MB_EX_ACKNOWLEDGE:
        eException = MB_PDU_EX_ACKNOWLEDGE;
        break;
    case MB_EX_SLAVE_BUSY:
        eException = MB_PDU_EX_SLAVE_BUSY;
        break;
    case MB_EX_MEMORY_PARITY_ERROR:
        eException = MB_PDU_EX_MEMORY_PARITY_ERROR;
        break;
    case MB_EX_GATEWAY_PATH_UNAVAILABLE:
        eException = MB_PDU_EX_GATEWAY_PATH_UNAVAILABLE;
        break;
    case MB_EX_GATEWAY_TARGET_FAILED:
        eException = MB_PDU_EX_GATEWAY_TARGET_FAILED;
        break;
    default:
        eException = MB_PDU_EX_SLAVE_DEVICE_FAILURE;
        break;
    }
    return eException;
}

eMBErrorCode
eMBExceptionToErrorcode( UBYTE eMBPDUException )
{
    eMBErrorCode    eStatus = MB_EIO;

    switch ( eMBPDUException )
    {
    case MB_PDU_EX_ILLEGAL_FUNCTION:
        eStatus = MB_EX_ILLEGAL_FUNCTION;
        break;
    case MB_PDU_EX_ILLEGAL_DATA_ADDRESS:
        eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
        break;
    case MB_PDU_EX_ILLEGAL_DATA_VALUE:
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        break;
    case MB_PDU_EX_SLAVE_DEVICE_FAILURE:
        eStatus = MB_EX_SLAVE_DEVICE_FAILURE;
        break;
    case MB_PDU_EX_ACKNOWLEDGE:
        eStatus = MB_EX_ACKNOWLEDGE;
        break;
    case MB_PDU_EX_SLAVE_BUSY:
        eStatus = MB_EX_SLAVE_BUSY;
        break;
    case MB_PDU_EX_MEMORY_PARITY_ERROR:
        eStatus = MB_EX_MEMORY_PARITY_ERROR;
        break;
    case MB_PDU_EX_GATEWAY_PATH_UNAVAILABLE:
        eStatus = MB_EX_GATEWAY_PATH_UNAVAILABLE;
        break;
    case MB_PDU_EX_GATEWAY_TARGET_FAILED:
        eStatus = MB_EX_GATEWAY_TARGET_FAILED;
        break;
    default:
        break;
    }
    return eStatus;
}
