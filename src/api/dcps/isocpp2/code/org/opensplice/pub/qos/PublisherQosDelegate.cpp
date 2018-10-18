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
#include <org/opensplice/pub/qos/PublisherQosDelegate.hpp>

#include "u_publisherQos.h"

#include "dds_namedQosTypesSplType.h"

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{

PublisherQosDelegate::PublisherQosDelegate()
{
    this->defaults();
}

PublisherQosDelegate::PublisherQosDelegate(
    const PublisherQosDelegate& other)
    : presentation_(other.presentation_),
      partition_(other.partition_),
      gdata_(other.gdata_),
      factory_policy_(other.factory_policy_)
{
}

PublisherQosDelegate::~PublisherQosDelegate()
{
}

void
PublisherQosDelegate::policy(const dds::core::policy::Presentation& presentation)
{
    presentation_ = presentation;
}

void
PublisherQosDelegate::policy(const dds::core::policy::Partition& partition)
{
    partition.delegate().check();
    partition_ = partition;
}

void
PublisherQosDelegate::policy(const dds::core::policy::GroupData& gdata)
{
    gdata.delegate().check();
    gdata_ = gdata;
}

void
PublisherQosDelegate::policy(const dds::core::policy::EntityFactory& factory_policy)
{
    factory_policy.delegate().check();
    factory_policy_ = factory_policy;
}

u_publisherQos
PublisherQosDelegate::u_qos() const
{
    u_publisherQos qos = u_publisherQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->presentation  = presentation_   .delegate().v_policyI();
    qos->partition     = partition_      .delegate().v_policyI();
    qos->groupData     = gdata_          .delegate().v_policyI();
    qos->entityFactory = factory_policy_ .delegate().v_policyI();
    return qos;
}

void
PublisherQosDelegate::u_qos(const u_publisherQos qos)
{
    assert(qos);
    presentation_   .delegate().v_policyI(qos->presentation );
    partition_      .delegate().v_policyI(qos->partition    );
    gdata_          .delegate().v_policyI(qos->groupData    );
    factory_policy_ .delegate().v_policyI(qos->entityFactory);
}

void
PublisherQosDelegate::named_qos(const struct _DDS_NamedPublisherQos &qos)
{
    /* We only need the QoS part of the named QoS. */
    const struct _DDS_PublisherQos *q = &qos.publisher_qos;
    /* The idl policies are aligned the same as the kernel/builtin representation.
     * So, cast and use the kernel policy translations (or builtin when available). */
    presentation_   .delegate().v_policy((v_presentationPolicy&)    (q->presentation)  );
    partition_      .delegate().v_policy((v_builtinPartitionPolicy&)(q->partition)     );
    gdata_          .delegate().v_policy((v_builtinGroupDataPolicy&)(q->group_data)    );
    factory_policy_ .delegate().v_policy((v_entityFactoryPolicy&)   (q->entity_factory));
}

void
PublisherQosDelegate::check() const
{
    /* Policies are checked when set.
     * No consistency check between policies needed. */
}

bool
PublisherQosDelegate::operator ==(const PublisherQosDelegate& other) const
{
    return other.presentation_   == presentation_ &&
           other.partition_      == partition_    &&
           other.gdata_          == gdata_        &&
           other.factory_policy_ == factory_policy_;
}

PublisherQosDelegate&
PublisherQosDelegate::operator =(const PublisherQosDelegate& other)
{
    presentation_   = other.presentation_;
    partition_      = other.partition_;
    gdata_          = other.gdata_;
    factory_policy_ = other.factory_policy_;
    return *this;
}

void
PublisherQosDelegate::defaults()
{
    /* Get default QoS from userlayer and copy result. */
    u_publisherQos qos = u_publisherQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    this->u_qos(qos);
    u_publisherQosFree(qos);
}

}
}
}
}
