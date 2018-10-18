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

#ifndef ORG_OPENSPLICE_TOPIC_TOPICDESCRIPTIONDELEGATE_HPP_
#define ORG_OPENSPLICE_TOPIC_TOPICDESCRIPTIONDELEGATE_HPP_

#include <dds/domain/DomainParticipant.hpp>
#include <org/opensplice/core/ObjectDelegate.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{

class OMG_DDS_API TopicDescriptionDelegate : public virtual org::opensplice::core::ObjectDelegate
{
public:
    typedef ::dds::core::smart_ptr_traits< TopicDescriptionDelegate >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< TopicDescriptionDelegate >::weak_ref_type weak_ref_type;

public:
    TopicDescriptionDelegate(const dds::domain::DomainParticipant& dp,
                             const std::string& name,
                             const std::string& type_name);
    virtual ~TopicDescriptionDelegate();

public:

    /**
     *  @internal Get the name used to create the TopicDescription.
     */
    const std::string& name() const;

    /**
     *  @internal The type_name used to create the TopicDescription.
     */
    const std::string& type_name() const;

    /**
     *  @internal This operation returns the DomainParticipant to which the
     * TopicDescription belongs.
     */
    const dds::domain::DomainParticipant& domain_participant() const;

    void incrNrDependents();

    void decrNrDependents();

    bool hasDependents() const;

    virtual std::string reader_expression() const = 0;

    virtual std::vector<c_value> reader_parameters() const = 0;

protected:
    dds::domain::DomainParticipant myParticipant;
    std::string myTopicName;
    std::string myTypeName;
    uint32_t nrDependents;
};

}
}
}

#endif /* ORG_OPENSPLICE_TOPIC_TOPICDESCRIPTIONDELEGATE_HPP_ */
