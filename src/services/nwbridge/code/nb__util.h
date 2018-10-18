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
#ifndef NB__UTIL_H_
#define NB__UTIL_H_

#include "nb__common.h"
#include "c_typebase.h"
#include "c_iterator.h"

#include "vortex_os.h"
#include "os_iterator.h" /* For os_equality */

#define NB_SET_STATEBASED_TOPICQOS(qos) \
    do { \
        (qos)->durability.v.kind = V_DURABILITY_TRANSIENT; \
        (qos)->history.v.kind = V_HISTORY_KEEPLAST; \
        (qos)->history.v.depth = 1; \
        (qos)->reliability.v.kind = V_RELIABILITY_RELIABLE; \
        (qos)->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000); /* 100ms */ \
        (qos)->orderby.v.kind = V_ORDERBY_RECEPTIONTIME; \
        /* This is required to match the default qos from DDS API */ \
        (qos)->liveliness.v.lease_duration = OS_DURATION_INFINITE; \
    } while (0)

#define NB_SET_STATEBASED_WRITERQOS(qos) \
    do { \
        /* Copy from topic-QoS */ \
        NB_SET_STATEBASED_TOPICQOS(qos); \
        /* This allows an event-based reader to see all status updates; ultra convenient :). */ \
        (qos)->history.v.kind = V_HISTORY_KEEPALL; \
    } while (0)

#define NB_SET_STATEBASED_READERQOS(qos) \
    do { \
        NB_SET_STATEBASED_WRITERQOS(qos); \
    } while (0)

/* Compare function for ut_table elements using a string as key */
os_equality nb_compareByName(void *o1,
                             void *o2,
                             void *arg) __nonnull((1,2))
                                        __attribute_pure__;

os_equality nb_compareByGid(void *o1,
                            void *o2,
                            void *arg) __nonnull((1,2))
                                       __attribute_pure__;

c_bool nb_match(const char * const * partitions,
                c_ulong partitionsLen,
                const char *topicName,
                const char * const * includes,
                const char * const * excludes) __nonnull_all__
                                               __attribute_pure__;

/* Free a c_iter containing u_cfNode */
void nb_cfNodeIterFree(c_iter iter) __nonnull_all__;

#endif /* NB__UTIL_H_ */
