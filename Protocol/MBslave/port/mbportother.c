/*
 * MODBUS Library: ARM STM32 Port
 * Copyright (c) Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbportother.c,v 1.1 2017/08/06 20:17:01 embedded-solutions.cwalter Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
#include "stm32f30x_conf.h"
#include "stm32f30x.h"//zhangyong
#include "core_cmFunc.h"//zhangyong

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"
#include "common/mbtypes.h"
#include "common/mbframe.h"
#include "common/mbutils.h"
#include "common.h"

/* ----------------------- Modbus includes ----------------------------------*/

/* ----------------------- Defines ------------------------------------------*/
#define PORT_INTERRUPT_PRIORITY_MAX     ( 1 )

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Static variables ---------------------------------*/

//static UBYTE    ubNesting = 0;
//static unsigned long ulOldBasePRI;

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/

//void
//assert_failed( UBYTE * file, ULONG line )
//{
  //  ( void )file;
  //  ( void )line;
//    vMBPAssert( file, line );
//}

void
vMBPAssert( UBYTE * file, ULONG line )
{
//    volatile BOOL   bBreakOut = FALSE;

 //   vMBPEnterCritical(  );
  //  while( !bBreakOut );
}

void
vMBPEnterCritical( void )
{
/* BEGIN: Deleted by lwe004, 2017/9/19 */
/*
    unsigned long   curBASEPri;
  //  unsigned long   curBASEPri = __get_BASEPRI(  );
  //  __set_BASEPRI( PORT_INTERRUPT_PRIORITY_MAX );  zhangyong
    if( 0 == ubNesting )
    {
        ulOldBasePRI = curBASEPri;
    }
    ubNesting++;
*/
/* END:   Deleted by lwe004, 2017/9/19   PN:单任务运行，临界保护部分函数留空 */

}

void
vMBPExitCritical( void )
{
/* BEGIN: Deleted by lwe004, 2017/9/19 */
/*
    ubNesting--;
    if( 0 == ubNesting )
    {

        __set_BASEPRI( ulOldBasePRI );


    }
*/
/* END:   Deleted by lwe004, 2017/9/19   PN:单任务运行，临界保护部分函数留空 */

}
