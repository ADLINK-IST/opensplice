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

#ifndef CONTANTS_H_
#define CONTANTS_H_

#include "ccpp.h"
#include "DomainParticipantFactory.h"
#include "QosUtils.h"
#include "cpp_dcps_if.h"

#define TheParticipantFactory               (::DDS::DomainParticipantFactory::get_instance())

#define PARTICIPANTFACTORY_QOS_DEFAULT      (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_domainParticipantFactoryQos_default())
#define PARTICIPANT_QOS_DEFAULT             (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_domainParticipantQos_default())
#define TOPIC_QOS_DEFAULT                   (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_topicQos_default())
#define PUBLISHER_QOS_DEFAULT               (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_publisherQos_default())
#define SUBSCRIBER_QOS_DEFAULT              (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_subscriberQos_default())
#define DATAWRITER_QOS_DEFAULT              (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_dataWriterQos_default())
#define DATAREADER_QOS_DEFAULT              (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_dataReaderQos_default())
#define DATAREADERVIEW_QOS_DEFAULT          (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_dataReaderViewQos_default())

#define DATAWRITER_QOS_USE_TOPIC_QOS        (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_dataWriterQos_use_topic())
#define DATAREADER_QOS_USE_TOPIC_QOS        (*::DDS::OpenSplice::Utils::FactoryDefaultQosHolder::get_dataReaderQos_use_topic())

namespace DDS
{
    OS_API extern const ::DDS::Duration_t DURATION_ZERO;
    OS_API extern const ::DDS::Duration_t DURATION_INFINITE;

    OS_API extern const ::DDS::Time_t TIMESTAMP_INVALID;
    OS_API extern const ::DDS::Time_t TIMESTAMP_CURRENT;

    // Note: ANY_STATUS is deprecated, please use spec version specific constants.
    OS_API extern const ::DDS::StatusKind ANY_STATUS;
    // STATUS_MASK_ANY_V1_2 is all standardised status bits as of V1.2 of the
    // specification.
    OS_API extern const ::DDS::StatusKind STATUS_MASK_ANY_V1_2;
    OS_API extern const ::DDS::StatusKind STATUS_MASK_ANY;
    OS_API extern const ::DDS::StatusKind STATUS_MASK_NONE;

    /* ReturnCode which indicates if an instance handle has expired.
     * This ReturnCode is only to be used internally and is converted
     * to a ReturnCode BAD_PARAMETER when returning to the user.
     */
    const DDS::Long RETCODE_HANDLE_EXPIRED = (DDS::Long) 13UL;
}
#undef OS_API


#endif /* CONTANTS_H_ */
