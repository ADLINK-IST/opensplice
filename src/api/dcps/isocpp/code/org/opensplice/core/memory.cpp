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

#include <org/opensplice/core/memory.hpp>
#include <org/opensplice/core/exception_helper.hpp>
#include <sstream>
#include <algorithm>

org::opensplice::core::DPDeleter::DPDeleter() : is_closed_(false) { }

void org::opensplice::core::DPDeleter::operator()(DDS::DomainParticipant* dp)
{
    if(!is_closed_)
    {
        DDS::DomainParticipantFactory_var dpf = DDS::DomainParticipantFactory::get_instance();

        org::opensplice::core::check_and_throw(dp->delete_contained_entities(), OSPL_CONTEXT_LITERAL("Calling ::delete_contained_entities"));

        DDS::ReturnCode_t result = dpf->delete_participant(dp);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_participant"));

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted Participant at: " << std::hex << dp << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
    DDS::release(dp);
}

void org::opensplice::core::DPDeleter::close(DDS::DomainParticipant* dp)
{
    if(!is_closed_)
    {
        DDS::DomainParticipantFactory_var dpf = DDS::DomainParticipantFactory::get_instance();

        org::opensplice::core::check_and_throw(dp->delete_contained_entities(), OSPL_CONTEXT_LITERAL("Calling ::delete_contained_entities"));

        DDS::ReturnCode_t result = dpf->delete_participant(dp);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_participant"));
        is_closed_ = true;

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted Participant at: " << std::hex << dp << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
}

org::opensplice::core::PubDeleter::PubDeleter(const org::opensplice::core::DDS_DP_REF& dp) :
    dp_(dp), is_closed_(false) { }

org::opensplice::core::PubDeleter::~PubDeleter()  { }

void org::opensplice::core::PubDeleter::operator()(DDS::Publisher* p)
{
    if(!is_closed_)
    {
        DDS::ReturnCode_t result = dp_->delete_publisher(p);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_publisher"));

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted Publisher at: " << std::hex << p << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
    DDS::release(p);
}

void org::opensplice::core::PubDeleter::close(DDS::Publisher* p)
{
    if(!is_closed_)
    {
        DDS::ReturnCode_t result = dp_->delete_publisher(p);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_publisher"));
        is_closed_ = true;

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted Publisher at: " << std::hex << p << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
}

org::opensplice::core::DWDeleter::DWDeleter(const DDS_PUB_REF& pub) :
    pub_(pub), is_closed_(false) { }

org::opensplice::core::DWDeleter::~DWDeleter() { }

void org::opensplice::core::DWDeleter::operator()(DDS::DataWriter* w)
{
    if(!is_closed_)
    {
        DDS::ReturnCode_t result = pub_->delete_datawriter(w);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_datawriter"));

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted DataWriter at: " << std::hex << w << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
    DDS::release(w);
}

void org::opensplice::core::DWDeleter::close(DDS::DataWriter* w)
{
    if(!is_closed_)
    {
        DDS::ReturnCode_t result = pub_->delete_datawriter(w);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_datawriter"));
        is_closed_ = true;

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted DataWriter at: " << std::hex << w << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
}


org::opensplice::core::SubDeleter::SubDeleter(const DDS_DP_REF& dp) :
    dp_(dp), is_closed_(false), is_builtin_(false) { }

org::opensplice::core::SubDeleter::~SubDeleter() { }

void org::opensplice::core::SubDeleter::operator()(DDS::Subscriber* s)
{
    if(!is_builtin_)
    {
        if(!is_closed_)
        {
            DDS::ReturnCode_t result = dp_->delete_subscriber(s);
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_subscriber"));

            if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
            {
                std::ostringstream oss;
                oss << "Deleted Subscriber at: " << std::hex << s << std::dec;
                OMG_DDS_LOG("MM", oss.str().c_str());
            }
        }
        DDS::release(s);
    }
}

void org::opensplice::core::SubDeleter::close(DDS::Subscriber* s)
{
    if(!is_closed_ && !is_builtin_)
    {
        DDS::ReturnCode_t result = dp_->delete_subscriber(s);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_subscriber"));
        is_closed_ = true;

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted Subscriber at: " << std::hex << s << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
    if(is_builtin_)
    {
        throw 0;
    }
}

org::opensplice::core::DRDeleter::DRDeleter(const DDS_SUB_REF& sub) :
    sub_(sub), is_closed_(false), is_builtin_(false) { }

org::opensplice::core::DRDeleter::~DRDeleter() { }

void org::opensplice::core::DRDeleter::operator()(DDS::DataReader* r)
{
    if(!is_builtin_)
    {
        if(!is_closed_)
        {
            DDS::ReturnCode_t result = sub_->delete_datareader(r);
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_datareader"));

            if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
            {
                std::ostringstream oss;
                oss << "Deleted DataReader at: " << std::hex << r << std::dec;
                OMG_DDS_LOG("MM", oss.str().c_str());
            }
        }
        DDS::release(r);
    }
}

void org::opensplice::core::DRDeleter::close(DDS::DataReader* r)
{
    if(!is_closed_ && !is_builtin_)
    {
        DDS::ReturnCode_t result = sub_->delete_datareader(r);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_datareader"));
        is_closed_ = true;

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted DataReader at: " << std::hex << r << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
    if(is_builtin_)
    {
        throw 0;
    }
}

org::opensplice::core::TopicDeleter::TopicDeleter(const DDS_DP_REF& dp) :
    dp_(dp), is_closed_(false) { }

org::opensplice::core::TopicDeleter::~TopicDeleter() { }

void org::opensplice::core::TopicDeleter::operator()(DDS::Topic* t)
{
    if(!is_closed_)
    {
        DDS::ReturnCode_t result = dp_->delete_topic(t);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_topic"));

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted Topic at: " << std::hex << t << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
    DDS::release(t);
}

void org::opensplice::core::TopicDeleter::close(DDS::Topic* t)
{
    if(!is_closed_)
    {
        DDS::ReturnCode_t result = dp_->delete_topic(t);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_topic"));
        is_closed_ = true;

        if(dds::core::detail::maplog("MM") >= os_reportVerbosity)
        {
            std::ostringstream oss;
            oss << "Deleted Topic at: " << std::hex << t << std::dec;
            OMG_DDS_LOG("MM", oss.str().c_str());
        }
    }
}
