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

#ifndef ORG_OPENSPLICE_PUB_QOS_QOSCONVERTER_HPP_
#define ORG_OPENSPLICE_PUB_QOS_QOSCONVERTER_HPP_

#include <dds/core/types.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{
dds::pub::qos::DataWriterQos
OSPL_ISOCPP_IMPL_API convertQos(const DDS::DataWriterQos& from);

DDS::DataWriterQos
OSPL_ISOCPP_IMPL_API convertQos(const dds::pub::qos::DataWriterQos& from);

dds::pub::qos::PublisherQos
OSPL_ISOCPP_IMPL_API convertQos(const DDS::PublisherQos& from);

DDS::PublisherQos
OSPL_ISOCPP_IMPL_API convertQos(const dds::pub::qos::PublisherQos& from);
}
}
}
}

#endif /* ORG_OPENSPLICE_PUB_QOS_QOSCONVERTER_HPP_ */
