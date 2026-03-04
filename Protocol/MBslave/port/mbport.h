/*
 * MODBUS Library: ARM STM32 Port (FWLIB 2.0x)
 * Copyright (c) Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * ARM STM32 Port by Niels Andersen, Elcanic A/S <niels.andersen.elcanic@gmail.com>
 *
 * $Id: mbport.h,v 1.1 2017/08/06 20:17:01 embedded-solutions.cwalter Exp $
 */

#ifndef _MBM_PORT_H
#define _MBM_PORT_H

#include <assert.h>
//#include "stm32f10x_lib.h"  /* STM32 FW library */

#ifdef _cplusplus
extern          "C"
{
#endif

/* ----------------------- Defines ------------------------------------------*/

#define INLINE
#define STATIC                         static

#define PR_BEGIN_EXTERN_C              extern "C" {
#define PR_END_EXTERN_C                }
#define MBP_ASSERT( x )                ((void)0)

/* BEGIN: Added by lwe004, 2017/9/27   PN:临界区保护控制，设置1 时保护，其余不保护 */

#ifndef CRITICAL_PROTECT_CONTROL
#define  CRITICAL_PROTECT_CONTROL  ( 1 )
#endif
/* END:   Modified by lwe004, 2017/9/27    */

/* END:   Added by lwe004, 2017/9/27 */

/* BEGIN: Modified by lwe004, 2017/9/27 PN:临界区保护控制 */
/*
#define MBP_ENTER_CRITICAL_SECTION( )  vMBPEnterCritical( )
#define MBP_EXIT_CRITICAL_SECTION( )   vMBPExitCritical( )
*/
#if CRITICAL_PROTECT_CONTROL == 1
#define MBP_ENTER_CRITICAL_SECTION( )  
#define MBP_EXIT_CRITICAL_SECTION( )   
#else
#define MBP_ENTER_CRITICAL_SECTION( )  vMBPEnterCritical( )
#define MBP_EXIT_CRITICAL_SECTION( )   vMBPExitCritical( )
#endif
/* END:   Modified by lwe004, 2017/9/27    */

#define MBP_FORCE_SERV2PROTOTYPES      ( 1 )  //zhangyong
  
#ifndef TRUE
#define TRUE                           ( BOOL )1
#endif

#ifndef FALSE
#define FALSE                          ( BOOL )0
#endif

#define MB_PREEMP_PRIORITY              ( 0 )
#define MBP_EVENTHDL_INVALID            NULL
#define MBP_TIMERHDL_INVALID            NULL
#define MBP_SERIALHDL_INVALID           NULL
#define MBP_TCPHDL_INVALID              NULL

#define MB_UART_1                       1
#define MB_UART_2                       2

/* ----------------------- Type definitions ---------------------------------*/
typedef void       *xMBPEventHandle;
typedef void       *xMBPTimerHandle;
typedef void       *xMBPSerialHandle;
typedef void       *xMBPTCPHandle;
typedef void       *xMBPTCPClientHandle;

typedef char        BOOL;

typedef char        BYTE;
typedef unsigned char UBYTE;

typedef unsigned char UCHAR;
typedef char        CHAR;

typedef unsigned short USHORT;
typedef short       SHORT;

typedef unsigned long ULONG;
typedef long        LONG;

typedef enum
{
    MBP_DEBUGPIN_0 = 0,
    MBP_DEBUGPIN_1 = 1
} eMBPDebugPin;
/* ----------------------- Function prototypes ------------------------------*/
void                vMBPAssert( UBYTE * file, ULONG line );
void                vMBPEnterCritical( void );
void                vMBPExitCritical( void );
void                vMBPSetDebugPin( eMBPDebugPin ePinName, BOOL bTurnOn );
#ifdef _cplusplus
}
#endif

#endif
