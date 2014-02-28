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
#ifndef OSPL_DDS_SUB_DETAIL_QUERY_HPP_
#define OSPL_DDS_SUB_DETAIL_QUERY_HPP_

/**
 * @file
 */

// Implementation

#include <vector>
#include <iterator>

#include <dds/sub/DataReader.hpp>
#include <dds/sub/AnyDataReader.hpp>

namespace dds
{
namespace sub
{
namespace detail
{
class FooQuery;
}
}
}

class OSPL_ISOCPP_IMPL_API dds::sub::detail::FooQuery
{
public:
    typedef std::vector<std::string>::iterator iterator;
    typedef std::vector<std::string>::const_iterator const_iterator;

public:

    template<typename T>
    FooQuery(const dds::sub::DataReader<T>& dr, const std::string& query_expression) :
        adr_(dr),
        expression_(query_expression)
    {}

    template<typename T, typename FWIterator>
    FooQuery(const dds::sub::DataReader<T>& dr, const std::string& query_expression,
             const FWIterator& params_begin, const FWIterator& params_end)
        : adr_(dr),
          expression_(query_expression)
    {
        std::copy(params_begin, params_end,
                  std::back_insert_iterator<std::vector<std::string> >(params_));
    }

    const std::string& expression() const
    {
        return expression_;
    }

    void expression(const std::string& expr)
    {
        expression_ = expr;
    }

    /**
     *  @internal Provides the begin iterator to the parameter list.
     */
    iterator begin()
    {
        return params_.begin();
    }

    /**
     *  @internal The end iterator to the parameter list.
     */
    iterator end()
    {
        return params_.end();
    }

    const_iterator begin() const
    {
        return params_.begin();
    }

    /**
     *  @internal The end iterator to the parameter list.
     */
    const_iterator end() const
    {
        return params_.end();
    }

    template<typename FWIterator>
    void parameters(const FWIterator& begin, const FWIterator end)
    {
        params_.erase(params_.begin(), params_.end());
        std::copy(begin, end,
                  std::back_insert_iterator<std::vector<std::string> >(params_));
    }

    void add_parameter(const std::string& param)
    {
        params_.push_back(param);
    }

    uint32_t parameters_length() const
    {
        return static_cast<uint32_t>(params_.size());
    }

    const dds::sub::AnyDataReader& data_reader() const
    {
        return adr_;
    }

private:
    dds::sub::AnyDataReader adr_;
    std::string expression_;
    std::vector<std::string> params_;
};

namespace dds
{
namespace sub
{
namespace detail
{
typedef dds::sub::TQuery<dds::sub::detail::FooQuery> Query;
}
}
}
// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_QUERY_HPP_ */
