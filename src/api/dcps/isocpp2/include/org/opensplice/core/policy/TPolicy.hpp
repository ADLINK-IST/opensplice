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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_POLICY_TPOLICY_HPP_
#define ORG_OPENSPLICE_CORE_POLICY_TPOLICY_HPP_

#include <org/opensplice/core/policy/ProprietaryPolicyKind.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
namespace policy
{


/**
 */
template <typename D>
class TShare : public dds::core::Value<D>
{
public:
    /**
     * Creates a Share QoS instance
     */
    TShare() { }

    /**
     * Creates a Share QoS instance
     *
     * @param name the name
     * @param enable state
     */
    TShare(const std::string& name, bool enable) : dds::core::Value<D>(name, enable) { }

    /**
     * Copies a Share QoS instance
     *
     * @param other the TShare QoS instance to copy
     */
    TShare(const TShare& other) : dds::core::Value<D>(other.delegate()) { }

public:
    /**
     * Sets the name
     *
     * @param name the name
     */
    TShare& name(const std::string& name)
    {
        this->delegate().name(name);
        return *this;
    }

    /**
     * Gets the name
     *
     * @return the name
     */
    std::string name() const
    {
        return this->delegate().name();
    }

    /**
     * Sets enable state
     *
     * @param enable state
     */
    TShare& enable(bool enable)
    {
        this->delegate().enable(enable);
        return *this;
    }

    /**
     * Gets enable state
     *
     * @return enable state
     */
    bool enable() const
    {
        return this->delegate().enable();
    }
};




template <typename D>
class TProductData : public dds::core::Value<D>
{
public:
    /**
     * Creates a ProductData QoS instance
     */
    TProductData() { }

    /**
     * Creates a ProductData QoS instance
     *
     * @param value the value
     */
    TProductData(const std::string& name) : dds::core::Value<D>(name) { }

    /**
     * Copies a ProductData QoS instance
     *
     * @param other the TProductData QoS instance to copy
     */
    TProductData(const TProductData& other) : dds::core::Value<D>(other.delegate()) { }

public:
    /**
     * Sets the name
     *
     * @param value the name
     */
    TProductData& name(const std::string& name)
    {
        this->delegate().name(name);
        return *this;
    }

    /**
     * Gets the name
     *
     * @return the name
     */
    std::string name() const
    {
        return this->delegate().name();
    }
};


template <typename D>
class TSubscriptionKey : public dds::core::Value<D>
{
public:
    /**
     * Creates a SubscriptionKey QoS instance
     */
    TSubscriptionKey() { }

    /**
     * Creates a SubscriptionKey QoS instance
     *
     * @param use_key_list use the key list or not
     * @param key a single key
     */
    explicit TSubscriptionKey(bool use_key_list, const std::string& key) :
        dds::core::Value<D>(use_key_list, key) { }

    /**
     * Creates a SubscriptionKey QoS instance
     *
     * @param use_key_list use the key list or not
     * @param keys a sequence containing multiple keys
     */
    explicit TSubscriptionKey(bool use_key_list, const dds::core::StringSeq& keys) :
        dds::core::Value<D>(use_key_list, keys) { }


    /**
     * Copies a SubscriptionKey QoS instance
     *
     * @param other the TSubscriptionKey QoS instance to copy
     */
    TSubscriptionKey(const TSubscriptionKey& other) :
        dds::core::Value<D>(other.delegate()) { }

public:
    /**
     * Sets the key
     *
     * @param key the key
     */
    TSubscriptionKey& key(const std::string& key)
    {
        this->delegate().key(key);
        return *this;
    }

    /**
     * Sets multiple keys
     *
     * @param names a sequence containing multiple keys
     */
    TSubscriptionKey& key(const dds::core::StringSeq& keys)
    {
        this->delegate().key(keys);
        return *this;
    }

    /**
     * Gets the keys
     *
     * @return a sequence containing the keys
     */
    const dds::core::StringSeq key() const
    {
        return this->delegate().key();
    }

    /**
     * Sets use_key_list state
     *
     * @param use_key_list state
     */
    TSubscriptionKey& use_key_list(bool use_key_list)
    {
        this->delegate().use_key_list(use_key_list);
        return *this;
    }

    /**
     * Gets use_key_list state
     *
     * @return use_key_list state
     */
    bool use_key_list() const
    {
        return this->delegate().use_key_list();
    }
};

template <typename D>
class TReaderLifespan : public dds::core::Value<D>
{
public:
    /**
     * Creates a ReaderLifespan QoS instance
     */
    TReaderLifespan() { }

    /**
     * Creates a ReaderLifespan QoS instance
     *
     * @param used Indicates ReaderLifespan is used
     * @param duration ReaderLifespan expiration duration
     */
    explicit TReaderLifespan(bool used, const dds::core::Duration& duration) :
        dds::core::Value<D>(used, duration) { }

    /**
     * Copies a Lifespan QoS instance
     *
     * @param other the Lifespan QoS instance to copy
     */
    TReaderLifespan(const TReaderLifespan& other) :
        dds::core::Value<D>(other.delegate()) { }


public:
    /**
     * Sets the used flag
     *
     * @param the used flag
     */
    TReaderLifespan& used(bool used)
    {
        this->delegate().used(used);
        return *this;
    }

    /**
     * Gets the used flag
     *
     * @return true if used
     */
    bool used() const
    {
        return this->delegate().used();
    }

    /**
     * Sets the expiration duration
     *
     * @param duration expiration duration
     */
    TReaderLifespan& duration(const dds::core::Duration& duration)
    {
        this->delegate().duration(duration);
        return *this;
    }

    /**
     * Gets the expiration duration
     *
     * @return expiration duration
     */
    const dds::core::Duration duration() const
    {
        return this->delegate().duration();
    }
};




/**
 * Base scheduling class
 */
template <typename D>
class TScheduling : public dds::core::Value<D>
{
protected:
    /**
     * Creates a Scheduling QoS instance
     */
    TScheduling() { }

    /**
     * Creates a Scheduling QoS instance
     *
     * @param value the value
     */
    TScheduling(const org::opensplice::core::policy::SchedulingKind::Type& scheduling_kind,
                const org::opensplice::core::policy::SchedulingPriorityKind::Type& scheduling_priority_kind,
                int32_t scheduling_priority)
        : dds::core::Value<D>(scheduling_kind, scheduling_priority_kind, scheduling_priority)
    {
    }

    /**
     * Copies a Scheduling QoS instance
     *
     * @param other the Scheduling QoS instance to copy
     */
    TScheduling(const TScheduling& other)
        : dds::core::Value<D>(other.delegate())
    {
    }

public:
    /**
     * Sets the scheduling kind
     *
     * @param scheduling_kind the scheduling_kind
     */
    TScheduling& scheduling_kind(const org::opensplice::core::policy::SchedulingKind::Type& scheduling_kind)
    {
        this->delegate().scheduling_kind(scheduling_kind);
        return *this;
    }

    /**
     * Gets the scheduling kind
     *
     * @return the scheduling_kind
     */
    org::opensplice::core::policy::SchedulingKind::Type scheduling_kind() const
    {
        return this->delegate().scheduling_kind();
    }

    /**
     * Sets the scheduling priority kind
     *
     * @param scheduling_priority_kind the scheduling_priority_kind
     */
    TScheduling& scheduling_priority_kind(const org::opensplice::core::policy::SchedulingPriorityKind::Type& scheduling_priority_kind)
    {
        this->delegate().scheduling_priority_kind(scheduling_priority_kind);
        return *this;
    }

    /**
     * Gets the scheduling priority kind
     *
     * @return the scheduling_priority_kind
     */
    org::opensplice::core::policy::SchedulingPriorityKind::Type scheduling_priority_kind() const
    {
        return this->delegate().scheduling_priority_kind();
    }

    /**
     * Sets the scheduling priority
     *
     * @param scheduling_priority the scheduling_priority
     */
    TScheduling& scheduling_priority(int32_t scheduling_priority)
    {
        this->delegate().scheduling_priority(scheduling_priority);
        return *this;
    }

    /**
     * Gets the scheduling priority
     *
     * @return the scheduling_priority
     */
    int32_t scheduling_priority() const
    {
        return this->delegate().scheduling_priority();
    }
};


/**
 * Listener specific scheduling class
 */
template <typename D>
class TListenerScheduling : public org::opensplice::core::policy::TScheduling<D>
{
public:
    /**
     * Creates a ListenerScheduling QoS instance
     */
    TListenerScheduling() { }
    explicit TListenerScheduling(const org::opensplice::core::policy::SchedulingKind::Type& scheduling_kind,
                                 const org::opensplice::core::policy::SchedulingPriorityKind::Type& scheduling_priority_kind,
                                 int32_t scheduling_priority)
        : org::opensplice::core::policy::TScheduling<D>(scheduling_kind, scheduling_priority_kind, scheduling_priority)
    {
    }

    /**
     * Copies a ListenerScheduling QoS instance
     *
     * @param other the ListenerScheduling QoS instance to copy
     */
    TListenerScheduling(const TListenerScheduling& other)
        : org::opensplice::core::policy::TScheduling<D>(other)
    {
    }
};


/**
 * Watchdog specific scheduling class
 */
template <typename D>
class TWatchdogScheduling : public org::opensplice::core::policy::TScheduling<D>
{
public:
    /**
     * Creates a WatchdogScheduling QoS instance
     */
    TWatchdogScheduling() { }
    explicit TWatchdogScheduling(const org::opensplice::core::policy::SchedulingKind::Type& scheduling_kind,
                                 const org::opensplice::core::policy::SchedulingPriorityKind::Type& scheduling_priority_kind,
                                 int32_t scheduling_priority)
        : org::opensplice::core::policy::TScheduling<D>(scheduling_kind, scheduling_priority_kind, scheduling_priority)
    {
    }

    /**
     * Copies a WatchdogScheduling QoS instance
     *
     * @param other the WatchdogScheduling QoS instance to copy
     */
    TWatchdogScheduling(const TWatchdogScheduling& other)
        : org::opensplice::core::policy::TScheduling<D>(other)
    {
    }

};


}
}
}
}


#endif /* ORG_OPENSPLICE_CORE_POLICY_TPOLICY_HPP_ */
