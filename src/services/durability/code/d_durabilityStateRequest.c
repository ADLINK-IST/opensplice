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
#include "d__durabilityStateRequest.h"
#include "d__admin.h"
#include "d__types.h"
#include "client_durabilitySplType.h"
#include "d__misc.h"


/**
 * \brief Create a durabilityStateRequest
 */
d_durabilityStateRequest
d_durabilityStateRequestNew(
    d_admin admin,
    struct _DDS_DurabilityVersion_t version,
    struct _DDS_RequestId_t requestId,
    d_topic topic,
    c_iter partitions,
    c_iter serverIds,
    c_time timeout,
    c_iter extensions)
{
    d_durabilityStateRequest request = NULL;

    OS_UNUSED_ARG(admin);

    /* Allocate durabilityStateRequest */
    request = d_durabilityStateRequest(os_malloc(C_SIZEOF(d_durabilityStateRequest)));
    if (request) {
        /* Call super-init */
        d_objectInit(d_object(request), D_DURABILITY_STATE_REQ,
                     (d_objectDeinitFunc)d_durabilityStateRequestDeinit);
        /* Initialize durabilityStateRequest */
        request->version = version;
        request->requestId = requestId;
        request->timeout = timeout;
        request->topic = os_strdup(topic);
        request->partitions = c_iterCopy(partitions);  /* NULL equals empty iter */
        request->serverIds = c_iterCopy(serverIds);    /* NULL equals empty iter */
        request->extensions = c_iterCopy(extensions);  /* NULL equals empty iter */
        request->forMe = FALSE;
        request->forEverybody = (c_iterLength(serverIds) == 0);
    }
    return request;
}


/**
 * \brief Deinitialize a durabilityStateRequest
 */
void
d_durabilityStateRequestDeinit(
    d_durabilityStateRequest durabilityStateRequest)
{
    char *partition;
    struct _DDS_Gid_t *serverId;
    struct _DDS_NameValue_t *extension;

    assert(d_durabilityStateRequestIsValid(durabilityStateRequest));

    if (durabilityStateRequest->topic) {
        os_free(durabilityStateRequest->topic);
    }
    if (durabilityStateRequest->partitions) {
        partition = c_iterTakeFirst(durabilityStateRequest->partitions);
        while (partition) {
            os_free(partition);
            partition = c_iterTakeFirst(durabilityStateRequest->partitions);
        }
    }
    if (durabilityStateRequest->serverIds) {
        serverId = c_iterTakeFirst(durabilityStateRequest->serverIds);
        while (serverId) {
            os_free(serverId);
            serverId = c_iterTakeFirst(durabilityStateRequest->serverIds);
        }
    }
    if (durabilityStateRequest->extensions) {
        extension = c_iterTakeFirst(durabilityStateRequest->extensions);
        while (extension) {
            os_free(extension);
            extension = c_iterTakeFirst(durabilityStateRequest->extensions);
        }
    }
    /* call super-deinit */
    d_objectDeinit(d_object(durabilityStateRequest));
}


/**
 * \brief Free the durabilityStateRequest
 */
void
d_durabilityStateRequestFree(
    d_durabilityStateRequest durabilityStateRequest)
{
    assert(d_durabilityStateRequestIsValid(durabilityStateRequest));

    d_objectFree(d_object(durabilityStateRequest));
}


/**
 * \brief Check if the durabilityStateRequest contains errors.
 *
 * If errors are encountered, the error code and error string are returned.
 * Only one error message will be returned.
 *
 * @return TRUE, if no errors are detected, FALSE otherwise
 */
c_ulong
d_durabilityStateRequestSanityCheck(
    d_durabilityStateRequest durabilityStateRequest)
{
    assert(d_durabilityStateRequestIsValid(durabilityStateRequest));

    if (durabilityStateRequest->topic == NULL) {
        return D_DDS_RETCDE_NO_TOPIC;
    } else if ( c_iterLength(durabilityStateRequest->partitions) == 0 ) {
        return D_DDS_RETCDE_NO_PARTITIONS;
    }
    /* No sanity error */
    return D_DDS_RETCDE_NO_ERROR;
}
