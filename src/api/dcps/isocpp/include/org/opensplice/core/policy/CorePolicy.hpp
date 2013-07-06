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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_POLICY_CORE_POLICY_IMPL_HPP_
#define ORG_OPENSPLICE_CORE_POLICY_CORE_POLICY_IMPL_HPP_

#include <dds/core/types.hpp>
#include <dds/core/LengthUnlimited.hpp>
#include <dds/core/Duration.hpp>

//==============================================================================
// DDS Policy Classes
namespace org { namespace opensplice { namespace core { namespace policy {

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
    class UserData {
    public:
      /**
       * Create a <code>UserData</code> instance with an empty user data.
       */
      UserData() : value_() { }

      /**
       * Create a <code>UserData</code> instance.
       *
       * @param seq the sequence of octet representing the user data
       */
      explicit UserData(const dds::core::ByteSeq& seq) : value_(seq) { }

      /**
       * Set the value for the user data.
       *
       * @param seq a sequence of octet representing the user data.
       */
      void value(const dds::core::ByteSeq& seq) {
        value_ = seq;
      }

      /**
       * Get/Set the user data.
       *
       * @return the sequence of octet representing the user data
       */
      dds::core::ByteSeq& value() {
        return value_;
      }
      /**
       * Get the user data.
       *
       * @return the sequence of octet representing the user data
       */
      const dds::core::ByteSeq& value() const {
        return value_;
      }

    bool operator ==(const UserData& other) const
    {
            return other.value() == value_;
    }
    private:
      dds::core::ByteSeq value_;
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
    class GroupData {
    public:
      /**
       * Create a <code>GroupData</code> instance.
       */
      GroupData() : value_() { }

      /**
       * Create a <code>GroupData</code> instance.
       *
       * @param seq the group data value
       */
      explicit GroupData(const dds::core::ByteSeq& seq) : value_(seq) { }

      /**
       * Set the value for this <code>GroupData</code>
       *
       * @param seq the group data value
       */
      void value(const dds::core::ByteSeq& seq) {
        value_ = seq;
      }
      /**
       * Get/Set the value for this <code>GroupData</code>
       *
       * @return  the group data value
       */
      dds::core::ByteSeq& value() {
        return value_;
      }

      /**
       * Get the value for this <code>GroupData</code>
       *
       * @return  the group data value
       */
      const dds::core::ByteSeq& value() const {
        return value_;
      }

    bool operator ==(const GroupData& other) const
    {
            return other.value() == value_;
    }
    private:
      dds::core::ByteSeq value_;
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
    class TopicData {
    public:
      TopicData() : value_() { }

      explicit TopicData(const dds::core::ByteSeq& seq) : value_(seq) { }

      void value(const dds::core::ByteSeq& seq) {
        value_ = seq;
      }

      const dds::core::ByteSeq& value() const {
        return value_;
      }

      dds::core::ByteSeq& value() {
        return value_;
      }
    bool operator ==(const TopicData& other) const
    {
            return other.value() == value_;
    }
    private:
      dds::core::ByteSeq value_;

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
     * enable operation (see Section 7.1.2.1.1.7, “enable”). The default setting
     * of autoenable_created_entities = TRUE means that, by default, it is not
     * necessary to explicitly call enable on newly created entities.
     */
    class EntityFactory {
    public:
      EntityFactory() : auto_enable_(true) { }

      explicit EntityFactory(bool the_auto_enable)
      : auto_enable_(the_auto_enable) { }

      void auto_enable(bool on) {
        auto_enable_ = on;
      }
      bool auto_enable() const {
        return auto_enable_;
      }

      bool& auto_enable() {
        return auto_enable_;
      }
    bool operator ==(const EntityFactory& other) const
    {
            return other.auto_enable() == auto_enable_;
    }
    private:
      bool auto_enable_;
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
    class TransportPriority {
    public:
      explicit TransportPriority(uint32_t prio) : value_(prio) { }
      TransportPriority() : value_(0) { }
    public:
      void value(uint32_t prio) {
        value_ = prio;
      }
      uint32_t value() const {
        return value_;
      }
      uint32_t& value() {
        return value_;
      }
    bool operator ==(const TransportPriority& other) const
    {
            return other.value() == value_;
    }
    private:
      uint32_t value_;
    };

    //==============================================================================

    /**
     * The purpose of this QoS is to avoid delivering “stale” data to the
     * application. Each data sample written by the DataWriter has an associated
     * expiration time beyond which the data should not be delivered to any
     * application. Once the sample expires, the data will be removed from the
     * DataReader caches as well as from the transient and persistent
     * information caches. The expiration time of each sample is computed by
     * adding the duration specified by the LIFESPAN QoS to the source timestamp.
     * As described in Section 7.1.2.4.2.11, “write and Section 7.1.2.4.2.12,
     * write_w_timestamp the source timestamp is either automatically computed by
     * the Service each time the DataWriter write operation is called, or else
     * supplied by the application by means of the write_w_timestamp operation.
     *
     * This QoS relies on the sender and receiving applications having their clocks
     * sufficiently synchronized. If this is not the case and the Service can
     * detect it, the DataReader is allowed to use the reception timestamp instead
     * of the source timestamp in its computation of the expiration time.
     */
    class Lifespan {
    public:
      explicit Lifespan(const dds::core::Duration& d) : duration_(d) { }
      Lifespan() : duration_(dds::core::Duration::infinite()) { }

    public:
      void duration(const dds::core::Duration& d) {
        duration_ = d;
      }
      const dds::core::Duration duration() const {
        return duration_;
      }

      dds::core::Duration& duration() {
        return duration_;
      }
    bool operator ==(const Lifespan& other) const
    {
            return other.duration() == duration_;
    }
    private:
      dds::core::Duration duration_;
    };

    //==============================================================================

    /**
     * This policy is useful for cases where a Topic is expected to have each
     * instance updated periodically. On the publishing side this setting
     * establishes a contract that the application must meet. On the subscribing
     * side the setting establishes a minimum requirement for the remote publishers
     * that are expected to supply the data values. When the Service ‘matches’ a
     * DataWriter and a DataReader it checks whether the settings are compatible
     * (i.e., offered deadline period<= requested deadline period) if they are not,
     * the two entities are informed (via the listener or condition mechanism)
     * of the incompatibility of the QoS settings and communication will not occur.
     * Assuming that the reader and writer ends have compatible settings, the
     * fulfillment of this contract is monitored by the Service and the application
     * is informed of any violations by means of the proper listener or condition.
     * The value offered is considered compatible with the value requested if and
     * only if the inequality “offered deadline period <= requested deadline period”
     * evaluates to ‘TRUE.’ The setting of the DEADLINE policy must be set
     * consistently with that of the TIME_BASED_FILTER.
     * For these two policies to be consistent the settings must be such that
     * “deadline period>= minimum_separation.”
     */
    class Deadline {
    public:
      explicit Deadline(const dds::core::Duration& d) : period_(d) { }

      Deadline() : period_(dds::core::Duration::infinite()) { }

    public:
      void period(const dds::core::Duration& d) {
        period_ = d;
      }
      const dds::core::Duration period() const {
        return period_;
      }
    bool operator ==(const Deadline& other) const
    {
            return other.period() == period_;
    }
    private:
      dds::core::Duration period_;
    };

    //==============================================================================

    class LatencyBudget {
    public:
      explicit LatencyBudget(const dds::core::Duration& d) : duration_(d) { }
      LatencyBudget() : duration_(dds::core::Duration::zero()) { }

    public:
      void duration(const dds::core::Duration& d) {
        duration_ = d;
      }
      const dds::core::Duration duration() const {
        return duration_;
      }
      dds::core::Duration& duration()  {
        return duration_;
      }
    bool operator ==(const LatencyBudget& other) const
    {
            return other.duration() == duration_;
    }
    private:
      dds::core::Duration duration_;
    };

    //==============================================================================

    class TimeBasedFilter {
    public:
      TimeBasedFilter() : min_sep_(dds::core::Duration::zero()) { }
      explicit TimeBasedFilter(const dds::core::Duration& the_min_separation)
        : min_sep_(the_min_separation) { }

    public:
      void min_separation(const dds::core::Duration& ms) {
        min_sep_ = ms;
      }
      const dds::core::Duration min_separation() const {
        return min_sep_;
      }

      dds::core::Duration& min_separation() {
        return min_sep_;
      }
    bool operator ==(const TimeBasedFilter& other) const
    {
            return other.min_separation() == min_sep_;
    }
    private:
      dds::core::Duration min_sep_;
    };

    //==============================================================================

    class Partition {
    public:
      explicit Partition(const std::string& partition) : name_() {
        name_.push_back(partition);
      }
      explicit Partition(const dds::core::StringSeq& partitions)
        : name_(partitions) { }

      Partition() : name_() {
        name_.push_back("");
      }

    public:
      void name(const dds::core::StringSeq& partitions) {
        name_ = partitions;
      }

      const dds::core::StringSeq& name() const {
        return name_;
      }

      dds::core::StringSeq& name() {
        return name_;
      }
    bool operator ==(const Partition& other) const
    {
            return other.name() == name_;
    }
    private:
      dds::core::StringSeq name_;
    };

    //==============================================================================

    class Ownership {
    public:

    public:
      Ownership(dds::core::policy::OwnershipKind::Type the_kind) : kind_(the_kind) { }
      Ownership() : kind_(dds::core::policy::OwnershipKind::SHARED) { }

    public:
      void kind(dds::core::policy::OwnershipKind::Type the_kind) {
        kind_ = the_kind;
      }
      dds::core::policy::OwnershipKind::Type kind() const {
        return kind_;
      }

      dds::core::policy::OwnershipKind::Type& kind(){
        return kind_;
      }
    bool operator ==(const Ownership& other) const
    {
            return other.kind() == kind_;
    }
    private:
      dds::core::policy::OwnershipKind::Type kind_;
    };

    //==============================================================================

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT

    class OwnershipStrength {
    public:
    OwnershipStrength() : s_(0) {}
    explicit OwnershipStrength(int32_t s) : s_(s) { }

      int32_t strength() const { return s_; }
      int32_t& strength() { return s_; }
      void strength(int32_t s) { s_ = s; }

    bool operator ==(const OwnershipStrength& other) const
    {
            return other.strength() == s_;
    }

    private:
      int32_t s_;
    };

#endif  // OMG_DDS_OWNERSHIP_SUPPORT


    //==============================================================================


    class WriterDataLifecycle {
    public:
      WriterDataLifecycle() : autodispose_(true) {}
      WriterDataLifecycle(bool the_autodispose)
      : autodispose_(the_autodispose) { }

      bool autodispose() const { return autodispose_; }
      bool& autodispose() { return autodispose_; }
      void autodispose(bool b) { autodispose_ = b; }
    bool operator ==(const WriterDataLifecycle& other) const
    {
            return other.autodispose() == autodispose_;
    }
    private:
      bool autodispose_;
    };


    //==============================================================================

    class ReaderDataLifecycle {
    public:
      ReaderDataLifecycle()
        : autopurge_nowriter_samples_delay_(dds::core::Duration::infinite()),
          autopurge_disposed_samples_delay_(dds::core::Duration::infinite()) { }

      ReaderDataLifecycle(const dds::core::Duration& the_nowriter_delay,
                  const dds::core::Duration& the_disposed_samples_delay)
        : autopurge_nowriter_samples_delay_(the_nowriter_delay),
          autopurge_disposed_samples_delay_(the_disposed_samples_delay) { }

    public:

      const dds::core::Duration autopurge_nowriter_samples_delay() const {
        return autopurge_nowriter_samples_delay_;
      }

      void autopurge_nowriter_samples_delay(const dds::core::Duration& d) {
        autopurge_nowriter_samples_delay_ = d;
      }

      const dds::core::Duration autopurge_disposed_samples_delay() const {
        return autopurge_disposed_samples_delay_;
      }

      void autopurge_disposed_samples_delay(const dds::core::Duration& d) {
        autopurge_disposed_samples_delay_ = d;
      }

    bool operator ==(const ReaderDataLifecycle& other) const
    {
            return other.autopurge_nowriter_samples_delay() == autopurge_nowriter_samples_delay_ &&
            other.autopurge_disposed_samples_delay() == autopurge_disposed_samples_delay();
    }

    private:
      dds::core::Duration autopurge_nowriter_samples_delay_;
      dds::core::Duration autopurge_disposed_samples_delay_;
    };

    //==============================================================================

    class Durability {
    public:

    public:
      Durability(dds::core::policy::DurabilityKind::Type the_kind) : kind_(the_kind) { }
      Durability() : kind_(dds::core::policy::DurabilityKind::VOLATILE) { }
    public:
      void durability(dds::core::policy::DurabilityKind::Type the_kind) {
        kind_ = the_kind;
      }
      dds::core::policy::DurabilityKind::Type durability() const {
        return kind_;
      }
      dds::core::policy::DurabilityKind::Type& durability() {
        return kind_;
      }
      void kind(dds::core::policy::DurabilityKind::Type the_kind) {
        kind_ = the_kind;
      }
      dds::core::policy::DurabilityKind::Type& kind() {
        return kind_;
      }
      dds::core::policy::DurabilityKind::Type kind() const {
        return kind_;
      }
    bool operator ==(const Durability& other) const
    {
            return other.kind() == kind_;
    }
    public:
      dds::core::policy::DurabilityKind::Type kind_;
    };


    //==============================================================================

    class Presentation {

    public:
      Presentation()
        :  access_scope_(dds::core::policy::PresentationAccessScopeKind::INSTANCE),
           coherent_access_(false),
           ordered_access_(false) { }

      Presentation(dds::core::policy::PresentationAccessScopeKind::Type the_access_scope,
               bool the_coherent_access,
               bool the_ordered_access)
        :  access_scope_(the_access_scope),
           coherent_access_(the_coherent_access),
           ordered_access_(the_ordered_access)
      { }

      void access_scope(dds::core::policy::PresentationAccessScopeKind::Type as) {
        access_scope_ = as;
      }
      dds::core::policy::PresentationAccessScopeKind::Type& access_scope() {
            return access_scope_;
      }
      dds::core::policy::PresentationAccessScopeKind::Type access_scope() const {
        return access_scope_;
      }
      void coherent_access(bool on) {
        coherent_access_ = on;
      }
      bool& coherent_access()  {
            return coherent_access_;
      }
      bool coherent_access() const {
        return coherent_access_;
      }
      void ordered_access(bool on) {
        ordered_access_ = on;
      }
      bool& ordered_access() {
            return ordered_access_;
      }
      bool ordered_access() const {
        return ordered_access_;
      }
    bool operator ==(const Presentation& other) const
    {
            return other.access_scope() == access_scope_ &&
            other.coherent_access() == coherent_access_ &&
            other.ordered_access() == ordered_access_;
    }
    private:
      dds::core::policy::PresentationAccessScopeKind::Type access_scope_;
      bool coherent_access_;
      bool ordered_access_;
    };
    //==============================================================================

    class Reliability {
    public:
    public:
      Reliability()
        :  kind_(dds::core::policy::ReliabilityKind::BEST_EFFORT),
           max_blocking_time_(dds::core::Duration::zero()) { }

      Reliability(dds::core::policy::ReliabilityKind::Type the_kind,
              const dds::core::Duration& the_max_blocking_time)
        :  kind_(the_kind),
           max_blocking_time_(the_max_blocking_time) { }

    public:

      void kind(dds::core::policy::ReliabilityKind::Type the_kind) {
        kind_ = the_kind;
      }
      dds::core::policy::ReliabilityKind::Type kind() const {
        return kind_;
      }

      void max_blocking_time(const dds::core::Duration& d) {
        max_blocking_time_ = d;
      }
      const dds::core::Duration max_blocking_time() const {
        return max_blocking_time_;
      }
        bool operator ==(const Reliability& other) const
    {
            return other.kind() == kind_ &&
            other.max_blocking_time() == max_blocking_time_;
    }
    private:
      dds::core::policy::ReliabilityKind::Type        kind_;
      dds::core::Duration    max_blocking_time_;
    };

    //==============================================================================

    class DestinationOrder {

    public:
      explicit DestinationOrder(dds::core::policy::DestinationOrderKind::Type the_kind)
        : kind_(the_kind) { }
      DestinationOrder() : kind_(dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP) { }

    public:
      void kind(dds::core::policy::DestinationOrderKind::Type the_kind) {
        kind_ = the_kind;
      }
      dds::core::policy::DestinationOrderKind::Type& kind() {
            return kind_;
      }
      dds::core::policy::DestinationOrderKind::Type kind() const {
        return kind_;
      }

    bool operator ==(const DestinationOrder& other) const
    {
            return other.kind() == kind_;
    }
    private:
      dds::core::policy::DestinationOrderKind::Type kind_;
    };

    //==============================================================================

    class History {
    public:
      History()
        :  kind_(dds::core::policy::HistoryKind::KEEP_LAST),
           depth_(1)
      { }


      History(dds::core::policy::HistoryKind::Type the_kind, int32_t the_depth)
        :  kind_(the_kind),
           depth_(the_depth)
      { }

      dds::core::policy::HistoryKind::Type kind() const {
        return kind_;
      }
      dds::core::policy::HistoryKind::Type& kind() {
        return kind_;
      }

      void kind(dds::core::policy::HistoryKind::Type the_kind) {
        kind_ = the_kind;
      }

      int32_t depth() const {
        return depth_;
      }
      int32_t& depth() {
        return depth_;
      }
      void depth(int32_t the_depth) {
        depth_ = the_depth;
      }

    bool operator ==(const History& other) const
    {
        return other.kind() == kind_ &&
        other.depth() == depth_;
    }
    private:
      dds::core::policy::HistoryKind::Type kind_;
      int32_t depth_;
    };
    //==============================================================================


    class ResourceLimits {
    public:
      ResourceLimits()
        :  max_samples_(dds::core::LENGTH_UNLIMITED),
           max_instances_(dds::core::LENGTH_UNLIMITED),
           max_samples_per_instance_(dds::core::LENGTH_UNLIMITED)
      { }

      ResourceLimits(int32_t the_max_samples,
             int32_t the_max_instances,
             int32_t the_max_samples_per_instance)
        :  max_samples_(the_max_samples),
           max_instances_(the_max_instances),
           max_samples_per_instance_(the_max_samples_per_instance)
      { }

    public:
      void max_samples(int32_t samples) {
        max_samples_ = samples;
      }
      int32_t& max_samples() {
        return max_samples_;
      }
      int32_t max_samples() const {
        return max_samples_;
      }

      void max_instances(int32_t the_max_instances) {
        max_instances_ = the_max_instances;
      }
      int32_t& max_instances() {
        return max_instances_;
      }
      int32_t max_instances() const {
        return max_instances_;
      }

      void max_samples_per_instance(int32_t the_max_samples_per_instance) {
        max_samples_per_instance_ = the_max_samples_per_instance;
      }
      int32_t& max_samples_per_instance()  {
        return max_samples_per_instance_;
      }
      int32_t max_samples_per_instance() const {
        return max_samples_per_instance_;
      }
    bool operator ==(const ResourceLimits& other) const
    {
            return other.max_samples() == max_samples_ &&
            other.max_instances() == max_instances_ &&
            other.max_samples_per_instance() == max_samples_per_instance_;
    }
    private:
      int32_t max_samples_;
      int32_t max_instances_;
      int32_t max_samples_per_instance_;
    };


    //==============================================================================

    class Liveliness {
    public:
    public:
      Liveliness()
        :  kind_(dds::core::policy::LivelinessKind::AUTOMATIC),
           lease_duration_(dds::core::Duration::infinite())
      { }

      Liveliness(dds::core::policy::LivelinessKind::Type the_kind,
             dds::core::Duration the_lease_duration)
        :  kind_(the_kind),
           lease_duration_(the_lease_duration)
      { }
      void kind(dds::core::policy::LivelinessKind::Type the_kind) {
        kind_ = the_kind;
      }
      dds::core::policy::LivelinessKind::Type& kind() {
        return kind_;
      }
      dds::core::policy::LivelinessKind::Type kind() const {
        return kind_;
      }
      void lease_duration(const dds::core::Duration& the_lease_duration) {
        lease_duration_ = the_lease_duration;
      }
      dds::core::Duration& lease_duration() {
        return lease_duration_;
      }
      const dds::core::Duration lease_duration() const {
        return lease_duration_;
      }

    bool operator ==(const Liveliness& other) const
    {
        return other.kind() == kind_ &&
        other.lease_duration() == lease_duration_;
    }
    private:
      dds::core::policy::LivelinessKind::Type            kind_;
      dds::core::Duration     lease_duration_;
    };

    //==============================================================================

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

    class DurabilityService {
    public:
      DurabilityService() :
        cleanup_delay_(dds::core::Duration::zero()),
        history_kind_(dds::core::policy::HistoryKind::KEEP_LAST),
        history_depth_(1),
        max_samples_(dds::core::LENGTH_UNLIMITED),
        max_instances_(dds::core::LENGTH_UNLIMITED),
        max_samples_per_instance_(dds::core::LENGTH_UNLIMITED)
      { }

      DurabilityService(const dds::core::Duration& the_service_cleanup_delay,
                dds::core::policy::HistoryKind::Type the_history_kind,
                int32_t the_history_depth,
                int32_t the_max_samples,
                int32_t the_max_instances,
                int32_t the_max_samples_per_instance)
        : cleanup_delay_(the_service_cleanup_delay),
          history_kind_(the_history_kind),
          history_depth_(the_history_depth),
          max_samples_(the_max_samples),
          max_instances_(the_max_instances),
          max_samples_per_instance_(the_max_samples_per_instance)
      { }

    public:
      void service_cleanup_delay(const dds::core::Duration& d)  {
        cleanup_delay_ = d;
      }
      const dds::core::Duration service_cleanup_delay() const {
        return cleanup_delay_;
      }

      void history_kind(dds::core::policy::HistoryKind::Type the_kind) {
        history_kind_ = the_kind;
      }
      dds::core::policy::HistoryKind::Type history_kind() const {
        return history_kind_;
      }

      void history_depth(int32_t the_depth) {
        history_depth_ = the_depth;
      }
      int32_t history_depth() const {
        return history_depth_;
      }

      void max_samples(int32_t the_max_samples) {
        max_samples_ = the_max_samples;
      }
      int32_t max_samples() const {
        return max_samples_;
      }

      void max_instances(int32_t the_max_instances) {
        max_instances_ = the_max_instances;
      }
      int32_t max_instances() const {
        return max_instances_;
      }

      void max_samples_per_instance(int32_t the_max_samples_per_instance) {
        max_samples_per_instance_ = the_max_samples_per_instance;
      }
      int32_t max_samples_per_instance() const {
        return max_samples_per_instance_;
      }

    bool operator ==(const DurabilityService& other) const
    {
            return other.service_cleanup_delay() == cleanup_delay_ &&
            other.history_kind() == history_kind_ &&
            other.history_depth() == history_depth_ &&
            other.max_samples() == max_samples_ &&
            other.max_instances() == max_instances_ &&
            other.max_samples_per_instance() == max_samples_per_instance_;
    }
    private:
      dds::core::Duration cleanup_delay_;
      dds::core::policy::HistoryKind::Type history_kind_;
      int32_t history_depth_;
      int32_t max_samples_;
      int32_t max_instances_;
      int32_t max_samples_per_instance_;
    };

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


#ifdef  OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

    class DataRepresentation { };

#endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT


#ifdef  OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

    class TypeConsistencyEnforcement { };

#endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

      } } } }  // namespace org::opensplice::core::policy

#endif /* ORG_OPENSPLICE_CORE_POLICY_CORE_POLICY_IMPL_HPP_ */
