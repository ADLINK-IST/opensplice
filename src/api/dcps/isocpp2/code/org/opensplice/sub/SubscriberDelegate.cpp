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

#include <dds/sub/DataReader.hpp>
#include <dds/sub/AnyDataReader.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/SubscriberListener.hpp>

#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/sub/SubscriberDelegate.hpp>
#include <org/opensplice/sub/AnyDataReaderDelegate.hpp>
#include <org/opensplice/core/ScopedLock.hpp>

#include "u_types.h"
#include "u_publisher.h"

#include "v_status.h"

/* My compiler substitutes the v_status() function of the Status delegates
 * with the v_status cast macro of the kernel. This prevents compilation.
 * To solve the issue, undefine the marcro: */
#undef v_status


namespace org
{
namespace opensplice
{
namespace sub
{

SubscriberDelegate::SubscriberDelegate(
    const dds::domain::DomainParticipant& dp,
    const dds::sub::qos::SubscriberQos& qos,
    dds::sub::SubscriberListener* listener,
    const dds::core::status::StatusMask& event_mask) :
    dp_(dp),
    qos_(qos)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);

    u_subscriber uSub;
    u_participant uPar;
    u_subscriberQos uQos;

    uPar = u_participant(this->dp_.delegate()->get_user_handle());
    if (!uPar) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not get subscriber participant.");
    }

    qos.delegate().check();
    uQos = qos.delegate().u_qos();
    if (!uQos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not convert subscriber QoS.");
    }

    std::string name = this->dp_.delegate()->create_child_name("subscriber");
    uSub = u_subscriberNew(uPar, name.c_str(), uQos, false);
    u_subscriberQosFree (uQos);
    if (!uSub) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not create subscriber.");
    }

    /* ObjectDelegate class will free the userlayer object in its destructor. */
    this->userHandle = u_object(uSub);
    this->listener_set((void*)listener, event_mask);
    set_domain_id(dp.delegate()->get_domain_id());
}

SubscriberDelegate::~SubscriberDelegate()
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
SubscriberDelegate::init(ObjectDelegate::weak_ref_type weak_ref)
{
    /* Set weak_ref before passing ourselves to other isocpp objects. */
    this->set_weak_ref(weak_ref);
    /* Register Publisher at Participant. */
    this->dp_.delegate()->add_subscriber(*this);
    /* Use listener dispatcher from the domain participant. */
    this->listener_dispatcher_set(this->dp_.delegate()->listener_dispatcher_get());
    /* This only starts listening when the status mask shows interest. */
    this->listener_enable();
    /* Enable when needed. */
    if (this->dp_.delegate()->is_auto_enable()) {
        this->enable();
    }
}

void
SubscriberDelegate::close()
{
    /* Stop listener and remove dispatcher. */
    this->listener(NULL, dds::core::status::StatusMask::none());
    this->listener_dispatcher_reset();

    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    /* Close the datareaders. */
    this->readers.all_close();

    /* Unregister Subscriber from Participant. */
    this->dp_.delegate()->remove_subscriber(*this);

    org::opensplice::core::EntityDelegate::close();
}

const dds::sub::qos::SubscriberQos&
SubscriberDelegate::qos() const
{
    this->check();
    return this->qos_;
}

void
SubscriberDelegate::qos(const dds::sub::qos::SubscriberQos& sqos)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_subscriberQos uQos;
    u_result uResult;

    sqos.delegate().check();
    uQos = sqos.delegate().u_qos();
    if (!uQos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not convert subscriber qos.");
    }

    uResult = u_subscriberSetQos(u_subscriber(this->userHandle), uQos);
    u_subscriberQosFree(uQos);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not set subscriber qos.");

    this->qos_ = sqos;
}

dds::sub::qos::DataReaderQos
SubscriberDelegate::default_datareader_qos() const
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    dds::sub::qos::DataReaderQos qos = this->default_dr_qos_;
    return qos;
}

void
SubscriberDelegate::default_datareader_qos(const dds::sub::qos::DataReaderQos& drqos)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    drqos.delegate().check();
    this->default_dr_qos_ = drqos;
}

void
SubscriberDelegate::begin_coherent_access()
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    this->check();
    u_result uResult = u_subscriberBeginAccess(u_subscriber(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not begin coherent access.");
    ISOCPP_REPORT_STACK_END();
}

void
SubscriberDelegate::end_coherent_access()
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    this->check();
    u_result uResult = u_subscriberEndAccess(u_subscriber(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not end coherent access.");
    ISOCPP_REPORT_STACK_END();
}


/*
void SubscriberDelegate::init_builtin(DDS::Subscriber_ptr ddssub)
{
    if(ddssub == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                    OSPL_CONTEXT_LITERAL(
                        "dds::core::NullReferenceError : Unable to get builtin Subscriber. "
                        "Nil return from ::get_builtin_subscriber")));

    DDS::SubscriberQos qos;
    DDS::ReturnCode_t result = ddssub->get_qos(qos);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_qos"));
    qos_ = org::opensplice::sub::qos::convertQos(qos);

    sub_.reset(ddssub, ::org::opensplice::core::SubDeleter(dp_->dp_));

    DDS::DataReaderQos oldqos;
    result = sub_->get_default_datareader_qos(oldqos);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_default_datareader_qos"));

    default_dr_qos_ = org::opensplice::sub::qos::convertQos(oldqos);
    entity_ = DDS::Entity::_narrow(ddssub);

    org::opensplice::core::SubDeleter* d = OSPL_CXX11_STD_MODULE::get_deleter<org::opensplice::core::SubDeleter>(sub_);
    if(d)
    {
        d->set_builtin();
    }
}
*/

const dds::domain::DomainParticipant&
SubscriberDelegate::participant() const
{
    this->check();
    return dp_;
}

void
SubscriberDelegate::listener(dds::sub::SubscriberListener* listener,
                            const ::dds::core::status::StatusMask& mask)
{
    /* EntityDelegate takes care of thread safety. */
    this->listener_set((void*)listener, mask);
    this->listener_enable();
}

dds::sub::SubscriberListener*
SubscriberDelegate::listener() const
{
    this->check();
    return reinterpret_cast<dds::sub::SubscriberListener*>(this->listener_get());
}

bool
SubscriberDelegate::contains_entity(
    const dds::core::InstanceHandle& handle)
{
    return this->readers.contains(handle);
}

void
SubscriberDelegate::add_datareader(
    org::opensplice::core::EntityDelegate& datareader)
{
    this->readers.insert(datareader);
}

void
SubscriberDelegate::remove_datareader(
    org::opensplice::core::EntityDelegate& datareader)
{
    this->readers.erase(datareader);
}

std::vector<org::opensplice::sub::AnyDataReaderDelegate::ref_type>
SubscriberDelegate::find_datareaders(const std::string& topic_name)
{
    std::vector<org::opensplice::sub::AnyDataReaderDelegate::ref_type> readers;
    //org::opensplice::sub::AnyDataReaderDelegate::ref_type reader;

    org::opensplice::core::EntitySet::vector entities;
    org::opensplice::core::EntitySet::vectorIterator iter;

    entities = this->readers.copy();
    readers.reserve(entities.size());
    for (iter = entities.begin(); iter != entities.end(); ++iter) {
        org::opensplice::core::ObjectDelegate::ref_type ref = iter->lock();
        if (ref) {
            org::opensplice::sub::AnyDataReaderDelegate::ref_type tmp =
                    OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<AnyDataReaderDelegate>(ref);
            assert(tmp);
            if (tmp->topic_description().name() == topic_name) {
                readers.push_back(tmp);
            }
        }
    }

    return readers;
}

std::vector<org::opensplice::sub::AnyDataReaderDelegate::ref_type>
SubscriberDelegate::get_datareaders(
    const dds::sub::status::DataState& mask)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    std::vector<org::opensplice::sub::AnyDataReaderDelegate::ref_type> readers;
    u_dataReader uReader;
    u_sampleMask uMask;
    u_result uResult;
    c_iter uList;

    /* Get list from user layer. */
    uMask = org::opensplice::sub::AnyDataReaderDelegate::getUserMask(mask);
    uResult = u_subscriberGetDataReaders(u_subscriber(this->userHandle), uMask, &uList);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not get datareaders.");

    /* Translate user layer list. */
    readers.reserve(c_iterLength(uList));
    while ((uReader = u_dataReader(c_iterTakeFirst(uList))) != NULL) {
        org::opensplice::core::ObjectDelegate::ref_type reader =
                org::opensplice::core::EntityDelegate::extract_strong_ref(u_entity(uReader));
        if (reader) {
            readers.push_back(OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<AnyDataReaderDelegate>(reader));
        }
    }
    c_iterFree(uList);

    return readers;
}

void
SubscriberDelegate::notify_datareaders()
{

}

dds::sub::TSubscriber<SubscriberDelegate>
SubscriberDelegate::wrapper()
{
    SubscriberDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<SubscriberDelegate>(this->get_strong_ref());
    dds::sub::Subscriber sub(ref);
    return sub;
}

bool
SubscriberDelegate::is_auto_enable() const
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    bool autoEnable = this->qos_.delegate().policy<dds::core::policy::EntityFactory>().delegate().auto_enable();

    return autoEnable;
}


void
SubscriberDelegate::listener_notify(
        ObjectDelegate::ref_type source,
        uint32_t                 triggerMask,
        void                    *eventData,
        void                    *l)
{
    /* The EntityDelegate takes care of the thread safety and always
     * provides a listener and source. */
    dds::sub::SubscriberListener* listener =
            reinterpret_cast<dds::sub::SubscriberListener*>(l);
    assert(listener);

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
        AnyDataReaderDelegate::ref_type ref =
                OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<AnyDataReaderDelegate>(source);
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
}

void
SubscriberDelegate::reset_data_on_readers()
{
    u_result uResult;

    /* TODO: This is a bit tricky, the entity may already been deleted and in that case
    * this operation will perform a dirty memory read.
    * It may be better to wipe all pending events belonging to an entity when it is deleted or
    * if that is too intrusive find another way to safely detect/avoid deletion.
    * NOTE: This was copied from SAC.
    */
    uResult = u_observableAction(
                        u_observable(this->userHandle),
                        SubscriberDelegate::reset_data_on_readers_callback,
                        NULL);

    if (uResult != U_RESULT_OK) {
        ISOCPP_REPORT_WARNING("Could not reset data available status.");
    }
}

void
SubscriberDelegate::reset_data_on_readers_callback(
   v_public p,
   c_voidp arg)
{
    OS_UNUSED_ARG(arg);

    v_statusReset(v_entity(p)->status, V_EVENT_ON_DATA_ON_READERS);
}




}
}
}
