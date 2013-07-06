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

#ifndef D_NAMESPACESREQUEST_H
#define D_NAMESPACESREQUEST_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_nameSpacesRequest(s) ((d_nameSpacesRequest)(s))

d_nameSpacesRequest d_nameSpacesRequestNew  (d_admin admin);

void                d_nameSpacesRequestFree  (d_nameSpacesRequest request);

#if defined (__cplusplus)
}
#endif

#endif /*D_NAMESPACESREQUEST_H*/
