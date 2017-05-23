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

#include <org/opensplice/core/QosProviderDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
#include <cmn_qosProvider.h>
#include <u_types.h>


namespace org
{
namespace opensplice
{
namespace core
{
namespace helper
{

/**************************************************************
 * Helper data and functions to parse cmn QosProvider results *
 * and possibly throw proper exceptions.                      *
 **************************************************************/
typedef struct {
    u_result code;
    const char* c_str;
} qpr_translation;

const qpr_translation qpr_translations[] =
{
    { V_RESULT_OK,                   "QP_RESULT_OK"                   },
    { V_RESULT_PRECONDITION_NOT_MET, "QP_RESULT_NO_DATA"              },
    { V_RESULT_OUT_OF_MEMORY,        "QP_RESULT_OUT_OF_MEMORY"        },
    { V_RESULT_ILL_PARAM,            "QP_RESULT_PARSE_ERROR"          },
    { V_RESULT_ILL_PARAM,            "QP_RESULT_ILL_PARAM"            },
    { V_RESULT_ILL_PARAM,            "QP_RESULT_UNKNOWN_ELEMENT"      },
    { V_RESULT_ILL_PARAM,            "QP_RESULT_UNEXPECTED_ELEMENT"   },
    { V_RESULT_ILL_PARAM,            "QP_RESULT_UNKNOWN_ARGUMENT"     },
    { V_RESULT_ILL_PARAM,            "QP_RESULT_ILLEGAL_VALUE"        },
    { V_RESULT_UNSUPPORTED,          "QP_RESULT_NOT_IMPLEMENTED"      }
};

const std::size_t qpr_translations_len = (sizeof(qpr_translations) / sizeof(qpr_translations[0]));

static qpr_translation translate(cmn_qpResult qpResult)
{
    qpr_translation result;
    result.code  = V_RESULT_UNDEFINED;
    result.c_str = "unknown code";
    if (qpResult >= 0 && (static_cast<std::size_t>(qpResult) < qpr_translations_len)) {
        result = qpr_translations[qpResult];
    }
    return result;
}

#define ISOCPP_QP_RESULT_CHECK_AND_THROW(qpResult)                          \
        org::opensplice::core::utils::check_u_result_and_throw_exception(   \
            org::opensplice::core::helper::translate(qpResult).code,        \
            __FILE__,                                                       \
            __LINE__,                                                       \
            OS_PRETTY_FUNCTION,                                             \
            org::opensplice::core::helper::translate(qpResult).c_str)

}
}
}
} /* org::opensplice::core::helper namespace end */





const C_STRUCT(cmn_qosProviderInputAttr) org::opensplice::core::QosProviderDelegate::qosProviderAttr = {
    { &QosProviderDelegate::named_qos__copyOut<struct _DDS_NamedDomainParticipantQos, dds::domain::qos::DomainParticipantQos>  },
    { &QosProviderDelegate::named_qos__copyOut<struct _DDS_NamedTopicQos,             dds::topic::qos::TopicQos             >  },
    { &QosProviderDelegate::named_qos__copyOut<struct _DDS_NamedSubscriberQos,        dds::sub::qos::SubscriberQos          >  },
    { &QosProviderDelegate::named_qos__copyOut<struct _DDS_NamedDataReaderQos,        dds::sub::qos::DataReaderQos          >  },
    { &QosProviderDelegate::named_qos__copyOut<struct _DDS_NamedPublisherQos,         dds::pub::qos::PublisherQos           >  },
    { &QosProviderDelegate::named_qos__copyOut<struct _DDS_NamedDataWriterQos,        dds::pub::qos::DataWriterQos          >  }
};




org::opensplice::core::QosProviderDelegate::QosProviderDelegate(const std::string& uri, const std::string& id)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();
    if(uri.empty()) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_PRECONDITION_NOT_MET_ERROR, "Invalid Qos Provider URI (empty)");
    }

    this->qosProvider = cmn_qosProviderNew(uri.c_str(), id.c_str(), &this->qosProviderAttr);
    if (this->qosProvider == NULL) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "QoSProvider not properly instantiated");
    }
}

org::opensplice::core::QosProviderDelegate::~QosProviderDelegate()
{
    cmn_qosProviderFree(this->qosProvider);
}

dds::domain::qos::DomainParticipantQos
org::opensplice::core::QosProviderDelegate::participant_qos(const char* id)
{
    cmn_qpResult qpResult;
    dds::domain::qos::DomainParticipantQos qos;
    qpResult = cmn_qosProviderGetParticipantQos(this->qosProvider, id, &qos);
    ISOCPP_QP_RESULT_CHECK_AND_THROW(qpResult);
    return qos;
}

dds::topic::qos::TopicQos
org::opensplice::core::QosProviderDelegate::topic_qos(const char* id)
{
    cmn_qpResult qpResult;
    dds::topic::qos::TopicQos qos;
    qpResult = cmn_qosProviderGetTopicQos(this->qosProvider, id, &qos);
    ISOCPP_QP_RESULT_CHECK_AND_THROW(qpResult);
    return qos;
}

dds::sub::qos::SubscriberQos
org::opensplice::core::QosProviderDelegate::subscriber_qos(const char* id)
{
    cmn_qpResult qpResult;
    dds::sub::qos::SubscriberQos qos;
    qpResult = cmn_qosProviderGetSubscriberQos(this->qosProvider, id, &qos);
    ISOCPP_QP_RESULT_CHECK_AND_THROW(qpResult);
    return qos;
}

dds::sub::qos::DataReaderQos
org::opensplice::core::QosProviderDelegate::datareader_qos(const char* id)
{
    cmn_qpResult qpResult;
    dds::sub::qos::DataReaderQos qos;
    qpResult = cmn_qosProviderGetDataReaderQos(this->qosProvider, id, &qos);
    ISOCPP_QP_RESULT_CHECK_AND_THROW(qpResult);
    return qos;
}

dds::pub::qos::PublisherQos
org::opensplice::core::QosProviderDelegate::publisher_qos(const char* id)
{
    cmn_qpResult qpResult;
    dds::pub::qos::PublisherQos qos;
    qpResult = cmn_qosProviderGetPublisherQos(this->qosProvider, id, &qos);
    ISOCPP_QP_RESULT_CHECK_AND_THROW(qpResult);
    return qos;
}

dds::pub::qos::DataWriterQos
org::opensplice::core::QosProviderDelegate::datawriter_qos(const char* id)
{
    cmn_qpResult qpResult;
    dds::pub::qos::DataWriterQos qos;
    qpResult = cmn_qosProviderGetDataWriterQos(this->qosProvider, id, &qos);
    ISOCPP_QP_RESULT_CHECK_AND_THROW(qpResult);
    return qos;
}

template <typename FROM, typename TO>
void
org::opensplice::core::QosProviderDelegate::named_qos__copyOut(
    void *from,
    void *to)
{
    FROM *named_qos = (FROM*)from;
    TO   *dds_qos   = (TO*)to;
    /* Do the copy. */
    dds_qos->delegate().named_qos(*named_qos);
}
