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

#include "dds_dcps.h"
#include "kernelModuleI.h"

#define C(a, b) char eq_##a##_##b[((int) (a) == (int) (b)) ? 1 : -1]
#define T(name, chks) struct static_check_##name { chks; char dummy; }

T (DDS_DurabilityQosPolicyKind,
   C (DDS_VOLATILE_DURABILITY_QOS, V_DURABILITY_VOLATILE);
   C (DDS_TRANSIENT_LOCAL_DURABILITY_QOS, V_DURABILITY_TRANSIENT_LOCAL);
   C (DDS_TRANSIENT_DURABILITY_QOS, V_DURABILITY_TRANSIENT);
   C (DDS_PERSISTENT_DURABILITY_QOS, V_DURABILITY_PERSISTENT));

T (DDS_PresentationQosPolicyAccessScopeKind,
   C (DDS_INSTANCE_PRESENTATION_QOS, V_PRESENTATION_INSTANCE);
   C (DDS_TOPIC_PRESENTATION_QOS, V_PRESENTATION_TOPIC);
   C (DDS_GROUP_PRESENTATION_QOS, V_PRESENTATION_GROUP));

T (DDS_OwnershipQosPolicyKind,
   C (DDS_SHARED_OWNERSHIP_QOS, V_OWNERSHIP_SHARED);
   C (DDS_EXCLUSIVE_OWNERSHIP_QOS, V_OWNERSHIP_EXCLUSIVE));

T (DDS_LivelinessQosPolicyKind,
   C (DDS_AUTOMATIC_LIVELINESS_QOS, V_LIVELINESS_AUTOMATIC);
   C (DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, V_LIVELINESS_PARTICIPANT);
   C (DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS, V_LIVELINESS_TOPIC));

T (DDS_ReliabilityQosPolicyKind,
   C (DDS_BEST_EFFORT_RELIABILITY_QOS, V_RELIABILITY_BESTEFFORT);
   C (DDS_RELIABLE_RELIABILITY_QOS, V_RELIABILITY_RELIABLE));

T (DDS_DestinationOrderQosPolicyKind,
   C (DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, V_ORDERBY_RECEPTIONTIME);
   C (DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS, V_ORDERBY_SOURCETIME));

T (DDS_HistoryQosPolicyKind,
   C (DDS_KEEP_LAST_HISTORY_QOS, V_HISTORY_KEEPLAST);
   C (DDS_KEEP_ALL_HISTORY_QOS, V_HISTORY_KEEPALL));

#if 0 /* no equivalent in kernel */
T (DDS_InvalidSampleVisibilityQosPolicyKind,
   C (DDS_NO_INVALID_SAMPLES, ...);
   C (DDS_MINIMUM_INVALID_SAMPLES, ...);
   C (DDS_ALL_INVALID_SAMPLES, ...));
#endif

T (DDS_SchedulingClassQosPolicyKind,
   C (DDS_SCHEDULE_DEFAULT, V_SCHED_DEFAULT);
   C (DDS_SCHEDULE_TIMESHARING, V_SCHED_TIMESHARING);
   C (DDS_SCHEDULE_REALTIME, V_SCHED_REALTIME));

T (DDS_SchedulingPriorityQosPolicyKind,
   C (DDS_PRIORITY_RELATIVE, V_SCHED_PRIO_RELATIVE);
   C (DDS_PRIORITY_ABSOLUTE, V_SCHED_PRIO_ABSOLUTE));
