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

#ifndef D__ADMIN_H
#define D__ADMIN_H

#include "d__types.h"
#include "d__lock.h"
#include "d__statistics.h"
#include "d__conflictMonitor.h"
#include "d__client.h"
#include "u_user.h"
#include "os_cond.h"
#include "os_mutex.h"
#include "c_iterator.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \brief Durability topic definitions
 */
#define D_GROUPS_REQ_TOPIC_NAME              "d_groupsRequest"
#define D_GROUPS_REQ_TYPE_NAME               D_MODULE_NAME"::d_groupsRequest_s"
#define D_GROUPS_REQ_TOP_NAME                "v_message<" D_GROUPS_REQ_TYPE_NAME ">"
#define D_GROUPS_REQ_KEY_LIST                "parentMsg.senderAddress.systemId"

#define D_SAMPLE_REQ_TOPIC_NAME              "d_sampleRequest"
#define D_SAMPLE_REQ_TYPE_NAME               D_MODULE_NAME"::d_sampleRequest_s"
#define D_SAMPLE_REQ_TOP_NAME                "v_message<" D_SAMPLE_REQ_TYPE_NAME ">"
#define D_SAMPLE_REQ_KEY_LIST                "parentMsg.senderAddress.systemId"

#define D_STATUS_TOPIC_NAME                  "d_status"
#define D_STATUS_TYPE_NAME                   D_MODULE_NAME"::d_status_s"
#define D_STATUS_TOP_NAME                    "v_message<" D_STATUS_TYPE_NAME ">"
#define D_STATUS_KEY_LIST                    "parentMsg.senderAddress.systemId"

#define D_NEWGROUP_TOPIC_NAME                "d_newGroup"
#define D_NEWGROUP_TYPE_NAME                 D_MODULE_NAME"::d_newGroup_s"
#define D_NEWGROUP_TOP_NAME                  "v_message<" D_NEWGROUP_TYPE_NAME ">"
#define D_NEWGROUP_KEY_LIST                  "parentMsg.senderAddress.systemId"

#define D_SAMPLE_CHAIN_TOPIC_NAME            "d_sampleChain"
#define D_SAMPLE_CHAIN_TYPE_NAME             D_MODULE_NAME"::d_sampleChain_s"
#define D_SAMPLE_CHAIN_TOP_NAME              "v_message<" D_SAMPLE_CHAIN_TYPE_NAME ">"
#define D_SAMPLE_CHAIN_KEY_LIST              "parentMsg.senderAddress.systemId"

#define D_NAMESPACES_TOPIC_NAME              "d_nameSpaces"
#define D_NAMESPACES_TYPE_NAME               D_MODULE_NAME"::d_nameSpaces_s"
#define D_NAMESPACES_TOP_NAME                "v_message<" D_NAMESPACES_TYPE_NAME ">"
#define D_NAMESPACES_KEY_LIST                "parentMsg.senderAddress.systemId"

#define D_NAMESPACES_REQ_TOPIC_NAME          "d_nameSpacesRequest"
#define D_NAMESPACES_REQ_TYPE_NAME           D_MODULE_NAME"::d_nameSpacesRequest_s"
#define D_NAMESPACES_REQ_TOP_NAME            "v_message<" D_NAMESPACES_REQ_TYPE_NAME ">"
#define D_NAMESPACES_REQ_KEY_LIST            "parentMsg.senderAddress.systemId"

#define D_DELETE_DATA_TOPIC_NAME             "d_deleteData"
#define D_DELETE_DATA_TYPE_NAME              D_MODULE_NAME"::d_deleteData_s"
#define D_DELETE_DATA_TOP_NAME               "v_message<" D_DELETE_DATA_TYPE_NAME ">"
#define D_DELETE_DATA_KEY_LIST               "parentMsg.senderAddress.systemId"

#define D_DURABILITY_STATE_REQUEST_TOPIC_NAME "d_durabilityStateRequest"
#define D_DURABILITY_STATE_REQUEST_TYPE_NAME  "DDS::DurabilityStateRequest"
#define D_DURABILITY_STATE_REQUEST_TOP_NAME   "v_message<" D_DURABILITY_STATE_REQUEST_TYPE_NAME ">"
#define D_DURABILITY_STATE_REQUEST_KEY_LIST   "requestId.clientId.prefix,requestId.clientId.suffix"

#define D_DURABILITY_STATE_TOPIC_NAME         "d_durabilityState"
#define D_DURABILITY_STATE_TYPE_NAME          "DDS::DurabilityState"
#define D_DURABILITY_STATE_TOP_NAME           "v_message<" D_DURABILITY_STATE_TYPE_NAME ">"
#define D_DURABILITY_STATE_KEY_LIST           "serverId.prefix,serverId.suffix"

#define D_HISTORICAL_DATA_REQUEST_TOPIC_NAME "d_historicalDataRequest"
#define D_HISTORICAL_DATA_REQUEST_TYPE_NAME  "DDS::HistoricalDataRequest"
#define D_HISTORICAL_DATA_REQUEST_TOP_NAME   "v_message<" D_HISTORICAL_DATA_REQUEST_TYPE_NAME ">"
#define D_HISTORICAL_DATA_REQUEST_KEY_LIST   "requestId.clientId.prefix,requestId.clientId.suffix"

#define D_HISTORICAL_DATA_TOPIC_NAME         "d_historicalData"
#define D_HISTORICAL_DATA_TYPE_NAME          "DDS::HistoricalData"
#define D_HISTORICAL_DATA_TOP_NAME           "v_message<" D_HISTORICAL_DATA_TYPE_NAME ">"
#define D_HISTORICAL_DATA_KEY_LIST           "serverId.prefix,serverId.suffix"

#define D_CAPABILITY_TOPIC_NAME              "d_capability"
#define D_CAPABILITY_TYPE_NAME               D_MODULE_NAME"::d_capability_s"
#define D_CAPABILITY_TOP_NAME                "v_message<" D_CAPABILITY_TYPE_NAME ">"
#define D_CAPABILITY_KEY_LIST                "parentMsg.senderAddress.systemId"


/**
 * \brief Flags to indicate which mutexes and conditions are initialized
 */
#define D__INIT_FLAG_NONE                                        (0u)
#define D__INIT_FLAG_EVENT_MUTEX                              (1u<<0)
#define D__INIT_FLAG_EVENT_COND                               (1u<<1)
#define D__INIT_FLAG_EVENT_THREAD                             (1u<<2)
#define D__INIT_FLAG_CONFLICTQUEUE_MUTEX                      (1u<<3)
#define D__INIT_FLAG_SEQNUM_MUTEX                             (1u<<4)

/* Sequence numbers.
 * Sequence numbers are encoded in the seconds field of the
 * productionTimestamp if bit 30 of the nanoseconds field is
 * set.
 */
#define D_MAX_VALID_SEQNUM        0xFFFFFFFEul
#define D_SEQNUM_INFINITE         0xFFFFFFFFul
#define D_SEQNUM_ISVALID(seqnum)  ((seqnum) <= D_MAX_VALID_SEQNUM)

/**
 * Macro that checks the d_admin validity.
 * Because d_admin is a concrete class typechecking is required.
 */
#define             d_adminIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_ADMIN)

/**
 * Macro that checks the d_adminEvent validity.
 * Because d_adminEvent is a concrete class typechecking is required.
 */
#define             d_adminEventIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_ADMIN_EVENT)

/**
 * \brief The d_admin cast macro.
 *
 * This macro casts an object to a d_admin object.
 */
#define d_admin(_this) ((d_admin)(_this))

/**
 * \brief The d_adminEvent cast macro.
 *
 * This macro casts an object to a d_adminEvent object.
 */
#define d_adminEvent(_this) ((d_adminEvent)(_this))

C_STRUCT(d_admin){
    C_EXTENDS(d_lock);
    d_durability        durability;
    d_table             unconfirmedFellows;
    d_table             fellows;
    d_table             clients;
    d_table             groups;
    d_table             readerRequests;
    d_networkAddress    myAddress;
    c_ulong             alignerGroupCount;
    d_fellow            cachedFellow;

    d_publisher         publisher;
    d_subscriber        subscriber;

    u_topic             groupsRequestTopic;
    u_topic             sampleRequestTopic;
    u_topic             statusTopic;
    u_topic             newGroupTopic;
    u_topic             sampleChainTopic;
    u_topic             nameSpacesTopic;
    u_topic             nameSpacesRequestTopic;
    u_topic             deleteDataTopic;
    u_topic             durabilityStateRequestTopic;
    u_topic             durabilityStateTopic;
    u_topic             historicalDataRequestTopic;
    u_topic             historicalDataTopic;
    u_topic             capabilityTopic;

    d_actionQueue       actionQueue;

    os_mutex            eventMutex;
    c_iter              eventListeners;
    c_iter              eventQueue;
    os_cond             eventCondition;
    os_threadId         eventThread;
    c_bool              eventThreadTerminate;

    c_iter              nameSpaces;

    c_ulong             initMask;             /* Mask to indicate which mutexes have been initialized */
    d_table             terminateFellows;     /* Table of fellows that have recently terminated */

    c_iter              conflictQueue;        /* Detected conflicts */
    os_mutex            conflictQueueMutex;

    d_conflictMonitor   conflictMonitor;
    d_conflictResolver  conflictResolver;

    os_uint32           seqnum;               /* durability service specific message sequence number */
    os_mutex            seqnumMutex;          /* Mutex to automically increment the sequence number */

    d_table             initial_fellows;       /* The list of fellows used when resolving an initial conflict */
};

C_CLASS(d_adminEvent);

C_STRUCT(d_adminEvent){
    C_EXTENDS(d_object);
    c_ulong  event;
    d_fellow fellow;
    d_nameSpace nameSpace;
    d_group  group;
    c_voidp userData;
};

struct cleanupData{
    c_iter fellows;
    d_durability durability;
};

struct sendData{
    d_admin admin;
    d_configuration configuration;
    d_networkAddress addressee;
};

struct masterCountForRoleHelper
{
    d_name role;
    d_nameSpace nameSpace;
    c_ulong masterCount;
};

struct collectMatchingGroupsHelper {
    d_admin admin;
    char *topic;            /* topic to match */
    c_iter partitions;      /* list of partition expressions to match */
    d_table matchingGroups; /* table of collected matching groups */
    c_bool forMe;           /* originating request was addressed to me */
    c_bool forEverybody;    /* originating request was addressed to everybody */
    c_bool isAligner;       /* Indicates if I am an aligner for one of the matching partition/topic expressions */
    c_bool isResponsible;   /* Indicates if I am responsible for alignment */
    c_bool masterKnown;     /* Indicates if a master confirmed master is selected */
    c_bool groupFound;      /* Indicates if a group is found for which this master is responsible */
    c_bool isHistoricalDataRequest;  /* Indicates if the request was a historicalDataRequest or a DurabilityStateRequest */
    os_uint64 seqnum;              /* durability service specific message sequence number */
};


void                    d_adminUpdateStatistics                 (d_admin admin,
                                                                 d_adminStatisticsInfo statistics);

d_admin                 d_adminNew                              (d_durability durability);

void                    d_adminDeinit                           (d_admin admin);

void                    d_adminFree                             (d_admin admin);

void                    d_adminInitSubscriber                   (d_admin admin);

d_durability            d_adminGetDurability                    (d_admin admin);

d_publisher             d_adminGetPublisher                     (d_admin admin);

d_subscriber            d_adminGetSubscriber                    (d_admin admin);

c_bool                  d_adminAddLocalGroup                    (d_admin admin,
                                                                 d_group group);

d_group                 d_adminGetLocalGroup                    (d_admin admin,
                                                                 const c_char* partition,
                                                                 const c_char* topic,
                                                                 d_durabilityKind kind);

c_ulong                 d_adminGetAlignerGroupCount             (d_admin admin);

void                    d_adminGroupWalk                        (d_admin admin,
                                                                 c_bool ( * action ) (
                                                                    d_group group,
                                                                    c_voidp userData),
                                                                 c_voidp args);

d_fellow                d_adminGetUnconfirmedFellow             (d_admin admin,
                                                                 d_networkAddress address);

d_fellow                d_adminGetFellow                        (d_admin admin,
                                                                 d_networkAddress address);

d_fellow                d_adminAddFellow                        (d_admin admin,
                                                                 d_fellow fellow);

d_fellow                d_adminRemoveFellow                     (d_admin admin,
                                                                 d_fellow fellow,
                                                                 c_bool trackFellow);

void                    d_adminAsymRemoveFellow                 (d_admin admin,
                                                                 d_fellow fellow,
                                                                 c_bool trackFellow);

d_networkAddress        d_adminGetMyAddress                     (d_admin admin);

c_bool                  d_adminFellowWalk                       (d_admin admin,
                                                                 c_bool ( * action ) (
                                                                    d_fellow fellow,
                                                                    c_voidp userData),
                                                                 c_voidp userData);

c_bool                  d_adminInitialFellowWalk                (d_admin admin,
                                                                 c_bool ( * action ) (
                                                                    d_fellow fellow,
                                                                    c_voidp userData),
                                                                 c_voidp userData);

c_ulong                 d_adminGetFellowCount                   (d_admin admin);

c_ulong                 d_adminGetInitialFellowCount            (d_admin admin);

void                    d_adminCleanupTerminateFellows          (d_admin admin);

void                    d_adminAddNameSpace                     (d_admin admin,
                                                                 d_nameSpace nameSpace);

void                    d_adminNameSpaceWalk                    (d_admin admin,
                                                                 void (*action)(
                                                                     d_nameSpace nameSpace,
                                                                     c_voidp userData),
                                                                 c_voidp userData);

c_iter                  d_adminNameSpaceCollect                 (d_admin admin);
void                    d_adminNameSpaceCollectFree             (d_admin admin, c_iter nameSpaces);

c_ulong                 d_adminGetNameSpacesCount               (d_admin admin);

d_nameSpace             d_adminGetNameSpaceForGroupNoLock       (d_admin admin,
                                                                 d_partition partition,
                                                                 d_topic topic);

d_nameSpace             d_adminGetNameSpaceForGroup             (d_admin admin,
                                                                 d_partition partition,
                                                                 d_topic topic);

c_bool                  d_adminInNameSpace                      (d_nameSpace ns,
                                                                 d_partition partition,
                                                                 d_topic topic,
                                                                 c_bool aligner);

c_bool                  d_adminGroupInInitialAligneeNS          (d_admin admin,
                                                                 d_partition partition,
                                                                 d_topic topic);

/**
 * Determines whether the supplied partition-topic combination is in the
 * aligner namespace for persistent data.
 *
 * @param admin The admin that contains the namespaces.
 * @param partition The partition to check.
 * @param topic The topic to check.
 * @return TRUE if it is in the namespace, FALSE otherwise.
 */
c_bool                  d_adminGroupInAlignerNS                 (d_admin admin,
                                                                 d_partition partition,
                                                                 d_topic topic);
/**
 * Determines whether the supplied partition-topic combination is in the
 * alignee namespace for non-volatile data and the
 * alignmentKind != D_ALIGNMENT_ON_REQUEST.
 *
 * @param admin The admin that contains the namespaces.
 * @param partition The partition to check.
 * @param topic The topic to check.
 * @param kind The durability kind.
 * @return TRUE if it is in the namespace, FALSE otherwise.
 */
c_bool                  d_adminGroupInActiveAligneeNS           (d_admin admin,
                                                                 d_partition partition,
                                                                 d_topic topic);
/**
 * Determines whether the supplied partition-topic combination is in the
 * alignee namespace for non-volatile data.
 *
 * @param admin The admin that contains the namespaces.
 * @param partition The partition to check.
 * @param topic The topic to check.
 * @param kind The durability kind.
 * @return TRUE if it is in the namespace, FALSE otherwise.
 */
c_bool                  d_adminGroupInAligneeNS                 (d_admin admin,
                                                                 d_partition partition,
                                                                 d_topic topic);

u_topic                 d_adminGetStatusTopic                   (d_admin admin);

u_topic                 d_adminGetNewGroupTopic                 (d_admin admin);

u_topic                 d_adminGetGroupsRequestTopic            (d_admin admin);

u_topic                 d_adminGetStatusRequestTopic            (d_admin admin);

u_topic                 d_adminGetSampleRequestTopic            (d_admin admin);

u_topic                 d_adminGetSampleChainTopic              (d_admin admin);

u_topic                 d_adminGetNameSpacesTopic               (d_admin admin);

u_topic                 d_adminGetNameSpacesRequestTopic        (d_admin admin);

u_topic                 d_adminGetDeleteDataTopic               (d_admin admin);

u_topic                 d_adminGetDurabilityStateRequestTopic   (d_admin admin);

u_topic                 d_adminGetDurabilityStateTopic          (d_admin admin);

u_topic                 d_adminGetHistoricalDataRequestTopic    (d_admin admin);

u_topic                 d_adminGetHistoricalDataTopic           (d_admin admin);

u_topic                 d_adminGetCapabilityTopic               (d_admin admin);

c_bool                  d_adminAreLocalGroupsComplete           (d_admin admin,
                                                                 c_bool report);

void                    d_adminAddListener                      (d_admin admin,
                                                                 d_eventListener listener);

void                    d_adminRemoveListener                   (d_admin admin,
                                                                 d_eventListener listener);

void                    d_adminNotifyListeners                  (d_admin admin,
                                                                 c_ulong mask,
                                                                 d_fellow fellow,
                                                                 d_nameSpace nameSpace,
                                                                 d_group group,
                                                                 c_voidp userData);

d_actionQueue           d_adminGetActionQueue                   (d_admin admin);

c_ulong                 d_adminGetIncompatibleStateCount        (d_admin admin);

c_ulong                 d_adminGetIncompatibleDataModelCount    (d_admin admin);

c_bool                  d_adminAddReaderRequest                 (d_admin admin,
                                                                 d_readerRequest request);

c_bool                  d_adminRemoveReaderRequest              (d_admin admin,
                                                                 d_networkAddress source);

d_readerRequest         d_adminGetReaderRequest                 (d_admin admin,
                                                                 d_networkAddress source);

c_bool                  d_adminCheckReaderRequestFulfilled      (d_admin admin,
                                                                 d_readerRequest request);

d_nameSpace             d_adminGetNameSpace                     (d_admin admin,
                                                                 os_char* name);

void                    d_adminReportMaster                     (d_admin admin,
                                                                 d_fellow fellow,
                                                                 d_nameSpace nameSpace);

void                    d_adminReportDelayedInitialSet          (d_admin admin,
                                                                 d_nameSpace nameSpace,
                                                                 d_fellow fellow);

c_bool                  d_nameSpaceCountMastersForRoleWalk      (d_fellow fellow,
                                                                 void* userData);

u_topic                 d_adminInitTopic                        (d_admin admin,
                                                                 const c_char* topicName,
                                                                 const c_char* typeName,
                                                                 const c_char* keyList,
                                                                 v_reliabilityKind reliability,
                                                                 v_historyQosKind historyKind,
                                                                 v_orderbyKind orderKind,
                                                                 c_long historyDepth);

void                    d_adminInitAddress                      (v_public entity,
                                                                 c_voidp args);

c_bool                  d_adminSendLocalGroupsAction            (d_group group,
                                                                 c_voidp userData);

d_adminEvent            d_adminEventNew                         (c_ulong event,
                                                                 d_fellow fellow,
                                                                 d_nameSpace nameSpace,
                                                                 d_group group,
                                                                 c_voidp userData);

void                    d_adminEventDeinit                      (d_adminEvent event);

void                    d_adminEventFree                        (d_adminEvent event);

void*                   d_adminEventThreadStart                 (void* arg);

void                    d_adminCollectMatchingGroups            (d_admin admin,
                                                                 c_voidp arg);

char *                  d_adminGetStaticFilterExpression        (d_admin admin,
                                                                 char *partition,
                                                                 char *topic);

d_client                d_adminGetClient                        (d_admin admin,
                                                                 d_networkAddress address);

d_client                d_adminAddClient                        (d_admin admin,
                                                                 d_client client);

d_client                d_adminRemoveClient                     (d_admin admin,
                                                                 d_client client);

d_client                d_adminFindClientByClientId             (d_admin admin,
                                                                 struct _DDS_Gid_t clientId);

os_uint32               d_adminGetNextSeqNum                    (d_admin admin);

c_iter                  d_adminGetNameSpaces                    (d_admin admin);

void                    d_admin_sync_mergeStates                (d_admin admin);

void                    d_adminReportGroup                      (d_admin admin,d_group group);

/* Marks all groups that fall within a namespace of durability complete/incomplete. */
void                    d_adminMarkNameSpaceKernelGroupsCompleteness  (_Inout_ d_admin admin,
                                                                 _In_ c_bool complete);

void                    d_adminStoreGroup                       (d_admin admin, d_group group);

void                    d_adminSetConflictResolver              (d_admin admin, d_conflictResolver conflictResolver);

void                    d_adminInitialFellowsCreate             (d_admin admin);

void                    d_adminInitialFellowsDestroy            (d_admin admin);

char *                  d_adminGetInitialFellowsString          (d_admin admin);

#if defined (__cplusplus)
}
#endif

#endif /* D__ADMIN_H */
