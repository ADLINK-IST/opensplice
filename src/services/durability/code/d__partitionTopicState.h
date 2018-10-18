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

#ifndef D__PARTITION_TOPIC_STATE_H
#define D__PARTITION_TOPIC_STATE_H

#include "d__types.h"
#include "d__admin.h"
#include "client_durabilitySplType.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_partitionTopicState validity.
 * Because d_partitionTopicState is a concrete class typechecking is required.
 */
#define             d_partitionTopicStateIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_PART_TOPIC_STATE)


/**
 * \brief The d_partitionTopicState cast macro.
 *
 * This macro casts an object to a d_partitionTopicState object.
 */
#define d_partitionTopicState(s) ((d_partitionTopicState)(s))


C_STRUCT(d_partitionTopicState) {
    C_EXTENDS(d_object);
    d_topic topic;
    d_partition partition;
    d_completeness completeness;
    c_iter extensions;
};


d_partitionTopicState            d_partitionTopicStateNew                 (d_group group);

void                             d_partitionTopicStateDeinit              (d_partitionTopicState state);

void                             d_partitionTopicStateFree                (d_partitionTopicState state);

#if defined (__cplusplus)
}
#endif

#endif /* D__PARTITION_TOPIC_STATE_H */
