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

#include "d__historicalData.h"
#include "d__historicalDataRequest.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__publisher.h"
#include "d__subscriber.h"
#include "d__historicalDataRequestListener.h"
#include "d__configuration.h"
#include "d__misc.h"
#include "d_qos.h"
#include "v_time.h"
#include "os_report.h"

c_bool delete_pubinfo(void *listener,void *arg);


/* Implementation of the callback to answer a request */
c_bool
delete_pubinfo(
    void *listener,
    void *arg)
{
    OS_UNUSED_ARG(listener);
    OS_UNUSED_ARG(arg);

    /* The only action is to cleanup the pubinfo.
     * This will be done by the cleanup_node in
     * d_historicalDataRequestListener::d_historicalDataRequestListenerHandlerThread */
    return FALSE;
}


/* Add pubInfo to deathrow, schedule execution in 10s from now.
 * Returns pubInfo if an existing pubInfo is rescheduled, NULL otherwise */
static struct pubInfo *
insert_pubinfo (d_historicalDataRequestListener listener, struct pubInfo * pubinfo)
{
  ut_avlIPath_t ip;
  struct request_admin_node *n;
  os_mutexLock (&listener->queue_lock);
  if ((n = ut_avlLookupIPath (&listener->pubinfo_avltreedef, &listener->pubinfo_tree, pubinfo, &ip)) != NULL) {
    /* This should never happen */
    os_mutexUnlock (&listener->queue_lock);
    assert(0);
    return NULL;
  } else {
    /* Insert the node in the tree and in the priority queue */
    n = os_malloc (sizeof (*n));
    n->insert_time = os_timeEGet();
    n->expiration_time = os_timeEAdd(n->insert_time, OS_DURATION_INIT(10, 0));
    n->handle_time = OS_TIMEE_ZERO;
    n->callback = delete_pubinfo;    /* callback function */
    n->arg = pubinfo;                /* argument of the callback function */
    ut_avlInsertIPath (&listener->pubinfo_avltreedef, &listener->pubinfo_tree, n, &ip);
    ut_fibheapInsert (&prioqueue_fhdef, &listener->prioqueue, n);
    d_trace(D_TRACE_PRIO_QUEUE, "%s: avlnode %p created and added to deathrow, insert_time=%"PA_PRItime", expiration_time = %"PA_PRItime"\n",
        OS_FUNCTION, (void *)n, OS_TIMEE_PRINT(n->insert_time), OS_TIMEE_PRINT(n->expiration_time));
    if (n == ut_fibheapMin (&prioqueue_fhdef, &listener->prioqueue)) {
        d_trace(D_TRACE_PRIO_QUEUE, "%s: avlnode %p added to front of queue\n", OS_FUNCTION, (void *)n);
        os_condSignal (&listener->cond);
    }
    os_mutexUnlock (&listener->queue_lock);
    return NULL;
  }
}


/* Release pubInfo from deathrow, return NULL if not found */
struct pubInfo *
release_pubinfo (d_historicalDataRequestListener listener, struct pubInfo * pubinfo)
{
  ut_avlIPath_t ip;
  struct request_admin_node *n;
  os_mutexLock (&listener->queue_lock);
  if ((n = ut_avlLookupIPath (&listener->pubinfo_avltreedef, &listener->pubinfo_tree, pubinfo, &ip)) != NULL) {
    /* An old pubinfo (according to compare_request_by_pubinfo) already exists.
     * Take and return the old request */
    /* Remove the node from the priority queue and the tree */
    struct pubInfo *old_pubinfo = (struct pubInfo *)(n->arg);
    d_trace(D_TRACE_PRIO_QUEUE, "%s: avlnode %p released from the deathrow, insert_time=%"PA_PRItime", expiration_time=%"PA_PRItime"\n",
        OS_FUNCTION, (void *)n, OS_TIMEE_PRINT(n->insert_time), OS_TIMEE_PRINT(n->expiration_time));
    ut_avlDelete (&listener->pubinfo_avltreedef, &listener->pubinfo_tree, n);
    ut_fibheapDelete (&prioqueue_fhdef, &listener->prioqueue, n);
    os_mutexUnlock (&listener->queue_lock);
    return old_pubinfo;
  } else {
    d_trace(D_TRACE_PRIO_QUEUE, "%s: no matching avlnode found\n", OS_FUNCTION);
    os_mutexUnlock (&listener->queue_lock);
    return NULL;
  }
}


struct pubInfo *
get_or_create_pubinfo (d_historicalData historicalData)
{
    d_durability durability;
    d_subscriber subscriber;
    d_historicalDataRequestListener listener;
    d_configuration config;
    struct pubInfo *tmp_pubinfo, *found_pubinfo;
    v_publisherQos publisherQos;
    v_writerQos writerQos;
    u_result ur;

    assert(d_historicalDataIsValid(historicalData));

    durability = historicalData->durability;
    config = d_durabilityGetConfiguration(durability);
    subscriber = d_adminGetSubscriber(durability->admin);
    listener = d_subscriberGetHistoricalDataRequestListener(subscriber);
    tmp_pubinfo = (struct pubInfo *)os_malloc(sizeof(struct pubInfo));
    tmp_pubinfo->publisher = NULL;
    tmp_pubinfo->writer = NULL;
    tmp_pubinfo->partition = os_strdup(historicalData->alignmentPartition);
    if ((found_pubinfo = release_pubinfo(listener, tmp_pubinfo)) == NULL) {
        /* no publisher/writer exist for this combination yet */
        if ((publisherQos = d_publisherQosNew(historicalData->alignmentPartition)) == NULL) {
            goto err_allocPublisherQos;
        }
        if ((tmp_pubinfo->publisher = u_publisherNew(u_participant(d_durabilityGetService(durability)), config->clientDurabilityPublisherName, publisherQos, TRUE)) == NULL) {
            d_publisherQosFree(publisherQos);
            goto err_allocPublisher;
        }
        d_publisherQosFree(publisherQos);
        /* There are historicalData readers out there that read the data with a liveliness.lease_duration 0.
         * Since the liveliness qos matches only if the writerQos is less than or equal to the reader qos
         * we must ensure that the historicalDataWriter has a lease_duration of 0 also to prevent any
         * backwards compatibilites.
         * Because the writer and publisher will be destroyed when all historicalData has been
         * written we must ensure that autodispose_unregistered_instances is set to FALSE. This
         * ensures that the data is not removed when the writer is destroyed. */
        if ((writerQos = d_writerQosNew(V_DURABILITY_VOLATILE,V_RELIABILITY_RELIABLE,V_ORDERBY_SOURCETIME, config->alignerLatencyBudget, config->alignerTransportPriority)) == NULL) {
            goto err_allocWriterQos;
        }
        writerQos->lifecycle.v.autodispose_unregistered_instances = FALSE;
        if ((tmp_pubinfo->writer = u_writerNew (tmp_pubinfo->publisher, "historicalDataWriter", d_adminGetHistoricalDataTopic(durability->admin), writerQos)) == NULL) {
            d_writerQosFree(writerQos);
            goto err_allocWriter;
        }
        d_writerQosFree(writerQos);
        if (u_entityEnable(u_entity(tmp_pubinfo->writer)) != U_RESULT_OK) {
           goto err_writerEnable;
        }
        if ((ur = u_observableAction(u_observable(tmp_pubinfo->writer),d_publisherEnsureServicesAttached,durability)) != U_RESULT_OK) {
           goto err_observableAction;
        }
        historicalData->pubinfo = tmp_pubinfo;
    } else {
        /* a publisher/writer already exist */
        os_free(tmp_pubinfo->partition);
        os_free(tmp_pubinfo);
        historicalData->pubinfo = found_pubinfo;
    }
    return historicalData->pubinfo;

err_observableAction:
err_writerEnable:
    u_objectFree(u_object(tmp_pubinfo->writer));
err_allocWriter:
err_allocWriterQos:
    u_objectFree(u_object(tmp_pubinfo->publisher));
err_allocPublisher:
err_allocPublisherQos:
    os_free(tmp_pubinfo->partition);
    os_free(tmp_pubinfo);
    return NULL;
}


static v_copyin_result
d_publisherHistoricalDataWriterCopy(
    c_type type,
    const void *from,
    void *to)
{
    d_historicalData historicalData = d_historicalData(from);
    struct _DDS_HistoricalData *msgTo = (struct _DDS_HistoricalData *)to;
    c_base base = c_getBase(type);

    /* Copy the version */
    msgTo->version = historicalData->version;
    /* Copy my server gid */
    msgTo->serverId = historicalData->serverId;
    /* Copy the list of requestIds */
    {
        c_ulong len = 1;
        c_type subtype0;
        struct _DDS_RequestId_t *dst0;

        subtype0 = c_type(c_metaResolve (c_metaObject(base), "DDS::RequestId_t"));
        dst0 = (struct _DDS_RequestId_t *)c_sequenceNew_s(subtype0, len, len);
        c_free(subtype0);
        if (!dst0) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'HistoricalData.requestIds' could not be allocated.");
            goto err_allocRequestIds;
        }
        {
            c_iterIter iter = c_iterIterGet(historicalData->historicalDataRequest->requestIds);
            struct _DDS_RequestId_t *requestId;
            c_ulong i = 0;

            while ((requestId = c_iterNext(&iter)) != NULL) {
                dst0[i].clientId = requestId->clientId;
                dst0[i].requestId = requestId->requestId;
                i++;
            }
        }
        msgTo->requestIds = (c_sequence)dst0;
    }
    /* Copy content */
    msgTo->content._d = historicalData->content._d;
    /* Copy historicalDataContent */
    switch (msgTo->content._d) {
        case HISTORICAL_DATA_KIND_BEAD:
            memcpy(&msgTo->content._u.bead.writerId, &historicalData->content._u.bead.writerId, 16);
            msgTo->content._u.bead.sourceTimestamp = historicalData->content._u.bead.sourceTimestamp;
            msgTo->content._u.bead.sequenceNumber = historicalData->content._u.bead.sequenceNumber;
            msgTo->content._u.bead.qos = historicalData->content._u.bead.qos;
            {
                c_ulong len = c_iterLength(historicalData->partitions);
                c_type subtype0;
                c_string *dst0;

                subtype0 = c_type(c_metaResolve (c_metaObject(base), "c_string"));
                dst0 = (c_string *)c_sequenceNew_s(subtype0, len, len);
                c_free(subtype0);
                if (!dst0) {
                    OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'HistoricalData.partitions' could not be allocated.");
                    goto err_allocPartitions;
                }
                {
                    c_iterIter iter = c_iterIterGet(historicalData->partitions);
                    char * partition;
                    c_ulong i = 0;

                    while ((partition = (char *)c_iterNext(&iter)) != NULL) {
                        dst0[i] = c_stringNew_s(base, partition);
                        if (!dst0[i]) {
                            goto err_allocPartition;
                        }
                        i++;
                    }
                }
                msgTo->content._u.bead.partitions = (c_sequence)dst0;
            }
            msgTo->content._u.bead.kind = historicalData->content._u.bead.kind;
            memcpy(msgTo->content._u.bead.keyHash, historicalData->content._u.bead.keyHash, 16);
            msgTo->content._u.bead.serializationFormat = historicalData->content._u.bead.serializationFormat;
            {
                c_type subtype0;
                c_ulong len;
                c_octet *dst0;

                len = historicalData->beadSize;
                subtype0 = c_type(c_metaResolve (c_metaObject(base), "c_octet"));
                dst0 = (c_octet *)c_sequenceNew_s(subtype0, len, len);
                c_free(subtype0);
                if (!dst0) {
                    OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'HistoricalData.content._u.bead.payload' could not be allocated.");
                    goto err_allocPayload;
                }
                memcpy(dst0, historicalData->blob, len);
                msgTo->content._u.bead.payload=(c_sequence)dst0;
            }
            break;
        case HISTORICAL_DATA_KIND_LINK:
            msgTo->content._u.link.sampleCount = historicalData->content._u.link.sampleCount;
            msgTo->content._u.link.completeness = historicalData->content._u.link.completeness;
            msgTo->content._u.link.errorCode = historicalData->content._u.link.errorCode;
            break;
        default:
            OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'HistoricalData.content._d' is invalid");
            goto err_invalidUnionSelector;
            break;
    }
    /* Copy the extensions */
    {
        c_ulong len = c_iterLength(historicalData->extensions);
        c_type subtype0;
        struct _DDS_NameValue_t *dst0;

        subtype0 = c_type(c_metaResolve (c_metaObject(base), "DDS::NameValue_t"));
        dst0 = (struct _DDS_NameValue_t *)c_sequenceNew_s(subtype0, len, len);
        c_free(subtype0);
        if (!dst0) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'HistoricalData.extensions' could not be allocated.");
            goto err_allocExtensionsSeq;
        }
        {
            c_iterIter iter = c_iterIterGet(historicalData->extensions);
            struct _DDS_NameValue_t *extension;
            c_ulong i = 0;

            while ((extension = c_iterNext(&iter)) != NULL) {
                dst0[i].name = extension->name;
                dst0[i].value = extension->value;
                i++;
            }
        }
        msgTo->extensions = (c_sequence)dst0;
    }
    return V_COPYIN_RESULT_OK;


err_allocExtensionsSeq:
err_invalidUnionSelector:
err_allocPayload:
err_allocPartition:
    {
        c_iterIter iter = c_iterIterGet(historicalData->partitions);
        char * partition;

        while ((partition = (char *)c_iterNext(&iter)) != NULL) {
            c_free(partition);
        }
    }
err_allocPartitions:
err_allocRequestIds:

    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}



/**
 * \brief Write a HistoricalData message
 */
c_bool
d_historicalDataWrite(
    c_voidp *arg)
{
    d_durability durability;
    c_bool result = FALSE;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_serviceState state;
    d_historicalData historicalData = d_historicalData(arg);
    d_thread self = d_threadLookupSelf ();

    durability = historicalData->durability;
    state = d_durabilityGetState(durability);
    if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
        /* The service is not about to terminate, so it is OK to
         * publish historicalData messages. */
        if (historicalData->content._d == HISTORICAL_DATA_KIND_BEAD) {
            d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST, "TRACE %s: write bead msg %p\n", OS_FUNCTION, (void *)historicalData->vmessage);
        } else {
            d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST, "TRACE %s: write link\n", OS_FUNCTION);
        }
        terminate = FALSE;
        resendCount = 0;
        while ((!result) && (!terminate)) {
            d_threadAwake(self);
            ur = u_writerWrite(historicalData->pubinfo->writer,
                               d_publisherHistoricalDataWriterCopy,
                               historicalData, os_timeWGet(), U_INSTANCEHANDLE_NIL);
            if (ur == U_RESULT_OK) {
                result = TRUE;
            } else if (ur == U_RESULT_TIMEOUT) {
                terminate = d_durabilityMustTerminate(durability);
                resendCount++;
                if (terminate) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Failed to send historicalData message because durability is terminating.\n");
                } else if (resendCount == 1) {
                   /* Only a single log line in case of resends to prevent log pollution */
                    d_printTimedEvent(durability, D_LEVEL_WARNING,
                        "Failed to publish historicalData message with result %s, try to resend.\n", u_resultImage(ur));
                    OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                        "Failed to publish historicalData message with result %s, try to resend.", u_resultImage(ur));
                }
            } else {
                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                        "Publication of historicalData message FAILED with result %s, I am going to terminate\n", u_resultImage(ur));
                OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                        "Publication of historicalData message FAILED with result %s, I am going to terminate", u_resultImage(ur));
                d_durabilityTerminate(durability, TRUE);
                terminate = d_durabilityMustTerminate(durability);
            }
        } /* while */
        if ((result) && (resendCount > 0) && (!terminate)) {
                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                        "Publication of historicalData message succeeded after %d resends\n", resendCount);
        }
    }
    return result;
}


static struct _DDS_DurabilityVersion_t
respond_with_version(
    d_durability durability,
    struct _DDS_DurabilityVersion_t version)
{
    struct _DDS_DurabilityVersion_t v;
    assert(d_durabilityIsValid(durability));

    v = d_durabilityGetMyVersion(durability);
    if ((version.major < v.major) || ((version.major == v.major) && (version.minor < v.minor))) {
        v = version;
    }
    return v;
}


d_historicalData
d_historicalDataNew(
    d_admin admin,
    d_historicalDataRequest historicalDataRequest)
{
    d_historicalData historicalData;
    d_durability durability;

    assert(d_adminIsValid(admin));

    durability = d_adminGetDurability(admin);

    /* Allocate historical data request */
    historicalData = d_historicalData(os_malloc(C_SIZEOF(d_historicalData)));
    memset(historicalData, 0, sizeof(C_STRUCT(d_historicalData)));
    /* Call super-init */
    d_objectInit(d_object(historicalData), D_HISTORICAL_DATA, (d_objectDeinitFunc)d_historicalDataDeinit);
    /* Initialize historicalData */
    historicalData->durability = durability;
    historicalData->historicalDataRequest = historicalDataRequest;
    /* If the client uses a lower version than the server,
     * then the server must advertise the client's version
     * in the response to make sure that the client's
     * understands the message. */
    historicalData->version = respond_with_version(durability, historicalDataRequest->version);
    historicalData->serverId = d_durabilityGetMyServerId(durability);
    historicalData->errorCode = historicalDataRequest->errorCode;
    historicalData->requestIds = c_iterCopy(historicalDataRequest->requestIds);
    historicalData->partitions = c_iterNew(NULL);
    /* The historicalDataRequest already contains the list of matching
     * alignmentPartitions. According to the spec we must take the first
     * matching one */
    if (c_iterLength(historicalDataRequest->alignmentPartition) == 0) {
        historicalData->alignmentPartition = os_strdup("");   /* use default partition */
    } else {
        historicalData->alignmentPartition = os_strdup((char *)c_iterObject(historicalDataRequest->alignmentPartition, 0));
    }
    historicalData->extensions = c_iterNew(NULL);
    /* Set the attributes for the historicalDataState */
    historicalData->errorCode         = historicalDataRequest->errorCode;
    historicalData->group             = NULL;
    historicalData->count             = 0;
    historicalData->writeCount        = 0;
    historicalData->disposeCount      = 0;
    historicalData->writeDisposeCount = 0;
    historicalData->registerCount     = 0;
    historicalData->unregisterCount   = 0;
    historicalData->skipCount         = 0;
    historicalData->beadSize          = 0;
    historicalData->totalSize         = 0;
    /* Initial completeness */
    historicalData->completeness      = D_DDS_COMPLETENESS_COMPLETE;
    /* Create a writer/publisher */
    get_or_create_pubinfo(historicalData);
    return historicalData;
}


void
d_historicalDataDeinit(
    d_historicalData historicalData)
{

    d_admin admin;
    d_subscriber subscriber;
    d_historicalDataRequestListener listener;

    assert(d_historicalDataIsValid(historicalData));

    /* DDSI will drop data for writers that it doesn't know.
     * In case the publisher and writer and destroyed before
     * DDSI is able to send the data, DDSI will drop the data.
     * To prevent this from happening we will put the publisher
     * and writer on a death row. This gives DDSI some time to
     * send the data before the publisher and writer are destroyed. */
    admin = historicalData->durability->admin;
    subscriber  = d_adminGetSubscriber(admin);
    listener = d_subscriberGetHistoricalDataRequestListener(subscriber);
    /* cache the publisher and writer on the deathrow */
    insert_pubinfo(listener, historicalData->pubinfo);

    if (historicalData->requestIds) {
        /* RequestIds is an iter that was created using c_iterCopy.
         * Because c_iterCopy does not copy the contents we do not
         * have to free the contents. */
        c_iterFree(historicalData->requestIds);
        historicalData->requestIds = NULL;
    }
    if (historicalData->alignmentPartition) {
        os_free(historicalData->alignmentPartition);
        historicalData->alignmentPartition = NULL;
    }
    if (historicalData->extensions) {
        c_iterFree(historicalData->extensions);
        historicalData->extensions = NULL;
    }
    if (historicalData->partitions) {
        c_iterFree(historicalData->partitions);
        historicalData->partitions = NULL;
    }
    /* Call super deinit */
    d_objectDeinit(d_object(historicalData));
}


void
d_historicalDataFree(
    d_historicalData historicalData)
{
    assert(d_historicalDataIsValid(historicalData));

    d_objectFree(d_object(historicalData));
}
