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
#ifndef DLRL_KERNEL_EVENT_DISPATCHER_H
#define DLRL_KERNEL_EVENT_DISPATCHER_H

/* user layer includes */
#include "u_waitset.h"
#include "u_waitsetEvent.h"
#include "u_participant.h"
#include "u_entity.h"

/* DLRL util includes */
#include "DLRL_Exception.h"

/* DLRL kernel includes */
#include "DK_Entity.h"
#include "DK_Types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* this waitset is simply a wrapper around the user layer waitset. It is used to allow the DLRL kernel to
 * reference count the user layer waitset, so that once all references dissapear the user layer waitset destroy
 * function is automatically called! This is usefull when using the waitset in multiple threads at once, but where
 * you only want to delete it once to avoid corrupting the user layer waitset memory!
 */
typedef LOC_boolean (*DK_EventDispatcher_us_eventAction)(
    DK_Entity* _this,
    DLRL_Exception* exception,
    void* userData);

typedef void
(*DK_EventDispatcher_us_waitsetDetachAction)(
    DLRL_Exception* exception,
    u_waitset waitset,
    DK_Entity* interestedEntity);

typedef void
(*DK_EventDispatcher_us_waitsetAttachAction)(
    DLRL_Exception* exception,
    u_waitset waitset,
    DK_Entity* interestedEntity);

struct DK_EventDispatcher_s
{
    DK_Entity entity;
    u_waitset waitset;
    DK_Entity* interestedEntity;
    DK_EventDispatcher_us_eventAction eventAction;
    LOC_boolean terminate;
    void* userData;
    os_threadId threadId;
};

DK_EventDispatcher*
DK_EventDispatcher_new(
    DLRL_Exception* exception,
    void* userData,
    DK_Entity* interestedEntity,
    DK_EventDispatcher_us_eventAction eventAction,
    u_participant participant);

os_threadId
DK_EventDispatcher_us_getThreadId(
    DK_EventDispatcher* _this);

void
DK_EventDispatcher_us_setTerminate(
    DK_EventDispatcher* _this);

LOC_boolean
DK_EventDispatcher_us_terminate(
    DK_EventDispatcher* _this);

void
DK_EventDispatcher_us_activate(
    DK_EventDispatcher* _this,
    DLRL_Exception* exception,
    DK_EventDispatcher_us_waitsetAttachAction waitsetAttachAction);

void
DK_EventDispatcher_us_deactivate(
    DK_EventDispatcher* _this,
    DLRL_Exception* exception,
    DK_EventDispatcher_us_waitsetDetachAction waitsetDetachAction);

void
DK_EventDispatcher_us_notify(
    DK_EventDispatcher* _this,
    DLRL_Exception* exception);

u_waitset
DK_EventDispatcher_us_getWaitSet(
    DK_EventDispatcher* _this);

void*
DK_EventDispatcher_us_getUserData(
    DK_EventDispatcher* _this);

DK_Entity*
DK_EventDispatcher_us_getInterestedEntity(
    DK_EventDispatcher* _this);

DK_EventDispatcher_us_eventAction
DK_EventDispatcher_us_getEventAction(
    DK_EventDispatcher* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_EVENT_DISPATCHER_H */
