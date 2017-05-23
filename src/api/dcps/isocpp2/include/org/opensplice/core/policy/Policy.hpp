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

#ifndef ORG_OPENSPLICE_CORE_POLICY_POLICY_HPP_
#define ORG_OPENSPLICE_CORE_POLICY_POLICY_HPP_


/******************************************************************************
 *
 * PROPRIETARY POLICIES
 *
 ******************************************************************************/

#include <org/opensplice/core/policy/PolicyDelegate.hpp>
#include <org/opensplice/core/policy/TPolicy.hpp>


/* Use same macro as in dds/core/policy/CorePolicy.hpp, called OMG_DDS_POLICY_TRAITS */
#define ORG_OPENSPLICE_POLICY_TRAITS(POLICY, ID) \
    template <> \
    class policy_id<POLICY> { \
    public: \
        static const dds::core::policy::QosPolicyId value = ID; \
    };\
    template <> \
    class policy_name<POLICY> { \
    public:\
        static const std::string& name(); \
    };



namespace org
{
namespace opensplice
{
namespace core
{
namespace policy
{


/*
 * Proprietary policies values
 */
typedef org::opensplice::core::policy::TListenerScheduling<org::opensplice::core::policy::SchedulingDelegate> ListenerScheduling;

typedef org::opensplice::core::policy::TProductData<org::opensplice::core::policy::ProductDataDelegate> ProductData;

typedef org::opensplice::core::policy::TReaderLifespan<org::opensplice::core::policy::ReaderLifespanDelegate> ReaderLifespan;

typedef org::opensplice::core::policy::TShare<org::opensplice::core::policy::ShareDelegate> Share;

typedef org::opensplice::core::policy::TSubscriptionKey<org::opensplice::core::policy::SubscriptionKeyDelegate> SubscriptionKey;

typedef org::opensplice::core::policy::TWatchdogScheduling<org::opensplice::core::policy::SchedulingDelegate> WatchdogScheduling;

}
}
}
}


/*
 * Proprietary policies traits (need to be in dds::core::policy namespace).
 */
namespace dds
{
namespace core
{
namespace policy
{
template <typename Policy>
class policy_id;
template <typename Policy>
class policy_name;

/* Currently, the spec ends with id 22.
 * Be sure to leave some space for the future. So start with 100. */
ORG_OPENSPLICE_POLICY_TRAITS(org::opensplice::core::policy::ListenerScheduling, 100)
ORG_OPENSPLICE_POLICY_TRAITS(org::opensplice::core::policy::ProductData,        101)
ORG_OPENSPLICE_POLICY_TRAITS(org::opensplice::core::policy::ReaderLifespan,     102)
ORG_OPENSPLICE_POLICY_TRAITS(org::opensplice::core::policy::Share,              103)
ORG_OPENSPLICE_POLICY_TRAITS(org::opensplice::core::policy::SubscriptionKey,    104)
ORG_OPENSPLICE_POLICY_TRAITS(org::opensplice::core::policy::WatchdogScheduling, 105)
}
}
}


#undef ORG_OPENSPLICE_POLICY_TRAITS

#endif /* ORG_OPENSPLICE_CORE_POLICY_POLICY_HPP_ */
