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


#include <dds/pub/DataWriter.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/sub/AnyDataReader.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/domain/DomainParticipantListener.hpp>

#include <org/opensplice/domain/DomainParticipantDelegate.hpp>
#include <org/opensplice/domain/DomainParticipantRegistry.hpp>
#include <org/opensplice/domain/DomainParticipantListener.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/core/ScopedLock.hpp>
#include <org/opensplice/core/ListenerDispatcher.hpp>
#include <org/opensplice/pub/AnyDataWriterDelegate.hpp>
#include <org/opensplice/sub/AnyDataReaderDelegate.hpp>
#include <org/opensplice/topic/AnyTopicDelegate.hpp>
#include <org/opensplice/sub/BuiltinSubscriberDelegate.hpp>
#include <org/opensplice/core/TimeUtils.hpp>


#include "os_stdlib.h"
#include "os_time.h"

#include "u_user.h"
#include "u_participantQos.h"
#include "u_participant.h"
#include "u_domain.h"

org::opensplice::core::Mutex org::opensplice::domain::DomainParticipantDelegate::default_participant_qos_lock_;
dds::domain::qos::DomainParticipantQos org::opensplice::domain::DomainParticipantDelegate::default_participant_qos_;


org::opensplice::domain::DomainParticipantDelegate::DomainParticipantDelegate (
        uint32_t id,
        const dds::domain::qos::DomainParticipantQos& qos,
        dds::domain::DomainParticipantListener *listener,
        const dds::core::status::StatusMask& event_mask)
    : qos_(qos)
{
    os_uint32 timeout = 1;

    /* u_userInitialise() needs to be called BEFORE the start of the REPORT_STACK,
     * because the REPORT_STACK does not function without proper UserLayer initialization.
     * Therefore, if initialization fails do not throw exceptions that depend on the
     * report stack. Instead throw a primitive Exception directly.
     */
    u_result result = u_userInitialise();
    if (result != U_RESULT_OK) throw dds::core::Error("Could not initialize UserLayer");

    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    // validate the qos and get the corresponding kernel qos
    qos.delegate().check();
    u_participantQos uQos = qos.delegate().u_qos();

    os_char *participantName = u_userGetProcessName();
    u_participant uParticipant = u_participantNew(NULL, id, timeout, participantName, uQos, FALSE);
    os_free(participantName);
    u_participantQosFree(uQos);

    if (!uParticipant) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Failed to create DomainParticipant");
    }

    this->userHandle = u_object(uParticipant);

    set_domain_id(u_participantGetDomainId(u_participant(this->userHandle)));

    this->listener_set((void*)listener, event_mask);
}

void
org::opensplice::domain::DomainParticipantDelegate::init(ObjectDelegate::weak_ref_type weak_ref)
{
    /* Set weak_ref. */
    this->set_weak_ref(weak_ref);
    /* Create and set listener dispatcher. */
    /* TODO: Add Scheduling to listener dispatcher creation. */
    this->listener_dispatcher_set(
            org::opensplice::core::ListenerDispatcher::create(
                    u_participant(this->userHandle),
                    this->qos_->policy<org::opensplice::core::policy::ListenerScheduling>()));
    /* This only starts listening when the status mask shows interest. */
    this->listener_enable();
    /* No 'factory': always enable. */
    this->enable();
}

org::opensplice::domain::DomainParticipantDelegate::~DomainParticipantDelegate()
{
    if (!this->closed) {
        try {
            close();
        } catch (...) {

        }
    }
}

uint32_t
org::opensplice::domain::DomainParticipantDelegate::domain_id()
{
    u_domainId_t id = u_participantGetDomainId(u_participant(this->userHandle));

    if (id == U_DOMAIN_ID_INVALID) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ALREADY_CLOSED_ERROR, "Failed to get domain id, domain already closed.");
    }
    return id;
}

void
org::opensplice::domain::DomainParticipantDelegate::assert_liveliness()
{
    check();

    u_result uResult = u_participantAssertLiveliness(u_participant(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not assert liveliness.");
}

bool
org::opensplice::domain::DomainParticipantDelegate::contains_entity(
    const dds::core::InstanceHandle& handle)
{
    bool contains = false;

    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    contains = this->publishers.contains(handle);

    if (!contains) {
        contains = this->subscribers.contains(handle);
    }

    if (!contains) {
        contains = this->topics.contains(handle);
    }

    scopedLock.unlock();

    return contains;
}


dds::core::Time
org::opensplice::domain::DomainParticipantDelegate::current_time() const
{
    os_timeW now = os_timeWGet();

    return dds::core::Time(OS_TIMEW_GET_SECONDS(now), OS_TIMEW_GET_NANOSECONDS(now));
}

const dds::domain::qos::DomainParticipantQos&
org::opensplice::domain::DomainParticipantDelegate::qos() const
{
    this->check();
    return this->qos_;
}


void
org::opensplice::domain::DomainParticipantDelegate::qos(
        const dds::domain::qos::DomainParticipantQos& qos)
{
    qos.delegate().check();

    u_participantQos pQos = qos.delegate().u_qos();

    u_result uResult = u_participantSetQos(u_participant(this->userHandle), pQos);
    u_participantQosFree(pQos);

    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not set participant qos.");

    this->lock();
    this->qos_ = qos;
    this->unlock();
}

void
org::opensplice::domain::DomainParticipantDelegate::close()
{
    /* Stop listener. */
    this->listener(NULL, dds::core::status::StatusMask::none());
    /* Remove and delete dispatcher. */
    org::opensplice::core::ListenerDispatcher::destroy(this->listener_dispatcher_reset());

    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    this->publishers.all_close();
    this->subscribers.all_close();
    this->cfTopics.all_close();
    this->topics.all_close();

    org::opensplice::domain::DomainParticipantRegistry::remove(this);

    org::opensplice::core::EntityDelegate::close();

    scopedLock.unlock();
}

dds::topic::qos::TopicQos
org::opensplice::domain::DomainParticipantDelegate::default_topic_qos() const
{
    dds::topic::qos::TopicQos qos;

    this->lock();
    qos = this->default_topic_qos_;
    this->unlock();

    return qos;
}

void
org::opensplice::domain::DomainParticipantDelegate::default_topic_qos(
        const dds::topic::qos::TopicQos& qos)
{
    qos.delegate().check();

    this->lock();
    this->default_topic_qos_ = qos;
    this->unlock();
}

dds::pub::qos::PublisherQos
org::opensplice::domain::DomainParticipantDelegate::default_publisher_qos() const
{
    dds::pub::qos::PublisherQos qos;

    this->lock();
    qos = this->default_pub_qos_;
    this->unlock();

    return qos;
}

void
org::opensplice::domain::DomainParticipantDelegate::default_publisher_qos(
        const ::dds::pub::qos::PublisherQos& qos)
{
    qos.delegate().check();

    this->lock();
    this->default_pub_qos_ = qos;
    this->unlock();
}

dds::sub::qos::SubscriberQos
org::opensplice::domain::DomainParticipantDelegate::default_subscriber_qos() const
{
    dds::sub::qos::SubscriberQos qos;

    this->lock();
    qos = this->default_sub_qos_;
    this->unlock();

    return qos;
}

void
org::opensplice::domain::DomainParticipantDelegate::default_subscriber_qos(
        const ::dds::sub::qos::SubscriberQos& qos)
{
    qos.delegate().check();

    this->lock();
    this->default_sub_qos_ = qos;
    this->unlock();
}

dds::domain::qos::DomainParticipantQos
org::opensplice::domain::DomainParticipantDelegate::default_participant_qos()
{
    default_participant_qos_lock_.lock();
    dds::domain::qos::DomainParticipantQos qos = default_participant_qos_;
    default_participant_qos_lock_.unlock();

    return qos;
}

void
org::opensplice::domain::DomainParticipantDelegate::default_participant_qos(
        const ::dds::domain::qos::DomainParticipantQos& qos)
{
    qos.delegate().check();

    default_participant_qos_lock_.lock();
    default_participant_qos_= qos;
    default_participant_qos_lock_.unlock();
}

void
org::opensplice::domain::DomainParticipantDelegate::add_publisher(
        org::opensplice::core::EntityDelegate& publisher)
{
    this->publishers.insert(publisher);
}

void
org::opensplice::domain::DomainParticipantDelegate::remove_publisher(
        org::opensplice::core::EntityDelegate& publisher)
{
    this->publishers.erase(publisher);
}

void
org::opensplice::domain::DomainParticipantDelegate::add_subscriber(
        org::opensplice::core::EntityDelegate& subscriber)
{
    this->subscribers.insert(subscriber);
}

void
org::opensplice::domain::DomainParticipantDelegate::remove_subscriber(
        org::opensplice::core::EntityDelegate& subscriber)
{
    this->subscribers.erase(subscriber);
}

void
org::opensplice::domain::DomainParticipantDelegate::add_topic(
        org::opensplice::core::EntityDelegate& topic)
{
    this->topics.insert(topic);
}

void
org::opensplice::domain::DomainParticipantDelegate::remove_topic(
        org::opensplice::core::EntityDelegate& topic)
{
    this->topics.erase(topic);
}

void
org::opensplice::domain::DomainParticipantDelegate::add_cfTopic(
        org::opensplice::core::ObjectDelegate& cfTopic)
{
    this->cfTopics.insert(cfTopic);
}

void
org::opensplice::domain::DomainParticipantDelegate::remove_cfTopic(
        org::opensplice::core::ObjectDelegate& cfTopic)
{
    this->cfTopics.erase(cfTopic);
}


u_topic
org::opensplice::domain::DomainParticipantDelegate::lookup_topic(
        const std::string& topic_name,
        const dds::core::Duration& timeout)
{
    u_topic uTopic = NULL;
    os_duration d;

    check();

    u_participant uParticipant = u_participant(this->userHandle);

    /* Find the Topic in the user layer. */
    d = org::opensplice::core::timeUtils::convertDuration(timeout);
    c_iter list = u_participantFindTopic(uParticipant, topic_name.c_str(), d);
    if (c_iterLength(list) != 0) {
        assert(c_iterLength(list) == 1); /* Accept zero or one topic. */
        uTopic = u_topic(c_iterTakeFirst(list));
        if (uTopic == NULL) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Failed to get user layer topic");
        }
    }
    c_iterFree(list);
    list = NULL;

    return uTopic;
}

void
org::opensplice::domain::DomainParticipantDelegate::lookup_topics(
        const std::string& type_name,
        std::vector<u_topic>& topics,
        uint32_t max_size)
{
    topics.clear();

    check();

    u_participant uParticipant = u_participant(this->userHandle);

    /* Find the Topic in the user layer. */
    c_iter list = u_participantFindTopic(uParticipant, "*", 0);
    uint32_t n = 0;
    if (c_iterLength(list) != 0) {
        u_topic uTopic = u_topic(c_iterTakeFirst(list));
        while ((uTopic != NULL) && (n < max_size)) {
            if (!type_name.empty()) {
                char *tn = u_topicTypeName(uTopic);
                if (type_name == tn) {
                    topics.push_back(uTopic);
                    n++;
                }
                os_free(tn);
            } else {
                topics.push_back(uTopic);
                n++;
            }
            uTopic = u_topic(c_iterTakeFirst(list));
        }
        while (uTopic != NULL) {
            uTopic = u_topic(c_iterTakeFirst(list));
        }
    }
    c_iterFree(list);
}


org::opensplice::core::EntityDelegate::ref_type
org::opensplice::domain::DomainParticipantDelegate::find_topic(
        const std::string& topic_name)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    org::opensplice::topic::AnyTopicDelegate::ref_type topic;

    org::opensplice::core::EntitySet::vector entities;
    org::opensplice::core::EntitySet::vectorIterator iter;

    entities = this->topics.copy();
    iter = entities.begin();

    for (iter = entities.begin(); iter != entities.end(); ++iter) {
        org::opensplice::core::ObjectDelegate::ref_type ref = iter->lock();
        if (ref) {
            org::opensplice::topic::AnyTopicDelegate::ref_type tmp =
                    OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::topic::AnyTopicDelegate>(ref);
            assert(tmp);
            if (tmp->name() == topic_name) {
                topic = tmp;
            }
        }
    }

    return topic;
}

org::opensplice::core::ObjectDelegate::ref_type
org::opensplice::domain::DomainParticipantDelegate::find_cfTopic(
        const std::string& topic_name)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    org::opensplice::topic::TopicDescriptionDelegate::ref_type cftopic;

    org::opensplice::core::ObjectSet::vector entities;
    org::opensplice::core::ObjectSet::vectorIterator iter;

    entities = this->cfTopics.copy();
    iter = entities.begin();

    for (iter = entities.begin(); iter != entities.end(); ++iter) {
        org::opensplice::core::ObjectDelegate::ref_type ref = iter->lock();
        if (ref) {
            org::opensplice::topic::TopicDescriptionDelegate::ref_type tmp =
                    OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::topic::TopicDescriptionDelegate>(ref);
            assert(tmp);
            if (tmp->name() == topic_name) {
                cftopic = tmp;
            }
        }
    }

    return cftopic;
}

org::opensplice::core::EntityDelegate::ref_type
org::opensplice::domain::DomainParticipantDelegate::builtin_subscriber()
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    org::opensplice::core::EntityDelegate::ref_type builtinSub = this->builtin_subscriber_.lock();

    return builtinSub;
}

void
org::opensplice::domain::DomainParticipantDelegate::builtin_subscriber(
    const org::opensplice::core::EntityDelegate::ref_type subscriber)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    this->builtin_subscriber_ = subscriber;
}

u_participant
org::opensplice::domain::DomainParticipantDelegate::registerType(
        const std::string typeName,
        const std::string typeDescriptor,
        org::opensplice::topic::DataRepresentationId_t dataRepresentationId,
        const std::vector<os_uchar> typeHash,
        const std::vector<os_uchar> metaData,
        const std::vector<os_uchar> extentions) const
{
    u_participant participant;
    u_domain uDomain;
    u_result uResult;
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    C_STRUCT(u_typeRepresentation) tr;

    if (dataRepresentationId == org::opensplice::topic::INVALID_REPRESENTATION) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
                "Topic type without traits detected. This can happen by using a non-topic type or including the wrong header file.");
    }

    participant = u_participant(this->userHandle);
    uDomain = u_participantDomain(participant);
    uResult = u_domain_load_xml_descriptor(uDomain, typeDescriptor.c_str());
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Type conflict detected.");
    if (dataRepresentationId != org::opensplice::topic::OSPL_REPRESENTATION) {
        memset(&tr, 0, sizeof(tr));
        tr.typeName = typeName.c_str();
        tr.dataRepresentationId = dataRepresentationId;
        tr.typeHash = u_typeHashFromArray(&typeHash[0], typeHash.size());
        tr.metaData = &metaData[0];
        tr.metaDataLen = metaData.size();
        if (extentions.size()) {
            tr.extentions = &extentions[0];
            tr.extentionsLen = extentions.size();
        }
        uResult = u_participantRegisterTypeRepresentation(participant, &tr);
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "TypeRepresentation registration failed.");
    }

    return participant;
}


void
org::opensplice::domain::DomainParticipantDelegate::listener(
        dds::domain::DomainParticipantListener *listener,
        const ::dds::core::status::StatusMask& mask)
{
    /* EntityDelegate takes care of thread safety. */
    this->listener_set((void*)listener, mask);
    this->listener_enable();
}


dds::domain::DomainParticipantListener*
org::opensplice::domain::DomainParticipantDelegate::listener() const
{
    return reinterpret_cast<dds::domain::DomainParticipantListener*>(this->listener_get());
}


dds::domain::TDomainParticipant<org::opensplice::domain::DomainParticipantDelegate>
org::opensplice::domain::DomainParticipantDelegate::wrapper()
{
    DomainParticipantDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DomainParticipantDelegate>(this->get_strong_ref());
    dds::domain::DomainParticipant dp(ref);
    return dp;
}

bool
org::opensplice::domain::DomainParticipantDelegate::is_auto_enable() const
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    return this->qos_.delegate().policy<dds::core::policy::EntityFactory>().delegate().auto_enable();
}

void
org::opensplice::domain::DomainParticipantDelegate::listener_notify(
        ObjectDelegate::ref_type source,
        uint32_t                 triggerMask,
        void                    *eventData,
        void                    *l)
{
    /* The EntityDelegate takes care of the thread safety and always
     * provides a listener and source. */
    dds::domain::DomainParticipantListener* listener =
            reinterpret_cast<dds::domain::DomainParticipantListener*>(l);
    assert(listener);

    /* Events that take an AnyDataWriter. */
    if ((triggerMask & V_EVENT_LIVELINESS_LOST         ) ||
        (triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED ) ||
        (triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) ||
        (triggerMask & V_EVENT_PUBLICATION_MATCHED     ) )
    {
        /* Get AnyDataWriter from given source EntityDelegate. */
        org::opensplice::pub::AnyDataWriterDelegate::ref_type ref =
                OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::pub::AnyDataWriterDelegate>(
                        source);
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

    /* Events that take an AnyDataReader. */
    if ((triggerMask & V_EVENT_DATA_AVAILABLE            ) ||
        (triggerMask & V_EVENT_SAMPLE_REJECTED           ) ||
        (triggerMask & V_EVENT_LIVELINESS_CHANGED        ) ||
        (triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED ) ||
        (triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) ||
        (triggerMask & V_EVENT_SAMPLE_LOST               ) ||
        (triggerMask & V_EVENT_SUBSCRIPTION_MATCHED      ) )
    {
        /* Get AnyDataReader from given source EntityDelegate. */
        org::opensplice::sub::AnyDataReaderDelegate::ref_type ref =
                OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::sub::AnyDataReaderDelegate>(
                        source);
        dds::sub::AnyDataReader reader = ref->wrapper_to_any();

        if (triggerMask & V_EVENT_DATA_AVAILABLE) {
            ref->reset_data_available();
            listener->on_data_available(reader);
        }

        if (triggerMask & V_EVENT_SAMPLE_REJECTED) {
            dds::core::status::SampleRejectedStatus status;
            status.delegate().v_status(v_readerStatus(eventData)->sampleRejected);
            listener->on_sample_rejected(reader, status);
        }

        if (triggerMask & V_EVENT_LIVELINESS_CHANGED) {
            dds::core::status::LivelinessChangedStatus status;
            status.delegate().v_status(v_readerStatus(eventData)->livelinessChanged);
            listener->on_liveliness_changed(reader, status);
        }

        if (triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            dds::core::status::RequestedDeadlineMissedStatus status;
            status.delegate().v_status(v_readerStatus(eventData)->deadlineMissed);
            listener->on_requested_deadline_missed(reader, status);
        }

        if (triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            dds::core::status::RequestedIncompatibleQosStatus status;
            status.delegate().v_status(v_readerStatus(eventData)->incompatibleQos);
            listener->on_requested_incompatible_qos(reader, status);
        }

        if (triggerMask & V_EVENT_SAMPLE_LOST) {
            dds::core::status::SampleLostStatus status;
            status.delegate().v_status(v_readerStatus(eventData)->sampleLost);
            listener->on_sample_lost(reader, status);
        }

        if (triggerMask & V_EVENT_SUBSCRIPTION_MATCHED) {
            dds::core::status::SubscriptionMatchedStatus status;
            status.delegate().v_status(v_readerStatus(eventData)->subscriptionMatch);
            listener->on_subscription_matched(reader, status);
        }
    }

    /* Events that take a Subscriber. */
    if (triggerMask & V_EVENT_ON_DATA_ON_READERS) {
        /* Only trigger when V_EVENT_DATA_AVAILABLE isn't triggered. */
        if (!(triggerMask & V_EVENT_DATA_AVAILABLE)) {
            org::opensplice::sub::SubscriberDelegate::ref_type ref =
                    OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::sub::SubscriberDelegate>(
                            source);
            ref->reset_data_on_readers();
            dds::sub::Subscriber subscriber(ref);
            listener->on_data_on_readers(subscriber);
        }
    }

    /* Events that take a topic. */
    if ((triggerMask & V_EVENT_INCONSISTENT_TOPIC) ||
        (triggerMask & V_EVENT_ALL_DATA_DISPOSED ) )
    {
        /* Get AnyTopic from given source EntityDelegate. */
        org::opensplice::topic::AnyTopicDelegate::ref_type ref =
                OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::topic::AnyTopicDelegate>(
                        source);
        dds::topic::AnyTopic topic = ref->wrapper_to_any();

        if (triggerMask & V_EVENT_INCONSISTENT_TOPIC) {
            dds::core::status::InconsistentTopicStatus status;
            status.delegate().v_status(v_topicStatus(eventData)->inconsistentTopic);
            listener->on_inconsistent_topic(topic, status);
        }

        if (triggerMask & V_EVENT_ALL_DATA_DISPOSED ) {
            org::opensplice::domain::DomainParticipantListener* extListener =
                      dynamic_cast<org::opensplice::domain::DomainParticipantListener*>(listener);
            if (extListener) {
                org::opensplice::core::status::AllDataDisposedTopicStatus status;
                status.delegate().v_status(v_topicStatus(eventData)->allDataDisposed);
                extListener->on_all_data_disposed(topic, status);
            }
        }
    }
}

void
org::opensplice::domain::DomainParticipantDelegate::delete_historical_data(
        const std::string& partition_expression,
        const std::string& topic_expression)
{
    this->check();
    u_result uResult = u_participantDeleteHistoricalData(u_participant(this->userHandle),
                                                         partition_expression.c_str(),
                                                         topic_expression.c_str());
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Historical data not deleted");
}

void
org::opensplice::domain::DomainParticipantDelegate::create_persistent_snapshot(
        const std::string& partition_expression,
        const std::string& topic_expression,
        const std::string& uri)
{
    this->check();
    u_result uResult = u_domainCreatePersistentSnapshot(u_participantDomain(u_participant(this->userHandle)),
                                                        partition_expression.c_str(),
                                                        topic_expression.c_str(),
                                                        uri.c_str());
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Failed to create snapshot");
}

std::string
org::opensplice::domain::DomainParticipantDelegate::create_child_name(
        const std::string& prefix)
{
    std::ostringstream child;
    os_char *name;

    /* Get the participants' name. */
    name = u_entityName(u_entity(this->get_user_handle()));
    if (name) {
        /* Remove possibly " <pid>" name extension. */
        std::string base(name);
        /* Does base end with '>'? */
        if (*base.rbegin() == '>') {
            std::ostringstream pid;
            pid << " <" << os_procIdSelf() << ">";
            std::string::size_type end = base.find(pid.str());
            if (end != std::string::npos) {
                base.erase(end, pid.tellp());
            }
        }

        child << prefix << "<" << base << ">";

        os_free(name);
    }

    return child.str();
}

void
org::opensplice::domain::DomainParticipantDelegate::detach_all_domains(
        bool block_operations,
        bool delete_entities)
{
    uint32_t flags = 0;
    u_result ures;

    if (block_operations) {
        flags |= U_USER_BLOCK_OPERATIONS;
    }
    if (delete_entities) {
        flags |= U_USER_DELETE_ENTITIES;
    }

    ures = u_userDetach(flags);
    ISOCPP_U_RESULT_CHECK_AND_THROW(ures, "Could not detach from all domains.");
}
