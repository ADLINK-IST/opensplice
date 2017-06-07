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

#include <dds/core/policy/CorePolicy.hpp>

#include <org/opensplice/core/policy/PolicyDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
#include "c_stringSupport.h"
#include "v_policy.h"
#include "dds_namedQosTypesSplType.h"
#include "os_abstract.h"

/*
 * Proprietary policies traits.
 */
OMG_DDS_DEFINE_POLICY_TRAITS(org::opensplice::core::policy::ListenerScheduling, "ListenerScheduling")
OMG_DDS_DEFINE_POLICY_TRAITS(org::opensplice::core::policy::ProductData,        "ProductData")
OMG_DDS_DEFINE_POLICY_TRAITS(org::opensplice::core::policy::ReaderLifespan,     "ReaderLifespan")
OMG_DDS_DEFINE_POLICY_TRAITS(org::opensplice::core::policy::Share,              "Share")
OMG_DDS_DEFINE_POLICY_TRAITS(org::opensplice::core::policy::SubscriptionKey,    "SubscriptionKey")
OMG_DDS_DEFINE_POLICY_TRAITS(org::opensplice::core::policy::WatchdogScheduling, "WatchdogScheduling")


namespace org
{
namespace opensplice
{
namespace core
{
namespace policy
{


namespace helper
{

/*
 * private
 */
void
convertByteSeq(
        const dds::core::ByteSeq &from,
        c_array &to,
        c_long  &size)
{
    uint8_t *byteArray;

    if (to != NULL) {
        os_free(to);
        to = NULL;
    }
    size = from.size();

    if (size > 0) {
        byteArray = static_cast<uint8_t*>(os_malloc(size));
        if (!byteArray) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal byte sequence.");
        }
        for(c_long i = 0; i < size; i++)
        {
            byteArray[i] = from[i];
        }
        to = (c_array)byteArray;
    }
}

/* For c_array and c_sequence. */
void
convertByteSeq(
        const c_array   from,
        const c_long    size,
        dds::core::ByteSeq  &to)
{
    const uint8_t *byteArray = reinterpret_cast<const uint8_t*>(from);
    to.clear();
    to.insert(to.end(), byteArray, byteArray + size);
}


void
convertStringSeq(
        const dds::core::StringSeq &from,
        char *&to,
        const char *delimiter)
{
    unsigned long size = 0;
    unsigned long i;

    assert(delimiter);

    /* Add all string lengths to get total size. */
    for (i = 0; i < from.size(); i++) {
        size += from[i].length();
    }

    if (size > 0) {
        size += (from.size() * strlen(delimiter)) + 1;
        if (to != NULL) {
            os_free(to);
        }
        to = (char*) os_malloc(size);
        if (!to) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal string sequence.");
        }

        to[0] = '\0';
        for (i = 0; i < from.size(); i++) {
            if (i != 0) {
                os_strcat(to, delimiter);
            }
            os_strcat(to, from[i].c_str());
        }
    } else {
        if (to != NULL) {
            os_free(to);
        }
        to = (char*) os_malloc(1);
        to[0] = '\0';
    }
}

void
convertStringSeq(
        const char *from,
        dds::core::StringSeq &to,
        const char *delimiter)
{
    c_iter iter;
    c_long size;
    c_long i;
    char* str;

    assert(delimiter);

    to.clear();

    if (from != NULL) {
        iter = c_splitString(from, delimiter);
        if (iter) {
            size = c_iterLength(iter);
            for ( i = 0UL; i < size; i++ ) {
                str = (char*)c_iterTakeFirst(iter);
                /* Conversion to std::string is automatic copy. */
                to.push_back(str);
                os_free(str);
            }
            c_iterFree(iter);
        }
    }
}

void
convertStringSeq(
        const c_sequence &from,
        dds::core::StringSeq &to)
{
    int32_t size = c_arraySize((c_array)from);

    to.clear();
    to.reserve(size);
    for (int32_t i = 0; i < size; i++) {
        to.push_back((const char *)from[i] ? (const char *)from[i] : "");
    }
}

static dds::core::Duration
convertDuration(
        const os_duration &from)
{
    if (OS_DURATION_ISINFINITE(from)) {
        return dds::core::Duration::infinite();
    }
    return dds::core::Duration(from/OS_DURATION_SECOND, from%OS_DURATION_SECOND);
}

static dds::core::Duration
convertDuration(
        const v_duration &from)
{
    return dds::core::Duration(from.seconds, from.nanoseconds);
}

static os_duration
convertDuration(
        const dds::core::Duration &from)
{
    os_duration to = OS_DURATION_INVALID;

    if (from == dds::core::Duration::infinite()) {
        to = OS_DURATION_INFINITE;
    } else if ((from.sec() >= 0) && (from.sec() <= OS_TIME_INFINITE_SEC)) {
        to = OS_DURATION_INIT(from.sec(), from.nanosec());
    } else {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
                               "Specified duration is negative or to large: (%" PA_PRId64 ".%09u)",
                               from.sec(), from.nanosec());
    }
    return to;
}

static v_duration
convertToVDuration(
        const dds::core::Duration &from)
{
    v_duration to = V_DURATION_INVALID;

    if (from == dds::core::Duration::infinite()) {
        to = V_DURATION_INFINITE;
    } else if ((from.sec() >= 0) && (from.sec() <= OS_TIME_INFINITE_SEC)) {
        to.seconds = (c_long)(from.sec());
        to.nanoseconds = (c_ulong)(from.nanosec());
    } else {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
                               "Specified duration is negative or to large: (%" PA_PRId64 ".%09u)",
                               from.sec(), from.nanosec());
    }
    return to;
}

} /* namespace helper */



//==============================================================================

DeadlineDelegate::DeadlineDelegate(const DeadlineDelegate& other)
    : period_(other.period_)
{
    this->check();
}

DeadlineDelegate::DeadlineDelegate(const dds::core::Duration& d)
    : period_(d)
{
}

void DeadlineDelegate::period(const dds::core::Duration& d)
{
    period_ = d;
}

dds::core::Duration DeadlineDelegate::period() const
{
    return period_;
}

bool DeadlineDelegate::operator ==(const DeadlineDelegate& other) const
{
    return other.period() == period_;
}

void DeadlineDelegate::check() const
{
    /* A Duration object is always valid:
     *      The period duration is always valid:
     *          Nothing to check. */
}

void DeadlineDelegate::check_against(const org::opensplice::core::policy::TimeBasedFilterDelegate& filter) const
{
    filter.check_against(*this);
}

void DeadlineDelegate::v_policy(const v_deadlinePolicy& policy)
{
    period_ = helper::convertDuration(policy.period);
}

void DeadlineDelegate::v_policyI(const v_deadlinePolicyI& policy)
{
    period_ = helper::convertDuration(policy.v.period);
}

v_deadlinePolicy DeadlineDelegate::v_policy() const
{
    v_deadlinePolicy policy;
    policy.period = helper::convertToVDuration(period_);
    return policy;
}

v_deadlinePolicyI DeadlineDelegate::v_policyI() const
{
    v_deadlinePolicyI policy;
    policy.v.period = helper::convertDuration(period_);
    return policy;
}


//==============================================================================

DestinationOrderDelegate::DestinationOrderDelegate(const DestinationOrderDelegate& other)
    : kind_(other.kind_)
{
}

DestinationOrderDelegate::DestinationOrderDelegate(dds::core::policy::DestinationOrderKind::Type kind)
    : kind_(kind)
{
    this->check();
}

void DestinationOrderDelegate::kind(dds::core::policy::DestinationOrderKind::Type kind)
{
    kind_ = kind;
}

dds::core::policy::DestinationOrderKind::Type DestinationOrderDelegate::kind() const
{
    return kind_;
}

bool DestinationOrderDelegate::operator ==(const DestinationOrderDelegate& other) const
{
    return other.kind() == kind_;
}

void DestinationOrderDelegate::check() const
{
    /* The kind correctness is enforced by the compiler: nothing to check. */
}

void DestinationOrderDelegate::v_policy(const v_orderbyPolicy& policy)
{
    switch(policy.kind)
    {
    case V_ORDERBY_RECEPTIONTIME:
        kind_ = dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP;
        break;
    case V_ORDERBY_SOURCETIME:
        kind_ = dds::core::policy::DestinationOrderKind::BY_SOURCE_TIMESTAMP;
        break;
    default:
        assert(0);
        break;
    }
}

void DestinationOrderDelegate::v_policyI(const v_orderbyPolicyI& policy)
{
    v_policy (policy.v);
}

v_orderbyPolicy DestinationOrderDelegate::v_policy() const
{
    v_orderbyPolicy policy;
    switch(kind_)
    {
    case dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP:
        policy.kind = V_ORDERBY_RECEPTIONTIME;
        break;
    case dds::core::policy::DestinationOrderKind::BY_SOURCE_TIMESTAMP:
        policy.kind = V_ORDERBY_SOURCETIME;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.kind = V_ORDERBY_RECEPTIONTIME;
        break;
    }
    return policy;
}

v_orderbyPolicyI DestinationOrderDelegate::v_policyI() const
{
    v_orderbyPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

DurabilityDelegate::DurabilityDelegate(const DurabilityDelegate& other)
    : kind_(other.kind_)
{
}

DurabilityDelegate::DurabilityDelegate(dds::core::policy::DurabilityKind::Type kind)
    : kind_(kind)
{
    this->check();
}

void DurabilityDelegate::kind(dds::core::policy::DurabilityKind::Type kind)
{
    kind_ = kind;
}

dds::core::policy::DurabilityKind::Type DurabilityDelegate::kind() const
{
    return kind_;
}

bool DurabilityDelegate::operator ==(const DurabilityDelegate& other) const
{
    return other.kind() == kind_;
}

void DurabilityDelegate::check() const
{
    /* The kind correctness is enforced by the compiler: nothing to check. */
}

void DurabilityDelegate::v_policy(const v_durabilityPolicy& policy)
{
    switch(policy.kind)
    {
    case V_DURABILITY_VOLATILE:
        kind_ = dds::core::policy::DurabilityKind::VOLATILE;
        break;
    case V_DURABILITY_TRANSIENT_LOCAL:
        kind_ = dds::core::policy::DurabilityKind::TRANSIENT_LOCAL;
        break;
    case V_DURABILITY_TRANSIENT:
        kind_ = dds::core::policy::DurabilityKind::TRANSIENT;
        break;
    case V_DURABILITY_PERSISTENT:
        kind_ = dds::core::policy::DurabilityKind::PERSISTENT;
        break;
    default:
        assert(0);
        break;
    }
}

void DurabilityDelegate::v_policyI(const v_durabilityPolicyI& policy)
{
    v_policy (policy.v);
}
    
v_durabilityPolicy DurabilityDelegate::v_policy() const
{
    v_durabilityPolicy policy;
    switch(kind_)
    {
    case dds::core::policy::DurabilityKind::VOLATILE:
        policy.kind = V_DURABILITY_VOLATILE;
        break;
    case dds::core::policy::DurabilityKind::TRANSIENT_LOCAL:
        policy.kind = V_DURABILITY_TRANSIENT_LOCAL;
        break;
    case dds::core::policy::DurabilityKind::TRANSIENT:
        policy.kind = V_DURABILITY_TRANSIENT;
        break;
    case dds::core::policy::DurabilityKind::PERSISTENT:
        policy.kind = V_DURABILITY_PERSISTENT;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.kind = V_DURABILITY_VOLATILE;
        break;
    }
    return policy;
}

v_durabilityPolicyI DurabilityDelegate::v_policyI() const
{
    v_durabilityPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

DurabilityServiceDelegate::DurabilityServiceDelegate(const DurabilityServiceDelegate& other)
    : cleanup_delay_(other.cleanup_delay_),
      history_kind_(other.history_kind_),
      history_depth_(other.history_depth_),
      max_samples_(other.max_samples_),
      max_instances_(other.max_instances_),
      max_samples_per_instance_(other.max_samples_per_instance_)
{
}

DurabilityServiceDelegate::DurabilityServiceDelegate(const dds::core::Duration& service_cleanup_delay,
                                                     dds::core::policy::HistoryKind::Type history_kind,
                                                     int32_t history_depth,
                                                     int32_t max_samples,
                                                     int32_t max_instances,
                                                     int32_t max_samples_per_instance)
    : cleanup_delay_(service_cleanup_delay),
      history_kind_(history_kind),
      history_depth_(history_depth),
      max_samples_(max_samples),
      max_instances_(max_instances),
      max_samples_per_instance_(max_samples_per_instance)
{
    this->check();
}

void DurabilityServiceDelegate::service_cleanup_delay(const dds::core::Duration& d)
{
    cleanup_delay_ = d;
}

const dds::core::Duration DurabilityServiceDelegate::service_cleanup_delay() const
{
    return cleanup_delay_;
}

void DurabilityServiceDelegate::history_kind(dds::core::policy::HistoryKind::Type kind)
{
    history_kind_ = kind;
}

dds::core::policy::HistoryKind::Type DurabilityServiceDelegate::history_kind() const
{
    return history_kind_;
}

void DurabilityServiceDelegate::history_depth(int32_t depth)
{
    history_depth_ = depth;
}

int32_t DurabilityServiceDelegate::history_depth() const
{
    return history_depth_;
}

void DurabilityServiceDelegate::max_samples(int32_t max_samples)
{
    max_samples_ = max_samples;
}

int32_t DurabilityServiceDelegate::max_samples() const
{
    return max_samples_;
}

void DurabilityServiceDelegate::max_instances(int32_t max_instances)
{
    max_instances_ = max_instances;
}

int32_t DurabilityServiceDelegate::max_instances() const
{
    return max_instances_;
}

void DurabilityServiceDelegate::max_samples_per_instance(int32_t max_samples_per_instance)
{
    max_samples_per_instance_ = max_samples_per_instance;
}

int32_t DurabilityServiceDelegate::max_samples_per_instance() const
{
    return max_samples_per_instance_;
}

bool DurabilityServiceDelegate::operator ==(const DurabilityServiceDelegate& other) const
{
    return other.service_cleanup_delay() == cleanup_delay_ &&
           other.history_kind() == history_kind_ &&
           other.history_depth() == history_depth_ &&
           other.max_samples() == max_samples_ &&
           other.max_instances() == max_instances_ &&
           other.max_samples_per_instance() == max_samples_per_instance_;
}

void DurabilityServiceDelegate::check() const
{
    /* The kind correctness is enforced by the compiler. */
    /* A Duration object is always valid: the service_cleanup_delay is valid. */

    if ((max_samples_ < 0) && (max_samples_ != dds::core::LENGTH_UNLIMITED)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR, "Invalid DurabilityService::max_samples (%ld) value.", max_samples_);
    }
    if ((max_instances_ < 0) && (max_instances_ != dds::core::LENGTH_UNLIMITED)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR, "Invalid DurabilityService::max_instances (%ld) value.", max_instances_);
    }
    if ((max_samples_per_instance_ < 0) && (max_samples_per_instance_ != dds::core::LENGTH_UNLIMITED)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR, "Invalid DurabilityService::max_samples_per_instance (%ld) value.", max_samples_per_instance_);
    }

    if ((history_kind_ == dds::core::policy::HistoryKind::KEEP_LAST) &&
        (history_depth_ <= 0)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INCONSISTENT_POLICY_ERROR, "DurabilityService: history_depth (%ld) not consistent with KEEP_LAST", history_depth_);
    }

    if ((max_samples_per_instance_ != dds::core::LENGTH_UNLIMITED) &&
        (history_depth_ > max_samples_per_instance_)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INCONSISTENT_POLICY_ERROR, "DurabilityService: history_depth (%ld) not consistent with max_samples_per_instance (%ld)", history_depth_, max_samples_per_instance_);
    }
}

void DurabilityServiceDelegate::v_policy(const v_durabilityServicePolicy& policy)
{
    cleanup_delay_ = helper::convertDuration(policy.service_cleanup_delay);
    switch (policy.history_kind)
    {
    case V_HISTORY_KEEPLAST:
        history_kind_ = dds::core::policy::HistoryKind::KEEP_LAST;
        break;
    case V_HISTORY_KEEPALL:
        history_kind_ = dds::core::policy::HistoryKind::KEEP_ALL;
        break;
    default:
        assert(0);
        break;
    }
    history_depth_ = policy.history_depth;
    max_samples_ = policy.max_samples;
    max_instances_ = policy.max_instances;
    max_samples_per_instance_ = policy.max_samples_per_instance;
}


void DurabilityServiceDelegate::v_policyI(const v_durabilityServicePolicyI& policy)
{
    cleanup_delay_ = helper::convertDuration(policy.v.service_cleanup_delay);
    switch (policy.v.history_kind)
    {
    case V_HISTORY_KEEPLAST:
        history_kind_ = dds::core::policy::HistoryKind::KEEP_LAST;
        break;
    case V_HISTORY_KEEPALL:
        history_kind_ = dds::core::policy::HistoryKind::KEEP_ALL;
        break;
    default:
        assert(0);
        break;
    }
    history_depth_ = policy.v.history_depth;
    max_samples_ = policy.v.max_samples;
    max_instances_ = policy.v.max_instances;
    max_samples_per_instance_ = policy.v.max_samples_per_instance;
}

v_durabilityServicePolicy DurabilityServiceDelegate::v_policy() const
{
    v_durabilityServicePolicy policy;
    policy.service_cleanup_delay = helper::convertToVDuration(cleanup_delay_);
    switch(history_kind_)
    {
    case dds::core::policy::HistoryKind::KEEP_LAST:
        policy.history_kind = V_HISTORY_KEEPLAST;
        break;
    case dds::core::policy::HistoryKind::KEEP_ALL:
        policy.history_kind = V_HISTORY_KEEPALL;
        break;
    default:
        assert(0);
        break;
    }
    policy.history_depth = history_depth_;
    policy.max_samples = max_samples_;
    policy.max_instances = max_instances_;
    policy.max_samples_per_instance = max_samples_per_instance_;
    return policy;
}

v_durabilityServicePolicyI DurabilityServiceDelegate::v_policyI() const
{
    v_durabilityServicePolicyI policy;
    policy.v.service_cleanup_delay = (helper::convertDuration(cleanup_delay_));
    switch(history_kind_)
    {
    case dds::core::policy::HistoryKind::KEEP_LAST:
        policy.v.history_kind = V_HISTORY_KEEPLAST;
        break;
    case dds::core::policy::HistoryKind::KEEP_ALL:
        policy.v.history_kind = V_HISTORY_KEEPALL;
        break;
    default:
        assert(0);
        break;
    }
    policy.v.history_depth = history_depth_;
    policy.v.max_samples = max_samples_;
    policy.v.max_instances = max_instances_;
    policy.v.max_samples_per_instance = max_samples_per_instance_;
    return policy;
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


//==============================================================================

EntityFactoryDelegate::EntityFactoryDelegate(const EntityFactoryDelegate& other)
    : auto_enable_(other.auto_enable_)
{
}

EntityFactoryDelegate::EntityFactoryDelegate(bool auto_enable)
    : auto_enable_(auto_enable)
{
    this->check();
}

void EntityFactoryDelegate::auto_enable(bool on)
{
    auto_enable_ = on;
}

bool EntityFactoryDelegate::auto_enable() const
{
    return auto_enable_;
}

bool EntityFactoryDelegate::operator ==(const EntityFactoryDelegate& other) const
{
    return other.auto_enable() == auto_enable_;
}

void EntityFactoryDelegate::check() const
{
    /* The auto_enable_ is just a boolean: nothing to check. */
}

void EntityFactoryDelegate::v_policy(const v_entityFactoryPolicy& policy)
{
    auto_enable_ = policy.autoenable_created_entities;
}

void EntityFactoryDelegate::v_policyI(const v_entityFactoryPolicyI& policy)
{
    v_policy (policy.v);
}

v_entityFactoryPolicy EntityFactoryDelegate::v_policy() const
{
    v_entityFactoryPolicy policy;
    policy.autoenable_created_entities = auto_enable_;
    return policy;
}

v_entityFactoryPolicyI EntityFactoryDelegate::v_policyI() const
{
    v_entityFactoryPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

GroupDataDelegate::GroupDataDelegate()
    : value_()
{
}

GroupDataDelegate::GroupDataDelegate(const GroupDataDelegate& other)
    : value_(other.value_)
{
}

GroupDataDelegate::GroupDataDelegate(const dds::core::ByteSeq& seq)
    : value_(seq)
{
    this->check();
}

void GroupDataDelegate::value(const dds::core::ByteSeq& seq)
{
    value_ = seq;
}

const dds::core::ByteSeq& GroupDataDelegate::value() const
{
    return value_;
}

bool GroupDataDelegate::operator ==(const GroupDataDelegate& other) const
{
    return other.value() == value_;
}

void GroupDataDelegate::check() const
{
    /* The value_ is just a sequence: nothing to check. */
}

void GroupDataDelegate::v_policyI(const v_groupDataPolicyI& policy)
{
    helper::convertByteSeq(policy.v.value, policy.v.size, value_);
}

void GroupDataDelegate::v_policy(const v_builtinGroupDataPolicy& policy)
{
    helper::convertByteSeq(policy.value, c_arraySize(policy.value), value_);
}

v_groupDataPolicyI GroupDataDelegate::v_policyI() const
{
    v_groupDataPolicyI policy = { 0 };
    helper::convertByteSeq(value_, policy.v.value, policy.v.size);
    return policy;
}

//==============================================================================

HistoryDelegate::HistoryDelegate(const HistoryDelegate& other)
    :  kind_(other.kind_),
       depth_(other.depth_)
{
}

HistoryDelegate::HistoryDelegate(dds::core::policy::HistoryKind::Type kind, int32_t depth)
    :  kind_(kind),
       depth_(depth)
{
    this->check();
}

dds::core::policy::HistoryKind::Type HistoryDelegate::kind() const
{
    return kind_;
}

void HistoryDelegate::kind(dds::core::policy::HistoryKind::Type kind)
{
    kind_ = kind;
}

int32_t HistoryDelegate::depth() const
{
    return depth_;
}

void HistoryDelegate::depth(int32_t depth)
{
    depth_ = depth;
}

bool HistoryDelegate::operator ==(const HistoryDelegate& other) const
{
    return other.kind() == kind_ &&
           other.depth() == depth_;
}

void HistoryDelegate::check() const
{
    /* The kind correctness is enforced by the compiler. */

    if ((kind_ == dds::core::policy::HistoryKind::KEEP_LAST) &&
        (depth_ <= 0)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INCONSISTENT_POLICY_ERROR, "History::depth (%ld) not consistent with KEEP_LAST", depth_);
    }
}

void HistoryDelegate::check_against(const org::opensplice::core::policy::ResourceLimitsDelegate& limits) const
{
    limits.check_against(*this);
}

void HistoryDelegate::v_policy(const v_historyPolicy& policy)
{
    switch (policy.kind) {
    case V_HISTORY_KEEPLAST:
        kind_= dds::core::policy::HistoryKind::KEEP_LAST;
        break;
    case V_HISTORY_KEEPALL:
        kind_ = dds::core::policy::HistoryKind::KEEP_ALL;
        break;
    default:
        assert(0);
        break;
    }
    depth_ = policy.depth;
}

void HistoryDelegate::v_policyI(const v_historyPolicyI& policy)
{
    v_policy (policy.v);
}

v_historyPolicy HistoryDelegate::v_policy() const
{
    v_historyPolicy policy;
    switch(kind_)
    {
    case dds::core::policy::HistoryKind::KEEP_LAST:
        policy.kind = V_HISTORY_KEEPLAST;
        break;
    case dds::core::policy::HistoryKind::KEEP_ALL:
        policy.kind = V_HISTORY_KEEPALL;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.kind = V_HISTORY_KEEPLAST;
        break;
    }
    policy.depth = depth_;
    return policy;
}

v_historyPolicyI HistoryDelegate::v_policyI() const
{
    v_historyPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

LatencyBudgetDelegate::LatencyBudgetDelegate(const LatencyBudgetDelegate& other)
    : duration_(other.duration_)
{
}

LatencyBudgetDelegate::LatencyBudgetDelegate(const dds::core::Duration& d)
    : duration_(d)
{
    this->check();
}

void LatencyBudgetDelegate::duration(const dds::core::Duration& d)
{
    duration_ = d;
}

const dds::core::Duration LatencyBudgetDelegate::duration() const
{
    return duration_;
}

bool LatencyBudgetDelegate::operator ==(const LatencyBudgetDelegate& other) const
{
    return other.duration() == duration_;
}

void LatencyBudgetDelegate::check() const
{
    /* A Duration object is always valid:
     *      The duration_ is always valid:
     *          Nothing to check. */
}

void LatencyBudgetDelegate::v_policy(const v_latencyPolicy& policy)
{
    duration_ = helper::convertDuration(policy.duration);
}

void LatencyBudgetDelegate::v_policyI(const v_latencyPolicyI& policy)
{
    duration_ = helper::convertDuration(policy.v.duration);
}

v_latencyPolicy LatencyBudgetDelegate::v_policy() const
{
    v_latencyPolicy policy;
    policy.duration = helper::convertToVDuration(duration_);
    return policy;
}

v_latencyPolicyI LatencyBudgetDelegate::v_policyI() const
{
    v_latencyPolicyI policy;
    policy.v.duration = helper::convertDuration(duration_);
    return policy;
}


//==============================================================================

LifespanDelegate::LifespanDelegate(const LifespanDelegate& other)
    : duration_(other.duration_)
{
}

LifespanDelegate::LifespanDelegate(const dds::core::Duration& d)
    : duration_(d)
{
    this->check();
}

void LifespanDelegate::duration(const dds::core::Duration& d)
{
    duration_ = d;
}

const dds::core::Duration LifespanDelegate::duration() const
{
    return duration_;
}

bool LifespanDelegate::operator ==(const LifespanDelegate& other) const
{
    return other.duration() == duration_;
}

void LifespanDelegate::check() const
{
    /* A Duration object is always valid:
     *      The duration_ is always valid:
     *          Nothing to check. */
}

void LifespanDelegate::v_policy(const v_lifespanPolicy& policy)
{
    duration_ = helper::convertDuration(policy.duration);
}

void LifespanDelegate::v_policyI(const v_lifespanPolicyI& policy)
{
    duration_ = helper::convertDuration(policy.v.duration);
}

v_lifespanPolicy LifespanDelegate::v_policy() const
{
    v_lifespanPolicy policy;
    policy.duration = helper::convertToVDuration(duration_);
    return policy;
}

v_lifespanPolicyI LifespanDelegate::v_policyI() const
{
    v_lifespanPolicyI policy;
    policy.v.duration = helper::convertDuration(duration_);
    return policy;
}

//==============================================================================

LivelinessDelegate::LivelinessDelegate(const LivelinessDelegate& other)
    : kind_(other.kind_),
      lease_duration_(other.lease_duration_)
{
}

LivelinessDelegate::LivelinessDelegate(dds::core::policy::LivelinessKind::Type kind,
                                       dds::core::Duration lease_duration)
    : kind_(kind),
      lease_duration_(lease_duration)
{
    this->check();
}

void LivelinessDelegate::kind(dds::core::policy::LivelinessKind::Type kind)
{
    kind_ = kind;
}

dds::core::policy::LivelinessKind::Type LivelinessDelegate::kind() const
{
    return kind_;
}

void LivelinessDelegate::lease_duration(const dds::core::Duration& lease_duration)
{
    lease_duration_ = lease_duration;
}

const dds::core::Duration LivelinessDelegate::lease_duration() const
{
    return lease_duration_;
}

bool LivelinessDelegate::operator ==(const LivelinessDelegate& other) const
{
    return other.kind() == kind_ &&
           other.lease_duration() == lease_duration_;
}

void LivelinessDelegate::check() const
{
    /* The kind correctness is enforced by the compiler. */

    /* A Duration object is always valid:
     *      The lease_duration_ is always valid:
     *          Nothing to check. */
}

void LivelinessDelegate::v_policy(const v_livelinessPolicy& policy)
{
    switch (policy.kind) {
    case V_LIVELINESS_AUTOMATIC:
        kind_= dds::core::policy::LivelinessKind::AUTOMATIC;
        break;
    case V_LIVELINESS_PARTICIPANT:
        kind_= dds::core::policy::LivelinessKind::MANUAL_BY_PARTICIPANT;
        break;
    case V_LIVELINESS_TOPIC:
        kind_ = dds::core::policy::LivelinessKind::MANUAL_BY_TOPIC;
        break;
    default:
        assert(0);
        break;
    }
    lease_duration_ = helper::convertDuration(policy.lease_duration);
}

void LivelinessDelegate::v_policyI(const v_livelinessPolicyI& policy)
{
    switch (policy.v.kind) {
    case V_LIVELINESS_AUTOMATIC:
        kind_= dds::core::policy::LivelinessKind::AUTOMATIC;
        break;
    case V_LIVELINESS_PARTICIPANT:
        kind_= dds::core::policy::LivelinessKind::MANUAL_BY_PARTICIPANT;
        break;
    case V_LIVELINESS_TOPIC:
        kind_ = dds::core::policy::LivelinessKind::MANUAL_BY_TOPIC;
        break;
    default:
        assert(0);
        break;
    }
    lease_duration_ = helper::convertDuration(policy.v.lease_duration);
}

v_livelinessPolicy LivelinessDelegate::v_policy() const
{
    v_livelinessPolicy policy;
    switch (kind_) {
    case dds::core::policy::LivelinessKind::AUTOMATIC:
        policy.kind = V_LIVELINESS_AUTOMATIC;
        break;
    case dds::core::policy::LivelinessKind::MANUAL_BY_PARTICIPANT:
        policy.kind = V_LIVELINESS_PARTICIPANT;
        break;
    case dds::core::policy::LivelinessKind::MANUAL_BY_TOPIC:
        policy.kind = V_LIVELINESS_TOPIC;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.kind = V_LIVELINESS_AUTOMATIC;
        break;
    }
    policy.lease_duration = helper::convertToVDuration(lease_duration_);
    return policy;
}

v_livelinessPolicyI LivelinessDelegate::v_policyI() const
{
    v_livelinessPolicyI policy;
    switch (kind_) {
    case dds::core::policy::LivelinessKind::AUTOMATIC:
        policy.v.kind = V_LIVELINESS_AUTOMATIC;
        break;
    case dds::core::policy::LivelinessKind::MANUAL_BY_PARTICIPANT:
        policy.v.kind = V_LIVELINESS_PARTICIPANT;
        break;
    case dds::core::policy::LivelinessKind::MANUAL_BY_TOPIC:
        policy.v.kind = V_LIVELINESS_TOPIC;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.v.kind = V_LIVELINESS_AUTOMATIC;
        break;
    }
    policy.v.lease_duration = helper::convertDuration(lease_duration_);
    return policy;
}
    
//==============================================================================

OwnershipDelegate::OwnershipDelegate(const OwnershipDelegate& other)
    : kind_(other.kind_)
{
}

OwnershipDelegate::OwnershipDelegate(dds::core::policy::OwnershipKind::Type kind)
    : kind_(kind)
{
    this->check();
}

void OwnershipDelegate::kind(dds::core::policy::OwnershipKind::Type kind)
{
    kind_ = kind;
}

dds::core::policy::OwnershipKind::Type OwnershipDelegate::kind() const
{
    return kind_;
}

bool OwnershipDelegate::operator ==(const OwnershipDelegate& other) const
{
    return other.kind() == kind_;
}

void OwnershipDelegate::check() const
{
    /* The kind correctness is enforced by the compiler: nothing to check. */
}

void OwnershipDelegate::v_policy(const v_ownershipPolicy& policy)
{
    switch (policy.kind) {
    case V_OWNERSHIP_SHARED:
        kind_ = dds::core::policy::OwnershipKind::SHARED;
        break;
#ifdef OMG_DDS_OWNERSHIP_SUPPORT
    case V_OWNERSHIP_EXCLUSIVE:
        kind_ = dds::core::policy::OwnershipKind::EXCLUSIVE;
        break;
#endif // OMG_DDS_OWNERSHIP_SUPPORT
    default:
        assert(0);
        break;
    }
}

void OwnershipDelegate::v_policyI(const v_ownershipPolicyI& policy)
{
    v_policy (policy.v);
}

v_ownershipPolicy OwnershipDelegate::v_policy() const
{
    v_ownershipPolicy policy;
    switch (kind_) {
    case dds::core::policy::OwnershipKind::SHARED:
        policy.kind = V_OWNERSHIP_SHARED;
        break;
#ifdef OMG_DDS_OWNERSHIP_SUPPORT
    case dds::core::policy::OwnershipKind::EXCLUSIVE:
        policy.kind = V_OWNERSHIP_EXCLUSIVE;
        break;
#endif // OMG_DDS_OWNERSHIP_SUPPORT
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.kind = V_OWNERSHIP_SHARED;
        break;
    }
    return policy;
}

v_ownershipPolicyI OwnershipDelegate::v_policyI() const
{
    v_ownershipPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT

OwnershipStrengthDelegate::OwnershipStrengthDelegate(const OwnershipStrengthDelegate& other)
    : strength_(other.strength_)
{
}

OwnershipStrengthDelegate::OwnershipStrengthDelegate(int32_t s)
    : strength_(s)
{
    this->check();
}

int32_t OwnershipStrengthDelegate::strength() const
{
    return strength_;
}

void OwnershipStrengthDelegate::strength(int32_t s)
{
    strength_ = s;
}

bool OwnershipStrengthDelegate::operator ==(const OwnershipStrengthDelegate& other) const
{
    return other.strength() == strength_;
}

void OwnershipStrengthDelegate::check() const
{
    /* The strength is just a int32_t: nothing to check. */
}

void OwnershipStrengthDelegate::v_policy(const v_strengthPolicy& policy)
{
    strength_ = policy.value;
}

void OwnershipStrengthDelegate::v_policyI(const v_strengthPolicyI& policy)
{
    v_policy (policy.v);
}

v_strengthPolicy OwnershipStrengthDelegate::v_policy() const
{
    v_strengthPolicy policy;
    policy.value = strength_;
    return policy;
}

v_strengthPolicyI OwnershipStrengthDelegate::v_policyI() const
{
    v_strengthPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

#endif  // OMG_DDS_OWNERSHIP_SUPPORT


//==============================================================================

PartitionDelegate::PartitionDelegate(const PartitionDelegate& other)
    : name_(other.name_)
{
}

PartitionDelegate::PartitionDelegate(const std::string& partition)
    : name_()
{
    name_.push_back(partition);
    this->check();
}

PartitionDelegate::PartitionDelegate(const dds::core::StringSeq& partitions)
    : name_(partitions)
{
    this->check();
}

void PartitionDelegate::name(const std::string& partition)
{
    name_.clear();
    name_.push_back(partition);
}

void PartitionDelegate::name(const dds::core::StringSeq& partitions)
{
    name_ = partitions;
}

const dds::core::StringSeq& PartitionDelegate::name() const
{
    return name_;
}

bool PartitionDelegate::operator ==(const PartitionDelegate& other) const
{
    return other.name() == name_;
}

void PartitionDelegate::check() const
{
    /* The name_ is just a sequence: nothing to check. */
}

void PartitionDelegate::v_policyI(const v_partitionPolicyI& policy)
{
    /* typedef char* c_string;
     * typedef c_string v_partitionPolicyI;
     * So, this policy doesn't have elements.
     * So, copy value directly from the 'policy' argument. */
    helper::convertStringSeq(policy.v, name_, ",");
}

void PartitionDelegate::v_policy(const v_builtinPartitionPolicy& policy)
{
    helper::convertStringSeq(policy.name, name_);
}

v_partitionPolicyI PartitionDelegate::v_policyI() const
{
    v_partitionPolicyI policy = { NULL };
    /* typedef char* c_string;
     * typedef c_string v_partitionPolicyI;
     * So, this policy doesn't have elements.
     * So, copy value directly into the 'policy' variable. */
    helper::convertStringSeq(name_, policy.v, ",");
    return policy;
}


//==============================================================================

PresentationDelegate::PresentationDelegate(const PresentationDelegate& other)
    : access_scope_(other.access_scope_),
      coherent_access_(other.coherent_access_),
      ordered_access_(other.ordered_access_)
{
}

PresentationDelegate::PresentationDelegate(dds::core::policy::PresentationAccessScopeKind::Type access_scope,
                                           bool coherent_access,
                                           bool ordered_access)
    : access_scope_(access_scope),
      coherent_access_(coherent_access),
      ordered_access_(ordered_access)
{
}

void PresentationDelegate::access_scope(dds::core::policy::PresentationAccessScopeKind::Type as)
{
    access_scope_ = as;
}

dds::core::policy::PresentationAccessScopeKind::Type PresentationDelegate::access_scope() const
{
    return access_scope_;
}

void PresentationDelegate::coherent_access(bool on)
{
    coherent_access_ = on;
}

bool PresentationDelegate::coherent_access() const
{
    return coherent_access_;
}

void PresentationDelegate::ordered_access(bool on)
{
    ordered_access_ = on;
}

bool PresentationDelegate::ordered_access() const
{
    return ordered_access_;
}

bool PresentationDelegate::operator ==(const PresentationDelegate& other) const
{
    return other.access_scope() == access_scope_ &&
           other.coherent_access() == coherent_access_ &&
           other.ordered_access() == ordered_access_;
}

void PresentationDelegate::v_policy(const v_presentationPolicy& policy)
{
    switch (policy.access_scope) {
    case V_PRESENTATION_INSTANCE:
        access_scope_ = dds::core::policy::PresentationAccessScopeKind::INSTANCE;
        break;
    case V_PRESENTATION_TOPIC:
        access_scope_ = dds::core::policy::PresentationAccessScopeKind::TOPIC;
        break;
#ifdef OMG_DDS_OBJECT_MODEL_SUPPORT
    case V_PRESENTATION_GROUP:
        access_scope_ = dds::core::policy::PresentationAccessScopeKind::GROUP;
        break;
#endif // OMG_DDS_OBJECT_MODEL_SUPPORT
    default:
        assert(0);
        break;
    }
    coherent_access_ = policy.coherent_access;
    ordered_access_  = policy.ordered_access;
}

void PresentationDelegate::v_policyI(const v_presentationPolicyI& policy)
{
    v_policy (policy.v);
}

v_presentationPolicy PresentationDelegate::v_policy() const
{
    v_presentationPolicy policy;
    switch (access_scope_) {
    case dds::core::policy::PresentationAccessScopeKind::INSTANCE:
        policy.access_scope = V_PRESENTATION_INSTANCE;
        break;
    case dds::core::policy::PresentationAccessScopeKind::TOPIC:
        policy.access_scope = V_PRESENTATION_TOPIC;
        break;
#ifdef OMG_DDS_OBJECT_MODEL_SUPPORT
    case dds::core::policy::PresentationAccessScopeKind::GROUP:
        policy.access_scope = V_PRESENTATION_GROUP;
        break;
#endif // OMG_DDS_OBJECT_MODEL_SUPPORT
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.access_scope = V_PRESENTATION_INSTANCE;
        break;
    }
    policy.coherent_access = coherent_access_;
    policy.ordered_access  = ordered_access_;
    return policy;
}

v_presentationPolicyI PresentationDelegate::v_policyI() const
{
    v_presentationPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

ProductDataDelegate::ProductDataDelegate()
    : name_("")
{
}

ProductDataDelegate::ProductDataDelegate(const ProductDataDelegate& other)
    : name_(other.name_)
{
}

ProductDataDelegate::ProductDataDelegate(const std::string& name)
    : name_(name)
{
    this->check();
}

void ProductDataDelegate::name(const std::string& name)
{
    name_ = name;
}

std::string ProductDataDelegate::name() const
{
    return name_;
}

bool ProductDataDelegate::operator ==(const ProductDataDelegate& other) const
{
    return other.name() == name_;
}

void ProductDataDelegate::check() const
{
    /* The name_ is just a string: nothing to check. */
}

void ProductDataDelegate::v_policy(const v_productDataPolicy& policy)
{
    if (policy.value != NULL) {
        name_ = policy.value;
    } else {
        name_ = "";
    }
}

void ProductDataDelegate::v_policyI(const v_productDataPolicyI& policy)
{
    v_policy (policy.v);
}

v_productDataPolicy ProductDataDelegate::v_policy() const
{
    v_productDataPolicy policy;
    if (name_.size() > 0) {
        policy.value = os_strdup(name_.c_str());
    } else {
        policy.value = (char*)NULL;
    }
    return policy;
}

v_productDataPolicyI ProductDataDelegate::v_policyI() const
{
    v_productDataPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

ReaderDataLifecycleDelegate::ReaderDataLifecycleDelegate(const ReaderDataLifecycleDelegate& other)
    : autopurge_nowriter_samples_delay_(other.autopurge_nowriter_samples_delay_),
      autopurge_disposed_samples_delay_(other.autopurge_disposed_samples_delay_),
      autopurge_dispose_all_(other.autopurge_dispose_all_),
      enable_invalid_samples_(other.enable_invalid_samples_),
      invalid_sample_visibility_(other.invalid_sample_visibility_)
{
}

ReaderDataLifecycleDelegate::ReaderDataLifecycleDelegate(const dds::core::Duration& nowriter_delay,
                                                         const dds::core::Duration& disposed_samples_delay)
    : autopurge_nowriter_samples_delay_(nowriter_delay),
      autopurge_disposed_samples_delay_(disposed_samples_delay),
      autopurge_dispose_all_(false),
      enable_invalid_samples_(true),
      invalid_sample_visibility_(org::opensplice::core::policy::InvalidSampleVisibility::MINIMUM_INVALID_SAMPLES)
{
    this->check();
}

const dds::core::Duration ReaderDataLifecycleDelegate::autopurge_nowriter_samples_delay() const
{
    return autopurge_nowriter_samples_delay_;
}

void ReaderDataLifecycleDelegate::autopurge_nowriter_samples_delay(const dds::core::Duration& d)
{
    autopurge_nowriter_samples_delay_ = d;
}

const dds::core::Duration ReaderDataLifecycleDelegate::autopurge_disposed_samples_delay() const
{
    return autopurge_disposed_samples_delay_;
}

void ReaderDataLifecycleDelegate::autopurge_disposed_samples_delay(const dds::core::Duration& d)
{
    autopurge_disposed_samples_delay_ = d;
}

bool ReaderDataLifecycleDelegate::autopurge_dispose_all() const
{
    return autopurge_dispose_all_;
}

void ReaderDataLifecycleDelegate::autopurge_dispose_all(bool b)
{
    autopurge_dispose_all_ = b;
}

bool ReaderDataLifecycleDelegate::enable_invalid_samples() const
{
    return enable_invalid_samples_;
}

void ReaderDataLifecycleDelegate::enable_invalid_samples(bool b)
{
    enable_invalid_samples_ = b;
}

void ReaderDataLifecycleDelegate::invalid_sample_visibility(org::opensplice::core::policy::InvalidSampleVisibility::Type visibility)
{
    invalid_sample_visibility_ = visibility;
}

org::opensplice::core::policy::InvalidSampleVisibility::Type ReaderDataLifecycleDelegate::invalid_sample_visibility() const
{
    return invalid_sample_visibility_;
}

bool ReaderDataLifecycleDelegate::operator ==(const ReaderDataLifecycleDelegate& other) const
{
    return other.autopurge_nowriter_samples_delay() == autopurge_nowriter_samples_delay_ &&
           other.autopurge_disposed_samples_delay() == autopurge_disposed_samples_delay_ &&
           other.autopurge_dispose_all()            == autopurge_dispose_all_            &&
           other.enable_invalid_samples()           == enable_invalid_samples_           &&
           other.invalid_sample_visibility()        == invalid_sample_visibility_;
}

void ReaderDataLifecycleDelegate::check() const
{
    /* A Duration object is always valid:
     *      The autopurge_nowriter_samples_delay_ duration is always valid
     *      The autopurge_disposed_samples_delay_ duration is always valid
     *          Nothing to check. */
}

void ReaderDataLifecycleDelegate::v_policy(const v_readerLifecyclePolicy& policy)
{
    autopurge_nowriter_samples_delay_ = helper::convertDuration(policy.autopurge_nowriter_samples_delay);
    autopurge_disposed_samples_delay_ = helper::convertDuration(policy.autopurge_disposed_samples_delay);
    autopurge_dispose_all_  = policy.autopurge_dispose_all;
    enable_invalid_samples_ = policy.enable_invalid_samples;
    switch (policy.invalid_sample_visibility) {
    case V_VISIBILITY_NO_INVALID_SAMPLES:
        invalid_sample_visibility_ = org::opensplice::core::policy::InvalidSampleVisibility::NO_INVALID_SAMPLES;
        break;
    case V_VISIBILITY_MINIMUM_INVALID_SAMPLES:
        invalid_sample_visibility_ = org::opensplice::core::policy::InvalidSampleVisibility::MINIMUM_INVALID_SAMPLES;
        break;
    case V_VISIBILITY_ALL_INVALID_SAMPLES:
        invalid_sample_visibility_ = org::opensplice::core::policy::InvalidSampleVisibility::ALL_INVALID_SAMPLES;
        break;
    default:
        assert(0);
        break;
    }
}

void ReaderDataLifecycleDelegate::v_policyI(const v_readerLifecyclePolicyI& policy)
{
    autopurge_nowriter_samples_delay_ = helper::convertDuration(policy.v.autopurge_nowriter_samples_delay);
    autopurge_disposed_samples_delay_ = helper::convertDuration(policy.v.autopurge_disposed_samples_delay);
    autopurge_dispose_all_  = policy.v.autopurge_dispose_all;
    enable_invalid_samples_ = policy.v.enable_invalid_samples;
    switch (policy.v.invalid_sample_visibility) {
    case V_VISIBILITY_NO_INVALID_SAMPLES:
        invalid_sample_visibility_ = org::opensplice::core::policy::InvalidSampleVisibility::NO_INVALID_SAMPLES;
        break;
    case V_VISIBILITY_MINIMUM_INVALID_SAMPLES:
        invalid_sample_visibility_ = org::opensplice::core::policy::InvalidSampleVisibility::MINIMUM_INVALID_SAMPLES;
        break;
    case V_VISIBILITY_ALL_INVALID_SAMPLES:
        invalid_sample_visibility_ = org::opensplice::core::policy::InvalidSampleVisibility::ALL_INVALID_SAMPLES;
        break;
    default:
        assert(0);
        break;
    }
}

v_readerLifecyclePolicy ReaderDataLifecycleDelegate::v_policy() const
{
    v_readerLifecyclePolicy policy;
    policy.autopurge_nowriter_samples_delay = helper::convertToVDuration(autopurge_nowriter_samples_delay_);
    policy.autopurge_disposed_samples_delay = helper::convertToVDuration(autopurge_disposed_samples_delay_);
    policy.autopurge_dispose_all = autopurge_dispose_all_;
    policy.enable_invalid_samples = enable_invalid_samples_;
    if (!enable_invalid_samples_) {
        policy.invalid_sample_visibility = V_VISIBILITY_NO_INVALID_SAMPLES;
    } else {
        switch (invalid_sample_visibility_) {
        case org::opensplice::core::policy::InvalidSampleVisibility::NO_INVALID_SAMPLES:
            policy.invalid_sample_visibility = V_VISIBILITY_NO_INVALID_SAMPLES;
            policy.enable_invalid_samples = FALSE;
            break;
        case org::opensplice::core::policy::InvalidSampleVisibility::MINIMUM_INVALID_SAMPLES:
            policy.invalid_sample_visibility = V_VISIBILITY_MINIMUM_INVALID_SAMPLES;
            break;
        case org::opensplice::core::policy::InvalidSampleVisibility::ALL_INVALID_SAMPLES:
            policy.invalid_sample_visibility = V_VISIBILITY_ALL_INVALID_SAMPLES;
            break;
        default:
            assert(0);
            break;
        }
    }
    return policy;
}

v_readerLifecyclePolicyI ReaderDataLifecycleDelegate::v_policyI() const
{
    v_readerLifecyclePolicyI policy;
    policy.v.autopurge_nowriter_samples_delay = (helper::convertDuration(autopurge_nowriter_samples_delay_));
    policy.v.autopurge_disposed_samples_delay = (helper::convertDuration(autopurge_disposed_samples_delay_));
    policy.v.autopurge_dispose_all = autopurge_dispose_all_;
    policy.v.enable_invalid_samples = enable_invalid_samples_;
    if (!enable_invalid_samples_) {
        policy.v.invalid_sample_visibility = V_VISIBILITY_NO_INVALID_SAMPLES;
    } else {
        switch (invalid_sample_visibility_) {
        case org::opensplice::core::policy::InvalidSampleVisibility::NO_INVALID_SAMPLES:
            policy.v.invalid_sample_visibility = V_VISIBILITY_NO_INVALID_SAMPLES;
            policy.v.enable_invalid_samples = FALSE;
            break;
        case org::opensplice::core::policy::InvalidSampleVisibility::MINIMUM_INVALID_SAMPLES:
            policy.v.invalid_sample_visibility = V_VISIBILITY_MINIMUM_INVALID_SAMPLES;
            break;
        case org::opensplice::core::policy::InvalidSampleVisibility::ALL_INVALID_SAMPLES:
            policy.v.invalid_sample_visibility = V_VISIBILITY_ALL_INVALID_SAMPLES;
            break;
        default:
            assert(0);
            break;
        }
    }
    return policy;
}
    
//==============================================================================

ReaderLifespanDelegate::ReaderLifespanDelegate()
    : used_(false), duration_(dds::core::Duration::infinite())
{
}

ReaderLifespanDelegate::ReaderLifespanDelegate(
        const ReaderLifespanDelegate& other)
    : used_(other.used_), duration_(other.duration_)
{
}

ReaderLifespanDelegate::ReaderLifespanDelegate(bool used, const dds::core::Duration& d)
    : used_(used), duration_(d)
{
    this->check();
}

void ReaderLifespanDelegate::used(bool used)
{
    used_ = used;
}

bool ReaderLifespanDelegate::used() const
{
    return used_;
}

void ReaderLifespanDelegate::duration(const dds::core::Duration& d)
{
    duration_ = d;
}

const dds::core::Duration ReaderLifespanDelegate::duration() const
{
    return duration_;
}

bool ReaderLifespanDelegate::operator ==(const ReaderLifespanDelegate& other) const
{
    return other.used() == used_ &&
           other.duration() == duration_;
}

void ReaderLifespanDelegate::check() const
{
    /* A Duration object is always valid:
     *      The duration_ is always valid:
     *          Nothing to check. */
}

void ReaderLifespanDelegate::v_policy(const v_readerLifespanPolicy& policy)
{
    used_ = policy.used;
    duration_ = helper::convertDuration(policy.duration);
}

void ReaderLifespanDelegate::v_policyI(const v_readerLifespanPolicyI& policy)
{
    used_ = policy.v.used;
    duration_ = helper::convertDuration(policy.v.duration);
}

v_readerLifespanPolicy ReaderLifespanDelegate::v_policy() const
{
    v_readerLifespanPolicy policy;
    policy.used =  used_;
    policy.duration = helper::convertToVDuration(duration_);
    return policy;
}

v_readerLifespanPolicyI ReaderLifespanDelegate::v_policyI() const
{
    v_readerLifespanPolicyI policy;
    policy.v.used =  used_;
    policy.v.duration = helper::convertDuration(duration_);
    return policy;
}


//==============================================================================

ReliabilityDelegate::ReliabilityDelegate(const ReliabilityDelegate& other)
    :  kind_(other.kind_),
       max_blocking_time_(other.max_blocking_time_),
       synchronous_(other.synchronous_)
{
}

ReliabilityDelegate::ReliabilityDelegate(dds::core::policy::ReliabilityKind::Type kind,
                                         const dds::core::Duration& max_blocking_time)
    :  kind_(kind),
       max_blocking_time_(max_blocking_time),
       synchronous_(false)
{
    this->check();
}

void ReliabilityDelegate::kind(dds::core::policy::ReliabilityKind::Type kind)
{
    kind_ = kind;
}

dds::core::policy::ReliabilityKind::Type ReliabilityDelegate::kind() const
{
    return kind_;
}

void ReliabilityDelegate::max_blocking_time(const dds::core::Duration& d)
{
    max_blocking_time_ = d;
}

const dds::core::Duration ReliabilityDelegate::max_blocking_time() const
{
    return max_blocking_time_;
}

bool ReliabilityDelegate::synchronous() const
{
    return synchronous_;
}

void ReliabilityDelegate::synchronous(bool b)
{
    synchronous_ = b;
}

bool ReliabilityDelegate::operator ==(const ReliabilityDelegate& other) const
{
    return other.kind() == kind_ &&
           other.max_blocking_time() == max_blocking_time_ &&
           other.synchronous() == synchronous_;
}

void ReliabilityDelegate::check() const
{
    /* The kind correctness is enforced by the compiler. */
    /* A Duration object is always valid: the max_blocking_time_ is valid. */
}

void ReliabilityDelegate::v_policy(const v_reliabilityPolicy& policy)
{
    switch (policy.kind) {
    case V_RELIABILITY_BESTEFFORT:
        kind_ = dds::core::policy::ReliabilityKind::BEST_EFFORT;
        break;
    case V_RELIABILITY_RELIABLE:
        kind_ = dds::core::policy::ReliabilityKind::RELIABLE;
        break;
    default:
        assert(0);
        break;
    }
    max_blocking_time_ = helper::convertDuration(policy.max_blocking_time);
    synchronous_ = policy.synchronous;
}

void ReliabilityDelegate::v_policyI(const v_reliabilityPolicyI& policy)
{
    switch (policy.v.kind) {
    case V_RELIABILITY_BESTEFFORT:
        kind_ = dds::core::policy::ReliabilityKind::BEST_EFFORT;
        break;
    case V_RELIABILITY_RELIABLE:
        kind_ = dds::core::policy::ReliabilityKind::RELIABLE;
        break;
    default:
        assert(0);
        break;
    }
    max_blocking_time_ = helper::convertDuration(policy.v.max_blocking_time);
    synchronous_ = policy.v.synchronous;
}

v_reliabilityPolicy ReliabilityDelegate::v_policy() const
{
    v_reliabilityPolicy policy;
    switch (kind_) {
    case dds::core::policy::ReliabilityKind::BEST_EFFORT:
        policy.kind = V_RELIABILITY_BESTEFFORT;
        break;
    case dds::core::policy::ReliabilityKind::RELIABLE:
        policy.kind = V_RELIABILITY_RELIABLE;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.kind = V_RELIABILITY_BESTEFFORT;
        break;
    }
    policy.max_blocking_time = helper::convertToVDuration(max_blocking_time_);
    policy.synchronous = synchronous_;
    return policy;
}

v_reliabilityPolicyI ReliabilityDelegate::v_policyI() const
{
    v_reliabilityPolicyI policy;
    switch (kind_) {
    case dds::core::policy::ReliabilityKind::BEST_EFFORT:
        policy.v.kind = V_RELIABILITY_BESTEFFORT;
        break;
    case dds::core::policy::ReliabilityKind::RELIABLE:
        policy.v.kind = V_RELIABILITY_RELIABLE;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.v.kind = V_RELIABILITY_BESTEFFORT;
        break;
    }
    policy.v.max_blocking_time = helper::convertDuration(max_blocking_time_);
    policy.v.synchronous = synchronous_;
    return policy;
}

//==============================================================================

ResourceLimitsDelegate::ResourceLimitsDelegate(const ResourceLimitsDelegate& other)
    : max_samples_(other.max_samples_),
      max_instances_(other.max_instances_),
      max_samples_per_instance_(other.max_samples_per_instance_)
{
}

ResourceLimitsDelegate::ResourceLimitsDelegate(int32_t max_samples,
                                               int32_t max_instances,
                                               int32_t max_samples_per_instance)
    : max_samples_(max_samples),
      max_instances_(max_instances),
      max_samples_per_instance_(max_samples_per_instance)
{
    this->check();
}

void ResourceLimitsDelegate::max_samples(int32_t samples)
{
    max_samples_ = samples;
}

int32_t ResourceLimitsDelegate::max_samples() const
{
    return max_samples_;
}

void ResourceLimitsDelegate::max_instances(int32_t max_instances)
{
    max_instances_ = max_instances;
}

int32_t ResourceLimitsDelegate::max_instances() const
{
    return max_instances_;
}

void ResourceLimitsDelegate::max_samples_per_instance(int32_t max_samples_per_instance)
{
    max_samples_per_instance_ = max_samples_per_instance;
}

int32_t ResourceLimitsDelegate::max_samples_per_instance() const
{
    return max_samples_per_instance_;
}

bool ResourceLimitsDelegate::operator ==(const ResourceLimitsDelegate& other) const
{
    return other.max_samples() == max_samples_ &&
           other.max_instances() == max_instances_ &&
           other.max_samples_per_instance() == max_samples_per_instance_;
}

void ResourceLimitsDelegate::check() const
{
    if ((max_samples_ <= 0) && (max_samples_ != dds::core::LENGTH_UNLIMITED)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR, "Invalid ResourceLimits::max_samples (%ld) value.", max_samples_);
    }
    if ((max_instances_ <= 0) && (max_instances_ != dds::core::LENGTH_UNLIMITED)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR, "Invalid ResourceLimits::max_instances (%ld) value.", max_instances_);
    }
    if ((max_samples_per_instance_ <= 0) && (max_samples_per_instance_ != dds::core::LENGTH_UNLIMITED)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR, "Invalid ResourceLimits::max_samples_per_instance (%ld) value.", max_samples_per_instance_);
    }
}

void ResourceLimitsDelegate::check_against(const org::opensplice::core::policy::HistoryDelegate& history) const
{
    if (history.kind() == dds::core::policy::HistoryKind::KEEP_LAST) {
        if (this->max_samples_per_instance_ != dds::core::LENGTH_UNLIMITED) {
            if (history.depth() > this->max_samples_per_instance_) {
                ISOCPP_THROW_EXCEPTION(ISOCPP_INCONSISTENT_POLICY_ERROR, "History::depth (%ld) > ResourceLimits::max_samples_per_instance (%ld) with KEEP_LAST", history.depth(), max_samples_per_instance_);
            }
        }
    }
}

void ResourceLimitsDelegate::v_policy(const v_resourcePolicy& policy)
{
    max_samples_ = policy.max_samples;
    max_instances_ = policy.max_instances;
    max_samples_per_instance_ = policy.max_samples_per_instance;
}

void ResourceLimitsDelegate::v_policyI(const v_resourcePolicyI& policy)
{
    v_policy (policy.v);
}

v_resourcePolicy ResourceLimitsDelegate::v_policy() const
{
    v_resourcePolicy policy;
    policy.max_samples = max_samples_;
    policy.max_instances = max_instances_;
    policy.max_samples_per_instance = max_samples_per_instance_;
    return policy;
}

v_resourcePolicyI ResourceLimitsDelegate::v_policyI() const
{
    v_resourcePolicyI policy;
    policy.v = v_policy ();
    return policy;
}
    
//==============================================================================

SchedulingDelegate::SchedulingDelegate()
     : scheduling_kind_(org::opensplice::core::policy::SchedulingKind::SCHEDULE_DEFAULT),
       scheduling_priority_kind_(org::opensplice::core::policy::SchedulingPriorityKind::PRIORITY_RELATIVE),
       scheduling_priority_(0)
{
}

SchedulingDelegate::SchedulingDelegate(const SchedulingDelegate& other)
    : scheduling_kind_(other.scheduling_kind_),
      scheduling_priority_kind_(other.scheduling_priority_kind_),
      scheduling_priority_(other.scheduling_priority_)
{
}

SchedulingDelegate::SchedulingDelegate(
            const org::opensplice::core::policy::SchedulingKind::Type& scheduling_kind,
            const org::opensplice::core::policy::SchedulingPriorityKind::Type& scheduling_priority_kind,
            int32_t scheduling_priority)
    : scheduling_kind_(scheduling_kind),
      scheduling_priority_kind_(scheduling_priority_kind),
      scheduling_priority_(scheduling_priority)
{
}

void SchedulingDelegate::scheduling_kind(const org::opensplice::core::policy::SchedulingKind::Type& scheduling_kind)
{
    scheduling_kind_ = scheduling_kind;
}

org::opensplice::core::policy::SchedulingKind::Type SchedulingDelegate::scheduling_kind() const
{
    return scheduling_kind_;
}

void SchedulingDelegate::scheduling_priority_kind(const org::opensplice::core::policy::SchedulingPriorityKind::Type& scheduling_priority_kind)
{
    scheduling_priority_kind_ = scheduling_priority_kind;
}

org::opensplice::core::policy::SchedulingPriorityKind::Type SchedulingDelegate::scheduling_priority_kind() const
{
    return scheduling_priority_kind_;
}

void SchedulingDelegate::scheduling_priority(int32_t scheduling_priority)
{
    scheduling_priority_ = scheduling_priority;
}

int32_t SchedulingDelegate::scheduling_priority() const
{
    return scheduling_priority_;
}

bool SchedulingDelegate::operator ==(const SchedulingDelegate& other) const
{
    return other.scheduling_kind() == scheduling_kind_ &&
           other.scheduling_priority_kind() == scheduling_priority_kind_ &&
           other.scheduling_priority() == scheduling_priority_;
}

void SchedulingDelegate::check() const
{
    /* The enum correctness is enforced by the compiler. */
}

void SchedulingDelegate::v_policyI(const v_schedulePolicyI& policy)
{
    switch (policy.v.kind) {
    case V_SCHED_DEFAULT:
        scheduling_kind_ = org::opensplice::core::policy::SchedulingKind::SCHEDULE_DEFAULT;
        break;
    case V_SCHED_TIMESHARING:
        scheduling_kind_ = org::opensplice::core::policy::SchedulingKind::SCHEDULE_TIMESHARING;
        break;
    case V_SCHED_REALTIME:
        scheduling_kind_ = org::opensplice::core::policy::SchedulingKind::SCHEDULE_REALTIME;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        scheduling_kind_ = org::opensplice::core::policy::SchedulingKind::SCHEDULE_DEFAULT;
        break;
    }
    switch (policy.v.priorityKind) {
    case V_SCHED_PRIO_RELATIVE:
        scheduling_priority_kind_ = org::opensplice::core::policy::SchedulingPriorityKind::PRIORITY_RELATIVE;
        break;
    case V_SCHED_PRIO_ABSOLUTE:
        scheduling_priority_kind_ = org::opensplice::core::policy::SchedulingPriorityKind::PRIORITY_ABSOLUTE;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        scheduling_priority_kind_ = org::opensplice::core::policy::SchedulingPriorityKind::PRIORITY_RELATIVE;
        break;
    }
    scheduling_priority_ = policy.v.priority;
}

v_schedulePolicyI SchedulingDelegate::v_policyI() const
{
    v_schedulePolicyI policy;
    switch (scheduling_kind_) {
    case org::opensplice::core::policy::SchedulingKind::SCHEDULE_DEFAULT:
        policy.v.kind = V_SCHED_DEFAULT;
        break;
    case org::opensplice::core::policy::SchedulingKind::SCHEDULE_TIMESHARING:
        policy.v.kind = V_SCHED_TIMESHARING;
        break;
    case org::opensplice::core::policy::SchedulingKind::SCHEDULE_REALTIME:
        policy.v.kind = V_SCHED_REALTIME;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.v.kind = V_SCHED_DEFAULT;
        break;
    }
    switch (scheduling_priority_kind_) {
    case org::opensplice::core::policy::SchedulingPriorityKind::PRIORITY_RELATIVE:
        policy.v.priorityKind = V_SCHED_PRIO_RELATIVE;
        break;
    case org::opensplice::core::policy::SchedulingPriorityKind::PRIORITY_ABSOLUTE:
        policy.v.priorityKind = V_SCHED_PRIO_ABSOLUTE;
        break;
    default:
        assert(0);
        /* Just to satisfy the compiler. */
        policy.v.priorityKind = V_SCHED_PRIO_RELATIVE;
        break;
    }
    policy.v.priority = scheduling_priority_;
    return policy;
}

void SchedulingDelegate::os_thread_attr(os_threadAttr *osThreadAttr) const
{
    assert(osThreadAttr);

    switch (scheduling_kind_) {
    case org::opensplice::core::policy::SchedulingKind::SCHEDULE_DEFAULT:
        osThreadAttr->schedClass = os_procAttrGetClass ();
        break;
    case org::opensplice::core::policy::SchedulingKind::SCHEDULE_TIMESHARING:
        osThreadAttr->schedClass = OS_SCHED_REALTIME;
    break;
    case org::opensplice::core::policy::SchedulingKind::SCHEDULE_REALTIME:
        osThreadAttr->schedClass = OS_SCHED_TIMESHARE;
    break;
    }
    /* Configure thread scheduling priority. */
    osThreadAttr->schedPriority = scheduling_priority_;
    if (scheduling_priority_kind_ == org::opensplice::core::policy::SchedulingPriorityKind::PRIORITY_RELATIVE) {
        osThreadAttr->schedPriority += os_procAttrGetPriority ();
    }
}


//==============================================================================

ShareDelegate::ShareDelegate()
    : name_(""),
      enable_(false)
{
}

ShareDelegate::ShareDelegate(const ShareDelegate& other)
    : name_(other.name_),
      enable_(other.enable_)
{
    this->check();
}

ShareDelegate::ShareDelegate(const std::string& name, bool enable)
    : name_(name),
      enable_(enable)
{
}

void ShareDelegate::name(const std::string& name)
{
    name_ = name;
}

std::string ShareDelegate::name() const
{
    return name_;
}

void ShareDelegate::enable(bool enable)
{
    enable_ = enable;
}

bool ShareDelegate::enable() const
{
    return enable_;
}

bool ShareDelegate::operator ==(const ShareDelegate& other) const
{
    return other.name() == name_ &&
           other.enable() == enable_;
}

void ShareDelegate::check() const
{
    if (enable_) {
        if (name_.size() == 0) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR, "When Share is enabled, it needs a name");
        }
    }
}

void ShareDelegate::v_policy(const v_sharePolicy& policy)
{
    if (policy.name != NULL) {
        name_ = policy.name;
    } else {
        name_ = "";
    }
    enable_ = policy.enable;
}

void ShareDelegate::v_policyI(const v_sharePolicyI& policy)
{
    v_policy (policy.v);
}

v_sharePolicy ShareDelegate::v_policy() const
{
    v_sharePolicy policy;
    if (name_.size() > 0) {
        policy.name = os_strdup(name_.c_str());
    } else {
        policy.name = (char*)NULL;
    }
    policy.enable = enable_;
    return policy;
}

v_sharePolicyI ShareDelegate::v_policyI() const
{
    v_sharePolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

SubscriptionKeyDelegate::SubscriptionKeyDelegate()
    : use_key_list_(false),
      keys_()
{
}

SubscriptionKeyDelegate::SubscriptionKeyDelegate(const SubscriptionKeyDelegate& other)
    : use_key_list_(other.use_key_list_),
      keys_(other.keys_)
{
}

SubscriptionKeyDelegate::SubscriptionKeyDelegate(bool use_key_list, const std::string& key)
    : use_key_list_(use_key_list),
      keys_()
{
    keys_.push_back(key);
    this->check();
}

SubscriptionKeyDelegate::SubscriptionKeyDelegate(bool use_key_list, const dds::core::StringSeq& keys)
    : use_key_list_(use_key_list),
      keys_(keys)
{
    this->check();
}

void SubscriptionKeyDelegate::key(const std::string& key)
{
    keys_.clear();
    keys_.push_back(key);
}
void SubscriptionKeyDelegate::key(const dds::core::StringSeq& keys)
{
    keys_ = keys;
}

const dds::core::StringSeq& SubscriptionKeyDelegate::key() const
{
    return keys_;
}

void SubscriptionKeyDelegate::use_key_list(bool use_key_list)
{
    use_key_list_ = use_key_list;
}
bool SubscriptionKeyDelegate::use_key_list() const
{
    return use_key_list_;
}

bool SubscriptionKeyDelegate::operator ==(const SubscriptionKeyDelegate& other) const
{
    return other.key() == keys_ &&
           other.use_key_list() == use_key_list_;
}

void SubscriptionKeyDelegate::check() const
{
    /* The keys_ and use_key_list_ are just a sequence and boolean: nothing to check. */
}

void SubscriptionKeyDelegate::v_policy(const struct _DDS_SubscriptionKeyQosPolicy& policy)
{
    use_key_list_ = policy.use_key_list;
    helper::convertStringSeq(c_sequence(policy.key_list), keys_);
}

void SubscriptionKeyDelegate::v_policy(const v_userKeyPolicy& policy)
{
    use_key_list_ = policy.enable;
    helper::convertStringSeq(policy.expression, keys_, ",");
}

void SubscriptionKeyDelegate::v_policyI(const v_userKeyPolicyI& policy)
{
    v_policy (policy.v);
}

v_userKeyPolicy SubscriptionKeyDelegate::v_policy() const
{
    v_userKeyPolicy policy = { 0 };
    policy.enable = use_key_list_;
    helper::convertStringSeq(keys_, policy.expression, ",");
    return policy;
}

v_userKeyPolicyI SubscriptionKeyDelegate::v_policyI() const
{
    v_userKeyPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

TimeBasedFilterDelegate::TimeBasedFilterDelegate(const TimeBasedFilterDelegate& other)
    : min_sep_(other.min_sep_)
{
}

TimeBasedFilterDelegate::TimeBasedFilterDelegate(const dds::core::Duration& min_separation)
    : min_sep_(min_separation)
{
    this->check();
}

void TimeBasedFilterDelegate::min_separation(const dds::core::Duration& ms)
{
    min_sep_ = ms;
}

const dds::core::Duration TimeBasedFilterDelegate::min_separation() const
{
    return min_sep_;
}

bool TimeBasedFilterDelegate::operator ==(const TimeBasedFilterDelegate& other) const
{
    return other.min_separation() == min_sep_;
}

void TimeBasedFilterDelegate::check() const
{
    /* A Duration object is always valid:
     *      The min_sep_ duration is always valid:
     *          Nothing to check. */
}

void TimeBasedFilterDelegate::check_against(const org::opensplice::core::policy::DeadlineDelegate& deadline) const
{
    if (deadline.period() < this->min_sep_) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INCONSISTENT_POLICY_ERROR, "Deadline: period < TimeBasedFilter::min_separation");
    }
}

void TimeBasedFilterDelegate::v_policy(const v_pacingPolicy& policy)
{
    min_sep_ = helper::convertDuration(policy.minSeperation);
}

void TimeBasedFilterDelegate::v_policyI(const v_pacingPolicyI& policy)
{
    min_sep_ = helper::convertDuration(policy.v.minSeperation);
}

v_pacingPolicy TimeBasedFilterDelegate::v_policy() const
{
    v_pacingPolicy policy;
    policy.minSeperation = helper::convertToVDuration(min_sep_);
    return policy;
}

v_pacingPolicyI TimeBasedFilterDelegate::v_policyI() const
{
    v_pacingPolicyI policy;
    policy.v.minSeperation = helper::convertDuration(min_sep_);
    return policy;
}


//==============================================================================

TopicDataDelegate::TopicDataDelegate() : value_()
{
}

TopicDataDelegate::TopicDataDelegate(const TopicDataDelegate& other)
    : value_(other.value_)
{
}

TopicDataDelegate::TopicDataDelegate(const dds::core::ByteSeq& seq)
    : value_(seq)
{
    this->check();
}

void TopicDataDelegate::value(const dds::core::ByteSeq& seq)
{
    value_ = seq;
}

const dds::core::ByteSeq& TopicDataDelegate::value() const
{
    return value_;
}

bool TopicDataDelegate::operator ==(const TopicDataDelegate& other) const
{
    return other.value() == value_;
}

void TopicDataDelegate::check() const
{
    /* The value_ is just a sequence: nothing to check. */
}

void TopicDataDelegate::v_policyI(const v_topicDataPolicyI& policy)
{
    helper::convertByteSeq(policy.v.value, policy.v.size, value_);
}

void TopicDataDelegate::v_policy(const v_builtinTopicDataPolicy& policy)
{
    helper::convertByteSeq(policy.value, c_arraySize(policy.value), value_);
}

v_topicDataPolicyI TopicDataDelegate::v_policyI() const
{
    v_topicDataPolicyI policy = { 0 };
    helper::convertByteSeq(value_, policy.v.value, policy.v.size);
    return policy;
}


//==============================================================================

TransportPriorityDelegate::TransportPriorityDelegate(const TransportPriorityDelegate& other)
    : value_(other.value_)
{
}

TransportPriorityDelegate::TransportPriorityDelegate(uint32_t prio)
    : value_(prio)
{
    this->check();
}

void TransportPriorityDelegate::value(uint32_t prio)
{
    value_ = prio;
}

uint32_t TransportPriorityDelegate::value() const
{
    return value_;
}

bool TransportPriorityDelegate::operator ==(const TransportPriorityDelegate& other) const
{
    return other.value() == value_;
}

void TransportPriorityDelegate::check() const
{
    /* Any value is valid: nothing to check. */
}

void TransportPriorityDelegate::v_policy(const v_transportPolicy& policy)
{
    value_ = policy.value;
}

void TransportPriorityDelegate::v_policyI(const v_transportPolicyI& policy)
{
    v_policy (policy.v);
}

v_transportPolicy TransportPriorityDelegate::v_policy() const
{
    v_transportPolicy policy;
    policy.value = value_;
    return policy;
}

v_transportPolicyI TransportPriorityDelegate::v_policyI() const
{
    v_transportPolicyI policy;
    policy.v = v_policy ();
    return policy;
}

//==============================================================================

UserDataDelegate::UserDataDelegate()
    : value_()
{
}

UserDataDelegate::UserDataDelegate(const UserDataDelegate& other)
    : value_(other.value_)
{
}

UserDataDelegate::UserDataDelegate(const dds::core::ByteSeq& seq)
    : value_(seq)
{
    this->check();
}

void UserDataDelegate::value(const dds::core::ByteSeq& seq)
{
    value_ = seq;
}

const dds::core::ByteSeq UserDataDelegate::value() const
{
    return value_;
}

bool UserDataDelegate::operator ==(const UserDataDelegate& other) const
{
    return other.value() == value_;
}

void UserDataDelegate::check() const
{
    /* The value_ is just a sequence: nothing to check. */
}

void UserDataDelegate::v_policy(const v_userDataPolicy& policy)
{
    helper::convertByteSeq(policy.value, policy.size, value_);
}

void UserDataDelegate::v_policyI(const v_userDataPolicyI& policy)
{
    v_policy (policy.v);
}

void UserDataDelegate::v_policy(const v_builtinUserDataPolicy& policy)
{
    helper::convertByteSeq(policy.value, c_arraySize(policy.value), value_);
}

v_userDataPolicyI UserDataDelegate::v_policyI() const
{
    v_userDataPolicyI policy = { 0 };
    helper::convertByteSeq(value_, policy.v.value, policy.v.size);
    return policy;
}


//==============================================================================

WriterDataLifecycleDelegate::WriterDataLifecycleDelegate(const WriterDataLifecycleDelegate& other)
    : autodispose_(other.autodispose_),
      autopurge_suspended_samples_delay_(other.autopurge_suspended_samples_delay_),
      autounregister_instance_delay_(other.autounregister_instance_delay_)
{
}

WriterDataLifecycleDelegate::WriterDataLifecycleDelegate(bool autodispose)
    : autodispose_(autodispose),
      autopurge_suspended_samples_delay_(dds::core::Duration::infinite()),
      autounregister_instance_delay_(dds::core::Duration::infinite())
{
    this->check();
}

bool WriterDataLifecycleDelegate::autodispose() const
{
    return autodispose_;
}

void WriterDataLifecycleDelegate::autodispose(bool b)
{
    autodispose_ = b;
}

void WriterDataLifecycleDelegate::autopurge_suspended_samples_delay(const dds::core::Duration& d)
{
    autopurge_suspended_samples_delay_ = d;
}

const dds::core::Duration WriterDataLifecycleDelegate::autopurge_suspended_samples_delay() const
{
    return autopurge_suspended_samples_delay_;
}

void WriterDataLifecycleDelegate::autounregister_instance_delay(const dds::core::Duration& d)
{
    autounregister_instance_delay_ = d;
}

const dds::core::Duration WriterDataLifecycleDelegate::autounregister_instance_delay() const
{
    return autounregister_instance_delay_;
}

bool WriterDataLifecycleDelegate::operator ==(const WriterDataLifecycleDelegate& other) const
{
    return other.autodispose() == autodispose_;
}

void WriterDataLifecycleDelegate::check() const
{
    /* The autodispose is just a boolean: nothing to check. */
}

void WriterDataLifecycleDelegate::v_policy(const v_writerLifecyclePolicy& policy)
{
    autodispose_ = policy.autodispose_unregistered_instances;
    autopurge_suspended_samples_delay_ = helper::convertDuration(policy.autopurge_suspended_samples_delay);
    autounregister_instance_delay_ = helper::convertDuration(policy.autounregister_instance_delay);
}

void WriterDataLifecycleDelegate::v_policyI(const v_writerLifecyclePolicyI& policy)
{
    autodispose_ = policy.v.autodispose_unregistered_instances;
    autopurge_suspended_samples_delay_ = helper::convertDuration(policy.v.autopurge_suspended_samples_delay);
    autounregister_instance_delay_ = helper::convertDuration(policy.v.autounregister_instance_delay);
}

v_writerLifecyclePolicy WriterDataLifecycleDelegate::v_policy() const
{
    v_writerLifecyclePolicy policy;
    policy.autodispose_unregistered_instances = autodispose_;
    policy.autopurge_suspended_samples_delay = helper::convertToVDuration(autopurge_suspended_samples_delay_);
    policy.autounregister_instance_delay = helper::convertToVDuration(autounregister_instance_delay_);
    return policy;
}


v_writerLifecyclePolicyI WriterDataLifecycleDelegate::v_policyI() const
{
    v_writerLifecyclePolicyI policy;
    policy.v.autodispose_unregistered_instances = autodispose_;
    policy.v.autopurge_suspended_samples_delay = helper::convertDuration(autopurge_suspended_samples_delay_);
    policy.v.autounregister_instance_delay = helper::convertDuration(autounregister_instance_delay_);
    return policy;
}


#ifdef  OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

class DataRepresentationDelegate { };

#endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT


#ifdef  OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

class TypeConsistencyEnforcementDelegate { };

#endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

}
}
}
}
