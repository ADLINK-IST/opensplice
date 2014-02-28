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

#ifndef ORG_OPENSPLICE_CORE_STATUSCONDITIONIMPL_HPP_
#define ORG_OPENSPLICE_CORE_STATUSCONDITIONIMPL_HPP_

#include <dds/core/status/State.hpp>
#include <dds/core/Entity.hpp>
#include <org/opensplice/core/ConditionImpl.hpp>
#include <dds/sub/cond/detail/Executor.hpp>
#include <org/opensplice/core/exception_helper.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

/**
 *  @internal With StatusCondition, a handler functor can be passed in at construction time which is then
 * executed by WaitSetImpl which calls it's dispatch member function when the condition is triggered.
 */
class StatusConditionImpl : public org::opensplice::core::ConditionImpl
{
public:
    StatusConditionImpl() : executor_(0), entity_(dds::core::null) { }

    StatusConditionImpl(const dds::core::Entity& entity) : executor_(0), entity_(entity)
    {
        status_condition_ = entity_->get_dds_entity()->get_statuscondition();
        if(status_condition_.in() == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                        OSPL_CONTEXT_LITERAL("dds::core::NullReferenceError : Unable to get StatusCondition. "
                                             "Nil return from ::get_statuscondition")));
        dds::core::status::StatusMask tmp(status_condition_->get_enabled_statuses());
        mask_ = tmp;
        condition_ = status_condition_.in();
    }

    template<typename FUN>
    StatusConditionImpl(const dds::core::Entity& e, const FUN& functor)
        : executor_(new dds::sub::cond::detail::ParametrizedExecutor<FUN, dds::core::Entity >(
                        functor, e)), entity_(e)
    {
        status_condition_ = entity_->get_dds_entity()->get_statuscondition();
        if(status_condition_.in() == 0)
        {
            throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                                                    OSPL_CONTEXT_LITERAL("dds::core::NullReferenceError : Unable to get StatusCondition. "
                                                            "Nil return from ::get_statuscondition")));
        }
        dds::core::status::StatusMask tmp(status_condition_->get_enabled_statuses());
        mask_ = tmp;
        condition_ = status_condition_.in();
    }

    ~StatusConditionImpl()
    {
        if(executor_)
        {
            delete executor_;
        }
    }

    void enabled_statuses(const dds::core::status::StatusMask& status)
    {
        DDS::ReturnCode_t result = status_condition_->set_enabled_statuses(status.to_ulong());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_enabled_statuses"));
        mask_ = status;
    }

    const dds::core::status::StatusMask enabled_statuses() const
    {
        return mask_;
    }

    dds::core::Entity& entity()
    {
        return entity_;
    }

    virtual void dispatch()
    {
        executor_->exec();
    }
private:
    DDS::StatusCondition_var status_condition_;
    dds::sub::cond::detail::Executor* executor_;
    dds::core::Entity entity_;
    dds::core::status::StatusMask mask_;
};

}
}
}

#endif  /* ORG_OPENSPLICE_CORE_STATUSCONDITIONIMPL_HPP_ */
