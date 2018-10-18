/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef NB__COMMON_H_
#define NB__COMMON_H_

#define NB_PARTITION "NetworkingBridge"
/* The partition name for statuses */
#define NB_STATUS_PARTITION NB_PARTITION

#define NB_SERVICE_STATUS_TOPIC_NAME        "nb_serviceStatus"
#define NB_SERVICE_STATUS_TOPIC_TYPE_NAME   "NetworkingBridge::ServiceStatus"
#define NB_SERVICE_STATUS_TOPIC_KEY_LIST    "serviceId"
#define NB_SERVICE_STATUS_WRITER            NB_SERVICE_STATUS_TOPIC_NAME "Writer"
#define NB_STATUS_PUBLISHER                 "StatusPublisher"

/* The names for the subscription to publication info */
#define NB_INTEREST_PARTITION V_BUILTIN_PARTITION
#define NB_INTEREST_SUBSCRIBER "BuiltinSubscriber"
#define NB_INTEREST_READER_SUFFIX "Reader"
#define NB_INTEREST_WRITER_SUFFIX "Writer"
#define NB_INTEREST_PUBREADER "DCPSPublicationReader"
#define NB_INTEREST_SUBREADER "DCPSSubscriptionReader"
#define NB_INTEREST_CMREADER "CMDataReaderReader"
/* The entity names of the interest publisher */
#define NB_INTEREST_PUBLISHER "InterestPublisher"
#define NB_INTEREST_SUBWRITER "InterestWriter"

#endif /* NB__COMMON_H_ */
