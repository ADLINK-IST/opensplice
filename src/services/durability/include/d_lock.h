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

#ifndef D_LOCK_H
#define D_LOCK_H

#include "d__types.h"
#include "os_mutex.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_CLASS(d_lock);

C_STRUCT(d_lock){
    C_EXTENDS(d_object);
    d_objectDeinitFunc deinit;
    os_mutex lock;
};

#define d_lock(d)    ((d_lock)(d))

void    d_lockInit  (d_lock object,
                     d_kind kind,
                     d_objectDeinitFunc deinit);

void    d_lockDeinit(d_object object);

void    d_lockFree  (d_lock object,
                     d_kind kind);

void    d_lockLock  (d_lock object);

void    d_lockUnlock(d_lock object);

#if defined (__cplusplus)
}
#endif

#endif /*D_LOCK_H*/
