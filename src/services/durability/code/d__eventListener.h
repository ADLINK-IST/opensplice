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

#ifndef D__EVENTLISTENER_H
#define D__EVENTLISTENER_H

#include "d__types.h"
#include "d__listener.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define D_NONE                             (0)
#define D_GROUP_LOCAL_NEW                  (0x0001U << 0)
#define D_FELLOW_NEW                       (0x0001U << 1)
#define D_FELLOW_REMOVED                   (0x0001U << 2)
#define D_FELLOW_LOST                      (0x0001U << 3)
#define D_NAMESPACE_NEW                    (0x0001U << 4)
#define D_NAMESPACE_STATE_CONFLICT         (0x0001U << 5)
#define D_NAMESPACE_MASTER_CONFLICT        (0x0001U << 6)
#define D_NAMESPACE_DELAYED_INITIAL        (0x0001U << 7)
#define D_GROUP_LOCAL_COMPLETE             (0x0001U << 8)
/**
 * Macro that checks the d_eventListener validity.
 * Because d_eventListener is a concrete class typechecking is required.
 */
#define             d_eventListenerIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_EVENT_LISTENER)

/**
 * \brief The d_eventListener cast macro.
 *
 * This macro casts an object to a d_eventListener object.
 */
#define d_eventListener(_this) ((d_eventListener)(_this))

typedef c_bool      (*d_eventListenerFunc)      (c_ulong event, 
                                                 d_fellow fellow, 
                                                 d_nameSpace nameSpace,
                                                 d_group group, 
                                                 c_voidp userData,
                                                 c_voidp args);

C_STRUCT(d_eventListener){
    C_EXTENDS(d_object);
    c_ulong interest;
    d_eventListenerFunc func;
    c_voidp args;
};

d_eventListener     d_eventListenerNew          (c_ulong interest,
                                                 d_eventListenerFunc func,
                                                 c_voidp args);

c_voidp             d_eventListenerGetUserData  (d_eventListener listener);

void                d_eventListenerFree         (d_eventListener listener);

void                d_eventListenerDeinit       (d_eventListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__EVENTLISTENER_H */
