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

#ifndef D_STATUSREQUEST_H
#define D_STATUSREQUEST_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_statusRequest(s) ((d_statusRequest)(s))

d_statusRequest d_statusRequestNew  (d_admin admin);

void            d_statusRequestFree (d_statusRequest request);

#if defined (__cplusplus)
}
#endif

#endif /*D_STATUSREQUEST_H*/
