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

#ifndef D__CONFLICT_MONITOR_H
#define D__CONFLICT_MONITOR_H

#include "d__types.h"
#include "d__conflict.h"
#include "d__admin.h"
#include "os_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_conflictMonitor validity.
 * Because d_conflictMonitor is a concrete class typechecking is required.
 */
#define             d_conflictMonitorIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_CONFLICT_MONITOR)

/**
 * \brief The <code>d_conflictMonitor</code> cast macro.
 *
 * This macro casts an object to a <code>d_conflictMonitor</code> object.
 */
#define d_conflictMonitor(_this) ((d_conflictMonitor)(_this))

C_STRUCT(d_conflictMonitor) {
    C_EXTENDS(d_lock);
    os_timeM lastTimeChecked;
    d_admin admin;
};


d_conflictMonitor  d_conflictMonitorNew                    (d_admin admin);

void               d_conflictMonitorDeinit                 (d_conflictMonitor conflictMonitor);

void               d_conflictMonitorFree                   (d_conflictMonitor conflictMonitor);

void               d_conflictMonitorCheckForConflicts      (d_conflictMonitor conflictMonitor,
                                                            d_fellow fellow,
                                                            d_nameSpace nameSpace);

void               d_conflictMonitorCheckFellowDisconnected   (d_conflictMonitor conflictMonitor,
                                                            d_fellow fellow);

void               d_conflictMonitorCheckFellowConnected   (d_conflictMonitor conflictMonitor,
                                                            d_fellow fellow);

void               d_conflictMonitorCheckFederationDisconnected(d_conflictMonitor conflictMonitor);

d_conflict         d_conflictMonitorEvaluateConflict       (d_conflictMonitor conflictMonitor,
                                                            d_conflict conflict);




#if defined (__cplusplus)
}
#endif

#endif /* D__CONFLICT_MONITOR_H */
