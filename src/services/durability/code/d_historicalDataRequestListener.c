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
#include "vortex_os.h"
#include "d__historicalDataRequestListener.h"
#include "d__historicalDataRequest.h"
#include "d__historicalData.h"
#include "d__configuration.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__actionQueue.h"
#include "d__readerListener.h"
#include "d__misc.h"
#include "d__thread.h"
#include "d__waitset.h"
#include "d__group.h"
#include "d__publisher.h"
#include "d__nameSpace.h"
#include "d__eventListener.h"
#include "d_qos.h"
#include "u_listener.h"
#include "v_message.h"
#include "v_dataReaderSample.h"
#include "v_observer.h"
#include <stddef.h>   /* for definition of offsetof */

#include "ut_md5.h"
#include "v_group.h"
#include "v_builtin.h"
#include "v_time.h"
#include "v_state.h"
#include "v_historicalDataRequest.h"
#include "v_groupInstance.h"
#include "v_entity.h"
#include "v_message.h"
#include "v_dataReaderInstance.h"
#include "v_topic.h"
#include "v_messageQos.h"
#include "sd_serializer.h"
#include "sd_serializerBigE.h"
#include "sd_cdr.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_time.h"
#include "c_misc.h"

#include "client_durabilitySplType.h"

/**
 * Macro that checks the d_historicalDataRequestListener validity.
 * Because d_historicalDataRequestListener is a concrete class typechecking is required.
 */
#define d_historicalDataRequestListenerIsValid(_this)   \
        d_listenerIsValidKind(d_listener(_this), D_HISTORICAL_DATA_REQ_LISTENER)

/**
 * \brief The d_historicalDataRequestListener cast macro.
 *
 * This macro casts an object to a d_historicalDataRequestListener object.
 */
#define d_historicalDataRequestListener(_this) ((d_historicalDataRequestListener)(_this))

static int compare_by_prio (const void *va, const void *vb);

/* Callback function signatures */
c_bool answer_request(void *listener, void *arg);
static void cleanup_request (void *n);
static void cleanup_pubinfo (void *n);

const ut_fibheapDef_t prioqueue_fhdef = UT_FIBHEAPDEF_INITIALIZER(offsetof (struct request_admin_node, fhnode), compare_by_prio);

static int
compare_request_by_requestId (const void *va, const void *vb)
{
    const d_historicalDataRequest a = d_historicalDataRequest(va);
    const d_historicalDataRequest b = d_historicalDataRequest(vb);

    /* Compare requests based on their requestId.
     * A precondition is that a historicalDataRequest may only contain a single requestId.
     * Combined requests containing multiple requestIds should NOT be compared!
     */
    assert(c_iterLength(a->requestIds)==1);
    assert(c_iterLength(b->requestIds)==1);

    return d_historicalDataRequestCompareByRequestId(a, b);
}


static int
compare_pubinfo_by_partition (const void *va, const void *vb)
{
    const struct pubInfo *a = (struct pubInfo *)va;
    const struct pubInfo *b = (struct pubInfo *)vb;

    /* Compare pubInfo based on their partition. */

    assert(a);
    assert(b);
    assert(a->partition);
    assert(b->partition);

    return strcmp(a->partition, b->partition);
}


static int
compare_by_prio (const void *va, const void *vb)
{
    const struct request_admin_node *a = (struct request_admin_node *)va;
    const struct request_admin_node *b = (struct request_admin_node *)vb;

    os_compare eq;
    eq = os_timeECompare(a->expiration_time, b->expiration_time);
    if (eq == OS_LESS) {
      return -1;
    } else if (eq == OS_MORE) {
      return 1;
    } else {
      return 0;
    }
}


static os_timeE
time_of_next_request_unlocked (d_historicalDataRequestListener listener)
{
  struct request_admin_node *n;
  os_timeE t;
  if ((n = ut_fibheapMin (&prioqueue_fhdef, &listener->prioqueue)) != NULL) {
      t = n->expiration_time;
  } else {
      t = OS_TIMEE_INFINITE;
  }
  return t;
}

d_historicalDataRequest
reschedule_request (d_historicalDataRequestListener listener, d_historicalDataRequest historicalDataRequest, os_duration d)
{
  ut_avlIPath_t ip;
  struct request_admin_node *n;

  assert(d_historicalDataRequestListenerIsValid(listener));
  assert(d_historicalDataRequestIsValid(historicalDataRequest));

  os_mutexLock (&listener->queue_lock);
  if ((n = ut_avlLookupIPath (&listener->request_avltreedef, &listener->request_id_tree, historicalDataRequest, &ip)) != NULL) {
    /* An old request (according to compare_request_by_requestId) already exists.
     * Update the expiration time of the old request and return the old request
     */
    d_historicalDataRequest oldreq = d_historicalDataRequest(n->arg);
    oldreq->expirationTime = os_timeEAdd(oldreq->expirationTime, d);
    d_trace(D_TRACE_PRIO_QUEUE, "%s: existing avlnode %p found, insert_time=%"PA_PRItime", expiration_time = %"PA_PRItime"\n",
        OS_FUNCTION, (void *)n, OS_TIMEE_PRINT(n->insert_time), OS_TIMEE_PRINT(n->expiration_time));
    os_mutexUnlock (&listener->queue_lock);
    return oldreq;
  } else {
    /* Insert the node in the tree and in the priority queue */
    n = os_malloc (sizeof (*n));
    n->insert_time = os_timeEGet();
    n->expiration_time = os_timeEAdd(historicalDataRequest->expirationTime, d);
    n->handle_time = OS_TIMEE_ZERO;
    n->callback = answer_request;    /* callback function */
    n->arg = historicalDataRequest;  /* argument of the callback function */
    d_trace(D_TRACE_PRIO_QUEUE, "%s: avlnode %p rescheduled insert_time=%"PA_PRItime", expiration_time = %"PA_PRItime"\n",
        OS_FUNCTION, (void *)n, OS_TIMEE_PRINT(n->insert_time), OS_TIMEE_PRINT(n->expiration_time));
    ut_avlInsertIPath (&listener->request_avltreedef, &listener->request_id_tree, n, &ip);
    ut_fibheapInsert (&prioqueue_fhdef, &listener->prioqueue, n);
    /* Signal if the element was added to the front of the priority queue */
    if (n == ut_fibheapMin (&prioqueue_fhdef, &listener->prioqueue)) {
        d_trace(D_TRACE_PRIO_QUEUE, "%s: avlnode %p rescheduled to front of queue\n", OS_FUNCTION, (void *)n, (void *)historicalDataRequest);
        os_condSignal (&listener->cond);
    }
    os_mutexUnlock (&listener->queue_lock);
    return NULL;
  }
}

/**
 * Inserts a historicalDatataRequest in the priority queue
 */
d_historicalDataRequest
insert_request (d_historicalDataRequestListener listener, d_historicalDataRequest historicalDataRequest)
{
  ut_avlIPath_t ip;
  struct request_admin_node *n;

  assert(d_historicalDataRequestListenerIsValid(listener));
  assert(d_historicalDataRequestIsValid(historicalDataRequest));

  os_mutexLock (&listener->queue_lock);
  if ((n = ut_avlLookupIPath (&listener->request_avltreedef, &listener->request_id_tree, historicalDataRequest, &ip)) != NULL) {
    /* An old request (according to compare_request_by_requestId) already exists.
     * Ignore the new request and return the old request
     */
    d_historicalDataRequest oldreq = d_historicalDataRequest(n->arg);
    d_trace(D_TRACE_PRIO_QUEUE, "%s: existing avlnode %p found, insert_time=%"PA_PRItime", expiration_time = %"PA_PRItime"\n",
        OS_FUNCTION, (void *)n, OS_TIMEE_PRINT(n->insert_time), OS_TIMEE_PRINT(n->expiration_time));
    d_historicalDataRequestFree(historicalDataRequest);
    os_mutexUnlock (&listener->queue_lock);
    return oldreq;
  } else {
    /* Insert the node in the tree and in the priority queue */
    n = os_malloc (sizeof (*n));
    n->insert_time = os_timeEGet();
    n->expiration_time = historicalDataRequest->expirationTime;
    n->handle_time = OS_TIMEE_ZERO;
    n->callback = answer_request;    /* callback function */
    n->arg = historicalDataRequest;  /* argument of the callback function */
    d_trace(D_TRACE_PRIO_QUEUE, "%s: avlnode %p created insert_time=%"PA_PRItime", expiration_time = %"PA_PRItime"\n",
        OS_FUNCTION, (void *)n, OS_TIMEE_PRINT(n->insert_time), OS_TIMEE_PRINT(n->expiration_time));
    ut_avlInsertIPath (&listener->request_avltreedef, &listener->request_id_tree, n, &ip);
    ut_fibheapInsert (&prioqueue_fhdef, &listener->prioqueue, n);
    /* Signal if the element was added to the front of the priority queue */
    if (n == ut_fibheapMin (&prioqueue_fhdef, &listener->prioqueue)) {
        d_trace(D_TRACE_PRIO_QUEUE, "%s: avlnode %p added to front of queue\n", OS_FUNCTION, (void *)n, (void *)historicalDataRequest);
        os_condSignal (&listener->cond);
    }
    os_mutexUnlock (&listener->queue_lock);
    return NULL;
  }
}

/**
 * Take the next node from the priority queue.
 *
 * It is assumed that the caller locks the request queue.
 */
static struct request_admin_node *
take_next_request_unlocked (
    d_historicalDataRequestListener listener)
{
  struct request_admin_node *n;

  assert(d_historicalDataRequestListenerIsValid(listener));

  if ((n = ut_fibheapExtractMin (&prioqueue_fhdef, &listener->prioqueue)) != NULL) {
    /* To determine from which tree to delete the node we use the callback */
    if (n->callback == answer_request) {
       ut_avlDelete (&listener->request_avltreedef, &listener->request_id_tree, n);
    } else if (n->callback == delete_pubinfo) {
       ut_avlDelete (&listener->pubinfo_avltreedef, &listener->pubinfo_tree, n);
    } else {
       abort();
    }
    d_trace(D_TRACE_PRIO_QUEUE, "%s: avlnode %p extracted from queue\n", OS_FUNCTION, (void *)n);
    return n;
  } else {
    d_trace(D_TRACE_PRIO_QUEUE, "%s: queue is empty\n", OS_FUNCTION, (void *)n);
    return NULL;
  }
}

/**
 * \brief Indicates if the specified type is a primitive type.
 *
 * For primitive types it returns the size of the type
 * in number of bytes.
 *
 * If the type is not a keyType then 0 is returned.
 */
static int isKeyType (c_type type)
{
  switch (c_baseObjectKind (type))
  {
    case M_PRIMITIVE:
      switch (c_primitiveKind ((c_object) type)) {
        case P_BOOLEAN:
        case P_CHAR:
        case P_OCTET:
        case P_SHORT:
        case P_USHORT:
        case P_LONG:
        case P_ULONG:
        case P_LONGLONG:
        case P_ULONGLONG:
        case P_FLOAT:
        case P_DOUBLE:
            return 1;
        default:
          return 0;
      }
      /* NOTREACHED */
    case M_ENUMERATION:
      return 1;
    case M_COLLECTION:
      switch (c_collectionTypeKind ((c_object) type))
      {
        case OSPL_C_STRING:
          return 1;
        default:
          return 0;
      }
      /* NOTREACHED */
    default:
      return 0;
  }
}


/**
 * \brief Calculate the keyHash associated with a vgroup
 *
 * The keyHash is a CDR-BE encoded hash of the message keys. The
 * keyHash is calculated conform section 9.6.3.3 of the DDSI spec
 * (v2.2).
 *
 * Keys are hashed in the order in which they appear in the
 * messageKeyList of the topic.
 *
 * NOTE:
 * The current implementations is a first attempt.
 * Initially, 16 bytes are allocated for the keys. If keys do not
 * fit in these 16-bytes a realloc is performed every time a key
 * is added. From a performance point of view it may be better
 * to initially allocate a larger memory for key storage in order
 * to prevent successive calls to realloc.
 */
static void
calculate_key_hash(
    v_group vgroup,
    v_message vmessage,
    c_octet keyHash[16])
{
    c_array messageKeyList;
    c_ulong i, nrOfKeys;
    c_type keyTypes[32];
    c_value keyValues[32];
    size_t size;
    c_ulong maxSize;
    c_bool fit = TRUE;
    char *keys;                   /* temporary key storage */
    size_t offset = 0;
    size_t length = 16;
    ut_md5_state_t md5st;

    keys = (char *)os_malloc(16);
    memset(keys,0x00,16);
    messageKeyList = v_topicMessageKeyList(v_groupTopic(vgroup));
    nrOfKeys = c_arraySize(messageKeyList);
    for (i=0; i<nrOfKeys; i++) {
        keyValues[i] = c_fieldValue(messageKeyList[i], vmessage);
        keyTypes[i] = c_fieldType(messageKeyList[i]);
        if (isKeyType(keyTypes[i])) {
            switch (c_baseObjectKind(keyTypes[i])) {
                case M_PRIMITIVE:
                case M_ENUMERATION:
                    /* Get the size of the key */
                    size = keyTypes[i]->size;
                    if (offset + size > length) {
                        /* There is not enough room in the keys list.
                         * Extend the keys list so that the new value fits */
                        length = offset + size;
                        keys = os_realloc(keys, length);
                    }
                    /* The spec requires the keys to be in BE, so
                     * make sure to swap when possible */
                    switch (size) {
                        case 2:
                            {
                                unsigned short x = d_swap2uToBE(*((const unsigned short *) &keyValues[i].is));
                                memcpy(keys+offset, &x, sizeof(x));
                                break;
                            }
                        case 4:
                            {
                                unsigned x = d_swap4uToBE(*((const unsigned *) &keyValues[i].is));
                                memcpy(keys+offset, &x, sizeof(x));
                                break;
                            }
                        case 8:
                            {
                                unsigned long long x = d_swap8uToBE(*((const unsigned long long *) &keyValues[i].is));
                                memcpy(keys+offset, &x, sizeof(x));
                                break;
                            }
                        default:
                            /* Size must be 1 byte, no need to swap */
                            assert(size==1);
                            memcpy(keys+offset, &keyValues[i].is, size);
                    }
                    /* Update the offset */
                     offset = offset + size;
                    break;
                case M_COLLECTION:
                    /* We now need to calculate the size of the collection.
                     * The only supported collection type is C_STRING.
                     * This is usually a 4-byte pointer reference, followed
                     * by the string itself
                     */
                    maxSize = c_collectionTypeMaxSize(keyTypes[i]);
                    if (maxSize == 0) {
                        /* This is an unbounded string, it will not fit by definition */
                        fit = FALSE;
                    }
                    /* Store the number of elements and their references */
                    size = strlen(keyValues[i].is.String) + 1;  /* including the '\0' */
                    if ((offset + 4 + size) > length) {
                        /* There is not enough room in the keys list.
                         * Extend the keys list so that the new value fits
                         */
                        length = offset + 4 + (maxSize == 0 ? size : maxSize);
                        keys = os_realloc(keys, length);
                        /* Add padding */
                        memset(keys+offset, 0x00, length-offset);
                    }
                    /* Copy the string length (in BE) and the string */
                    {
                        unsigned x = d_swap4uToBE(*((const unsigned *)&size));
                        memcpy(keys+offset, &x, sizeof(x));
                    }
                    memcpy(keys+offset+4, keyValues[i].is.String, size);
                    offset = offset + 4 + size;
                    break;
                default:
                    /* This should never be possible */
                    assert(FALSE);
             }
        }
    } /* for */
    if (fit && length <= 16) {
        memcpy(keyHash,keys,16);
    } else {
        /* MD5 hash required */
        ut_md5_init(&md5st);
        ut_md5_append(&md5st, (const unsigned char *) keys, (unsigned) length);
        ut_md5_finish(&md5st, keyHash);
    }
    os_free(keys);
    return;
}


/* When flushing the group, the kernel may return both v_messages and v_registrations. When
 * a v_registration is flushed this can indicate a register or an unregister which is
 * specified by the flushType. The flushed objects are stored in a temporary list which is
 * eventually used to align the samples to other nodes. When the function interpreting this list
 * encounters a v_registration it must be able to tell whether this was a register or an unregister.
 * That information is stored by using this struct.
 */
typedef struct flushedDataObject {
    c_object object;            /* v_message or v_registration */
    v_groupFlushType flushType; /* MESSAGE, REGISTRATION or UNREGISTRATION */
} flushedDataObject;



static struct _DDS_Duration_t
os_duration_to_DDS_Duration_t(
    os_duration d)
{
    struct _DDS_Duration_t d2;
    c_time t;

    t = c_timeFromDuration(d);
    d2.sec = t.seconds;
    d2.nanosec = t.nanoseconds;
    return d2;
}


/**
 * \brief Publish a bead.
 */
static void
writeBead(
    c_object object,    /* vmessage to write */
    c_voidp userData)
{
    v_message vmessage;
    d_historicalData historicalData = d_historicalData(userData);
    struct sd_cdrSerdata *serializedData = NULL;
    const void *blob;   /* *blob will contain the address of the blob */
    d_historicalDataRequest historicalDataRequest = historicalData->historicalDataRequest;

    vmessage = (v_message)object;
    historicalData->vmessage = vmessage;
    d_trace(D_TRACE_CLIENT_DURABILITY, "%s: BEAD msg %p state=%u time=%"PA_PRItime" wrgid=%u:%u:%u\n",
        OS_FUNCTION, (void *)vmessage, v_nodeState(vmessage), OS_TIMEW_PRINT(vmessage->writeTime),
        vmessage->writerGID.systemId, vmessage->writerGID.localId, vmessage->writerGID.serial);
    /* Serialize the userData associated with the vmessage.
     * The serialization format depends on what the client has requested.
     */
    if (v_stateTest(v_nodeState(vmessage), L_UNREGISTER) && (!historicalDataRequest->include_payload_unregistrations)) {
        d_trace(D_TRACE_CLIENT_DURABILITY, "%s: BEAD msg %p beadsize set to 0\n", OS_FUNCTION, (void *)vmessage);
        historicalData->beadSize = 0;
        blob = NULL;
    } else {
        switch (historicalDataRequest->serializationFormat) {
            case PAYLOAD_SERIALIZATION_FORMAT_CDR_ANY:
                serializedData = sd_cdrSerialize(historicalData->cdrInfo, vmessage + 1);
                break;
            case PAYLOAD_SERIALIZATION_FORMAT_CDR_BE:
                serializedData = sd_cdrSerializeBE(historicalData->cdrInfo, vmessage + 1);
                break;
            case PAYLOAD_SERIALIZATION_FORMAT_CDR_LE:
                serializedData = sd_cdrSerializeLE(historicalData->cdrInfo, vmessage + 1);
                break;
            default:
                /* This should never happen, because if the serializationFormat is
                 * invalid we will never reach writeBead
                 */
                assert(FALSE);
                return;
        }
        historicalData->beadSize = sd_cdrSerdataBlob(&blob, serializedData);
    }
    /* Set the counters */
    historicalData->count++;
    if ((v_stateTest(v_nodeState(vmessage), L_WRITE)) &&
        (v_stateTest(v_nodeState(vmessage), L_DISPOSED))) {
        historicalData->writeDisposeCount++;
        historicalData->messageKind = 3;                            /* MESSAGE_FLAG_WRITE_DISPOSE */
    } else if(v_stateTest(v_nodeState(vmessage), L_WRITE)) {
        historicalData->writeCount++;
        historicalData->messageKind = 0;                            /* MESSAGE_FLAG_WRITE */
    } else if(v_stateTest(v_nodeState(vmessage), L_DISPOSED)) {
        historicalData->disposeCount++;
        historicalData->messageKind = 1;                            /* MESSAGE_FLAG_DISPOSE */
    } else if(v_stateTest(v_nodeState(vmessage), L_REGISTER)) {
        historicalData->registerCount++;
        historicalData->messageKind = 4;                            /* MESSAGE_FLAG_REGISTER */
    } else if(v_stateTest(v_nodeState(vmessage), L_UNREGISTER)) {
        historicalData->unregisterCount++;
        historicalData->messageKind = 2;                            /* MESSAGE_FLAG_UNREGISTER */
    }
    historicalData->totalSize += historicalData->beadSize;
    /* Set the content */
    historicalData->content._d = HISTORICAL_DATA_KIND_BEAD;
    historicalData->content._u.bead.sourceTimestamp.sec = (c_long)OS_TIMEW_GET_SECONDS(vmessage->writeTime);
    historicalData->content._u.bead.sourceTimestamp.nanosec = (c_ulong)OS_TIMEW_GET_NANOSECONDS(vmessage->writeTime);
    /* writerId */
    memset(historicalData->content._u.bead.writerId, 0x00, 16);
    memcpy(historicalData->content._u.bead.writerId, &vmessage->writerGID, 12);
    /* sequence number */
    historicalData->content._u.bead.sequenceNumber = vmessage->sequenceNumber;
    /* Qos settings */
    historicalData->content._u.bead.qos.reliabilityKind = (enum _DDS_ReliabilityQosPolicyKind) v_messageQos_reliabilityKind(vmessage->qos);
    historicalData->content._u.bead.qos.ownership.kind = (enum _DDS_OwnershipQosPolicyKind) v_messageQos_ownershipKind(vmessage->qos);
    historicalData->content._u.bead.qos.destinationOrderKind = (enum _DDS_DestinationOrderQosPolicyKind) v_messageQos_orderbyKind(vmessage->qos);
    historicalData->content._u.bead.qos.writerLifecycleAutoDisposeUnregisteredInstances = v_messageQos_isAutoDispose(vmessage->qos);
    historicalData->content._u.bead.qos.durabilityKind = (enum _DDS_DurabilityQosPolicyKind) v_messageQos_durabilityKind(vmessage->qos);
    historicalData->content._u.bead.qos.presentation.access_scope = (enum _DDS_PresentationQosPolicyAccessScopeKind) v_messageQos_presentationKind(vmessage->qos);
    historicalData->content._u.bead.qos.presentation.coherent_access = v_messageQos_isCoherentAccess(vmessage->qos);
    historicalData->content._u.bead.qos.presentation.ordered_access = v_messageQos_isOrderedAccess(vmessage->qos);
    historicalData->content._u.bead.qos.liveliness.kind = (enum _DDS_LivelinessQosPolicyKind) v_messageQos_livelinessKind(vmessage->qos);
    historicalData->content._u.bead.qos.latencyBudget.duration = os_duration_to_DDS_Duration_t(v_messageQos_getLatencyPeriod(vmessage->qos));
    historicalData->content._u.bead.qos.deadline.period = os_duration_to_DDS_Duration_t(v_messageQos_getDeadlinePeriod(vmessage->qos));
    historicalData->content._u.bead.qos.liveliness.lease_duration = os_duration_to_DDS_Duration_t(v_messageQos_getLivelinessPeriod(vmessage->qos));
    historicalData->content._u.bead.qos.lifespan.duration = os_duration_to_DDS_Duration_t(v_messageQos_getLifespanPeriod(vmessage->qos));
    historicalData->content._u.bead.qos.transportPriority.value = (c_long) v_messageQos_getTransportPriority(vmessage->qos);
    /* kind */
    historicalData->content._u.bead.kind = historicalData->messageKind;
    /* partitions; just use the partition of the group */
    historicalData->partitions = c_iterNew(historicalData->group->partition);
    /* keyHash */
    calculate_key_hash(historicalData->vgroup, historicalData->vmessage, historicalData->content._u.bead.keyHash);
    /* serializationFormat, same as in request except when request
     * has PAYLOAD_SERIALIZATION_FORMAT_CDR_ANY. In that case use the
     * natively supported serialization format.
     */
    historicalData->content._u.bead.serializationFormat = historicalDataRequest->serializationFormat;
    if (historicalData->content._u.bead.serializationFormat == PAYLOAD_SERIALIZATION_FORMAT_CDR_ANY) {
        historicalData->content._u.bead.serializationFormat = (pa_getEndianNess() == pa_endianBig) ? PAYLOAD_SERIALIZATION_FORMAT_CDR_BE : PAYLOAD_SERIALIZATION_FORMAT_CDR_LE;
    }
    /* payload */
    if (historicalData->beadSize > 0) {
        assert(blob != NULL);
        historicalData->blob = os_malloc(historicalData->beadSize);
        memcpy(historicalData->blob, blob, historicalData->beadSize);
    } else {
        historicalData->blob = NULL;
    }
    /* Publish the bead */
    d_historicalDataWrite((c_voidp)historicalData);
    /* Cleanup resources */
    os_free(historicalData->blob);
    historicalData->content._u.bead.payload = NULL;
    if (serializedData) {
        sd_cdrSerdataFree(serializedData);
    }
    return;
}


static void
writeLink(
    c_voidp userData)
{
    d_historicalData historicalData = d_historicalData(userData);

    historicalData->content._d = HISTORICAL_DATA_KIND_LINK;
    if (historicalData->errorCode == D_DDS_RETCDE_NO_ERROR) {
        /* No error */
        historicalData->content._u.link.sampleCount = historicalData->count;
        historicalData->content._u.link.completeness = historicalData->completeness;
    } else {
        /* Error */
        historicalData->content._u.link.sampleCount = 0;
        historicalData->content._u.link.completeness = 0;
    }
    historicalData->content._u.link.errorCode = historicalData->errorCode;
    /* Write the LINK */
    d_historicalDataWrite((c_voidp)historicalData);
    d_printTimedEvent(historicalData->durability, D_LEVEL_FINE,
                    "WRITE: %d, DISPOSED: %d, WRITE_DISPOSED: %d, REGISTER: %d, UNREGISTER: %d (SKIPPED: %d) with completeness %d and errorCode %d\n",
                    historicalData->writeCount, historicalData->disposeCount, historicalData->writeDisposeCount,
                    historicalData->registerCount, historicalData->unregisterCount, historicalData->skipCount,
                    historicalData->content._u.link.completeness,
                    historicalData->content._u.link.errorCode);
    return;
}


static c_bool
collectSamples(
    c_object object,
    v_groupInstance instance,
    v_groupFlushType flushType,
    c_voidp userData)
{
    d_historicalData historicalData = d_historicalData(userData);
    d_historicalDataRequest historicalDataRequest = historicalData->historicalDataRequest;
    v_message message;
    v_registration registration;
    c_bool process;
    v_group vgroup;
    vgroup = instance->group;
    process = TRUE;

    switch (flushType) {
    case V_GROUP_FLUSH_REGISTRATION:
        registration = (v_registration)object;
        if ( (!OS_TIMEW_ISINVALID(historicalDataRequest->endTime)) &&
             (os_timeWCompare(registration->writeTime, historicalDataRequest->endTime) == OS_MORE) ) {
            /* Don't process if produced after the specified endTime */
            d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST,
                        "V_GROUP_FLUSH_REGISTRATION writeTime (%"PA_PRItime") later than specified endTime (%"PA_PRItime"), discarding\n",
                         OS_TIMEW_PRINT(registration->writeTime), OS_TIMEW_PRINT(historicalDataRequest->endTime));
            process = FALSE;
        } else if (strcmp(v_groupName(vgroup), "Group<__BUILT-IN PARTITION__,DCPSTopic>") == 0) {
            /* Don't process registrations for DCPSTopic as this would introduce a scalability issue since
             * there is always an alive writer per federation. If these registrations would be aligned there
             * would be n registrations stored per builtin topic instance where n is the number of
             * federations in a system.
             * The only condition for aligning a registration is when there are multiple writers
             * for the same instance. Therefore this is not an issue for other (builtin) topics.
             */
            process = FALSE;
        } else if ( (!OS_TIMEW_ISINVALID(historicalDataRequest->startTime)) &&
                    (os_timeWCompare(registration->writeTime, historicalDataRequest->startTime) == OS_LESS) ) {
            /* Don't process if produced before the specified startTime */
            d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST,
                        "V_GROUP_FLUSH_REGISTRATION writeTime (%"PA_PRItime") prior to specified startTime (%"PA_PRItime"), discarding\n",
                         OS_TIMEW_PRINT(registration->writeTime), OS_TIMEW_PRINT(historicalDataRequest->startTime));
            process = FALSE;
        }
        break;
    case V_GROUP_FLUSH_UNREGISTRATION:
        registration = (v_registration)object;
        if ( (!OS_TIMEW_ISINVALID(historicalDataRequest->endTime)) &&
             (os_timeWCompare(registration->writeTime, historicalDataRequest->endTime) == OS_MORE) ) {
            /* Don't process if produced after the specified endTime */
            d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST,
                        "V_GROUP_FLUSH_UNREGISTRATION writeTime (%"PA_PRItime") later than specified endTime (%"PA_PRItime"), discarding\n",
                         OS_TIMEW_PRINT(registration->writeTime), OS_TIMEW_PRINT(historicalDataRequest->endTime));
             process = FALSE;
        } else if ( (!OS_TIMEW_ISINVALID(historicalDataRequest->startTime)) &&
                    (os_timeWCompare(registration->writeTime, historicalDataRequest->startTime) == OS_LESS) ) {
             /* Don't process if produced before the specified startTime */
            d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST,
                        "V_GROUP_FLUSH_UNREGISTRATION writeTime (%"PA_PRItime") prior to specified startTime (%"PA_PRItime"), discarding\n",
                         OS_TIMEW_PRINT(registration->writeTime), OS_TIMEW_PRINT(historicalDataRequest->startTime));
            process = FALSE;
        }
        break;
    case V_GROUP_FLUSH_MESSAGE:
        message = (v_message)object;
        if ( (!OS_TIMEW_ISINVALID(historicalDataRequest->endTime)) &&
             (os_timeWCompare(message->writeTime, historicalDataRequest->endTime) == OS_MORE) ) {
            /* Don't process if writeTime exceeds specified endTime */
            d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST,
                        "V_GROUP_FLUSH_MESSAGE writeTime (%"PA_PRItime") later than specified endTime (%"PA_PRItime"), discarding\n",
                         OS_TIMEW_PRINT(message->writeTime), OS_TIMEW_PRINT(historicalDataRequest->endTime));
            process = FALSE;
        } else if ( (!OS_TIMEW_ISINVALID(historicalDataRequest->startTime)) &&
                    (os_timeWCompare(message->writeTime, historicalDataRequest->startTime) == OS_LESS) ) {
            /* Don't process if produced before the specified startTime */
            d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST,
                        "V_GROUP_FLUSH_MESSAGE writeTime (%"PA_PRItime") prior to specified startTime (%"PA_PRItime"), discarding\n",
                         OS_TIMEW_PRINT(message->writeTime), OS_TIMEW_PRINT(historicalDataRequest->startTime));
            process = FALSE;
        }
        break;
    default:
        process = FALSE;
        OS_REPORT(OS_ERROR, "durability::d_historicalDataRequestListener::collectSamples", 0,
                  "Internal error (received unknown message type)");
        break;
    }
    if (process == TRUE) {
        /* Collect the sample data*/
        flushedDataObject *objectData = os_malloc(sizeof(struct flushedDataObject));
        objectData->object = c_keep(object);
        objectData->flushType = flushType;
        historicalData->list = c_iterAppend(historicalData->list, objectData);
        historicalData->instances = c_iterAppend(historicalData->instances, c_keep(instance));
    } else {
        /* No need to collect */
        historicalData->skipCount++;
    }
    return FALSE;
}


/**
 * Set the completeness of the historicalData based on the completeness on the group.
 *
 * The historicalData->completeness represents the aggregated completeness of multiple groups.
 *
 * The aggregated completeness will be the weakest, .i.e,
 *   if one of the groups is D_DDS_COMPLETENESS_UNKNOWN then the aggregated group will be D_DDS_COMPLETENESS_UNKNOWN.
 *   if one of the groups is D_DDS_INCOMPLETENESS then the aggregated group will be D_DDS_INCOMPLETENESS
 *   if all groups are D_DDS_INCOMPLETE hen the aggregated group will be D_DDS_INCOMPLETE
 */
static void
setCompleteness(
    d_historicalData historicalData,
    _DDS_Completeness_t completeness)
{
    if (historicalData->completeness > completeness) {
        historicalData->completeness = completeness;
    }
    return;
}



/**
 * Send a historicalData message for the specified group.
 *
 * The historicalData to sent is the response to the historicalDataRequest.
 **/
static c_bool
sendHistoricalData(
    d_group group,
    c_voidp userData)
{
    v_message vmessage;
    v_registration registration;
    v_groupInstance instance;
    c_bool proceed;

    d_historicalData historicalData = d_historicalData(userData);
    flushedDataObject *objectData;
    d_thread self = d_threadLookupSelf();
    v_resourcePolicyI resourceLimits;
    v_historicalDataRequest vrequest;
    d_historicalDataRequest historicalDataRequest;

    assert(d_groupIsValid(group));
    assert(d_historicalDataIsValid(historicalData));

    historicalDataRequest = historicalData->historicalDataRequest;
    historicalData->list = c_iterNew(NULL);
    historicalData->instances = c_iterNew(NULL);
    historicalData->group = d_group(d_objectKeep(d_object(group)));
    historicalData->vgroup = d_groupGetKernelGroup(group);
    if (historicalData->vgroup) {
        d_tracegroup(d_threadsDurability(), historicalData->vgroup, OS_FUNCTION);
        /* Use the specified serializer to serialize the data.
         * Currently, the only supported serializer is cdr.
         * First serialize and compile the topic type, and
         * then serialize the bead data. This is only needed
         * for the first time for a topic.
         */
        /* Currently the only defined serializer is cdr */
        if ((historicalData->cdrInfo = sd_cdrInfoNew(v_topicDataType(historicalData->vgroup->topic))) == NULL) {
            goto err_allocCdrInfo;
        }
        if (sd_cdrCompile(historicalData->cdrInfo) != 0) {
            goto err_cdrCompile;
        }
        /* Set the resourceLimits */
        resourceLimits.v.max_samples = historicalDataRequest->maxSamples;
        resourceLimits.v.max_instances = historicalDataRequest->maxInstances;
        resourceLimits.v.max_samples_per_instance = historicalDataRequest->maxSamplesPerInstance;
        /* Retrieve historical data for this group */
        vrequest = v_historicalDataRequestNew(
                    v_objectKernel(historicalData->vgroup),
                    historicalDataRequest->sqlFilter,
                    (const c_char**)historicalDataRequest->sqlFilterParams,
                    historicalDataRequest->sqlFilterParamsSize,
                    historicalDataRequest->startTime,
                    historicalDataRequest->endTime,
                    &resourceLimits,
                    OS_DURATION_ZERO);
        (void)d_shmAllocAssert(vrequest, "Allocation of v_historicalDataRequestNew failed.");
        v_groupFlushActionWithCondition(historicalData->vgroup, vrequest, collectSamples, historicalData);
        c_free(vrequest);
    } else {
        d_printTimedEvent(historicalData->durability, D_LEVEL_FINEST,
                    "No kernel group for topic '%s.%s' found (yet), so unable to retrieve historical data for this group\n",
                     historicalData->group->partition, historicalData->group->topic);
    }
    /* Send the beads */
    proceed = TRUE;
    objectData = c_iterTakeFirst(historicalData->list);
    while (objectData && proceed) {
        assert(historicalData->vgroup); /* for the benefit of Clang's static analyzer */
        d_threadAwake(self);
        instance = c_iterTakeFirst(historicalData->instances);
        assert(instance);
        if (c_instanceOf(objectData->object, "v_registration")) {
            /* Registration or unregistration */
            registration = (v_registration)objectData->object;
            vmessage = v_groupInstanceCreateMessage(instance);
            if (vmessage) {
                vmessage->writerGID = registration->writerGID;
                vmessage->qos = c_keep(registration->qos);
                vmessage->writeTime = registration->writeTime;
                vmessage->sequenceNumber = registration->sequenceNumber;
                if (objectData->flushType == V_GROUP_FLUSH_REGISTRATION) {
                    v_stateSet(v_nodeState(vmessage), L_REGISTER);
                } else if(objectData->flushType == V_GROUP_FLUSH_UNREGISTRATION) {
                    v_stateSet(v_nodeState(vmessage), L_UNREGISTER);
                }
                if (v_stateTest(registration->state, L_IMPLICIT) == TRUE) {
                    v_stateSet(v_nodeState(vmessage), L_IMPLICIT);
                }
                writeBead(vmessage, historicalData);
                c_free(vmessage);
            } else {
                proceed = d_shmAllocAssert(vmessage, "Allocation of sample failed.");
            }
        } else {
            /* Normal data */
            vmessage = v_message(objectData->object);
            assert(vmessage);
            if (v_stateTest(v_nodeState(vmessage), L_WRITE) ||
                    c_getType(vmessage) != v_kernelType(v_objectKernel(instance), K_MESSAGE)) {
                writeBead(vmessage, historicalData);
            } else {
                /* If the message is a mini-message without keys, temporarily replace it with
                 * a typed message that does include the keys. That way the bead becomes self-
                 * describing and so the receiving node can deduct its instance again.
                 */
                vmessage = v_groupInstanceCreateTypedInvalidMessage(instance, vmessage);
                if (vmessage) {
                    writeBead(vmessage, historicalData);
                    c_free(vmessage);
                } else {
                    proceed = d_shmAllocAssert(vmessage, "Allocation of sample failed.");
                }
            }
        }
        c_free(instance);
        c_free(objectData->object);
        os_free(objectData);
        objectData = c_iterTakeFirst(historicalData->list);
    } /* while */
    /* Update the completeness state depending on the completeness state of the group */
    setCompleteness(historicalData, d_mapCompleteness(d_groupGetCompleteness(historicalData->group)));
    /* clear objectData */
    objectData = c_iterTakeFirst(historicalData->list);
    while (objectData) {
        c_free(objectData->object);
        os_free(objectData);
        objectData = c_iterTakeFirst(historicalData->list);
    }
    c_iterFree(historicalData->list);

    instance = c_iterTakeFirst(historicalData->instances);
    while (instance) {
        c_free(instance);
        instance = c_iterTakeFirst(historicalData->instances);
    }
    c_iterFree(historicalData->instances);

    if (historicalData->cdrInfo != NULL) {
        sd_cdrInfoFree(historicalData->cdrInfo);
    }
    c_free(historicalData->vgroup);
    d_groupFree(historicalData->group);
    return TRUE;

err_cdrCompile:
    sd_cdrInfoFree(historicalData->cdrInfo);
err_allocCdrInfo:
    return TRUE;
}

/* Decides whether the server can answer the request
 *
 * Returns TRUE if the server can answer the request, FALSE if it is rescheduled
 */
static c_bool
can_answer(d_historicalDataRequest historicalDataRequest)
{
    d_durability durability = d_threadsDurability();
    d_admin admin = durability->admin;
    d_configuration config = d_durabilityGetConfiguration(durability);
    c_bool answer_now = TRUE;
    os_duration readerDetectionPeriod = OS_DURATION_INIT(30,0);  /* Use 30 seconds to detect the historicalDataReader */

    if ((config->waitForRemoteReaders) && (historicalDataRequest->errorCode == D_DDS_RETCDE_NO_ERROR)) {

        c_iterIter iter= c_iterIterGet(historicalDataRequest->requestIds);
        c_bool allReadersDetected = TRUE;
        d_client client;
        struct _DDS_RequestId_t *requestId;

        /* Walk over all requestIds and lookup the corresponding clients.
         * As soon as all of them have a discovered reader we will answer the request.
         * In case there are one or more reader for which the reader has not yet been
         * detected we are going to postpone the answer until all readers have been
         * detected. When the reader for one of more clients have not been detected
         * after 30 sec an error link will be send using error code D_DDS_RETCDE_READER_NOT_KNOWN
         */
        while (((requestId = (struct _DDS_RequestId_t *)c_iterNext(&iter)) != NULL) && (allReadersDetected)) {
            if ((client = d_adminFindClientByClientId(admin, requestId->clientId)) != NULL) {
                allReadersDetected = (((client->readers) & (D_HISTORICALDATA_READER_FLAG)) == (D_HISTORICALDATA_READER_FLAG));
            }
            /* If the client could not be found anymore assume all readers have detected
             * to prevent rescheduling of the request */
        }
        if ((c_iterLength(historicalDataRequest->requestIds) == 0)) {
            /* No clients anymore, send back D_DDS_RETCDE_NO_CLIENTS */
            d_printTimedEvent(durability, D_LEVEL_FINEST, d_threadSelfName(),
                 "No clients found for historicalDataRequest %p\n", (void*)historicalDataRequest);
            historicalDataRequest->errorCode = D_DDS_RETCDE_NO_CLIENTS;
            answer_now = TRUE;
        } else if (allReadersDetected) {
            /* Ready to send the answer */
            answer_now = TRUE;
        } else {
            /* Not yet all readers detected */
            os_duration diff = os_timeEDiff(os_timeEGet(), historicalDataRequest->receptionTime);
            if (os_durationCompare(diff, readerDetectionPeriod) == OS_MORE) {

                d_printTimedEvent(durability, D_LEVEL_FINEST, d_threadSelfName(),
                     "Unable to detect the client's historicalData reader within %"PA_PRIduration" seconds\n",
                     OS_DURATION_PRINT(readerDetectionPeriod));

                historicalDataRequest->errorCode = D_DDS_RETCDE_READER_NOT_KNOWN;
                answer_now = TRUE;
            } else {
                answer_now = FALSE;
            }
        }
    }
    return answer_now;
}


static c_bool
d_historicalDataRequestListenerAnswer(
    d_historicalDataRequestListener listener,
    d_historicalDataRequest historicalDataRequest)
{
    d_admin admin;
    d_durability durability;
    struct collectMatchingGroupsHelper helper;
    d_historicalData historicalData;

    assert(d_historicalDataRequestListenerIsValid(listener));
    assert(d_historicalDataRequestIsValid(historicalDataRequest));

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    d_printTimedEvent(durability, D_LEVEL_FINER,
         "Going to answer %d historicalDataRequest(s)\n", c_iterLength(historicalDataRequest->requestIds));
    /* Create an historicalData to answer the historicalDataRequest */
    historicalData = d_historicalDataNew(admin, historicalDataRequest);
    if (historicalData->errorCode == D_DDS_RETCDE_NO_ERROR) {
        /* no error, retrieve matching groups */
        helper.admin = admin;
        helper.topic = historicalDataRequest->topic;
        helper.partitions = historicalDataRequest->partitions;
        if ((helper.matchingGroups = d_tableNew(d_groupCompare, d_groupFree)) == NULL) {
            goto err_allocMatchingGroups;
        }
        helper.forMe = historicalDataRequest->forMe;
        helper.forEverybody = historicalDataRequest->forEverybody;
        helper.isAligner = FALSE;
        helper.isResponsible = FALSE;
        helper.masterKnown = FALSE;
        helper.groupFound = FALSE;
        helper.isHistoricalDataRequest = TRUE;
        d_printTimedEvent(durability, D_LEVEL_FINEST,
              "Collecting groups that match historicalDataRequest\n");
        /* Collect all matching groups.
         * As a side effect, also set isAligner and isReponsible
         */
        d_adminCollectMatchingGroups(admin, &helper);
        d_printTimedEvent(durability, D_LEVEL_FINER,
              "Total %d groups matched historicalDataRequest, publish the response to partition '%s'\n",
              d_tableSize(helper.matchingGroups),historicalData->alignmentPartition);
        if (d_tableSize(helper.matchingGroups) > 0) {
            /* Send historicalData for each matching group. */
            (void)d_tableWalk(helper.matchingGroups, sendHistoricalData, historicalData);
            writeLink(historicalData);
        } else {
            /* No matching group found.
             * This can be because the server is not configured as aligner,
             * or if the server is not responsible because it is not the master,
             * or simply if the server does not know about the group
             * Set the proper error code in each case. */
            if (!helper.isAligner) {
                d_printTimedEvent(durability, D_LEVEL_FINEST, "  No aligner\n");
                historicalData->errorCode = D_DDS_RETCDE_SERVER_IS_NOT_ALIGNER;   /* Server is not configured to be an aligner for the requested dataset */
            } else if (!helper.masterKnown) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,  "  No master selected\n");
                historicalData->errorCode = D_DDS_RETCDE_NO_MASTER_SELECTED;  /* A master has not yet been determined */
            } else if (!helper.groupFound) {
                d_printTimedEvent(durability, D_LEVEL_FINEST, "  No matching groups found\n");
                historicalData->errorCode = D_DDS_RETCDE_GROUP_NOT_FOUND ;   /* Server is not configured to be an aligner for the requested dataset */
            } else if (!helper.isResponsible) {
                d_printTimedEvent(durability, D_LEVEL_FINEST, "  Not responsible\n");
                historicalData->errorCode = D_DDS_RETCDE_SERVER_IS_NOT_RESPONSIBLE;   /* Request will be handled by a different durability service */
            }
            /* TODO:
             * if the request contains a wildcard and there no groups found, then the link should be complete with 0 samples.
             * if the request does not contains a wildcard and there is no matching group then the link should indicate GROUP_UNKNOWN with 0 samples.
             */
            writeLink(historicalData);
        }
        d_tableFree(helper.matchingGroups);
    } else {
        /* An error occurred, write a link containing the error */
        writeLink(historicalData);
    }
    d_historicalDataFree(historicalData);
    return FALSE;  /* Do not reschedule */

err_allocMatchingGroups:
    /* Clean up the request and the response*/
    d_historicalDataFree(historicalData);
    d_historicalDataRequestFree(historicalDataRequest);
    return FALSE;
}


/* Implementation of the callback to answer a request
 *
 * Returns TRUE if request is rescheduled, FALSE when it is handled*/
c_bool
answer_request(
    void *listener,
    void *arg)
{
    d_historicalDataRequestListener historicalDataRequestListener = d_historicalDataRequestListener(listener);
    d_historicalDataRequest historicalDataRequest = d_historicalDataRequest(arg);

    assert(d_historicalDataRequestListenerIsValid(listener));
    assert(d_historicalDataRequestIsValid(historicalDataRequest));

    /* Check whether to reschedule or to answer the request */
    if (can_answer(historicalDataRequest)) {
        (void)d_historicalDataRequestListenerAnswer(historicalDataRequestListener, historicalDataRequest);
        return FALSE;
    } else {
        /* Reschedule request over 0.5 sec */
        (void)reschedule_request(historicalDataRequestListener, historicalDataRequest, OS_DURATION_INIT(0,500000000));
        return TRUE;
    }
}


/* Returns:
 *  - historicalDataRequest, if historicalDataRequest must be answered now
 *  - NULL, if no historicalDataRequest must be answered now, wait_time contains the time to wait (either finite time or infinite)
 *
 * It is assumed that the caller locks the request queue.
 */
static struct request_admin_node *
get_request_unlocked(
    d_historicalDataRequestListener listener,
    os_duration *wait_time)
{
    os_timeE now;
    os_timeE expiration_time;
    struct request_admin_node *n = NULL;

    assert(d_historicalDataRequestListenerIsValid(listener));

    expiration_time = time_of_next_request_unlocked(listener);
    if (OS_TIMEE_ISINFINITE(expiration_time)) {
        *wait_time = OS_DURATION_INFINITE;
        return NULL;
    }
    now = os_timeEGet();
    *wait_time = os_timeEDiff(expiration_time, now);
    if (os_durationCompare(*wait_time, ((os_duration) 0x0)) != OS_MORE) {
        if ((n = take_next_request_unlocked(listener)) != NULL) {
            n->handle_time = now;
        }
    }
    return n;
}

static void cleanup_node(
    struct request_admin_node *n)
{
    assert(n);

    /* Only request must be cleaned up.
     * PubInfo nodes are already scheduled for destruction
     */
    if (n->callback == answer_request) {
        /* cleanup the request */
        cleanup_request(n);
    } else if (n->callback == delete_pubinfo) {
        cleanup_pubinfo(n);
    }
}


static void *
d_historicalDataRequestListenerHandlerThread(
    void* arg)
{
    d_thread self = d_threadLookupSelf ();
    d_historicalDataRequestListener listener;
    os_duration wait_time;
    struct request_admin_node *n = NULL;
    os_result osr;
    c_bool rescheduled = FALSE;

    listener = d_historicalDataRequestListener(arg);

    os_mutexLock(&listener->queue_lock);
    while (listener->terminate == FALSE) {
        /* Take the first element from the queue, but only if it has expired */
        n = get_request_unlocked(listener, &wait_time);
        while ((n) && (listener->terminate == FALSE)) {
            os_mutexUnlock(&listener->queue_lock);
            /* execute the callback */
            if (n->callback) {
                d_trace(D_TRACE_PRIO_QUEUE, "%s: handle avlnode %p, insert_time=%"PA_PRItime", expiration_time = %"PA_PRItime"\n",
                    OS_FUNCTION, (void *)n, OS_TIMEE_PRINT(n->insert_time), OS_TIMEE_PRINT(n->expiration_time));
                rescheduled = n->callback(listener, n->arg);
            }
            if (rescheduled) {
                os_free(n);
            } else {
                cleanup_node(n);
            }
            /* Get the next request */
            os_mutexLock(&listener->queue_lock);
            n = get_request_unlocked(listener, &wait_time);
        }
        /* Wait until wait_time expires or the condition is triggered */
        if (listener->terminate == FALSE) {
            if (OS_DURATION_ISINFINITE(wait_time)) {
                d_condWait(self, &listener->cond, &listener->queue_lock);
            } else if ((osr = d_condTimedWait(self, &listener->cond, &listener->queue_lock, wait_time)) == os_resultFail) {
                OS_REPORT(OS_CRITICAL, D_CONTEXT_DURABILITY, 0, "d_condTimedWait failed; terminating");
                /* terminate durability */
                d_durabilityTerminate(d_threadsDurability(), TRUE);
            }
        }
    } /* while */
    os_mutexUnlock(&listener->queue_lock);
    /* In case the thread is about to terminate it might be that n was still taken */
    if (n) {
        cleanup_node(n);
    }
    return NULL;
}


/**
 * \brief Add a historicalDataRequest to the list of pending requests.
 */
static void
d_historicalDataRequestListenerAddRequest(
    d_historicalDataRequestListener listener,
    d_historicalDataRequest historicalDataRequest)
{
    assert(d_historicalDataRequestListenerIsValid(listener));
    assert(d_historicalDataRequestIsValid(historicalDataRequest));

    /* Insert the request in priority queue and possibly signal the handler thread */
    (void)insert_request(listener, historicalDataRequest);
    return;
}

static
u_actionResult
processHistoricalDataRequest(
    c_object o,
    c_voidp userData /* listener * */)
{
    d_historicalDataRequestListener listener;
    struct _DDS_HistoricalDataRequest *request;
    d_historicalDataRequest historicalDataRequest;
    v_dataReaderSample s;
    v_actionResult result = 0;
    v_message message;
    d_admin admin;
    d_durability durability;
    d_client client, duplicate;
    d_networkAddress clientAddr;

    listener = d_historicalDataRequestListener(userData);
    s = v_dataReaderSample(o);
    if (s != NULL) {
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        v_actionResultSet(result, V_PROCEED);
        if (v_dataReaderSampleStateTest(s, L_VALIDDATA) && !(v_dataReaderSampleInstanceStateTest(s, L_DISPOSED))) {
            message = v_dataReaderSampleTemplate(s)->message;

            request = (struct _DDS_HistoricalDataRequest *)C_DISPLACE(message, C_MAXALIGNSIZE(sizeof(*message)));
            /* create a d_historicalDataRequest object to store the request */
            if ((historicalDataRequest = d_historicalDataRequestNew(admin, request)) == NULL) {
                goto alloc_historicalDataRequest;
            }
            /* Get or create client and store the requestId.clientId */
            clientAddr = d_networkAddressNew(message->writerGID.systemId, 0, 0);
            client = d_clientNew(clientAddr);
            if ((duplicate = d_adminAddClient(admin, client)) != client) {
                d_clientFree(client);
                client = duplicate;
            }
            d_clientSetClientId(client, request->requestId.clientId);
            d_printTimedEvent(durability, D_LEVEL_FINE,
                 "Received historicalDataRequest %lu for topic '%s' from client %lld.%lld at federation %u\n",
                 request->requestId.requestId,
                 request->topic ? request->topic : "NULL",
                 request->requestId.clientId.prefix,
                 request->requestId.clientId.suffix,
                 clientAddr->systemId);
            /* Check if the request is for me or for everybody*/
            if ((!historicalDataRequest->forMe) && (!historicalDataRequest->forEverybody)) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                     "historicalDataRequest %lu for topic '%s' from client %lld.%lld at federation %u is not meant for me, ignoring this request\n",
                     request->requestId.requestId,
                     request->requestId.clientId.prefix,
                     request->requestId.clientId.suffix,
                     clientAddr->systemId);
                goto skip_notMeantForMe;
            }
            /* The request is meant for me so I must answer, even if the request contains errors. */
            historicalDataRequest->errorCode = d_historicalDataRequestSanityCheck(historicalDataRequest);
            /* Calculate the expirationTime based on the historicalData->timeout.
             * This must be a valid timeout, otherwise the sanity check has catched it
             */
            if (historicalDataRequest->errorCode == D_DDS_RETCDE_NO_ERROR) {
                if (os_durationCompare(historicalDataRequest->timeout, OS_DURATION_INFINITE) == OS_EQUAL) {
                    historicalDataRequest->expirationTime = OS_TIMEE_INFINITE;
                } else {
                    historicalDataRequest->expirationTime = os_timeEAdd(historicalDataRequest->receptionTime, historicalDataRequest->timeout);
                }
            } else {
                /* sanity error encountered, answer immediately */
                historicalDataRequest->expirationTime = OS_TIMEE_ZERO;
            }
            /* Schedule the answer to the request */
            d_historicalDataRequestListenerAddRequest(listener, historicalDataRequest);
            /* Free the stuff */
            d_networkAddressFree(clientAddr);
            d_clientFree(client);
        } else if (v_dataReaderSampleInstanceStateTest(s, L_DISPOSED)) {
            /* The request is disposed, so in this case we should remove pending requests. */
            d_printTimedEvent(durability, D_LEVEL_FINEST, "request is disposed\n");
        }
    }
    return result;

skip_notMeantForMe:
    d_historicalDataRequestFree(historicalDataRequest);
    d_networkAddressFree(clientAddr);
alloc_historicalDataRequest:
    return result;
}


static c_ulong
d_historicalDataRequestListenerAction(
    u_object o,            /* the observable that triggered the event */
    v_waitsetEvent event,  /* the event that was triggered */
    c_voidp userData       /* d_historicalDataRequestListener */)
{
    d_historicalDataRequestListener listener;
    u_result ur;
    d_admin admin;
    d_durability durability;
    d_thread self = d_threadLookupSelf();

    OS_UNUSED_ARG(o);

    listener = d_historicalDataRequestListener(userData);
    d_lockLock(d_lock(listener));
    d_threadAwake(self);
    /* Read HistoricalDataRequests and process them */
    ur = u_dataReaderTake(listener->dataReader, U_STATE_ANY, processHistoricalDataRequest, listener, OS_DURATION_ZERO);
    if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                "Failed to read data from historicalDataRequestListener (result: %s)", u_resultImage(ur));
        d_durabilityTerminate(durability, TRUE);
    }
    d_threadAsleep(self, ~0u);
    d_lockUnlock(d_lock(listener));
    return event->kind;
}


static void
d_historicalDataRequestListenerDeinit(
    d_historicalDataRequestListener listener)
{
    assert(d_historicalDataRequestListenerIsValid(listener));

    /* Stop the historicalDataRequestListener */
    d_historicalDataRequestListenerStop(listener);

    /* Destroyed the historicalDataRequestHandler thread */
    os_mutexLock(&listener->queue_lock);
    listener->terminate = TRUE;
    (void)os_condBroadcast(&listener->cond);
    os_mutexUnlock(&listener->queue_lock);
    if (listener->handlerThreadExists) {
        d_threadWaitExit(listener->handlerThread, NULL);
    }
    if (listener->waitsetData) {
        d_waitsetEntityFree(listener->waitsetData);
        listener->waitsetData = NULL;
    }
    if (listener->dataReader) {
        u_objectFree(u_object(listener->dataReader));
        listener->dataReader = NULL;
    }
    ut_avlFree(&listener->request_avltreedef, &listener->request_id_tree, cleanup_request);
    ut_avlFree(&listener->pubinfo_avltreedef, &listener->pubinfo_tree, cleanup_pubinfo);
    /* Destroy mutex and condition */
    (void)os_condDestroy(&listener->cond);
    (void)os_mutexDestroy(&listener->queue_lock);
    listener->subscriber = NULL;
    /* Call super-deinit */
    d_listenerDeinit(d_listener(listener));
}

/**
 * \brief Create an historicalDataRequestListener object
 *
 * The listener listens to historicalDataRequest messages that
 * are published on the durability partition. Requests that
 * originate from the same node as the historicalDataRequestListener
 * are discarded.
 */
d_historicalDataRequestListener
d_historicalDataRequestListenerNew(
    d_subscriber subscriber)
{
    d_historicalDataRequestListener listener;
    v_readerQos readerQos = NULL;
    os_threadAttr attr;
    os_threadAttr tattr;
    os_result osr;
    struct _DDS_Gid_t myServerId;
    c_value ps[2];

    /* Tree definition for tree that organises historicalDataRequests based on requestId and topic */
    ut_avlTreedef_t request_avltreedef = UT_AVL_TREEDEF_INITIALIZER_INDKEY (
                                            offsetof (struct request_admin_node, request_avlnode), /* node in tree containing pending requests */
                                            offsetof (struct request_admin_node, arg),             /* attribute used for comparision */
                                            compare_request_by_requestId,                          /* comparison function */
                                            0);                                                    /* flags */
    ut_avlTreedef_t pubinfo_avltreedef = UT_AVL_TREEDEF_INITIALIZER_INDKEY (
                                            offsetof (struct request_admin_node, pubinfo_avlnode), /* node in tree containing pending publishers */
                                            offsetof (struct request_admin_node, arg),             /* attribute used for comparision */
                                            compare_pubinfo_by_partition,                          /* comparison function */
                                            0);                                                    /* flags */

    assert(d_subscriberIsValid(subscriber));

    /* Allocate historicalDataRequestListener object */
    listener = d_historicalDataRequestListener(os_malloc(C_SIZEOF(d_historicalDataRequestListener)));
    /* Call super-init */
     d_listenerInit(d_listener(listener), D_HISTORICAL_DATA_REQ_LISTENER, subscriber, NULL,
                    (d_objectDeinitFunc)d_historicalDataRequestListenerDeinit);
    /* Initialize the historicalDataRequestListener */
    /* Initialize priority queue for requests */
    listener->request_avltreedef = request_avltreedef;
    listener->pubinfo_avltreedef = pubinfo_avltreedef;
    ut_avlInit (&listener->request_avltreedef, &listener->request_id_tree);
    ut_avlInit (&listener->pubinfo_avltreedef, &listener->pubinfo_tree);
    ut_fibheapInit (&prioqueue_fhdef, &listener->prioqueue);
    /* Create mutexes and condition */
    listener->terminate = FALSE;
    if ((osr = os_mutexInit(&listener->queue_lock, NULL)) != os_resultSuccess) {
        goto err_mutex_init;
    }
    if ((osr = os_condInit(&listener->cond, &listener->queue_lock, NULL)) != os_resultSuccess) {
        goto err_cond_init;
    }
    /* start historicalDataRequestHandlerThread */
    os_threadAttrInit(&tattr);
    if ((osr = d_threadCreate(&(listener->handlerThread), "historicalDataRequestHandler", &tattr, (void*(*)(void*))d_historicalDataRequestListenerHandlerThread, (void*)listener)) != os_resultSuccess) {
        listener->handlerThreadExists = FALSE;
        goto err_handler_thread;
    }
    listener->handlerThreadExists = TRUE;
    /* d_readerQosNew does not always return the proper settings.
     * The following settings ensure that the reader qos is correct
     */
    listener->subscriber = subscriber;
    if ((readerQos = d_readerQosNew(V_DURABILITY_VOLATILE, V_RELIABILITY_RELIABLE)) == NULL) {
        goto err_readerQos;
    }
    readerQos->history.v.kind = V_HISTORY_KEEPALL;
    readerQos->history.v.depth = V_LENGTH_UNLIMITED;
    readerQos->liveliness.v.lease_duration = OS_DURATION_INFINITE;
    readerQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
    /* Prevent that the historicalDataReader listens to requests from itself. */
    myServerId = d_durabilityGetMyServerId(d_adminGetDurability(subscriber->admin));
    ps[0].kind = V_LONGLONG;
    ps[0].is.LongLong = myServerId.prefix;
    ps[1].kind = V_LONGLONG;
    ps[1].is.LongLong = myServerId.suffix;
    listener->dataReader = u_subscriberCreateDataReader(
        subscriber->subscriber2,
        D_HISTORICAL_DATA_REQUEST_TOPIC_NAME "Reader",
        "select * from " D_HISTORICAL_DATA_REQUEST_TOPIC_NAME " where requestId.clientId.prefix <> %0 OR requestId.clientId.suffix <> %1",
        ps, 2,
        readerQos);
    d_readerQosFree(readerQos);
    if (listener->dataReader == NULL) {
        goto err_dataReader;
    }
    /* No need to obtain historical data because topic is volatile */
    if(u_entityEnable(u_entity(listener->dataReader)) != U_RESULT_OK) {
        goto err_dataReaderEnable;
    }
    os_threadAttrInit(&attr);
    listener->waitsetData = d_waitsetEntityNew(
                    "historicalDataRequestListener",
                    u_object(listener->dataReader),
                    d_historicalDataRequestListenerAction,
                    V_EVENT_DATA_AVAILABLE,
                    attr, listener);
    if (!listener->waitsetData) {
        goto err_waitsetData;
    }
    return listener;

err_waitsetData:
    /* No undo for u_entityEnable(...) */
err_dataReaderEnable:
err_dataReader:
err_readerQos:
err_handler_thread:
err_cond_init:
err_mutex_init:
    d_historicalDataRequestListenerFree(listener);
    return NULL;
}


void
d_historicalDataRequestListenerFree(
    d_historicalDataRequestListener listener)
{
    assert(d_historicalDataRequestListenerIsValid(listener));

    d_objectFree(d_object(listener));
}


static void
cleanup_request (void *n)
{
    struct request_admin_node *m = (struct request_admin_node *)n;

    d_trace(D_TRACE_PRIO_QUEUE, "%s: cleanup avlnode %p\n", OS_FUNCTION, (void *)n);
    d_historicalDataRequestFree(m->arg);
    os_free(m);
}


static void
cleanup_pubinfo (void *n)
{
    struct request_admin_node *m = (struct request_admin_node *)n;
    struct pubInfo *pubinfo = (struct pubInfo *)(m->arg);

    d_trace(D_TRACE_PRIO_QUEUE, "%s: cleanup avlnode %p\n", OS_FUNCTION, (void *)n);
    u_objectFree(u_object(pubinfo->writer));
    u_objectFree(u_object(pubinfo->publisher));
    os_free(pubinfo->partition);
    os_free(m);
}


static c_bool
d_historicalDataRequestListenerNotifyEvent(
    c_ulong event,
    d_fellow fellow,
    d_nameSpace ns,
    d_group group,
    c_voidp eventUserData,
    c_voidp userData)
{
    d_historicalDataRequestListener listener = d_historicalDataRequestListener(userData);

    assert(d_historicalDataRequestListenerIsValid(listener));

    OS_UNUSED_ARG(event);
    OS_UNUSED_ARG(fellow);
    OS_UNUSED_ARG(ns);
    OS_UNUSED_ARG(group);
    OS_UNUSED_ARG(eventUserData);
    OS_UNUSED_ARG(userData);
    OS_UNUSED_ARG(listener);

    switch (event) {
        case D_GROUP_LOCAL_COMPLETE :
#if 0
            printf("*** %s.%s D_GROUP_LOCAL_COMPLETE\n", d_groupGetPartition(group), d_groupGetTopic(group));
#endif
            /* TODO: move the pending historicalDataRequests with matching partition/topic to the front of the queue */
            break;
        case D_FELLOW_REMOVED :
#if 0
            printf("*** D_FELLOW_REMOVED\n");
#endif
            break;
        default : ;
#if 0
           printf("*** not triggered\n");
#endif
    }
    return TRUE;
}

c_bool
d_historicalDataRequestListenerStart(
    d_historicalDataRequestListener listener)
{
    c_bool result;
    d_waitset waitset;
    u_result ur;
    d_admin admin;
    result = FALSE;

    assert(d_historicalDataRequestListenerIsValid(listener));

    d_lockLock(d_lock(listener));
    if (d_listener(listener)->attached == FALSE) {
        waitset = d_subscriberGetWaitset(listener->subscriber);
        result = d_waitsetAttach(waitset, listener->waitsetData);
        admin = d_listenerGetAdmin(d_listener(listener));
        /* Create an eventListener that reacts to relevant events */
        listener->historicalDataRequestEventListener = d_eventListenerNew(D_GROUP_LOCAL_COMPLETE /* | D_READERS_DETECTED */ | D_FELLOW_REMOVED, d_historicalDataRequestListenerNotifyEvent, listener);
        d_adminAddListener(admin, listener->historicalDataRequestEventListener);

        if (result == TRUE) {
            /* A V_DATA_AVAILABLE event is only generated when new data
             * arrives. It is NOT generated when historical data is inserted.
             * In case this durability service receives historical
             * DCPSSubscriptions from another durability service that
             * was started earlier, all these DCPSSubscriptions are
             * inserted in the reader at creation time. To read these
             * we must explicitly trigger a take action.
             */
            ur = u_dataReaderTake(listener->dataReader, U_STATE_ANY, processHistoricalDataRequest, listener, OS_DURATION_ZERO);
            if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
                OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                        "Failed to read data from historicalDataRequestListenerReader (result: %s)", u_resultImage(ur));
            } else {
                d_listener(listener)->attached = TRUE;
                result = TRUE;
            }
        }
    } else {
        result = TRUE;
    }
    d_lockUnlock(d_lock(listener));
    /* Notify the listeners */
    (void)u_observableNotify(u_observable(listener->dataReader));
    return result;
}


c_bool
d_historicalDataRequestListenerStop(
    d_historicalDataRequestListener listener)
{
    c_bool result = FALSE;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_historicalDataRequestListenerIsValid(listener));

    d_lockLock(d_lock(listener));
    if (d_listener(listener)->attached == TRUE) {
        admin = d_listenerGetAdmin(d_listener(listener));
        subscriber = d_adminGetSubscriber(admin);
        waitset = d_subscriberGetWaitset(subscriber);
        d_lockUnlock(d_lock(listener));
        result = d_waitsetDetach(waitset, listener->waitsetData);
        d_lockLock(d_lock(listener));
        assert(result == TRUE);
        if (result) {
            d_adminRemoveListener(admin, listener->historicalDataRequestEventListener);
            d_eventListenerFree(listener->historicalDataRequestEventListener);
            d_listener(listener)->attached = FALSE;
            result = TRUE;
        }
    } else {
        result = TRUE;
    }
    d_lockUnlock(d_lock(listener));

    return result;
}



