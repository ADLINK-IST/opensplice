/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
