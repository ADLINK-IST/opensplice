#ifndef CCPP_DDS_DCPS_H
#define CCPP_DDS_DCPS_H

#include "ccpp.h"
#include "ccpp_DomainParticipantFactory.h"
#include "ccpp_GuardCondition.h"
#include "ccpp_WaitSet.h"
#include "ccpp_ErrorInfo.h"
#include "ccpp_dds_dcps_builtintopics.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define TheParticipantFactory            (::DDS::DomainParticipantFactory::get_instance())
#define PARTICIPANT_QOS_DEFAULT         (*::DDS::DomainParticipantFactory::participant_qos_default())
#define TOPIC_QOS_DEFAULT               (*::DDS::DomainParticipantFactory::topic_qos_default())
#define PUBLISHER_QOS_DEFAULT           (*::DDS::DomainParticipantFactory::publisher_qos_default())
#define SUBSCRIBER_QOS_DEFAULT          (*::DDS::DomainParticipantFactory::subscriber_qos_default())
#define DATAREADER_QOS_DEFAULT          (*::DDS::DomainParticipantFactory::datareader_qos_default())
#define DATAREADER_QOS_USE_TOPIC_QOS    (*::DDS::DomainParticipantFactory::datareader_qos_use_topic_qos())
#define DATAWRITER_QOS_DEFAULT          (*::DDS::DomainParticipantFactory::datawriter_qos_default())
#define DATAWRITER_QOS_USE_TOPIC_QOS    (*::DDS::DomainParticipantFactory::datawriter_qos_use_topic_qos())

namespace DDS
{
  const ::DDS::Duration_t DURATION_ZERO               = {0L,0U};
  const ::DDS::Duration_t DURATION_INFINITE           = {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC};
  const ::DDS::StatusKind ANY_STATUS                  = 0xFFFF;


}

#endif /* CCPP_DDS_DCPS_H */

