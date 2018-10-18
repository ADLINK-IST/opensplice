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

#ifndef ORG_OPENSPLICE_CORE_STATUS_STATUS_DELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_STATUS_STATUS_DELEGATE_HPP_

#include <org/opensplice/core/config.hpp>
#include "v_event.h"

namespace org
{
namespace opensplice
{
namespace core
{

class InconsistentTopicStatusDelegate
{
public:
    InconsistentTopicStatusDelegate() : total_count_(0), total_count_change_(0) { }

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

    bool operator ==(const InconsistentTopicStatusDelegate& other) const
    {
        return other.total_count() == total_count_ ;
    }

public:
    void v_status(const v_inconsistentTopicInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
};


class SampleLostStatusDelegate
{
public:
    SampleLostStatusDelegate() : total_count_(0), total_count_change_(0) { }

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

    bool operator ==(const SampleLostStatusDelegate& other) const
    {
        return other.total_count() == total_count_;
    }

public:
    void v_status(const v_sampleLostInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
};

class SampleRejectedStatusDelegate
{
public:
    SampleRejectedStatusDelegate()
        : total_count_(0),
          total_count_change_(0),
          last_reason_(dds::core::status::SampleRejectedState::not_rejected()),
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

    bool operator ==(const SampleRejectedStatusDelegate& other) const
    {
        return other.total_count() == total_count_ &&
               other.last_reason() == last_reason_ &&
               other.last_instance_handle() == last_instance_handle_;
    }

public:
    void v_status(const v_sampleRejectedInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
        switch (from.lastReason) {
        case S_NOT_REJECTED:
            last_reason_ = dds::core::status::SampleRejectedState::not_rejected();
            break;
        case S_REJECTED_BY_INSTANCES_LIMIT:
            last_reason_ = dds::core::status::SampleRejectedState::rejected_by_instances_limit();
            break;
        case S_REJECTED_BY_SAMPLES_LIMIT:
            last_reason_ = dds::core::status::SampleRejectedState::rejected_by_samples_limit();
            break;
        case S_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
            last_reason_ = dds::core::status::SampleRejectedState::rejected_by_samples_per_instance_limit();
            break;
        default:
            ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Invalid SampleRejectedStatus::last_reason from kernel");
            break;
        }
        last_instance_handle_ = dds::core::InstanceHandle(u_instanceHandleFromGID(from.instanceHandle));
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::status::SampleRejectedState last_reason_;
    dds::core::InstanceHandle last_instance_handle_;
};


class LivelinessLostStatusDelegate
{
public:
    LivelinessLostStatusDelegate() : total_count_(0), total_count_change_(0) { }

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

    bool operator ==(const LivelinessLostStatusDelegate& other) const
    {
        return other.total_count() == total_count_;
    }

public:
    void v_status(const v_livelinessLostInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
};


class LivelinessChangedStatusDelegate 
{
public:
    LivelinessChangedStatusDelegate() :
        alive_count_(0), 
        not_alive_count_(0), 
        alive_count_change_(0), 
        not_alive_count_change_(0),
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

    bool operator ==(const LivelinessChangedStatusDelegate& other) const
    {
        return other.alive_count() == alive_count_ &&
               other.not_alive_count() == not_alive_count_;
    }

public:
    void v_status(const v_livelinessChangedInfo &from)
    {
        alive_count_ = from.activeCount;
        not_alive_count_ = from.inactiveCount;
        alive_count_change_ = from.activeChanged;
        not_alive_count_change_ = from.inactiveChanged;
        last_publication_handle_ = dds::core::InstanceHandle(u_instanceHandleFromGID(from.instanceHandle));
    }

protected:
    int32_t alive_count_;
    int32_t not_alive_count_;
    int32_t alive_count_change_;
    int32_t not_alive_count_change_;
    dds::core::InstanceHandle last_publication_handle_;

};

class OfferedDeadlineMissedStatusDelegate
{
public:
    OfferedDeadlineMissedStatusDelegate() :
        total_count_(0), 
        total_count_change_(0), 
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

    bool operator ==(const OfferedDeadlineMissedStatusDelegate& other) const
    {
        return other.total_count() == total_count_ &&
               other.last_instance_handle() == last_instance_handle_;
    }

public:
    void v_status(const v_deadlineMissedInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
        last_instance_handle_ = dds::core::InstanceHandle(from.instanceHandle);
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::InstanceHandle last_instance_handle_;
};

class RequestedDeadlineMissedStatusDelegate
{
public:
    RequestedDeadlineMissedStatusDelegate() :
        total_count_(0), 
        total_count_change_(0), 
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

    bool operator ==(const RequestedDeadlineMissedStatusDelegate& other) const
    {
        return other.total_count() == total_count_ &&
               other.last_instance_handle() == last_instance_handle_;
    }

public:
    void v_status(const v_deadlineMissedInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
        last_instance_handle_ = dds::core::InstanceHandle(from.instanceHandle);
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::InstanceHandle last_instance_handle_;
};


class OfferedIncompatibleQosStatusDelegate
{
public:
    OfferedIncompatibleQosStatusDelegate() : 
        total_count_(0), 
        total_count_change_(0), 
        last_policy_id_(0) { }

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

    bool operator ==(const OfferedIncompatibleQosStatusDelegate& other) const
    {
        return other.total_count() == total_count_ &&
               other.last_policy_id() == last_policy_id_ &&
               other.policies() == policies_;
    }

public:
    void v_status(const v_incompatibleQosInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
        last_policy_id_ = from.lastPolicyId;
        policies_.clear();
        for (c_long i = 0; (i < V_POLICY_ID_COUNT) && (i < from.totalCount); i++)
        {
          if (from.policyCount[i] != 0) {
              dds::core::policy::QosPolicyCount policy(i, from.policyCount[i]);
              policies_.push_back(policy);
          }
        }
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::policy::QosPolicyId last_policy_id_;
    dds::core::policy::QosPolicyCountSeq policies_;
};

class RequestedIncompatibleQosStatusDelegate
{
public:
    RequestedIncompatibleQosStatusDelegate() :
        total_count_(0), 
        total_count_change_(0), 
        last_policy_id_(0) { }

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

    bool operator ==(const RequestedIncompatibleQosStatusDelegate& other) const
    {
        return other.total_count() == total_count_ &&
               other.last_policy_id() == last_policy_id_ &&
               other.policies() == policies_;
    }

public:
    void v_status(const v_incompatibleQosInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
        last_policy_id_ = from.lastPolicyId;
        policies_.clear();
        for (c_long i = 0; (i < V_POLICY_ID_COUNT) && (i < from.totalCount); i++)
        {
          if (from.policyCount[i] != 0) {
              dds::core::policy::QosPolicyCount policy(i, from.policyCount[i]);
              policies_.push_back(policy);
          }
        }
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    dds::core::policy::QosPolicyId last_policy_id_;
    dds::core::policy::QosPolicyCountSeq policies_;

};


class PublicationMatchedStatusDelegate
{
public:
    PublicationMatchedStatusDelegate() :
        total_count_(0), 
        total_count_change_(0), 
        current_count_(0), 
        current_count_change_(0), 
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

    bool operator ==(const PublicationMatchedStatusDelegate& other) const
    {
        return other.total_count() == total_count_ &&
               other.current_count() == current_count_ &&
               other.last_subscription_handle() == last_subscription_handle_;
    }

public:
    void v_status(const v_topicMatchInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
        current_count_ = from.currentCount;
        current_count_change_ = from.currentChanged;
        last_subscription_handle_ = dds::core::InstanceHandle(u_instanceHandleFromGID(from.instanceHandle));
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    int32_t current_count_;
    int32_t current_count_change_;
    dds::core::InstanceHandle last_subscription_handle_;
};

class SubscriptionMatchedStatusDelegate
{
public:
    SubscriptionMatchedStatusDelegate() :
        total_count_(0), 
        total_count_change_(0), 
        current_count_(0), 
        current_count_change_(0), 
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

    bool operator ==(const SubscriptionMatchedStatusDelegate& other) const
    {
        return other.total_count() == total_count_ &&
               other.current_count() == current_count_ &&
               other.last_publication_handle() == last_publication_handle_;
    }

public:
    void v_status(const v_topicMatchInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
        current_count_ = from.currentCount;
        current_count_change_ = from.currentChanged;
        last_publication_handle_ = dds::core::InstanceHandle(u_instanceHandleFromGID(from.instanceHandle));
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
    int32_t current_count_;
    int32_t current_count_change_;
    dds::core::InstanceHandle last_publication_handle_;

};

class AllDataDisposedTopicStatusDelegate
{
public:
    AllDataDisposedTopicStatusDelegate() :
        total_count_(0), 
        total_count_change_(0) { }

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

    bool operator ==(const AllDataDisposedTopicStatusDelegate& other) const
    {
        return other.total_count() == total_count_;
    }

public:
    void v_status(const v_allDataDisposedInfo &from)
    {
        total_count_ = from.totalCount;
        total_count_change_ = from.totalChanged;
    }

protected:
    int32_t total_count_;
    int32_t total_count_change_;
};


}
}
}  /* namespace org::opensplice::core */

#endif /* ORG_OPENSPLICE_CORE_STATUS_STATUS_DELEGATE_HPP_ */
