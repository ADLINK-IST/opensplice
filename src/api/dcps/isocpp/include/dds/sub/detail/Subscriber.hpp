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
#ifndef OSPL_DDS_SUB_DETAIL_SUBSCRIBER_HPP_
#define OSPL_DDS_SUB_DETAIL_SUBSCRIBER_HPP_

/**
 * @file
 */

// Implementation

#include <org/opensplice/sub/SubscriberDelegate.hpp>
#include <dds/sub/TSubscriber.hpp>

// #include <dds/core/status/Status.hpp>

namespace dds
{
namespace sub
{
namespace detail
{

typedef dds::sub::TSubscriber<org::opensplice::sub::SubscriberDelegate> Subscriber;

}
}
}
// End of implementation
#endif /* OSPL_DDS_SUB_DETAIL_SUBSCRIBER_HPP_ */
