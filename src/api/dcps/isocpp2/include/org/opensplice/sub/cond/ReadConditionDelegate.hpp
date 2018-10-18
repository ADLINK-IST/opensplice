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

#ifndef ORG_OPENSPLICE_SUB_COND_READCONDITION_DELEGATE_HPP_
#define ORG_OPENSPLICE_SUB_COND_READCONDITION_DELEGATE_HPP_

#include <dds/sub/AnyDataReader.hpp>

#include <org/opensplice/core/cond/ConditionDelegate.hpp>
#include <org/opensplice/sub/QueryDelegate.hpp>



namespace org
{
namespace opensplice
{
namespace sub
{
namespace cond
{


class OMG_DDS_API ReadConditionDelegate :
    public virtual org::opensplice::core::cond::ConditionDelegate,
    public virtual org::opensplice::sub::QueryDelegate
{
public:
    ReadConditionDelegate(
            const dds::sub::AnyDataReader& dr,
            const dds::sub::status::DataState& state_filter);

    template<typename FUN>
    ReadConditionDelegate(
            const dds::sub::AnyDataReader& dr,
            const dds::sub::status::DataState& state_filter,
            const FUN& functor)
                :  QueryDelegate(dr, state_filter)
     {
        this->set_handler<FUN>(functor);
     }

    ~ReadConditionDelegate();

    void init(ObjectDelegate::weak_ref_type weak_ref);

    void close();

    virtual bool trigger_value() const;

    u_observable get_user_condition();

    bool modify_state_filter(dds::sub::status::DataState& s);

private:
    static unsigned char always_true(void *object, void *arg);

};

}
}
}
}

#endif /* ORG_OPENSPLICE_SUB_COND_READCONDITION_DELEGATE_HPP_ */
