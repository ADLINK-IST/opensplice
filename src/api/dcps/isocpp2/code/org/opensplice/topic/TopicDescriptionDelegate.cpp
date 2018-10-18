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

#include <org/opensplice/topic/TopicDescriptionDelegate.hpp>
#include <org/opensplice/core/ScopedLock.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{

static org::opensplice::core::Mutex   g_implicit_mutex;
static dds::domain::DomainParticipant g_implicit_participant(dds::core::null);
static int g_implicit_ref = 0;

TopicDescriptionDelegate::TopicDescriptionDelegate(
        const dds::domain::DomainParticipant& dp,
        const std::string& name,
        const std::string& type_name)
    : myParticipant(dp),
      myTopicName(name),
      myTypeName(type_name),
      nrDependents(0)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();
    /* Create implicit singleton participant when needed. */
    if (myParticipant.is_nil()) {
        org::opensplice::core::ScopedMutexLock scopedLock(g_implicit_mutex);
        if (g_implicit_participant.is_nil()) {
            g_implicit_participant = dds::domain::DomainParticipant(org::opensplice::domain::default_id());
        }
        g_implicit_ref++;
        myParticipant = g_implicit_participant;
    }
    this->set_domain_id(myParticipant->get_domain_id());
}

TopicDescriptionDelegate::~TopicDescriptionDelegate()
{
    if (!closed) {
        try {
            close();
        } catch (...) {

        }
    }
    if (myParticipant == g_implicit_participant) {
        /* When the last topic with a reference to the implicit participant
         * is deleted, we have to delete the participant as well. For various
         * reasons. But the main is that destroying a global static entity, due
         * to a process exit, causes crashes. This is because all dds stuff is
         * already cleaned up before that kind of global object destruction
         * takes place. */
        org::opensplice::core::ScopedMutexLock scopedLock(g_implicit_mutex);
        g_implicit_ref--;
        if (g_implicit_ref == 0) {
            g_implicit_participant = dds::core::null;
        }
    }
}

const std::string&
TopicDescriptionDelegate::name() const
{
    return myTopicName;
}

const std::string&
TopicDescriptionDelegate::type_name() const
{
    return myTypeName;
}

const dds::domain::DomainParticipant&
TopicDescriptionDelegate::domain_participant() const
{
    return myParticipant;
}

void
TopicDescriptionDelegate::incrNrDependents()
{
    nrDependents++;
}

void
TopicDescriptionDelegate::decrNrDependents()
{
    nrDependents--;
}

bool
TopicDescriptionDelegate::hasDependents() const
{
    return (nrDependents > 0);
}


}
}
}
