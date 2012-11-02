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
#ifndef V_WAITSET_H
#define V_WAITSET_H

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \class v_waitset
 *
 * The waitset extends the <code>v_observer</code> class and can be used to wait
 * on events. While waiting for events, every event is administrated
 */

/**
 * \brief The <code>v_waitset</code> cast method.
 *
 * This method casts a kernel object to a <code>v_waitset</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_waitset</code> or one of its subclasses.
 */
#define v_waitset(o)   (C_CAST(o,v_waitset))

#include "v_participant.h"
#include "v_event.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * The definition of the action callback method, which is called when
 * one or more events are received.
 *
 * \param e   a received event.
 * \param arg a pointer to user data needed to perform the action.
 */
typedef void (*v_waitsetAction)(v_waitsetEvent e, c_voidp arg);

/**
 * Creates a new waitset.
 *
 * \param kernel a reference to the kernel
 * \return       the reference to the newly created waitset.
 */
OS_API v_waitset
v_waitsetNew(
    v_participant p);

/**
 * Destroys the referenced waitset.
 *
 * \param ws a reference to the waitset object.
 */
OS_API void
v_waitsetFree(
    v_waitset ws);

OS_API c_bool
v_waitsetAttach(
    v_waitset _this,
    v_observable o,
    c_voidp userData);

OS_API c_bool
v_waitsetDetach(
    v_waitset _this,
    v_observable o);

/**
 * Wait until at least one event is received.
 * The caller is blocked until at least one event is received. At receiving
 * event(s) the given action routine is called for each received event. The
 * given 'arg' parameter is passed-through as-is.
 *
 * \param ws     a reference to the waitset object.
 * \param action reference to the action callback method.
 * \param arg    reference to data passed on to the action method.
 */
OS_API v_result
v_waitsetWait(
     v_waitset ws,
     v_waitsetAction action,
     c_voidp arg);

/**
 * Wait until at least one event is received or the specified timeout has
 * elapsed.
 * The caller is blocked until at least one event is received or the specified
 * timeout ('time') parameter has elapsed. At receiving event(s) the given
 * action routine is called for each received event. The given 'arg' parameter
 * is passed-through as-is. On timeout the action routine is not called.
 *
 * \param ws     a reference to the waitset object.
 * \param action reference to the action callback method.
 * \param arg    reference to data passed on to the action method.
 * \param time   the maximum time to wait until an event is received.
 */
OS_API v_result
v_waitsetTimedWait(
    v_waitset ws,
    v_waitsetAction action,
    c_voidp arg,
    const c_time time);

/**
 * Wake-up the thread(s) waiting on the waitset. The event returned to the waiting
 * thread is V_EVENT_TRIGGER.
 *
 * \param ws     a reference to the waitset object.
 * \param eventArg this argument is passed as user data of the event.
 */
OS_API void
v_waitsetTrigger(
    v_waitset ws,
    c_voidp eventArg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
