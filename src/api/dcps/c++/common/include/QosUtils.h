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
#ifndef DDS_OPENSPLICE_QOSUTILS_H
#define DDS_OPENSPLICE_QOSUTILS_H


#include "ccpp.h"
#include "u_user.h"
#include "cpp_dcps_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


namespace DDS {
namespace OpenSplice {
namespace Utils {


/*
 * Default Qosses
 */
struct OS_API FactoryDefaultQosHolder
{
    static const DDS::DataReaderQos * get_dataReaderQos_default();
    static const DDS::DataReaderQos * get_dataReaderQos_use_topic();
    static const DDS::DataReaderViewQos * get_dataReaderViewQos_default();
    static const DDS::DataWriterQos * get_dataWriterQos_default();
    static const DDS::DataWriterQos * get_dataWriterQos_use_topic();
    static const DDS::DomainParticipantFactoryQos * get_domainParticipantFactoryQos_default();
    static const DDS::DomainParticipantQos * get_domainParticipantQos_default();
    static const DDS::PublisherQos * get_publisherQos_default();
    static const DDS::SubscriberQos * get_subscriberQos_default();
    static const DDS::TopicQos * get_topicQos_default();
};

/*
 * QoS validations
 */
OS_API DDS::ReturnCode_t qosIsConsistent( const DDS::DataReaderQos               &qos);
OS_API DDS::ReturnCode_t qosIsConsistent( const DDS::DataReaderViewQos           &qos);
OS_API DDS::ReturnCode_t qosIsConsistent( const DDS::DataWriterQos               &qos);
OS_API DDS::ReturnCode_t qosIsConsistent( const DDS::DomainParticipantFactoryQos &qos);
OS_API DDS::ReturnCode_t qosIsConsistent( const DDS::DomainParticipantQos        &qos);
OS_API DDS::ReturnCode_t qosIsConsistent( const DDS::PublisherQos                &qos);
OS_API DDS::ReturnCode_t qosIsConsistent( const DDS::SubscriberQos               &qos);
OS_API DDS::ReturnCode_t qosIsConsistent( const DDS::TopicQos                    &qos);


/*
 * QoS comparison
 */
OS_API DDS::Boolean
qosIsEqual (
    const DDS::DataReaderQos &a,
    const DDS::DataReaderQos &b);

OS_API DDS::Boolean
qosIsEqual (
    const DDS::DataReaderViewQos &a,
    const DDS::DataReaderViewQos &b);

OS_API DDS::Boolean
qosIsEqual (
    const DDS::DataWriterQos &a,
    const DDS::DataWriterQos &b);

OS_API DDS::Boolean
qosIsEqual (
    const DDS::DomainParticipantFactoryQos &a,
    const DDS::DomainParticipantFactoryQos &b);

OS_API DDS::Boolean
qosIsEqual (
    const DDS::DomainParticipantQos &a,
    const DDS::DomainParticipantQos &b);

OS_API DDS::Boolean
qosIsEqual (
    const DDS::PublisherQos &a,
    const DDS::PublisherQos &b);

OS_API DDS::Boolean
qosIsEqual (
    const DDS::SubscriberQos &a,
    const DDS::SubscriberQos &b);

OS_API DDS::Boolean
qosIsEqual (
    const DDS::TopicQos &a,
    const DDS::TopicQos &b);


/*
 * QoS conversions
 */
OS_API DDS::ReturnCode_t copyQosIn( const DDS::DataReaderQos        &from, u_readerQos        &to);
OS_API DDS::ReturnCode_t copyQosIn( const DDS::DataReaderViewQos    &from, u_dataViewQos      &to);
OS_API DDS::ReturnCode_t copyQosIn( const DDS::DataWriterQos        &from, u_writerQos        &to);
OS_API DDS::ReturnCode_t copyQosIn( const DDS::DomainParticipantQos &from, u_participantQos   &to);
OS_API DDS::ReturnCode_t copyQosIn( const DDS::PublisherQos         &from, u_publisherQos     &to);
OS_API DDS::ReturnCode_t copyQosIn( const DDS::SubscriberQos        &from, u_subscriberQos    &to);
OS_API DDS::ReturnCode_t copyQosIn( const DDS::TopicQos             &from, u_topicQos         &to);

OS_API DDS::ReturnCode_t copyQosOut( const u_readerQos      &from,  DDS::DataReaderQos        &to);
OS_API DDS::ReturnCode_t copyQosOut( const u_dataViewQos    &from,  DDS::DataReaderViewQos    &to);
OS_API DDS::ReturnCode_t copyQosOut( const u_writerQos      &from,  DDS::DataWriterQos        &to);
OS_API DDS::ReturnCode_t copyQosOut( const u_participantQos &from,  DDS::DomainParticipantQos &to);
OS_API DDS::ReturnCode_t copyQosOut( const u_publisherQos   &from,  DDS::PublisherQos         &to);
OS_API DDS::ReturnCode_t copyQosOut( const u_subscriberQos  &from,  DDS::SubscriberQos        &to);
OS_API DDS::ReturnCode_t copyQosOut( const u_topicQos       &from,  DDS::TopicQos             &to);


} /* end namespace Utils */
} /* end namespace OpenSplice */
} /* end namespace DDS */

#undef OS_API
#endif /* DDS_OPENSPLICE_QOSUTILS_H */


