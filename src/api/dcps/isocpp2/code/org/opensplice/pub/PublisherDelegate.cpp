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

#include <dds/pub/DataWriter.hpp>
#include <dds/pub/AnyDataWriter.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/PublisherListener.hpp>

#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/pub/PublisherDelegate.hpp>
#include <org/opensplice/pub/AnyDataWriterDelegate.hpp>
#include <org/opensplice/core/ScopedLock.hpp>

#include "u_types.h"
#include "u_publisher.h"
#include "u_publisherQos.h"

namespace org
{
namespace opensplice
{
namespace pub
{

PublisherDelegate::PublisherDelegate(const dds::domain::DomainParticipant& dp,
                                     const dds::pub::qos::PublisherQos& qos,
                                     dds::pub::PublisherListener* listener,
                                     const dds::core::status::StatusMask& event_mask)
    :   dp_(dp),
        qos_(qos),
        default_dwqos_()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);

    u_publisher uPub;
    u_participant uPar;
    u_publisherQos uQos;

    uPar = u_participant(this->dp_.delegate()->get_user_handle());
    if (!uPar) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not get publisher participant.");
    }

    qos.delegate().check();
    uQos = qos.delegate().u_qos();
    if (!uQos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not convert publisher QoS.");
    }

    std::string name = this->dp_.delegate()->create_child_name("publisher");
    uPub = u_publisherNew(uPar, name.c_str(), uQos, false);
    u_publisherQosFree (uQos);
    if (!uPub) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not create publisher.");
    }

    /* ObjectDelegate class will free the userlayer object in its destructor. */
    this->userHandle = u_object(uPub);
    this->listener_set((void*)listener, event_mask);
    set_domain_id(dp.delegate()->get_domain_id());
}

PublisherDelegate::~PublisherDelegate()
{
    if (!this->closed) {
        try {
            this->close();
        } catch (...) {
            /* Empty: the exception throw should have already traced an error. */
        }
    }
}

void
PublisherDelegate::init(ObjectDelegate::weak_ref_type weak_ref)
{
    /* Set weak_ref before passing ourselves to other isocpp objects. */
    this->set_weak_ref(weak_ref);
    /* Register Publisher at Participant. */
    this->dp_.delegate()->add_publisher(*this);
    /* Use listener dispatcher from the domain participant. */
    this->listener_dispatcher_set(this->dp_.delegate()->listener_dispatcher_get());
    /* This only starts listening when the status mask shows interest. */
    this->listener_enable();
    /* Enable when needed; in ISOCPP2 there is no factory for the DomainParticipant,
     * so the DomainParticipant is always enabled. */
    assert(this->dp_.delegate()->is_enabled());
    if (this->dp_.delegate()->is_auto_enable()) {
        this->enable();
    }
}

void
PublisherDelegate::close()
{
    /* Stop listener and remove dispatcher. */
    this->listener(NULL, dds::core::status::StatusMask::none());
    this->listener_dispatcher_reset();

    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    /* Close the datawriters. */
    this->writers.all_close();

    /* Unregister Publisher from Participant. */
    this->dp_.delegate()->remove_publisher(*this);

    org::opensplice::core::EntityDelegate::close();
}

const dds::pub::qos::PublisherQos&
PublisherDelegate::qos() const
{
    this->check();
    return this->qos_;
}

void
PublisherDelegate::qos(const dds::pub::qos::PublisherQos& pqos)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_publisherQos uQos;
    u_result uResult;

    pqos.delegate().check();
    uQos = pqos.delegate().u_qos();
    if (!uQos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not convert publisher qos.");
    }

    uResult = u_publisherSetQos(u_publisher(this->userHandle), uQos);
    u_publisherQosFree(uQos);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not set publisher qos.");

    this->qos_ = pqos;
}

dds::pub::qos::DataWriterQos
PublisherDelegate::default_datawriter_qos() const
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    dds::pub::qos::DataWriterQos qos = this->default_dwqos_;
    return qos;
}

void
PublisherDelegate::default_datawriter_qos(const dds::pub::qos::DataWriterQos& dwqos)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    dwqos.delegate().check();
    this->default_dwqos_ = dwqos;
}

void
PublisherDelegate::suspend_publications()
{
    this->check();
    u_result uResult = u_publisherSuspend(u_publisher(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not suspend publications.");
}

void
PublisherDelegate::resume_publications()
{
    this->check();
    u_result uResult = u_publisherResume(u_publisher(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not resume publications.");
}

void
PublisherDelegate::begin_coherent_changes()
{
    this->check();
    u_result uResult = u_publisherCoherentBegin(u_publisher(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not begin coherent changes.");
}

void
PublisherDelegate::end_coherent_changes()
{
    this->check();
    u_result uResult = u_publisherCoherentEnd(u_publisher(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not end coherent changes.");
}

void
PublisherDelegate::wait_for_acknowledgments(const dds::core::Duration& max_wait)
{
    ISOCPP_THROW_EXCEPTION(ISOCPP_UNSUPPORTED_ERROR, "Function not currently supported");
}

void
PublisherDelegate::listener(dds::pub::PublisherListener* listener,
                            const ::dds::core::status::StatusMask& mask)
{
    /* EntityDelegate takes care of thread safety. */
    this->listener_set((void*)listener, mask);
    this->listener_enable();
}

dds::pub::PublisherListener*
PublisherDelegate::listener() const
{
    this->check();
    return reinterpret_cast<dds::pub::PublisherListener*>(this->listener_get());
}

const dds::domain::DomainParticipant&
PublisherDelegate::participant() const
{
    this->check();
    return dp_;
}

bool
PublisherDelegate::contains_entity(
    const dds::core::InstanceHandle& handle)
{
    return this->writers.contains(handle);
}

void
PublisherDelegate::add_datawriter(
    org::opensplice::core::EntityDelegate& datawriter)
{
    this->writers.insert(datawriter);
}

void
PublisherDelegate::remove_datawriter(
    org::opensplice::core::EntityDelegate& datawriter)
{
    this->writers.erase(datawriter);
}

org::opensplice::pub::AnyDataWriterDelegate::ref_type
PublisherDelegate::find_datawriter(const std::string& topic_name)
{
    org::opensplice::pub::AnyDataWriterDelegate::ref_type writer;
    org::opensplice::core::EntitySet::vector vwriters;
    org::opensplice::core::EntitySet::vectorIterator iter;

    vwriters = this->writers.copy();
    for (iter = vwriters.begin(); (!writer) && (iter != vwriters.end()); ++iter) {
        org::opensplice::core::ObjectDelegate::ref_type ref = iter->lock();
        if (ref) {
            org::opensplice::pub::AnyDataWriterDelegate::ref_type tmp =
                    OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<AnyDataWriterDelegate>(ref);
            assert(tmp);
            if (tmp->topic_description().name() == topic_name) {
                writer = tmp;
            }
        }
    }

    return writer;
}

dds::pub::TPublisher<PublisherDelegate>
PublisherDelegate::wrapper()
{
    PublisherDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<PublisherDelegate>(this->get_strong_ref());
    dds::pub::Publisher pub(ref);
    return pub;
}

bool
PublisherDelegate::is_auto_enable() const
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    bool autoEnable = this->qos_.delegate().policy<dds::core::policy::EntityFactory>().delegate().auto_enable();

    return autoEnable;
}

void
PublisherDelegate::listener_notify(ObjectDelegate::ref_type source,
                                   uint32_t                 triggerMask,
                                   void                    *eventData,
                                   void                    *l)
{
    /* The EntityDelegate takes care of the thread safety and always
     * provides a listener and source. */
    dds::pub::PublisherListener* listener =
            reinterpret_cast<dds::pub::PublisherListener*>(l);
    assert(listener);

    /* Get AnyDataWriter from given source EntityDelegate. */
    AnyDataWriterDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<AnyDataWriterDelegate>(source);
    dds::pub::AnyDataWriter writer = ref->wrapper_to_any();

    if (triggerMask & V_EVENT_LIVELINESS_LOST) {
        dds::core::status::LivelinessLostStatus status;
        status.delegate().v_status(v_writerStatus(eventData)->livelinessLost);
        listener->on_liveliness_lost(writer, status);
    }

    if (triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
        dds::core::status::OfferedDeadlineMissedStatus status;
        status.delegate().v_status(v_writerStatus(eventData)->deadlineMissed);
        listener->on_offered_deadline_missed(writer, status);
    }

    if (triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
        dds::core::status::OfferedIncompatibleQosStatus status;
        status.delegate().v_status(v_writerStatus(eventData)->incompatibleQos);
        listener->on_offered_incompatible_qos(writer, status);
    }

    if (triggerMask & V_EVENT_PUBLICATION_MATCHED) {
        dds::core::status::PublicationMatchedStatus status;
        status.delegate().v_status(v_writerStatus(eventData)->publicationMatch);
        listener->on_publication_matched(writer, status);
    }
}

}
}
}
