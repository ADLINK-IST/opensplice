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

#ifndef ORG_OPENSPLICE_CORE_GUARD_CONDITION_IMPL_HPP_
#define ORG_OPENSPLICE_CORE_GUARD_CONDITION_IMPL_HPP_

#include <org/opensplice/core/FunctorHolder.hpp>
#include <org/opensplice/core/ConditionImpl.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

/**
 *  @internal With GuardCondition, a handler functor can be passed in at construction time which is then
 * executed by WaitSetImpl which calls it's dispatch member function when the condition is triggered.
 */
class GuardConditionImpl : public org::opensplice::core::ConditionImpl
{
public:
    GuardConditionImpl() : fholder_(0)
    {
        guard_condition_ = new DDS::GuardCondition();
        condition_ = guard_condition_.in();
    }

    template <typename Functor>
    GuardConditionImpl(const Functor& func) : fholder_(new VoidFunctorHolder<Functor>(func))
    {
        guard_condition_ = new DDS::GuardCondition();
        condition_ = guard_condition_.in();
    }

    virtual ~GuardConditionImpl()
    {

    }

    inline void trigger_value(bool b)
    {
        guard_condition_->set_trigger_value(b);
    }

    using org::opensplice::core::ConditionImpl::get_trigger_value;

    virtual void dispatch()
    {
        fholder_->invoke();
    }

public:
    template <typename Functor>
    void set_handler(const Functor& func)
    {
        org::opensplice::core::FunctorHolder* tmp = fholder_;
        fholder_ = new org::opensplice::core::VoidFunctorHolder<Functor>(func);
        if(tmp != 0)
        {
            delete tmp;
        }
    }

    void reset_handler()
    {
        if(fholder_)
        {
            org::opensplice::core::FunctorHolder* tmp = fholder_;
            fholder_ = 0;
            delete tmp;
        }
    }

private:
    GuardConditionImpl& operator= (const GuardConditionImpl&);
    GuardConditionImpl(const GuardConditionImpl&);
    DDS::GuardCondition_var guard_condition_;

private:
    org::opensplice::core::FunctorHolder* fholder_;
};

}
}
}

#endif /* ORG_OPENSPLICE_CORE_GUARD_CONDITION_IMPL_HPP_ */
