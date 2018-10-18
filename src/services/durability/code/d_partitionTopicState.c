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
#include "d__partitionTopicState.h"
#include "d__group.h"
#include "d__types.h"
#include "d__durabilityStateRequest.h"
#include "d__durability.h"
#include "d__misc.h"
#include "client_durabilitySplType.h"


/**
 * \brief Create a partitionTopicState
 */
d_partitionTopicState
d_partitionTopicStateNew(
    d_group group)
{
    d_partitionTopicState state;

    assert(d_groupIsValid(group));

    /* Allocate partitionTopicState */
    state = d_partitionTopicState(os_malloc(C_SIZEOF(d_partitionTopicState)));
    if (state) {
        /* Call super-init */
        d_objectInit(d_object(state), D_PART_TOPIC_STATE,
                     (d_objectDeinitFunc)d_partitionTopicStateDeinit);
        /* Initialize partitionTopicState */
        state->topic = d_groupGetTopic(group);         /* uses os_strdup, so must be freed when deinit is called */
        if (!state->topic) {
            goto err_allocPartitionTopicStateTopic;
        }
        state->partition = d_groupGetPartition(group); /* uses os_strdup, so must be freed when deinit is called */
        if (!state->partition) {
            goto err_allocPartitionTopicStatePartition;
        }
        state->completeness = d_mapCompleteness(d_groupGetCompleteness(group));
        state->extensions = c_iterNew(NULL);
        if (!state->extensions) {
            goto err_allocPartitionTopicStateExtensions;
        }
    }
    return state;

err_allocPartitionTopicStateExtensions:
err_allocPartitionTopicStatePartition:
err_allocPartitionTopicStateTopic:
    d_partitionTopicStateFree(state);
    return NULL;
}

/**
 * \brief Deinitialize a partitionTopicState
 */
void
d_partitionTopicStateDeinit(
    d_partitionTopicState state)
{
    struct _DDS_NameValue_t *extension;

    assert(d_partitionTopicStateIsValid(state));

    if (state->topic) {
        os_free(state->topic);
        state->topic = NULL;
    }
    if (state->partition) {
        os_free(state->partition);
        state->partition = NULL;
    }
    if (state->extensions) {
        extension = (struct _DDS_NameValue_t *)c_iterTakeFirst(state->extensions);
        while (extension) {
            os_free(extension);
            extension = (struct _DDS_NameValue_t *)c_iterTakeFirst(state->extensions);
        }
        c_iterFree(state->extensions);
        state->extensions = NULL;
    }
    /* call super-deinit */
    d_objectDeinit(d_object(state));
}


/**
 * \brief Free the durabilityState
 */
void
d_partitionTopicStateFree(
    d_partitionTopicState state)
{
    assert(d_partitionTopicStateIsValid(state));

    d_objectFree(d_object(state));
}

