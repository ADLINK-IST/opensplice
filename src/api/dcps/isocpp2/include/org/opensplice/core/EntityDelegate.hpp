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

#ifndef ORG_OPENSPLICE_CORE_ENTITY_DELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_ENTITY_DELEGATE_HPP_

#include <dds/core/status/State.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <org/opensplice/core/UserObjectDelegate.hpp>
#include <org/opensplice/ForwardDeclarations.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

class OMG_DDS_API EntityDelegate :
    public virtual ::org::opensplice::core::UserObjectDelegate
{
public:
    typedef ::dds::core::smart_ptr_traits< EntityDelegate >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< EntityDelegate >::weak_ref_type weak_ref_type;

    EntityDelegate();
    virtual ~EntityDelegate();

    /**
     *  @internal Enables this entity.
     */
    void enable();

    ::dds::core::status::StatusMask status_changes() const;

    ::dds::core::InstanceHandle instance_handle() const;

    bool contains_entity(const ::dds::core::InstanceHandle& handle);

    ObjectDelegate::ref_type get_statusCondition();

    virtual void close();

    virtual void retain();

    /*
     * Listener related functionality.
     */
protected:
    /* This will be used to store the listener and listener mask. */
    void
    listener_set(void *listener, const dds::core::status::StatusMask& mask);
    void*
    listener_get() const;

    /* This will be used to set/reset the listener mask.
     * The listener will be stopped when the stored mask is
     * dds::core::status::StatusMask::none(). It will be
     * started (kept running) otherwise). */
    void
    listener_enable();

    /* This is called by the EntityDelegate itself after it received
     * an event from the listener dispatcher. The child classes should
     * implement the actual notification to the listeners. */
    virtual void
    listener_notify(ObjectDelegate::ref_type source,
                    uint32_t       triggerMask,
                    void           *eventData,
                    void           *listener) = 0;
public:
    /* This is called by the listener dispatcher when an event
     * was triggered. The child classes should implement the
     * actual notification. */
    void
    listener_entity_notify(ObjectDelegate::ref_type source,
                           uint32_t       triggerMask,
                           void           *eventData);

    /* Listener dispatchers can be created and shared by the child classes. */
    org::opensplice::core::ListenerDispatcher*
    listener_dispatcher_get();
    void
    listener_dispatcher_set(org::opensplice::core::ListenerDispatcher* ld);
    org::opensplice::core::ListenerDispatcher*
    listener_dispatcher_reset();

public:
    static ObjectDelegate::ref_type extract_strong_ref(u_entity e);

protected:
    static volatile unsigned int entityID_;
    bool enabled_;
    os_uint64 maxSupportedSeconds_;

private:
    org::opensplice::core::ListenerDispatcher *listener_dispatcher;
    dds::core::status::StatusMask listener_mask;
    void *listener;
    Mutex listener_mutex;
    ObjectDelegate::weak_ref_type myStatusCondition;
};

}
}
}

#endif /* ORG_OPENSPLICE_CORE_ENTITY_DELEGATE_HPP_ */
