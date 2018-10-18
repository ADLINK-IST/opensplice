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
#ifndef OSPL_ORG_OPENSPICE_SUB_QUERY_DELEGATE_HPP_
#define OSPL_ORG_OPENSPICE_SUB_QUERY_DELEGATE_HPP_

/**
 * @file
 */

#include <dds/core/macros.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/AnyDataReader.hpp>

#include <org/opensplice/core/UserObjectDelegate.hpp>
#include <org/opensplice/core/Mutex.hpp>


#include <vector>
#include <iterator>




namespace org
{
namespace opensplice
{
namespace sub
{


class OMG_DDS_API QueryDelegate : public virtual org::opensplice::core::UserObjectDelegate
{
public:
    typedef std::vector<std::string>::iterator iterator;
    typedef std::vector<std::string>::const_iterator const_iterator;
    typedef ::dds::core::smart_ptr_traits<QueryDelegate>::ref_type Ref;
    typedef ::dds::core::smart_ptr_traits<QueryDelegate>::weak_ref_type WeakRef;

public:
    QueryDelegate(const dds::sub::AnyDataReader& dr,
                  const dds::sub::status::DataState& state_filter = dds::sub::status::DataState::any());

    QueryDelegate(const dds::sub::AnyDataReader& dr,
                  const std::string& query_expression,
                  const dds::sub::status::DataState& state_filter = dds::sub::status::DataState::any());

    QueryDelegate(const dds::sub::AnyDataReader& dr,
                  const std::string& query_expression,
                  const std::vector<std::string>& params,
                  const dds::sub::status::DataState& state_filter = dds::sub::status::DataState::any());

    virtual ~QueryDelegate();

    void init(ObjectDelegate::weak_ref_type weak_ref);

    void close();

    const std::string& expression() const;

    void expression(const std::string& expr);

    iterator begin();

    iterator end();

    const_iterator begin() const;

    const_iterator end() const;

    void add_parameter(const std::string& param);

    uint32_t parameters_length() const;

    void parameters(const std::vector<std::string>& params);

    std::vector<std::string> parameters();

    void clear_parameters();

    const dds::sub::AnyDataReader& data_reader() const;

    virtual void state_filter(dds::sub::status::DataState& s);

    virtual dds::sub::status::DataState state_filter();

    u_query get_user_query();

    u_observable get_user_condition();

    virtual bool modify_state_filter(dds::sub::status::DataState& s);

    bool state_filter_equal(dds::sub::status::DataState& s);

protected:
    void deinit();

private:
    u_query create_user_query();
    u_query set_query_parameters();

private:
    dds::sub::AnyDataReader reader_;
    std::string expression_;
    std::vector<std::string> params_;
    dds::sub::status::DataState state_filter_;
    bool modified_;
};


}
}
}


#endif /* OSPL_ORG_OPENSPICE_SUB_QUERY_DELEGATE_HPP_ */
