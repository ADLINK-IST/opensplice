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
#include "d__durabilityState.h"
#include "d__types.h"
#include "d__durabilityStateRequest.h"
#include "d__partitionTopicState.h"
#include "d__durability.h"
#include "client_durabilitySplType.h"


/**
 * \brief Create a durabilityState
 *
 * The durabilityState object
 */
d_durabilityState
d_durabilityStateNew(
    d_admin admin)
{
    d_durability durability;
    d_durabilityState durabilityState = NULL;

    assert(d_adminIsValid(admin));

    durability = d_adminGetDurability(admin);

    /* Allocate durabilityState */
    durabilityState = d_durabilityState(os_malloc(C_SIZEOF(d_durabilityState)));
    if (durabilityState) {
        /* Call super-init */
        d_objectInit(d_object(durabilityState), D_DURABILITY_STATE,
                     (d_objectDeinitFunc)d_durabilityStateDeinit);
        /* Initialize durabilityState */
        durabilityState->version = d_durabilityGetMyVersion(durability);
        durabilityState->serverId = d_durabilityGetMyServerId(durability);
        durabilityState->requestIds = c_iterNew(NULL);
        if (!durabilityState->requestIds) {
            goto err_allocRequestIds;
        }
        durabilityState->dataState = c_iterNew(NULL);
        if (!durabilityState->dataState) {
            goto err_allocDataState;
        }
        durabilityState->extensions = c_iterNew(NULL);
        if (!durabilityState->extensions) {
            goto err_allocExtensions;
        }
    }
    return durabilityState;

err_allocExtensions:
err_allocDataState:
err_allocRequestIds:
    d_durabilityStateFree(durabilityState);
    return NULL;
}

/**
 * \brief Deinitialize a durabilityState
 */
void
d_durabilityStateDeinit(
    d_durabilityState durabilityState)
{
    struct _DDS_RequestId_t *requestId;
    d_partitionTopicState state;
    struct _DDS_NameValue_t *extension;

    assert(d_durabilityStateIsValid(durabilityState));

    if (durabilityState->requestIds) {
        requestId = (struct _DDS_RequestId_t *)c_iterTakeFirst(durabilityState->requestIds);
        while (requestId != NULL) {
            os_free(requestId);
            requestId = (struct _DDS_RequestId_t *)c_iterTakeFirst(durabilityState->requestIds);
        }
        c_iterFree(durabilityState->requestIds);
    }
    if (durabilityState->dataState) {
        state = d_partitionTopicState(c_iterTakeFirst(durabilityState->dataState));
        while (state) {
            d_partitionTopicStateFree(state);
            state = d_partitionTopicState(c_iterTakeFirst(durabilityState->dataState));
        }
        c_iterFree(durabilityState->dataState);
    }
    if (durabilityState->extensions) {
        extension = (struct _DDS_NameValue_t *)c_iterTakeFirst(durabilityState->extensions);
        while (extension) {
            os_free(extension);
            extension = (struct _DDS_NameValue_t *)c_iterTakeFirst(durabilityState->extensions);
        }
        c_iterFree(durabilityState->extensions);
    }
    /* call super-deinit */
    d_objectDeinit(d_object(durabilityState));
}


/**
 * \brief Free the durabilityState
 */
void
d_durabilityStateFree(
    d_durabilityState durabilityState)
{
    assert(d_durabilityStateIsValid(durabilityState));

    d_objectFree(d_object(durabilityState));
}

