/* -*- mode: c; c-file-style: "k&r"; c-basic-offset: 4; -*- */

/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef D_STOREKV_H
#define D_STOREKV_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_storeKV(s) ((d_storeKV)(s))

d_storeKV       d_storeNewKV               (u_participant participant);

d_storeResult   d_storeFreeKV              (d_storeKV store);

#if defined (__cplusplus)
}
#endif

#endif /*D_STOREKV_H*/
