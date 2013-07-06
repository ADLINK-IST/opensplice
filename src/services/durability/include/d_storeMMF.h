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

#ifndef D_STOREMMF_H
#define D_STOREMMF_H

#include "d__types.h"
#include "d_store.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_storeMMF(s) ((d_storeMMF)(s))

#define d_storeMMFGetKernel(s) (d_storeMMF(s)->storeKernel)

#define d_storeMMFGetBase(s) (c_getBase(d_storeMMFGetKernel(s)))

d_storeMMF      d_storeNewMMF               (u_participant participant);

d_storeResult   d_storeFreeMMF              (d_storeMMF store);

#if defined (__cplusplus)
}
#endif

#endif /*D_STOREMEMMAPFILE_H*/
