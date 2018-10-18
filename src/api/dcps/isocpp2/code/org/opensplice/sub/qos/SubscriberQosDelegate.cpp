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

#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/sub/qos/SubscriberQosDelegate.hpp>

#include "u_subscriberQos.h"

#include "dds_namedQosTypesSplType.h"

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

SubscriberQosDelegate::SubscriberQosDelegate()
{
    this->defaults();
}

SubscriberQosDelegate::SubscriberQosDelegate(
    const SubscriberQosDelegate& other)
    : presentation_(other.presentation_),
      partition_(other.partition_),
      group_data_(other.group_data_),
      entity_factory_(other.entity_factory_),
      share_(other.share_)

{
}

SubscriberQosDelegate::~SubscriberQosDelegate()
{
}

void
SubscriberQosDelegate::policy(const dds::core::policy::Presentation& presentation)
{
    presentation_ = presentation;
}

void
SubscriberQosDelegate::policy(const dds::core::policy::Partition& partition)
{
    partition.delegate().check();
    partition_ = partition;
}

void
SubscriberQosDelegate::policy(const dds::core::policy::GroupData& group_data)
{
    group_data.delegate().check();
    group_data_ = group_data;
}

void
SubscriberQosDelegate::policy(const dds::core::policy::EntityFactory& entity_factory)
{
    entity_factory.delegate().check();
    entity_factory_ = entity_factory;
}

void
SubscriberQosDelegate::policy(const org::opensplice::core::policy::Share& share)
{
    share.delegate().check();
    share_ = share;
}

u_subscriberQos
SubscriberQosDelegate::u_qos() const
{
    u_subscriberQos qos = u_subscriberQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->presentation  = presentation_   .delegate().v_policyI();
    qos->partition     = partition_      .delegate().v_policyI();
    qos->groupData     = group_data_     .delegate().v_policyI();
    qos->entityFactory = entity_factory_ .delegate().v_policyI();
    qos->share         = share_          .delegate().v_policyI();
    return qos;
}

void
SubscriberQosDelegate::u_qos(const u_subscriberQos qos)
{
    assert(qos);
    presentation_   .delegate().v_policyI(qos->presentation );
    partition_      .delegate().v_policyI(qos->partition    );
    group_data_     .delegate().v_policyI(qos->groupData    );
    entity_factory_ .delegate().v_policyI(qos->entityFactory);
    share_          .delegate().v_policyI(qos->share        );
}

void
SubscriberQosDelegate::named_qos(const struct _DDS_NamedSubscriberQos &qos)
{
    /* We only need the QoS part of the named QoS. */
    const struct _DDS_SubscriberQos *q = &qos.subscriber_qos;
    /* The idl policies are aligned the same as the kernel/builtin representation.
     * So, cast and use the kernel policy translations (or builtin when available). */
    presentation_   .delegate().v_policy((v_presentationPolicy&)    (q->presentation)  );
    partition_      .delegate().v_policy((v_builtinPartitionPolicy&)(q->partition)     );
    group_data_     .delegate().v_policy((v_builtinGroupDataPolicy&)(q->group_data)    );
    entity_factory_ .delegate().v_policy((v_entityFactoryPolicy&)   (q->entity_factory));
    share_          .delegate().v_policy((v_sharePolicy&)           (q->share)         );
}

void
SubscriberQosDelegate::check() const
{
    /* Policies are checked when set.
     * No consistency check between policies needed. */
}

bool
SubscriberQosDelegate::operator ==(const SubscriberQosDelegate& other) const
{
    return other.presentation_   == presentation_   &&
           other.partition_      == partition_      &&
           other.group_data_     == group_data_     &&
           other.entity_factory_ == entity_factory_ &&
           other.share_          == share_;
}

SubscriberQosDelegate&
SubscriberQosDelegate::operator =(const SubscriberQosDelegate& other)
{
    presentation_   = other.presentation_;
    partition_      = other.partition_;
    group_data_     = other.group_data_;
    entity_factory_ = other.entity_factory_;
    share_          = other.share_;
    return *this;
}

void
SubscriberQosDelegate::defaults()
{
    /* Get default QoS from userlayer and copy result. */
    u_subscriberQos qos = u_subscriberQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    this->u_qos(qos);
    u_subscriberQosFree(qos);
}

}
}
}
}
