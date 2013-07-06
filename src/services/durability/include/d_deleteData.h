/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
                                     d_timestamp actionTime,
                                     const c_char* partitionExpr,
                                     const c_char* topicExpr);

void            d_deleteDataFree    (d_deleteData deleteData);

#if defined (__cplusplus)
}
#endif

#endif /*D_DELETEDATA_H*/
