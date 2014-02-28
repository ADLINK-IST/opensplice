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
