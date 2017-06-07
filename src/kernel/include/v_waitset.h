/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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

#ifdef OSPL_BUILD_CORE
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
typedef os_boolean (*v_waitsetAction2)(c_voidp userData, c_voidp arg);

#define v_waitsetEvent(e) (C_CAST(e,v_waitsetEvent))

#define v_waitsetEventKind(e) (v_waitsetEvent(e)->kind)
#define v_waitsetEventSource(e) (v_waitsetEvent(e)->source)
#define v_waitsetEventData(e) (v_waitsetEvent(e)->userData)

#define v_waitsetEventConnectWriterGroup(e) \
        (v_group(v_waitsetEvent(e)->eventData))

#define v_waitsetEventHistoryRequestFilter(e) \
        (((v_historicalDataRequest)v_waitsetEvent(e)->eventData)->filter)
#define v_waitsetEventHistoryRequestResourceLimits(e) \
        (((v_historicalDataRequest)v_waitsetEvent(e)->eventData)->resourceLimits)
#define v_waitsetEventHistoryRequestMinTimestamp(e)\
        (((v_historicalDataRequest)v_waitsetEvent(e)->eventData)->minSourceTimestamp)
#define v_waitsetEventHistoryRequestMaxTimestamp(e)\
        (((v_historicalDataRequest)v_waitsetEvent(e)->eventData)->maxSourceTimestamp)
#define v_waitsetEventHistoryRequestFilterParams(e)\
        (((v_historicalDataRequest)v_waitsetEvent(e)->eventData)->filterParams)

#define v_waitsetEventHistoryDeleteTime(e) \
        (((v_historicalDeleteRequest)v_waitsetEvent(e)->eventData)->deleteTime)
#define v_waitsetEventHistoryDeletePartitionExpr(e) \
        (((v_historicalDeleteRequest)v_waitsetEvent(e)->eventData)->partitionExpr)
#define v_waitsetEventHistoryDeleteTopicExpr(e)\
        (((v_historicalDeleteRequest)v_waitsetEvent(e)->eventData)->topicExpr)

#define v_waitsetEventPersistentSnapshotPartitionExpr(e) \
        (((v_persistentSnapshotRequest)v_waitsetEvent(e)->eventData)->partitionExpr)
#define v_waitsetEventPersistentSnapshotTopicExpr(e) \
        (((v_persistentSnapshotRequest)v_waitsetEvent(e)->eventData)->topicExpr)
#define v_waitsetEventPersistentSnapshotURI(e) \
        (((v_persistentSnapshotRequest)v_waitsetEvent(e)->eventData)->uri)

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

/**
 * This operation adds an observable to the waitset.
 *
 * \param _this  The waitset.
 * \param o      The observable object.
 * \param userData  User speciofic data associated to this
 *                  waitset-observable relation.
 *                  If an event on this observable triggers
 *                  the waitset this user data will be returned
 *                  as part of the event.
 */
OS_API c_bool
v_waitsetAttach(
    v_waitset _this,
    v_observable o,
    c_voidp userData);

/**
 * This operation removes an observable from the waitset.
 *
 * \param _this  The waitset.
 * \param o      The observable object.
 * \return       -1 on failure and otherwise it returns the
 *               number of remaining observables.
 */
OS_API c_long
v_waitsetDetach(
    v_waitset _this,
    v_observable o);

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
v_waitsetWait(
    v_waitset ws,
    v_waitsetAction action,
    c_voidp arg,
    const os_duration time);

OS_API v_result
v_waitsetWait2(
    v_waitset _this,
    v_waitsetAction2 action,
    c_voidp arg,
    const os_duration time);

OS_API c_ulong
v_waitsetCount(
    v_waitset _this);

/**
 * Wake-up the thread(s) waiting on the waitset.
 * The event returned to the waiting thread is V_EVENT_TRIGGER.
 *
 * \param ws     a reference to the waitset object.
 * \param eventArg this argument is passed as user data of the event.
 */
OS_API void
v_waitsetTrigger(
    v_waitset ws,
    c_voidp eventArg);

OS_API void
v_waitsetLogEvents(
    v_waitset _this,
    c_bool enable);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
