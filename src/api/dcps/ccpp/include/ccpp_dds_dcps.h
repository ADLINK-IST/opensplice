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
#ifndef CCPP_DDS_DCPS_H
#define CCPP_DDS_DCPS_H

#include "ccpp.h"
#include "ccpp_DomainParticipantFactory.h"
#include "ccpp_GuardCondition.h"
#include "ccpp_QosProvider.h"
#include "ccpp_WaitSet.h"
#include "ccpp_ErrorInfo.h"
#include "ccpp_dds_builtinTopics.h"
#include "ccpp_dds_dcps_builtintopics.h"
#include "ccpp_dds_namedQosTypes.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define TheParticipantFactory            (::DDS::DomainParticipantFactory::get_instance())
#define PARTICIPANT_QOS_DEFAULT         (*::DDS::DomainParticipantFactory::participant_qos_default())
#define TOPIC_QOS_DEFAULT               (*::DDS::DomainParticipantFactory::topic_qos_default())
#define PUBLISHER_QOS_DEFAULT           (*::DDS::DomainParticipantFactory::publisher_qos_default())
#define SUBSCRIBER_QOS_DEFAULT          (*::DDS::DomainParticipantFactory::subscriber_qos_default())
#define DATAREADER_QOS_DEFAULT          (*::DDS::DomainParticipantFactory::datareader_qos_default())
#define DATAREADERVIEW_QOS_DEFAULT      (*::DDS::DomainParticipantFactory::datareaderview_qos_default())
#define DATAREADER_QOS_USE_TOPIC_QOS    (*::DDS::DomainParticipantFactory::datareader_qos_use_topic_qos())
#define DATAWRITER_QOS_DEFAULT          (*::DDS::DomainParticipantFactory::datawriter_qos_default())
#define DATAWRITER_QOS_USE_TOPIC_QOS    (*::DDS::DomainParticipantFactory::datawriter_qos_use_topic_qos())

namespace DDS
{
  const ::DDS::Duration_t DURATION_ZERO               = {DURATION_ZERO_SEC, DURATION_ZERO_NSEC};
  const ::DDS::Duration_t DURATION_INFINITE           = {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC};
  const ::DDS::Time_t     TIMESTAMP_INVALID           = {TIMESTAMP_INVALID_SEC, TIMESTAMP_INVALID_NSEC};
  // Note: ANY_STATUS is deprecated, please use spec version specific constants.
  const ::DDS::StatusKind ANY_STATUS                  = 0x7FE7;
  // STATUS_MASK_ANY_V1_2 is all standardised status bits as of V1.2 of the
  // specification.
  const ::DDS::StatusKind STATUS_MASK_ANY_V1_2        = 0x7FE7;
  const ::DDS::StatusKind STATUS_MASK_NONE            = 0x0;
}

#endif /* CCPP_DDS_DCPS_H */

