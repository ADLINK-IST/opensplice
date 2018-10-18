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
#ifndef CPP_DDS_OPENSPLICE_CONTENTFILTEREDTOPIC_H
#define CPP_DDS_OPENSPLICE_CONTENTFILTEREDTOPIC_H

#include "CppSuperClass.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "cpp_dcps_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    namespace OpenSplice
    {
        /* TODO: This is just a mock class; implement properly. */
        class OS_API ContentFilteredTopic
            : public virtual ::DDS::ContentFilteredTopic,
              public ::DDS::OpenSplice::CppSuperClass,
              public ::DDS::OpenSplice::TopicDescription

        {
            friend class ::DDS::OpenSplice::DomainParticipant;
            friend class ::DDS::OpenSplice::DataReader;

        protected:
            DDS::OpenSplice::Topic *relatedTopic;
            DDS::String_var filterExpression;
            DDS::StringSeq filterParameters;

            ContentFilteredTopic();

            virtual ~ContentFilteredTopic();

            DDS::ReturnCode_t
            virtual init(
                    DDS::OpenSplice::DomainParticipant *participant,
                    const DDS::Char *topic_name,
                    DDS::OpenSplice::Topic *related_topic,
                    const DDS::Char *filter_expression,
                    const DDS::StringSeq &filter_parameters);

            DDS::ReturnCode_t nlReq_init(
                    DDS::OpenSplice::DomainParticipant *participant,
                    const DDS::Char *topic_name,
                    DDS::OpenSplice::Topic *related_topic,
                    const DDS::Char *filter_expression,
                    const DDS::StringSeq &filter_parameters);

            virtual DDS::ReturnCode_t wlReq_deinit();

        public:
            virtual char *
            get_filter_expression (
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t
            get_expression_parameters (
              ::DDS::StringSeq & expression_parameters
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t
            set_expression_parameters (
              const ::DDS::StringSeq & expression_parameters
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::Topic_ptr
            get_related_topic (
            ) THROW_ORB_EXCEPTIONS;

        }; /* class ContentFilteredTopic */
    }; /* namespace OpenSplice */
}; /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_CONTENTFILTEREDTOPIC_H */
