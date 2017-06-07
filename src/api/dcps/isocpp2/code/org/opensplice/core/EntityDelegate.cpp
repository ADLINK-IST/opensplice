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


/**
 * @file
 */

#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/core/status/StatusUtils.hpp>
#include <org/opensplice/core/ListenerDispatcher.hpp>
#include <org/opensplice/core/ScopedLock.hpp>

#include <dds/core/cond/StatusCondition.hpp>

#include <v_status.h>
#include <u_observable.h>



org::opensplice::core::EntityDelegate::EntityDelegate() :
     enabled_(true),
     maxSupportedSeconds_(OS_TIME_MAX_VALID_SECONDS),
     listener_dispatcher(NULL),
     listener_mask(0),
     listener(NULL)
{
}

org::opensplice::core::EntityDelegate::~EntityDelegate()
{
    /* Listener dispatcher should have been reset. */
    assert(!(this->listener_dispatcher));
}

void
org::opensplice::core::EntityDelegate::enable()
{
    check();

    u_result uResult = u_entityEnable(u_entity(userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Enabling failed");

    enabled_ = true;
    if (!u_observableGetY2038Ready(u_observable(userHandle))) {
        maxSupportedSeconds_ = INT32_MAX;
    }
}

static void
getStatusMask(
    v_public p,
    c_voidp arg)
{
    ::dds::core::status::StatusMask *mask = reinterpret_cast< ::dds::core::status::StatusMask *>(arg);
    c_ulong vMask;
    v_kind vKind;

    vMask = v_statusGetMask(v_entity(p)->status);
    vKind = v_objectKind(v_object(p));

    *mask = org::opensplice::core::utils::vEventMaskToStatusMask(vMask, vKind);
}

::dds::core::status::StatusMask
org::opensplice::core::EntityDelegate::status_changes() const
{
    ::dds::core::status::StatusMask mask;

    check();

    u_result uResult = u_observableAction(u_observable(this->userHandle), getStatusMask, &mask);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not get status changes");

    return mask;
}

::dds::core::InstanceHandle
org::opensplice::core::EntityDelegate::instance_handle() const
{
    ::dds::core::InstanceHandle handle(::dds::core::null);

    check();

    u_instanceHandle h = u_entityGetInstanceHandle(u_entity(userHandle));
    if (h == U_INSTANCEHANDLE_NIL) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ALREADY_CLOSED_ERROR, "Getting instance handle failed");
    }
    handle = dds::core::InstanceHandle(h);

    return handle;
}

bool
org::opensplice::core::EntityDelegate::contains_entity(
    const ::dds::core::InstanceHandle& handle)
{
    return false;
}

org::opensplice::core::ObjectDelegate::ref_type
org::opensplice::core::EntityDelegate::get_statusCondition()
{

    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    org::opensplice::core::ObjectDelegate::ref_type mySCObj = this->myStatusCondition.lock();
    if (!mySCObj) {
        u_entity uEntity = u_entity(this->userHandle);
        dds::core::cond::TStatusCondition<org::opensplice::core::cond::StatusConditionDelegate> mySC(
                new org::opensplice::core::cond::StatusConditionDelegate(this, uEntity));
        mySC.delegate()->init(mySC.delegate());
        this->myStatusCondition = mySC.delegate()->get_weak_ref();
        mySCObj = mySC.delegate()->get_strong_ref();
    }

    return mySCObj;
}


void
org::opensplice::core::EntityDelegate::close()
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ObjectDelegate::ref_type mySCObj = this->myStatusCondition.lock();
    if (mySCObj) {
        mySCObj->close();
    }
    org::opensplice::core::UserObjectDelegate::close();
}

void
org::opensplice::core::EntityDelegate::retain()
{
	ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    ISOCPP_THROW_EXCEPTION(ISOCPP_UNSUPPORTED_ERROR, "The retain() is not supported");
}

void
org::opensplice::core::EntityDelegate::listener_set(void *listener, const dds::core::status::StatusMask& mask)
{
    org::opensplice::core::ScopedMutexLock scopedListenerLock(this->listener_mutex);

    this->check();
    this->listener_mask = mask;
    this->listener = listener;
}

void*
org::opensplice::core::EntityDelegate::listener_get() const
{
    return this->listener;
}

void
org::opensplice::core::EntityDelegate::listener_enable()
{
    org::opensplice::core::ScopedMutexLock scopedListenerLock(this->listener_mutex);

    this->check();

    if (this->listener_dispatcher) {
        if (this->listener_mask == dds::core::status::StatusMask::none()) {
            this->listener_dispatcher->remove_listener(this, u_entity(this->userHandle));
        } else {
            this->listener_dispatcher->add_listener(this, u_entity(this->userHandle), this->listener_mask);
        }
    }
}

void
org::opensplice::core::EntityDelegate::listener_entity_notify(
        ObjectDelegate::ref_type source,
        uint32_t       triggerMask,
        void           *eventData)
{
    org::opensplice::core::ScopedMutexLock scopedListenerLock(this->listener_mutex, false);

    assert(source);

    if (scopedListenerLock.try_lock()) {
        /*
         * Unfortunatelly, the application could have deleted this listener without
         * resetting (setting null) it from this entity. There's no way to check or prevent
         * that from within this implementation.
         *
         * When the application does it properly and resets the listener before deleting
         * the listener, then the listener lock will assure that the listener still exists
         * while calling it.
         */
        if (this->listener) {
            /* Let the child entities actually handle the notifications. */
            this->listener_notify(source, triggerMask, eventData, this->listener);
        }
    } else {
        /*
         * The listener lock is already locked, meaning that this event is triggered during
         * changing the listener settings (most probably caused by that changing of the
         * listener settings).
         *
         * Ignore this event because the listener state is undefined and we can't block forever
         * because a listener settings change could wait for this to finish. If we'd block, we
         * can cause a deadlock. Ignoring the event is the savest.
         */
    }
}

org::opensplice::core::ListenerDispatcher*
org::opensplice::core::EntityDelegate::listener_dispatcher_get()
{
    return this->listener_dispatcher;
}

void
org::opensplice::core::EntityDelegate::listener_dispatcher_set(
        org::opensplice::core::ListenerDispatcher *ld)
{
    /* Add only once. */
    assert(ld);
    assert(!(this->listener_dispatcher));
    this->listener_dispatcher = ld;

    /* We need to also add 'this' as a potentional source
     * for events within the dispatcher. */
    this->listener_dispatcher->add_source(this, u_entity(this->userHandle));
}

org::opensplice::core::ListenerDispatcher*
org::opensplice::core::EntityDelegate::listener_dispatcher_reset()
{
    org::opensplice::core::ScopedMutexLock scopedListenerLock(this->listener_mutex);

    this->check();

    org::opensplice::core::ListenerDispatcher *old = this->listener_dispatcher;
    if (old) {
        /* This will possibly block until possible listener is actually removed. */
        this->listener_dispatcher->remove_source(this, u_entity(this->userHandle));
        this->listener_dispatcher = NULL;
    }

    return old;
}

org::opensplice::core::ObjectDelegate::ref_type
org::opensplice::core::EntityDelegate::extract_strong_ref(u_entity e) {
    org::opensplice::core::ObjectDelegate::ref_type result;
    org::opensplice::core::EntityDelegate* e_ptr;
    void* v_ptr;

    u_kind kind = u_objectKind(u_object(e));
    if ((kind > U_UNDEFINED) && (kind < U_COUNT)) {
        v_ptr = u_observableGetUserData(u_observable(e));
        assert(v_ptr);
        e_ptr = reinterpret_cast<org::opensplice::core::EntityDelegate*>(v_ptr);
        assert (e_ptr != NULL);
        result = e_ptr->get_strong_ref();
    } else {
        /* Probably a deleted entity. */
    }

    return result;
}
