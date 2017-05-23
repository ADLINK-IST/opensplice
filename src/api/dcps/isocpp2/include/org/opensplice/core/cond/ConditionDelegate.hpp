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

#ifndef ORG_OPENSPLICE_CORE_COND_CONDITION_DELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_COND_CONDITION_DELEGATE_HPP_

#include <org/opensplice/core/UserObjectDelegate.hpp>
#include <org/opensplice/core/cond/FunctorHolder.hpp>
#include <org/opensplice/core/Mutex.hpp>
#include <org/opensplice/core/ScopedLock.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

#include <set>

namespace dds
{
namespace core
{
namespace cond
{
template <typename DELEGATE> class TCondition;
}
}
}

namespace org
{
namespace opensplice
{
namespace core
{
namespace cond
{

class WaitSetDelegate;

class OMG_DDS_API ConditionDelegate : public virtual org::opensplice::core::UserObjectDelegate
{
public:
    typedef std::set<WaitSetDelegate *> waitsetList;
    typedef waitsetList::iterator       waitsetIterator;
    typedef ::dds::core::smart_ptr_traits< ConditionDelegate >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< ConditionDelegate >::weak_ref_type weak_ref_type;

    ConditionDelegate();

    ~ConditionDelegate();

    void init(ObjectDelegate::weak_ref_type weak_ref);

    void close();

    virtual bool trigger_value() const = 0;

    template <typename FUN>
    void set_handler(FUN& functor) {
        org::opensplice::core::ScopedObjectLock scopedLock(*this);

        if (this->myFunctor) {
            delete this->myFunctor;
        }
        myFunctor = new org::opensplice::core::cond::FunctorHolder<FUN>(functor);
    }
    template <typename FUN>
    void set_handler(const FUN& functor) {
        org::opensplice::core::ScopedObjectLock scopedLock(*this);

        if (this->myFunctor) {
            delete this->myFunctor;
        }
        myFunctor = new org::opensplice::core::cond::ConstFunctorHolder<FUN>(functor);
    }

    void reset_handler();

    virtual void add_waitset(
            const dds::core::cond::TCondition<ConditionDelegate>& cond,
            org::opensplice::core::cond::WaitSetDelegate *waitset);

    virtual bool remove_waitset(
            org::opensplice::core::cond::WaitSetDelegate *waitset);

    virtual void dispatch();

    virtual u_observable get_user_condition();

    u_observable get_user_condition_unlocked();

    dds::core::cond::TCondition<ConditionDelegate> wrapper();

protected:
    waitsetList waitsets;
    org::opensplice::core::Mutex waitSetListMutex;

private:
    org::opensplice::core::cond::FunctorHolderBase *myFunctor;
};

}
}
}
}

#endif /* ORG_OPENSPLICE_CORE_COND_CONDITION_DELEGATE_HPP_ */
