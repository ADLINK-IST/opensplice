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

#ifndef D_STOREXML_H
#define D_STOREXML_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_storeXML(s) ((d_storeXML)(s))

d_storeXML      d_storeNewXML               (u_participant participant);

d_storeResult   d_storeFreeXML              (d_storeXML store);

#if defined (__cplusplus)
}
#endif

#endif /*D_STOREXML_H*/
