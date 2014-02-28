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
#ifndef OSPL_DDS_SUB_COND_DETAIL_QUERYCONDITION_HPP_
#define OSPL_DDS_SUB_COND_DETAIL_QUERYCONDITION_HPP_

/**
 * @file
 */

// Implementation

#include <dds/sub/cond/detail/ReadCondition.hpp>
#include <dds/sub/Query.hpp>
#include <org/opensplice/core/exception_helper.hpp>

#include <iostream>

#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

namespace dds
{
namespace sub
{
namespace cond
{
namespace detail
{
class QueryCondition;
}
}
}
}

/**
 *  @internal QueryCondition inherits from ReadCondition and additionally provides the ability
 * to store a query made up of a query expression and parameters. As with ReadCondition,
 * a handler functor can be passed in at construction time which is then executed
 * by WaitSetImpl which calls it's dispatch member function when the condition is triggered.
 */
class dds::sub::cond::detail::QueryCondition: public dds::sub::cond::detail::ReadCondition
{
public:
    typedef std::vector<std::string>::iterator iterator;
    typedef std::vector<std::string>::const_iterator const_iterator;
public:
    QueryCondition() : ReadCondition(), query_(dds::core::null) { }

    QueryCondition(
        const dds::sub::Query& query,
        const dds::sub::status::DataState& data_state)
        : ReadCondition(query.data_reader(), data_state, true),
          query_(query)
    {
        DDS::StringSeq params;
        params.length(static_cast<DDS::ULong>(query.parameters_length() + 1));
        for(unsigned int i = 0; i < query.parameters_length(); i++)
        {
            params[i] = (query.begin() + i)->c_str();
        }
        params[query.parameters_length()] = DDS::string_dup("");
        query_condition_ = adr_->get_dds_datareader()->create_querycondition(status_.sample_state().to_ulong(),
                           status_.view_state().to_ulong(), status_.instance_state().to_ulong(), query.expression().c_str(), params);

        if(query_condition_.in() == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                        OSPL_CONTEXT_LITERAL("dds::core::NullReferenceError : Unable to create QueryCondition. "
                                             "Nil return from ::create_querycondition")));
        condition_ = query_condition_.in();
    }

    template <typename FUN>
    QueryCondition(const dds::sub::Query& query,
                   const dds::sub::status::DataState& data_state, const FUN& functor)
        : ReadCondition(query.data_reader(), data_state, functor, true),
          query_(query)
    {
        DDS::StringSeq params;
        params.length(static_cast<DDS::ULong>(query.parameters_length()));
        for(unsigned int i = 0; i < query.parameters_length(); i++)
        {
            params[i] = (query.begin() + i)->c_str();
        }
        query_condition_ = adr_->get_dds_datareader()->create_querycondition(status_.sample_state().to_ulong(),
                           status_.view_state().to_ulong(), status_.instance_state().to_ulong(), query.expression().c_str(), params);
        if(query_condition_.in() == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                        OSPL_CONTEXT_LITERAL("dds::core::NullReferenceError : Unable to create QueryCondition. "
                                             "Nil return from ::create_querycondition")));
        condition_ = query_condition_.in();
    }

    virtual ~QueryCondition()
    {
        if(query_condition_)
        {
            DDS::ReturnCode_t result = adr_->get_dds_datareader()->delete_readcondition(query_condition_.in());
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_readcondition"));
        }
    }

    void expression(const std::string& expr)
    {
        query_.expression(expr);
    }

    const std::string& expression()
    {
        return query_.expression();
    }


    /**
     *  @internal Provides the begin iterator to the parameter list.
     */
    iterator begin()
    {
        return query_.begin();
    }

    /**
     *  @internal The end iterator to the parameter list.
     */
    iterator end()
    {
        return query_.end();
    }

    const_iterator begin() const
    {
        return query_.begin();
    }

    /**
     *  @internal The end iterator to the parameter list.
     */
    const_iterator end() const
    {
        return query_.end();
    }

    template<typename FWIterator>
    void parameters(const FWIterator& begin, const FWIterator end)
    {
        query_.parameters(begin, end);
    }

    void add_parameter(const std::string& param)
    {
        query_.add_parameter(param);
    }

    uint32_t parameters_length() const
    {
        return query_.parameters_length();
    }

    const AnyDataReader& data_reader()
    {
        return query_.data_reader();
    }

private:
    DDS::QueryCondition_var query_condition_;
    dds::sub::Query query_;
};

#endif  // OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

// End of implementation

#endif /* OSPL_DDS_SUB_COND_DETAIL_QUERYCONDITION_HPP_ */
