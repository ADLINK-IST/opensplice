/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
*
*/


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_SUB_SAMPLE_INFO_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_SAMPLE_INFO_IMPL_HPP_

#include <org/opensplice/core/config.hpp>
#include <dds/sub/Rank.hpp>
#include <dds/sub/GenerationCount.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
class SampleInfoImpl;
}
}
}

/**
 *  @internal This is a "brutal" implementation of the SampleInfo which is
 * intented to give encapsulation to an existing SampleInfo by simply
 * casting the DDS::SampleInfo into an org::opensplice::sub::SampleInfo type.
 *
 * This is kind of ugly but very efficient!
 */
class org::opensplice::sub::SampleInfoImpl : private DDS::SampleInfo
{
public:
    SampleInfoImpl() { }
public:

    inline const dds::core::Time
    timestamp() const
    {
        return dds::core::Time(this->source_timestamp.sec, this->source_timestamp.nanosec);
    }

    inline const dds::sub::status::DataState
    state() const
    {
        return dds::sub::status::DataState(dds::sub::status::SampleState(this->sample_state),
                                           dds::sub::status::ViewState(this->view_state),
                                           dds::sub::status::InstanceState(this->instance_state));
    }

    inline dds::sub::GenerationCount
    generation_count() const
    {
        return dds::sub::GenerationCount(this->disposed_generation_count,
                                         this->no_writers_generation_count);
    }

    inline dds::sub::Rank
    rank() const
    {
        return dds::sub::Rank(this->sample_rank,
                              this->generation_rank,
                              this->absolute_generation_rank);
    }

    inline bool
    valid() const
    {
        /** @internal @bug OSPL-918 DDS::Boolean is not (yet!) a bool
            @todo Remove fudge when OSPL-918 fixed
            @see http://jira.prismtech.com:8080/browse/OSPL-918 */
        return (this->valid_data ? true : false);
    }

    inline dds::core::InstanceHandle
    instance_handle() const
    {
        return dds::core::InstanceHandle(this->DDS::SampleInfo::instance_handle);
    }

    inline dds::core::InstanceHandle
    publication_handle() const
    {
        return dds::core::InstanceHandle(this->DDS::SampleInfo::publication_handle);
    }

    bool operator==(const SampleInfoImpl& other) const
    {
        return other.sample_state == sample_state && other.view_state == view_state
               && other.instance_state == instance_state && other.valid_data == valid_data
               && other.valid_data == valid_data && other.source_timestamp.sec == source_timestamp.sec
               && other.source_timestamp.nanosec == source_timestamp.nanosec && other.DDS::SampleInfo::instance_handle == this->DDS::SampleInfo::instance_handle
               && other.DDS::SampleInfo::publication_handle == this->DDS::SampleInfo::publication_handle && other.disposed_generation_count == disposed_generation_count
               && other.no_writers_generation_count == no_writers_generation_count && other.sample_rank == sample_rank
               && other.generation_rank == generation_rank && other.absolute_generation_rank == absolute_generation_rank
               && other.reception_timestamp.sec == reception_timestamp.sec && other.reception_timestamp.nanosec == reception_timestamp.nanosec;
    }

};


#endif /* ORG_OPENSPLICE_SUB_SAMPLE_INFO_IMPL_HPP_ */
