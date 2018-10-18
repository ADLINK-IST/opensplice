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
#ifndef V_EVENT_H
#define V_EVENT_H

/** \file kernel/include/v_event.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "c_time.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if 0
#define EVENT_TRACE printf("PID<%d> : ", os_procIdSelf()); printf
#else
#define EVENT_TRACE(...)
#endif

#define V_EVENT_UNDEFINED                  (0x00000000U)
#define V_EVENT_OBJECT_DESTROYED           (0x00000001U)
#define V_EVENT_INCONSISTENT_TOPIC         (0x00000001U << 1)  /* 0x00000002U : 2 */
#define V_EVENT_SAMPLE_REJECTED            (0x00000001U << 2)  /* 0x00000004U : 4 */
#define V_EVENT_SAMPLE_LOST                (0x00000001U << 3)  /* 0x00000008U : 8 */
#define V_EVENT_OFFERED_DEADLINE_MISSED    (0x00000001U << 4)  /* 0x00000010U : 16 */
#define V_EVENT_REQUESTED_DEADLINE_MISSED  (0x00000001U << 5)  /* 0x00000020U : 32 */
#define V_EVENT_OFFERED_INCOMPATIBLE_QOS   (0x00000001U << 6)  /* 0x00000040U : 64 */
#define V_EVENT_REQUESTED_INCOMPATIBLE_QOS (0x00000001U << 7)  /* 0x00000080U : 128 */
#define V_EVENT_LIVELINESS_ASSERT          (0x00000001U << 8)  /* 0x00000100U : 256 */
#define V_EVENT_LIVELINESS_CHANGED         (0x00000001U << 9)  /* 0x00000200U : 512 */
#define V_EVENT_LIVELINESS_LOST            (0x00000001U << 10) /* 0x00000400U : 1024 */
#define V_EVENT_SERVICES_CHANGES           (0x00000001U << 11) /* 0x00000800U : 2048 */
#define V_EVENT_DATA_AVAILABLE             (0x00000001U << 12) /* 0x00001000U : 4096 */
#define V_EVENT_PUBLICATION_MATCHED        (0x00000001U << 13) /* 0x00002000U : 8192 */
#define V_EVENT_SUBSCRIPTION_MATCHED       (0x00000001U << 14) /* 0x00004000U : 16384 */
#define V_EVENT_NEW_GROUP                  (0x00000001U << 15) /* 0x00008000U : 32768 */
#define V_EVENT_SERVICESTATE_CHANGED       (0x00000001U << 16) /* 0x00010000U : 65536 */
#define V_EVENT_LEASE_RENEWED              (0x00000001U << 17) /* 0x00020000U : 131072 */
#define V_EVENT_LEASE_EXPIRED              (0x00000001U << 18) /* 0x00040000U : 262144 */
#define V_EVENT_TRIGGER                    (0x00000001U << 19) /* 0x00080000U : 524288 */
#define V_EVENT_TIMEOUT                    (0x00000001U << 20) /* 0x00100000U : 1048576 */
#define V_EVENT_TERMINATE                  (0x00000001U << 21) /* 0x00200000U : 2097152 */
#define V_EVENT_HISTORY_DELETE             (0x00000001U << 22) /* 0x00400000U : 4194304 */
#define V_EVENT_HISTORY_REQUEST            (0x00000001U << 23) /* 0x00800000U : 8388608 */
#define V_EVENT_PERSISTENT_SNAPSHOT        (0x00000001U << 24) /* 0x01000000U : 16777216 */
#define V_EVENT_ALL_DATA_DISPOSED          (0x00000001U << 25) /* 0x02000000U : 33554432 */
#define V_EVENT_ON_DATA_ON_READERS         (0x00000001U << 26) /* 0x04000000U : 67108864 */
#define V_EVENT_CONNECT_WRITER             (0x00000001U << 27) /* 0x08000000U : 134217728 */
#define V_EVENT_PREPARE_DELETE             (0x00000001U << 28) /* 0x10000000U : 268435456 */
#define V_EVENTMASK_ALL                    (0xffffffffU)

#define v_eventTest(events,kind) (((events)&(kind))==(kind))
#define v_eventMaskTest(events,mask) (((events)&(mask))!=0)

C_CLASS(v_event);
C_STRUCT(v_event) {
    v_eventKind kind;
    v_observable source;
    c_voidp data;
    c_bool handled; /* if true then listeners will not be invoked. */
};

C_CLASS(v_historyDeleteEventData);
C_STRUCT(v_historyDeleteEventData) {
    c_char *partitionExpression;
    c_char *topicExpression;
    c_time deleteTime;
};


#undef OS_API

#endif /* V_EVENT_H */
