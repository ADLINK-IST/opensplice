/*
*                         Vortex OpenSplice
*
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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


/**
 * @file
 */

#include <org/opensplice/core/ScopedLock.hpp>
#include <org/opensplice/core/ListenerDispatcher.hpp>
#include <org/opensplice/core/status/StatusUtils.hpp>


#if 0
#define TRACE_EVENT printf
#else
#define TRACE_EVENT(...)
#endif

#define LISTENER_INITIAL_EVENTS 16


org::opensplice::core::ListenerDispatcher::ListenerDispatcher(
    u_participant                                            participant,
    const org::opensplice::core::policy::ListenerScheduling& scheduling)
    : threadId(OS_THREAD_ID_NONE),
      threadState(STOPPED),
      scheduling(scheduling)
{
    u_cfElement cfg;
    os_result osResult;

    assert(participant);

    osResult = os_mutexInit(&this->mutex, NULL);
    ISOCPP_OS_RESULT_CHECK_AND_THROW(osResult, "Could not initialize mutex.");

    osResult = os_condInit (&this->cond, &this->mutex, NULL);
    ISOCPP_OS_RESULT_CHECK_AND_THROW(osResult, "Could not initialize cond.");

    cfg = u_participantGetConfiguration(participant);
    if(cfg){
        this->stackSize = this->getStackSize(cfg);
        u_cfElementFree(cfg);
    } else {
        this->stackSize = DEFAULT_STACKSIZE;
    }

    this->uParticipant = participant;
    this->uListener = u_listenerNew(u_entity(participant), OS_TRUE);
    if (!this->uListener) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not get user layer listener.");
    }

    this->eventListHead = NULL;
    this->eventListTail = NULL;
    this->freeList = NULL;

    for (unsigned i = 0; i < LISTENER_INITIAL_EVENTS; i++) {
        v_listenerEvent event = new C_STRUCT(v_listenerEvent);
        event->next = this->freeList;
        this->freeList = event;
    }
}

org::opensplice::core::ListenerDispatcher::~ListenerDispatcher()
{
    os_result osResult;
    os_duration timeout = 100*OS_DURATION_MILLISECOND;
    int i = 0;
    bool stop = false;
    v_listenerEvent event;

    /* This is called when the parent DomainParticipant is deleted.
     * The DomainParticipant will only be deleted when the ListenerDispatcher
     * thread is not in the running state.
     * - either no listener is attached and thus no thread is running
     * - or the DomainParticipant is deleted as last thing happening in
     *   the ListenDispatcher thread (see threadWrapper()).
     * Either way, the thread state will be STOPPED.
     */
    assert(this->threadState == STOPPED);

    /* Stop thread, which also waits until it is stopped. */
    if (this->threadState == RUNNING) {
        this->threadState = STOPPING;
    }
    while ((this->threadState != STOPPED) && (!stop)) {
        /* This loop will poll at 100ms and repeat notifying the uListener because
         * triggers may be missed by the listenerEventThread when the trigger is
         * issued just before the listenerEventThread enters the u_listenerWait.
         * Polling can be avoided if u_listenerNotify not only generates a trigger
         * but also internally keeps an event indicating the notification but the
         * current implementation is considered acceptable since this is only
         * during stopping the thread.
         */
        (void) u_listenerNotify(this->uListener);
        osResult = os_condTimedWait(&this->cond, &this->mutex, timeout);
        if (osResult == os_resultFail) {
            stop = true;
        } else if (osResult == os_resultTimeout) {
            i++;
            if (i > 50) { /* MAX blocking time = 50*100ms */
                stop = true;
            }
        }
    }
    if (this->uListener) {
        u_objectFree (u_object (this->uListener));
        this->uListener = NULL;
    }

    event = this->eventListHead;
    while (event) {
        this->eventListHead = event->next;
        c_free(c_object(event->eventData));
        delete event;
        event = this->eventListHead;
    }

    event = this->freeList;
    while (event) {
        this->freeList = event->next;
        delete event;
        event = this->freeList;
    }

    os_condDestroy (&this->cond);
    os_mutexDestroy (&this->mutex);
}

void
org::opensplice::core::ListenerDispatcher::add_listener(
    org::opensplice::core::EntityDelegate* observable,
    u_entity uEntity,
    const dds::core::status::StatusMask& mask)
{
    u_result uResult;
    v_eventMask vMask;

    assert(observable);
    assert(uEntity);

    os_mutexLock(&this->mutex);

    /* (re)Insert observable into the set. */
    this->observables.insert(observable);

    /* Set the listener on the user layer. */
    vMask = org::opensplice::core::utils::vEventMaskFromStatusMask(mask);
    TRACE_EVENT("0x%08lx::ListenerDispatcher::add(observable = 0x%08lx, uEntity = 0x%08lx, mask = 0x%08lx)\n",
                 this, observable, uEntity, vMask);
    uResult = u_entitySetListener(uEntity, this->uListener, NULL, vMask);
    try {
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not set listener on user layer.");

        /* This won't start the thread when it is already running. */
        this->threadStart();
        os_mutexUnlock(&this->mutex);
    } catch (...) {
        (void)this->observables.erase(observable);
        os_mutexUnlock(&this->mutex);
        throw;
    }
}

void
org::opensplice::core::ListenerDispatcher::remove_listener(
    org::opensplice::core::EntityDelegate* observable,
    u_entity uEntity)
{
    os_threadId stoppedThread = OS_THREAD_ID_NONE;
    u_result uResult;
    os_result oResult;
    assert(observable);
    assert(uEntity);

    os_mutexLock(&this->mutex);

    /* Remove observable from local management. */
    if (this->observables.erase(observable) != 1) {
        /* Trying to remove a listener that was not added... */
        os_mutexUnlock(&this->mutex);
        return;
    }

    TRACE_EVENT("0x%08lx::ListenerDispatcher::remove_listener(observable = 0x%08lx, uEntity = 0x%08lx)\n",
                 this, observable, uEntity);

    org::opensplice::core::ScopedMutexLock scopedLock(this->entities_mutex);
    this->invalid_user_entities.insert(uEntity);
    TRACE_EVENT("0x%08lx::ListenerDispatcher::remove_listener(): invalid_user_entities count %d\n", this, (int)this->invalid_user_entities.size());
    scopedLock.unlock();

    /* Remove observable from user layer. */
    uResult = u_entitySetListener(uEntity, NULL, NULL, 0);
    if ((uResult != U_RESULT_OK) && (uResult != U_RESULT_ALREADY_DELETED)){
        /* Ignore ALREADY_DELETED as this indicates the user layer is shut down. Due to this shut down
         * we nee to clean up, which must be done even when the user layer is gone. Throwing an exception
         * due to the fact the user layer is deleted will cause the caller to skip al clean-up.
         */
        os_mutexUnlock(&this->mutex);
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not (re)set listener on user layer.");
    }

    /* Stop the dispatcher when there are no more observables. */
    TRACE_EVENT("0x%08lx::ListenerDispatcher::remove_listener(): observable count %d\n", this, (int)this->observables.size());
    TRACE_EVENT("0x%08lx::ListenerDispatcher::remove_listener(): thread state %d\n", this, (int)this->threadState);
    if ((this->observables.size() == 0) && (this->threadState == RUNNING)) {
        this->threadState = STOPPING;
        TRACE_EVENT("0x%08lx::ListenerDispatcher::remove_listener(): wake up thread\n", this);
        (void)u_listenerNotify(this->uListener);
        stoppedThread = this->threadId;
    }

    /* Wait until dispatcher thread is stopped when needed. */
    os_mutexUnlock(&this->mutex);
    if ((os_threadIdToInteger(stoppedThread) != os_threadIdToInteger(OS_THREAD_ID_NONE)) &&
        (os_threadIdToInteger(stoppedThread) != os_threadIdToInteger(os_threadIdSelf())) ) {
        /* Don't wait, we're the thread itself or it isn't stopped. */
        oResult = os_threadWaitExit(stoppedThread, NULL);
        ISOCPP_OS_RESULT_CHECK_AND_THROW(oResult, "Waiting for thread exit failed");
    }
}

void
org::opensplice::core::ListenerDispatcher::add_source(
    org::opensplice::core::EntityDelegate* source,
    u_entity uEntity)
{
    /* We need to add the source to the userlayer to get back to the original entity. */
    assert(source);
    assert(uEntity);
    TRACE_EVENT("0x%08lx::ListenerDispatcher::add_source(observable = 0x%08lx, uEntity = 0x%08lx)\n",
                 this, source, uEntity);
    u_observableSetUserData(u_observable(uEntity), source);
}

void
org::opensplice::core::ListenerDispatcher::remove_source(
    org::opensplice::core::EntityDelegate* source,
    u_entity uEntity)
{
    assert(source);
    assert(uEntity);

    TRACE_EVENT("0x%08lx::ListenerDispatcher::remove_source(observable = 0x%08lx, uEntity = 0x%08lx)\n",
                 this, source, uEntity);

    org::opensplice::core::ScopedMutexLock scopedLock(this->entities_mutex);
    bool triggered = u_entityDisableCallbacks(uEntity);
    if (triggered) {
        this->invalid_user_entities.insert(uEntity);
        TRACE_EVENT("0x%08lx::ListenerDispatcher::remove_source(): invalid_user_entities count %d\n", this, (int)this->invalid_user_entities.size());
    }
    scopedLock.unlock();
    u_observableSetUserData(u_observable(uEntity), NULL);
}

uint32_t
org::opensplice::core::ListenerDispatcher::getStackSize(
    u_cfElement configuration)
{
    u_cfData cfg_data = NULL;
    c_iter nodes;
    u_cfNode tmp;
    c_ulong stackSize = DEFAULT_STACKSIZE;

    assert(configuration);

    /* Resolve StackSize parameter. */
    nodes = u_cfElementXPath(configuration, "Domain/Listeners/StackSize/#text");
    tmp = u_cfNode(c_iterTakeFirst(nodes));
    if (tmp != NULL) {
        if (u_cfNodeKind(tmp) == V_CFDATA) {
            cfg_data = u_cfData(tmp);
        } else {
            u_cfNodeFree(tmp);
        }
        tmp = u_cfNode(c_iterTakeFirst(nodes));
    }
    while (tmp != NULL) {
        u_cfNodeFree(tmp);
        tmp = u_cfNode(c_iterTakeFirst(nodes));
    }
    c_iterFree(nodes);

    /* Translate StackSize parameter. */
    if (cfg_data != NULL) {
        if (!u_cfDataULongValue(cfg_data, &stackSize)) {
            stackSize = DEFAULT_STACKSIZE;
        }
        u_cfDataFree(cfg_data);
    }

    return (uint32_t)stackSize;
}

void
org::opensplice::core::ListenerDispatcher::threadStart()
{
    os_result osResult;
    os_threadAttr osThreadAttr;

    switch (this->threadState) {
    case STOPPED:
        TRACE_EVENT("0x%08lx::ListenerDispatcher::threadStart(): STARTING\n", this);
          this->threadState = STARTING;
          os_condBroadcast(&this->cond);
          os_threadAttrInit(&osThreadAttr);
          this->scheduling->os_thread_attr(&osThreadAttr);
          if (this->stackSize != DEFAULT_STACKSIZE) {
              osThreadAttr.stackSize = this->stackSize;
          }

          /* Create thread */
          osResult = os_threadCreate(&this->threadId,
                                     "ListenerDispatcherThread",
                                     &osThreadAttr,
                                     &org::opensplice::core::ListenerDispatcher::threadWrapper,
                                     (void *)this);
          ISOCPP_OS_RESULT_CHECK_AND_THROW(osResult, "Failed to start thread.");
        break;
    case STOPPING:
        this->threadState = RUNNING;
        break;

    /* No action required, Wait for state to be RUNNING. */
    default:
        break;
    }

    while (this->threadState == STARTING) {
        /* Wait for event handler loop to switch state to !STARTING. */
        os_condWait (&this->cond, &this->mutex);
    }

    os_condBroadcast (&this->cond);
}


void *
org::opensplice::core::ListenerDispatcher::threadWrapper(
    void *arg)
{
    org::opensplice::core::ListenerDispatcher *dispatcher = static_cast<org::opensplice::core::ListenerDispatcher*>(arg);

    /* We have to hold a reference to the parent DomainParticipant.
     *
     * The ListenerDispatcher contains this thread, which holds strong
     * references (aka shared pointers) to Entities. Because of this,
     * entities can be destructed within this ListenerDispatcher thread
     * context.
     * This, in the end, can mean that the ListenerDispatcher itself is
     * destroyed within its own thread.
     *
     * That will happen when a event is dispatched on the DomainParticipant
     * (which is the parent of this ListenerDispatcher) and the application
     * decreases the refcount of the participant (for instance when the
     * participant goes of of scope).
     * Now, when the ListenerDispatcher thread decreases the refcount,
     * the participant will be deleted, causing the ListenerDispatcher to
     * be deleted while the thread (that uses the ListenerDispatcher object)
     * is still running.
     *
     * This causes all kinds of problems, as you can imagine.
     *
     * You can't wait for the thread to finish in the destructor, because
     * the destruction takes place in that threads' context.
     *
     * The following solution has been chosen.
     * When the thread is started, it always increases the refcount of the
     * parent participant.
     * When the thread is stopped, it decreases the refcount, which can
     * cause the participant (and thus this dispatcher) to be destroyed. By
     * letting the possible destruction take place in this static function,
     * we won't need access to the object anymore.
     *
     * However, when the application participant goes out of scope, this
     * thread will keep the participant object alive. This means that it
     * will never be destroyed.
     * When this thread is the only one with a reference to the participant
     * object, then it can safely stop the thread, which will destroy the
     * participant.
     */
    org::opensplice::core::ObjectDelegate::ref_type participant(
            org::opensplice::core::EntityDelegate::extract_strong_ref(u_entity(dispatcher->uParticipant)));

    dispatcher->thread(participant);

    return NULL;
}

void
org::opensplice::core::ListenerDispatcher::thread(const org::opensplice::core::ObjectDelegate::ref_type& participant)
{
    os_result osResult;
    u_result uResult = U_RESULT_OK;

    /* Needed to remove the 'variable set but not used' compile warning. */
    OS_UNUSED_ARG(osResult);

    TRACE_EVENT("0x%08lx::ListenerDispatcher::thread(): Start\n", this);

    /* Indicate that the thread is running. */
    os_mutexLock(&this->mutex);
    if (this->threadState == STARTING) {
        TRACE_EVENT("0x%08lx::ListenerDispatcher::thread(): STARTING\n", this);
        this->threadState = RUNNING;
        os_condBroadcast(&this->cond);
    }

    TRACE_EVENT("0x%08lx::ListenerDispatcher::thread(): RUNNING\n", this);

    /* The main listening thread loop. */
    while ((uResult == U_RESULT_OK) && (this->threadState == RUNNING) && (participant.use_count() != 1)) {
        os_mutexUnlock(&this->mutex);

        //TRACE_EVENT("0x%08lx::ListenerDispatcher::thread(): Enter Wait\n", this);

        uResult = u_listenerWait(this->uListener,
                                 org::opensplice::core::ListenerDispatcher::eventHandlerWrapper,
                                 this,
                                 10*OS_DURATION_MILLISECOND);

        if (uResult == U_RESULT_OK) {
            this->processEvents();
        }

        if (uResult != U_RESULT_TIMEOUT) {
            TRACE_EVENT("0x%08lx::ListenerDispatcher::thread(): Exit Wait, uResult = %s, state %d\n", this, u_resultImage(uResult), (int)this->threadState);
        }

        os_mutexLock(&this->mutex);

        if (uResult == U_RESULT_TIMEOUT) {
            uResult = U_RESULT_OK;
        }
    }

    /* Indicate that the thread has shut down. */
    this->threadState = STOPPED;
    TRACE_EVENT("0x%08lx::ListenerDispatcher::thread(): STOPPED\n", this);
    os_condBroadcast(&this->cond);
    os_mutexUnlock(&this->mutex);

    TRACE_EVENT("0x%08lx::ListenerDispatcher::thread(): Exit\n", this);
    this->threadId = OS_THREAD_ID_NONE;
}

static v_status
copyStatus(
    v_status s)
{
    v_status copy = NULL;

    if (s) {
        switch (v_objectKind(s)) {
        case K_KERNELSTATUS:
            copy = static_cast<v_status>(os_malloc(sizeof(C_STRUCT(v_kernelStatus))));
            memcpy(copy, s, sizeof(C_STRUCT(v_kernelStatus)));
            break;
        case K_TOPICSTATUS:
            copy = static_cast<v_status>(os_malloc(sizeof(C_STRUCT(v_topicStatus))));
            memcpy(copy, s, sizeof(C_STRUCT(v_topicStatus)));
            break;
        case K_DOMAINSTATUS:
            copy = static_cast<v_status>(os_malloc(sizeof(C_STRUCT(v_partitionStatus))));
            memcpy(copy, s, sizeof(C_STRUCT(v_partitionStatus)));
            break;
        case K_WRITERSTATUS:
            copy = static_cast<v_status>(os_malloc(sizeof(C_STRUCT(v_writerStatus))));
            memcpy(copy, s, sizeof(C_STRUCT(v_writerStatus)));
            break;
        case K_READERSTATUS:
            copy = static_cast<v_status>(os_malloc(sizeof(C_STRUCT(v_readerStatus))));
            memcpy(copy, s, sizeof(C_STRUCT(v_readerStatus)));
            break;
        case K_PARTICIPANTSTATUS:
            copy = static_cast<v_status>(os_malloc(sizeof(C_STRUCT(v_status))));
            memcpy(copy, s, sizeof(C_STRUCT(v_status)));
            break;
        case K_SUBSCRIBERSTATUS:
            copy = static_cast<v_status>(os_malloc(sizeof(C_STRUCT(v_status))));
            memcpy(copy, s, sizeof(C_STRUCT(v_status)));
            break;
        case K_PUBLISHERSTATUS:
            copy = static_cast<v_status>(os_malloc(sizeof(C_STRUCT(v_status))));
            /* These status are just instantations of v_status and have no
             * addition attributes!
             */
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


void
org::opensplice::core::ListenerDispatcher::addEvent(
    v_listenerEvent e)
{
    v_listenerEvent event = this->freeList;
    if (event) {
        this->freeList = event->next;
    } else {
        event = new C_STRUCT(v_listenerEvent);
    }

    /* Copy the listener event */
    event->kind = e->kind;
    event->source = e->source;
    event->userData = e->userData;
    if (event->kind == V_EVENT_DATA_AVAILABLE) {
        /* the eventData is a reference to the actual data causing this event. */
        event->eventData = e->eventData;
    } else {
        /* the event is caused by a communication status change, the eventData contains the status that is changed. */
        event->eventData = copyStatus(v_status(e->eventData));
    }
    event->next = NULL;

    if (this->eventListTail) {
        this->eventListTail->next = event;
    } else {
        this->eventListHead = event;
    }
    this->eventListTail = event;
}

void
org::opensplice::core::ListenerDispatcher::eventHandlerWrapper(
    v_listenerEvent event,
    c_voidp arg)
{
    org::opensplice::core::ListenerDispatcher *dispatcher = static_cast<org::opensplice::core::ListenerDispatcher*>(arg);
    dispatcher->addEvent(event);
}

void
org::opensplice::core::ListenerDispatcher::eventHandler (
    v_listenerEvent event)
{
    u_entity uSource;
    u_entity uListener;

    assert(event);

    if (event->kind & V_EVENT_TRIGGER) {
        /* Nothing to deliver so ignore. */
        TRACE_EVENT("static::ListenerDispatcher::eventHandler: received V_EVENT_TRIGGER\n");
        return;
    }

    /* Prevent entity deletion during callback by not allowing the removal of listeners. */
    org::opensplice::core::ScopedMutexLock scopedLock(this->entities_mutex);

    /* Extract source entity from event source. */
    uSource = u_entity(event->source);
    assert (uSource != NULL);
    TRACE_EVENT("static::ListenerDispatcher::eventHandler: u source 0x%08lx (%d)\n", (unsigned long)uSource, (int)u_objectKind(u_object(uSource)));

    /* Extract listening entity instance from user data. */
    uListener = u_entity(event->userData);
    assert (uListener != NULL);
    TRACE_EVENT("static::ListenerDispatcher::eventHandler: u listener 0x%08lx (%d)\n", (unsigned long)uListener, (int)u_objectKind(u_object(uListener)));

    /* Check if we've received events from entities that are being/have been removed. */
    std::multiset<u_entity>::iterator itSource = this->invalid_user_entities.find(uSource);
    if ((itSource != this->invalid_user_entities.end()) ||
        (this->invalid_user_entities.find(uListener) != this->invalid_user_entities.end()) ) {
        /* We've received events from entity/entities that are being/have been removed. */
        if (event->kind & (V_EVENT_OBJECT_DESTROYED | V_EVENT_PREPARE_DELETE)) {
            /* We've received the destroy event, so we can delete the related entity from the adminstration. */
            this->invalid_user_entities.erase(itSource);
        }
        TRACE_EVENT("static::ListenerDispatcher::remove_listener(): invalid_user_entities count %d\n", (int)this->invalid_user_entities.size());
        /* Ignore these events. */
        return;
    }

    org::opensplice::core::ObjectDelegate::ref_type sRef(
            org::opensplice::core::EntityDelegate::extract_strong_ref(uSource));
    org::opensplice::core::ObjectDelegate::ref_type lRef(
            org::opensplice::core::EntityDelegate::extract_strong_ref(uListener));

    /* The strong refs go out of scope, meaning that it is possible that the related entities
     * are deleted. This will result in listener removals.
     * Be sure to be unlocked before the strong refs go out of scope.
     */
    scopedLock.unlock();

    TRACE_EVENT("static::ListenerDispatcher::eventHandler: flags 0x%08lx\n", event->kind);

    if (event->kind & (V_EVENT_OBJECT_DESTROYED | V_EVENT_PREPARE_DELETE)) {
        /* We should only get these events from invalid entities, which means that they
         * should already have been handled.
         */
        assert(false);
    } else {
        /* Call entity to notify its listener of the current event
         * (when they're not in the process of being deleted.
         */
        if (lRef && sRef) {
            TRACE_EVENT("static::ListenerDispatcher::eventHandler: trigger 0x%08lx\n", (unsigned long)(lRef.get()));
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::core::EntityDelegate>(lRef)
                                        ->listener_entity_notify(sRef, event->kind, event->eventData);
        }
    }
}


void
org::opensplice::core::ListenerDispatcher::processEvents()
{
    v_listenerEvent event;

    event = this->eventListHead;
    while (event) {
        this->eventListHead = event->next;
        if (!this->eventListHead) {
            this->eventListTail = NULL;
        }
        this->eventHandler(event);
        if (event->kind != V_EVENT_DATA_AVAILABLE) {
            /* eventData is either a reference to a copy of the entities status in case of
            * communication events or a reference to the data in case of a data available event.
            * only copies on heap need to be freed, i.e. all other than data available events.
            */
            os_free(event->eventData);
        }
        event->next = this->freeList;
        this->freeList = event;
        event = this->eventListHead;
    }
}

