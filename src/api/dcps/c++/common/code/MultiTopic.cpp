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
#include "u_user.h"
#include "DomainParticipant.h"
#include "TopicDescription.h"
#include "MultiTopic.h"


DDS::OpenSplice::MultiTopic::MultiTopic() :
    DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::MULTITOPIC)
{
}

DDS::ReturnCode_t
DDS::OpenSplice::MultiTopic::init(
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *topic_name,
    const DDS::Char *type_name,
    const DDS::Char *subscription_expression,
    const DDS::StringSeq &expression_parameters)
{
    return nlReq_init(participant,
                         topic_name,
                         type_name,
                         subscription_expression,
                         expression_parameters);
}

DDS::ReturnCode_t
DDS::OpenSplice::MultiTopic::nlReq_init(
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *topic_name,
    const DDS::Char *type_name,
    const DDS::Char *subscription_expression,
    const DDS::StringSeq &expression_parameters)
{
    ReturnCode_t result;
    OS_UNUSED_ARG(expression_parameters);
    result = DDS::OpenSplice::CppSuperClass::nlReq_init();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::TopicDescription::nlReq_init(participant,
                                                        topic_name,
                                                        type_name,
                                                        subscription_expression,
                                                        NULL);
        setDomainId(participant->getDomainId());
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::MultiTopic::wlReq_deinit()
{
    DDS::ReturnCode_t result;

    result = DDS::OpenSplice::TopicDescription::wlReq_deinit();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::CppSuperClass::wlReq_deinit();
    }
    return result;
}

DDS::OpenSplice::MultiTopic::~MultiTopic()
{
}

char *
DDS::OpenSplice::MultiTopic::get_subscription_expression (
) THROW_ORB_EXCEPTIONS
{
    return (char*)NULL;
}

DDS::ReturnCode_t
DDS::OpenSplice::MultiTopic::get_expression_parameters (
    ::DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(expression_parameters);
    return DDS::RETCODE_UNSUPPORTED;
}

DDS::ReturnCode_t
DDS::OpenSplice::MultiTopic::set_expression_parameters (
    const ::DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(expression_parameters);
    return DDS::RETCODE_UNSUPPORTED;
}
