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
#include "ContentFilteredTopic.h"
#include "DomainParticipant.h"
#include "TopicDescription.h"
#include "TypeSupportMetaHolder.h"
#include "ReportUtils.h"

DDS::OpenSplice::ContentFilteredTopic::ContentFilteredTopic() :
    DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::CONTENTFILTEREDTOPIC),
    relatedTopic(NULL)
{
}

DDS::ReturnCode_t
DDS::OpenSplice::ContentFilteredTopic::init(
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *topic_name,
    DDS::OpenSplice::Topic *related_topic,
    const DDS::Char *filter_expression,
    const DDS::StringSeq &filter_parameters)
{
    return this->nlReq_init(participant, topic_name, related_topic, filter_expression, filter_parameters);
}

DDS::ReturnCode_t
DDS::OpenSplice::ContentFilteredTopic::nlReq_init(
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *topic_name,
    DDS::OpenSplice::Topic *related_topic,
    const DDS::Char *filter_expression,
    const DDS::StringSeq &filter_parameters)
{
    DDS::ReturnCode_t result;
    DDS::OpenSplice::TypeSupportMetaHolder *type_holder = NULL;
    DDS::Char *expression = NULL;
    const char *related_topic_name = NULL;
    const char *related_type_name = NULL;
    DDS::ULong length;
    const DDS::Char *format = "select * from %s where %s";

    assert(participant);
    assert(topic_name);
    assert(related_topic);
    assert(filter_expression);

    result = DDS::OpenSplice::CppSuperClass::nlReq_init();
    if (result == DDS::RETCODE_OK) {
        result = related_topic->write_lock();
        if (result == DDS::RETCODE_OK) {
            type_holder        = related_topic->tsMetaHolder;
            related_type_name  = related_topic->type_name.in();
            related_topic_name = related_topic->topic_name.in();
            result = related_topic->validate_filter(filter_expression, filter_parameters);
            if (result == DDS::RETCODE_OK) {
                length = strlen(format) + strlen(related_topic_name) + strlen(filter_expression) + 1;
                expression = DDS::string_alloc(length);
                if (expression) {
                    snprintf(expression, length, format, related_topic_name, filter_expression);
                    result = DDS::OpenSplice::TopicDescription::nlReq_init(participant,
                                                                      topic_name,
                                                                      related_type_name,
                                                                      expression,
                                                                      type_holder);
                    DDS::string_free(expression);
                    if (result == DDS::RETCODE_OK) {
                        (void) DDS::Topic::_duplicate(related_topic);
                        relatedTopic = related_topic;
                        filterExpression = DDS::string_dup(filter_expression);
                        filterParameters = filter_parameters;
                        related_topic->wlReq_incrNrUsers();
                        setDomainId(relatedTopic->getDomainId());
                    }
                } else {
                    result = DDS::RETCODE_OUT_OF_RESOURCES;
                    CPP_REPORT(result, "Could not allocate memory.");
                }
            }
            related_topic->unlock();
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ContentFilteredTopic::wlReq_deinit()
{
    DDS::ReturnCode_t result = DDS::RETCODE_PRECONDITION_NOT_MET;

    if (this->rlReq_getNrUsers() == 0) {
        result = this->relatedTopic->write_lock();
        if (result == DDS::RETCODE_OK) {
            this->relatedTopic->wlReq_decrNrUsers();
            this->relatedTopic->unlock();
            DDS::release(this->relatedTopic);
            this->relatedTopic = NULL;

            result = DDS::OpenSplice::TopicDescription::wlReq_deinit();
            if (result == DDS::RETCODE_OK) {
                result = DDS::OpenSplice::CppSuperClass::wlReq_deinit();
            }
        }
    } else {
        CPP_REPORT(result, "ContentFilteredTopic still in use.");
    }
    return result;
}

DDS::OpenSplice::ContentFilteredTopic::~ContentFilteredTopic()
{
}

char *
DDS::OpenSplice::ContentFilteredTopic::get_filter_expression (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    char *expression = NULL;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        expression = DDS::string_dup(filterExpression);
        if (expression == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not copy filter expression.");
        }
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return expression;
}

DDS::ReturnCode_t
DDS::OpenSplice::ContentFilteredTopic::get_expression_parameters (
    ::DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        expression_parameters = filterParameters;
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ContentFilteredTopic::set_expression_parameters (
    const ::DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS
{
    // We currently have no user layer object to forward the request to.
    OS_UNUSED_ARG(expression_parameters);
    DDS::ReturnCode_t result = DDS::RETCODE_UNSUPPORTED;

    CPP_REPORT_STACK();

    CPP_REPORT(result, "Operation not yet supported.");

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::Topic_ptr
DDS::OpenSplice::ContentFilteredTopic::get_related_topic (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Topic_ptr related_topic = NULL;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        related_topic = DDS::Topic::_duplicate(relatedTopic);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return related_topic;
}
