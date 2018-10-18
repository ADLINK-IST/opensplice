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

#ifndef D_GROUPSREQUEST_H
#define D_GROUPSREQUEST_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_groupsRequest(s) ((d_groupsRequest)(s))

d_groupsRequest     d_groupsRequestNew     (d_admin admin, 
                                            d_partition partition,
                                            d_topic topic);

void                d_groupsRequestFree    (d_groupsRequest groupsRequest);

#if defined (__cplusplus)
}
#endif

#endif /* D_GROUPSREQUEST_H */
