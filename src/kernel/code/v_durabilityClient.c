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
#include "v_durabilityClient.h"

#include "v__topic.h"
#include "v__topicImpl.h"
#include "v__writer.h"
#include "v__publisher.h"
#include "v__subscriber.h"
#include "v__reader.h"
#include "v__policy.h"
#include "v__group.h"
#include "v_public.h"
#include "v_topicQos.h"
#include "v_writerQos.h"
#include "v_publisherQos.h"
#include "v_dataReaderInstance.h"
#include "v_observable.h"
#include "v_observer.h"
#include "v_builtin.h"
#include "v_waitset.h"
#include "v_handle.h"
#include "v_groupSet.h"
#include "v_messageQos.h"
#include "ut_avl.h"
#include "sd_cdr.h"
#include <stddef.h>   /* for definition of offsetof */

/* Included load functions; the load-functions are generated with
 * idlpp -m SPLLOAD. */
#include "client_durabilitySplType.h"
#include "client_durabilitySplLoad.c"
#include "dds_builtinTopicsSplType.h"

#include "vortex_os.h"
#include "os_report.h"

#define V_DC_SUBSCRIBER_NAME                     "subscriber <DurabilityClient>"
#define V_DC_PUBLISHER_NAME                      "publisher <DurabilityClient>"

#define V_DC_STATE_FLAG            (0x0001U << 0) /*    1 */
#define V_DC_STATE_REQUEST_FLAG    (0x0001U << 1) /*    2 */
#define V_DC_DATA_FLAG             (0x0001U << 2) /*    4 */
#define V_DC_DATA_REQUEST_FLAG     (0x0001U << 3) /*    8 */


/* Vendor ids, used for client-durability */
#define V_DC_VENDORID_UNKNOWN                          {{ 0x00, 0x00 }}
#define V_DC_VENDORID_RTI                              {{ 0x01, 0x01 }}
#define V_DC_VENDORID_PRISMTECH_OSPL                   {{ 0x01, 0x02 }}
#define V_DC_VENDORID_OCI                              {{ 0x01, 0x03 }}
#define V_DC_VENDORID_MILSOFT                          {{ 0x01, 0x04 }}
#define V_DC_VENDORID_KONGSBERG                        {{ 0x01, 0x05 }}
#define V_DC_VENDORID_TWINOAKS                         {{ 0x01, 0x06 }}
#define V_DC_VENDORID_LAKOTA                           {{ 0x01, 0x07 }}
#define V_DC_VENDORID_ICOUP                            {{ 0x01, 0x08 }}
#define V_DC_VENDORID_ETRI                             {{ 0x01, 0x09 }}
#define V_DC_VENDORID_RTI_MICRO                        {{ 0x01, 0x0a }}
#define V_DC_VENDORID_PRISMTECH_JAVA                   {{ 0x01, 0x0b }}
#define V_DC_VENDORID_PRISMTECH_GATEWAY                {{ 0x01, 0x0c }}
#define V_DC_VENDORID_PRISMTECH_LITE                   {{ 0x01, 0x0d }}
#define V_DC_VENDORID_TECHNICOLOR                      {{ 0x01, 0x0e }}
#define V_DC_VENDORID_EPROSIMA                         {{ 0x01, 0x0f }}
#define V_DC_VENDORID_PRISMTECH_CLOUD                  {{ 0x01, 0x20 }}

/* Durability version */
#define V_DC_VERSION_MAJOR                             2  /* From version 2.0 extended time (64-bit) is supported */
#define V_DC_VERSION_MINOR                             0
#define V_DC_VERSION_VENDOR_ID                         V_DC_VENDORID_PRISMTECH_OSPL

#define PAYLOAD_SERIALIZATION_FORMAT_CDR_ANY           0
#define PAYLOAD_SERIALIZATION_FORMAT_CDR_BE            1
#define PAYLOAD_SERIALIZATION_FORMAT_CDR_LE            2

#define HISTORICAL_DATA_KIND_BEAD                      0
#define HISTORICAL_DATA_KIND_LINK                      1

#define BEAD_MESSAGE_FLAG_WRITE                        0
#define BEAD_MESSAGE_FLAG_DISPOSE                      1
#define BEAD_MESSAGE_FLAG_UNREGISTER                   2
#define BEAD_MESSAGE_FLAG_WRITE_DISPOSE                3
#define BEAD_MESSAGE_FLAG_REGISTER                     4

#define COMPLETENESS_UNKNOWN                           0
#define COMPLETENESS_INCOMPLETE                        1
#define COMPLETENESS_COMPLETE                          2

#define V_DC_ALL_READERS_AVAILABLE(flags) \
            ((flags & (V_DC_STATE_REQUEST_FLAG|V_DC_DATA_REQUEST_FLAG)) == \
                      (V_DC_STATE_REQUEST_FLAG|V_DC_DATA_REQUEST_FLAG)  )


/*
 * Macros to extract the data part from a v_message or v_dataReaderSample.
 */
#define v__dcMessageGetData(msg)                   (C_DISPLACE(v_message(msg),C_MAXALIGNSIZE(C_SIZEOF(v_message))))

#define v__dcMessageGetDurabilityState(msg)        ((struct _DDS_DurabilityState *)       v__dcMessageGetData(msg))
#define v__dcMessageGetDurabilityStateRequest(msg) ((struct _DDS_DurabilityStateRequest *)v__dcMessageGetData(msg))
#define v__dcMessageGetHistoricalData(msg)         ((struct _DDS_HistoricalData *)        v__dcMessageGetData(msg))
#define v__dcMessageGetHistoricalDataRequest(msg)  ((struct _DDS_HistoricalDataRequest *) v__dcMessageGetData(msg))
#define v__dcMessageGetSubscriptionInfo(msg)       ((struct v_subscriptionInfo *)         v__dcMessageGetData(msg))
#define v__dcMessageGetSystemId(msg)               (msg->writerGID.systemId)

#define v__dcSampleGetDurabilityState(s)        (v__dcMessageGetDurabilityState      (v_dataReaderSampleTemplate(s)->message))
#define v__dcSampleGetDurabilityStateRequest(s) (v__dcMessageDurabilityStateRequest  (v_dataReaderSampleTemplate(s)->message))
#define v__dcSampleGetHistoricalData(s)         (v__dcMessageGetHistoricalData       (v_dataReaderSampleTemplate(s)->message))
#define v__dcSampleGetHistoricalDataRequest(s)  (v__dcMessageGetHistoricalDataRequest(v_dataReaderSampleTemplate(s)->message))
#define v__dcSampleGetSubscriptionInfo(s)       (v__dcMessageGetSubscriptionInfo     (v_dataReaderSampleTemplate(s)->message))
#define v__dcSampleGetSystemId(s)               (v__dcMessageGetSystemId             (v_dataReaderSampleTemplate(s)->message))


const char *builtin_topics[] = {
    V_TOPICINFO_NAME,
    V_PARTICIPANTINFO_NAME,
    V_PUBLICATIONINFO_NAME,
    V_SUBSCRIPTIONINFO_NAME,
    V_CMPARTICIPANTINFO_NAME,
    V_CMDATAWRITERINFO_NAME,
    V_CMDATAREADERINFO_NAME,
    V_CMPUBLISHERINFO_NAME,
    V_CMSUBSCRIBERINFO_NAME,
    V_HEARTBEATINFO_NAME,
    V_DELIVERYINFO_NAME,
    NULL   /* Must be the last element, used as delimiter */
};


typedef void (*v__dcDataAction)(v_durabilityClient _this, v_dataReaderSample sample);


struct v__dcTopicInfo {
    const unsigned int id;
    const char *topicName;
    const char *typeName;
    const char *keyList;
};


#if 0
C_CLASS(v__dcTopicInfo);
C_STRUCT(v__dcTopicInfo) {
    const char *topicName;
    const char *typeName;
    const char *keyList;
};
#endif


static struct v__dcTopicInfo v__dcTopics[] =
{
    { /* V_DC_TOPIC_STATE_ID */
      V_DC_TOPIC_STATE_ID,         "d_durabilityState",        "DDS::DurabilityState",        "serverId.prefix,serverId.suffix"                     },
    { /* V_DC_TOPIC_STATE_REQUEST_ID */
      V_DC_TOPIC_STATE_REQUEST_ID, "d_durabilityStateRequest", "DDS::DurabilityStateRequest", "requestId.clientId.prefix,requestId.clientId.suffix" },
    { /* V_DC_TOPIC_DATA_ID */
      V_DC_TOPIC_DATA_ID,          "d_historicalData",         "DDS::HistoricalData",         "serverId.prefix,serverId.suffix"                     },
    { /* V_DC_TOPIC_DATA_REQUEST_ID */
      V_DC_TOPIC_DATA_REQUEST_ID,  "d_historicalDataRequest",  "DDS::HistoricalDataRequest",  "requestId.clientId.prefix,requestId.clientId.suffix" }
};


#undef c_metaObject /* Undef casting macro to avoid ambiguity with return type of function pointer. */
#if 0
typedef c_metaObject (*v__dcLoad)(c_base base);
#endif
typedef struct c_metaObject_s * (*v__dcLoad)(c_base base);


struct v__dcDataType {
    const char *info;
    v__dcLoad loadType;
    v_durabilityClientTopicIds  topicId;
    v_durabilityClientReaderIds readerId;
    v_durabilityClientWriterIds writerId;
};


static struct v__dcDataType v__dcDataTypes[] =
{
    { "DurabilityStateRequest", __DDS_DurabilityStateRequest__load,
      V_DC_TOPIC_STATE_REQUEST_ID, V_DC_READER_ID_COUNT,    V_DC_WRITER_STATE_REQUEST_ID },
    { "DurabilityState",         __DDS_DurabilityState__load,
      V_DC_TOPIC_STATE_ID,         V_DC_READER_STATE_ID,    V_DC_WRITER_ID_COUNT         },
    { "HistoricalDataRequest",  __DDS_HistoricalDataRequest__load,
      V_DC_TOPIC_DATA_REQUEST_ID,  V_DC_READER_ID_COUNT,    V_DC_WRITER_DATA_REQUEST_ID  },
    { "HistoricalData",            __DDS_HistoricalData__load,
      V_DC_TOPIC_DATA_ID,          V_DC_READER_DATA_ID,     V_DC_WRITER_ID_COUNT         }
};


/* Internal structure to administrate pending chains */
struct chain_node {
    ut_avlNode_t avlnode;                  /* node in chains tree */
    struct _DDS_RequestId_t requestId;     /* table key */
    v_handle rhandle;                      /* handle to the reader that issued the request */
    v_group vgroup;                        /* local vgroup */
    v_message request;                     /* the original request */
    ut_avlCTree_t beads;                   /* beads for this chain */
    c_ulong sampleCount;
    c_ulong completeness;
    c_ulong errorCode;
    v_message link;
    c_long samplesExpect;                  /* number of samples to expect */
};


/* Internal structure to administrate links in a chain */
struct bead_node {
    ut_avlNode_t avlnode;                  /* node in bead tree */
    c_ulong refCount;
    v_message message;
    struct chain_node *chain;              /* backref to chain node containing this bead */
};


/* Internal structure to administrate server nodes */
struct server_node {
    ut_avlNode_t avlnode;                 /* node in tree of servers */
    struct _DDS_Gid_t serverId;           /* id of the server */
    c_ulong systemId;                     /* systemId of the server */
    ut_avlCTree_t groups;                 /* groups managed by this server */
    c_ulong flags;                        /* indicator for the readers that have been detected */
    c_bool statesRequested;               /* indicator if a DurabilityStateRequest has been sent to this server */
};


/* Internal structure for group nodes.
 * These reflect the groups on server nodes. */
struct group_node {
    ut_avlNode_t avlnode;                 /* node in tree of groups */
    struct _DDS_PartitionTopicState_t *state;  /* Last known state of the PartitionTopic on the server */
};



static c_bool           v__dcCreateEntities(v_durabilityClient _this);
static void             v__dcEnable(v_durabilityClient _this);
static void             v__dcEventDispatcher(v_durabilityClient _this);
static void             v__dcEventHistoricalDataRequest(v_durabilityClient _this, v_durabilityClientEvent event);
static void             v__dcEventDataAvailable(v_durabilityClient _this, v_durabilityClientEvent event);
static void             v__dcEventTerminate(v_durabilityClient _this, v_durabilityClientEvent event);
static c_string         v__dcGetSubPartition(v_durabilityClient _this);
static void             v__dcHandleDurabilityState(v_durabilityClient _this, v_dataReaderSample sample);
static void             v__dcHandleHistoricalData(v_durabilityClient _this, v_dataReaderSample sample);
static void             v__dcHandleSubscription(v_durabilityClient _this, v_dataReaderSample sample);
static v_actionResult   v__dcKeepReaderSamples(c_object o, c_voidp arg);
static v_publisherQos   v__dcNewQosPublisher(v_kernel kernel, c_string partition);
static v_subscriberQos  v__dcNewQosSubscriber(v_kernel kernel, c_string partition);
static v_topicQos       v__dcNewQosTopic(v_kernel kernel);
static v_readerQos      v__dcNewQosReader(v_kernel kernel);
static v_writerQos      v__dcNewQosWriter(v_kernel kernel);
static v_dataReader     v__dcNewBuiltinReader(v_kernel kernel, v_subscriber subscriber, int builtinId);
static v_dataReader     v__dcNewReader(v_subscriber subscriber, v_topic topic, v_readerQos qos);
static v_writer         v__dcNewWriter(v_publisher publisher, v_topic topic, v_writerQos qos);
static void             v__dcReadData(v_durabilityClient _this, v_dataReader reader, v__dcDataAction action);
static c_bool           v__dcSendMsg(v_writer writer, v_message msg);
static void             v__dcWaitsetAction(v_waitsetEvent e, c_voidp arg);
static void             v__dcWaitsetSetup(v_durabilityClient _this);
static struct server_node * v_dcRegisterServerByGid(v_durabilityClient _this, v_gid servergid, c_ulong flag);
void                        v_dcUnregisterServerByGid(v_durabilityClient _this, v_gid serverGid,c_ulong flag);
static struct server_node * v_dcRegisterServerByServerId(v_durabilityClient _this, struct _DDS_Gid_t serverId);
static struct _DDS_Gid_t               v__dcMyGid    (v_durabilityClient _this);
static struct _DDS_VendorId_t          v__dcMyVendor (v_durabilityClient _this);
static struct _DDS_DurabilityVersion_t v__dcMyVersion(v_durabilityClient _this);
static void                            v__dcCheckChainComplete(v_durabilityClient _this, struct chain_node *n, c_bool retry);
static void cleanup_chain(void *n);
static void cleanup_bead(void *n);
static void cleanup_server(void *n);
static void cleanup_group(void *n);
static void v_dcUpdateDurabilityState(v_durabilityClient _this, struct _DDS_RequestId_t *requestId, v_message msg, struct _DDS_DurabilityState *data, struct server_node *n);
void v_durabilityClientFree (v_durabilityClient _this);

/******************************************************************************
 * Private functions
 ******************************************************************************/


static int
compare_by_requestId(const void *va, const void *vb)
{
    const struct _DDS_RequestId_t *a = va;
    const struct _DDS_RequestId_t *b = vb;

    /* Compare requests based on their requestId. */
    if (a->clientId.prefix < b->clientId.prefix) {
        return -1;
    } else if (a->clientId.prefix > b->clientId.prefix) {
        return 1;
    } else if (a->clientId.suffix < b->clientId.suffix) {
        return -1;
    } else if (a->clientId.suffix > b->clientId.suffix) {
        return 1;
    } else if (a->requestId < b->requestId) {
        return -1;
    } else if (a->requestId > b->requestId) {
        return 1;
    }
    return 0;
}

static int
compare_by_message(const void *va, const void *vb)
{
    const v_message a = v_message(va);
    const v_message b = v_message(vb);

    /* Compare based on message */
    return v_messageCompare(a,b);
}


static int
compare_by_serverId(const void *va, const void *vb)
{
    const struct _DDS_Gid_t *a = va;
    const struct _DDS_Gid_t *b = vb;

    /* Compare requests based on their requestId. */
    if (a->prefix < b->prefix) {
        return -1;
    } else if (a->prefix > b->prefix) {
        return 1;
    } else if (a->suffix < b->suffix) {
        return -1;
    } else if (a->suffix > b->suffix) {
        return 1;
    }
    return 0;
}


static int
compare_by_state(const void *va, const void *vb)
{
    const struct _DDS_PartitionTopicState_t *a = va;
    const struct _DDS_PartitionTopicState_t *b = vb;
    int result;

    /* Compare ParitionTopicState based on partition and topic name */

    result = strcmp(a->topic, b->topic);
    if (result == 0) {
        result = strcmp(a->partition, b->partition);
    }
    return result;
}

static ut_avlCTreedef_t chains_avltreedef = UT_AVL_CTREEDEF_INITIALIZER (
                                            offsetof (struct chain_node, avlnode),   /* node in tree */
                                            offsetof (struct chain_node, requestId), /* attribute used for comparision */
                                            compare_by_requestId,                    /* comparison function */
                                            0);                                      /* flags */

static ut_avlCTreedef_t beads_avltreedef = UT_AVL_CTREEDEF_INITIALIZER_INDKEY (
                                            offsetof (struct bead_node, avlnode),    /* node in tree */
                                            offsetof (struct bead_node, message),    /* attribute used for comparision */
                                            compare_by_message,                      /* comparison function */
                                            0);                                      /* flags */

static ut_avlCTreedef_t servers_avltreedef = UT_AVL_CTREEDEF_INITIALIZER (
                                            offsetof (struct server_node, avlnode),  /* node in tree */
                                            offsetof (struct server_node, serverId), /* attribute used for comparision */
                                            compare_by_serverId,                     /* comparison function */
                                            0);                                      /* flags */

static ut_avlCTreedef_t groups_avltreedef = UT_AVL_CTREEDEF_INITIALIZER_INDKEY (
                                            offsetof (struct group_node, avlnode),   /* node in tree */
                                            offsetof (struct group_node, state),     /* attribute used for comparision */
                                            compare_by_state,                        /* comparison function */
                                            0);                                      /* flags */

c_bool
v_dcRegisterHistoricalDataRequest(
    v_durabilityClient _this,
    v_handle rhandle,
    struct _DDS_RequestId_t *requestId,
    v_group vgroup,
    v_message request)
{
    ut_avlIPath_t ip;
    struct chain_node *n;
    c_bool created = FALSE;

    assert(request);

    if ((n = ut_avlCLookupIPath (&chains_avltreedef, (ut_avlCTree_t *)_this->chains, requestId, &ip)) == NULL) {
        /* create a chain administration for this requestId */
        n = os_malloc (sizeof (*n));
        n->rhandle = rhandle;
        n->requestId.clientId.prefix = requestId->clientId.prefix;
        n->requestId.clientId.suffix= requestId->clientId.suffix;
        n->requestId.requestId = requestId->requestId;
        n->vgroup = c_keep(vgroup);
        n->request = c_keep(request);
        n->sampleCount = 0;
        n->completeness = 0;
        n->errorCode = 0;
        n->samplesExpect = 0;
        n->link = NULL;
        /* Create tables to store the beads and links */
        ut_avlCInit(&beads_avltreedef, &n->beads);
        ut_avlCInsertIPath (&chains_avltreedef, (ut_avlCTree_t *)_this->chains, n, &ip);
        V_DC_TRACE("%s - chain administration %p created for %s.%s with requestId (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32")\n",
                    OS_FUNCTION, (void *)n, v_partitionName(vgroup->partition), v_topicName(vgroup->topic), requestId->clientId.prefix, requestId->clientId.suffix, requestId->requestId);
        created = TRUE;
    } else  if (v_messageCompare(n->request, request) == 0) {
        V_DC_TRACE("%s - chain administration %p for %s.%s with requestId (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32") already present, reuse\n",
                  OS_FUNCTION, (void *)n, v_partitionName(vgroup->partition), v_topicName(vgroup->topic), requestId->clientId.prefix, requestId->clientId.suffix, requestId->requestId);
    } else {
        V_DC_TRACE("%s - chain administration %p contains for %s.%s with requestId (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32"), this differs from the current request for %s.%s, drop\n",
                  OS_FUNCTION, (void *)n, v_partitionName(n->vgroup), v_topicName(n->vgroup), requestId->clientId.prefix, requestId->clientId.suffix, requestId->requestId,
                  v_partitionName(vgroup), v_topicName(vgroup));
    }
    return created;
}


static void
v_dcInsertHistoricalDataBead(
    v_durabilityClient _this,
    struct chain_node *chain,
    v_message msg,
    struct _DDS_HistoricalData *data)
{
    ut_avlIPath_t ip;
    struct bead_node *b;

    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(data);

    assert(data->content._d == HISTORICAL_DATA_KIND_BEAD); /* bead */

    if ((b = ut_avlCLookupIPath (&beads_avltreedef, &chain->beads, msg, &ip)) == NULL) {
        /* The bead does not exist yet */
        b = os_malloc(sizeof(*b));
        b->refCount = 1;
        b->message = c_keep(msg);
        b->chain = chain;          /* not refCounted for performance reasons, note that chain my not be removed before cleaning its beads */
        ut_avlCInsertIPath (&beads_avltreedef, &chain->beads, b, &ip);
        chain->samplesExpect++;
    } else {
        /* Duplicate message */
        b->refCount++;
    }
    V_DC_TRACE("%s - msg %p added to chain administration %p\n", OS_FUNCTION, (void *)msg, (void *)chain);
}


static void
v_dcInsertHistoricalDataLink(
    v_durabilityClient _this,
    struct chain_node *chain,
    v_message msg,
    struct _DDS_HistoricalData *data)
{
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(msg);

    assert(data->content._d == HISTORICAL_DATA_KIND_LINK);    /* link */
    assert(chain->link == NULL);      /* not received yet */

    chain->sampleCount = data->content._u.link.sampleCount;
    chain->completeness = data->content._u.link.completeness;
    chain->errorCode = data->content._u.link.errorCode;
    chain->link = c_keep(msg);
    V_DC_TRACE("%s - link %p added to chain administration %p\n", OS_FUNCTION, (void *)msg, (void *)chain);
}


/* Insert historicalData */
void
v_dcInsertHistoricalData(
    v_durabilityClient _this,
    struct _DDS_RequestId_t *requestId,
    v_message msg,
    struct _DDS_HistoricalData *data)
{
    struct chain_node *n;

    if ((n = ut_avlCLookup (&chains_avltreedef, (ut_avlCTree_t *)_this->chains, requestId)) == NULL) {
        V_DC_TRACE("%s - No chain administration exists for request requestId (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32"), dropping the bead\n",
                   OS_FUNCTION, requestId->clientId.prefix, requestId->clientId.suffix, requestId->requestId);
        c_free(msg);
    } else {
        /* Add the data to the correct table */
        switch (data->content._d) {
            case HISTORICAL_DATA_KIND_BEAD:  /* bead */
                v_dcInsertHistoricalDataBead(_this, n, msg, data);
                v__dcCheckChainComplete(_this, n, FALSE);
                break;
            case HISTORICAL_DATA_KIND_LINK:  /* link */
                v_dcInsertHistoricalDataLink(_this, n, msg, data);
                v__dcCheckChainComplete(_this, n, FALSE);
                break;
            default :
                OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                          "Invalid union discriminator %d received in HistoricalData message", data->content._d);
                V_DC_TRACE("%s - Invalid discriminator %d in historicalData, chain %p msg %p, requestId (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32"), dropping the bead\n",
                          OS_FUNCTION, data->content._d, (void *)n, (void *)msg, requestId->clientId.prefix, requestId->clientId.suffix, requestId->requestId);
                c_free(msg);
                assert(FALSE);
        }
    }
}


static void tracegroupGenKeystr(char *keystr, size_t keystr_size, v_groupInstance gi)
{
    v_group g = gi->group;
    c_array instanceKeyList = c_tableKeyList(g->instances);
    c_ulong i, nrOfKeys = c_arraySize(instanceKeyList);
    size_t pos = 0;
    assert (keystr_size >= 4); /* for ...\0 */
    for (i = 0; i < nrOfKeys; i++)
    {
        c_value v = c_fieldValue(instanceKeyList[i], gi);
        char *vimg = c_valueImage(v);
        int n = snprintf(keystr + pos, keystr_size - pos, "%s%s", (i == 0) ? "" : ";", vimg);
        c_valueFreeRef(v);
        os_free(vimg);
        if (n > 0) { pos += (size_t)n; } else { break; }
    }
    if (i < nrOfKeys || pos >= keystr_size) {
        if (pos >= keystr_size - 4) {
            pos = keystr_size - 4;
        }
        strcpy(keystr + pos, "...");
    }
    c_free(instanceKeyList);
}


#if 0
static void tracegroupGenMsgKeystr(char *keystr, size_t keystr_size, v_group g, v_message msg)
{
    c_array messageKeyList = v_topicMessageKeyList(v_groupTopic(g));
    c_ulong i, nrOfKeys = c_arraySize(messageKeyList);
    size_t pos = 0;
    assert (keystr_size >= 4); /* for ...\0 */
    for (i = 0; i < nrOfKeys; i++)
    {
        c_value v = c_fieldValue(messageKeyList[i], msg);
        char *vimg = c_valueImage(v);
        int n = snprintf(keystr + pos, keystr_size - pos, "%s%s", (i == 0) ? "" : ";", vimg);
        c_valueFreeRef(v);
        os_free(vimg);
        if (n > 0) { pos += (size_t)n; } else { break; }
    }
    if (i < nrOfKeys || pos >= keystr_size) {
        if (pos >= keystr_size - 4) {
            pos = keystr_size - 4;
        }
        strcpy(keystr + pos, "...");
    }
}
#endif

static void tracegroupInstance(v_groupInstance gi, const char *prefix)
{
    v_groupSample s;
    char keystr[1024];

    OS_UNUSED_ARG(prefix);

    tracegroupGenKeystr(keystr, sizeof (keystr), gi);
    V_DC_TRACE("%s - %sInstance %p state=%u epoch=%"PA_PRItime" key={%s}\n", OS_FUNCTION, prefix, (void*)gi, gi->state, OS_TIMEE_PRINT(gi->epoch), keystr);
    s = v_groupSample(gi->oldest);
    while (s != NULL) {
        v_message msg = v_groupSampleTemplate(s)->message;
        OS_UNUSED_ARG(msg);
        V_DC_TRACE("%s - %s  Sample %p msg %p state=%u time=%"PA_PRItime" wrgid=%u:%u:%u\n",
              OS_FUNCTION, prefix, (void*)s, (void*)msg, msg->_parent.nodeState, OS_TIMEW_PRINT(msg->writeTime), msg->writerGID.systemId, msg->writerGID.localId, msg->writerGID.serial);
        s = s->newer;
    }
}

static c_bool tracegroupHelper(c_object obj, void *arg)
{
    OS_UNUSED_ARG(arg);

    tracegroupInstance ((v_groupInstance)obj, "  ");
    return 1;
}


static void tracegroup(v_group g, const char *info)
{
    OS_UNUSED_ARG(info);

    V_DC_TRACE("%s - Group %s.%s lastDisposeAllTime=%"PA_PRItime" - %s\n",
        OS_FUNCTION, v_partitionName(v_groupPartition(g)), v_topicName(v_groupTopic(g)), OS_TIMEW_PRINT(g->lastDisposeAllTime), info);
    c_mutexLock(&g->mutex);
    (void)c_walk((c_collection)g->instances, tracegroupHelper, NULL);
    c_mutexUnlock(&g->mutex);
}


static v_message
create_message(v_group group, struct _DDS_HistoricalData *bead, v_messageQos mqos, c_array messageKeyList, c_ulong nrOfKeys, c_value keyValues[], c_ulong nodeState)
{
    v_message message;
    c_ulong i;

    assert(group);
    assert(bead);
    assert(mqos);

    /* Currently, beads have a writerGID (12 bytes) to identify the writer, not a GUID (16 bytes).
     * Since the only server currently available is a durability service that uses beads with
     * writerGID this is OK. If some time in the future a server is constructed that uses
     * GUIDs to identify the writer the code below must be changed. */
    if (bead->content._u.bead.writerId[12] | bead->content._u.bead.writerId[13] | bead->content._u.bead.writerId[14] | bead->content._u.bead.writerId[15]) {
        OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                  "The writerId of bead %p is not a valid writerGID", (void *)bead);
        abort();
    }
    message = v_topicMessageNew_s(v_groupTopic(group));
    if (message != NULL) {
        v_nodeState(message) = nodeState;
        memcpy(&message->writerGID, bead->content._u.bead.writerId, sizeof(message->writerGID));
        message->writeTime = OS_TIMEW_INIT(bead->content._u.bead.sourceTimestamp.sec, bead->content._u.bead.sourceTimestamp.nanosec);
        message->qos = c_keep(mqos);
        for (i=0;i<nrOfKeys;i++) {
            c_fieldAssign(messageKeyList[i], message, keyValues[i]);
        }
    }
    return message;
}


/* Inject a message to the group or reader */
static v_writeResult
inject_message(v_durabilityClient _this, v_message msg, v_messageQos mqos, struct _DDS_HistoricalData *bead, v_group group, v_entry entry)
{
    c_array messageKeyList;
    c_ulong i, nrOfKeys;
    c_value keyValues[32];
    v_registration registration;
    v_groupInstance instance = NULL;
    v_message registerMessage = NULL;
    v_message unregisterMessage = NULL;
    c_bool doRegister = FALSE;
    c_bool doUnregister = FALSE;
    c_base base;
    v_writeResult writeResult = V_WRITE_SUCCESS;
    v_resendScope resendScope = V_RESEND_NONE;

    OS_UNUSED_ARG(_this);

    base = c_getBase(group);
    /* Get key values associated with the bead msg */
    messageKeyList = v_topicMessageKeyList(v_groupTopic(group));
    if ((nrOfKeys = c_arraySize(messageKeyList)) > 32) {
        OS_REPORT(OS_ERROR, OS_FUNCTION, 0, "number of keys for topic %s (%d) exceeds limit of 32", v_topicName(v_groupTopic(group)), nrOfKeys);
        writeResult = V_WRITE_PRE_NOT_MET;
        goto err_nrOfKeys;
    }
    for (i = 0; i < nrOfKeys; i++) {
        keyValues[i] = c_fieldValue(messageKeyList[i], msg);
    }
    /* get the instance associated with the msg key, or remember to register the instance */
    instance = v_groupLookupInstanceAndRegistration(group, keyValues, msg->writerGID, v_gidCompare, &registration);
    if (instance) {
        if (registration) {
            c_free(registration);
        } else {
            doRegister = TRUE;
        }
    } else {
        doRegister = TRUE;
    }

    /* Create registration and acquire the instance */
    if (doRegister) {
        if ((registerMessage = create_message(group, bead, mqos, messageKeyList, nrOfKeys, keyValues, L_REGISTER)) == NULL) {
            writeResult = V_WRITE_OUT_OF_RESOURCES;
            goto err_register;
        }
        if (!c_baseMakeMemReservation(base, C_MM_RESERVATION_HIGH)) {
            c_free(registerMessage);
            writeResult = V_WRITE_OUT_OF_RESOURCES;
            goto err_register_claim;
        }
        (void)v_groupWrite(group, registerMessage, &instance, V_NETWORKID_ANY, &resendScope);
        c_baseReleaseMemReservation(base, C_MM_RESERVATION_HIGH);
        if (v_gidIsNil(registerMessage->writerGID)) {
            /* An unregister message must be sent to */
            if ((unregisterMessage = create_message(group, bead, mqos, messageKeyList, nrOfKeys, keyValues, L_UNREGISTER)) == NULL) {
                writeResult = V_WRITE_OUT_OF_RESOURCES;
                goto err_unregister;
            }
            doUnregister = TRUE;
        }
        c_free(registerMessage);
    }
    for (i = 0; i < nrOfKeys; i++) {
        /* c_fieldValue adds a ref, so undo the ref */
        c_valueFreeRef(keyValues[i]);
    }

    /* At this point we should have an instance */
    assert(instance);

    /* Send the data */
    if (writeResult == V_WRITE_SUCCESS) {
        if (!c_baseMakeMemReservation(base, C_MM_RESERVATION_HIGH)) {
            writeResult = V_WRITE_OUT_OF_RESOURCES;
            goto err_data_claim;
        }
        if (entry) {
            /* no caching, deliver to the reader */
            writeResult = v_groupWriteNoStreamWithEntry(group, msg, &instance, V_NETWORKID_ANY, entry);
        } else {
            /* caching, write to group */
            writeResult = v_groupWrite(group, msg, &instance, V_NETWORKID_ANY, &resendScope);
        }
        c_baseReleaseMemReservation(base, C_MM_RESERVATION_HIGH);
    }

    /* Send unregister when needed. */
    if (writeResult == V_WRITE_SUCCESS) {
        if (doUnregister) {
            if (!c_baseMakeMemReservation(c_getBase(group), C_MM_RESERVATION_HIGH)) {
                writeResult = V_WRITE_OUT_OF_RESOURCES;
                goto err_unregister_claim;
            }
            writeResult = v_groupWrite(group, unregisterMessage, &instance, V_NETWORKID_ANY, &resendScope);
            /* Note that unregister message will never be rejected */
            c_baseReleaseMemReservation(c_getBase(group), C_MM_RESERVATION_HIGH);
            c_free(unregisterMessage);
            unregisterMessage = NULL;
        }
    }

    c_free(instance);
    return writeResult;

err_unregister_claim:
    if (unregisterMessage) {
        c_free(unregisterMessage);
    }

err_data_claim:
err_unregister:
err_register_claim:
    c_free(registerMessage);
err_register:
    c_free(instance);
err_nrOfKeys:
    return writeResult;
}


static v_reliabilityKind
mapReliabilityQosPolicyKind(
    enum _DDS_ReliabilityQosPolicyKind kind)
{
     v_reliabilityKind result = V_RELIABILITY_BESTEFFORT;

    switch (kind) {
        case _DDS_BEST_EFFORT_RELIABILITY_QOS :
            result = V_RELIABILITY_BESTEFFORT;
            break;
        case _DDS_RELIABLE_RELIABILITY_QOS :
            result = V_RELIABILITY_RELIABLE;
            break;
        default :
            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                      "Invalid _DDS_ReliabilityQosPolicyKind %d received in HistoricalData message", kind);
            assert(FALSE);
    }
    return result;
}


static v_ownershipKind
mapOwnershipQosPolicyKind(
    enum _DDS_OwnershipQosPolicyKind kind)
{
     v_ownershipKind result = V_OWNERSHIP_SHARED;

    switch (kind) {
        case _DDS_SHARED_OWNERSHIP_QOS :
            result = V_OWNERSHIP_SHARED;
            break;
        case _DDS_EXCLUSIVE_OWNERSHIP_QOS :
            result = V_OWNERSHIP_EXCLUSIVE;
            break;
        default :
            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                      "Invalid _DDS_OwnershipQosPolicyKind %d received in HistoricalData message", kind);
            assert(FALSE);
    }
    return result;
}


static v_orderbyKind
mapDestinationOrderQosPolicyKind(
    enum _DDS_DestinationOrderQosPolicyKind kind)
{
     v_orderbyKind result = V_ORDERBY_RECEPTIONTIME;

    switch (kind) {
        case _DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS :
            result = V_ORDERBY_RECEPTIONTIME;
            break;
        case _DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS :
            result = V_ORDERBY_SOURCETIME;
            break;
        default :
            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                      "Invalid _DDS_DestinationOrderQosPolicyKind %d received in HistoricalData message", kind);
            assert(FALSE);
    }
    return result;
}


static v_durabilityKind
mapDurabilityQosPolicyKind(
    enum _DDS_DurabilityQosPolicyKind kind)
{
     v_durabilityKind result = V_DURABILITY_VOLATILE;

    switch (kind) {
        case _DDS_VOLATILE_DURABILITY_QOS :
            result = V_DURABILITY_VOLATILE;
            break;
        case _DDS_TRANSIENT_LOCAL_DURABILITY_QOS :
            result = V_DURABILITY_TRANSIENT_LOCAL;
            break;
        case _DDS_TRANSIENT_DURABILITY_QOS :
            result = V_DURABILITY_TRANSIENT;
            break;
        case _DDS_PERSISTENT_DURABILITY_QOS :
            result = V_DURABILITY_PERSISTENT;
            break;
        default :
            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                      "Invalid _DDS_DurabilityQosPolicyKind %d received in HistoricalData message", kind);
            assert(FALSE);
    }
    return result;
}


static v_livelinessKind
mapLivelinessQosPolicyKind(
    enum _DDS_LivelinessQosPolicyKind kind)
{
     v_livelinessKind result = V_LIVELINESS_AUTOMATIC;

    switch (kind) {
        case _DDS_AUTOMATIC_LIVELINESS_QOS :
            result = V_LIVELINESS_AUTOMATIC;
            break;
        case _DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS :
            result = V_LIVELINESS_PARTICIPANT;
            break;
        case  _DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS :
            result = V_LIVELINESS_TOPIC;
            break;
        default :
            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                      "Invalid _DDS_LivelinessQosPolicyKind %d received in HistoricalData message", kind);
            assert(FALSE);
    }
    return result;
}


static v_presentationKind
mapPresentationQosPolicyAccessScopeKind(
    enum _DDS_PresentationQosPolicyAccessScopeKind kind)
{
    v_presentationKind result = V_PRESENTATION_INSTANCE;

    switch (kind) {
        case _DDS_INSTANCE_PRESENTATION_QOS :
            result = V_PRESENTATION_INSTANCE;
            break;
        case _DDS_TOPIC_PRESENTATION_QOS :
            result = V_PRESENTATION_TOPIC;
            break;
        case _DDS_GROUP_PRESENTATION_QOS :
            result = V_PRESENTATION_GROUP;
            break;
        default :
            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                      "Invalid _DDS_PresentationQosPolicyAccessScopeKind %d received in HistoricalData message", kind);
            assert(FALSE);
    }
    return result;
}



static os_duration
os_duration_from_DDS_Duration_t(
    struct _DDS_Duration_t d)
{
    os_duration d2;
    c_time t;

    t.seconds = d.sec;
    t.nanoseconds = d.nanosec;
    d2 = c_timeToDuration(t);
    return d2;
}



v_writeResult
v__dcChainBeadInject(
    v_durabilityClient _this,
    struct sd_cdrInfo *cdrInfo,
    struct chain_node *chain,
    struct bead_node *b,
    v_entry entry)
{
    struct _DDS_HistoricalData *bead;
    int rc = -1;
    v_message msg;
    v_writerQos wqos;
    v_messageQos mqos;
    v_writeResult writeResult = V_WRITE_SUCCESS;
    v_presentationKind access_scope;
    v_group group;

    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(cdrInfo);

    assert(_this);
    assert(chain);
    assert(b);

    V_DC_TRACE("%s - inject bead %p\n", OS_FUNCTION, (void *)b);

    group = chain->vgroup;
    bead = v__dcMessageGetHistoricalData(b->message);
    /* create a message that will contain the content of bead contents */
    if ((msg = v_topicMessageNew_s(v_groupTopic(group))) == NULL) {
        OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                  "Unable to create topic %s to store a bead", v_topicName(group->topic));
        V_DC_TRACE("%s - Unable to create topic %s to store a bead\n", OS_FUNCTION, v_topicName(group->topic));
        writeResult = V_WRITE_OUT_OF_RESOURCES;
        goto err_alloc_msg;
    }
    memcpy(&msg->writerGID, bead->content._u.bead.writerId, sizeof(msg->writerGID));
    msg->writeTime = OS_TIMEW_INIT(bead->content._u.bead.sourceTimestamp.sec, bead->content._u.bead.sourceTimestamp.nanosec);
    msg->sequenceNumber = bead->content._u.bead.sequenceNumber;
    /* Create a writerQos, use the HistoricalDataRequestWriter qos as template.
     * Subsequently set the correct messageQos values */
    wqos = v_writerQosNew(v_objectKernel(group), NULL);  /* default writer qos */
    if (wqos == NULL) {
        OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR, "Unable to writer qos");
        V_DC_TRACE("%s - Unable to create writer qos\n", OS_FUNCTION);
        writeResult = V_WRITE_OUT_OF_RESOURCES;
        goto err_alloc_wqos;
    }
    wqos->reliability.v.kind = mapReliabilityQosPolicyKind(bead->content._u.bead.qos.reliabilityKind);
    wqos->ownership.v.kind = mapOwnershipQosPolicyKind(bead->content._u.bead.qos.ownership.kind);
    wqos->orderby.v.kind = mapDestinationOrderQosPolicyKind(bead->content._u.bead.qos.destinationOrderKind);
    wqos->lifecycle.v.autodispose_unregistered_instances = bead->content._u.bead.qos.writerLifecycleAutoDisposeUnregisteredInstances;
    wqos->durability.v.kind = mapDurabilityQosPolicyKind(bead->content._u.bead.qos.durabilityKind);

    wqos->latency.v.duration = os_duration_from_DDS_Duration_t(bead->content._u.bead.qos.latencyBudget.duration);
    wqos->deadline.v.period = os_duration_from_DDS_Duration_t(bead->content._u.bead.qos.deadline.period);
    wqos->liveliness.v.kind = mapLivelinessQosPolicyKind(bead->content._u.bead.qos.liveliness.kind);
    wqos->liveliness.v.lease_duration = os_duration_from_DDS_Duration_t(bead->content._u.bead.qos.liveliness.lease_duration);
    wqos->lifespan.v.duration = os_duration_from_DDS_Duration_t(bead->content._u.bead.qos.lifespan.duration);
    wqos->transport.v.value = bead->content._u.bead.qos.transportPriority.value;

    access_scope = mapPresentationQosPolicyAccessScopeKind(bead->content._u.bead.qos.presentation.access_scope);
    if ((mqos = v_messageQos_from_wqos_new(wqos, NULL, access_scope, bead->content._u.bead.qos.presentation.coherent_access, bead->content._u.bead.qos.presentation.ordered_access)) == NULL) {
        OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR, "Unable to create message qos");
        V_DC_TRACE("%s - Unable to create message qos", OS_FUNCTION);
        writeResult = V_WRITE_PRE_NOT_MET;
        goto err_alloc_mqos;
    }
    msg->qos = mqos;
    switch (bead->content._u.bead.serializationFormat) {
        case PAYLOAD_SERIALIZATION_FORMAT_CDR_ANY: /* any, not valid */
                OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                          "Invalid serializationFormat %d received in HistoricalData message", bead->content._u.bead.serializationFormat);
                V_DC_TRACE("%s - Invalid serializationFormat %d received in HistoricalData message", OS_FUNCTION, bead->content._u.bead.serializationFormat);
                writeResult = V_WRITE_PRE_NOT_MET;
                goto err_deserialize_invalid;
                break;
        case  PAYLOAD_SERIALIZATION_FORMAT_CDR_BE: /* BE */
                if ((rc = sd_cdrDeserializeRawBE (msg+1, cdrInfo, c_sequenceSize(bead->content._u.bead.payload), bead->content._u.bead.payload)) < 0) {
                    OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                              "Deserialization failure (%d) (Big Endian)", rc);
                    V_DC_TRACE("%s - Deserialization failure (%d) (Big Endian)", OS_FUNCTION, rc);
                    writeResult = V_WRITE_PRE_NOT_MET;
                    goto err_deserialize_le;
                }
                break;
        case  PAYLOAD_SERIALIZATION_FORMAT_CDR_LE: /* LE */
                if ((rc = sd_cdrDeserializeRawLE (msg+1, cdrInfo, c_sequenceSize(bead->content._u.bead.payload), bead->content._u.bead.payload)) < 0) {
                    OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                              "Deserialization failure (%d) (Little Endian)", rc);
                    V_DC_TRACE("%s - Deserialization failure (%d) (Little Endian)", OS_FUNCTION, rc);
                    writeResult = V_WRITE_PRE_NOT_MET;
                    goto err_deserialize_be;
                }
                break;
        default: /* unknown */
                OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                          "Unknown serializationFormat %d received in HistoricalData message", bead->content._u.bead.serializationFormat);
                V_DC_TRACE(" %s - Unknown serializationFormat %d received in HistoricalData message", OS_FUNCTION, bead->content._u.bead.serializationFormat);
                writeResult = V_WRITE_PRE_NOT_MET;
                goto err_deserialize_unknown;
    }

    switch (bead->content._u.bead.kind) {
        case BEAD_MESSAGE_FLAG_WRITE         : v_stateSet(v_nodeState(msg), L_WRITE);  break;
        case BEAD_MESSAGE_FLAG_DISPOSE       : v_stateSet(v_nodeState(msg), L_DISPOSED); break;
        case BEAD_MESSAGE_FLAG_UNREGISTER    : v_stateSet(v_nodeState(msg), L_UNREGISTER); break;
        case BEAD_MESSAGE_FLAG_WRITE_DISPOSE : v_stateSet(v_nodeState(msg), L_WRITE | L_DISPOSED); break;
        case BEAD_MESSAGE_FLAG_REGISTER      : v_stateSet(v_nodeState(msg), L_REGISTER); break;
        default :
                OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
                          "Unknown message flag %d received in HistoricalData message", bead->content._u.bead.kind);
                V_DC_TRACE(" %s - Unknown message flag %d received in HistoricalData message", OS_FUNCTION, bead->content._u.bead.kind);
                goto err_bead_kind;
    }
    /* Inject the message to the group or reader.
     * Possible return values of inject_message are V_WRITE_SUCCESS, V_WRITE_PRE_NOT_MET, V_WRITE_OUT_OF_RESOURCES, or V_WRITE_REJECTED.
     * Only in case the message is injected successfully the bead is removed. */
    if ((writeResult = inject_message(_this, msg, mqos, bead, group, entry)) == V_WRITE_SUCCESS) {
        ut_avlCDelete (&beads_avltreedef, &chain->beads, b);
        cleanup_bead(b);
    }
    c_free(wqos);
    c_free(msg);  /* This also frees mqos */
    return writeResult;

err_bead_kind:
err_deserialize_unknown:
err_deserialize_le:
err_deserialize_be:
err_deserialize_invalid:
    c_free(mqos);
err_alloc_mqos:
    c_free(wqos);
err_alloc_wqos:
    c_free(msg);
err_alloc_msg:
    ut_avlCDelete (&beads_avltreedef, &chain->beads, b);
    cleanup_bead(b);
    return writeResult;
}


struct findEntryHelper {
    c_char* partition;
    c_char* topic;
    v_entry current;
    v_entry entry;
};


static c_bool
findEntryGroup(
    v_proxy proxy,
    c_voidp args)
{
    v_group vgroup;
    v_handleResult handleResult;
    c_bool result;
    c_string topicName, partitionName;

    struct findEntryHelper *entryHelper;
    entryHelper = (struct findEntryHelper*)args;
    result = TRUE;

    handleResult = v_handleClaim(proxy->source, (v_object*)(&vgroup));

    if (handleResult == V_HANDLE_OK) {
        topicName = v_entityName(v_groupTopic(vgroup));
        partitionName = v_entityName(v_groupPartition(vgroup));

        if (topicName && partitionName) {
            if(strcmp(entryHelper->topic, topicName) == 0){
                if(strcmp(entryHelper->partition, partitionName) == 0){
                    entryHelper->entry = entryHelper->current;
                    result = FALSE;
                }
            }
        }
        v_handleRelease(proxy->source);
    }
    return result;
}


static c_bool
findEntry(
    v_entry entry,
    c_voidp args)
{
    struct findEntryHelper *entryHelper;
    entryHelper = (struct findEntryHelper*)args;
    entryHelper->current = entry;

    return c_tableWalk(entry->groups, (c_action)findEntryGroup, args);
}


#if 0
static int
handleCompare(
    v_handle handle1,
    v_handle handle2)
{
    int result = 0;

    if (handle1.index < handle2.index) {
        result = -1;
    } else if (handle1.index > handle2.index) {
        result = 1;
    } else if (handle1.serial < handle2.serial) {
        result = -1;
    } else if (handle1.serial > handle2.serial) {
        result = 1;
    }
    return result;
}
#endif


void
reschedule_rejected_chain(
    v_durabilityClient _this,
    struct chain_node *chain)
{
    ut_avlIPath_t ip;
    struct chain_node *n;

    assert(_this);
    assert(chain);

    if ((n = ut_avlCLookupIPath (&chains_avltreedef, (ut_avlCTree_t *)_this->rejected_chains, &chain->requestId, &ip)) == NULL) {
        ut_avlCInsertIPath (&chains_avltreedef, (ut_avlCTree_t *)_this->rejected_chains, chain, &ip);
        V_DC_TRACE("%s - bead injection rejected for chain administration %p for %s.%s with requestId (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32", rescheduled...\n",
            OS_FUNCTION, (void *)chain, v_partitionName(chain->vgroup->partition), v_topicName(chain->vgroup->topic), chain->requestId.clientId.prefix, chain->requestId.clientId.suffix, chain->requestId.requestId);
    }
    return;
}


static void
v__dcCheckChainComplete(
    v_durabilityClient _this,
    struct chain_node *chain,
    c_bool retry)
{
    struct bead_node *b;
    struct _DDS_HistoricalDataRequest *request;
    struct sd_cdrInfo *cdrInfo;
    v_reader vreader, *vreaderPtr;
    struct findEntryHelper entryHelper;
    c_bool cache = TRUE;
    c_bool doInject = TRUE;
    c_bool claimed = FALSE;

    assert(chain);

    OS_UNUSED_ARG(request);

    if (chain->link != NULL) {
        if (((os_int32)(ut_avlCCount(&chain->beads)) == chain->samplesExpect) || retry) {
            v_writeResult writeResult = V_WRITE_SUCCESS;

            if (retry) {
                V_DC_TRACE("%s - retrying chain %p, still to insert %u beads\n", OS_FUNCTION, (void *)chain, (c_ulong)ut_avlCCount(&chain->beads));
            } else {
                V_DC_TRACE("%s - chain %p has become complete, going to insert %u beads\n", OS_FUNCTION, (void *)chain, (c_ulong)ut_avlCCount(&chain->beads));
            }
            /* get the topic from the request associated with this chain */
            entryHelper.partition = v_partitionName(v_groupPartition(chain->vgroup));
            entryHelper.topic     = v_topicName(v_groupTopic(chain->vgroup));
            entryHelper.entry     = NULL;
            request = v__dcMessageGetHistoricalDataRequest(chain->request);
            if ((cdrInfo = sd_cdrInfoNew(v_topicDataType(v_groupTopic(chain->vgroup)))) == NULL) {
                OS_REPORT(OS_ERROR,
                          "kernel::v_durabilityClient::v__dcCheckChainComplete", V_RESULT_INTERNAL_ERROR,
                          "Unable to create the serializer");
                goto err_alloc_cdrInfo;
            }
            if (sd_cdrCompile (cdrInfo) < 0) {
                OS_REPORT(OS_ERROR,
                          "kernel::v_durabilityClient::v__dcCheckChainComplete", V_RESULT_INTERNAL_ERROR,
                          "Unable to use the serializer");
                goto err_compile;
            }
            if (!cache) {
                /* The data must be delivered to the reader only, so we must lookup
                 * the corresponding reader entry. Be aware that the reader may
                 * have left already. */
                if (v_handleClaim(chain->rhandle, (v_object*)(vreaderPtr = &vreader)) == V_HANDLE_OK) {
                    claimed = TRUE;
                    v_readerWalkEntries(vreader, (c_action)findEntry, &entryHelper);
                    /* The entry associated with the reader that did the request is now in entryHelper.entry,
                     * or NULL if not found */
                    if (!entryHelper.entry) {
                        /* There is no entryset to deliver the data to */
                        OS_REPORT(OS_ERROR,
                                  "kernel::v_durabilityClient::v__dcCheckChainComplete", V_RESULT_INTERNAL_ERROR,
                                  "Unable to lookup entry for reader [%d,%d] for group '%s.%s'", chain->rhandle.index, chain->rhandle.serial, entryHelper.partition, entryHelper.topic);
                        /* no need to inject the beads in this case */
                        doInject = FALSE;
                    }
                }
            }

            /* Inject the beads */
            if (doInject) {
                tracegroup(chain->vgroup, "before");
                b = ut_avlCFindMin (&beads_avltreedef, &chain->beads);
                while (b && (!_this->terminate) && (writeResult == V_WRITE_SUCCESS)) {
                    struct bead_node *next = ut_avlCFindSucc (&beads_avltreedef, &chain->beads, b);
                    writeResult = v__dcChainBeadInject(_this, cdrInfo, chain, b, entryHelper.entry);
                    b = next;
                }
                tracegroup(chain->vgroup, "after");
            }

            /* Remove the chain from the list */
            if (!retry) {
                ut_avlCDelete (&chains_avltreedef, (ut_avlCTree_t *)_this->chains, chain);
            } else {
                ut_avlCDelete (&chains_avltreedef, (ut_avlCTree_t *)_this->rejected_chains, chain);
            }

            /* Check if succeeded */
            if (writeResult == V_WRITE_REJECTED) {
                /* Rejected, reinsert the chain and try again */
                reschedule_rejected_chain(_this, chain);
            } else if (writeResult == V_WRITE_SUCCESS) {
                /* All beads successfully injected */
                if (cache) {
                    v_groupCompleteSet(chain->vgroup, TRUE);
                }
            } else {
                /* V_WRITE_PRE_NOT_MET or V_WRITE_OUT_OF_RESOURCES */
                V_DC_TRACE("%s - Unrecoverable error %d when injecting chain %p, discarding the chain\n", OS_FUNCTION, writeResult, (void *)chain);
            }

            if (claimed) {
               v_handleRelease(chain->rhandle);
            }
            /* cleanup chain and free cdrInfo */
            if (writeResult != V_WRITE_REJECTED) {
                cleanup_chain(chain);
            }
            sd_cdrInfoFree(cdrInfo);
        }
    }
    return;

err_compile:
    sd_cdrInfoFree(cdrInfo);
err_alloc_cdrInfo:
    /* Delete the chain from the list of chains and cleanup the chain */
    ut_avlCDelete (&chains_avltreedef, (ut_avlCTree_t *)_this->chains, chain);
    cleanup_chain(chain);
    return;
}



static c_bool
v__dcCreateEntities(v_durabilityClient _this)
{
    v_kernel kernel      = NULL;

    v_subscriberQos sQos = NULL;
    v_publisherQos pQos  = NULL;
    v_writerQos wQos     = NULL;
    v_readerQos rQos     = NULL;
    v_topicQos tQos      = NULL;
    c_string subPartitions;

    int i,j;

    kernel = v_objectKernel(_this);
    assert(kernel);

    /*
     * Get QoSses for the Durability Client protocol.
     */
    subPartitions = v__dcGetSubPartition(_this);
    pQos = v__dcNewQosPublisher (kernel, _this->partitions[V_DC_PARTITION_REQUEST_ID]);
    sQos = v__dcNewQosSubscriber(kernel, subPartitions);
    tQos = v__dcNewQosTopic     (kernel);
    wQos = v__dcNewQosWriter    (kernel);
    rQos = v__dcNewQosReader    (kernel);
    if ((pQos == NULL) || (sQos == NULL) || (tQos == NULL) || (wQos == NULL) || (rQos == NULL)) {
        c_free(subPartitions);
        OS_REPORT(OS_ERROR,
                  "kernel::v_durabilityClient::v__dcCreateEntities", V_RESULT_INTERNAL_ERROR,
                  "Failed to create one or more QoS");
        goto err_alloc_qos;
    }
    c_free(subPartitions);
    /*
     * Create the type-independent entities
     */
    _this->waitset = v_waitsetNew (_this->participant);
    _this->publisher  = v_publisherNew (_this->participant, V_DC_PUBLISHER_NAME,  pQos, FALSE);
    _this->subscriber = v_subscriberNew(_this->participant, V_DC_SUBSCRIBER_NAME, sQos, FALSE);
    if ((_this->waitset    == NULL) ||
        (_this->publisher  == NULL) ||
        (_this->subscriber == NULL) ){
        OS_REPORT(OS_ERROR,
                  "kernel::v_durabilityClient::v__dcCreateEntities", V_RESULT_INTERNAL_ERROR,
                  "Failed to create one or more type-independent Entities");
        goto err_alloc_untyped_entities;
    }

    /*
     * Create the typed entities
     */
    for (i=0; i < (int)(sizeof(v__dcDataTypes) / sizeof(*v__dcDataTypes)); i++) {
        v_durabilityClientReaderIds readerId;
        v_durabilityClientWriterIds writerId;
        v_durabilityClientTopicIds topicId;
        struct v__dcTopicInfo *topicInfo;

        /* Create the topic. */
        topicId = v__dcDataTypes[i].topicId;
        topicInfo = &(v__dcTopics[topicId]);
        _this->topics[topicId] = v_topic(v_topicImplNew(kernel,
                                                        topicInfo->topicName,
                                                        topicInfo->typeName,
                                                        topicInfo->keyList,
                                                        tQos,
                                                        TRUE));
        if (_this->topics[topicId] == NULL) {
            OS_REPORT(OS_ERROR,
                      "kernel::v_durabilityClient::v__dcCreateEntities", V_RESULT_INTERNAL_ERROR,
                      "Failed to create the topic for %s",  v__dcDataTypes[i].info);
            goto err_alloc_topic;
        }

        /* Create the reader, when needed. */
        readerId = v__dcDataTypes[i].readerId;
        if (readerId != V_DC_READER_ID_COUNT) {
            _this->readers[readerId] = v__dcNewReader(_this->subscriber,
                                                      _this->topics[topicId],
                                                      rQos);
            if (_this->readers[readerId] == NULL) {
                OS_REPORT(OS_ERROR,
                          "kernel::v_durabilityClient::v__dcCreateEntities", V_RESULT_INTERNAL_ERROR,
                          "Failed to create the reader for %s",  v__dcDataTypes[i].info);
                goto err_alloc_reader;
            }
        }

        /* Create the writer, when needed. */
        writerId = v__dcDataTypes[i].writerId;
        if (writerId != V_DC_WRITER_ID_COUNT) {
            _this->writers[writerId] = v__dcNewWriter(_this->publisher,
                                                      _this->topics[topicId],
                                                      wQos);
            if (_this->writers[writerId] == NULL) {
                OS_REPORT(OS_ERROR,
                          "kernel::v_durabilityClient::v__dcCreateEntities", V_RESULT_INTERNAL_ERROR,
                          "Failed to create the writer for %s",  v__dcDataTypes[i].info);
                goto err_alloc_writer;
            }
        }
    }

    /*
     * Create builtin subscription info reader to detect servers readers.
     */
    _this->readers[V_DC_READER_SUBSCRIPTIONINFO_ID] = v__dcNewBuiltinReader(kernel,
                                                                            _this->builtinSubscriber,
                                                                            V_SUBSCRIPTIONINFO_ID);
    if (_this->readers[V_DC_READER_SUBSCRIPTIONINFO_ID] == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_durabilityClient::v__dcCreateEntities", V_RESULT_INTERNAL_ERROR,
                  "Failed to create the reader for BuiltinSubscriptionInfo");
        goto err_alloc_builtin_reader;
    }

    v__dcWaitsetSetup(_this);

    v_readerQosFree(rQos);
    v_writerQosFree(wQos);
    v_topicQosFree(tQos);
    v_publisherQosFree(pQos);
    v_subscriberQosFree(sQos);

    return TRUE;

   /* Cleanup stuff */
err_alloc_builtin_reader:
err_alloc_writer:
    v_dataReaderFree(_this->readers[v__dcDataTypes[i].readerId]);
err_alloc_reader:
    v_topicFree(_this->topics[v__dcDataTypes[i].topicId]);
err_alloc_topic:
    for (j=0; j < i; j++) {
        v_topicFree(_this->topics[v__dcDataTypes[j].topicId]);
        v_dataReaderFree(_this->readers[v__dcDataTypes[j].readerId]);
        v_writerFree(_this->writers[v__dcDataTypes[j].writerId]);
    }
err_alloc_untyped_entities:
    c_free(_this->waitset);
    c_free(_this->publisher);
    c_free(_this->subscriber);
err_alloc_qos:
    c_free (rQos);
    c_free (wQos);
    c_free (tQos);
    c_free (pQos);
    c_free (sQos);
    return FALSE;
}


static void
v__dcEnable(v_durabilityClient _this)
{
    (void)v_entityEnable(v_entity(_this->publisher));
    (void)v_entityEnable(v_entity(_this->subscriber));
    (void)v_entityEnable(v_entity(_this->readers[V_DC_READER_STATE_ID]));
    (void)v_entityEnable(v_entity(_this->writers[V_DC_WRITER_STATE_REQUEST_ID]));
    (void)v_entityEnable(v_entity(_this->readers[V_DC_READER_DATA_ID]));
    (void)v_entityEnable(v_entity(_this->writers[V_DC_WRITER_DATA_REQUEST_ID]));
    (void)v_entityEnable(v_entity(_this->readers[V_DC_READER_SUBSCRIPTIONINFO_ID]));
}


static void
v__dcEventDispatcher(v_durabilityClient _this)
{
    v_durabilityClientEvent event;

    c_mutexLock(&_this->mutex);

    for (event = c_removeAt(_this->queue, 0); event != NULL; event = c_removeAt(_this->queue, 0))
    {
        if (!_this->terminate) {
            /* Multiple flags can be set as the same time, e.g., V_EVENT_HISTORY_REQUEST | V_EVENT_DATA_AVAILABLE */
            if (v_stateTest(event->kind, V_EVENT_HISTORY_REQUEST)) {
                v__dcEventHistoricalDataRequest(_this, event);
            }
            if (v_stateTest(event->kind, V_EVENT_DATA_AVAILABLE)) {
                v__dcEventDataAvailable(_this, event);
            }
            if (v_stateTest(event->kind, V_EVENT_TERMINATE)) {
                v__dcEventTerminate(_this, event);
            }
        }
        c_free(event);
    }
    c_mutexUnlock(&_this->mutex);
}


c_bool
is_builtin_group(
    char *partition,
    char *topic)
{
    assert(partition);
    assert(topic);
    if(strcmp(partition, V_BUILTIN_PARTITION) != 0) {
        return FALSE;
    } else {
        int i;
        for (i = 0; builtin_topics[i] != NULL; i++) {
            if (strcmp (topic, builtin_topics[i]) == 0) {
                return TRUE;
            }
        }
        return FALSE;
    }
}


/* Default request-combine period (same as //OpenSplice/DurabilityService/Network/Alignment/RequestCombinePeriod/Operational) */
const os_duration DEFAULT_REQUEST_COMBINE_TIME = OS_DURATION_INIT(0, 10000000);  /* 10 ms */

static os_duration
setTimeout(
    os_duration timeout)
{
    os_duration t = OS_DURATION_INVALID;
    if (!OS_DURATION_ISINVALID(timeout)) {
        if (OS_DURATION_ISINFINITE(timeout)) {
            t = OS_DURATION_INFINITE;
        } else if (timeout < DEFAULT_REQUEST_COMBINE_TIME) {
            t = OS_DURATION_ZERO;
        } else {
            t = DEFAULT_REQUEST_COMBINE_TIME;
        }
    }
    return t;
}


static void
v__dcEventHistoricalDataRequest(v_durabilityClient _this, v_durabilityClientEvent event)
{
    v_historicalDataRequest req = event->eventData;
    v_reader rd;
    c_iter alignmentPartition;
    c_iter ps = NULL;
    char *topic;
    c_iter filterParams;
    c_ulong i;
    v_handle rhandle;
    c_bool doRequest = FALSE;
    os_duration timeout;


    /* The client should only request historical data for groups that
     * matches the durablePolicies configuration */

    /* The client should not send a request for builtin topics that are
     * supposed to be discovered by DDSI.
     *
     * NOTE: for the moment we prevent requests for builtin topics by
     * only looking at the topics. We do not (yet) look at the partition
     * where the topics are published. In case the partition is an expression
     * the server cannot decide anymore if the topic must be aligned or not.
     * Luckily, in the current implementation the partition is always
     * a concrete partition (i.e., no pattern). */

    if (v_handleClaim(event->entity, (v_object *) &rd) != V_HANDLE_OK) {
        return;
    } else {
        /* Retrieve the partitions */
        c_iter kps = NULL;
        v_partition p;
        c_bool cache;
        v_subscriber subscriber;

        assert (v_objectKind(rd) == K_DATAREADER);
        /* Make sure that the subscriber exists, the subscriber could be removed while the reader is being destroyed */
        v_observerLock(v_observer(rd));
        subscriber = c_keep(rd->subscriber);
        v_observerUnlock(v_observer(rd));
        if (subscriber != NULL) {
            rhandle = v_publicHandle(v_public(rd));
            topic = c_keep(v_entityName(v_dataReaderGetTopic(v_dataReader(rd))));
            c_mutexLock(&v_subscriber(subscriber)->partitions->mutex);
            kps = ospl_c_select(v_subscriber(subscriber)->partitions->partitions, 0);
            c_mutexUnlock(&v_subscriber(subscriber)->partitions->mutex);
            while ((p = v_partition(c_iterTakeFirst(kps))) != NULL) {
                /* skip builtin topics and groups that do not need to be aligned by client durability */
                if ((!is_builtin_group(v_partitionName(p), topic)) && (v_durabilityClientIsResponsibleForAlignment(_this, v_partitionName(p), topic, &cache))) {
                    ps = c_iterAppend(ps, c_keep(v_entity(p)->name));
                    doRequest = TRUE;
                }
                c_free(p);
            }
            c_iterFree(kps);
            c_free(subscriber);
        }
        v_handleRelease(event->entity);
    }

    if (doRequest) {
        /* Construct the request */
        filterParams = c_iterNew(NULL);
        for (i = 0; i < c_arraySize(req->filterParams); i++) {
            filterParams = c_iterAppend(filterParams, req->filterParams[i]);
        }
        alignmentPartition = c_iterNew(_this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID]);
        /* Set the combine period to 10ms if timeout >= 10ms, or 0ms otherwise */
        timeout = setTimeout(req->timeout);
        /* Send the historicalDataRequest */
        v_dcSendMsgDataRequest(
                _this,
                rhandle,
                ps,
                topic,
                req->filter,
                filterParams,
                req->minSourceTimestamp,
                req->maxSourceTimestamp,
                req->resourceLimits.v.max_samples,
                req->resourceLimits.v.max_instances,
                req->resourceLimits.v.max_samples_per_instance,
                alignmentPartition,
                timeout);
        c_free(topic);
        c_iterFree(filterParams);
        c_iterFree(alignmentPartition);
    }
    {
        /* Cleanup the partitions */
        char *n;
        while ((n = c_iterTakeFirst(ps)) != NULL) {
            c_free(n);
        }
        c_iterFree(ps);
    }
    return;
}


static void
v__dcEventDataAvailable(v_durabilityClient _this, v_durabilityClientEvent event)
{
    if (v_handleIsEqual(v_publicHandle((v_public)_this->readers[V_DC_READER_STATE_ID]), event->entity)) {
        v__dcReadData(_this,
                      _this->readers[V_DC_READER_STATE_ID],
                      v__dcHandleDurabilityState);
    } else if (v_handleIsEqual(v_publicHandle((v_public)_this->readers[V_DC_READER_DATA_ID]), event->entity)) {
        v__dcReadData(_this,
                      _this->readers[V_DC_READER_DATA_ID],
                      v__dcHandleHistoricalData);
    } else if (v_handleIsEqual(v_publicHandle((v_public)_this->readers[V_DC_READER_SUBSCRIPTIONINFO_ID]), event->entity)) {
        v__dcReadData(_this,
                      _this->readers[V_DC_READER_SUBSCRIPTIONINFO_ID],
                      v__dcHandleSubscription);
    } else {
        OS_REPORT(OS_ERROR,
                  "kernel::v_durabilityClient::v__dcEventDataAvailable", V_RESULT_INTERNAL_ERROR,
                  "Unknown source");
        assert(FALSE);
    }
}


static void
cleanup_bead(
    void *n)
{
    struct bead_node *m = (struct bead_node *)n;

    assert(m);
    assert(m->chain);

    V_DC_TRACE("%s - Cleaning up bead %p\n", OS_FUNCTION, (void *)m);
    c_free(m->message);
    m->chain->samplesExpect--;
    os_free(m);
}


static void
cleanup_chain(
    void *n)
{
    struct chain_node *m = (struct chain_node *)n;

    assert(m);

    V_DC_TRACE("%s - Cleaning up chain %p\n", OS_FUNCTION, (void *)m);
    ut_avlCFree(&beads_avltreedef, &m->beads, cleanup_bead);
    c_free(m->vgroup);
    c_free(m->request);
    if (m->link) {
        c_free(m->link);
    }
    os_free(m);
}


static void
cleanup_group(
    void *n)
{
    struct group_node *m = (struct group_node *)n;

    assert(m);

    V_DC_TRACE("%s - Cleaning up group %p\n", OS_FUNCTION, (void *)m);
    os_free(m->state->partition);
    os_free(m->state->topic);
    os_free(m->state);
    os_free(m);
}


static void
cleanup_server(
    void *n)
{
    struct server_node *m = (struct server_node *)n;

    assert(m);

    V_DC_TRACE("%s - Cleaning up server %p\n", OS_FUNCTION, (void *)m);
    ut_avlCFree(&groups_avltreedef, &m->groups, cleanup_group);
    os_free(m);
}


static void
v__dcEventTerminate(v_durabilityClient _this, v_durabilityClientEvent event)
{
    OS_UNUSED_ARG(event);

    /* Clean up the resources associated with this thread */
    v_durabilityClientFree(_this);
}


static c_string
v__dcGetSubPartition(v_durabilityClient _this)
{
    v_kernel kernel;
    c_base base;

    c_string partition;
    unsigned long size;
    size_t rSize, gSize, pSize;

    kernel = v_objectKernel(_this);
    assert(kernel);
    base = c_getBase(kernel);
    assert(base);

    /* Create a string consisting of all partitions separated by a ',' */
    rSize = strlen(_this->partitions[V_DC_PARTITION_REQUEST_ID]);
    gSize = strlen(_this->partitions[V_DC_PARTITION_GLOBAL_DATA_ID]);
    pSize = strlen(_this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID]);
    size = (unsigned long)(rSize + gSize + pSize +3); /* 2 delimiters (",") and a "\0" */
    partition = c_stringMalloc(base, size);
    memcpy(partition, _this->partitions[V_DC_PARTITION_REQUEST_ID], rSize);
    size = rSize;
    if (strcmp(_this->partitions[V_DC_PARTITION_GLOBAL_DATA_ID], _this->partitions[V_DC_PARTITION_REQUEST_ID]) != 0) {
        partition[size] = ',';
        memcpy(partition+size+1, _this->partitions[V_DC_PARTITION_GLOBAL_DATA_ID], gSize);
        size = size + 1 + gSize;
    }
    if ((strcmp(_this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID], _this->partitions[V_DC_PARTITION_REQUEST_ID]   ) != 0) &&
        (strcmp(_this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID], _this->partitions[V_DC_PARTITION_GLOBAL_DATA_ID]) != 0) ) {
        partition[size] = ',';
        memcpy(partition+size+1, _this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID], pSize);
        size = size + 1 + pSize;
    }
    partition[size] = '\0';
    return partition;
}


static void
v__dcHandleDurabilityState(v_durabilityClient _this, v_dataReaderSample sample)
{
    struct _DDS_DurabilityState *data = v__dcSampleGetDurabilityState(sample);
    v_message msg = v_dataReaderSampleTemplate(sample)->message;
    c_ulong i;
    struct _DDS_Gid_t myId = v__dcMyGid(_this);
    struct _DDS_RequestId_t *requestId = NULL;
    c_bool forMe = FALSE;
    c_bool forEverybody = FALSE;

    if (v_dataReaderSampleStateTest(sample, L_VALIDDATA) && !(v_dataReaderSampleInstanceStateTest(sample, L_DISPOSED))) {
        if ((data->serverId.prefix == myId.prefix) && (data->serverId.suffix == myId.suffix)) {
            V_DC_TRACE("%s -   sent by myself, IGNORE\n", OS_FUNCTION);
        } else {
            /* Decide if the request if forMe or forEverybody */
            if (c_sequenceSize(data->requestIds) == 0) {
                V_DC_TRACE("%s -   addressed to everybody, so also to me\n", OS_FUNCTION);
                forEverybody = TRUE;
            } else {
                struct _DDS_RequestId_t *requestIds = (struct _DDS_RequestId_t *)data->requestIds;
                /* The list of requestIds may not be empty */
                for (i=0; i<c_sequenceSize(data->requestIds) && (!forMe); i++) {
                    if ((requestIds[i].clientId.prefix == myId.prefix) && (requestIds[i].clientId.suffix == myId.suffix)) {
                        forMe = TRUE;
                        V_DC_TRACE("%s -   addressed to me\n", OS_FUNCTION);
                    }
                }
            }
            if (forMe || forEverybody) {
                struct server_node *n;
                /* Apparently this is a relevant server for me.
                 * Register an entry for the server if not do so already
                 * and insert the data associated with message */
                n = v_dcRegisterServerByServerId(_this, data->serverId);
                v_dcUpdateDurabilityState(_this, requestId, msg, data, n);
            } else {
                V_DC_TRACE("%s -   request NOT for me, ignore", OS_FUNCTION);
            }
        }
    }
}


struct find_group_helper {
    char *groupName;
    v_group found;
};


static c_bool
find_group_by_name(
    c_object o,
    c_voidp args)
{
    c_bool result = TRUE;

    v_group g = v_group(o);
    struct find_group_helper *helper = (struct find_group_helper *)args;
    if (strcmp(g->name, helper->groupName) == 0) {
        helper->found = c_keep(g);
        result = FALSE;
    }
    return result;
}


/* Retrieve the local vgroup for the specified partition and topic names */
static v_group
find_group(
    v_durabilityClient _this,
    char *partition,
    char *topic)
{
    v_kernel kernel;
    struct find_group_helper helper;
    os_size_t groupNameLen;

    assert(partition);
    assert(topic);

    kernel = v_objectKernel(_this);
    assert(kernel);
    /* Construct the groupNamefrom the partiton and the topic */
    groupNameLen = sizeof(V_GROUP_NAME_TEMPLATE) - sizeof("%s%s") + 1 /* \0 */ + strlen(partition) + strlen(topic);
    helper.groupName = os_malloc(groupNameLen);
    snprintf(helper.groupName, groupNameLen, V_GROUP_NAME_TEMPLATE, partition, topic);
    helper.found = NULL;
    v_groupSetWalk(kernel->groupSet, find_group_by_name, &helper);
    os_free(helper.groupName);
    return helper.found;
}


static void
v__dcHandleHistoricalData(v_durabilityClient _this, v_dataReaderSample sample)
{
    struct _DDS_HistoricalData *data = v__dcSampleGetHistoricalData(sample);
    v_message msg = v_dataReaderSampleTemplate(sample)->message;
    c_ulong i;
    struct _DDS_Gid_t myId = v__dcMyGid(_this);
    c_bool forMe = FALSE;

    {   /* To print the message */
        char tmpstr[30];
        if (data->content._d == HISTORICAL_DATA_KIND_LINK) {
            snprintf(tmpstr, sizeof(tmpstr), "L %u %u %u", data->content._u.link.sampleCount, data->content._u.link.completeness, data->content._u.link.errorCode);
        }
        V_DC_TRACE("%s - Sample %p msg %p state=%u time=%"PA_PRItime" wrgid=%u:%u:%u %s\n",
            OS_FUNCTION, (void*)sample, (void*)msg, msg->_parent.nodeState, OS_TIMEW_PRINT(msg->writeTime),
            msg->writerGID.systemId, msg->writerGID.localId, msg->writerGID.serial,
            (data->content._d == HISTORICAL_DATA_KIND_BEAD) ? "B" : ((data->content._d == HISTORICAL_DATA_KIND_LINK) ? tmpstr : "U"));
    }

    if (v_dataReaderSampleStateTest(sample, L_VALIDDATA) && !(v_dataReaderSampleInstanceStateTest(sample, L_DISPOSED))) {

        if ((data->serverId.prefix == myId.prefix) && (data->serverId.suffix == myId.suffix)) {
            V_DC_TRACE("%s -   sent by myself, IGNORE\n", OS_FUNCTION);
        } else if (c_sequenceSize(data->requestIds) == 0) {
            V_DC_TRACE("%s -   empty list of requestIds, IGNORE\n", OS_FUNCTION);
        } else {
            struct _DDS_RequestId_t request;
            struct _DDS_RequestId_t *requestIds = (struct _DDS_RequestId_t *)data->requestIds;
            /* The list of requestIds may not be empty */
            for (i=0; i<c_sequenceSize(data->requestIds) && (!forMe); i++) {
                request = requestIds[i];
                if ((request.clientId.prefix == myId.prefix) && (request.clientId.suffix == myId.suffix)) {
                    forMe = TRUE;
                }
            }
            if (forMe) {
                V_DC_TRACE("%s -   historicalData contains requestId (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32") and is for me, going to insert message\n",
                        OS_FUNCTION, request.clientId.prefix, request.clientId.suffix, request.requestId);
                v_dcInsertHistoricalData(_this, &request, msg, data);
            } else {
                V_DC_TRACE("%s -   historicalData is NOT for me\n", OS_FUNCTION);
            }
        }
    } else {
        /* TODO: historicalDataMessage is disposed, clean up all requests from this fellow */
    }
}


static void
v__dcHandleSubscription(v_durabilityClient _this, v_dataReaderSample sample)
{
    struct v_subscriptionInfo *info = v__dcSampleGetSubscriptionInfo(sample);
    c_ulong flag = 0;

    if (v_dataReaderSampleStateTest(sample, L_VALIDDATA)) {      /* FIXME: Not sure what to do with invalid samples, e.g., when splicedaemon disposes */
        /*
         * Check if this subscription update has to do with
         * a DurabilityStateRequest reader or HistoricalDataRequest
         * reader on a remote server node.
         */
        if (strcmp(info->topic_name, v__dcTopics[V_DC_TOPIC_STATE_REQUEST_ID].topicName) == 0) {
            flag = V_DC_STATE_REQUEST_FLAG;
        } else if (strcmp(info->topic_name, v__dcTopics[V_DC_TOPIC_DATA_REQUEST_ID].topicName) == 0) {
            flag = V_DC_DATA_REQUEST_FLAG;
        }

        if (flag) {
            c_sequence partitions = info->partition.name;
            c_ulong partitionsCnt = c_arraySize(partitions);
            c_ulong i;

            /* Check if this server request reader is on our request partition.  */
            for (i = 0; i < partitionsCnt; i++) {
                if (strcmp(partitions[i], _this->partitions[V_DC_PARTITION_REQUEST_ID]) == 0) {
                    break;
                }
            }
            if (i == partitionsCnt) {
                /* This is not related to our request partition:
                 * reset flag to ignore this server reader. */
                flag = 0;
                V_DC_TRACE("Server %u expects requests on partition '%s' whereas I publish requests on partition '%s', ignore this server\n",
                            info->participant_key.systemId,
                            (char*)info->partition.name[0],
                            _this->partitions[V_DC_PARTITION_REQUEST_ID]);
            }
        }

        /* Now handle the subscription message */
        if (flag) {
            if (v_dataReaderInstanceStateTest(v_readerSampleInstance(sample), L_DISPOSED)) {
                /* Unregister the server and delete if no readers are available anymore */
                (void)v_dcUnregisterServerByGid(_this, info->participant_key, flag);
            } else {
                /* Create a server if not exist and set the flag */
                (void)v_dcRegisterServerByGid(_this, info->participant_key, flag);
            }
        }
    }
}


static v_actionResult
v__dcKeepReaderSamples(
    c_object o,
    c_voidp arg)
{
    v_dataReaderSample s;
    c_iter samples = (c_iter)arg;

    assert(samples);

    if (o) {
        s = v_dataReaderSample(o);
        if (s != NULL) {
            (void)c_iterAppend(samples, c_keep(s));
        }
    }

    return V_PROCEED;
}


struct _DDS_Gid_t
convert_gid (v_gid gid)
{
    struct _DDS_Gid_t ddsGid;

    ddsGid.prefix = (((c_longlong)v_gidSystemId(gid)) << 32) | (c_longlong)gid.localId;
    ddsGid.suffix = v_gidLifecycleId(gid);
    return ddsGid;
}


static struct _DDS_Gid_t
v__dcMyGid(v_durabilityClient _this)
{
    v_gid vGid;

    assert(_this);

    vGid = v_kernel(v_object(_this)->kernel)->GID;
    return convert_gid(vGid);

}


static struct _DDS_VendorId_t
v__dcMyVendor(v_durabilityClient _this)
{
    struct _DDS_VendorId_t vendor = V_DC_VERSION_VENDOR_ID;
    OS_UNUSED_ARG(_this);
    return vendor;
}


static struct _DDS_DurabilityVersion_t
v__dcMyVersion(v_durabilityClient _this)
{
    struct _DDS_DurabilityVersion_t version;

    /* Set the client durability version.
     * Note that the version must be > 1.0 is timestamps beyond 2038 are used */
    version.major = V_DC_VERSION_MAJOR;
    version.minor = V_DC_VERSION_MINOR;
    version.vendorId = v__dcMyVendor(_this);
    return version;
}


static v_publisherQos
v__dcNewQosPublisher(v_kernel kernel, c_string partition)
{
    v_publisherQos pQos;

    assert(partition);

    pQos = v_publisherQosNew(kernel, NULL);
    if (pQos == NULL) {
        OS_REPORT(OS_ERROR,
                    "kernel::v_durabilityClient::v__dcNewQosPublisher", V_RESULT_INTERNAL_ERROR,
                    "Operation failed because v_publisherQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                    (os_address) kernel);
        return NULL;
    }

    c_free(pQos->partition.v); /* free default partition "" */
    pQos->partition.v = c_keep(partition);

    return pQos;
}


static v_subscriberQos
v__dcNewQosSubscriber(v_kernel kernel, c_string partition)
{
    v_subscriberQos sQos;

    assert(partition);

    sQos = v_subscriberQosNew(kernel, NULL);
    if (sQos == NULL) {
        OS_REPORT(OS_ERROR,
                    "kernel::v_durabilityClient::v__dcNewQosSubscriber", V_RESULT_INTERNAL_ERROR,
                    "Operation failed because v_subscriberQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                    (os_address) kernel);
        return NULL;
    }

    c_free(sQos->partition.v); /* free default partition "" */
    sQos->partition.v = c_keep(partition);

    return sQos;
}


static v_topicQos
v__dcNewQosTopic(v_kernel kernel)
{
    v_topicQos tQos;

    tQos = v_topicQosNew(kernel, NULL);
    if (tQos == NULL) {
        OS_REPORT (OS_ERROR,
                     "kernel::v_durabilityClient::v__dcNewQosTopic", V_RESULT_INTERNAL_ERROR,
                     "Operation failed because v_topicQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                     (os_address) kernel);
        return NULL;
    }
    tQos->history.v.kind = V_HISTORY_KEEPALL;
    tQos->history.v.depth = V_LENGTH_UNLIMITED;
    tQos->orderby.v.kind = V_ORDERBY_SOURCETIME;
    tQos->durability.v.kind  = V_DURABILITY_VOLATILE;
    tQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
    tQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);

    return tQos;
}


static v_readerQos
v__dcNewQosReader(v_kernel kernel)
{
    v_readerQos rQos;

    rQos = v_readerQosNew(kernel, NULL);
    if (rQos == NULL) {
        OS_REPORT (OS_ERROR,
                     "kernel::v_durabilityClient::v__dcNewQosReader", V_RESULT_INTERNAL_ERROR,
                     "Operation failed because v_readerQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                     (os_address) kernel);
        return NULL;
    }
    rQos->history.v.kind = V_HISTORY_KEEPALL;
    rQos->history.v.depth = V_LENGTH_UNLIMITED;
    rQos->orderby.v.kind = V_ORDERBY_SOURCETIME;
    rQos->durability.v.kind  = V_DURABILITY_VOLATILE;
    rQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
    rQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);

    return rQos;
}


static v_writerQos
v__dcNewQosWriter(v_kernel kernel)
{
    v_writerQos wQos;

    wQos = v_writerQosNew(kernel, NULL);
    if (wQos == NULL) {
        OS_REPORT (OS_ERROR,
                     "kernel::v_durabilityClient::v__dcNewQosWriter", V_RESULT_INTERNAL_ERROR,
                     "Operation failed because v_writerQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                     (os_address) kernel);
        return NULL;
    }
    wQos->history.v.kind = V_HISTORY_KEEPALL;
    wQos->history.v.depth = V_LENGTH_UNLIMITED;
    wQos->orderby.v.kind = V_ORDERBY_SOURCETIME;
    wQos->durability.v.kind  = V_DURABILITY_VOLATILE;
    wQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
    wQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
    wQos->lifecycle.v.autodispose_unregistered_instances = FALSE;

    return wQos;
}


static v_dataReader
v__dcNewBuiltinReader(v_kernel kernel, v_subscriber subscriber, int builtinId)
{
    v_dataReader dr = NULL;
    v_readerQos rQos;
    v_topic topic;

    assert(kernel);
    assert(subscriber);

    rQos = v_readerQosNew(kernel, NULL);
    if (rQos == NULL) {
        OS_REPORT (OS_ERROR,
                     "kernel::v_durabilityClient::v__dcNewBuiltinReader", V_RESULT_INTERNAL_ERROR,
                     "Operation failed because v_readerQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                     (os_address) kernel);
        return NULL;
    }
    rQos->durability.v.kind = V_DURABILITY_TRANSIENT;
    rQos->reliability.v.kind = V_RELIABILITY_RELIABLE;

    topic = v_builtinTopicLookup(kernel->builtin, builtinId);
    if (topic) {
        dr = v__dcNewReader(subscriber, topic, rQos);
    } else {
        OS_REPORT (OS_ERROR,
                     "kernel::v_durabilityClient::v__dcNewBuiltinReader", V_RESULT_INTERNAL_ERROR,
                     "Could not find builtin topic (%d)", builtinId);
    }

    v_readerQosFree(rQos);

    return dr;
}


static v_dataReader
v__dcNewReader(v_subscriber subscriber, v_topic topic, v_readerQos qos)
{
    v_dataReader dr = NULL;
    char name[64];
    char expr_str[64];
    q_expr expr_q;

    assert(subscriber);
    assert(topic);
    assert(qos);
    assert(v_entity(topic)->name);

    if (snprintf(name, sizeof(name), "reader <%s>", v_entity(topic)->name) >= (int)sizeof(name)) {
        assert (0);
    }
    if (snprintf(expr_str, sizeof(expr_str), "select * from %s", v_entity(topic)->name) >= (int)sizeof(expr_str)) {
        assert (0);
    }
    expr_q = q_parse(expr_str);
    dr = v_dataReaderNew(subscriber, name, expr_q, NULL, qos, FALSE);

    q_dispose(expr_q);

    return dr;
}


static v_writer
v__dcNewWriter(v_publisher publisher, v_topic topic, v_writerQos qos)
{
    v_writer dw = NULL;
    char name[64];

    assert(publisher);
    assert(topic);
    assert(qos);
    assert(v_entity(topic)->name);

    if (snprintf (name, sizeof(name), "writer <%s>", v_entity(topic)->name) >= (int)sizeof(name)) {
        assert (0);
    }
    dw = v_writerNew(publisher, name, topic, qos);

    return dw;
}

static void
v__dcReadData(v_durabilityClient _this, v_dataReader reader, v__dcDataAction action)
{
    v_result result;
    c_iter samples = c_iterNew(NULL);
    v_dataReaderSample sample;

    assert(_this);
    assert(reader);
    assert(action);

    if (samples) {
        result = v_dataReaderTake(reader, V_MASK_ANY, v__dcKeepReaderSamples, samples, OS_DURATION_ZERO);
        if (result == V_RESULT_OK) {
            for (sample = c_iterTakeFirst(samples); sample != NULL; sample = c_iterTakeFirst(samples)) {
                if (!_this->terminate) {
                    action(_this, sample);
                }
                c_free(sample);
            }
        } else if (result == V_RESULT_NO_DATA) {
            /* NOOP */
        } else {
            OS_REPORT (OS_ERROR,
                       "kernel::v_durabilityClient::v__dcReadMsg", result,
                       "Failed to take the message (%s).", v_resultImage(result));
        }
        c_iterFree(samples);
    } else {
        OS_REPORT (OS_ERROR,
                   "kernel::v_durabilityClient::v__dcReadMsg", V_RESULT_INTERNAL_ERROR,
                   "Failed to create samples list.");
    }
}


static c_bool
v__dcSendMsg(v_writer writer, v_message msg)
{
    c_bool written = TRUE;

    assert(msg);
    assert(writer);

    /* When calling v_writerWrite for the DurabilityStateWriter and
     * the HistoricalDataRequest writer the only possible return
     * values are V_WRITE_SUCCESS, V_WRITE_ERROR and V_WRITE_OUT_OF_RESOURCES.
     * Because there are no resource limits for these writer V_WRITE_REJECTED
     * and V_WRITE_TIMEOUT will not occur. */
    if (v_writerWrite(writer, msg, os_timeWGet(), NULL) != V_WRITE_SUCCESS) {
        OS_REPORT(OS_ERROR,
                      "kernel::v_durabilityClient::v_dcSendMsgDataRequest", 0,
                      "Unable to write a request");
        written = FALSE;
    }
    return written;
}

static struct _DDS_Time_t
ddsTimeFromTimeW (
    const os_timeW t,
    c_bool y2038ready)
{
    struct _DDS_Time_t t2;

    /* Depending on the Y2038READY setting legacy times are used or not */

    if (OS_TIMEW_ISINVALID(t)) {
        /* DDS_TIME_INVALID */
        t2.sec = (c_long)-1;
        t2.nanosec = (c_ulong)4294967295U;
    } else {
        assert(OS_TIMEW_ISNORMALIZED(t));
        if (y2038ready) {
            /* 64-bit time */
            t2.sec = (c_long)(OS_TIMEW_GET_VALUE(t) >> 32);
            t2.nanosec = (c_ulong)OS_TIMEW_GET_VALUE(t) & 0xFFFFFFFFU;
        } else {
            /* legacy time */
            c_time ct;

            ct = c_timeFromTimeW(t);
            t2.sec = ct.seconds;
            t2.nanosec = ct.nanoseconds;
        }
    }
    return t2;
}



c_bool
v_dcSendMsgDataRequest(
    v_durabilityClient _this,
    v_handle rhandle,
    c_iter partitions,
    c_char *topic,
    c_char* filter,
    c_iter params,
    os_timeW minSourceTime,
    os_timeW maxSourceTime,
    c_long max_samples,
    c_long max_instances,
    c_long max_samples_per_instance,
    c_iter alignmentPartition,
    os_duration timeout)
{
    struct _DDS_HistoricalDataRequest *data;
    c_bool send = FALSE;
    c_base base;
    v_message msg;
    c_iterIter iter;
    char *partition;
    v_group vgroup;

    assert(_this);
    assert(topic);

    base = c_getBase(_this);
    assert(base);
    /* A current limitation is that wildcards in partition/topics are
     * not supported. The reason is that it is currently impossible
     * to find out on what partition a bead was published, and
     * hence it is impossible to reconstruct in which group a bead
     * must be injected. Similarly, a sequence of more than one
     * partitions are currently not supported. In the latter case we will
     * generate a dedicated request for each individual partition. */
    if (c_iterLength(partitions) == 0) {
        OS_REPORT(OS_ERROR,
                      "kernel::v_durabilityClient::v_dcSendMsgDataRequest", 0,
                      "Unable to send message, empty partitions list");
        return FALSE;
    }
    iter = c_iterIterGet(partitions);
    while (((partition = (char *)c_iterNext(&iter)) != NULL)) {
        /* TODO: check that partition does not contain a wildcard because we cannot match locally */
        /* Only send HistoricalDataRequests for groups that locally exist */
        if ((vgroup = find_group(_this, partition, topic)) == NULL) {
             V_DC_TRACE("%s - No local vgroup found for %s.%s, not requesting data for this group\n", OS_FUNCTION, partition, topic);
             continue;
        }
        if ((msg = v_topicMessageNew(_this->topics[V_DC_TOPIC_DATA_REQUEST_ID])) == NULL) {
            OS_REPORT(OS_ERROR,
                      "kernel::v_durabilityClient::v_dcSendMsgDataRequest", 0,
                      "Failed to create message");
            c_free(vgroup);
            return FALSE;
        }

        /* Get data part of the message. */
        data = v__dcMessageGetHistoricalDataRequest(msg);

        /* Fill message. */
        data->requestId.clientId = v__dcMyGid(_this);
        data->requestId.requestId = ++_this->requestId;
        data->version = v__dcMyVersion(_this);
        {
            c_type subtype0;
            c_string *dst0;

            subtype0 = c_type(c_resolve(base, "c_string"));
            dst0 = (c_string *)c_sequenceNew_s(subtype0, 0, 1);
            c_free(subtype0);
            if (!dst0) {
                OS_REPORT (OS_ERROR, OS_FUNCTION, 0, "Member 'partitions' could not be allocated.");
                goto err_allocPartitions;
            }
            {
                dst0[0] = c_stringNew_s(base, partition);
            }
            data->partitions = (c_sequence)dst0;
        }
        data->topic = c_stringNew(base, topic);
        data->serializationFormat = (pa_getEndianNess() == pa_endianBig) ? PAYLOAD_SERIALIZATION_FORMAT_CDR_BE : PAYLOAD_SERIALIZATION_FORMAT_CDR_LE;
        data->startTime = ddsTimeFromTimeW(minSourceTime, _this->y2038ready);
        data->endTime = ddsTimeFromTimeW(maxSourceTime, _this->y2038ready);
        data->sqlFilter = (filter) ? c_stringNew(base, filter) : NULL;
        {
            c_ulong len = c_iterLength(params);
            c_type subtype0;
            c_string *dst0;

            subtype0 = c_type(c_resolve(base, "c_string"));
            dst0 = (c_string *)c_sequenceNew_s(subtype0, 0, len);
            c_free(subtype0);
            if (!dst0) {
                OS_REPORT (OS_ERROR, OS_FUNCTION, 0, "Member 'params' could not be allocated.");
                goto err_allocParams;
            }
            {
                c_iterIter iter = c_iterIterGet(params);
                char * param;
                c_ulong i = 0;

                while ((param = (char *)c_iterNext(&iter)) != NULL) {
                    dst0[i] = c_stringNew_s(base, param);
                    if (!dst0[i]) {
                        c_ulong j;
                        for (j=0; j<i; j++) c_free(dst0[j]);
                        goto err_allocParam;
                    }
                    i++;
                }
            }
            data->sqlFilterParams = (c_sequence)dst0;
        }
        data->maxSamples= max_samples;
        data->maxInstances = max_instances;
        data->maxSamplesPerInstance = max_samples_per_instance;
        {
            c_ulong len = c_iterLength(alignmentPartition);
            c_type subtype0;
            c_string *dst0;

            subtype0 = c_type(c_resolve(base, "c_string"));
            dst0 = (c_string *)c_sequenceNew_s(subtype0, 0, len);
            c_free(subtype0);
            if (!dst0) {
                OS_REPORT (OS_ERROR, OS_FUNCTION, 0, "Member 'alignmentPartition' could not be allocated.");
                goto err_allocAlignmentPartitions;
            }
            data->alignmentPartition = (c_sequence)dst0;
            {
                c_iterIter iter = c_iterIterGet(alignmentPartition);
                char * partition;
                c_ulong i = 0;

                while ((partition = (char *)c_iterNext(&iter)) != NULL) {
                    dst0[i] = c_stringNew_s(base, partition);
                    if (!dst0[i]) {
                        c_ulong j;
                        for (j=0; j<i; j++) c_free(dst0[j]);
                        goto err_allocAlignmentPartition;
                    }
                    i++;
                }
            }
        }
        data->timeout.sec = (c_long)OS_DURATION_GET_SECONDS(timeout);
        data->timeout.nanosec = (c_ulong)OS_DURATION_GET_NANOSECONDS(timeout);

        /* Administrate the request */
        if (v_dcRegisterHistoricalDataRequest(_this, rhandle, &data->requestId, vgroup, msg) == TRUE) {

            V_DC_TRACE("%s - Request (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32") created for topic '%s.%s' using alignment partition '%s'\n",
                       OS_FUNCTION, data->requestId.clientId.prefix, data->requestId.clientId.suffix, data->requestId.requestId, partition, topic,
                       (c_iterLength(alignmentPartition) == 0) ? "(null)" : (char *)c_iterObject(alignmentPartition, 0));

            send = v__dcSendMsg(_this->writers[V_DC_WRITER_DATA_REQUEST_ID], msg);
        }
        c_free(vgroup);
        c_free(msg);
    } /* while */
    return send;

err_allocAlignmentPartition:
err_allocAlignmentPartitions:
err_allocParam:
err_allocParams:
err_allocPartitions:
    c_free(msg);
    return FALSE;
}

static struct server_node *
v_dcRegisterServerByServerId(
    v_durabilityClient _this,
    struct _DDS_Gid_t serverId)
{
    ut_avlIPath_t ip;
    struct server_node *n = NULL;

    if ((n = ut_avlCLookupIPath (&servers_avltreedef, (ut_avlCTree_t *)_this->servers2, &serverId, &ip)) == NULL) {
        /* create an administration entry for this server */
        n = os_malloc (sizeof (*n));
        n->serverId.prefix = serverId.prefix;
        n->serverId.suffix = serverId.suffix;
        n->flags = 0;
        n->statesRequested = FALSE;
        /* Initialize the table to store the groups associated with this server */
        ut_avlCInit(&groups_avltreedef, &n->groups);
        ut_avlCInsertIPath (&groups_avltreedef, (ut_avlCTree_t *)_this->servers2, n, &ip);
        V_DC_TRACE("%s - server %p created with serverId (%"PA_PRId64".%"PA_PRId64")\n",
                    OS_FUNCTION, (void *)n, n->serverId.prefix, n->serverId.suffix);
    }
    return n;
}


static struct server_node *
v_dcRegisterServerByGid(
    v_durabilityClient _this,
    v_gid serverGid,
    c_ulong flag)
{
    struct _DDS_Gid_t serverId;
    struct server_node *n;

    serverId = convert_gid(serverGid);
    n = v_dcRegisterServerByServerId(_this, serverId);
    assert(n);
    n->flags |= flag;
    return n;
}


void
v_dcUnregisterServerByGid(
    v_durabilityClient _this,
    v_gid serverGid,
    c_ulong flag)
{
    struct server_node *n;
    struct _DDS_Gid_t serverId;

    serverId = convert_gid(serverGid);
    if ((n = ut_avlCLookup (&servers_avltreedef, (ut_avlCTree_t *)_this->servers2, &serverId)) != NULL) {
        if (v_stateTest(n->flags, flag)) {
            v_stateClear(n->flags, flag);
            if (n->flags == 0) {
                /* All readers lost, delete the server entry */
                V_DC_TRACE("%s - server %p with serverId (%"PA_PRId64".%"PA_PRId64") deleted\n",
                    OS_FUNCTION, (void *)n, n->serverId.prefix, n->serverId.suffix);
                ut_avlCDelete (&servers_avltreedef, (ut_avlCTree_t *)_this->servers2, n);
                cleanup_server(n);
            }
        }
    }
    return ;
}


static void
v__dcWaitsetAction(v_waitsetEvent e, c_voidp arg)
{
    v_durabilityClient _this = (v_durabilityClient)arg;
    v_durabilityClientEvent event =
            c_new(v_kernelType(v_objectKernel(_this), K_DURABILITYCLIENTEVENT));

    assert(e);
    assert(_this);
    assert(event);

    event->kind = e->kind;
    event->entity = e->source;
    event->eventData = c_keep(e->eventData);

    c_mutexLock(&_this->mutex);
    c_append(_this->queue, event);   /* keeps event */
    c_mutexUnlock(&_this->mutex);
    c_free(event);
}


static void
v__dcWaitsetSetup(v_durabilityClient _this)
{
    v_observerSetEventMask(v_observer(_this->waitset),
                           V_EVENT_DATA_AVAILABLE | V_EVENT_TERMINATE | V_EVENT_HISTORY_REQUEST);

    (void)v_observableAddObserver(v_observable(_this->participant), v_observer(_this->waitset), NULL);   /* to trigger the participant */
    (void)v_observableAddObserver(v_observable(_this->readers[V_DC_READER_STATE_ID]),            v_observer(_this->waitset), NULL);
    (void)v_observableAddObserver(v_observable(_this->readers[V_DC_READER_DATA_ID]),             v_observer(_this->waitset), NULL);
    (void)v_observableAddObserver(v_observable(_this->readers[V_DC_READER_SUBSCRIPTIONINFO_ID]), v_observer(_this->waitset), NULL);
}


/******************************************************************************
 * constructor/destructor
 ******************************************************************************/

v_durabilityClient
v_durabilityClientNew(
    v_spliced spliced,
    c_iter durablePolicies,
    const char* partitionRequest,
    const char* partitionDataGlobal,
    const char* partitionDataPrivate)
{
    v_durabilityClient _this  = NULL;
    v_participant participant = NULL;
    v_kernel kernel           = NULL;
    c_base   base             = NULL;

    V_DC_TRACE("%s\n", __FUNCTION__);

    assert(spliced);
    participant = v_participant(spliced);
    assert(participant);
    kernel = v_objectKernel(spliced);
    assert(kernel);
    base = c_getBase(kernel);
    assert(base);

    /*
     * Create v_durabilityClient object.
     */
    _this = v_durabilityClient(v_objectNew(kernel, K_DURABILITYCLIENT));
    if (_this == NULL) {
        OS_REPORT (OS_ERROR,
                   "kernel::v_durabilityClient::v_durabilityClientNew", V_RESULT_INTERNAL_ERROR,
                   "Failed to allocate v_durabilityClient object.");
        goto err_alloc;
    }
    _this->servers = c_tableNew(v_kernelType(kernel, K_DURABILITYCLIENTSERVER), "systemId");
    if (_this->servers  == NULL) {
        OS_REPORT (OS_ERROR,
                   "kernel::v_durabilityClient::v_durabilityClientNew", V_RESULT_INTERNAL_ERROR,
                   "Failed to create servers table.");
        c_free(_this);
        goto err_alloc;
    }
    _this->queue = c_listNew(v_kernelType(kernel, K_DURABILITYCLIENTEVENT));
    if (_this->queue == NULL) {
        OS_REPORT (OS_ERROR,
                   "kernel::v_durabilityClient::v_durabilityClientNew", V_RESULT_INTERNAL_ERROR,
                   "Failed to create events queue.");
        c_free(_this);
        goto err_alloc;
    }
    _this->requestId = 0;
    _this->chains = os_malloc(sizeof(ut_avlCTree_t));
    ut_avlCInit (&chains_avltreedef, (ut_avlCTree_t *)_this->chains);

    _this->rejected_chains = os_malloc(sizeof(ut_avlCTree_t));
    ut_avlCInit (&chains_avltreedef, (ut_avlCTree_t *)_this->rejected_chains);

    _this->servers2 = os_malloc(sizeof(ut_avlCTree_t));
    ut_avlCInit (&servers_avltreedef, (ut_avlCTree_t *)_this->servers2);

    _this->builtinSubscriber = c_keep(spliced->builtinSubscriber);
    _this->participant = c_keep(participant);
    _this->terminate = FALSE;

    if (c_mutexInit(base, &_this->mutex) != SYNC_RESULT_SUCCESS) {
        OS_REPORT (OS_ERROR,
                   "kernel::v_durabilityClient::v_durabilityClientNew", V_RESULT_INTERNAL_ERROR,
                   "Failed to initialize mutex.");
        c_free(_this);
        goto err_alloc;
    }

    _this->y2038ready = c_baseGetY2038Ready(base);

    _this->durablePolicies = (c_voidp)durablePolicies;
    _this->partitions[V_DC_PARTITION_REQUEST_ID]      = c_stringNew(base, partitionRequest);
    _this->partitions[V_DC_PARTITION_GLOBAL_DATA_ID]  = c_stringNew(base, partitionDataGlobal);
    _this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID] = c_stringNew(base, partitionDataPrivate);

    if (!v__dcCreateEntities(_this)) {
        OS_REPORT (OS_ERROR,
                   "kernel::v_durabilityClient::v_durabilityClientNew", V_RESULT_INTERNAL_ERROR,
                   "Failed to create durability client entities.");
        c_free(_this);
        goto err_alloc;
    }

    return _this;

err_alloc:
    assert(FALSE);
    return NULL;
}



static void
v_durabilityClientDeinit(
    v_durabilityClient _this)
{
    assert(_this);

    /* Clean up the resources associated with this thread */
    if (_this->chains) {
        ut_avlCFree(&chains_avltreedef, (ut_avlCTree_t *)_this->chains, cleanup_chain);
        _this->chains = NULL;
    }
    if (_this->rejected_chains) {
        ut_avlCFree(&chains_avltreedef, (ut_avlCTree_t *)_this->rejected_chains, cleanup_chain);
        _this->rejected_chains = NULL;
    }
    if (_this->servers2) {
        ut_avlCFree (&servers_avltreedef, (ut_avlCTree_t *)_this->servers2, cleanup_server);
        _this->servers2 = NULL;
    }
    /* Free all shared-memory attributes */
    c_free(_this);
}


void
v_durabilityClientFree(
    v_durabilityClient _this)
{
    assert(_this);

    v_durabilityClientDeinit(_this);
}


void
retry_rejected_chains(
    v_durabilityClient _this)
{
    struct chain_node *c;

    assert(_this);

    /* This function is called while walking over the chains (see checkChainComplete ).
     * We cannot use ut_avlCIterFirst() because the checkChainComplete()
     * deletes the chain and hence modifies the tree, thereby invalidating
     * the iterator. Instead, we will be using ut_avlCFindMin and ut_avlCFindSucc. */
    if (!ut_avlCIsEmpty(_this->rejected_chains)) {
        /* Retry rejected chains */
        c = ut_avlCFindMin (&chains_avltreedef, (ut_avlCTree_t *)_this->rejected_chains);
        while (c && (!_this->terminate)) {
            struct chain_node *next = ut_avlCFindSucc (&chains_avltreedef, (ut_avlCTree_t *)_this->rejected_chains, c);
            v__dcCheckChainComplete(_this, c, TRUE);
            c = next;
        }
    }
}

static char *
get_durable_policies_str(
    v_durabilityClient _this)
{
    char *result;

    assert(_this);

    if (_this->durablePolicies == NULL) {
        result = os_malloc(7);
        os_strcpy(result, "(null)");
    } else {
        struct durablePolicy *policy;
        c_iter durablePolicies = (c_iter)_this->durablePolicies;
        c_iterIter iter = c_iterIterGet(durablePolicies);

        result = os_malloc(1);
        strcpy(result, "\0");
        while ((policy = (struct durablePolicy *)c_iterNext(&iter)) != NULL) {
            char *tmp;
            tmp = os_malloc(strlen(result) + 1 /*   */ + 1 /* ( */ + strlen(policy->obtain) + 1 /* , */  + 5 /* TRUE, FALSE */ + 1 /* ) */);
            os_sprintf(tmp, "%s (%s,%s)", result, policy->obtain, policy->cache ? "TRUE" : "FALSE");
            os_free(result);
            result = tmp;
        }
    }
    return result;
}



/******************************************************************************
 * Public functions
 ******************************************************************************/

void
v_durabilityClientMain(v_durabilityClient _this)
{
    os_duration retry_time = OS_DURATION_INIT( 0, 100000000 );  /* 100 ms */
    os_duration timeout;

    V_DC_TRACE("%s [start]\n", __FUNCTION__);
    if (_this) {
        char *durablePoliciesStr = "";

        assert(C_TYPECHECK(_this, v_durabilityClient));

        /* Initialization should have been done. */
        assert(_this->partitions[V_DC_PARTITION_REQUEST_ID]);
        assert(_this->partitions[V_DC_PARTITION_GLOBAL_DATA_ID]);
        assert(_this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID]);

        /* Enable the readers and writers */
        v__dcEnable(_this);

        durablePoliciesStr = get_durable_policies_str(_this);

        V_DC_TRACE("%s - Request partition:      \"%s\"\n", OS_FUNCTION, _this->partitions[V_DC_PARTITION_REQUEST_ID]);
        V_DC_TRACE("%s - Global data partition:  \"%s\"\n", OS_FUNCTION, _this->partitions[V_DC_PARTITION_GLOBAL_DATA_ID]);
        V_DC_TRACE("%s - Private data partition: \"%s\"\n", OS_FUNCTION, _this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID]);
        V_DC_TRACE("%s - DurablePolicies:        %s\n", OS_FUNCTION, durablePoliciesStr);

        os_free(durablePoliciesStr);

        while (!_this->terminate) {
            /* Handle events in the queue. */
            v__dcEventDispatcher(_this);
            /* If there are still rejected messages then periodically try to inject them. */
            timeout = (ut_avlCIsEmpty(_this->rejected_chains)) ? retry_time : OS_DURATION_INFINITE;
            /* Wait for events or timeout. */
            if ((v_waitsetWait(_this->waitset, v__dcWaitsetAction, _this, timeout)) == V_RESULT_TIMEOUT) {
                if (!_this->terminate) {
                    retry_rejected_chains(_this);
                }
            }
        }
    }
    V_DC_TRACE("%s [stop]\n", __FUNCTION__);
}

void
v_durabilityClientTerminate(v_durabilityClient _this)
{
    v_observer o;
    C_STRUCT(v_event) term;

    V_DC_TRACE("%s\n", __FUNCTION__);

    if (_this) {
        assert(C_TYPECHECK(_this, v_durabilityClient));
        _this->terminate = TRUE;

        term.kind = V_EVENT_TERMINATE;
        term.source = v_observable(_this->publisher->participant);
        term.data = NULL;

        o = v_observer(_this->waitset);
        v_observerLock(o);
        v_observerNotify(o, &term, NULL);
        v_observerUnlock(o);
    }
}

void
v_durabilityClientLoadTypes(v_spliced spliced)
{
    int i;
    c_base base;
    v_kernel kernel;

    V_DC_TRACE("%s\n", __FUNCTION__);

    assert(spliced);
    kernel = v_objectKernel(spliced);
    assert(kernel);
    base = c_getBase(kernel);
    assert(base);

    for (i=0; i < (int)(sizeof(v__dcDataTypes) / sizeof(*v__dcDataTypes)); i++) {
        /* Register the data type. */
        if (!v__dcDataTypes[i].loadType(base)) {
            OS_REPORT(OS_ERROR,
                      "kernel::v_durabilityClient::v_durabilityClientLoadTypes", V_RESULT_INTERNAL_ERROR,
                      "Failed to load %s type\n", v__dcDataTypes[i].info);
        }
    }
}


c_bool
v_durabilityClientIsResponsibleForAlignment(
    v_durabilityClient _this,
    char *partition,
    char *topic,
    c_bool *cache)
{
    c_bool result = FALSE;
    c_iter durablePolicies;
    c_iterIter iter;

    assert(_this);

    /* Verify if a durablePolicy matches the partition and topic */
    durablePolicies = (c_iter)_this->durablePolicies;
    iter = c_iterIterGet(durablePolicies);
    if (c_iterLength(durablePolicies) != 0) {
        char *str;
        c_value p, q, r;
        struct durablePolicy *policy;

        str = os_malloc(strlen(partition) + strlen(topic) + 2);
        os_sprintf(str, "%s.%s", partition, topic);
        p.kind = q.kind = V_STRING;
        while (((policy = (struct durablePolicy *)c_iterNext(&iter)) != NULL) && (!result)) {
            p.is.String = (char *) policy->obtain;
            q.is.String = (char *) str;
            r = c_valueStringMatch (p, q);
            result = r.is.Boolean;
            if (result) {
                V_DC_TRACE("%s - Group %s matches durable policy pattern %s with cache=%d\n", OS_FUNCTION, str, policy->obtain, policy->cache);
                *cache = policy->cache;
            }
        }
        os_free(str);
    }
    return result;
}


c_bool
v_dcRegisterPartitionTopicState(
    v_durabilityClient _this,
    struct server_node *n,
    struct _DDS_PartitionTopicState_t *state)
{
    ut_avlIPath_t ip;
    c_bool created = FALSE;
    struct group_node *g;

    OS_UNUSED_ARG(_this);

    if ((g = ut_avlCLookupIPath (&groups_avltreedef, &n->groups, state, &ip)) == NULL) {
        /* The group does not yet exist for this server */
        g = os_malloc(sizeof(*g));
        g->state = os_malloc(sizeof(*state));
        g->state->partition = os_strdup(state->partition);
        g->state->topic = os_strdup(state->topic);
        g->state->completeness = state->completeness;
        V_DC_TRACE("%s - group %p created at server %p for %s.%s and completeness %d\n",
                    OS_FUNCTION, (void *)g, (void *)n, state->partition, state->topic, state->completeness);
        ut_avlCInsertIPath (&groups_avltreedef, &n->groups, g, &ip);
        created = TRUE;
    } else if (g->state->completeness != state->completeness) {
            V_DC_TRACE("%s - server group %p for %s.%s changed completeness state from %d to %d\n",
                        OS_FUNCTION, (void *)g, state->partition, state->topic, g->state->completeness, state->completeness);
            g->state->completeness = state->completeness;
    }
    return created;
}


/* Verify if the reader associated with the groupEntry is non-volatile */
static c_bool
has_non_volatile_reader(v_groupEntry groupEntry, c_voidp arg)
{
    c_bool *interest = (c_bool *)arg;

    if (!*interest) {
        v_reader r;

        r = v_reader(v_dataReaderEntry(groupEntry->entry));
        if (r) {
            if (r->qos) {
                *interest = (r->qos->durability.v.kind > V_DURABILITY_VOLATILE);
            }
        }
    }
    return TRUE;
}


/* Update groups with the provided state information */
static void
v_dcUpdateDurabilityState(
    v_durabilityClient _this,
    struct _DDS_RequestId_t *requestId,
    v_message msg,
    struct _DDS_DurabilityState *data,
    struct server_node *n)
{
    c_ulong i;
    v_kernel kernel;
    struct _DDS_PartitionTopicState_t state;
    struct _DDS_PartitionTopicState_t *dataStates = (struct _DDS_PartitionTopicState_t *)data->dataState;

    OS_UNUSED_ARG(msg);
    OS_UNUSED_ARG(requestId);

    kernel = v_objectKernel(_this);
    /* Retrieve the data states published by the server */
    for(i=0; i<c_sequenceSize(data->dataState); i++) {
        v_group vgroup;
        c_bool interest = FALSE;
        c_bool cache;

        state = dataStates[i];
        assert(state.partition);
        assert(state.topic);
        V_DC_TRACE("%s - PartitionTopic state '%s.%s' (completeness: %d) received from server %p\n",
                  OS_FUNCTION, state.partition, state.topic, state.completeness, (void *)n);
        if (!v_durabilityClientIsResponsibleForAlignment(_this, state.partition, state.topic, &cache)) {
            V_DC_TRACE("%s   - No match with my DurableProperties, ignore the PartitionTopic state\n",
                      OS_FUNCTION);
            return;
        }
        /* Register the server's PartitionTopic state */
        v_dcRegisterPartitionTopicState(_this, n, &state);
        /* No need to request historical data if no matching local group exists.
         * Note that in case the client wants to acquire ALL data from  all groups
         * then the client should create a local group and request data anyway. */
        if ((vgroup = v_groupSetGet(kernel->groupSet, state.partition, state.topic)) == NULL) {
            /* The client will NOT request historical data for group that are not known locally. */
            V_DC_TRACE("%s   - No local reader for '%s.%s', so no need to request historical data\n",
                      OS_FUNCTION, state.partition, state.topic);
            return;
        }
        /* No need to request data if the group is already locally complete  */
        if (v_groupCompleteGet(vgroup)) {
            V_DC_TRACE("%s   - No need to request historical data for '%s.%s' because the local group is already complete\n",
                      OS_FUNCTION, state.partition, state.topic);
            return;
        }
        /* No need to do anything if the server does not have the PartitionTopic state complete */
        if (state.completeness != COMPLETENESS_COMPLETE) {
            V_DC_TRACE("%s   - Server %p does not have PartitionTopic state '%s.%s' complete yet, ignoring\n",
                      OS_FUNCTION, (void *)n, state.partition, state.topic);
            return;
        }
        /* Now iterate over the associated readers and retrieve all
         * non-volatile readers that are interested in historical data */
        (void)v_groupEntrySetWalk(&vgroup->topicEntrySet, has_non_volatile_reader, &interest);
        if (!interest) {
            V_DC_TRACE("%s   - There are no non-volatile readers yet interested in historical data for '%s.%s', ignoring\n",
                      OS_FUNCTION, state.partition, state.topic);
            return;
        }

        /* There are non-volatile readers that are are interested in historical data
         * but the group is not complete. We are going to send a historicalDataRequest in
         * that case.
         */
        {
            c_iter ps =  c_iterNew(state.partition);
            c_iter filterParams = c_iterNew(NULL);
            c_iter alignmentPartition = c_iterNew(_this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID]);
            os_duration timeout = OS_DURATION_INIT(0, 100000000);

            V_DC_TRACE("%s   - Sending HistoricalDataRequest for %s.%s using alignment partition %s\n",
                    OS_FUNCTION, state.partition, state.topic, _this->partitions[V_DC_PARTITION_PRIVATE_DATA_ID]);

            v_dcSendMsgDataRequest(
                    _this,
                    V_HANDLE_NIL,
                    ps,
                    state.topic,
                    NULL,
                    filterParams,
                    OS_TIMEW_INVALID,
                    OS_TIMEW_INVALID,
                    -1,
                    -1,
                    -1,
                    alignmentPartition,
                    timeout);
            c_iterFree(ps);
            c_iterFree(filterParams);
            c_iterFree(alignmentPartition);
        }
    }
    return;
}

