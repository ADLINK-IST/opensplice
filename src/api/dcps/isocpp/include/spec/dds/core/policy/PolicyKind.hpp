#ifndef OMG_DDS_CORE_POLICY_POLICYKIND_HPP_
#define OMG_DDS_CORE_POLICY_POLICYKIND_HPP_

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

#include <dds/core/detail/conformance.hpp>
#include <dds/core/SafeEnumeration.hpp>

namespace dds
{
namespace core
{
namespace policy
{

/** @todo raise spec issue **/
#if defined (__SUNPRO_CC) && defined(SHARED)
#   undef SHARED
#endif
struct OwnershipKind_def
{
    enum Type
    {
        SHARED
        #ifdef  OMG_DDS_OWNERSHIP_SUPPORT
        ,
        EXCLUSIVE
        #endif  // OMG_DDS_OWNERSHIP_SUPPORT
    };
};

typedef dds::core::safe_enum<OwnershipKind_def> OwnershipKind;

struct DurabilityKind_def
{
    enum Type
    {
        VOLATILE,
        TRANSIENT_LOCAL

        #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
        ,
        TRANSIENT,
        PERSISTENT
        #endif  // #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    };
};
typedef dds::core::safe_enum<DurabilityKind_def> DurabilityKind;

struct PresentationAccessScopeKind_def
{
    enum Type
    {
        INSTANCE,
        TOPIC

        #ifdef  OMG_DDS_OBJECT_MODEL_SUPPORT
        ,
        GROUP
        #endif  // OMG_DDS_OBJECT_MODEL_SUPPORT
    };
};
typedef dds::core::safe_enum<PresentationAccessScopeKind_def> PresentationAccessScopeKind;


struct ReliabilityKind_def
{
    enum Type
    {
        BEST_EFFORT,
        RELIABLE
    };
};
typedef dds::core::safe_enum<ReliabilityKind_def> ReliabilityKind;


struct DestinationOrderKind_def
{
    enum Type
    {
        BY_RECEPTION_TIMESTAMP,
        BY_SOURCE_TIMESTAMP
    };
};

typedef dds::core::safe_enum<DestinationOrderKind_def> DestinationOrderKind;

struct HistoryKind_def
{
    enum Type
    {
        KEEP_LAST,
        KEEP_ALL
    };
};

typedef dds::core::safe_enum<HistoryKind_def> HistoryKind;

struct LivelinessKind_def
{
    enum Type
    {
        AUTOMATIC,
        MANUAL_BY_PARTICIPANT,
        MANUAL_BY_TOPIC
    };
};
typedef dds::core::safe_enum<LivelinessKind_def> LivelinessKind;

struct TypeConsistencyEnforcementKind_def
{
    enum Type
    {
        EXACT_TYPE_TYPE_CONSISTENCY,
        EXACT_NAME_TYPE_CONSISTENCY,
        DECLARED_TYPE_CONSISTENCY,
        ASSIGNABLE_TYPE_CONSISTENCY
    };
};

typedef dds::core::safe_enum<TypeConsistencyEnforcementKind_def> TypeConsistencyEnforcementKind;

}
}
}
#endif /* OMG_DDS_CORE_POLICY_POLICYKIND_HPP_ */
