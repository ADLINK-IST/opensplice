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
namespace dds
{
namespace core
{
namespace policy
{

//==============================================================================
/**
 * The purpose of this QoS is to allow the application to attach additional
 * information to the created Entity objects such that when a remote application
 * discovers their existence it can access that information and use it for its
 * own purposes. One possible use of this QoS is to attach security credentials
 * or some other information that can be used by the remote application to
 * authenticate the source. In combination with operations such as
 * dds::domain::ignore, dds::pub::ignore, dds::sub::ignore,
 * and dds::topic::ignore, this QoS can assist an application to define and
 * enforce its own security policies. The use of this QoS is not limited to
 * security, rather it offers a simple yet flexible extensibility mechanism.
 */
template <typename D>
class TUserData : public dds::core::Value<D>
{
public:
    /**
     * Creates a UserData QoS instance with an empty UserData
     */
    TUserData();

    /**
     * Creates a UserData QoS instance
     *
     * @param sequence the sequence of octets
     */
    explicit TUserData(const dds::core::ByteSeq& sequence);

    /**
     * Creates a UserData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    TUserData(const uint8_t* value_begin, const uint8_t* value_end);

    /**
     * Copies a UserData QoS instance
     *
     * @param other the UserData QoS instance to copy
     */
    TUserData(const TUserData& other);

public:
    /**
     * Sets the sequence
     *
     * @param sequence a sequence of octets
     */
    TUserData& value(const dds::core::ByteSeq& sequence);

    /**
     * Sets the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template <typename OCTET_ITER>
    TUserData& value(OCTET_ITER begin, OCTET_ITER end);

    /**
     * Gets the sequence
     *
     * @return a sequence of octets
     */
    const dds::core::ByteSeq value() const;

    /**
     * Gets a pointer to the first octet in the sequence
     *
     * @return a pointer to the first octet in the sequence
     */
    const uint8_t* begin() const;

    /**
     * Gets a pointer to the last octet in the sequence
     *
     * @return a pointer to the first octet in the sequence
     */
    const uint8_t* end() const;
};

//==============================================================================

/**
 * The purpose of this QoS is to allow the application to attach additional
 * information to the created Publisher or Subscriber.
 * The value of the GroupData is available to the application on the
 * DataReader and DataWriter entities and is propagated by means of the
 * built-in topics.
 *
 * This QoS can be used by an application in combination with the
 * DataReaderListener and DataWriterListener to implement matching policies
 * similar to those of the Partition QoS, except that the decision can be made
 * based on an application-defined policy.
 */
template <typename D>
class TGroupData : public dds::core::Value<D>
{
public:
    /**
     * Creates a GroupData QoS instance
     */
    TGroupData();

    /**
     * Creates a GroupData QoS instance
     *
     * @param sequence the sequence of octets representing the GroupData
     */
    explicit TGroupData(const dds::core::ByteSeq& sequence);

    /**
     * Copies a GroupData QoS instance
     *
     * @param other the GroupData QoS instance to copy
     */
    TGroupData(const TGroupData& other);

    /**
     * Creates a GroupData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    TGroupData(const uint8_t* value_begin, const uint8_t* value_end);

public:
    /**
     * Set the sequence
     *
     * @param sequence a sequence of octets
     */
    TGroupData& value(const dds::core::ByteSeq& sequence);

    /**
     * Set the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template <typename OCTET_ITER>
    TGroupData& value(OCTET_ITER begin, OCTET_ITER end);

    /**
     * Get the sequence
     *
     * @return a sequence of octets
     */
    const dds::core::ByteSeq value() const;

    /**
     * Gets a pointer to the first octet in the sequence
     *
     * @return a pointer to the first octet in the sequence
     */
    const uint8_t* begin() const;

    /**
     * Gets a pointer to the last octet in the sequence
     *
     * @return a pointer to the last octet in the sequence
     */
    const uint8_t* end() const;
};

//==============================================================================

/**
 * The purpose of this QoS is to allow the application to attach additional
 * information to the created Topic such that when a remote application
 * discovers their existence it can examine the information and use it in
 * an application-defined way. In combination with the listeners on the
 * DataReader and DataWriter, as well as by means of operations such as
 * dds::topic::ignore, this QoS can assist an application to extend the provided QoS.
 */
template <typename D>
class TTopicData : public dds::core::Value<D>
{
public:
    /**
     * Creates a TopicData QoS instance
     */
    TTopicData();

    /**
     * Creates a TopicData QoS instance
     *
     * @param sequence the sequence of octets representing the TopicData
     */
    explicit TTopicData(const dds::core::ByteSeq& sequence);

    /**
     * Copies a TopicData QoS instance
     *
     * @param other the TopicData QoS instance to copy
     */
    TTopicData(const TTopicData& other);

    /**
     * Creates a TopicData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    TTopicData(const uint8_t* value_begin, const uint8_t* value_end);

public:
    /**
     * Set the sequence
     *
     * @param sequence a sequence of octets
     */
    TTopicData& value(const dds::core::ByteSeq& sequence);

    /**
     * Set the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template <typename OCTET_ITER>
    TTopicData& value(OCTET_ITER begin, OCTET_ITER end);

    /**
     * Get the sequence
     *
     * @return a sequence of octets
     */
    const dds::core::ByteSeq value() const;

    /**
     * Gets a pointer to the first octet in the sequence
     *
     * @return a pointer to the first octet in the sequence
     */
    const uint8_t* begin() const;

    /**
     * Gets a pointer to the last octet in the sequence
     *
     * @return a pointer to the last octet in the sequence
     */
    const uint8_t* end() const;
};


//==============================================================================

/**
 * This policy is mutable. A change in the policy affects only the entities
 * created AFTER the change, not the previously-created entities.
 * The setting of autoenable_created_entities to true indicates that the
 * newly-created object will be enabled by default.
 *
 * A setting of false indicates that the Entity will not be automatically
 * enabled. The application will need to enable it explicitly by means of the
 * enable operation (see Section 7.1.2.1.1.7, "enable"). The default setting
 * of autoenable_created_entities = true means that, by default, it is not
 * necessary to explicitly call enable on newly-created entities.
 */
template <typename D>
class TEntityFactory : public dds::core::Value<D>
{
public:
    /**
     * Creates an EntityFactory QoS instance
     *
     * @param autoenable_created_entities boolean indicating whether
     * created Entities should be automatically enabled
     */
    explicit TEntityFactory(bool autoenable_created_entities = true);

    /**
     * Copies an EntityFactory QoS instance
     *
     * @param other the EntityFactory QoS instance to copy
     */
    TEntityFactory(const TEntityFactory& other);

public:
    /**
     * Sets a boolean indicating whether created Entities should be
     * automatically enabled
     *
     * @param autoenable_created_entities boolean indicating whether
     * created Entities should be automatically enabled
     */
    TEntityFactory& autoenable_created_entities(bool autoenable_created_entities);

    /**
     * Gets a boolean indicating whether Entities should be automatically enabled
     *
     * @return boolean indicating whether created Entities should be automatically
     * enabled
     */
    bool autoenable_created_entities() const;

public:
    /**
     * @return an EntityFactory QoS instance with autoenable_created_entities
     * set to true
     */
    static TEntityFactory AutoEnable();

    /**
     * @return an EntityFactory QoS instance with autoenable_created_entities
     * set to false
     */
    static TEntityFactory ManuallyEnable();
};

//==============================================================================

/**
 * The purpose of this QoS is to allow the application to take advantage of
 * transports capable of sending messages with different priorities.
 *
 * This policy is considered a hint. The policy depends on the ability of the
 * underlying transports to set a priority on the messages they send.
 * Any value within the range of a 32-bit signed integer may be chosen;
 * higher values indicate higher priority. However, any further interpretation
 * of this policy is specific to a particular transport and a particular
 * implementation of the Service. For example, a particular transport is
 * permitted to treat a range of priority values as equivalent to one another.
 * It is expected that during transport configuration the application would
 * provide a mapping between the values of the TransportPriority set on
 * DataWriter and the values meaningful to each transport. This mapping would
 * then be used by the infrastructure when propagating the data written by
 * the DataWriter.
 */
template <typename D>
class TTransportPriority : public dds::core::Value<D>
{
public:
    /**
     * Creates a TransportPriority QoS instance
     *
     * @param priority the priority value
     */
    explicit TTransportPriority(int32_t priority = 0);

    /**
     * Copies a TransportPriority QoS instance
     *
     * @param other the TransportPriority QoS instance to copy
     */
    TTransportPriority(const TTransportPriority& other);

public:
    /**
     * Sets the priority value
     *
     * @param priority the priority value
     */
    TTransportPriority& value(int32_t priority);

    /**
     * Gets the priority value
     *
     * @return the priority value
     */
    int32_t value() const;
};

//==============================================================================

/**
 * The purpose of this QoS is to avoid delivering "stale" data to the
 * application. Each data sample written by the DataWriter has an associated
 * expiration time beyond which the data should not be delivered to any
 * application. Once the sample expires, the data will be removed from the
 * DataReader caches as well as from the transient and persistent
 * information caches.
 *
 * The expiration time of each sample is computed by adding the duration specified
 * by the Lifespan QoS to the source timestamp. The source timestamp is either
 * automatically computed by the Service each time the DataWriter write operation
 * is called, or else supplied by the application by means of the
 * write(const T& sample, const dds::core::Time& timestamp); operation.
 *
 * This QoS relies on the sender and the receiving applications having their
 * clocks sufficiently synchronized. If this is not the case and the Service can
 * detect it, the DataReader is allowed to use the reception timestamp instead
 * of the source timestamp in its computation of the expiration time.
 */
template <typename D>
class TLifespan : public dds::core::Value<D>
{
public:
    /**
     * Creates a Lifespan QoS instance
     *
     * @param duration Lifespan expiration duration
     */
    explicit TLifespan(const dds::core::Duration& duration = dds::core::Duration::infinite());

    /**
     * Copies a Lifespan QoS instance
     *
     * @param other the Lifespan QoS instance to copy
     */
    TLifespan(const TLifespan& other);

public:
    /**
     * Sets the expiration duration
     *
     * @param duration expiration duration
     */
    TLifespan& duration(const dds::core::Duration& duration);

    /**
     * Gets the expiration duration
     *
     * @return expiration duration
     */
    const dds::core::Duration duration() const;
};

//==============================================================================

/**
 * This policy is useful for cases where a Topic is expected to have each
 * instance updated periodically. On the publishing side this setting
 * establishes a contract that the application must meet. On the subscribing
 * side the setting establishes a minimum requirement for the remote publishers
 * that are expected to supply the data values.
 *
 * When the Service matches a DataWriter and a DataReader it checks whether the
 * settings are compatible (i.e., offered deadline period<= requested deadline
 * period) if they are not, the two entities are informed (via the listener or
 * condition mechanism) of the incompatibility of the QoS settings and
 * communication will not occur.
 *
 * Assuming that the reader and writer ends have compatible settings, the
 * fulfillment of this contract is monitored by the Service and the application
 * is informed of any violations by means of the proper listener or condition.
 *
 * The value offered is considered compatible with the value requested if and
 * only if the inequality "offered deadline period <= requested deadline period"
 * evaluates to true.
 *
 * The setting of the Deadline policy must be set consistently with that of the
 * TimeBasedFilter policy. For these two policies to be consistent the settings
 * must be such that "deadline period >= minimum_separation".
 */
template <typename D>
class TDeadline : public dds::core::Value<D>
{
public:
    /**
     * Creates a Deadline QoS instance
     *
     * @param period deadline period
     */
    explicit TDeadline(const dds::core::Duration& period = dds::core::Duration::infinite());

    /**
     * Copies a Deadline QoS instance
     *
     * @param other the Deadline QoS instance to copy
     */
    TDeadline(const TDeadline& other);

public:
    /**
     * Sets the deadline period
     *
     * @param period deadline period
     */
    TDeadline& period(const dds::core::Duration& period);

    /**
     * Gets the deadline period
     *
     * @return deadline period
     */
    const dds::core::Duration period() const;
};

//==============================================================================

/**
 * This policy provides a means for the application to indicate to the middleware
 * the 'urgency' of the data-communication. By having a non-zero duration the
 * Service can optimize its internal operation.
 *
 * This policy is considered a hint. There is no specified mechanism as to how the
 * service should take advantage of this hint.
 *
 * The value offered is considered compatible with the value requested if and only
 * if the inequality "offered duration <= requested duration" evaluates to TRUE.
 */
template <typename D>
class TLatencyBudget : public dds::core::Value<D>
{
public:
    /**
     * Creates a LatencyBudget QoS instance
     *
     * @param duration duration
     */
    explicit TLatencyBudget(const dds::core::Duration& duration = dds::core::Duration::zero());

    /**
     * Copies a LatencyBudget QoS instance
     *
     * @param other the LatencyBudget QoS instance to copy
     */
    TLatencyBudget(const TLatencyBudget& other);

public:
    /**
     * Sets the duration
     *
     * @param duration duration
     */
    TLatencyBudget& duration(const dds::core::Duration& duration);

    /**
     * Gets the duration
     *
     * @return duration
     */
    const dds::core::Duration duration() const;
};

//==============================================================================

/**
 * This policy allows a DataReader to indicate that it does not necessarily want
 * to see all values of each instance published under the Topic. Rather, it wants
 * to see at most one change every minimum_separation period.
 *
 * The TimeBasedFilter applies to each instance separately, that is, the constraint
 * is that the DataReader does not want to see more than one sample of each
 * instance per minumum_separation period.
 *
 * This setting allows a DataReader to further decouple itself from the DataWriter
 * objects. It can be used to protect applications that are running on a
 * heterogeneous network where some nodes are capable of generating data much
 * faster than others can consume it. It also accommodates the fact that for
 * fast-changing data different subscribers may have different requirements as
 * to how frequently they need to be notified of the most current values.
 *
 * The setting of a TimeBasedFilter (that is, the selection of a minimum_separation
 * with a value greater than zero) is compatible with all settings of the History
 * and Reliability QoS. The TimeBasedFilter specifies the samples that are of
 * interest to the DataReader. The History and Reliability QoS affect the behavior
 * of the middleware with respect to the samples that have been determined to be
 * of interest to the DataReader; that is, they apply after the TimeBasedFilter
 * has been applied.
 *
 * In the case where the reliability QoS kind is Reliable then in steady-state,
 * defined as the situation where the DataWriter does not write new samples for a
 * period 'long' compared to the minimum_separation, the system should guarantee
 * delivery of the last sample to the DataReader.
 *
 * The setting of the TimeBasedFilter minimum_separation must be consistent with
 * the Deadline period. For these two QoS policies to be consistent they must verify
 * that "period >= minimum_separation". An attempt to set these policies in an
 * inconsistent manner when an entity is created via a set qos operation will
 * cause the operation to fail.
 */
template <typename D>
class TTimeBasedFilter : public dds::core::Value<D>
{
public:
    /**
     * Creates a TimeBasedFilter QoS instance
     *
     * @param period minimum separation period
     */
    explicit TTimeBasedFilter(
        const dds::core::Duration& period = dds::core::Duration::zero());

    /**
     * Copies a TimeBasedFilter QoS instance
     *
     * @param other the TimeBasedFilter QoS instance to copy
     */
    TTimeBasedFilter(const TTimeBasedFilter& other);

public:
    /**
     * Sets the minimum separation period
     *
     * @param period minimum separation period
     */
    TTimeBasedFilter& minimum_separation(const dds::core::Duration& period);

    /**
     * Gets the minimum separation period
     *
     * @return minimum separation period
     */
    const dds::core::Duration minimum_separation() const;
};


//==============================================================================

/**
 * This policy allows the introduction of a logical partition concept inside the
 * "physical" partition induced by a domain. For a DataReader to see the changes
 * made to an instance by a DataWriter, not only the Topic must match, but also
 * they must share a common partition. Each string in the list that defines this
 * QoS policy defines a partition name. A partition name may contain wildcards.
 * Sharing a common partition means that one of the partition names matches.
 * Failure to match partitions is not considered an "incompatible" QoS and does
 * not trigger any listeners or conditions.
 *
 * This policy is changeable. A change of this policy can potentially modify
 * the "match" of existing DataReader and DataWriter entities. It may establish
 * new "matches" that did not exist before, or break existing matches.
 *
 * Partition names can be regular expressions and include wildcards as defined by
 * the POSIX fnmatch API (1003.2-1992 section B.6). Either Publisher or Subscriber
 * may include regular expressions in partition names, but no two names that both
 * contain wildcards will ever be considered to match. This means that although
 * regular expressions may be used both at publisher as well as subscriber side,
 * the service will not try to match two regular expressions (between publishers
 * and subscribers).
 *
 * Partitions are different from creating Entity objects in different domains in
 * several ways. First, entities belonging to different domains are completely
 * isolated from each other; there is no traffic, meta-traffic or any other way
 * for an application or the Service itself to see entities in a domain it does
 * not belong to. Second, an Entity can only belong to one domain whereas an
 * Entity can be in multiple partitions. Finally, as far as the DDS Service is
 * concerned, each unique data instance is identified by the tuple
 * (domainId, Topic, key). Therefore two Entity objects in different domains
 * cannot refer to the same data instance. On the other hand, the same
 * data-instance can be made available (published) or requested (subscribed) on
 * one or more partitions.
 */
template <typename D>
class TPartition : public dds::core::Value<D>
{
public:
    /**
     * Creates a Partition QoS instance
     *
     * @param name partition name
     */
    explicit TPartition(const std::string& name = "");

    /**
     * Creates a Partition QoS instance
     *
     * @param names a sequence containing multiple partition names
     */
    explicit TPartition(const dds::core::StringSeq& names);

    /**
     * Copies a Partition QoS instance
     *
     * @param other the Partition QoS instance to copy
     */
    TPartition(const TPartition& other);

public:
    /**
     * Sets the partition name
     *
     * @param name the partition name
     */
    TPartition& name(const std::string& name);

    /**
     * Sets multiple partition names
     *
     * @param names a sequence containing multiple partition names
     */
    TPartition& name(const dds::core::StringSeq& names);

    /**
     * Gets the partition names
     *
     * @return a sequence containing the partition names
     */
    const dds::core::StringSeq name() const;
};

//==============================================================================
#ifdef OMG_DDS_OWNERSHIP_SUPPORT

/**
 * This policy controls whether the Service allows multiple DataWriter objects
 * to update the same instance (identified by Topic + key) of a data-object.
 * There are two kinds of OWNERSHIP selected by the setting of the kind: SHARED
 * and EXCLUSIVE.
 *
 * The SHARED kind indicates that the Service does not enforce unique ownership
 * for each instance. In this case, multiple writers can update the same
 * data-object instance. The subscriber to the Topic will be able to access
 * modifications from all DataWriter objects, subject to the settings of other
 * QoS that may filter particular samples (e.g., the TimeBasedFilter or History
 * QoS policy). In any case there is no "filtering" of modifications made based
 * on the identity of the DataWriter that causes the modification.
 *
 * The EXCLUSIVE kind indicates that each instance of a data-object can only be
 * modified by one DataWriter. In other words, at any point in time a single
 * DataWriter "owns" each instance and is the only one whose modifications will
 * be visible to the DataReader objects. The owner is determined by selecting the
 * DataWriter with the highest value of the strength that is both "alive" (as
 * defined by the Liveliness QoS policy) and has not violated its Deadline contract
 * with regard to the data-instance. Ownership can therefore change as a result
 * of (a) a DataWriter in the system with a higher value of the strength that
 * modifies the instance, (b) a change in the strength value of the DataWriter
 * that owns the instance, (c) a change in the liveliness of the DataWriter that
 * owns the instance, and (d) a deadline with regard to the instance that is
 * missed by the DataWriter that owns the instance.
 */
template <typename D>
class TOwnership : public dds::core::Value<D>
{
public:
    #   if defined (__SUNPRO_CC) && defined(SHARED)
#       undef SHARED
    #   endif
    /**
     * Creates an Ownership QoS instance
     *
     * @param kind the kind
     */
    explicit TOwnership(
        dds::core::policy::OwnershipKind::Type kind = dds::core::policy::OwnershipKind::SHARED);

    /**
     * Copies an Ownership QoS instance
     *
     * @param other the Ownership QoS instance to copy
     */
    TOwnership(const TOwnership& other);

public:
    /**
     * Set the kind
     *
     * @param kind the kind
     */
    TOwnership& kind(dds::core::policy::OwnershipKind::Type kind);

    /**
     * Get the kind
     *
     * @param kind the kind
     */
    dds::core::policy::OwnershipKind::Type kind() const;

public:
    /**
     * @return an Ownership QoS instance with the kind set to EXCLUSIVE
     */
    static TOwnership Exclusive();

    /**
     * @return an Ownership QoS instance with the kind set to SHARED
     */
    static TOwnership Shared();
};


//==============================================================================

/**
 * This QoS policy should be used in combination with the Ownership policy. It
 * only applies to the situation case where Ownership kind is set to EXCLUSIVE.
 *
 * The value of the OwnershipStrength is used to determine the ownership of a
 * data-instance (identified by the key). The arbitration is performed by the
 * DataReader. The rules used to perform the arbitration are described in
 * Section 7.1.3.9.2, EXCLUSIVE kind.
 */
template <typename D>
class TOwnershipStrength : public dds::core::Value<D>
{
public:
    /**
     * Creates an OwnershipStrength QoS instance
     *
     * @param strength ownership strength
     */
    explicit TOwnershipStrength(int32_t strength = 0);

    /**
     * Copies an OwnershipStrength QoS instance
     *
     * @param other the OwnershipStrength QoS instance to copy
     */
    TOwnershipStrength(const TOwnershipStrength& other);

public:
    /**
     * Gets the ownership strength value
     *
     * @return the ownership strength value
     */
    int32_t value() const;

    /**
     * Sets the ownership strength value
     *
     * @param strength the ownership strength value
     */
    TOwnershipStrength& value(int32_t strength);
};

#endif  // OMG_DDS_OWNERSHIP_SUPPORT
//==============================================================================

/**
 * This policy controls the behavior of the DataWriter with regard to the
 * lifecycle of the data-instances it manages, that is, the data-instances that
 * have been either explicitly registered with the DataWriter using the register
 * operations (see Section 7.1.2.4.2.5 and Section 7.1.2.4.2.6) or implicitly by
 * directly writing the data (see Section 7.1.2.4.2.11 and Section 7.1.2.4.2.12).
 *
 * The autodispose_unregistered_instances flag controls the behavior when the
 * DataWriter unregisters an instance by means of the unregister operations (see
 * Section 7.1.2.4.2.7, and Section 7.1.2.4.2.8):
 *
 * * The setting "autodispose_unregistered_instances = TRUE" causes the DataWriter
 * to dispose the instance each time it is unregistered. The behavior is identical
 * to explicitly calling one of the dispose operations (Section 7.1.2.4.2.13,
 * and Section 7.1.2.4.2.14) on the instance prior to calling the unregister
 * operation.
 *
 * * The setting "autodispose_unregistered_instances = FALSE" will not cause this
 * automatic disposition upon unregistering. The application can still call one
 * of the dispose operations prior to unregistering the instance and accomplish
 * the same effect.
 */
template <typename D>
class TWriterDataLifecycle : public dds::core::Value<D>
{
public:
    /**
     * Creates a WriterDataLifecycle QoS instance
     *
     * @param autodispose_unregistered_instances ownership strength
     */
    explicit TWriterDataLifecycle(bool autodispose_unregistered_instances = 0);

    /**
     * Copies a WriterDataLifecycle QoS instance
     *
     * @param other the WriterDataLifecycle QoS instance to copy
     */
    TWriterDataLifecycle(const TWriterDataLifecycle& other);

public:
    /**
     * Gets a boolean indicating if unregistered instances should be autodisposed
     *
     * @return a boolean indicating if unregistered instances should be autodisposed
     */
    bool autodispose_unregistered_instances() const;

    /**
     * Sets a boolean indicating if unregistered instances should be autodisposed
     *
     * @param autodispose_unregistered_instances a boolean indicating if unregistered
     * instances should be autodisposed
     */
    TWriterDataLifecycle& autodispose_unregistered_instances(
        bool autodispose_unregistered_instances);

public:
    /**
     * @return a WriterDataLifecycle QoS instance with autodispose_unregistered_instances
     * set to true
     */
    static TWriterDataLifecycle AutoDisposeUnregisteredInstances();

    /**
     * @return a WriterDataLifecycle QoS instance with autodispose_unregistered_instances
     * set to false
     */
    static TWriterDataLifecycle ManuallyDisposeUnregisteredInstances();
};

//==============================================================================

/**
 * This policy controls the behavior of the DataReader with regard to the
 * lifecycle of the data-instances it manages; that is, the data-instances
 * that have been received and for which the DataReader maintains some internal
 * resources.
 *
 * The DataReader internally maintains the samples that have not been taken by
 * the application, subject to the constraints imposed by other QoS policies
 * such as History and ResourceLimits.
 *
 * The DataReader also maintains information regarding the identity, view_state
 * and instance_state of data-instances even after all samples have been "taken".
 * This is needed to properly compute the states when future samples arrive.
 *
 * Under normal circumstances the DataReader can only reclaim all resources for
 * instances for which there are no writers and for which all samples have been
 * "taken". The last sample the DataReader will have taken for that instance will
 * have an instance_state of either not_alive_no_writers or not_alive_disposed
 * depending on whether the last writer that had ownership of the instance
 * disposed it or not. Refer to Figure 7.11 for a statechart describing the
 * transitions possible for the instance_state. In the absence of the
 * ReaderDataLifecycle QoS this behavior could cause problems if the application
 * "forgets" to "take" those samples. The "untaken" samples will prevent the
 * DataReader from reclaiming the resources and they would remain in the DataReader
 * indefinitely.
 *
 * The autopurge_nowriter_samples_delay defines the maximum duration for which the
 * DataReader will maintain information regarding an instance once its instance_state
 * becomes not_alive_no_writers. After this time elapses, the DataReader will purge
 * all internal information regarding the instance, any untaken samples will also
 * be lost.
 *
 * The autopurge_disposed_samples_delay defines the maximum duration for which
 * the DataReader will maintain samples for an instance once its instance_state
 * becomes not_alive_disposed. After this time elapses, the DataReader will purge
 * all samples for the instance.
 */
template <typename D>
class TReaderDataLifecycle : public dds::core::Value<D>
{
public:
    /**
     * Creates a ReaderDataLifecycle QoS instance
     *
     * @param autopurge_nowriter_samples_delay the autopurge nowriter samples delay
     * @param autopurge_disposed_samples_delay the autopurge disposed samples delay
     */
    TReaderDataLifecycle(
        const dds::core::Duration& autopurge_nowriter_samples_delay = dds::core::Duration::infinite(),
        const dds::core::Duration& autopurge_disposed_samples_delay = dds::core::Duration::infinite());

    /**
     * Copies a ReaderDataLifecycle QoS instance
     *
     * @param other the ReaderDataLifecycle QoS instance to copy
     */
    TReaderDataLifecycle(const TReaderDataLifecycle& other);
public:
    /**
     * Gets the autopurge nowriter samples delay
     *
     * @return the autopurge nowriter samples delay
     */
    const dds::core::Duration autopurge_nowriter_samples_delay() const;

    /**
     * Sets the autopurge nowriter samples delay
     *
     * @param autopurge_nowriter_samples_delay the autopurge nowriter samples delay
     */
    TReaderDataLifecycle& autopurge_nowriter_samples_delay(
        const dds::core::Duration& autopurge_nowriter_samples_delay);

    /**
     * Gets the autopurge_disposed_samples_delay
     *
     * @return the autopurge disposed samples delay
     */
    const dds::core::Duration autopurge_disposed_samples_delay() const;

    /**
     * Sets the autopurge_disposed_samples_delay
     *
     * @return the autopurge disposed samples delay
     */
    TReaderDataLifecycle& autopurge_disposed_samples_delay(
        const dds::core::Duration& autopurge_disposed_samples_delay);

public:
    /**
     * @return a ReaderDataLifecycle QoS instance which will not autopurge disposed
     * samples
     */
    static TReaderDataLifecycle NoAutoPurgeDisposedSamples();

    /**
     * @param autopurge_disposed_samples_delay the autopurge disposed samples delay
     * @return a ReaderDataLifecycle QoS instance with autopurge_disposed_samples_delay
     * set to a specified value
     */
    static TReaderDataLifecycle AutoPurgeDisposedSamples(
        const dds::core::Duration& autopurge_disposed_samples_delay);
};

//==============================================================================

/**
 * The decoupling between DataReader and DataWriter offered by the Publish/
 * Subscribe paradigm allows an application to write data even if there are no
 * current readers on the network. Moreover, a DataReader that joins the network
 * after some data has been written could potentially be interested in accessing
 * the most current values of the data as well as potentially some history. This
 * QoS policy controls whether the Service will actually make data available to
 * late-joining readers. Note that although related, this does not strictly
 * control what data the Service will maintain internally. That is, the Service
 * may choose to maintain some data for its own purposes (e.g., flow control)
 * and yet not make it available to late-joining readers if the DURABILITY QoS
 * policy is set to VOLATILE.
 *
 * The value offered is considered compatible with the value requested if and
 * only if the inequality "offered kind >= requested kind" evaluates to TRUE.
 * For the purposes of this inequality, the values of DURABILITY kind are
 * considered ordered such that VOLATILE < TRANSIENT_LOCAL < TRANSIENT < PERSISTENT.
 *
 * For the purpose of implementing the DURABILITY QoS kind TRANSIENT or PERSISTENT,
 * the service behaves AS IF for each Topic that has TRANSIENT or PERSISTENT
 * durability kind there was a corresponding "built-in" DataReader and DataWriter
 * configured to have the same durability kind. In other words, it is AS IF
 * somewhere in the system (possibly on a remote node) there was a "built-in
 * durability DataReader" that subscribed to that Topic and a "built-in durability
 * DataWriter" that published that Topic as needed for the new subscribers that
 * join the system.
 *
 * For each Topic, the built-in fictitious "persistence service" DataReader and
 * DataWriter has its QoS configured from the Topic QoS of the corresponding Topic.
 * In other words, it is AS IF the service first did find_topic to access the
 * Topic, and then used the QoS from the Topic to configure the fictitious
 * built-in entities.
 *
 * A consequence of this model is that the transient or persistence serviced can
 * be configured by means of setting the proper QoS on the Topic.
 *
 * For a given Topic, the usual request/offered semantics apply to the matching
 * between any DataWriter in the system that writes the Topic and the built-in
 * transient/persistent DataReader for that Topic; similarly for the built-in
 * transient/persistent DataWriter for a Topic and any DataReader for the Topic.
 * As a consequence, a DataWriter that has an incompatible QoS with respect to
 * what the Topic specified will not send its data to the transient/persistent
 * service, and a DataReader that has an incompatible QoS with respect to the
 * specified in the Topic will not get data from it.
 *
 * Incompatibilities between local DataReader/DataWriter entities and the
 * corresponding fictitious "built-in transient/persistent entities" cause the
 * requested_incompatible_qos/offered_incompatible_qos status to change and the
 * corresponding Listener invocations and/or signaling of Condition and WaitSet
 * objects as they would with non-fictitious entities.
 */
template <typename D>
class TDurability : public dds::core::Value<D>
{
public:
    /**
     * Creates a Durability QoS instance
     *
     * @param kind the kind
     */
    explicit TDurability(
        dds::core::policy::DurabilityKind::Type kind = dds::core::policy::DurabilityKind::VOLATILE);

    /**
     * Copies a Durability QoS instance
     *
     * @param other the Durability QoS instance to copy
     */
    TDurability(const TDurability& other);

public:
    /**
    * Set the kind
    *
    * @param kind the kind
    */
    TDurability& kind(dds::core::policy::DurabilityKind::Type kind);

    /**
     * Get the kind
     *
     * @param kind the kind
     */
    dds::core::policy::DurabilityKind::Type  kind() const;

public:
    /**
     * @return a Durability QoS instance with the kind set to VOLATILE
     */
    static TDurability Volatile();

    /**
     * @return a Durability QoS instance with the kind set to TRANSIENT_LOCAL
     */
    static TDurability TransientLocal();

    /**
     * @return a Durability QoS instance with the kind set to TRANSIENT
     */
    static TDurability Transient();

    /**
     * @return a Durability QoS instance with the kind set to PERSISTENT
     */
    static TDurability Persistent();
};

//==============================================================================

/**
 * This QoS policy controls the extent to which changes to data-instances can be
 * made dependent on each other and also the kind of dependencies that can be
 * propagated and maintained by the Service.
 *
 * The setting of coherent_access controls whether the Service will preserve the
 * groupings of changes made by the publishing application by means of the
 * operations begin_coherent_change and end_coherent_change.
 *
 * The setting of ordered_access controls whether the Service will preserve the
 * order of changes.
 *
 * The granularity is controlled by the setting of the access_scope.
 *
 * If coherent_access is set, then the access_scope controls the maximum extent
 * of coherent changes. The behavior is as follows:
 *
 * * If access_scope is set to INSTANCE, the use of begin_coherent_change and
 * end_coherent_change has no effect on how the subscriber can access the data
 * because with the scope limited to each instance, changes to separate instances
 * are considered independent and thus cannot be grouped by a coherent change.
 *
 * * If access_scope is set to TOPIC, then coherent changes (indicated by their
 * enclosure within calls to begin_coherent_change and end_coherent_change) will
 * be made available as such to each remote DataReader independently. That is,
 * changes made to instances within each individual DataWriter will be available
 * as coherent with respect to other changes to instances in that same DataWriter,
 * but will not be grouped with changes made to instances belonging to a different
 * DataWriter.
 *
 * * If access_scope is set to GROUP, then coherent changes made to instances
 * through a DataWriter attached to a common Publisher are made available as a
 * unit to remote subscribers.
 *
 * If ordered_access is set, then the access_scope controls the maximum extent
 * for which order will be preserved by the Service.
 *
 * * If access_scope is set to INSTANCE (the lowest level), then changes to each
 * instance are considered unordered relative to changes to any other instance.
 * That means that changes (creations, deletions, modifications) made to two
 * instances are not necessarily seen in the order they occur. This is the case
 * even if it is the same application thread making the changes using the same
 * DataWriter.
 *
 * * If access_scope is set to TOPIC, changes (creations, deletions, modifications)
 * made by a single DataWriter are made available to subscribers in the same order
 * they occur. Changes made to instances through different DataWriter entities
 * are not necessarily seen in the order they occur. This is the case, even if the
 * changes are made by a single application thread using DataWriter objects
 * attached to the same Publisher.
 *
 * * Finally, if access_scope is set to GROUP, changes made to instances via
 * DataWriter entities attached to the same Publisher object are made available
 * to subscribers on the same order they occur.
 *
 * Note that this QoS policy controls the scope at which related changes are made
 * available to the subscriber. This means the subscriber can access the changes
 * in a coherent manner and in the proper order; however, it does not necessarily
 * imply that the Subscriber will indeed access the changes in the correct order.
 * For that to occur, the application at the subscriber end must use the proper
 * logic in reading the DataReader objects, as described in "Access to the data".
 *
 * The value offered is considered compatible with the value requested if and
 * only if the following conditions are met:
 *
 * 1. The inequality "offered access_scope >= requested access_scope" evaluates to
 * TRUE. For the purposes of this inequality, the values of Presentation
 * access_scope are considered ordered such that INSTANCE < TOPIC < GROUP.
 *
 * 2. Requested coherent_access is FALSE, or else both offered and requested
 * coherent_access are TRUE.
 *
 * 3. Requested ordered_access is FALSE, or else both offered and requested
 * ordered _access are TRUE.
 */
template <typename D>
class TPresentation : public dds::core::Value<D>
{
public:
    /**
     * Creates a Presentation QoS instance
     *
     * @param access_scope the access_scope kind
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     */
    TPresentation(
        dds::core::policy::PresentationAccessScopeKind::Type access_scope
        = dds::core::policy::PresentationAccessScopeKind::INSTANCE,
        bool coherent_access = false,
        bool ordered_access = false);

    /**
     * Copies a Presentation QoS instance
     *
     * @param other the Presentation QoS instance to copy
     */
    TPresentation(const TPresentation& other);

public:
    /**
     * Sets the access_scope kind
     *
     * @param access_scope the access_scope kind
     */
    TPresentation& access_scope(dds::core::policy::PresentationAccessScopeKind::Type access_scope);

    /**
     * Gets the access_scope kind
     *
     * @return the access_scope kind
     */
    dds::core::policy::PresentationAccessScopeKind::Type access_scope() const;

    /**
     * Sets the coherent_access setting
     *
     * @param coherent_access the coherent_access setting
     */
    TPresentation& coherent_access(bool coherent_access);

    /**
     * Gets the coherent_access setting
     *
     * @return the coherent_access setting
     */
    bool coherent_access() const;

    /**
     * Sets the ordered_access setting
     *
     * @param ordered_access the ordered_access setting
     */
    TPresentation& ordered_access(bool ordered_access);

    /**
     * Gets the ordered_access setting
     *
     * @return the ordered_access setting
     */
    bool ordered_access() const;

public:
    /**
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     *
     * @return a Presentation QoS instance with a GROUP access_score and coherent_access
     * and ordered_access set to the specified values
     */
    static TPresentation GroupAccessScope(bool coherent_access = false, bool ordered_access = false);

    /**
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     *
     * @return a Presentation QoS instance with a INSTANCE access_score and coherent_access
     * and ordered_access set to the specified values
     */
    static TPresentation InstanceAccessScope(bool coherent_access = false, bool ordered_access = false);

    /**
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     *
     * @return a Presentation QoS instance with a TOPIC access_score and coherent_access
     * and ordered_access set to the specified values
     */
    static TPresentation TopicAccessScope(bool coherent_access = false, bool ordered_access = false);
};

//==============================================================================

/**
 * This policy indicates the level of reliability requested by a DataReader or
 * offered by a DataWriter. These levels are ordered, BEST_EFFORT being lower
 * than RELIABLE. A DataWriter offering a level is implicitly offering all levels
 * below.
 *
 * The setting of this policy has a dependency on the setting of the ResourceLimits
 * policy. In case the Reliability kind is set to RELIABLE the write operation on
 * the DataWriter may block if the modification would cause data to be lost or
 * else cause one of the limits specified in the ResourceLimits to be exceeded.
 * Under these circumstances, the Reliability max_blocking_time configures the
 * maximum duration the write operation may block.
 *
 * If the Reliability kind is set to RELIABLE, data-samples originating from a
 * single DataWriter cannot be made available to the DataReader if there are
 * previous data-samples that have not been received yet due to a communication
 * error. In other words, the service will repair the error and retransmit
 * data-samples as needed in order to reconstruct a correct snapshot of the
 * DataWriter history before it is accessible by the DataReader.
 *
 * If the Reliability kind is set to BEST_EFFORT, the service will not retransmit
 * missing data-samples. However for data-samples originating from any one DataWriter
 * the service will ensure they are stored in the DataReader history in the same
 * order they originated in the DataWriter. In other words, the DataReader may miss
 * some data-samples but it will never see the value of a data-object change from
 * a newer value to an order value.
 *
 * The value offered is considered compatible with the value requested if and only
 * if the inequality "offered kind >= requested kind" evaluates to TRUE. For the
 * purposes of this inequality, the values of Reliability kind are considered
 * ordered such that BEST_EFFORT < RELIABLE.
 */
template <typename D>
class TReliability : public dds::core::Value<D>
{
public:
    /**
     * Creates a Reliability QoS instance
     *
     * @param kind the kind
     * @param max_blocking_time the max_blocking_time
     */
    TReliability(
        dds::core::policy::ReliabilityKind::Type kind = dds::core::policy::ReliabilityKind::BEST_EFFORT,
        const dds::core::Duration& max_blocking_time = dds::core::Duration::zero());

    /**
     * Copies a Reliability QoS instance
     *
     * @param other the Reliability QoS instance to copy
     */
    TReliability(const TReliability& other);

public:
    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    TReliability& kind(dds::core::policy::ReliabilityKind::Type kind);

    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::ReliabilityKind::Type kind() const;

    /**
     * Sets the max_blocking_time
     *
     * @param max_blocking_time the max_blocking_time
     */
    TReliability& max_blocking_time(const dds::core::Duration& max_blocking_time);

    /**
     * Gets the max_blocking_time
     *
     * @return the max_blocking_time
     */
    const dds::core::Duration max_blocking_time() const;

public:
    /**
     * @param the max_blocking_time
     * @return a Reliability QoS instance with the kind set to RELIABLE and the max_blocking_time
     * set to the supplied value
     */
    static TReliability Reliable(const dds::core::Duration& max_blocking_time = dds::core::Duration(0, 100000000));

    /**
     * @return a Reliability QoS instance with the kind set to BEST_EFFORT
     */
    static TReliability BestEffort();
};

//==============================================================================

/**
 * This policy controls how each subscriber resolves the final value of a data
 * instance that is written by multiple DataWriter objects (which may be
 * associated with different Publisher objects) running on different nodes.
 *
 * The setting BY_RECEPTION_TIMESTAMP indicates that, assuming the OWNERSHIP
 * policy allows it, the latest received value for the instance should be the
 * one whose value is kept. This is the default value.
 *
 * The setting BY_SOURCE_TIMESTAMP indicates that, assuming the OWNERSHIP
 * policy allows it, a timestamp placed at the source should be used. This is
 * the only setting that, in the case of concurrent same-strength DataWriter
 * objects updating the same instance, ensures all subscribers will end up with
 * the same final value for the instance. The mechanism to set the source
 * timestamp is middleware dependent.
 *
 * The value offered is considered compatible with the value requested if and
 * only if the inequality "offered kind >= requested kind" evaluates to TRUE.
 * For the purposes of this inequality, the values of DESTINATION_ORDER kind are
 * considered ordered such that BY_RECEPTION_TIMESTAMP < BY_SOURCE_TIMESTAMP.
 */
template <typename D>
class TDestinationOrder : public dds::core::Value<D>
{
public:
    /**
     * Creates a DestinationOrder QoS instance
     *
     * @param kind the kind
     */
    explicit TDestinationOrder(
        dds::core::policy::DestinationOrderKind::Type kind
        = dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP);

    /**
     * Copies a DestinationOrder QoS instance
     *
     * @param other the Reliability QoS instance to copy
     */
    TDestinationOrder(const TDestinationOrder& other);

public:
    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    TDestinationOrder& kind(dds::core::policy::DestinationOrderKind::Type kind);

    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::DestinationOrderKind::Type kind() const;

public:
    /**
     * @return a DestinationOrder QoS instance with the kind set to BY_SOURCE_TIMESTAMP
     */
    static TDestinationOrder SourceTimestamp();

    /**
     * @return a DestinationOrder QoS instance with the kind set to BY_RECEPTION_TIMESTAMP
     */
    static TDestinationOrder ReceptionTimestamp();
};

//==============================================================================

/**
 * 1. This policy controls the behavior of the Service when the value of an
 * instance changes before it is finally communicated to some of its existing
 * DataReader entities.
 *
 * 2. If the kind is set to KEEP_LAST, then the Service will only attempt to keep
 * the latest values of the instance and discard the older ones. In this case,
 * the value of depth regulates the maximum number of values (up to and including
 * the most current one) the Service will maintain and deliver. The default (and
 * most common setting) for depth is one, indicating that only the most recent
 * value should be delivered.
 *
 * 3. If the kind is set to KEEP_ALL, then the Service will attempt to maintain
 * and deliver all the values of the instance to existing subscribers. The
 * resources that the Service can use to keep this history are limited by the
 * settings of the RESOURCE_LIMITS QoS. If the limit is reached, then the behavior
 * of the Service will depend on the Reliability QoS. If the reliability kind is
 * BEST_EFFORT, then the old values will be discarded. If reliability is RELIABLE,
 * then the Service will block the DataWriter until it can deliver the necessary
 * old values to all subscribers.
 *
 * The setting of History depth must be consistent with the RESOURCE_LIMITS
 * max_samples_per_instance. For these two QoS to be consistent, they must verify
 * that depth <= max_samples_per_instance.
 */
template <typename D>
class THistory : public dds::core::Value<D>
{
public:
    /**
     * Creates a History QoS instance
     *
     * @param kind the kind
     * @param depth the history depth
     */
    THistory(dds::core::policy::HistoryKind::Type kind = dds::core::policy::HistoryKind::KEEP_LAST,
             int32_t depth = 1);

    /**
     * Copies a History QoS instance
     *
     * @param other the History QoS instance to copy
     */
    THistory(const THistory& other);

public:
    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::HistoryKind::Type kind() const;

    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    THistory& kind(dds::core::policy::HistoryKind::Type kind);

    /**
     * Gets the history depth
     *
     * @return the history depth
     */
    int32_t depth() const;

    /**
     * Sets the history depth
     *
     * @param the history depth
     */
    THistory& depth(int32_t depth);

public:
    /**
     * @return a History QoS instance with the kind set to KEEP_ALL
     */
    static THistory KeepAll();

    /**
     * @param depth the history depth
     * @return a History QoS instance with the kind set to KEEP_LAST and the
     * depth set to the supplied value
     */
    static THistory KeepLast(uint32_t depth);
};

//==============================================================================

/**
 * This policy controls the resources that the Service can use in order to meet
 * the requirements imposed by the application and other QoS settings.
 *
 * If the DataWriter objects are communicating samples faster than they are
 * ultimately taken by the DataReader objects, the middleware will eventually
 * hit against some of the QoS-imposed resource limits. Note that this may occur
 * when just a single DataReader cannot keep up with its corresponding DataWriter.
 * The behavior in this case depends on the setting for the RELIABILITY QoS. If
 * reliability is BEST_EFFORT, then the Service is allowed to drop samples. If
 * the reliability is RELIABLE, the Service will block the DataWriter or discard
 * the sample at the DataReader in order not to lose existing samples.
 *
 * The constant LENGTH_UNLIMITED may be used to indicate the absence of a
 * particular limit. For example setting max_samples_per_instance to
 * LENGH_UNLIMITED will cause the middleware to not enforce this particular limit.
 *
 * The setting of RESOURCE_LIMITS max_samples must be consistent with the
 * max_samples_per_instance. For these two values to be consistent they must
 * verify that "max_samples >= max_samples_per_instance".
 *
 * The setting of RESOURCE_LIMITS max_samples_per_instance must be consistent with
 * the HISTORY depth. For these two QoS to be consistent, they must verify that
 * "depth <= max_samples_per_instance".
 *
 * An attempt to set this policy to inconsistent values when an entity is created
 * via a set_qos operation will cause the operation to fail.
 */
template <typename D>
class TResourceLimits : public dds::core::Value<D>
{
public:
    /**
     * Creates a ResourceLimits QoS instance
     *
     * @param max_samples the max_samples value
     * @param max_instances the max_instances value
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    TResourceLimits(int32_t max_samples = dds::core::LENGTH_UNLIMITED,
                    int32_t max_instances = dds::core::LENGTH_UNLIMITED,
                    int32_t max_samples_per_instance = dds::core::LENGTH_UNLIMITED);

    /**
     * Copies a ResourceLimits QoS instance
     *
     * @param other the ResourceLimits QoS instance to copy
     */
    TResourceLimits(const TResourceLimits& other);

public:
    /**
     * Sets the max_samples value
     *
     * @param max_samples the max_samples value
     */
    TResourceLimits& max_samples(int32_t max_samples);

    /**
     * Gets the max_samples value
     *
     * @return the max_samples value
     */
    int32_t max_samples() const;

    /**
     * Sets the max_instances value
     *
     * @param max_instances the max_instances value
     */
    TResourceLimits& max_instances(int32_t max_instances);

    /**
     * Gets the max_instances value
     *
     * @return the max_instances value
     */
    int32_t max_instances() const;

    /**
     * Sets the max_samples_per_instance value
     *
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    TResourceLimits& max_samples_per_instance(int32_t max_samples_per_instance);

    /**
     * Gets the max_samples_per_instance value
     *
     * @return the max_samples_per_instance value
     */
    int32_t max_samples_per_instance() const;
};

//==============================================================================

/**
 * This policy controls the mechanism and parameters used by the Service to
 * ensure that particular entities on the network are still "alive". The
 * liveliness can also affect the ownership of a particular instance, as
 * determined by the Ownership QoS policy.
 *
 * This policy has several settings to support both data-objects that are updated
 * periodically as well as those that are changed sporadically. It also allows
 * customizing for different application requirements in terms of the kinds of
 * failures that will be detected by the liveliness mechanism.
 *
 * The AUTOMATIC liveliness setting is most appropriate for applications that
 * only need to detect failures at the process-level, but not application-logic
 * failures within a process. The Service takes responsibility for renewing the
 * leases at the required rates and thus, as long as the local process where a
 * DomainParticipant is running and the link connecting it to remote participants
 * remains connected, the entities within the DomainParticipant will be
 * considered alive. This requires the lowest overhead.
 *
 * The MANUAL settings (MANUAL_BY_PARTICIPANT, MANUAL_BY_TOPIC) require the
 * application on the publishing side to periodically assert the liveliness
 * before the lease expires to indicate the corresponding Entity is still alive.
 * The action can be explicit by calling the assert_liveliness operations, or
 * implicit by writing some data.
 *
 * The two possible manual settings control the granularity at which the
 * application must assert liveliness.
 *
 * * The setting MANUAL_BY_PARTICIPANT requires only that one Entity within the
 * publisher is asserted to be alive to deduce all other Entity objects within
 * the same DomainParticipant are also alive.
 *
 * * The setting MANUAL_BY_TOPIC requires that at least one instance within the
 * DataWriter is asserted.
 *
 * The value offered is considered compatible with the value requested if and
 * only if the following conditions are met:
 *
 * 1. the inequality "offered kind >= requested kind" evaluates to TRUE. For
 * the purposes of this inequality, the values of Liveliness kind are considered
 * ordered such that: AUTOMATIC < MANUAL_BY_PARTICIPANT < MANUAL_BY_TOPIC.
 *
 * 2. the inequality "offered lease_duration <= requested lease_duration" evaluates
 * to TRUE.
 *
 * Changes in Liveliness must be detected by the Service with a time-granularity
 * greater or equal to the lease_duration. This ensures that the value of the
 * LivelinessChangedStatus is updated at least once during each lease_duration and
 * the related Listeners and WaitSets are notified within a lease_duration from
 * the time the Liveliness changed.
 */
template <typename D>
class TLiveliness : public dds::core::Value<D>
{
public:
    /**
     * Creates a Liveliness QoS instance
     *
     * @param kind the kind
     * @param lease_duration the lease_duration
     */
    TLiveliness(
        dds::core::policy::LivelinessKind::Type kind = dds::core::policy::LivelinessKind::AUTOMATIC,
        const dds::core::Duration& lease_duration = dds::core::Duration::infinite());

    /**
     * Copies a Liveliness QoS instance
     *
     * @param other the Liveliness QoS instance to copy
     */
    TLiveliness(const TLiveliness& other);

public:
    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    TLiveliness& kind(dds::core::policy::LivelinessKind::Type kind);

    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::LivelinessKind::Type kind() const;

    /**
     * Sets the lease_duration
     *
     * @return the lease_duration
     */
    TLiveliness& lease_duration(const dds::core::Duration& lease_duration);

    /**
     * Gets the lease_duration
     *
     * @return the lease_duration
     */
    const dds::core::Duration lease_duration() const;

public:
    /**
     * @return a Liveliness QoS instance with the kind set to AUTOMATIC
     */
    static TLiveliness Automatic();

    /**
     * @return a Liveliness QoS instance with the kind set to MANUAL_BY_PARTICIPANT
     * and the lease_duration set to the supplied value
     */
    static TLiveliness ManualByParticipant(const dds::core::Duration& lease_duration = dds::core::Duration::infinite());

    /**
     * @return a Liveliness QoS instance with the kind set to MANUAL_BY_TOPIC
     * and the lease_duration set to the supplied value
     */
    static TLiveliness ManualByTopic(const dds::core::Duration& lease_duration = dds::core::Duration::infinite());
};


//==============================================================================
#ifdef OMG_DDS_PERSISTENCE_SUPPORT

/**
 * This policy is used to configure the HISTORY QoS and the ResourceLimits QoS
 * used by the DataReader and DataWriter used by the "persistence service".
 * The "persistence service" is the one responsible for implementing the
 * Durability kinds TRANSIENT and PERSISTENT. See Section 7.1.3.4, DURABILITY.
 *
 * The setting of the service_cleanup_delay controls when the TRANSIENT or
 * PERSISTENT service is able to remove all information regarding a data-instances.
 * Information on a data-instance is maintained until the following conditions are
 * met:
 *
 * 1. the instance has been explicitly disposed (instance_state =
 * not_alive_disposed),
 *
 * 2. and while in the not_alive_disposed state the system detects that there
 * are no more "alive" DataWriter entities writing the instance, that is, all
 * existing writers either unregister the instance (call unregister) or lose their
 * liveliness,
 *
 * 3. and a time interval longer that service_cleanup_delay has elapsed since the
 * moment the service detected that the previous two conditions were met.
 *
 * The utility of the service_cleanup_delay is apparent in the situation where an
 * application disposes an instance and it crashes before it has a chance to
 * complete additional tasks related to the disposition. Upon restart the application
 * may ask for initial data to regain its state and the delay introduced by the
 * service_cleanup_delay will allow the restarted application to receive the
 * information on the disposed instance and complete the interrupted tasks.
 */
template <typename D>
class TDurabilityService : public dds::core::Value<D>
{
public:
    /**
     * Creates a DurabilityService QoS instance
     *
     * @param service_cleanup_delay the service_cleanup_delay
     * @param history_kind the history_kind value
     * @param history_depth the history_depth value
     * @param max_samples the max_samples value
     * @param max_instances the max_instances value
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    TDurabilityService(
        const dds::core::Duration& service_cleanup_delay = dds::core::Duration::zero(),
        dds::core::policy::HistoryKind::Type history_kind = dds::core::policy::HistoryKind::KEEP_LAST,
        int32_t history_depth = 1,
        int32_t max_samples = dds::core::LENGTH_UNLIMITED,
        int32_t max_instances = dds::core::LENGTH_UNLIMITED,
        int32_t max_samples_per_instance = dds::core::LENGTH_UNLIMITED);

    /**
     * Copies a DurabilityService QoS instance
     *
     * @param other the DurabilityService QoS instance to copy
     */
    TDurabilityService(const TDurabilityService& other);

public:
    /**
     * Sets the service_cleanup_delay value
     *
     * @param service_cleanup_delay the service_cleanup_delay value
     */
    TDurabilityService& service_cleanup_delay(const dds::core::Duration& service_cleanup_delay);

    /**
     * Gets the service_cleanup_delay value
     *
     * @return the service_cleanup_delay
     */
    const dds::core::Duration service_cleanup_delay() const;

    /**
     * Sets the history_kind
     *
     * @param the history_kind
     */
    TDurabilityService& history_kind(dds::core::policy::HistoryKind::Type history_kind);

    /**
     * Gets the history_kind
     *
     * @return history_kind
     */
    dds::core::policy::HistoryKind::Type history_kind() const;

    /**
     * Sets the history_depth value
     *
     * @param history_depth the history_depth value
     */
    TDurabilityService& history_depth(int32_t history_depth);

    /**
     * Gets the history_depth value
     *
     * @return history_depth
     */
    int32_t history_depth() const;

    /**
     * Sets the max_samples value
     *
     * @param max_samples the max_samples value
     */
    TDurabilityService& max_samples(int32_t max_samples);

    /**
     * Gets the max_samples value
     *
     * @return the max_samples value
     */
    int32_t max_samples() const;

    /**
     * Sets the max_instances value
     *
     * @param max_instances the max_instances value
     */
    TDurabilityService& max_instances(int32_t max_instances);

    /** Gets the max_instances value
     *
     * @return the max_instances value
     */
    int32_t max_instances() const;

    /**
     * Sets the max_samples_per_instance value
     *
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    TDurabilityService& max_samples_per_instance(int32_t max_samples_per_instance);

    /**
     * Gets the max_samples_per_instance value
     *
     * @return the max_samples_per_instance value
     */
    int32_t max_samples_per_instance() const;
};

#endif  // OMG_DDS_PERSISTENCE_SUPPORT
//============================================================================

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

template <typename D>
class TDataRepresentation : public dds::core::Value<D>
{

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
};

#endif  // defined(OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT)


//============================================================================

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

template <typename D>
class TTypeConsistencyEnforcement : public dds::core::Value<D>
{
public:
    explicit TTypeConsistencyEnforcement(dds::core::policy::TypeConsistencyEnforcementKind::Type kind);

public:
    TTypeConsistencyEnforcement& kind(dds::core::policy::TypeConsistencyEnforcementKind::Type kind);
    dds::core::policy::TypeConsistencyEnforcementKind::Type  kind() const;
};

#endif  // defined(OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT)


//==============================================================================


}
}
}

#endif /* OMG_TDDS_CORE_POLICY_CORE_POLICY_HPP_ */
