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

#ifndef qp_defaultQos_H_
#define qp_defaultQos_H_

#if defined (__cplusplus)
extern "C" {
#endif

/* dds_namedQosTypesSplType is generated with idlpp -m SPLTYPE */
#include "dds_namedQosTypesSplType.h"

/* Default DomainParticipant-QoS according to specification in database-format.
 * NOTE: Sequences and strings are NULL! */
extern const struct _DDS_NamedDomainParticipantQos qp_NamedDomainParticipantQos_default;

/* Default Topic-QoS according to specification in database-format.
 * NOTE: Sequences and strings are NULL! */
extern const struct _DDS_NamedTopicQos qp_NamedTopicQos_default;

/* Default Publisher-QoS according to specification in database-format.
 * NOTE: Sequences and strings are NULL! */
extern const struct _DDS_NamedPublisherQos qp_NamedPublisherQos_default;

/* Default DataWriter-QoS according to specification in database-format.
 * NOTE: Sequences and strings are NULL! */
extern const struct _DDS_NamedDataWriterQos qp_NamedDataWriterQos_default;

/* Default Subscriber-QoS according to specification in database-format.
 * NOTE: Sequences and strings are NULL! */
extern const struct _DDS_NamedSubscriberQos qp_NamedSubscriberQos_default;

/* Default DataReader-QoS according to specification in database-format.
 * NOTE: Sequences and strings are NULL! */
extern const struct _DDS_NamedDataReaderQos qp_NamedDataReaderQos_default;

#if defined (__cplusplus)
}
#endif

#endif /* qp_defaultQos_H_ */
