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

#ifndef ORG_OPENSPLICE_SUB_ANY_DATA_READER_DELEGATE_HPP_
#define ORG_OPENSPLICE_SUB_ANY_DATA_READER_DELEGATE_HPP_

#include <dds/core/types.hpp>
#include <dds/core/Time.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/status/Status.hpp>
#include <dds/sub/status/detail/DataStateImpl.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>
#include <dds/sub/Sample.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/topic/TopicTraits.hpp>
#include <org/opensplice/core/ObjectSet.hpp>
#include <org/opensplice/ForwardDeclarations.hpp>
#include <dds/topic/TopicDescription.hpp>

#include <dds/topic/BuiltinTopic.hpp>

#include <u_dataReader.h>
#include "cmn_samplesList.h"


namespace dds { namespace sub {
template <typename DELEGATE>
class TAnyDataReader;
} }

namespace org { namespace opensplice { namespace sub {
class QueryContainer;
} } }

namespace dds
{
namespace sub
{
namespace detail
{

class SamplesHolder
{
public:
    SamplesHolder() {}
    virtual ~SamplesHolder() {}

    virtual void set_length(uint32_t len) = 0;
    virtual uint32_t get_length() const = 0;
    virtual SamplesHolder& operator++(int) = 0;
    virtual void *data() = 0;
    virtual detail::SampleInfo* info() = 0;
};

}
}
}



namespace org
{
namespace opensplice
{
namespace sub
{

class QueryDelegate;
class SubscriberDelegate;


class OMG_DDS_API AnyDataReaderDelegate : public org::opensplice::core::EntityDelegate
{
public:
    typedef ::dds::core::smart_ptr_traits< AnyDataReaderDelegate >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< AnyDataReaderDelegate >::weak_ref_type weak_ref_type;

    AnyDataReaderDelegate(const dds::sub::qos::DataReaderQos& qos,
                          const dds::topic::TopicDescription& td);
    virtual ~AnyDataReaderDelegate();

public:
    /* DDS API mirror. */
    dds::sub::qos::DataReaderQos qos() const;
    void qos(const dds::sub::qos::DataReaderQos& qos);

    /* Let DataReader<T> implement the subscriber handling to circumvent circular dependencies. */
    virtual const dds::sub::TSubscriber<org::opensplice::sub::SubscriberDelegate>& subscriber() const = 0;
    const dds::topic::TopicDescription& topic_description() const;

    void wait_for_historical_data(const dds::core::Duration& timeout);

    void wait_for_historical_data_w_condition (
        const std::string& filter_expression,
        const std::vector<std::string>& filter_parameters,
        const dds::core::Time& min_source_timestamp,
        const dds::core::Time& max_source_timestamp,
        const dds::core::policy::ResourceLimits& resource_limits,
        const dds::core::Duration& timeout);

    dds::core::status::LivelinessChangedStatus
    liveliness_changed_status();
    dds::core::status::SampleRejectedStatus
    sample_rejected_status();
    dds::core::status::SampleLostStatus
    sample_lost_status();
    dds::core::status::RequestedDeadlineMissedStatus
    requested_deadline_missed_status();
    dds::core::status::RequestedIncompatibleQosStatus
    requested_incompatible_qos_status();
    dds::core::status::SubscriptionMatchedStatus
    subscription_matched_status();

    ::dds::core::InstanceHandleSeq
     matched_publications();

    template <typename FwdIterator>
    uint32_t
    matched_publications(FwdIterator begin, uint32_t max_size)
    {
        ::dds::core::InstanceHandleSeq handleSeq = matched_publications();
        uint32_t size = (handleSeq.size() < max_size ? handleSeq.size() : max_size);
        for (uint32_t i = 0; i < size; i++, begin++) {
            *begin = handleSeq[i];
        }
        return size;
    }

    const dds::topic::PublicationBuiltinTopicData
    matched_publication_data(const ::dds::core::InstanceHandle& h);

public:
    /* Internal API. */
    dds::sub::TAnyDataReader<AnyDataReaderDelegate> wrapper_to_any();

    static u_sampleMask getUserMask(const dds::sub::status::DataState& state);

    void reset_data_available();

    void add_query(org::opensplice::sub::QueryDelegate& query);
    void remove_query(org::opensplice::sub::QueryDelegate& query);

    inline void setCopyIn(org::opensplice::topic::copyInFunction copyIn)
    {
        this->copyIn = copyIn;
    }

    inline org::opensplice::topic::copyInFunction getCopyIn() const
    {
        return this->copyIn;
    }

    void setCopyOut(org::opensplice::topic::copyOutFunction copyOut)
    {
        this->copyOut = copyOut;
    }

    inline org::opensplice::topic::copyOutFunction getCopyOut() const
    {
        return this->copyOut;
    }

    void read(
            const u_dataReader reader, const dds::sub::status::DataState& mask,
            dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void take(
            const u_dataReader reader, const dds::sub::status::DataState& mask,
            dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void read_instance(
            const u_dataReader reader, const dds::core::InstanceHandle& handle,
            const dds::sub::status::DataState& mask, dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void take_instance(
            const u_dataReader reader, const dds::core::InstanceHandle& handle,
            const dds::sub::status::DataState& mask, dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void read_next_instance(
            const u_dataReader reader, const dds::core::InstanceHandle& handle,
            const dds::sub::status::DataState& mask, dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void take_next_instance(
            const u_dataReader reader, const dds::core::InstanceHandle& handle,
            const dds::sub::status::DataState& mask, dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void read_w_condition(
            const u_query query, dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void take_w_condition(
            const u_query query, dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void read_instance_w_condition(
             const u_query query, const dds::core::InstanceHandle& handle,
             dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void take_instance_w_condition(
             const u_query query, const dds::core::InstanceHandle& handle,
             dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void read_next_instance_w_condition(
            const u_query query, const dds::core::InstanceHandle& handle,
            dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);

    void take_next_instance_w_condition(
            const u_query query, const dds::core::InstanceHandle& handle,
            dds::sub::detail::SamplesHolder& samples, uint32_t max_samples);


    void get_key_value(
            const u_dataReader reader,
            const dds::core::InstanceHandle& handle, void *key);

    u_instanceHandle lookup_instance(
            const u_dataReader reader, const void *key) const;

    void close();

private:
    static void
    reset_data_available_callback(v_public p, c_voidp arg);

private:
    typedef struct FlushActionArguments {
        AnyDataReaderDelegate& reader;
        dds::sub::detail::SamplesHolder& samples;
    } FlushActionArguments;

    static void flush_action(void *sample, cmn_sampleInfo sampleInfo, void *args);
    static void copy_sample_info(cmn_sampleInfo from, dds::sub::SampleInfo *to);
    static v_copyin_result copy_key(c_type t, const void *data, void *to);

protected:
    org::opensplice::topic::copyInFunction  copyIn;
    org::opensplice::topic::copyOutFunction copyOut;
    org::opensplice::core::ObjectSet queries;
    dds::sub::qos::DataReaderQos qos_;
    dds::topic::TopicDescription td_;

};


}
}
}

#endif /* ORG_OPENSPLICE_SUB_ANY_DATA_READER_DELEGATE_HPP_ */
