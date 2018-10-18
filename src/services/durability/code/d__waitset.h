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

#ifndef D__WAITSET_H
#define D__WAITSET_H

#include "d__types.h"
#include "d__lock.h"
#include "u_object.h"
#include "os_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Prototype for d_waitsetAction function */
typedef c_ulong (*d_waitsetAction)(u_object object, v_waitsetEvent event, c_voidp usrData);

/**
 * Macro that checks the d_waitset validity.
 * Because d_waitset is a concrete class typechecking is required.
 */
#define             d_waitsetIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_WAITSET)

/**
 * Macro that checks the d_waitsetEntity validity.
 * Because d_waitsetEntity is a concrete class typechecking is required.
 */
#define             d_waitsetEntityIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_WAITSET_ENTITY)

/**
 * \brief The d_waitset cast macro.
 *
 * This meacro casts an object to a d_waitset object.
 */
#define d_waitset(_this) ((d_waitset)(_this))

/**
 * \brief The d_waitsetEntity cast macro.
 *
 * This macro casts an object to a d_waitsetEntity object.
 */
#define d_waitsetEntity(_this) ((d_waitsetEntity)(_this))

C_STRUCT(d_waitsetEntity) {
    C_EXTENDS(d_object);
    c_char* name;
    d_waitset waitset;
    u_object object;
    c_ulong mask;
    d_waitsetAction action;
    os_threadAttr attr;
    c_voidp usrData;
};

C_STRUCT(d_waitset) {
    C_EXTENDS(d_lock);
    c_bool terminate;
    d_subscriber subscriber;
    c_iter entities;
    c_iter threads;
};

d_waitset           d_waitsetNew                        (d_subscriber subscriber);

void                d_waitsetDeinit                     (d_waitset waitset);

void                d_waitsetFree                       (d_waitset waitset);

c_bool              d_waitsetAttach                     (d_waitset waitset,
                                                         d_waitsetEntity we);

c_bool              d_waitsetDetach                     (d_waitset waitset,
                                                         d_waitsetEntity we);

d_subscriber        d_waitsetGetSubscriber              (d_waitset waitset);

d_waitsetEntity     d_waitsetEntityNew                  (const c_char* name,
                                                         u_object object,
                                                         d_waitsetAction action,
                                                         c_ulong mask,
                                                         os_threadAttr attr,
                                                         c_voidp usrData);

void                d_waitsetEntityDeinit               (d_waitsetEntity waitsetEntity);

void                d_waitsetEntityFree                 (d_waitsetEntity we);


#if defined (__cplusplus)
}
#endif

#endif /* D__WAITSET_H */
