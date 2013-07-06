/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef D_ADMIN_H
#define D_ADMIN_H

#include "d__types.h"
#include "u_user.h"
#include "d__statistics.h"
#include "d_lock.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_admin(f) ((d_admin)(f))

#define D_STATUS_REQ_TOPIC_NAME     "d_statusRequest"
#define D_STATUS_REQ_TYPE_NAME      D_MODULE_NAME"::d_statusRequest_s"
#define D_STATUS_REQ_TOP_NAME       "v_message<" D_STATUS_REQ_TYPE_NAME ">"
#define D_STATUS_REQ_KEY_LIST       "parentMsg.senderAddress.systemId"


#define D_GROUPS_REQ_TOPIC_NAME     "d_groupsRequest"
#define D_GROUPS_REQ_TYPE_NAME      D_MODULE_NAME"::d_groupsRequest_s"
#define D_GROUPS_REQ_TOP_NAME       "v_message<" D_GROUPS_REQ_TYPE_NAME ">"
#define D_GROUPS_REQ_KEY_LIST       "parentMsg.senderAddress.systemId"

#define D_SAMPLE_REQ_TOPIC_NAME     "d_sampleRequest"
#define D_SAMPLE_REQ_TYPE_NAME      D_MODULE_NAME"::d_sampleRequest_s"
#define D_SAMPLE_REQ_TOP_NAME       "v_message<" D_SAMPLE_REQ_TYPE_NAME ">"
#define D_SAMPLE_REQ_KEY_LIST       "parentMsg.senderAddress.systemId"

#define D_STATUS_TOPIC_NAME         "d_status"
#define D_STATUS_TYPE_NAME          D_MODULE_NAME"::d_status_s"
#define D_STATUS_TOP_NAME           "v_message<" D_STATUS_TYPE_NAME ">"
#define D_STATUS_KEY_LIST           "parentMsg.senderAddress.systemId"

#define D_NEWGROUP_TOPIC_NAME       "d_newGroup"
#define D_NEWGROUP_TYPE_NAME        D_MODULE_NAME"::d_newGroup_s"
#define D_NEWGROUP_TOP_NAME         "v_message<" D_NEWGROUP_TYPE_NAME ">"
#define D_NEWGROUP_KEY_LIST         "parentMsg.senderAddress.systemId"

#define D_SAMPLE_CHAIN_TOPIC_NAME   "d_sampleChain"
#define D_SAMPLE_CHAIN_TYPE_NAME    D_MODULE_NAME"::d_sampleChain_s"
#define D_SAMPLE_CHAIN_TOP_NAME     "v_message<" D_SAMPLE_CHAIN_TYPE_NAME ">"
#define D_SAMPLE_CHAIN_KEY_LIST     "parentMsg.senderAddress.systemId"

#define D_NAMESPACES_TOPIC_NAME     "d_nameSpaces"
#define D_NAMESPACES_TYPE_NAME      D_MODULE_NAME"::d_nameSpaces_s"
#define D_NAMESPACES_TOP_NAME       "v_message<" D_NAMESPACES_TYPE_NAME ">"
#define D_NAMESPACES_KEY_LIST       "parentMsg.senderAddress.systemId"

#define D_NAMESPACES_REQ_TOPIC_NAME "d_nameSpacesRequest"
#define D_NAMESPACES_REQ_TYPE_NAME  D_MODULE_NAME"::d_nameSpacesRequest_s"
#define D_NAMESPACES_REQ_TOP_NAME   "v_message<" D_NAMESPACES_REQ_TYPE_NAME ">"
#define D_NAMESPACES_REQ_KEY_LIST   "parentMsg.senderAddress.systemId"

#define D_DELETE_DATA_TOPIC_NAME  "d_deleteData"
#define D_DELETE_DATA_TYPE_NAME   D_MODULE_NAME"::d_deleteData_s"
#define D_DELETE_DATA_TOP_NAME    "v_message<" D_DELETE_DATA_TYPE_NAME ">"
#define D_DELETE_DATA_KEY_LIST    "parentMsg.senderAddress.systemId"

void                    d_adminUpdateStatistics                 (d_admin admin,
                                                                 d_adminStatisticsInfo statistics);

d_admin                 d_adminNew                              (d_durability durability);

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

d_fellow                d_adminGetFellow                        (d_admin admin,
                                                                 d_networkAddress address);

c_bool                  d_adminAddFellow                        (d_admin admin,
                                                                 d_fellow fellow);

d_fellow                d_adminRemoveFellow                     (d_admin admin,
                                                                 d_fellow fellow);

d_networkAddress        d_adminGetMyAddress                     (d_admin admin);

c_bool                  d_adminFellowWalk                       (d_admin admin,
                                                                 c_bool ( * action ) (
                                                                    d_fellow fellow,
                                                                    c_voidp userData),
                                                                 c_voidp userData);

c_ulong                 d_adminGetFellowCount                   (d_admin admin);

void                    d_adminCleanupFellows                   (d_admin admin,
                                                                 d_timestamp timestamp);

c_bool                  d_adminAddNameSpace                     (d_admin admin,
                                                                 d_nameSpace nameSpace);

void                    d_adminNameSpaceWalk                    (d_admin admin,
                                                                 void (*action)(
                                                                     d_nameSpace nameSpace,
                                                                     c_voidp userData),
                                                                 c_voidp userData);

c_iter                  d_adminNameSpaceCollect                 (d_admin admin);
void                    d_adminNameSpaceCollectFree             (d_admin admin, c_iter nameSpaces);

c_ulong                 d_adminGetNameSpacesCount               (d_admin admin);

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

c_bool                  d_adminAreLocalGroupsComplete           (d_admin admin);

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
                                                                 d_nameSpace fellowNameSpace,
                                                                 d_nameSpace oldFellowNameSpace);

void                    d_adminReportDelayedInitialSet          (d_admin admin,
                                                                 d_nameSpace nameSpace,
                                                                 d_fellow fellow);

#if defined (__cplusplus)
}
#endif

#endif /* D_ADMIN_H */
