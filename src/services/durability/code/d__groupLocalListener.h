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
#ifndef D__GROUPLOCALLISTENER_H
#define D__GROUPLOCALLISTENER_H

#include "d__types.h"
#include "d__listener.h"

#if defined (__cplusplus)
extern "C" {
#endif

d_groupLocalListener d_groupLocalListenerNew             (d_subscriber subscriber);

void                 d_groupLocalListenerFree            (d_groupLocalListener listener);

c_bool               d_groupLocalListenerStart           (d_groupLocalListener listener);

c_bool               d_groupLocalListenerStop            (d_groupLocalListener listener);

void                 d_groupLocalListenerHandleAlignment (d_groupLocalListener listener,
                                                          d_group dgroup,
                                                          d_readerRequest request);

void                 d_groupLocalListenerDetermineNewMasters(d_groupLocalListener listener,
                                                          c_iter nameSpaces);

void                 handleMergeAlignment                (d_groupLocalListener listener,
                                                          d_conflict conflict,
                                                          d_nameSpace fellowNameSpace,
                                                          c_iter fellows,
                                                          d_mergeState newState);

#if defined (__cplusplus)
}
#endif

#endif /* D__GROUPLOCALLISTENER_H */
