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
#include <dds/sub/DataReader.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/topic/TopicInstance.hpp>
#include <dds/topic/Topic.hpp>

#include <org/opensplice/sub/QueryDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

org::opensplice::sub::QueryDelegate::QueryDelegate(
    const dds::sub::AnyDataReader& dr,
    const dds::sub::status::DataState& state_filter) :
        reader_(dr), expression_("1=1"),
        state_filter_(state_filter), modified_(true)
{

}

org::opensplice::sub::QueryDelegate::QueryDelegate(
    const dds::sub::AnyDataReader& dr,
    const std::string& expression,
    const dds::sub::status::DataState& state_filter) :
        reader_(dr), expression_(expression),
        state_filter_(state_filter), modified_(true)
{

}

org::opensplice::sub::QueryDelegate::QueryDelegate(
    const dds::sub::AnyDataReader& dr,
    const std::string& expression,
    const std::vector<std::string>& params,
    const dds::sub::status::DataState& state_filter) :
         reader_(dr), expression_(expression),
         params_(params), state_filter_(state_filter), modified_(true)
{

}

org::opensplice::sub::QueryDelegate::~QueryDelegate()
{
    if (!this->closed) {
        try {
            this->close();
        } catch (...) {
            /* Empty: the exception throw should have already traced an error. */
        }
    }
}

void
org::opensplice::sub::QueryDelegate::init(
    ObjectDelegate::weak_ref_type weak_ref)
{
    this->set_weak_ref(weak_ref);
    (this->reader_)->add_query(*this);
}

/* The QueryContainer has a close and a deinit method.
 * When the QueryContainer is created as result of the
 * use of a Query with an DataReader::Selector then the
 * QueryContainer will be responsible for closing the
 * corresponding u_query handle which will be handled by
 * the close method.
 * The ReadConditionDelegate inherits both
 * from QueryContainer and from ConditionDelegate.
 * When the QueryContainer is created as result of being
 * the parent of a ReadConditionDelegate then the close of the
 * ReadConditionDelegate will call the deinit method of QueryContainer
 * which will remove the QueryContainer from the associated
 * DataReaderDelegate. Then the close of the ReadConditionDelegate
 * will call close on the ConditionDelegate to close the corresponding
 * u_query handle.
 */
void
org::opensplice::sub::QueryDelegate::deinit()
{
    (this->reader_)->remove_query(*this);
}

void
org::opensplice::sub::QueryDelegate::close()
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    deinit();

    UserObjectDelegate::close();
}

const std::string&
org::opensplice::sub::QueryDelegate::expression() const
{
    return expression_;
}

void
org::opensplice::sub::QueryDelegate::expression(
    const std::string& expr)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    if (this->expression_ != expr) {
        this->expression_ = expr;
        this->modified_ = true;
    }
}

org::opensplice::sub::QueryDelegate::iterator
org::opensplice::sub::QueryDelegate::begin()
{
    return params_.begin();
}

org::opensplice::sub::QueryDelegate::iterator
org::opensplice::sub::QueryDelegate::end()
{
    return params_.end();
}

org::opensplice::sub::QueryDelegate::const_iterator
org::opensplice::sub::QueryDelegate::begin() const
{
    return params_.begin();
}

org::opensplice::sub::QueryDelegate::const_iterator
org::opensplice::sub::QueryDelegate::end() const
{
    return params_.end();
}

void
org::opensplice::sub::QueryDelegate::add_parameter(
    const std::string& param)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    params_.push_back(param);
}

uint32_t
org::opensplice::sub::QueryDelegate::parameters_length() const
{
    this->lock();
    uint32_t len =  static_cast<uint32_t>(params_.size());
    this->unlock();

    return len;
}

void
org::opensplice::sub::QueryDelegate::parameters(const std::vector<std::string>& params)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    if (this->params_ != params) {
        this->params_ = params;
        if (this->userHandle != NULL) {
            set_query_parameters();
        }
    }
}

std::vector<std::string>
org::opensplice::sub::QueryDelegate::parameters()
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    return this->params_;
}


void
org::opensplice::sub::QueryDelegate::clear_parameters()
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    if (!this->params_.empty()) {
        this->params_.erase(this->params_.begin(), this->params_.end());
    }
}


const dds::sub::AnyDataReader&
org::opensplice::sub::QueryDelegate::data_reader() const
{
    return this->reader_;
}

void
org::opensplice::sub::QueryDelegate::state_filter(
    dds::sub::status::DataState& s)
{
    this->lock();
    if (this->state_filter_ != s) {
#if 0
    if ((s.sample_state()   != this->state_filter_.sample_state()) ||
        (s.view_state()     != this->state_filter_.view_state())   ||
        (s.instance_state() != this->state_filter_.instance_state())) {
#endif
        this->state_filter_ = s;
        this->modified_ = true;
    }
    this->unlock();
}

dds::sub::status::DataState
org::opensplice::sub::QueryDelegate::state_filter()
{
    this->lock();
    dds::sub::status::DataState filter = this->state_filter_;
    this->unlock();

    return filter;
}

bool
org::opensplice::sub::QueryDelegate::state_filter_equal(
    dds::sub::status::DataState& s)
{
    bool equal = false;
    this->lock();
#if 0
    if ((s.sample_state()   == this->state_filter_.sample_state()) &&
        (s.view_state()     == this->state_filter_.view_state())   &&
        (s.instance_state() == this->state_filter_.instance_state())) {
        equal = true;
    }
#endif
    equal = this->state_filter_ == s;
    this->unlock();

    return equal;
}

bool
org::opensplice::sub::QueryDelegate::modify_state_filter(
    dds::sub::status::DataState& s)
{
    this->state_filter(s);
    return true;
}

u_observable
org::opensplice::sub::QueryDelegate::get_user_condition()
{
    return u_observable(get_user_query());
}

u_query
org::opensplice::sub::QueryDelegate::get_user_query()
{
    u_query uQuery = NULL;

    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    if (this->modified_) {
        uQuery = this->create_user_query();
    } else {
        uQuery = u_query(this->userHandle);
    }

    return uQuery;
}

u_query
org::opensplice::sub::QueryDelegate::create_user_query()
{
    u_query uQuery;

    u_sampleMask uMask = org::opensplice::sub::AnyDataReaderDelegate::getUserMask(this->state_filter_);

    u_reader uReader = u_reader(this->reader_->get_user_handle());

    if (!this->params_.empty()) {
        const char **plist = new const char *[this->params_.size()];

        int i = 0;
        std::vector<std::string>::const_iterator it;
        for (it = this->params_.begin(); it != this->params_.end(); ++it, ++i) {
            plist[i] = (*it).c_str();
        }
        uQuery = u_queryNew(uReader, NULL, this->expression_.c_str(), plist, this->params_.size(), uMask);
        delete[] plist;
    } else {
        uQuery = u_queryNew(uReader, NULL, this->expression_.c_str(), NULL, 0, uMask);
    }

    if (uQuery == NULL) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "u_queryNew failed.");
    }

    if (this->userHandle) {
        u_objectClose(this->userHandle);
        u_objectFree(this->userHandle);
    }

    this->userHandle = u_object(uQuery);

    this->modified_ = false;

    return uQuery;
}

u_query
org::opensplice::sub::QueryDelegate::set_query_parameters()
{
    u_query uQuery;

    if (!this->params_.empty()) {
        const char **plist = new const char *[this->params_.size()];

        int i = 0;
        std::vector<std::string>::const_iterator it;
        for (it = this->params_.begin(); it != this->params_.end(); ++it, ++i) {
            plist[i] = (*it).c_str();
        }
        u_result uResult = u_querySet(u_query(this->userHandle), plist, this->params_.size());
        delete[] plist;
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_querySet failed.");
    }

    uQuery = u_query(this->userHandle);

    return uQuery;
}
