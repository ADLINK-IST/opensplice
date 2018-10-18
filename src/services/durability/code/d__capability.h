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

#ifndef D__CAPABILITY_H
#define D__CAPABILITY_H

#include "d__types.h"
#include "d__admin.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Number of capabilities for this durability service */
#define D_NUMBER_OF_CAPABILITIES   (5)

/* List of capability names */
#define D_CAPABILITY_GROUP_HASH    "groupHash"
#define D_CAPABILITY_EOT_SUPPORT   "EOTsupport"
#define D_CAPABILITY_Y2038READY    "Y2038Ready"
#define D_CAPABILITY_MASTER_SELECTION "masterSelection"
#define D_CAPABILITY_INCARNATION   "incarnation"

/**
 * \brief The d_capability cast macro.
 *
 * This macro casts an object to a d_capability object.
 */
#define d_capability(s) ((d_capability)(s))

d_capability                            d_capabilityNew                     (d_admin admin, c_ulong incarnation);

void                                    d_capabilityFree                    (d_capability capability);

char *                                  d_capabilityToString                (d_capability capability);

#if defined (__cplusplus)
}
#endif

#endif /* D__CAPABILITY_H */
