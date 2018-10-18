/* -*- mode: c; c-file-style: "k&r"; c-basic-offset: 4; -*- */

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

#ifndef D_STOREKV_H
#define D_STOREKV_H

#include "d__types.h"
#include "u_participant.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_storeKV(s) ((d_storeKV)(s))

d_storeKV       d_storeNewKV               (u_participant participant);

d_storeResult   d_storeFreeKV              (d_storeKV store);

#if defined (__cplusplus)
}
#endif

#endif /* D_STOREKV_H */
