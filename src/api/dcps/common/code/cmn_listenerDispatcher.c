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
#include "cmn_listenerDispatcher.h"
#include "os_report.h"
#include "u_listener.h"
#include "v_status.h"

#define CMN_LISTENER_INITIAL_EVENTS 16

typedef enum {
    STOPPED,
    STARTING,
    RUNNING,
    STOPPING
} cmn_listenerDispatcher_state;

OS_CLASS(cmn_listenerObservable);
OS_STRUCT(cmn_listenerObservable) {
    u_entity entity;
    cmn_listenerDispatcher_callback callback;
    void *callback_data;
    v_eventMask mask;
};

OS_STRUCT(cmn_listenerDispatcher) {
    os_mutex mutex;
    os_cond cond;
    os_threadId threadId;
    os_threadAttr threadAttr;
    os_boolean active;
    cmn_listenerDispatcher_state threadState;
    u_listener uListener;
    os_iter observables;
    v_listenerEvent eventListHead;
    v_listenerEvent eventListTail;
    v_listenerEvent freeList;
    cmn_listenerDispatcher_callback callback;
    void *callback_data;
};

static const os_uint32
cmn_listenerDispatcher_default_stack_size = 0;
//#define TRACE
#ifdef TRACE
#define TRACE_EVENT printf

static const os_char *
cmn_listenerDispatcher_stateImage (
    cmn_listenerDispatcher_state state)
{
    const os_char *image;

    switch (state) {
        case STOPPING:
            image = "STOPPING";
            break;
        case STARTING:
            image = "STARTING";
            break;
        case RUNNING:
            image = "RUNNING";
            break;
        default:
            assert (state == STOPPED);
            image = "STOPPED";
            break;
    }

    return image;
}
#else
#define TRACE_EVENT(...)
#endif

static os_int32
compare_observable(
    void *o,
    os_iterActionArg arg)
{
    cmn_listenerObservable observable = (cmn_listenerObservable)o;
    u_entity entity = u_entity(arg);

    return (observable->entity == entity);
}

static v_status
copyStatus(
    v_status s)
{
    v_status copy = NULL;

    if (s) {
        switch (v_objectKind(s)) {
        case K_KERNELSTATUS:
            copy = os_malloc(sizeof(C_STRUCT(v_kernelStatus)));
            memcpy(copy, s, sizeof(C_STRUCT(v_kernelStatus)));
            break;
        case K_TOPICSTATUS:
            copy = os_malloc(sizeof(C_STRUCT(v_topicStatus)));
            memcpy(copy, s, sizeof(C_STRUCT(v_topicStatus)));
            break;
        case K_DOMAINSTATUS:
            copy = os_malloc(sizeof(C_STRUCT(v_partitionStatus)));
            memcpy(copy, s, sizeof(C_STRUCT(v_partitionStatus)));
            break;
        case K_WRITERSTATUS:
            copy = os_malloc(sizeof(C_STRUCT(v_writerStatus)));
            memcpy(copy, s, sizeof(C_STRUCT(v_writerStatus)));
            break;
        case K_READERSTATUS:
            copy = os_malloc(sizeof(C_STRUCT(v_readerStatus)));
            memcpy(copy, s, sizeof(C_STRUCT(v_readerStatus)));
            break;
        case K_PARTICIPANTSTATUS:
            copy = os_malloc(sizeof(C_STRUCT(v_status)));
            memcpy(copy, s, sizeof(C_STRUCT(v_status)));
            break;
        case K_SUBSCRIBERSTATUS:
            copy = os_malloc(sizeof(C_STRUCT(v_status)));
            memcpy(copy, s, sizeof(C_STRUCT(v_status)));
            break;
        case K_PUBLISHERSTATUS:
            copy = os_malloc(sizeof(C_STRUCT(v_status)));
            /* These status are just instantations of v_status and have no
             * addition attributes! */
            memcpy(copy, s, sizeof(C_STRUCT(v_status)));
            break;
        default:
            OS_REPORT(OS_CRITICAL,
                    "v_statusCopyOut", V_RESULT_ILL_PARAM,
                    "Unknown object kind %d",
                    v_objectKind(s));
            break;
        }
    }
    return copy;
}

static void
cmn_listenerAction(
    v_listenerEvent e,
    void *arg)
{
    cmn_listenerDispatcher _this = arg;
    v_listenerEvent event;

    assert(_this);

    event = _this->freeList;
    if (event) {
        _this->freeList = event->next;
    } else {
        event = os_malloc(sizeof(*event));
    }

    /* Copy the listener event */
    event->kind = e->kind;
    event->source = e->source;
    event->userData = e->userData;
    event->listenerData = e->listenerData;
    if (event->kind == V_EVENT_DATA_AVAILABLE) {
        /* the eventData is a reference to the actual data causing this event. */
        event->eventData = e->eventData;
    } else {
        /* the event is caused by a communication status change, the eventData contains the status that is changed. */
        event->eventData = copyStatus(v_status(e->eventData));
    }
    event->next = NULL;

    if (_this->eventListTail) {
        _this->eventListTail->next = event;
    } else {
        _this->eventListHead = event;
    }
    _this->eventListTail = event;
}

static void
cmn_listenerProcessEvents(
    cmn_listenerDispatcher _this)
{
    cmn_listenerObservable observable;
    v_listenerEvent event;

    event = _this->eventListHead;
    while (event) {
        _this->eventListHead = event->next;
        if (!_this->eventListHead) {
            _this->eventListTail = NULL;
        }
        observable = event->listenerData;
        if (observable && observable->callback) {
            /* Specific source callback and callback_data */
            TRACE_EVENT ("cmn_listenerDispatcher::event_handler: observable->callback\n");
            observable->callback(event, observable->callback_data);
        }
        if (_this && _this->callback) {
            /* Destination callback and callback_data. */
            TRACE_EVENT ("cmn_listenerDispatcher::event_handler: dispatcher->callback\n");
            _this->callback(event, _this->callback_data);
        }
        if (event->kind != V_EVENT_DATA_AVAILABLE) {
            /* eventData is either a reference to a copy of the entities status in case of
             * comunnication events or a reference to the data in case of a data available event.
             * only copies on heap need to be freed, i.e. all other than data available events.
             */
            os_free(event->eventData);
        }
        event->next = _this->freeList;
        _this->freeList = event;
        event = _this->eventListHead;
    }
}

static void *
cmn_listenerDispatcher_main (
    void *data)
{
    os_int32 result = OS_RETCODE_OK;
    cmn_listenerDispatcher _this;
    u_result uResult;

    assert (data != NULL);

    _this = cmn_listenerDispatcher (data);

    TRACE_EVENT ("cmn_listenerDispatcher_main: %s(0x%x): Entered loop\n", OS_FUNCTION, _this);

    os_mutexLock (&_this->mutex);

    if (_this->threadState == STARTING) {
        _this->threadState = RUNNING;
        os_condBroadcast (&_this->cond);

        TRACE_EVENT("%s(0x%" PA_PRIxADDR "): Switched state to %s\n",
                    OS_FUNCTION, _this,
                    cmn_listenerDispatcher_stateImage (_this->threadState));

        while (result == OS_RETCODE_OK && _this->threadState == RUNNING) {
            os_mutexUnlock (&_this->mutex);

            TRACE_EVENT ("%s(0x%" PA_PRIxADDR "): Enter wait\n",
                OS_FUNCTION, _this);
            uResult = u_listenerWait (_this->uListener, cmn_listenerAction, _this, OS_DURATION_INFINITE);
            result = v_resultToReturnCode (uResult);
            TRACE_EVENT ("%s(0x%" PA_PRIxADDR "): Exited wait, result = %s\n",
                OS_FUNCTION, _this, os_returnCodeImage (result));

            if (uResult == U_RESULT_OK) {
                cmn_listenerProcessEvents(_this);
            }

            if (result == OS_RETCODE_TIMEOUT) {
                result = OS_RETCODE_OK;
            }

            os_mutexLock (&_this->mutex);
        }
    }

    _this->threadState = STOPPED;
    os_condBroadcast (&_this->cond);
    os_mutexUnlock (&_this->mutex);

    TRACE_EVENT ("%s(0x%" PA_PRIxADDR "): Exited loop\n", OS_FUNCTION, _this);

    return NULL;
}

static os_int32
cmn_listenerDispatcher_start (
    cmn_listenerDispatcher _this)
{
    os_int32 result = OS_RETCODE_OK;
    os_result osResult;
#ifdef TRACE
    cmn_listenerDispatcher_state state;
#endif

    assert (_this != NULL);

#ifdef TRACE
    state = _this->threadState;
#endif
    switch (_this->threadState) {
        /* Create thread and switch state to STARTING. */
        case STOPPED:
            osResult = os_threadCreate (
                &_this->threadId,
                "ListenerEventThread",
                &_this->threadAttr,
                &cmn_listenerDispatcher_main,
                (void *)_this);
            result = os_resultToReturnCode (osResult);
            if (result == OS_RETCODE_OK) {
                _this->threadState = STARTING;
            }
            break;
        /* Switch state to RUNNING. Thread main loop should still be active. */
        case STOPPING:
            _this->threadState = RUNNING;
            break;
        /* No action required, Wait for state to be RUNNING. */
        default:
            break;
    }

    TRACE_EVENT ("%s(0x%" PA_PRIxADDR "): state was %s, switched to %s\n",
                 OS_FUNCTION, _this,
                 cmn_listenerDispatcher_stateImage (state),
                 cmn_listenerDispatcher_stateImage (_this->threadState));

    if (result == OS_RETCODE_OK) {
        while (_this->threadState == STARTING) {
            /* Wait for event handler loop to switch state to !STARTING. */
            os_condWait (&_this->cond, &_this->mutex);
        }
    }

    if (result != OS_RETCODE_OK) {
        OS_REPORT (OS_ERROR, OS_FUNCTION, result, "Could not start listener");
    }

    (void)os_condBroadcast (&_this->cond);

    return result;
}

static os_int32
cmn_listenerDispatcher_stop (
    cmn_listenerDispatcher _this)
{
    os_int32 result = OS_RETCODE_OK;
    u_result uResult;
#ifdef TRACE
    cmn_listenerDispatcher_state state;
#endif

    assert (_this != NULL);

#ifdef TRACE
    state = _this->threadState;
#endif
    switch (_this->threadState) {
        /* Instruct thread to terminate and wait for state to be STOPPED. */
        case RUNNING:
            uResult = u_listenerTrigger (_this->uListener);
            result = v_resultToReturnCode (uResult);
            if (result == OS_RETCODE_OK) {
                _this->threadState = STOPPING;
            }
            break;
        case STARTING:
            _this->threadState = STOPPING;
            break;
        /* No action required. Wait for state to be STOPPED. */
        default:
            break;
    }

    TRACE_EVENT ("%s(0x%" PA_PRIxADDR "): state was %s, switched to %s\n",
                 OS_FUNCTION, _this,
                 cmn_listenerDispatcher_stateImage (state),
                 cmn_listenerDispatcher_stateImage (_this->threadState));

    if (result == OS_RETCODE_OK) {
        while (_this->threadState == STOPPING) {
            os_condWait (&_this->cond, &_this->mutex);
        }
    }

    if (_this->threadState == STOPPED) {
        os_ulong_int threadId, threadIdNone;

        threadId = os_threadIdToInteger (_this->threadId);
        threadIdNone = os_threadIdToInteger (OS_THREAD_ID_NONE);

        if (threadId != threadIdNone) {
            (void)os_threadWaitExit (_this->threadId, NULL);
            _this->threadId = OS_THREAD_ID_NONE;
        }
    }

    if (result != OS_RETCODE_OK) {
        OS_REPORT (OS_ERROR, OS_FUNCTION, result, "Could not stop listener");
    }

    (void)os_condBroadcast (&_this->cond);

    return result;
}

os_uint32
cmn_listenerDispatcher_stack_size (
    u_participant uParticipant)
{
    os_uint32 size = cmn_listenerDispatcher_default_stack_size;
    c_iter nodes = NULL;
    u_cfElement element = NULL;
    u_cfNode node = NULL;

    assert (uParticipant != NULL);

    element = u_participantGetConfiguration (uParticipant);
    if (element != NULL) {
        nodes = u_cfElementXPath (element, "Domain/Listeners/StackSize/#text");
        if (nodes != NULL) {
            node = u_cfNode (c_iterTakeFirst (nodes));
            if (node != NULL) {
                if (u_cfNodeKind (node) != V_CFDATA || !u_cfDataULongValue (u_cfData(node), &size)) {
                    OS_REPORT (
                        OS_WARNING, OS_FUNCTION, OS_RETCODE_BAD_PARAMETER,
                        "Domain/Listeners/StackSize element is invalid.");
                }
                u_cfNodeFree (node);
            }

            for (node = u_cfNode (c_iterTakeFirst (nodes));
                 node != NULL;
                 node = u_cfNode (c_iterTakeFirst (nodes)))
            {
                u_cfNodeFree (node);
            }
            c_iterFree (nodes);
        }
        u_cfElementFree (element);
    }

    return size;
}

cmn_listenerDispatcher
cmn_listenerDispatcher_new (
    u_participant uParticipant,
    os_schedClass scheduling_class,
    os_int32 scheduling_priority,
    cmn_listenerDispatcher_callback callback,
    void *callback_data,
    os_boolean combine)
{
    cmn_listenerDispatcher _this = NULL;
    v_listenerEvent event;
    int i;
    os_uint32 requested_stack;

    assert (uParticipant != NULL);

    _this = os_malloc(sizeof *_this);
    _this->threadId = OS_THREAD_ID_NONE;
    _this->threadState = STOPPED;
    _this->active = OS_FALSE;
    _this->callback = callback;
    _this->callback_data = callback_data;

    os_threadAttrInit (&_this->threadAttr);
    _this->threadAttr.schedClass = scheduling_class;
    _this->threadAttr.schedPriority = scheduling_priority;
    requested_stack = cmn_listenerDispatcher_stack_size (uParticipant);
    if ( requested_stack != 0 ) {
      /* Override platform specific default stack size */
       _this->threadAttr.stackSize = requested_stack;
    }

    _this->observables = os_iterNew(NULL);
    _this->eventListHead = NULL;
    _this->eventListTail = NULL;
    _this->freeList = NULL;

    for (i = 0; i < CMN_LISTENER_INITIAL_EVENTS; i++) {
        event = os_malloc(sizeof(*event));
        event->next = _this->freeList;
        _this->freeList = event;
    }

    if(os_mutexInit (&_this->mutex, NULL) != os_resultSuccess) {
        goto err_mutexInit;
    }

    if(os_condInit (&_this->cond, &_this->mutex, NULL) != os_resultSuccess) {
        goto err_condInit;
    }

    _this->uListener = u_listenerNew (u_entity (uParticipant), combine);
    if (_this->uListener == NULL) {
        goto err_listenerNew;
    }
    return _this;

/* Error handling */
err_listenerNew:
    os_condDestroy (&_this->cond);
err_condInit:
    os_mutexDestroy (&_this->mutex);
err_mutexInit:
    os_iterFree (_this->observables);
    event = _this->freeList;
    while (event) {
        _this->freeList = event->next;
        os_free(event);
        event = _this->freeList;
    }
    os_free (_this);
    return NULL;
}

os_int32
cmn_listenerDispatcher_free (
    cmn_listenerDispatcher _this)
{
    os_int32 result = OS_RETCODE_OK;
    v_listenerEvent event;
    void *observable;

    if (_this != NULL) {
        os_mutexLock(&_this->mutex);

        result = cmn_listenerDispatcher_stop (_this);
        if (result == OS_RETCODE_OK && _this->threadState != STOPPED) {
            result = OS_RETCODE_PRECONDITION_NOT_MET;
        }
        if (result == OS_RETCODE_OK) {
            if (_this->uListener != NULL) {
                (void)u_objectFree (u_object (_this->uListener));
                _this->uListener = NULL;
            }

            event = _this->eventListHead;
            while (event) {
                _this->eventListHead = event->next;
                if (event->kind != V_EVENT_DATA_AVAILABLE) {
                    /* eventData is either a reference to a copy of the entities status in case of
                     * comunnication events or a reference to the data in case of a data available event.
                     * only copies on heap need to be freed, i.e. all other than data available events.
                     */
                    os_free(event->eventData);
                }
                os_free(event);
                event = _this->eventListHead;
            }

            event = _this->freeList;
            while (event) {
                _this->freeList = event->next;
                os_free(event);
                event = _this->freeList;
            }

            while ((observable = os_iterTakeFirst(_this->observables)) != NULL) {
                os_free(observable);
            }
            os_iterFree(_this->observables);
            os_mutexUnlock (&_this->mutex);
            os_condDestroy (&_this->cond);
            os_mutexDestroy (&_this->mutex);
            os_free(_this);
        } else {
            os_mutexUnlock (&_this->mutex);
        }
    }

    return result;
}

os_int32
cmn_listenerDispatcher_add (
    cmn_listenerDispatcher _this,
    u_entity observable,
    cmn_listenerDispatcher_callback callback,
    void *callback_data,
    v_eventMask mask)
{
    cmn_listenerObservable found;
    os_int32 result = OS_RETCODE_OK;

    assert (_this != NULL);
    assert (observable != NULL);

    TRACE_EVENT ("%s(0x%" PA_PRIxADDR ", observable = 0x%"  PA_PRIxADDR ", mask = 0x%x)\n",
                 OS_FUNCTION, _this, observable, mask);

    os_mutexLock (&_this->mutex);

    found = os_iterReadAction(_this->observables, compare_observable, observable);
    if (found == NULL) {
        found = os_malloc(sizeof(*found));
        found->entity = observable;
        _this->observables = os_iterAppend (_this->observables, found);
    }
    found->callback = callback;
    found->callback_data = callback_data;
    found->mask = mask;
    (void)u_entitySetListener (observable, _this->uListener, found, mask);
    if (os_iterLength (_this->observables) == 1) {
        (void)cmn_listenerDispatcher_start (_this);
        /* result = cmn_listenerDispatcher_start (_this);
           TODO: OSPL-6104. Restore original listener if thread fails to
           start. Note that starting the dispatcher thread before all
           properties are updated introduces atomicity issues. */
    }
    os_mutexUnlock (&_this->mutex);

    return result;
}

os_int32
cmn_listenerDispatcher_remove (
    cmn_listenerDispatcher _this,
    u_entity observable)
{
    cmn_listenerObservable found, removed;
    os_int32 result = OS_RETCODE_OK;
    u_result uResult;

    assert (_this != NULL);
    assert (observable != NULL);

    TRACE_EVENT("%s(0x%" PA_PRIxADDR ", observable = 0x%" PA_PRIxADDR ")\n",
                OS_FUNCTION, _this, observable);

    os_mutexLock (&_this->mutex);

    found = os_iterReadAction(_this->observables, compare_observable, observable);
    if (found) {
        removed = os_iterTake (_this->observables, found);
        assert(removed == found);
        if (os_iterLength (_this->observables) == 0) {
            uResult = u_entitySetListener (observable, NULL, NULL, 0);
            result = v_resultToReturnCode (uResult);
            if (result == OS_RETCODE_OK) {
                result = cmn_listenerDispatcher_stop (_this);
            }
            if (result != OS_RETCODE_OK) {
                _this->observables = os_iterAppend (_this->observables, removed);
            }
        }
        if (result == OS_RETCODE_OK) {
            os_free(found);
        }
    }
    os_mutexUnlock (&_this->mutex);

    return result;
}

void
cmn_listenerDispatcher_get_scheduling (
    cmn_listenerDispatcher _this,
    os_schedClass *scheduling_class,
    os_int32 *scheduling_priority)
{
    assert (_this != NULL);

    os_mutexLock (&_this->mutex);

    if (scheduling_class != NULL) {
        *scheduling_class = _this->threadAttr.schedClass;
    }
    if (scheduling_priority != NULL) {
        *scheduling_priority = _this->threadAttr.schedPriority;
    }

    os_mutexUnlock (&_this->mutex);
}

os_int32
cmn_listenerDispatcher_set_scheduling (
    cmn_listenerDispatcher _this,
    os_schedClass scheduling_class,
    os_int32 scheduling_priority)
{
    os_int32 result = OS_RETCODE_OK;
    os_threadAttr rollback;

    assert (_this != NULL);

    os_mutexLock (&_this->mutex);

    /* only restart if scheduling policy changed */
    if (_this->threadAttr.schedClass != scheduling_class &&
        _this->threadAttr.schedPriority != scheduling_priority)
    {
        result = cmn_listenerDispatcher_stop (_this);
        if (result == OS_RETCODE_OK && _this->threadState != STOPPED) {
            result = OS_RETCODE_PRECONDITION_NOT_MET;
        }
        if (result == OS_RETCODE_OK) {
            rollback = _this->threadAttr;
            _this->threadAttr.schedClass = scheduling_class;
            _this->threadAttr.schedPriority = scheduling_priority;
            if (os_iterLength (_this->observables) != 0) {
                result = cmn_listenerDispatcher_start (_this);
                if (result != OS_RETCODE_OK) {
                    _this->threadAttr = rollback;
                }
            }
        }
    }

    os_mutexUnlock (&_this->mutex);

    return result;
}
