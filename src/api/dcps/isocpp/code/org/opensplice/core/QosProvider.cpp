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


#include <dds/core/Exception.hpp>
#include <org/opensplice/domain/qos/QosConverter.hpp>
#include <org/opensplice/topic/qos/QosConverter.hpp>
#include <org/opensplice/pub/qos/QosConverter.hpp>
#include <org/opensplice/sub/qos/QosConverter.hpp>

#include <org/opensplice/core/QosProvider.hpp>
#include <org/opensplice/core/exception_helper.hpp>

/* Includes necessary for pulling in QosProvider API */
#include <cmn_qosProvider.h>

const char* cmn_qpResult_codes[] =
{
    "QP_RESULT_OK",
    "QP_RESULT_NO_DATA",
    "QP_RESULT_OUT_OF_MEMORY",
    "QP_RESULT_PARSE_ERROR",
    "QP_RESULT_ILL_PARAM",
    "QP_RESULT_UNKNOWN_ELEMENT",
    "QP_RESULT_UNEXPECTED_ELEMENT",
    "QP_RESULT_UNKNOWN_ARGUMENT",
    "QP_RESULT_ILLEGAL_VALUE",
    "QP_RESULT_NOT_IMPLEMENTED"
};

const std::size_t cmn_qpResult_codes_len = (sizeof(cmn_qpResult_codes) / sizeof(cmn_qpResult_codes[0]));

std::string dds_return_code_to_string(cmn_qpResult code)
{
    std::string result(code >= 0 && static_cast<std::size_t>(code) < cmn_qpResult_codes_len ? cmn_qpResult_codes[code] : "out of range / unknown code");
    return result;
}

void check_and_throw_impl(cmn_qpResult code,
                          const std::string& context)
{
    if(code)
    {
        std::string message = ". DDS API call returned ";
        message += dds_return_code_to_string(code);
        switch(code)
        {
            case QP_RESULT_OUT_OF_MEMORY:
                throw dds::core::OutOfResourcesError(org::opensplice::core::exception_helper("dds::core::OutOfResourcesError : " + context + message));
            case QP_RESULT_ILL_PARAM:
            case QP_RESULT_NO_DATA:
                throw dds::core::InvalidArgumentError(org::opensplice::core::exception_helper("dds::core::InvalidArgumentError : " + context + message));
            default:
                std::stringstream str_build("dds::core::IllegalOperationError : " + context + message + ". Unknown return value is ");
                str_build << code;
                throw dds::core::IllegalOperationError(org::opensplice::core::exception_helper(str_build.str()));
        }
    }
}

static const C_STRUCT(cmn_qosProviderInputAttr) qosProviderAttr = {
    { /* Participant QoS */
        (cmn_qpCopyOut)&__DDS_NamedDomainParticipantQos__copyOut
    },
    { /* Topic QoS */
        (cmn_qpCopyOut)&__DDS_NamedTopicQos__copyOut
    },
    { /* Subscriber QoS */
        (cmn_qpCopyOut)&__DDS_NamedSubscriberQos__copyOut
    },
    { /* DataReader QoS */
        (cmn_qpCopyOut)&__DDS_NamedDataReaderQos__copyOut
    },
    { /* Publisher QoS */
        (cmn_qpCopyOut)&__DDS_NamedPublisherQos__copyOut
    },
    { /* DataWriter QoS */
        (cmn_qpCopyOut)&__DDS_NamedDataWriterQos__copyOut
    }
};

const C_STRUCT(cmn_qosProviderInputAttr) * org::opensplice::core::QosProvider::getQosProviderInputAttr()
{
    return &qosProviderAttr;
}

//////////////////////////////////////////
org::opensplice::core::QosProvider::QosProvider(const std::string& uri)
{
    if(uri.empty())
        throw dds::core::PreconditionNotMetError(org::opensplice::core::exception_helper(
                            OSPL_CONTEXT_LITERAL("dds::core::PreconditionNotMetError : Invalid Qos Provider URI")));

    this->qosProvider = cmn_qosProviderNew(uri.c_str(), NULL, getQosProviderInputAttr());

    if (this->qosProvider == 0)
        throw dds::core::Error(org::opensplice::core::exception_helper(
                            OSPL_CONTEXT_LITERAL("dds::core::Error : QoSProvider not properly instantiated")));

}

org::opensplice::core::QosProvider::QosProvider(const std::string& uri, const std::string& id)
{
    if(uri.empty())
        throw dds::core::PreconditionNotMetError(org::opensplice::core::exception_helper(
                            OSPL_CONTEXT_LITERAL("dds::core::PreconditionNotMetError : Invalid Qos Provider URI")));

    this->qosProvider = cmn_qosProviderNew(uri.c_str(), id.c_str(), getQosProviderInputAttr());

    if (this->qosProvider == 0)
        throw dds::core::Error(org::opensplice::core::exception_helper(
                            OSPL_CONTEXT_LITERAL("dds::core::Error : QoSProvider not properly instantiated")));
}

/** This might be better off in memory.cpp? **/
org::opensplice::core::QosProvider::~QosProvider()
{
    cmn_qosProviderFree(this->qosProvider);
}

dds::domain::qos::DomainParticipantQos
org::opensplice::core::QosProvider::participant_qos()
{
    std::string id;
    return this->participant_qos(id);
}

dds::domain::qos::DomainParticipantQos
org::opensplice::core::QosProvider::participant_qos(const std::string& id)
{
    DDS::NamedDomainParticipantQos ndpq;
    org::opensplice::core::check_and_throw(
        cmn_qosProviderGetParticipantQos(this->qosProvider, id.empty() ? NULL : id.c_str(), &ndpq)
        );
    return org::opensplice::domain::qos::convertQos(ndpq.domainparticipant_qos);
}

dds::topic::qos::TopicQos
org::opensplice::core::QosProvider::topic_qos()
{
    std::string id;
    return this->topic_qos(id);
}

dds::topic::qos::TopicQos
org::opensplice::core::QosProvider::topic_qos(const std::string& id)
{
    DDS::NamedTopicQos ntq;
    org::opensplice::core::check_and_throw(
        cmn_qosProviderGetTopicQos(this->qosProvider, id.empty() ? NULL : id.c_str(), &ntq)
        );
    return org::opensplice::topic::qos::convertQos(ntq.topic_qos);
}

dds::sub::qos::SubscriberQos
org::opensplice::core::QosProvider::subscriber_qos()
{
    std::string id;
    return this->subscriber_qos(id);
}

dds::sub::qos::SubscriberQos
org::opensplice::core::QosProvider::subscriber_qos(const std::string& id)
{
    DDS::NamedSubscriberQos nsq;
    org::opensplice::core::check_and_throw(
        cmn_qosProviderGetSubscriberQos(this->qosProvider, id.empty() ? NULL : id.c_str(), &nsq)
        );
    return org::opensplice::sub::qos::convertQos(nsq.subscriber_qos);
}

dds::sub::qos::DataReaderQos
org::opensplice::core::QosProvider::datareader_qos()
{
    std::string id;
    return this->datareader_qos(id);
}

dds::sub::qos::DataReaderQos
org::opensplice::core::QosProvider::datareader_qos(const std::string& id)
{
    DDS::NamedDataReaderQos ndrq;
    org::opensplice::core::check_and_throw(
        cmn_qosProviderGetDataReaderQos(this->qosProvider, id.empty() ? NULL : id.c_str(), &ndrq)
        );
    return org::opensplice::sub::qos::convertQos(ndrq.datareader_qos);
}


dds::pub::qos::PublisherQos
org::opensplice::core::QosProvider::publisher_qos()
{
    std::string id;
    return this->publisher_qos(id);
}

dds::pub::qos::PublisherQos
org::opensplice::core::QosProvider::publisher_qos(const std::string& id)
{
    DDS::NamedPublisherQos npq;
    org::opensplice::core::check_and_throw(
        cmn_qosProviderGetPublisherQos(this->qosProvider, id.empty() ? NULL : id.c_str(), &npq)
        );
    return org::opensplice::pub::qos::convertQos(npq.publisher_qos);
}

dds::pub::qos::DataWriterQos
org::opensplice::core::QosProvider::datawriter_qos()
{
    std::string id;
    return this->datawriter_qos(id);
}

dds::pub::qos::DataWriterQos
org::opensplice::core::QosProvider::datawriter_qos(const std::string& id)
{
    DDS::NamedDataWriterQos ndwq;
    org::opensplice::core::check_and_throw(
        cmn_qosProviderGetDataWriterQos(this->qosProvider, id.empty() ? NULL : id.c_str(), &ndwq)
        );
    return org::opensplice::pub::qos::convertQos(ndwq.datawriter_qos);
}
