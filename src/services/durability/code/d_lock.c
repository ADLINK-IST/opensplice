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

#include "d_lock.h"
#include "d_object.h"

void
d_lockInit(
    d_lock lock,
    d_kind kind,
    d_objectDeinitFunc deinit)
{
    os_mutexAttr attr;
    os_result osr;
    
    d_objectInit(d_object(lock), kind, d_lockDeinit);
    
    if(lock){
        lock->deinit = deinit;
        osr = os_mutexAttrInit(&attr);
        
        if(osr == os_resultSuccess){
            attr.scopeAttr = OS_SCOPE_PRIVATE;
            osr = os_mutexInit(&lock->lock, &attr);
            
            if(osr != os_resultSuccess){
                d_objectFree(d_object(lock), kind);
            }
        } else {
            d_objectFree(d_object(lock), kind);
        }
    }
}

void
d_lockDeinit(
    d_object lock)
{
    if(lock){
        if(d_lock(lock)->deinit){
            d_lock(lock)->deinit(lock);
        }
        os_mutexDestroy(&(d_lock(lock)->lock));
    }
}

void
d_lockFree(
    d_lock lock,
    d_kind kind)
{
    assert(d_objectIsValid(d_object(lock), kind) == TRUE);
    
    if(lock){
        d_objectFree(d_object(lock), kind);
    }
}

void
d_lockLock(
    d_lock lock)
{   
    if(lock){
        os_mutexLock(&lock->lock);
    }
}

void
d_lockUnlock(
    d_lock lock)
{   
    if(lock){
        os_mutexUnlock(&lock->lock);
    }
}

