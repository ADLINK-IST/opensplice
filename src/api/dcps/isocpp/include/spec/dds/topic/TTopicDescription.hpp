#ifndef OMG_DDS_T_TOPIC_TOPIC_DESCRIPTION_HPP_
#define OMG_DDS_T_TOPIC_TOPIC_DESCRIPTION_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/core/TEntity.hpp>
#include <dds/topic/TopicTraits.hpp>

namespace dds
{
namespace topic
{
template <typename T,
          template <typename Q> class DELEGATE>
class TopicDescription;
}
}


/**
 * TopicDescription represents the fact that both publications and
 * subscriptions are tied to a single data-type. Its attribute
 * type_name defines a unique resulting type for the publication
 * or the subscription and therefore creates an implicit
 * association with a TypeSupport. TopicDescription has also a
 * name that allows it to be retrieved locally.
 */
template <typename T, template <typename Q> class DELEGATE>
class dds::topic::TopicDescription : public ::dds::core::TEntity< DELEGATE<T> >
{
public:
    typedef T DataType;

public:
    OMG_DDS_REF_TYPE(TopicDescription, ::dds::core::TEntity, DELEGATE<T>)

public:
    virtual ~TopicDescription();

public:
    /**
     * Gets the Topic name.
     *
     * @return the Topic name
     */
    const std::string& name() const;

    /**
     * Gets the type_name.
     *
     * @return the type_name
     */
    const std::string& type_name() const;

    /**
     * Gets the DomainParticipant associated with the TopicDescription.
     *
     * @return the DomainParticipant
     */
    const dds::domain::DomainParticipant& domain_participant() const;

    /**
     * Creates a TopicDescription instance.
     *
     * @param dp the DomainParticipant
     * @param name the Topic name
     * @param type_name the type_name
     */
    TopicDescription(const dds::domain::DomainParticipant& dp,
                     const std::string& name,
                     const std::string& type_name = dds::topic::topic_type_name<T>::value());
};


#endif /* OMG_DDS_T_TOPIC_TOPIC_DESCRIPTION_HPP_ */
