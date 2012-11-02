/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define V_EVENT_UNDEFINED               (0x00000000U)
#define V_EVENT_OBJECT_DESTROYED        (0x00000001U)
#define V_EVENT_INCONSISTENT_TOPIC      (0x00000001U << 1)  /* 2 */
#define V_EVENT_SAMPLE_REJECTED         (0x00000001U << 2)  /* 4 */
#define V_EVENT_SAMPLE_LOST             (0x00000001U << 3)  /* 8 */
#define V_EVENT_DEADLINE_MISSED         (0x00000001U << 4)  /* 16 */
#define V_EVENT_INCOMPATIBLE_QOS        (0x00000001U << 5)  /* 32 */
#define V_EVENT_LIVELINESS_ASSERT       (0x00000001U << 6)  /* 64 */
#define V_EVENT_LIVELINESS_CHANGED      (0x00000001U << 7)  /* 128 */
#define V_EVENT_LIVELINESS_LOST         (0x00000001U << 8)  /* 256 */
#define V_EVENT_TOPIC_MATCHED           (0x00000001U << 9)  /* 512 */
#define V_EVENT_SERVICES_CHANGES        (0x00000001U << 10) /* 1024 */
#define V_EVENT_NEW_GROUP               (0x00000001U << 11) /* 2048 */
#define V_EVENT_DATA_AVAILABLE          (0x00000001U << 12) /* 4096 */
#define V_EVENT_SERVICESTATE_CHANGED    (0x00000001U << 13) /* 8192 */
#define V_EVENT_LEASE_RENEWED           (0x00000001U << 14) /* 16384 */
#define V_EVENT_LEASE_EXPIRED           (0x00000001U << 15) /* 32768 */
#define V_EVENT_TRIGGER                 (0x00000001U << 16) /* 65536 */
#define V_EVENT_TIMEOUT                 (0x00000001U << 17) /* 131072 */
#define V_EVENT_TERMINATE               (0x00000001U << 18) /* 262144 */
#define V_EVENT_HISTORY_DELETE          (0x00000001U << 19) /* 524288 */
#define V_EVENT_HISTORY_REQUEST         (0x00000001U << 20) /* 1048576 */
#define V_EVENT_PERSISTENT_SNAPSHOT     (0x00000001U << 21) /* 2097152 */
#define V_EVENT_ALL_DATA_DISPOSED       (0x00000001U << 22) /* 4194304 */
#define V_EVENTMASK_ALL                 (0xffffffffU)

#define v_eventTest(events,kind) (((events)&(kind))==(kind))
typedef c_ulong v_eventKind;

C_CLASS(v_event);
C_STRUCT(v_event) {
    v_eventKind kind;
    v_handle    source;
    c_voidp     userData;
};

C_CLASS(v_historyDeleteEventData);
C_STRUCT(v_historyDeleteEventData) {
    c_char *partitionExpression;
    c_char *topicExpression;
    c_time deleteTime;
};


#undef OS_API

#endif /* V_EVENT_H */
