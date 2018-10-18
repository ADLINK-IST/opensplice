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
#ifndef ORG_OPENSPLICE_CORE_LISTENERDISPATCHER_H_
#define ORG_OPENSPLICE_CORE_LISTENERDISPATCHER_H_

#include <dds/core/policy/CorePolicy.hpp>

#include <org/opensplice/core/EntityDelegate.hpp>

#include "u_entity.h"
#include "u_listener.h"
#include "u_participant.h"

#include <set>


namespace org
{
namespace opensplice
{
namespace core
{

class OMG_DDS_API ListenerDispatcher
{
public:
    typedef ::dds::core::smart_ptr_traits< ListenerDispatcher >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< ListenerDispatcher >::weak_ref_type weak_ref_type;

    ListenerDispatcher(u_participant participant,
                       const org::opensplice::core::policy::ListenerScheduling& scheduling);

    virtual ~ListenerDispatcher();

    void
    deinit();

    void
    add_listener(org::opensplice::core::EntityDelegate* observable,
                 u_entity uEntity,
                 const dds::core::status::StatusMask& mask);

    void
    remove_listener(org::opensplice::core::EntityDelegate* observable, u_entity uEntity);

    void
    add_source(org::opensplice::core::EntityDelegate* source, u_entity uEntity);

    void
    remove_source(org::opensplice::core::EntityDelegate* source, u_entity uEntity);

private:

    typedef enum {
       STOPPED,
       STARTING,
       RUNNING,
       STOPPING
    } threadState_t;

    uint32_t
    getStackSize(u_cfElement configuration);

    void
    threadStart();

    static void*
    threadWrapper(
        void *arg);

    void
    thread(const org::opensplice::core::ObjectDelegate::ref_type& participant);

    static void
    eventHandlerWrapper(
        v_listenerEvent event,
        c_voidp         arg);

    void
    eventHandler(
        v_listenerEvent event);

    void
    addEvent(
        v_listenerEvent event);

    void
    processEvents();

    /* Need to use os_mutex because we want to use a condition variable. */
    os_mutex                  mutex;
    os_cond                   cond;
    os_threadId               threadId;
    threadState_t             threadState;
    uint32_t                  stackSize;
    u_listener                uListener;
    u_participant             uParticipant;
    v_listenerEvent           eventListHead;
    v_listenerEvent           eventListTail;
    v_listenerEvent           freeList;
    org::opensplice::core::policy::ListenerScheduling scheduling;
    std::set<org::opensplice::core::EntityDelegate*> observables;
    std::multiset<u_entity> invalid_user_entities;
    org::opensplice::core::Mutex entities_mutex;

    static const uint32_t     DEFAULT_STACKSIZE = 0;
};

}
}
}


#endif /* ORG_OPENSPLICE_CORE_LISTENERDISPATCHER_H_ */
