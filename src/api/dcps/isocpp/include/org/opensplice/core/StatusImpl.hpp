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

#ifndef ORG_OPENSPLICE_CORE_STATUS_STATUS_IMPL_HPP_
#define ORG_OPENSPLICE_CORE_STATUS_STATUS_IMPL_HPP_

namespace org
{
namespace opensplice
{
namespace core
{

class InconsistentTopicStatusImpl
{
public:
    InconsistentTopicStatusImpl() { }

public:
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    bool operator ==(const InconsistentTopicStatusImpl& other) const
    {
        return other.total_count() == total_count_ && other.total_count_change() == total_count_change_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
};


class SampleLostStatusImpl
{
public:
    SampleLostStatusImpl() { }

public:
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    bool operator ==(const SampleLostStatusImpl& other) const
    {
        return other.total_count() == total_count_ && other.total_count_change() == total_count_change_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
};

class SampleRejectedStatusImpl
{
public:
    SampleRejectedStatusImpl()
        : last_reason_(dds::core::status::SampleRejectedState::not_rejected()),
          last_instance_handle_(dds::core::null) { }

public:
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    dds::core::status::SampleRejectedState last_reason() const
    {
        return last_reason_;
    }

    const dds::core::InstanceHandle last_instance_handle() const
    {
        return last_instance_handle_;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    void last_reason(dds::core::status::SampleRejectedState last_reason)
    {
        last_reason_ = last_reason;
    }

    void last_instance_handle(dds::core::InstanceHandle last_instance_handle)
    {
        last_instance_handle_ = last_instance_handle;
    }

    bool operator ==(const SampleRejectedStatusImpl& other) const
    {
        return other.total_count() == total_count_ &&
               other.total_count_change() == total_count_change_ &&
               other.last_reason() == last_reason_ &&
               other.last_instance_handle() == last_instance_handle_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::status::SampleRejectedState last_reason_;
    dds::core::InstanceHandle last_instance_handle_;
};


class LivelinessLostStatusImpl
{
public:
    LivelinessLostStatusImpl() { }

public:
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    bool operator ==(const LivelinessLostStatusImpl& other) const
    {
        return other.total_count() == total_count_ &&
               other.total_count_change() == total_count_change_;
    }
protected:
    int32_t total_count_;
    int32_t total_count_change_;
};


class LivelinessChangedStatusImpl
{
public:
    LivelinessChangedStatusImpl() :
        last_publication_handle_(dds::core::null) { }

public:

    int32_t alive_count() const
    {
        return alive_count_;
    }

    int32_t not_alive_count() const
    {
        return not_alive_count_;
    }

    int32_t alive_count_change() const
    {
        return alive_count_change_;
    }

    int32_t not_alive_count_change() const
    {
        return not_alive_count_change_;
    }

    void alive_count(int32_t alive_count)
    {
        alive_count_ = alive_count;
    }

    void not_alive_count(int32_t not_alive_count)
    {
        not_alive_count_ = not_alive_count;
    }

    void alive_count_change(int32_t alive_count_change)
    {
        alive_count_change_ = alive_count_change;
    }

    void not_alive_count_change(int32_t not_alive_count_change)
    {
        not_alive_count_change_ = not_alive_count_change;
    }

    void last_publication_handle(dds::core::InstanceHandle last_publication_handle)
    {
        last_publication_handle_ = last_publication_handle;
    }

    dds::core::InstanceHandle last_publication_handle() const
    {
        return last_publication_handle_;
    }

    bool operator ==(const LivelinessChangedStatusImpl& other) const
    {
        return other.alive_count() == alive_count_ &&
               other.not_alive_count() == not_alive_count_ &&
               other.alive_count_change() == alive_count_change_ &&
               other.not_alive_count_change() == not_alive_count_change_;
    }

protected:
    int32_t alive_count_;
    int32_t not_alive_count_;
    int32_t alive_count_change_;
    int32_t not_alive_count_change_;
    dds::core::InstanceHandle last_publication_handle_;

};

class OfferedDeadlineMissedStatusImpl
{
public:
    OfferedDeadlineMissedStatusImpl() :
        last_instance_handle_(dds::core::null) { }

public:
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    const dds::core::InstanceHandle last_instance_handle() const
    {
        return last_instance_handle_;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    void last_instance_handle(dds::core::InstanceHandle last_instance_handle)
    {
        last_instance_handle_ = last_instance_handle;
    }

    bool operator ==(const OfferedDeadlineMissedStatusImpl& other) const
    {
        return other.total_count() == total_count_ &&
               other.total_count_change() == total_count_change_ &&
               other.last_instance_handle() == last_instance_handle_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::InstanceHandle last_instance_handle_;
};

class RequestedDeadlineMissedStatusImpl
{
public:
    RequestedDeadlineMissedStatusImpl() :
        last_instance_handle_(dds::core::null) { }

public:
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    const dds::core::InstanceHandle last_instance_handle() const
    {
        return last_instance_handle_;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    void last_instance_handle(dds::core::InstanceHandle last_instance_handle)
    {
        last_instance_handle_ = last_instance_handle;
    }

    bool operator ==(const RequestedDeadlineMissedStatusImpl& other) const
    {
        return other.total_count() == total_count_ &&
               other.total_count_change() == total_count_change_ &&
               other.last_instance_handle() == last_instance_handle_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::InstanceHandle last_instance_handle_;
};


class OfferedIncompatibleQosStatusImpl
{
public:
    OfferedIncompatibleQosStatusImpl() { }

public  :
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    dds::core::policy::QosPolicyId last_policy_id() const
    {
        return last_policy_id_;
    }

    const dds::core::policy::QosPolicyCountSeq policies() const
    {
        return policies_;
    }

    const dds::core::policy::QosPolicyCountSeq& policies(dds::core::policy::QosPolicyCountSeq& dst) const
    {
        dst = policies_;
        return dst;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    void last_policy_id(dds::core::policy::QosPolicyId last_policy_id)
    {
        last_policy_id_ = last_policy_id;
    }

    void set_policies(dds::core::policy::QosPolicyCountSeq policies)
    {
        policies_ = policies;
    }

    bool operator ==(const OfferedIncompatibleQosStatusImpl& other) const
    {
        return other.total_count() == total_count_ &&
               other.total_count_change() == total_count_change_ &&
               other.last_policy_id() == last_policy_id_ &&
               other.policies() == policies_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::policy::QosPolicyId last_policy_id_;
    dds::core::policy::QosPolicyCountSeq policies_;
};

class RequestedIncompatibleQosStatusImpl
{
public:
    RequestedIncompatibleQosStatusImpl() { }

public  :
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    dds::core::policy::QosPolicyId last_policy_id() const
    {
        return last_policy_id_;
    }

    const dds::core::policy::QosPolicyCountSeq policies() const
    {
        return policies_;
    }

    const dds::core::policy::QosPolicyCountSeq& policies(dds::core::policy::QosPolicyCountSeq& dst) const
    {
        dst = policies_;
        return dst;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    void last_policy_id(dds::core::policy::QosPolicyId last_policy_id)
    {
        last_policy_id_ = last_policy_id;
    }

    void set_policies(dds::core::policy::QosPolicyCountSeq policies)
    {
        policies_ = policies;
    }

    bool operator ==(const RequestedIncompatibleQosStatusImpl& other) const
    {
        return other.total_count() == total_count_ &&
               other.total_count_change() == total_count_change_ &&
               other.last_policy_id() == last_policy_id_ &&
               other.policies() == policies_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::policy::QosPolicyId last_policy_id_;
    dds::core::policy::QosPolicyCountSeq policies_;

};


class PublicationMatchedStatusImpl
{
public:
    PublicationMatchedStatusImpl() :
        last_subscription_handle_(dds::core::null) { }

public:
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    int32_t current_count() const
    {
        return current_count_;
    }

    int32_t current_count_change() const
    {
        return current_count_change_;
    }

    const dds::core::InstanceHandle last_subscription_handle() const
    {
        return last_subscription_handle_;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    void current_count(int32_t current_count)
    {
        current_count_ = current_count;
    }

    void current_count_change(int32_t current_count_change)
    {
        current_count_change_ = current_count_change;
    }

    void last_subscription_handle(dds::core::InstanceHandle last_subscription_handle)
    {
        last_subscription_handle_ = last_subscription_handle;
    }

    bool operator ==(const PublicationMatchedStatusImpl& other) const
    {
        return other.total_count() == total_count_ &&
               other.total_count_change() == total_count_change_ &&
               other.current_count() == current_count_ &&
               other.current_count_change() == current_count_change_ &&
               other.last_subscription_handle() == last_subscription_handle_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    int32_t current_count_;
    int32_t current_count_change_;
    dds::core::InstanceHandle last_subscription_handle_;
};

class SubscriptionMatchedStatusImpl
{
public:
    SubscriptionMatchedStatusImpl() :
        last_publication_handle_(dds::core::null) { }

public:
    int32_t total_count() const
    {
        return total_count_;
    }

    int32_t total_count_change() const
    {
        return total_count_change_;
    }

    int32_t current_count() const
    {
        return current_count_;
    }

    int32_t current_count_change() const
    {
        return current_count_change_;
    }

    const dds::core::InstanceHandle last_publication_handle() const
    {
        return last_publication_handle_;
    }

    void total_count(int32_t total_count)
    {
        total_count_ = total_count;
    }

    void total_count_change(int32_t total_count_change)
    {
        total_count_change_ = total_count_change;
    }

    void current_count(int32_t current_count)
    {
        current_count_ = current_count;
    }

    void current_count_change(int32_t current_count_change)
    {
        current_count_change_ = current_count_change;
    }

    void last_publication_handle(dds::core::InstanceHandle last_publication_handle)
    {
        last_publication_handle_ = last_publication_handle;
    }

    bool operator ==(const SubscriptionMatchedStatusImpl& other) const
    {
        return other.total_count() == total_count_ &&
               other.total_count_change() == total_count_change_ &&
               other.current_count() == current_count_ &&
               other.current_count_change() == current_count_change_ &&
               other.last_publication_handle() == last_publication_handle_;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    int32_t current_count_;
    int32_t current_count_change_;
    dds::core::InstanceHandle last_publication_handle_;

};

}
}
}  /* namespace org::opensplice::core */

#endif /* ORG_OPENSPLICE_CORE_STATUS_STATUS_IMPL_HPP_ */
