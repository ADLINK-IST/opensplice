/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN_RUNNABLE_H
#define IN_RUNNABLE_H

/* OS abstraction includes. */
#include "os_defs.h"
#include "os_classbase.h"
#include "os_stdlib.h"
#include "os_thread.h"
#include "os_mutex.h"
#include "in__object.h"

#include "in__object.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif


typedef enum in_runState_e
{
    IN_RUNSTATE_NONE,
    IN_RUNSTATE_RUNNING,
    IN_RUNSTATE_TERMINATED
} in_runState;

typedef void* (*in_runnableMainFunc)(in_runnable _this);
typedef void (*in_runnableTriggerFunc)(in_runnable _this);

OS_CLASS(in_schedulingAttr);
OS_STRUCT(in_schedulingAttr)
{
    os_boolean schedulingPrioUseDefault;
    os_int32 schedulingPrioValue; /* Only valid if UseDefault == OS_FALSE */
    os_schedClass schedulingClass;
};

OS_STRUCT(in_runnable)
{
    OS_EXTENDS(in_object);
    os_char* name;
    os_threadId threadId;
    os_boolean terminate;
    os_mutex mutex;
    in_runState runState;
    in_runnableMainFunc mainFunc;
    in_runnableTriggerFunc triggerFunc;
    OS_STRUCT(in_schedulingAttr) schedulingAttr;
};

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_runnable(_this) ((in_runnable)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_runnableIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_runnableKeep(_this) in_runnable(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_runnableFree(_this) in_objectFree(in_object(_this))

void
in_runnableStart(
    in_runnable _this);

void
in_runnableStop(
    in_runnable _this);

os_boolean
in_runnableInit(
    in_runnable _this,
    const in_objectKind kind,
    const in_objectDeinitFunc deinit,
    const os_char* name,
    const os_char* pathName,
    const in_runnableMainFunc runnableMainFunc,
    const in_runnableTriggerFunc triggerFunc);

void
in_runnableDeinit(
    in_object _this);

const os_char*
in_runnableGetName(
    in_runnable _this);

os_boolean
in_runnableTerminationRequested(
    in_runnable _this);

void
in_runnableSetRunState(
    in_runnable _this,
    in_runState runState);

void
in_runnableTrigger(
    in_runnable _this);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_RUNNABLE_H */

