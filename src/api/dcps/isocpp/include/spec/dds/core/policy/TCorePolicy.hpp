#ifndef OMG_TDDS_CORE_POLICY_CORE_POLICY_HPP_
#define OMG_TDDS_CORE_POLICY_CORE_POLICY_HPP_

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
#include <dds/core/LengthUnlimited.hpp>
#include <dds/core/Value.hpp>
#include <dds/core/policy/PolicyKind.hpp>

//==============================================================================
// DDS Policy Classes
namespace dds { namespace core { namespace policy {

  //==============================================================================
  /**
   * The purpose of this QoS is to allow the application to attach additional
   * information to the created Entity objects such that when a remote application
   * discovers their existence it can access that information and use it for its
   * own purposes. One possible use of this QoS is to attach security credentials
   * or some other information that can be used by the remote application to
   * authenticate the source. In combination with operations such as
   * ignore_participant, ignore_publication, ignore_subscription,
   * and ignore_topic these QoS can assist an application to define and enforce
   * its own security policies. The use of this QoS is not limited to security,
   * rather it offers a simple, yet flexible extensibility mechanism.
   */
  template <typename D>
  class TUserData : public dds::core::Value<D> {
  public:
    /**
     * Create a <code>UserData</code> instance with an empty user data.
     */
    TUserData();

    /**
     * Create a <code>UserData</code> instance.
     *
     * @param seq the sequence of octet representing the user data
     */
    explicit TUserData(const dds::core::ByteSeq& seq);

    /** @todo Implement this */
    TUserData(const uint8_t* value_begin, const uint8_t* value_end);

    TUserData(const TUserData& other);

  public:
    /**
     * Set the value for the user data.
     *
     * @param seq a sequence of bytes representing the user data.
     */
    TUserData& value(const dds::core::ByteSeq& seq);

    /**
     * Set the value for the user data.
     */
    template <typename OCTET_ITER>
    TUserData& value(OCTET_ITER begin, OCTET_ITER end);

    /**
     * Get the user data.
     *
     * @return the sequence of bytes representing the user data
     */
    const dds::core::ByteSeq value() const;

    const uint8_t* begin() const;
    const uint8_t* end() const;
  };

  //==============================================================================

  /**
   * The purpose of this QoS is to allow the application to attach additional
   * information to the created Publisher or Subscriber.
   * The value of the GROUP_DATA is available to the application on the
   * DataReader and DataWriter entities and is propagated by means of the
   * built-in topics. This QoS can be used by an application combination with
   * the DataReaderListener and DataWriterListener to implement matching policies
   * similar to those of the PARTITION QoS except the decision can be made based
   * on an application-defined policy.
   */
  template <typename D>
  class TGroupData : public dds::core::Value<D> {
  public:
    /**
     * Create a <code>GroupData</code> instance.
     */
    TGroupData();

    /**
     * Create a <code>GroupData</code> instance.
     *
     * @param seq the group data value
     */
    explicit TGroupData(const dds::core::ByteSeq& seq);

    TGroupData(const TGroupData& other);

    TGroupData(const uint8_t* value_begin, const uint8_t* value_end);

  public:
    /**
     * Set the value for this <code>GroupData</code>
     *
     * @param seq the group data value
     */
    TGroupData& value(const dds::core::ByteSeq& seq);

    /**
     * Set the value for this <code>GroupData</code>
     */
    template <typename OCTET_ITER>
    TGroupData& value(OCTET_ITER begin, OCTET_ITER end);

    /**
     * Get the value for this <code>GroupData</code>
     *
     * @return  the group data value
     */
    const dds::core::ByteSeq value() const;

    /**
     * Get the value for this <code>GroupData</code>
     */
    dds::core::ByteSeq& value(dds::core::ByteSeq& dst) const;

    const uint8_t* begin() const;
    const uint8_t* end() const;
  };

  //==============================================================================

  /**
   * The purpose of this QoS is to allow the application to attach additional
   * information to the created Topic such that when a remote application
   * discovers their existence it can examine the information and use it in
   * an application-defined way. In combination with the listeners on the
   * DataReader and DataWriter as well as by means of operations such as
   * ignore_topic, these QoS can assist an application to extend the provided QoS.
   */
  template <typename D>
  class TTopicData : public dds::core::Value<D> {
  public:
    TTopicData();

    explicit TTopicData(const dds::core::ByteSeq& seq);

    TTopicData(const TTopicData& other);

    TTopicData(const uint8_t* value_begin, const uint8_t* value_end);

  public:
    /**
     * Set the value for the topic data.
     *
     * @param seq a sequence of bytes representing the topic data.
     */
    TTopicData& value(const dds::core::ByteSeq& seq);

    /**
     * Set the value for the topic data.
     */
    template <typename OCTET_ITER>
    TTopicData& value(OCTET_ITER begin, OCTET_ITER end);

    /**
     * Get the topic data.
     *
     * @return the sequence of bytes representing the topic data
     */
    const dds::core::ByteSeq value() const;

    /**
     * Get the topic data.
     */
    dds::core::ByteSeq& value(dds::core::ByteSeq& dst) const;

    const uint8_t* begin() const;
    const uint8_t* end() const;
  };


  //==============================================================================

  /**
   * This policy controls the behavior of the Entity as a factory for other
   * entities. This policy concerns only DomainParticipant (as factory for
   * Publisher, Subscriber, and Topic), Publisher (as factory for DataWriter),
   * and Subscriber (as factory for DataReader). This policy is mutable.
   * A change in the policy affects only the entities created after the change;
   * not the previously created entities.
   * The setting of autoenable_created_entities to TRUE indicates that the
   * newly created object will be enabled by default.
   * A setting of FALSE indicates that the Entity will not be automatically
   * enabled. The application will need to enable it explicitly by means of the
   * enable operation (see Section 7.1.2.1.1.7, "enable"). The default setting
   * of autoenable_created_entities = TRUE means that, by default, it is not
   * necessary to explicitly call enable on newly created entities.
   */
  template <typename D>
  class TEntityFactory : public dds::core::Value<D> {
  public:
    TEntityFactory();
    explicit TEntityFactory(bool the_auto_enable);
    TEntityFactory(const TEntityFactory& other);

  public:
    TEntityFactory& autoenable_created_entities(bool on);
    bool autoenable_created_entities() const;

  public:
    static TEntityFactory AutoEnable();
    static TEntityFactory ManuallyEnable();
  };

  //==============================================================================

  /**
   * The purpose of this QoS is to allow the application to take advantage of
   * transports capable of sending messages with different priorities.
   * This policy is considered a hint. The policy depends on the ability of the
   * underlying transports to set a priority on the messages they send.
   * Any value within the range of a 32-bit signed integer may be chosen;
   * higher values indicate higher priority. However, any further interpretation
   * of this policy is specific to a particular transport and a particular
   * implementation of the Service. For example, a particular transport is
   * permitted to treat a range of priority values as equivalent to one another.
   * It is expected that during transport configuration the application would
   * provide a mapping between the values of the TRANSPORT_PRIORITY set on
   * DataWriter and the values meaningful to each transport. This mapping would
   * then be used by the infrastructure when propagating the data written by
   * the DataWriter.
   */
  template <typename D>
  class TTransportPriority : public dds::core::Value<D> {
  public:
    explicit TTransportPriority(int32_t prio);

    TTransportPriority();

    TTransportPriority(const TTransportPriority& other);

  public:
    TTransportPriority& value(int32_t prio);
    int32_t value() const;
  };

  //==============================================================================

  /**
   * The purpose of this QoS is to avoid delivering "stale" data to the
   * application. Each data sample written by the DataWriter has an associated
   * expiration time beyond which the data should not be delivered to any
   * application. Once the sample expires, the data will be removed from the
   * DataReader caches as well as from the transient and persistent
   * information caches. The expiration time of each sample is computed by
   * adding the duration specified by the LIFESPAN QoS to the source timestamp.
   * As described in Section 7.1.2.4.2.11, write and Section 7.1.2.4.2.12,
   * write_w_timestamp the source timestamp is either automatically computed by
   * the Service each time the DataWriter write operation is called, or else
   * supplied by the application by means of the write_w_timestamp operation.
   *
   * This QoS relies on the sender and receiving applications having their clocks
   * sufficiently synchronized. If this is not the case and the Service can
   * detect it, the DataReader is allowed to use the reception timestamp instead
   * of the source timestamp in its computation of the expiration time.
   */
  template <typename D>
  class TLifespan : public dds::core::Value<D> {
  public:
    /**
     * Create a lifespan with a specified duration.
     */
    explicit TLifespan(const dds::core::Duration& d);

    /**
     * Create a <code>Lifespan</code> with infinite duration.
     */
    TLifespan();
    TLifespan(const TLifespan& other);

  public:
    TLifespan& duration(const dds::core::Duration& d);
    const dds::core::Duration duration() const;
  };

  //==============================================================================

  /**
   * This policy is useful for cases where a Topic is expected to have each
   * instance updated periodically. On the publishing side this setting
   * establishes a contract that the application must meet. On the subscribing
   * side the setting establishes a minimum requirement for the remote publishers
   * that are expected to supply the data values. When the Service matches a
   * DataWriter and a DataReader it checks whether the settings are compatible
   * (i.e., offered deadline period<= requested deadline period) if they are not,
   * the two entities are informed (via the listener or condition mechanism)
   * of the incompatibility of the QoS settings and communication will not occur.
   * Assuming that the reader and writer ends have compatible settings, the
   * fulfillment of this contract is monitored by the Service and the application
   * is informed of any violations by means of the proper listener or condition.
   * The value offered is considered compatible with the value requested if and
   * only if the inequality "offered deadline period <= requested deadline period"
   * evaluates to TRUE. The setting of the DEADLINE policy must be set
   * consistently with that of the TIME_BASED_FILTER.
   * For these two policies to be consistent the settings must be such that
   * "deadline period>= minimum_separation".
   */
  template <typename D>
  class TDeadline : public dds::core::Value<D> {
  public:
    explicit TDeadline(const dds::core::Duration& d);

    TDeadline(const TDeadline& other);

    /**
     * Create a deadline with infinite period.
     */
    TDeadline();

  public:
    TDeadline& period(const dds::core::Duration& d);
    const dds::core::Duration period() const;
  };

  //==============================================================================

  template <typename D>
  class TLatencyBudget : public dds::core::Value<D> {
  public:
    explicit TLatencyBudget(const dds::core::Duration& d);

    /**
     * Create a latency budget with zero duration.
     */
    TLatencyBudget();
    TLatencyBudget(const TLatencyBudget& other);

  public:
    TLatencyBudget& duration(const dds::core::Duration& d);
    const dds::core::Duration duration() const;
  };

  //==============================================================================
  template <typename D>
  class TTimeBasedFilter : public dds::core::Value<D> {
  public:
    /**
     * Create a time based filter with infinite duration.
     */
    TTimeBasedFilter();
    explicit TTimeBasedFilter(const dds::core::Duration& the_min_separation);
    TTimeBasedFilter(const TTimeBasedFilter& other);

  public:
    TTimeBasedFilter& minimum_separation(const dds::core::Duration& ms);
    const dds::core::Duration minimum_separation() const;
  };


  //==============================================================================
  template <typename D>
  class TPartition : public dds::core::Value<D> {
  public:
    TPartition();
    explicit TPartition(const std::string& partition);
    explicit TPartition(const dds::core::StringSeq& partitions);
    TPartition(const TPartition& other);

  public:
    TPartition& name(const dds::core::StringSeq& partitions);

    const dds::core::StringSeq name() const;

    dds::core::StringSeq& name(dds::core::StringSeq& dst) const;
  };

  //==============================================================================
#ifdef OMG_DDS_OWNERSHIP_SUPPORT

  template <typename D>
  class TOwnership : public dds::core::Value<D> {
  public:
    /**
     * Create an ownership policy set to shared.
     */
    TOwnership();
    explicit TOwnership(dds::core::policy::OwnershipKind::Type the_kind);
    TOwnership(const TOwnership& other);



  public:
    TOwnership& kind(dds::core::policy::OwnershipKind::Type the_kind);
    dds::core::policy::OwnershipKind::Type kind() const;

  public:
    static TOwnership Exclusive();
    static TOwnership Shared();
  };


  //==============================================================================


  template <typename D>
  class TOwnershipStrength : public dds::core::Value<D> {
  public:
    explicit TOwnershipStrength(int32_t s);

    TOwnershipStrength(const TOwnershipStrength& other);

  public:
    int32_t value() const;
    TOwnershipStrength& value(int32_t s);
  };

#endif  // OMG_DDS_OWNERSHIP_SUPPORT


  //==============================================================================

  template <typename D>
  class TWriterDataLifecycle : public dds::core::Value<D> {
  public:
    TWriterDataLifecycle();
    explicit TWriterDataLifecycle(bool the_autodispose);
    TWriterDataLifecycle(const TWriterDataLifecycle& other);

  public:
    bool autodispose_unregistered_instances() const;
    TWriterDataLifecycle& autodispose_unregistered_instances(bool b);

  public:
    static TWriterDataLifecycle AutoDisposeUnregisteredInstances();
    static TWriterDataLifecycle ManuallyDisposeUnregisteredInstances();
  };

  template <typename D>
  class TReaderDataLifecycle : public dds::core::Value<D> {
  public:
    /**
     * Create a reder data lifecycle with infinite delays for no-writer
     * and disposed-sample.
     */
    TReaderDataLifecycle();
    TReaderDataLifecycle(const dds::core::Duration& the_nowriter_delay,
        const dds::core::Duration& the_disposed_samples_delay);
    TReaderDataLifecycle(const TReaderDataLifecycle& other);
  public:
    const dds::core::Duration autopurge_nowriter_samples_delay() const;
    TReaderDataLifecycle& autopurge_nowriter_samples_delay(const dds::core::Duration& d);

    const dds::core::Duration autopurge_disposed_samples_delay() const;
    TReaderDataLifecycle& autopurge_disposed_samples_delay(const dds::core::Duration& d);

  public:
    static TReaderDataLifecycle NoAutoPurgeDisposedSamples();
    static TReaderDataLifecycle AutoPurgeDisposedSamples(const dds::core::Duration& d);
  };

  //==============================================================================
  template <typename D>
  class TDurability : public dds::core::Value<D> {
  public:
    /**
     * Create a volatile durability.
     */
    TDurability();
    explicit TDurability(dds::core::policy::DurabilityKind::Type the_kind);
    TDurability(const TDurability& other);

  public:
    TDurability& kind(dds::core::policy::DurabilityKind::Type the_kind);
    dds::core::policy::DurabilityKind::Type  kind() const;

  public:
    static TDurability Volatile();
    static TDurability TransientLocal();
    static TDurability Transient();
    static TDurability Persistent();
  };
  //==============================================================================
  template <typename D>
  class TPresentation : public dds::core::Value<D> {
  public:
    /**
     * Create a presentation policy with <code>Instance</code> access scope
     * and with coherent and ordered access set to false.
     *
     */
    TPresentation();
    TPresentation(dds::core::policy::PresentationAccessScopeKind::Type the_access_scope,
        bool the_coherent_access,
        bool the_ordered_access);

    TPresentation(const TPresentation& other);


  public:
    TPresentation& access_scope(dds::core::policy::PresentationAccessScopeKind::Type  as);
    dds::core::policy::PresentationAccessScopeKind::Type  access_scope() const;

    TPresentation& coherent_access(bool on);
    bool coherent_access() const;

    TPresentation& ordered_access(bool on);
    bool ordered_access() const;

  public:
    static TPresentation GroupAccessScope(bool coherent = false, bool ordered = false);
    static TPresentation InstanceAccessScope(bool coherent = false, bool ordered = false);
    static TPresentation TopicAccessScope(bool coherent = false, bool ordered = false);
  };

  //==============================================================================

  template <typename D>
  class TReliability : public dds::core::Value<D> {
  public:
    /**
     * Create a best effort reliability policy.
     */
    TReliability();
    TReliability(dds::core::policy::ReliabilityKind::Type the_kind,
        const dds::core::Duration& the_max_blocking_time);
    TReliability(const TReliability& other);

  public:

    TReliability& kind(dds::core::policy::ReliabilityKind::Type the_kind);
    dds::core::policy::ReliabilityKind::Type  kind() const;

    TReliability& max_blocking_time(const dds::core::Duration& d);
    const dds::core::Duration max_blocking_time() const;

  public:
    static TReliability Reliable(const dds::core::Duration& d = dds::core::Duration(0, 100000000));
    static TReliability BestEffort();
  };


  //==============================================================================

  template <typename D>
  class TDestinationOrder : public dds::core::Value<D> {
  public:
    /**
     * Create a by-source-timestamp destination order policy.
     */
    TDestinationOrder();
    explicit TDestinationOrder(dds::core::policy::DestinationOrderKind::Type the_kind);

    TDestinationOrder(const TDestinationOrder& other);

  public:
    TDestinationOrder& kind(dds::core::policy::DestinationOrderKind::Type the_kind);
    dds::core::policy::DestinationOrderKind::Type  kind() const;

  public:
    static TDestinationOrder SourceTimestamp();
    static TDestinationOrder ReceptionTimestamp();
  };

  //==============================================================================
  template <typename D>
  class THistory : public dds::core::Value<D> {
  public:
    /**
     * Creates a keep-last one history policy.
     */
    THistory();
    THistory(dds::core::policy::HistoryKind::Type the_kind, int32_t the_depth);
    THistory(const THistory& other);

  public:
    dds::core::policy::HistoryKind::Type  kind() const;
    THistory& kind(dds::core::policy::HistoryKind::Type the_kind);

    int32_t depth() const;
    THistory& depth(int32_t the_depth);

  public:
    static THistory KeepAll();
    static THistory KeepLast(uint32_t depth);
  };

  //==============================================================================

  template <typename D>
  class TResourceLimits : public dds::core::Value<D> {
  public:
    /**
     * Create unlimited resource limit policy;
     */
    TResourceLimits();
    TResourceLimits(int32_t the_max_samples,
        int32_t the_max_instances,
        int32_t the_max_samples_per_instance);
    TResourceLimits(const TResourceLimits& other);

  public:
    TResourceLimits& max_samples(int32_t samples);
    int32_t max_samples() const;

    TResourceLimits& max_instances(int32_t the_max_instances);
    int32_t max_instances() const;

    TResourceLimits& max_samples_per_instance(int32_t the_max_samples_per_instance);
    int32_t max_samples_per_instance() const;
  };



  //==============================================================================

  template <typename D>
  class TLiveliness : public dds::core::Value<D> {
  public:
    /**
     * Create automatic liveliness policy with infinite lease duration.
     */
    TLiveliness();
    TLiveliness(dds::core::policy::LivelinessKind::Type the_kind,
        const dds::core::Duration& the_lease_duration);
    TLiveliness(const TLiveliness& other);

  public:
    TLiveliness& kind(dds::core::policy::LivelinessKind::Type the_kind);
    dds::core::policy::LivelinessKind::Type kind() const;

    TLiveliness& lease_duration(const dds::core::Duration& the_lease_duration);
    const dds::core::Duration lease_duration() const;

  public:
    static TLiveliness Automatic();
    static TLiveliness ManualByParticipant(const dds::core::Duration& lease = dds::core::Duration::infinite());
    static TLiveliness ManualByTopic(const dds::core::Duration& lease = dds::core::Duration::infinite());
  };


  //==============================================================================

#ifdef OMG_DDS_PERSISTENCE_SUPPORT

  template <typename D>
  class TDurabilityService : public dds::core::Value<D> {
  public:
    TDurabilityService();

    TDurabilityService(
        const dds::core::Duration& the_service_cleanup_delay,
        dds::core::policy::HistoryKind::Type the_history_kind,
        int32_t the_history_depth,
        int32_t the_max_samples,
        int32_t the_max_instances,
        int32_t the_max_samples_per_instance);

    TDurabilityService(const TDurabilityService& other);


  public:
    TDurabilityService& service_cleanup_delay(const dds::core::Duration& d);
    const dds::core::Duration service_cleanup_delay() const;

    TDurabilityService& history_kind(dds::core::policy::HistoryKind::Type the_kind);
    dds::core::policy::HistoryKind::Type history_kind() const;

    TDurabilityService& history_depth(int32_t the_depth);

    int32_t history_depth() const;

    TDurabilityService& max_samples(int32_t the_max_samples);
    int32_t max_samples() const;

    TDurabilityService& max_instances(int32_t the_max_instances);
    int32_t max_instances() const;

    TDurabilityService& max_samples_per_instance(int32_t the_max_samples_per_instance);
    int32_t max_samples_per_instance() const;
  };

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


  //============================================================================

  //============================================================================

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

  template <typename D>
  class TDataRepresentation : public dds::core::Value<D> {

  public:
    explicit TDataRepresentation(
        const dds::core::policy::DataRepresentationIdSeq& value);

    TDataRepresentation(const TDataRepresentation& other)
    : dds::core::Value<D>(other.value())
      { }
  public:
    TDataRepresentation& value(const dds::core::policy::DataRepresentationIdSeq& value);

    const dds::core::policy::DataRepresentationIdSeq value() const;

    dds::core::policy::DataRepresentationIdSeq&
    value(dds::core::policy::DataRepresentationIdSeq& dst) const;
  };

#endif  // defined(OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT)


  //============================================================================

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

  template <typename D>
  class TTypeConsistencyEnforcement : public dds::core::Value<D> {
  public:
    explicit TTypeConsistencyEnforcement(dds::core::policy::TypeConsistencyEnforcementKind::Type kind);

  public:
    TTypeConsistencyEnforcement& kind(dds::core::policy::TypeConsistencyEnforcementKind::Type  value);
    dds::core::policy::TypeConsistencyEnforcementKind::Type  kind() const;
  };

#endif  // defined(OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT)


  //==============================================================================


} } }

#endif /* OMG_TDDS_CORE_POLICY_CORE_POLICY_HPP_ */
