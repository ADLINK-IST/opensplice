/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OSPL_DDS_CORE_POLICY_TCOREPOLICY_HPP_
#define OSPL_DDS_CORE_POLICY_TCOREPOLICY_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/policy/TCorePolicy.hpp>

// Implementation

namespace dds
{
namespace core
{
namespace policy
{

//TUserData

template <typename D>
TUserData<D>::TUserData() : dds::core::Value<D>() { }

template <typename D>
TUserData<D>::TUserData(const dds::core::ByteSeq& seq) : dds::core::Value<D>(seq) { }

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
TUserData<D>::TUserData(const uint8_t* value_begin, const uint8_t* value_end) { }

template <typename D>
TUserData<D>::TUserData(const TUserData& other) : dds::core::Value<D>(other.value()) { }

template <typename D>
TUserData<D>& TUserData<D>::value(const dds::core::ByteSeq& seq)
{
    this->delegate().value(seq);
    return *this;
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
template <typename OCTET_ITER>
TUserData<D>& TUserData<D>::value(OCTET_ITER begin, OCTET_ITER end)
{
    return *this;
}

template <typename D>
const dds::core::ByteSeq TUserData<D>::value() const
{
    return this->delegate().value();
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
const uint8_t* TUserData<D>::begin() const
{
    return NULL;
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
const uint8_t* TUserData<D>::end() const
{
    return NULL;
}

//TGroupData

template <typename D>
TGroupData<D>::TGroupData() : dds::core::Value<D>() { }

template <typename D>
TGroupData<D>::TGroupData(const dds::core::ByteSeq& seq) : dds::core::Value<D>(seq) { }

template <typename D>
TGroupData<D>::TGroupData(const TGroupData& other) : dds::core::Value<D>(other.value()) { }

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
TGroupData<D>::TGroupData(const uint8_t* value_begin, const uint8_t* value_end) { }

template <typename D>
TGroupData<D>& TGroupData<D>::value(const dds::core::ByteSeq& seq)
{
    this->delegate().value(seq);
    return *this;
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
template <typename OCTET_ITER>
TGroupData<D>& TGroupData<D>::value(OCTET_ITER begin, OCTET_ITER end)
{
    return *this;
}

template <typename D>
const dds::core::ByteSeq TGroupData<D>::value() const
{
    return this->delegate().value();
}

template <typename D>
dds::core::ByteSeq& TGroupData<D>::value(dds::core::ByteSeq& dst) const
{
    dst = this->delegate().value();
    return dst;
}
/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
const uint8_t* TGroupData<D>::begin() const
{
    return NULL;
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
const uint8_t* TGroupData<D>::end() const
{
    return NULL;
}

//TTopicData

template <typename D>
TTopicData<D>::TTopicData() : dds::core::Value<D>() { }

template <typename D>
TTopicData<D>::TTopicData(const dds::core::ByteSeq& seq) : dds::core::Value<D>(seq) { }

template <typename D>
TTopicData<D>::TTopicData(const TTopicData& other) : dds::core::Value<D>(other.value()) { }

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
TTopicData<D>::TTopicData(const uint8_t* value_begin, const uint8_t* value_end) { }

template <typename D>
TTopicData<D>& TTopicData<D>::value(const dds::core::ByteSeq& seq)
{
    this->delegate().value(seq);
    return *this;
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
template <typename OCTET_ITER>
TTopicData<D>& TTopicData<D>::value(OCTET_ITER begin, OCTET_ITER end)
{
    return *this;
}

template <typename D>
const dds::core::ByteSeq TTopicData<D>::value() const
{
    return this->delegate().value();
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
dds::core::ByteSeq& TTopicData<D>::value(dds::core::ByteSeq& dst) const
{
    return dst;
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
const uint8_t* TTopicData<D>::begin() const
{
    return NULL;
}

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
const uint8_t* TTopicData<D>::end() const
{
    return NULL;
}

//TEntityFactory

template <typename D>
TEntityFactory<D>::TEntityFactory() : dds::core::Value<D>(true) { }

template <typename D>
TEntityFactory<D>::TEntityFactory(bool the_auto_enable) : dds::core::Value<D>(the_auto_enable) { }

template <typename D>
TEntityFactory<D>::TEntityFactory(const TEntityFactory& other) : dds::core::Value<D>(other.autoenable_created_entities()) { }

template <typename D>
TEntityFactory<D>& TEntityFactory<D>::autoenable_created_entities(bool on)
{
    this->delegate().auto_enable(on);
    return *this;
}

template <typename D>
bool TEntityFactory<D>::autoenable_created_entities() const
{
    return this->delegate().auto_enable();
}

template <typename D>
TEntityFactory<D> TEntityFactory<D>::AutoEnable()
{
    return TEntityFactory(true);
}

template <typename D>
TEntityFactory<D> TEntityFactory<D>::ManuallyEnable()
{
    return TEntityFactory(false);
}

//TTransportPriority

template <typename D>
TTransportPriority<D>::TTransportPriority(int32_t prio) : dds::core::Value<D>(prio) { }

template <typename D>
TTransportPriority<D>::TTransportPriority() : dds::core::Value<D>(0) { }

template <typename D>
TTransportPriority<D>::TTransportPriority(const TTransportPriority& other) : dds::core::Value<D>(other.value()) { }

template <typename D>
TTransportPriority<D>& TTransportPriority<D>::value(int32_t prio)
{
    this->delegate().value(prio);
    return *this;
}

template <typename D>
int32_t TTransportPriority<D>::value() const
{
    return this->delegate().value();
}

//TLifeSpan

template <typename D>
TLifespan<D>::TLifespan(const dds::core::Duration& d) : dds::core::Value<D>(d) { }

template <typename D>
TLifespan<D>::TLifespan() : dds::core::Value<D>(dds::core::Duration::infinite()) { }

template <typename D>
TLifespan<D>::TLifespan(const TLifespan& other) : dds::core::Value<D>(other.duration()) { }

template <typename D>
TLifespan<D>& TLifespan<D>::duration(const dds::core::Duration& d)
{
    this->delegate().duration(d);
    return *this;
}

template <typename D>
const dds::core::Duration TLifespan<D>::duration() const
{
    return this->delegate().duration();
}

//TDeadline

template <typename D>
TDeadline<D>::TDeadline(const dds::core::Duration& d) : dds::core::Value<D>(d) { }

template <typename D>
TDeadline<D>::TDeadline(const TDeadline& other) : dds::core::Value<D>(other.period()) { }

template <typename D>
TDeadline<D>::TDeadline() : dds::core::Value<D>(dds::core::Duration::infinite()) { }

template <typename D>
TDeadline<D>& TDeadline<D>::period(const dds::core::Duration& d)
{
    this->delegate().period(d);
    return *this;
}

template <typename D>
const dds::core::Duration TDeadline<D>::period() const
{
    return this->delegate().period();
}

//TLatencyBudget

template <typename D>
TLatencyBudget<D>::TLatencyBudget(const dds::core::Duration& d) : dds::core::Value<D>(d) { }

template <typename D>
TLatencyBudget<D>::TLatencyBudget() : dds::core::Value<D>(dds::core::Duration::zero()) { }

template <typename D>
TLatencyBudget<D>::TLatencyBudget(const TLatencyBudget& other) : dds::core::Value<D>(other.duration()) { }

template <typename D>
TLatencyBudget<D>& TLatencyBudget<D>::duration(const dds::core::Duration& d)
{
    this->delegate().duration(d);
    return *this;
}

template <typename D>
const dds::core::Duration TLatencyBudget<D>::duration() const
{
    return this->delegate().duration();
}

//TTimeBasedFilter

template <typename D>
TTimeBasedFilter<D>::TTimeBasedFilter() : dds::core::Value<D>(dds::core::Duration::zero()) { }

template <typename D>
TTimeBasedFilter<D>::TTimeBasedFilter(const dds::core::Duration& the_min_separation) : dds::core::Value<D>(the_min_separation) { }

template <typename D>
TTimeBasedFilter<D>::TTimeBasedFilter(const TTimeBasedFilter& other) : dds::core::Value<D>(other.minimum_separation()) { }

template <typename D>
TTimeBasedFilter<D>& TTimeBasedFilter<D>::minimum_separation(const dds::core::Duration& ms)
{
    this->delegate().min_separation(ms);
    return *this;
}

template <typename D>
const dds::core::Duration TTimeBasedFilter<D>::minimum_separation() const
{
    return this->delegate().min_separation();
}

//TPartition

template <typename D>
TPartition<D>::TPartition(const std::string& partition) : dds::core::Value<D>(partition)
{

}

template <typename D>
TPartition<D>::TPartition(const dds::core::StringSeq& partitions) : dds::core::Value<D>(partitions)
{

}

template <typename D>
TPartition<D>::TPartition() : dds::core::Value<D>()
{
}

template <typename D>
TPartition<D>::TPartition(const TPartition& other) : dds::core::Value<D>(other.name())
{

}

template <typename D>
TPartition<D>&
TPartition<D>::name(const dds::core::StringSeq& partitions)
{
    this->delegate().name(partitions);
    return *this;
}

template <typename D>
const dds::core::StringSeq
TPartition<D>::name() const
{
    return this->delegate().name();
}

template <typename D>
dds::core::StringSeq&
TPartition<D>::name(dds::core::StringSeq& dst) const
{
    dst = this->delegate().name();
    return dst;
}

#ifdef OMG_DDS_OWNERSHIP_SUPPORT

//TOwnership

template <typename D>
TOwnership<D>::TOwnership(dds::core::policy::OwnershipKind::Type the_kind) : dds::core::Value<D>(the_kind) { }

template <typename D>
TOwnership<D>::TOwnership(const TOwnership& other) : dds::core::Value<D>(other.kind()) { }

template <typename D>
TOwnership<D>::TOwnership() : dds::core::Value<D>(dds::core::policy::OwnershipKind::SHARED) { }

template <typename D>
TOwnership<D>& TOwnership<D>::kind(dds::core::policy::OwnershipKind::Type the_kind)
{
    this->delegate().kind(the_kind);
    return *this;
}

template <typename D>
dds::core::policy::OwnershipKind::Type TOwnership<D>::kind() const
{
    return this->delegate().kind();
}

template <typename D>
TOwnership<D> TOwnership<D>::Exclusive()
{
    return TOwnership(dds::core::policy::OwnershipKind::EXCLUSIVE);
}

template <typename D>
TOwnership<D> TOwnership<D>::Shared()
{
    return TOwnership(dds::core::policy::OwnershipKind::SHARED);
}

//TOwnershipStrength

template <typename D>
TOwnershipStrength<D>::TOwnershipStrength(int32_t s) : dds::core::Value<D>(s) { }

template <typename D>
TOwnershipStrength<D>::TOwnershipStrength(const TOwnershipStrength& other) : dds::core::Value<D>(other.value()) { }

template <typename D>
int32_t TOwnershipStrength<D>::value() const
{
    return this->delegate().strength();
}

template <typename D>
TOwnershipStrength<D>& TOwnershipStrength<D>::value(int32_t s)
{
    this->delegate().strength(s);
    return *this;
}

#endif  // OMG_DDS_OWNERSHIP_SUPPORT

//TWriterDataLifeCycle

template <typename D>
TWriterDataLifecycle<D>::TWriterDataLifecycle() : dds::core::Value<D>(true) { }

template <typename D>
TWriterDataLifecycle<D>::TWriterDataLifecycle(bool the_autodispose) : dds::core::Value<D>(the_autodispose) { }

template <typename D>
TWriterDataLifecycle<D>::TWriterDataLifecycle(const TWriterDataLifecycle& other) : dds::core::Value<D>(other.autodispose_unregistered_instances()) { }

template <typename D>
bool TWriterDataLifecycle<D>::autodispose_unregistered_instances() const
{
    return this->delegate().autodispose();
}

template <typename D>
TWriterDataLifecycle<D>& TWriterDataLifecycle<D>::autodispose_unregistered_instances(bool b)
{
    this->delegate().autodispose(b);
    return *this;
}

template <typename D>
TWriterDataLifecycle<D> TWriterDataLifecycle<D>::AutoDisposeUnregisteredInstances()
{
    return TWriterDataLifecycle(true);
}

template <typename D>
TWriterDataLifecycle<D> TWriterDataLifecycle<D>::ManuallyDisposeUnregisteredInstances()
{
    return TWriterDataLifecycle(false);
}

//TReaderDataLifecycle

template <typename D>
TReaderDataLifecycle<D>::TReaderDataLifecycle() : dds::core::Value<D>(dds::core::Duration::infinite(), dds::core::Duration::infinite()) { }

template <typename D>
TReaderDataLifecycle<D>::TReaderDataLifecycle(const dds::core::Duration& the_nowriter_delay, const dds::core::Duration& the_disposed_samples_delay)
    : dds::core::Value<D>(the_nowriter_delay, the_disposed_samples_delay) { }

template <typename D>
TReaderDataLifecycle<D>::TReaderDataLifecycle(const TReaderDataLifecycle& other)
    : dds::core::Value<D>(other.autopurge_nowriter_samples_delay(), other.autopurge_disposed_samples_delay()) { }

template <typename D>
const dds::core::Duration TReaderDataLifecycle<D>::autopurge_nowriter_samples_delay() const
{
    return this->delegate().autopurge_nowriter_samples_delay();
}

template <typename D>
TReaderDataLifecycle<D>& TReaderDataLifecycle<D>::autopurge_nowriter_samples_delay(const dds::core::Duration& d)
{
    this->delegate().autopurge_nowriter_samples_delay(d);
    return *this;
}

template <typename D>
const dds::core::Duration TReaderDataLifecycle<D>::autopurge_disposed_samples_delay() const
{
    return this->delegate().autopurge_disposed_samples_delay();
}

template <typename D>
TReaderDataLifecycle<D>& TReaderDataLifecycle<D>::autopurge_disposed_samples_delay(const dds::core::Duration& d)
{
    this->delegate().autopurge_disposed_samples_delay(d);
    return *this;
}

template <typename D>
TReaderDataLifecycle<D> TReaderDataLifecycle<D>::NoAutoPurgeDisposedSamples()
{
    return TReaderDataLifecycle();
}

template <typename D>
TReaderDataLifecycle<D> TReaderDataLifecycle<D>::AutoPurgeDisposedSamples(const dds::core::Duration& d)
{
    return TReaderDataLifecycle().autopurge_disposed_samples_delay(d);
}

//TDurability

template <typename D>
TDurability<D>::TDurability(dds::core::policy::DurabilityKind::Type the_kind) : dds::core::Value<D>(the_kind) { }

template <typename D>
TDurability<D>::TDurability(const TDurability& other) : dds::core::Value<D>(other.kind()) { }

template <typename D>
TDurability<D>::TDurability() : dds::core::Value<D>(dds::core::policy::DurabilityKind::VOLATILE) { }

template <typename D>
TDurability<D>& TDurability<D>::kind(dds::core::policy::DurabilityKind::Type the_kind)
{
    this->delegate().kind(the_kind);
    return *this;
}

template <typename D>
dds::core::policy::DurabilityKind::Type TDurability<D>::kind() const
{
    return this->delegate().kind();
}

template <typename D>
TDurability<D> TDurability<D>::Volatile()
{
    return TDurability(dds::core::policy::DurabilityKind::VOLATILE);
}

template <typename D>
TDurability<D> TDurability<D>::TransientLocal()
{
    return TDurability(dds::core::policy::DurabilityKind::TRANSIENT_LOCAL);
}

template <typename D>
TDurability<D> TDurability<D>::Transient()
{
    return TDurability(dds::core::policy::DurabilityKind::TRANSIENT);
}

template <typename D>
TDurability<D> TDurability<D>::Persistent()
{
    return TDurability(dds::core::policy::DurabilityKind::PERSISTENT);
}

//TPresentation

template <typename D>
TPresentation<D>::TPresentation() : dds::core::Value<D>(dds::core::policy::PresentationAccessScopeKind::INSTANCE, false, false) { }

template <typename D>
TPresentation<D>::TPresentation(dds::core::policy::PresentationAccessScopeKind::Type the_access_scope, bool the_coherent_access, bool the_ordered_access)
    : dds::core::Value<D>(the_access_scope, the_coherent_access, the_ordered_access) { }

template <typename D>
TPresentation<D>::TPresentation(const TPresentation& other)
    : dds::core::Value<D>(other.access_scope(), other.coherent_access(), other.ordered_access()) { }

template <typename D>
TPresentation<D>& TPresentation<D>::access_scope(dds::core::policy::PresentationAccessScopeKind::Type  as)
{
    this->delegate().access_scope(as);
    return *this;
}

template <typename D>
dds::core::policy::PresentationAccessScopeKind::Type TPresentation<D>::access_scope() const
{
    return this->delegate().access_scope();
}

template <typename D>
TPresentation<D>& TPresentation<D>::coherent_access(bool on)
{
    this->delegate().coherent_access(on);
    return *this;
}

template <typename D>
bool TPresentation<D>::coherent_access() const
{
    return this->delegate().coherent_access();
}

template <typename D>
TPresentation<D>& TPresentation<D>::ordered_access(bool on)
{
    this->delegate().ordered_access(on);
    return *this;
}

template <typename D>
bool TPresentation<D>::ordered_access() const
{
    return this->delegate().ordered_access();
}

template <typename D>
TPresentation<D> TPresentation<D>::GroupAccessScope(bool coherent, bool ordered)
{
    return TPresentation(dds::core::policy::PresentationAccessScopeKind::GROUP, coherent, ordered);
}

template <typename D>
TPresentation<D> TPresentation<D>::InstanceAccessScope(bool coherent, bool ordered)
{
    return TPresentation(dds::core::policy::PresentationAccessScopeKind::INSTANCE, coherent, ordered);
}

template <typename D>
TPresentation<D> TPresentation<D>::TopicAccessScope(bool coherent, bool ordered)
{
    return TPresentation(dds::core::policy::PresentationAccessScopeKind::TOPIC, coherent, ordered);
}

//TReliability

template <typename D>
TReliability<D>::TReliability() : dds::core::Value<D>(dds::core::policy::ReliabilityKind::BEST_EFFORT, dds::core::Duration::zero()) { }

template <typename D>
TReliability<D>::TReliability(dds::core::policy::ReliabilityKind::Type the_kind, const dds::core::Duration& the_max_blocking_time)
    :  dds::core::Value<D>(the_kind, the_max_blocking_time) { }

template <typename D>
TReliability<D>::TReliability(const TReliability& other)
    : dds::core::Value<D>(other.kind(), other.max_blocking_time()) { }

template <typename D>
TReliability<D>& TReliability<D>::kind(dds::core::policy::ReliabilityKind::Type the_kind)
{
    this->delegate().kind(the_kind);
    return *this;
}

template <typename D>
dds::core::policy::ReliabilityKind::Type TReliability<D>::kind() const
{
    return this->delegate().kind();
}

template <typename D>
TReliability<D>& TReliability<D>::max_blocking_time(const dds::core::Duration& d)
{
    this->delegate().max_blocking_time(d);
    return *this;
}

template <typename D>
const dds::core::Duration TReliability<D>::max_blocking_time() const
{
    return this->delegate().max_blocking_time();
}

template <typename D>
TReliability<D> TReliability<D>::Reliable(const dds::core::Duration& d)
{
    return TReliability(dds::core::policy::ReliabilityKind::RELIABLE, d);
}

template <typename D>
TReliability<D> TReliability<D>::BestEffort()
{
    return TReliability(dds::core::policy::ReliabilityKind::BEST_EFFORT,
                        dds::core::Duration::zero());
}

//TDestinationOrder

template <typename D>
TDestinationOrder<D>::TDestinationOrder(dds::core::policy::DestinationOrderKind::Type the_kind)
    : dds::core::Value<D>(the_kind) { }

template <typename D>
TDestinationOrder<D>::TDestinationOrder(const TDestinationOrder& other) : dds::core::Value<D>(other.kind()) { }

template <typename D>
TDestinationOrder<D>::TDestinationOrder() : dds::core::Value<D>(dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP) { }

template <typename D>
TDestinationOrder<D>& TDestinationOrder<D>::kind(dds::core::policy::DestinationOrderKind::Type the_kind)
{
    this->delegate().kind(the_kind);
    return *this;
}

template <typename D>
dds::core::policy::DestinationOrderKind::Type TDestinationOrder<D>::kind() const
{
    return this->delegate().kind();
}

template <typename D>
TDestinationOrder<D> TDestinationOrder<D>::SourceTimestamp()
{
    return TDestinationOrder(dds::core::policy::DestinationOrderKind::BY_SOURCE_TIMESTAMP);
}

template <typename D>
TDestinationOrder<D> TDestinationOrder<D>::ReceptionTimestamp()
{
    return TDestinationOrder(dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP);
}

//THistory

template <typename D>
THistory<D>::THistory() : dds::core::Value<D>(dds::core::policy::HistoryKind::KEEP_LAST, 1) { }

template <typename D>
THistory<D>::THistory(dds::core::policy::HistoryKind::Type the_kind, int32_t the_depth)
    : dds::core::Value<D>(the_kind, the_depth) { }

template <typename D>
THistory<D>::THistory(const THistory& other) : dds::core::Value<D>(other.kind(), other.depth()) { }

template <typename D>
dds::core::policy::HistoryKind::Type THistory<D>::kind() const
{
    return this->delegate().kind();
}

template <typename D>
THistory<D>& THistory<D>::kind(dds::core::policy::HistoryKind::Type the_kind)
{
    this->delegate().kind(the_kind);
    return *this;
}

template <typename D>
int32_t THistory<D>::depth() const
{
    return this->delegate().depth();
}

template <typename D>
THistory<D>& THistory<D>::depth(int32_t the_depth)
{
    this->delegate().depth(the_depth);
    return *this;
}

template <typename D>
THistory<D> THistory<D>::KeepAll()
{
    return THistory(dds::core::policy::HistoryKind::KEEP_ALL, 1);
}

template <typename D>
THistory<D> THistory<D>::KeepLast(uint32_t depth)
{
    return THistory(dds::core::policy::HistoryKind::KEEP_LAST, depth);
}

//TResourceLimits

template <typename D>
TResourceLimits<D>::TResourceLimits() : dds::core::Value<D>(dds::core::LENGTH_UNLIMITED, dds::core::LENGTH_UNLIMITED, dds::core::LENGTH_UNLIMITED) { }

template <typename D>
TResourceLimits<D>::TResourceLimits(int32_t the_max_samples, int32_t the_max_instances, int32_t the_max_samples_per_instance)
    :  dds::core::Value<D>(the_max_samples, the_max_instances, the_max_samples_per_instance) { }

template <typename D>
TResourceLimits<D>::TResourceLimits(const TResourceLimits& other)
    : dds::core::Value<D>(other.max_samples(), other.max_instances(), other.max_samples_per_instance()) { }

template <typename D>
TResourceLimits<D>& TResourceLimits<D>::max_samples(int32_t samples)
{
    this->delegate().max_samples(samples);
    return *this;
}

template <typename D>
int32_t TResourceLimits<D>::max_samples() const
{
    return this->delegate().max_samples();
}

template <typename D>
TResourceLimits<D>& TResourceLimits<D>::max_instances(int32_t the_max_instances)
{
    this->delegate().max_instances(the_max_instances);
    return *this;
}

template <typename D>
int32_t TResourceLimits<D>::max_instances() const
{
    return this->delegate().max_instances();
}

template <typename D>
TResourceLimits<D>& TResourceLimits<D>::max_samples_per_instance(int32_t the_max_samples_per_instance)
{
    this->delegate().max_samples_per_instance(the_max_samples_per_instance);
    return *this;
}

template <typename D>
int32_t TResourceLimits<D>::max_samples_per_instance() const
{
    return this->delegate().max_samples_per_instance();
}

//TLiveliness

template <typename D>
TLiveliness<D>::TLiveliness() : dds::core::Value<D>(dds::core::policy::LivelinessKind::AUTOMATIC, dds::core::Duration::infinite()) { }

template <typename D>
TLiveliness<D>::TLiveliness(dds::core::policy::LivelinessKind::Type the_kind, const dds::core::Duration& the_lease_duration)
    : dds::core::Value<D>(the_kind, the_lease_duration) { }

template <typename D>
TLiveliness<D>::TLiveliness(const TLiveliness& other) : dds::core::Value<D>(other.kind(), other.lease_duration()) { }

template <typename D>
TLiveliness<D>& TLiveliness<D>::kind(dds::core::policy::LivelinessKind::Type the_kind)
{
    this->delegate().kind(the_kind);
    return *this;
}

template <typename D>
dds::core::policy::LivelinessKind::Type TLiveliness<D>::kind() const
{
    return this->delegate().kind();
}

template <typename D>
TLiveliness<D>& TLiveliness<D>::lease_duration(const dds::core::Duration& the_lease_duration)
{
    this->delegate().lease_duration(the_lease_duration);
    return *this;
}

template <typename D>
const dds::core::Duration TLiveliness<D>::lease_duration() const
{
    return this->delegate().lease_duration();
}

template <typename D>
TLiveliness<D> TLiveliness<D>::Automatic()
{
    return TLiveliness(dds::core::policy::LivelinessKind::AUTOMATIC, dds::core::Duration::infinite());
}

template <typename D>
TLiveliness<D> TLiveliness<D>::ManualByParticipant(const dds::core::Duration& lease)
{
    return TLiveliness(dds::core::policy::LivelinessKind::MANUAL_BY_PARTICIPANT, lease);
}

template <typename D>
TLiveliness<D> TLiveliness<D>::ManualByTopic(const dds::core::Duration& lease)
{
    return TLiveliness(dds::core::policy::LivelinessKind::MANUAL_BY_TOPIC, lease);
}

#ifdef OMG_DDS_PERSISTENCE_SUPPORT

//TDurabilityService

template <typename D>
TDurabilityService<D>::TDurabilityService() : dds::core::Value<D>() { }

template <typename D>
TDurabilityService<D>::TDurabilityService(
    const dds::core::Duration& the_service_cleanup_delay,
    dds::core::policy::HistoryKind::Type the_history_kind,
    int32_t the_history_depth,
    int32_t the_max_samples,
    int32_t the_max_instances,
    int32_t the_max_samples_per_instance)
    : dds::core::Value<D>(the_service_cleanup_delay,
                          the_history_kind,
                          the_history_depth,
                          the_max_samples,
                          the_max_instances,
                          the_max_samples_per_instance) { }

template <typename D>
TDurabilityService<D>::TDurabilityService(const TDurabilityService& other)
    : dds::core::Value<D>(other.service_cleanup_delay(),
                          other.history_kind(), other.history_depth(),
                          other.max_samples(), other.max_instances(), other.max_samples_per_instance()) { }

template <typename D>
TDurabilityService<D>& TDurabilityService<D>::service_cleanup_delay(const dds::core::Duration& d)
{
    this->delegate().service_cleanup_delay(d);
    return *this;
}

template <typename D>
const dds::core::Duration TDurabilityService<D>::service_cleanup_delay() const
{
    return this->delegate().service_cleanup_delay();
}

template <typename D>
TDurabilityService<D>& TDurabilityService<D>::history_kind(dds::core::policy::HistoryKind::Type the_kind)
{
    this->delegate().history_kind(the_kind);
    return *this;
}

template <typename D>
dds::core::policy::HistoryKind::Type TDurabilityService<D>::history_kind() const
{
    return this->delegate().history_kind();
}

template <typename D>
TDurabilityService<D>& TDurabilityService<D>::history_depth(int32_t the_depth)
{
    this->delegate().history_depth(the_depth);
    return *this;
}

template <typename D>
int32_t TDurabilityService<D>::history_depth() const
{
    return this->delegate().history_depth();
}

template <typename D>
TDurabilityService<D>& TDurabilityService<D>::max_samples(int32_t the_max_samples)
{
    this->delegate().max_samples(the_max_samples);
    return *this;
}

template <typename D>
int32_t TDurabilityService<D>::max_samples() const
{
    return this->delegate().max_samples();
}

template <typename D>
TDurabilityService<D>& TDurabilityService<D>::max_instances(int32_t the_max_instances)
{
    this->delegate().max_instances(the_max_instances);
    return *this;
}

template <typename D>
int32_t TDurabilityService<D>::max_instances() const
{
    return this->delegate().max_instances();
}

template <typename D>
TDurabilityService<D>& TDurabilityService<D>::max_samples_per_instance(int32_t the_max_samples_per_instance)
{
    this->delegate().max_samples_per_instance(the_max_samples_per_instance);
    return *this;
}

template <typename D>
int32_t TDurabilityService<D>::max_samples_per_instance() const
{
    return this->delegate().max_samples_per_instance();
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

//TDataRepresentation

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
TDataRepresentation<D>::TDataRepresentation(const dds::core::policy::DataRepresentationIdSeq& value);

template <typename D>
TDataRepresentation<D>::TDataRepresentation(const TDataRepresentation& other) : dds::core::Value<D>(other.value()) { }

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
TDataRepresentation<D>& TDataRepresentation<D>::value(const dds::core::policy::DataRepresentationIdSeq& value);

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
const dds::core::policy::DataRepresentationIdSeq TDataRepresentation<D>::value() const;

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
dds::core::policy::DataRepresentationIdSeq& TDataRepresentation<D>::value(dds::core::policy::DataRepresentationIdSeq& dst) const;

#endif  // defined(OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT)

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

//TTypeConsistencyEnforcement

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
TTypeConsistencyEnforcement<D>::TTypeConsistencyEnforcement(dds::core::policy::TypeConsistencyEnforcementKind::Type kind) { }

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
TTypeConsistencyEnforcement<D>& TTypeConsistencyEnforcement<D>::kind(dds::core::policy::TypeConsistencyEnforcementKind::Type  value) { }

/** @bug OSPL-1746 No implementation
 * @todo Implementation required - see OSPL-1746
 * @see http://jira.prismtech.com:8080/browse/OSPL-1746 */
template <typename D>
dds::core::policy::TypeConsistencyEnforcementKind::Type TTypeConsistencyEnforcement<D>::kind() const { }

#endif  // defined(OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT)

}
}
}

// End of implementation
// OMG PSM class declaration

#endif /* OSPL_DDS_CORE_POLICY_TCOREPOLICY_HPP_ */
// OMG PSM class declaration
