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
#ifndef OSPL_DDS_SUB_QOS_DETAIL_SUBSCRIBERQOS_HPP_
#define OSPL_DDS_SUB_QOS_DETAIL_SUBSCRIBERQOS_HPP_

/**
 * @file
 */

// Implementation

#include <dds/core/TEntityQos.hpp>
#include <org/opensplice/sub/qos/SubscriberQosImpl.hpp>

namespace dds
{
namespace sub
{
namespace qos
{
namespace detail
{
typedef ::dds::core::TEntityQos< ::org::opensplice::sub::qos::SubscriberQosImpl> SubscriberQos;
}
}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_QOS_DETAIL_SUBSCRIBERQOS_HPP_ */
