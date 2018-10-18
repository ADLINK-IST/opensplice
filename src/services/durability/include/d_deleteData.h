/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef D_DELETEDATA_H
#define D_DELETEDATA_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_deleteData(s) ((d_deleteData)(s))

d_deleteData    d_deleteDataNew     (d_admin admin,
                                     os_timeW actionTime,
                                     const c_char* partitionExpr,
                                     const c_char* topicExpr);

void            d_deleteDataFree    (d_deleteData deleteData);

#if defined (__cplusplus)
}
#endif

#endif /*D_DELETEDATA_H*/
