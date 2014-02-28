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
#ifndef OSPL_DDS_SUB_COND_DETAIL_READCONDITION_HPP_
#define OSPL_DDS_SUB_COND_DETAIL_READCONDITION_HPP_

/**
 * @file
 */

// Implementation

#include <dds/core/ref_traits.hpp>
#include <org/opensplice/core/ConditionImpl.hpp>
#include <dds/sub/cond/detail/Executor.hpp>
#include <dds/sub/AnyDataReader.hpp>
#include <org/opensplice/core/exception_helper.hpp>

namespace dds
{
namespace sub
{
namespace cond
{
namespace detail
{

/**
 *  @internal With ReadCondition, a handler functor can be passed in at construction time which is then
 * executed by WaitSetImpl which calls it's dispatch member function when the condition is triggered.
 */
class ReadCondition: public org::opensplice::core::ConditionImpl
{

public:
    ReadCondition() : executor_(0), adr_(dds::core::null) { }

    ReadCondition(const dds::sub::AnyDataReader& adr,
                  const dds::sub::status::DataState& status,
                  bool query = false) :
        executor_(0), adr_(adr), status_(status)
    {
        if(!query)
        {
            read_condition_ = adr_->get_dds_datareader()->create_readcondition(status_.sample_state().to_ulong(),
                              status_.view_state().to_ulong(), status_.instance_state().to_ulong());
            if(read_condition_.in() == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                            OSPL_CONTEXT_LITERAL("dds::core::NullReferenceError : Unable to create ReadCondition. "
                                                 "Nil return from ::create_readcondition")));
            condition_ = read_condition_.in();
        }
    }

    template<typename T, typename FUN>
    ReadCondition(const dds::sub::DataReader<T>& dr,
                  const dds::sub::status::DataState& status,
                  const FUN& functor,
                  bool query = false) :
        executor_(new ParametrizedExecutor<FUN, dds::sub::DataReader<T> >(
                      functor, dr)), adr_(dr), status_(status)
    {
        if(!query)
        {
            read_condition_ = adr_->get_dds_datareader()->create_readcondition(status_.sample_state().to_ulong(),
                              status_.view_state().to_ulong(), status_.instance_state().to_ulong());
            if(read_condition_.in() == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                            OSPL_CONTEXT_LITERAL("dds::core::NullReferenceError : Unable to create ReadCondition. "
                                                 "Nil return from ::create_readcondition")));
            condition_ = read_condition_.in();
        }
    }

    template<typename FUN>
    ReadCondition(const dds::sub::AnyDataReader& adr,
                  const dds::sub::status::DataState& status,
                  const FUN& functor,
                  bool query = false) :
        executor_(new ParametrizedExecutor<FUN, dds::sub::AnyDataReader >(
                      functor, adr)), adr_(adr), status_(status)
    {
        if(!query)
        {
            read_condition_ = adr_->get_dds_datareader()->create_readcondition(status_.sample_state().to_ulong(),
                              status_.view_state().to_ulong(), status_.instance_state().to_ulong());
            if(read_condition_.in() == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                            OSPL_CONTEXT_LITERAL("dds::core::NullReferenceError : Unable to create ReadCondition. "
                                                 "Nil return from ::create_readcondition")));
            condition_ = read_condition_.in();
        }
    }

    ~ReadCondition()
    {
        if(read_condition_.in())
        {
            DDS::ReturnCode_t result = adr_->get_dds_datareader()->delete_readcondition(read_condition_.in());
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_readcondition"));
        }
        if(executor_)
        {
            delete executor_;
        }
    }

    virtual void dispatch()
    {
        executor_->exec();
    }

    const dds::sub::status::DataState state_filter() const
    {
        return status_;
    }

    const AnyDataReader& data_reader() const
    {
        return adr_;
    }

private:
    dds::sub::cond::detail::Executor* executor_;
    DDS::ReadCondition_var read_condition_;

protected:
    dds::sub::AnyDataReader adr_;
    dds::sub::status::DataState status_;
};

}
}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_COND_DETAIL_READCONDITION_HPP_ */
