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
#include "d__sampleChainListener.h"
#include "d__readerListener.h"
#include "d__mergeAction.h"
#include "d__group.h"
#include "d__configuration.h"
#include "d__actionQueue.h"
#include "d__durability.h"
#include "d__fellow.h"
#include "d__listener.h"
#include "d__admin.h"
#include "d__nameSpace.h"
#include "d__readerRequest.h"
#include "d__readerListener.h"
#include "d__publisher.h"
#include "d__nameSpacesRequestListener.h"
#include "d__table.h"
#include "d__misc.h"
#include "d__groupLocalListener.h"
#include "d__group.h"
#include "d__eventListener.h"
#include "d__conflictResolver.h"
#include "d_sampleRequest.h"
#include "d_newGroup.h"
#include "d_sampleChain.h"
#include "d_groupsRequest.h"
#include "d_networkAddress.h"
#include "d_message.h"
#include "d__thread.h"
#include "d__groupHash.h"
#include "d_store.h"
#include "v_builtin.h"
#include "v_group.h"
#include "v_groupInstance.h"
#include "v_dataReaderInstance.h"
#include "v_writer.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_time.h"
#include "v_entry.h"
#include "v_entity.h"
#include "v_message.h"
#include "v_public.h"
#include "v_state.h"
#include "v_reader.h"
#include "v_messageExt.h"
#include "v_kernel.h"
#include "v_historicalDataRequest.h"
#include "sd_serializer.h"
#include "sd_serializerBigE.h"
#include "os_thread.h"
#include "os_heap.h"
#include "os_report.h"
#include "d__thread.h"

/**
 * \brief The d_sampleChainListener cast macro.
 *
 * This macro casts an object to a d_sampleChainListener object.
 */
#define d_sampleChainListener(_this) ((d_sampleChainListener)(_this))

/**
 * \brief The d_chainBead cast macro.
 *
 * This macro casts an object to a d_chainBead object.
 */
#define d_chainBead(_this) ((d_chainBead)(_this))

/**
 * \brief The d_chainLink cast macro.
 *
 * This macro casts an object to a d_chainBead object.
 */
#define d_chainLink(_this) ((d_chainLink)(_this))

#define d_chainKeep(chain) ((d_objectKeep(d_object(chain))))

C_STRUCT(d_sampleChainListener){
    C_EXTENDS(d_readerListener);
    d_table         chains;
    c_iter          chainsWaiting;
    c_ulong         id;
    d_eventListener fellowListener;
    d_actionQueue   resendQueue;
    c_iter          unfulfilledChains;
    d_table         mergeActions;
};

C_STRUCT(d_chainBead){
    d_table senders; /* list of fellows that sent it (subset of chain->fellows) */
    v_message message;
    c_value keyValues[32];
    c_ulong nrOfKeys;
    c_ulong refCount;
};

C_STRUCT(d_chainLink){
    d_networkAddress sender;
    c_ulong sampleCount;
    d_admin admin;
};
C_STRUCT(d_chain){
    C_EXTENDS(d_object);
    d_sampleRequest request;
    d_table         beads;
    d_table         links;
    d_table         fellows;     /* list of fellows to which a sampleRequest has been send */
    c_long          samplesExpect;
    c_ulong         receivedSize;
    c_type          xmsgType;
    c_type          messageType;
    sd_serializer   serializer;     /* serializer used to serialize "normal" data */
    sd_serializer   serializerEOT;  /* serializer used to serialize EOTs */
    sd_serializer   serializerNode;  /* serializer used to serialize v_nodeExt for deserializing just the v_message flags */
    v_group         vgroup;
};

struct chainCleanup{
    d_admin admin;
    d_sampleChainListener listener;
    d_fellow fellow;
    c_iter toRemove;
    c_iter beadsToRemove;
    c_long dupsFromFellowCount;
};

C_CLASS(d_resendAction);
C_STRUCT(d_resendAction){
    c_iter messages;
    d_group group;
    d_sampleChainListener listener;
};

#define d_resendAction(a) ((d_resendAction)(a))

void d_traceChain(d_chain chain)
{
    char str[1024];
    d_tableIter tableIter;
    d_fellow fellow;
    size_t pos = 0;

    /* determine the list of fellows to be addressed */
    str[0] = '\0';
    fellow = d_fellow(d_tableIterFirst(chain->fellows, &tableIter));
    while (fellow) {
        int n = snprintf(str + pos, sizeof(str) - pos, "%s%u", (strcmp(str, "") == 0) ? "" : ",", fellow->address->systemId);
        if (n > 0) { pos += (size_t)n; } else { break; }
        fellow = d_fellow(d_tableIterNext(&tableIter));
    }

    d_trace(D_TRACE_CHAINS, " - chain %p, request %p, group %s.%s fellows [%s] #beads %d, #links %d, receivedSize %d, samplesExpect %d\n",
            (void *)chain, (void *)chain->request, chain->request->partition, chain->request->topic, str,
            d_tableSize(chain->beads), d_tableSize(chain->links), chain->receivedSize, chain->samplesExpect);
}

static void
updateGroupStatistics(
    d_admin admin,
    d_group group)
{
    /*update statistics*/
    d_adminStatisticsInfo info = d_adminStatisticsInfoNew();
    info->kind = D_ADMIN_STATISTICS_GROUP;

    switch(d_groupGetKind(group)){
    case D_DURABILITY_VOLATILE:
        info->groupsIncompleteVolatileDif -=1;
        info->groupsCompleteVolatileDif +=1;
        break;
    case D_DURABILITY_TRANSIENT_LOCAL:
    case D_DURABILITY_TRANSIENT:
        info->groupsIncompleteTransientDif -=1;
        info->groupsCompleteTransientDif +=1;
        break;
    case D_DURABILITY_PERSISTENT:
        info->groupsIncompletePersistentDif -=1;
        info->groupsCompletePersistentDif +=1;
        break;
    default:
        break;
    }

    d_adminUpdateStatistics(admin, info);
    d_adminStatisticsInfoFree(info);

    return;
}

struct readBeadHelper {
    d_resendAction action;
    v_entry entry;
    c_ulong totalCount;
    c_ulong writeCount;
    c_ulong disposeCount;
    c_ulong writeDisposeCount;
    c_ulong registerCount;
    c_ulong unregisterCount;
    c_ulong eotCount;
    d_mergePolicy mergePolicy;
    d_thread self;
};

struct chainRequestHelper{
    d_chain chain;
    d_durability durability;
    d_fellow fellow;
    d_networkAddress master;
    d_name role;
    d_nameSpace nameSpace;
};

static v_writeResult
d_chainBeadInjectTryMessage(
    d_durability durability,
    v_message message,
    v_group group,
    v_entry entry);

static void
d_chainFellowFree(
    d_fellow fellow)
{
    d_fellowRequestRemove(fellow);
    d_fellowFree(fellow);
}

static c_bool
d_chainReportStatus(
    d_chain chain,
    d_durability durability)
{
    assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

    d_printTimedEvent(durability, D_LEVEL_FINEST,
        "- Group: '%s.%s', #Aligners: '%d', #Beads: '%d', #Links: '%d'\n",
        chain->request->partition,
        chain->request->topic,
        d_tableSize(chain->fellows),
        d_tableSize(chain->beads),
        d_tableSize(chain->links));

    return TRUE;
}

/**
 * \brief Compare two beads
 *
 * Two beads are the same if their messages are the same AND
 * their keys are the same.
 */
static int
d_chainBeadCompare(
    d_chainBead bead1,
    d_chainBead bead2)
{
    int result = 0;
    c_equality eq;
    c_ulong i;

    assert(bead1);
    assert(bead2);

    if (bead1->message && bead2->message) {
        if ((eq = v_messageCompare(bead1->message, bead2->message)) == C_GT) {
            result = 1;
        } else if (eq == C_LT) {
            result = -1;
        }
        /* The group administrates combined DISPOSE+UNREGISTER messages in the history of the instance
         * AND keeps the v_registration in the unregistrations of the instance.
         * Both are aligned and we need to ensure that the DISPOSE++UNREGISTER
         * is always injected before the UNREGISTER, hence the extra compares
         * below.
         */
        else if(v_messageStateTest(bead1->message, L_DISPOSED) && v_messageStateTest(bead1->message, L_UNREGISTER) && (v_messageState(bead1->message) != v_messageState(bead2->message))){
            result = -1;
        } else if(v_messageStateTest(bead2->message, L_DISPOSED) && v_messageStateTest(bead2->message, L_UNREGISTER) && (v_messageState(bead1->message) != v_messageState(bead2->message))){
            result = 1;
        } else if (v_nodeState(bead1->message) > v_nodeState(bead2->message)) {
            result = 1;
        } else if (v_nodeState(bead1->message) < v_nodeState(bead2->message)) {
            result = -1;
        } else {
            if (bead1->nrOfKeys == bead2->nrOfKeys) {
                for (i = 0; i < bead1->nrOfKeys; i++) {
                    if ((eq = c_valueCompare(bead1->keyValues[i], bead2->keyValues[i])) != C_EQ) {
                        result = (eq == C_LT) ? -1 : 1;
                        break;
                    }
                }
            } else {
                d_printTimedEvent(d_threadsDurability(), D_LEVEL_SEVERE,
                     "Unrecoverable error: beads in chain have different numbers of keys.");
                OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                     "Unrecoverable error: beads in chain have different numbers of keys.");
                d_durabilityTerminate(d_threadsDurability(), TRUE);
            }
        }
    } else if (!bead1->message) {
        result = -1;
    } else {
        result = 1;
    }
    return result;
}

static void
d_chainDeinit(
    d_chain chain)
{
    assert(d_chainIsValid(chain));

    if (chain->vgroup) {
        c_free(chain->vgroup);
        chain->vgroup = NULL;
    }
    if (chain->beads) {
        d_tableFree(chain->beads);
        chain->beads = NULL;
    }
    if (chain->links) {
        d_tableFree(chain->links);
        chain->links = NULL;
    }
    if (chain->request) {
        d_sampleRequestFree(chain->request);
        chain->request = NULL;
    }
    if (chain->fellows) {
        d_tableFree(chain->fellows);
        chain->fellows = NULL;
    }
    if (chain->serializer) {
        v_messageExtTypeFree (chain->xmsgType);
        sd_serializerFree(chain->serializer);
        chain->messageType = NULL; /* not ref counted */
        chain->xmsgType = NULL;
        chain->serializer = NULL;
    }
    if (chain->serializerEOT) {
        sd_serializerFree(chain->serializerEOT);
        chain->serializerEOT = NULL;
    }
    if (chain->serializerNode) {
        sd_serializerFree(chain->serializerNode);
        chain->serializerNode = NULL;
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(chain));
}

static int
d_chainLinkCompare(
    d_chainLink link1,
    d_chainLink link2)
{
    int result = 0;

    if(link1->sender->systemId > link2->sender->systemId){
       result = 1;
    } else if(link1->sender->systemId < link2->sender->systemId){
       result = -1;
    } else {
        if(link1->sender->localId > link2->sender->localId){
           result = 1;
        } else if(link1->sender->localId < link2->sender->localId){
           result = -1;
        } else {
            if(link1->sender->lifecycleId > link2->sender->lifecycleId){
               result = 1;
            } else if(link1->sender->lifecycleId < link2->sender->lifecycleId){
               result = -1;
            } else {
                result = 0;
            }
        }
    }
    return result;
}

static void
d_chainBeadFree(
    d_chainBead chainBead)
{
    c_ulong i;

    assert(chainBead);

    /* No parent class, deallocate chainBead */
    if (chainBead->message) {
        c_free(chainBead->message);
        chainBead->message = NULL;
    }
    d_tableFree(chainBead->senders);
    for (i=0;i<chainBead->nrOfKeys;i++) {
        c_valueFreeRef(chainBead->keyValues[i]);
    }
    os_free(chainBead);
}

static void
d_chainLinkFree(
    d_chainLink link)
{
    /* No parent class, deallocate chainLink */
    if (link->sender) {
        d_networkAddressFree(link->sender);
        link->sender = NULL;
    }
    os_free(link);
}

d_chain
d_chainNew(
    d_admin admin,
    d_sampleRequest request)
{
    d_chain chain;
    d_group group;

    assert(request);

    /* Allocate chain object */
    chain = d_chain(os_malloc(C_SIZEOF(d_chain)));
    if (chain) {
        /* Call super-init */
        d_objectInit(d_object(chain), D_CHAIN,
                     (d_objectDeinitFunc)d_chainDeinit);
        /* Initialize chain object */
        chain->request         = request;
        chain->beads           = d_tableNew(d_chainBeadCompare, d_chainBeadFree);
        chain->links           = d_tableNew(d_chainLinkCompare, d_chainLinkFree);
        chain->fellows         = d_tableNew(d_fellowCompare, d_chainFellowFree);
        chain->samplesExpect   = 0;
        chain->receivedSize    = 0;
        if (admin) {
            group = d_adminGetLocalGroup(admin,
                                     request->partition, request->topic,
                                     request->durabilityKind);
            chain->vgroup = d_groupGetKernelGroup(group);
            chain->xmsgType  = v_messageExtTypeNew (v_groupTopic (chain->vgroup));
            chain->messageType = v_topicMessageType (v_groupTopic (chain->vgroup));
            chain->serializer  = sd_serializerBigENewTyped (chain->xmsgType);
            chain->serializerEOT = sd_serializerBigENewTyped(c_resolve(c_getBase((c_object)chain->vgroup), "kernelModule::v_messageEOTExt"));
            chain->serializerNode = sd_serializerBigENewTyped(c_resolve(c_getBase((c_object)chain->vgroup), "kernelModule::v_nodeExt"));
        } else {
            chain->xmsgType = NULL;
            chain->messageType = NULL;
            chain->serializer = NULL;
            chain->serializerEOT = NULL;
            chain->serializerNode = NULL;
            chain->vgroup = NULL;
        }
    }
    return chain;
}

void
d_chainFree(
    d_chain chain)
{
    assert(d_chainIsValid(chain));

    d_objectFree(d_object(chain));
}

static c_bool
findAligner(
    d_fellow fellow,
    c_voidp args)
{
    c_ulong count;
    c_bool fellowComplete = FALSE;
    c_bool fellowMightKnowGroup = FALSE;
    c_bool checkFurther = TRUE;
    c_bool fellowHasGroup = FALSE;
    struct chainRequestHelper* helper;
    d_fellowAlignStatus status = D_ALIGN_FALSE;
    d_networkAddress fellowAddress, oldAddress;
    d_name fellowRole;

    c_ulong fellowMasterPriority = 0;

    helper = (struct chainRequestHelper*)args;
    fellowAddress = d_fellowGetAddress(fellow);

    if (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED) {
        fellowRole = d_fellowGetRole(fellow);

        /* Find a fellow that has the same role as me and
         * that is able to align data for the data for the group.
         */
        if (fellowRole && strcmp(fellowRole, helper->role) == 0) {
            fellowComplete = d_fellowIsCompleteForGroup(fellow,
                                            helper->chain->request->partition,
                                            helper->chain->request->topic,
                                            helper->chain->request->durabilityKind);
            status = d_fellowIsAlignerForGroup(fellow,
                                        helper->chain->request->partition,
                                        helper->chain->request->topic,
                                        helper->chain->request->durabilityKind,
                                        &fellowMasterPriority);
            fellowHasGroup = d_fellowHasGroup(fellow,
                   helper->chain->request->partition,
                   helper->chain->request->topic,
                   helper->chain->request->durabilityKind);
            fellowMightKnowGroup = (status == D_ALIGN_TRUE) || (status == D_ALIGN_UNKNOWN);
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                    "Finding aligner: myRole='%s', fellow=%u, fellowRole='%s', fellowComplete=%d, "
                    "fellowHasGroup=%d, fellowMightKnowGroup=%d and fellowAlignStatus=%d.\n",
                    helper->role, fellowAddress->systemId, fellowRole, fellowComplete,
                    fellowHasGroup, fellowMightKnowGroup, status);
        } else {
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                "Finding aligner: do nothing for fellow %u as its role (%s) is different than mine (%s).\n",
                fellowAddress->systemId, fellowRole, helper->role);
            d_networkAddressFree(fellowAddress);
            return TRUE;
        }
    } else {
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
            "Finding aligner: fellow %u not approved (yet).\n",
            fellowAddress->systemId);
        d_networkAddressFree(fellowAddress);
        return TRUE;
    }

    /* If the fellow can never know the group it is never a suitable candidate */
    if (!fellowMightKnowGroup) {
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                "Finding aligner: do nothing for fellow %u as it not an aligner.\n", fellowAddress->systemId);
        d_networkAddressFree(fellowAddress);
        return TRUE;
    }
    /* The fellow is never a suitable candidate aligner if it has masterPriority 0 and me too */
    else if (helper->nameSpace &&
             (d_nameSpaceGetMasterPriority(helper->nameSpace) == D_MINIMUM_MASTER_PRIORITY) &&
             (fellowMasterPriority == D_MINIMUM_MASTER_PRIORITY))
    {
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                "Finding aligner: do nothing for fellow %u as it has masterPriority 0 and me too.\n",
                fellowAddress->systemId);
        d_networkAddressFree(fellowAddress);
        return TRUE;
    }
    /* Fellow with non-zero masterPriority that has the set complete
     * might be a better candidate than the currently selected one */
    else if ((fellowComplete == TRUE) && (helper->fellow) &&
             (status != D_ALIGN_FALSE) && (fellowMasterPriority > D_MINIMUM_MASTER_PRIORITY))
    {
        /* fellow complete and aligner and old complete fellow exists. */
        count = d_fellowRequestCountGet(helper->fellow);

        if (d_networkAddressEquals(fellowAddress, helper->master)) {
            /* select the master. */
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                        "Finding aligner: select fellow %u as it is the master.\n",
                        fellowAddress->systemId);
            d_tableFree(helper->chain->fellows);
            helper->chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
            d_objectKeep(d_object(fellow));
            d_tableInsert(helper->chain->fellows, fellow);
            helper->fellow = fellow;
            d_fellowRequestAdd(fellow);
            checkFurther = FALSE;
        } else if (count > d_fellowRequestCountGet(fellow)) {
            /* select the one with the least open requests. */
            oldAddress = d_fellowGetAddress(helper->fellow);
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                    "Finding aligner: select fellow %u as it has %u open requests whereas current fellow %u has %u open requests.\n",
                    fellowAddress->systemId, d_fellowRequestCountGet(fellow),
                    oldAddress->systemId, count);
            d_networkAddressFree(oldAddress);
            d_tableFree(helper->chain->fellows);
            helper->chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
            d_objectKeep(d_object(fellow));
            d_tableInsert(helper->chain->fellows, fellow);
            helper->fellow = fellow;
            d_fellowRequestAdd(fellow);
        } else {
            /* Do nothing here */
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                "Finding aligner: do nothing for fellow %u as current selected fellow is better.\n",
                fellowAddress->systemId);
        }
    } else if ((fellowComplete == TRUE) && (status != D_ALIGN_FALSE)) {
        /* Fellow complete and no old complete fellow exists */
        d_tableFree(helper->chain->fellows);
        helper->chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
        d_objectKeep(d_object(fellow));
        d_tableInsert(helper->chain->fellows, fellow);
        helper->fellow = fellow;
        d_fellowRequestAdd(fellow);

        if (d_networkAddressEquals(fellowAddress, helper->master)) {
            checkFurther = FALSE;
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
				"Finding aligner: selecting fellow %u and not checking further.\n",
				fellowAddress->systemId);
        } else {
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
				"Finding aligner: selecting fellow %u and checking further.\n",
				fellowAddress->systemId);
        }

    /* Fellow is an aligner and might know the group,
     * so the fellow as an alignment candidate. This also
     * covers the case with masterPriority="0" */
    } else if ((fellowMightKnowGroup == TRUE) && (!helper->fellow)) {
        /* No complete fellow found (yet). */
        d_objectKeep(d_object(fellow));
        d_fellowRequestAdd(fellow);
        d_tableInsert(helper->chain->fellows, fellow);
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                "Finding aligner: adding fellow %u to list of candidates that is now %u long.\n",
                fellowAddress->systemId, d_tableSize(helper->chain->fellows));
    /* Fellow is not suitable */
    } else {
        /* Do nothing here */
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                "Finding aligner: do nothing for fellow %u as it is not suitable.\n",
                fellowAddress->systemId);
    }
    d_networkAddressFree(fellowAddress);

    return checkFurther;
}

static d_resendAction
d_resendActionNew(
    d_sampleChainListener listener,
    const d_group group)
{
    d_resendAction action;

    /* Allocate resendAction */
    action = d_resendAction(os_malloc(C_SIZEOF(d_resendAction)));
    if (action) {
        /* No parent class, initialize the resend action */
        action->messages = NULL;
        action->group = d_group(d_objectKeep(d_object(group)));
        action->listener = listener;
    }
    return action;
}

static void
d_resendActionFree(
    d_resendAction action)
{
    if (action) {
        v_message msg;
        while ((msg = c_iterTakeFirst(action->messages)) != NULL) {
            c_free(msg);
        }
        c_iterFree(action->messages);
        if (action->group) {
            d_groupFree(action->group);
        }
        os_free(action);
    }
    return;
}

static void
d_chainBeadInjectEOT(
    d_chainBead bead,
    v_group group,
    struct readBeadHelper *helper)
{
    v_resendScope resendScope = V_RESEND_NONE; /*TODO: resendScope not yet used here beyond this function */
    c_base base = c_getBase (group);

    assert(v_stateTest(v_nodeState(bead->message), L_ENDOFTRANSACTION));

    d_trace(D_TRACE_GROUP, "  Inject msg %p state=%u time=%"PA_PRItime" wrgid=%u:%u:%u EOT\n",
            (void*)bead->message, bead->message->_parent.nodeState,
            OS_TIMEW_PRINT(bead->message->writeTime), bead->message->writerGID.systemId,
            bead->message->writerGID.localId, bead->message->writerGID.serial);

    if (c_baseMakeMemReservation(base, C_MM_RESERVATION_ZERO)) {
        /* can use anu v_groupWrite* function as all do the same for EOT messages,
         * maybe a special groupWriteEOT is desired.
         */
        (void)v_groupWrite(group, bead->message, NULL, V_NETWORKID_ANY, &resendScope);
        c_baseReleaseMemReservation(base, C_MM_RESERVATION_ZERO);
    } else {
        (void)d_shmAllocAssert(NULL, "Write EOT message failed.");
        assert(FALSE);
    }
    helper->totalCount++;
    helper->eotCount++;
}

static void
d_chainBeadInjectSetMessageKeyFields(
    v_message message,
    v_group group,
    c_value keyValues[32])
{
    c_array messageKeyList = v_topicMessageKeyList(v_groupTopic(group));
    c_ulong nrOfKeys = c_arraySize(messageKeyList);
    c_ulong i;
    for (i = 0; i < nrOfKeys; i++) {
        c_fieldAssign(messageKeyList[i], message, keyValues[i]);
    }
}

static v_writeResult
d_chainBeadInjectTryMessage(
    d_durability durability,
    v_message message,
    v_group group,
    v_entry entry)
{
    c_base const base = c_getBase(group);
    v_resendScope resendScope; /*TODO: resendScope not yet used here beyond this function */
    char keystr[1024];
    v_writeResult writeResult;

    d_tracegroupGenMsgKeystr(keystr, sizeof (keystr), group, message);
    d_trace(D_TRACE_GROUP, "  Inject msg %p state=%u time=%"PA_PRItime" wrgid=%u:%u:%u key={%s}\n",
            (void*)message, v_messageState(message),
            OS_TIMEW_PRINT(message->writeTime), message->writerGID.systemId,
            message->writerGID.localId, message->writerGID.serial, keystr);

    {
        v_groupInstance instance = NULL;
        resendScope = V_RESEND_NONE;
        if (c_baseMakeMemReservation(base, C_MM_RESERVATION_ZERO)) {
            if (entry) {
                writeResult = v_groupWriteHistoricalToReader(group, message, entry);
            } else if (v_gidIsNil(message->writerGID)) {
                writeResult = v_groupWriteHistoricalData(
                        group, message, &instance, V_NETWORKID_ANY);
            } else {
                writeResult = v_groupWrite(
                        group, message, &instance, V_NETWORKID_ANY, &resendScope);
            }
            c_baseReleaseMemReservation(base, C_MM_RESERVATION_ZERO);
        } else {
            (void)d_shmAllocAssert(NULL, "Write bead message failed.");
            writeResult = V_WRITE_OUT_OF_RESOURCES;
        }

        if (writeResult != V_WRITE_SUCCESS) {
            d_trace(D_TRACE_GROUP, "    write failed with code %d\n", (int) writeResult);
        } else {
            d_trace(D_TRACE_GROUP, "    bead written\n");
        }
        if (instance) {
            d_tracegroupInstance(instance, durability, "    ");
            c_free(instance);
        }
    }
    return writeResult;
}

static void
d_chainBeadInjectRegular(
    d_chainBead bead,
    v_group group,
    struct readBeadHelper *helper)
{
    d_resendAction action = helper->action;
    d_durability durability = d_adminGetDurability(d_listenerGetAdmin((d_listener)action->listener));
    v_message message;
    v_message topicMsg = NULL;

    assert(!v_stateTest(v_nodeState(bead->message), L_ENDOFTRANSACTION));

    /* Mark the message in the bead as required by the policy */
    switch (helper->mergePolicy) {
        case D_MERGE_IGNORE:
        case D_MERGE_MERGE:
        case D_MERGE_DELETE:
            break;
        case D_MERGE_REPLACE:
            v_stateSet(v_nodeState(bead->message), L_REPLACED);
            break;
        case D_MERGE_CATCHUP:
            v_stateSet(v_nodeState(bead->message), L_MARK);
            break;
    }

    if (c_getType(bead->message) != v_kernelType(v_objectKernel(group), K_MESSAGE)) {
        message = bead->message;
    } else {
        /* If the message is a mini-message without keys, temporarily
         * replace it with a typed message that does include the
         * keys. That way the bead becomes self- describing and so the
         * receiving node can deduct its instance again.
         */
        topicMsg = v_topicMessageNew(v_groupTopic(group));
        message = topicMsg;
        if (message == NULL) {
            d_durabilityTerminate(durability, FALSE);
            return;
        } else {
            memcpy(message, bead->message, sizeof (*message));
            message->qos = c_keep(bead->message->qos);
            d_chainBeadInjectSetMessageKeyFields(message, group, bead->keyValues);
        }
    }

    if (d_chainBeadInjectTryMessage(durability, message, group, helper->entry) != V_WRITE_SUCCESS) {
        action->messages = c_iterAppend(action->messages, c_keep(message));
    }
    c_free(topicMsg);

    helper->totalCount++;
    if((v_stateTest(v_nodeState(bead->message), L_WRITE)) &&
       (v_stateTest(v_nodeState(bead->message), L_DISPOSED))) {
        helper->writeDisposeCount++;
    } else if(v_stateTest(v_nodeState(bead->message), L_WRITE)) {
        helper->writeCount++;
    } else if(v_stateTest(v_nodeState(bead->message), L_DISPOSED)) {
        helper->disposeCount++;
    } else if(v_stateTest(v_nodeState(bead->message), L_REGISTER)) {
        helper->registerCount++;
    } else if(v_stateTest(v_nodeState(bead->message), L_UNREGISTER)) {
        helper->unregisterCount++;
    }
}

c_bool
d_chainBeadInject(
    d_chainBead bead,
    c_voidp args)
{
    struct readBeadHelper *helper = (struct readBeadHelper*)args;
    v_group group = d_groupGetKernelGroup(helper->action->group);
    d_threadAwake (helper->self);
    if (v_stateTest(v_nodeState(bead->message), L_ENDOFTRANSACTION)) {
        d_chainBeadInjectEOT (bead, group, helper);
    } else {
        d_chainBeadInjectRegular (bead, group, helper);
    }
    c_free(group);
    return TRUE;
}

static c_bool
d_resendRejected(
    d_action action,
    c_bool terminate)
{
    d_thread self = d_threadLookupSelf ();
    d_resendAction resendData;
    v_message resendMessage;
    v_group group;
    c_bool callAgain;
    d_durability durability;
    d_admin admin;
    v_resendScope resendScope = V_RESEND_NONE; /* resendScope not yet used here beyond this function */

    callAgain = TRUE;
    resendData = d_resendAction(d_actionGetArgs(action));
    assert(c_iterLength(resendData->messages) != 0);
    group = d_groupGetKernelGroup(resendData->group);
    admin = d_listenerGetAdmin(d_listener(resendData->listener));
    durability = d_adminGetDurability(admin);
    if(terminate == FALSE){
        v_writeResult writeResult = V_WRITE_SUCCESS;
        resendMessage = c_iterTakeFirst(resendData->messages);
        terminate = d_durabilityMustTerminate(durability);
        while(resendMessage && (!terminate)){
            d_threadAwake (self);
            writeResult = d_chainBeadInjectTryMessage(durability, resendMessage, group, NULL);
            if (writeResult != V_WRITE_SUCCESS) {
                resendData->messages = c_iterInsert(resendData->messages, resendMessage);
                break;
            } else {
                c_free(resendMessage);
                resendMessage = c_iterTakeFirst(resendData->messages);
            }
            terminate = d_durabilityMustTerminate(durability);
        }

        if (resendMessage == NULL && (!terminate)) {
            assert(c_iterLength(resendData->messages) == 0);
            d_groupSetComplete(resendData->group, admin);
            /*Update statistics*/
            updateGroupStatistics(admin, resendData->group);
            d_sampleChainListenerReportGroup(resendData->listener, resendData->group);
            d_resendActionFree(resendData);
            callAgain = FALSE;
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "All data for group '%s.%s' has been resent. "
                              "Group is complete now.\n",
                              v_partitionName(v_groupPartition(group)),
                              v_topicName(v_groupTopic(group)));
        } else if (!terminate) {
            os_char buf[128] = { '\0' };
            (void)v_resendScopeImage (buf, sizeof(buf), resendScope);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "Resending data for group '%s.%s' failed with code %s. Resend scope (%s) "
                              "Trying again later.\n",
                              v_partitionName(v_groupPartition(group)),
                              v_topicName(v_groupTopic(group)),
                              v_writeResultString(writeResult),
                              buf);
        } else {
            /* Premature termination */
            callAgain = FALSE;
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "Going to terminate while resending data for group '%s.%s'\n",
                              v_partitionName(v_groupPartition(group)),
                              v_topicName(v_groupTopic(group)));
        }
        c_free(group);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                         "Terminating now, but alignment for group '%s.%s' "
                         "is not complete.\n",
                         v_partitionName(v_groupPartition(group)),
                         v_topicName(v_groupTopic(group)));
        OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                    "Terminating now, but alignment for group '%s.%s' "
                    "is not complete.\n",
                    v_partitionName(v_groupPartition(group)),
                    v_topicName(v_groupTopic(group)));
    }
    return callAgain;
}

struct findMergeHelper {
    d_chain chain;
    d_mergeAction action;
};

static c_bool
findMergeAction(
    d_mergeAction action,
    struct findMergeHelper* helper)
{
    d_chain found;

    found = d_mergeActionGetChain(action, helper->chain);

    if(found){
        helper->action = action;
    }
    return !found;
}

static d_mergeAction
d_sampleChainListenerGetMergeAction(
    d_sampleChainListener listener,
    d_chain chain)
{
    struct findMergeHelper helper;

    helper.chain = chain;
    helper.action = NULL;
    d_tableWalk(listener->mergeActions, findMergeAction, &helper);
    return helper.action;
}

static d_chain
d_sampleChainListenerFindChain(
    d_sampleChainListener listener,
    d_sampleChain sampleChain)
{
    d_chain chain, dummy;
    d_sampleRequest request;
    d_admin admin;
    d_durability durability;
    c_bool forMe;
    d_networkAddress myAddr;

    assert(listener);
    assert(sampleChain);

    admin             = d_listenerGetAdmin(d_listener(listener));
    myAddr            = d_adminGetMyAddress(admin);
    forMe             = d_sampleChainContainsAddressee(sampleChain, myAddr);

    if (forMe) {
        /* Create a dummy chain for lookup in listener->chains. */
        request = d_sampleRequestNew(admin, sampleChain->partition,
            sampleChain->topic, sampleChain->durabilityKind, OS_TIMEW_ZERO, FALSE, OS_TIMEW_ZERO, OS_TIMEW_ZERO);
        d_sampleRequestSetSource(request, &sampleChain->source);

        dummy = d_chainNew(NULL, request);

        chain = d_tableFind(listener->chains, dummy);

        if (!chain) {
           durability = d_adminGetDurability(admin);

           d_printTimedEvent(durability, D_LEVEL_WARNING,
               "Could not find chain for message where group is: %s.%s, kind is %u and source is (%u,%u,%u)\n",
               sampleChain->partition, sampleChain->topic, sampleChain->durabilityKind,
               sampleChain->source.systemId, sampleChain->source.localId, sampleChain->source.lifecycleId);
        }
        /* request is also freed by d_chainFree */
        d_chainFree(dummy);
    } else {
        chain = NULL;
    }
    d_networkAddressFree(myAddr);

    return chain;
}

static d_chainBead
d_chainBeadNew(
    d_fellow sender,
    v_message message,
    d_chain chain)
{
    d_chainBead chainBead = NULL;
    c_array messageKeyList;
    c_ulong i;

    assert(message);

    /* Allocate chainBead object */
    chainBead = d_chainBead(os_malloc(C_SIZEOF(d_chainBead)));
    if (chainBead) {
        /* No parent class, initialize chainBead */
        /* Only resolve the keys for non-EOTs because EOTs have no key fields
         */
       if (!v_messageStateTest(message, L_ENDOFTRANSACTION)) {
            memset(chainBead->keyValues, 0, sizeof(chainBead->keyValues));
            messageKeyList = v_topicMessageKeyList(v_groupTopic(chain->vgroup));
            chainBead->nrOfKeys = c_arraySize(messageKeyList);
            if (chainBead->nrOfKeys > 32) {
                OS_REPORT(OS_ERROR,
                    "d_sampleChainListener::d_chainBeadNew",0,
                    "too many keys %d exceeds limit of 32",
                    chainBead->nrOfKeys);
            } else {
                for (i=0;i<chainBead->nrOfKeys;i++) {
                    chainBead->keyValues[i] = c_fieldValue(messageKeyList[i], message);
                }
            }
        } else {
            /* EOTs do not have any keys */
            chainBead->nrOfKeys = 0;
        }
        /* In case of an unregister message, store the instance and an
         * untyped sample.
         */
        if (v_messageStateTest(message, L_UNREGISTER)) {
            chainBead->message = v_groupCreateUntypedInvalidMessage(
                    v_kernel(v_object(chain->vgroup)->kernel), message);
            assert(c_refCount(chainBead->message) == 1);
        } else {
            chainBead->message = c_keep(message);
        }
        chainBead->message->allocTime = os_timeEGet();
        /* all senders must be in chain->fellows, so no need to manipulate th
         * refcount nor a cleanup function
         */
        chainBead->senders = d_tableNew(d_fellowCompare, 0);
        d_tableInsert(chainBead->senders, sender);
        chainBead->refCount = 1;
    }
    return chainBead;
}

static d_chainLink
d_chainLinkNew(
    d_networkAddress sender,
    c_ulong sampleCount,
    d_admin admin)
{
    d_chainLink link;

    /* Allocate chainLink object */
    link = d_chainLink(os_malloc(C_SIZEOF(d_chainLink)));
    if (link) {
        /* No parent class, initialize chainLink */
        link->sender = d_networkAddressNew(
            sender->systemId,
            sender->localId,
            sender->lifecycleId);
        link->sampleCount = sampleCount;
        link->admin = admin;
    }
    return link;
}

struct takeWaitingChainHelper
{
    d_chain search;
    d_chain result;
};

static void
d_takeWaitingChainWalk (
    void* o,
    c_iterActionArg userData)
{
    d_chain chain;
    struct takeWaitingChainHelper* helper;

    chain = d_chain(o);
    helper = (struct takeWaitingChainHelper*)userData;

    if (!helper->result) {
        if (!d_chainCompare (chain, helper->search)) {
            helper->result = chain;
        }
    }
}

static d_chain
d_sampleChainListenerRemoveChain(
    d_sampleChainListener listener,
    d_chain chain)
{
    d_durability durability;
    d_admin admin;
    d_publisher publisher;
    d_networkAddress addressee;
    d_chain result;
    struct takeWaitingChainHelper walkData;

    assert (listener);
    assert (listener->chains);
    assert (chain);

    admin = d_listenerGetAdmin (d_listener(listener));
    durability = d_adminGetDurability (admin);
    publisher = d_adminGetPublisher(admin);

    /* Remove (complete) chain from listener */
    result = d_tableRemove (listener->chains, chain);

    /* Look for other chain containing same group in chainsWaiting list */
    walkData.search = chain;
    walkData.result = NULL;
    c_iterWalk (listener->chainsWaiting, d_takeWaitingChainWalk, &walkData);

    if (walkData.result) {
        c_iterTake (listener->chainsWaiting, walkData.result);

        /* Insert chain in chains list */
        d_tableInsert (listener->chains, walkData.result);

        /* Publish request */
        addressee = d_messageGetAddressee (d_message(chain->request));
        d_publisherSampleRequestWrite(publisher, chain->request, addressee);
        d_printTimedEvent(durability, D_LEVEL_FINE,
                         "Write delayed samplerequest for group %s.%s to fellow %u using source (%u,%u,%u)\n",
                         chain->request->partition,
                         chain->request->topic,
                         addressee->systemId,
                         chain->request->source.systemId,
                         chain->request->source.localId,
                         chain->request->source.lifecycleId);
    }

    return result;
}

static void
d_sampleChainListenerAction(
    d_listener listener,
    d_message message)
{
    d_sampleChain sampleChain;
    d_sampleChainListener sampleChainListener;
    d_chain chain;
    d_durability durability;
    d_admin admin;
    c_bool complete, proceed;
    d_chainBead bead;
    d_chainBead inserted;
    d_chainLink link;
    v_message vmessage;
    d_fellow fellow, dummy;
    d_networkAddress sender;
    d_networkAddress myAddr;
    d_subscriber subscriber;
    d_store store;
    d_group group;
    c_bool equal;
    d_storeResult result;

    admin = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    sampleChain = d_sampleChain(message);
    sampleChainListener = d_sampleChainListener(listener);
    chain = d_sampleChainListenerFindChain(sampleChainListener, sampleChain);

    sender = d_networkAddressNew(message->senderAddress.systemId,
                                 message->senderAddress.localId,
                                 message->senderAddress.lifecycleId);

    if (chain) {
        dummy = d_fellowNew(sender, D_STATE_COMPLETE, FALSE);
        fellow = d_tableFind(chain->fellows, dummy);
        if (!fellow) {
            chain = NULL;
        }
        d_fellowFree(dummy);
    }

    /* Chain might already have been removed, because this service assumed the
     * sender to be dead and chose a new fellow to align with.
     */
    if (chain) {
        assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

        proceed = d_shmAllocAllowed();

        if(proceed){
            /* Keep chain to prevent clean-up during phase the listener is
             * unlocked during deserialization of the sample.
             */
            d_chainKeep(chain);

            switch(sampleChain->msgBody._d){
                case BEAD:
                    d_listenerUnlock(listener);
                    {
                        v_node msgNode;
                        msgNode = (v_node) sd_serializerDeserialize(chain->serializerNode,
                                (sd_serializedData)(sampleChain->msgBody._u.bead.value));
                        if (msgNode == NULL) {
                            vmessage = NULL;
                        } else {
                            v_state msgState = msgNode->nodeState;
                            c_free(msgNode);
                            if (v_stateTest(msgState, L_ENDOFTRANSACTION)) {
                                v_messageEOTExt xmsg_eot;
                                xmsg_eot = (v_messageEOTExt )sd_serializerDeserialize(
                                        chain->serializerEOT,
                                        (sd_serializedData)(sampleChain->msgBody._u.bead.value));
                                vmessage = xmsg_eot ? v_messageEOTExtConvertFromExtType (xmsg_eot) : NULL;
                            } else {
                                v_messageExt xmsg;
                                xmsg = (v_messageExt) sd_serializerDeserialize(
                                        chain->serializer,
                                        (sd_serializedData)(sampleChain->msgBody._u.bead.value));
                                vmessage = xmsg ? v_messageExtConvertFromExtType (chain->messageType, xmsg) : NULL;
                            }
                        }
                    }
                    d_listenerLock(listener);
                    proceed = d_shmAllocAssert(vmessage, "Deserialization of received sample failed.");
                    assert(!proceed || vmessage != NULL);
                    if(proceed == TRUE){
                        /* Do not insert implicit unregistrations and disposes about yourself that are
                         * aligned by fellows. Typically, this situation occurs when the fellow's splice
                         * daemon unregisters (and/or  disposes) writers on a node that is disconnected.
                         * This reflects the state that the fellow THINKS the writers on my node are not
                         * alive anymore. When connection is restored, the conclusion of the fellow that
                         * my writer is not alive should NOT be forwarded to me because this implicit
                         * conclusion was wrong apparently.
                         */
                        myAddr = d_adminGetMyAddress(admin);
                        if ( ! ( v_messageStateTest(vmessage, L_IMPLICIT) &&
                                 (vmessage->writerGID.systemId == myAddr->systemId) ) ) {
                            /* the message is not implicit or is not supposedly written by
                             * myself so can be inserted safely
                             */
                            bead = d_chainBeadNew(fellow, vmessage, chain);
                            inserted = d_tableInsert(chain->beads, bead);
                            /* Duplicates are not inserted
                             * A message is considered a duplicate if the writer GID and the
                             * source timestamp are equal to the ones in the other message.
                             */
                            if (inserted != NULL) {
                                /* Bead already present, so number of expected samples must
                                 * be lowered.
                                 */
                                chain->samplesExpect--;

                                if (d_tableInsert(inserted->senders, fellow) == NULL) {
                                    inserted->refCount++;
                                }
                                assert(d_tableSize(inserted->senders) == inserted->refCount);
                                d_chainBeadFree(bead);
                            } else {
                                chain->receivedSize += sd_serializedDataGetTotalSize((sd_serializedData)(sampleChain->msgBody._u.bead.value));
                            }
                        } else {
                            /* The message was an implicit unregister or dispose message with
                             * a writerGID from myself, so it is a local conclusion by the
                             * fellow about my presence. No need to add it to the bead, I know
                             * best whether I am alive or not.
                             */
                            chain->samplesExpect--;
                        }
                        d_networkAddressFree(myAddr);
                        c_free(vmessage);
                    }
                    break;
                case LINK:
                    if(sampleChain->msgBody._u.link.completeness == D_GROUP_KNOWLEDGE_UNDEFINED){
                        d_printTimedEvent(durability, D_LEVEL_WARNING,
                            "Received link from fellow %u for group '%s.%s' where "
                            "it informs me it will not align me. This means that "
                            "our data model or state is incompatible. I need to "
                            "disconnect the fellow and discover it again.\n",
                            sender->systemId,
                            chain->request->partition,
                            chain->request->topic);
                        dummy = d_adminRemoveFellow(admin, fellow, FALSE);

                        if(dummy){
                            d_fellowFree(dummy);
                        }
                    } else {
                        /* FIXME: should validate things (that e.g., samplesExpect + nrSamples is defined) */
                        equal = FALSE;
                        group = d_adminGetLocalGroup(admin,
                                                     chain->request->partition,
                                                     chain->request->topic,
                                                     chain->request->durabilityKind);

                        if (sampleChain->msgBody._u.link.nrSamples == D_GROUP_IS_EQUAL) {
                            /* Remote group matches local group, no samples expected */
                            equal = TRUE;
                        } else {
                            chain->samplesExpect += (c_long) sampleChain->msgBody._u.link.nrSamples;
                        }

                        link = d_chainLinkNew(sender, sampleChain->msgBody._u.link.nrSamples, admin);
                        d_tableInsert(chain->links, link);
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                                "Received %slink from fellow %u for group %s.%s containing %u samples. #links == %u\n",
                                equal ? "equal " : "",
                                sender->systemId,
                                chain->request->partition,
                                chain->request->topic,
                                (link->sampleCount == D_GROUP_IS_EQUAL) ? 0 : link->sampleCount,
                                d_tableSize(chain->links));

                        if (d_groupIsStoreMessagesLoaded(group) == TRUE) {
                            subscriber = d_adminGetSubscriber(admin);
                            store = d_subscriberGetPersistentStore(subscriber);
                            result = d_storeMessagesLoadFlush(store, group, equal);
                            if (result == D_STORE_RESULT_OK) {
                                d_groupSetStoreMessagesLoaded(group, FALSE);
                            } else if (equal == TRUE) {
                                OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                                            "Failed to inject stored messages for group %s.%s\n",
                                            chain->request->partition,
                                            chain->request->topic);
                                assert(FALSE);
                            } else {
                                OS_REPORT(OS_WARNING, OS_FUNCTION, result,
                                            "Failed to reject stored messages for group %s.%s\n",
                                            chain->request->partition,
                                            chain->request->topic);
                            }
                        }
                    }
                    break;
                default:
                    OS_REPORT(OS_ERROR, "d_sampleChainListenerAction", 0,
                                "Illegal message discriminator value (%d) detected.",
                                sampleChain->msgBody._d);
                    assert(FALSE);
                    break;
            }
        }
        complete = d_sampleChainListenerCheckChainComplete(sampleChainListener, chain);
        d_chainFree(chain); /* Mirror the d_chainKeep earlier on */

        if (complete == TRUE) {
            chain = d_sampleChainListenerRemoveChain (sampleChainListener, chain);

            if(chain){
                assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);
                d_chainFree(chain);
            }
        }
    }
    d_networkAddressFree(sender);
    return;
}

static void
d_sampleChainListenerDeinit(
    d_sampleChainListener listener)
{
    d_admin admin;
    d_chain chain;

    assert(d_sampleChainListenerIsValid(listener));

    /* Stop the listener */
    d_sampleChainListenerStop(listener);
    /* Deallocate memory associated with the listener */
    if (listener->fellowListener) {
        admin = d_listenerGetAdmin(d_listener(listener));
        d_adminRemoveListener(admin, listener->fellowListener);
        d_eventListenerFree(listener->fellowListener);
        listener->fellowListener = NULL;
    }
    if (listener->chains) {
        d_tableFree(listener->chains);
        listener->chains = NULL;
    }
    if (listener->chainsWaiting) {
        chain = d_chain(c_iterTakeFirst(listener->chainsWaiting));
        while(chain){
            d_chainFree(chain);
            chain = d_chain(c_iterTakeFirst(listener->chainsWaiting));
        }
        c_iterFree(listener->chainsWaiting);
    }
    if (listener->unfulfilledChains) {
        chain = d_chain(c_iterTakeFirst(listener->unfulfilledChains));
        while(chain){
            d_chainFree(chain);
            chain = d_chain(c_iterTakeFirst(listener->unfulfilledChains));
        }
        c_iterFree(listener->unfulfilledChains);
    }
    if (listener->resendQueue) {
        d_actionQueueFree(listener->resendQueue);
        listener->resendQueue = NULL;
    }
    if (listener->mergeActions) {
        d_tableFree(listener->mergeActions);
        listener->mergeActions = NULL;
    }
    /* Call super-deinit */
    d_readerListenerDeinit(d_readerListener(listener));
}

static void
d_sampleChainListenerInit(
    d_sampleChainListener listener,
    d_subscriber subscriber)
{
    os_duration sleepTime = OS_DURATION_INIT(1,0);
    d_admin admin;
    d_durability durability;
    d_configuration config;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed
     */

    assert(d_subscriberIsValid(subscriber));

    admin       = d_subscriberGetAdmin(subscriber);
    assert(d_adminIsValid(admin));
    durability  = d_adminGetDurability(admin);
    assert(d_durabilityIsValid(durability));
    config      = d_durabilityGetConfiguration(durability);
    assert(d_configurationIsValid(config));
    /* Call super-init */
    d_readerListenerInit(d_readerListener(listener),
                         D_SAMPLE_CHAIN_LISTENER,
                         d_sampleChainListenerAction,
                         subscriber,
                         D_SAMPLE_CHAIN_TOPIC_NAME,
                         D_SAMPLE_CHAIN_TOP_NAME,
                         V_RELIABILITY_RELIABLE,
                         V_HISTORY_KEEPALL,
                         V_LENGTH_UNLIMITED,
                         config->aligneeScheduling,
                         (d_objectDeinitFunc)d_sampleChainListenerDeinit);
    /* Initialize the sampleChainListener */
    listener->chains = d_tableNew(d_chainCompare, d_chainFree);
    assert(listener->chains);
    listener->chainsWaiting = c_iterNew(NULL);
    assert (listener->chainsWaiting);
    listener->id = 0;
    listener->fellowListener = NULL;
    listener->resendQueue = d_actionQueueNew("resendQueue",sleepTime, config->aligneeScheduling);
    assert(listener->resendQueue);
    listener->unfulfilledChains = c_iterNew(NULL);
    assert(listener->unfulfilledChains);
    listener->mergeActions = d_tableNew(d_mergeActionCompare, d_mergeActionFree);
    assert(listener->mergeActions);
}


d_sampleChainListener
d_sampleChainListenerNew(
    d_subscriber subscriber)
{
    d_sampleChainListener listener;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate sampleChainListener object */
    listener = d_sampleChainListener(os_malloc(C_SIZEOF(d_sampleChainListener)));
    if (listener) {
        /* Initialize the sampleChainListener */
        d_sampleChainListenerInit(listener, subscriber);
    }
    return listener;
}


void
d_sampleChainListenerFree(
    d_sampleChainListener listener)
{
    assert(d_sampleChainListenerIsValid(listener));

    d_objectFree(d_object(listener));
}

static c_bool
d_sampleChainListenerCleanupBeads(
    d_chainBead bead,
    c_voidp userData)
{
    struct chainCleanup* data = userData;
    assert(d_tableSize(bead->senders) == bead->refCount);
    if (d_tableRemove(bead->senders, data->fellow) != NULL) {
        assert(bead->refCount > 0);
        if (--bead->refCount == 0) {
            data->beadsToRemove = c_iterInsert(data->beadsToRemove, bead);
        } else {
            data->dupsFromFellowCount++;
        }
    }
    return TRUE;
}

static c_bool
d_sampleChainListenerCleanupRequests(
    d_chain chain,
    c_voidp userData)
{
    d_chainLink link, link2;
    d_chainBead bead;
    struct chainCleanup* data;
    d_durability durability;
    c_bool complete;
    d_fellow found;

    data       = (struct chainCleanup*)userData;
    durability = d_adminGetDurability(data->admin);
    found      = d_tableRemove(chain->fellows, data->fellow);

    if(found){
        if(d_tableSize(chain->fellows) == 0){
            data->toRemove = c_iterInsert(data->toRemove, chain);
        } else {
            d_networkAddress fellowAddress = d_fellowGetAddress(data->fellow);
            link = d_chainLinkNew(fellowAddress, 0, data->admin);
            d_networkAddressFree(fellowAddress);
            link2 = d_tableRemove(chain->links, link);
            if (link->sender) {
                d_networkAddressFree(link->sender);
                link->sender = NULL;
            }
            os_free(link);

            if(link2){
                chain->samplesExpect -= (c_long) link2->sampleCount;

                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Fellow %u removed for group '%s.%s', number of expected "
                    "decreased with %lu\n",
                    found->address->systemId, chain->request->partition, chain->request->topic, link2->sampleCount);

                d_chainLinkFree(link2);
            }
            data->beadsToRemove = c_iterNew(NULL);
            data->dupsFromFellowCount = 0;
            d_tableWalk(chain->beads, d_sampleChainListenerCleanupBeads, data);

            chain->samplesExpect += data->dupsFromFellowCount;
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Fellow %u removed for group '%s.%s', %d duplicate samples were received already, number of expected "
                "increased to %u\n",
                found->address->systemId, chain->request->partition, chain->request->topic, data->dupsFromFellowCount, chain->samplesExpect);

            bead = c_iterTakeFirst(data->beadsToRemove);
            while(bead){
                bead = d_tableRemove(chain->beads, bead);

                if(bead){
                    d_chainBeadFree(bead);
                }
                bead = c_iterTakeFirst(data->beadsToRemove);
            }
            c_iterFree(data->beadsToRemove);
            complete = d_sampleChainListenerCheckChainComplete(data->listener, chain);

            if(complete == TRUE) {
                data->toRemove = c_iterInsert(data->toRemove, chain);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "%u fellows left to answer request for group %s.%s.\n",
                    d_tableSize(chain->fellows), chain->request->partition,
                    chain->request->topic);
            }
        }
        d_chainFellowFree(found);
    } else {
        /* This request has not been sent to the dead fellow.*/
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Request is not meant for fellow. %u fellows left to answer " \
            "request for group %s.%s.\n",
            d_tableSize(chain->fellows), chain->request->partition,
            chain->request->topic);
    }
    return TRUE;
}

static c_bool
d_sampleChainListenerNotifyFellowRemoved(
    c_ulong event,
    d_fellow fellow,
    d_nameSpace ns,
    d_group group,
    c_voidp eventUserData,
    c_voidp userData)
{
    d_durability durability;
    struct chainCleanup data;
    d_chain chain;
    d_group dgroup;
    d_sampleChainListener listener;
    d_subscriber subscriber;
    d_groupLocalListener glListener;
    c_iter requests;
    d_sampleRequest sampleRequest;
    d_readerRequest readerRequest;
    d_networkAddress source;
    d_mergeAction mergeAction;
    c_ulong chainCount;

    OS_UNUSED_ARG(ns);
    OS_UNUSED_ARG(eventUserData);
    OS_UNUSED_ARG(event);
    OS_UNUSED_ARG(group);
    listener = d_sampleChainListener(userData);

    assert(d_sampleChainListenerIsValid(listener));

    if(listener){
        data.admin = d_listenerGetAdmin(d_listener(listener));
        data.fellow = fellow;
        data.listener = listener;
        data.toRemove = c_iterNew(NULL);
        durability = d_adminGetDurability(data.admin);

        d_printTimedEvent(durability, D_LEVEL_FINE,
                "Fellow removed, checking %d requests.\n", d_tableSize(listener->chains));

        d_listenerLock(d_listener(listener));
        d_tableWalk(listener->chains, d_sampleChainListenerCleanupRequests, &data);
        chain = c_iterTakeFirst(data.toRemove);
        requests = c_iterNew(NULL);

        while(chain){
            /* Chain might be part of a mergeAction, if so; skip it*/
            mergeAction = d_sampleChainListenerGetMergeAction(listener, chain);
            if (mergeAction) {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                     "Removing chain from merge request for group %s.%s\n",
                     chain->request->partition, chain->request->topic);
                d_mergeActionRemoveChain(mergeAction, chain);
                chainCount = d_mergeActionGetChainCount(mergeAction);

                if (chainCount == 0) {
                    d_conflictResolver conflictResolver = data.admin->conflictResolver;

                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Removing merge action.\n");
                    d_tableRemove(listener->mergeActions, mergeAction);
                    /* Indicate that the mergeAction is completed */
                    if (conflictResolver) {
                        d_conflictResolverResetConflictInProgress(conflictResolver, mergeAction->conflict);
                    }
                    d_mergeActionFree(mergeAction);
                }

                /* Remove chain from chains list */
                if(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE){
                    chain = d_sampleChainListenerRemoveChain(listener, chain);
                }

            } else {
                /* Chain might be freed. This is a valid situation, because the
                 * fellow that was assumed dead by this service might still send
                 * some data after all. The sending of data can lead to the
                 * completeness of a chain. The chain will freed after the
                 * injection of its data.
                 *
                 * The chain is freed between the d_sampleChainListenerCleanupRequests
                 * and this point. Because this function has the listener lock, it
                 * is safe to check whether the chain has been freed.
                 */
                if(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE){
                    chain = d_sampleChainListenerRemoveChain(listener, chain);
                }
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Finding new aligner for group %s.%s\n",
                    chain->request->partition, chain->request->topic);

                /* Create a copy of the request. */
                requests = c_iterInsert(requests, d_sampleRequestCopy(chain->request, FALSE));

                if(d_sampleRequestSpecificReader(chain->request)){
                    source = d_networkAddressNew(chain->request->source.systemId,
                                                 chain->request->source.localId,
                                                 chain->request->source.lifecycleId);
                    readerRequest = d_adminGetReaderRequest(data.admin, source);
                    d_networkAddressFree(source);
                    d_readerRequestRemoveChain(readerRequest, chain);
                    d_readerRequestFree(readerRequest);
                }
            }
            d_chainFree(chain);
            chain = c_iterTakeFirst(data.toRemove);
        }
        c_iterFree(data.toRemove);
        d_listenerUnlock(d_listener(listener));

        subscriber = d_adminGetSubscriber(data.admin);
        glListener = d_subscriberGetGroupLocalListener(subscriber);

        for (sampleRequest = d_sampleRequest(c_iterTakeFirst(requests));
             sampleRequest != NULL;
             sampleRequest = d_sampleRequest(c_iterTakeFirst(requests)))
        {
            dgroup = d_adminGetLocalGroup(data.admin,
                            sampleRequest->partition,
                            sampleRequest->topic,
                            sampleRequest->durabilityKind);

            if(d_sampleRequestSpecificReader(sampleRequest)){
                source = d_networkAddressNew(
                        sampleRequest->source.systemId,
                         sampleRequest->source.localId,
                         sampleRequest->source.lifecycleId);
                readerRequest = d_adminGetReaderRequest(data.admin, source);
                d_networkAddressFree(source);
                d_readerRequestAddGroup(readerRequest, dgroup);
                d_groupLocalListenerHandleAlignment(glListener, dgroup, readerRequest);
                d_readerRequestFree(readerRequest);
            } else {
                d_groupLocalListenerHandleAlignment(glListener, dgroup, NULL);
            }
            d_sampleRequestFree(sampleRequest);
        }
        c_iterFree(requests);

        d_printTimedEvent(durability, D_LEVEL_FINER, "%d requests left.\n",
            d_tableSize(listener->chains));

    }
    return TRUE;
}


c_bool
d_sampleChainListenerStart(
    d_sampleChainListener listener)
{
    d_admin admin;
    c_bool result = FALSE;

    assert(d_sampleChainListenerIsValid(listener));

    if(listener){
        if(d_listenerIsAttached(d_listener(listener)) == FALSE){
            admin = d_listenerGetAdmin(d_listener(listener));
            listener->fellowListener = d_eventListenerNew(
                                    D_FELLOW_REMOVED,
                                    d_sampleChainListenerNotifyFellowRemoved,
                                    listener);
            d_adminAddListener(admin, listener->fellowListener);
            result = d_readerListenerStart(d_readerListener(listener));
        }
    }
    return result;
}


c_bool
d_sampleChainListenerStop(
    d_sampleChainListener listener)
{
    d_admin admin;
    c_bool result = FALSE;

    assert(d_sampleChainListenerIsValid(listener));

    if (listener) {
        if (d_listenerIsAttached(d_listener(listener)) == TRUE) {
            admin = d_listenerGetAdmin(d_listener(listener));
            d_adminRemoveListener(admin, listener->fellowListener);
            d_eventListenerFree(listener->fellowListener);
            listener->fellowListener = NULL;
            result = d_readerListenerStop(d_readerListener(listener));
        }
    }
    return result;
}


static void
d_sampleChainListenerAddChain(
    d_sampleChainListener listener,
    d_chain chain,
    d_networkAddress addressee)
{
    d_admin admin;
    d_durability durability;
    d_publisher publisher;

    admin      = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability (admin);
    publisher  = d_adminGetPublisher(admin);

    assert (listener);
    assert (listener->chains);

    /* Set addressee */
    d_messageSetAddressee(d_message(chain->request), addressee);
    /* Insert chain. Note that the chain is NOT refCounted. */
    (void)d_tableInsert(listener->chains, chain);
    /* The chain is new, send the sampleRequest */
    d_publisherSampleRequestWrite(publisher, chain->request, addressee);
    if (chain->request->filter) {
        d_printTimedEvent(durability, D_LEVEL_FINE,
                         "Write samplerequest for group %s.%s to fellow %u using source (%u,%u,%u) and filter expression '%s'\n",
                         chain->request->partition,
                         chain->request->topic,
                         d_message(chain->request)->addressee.systemId,
                         chain->request->source.systemId,
                         chain->request->source.localId,
                         chain->request->source.lifecycleId,
                         chain->request->filter);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINE,
                         "Write samplerequest for group %s.%s to fellow %u using source (%u,%u,%u)\n",
                         chain->request->partition,
                         chain->request->topic,
                         d_message(chain->request)->addressee.systemId,
                         chain->request->source.systemId,
                         chain->request->source.localId,
                         chain->request->source.lifecycleId);
    }
}


void
d_sampleChainListenerReportGroup(
    d_sampleChainListener listener,
    d_group group)
{
    d_admin admin;

    admin = d_listenerGetAdmin(d_listener(listener));
    d_adminReportGroup(admin, group);
}

void
d_sampleChainListenerTryFulfillChains(
    d_sampleChainListener listener,
    d_group group)
{
    c_iter copy, leftOver;
    d_chain chain;
    d_admin admin;
    d_durability durability;
    c_ulong length;
    d_topic topic;
    d_partition partition;
    d_thread self = d_threadLookupSelf ();

    assert(d_sampleChainListenerIsValid(listener));

    d_threadAwake(self);
    if(listener){
        d_listenerLock(d_listener(listener));


        /* Check of length must be within lock */
        length     = c_iterLength(listener->unfulfilledChains);

        if(length > 0){
            admin      = d_listenerGetAdmin(d_listener(listener));
            durability = d_adminGetDurability(admin);
            copy       = listener->unfulfilledChains;

            d_printTimedEvent(durability, D_LEVEL_FINER,
                "Trying to find aligner again for %d groups.\n",
                length);

            listener->unfulfilledChains = c_iterNew(NULL);

            if(group){
                partition = d_groupGetPartition(group);
                topic     = d_groupGetTopic(group);
            } else {
                partition = NULL;
                topic = NULL;
            }
            d_listenerUnlock(d_listener(listener));
            leftOver = c_iterNew(NULL);
            chain = d_chain(c_iterTakeFirst(copy));

            while(chain){
                if(group){
                    if ((strcmp(partition, chain->request->partition) == 0)&&
                        (strcmp(topic, chain->request->topic) == 0))
                    {
                        d_sampleChainListenerInsertRequest(listener, chain, FALSE);
                    } else {
                        leftOver = c_iterInsert(leftOver, chain);
                    }
                } else {
                    d_sampleChainListenerInsertRequest(listener, chain, FALSE);
                }
                chain = d_chain(c_iterTakeFirst(copy));
            }

            if(group){
                os_free(partition);
                os_free(topic);
            }

            /* Do NOT free chains themselves, this is handled by the insertRequest function. */
            c_iterFree(copy);

            d_listenerLock(d_listener(listener));
            chain = d_chain(c_iterTakeFirst(leftOver));

            while(chain){
                listener->unfulfilledChains = c_iterInsert(listener->unfulfilledChains, chain);
                chain = d_chain(c_iterTakeFirst(leftOver));
            }
            d_listenerUnlock(d_listener(listener));
            c_iterFree(leftOver);
            if (c_iterLength(listener->unfulfilledChains) > 0) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Still waiting for alignment of %d groups.\n",
                    c_iterLength(listener->unfulfilledChains));
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINER, "All groups are being alignment.\n");
            }
        } else {
            d_listenerUnlock(d_listener(listener));
        }
    }
    return;
}


static void
requestGroupFromMasterIfUnknown(
    d_admin admin,
    d_nameSpace nameSpace,
    d_partition partition,
    d_topic topic,
    d_durabilityKind kind)
{
    d_durability durability;
    d_publisher publisher;
    d_networkAddress master;
    d_fellow fellow;
    d_group fellowGroup;
    d_groupsRequest request;

    assert(d_objectIsValid(d_object(nameSpace), D_NAMESPACE));
    assert(d_objectIsValid(d_object(admin), D_ADMIN));

    if(admin && nameSpace && partition && topic){
        master = d_nameSpaceGetMaster(nameSpace);

        if(!d_networkAddressIsUnaddressed(master)){
            fellow = d_adminGetFellow(admin, master);

            if(fellow){
                fellowGroup = d_fellowGetGroup(fellow, partition, topic, kind);

                if(fellowGroup){
                    /* Group already known; no need to do anything */
                    d_groupFree(fellowGroup);
                } else {
                    request    = d_groupsRequestNew(admin, partition, topic);
                    durability = d_adminGetDurability(admin);
                    publisher  = d_adminGetPublisher(admin);

                    d_messageSetAddressee(d_message(request), master);
                    d_publisherGroupsRequestWrite(publisher, request, master);

                    d_printTimedEvent(durability, D_LEVEL_FINE,
                        "Requested info for group %s.%s from master fellow '%u'.\n",
                        partition, topic, master->systemId);
                    d_groupsRequestFree(request);
                }
                d_fellowFree(fellow);
            }
        }
        d_networkAddressFree(master);
    } else {
        OS_REPORT(OS_ERROR, "durability::requestGroupFromMasterIfUnknown", 0,
                    "Precondition not met: admin=0x%"PA_PRIxADDR", nameSpace=0x%"PA_PRIxADDR", partition=0x%"PA_PRIxADDR", topic=0x%"PA_PRIxADDR"",
                    (os_address)admin, (os_address)nameSpace, (os_address)partition, (os_address)topic);
        assert(FALSE);
    }
    return;
}

struct collectGroupData {
    d_sampleRequest request;
    c_bool checkTimeRange;
    c_bool isBuiltinDCPSTopicGroup;
    c_iter list;
};

static c_bool
collectGroupsAddList(
    c_object object,
    v_groupInstance instance,
    v_groupFlushType flushType,
    c_voidp userData)
{
    v_message message;
    v_registration registration;
    struct collectGroupData *data;
    c_bool process;

    OS_UNUSED_ARG(instance);

    data = (struct collectGroupData*)userData;
    process = TRUE;

#define CHECK_TIME_RANGE(writeTime, beginTime, endTime) \
    (os_timeWCompare(writeTime, endTime) == OS_MORE) ? FALSE : \
    (data->request->withTimeRange == TRUE) && (os_timeWCompare(writeTime, beginTime) == OS_LESS) ? FALSE : \
    TRUE

    switch(flushType){
    case V_GROUP_FLUSH_REGISTRATION:
        registration = (v_registration)object;
        if (data->isBuiltinDCPSTopicGroup) {
            /* Don't process registrations for DCPSTopic as this would introduce a scalability issue since
             * there is always an alive writer per federation. If these registrations would be aligned there
             * would be n registrations stored per builtin topic instance where n is the number of
             * federations in a system.
             * The only condition for aligning a registration is when there are multiple writers
             * for the same instance. Therefore this is not an issue for other (builtin) topics.
             */
            process = FALSE;
        } else if (data->checkTimeRange) {
            process = CHECK_TIME_RANGE(registration->writeTime, c_timeToTimeW(data->request->beginTime), c_timeToTimeW(data->request->endTime));
        }
        break;
    case V_GROUP_FLUSH_UNREGISTRATION:
        registration = (v_registration)object;
        if (data->checkTimeRange) {
            process = CHECK_TIME_RANGE(registration->writeTime, c_timeToTimeW(data->request->beginTime), c_timeToTimeW(data->request->endTime));
        }
        break;
    case V_GROUP_FLUSH_MESSAGE:
        message = (v_message)object;
        if (data->checkTimeRange) {
            process = CHECK_TIME_RANGE(message->writeTime, c_timeToTimeW(data->request->beginTime), c_timeToTimeW(data->request->endTime));
        }
        if((process == TRUE) && (data->isBuiltinDCPSTopicGroup == TRUE)){
            v_topicInfoTemplate tmpl = (v_topicInfoTemplate)message;
            if (d_inBuiltinTopicNames(tmpl->userData.name, strlen(tmpl->userData.name))) {
                /* Don't process messages for DCPSTopic which advertise BuiltinTopics */
                process = FALSE;
            }
        }
        break;
    case V_GROUP_FLUSH_TRANSACTION:
        message = (v_message)object;
        if (data->checkTimeRange) {
            process = CHECK_TIME_RANGE(message->writeTime, c_timeToTimeW(data->request->beginTime), c_timeToTimeW(data->request->endTime));
        }
        break;
    }
#undef CHECK_TIME_RANGE
    if (process == TRUE) {
        struct v_groupFlushData *flushData = os_malloc(sizeof(struct v_groupFlushData));
        flushData->object = c_keep(object);
        flushData->instance = c_keep(instance);
        flushData->flushType = flushType;
        data->list = c_iterAppend(data->list, flushData);
    }
    return FALSE;
}

static c_iter
collectGroups(
    d_chain chain)
{
    c_iter list;
    v_historicalDataRequest vrequest;
    struct collectGroupData data;
    v_resourcePolicyI resourceLimits;

    list = c_iterNew(NULL);
    /* Collect all data that needs to be sent. */
    data.request        = chain->request;
    data.list           = list;

    data.isBuiltinDCPSTopicGroup = FALSE;

    if ((strcmp(v_entity(chain->vgroup->partition)->name, V_BUILTIN_PARTITION) == 0) &&
        (strcmp(v_entity(chain->vgroup->topic)->name, V_TOPICINFO_NAME) == 0)) {
        data.isBuiltinDCPSTopicGroup = TRUE;
    } else {
        data.isBuiltinDCPSTopicGroup = FALSE;
    }

    if (((d_sampleRequestSpecificReader(chain->request)) && d_sampleRequestHasCondition(chain->request)) ||
        (chain->request->filter != NULL)) {
        /* This a request that originates from a reader OR
         * This is a normal alignment request with a static content filter.
         */
        data.checkTimeRange = FALSE;

        resourceLimits.v.max_samples              = chain->request->maxSamples;
        resourceLimits.v.max_instances            = chain->request->maxInstances;
        resourceLimits.v.max_samples_per_instance = chain->request->maxSamplesPerInstance;

        vrequest = v_historicalDataRequestNew(
                        v_objectKernel(chain->vgroup),
                        chain->request->filter,
                        (const c_char**)chain->request->filterParams,
                        chain->request->filterParamsCount,
                        c_timeToTimeW(chain->request->beginTime),
                        c_timeToTimeW(chain->request->endTime),
                        &resourceLimits,
                        OS_DURATION_ZERO);

        (void)d_shmAllocAssert(vrequest, "Allocation of historicalDataRequest failed.");

        v_groupFlushActionWithCondition(
                chain->vgroup, vrequest, collectGroupsAddList, &data);

        c_free(vrequest);
    } else {
        /* This is an alignment request without any filter.
         * align all samples in this case
         */
        data.checkTimeRange = TRUE;
        v_kernelGroupTransactionBeginAccess(v_objectKernel(chain->vgroup));
        v_groupFlushAction(chain->vgroup, collectGroupsAddList, &data);
        v_kernelGroupTransactionEndAccess(v_objectKernel(chain->vgroup));
    }

    return list;
}

static void
freeGroups(
    void *o,
    c_iterActionArg arg)
{
    struct v_groupFlushData *flushData = (struct v_groupFlushData *)o;

    OS_UNUSED_ARG(arg);

    c_free(flushData->object);
    c_free(flushData->instance);
}

static void
sampleChainAddLocalHash(
    d_chain chain)
{
    c_iter list;
    struct d_groupHash groupHash;
    c_char *hash;

    list = collectGroups(chain);

    d_groupHashCalculate(&groupHash, list);
    hash = d_groupHashToString(&groupHash);
    d_sampleRequestSetHash(chain->request, hash);

    os_free(hash);

    c_iterWalk(list, freeGroups, NULL);
    c_iterFree(list);
}

static void
sampleChainAddStoreHash(
    d_sampleChainListener listener,
    d_chain chain)
{
    d_subscriber subscriber;
    d_store store;
    d_admin admin;
    d_group group;
    d_storeResult result;
    struct d_groupHash groupHash;
    c_char *hash;

    assert(d_sampleChainListenerIsValid(listener));
    assert(chain);

    admin      = d_listenerGetAdmin(d_listener(listener));
    subscriber = d_adminGetSubscriber(admin);
    store      = d_subscriberGetPersistentStore(subscriber);
    group      = d_adminGetLocalGroup(admin,
                                      chain->request->partition,
                                      chain->request->topic,
                                      chain->request->durabilityKind);

    result = d_storeMessagesLoad(store, group, &groupHash);
    if (result == D_STORE_RESULT_OK) {
        hash = d_groupHashToString(&groupHash);
        d_sampleRequestSetHash(chain->request, hash);
        d_groupSetStoreMessagesLoaded(group, TRUE);
        os_free(hash);
    }
}

static c_bool
fellowHashCapableCheck(
    d_fellow fellow,
    c_voidp userData)
{
    c_bool *capable = (c_bool *)userData;

    if (d_fellowHasCapabilitySupport(fellow) &&
        d_fellowHasCapabilityGroupHash(fellow)) {
        *capable = TRUE;
    } else {
        *capable = FALSE;
    }

    return (*capable) ? FALSE : TRUE;
}

/**
 * \brief Insert a chain to the sampleChainListener
 *
 * The chain is used to correlate if all sampleRequests that correspond
 * to the chain have been received.
 *
 * The chain to insert may not yet exist in the sampleChain.
 */
void
d_sampleChainListenerInsertRequest(
    d_sampleChainListener listener,
    d_chain chain,
    c_bool reportGroupWhenUnfullfilled)
{
    d_admin admin = NULL;
    d_durability durability = NULL;
    d_configuration configuration;
    d_networkAddress addressee, source;
    struct chainRequestHelper data;
    d_group group;
    d_readerRequest readerRequest;
    c_bool iAmAligner;
    d_nameSpace nameSpace;
    c_bool notInitial, fulfilled;
    v_handle handle;
    d_aligneeStatistics as;
    d_thread self = d_threadLookupSelf ();

    assert(d_sampleChainListenerIsValid(listener));
    assert(chain);

    d_threadAwake(self);

    if (listener && chain) {
        d_listenerLock(d_listener(listener));
        if (d_tableFind(listener->chains, chain) == NULL) {
            /* New request. */
            admin = d_listenerGetAdmin(d_listener(listener));
            durability = d_adminGetDurability(admin);
        }
        if (!admin || !durability) {
            /* Request already send out (or failure getting admin/durability). */
            d_listenerUnlock(d_listener(listener));
        }
    }

    if (admin && durability) {
        configuration    = d_durabilityGetConfiguration(durability);
        nameSpace        = d_adminGetNameSpaceForGroup(
                                            admin,
                                            chain->request->partition,
                                            chain->request->topic);
        notInitial       = d_nameSpaceIsAlignmentNotInitial(nameSpace);
        data.chain       = chain;
        data.fellow      = NULL;
        data.role        = configuration->role;
        data.durability  = durability;
        data.nameSpace   = nameSpace;

        if (nameSpace) {
            data.master = d_nameSpaceGetMaster(nameSpace);

            if (d_nameSpaceMasterIsMe(nameSpace, admin)) {

                d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Trying to find aligners for group %s.%s for nameSpace %s for which I am master.\n",
                        chain->request->partition, chain->request->topic, d_nameSpaceGetName(nameSpace));
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Trying to find an aligner for group %s.%s for nameSpace %s that has master %u.\n",
                        chain->request->partition, chain->request->topic,
                        d_nameSpaceGetName(nameSpace),
                        data.master->systemId);
            }
        } else {
            data.master = NULL;

            d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Trying to find an aligner for group %s.%s for nameSpace %s that has no master.\n",
                        chain->request->partition, chain->request->topic,
                        d_nameSpaceGetName(nameSpace));
        }

        d_adminFellowWalk(admin, findAligner, &data);

        if (d_tableSize(chain->fellows) == 0) {
            d_printTimedEvent(durability, D_LEVEL_FINER,
                "Found no fellow aligners for group %s.%s.\n",
                chain->request->partition, chain->request->topic);

            iAmAligner    = d_adminGroupInAlignerNS(
                                admin,
                                chain->request->partition,
                                chain->request->topic);

            if (iAmAligner == FALSE) {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Group %s.%s will not be aligned until an aligner fellow becomes available.\n",
                    chain->request->partition, chain->request->topic);

                if (reportGroupWhenUnfullfilled) {
                    if (notInitial) {
                        requestGroupFromMasterIfUnknown(admin, nameSpace,
                            chain->request->partition,
                            chain->request->topic,
                            chain->request->durabilityKind);
                    }
                }
                listener->unfulfilledChains = c_iterInsert(listener->unfulfilledChains, chain);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "I am aligner, group %s.%s will be marked as complete as no (potential) fellow aligners exist.\n",
                    chain->request->partition, chain->request->topic);
                group = d_adminGetLocalGroup   (admin,
                           chain->request->partition,
                           chain->request->topic,
                           chain->request->durabilityKind);

                d_groupSetComplete(group, admin);
                updateGroupStatistics(admin, group);

                if (d_sampleRequestSpecificReader(chain->request)) {
                    source = d_networkAddressNew(
                            chain->request->source.systemId,
                            chain->request->source.localId,
                            chain->request->source.lifecycleId);
                    readerRequest = d_adminGetReaderRequest(admin, source);
                    d_networkAddressFree(source);
                    assert(readerRequest);
                    d_readerRequestRemoveChain(readerRequest, chain);
                    fulfilled = d_adminCheckReaderRequestFulfilled(admin, readerRequest);

                    if (fulfilled) {
                        handle = d_readerRequestGetHandle(readerRequest);
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                            "historicalDataRequest from reader [%d, %d] fulfilled.\n",
                            handle.index, handle.serial);
                    }

                    d_readerRequestFree(readerRequest);
                }
                d_chainFree(chain);

                if (reportGroupWhenUnfullfilled) {
                    d_sampleChainListenerReportGroup(listener, group);
                }
            }
        } else {
            assert(nameSpace);
            as = d_aligneeStatisticsNew();

            if (data.fellow) {
                addressee = d_fellowGetAddress(data.fellow);

                if (d_nameSpaceGetEqualityCheck(nameSpace) &&
                    d_fellowHasCapabilitySupport(data.fellow) &&
                    d_fellowHasCapabilityGroupHash(data.fellow)) {

                    if ((d_nameSpaceMasterIsMe(nameSpace, admin) == FALSE) &&
                        (d_durabilityGetState(durability) <= D_STATE_FETCH_INITIAL)) {
                        if ((d_nameSpaceGetDurabilityKind(nameSpace) == D_DURABILITY_PERSISTENT) ||
                            (d_nameSpaceGetDurabilityKind(nameSpace) == D_DURABILITY_ALL)) {
                            sampleChainAddStoreHash(listener,chain);
                        }
                    } else {
                        sampleChainAddLocalHash(chain);
                    }
                }

                /* Add chain to listener, send samplerequest when chain becomes active */
                d_sampleChainListenerAddChain(listener, chain, addressee);

                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Inserted new sampleRequest for group %s.%s for " \
                    "complete fellow %u.\n",
                    chain->request->partition, chain->request->topic,
                    d_message(chain->request)->addressee.systemId);
                as->aligneeRequestsSentDif = 1;
                as->aligneeRequestsOpenDif = 1;
                d_networkAddressFree(addressee);
            } else if (d_nameSpaceMasterIsMe(nameSpace, admin)) {
                c_bool hashCapable = FALSE;
                addressee = d_networkAddressUnaddressed();

                /* Check if at least one of the fellows supports AlignOnChange */
                if (d_nameSpaceGetEqualityCheck(nameSpace)) {
                    d_adminFellowWalk(admin, fellowHashCapableCheck, &hashCapable);
                    if (hashCapable) {
                        sampleChainAddLocalHash(chain);
                    }
                }

                /* Add chain to listener, send samplerequest when chain becomes active */
                d_sampleChainListenerAddChain(listener, chain, addressee);

                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "No complete fellow found for group %s.%s, but I am the master for this one so inserted request for %u fellows.\n",
                    chain->request->partition, chain->request->topic,
                    d_tableSize(chain->fellows));
                as->aligneeRequestsSentDif = 1;
                as->aligneeRequestsOpenDif = 1;
                d_networkAddressFree(addressee);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Found %u (potential) fellow aligners for group %s.%s.\n",
                        d_tableSize(chain->fellows),
                        chain->request->partition, chain->request->topic);

                d_tableFree(chain->fellows);
                chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
                listener->unfulfilledChains = c_iterInsert(listener->unfulfilledChains, chain);

                group = d_adminGetLocalGroup   (admin,
                                                chain->request->partition,
                                                chain->request->topic,
                                                chain->request->durabilityKind);

                d_printTimedEvent(durability, D_LEVEL_FINE,
                        "Group %s.%s will not be aligned until the master fellow becomes complete (reportGroupWhenUnfullfilled=%s, notInitial=%s).\n",
                        chain->request->partition, chain->request->topic,
                        reportGroupWhenUnfullfilled?"TRUE":"FALSE",
                        notInitial?"TRUE":"FALSE");

                if(reportGroupWhenUnfullfilled){
                    d_sampleChainListenerReportGroup(listener, group);

                    if (notInitial) {
                        requestGroupFromMasterIfUnknown(admin, nameSpace,
                                                chain->request->partition,
                                                chain->request->topic,
                                                chain->request->durabilityKind);
                    }
                }
            }
            as->aligneeRequestsWaiting = c_iterLength(listener->unfulfilledChains);
            d_durabilityUpdateStatistics(durability, d_statisticsUpdateAlignee, as);
            d_aligneeStatisticsFree(as);
        }
        if(data.master){
            d_networkAddressFree(data.master);
        }
        d_listenerUnlock(d_listener(listener));
    }
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

    if(handleResult == V_HANDLE_OK){
        topicName = v_entityName(v_groupTopic(vgroup));
        partitionName = v_entityName(v_groupPartition(vgroup));

        if(topicName && partitionName){
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

static c_bool
calculateEqualCount(
    d_chainLink link,
    c_voidp args)
{
    c_ulong *count = (c_ulong *) args;
    if (link->sampleCount == D_GROUP_IS_EQUAL) {
        (*count)++;
    }
    return TRUE;
}


c_bool
d_sampleChainListenerCheckChainComplete(
    d_sampleChainListener listener,
    d_chain chain)
{
    d_group dgroup;
    v_group vgroup;
    c_char *partition, *topic;
    d_admin admin;
    d_durability durability;
    c_bool result = FALSE;
    d_resendAction resendData;
    d_action action;
    d_networkAddress source;
    d_readerRequest readerRequest;
    d_aligneeStatistics as;
    v_handle handle;
    c_bool fulfilled;
    struct readBeadHelper beadHelper;
    struct findEntryHelper entryHelper;
    v_handleResult handleResult;
    v_reader vreader, *vreaderPtr;
    d_nameSpace nameSpace, myNameSpace;
    d_mergeAction mergeAction;
    d_mergeState newState;
    d_mergePolicy mergePolicy;
    c_ulong chainCount;
    v_writeResult writeResult;
    d_conflictResolver conflictResolver;
    c_ulong equalCount;
    c_ulong linkCount;
    os_timeW t;

    myNameSpace = NULL;

    assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

    if (d_tableSize(chain->fellows) == d_tableSize(chain->links)) {
        /* All fellows have sent their link */
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        conflictResolver = admin->conflictResolver;
        dgroup = d_adminGetLocalGroup(
                            admin,
                            chain->request->partition,
                            chain->request->topic,
                            chain->request->durabilityKind);
        if (chain->samplesExpect >= 0) {
            if (((c_ulong)chain->samplesExpect) == d_tableSize(chain->beads)) {
                /* All samples have been received. */
                vgroup = d_groupGetKernelGroup(dgroup);
                partition = d_groupGetPartition(dgroup);
                topic = d_groupGetTopic(dgroup);

                d_printTimedEvent(durability, D_LEVEL_FINE,
                    "Received %u beads for group %s.%s.\n",
                    d_tableSize(chain->beads), partition, topic);

                resendData = d_resendActionNew(listener, dgroup);
                as = d_aligneeStatisticsNew();
                beadHelper.action = resendData;
                beadHelper.totalCount = 0;
                beadHelper.writeCount = 0;
                beadHelper.disposeCount = 0;
                beadHelper.writeDisposeCount = 0;
                beadHelper.registerCount = 0;
                beadHelper.unregisterCount = 0;
                beadHelper.eotCount = 0;
                beadHelper.entry = NULL;
                beadHelper.self = d_threadLookupSelf ();

                if (d_sampleRequestSpecificReader(chain->request)) {
                    source = d_networkAddressNew(
                            chain->request->source.systemId,
                            chain->request->source.localId,
                            chain->request->source.lifecycleId);
                    readerRequest = d_adminGetReaderRequest(admin, source);
                    d_networkAddressFree(source);
                    assert(readerRequest);

                    handle = d_readerRequestGetHandle(readerRequest);
                    handleResult = v_handleClaim(handle, (v_object*)(vreaderPtr = &vreader));

                    if(handleResult == V_HANDLE_OK){
                        entryHelper.partition = chain->request->partition;
                        entryHelper.topic     = chain->request->topic;
                        entryHelper.entry     = NULL;
                        v_readerWalkEntries(vreader, (c_action)findEntry, &entryHelper);

                        if(entryHelper.entry){
                            beadHelper.entry = entryHelper.entry;
                            d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                    "Unable to lookup entry for reader "\
                                    "[%d, %d] for group %s.%s.\n",
                                    handle.index, handle.serial,
                                    chain->request->partition,
                                    chain->request->topic);
                        }
                        v_handleRelease(handle);
                    }

                    d_readerRequestRemoveChain(readerRequest, chain);
                    fulfilled = d_adminCheckReaderRequestFulfilled(admin, readerRequest);
                    d_resendActionFree(resendData);
                    d_readerRequestFree(readerRequest);
                    d_printTimedEvent(durability, D_LEVEL_FINE,
                        "Reader [%d, %d] has now received requested "\
                        "historical data for group %s.%s.\n",
                        handle.index, handle.serial,
                        chain->request->partition, chain->request->topic);

                    if(fulfilled){
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                            "historicalDataRequest from reader [%d, %d] fulfilled.\n",
                             handle.index, handle.serial);
                    }

                    d_printTimedEvent(durability, D_LEVEL_FINE,
                        "Injected: TOTAL: %d, WRITE: %d, DISPOSED: %d, WRITE_DISPOSED: %d, REGISTER: %d, UNREGISTER: %d, EOT: %d.\n",
                            beadHelper.totalCount, beadHelper.writeCount,
                            beadHelper.disposeCount, beadHelper.writeDisposeCount,
                            beadHelper.registerCount, beadHelper.unregisterCount,
                            beadHelper.eotCount);

                    as->aligneeSamplesTotalDif          = beadHelper.totalCount;
                    as->aligneeSamplesRegisterDif       = beadHelper.registerCount;
                    as->aligneeSamplesWriteDif          = beadHelper.writeCount;
                    as->aligneeSamplesDisposeDif        = beadHelper.disposeCount;
                    as->aligneeSamplesWriteDisposeDif   = beadHelper.writeDisposeCount;
                    as->aligneeSamplesUnregisterDif     = beadHelper.unregisterCount;
                    as->aligneeTotalSizeDif             = chain->receivedSize;
                } else {
                    /** Need to find out whether this chain is part of a
                     *  mergeAction.
                     */
                    mergeAction = d_sampleChainListenerGetMergeAction(listener, chain);

                    if (mergeAction) {

                        d_traceMergeAction(mergeAction, "Chain received");

                        equalCount = 0;
                        linkCount = d_tableSize(chain->links);
                        d_tableWalk(chain->links, calculateEqualCount, &equalCount);

                        nameSpace = d_mergeActionGetNameSpace(mergeAction);
                        myNameSpace = d_adminGetNameSpace(admin, d_nameSpaceGetName(nameSpace));
                        newState = d_mergeActionGetNewState(mergeAction);
                        mergePolicy = d_nameSpaceGetMergePolicy(myNameSpace, newState->role);
                        /* set the merge policy in the beadHelper */
                        beadHelper.mergePolicy = mergePolicy;

                        {
                            const char *mergePolicyStr;
                            switch (mergePolicy) {
                                case D_MERGE_DELETE: mergePolicyStr = "pre DELETE"; break;
                                case D_MERGE_REPLACE: mergePolicyStr = "pre REPLACE"; break;
                                case D_MERGE_CATCHUP: mergePolicyStr = "pre CATCHUP"; break;
                                case D_MERGE_MERGE: mergePolicyStr = "pre MERGE"; break;
                                case D_MERGE_IGNORE: mergePolicyStr = "pre IGNORE"; break;
                                default: mergePolicyStr = "pre UNDEFINED"; break;
                            }
                            d_tracegroup(durability, vgroup, mergePolicyStr);
                        }
                        switch(mergePolicy){
                        case D_MERGE_DELETE:
                            /* apply the DELETE merge policy */
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                    "Applying DELETE merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);
                            /* Dispose all data before the requestTime of the
                             * the sampleRequest that lead to the retrieval of the
                             * sampleChain.
                             */
                            d_timestampToTimeW(&t, &chain->request->requestTime, IS_Y2038READY(chain->request));
                            writeResult = v_groupDisposeAllMatchingInstances(vgroup, t, L_REPLACED, NULL, NULL);
                            if ( writeResult == V_WRITE_SUCCESS ) {
                                os_timeE te;

                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                                    "Samples before timestamp %"PA_PRItime" disposed for group %s.%s\n",
                                                    OS_TIMEW_PRINT(t),
                                                    chain->request->partition,
                                                    chain->request->topic);
                                /* Delete all historical data for the group with a writeTime up to and
                                 * including timestamp. The timestamp must be an elapsed time, so we
                                 * need to calculate the corresponding elapsed time of t.
                                 */
                                te = d_timeWToTimeE(t);
                                writeResult = v_groupDeleteHistoricalData(vgroup, te);
                                if ( writeResult == V_WRITE_SUCCESS ) {
                                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                                        "Historical data before timestamp %"PA_PRItime" deleted for group %s.%s\n",
                                                        OS_TIMEW_PRINT(t),
                                                        chain->request->partition,
                                                        chain->request->topic);
                                    /* Assume that each DELETE leads to state change of the set */
                                    d_mergeActionSetStateChanged(mergeAction, TRUE);
                                } else {
                                    OS_REPORT(OS_ERROR,
                                                "d_sampleChainListenerCheckChainComplete",0,
                                                "Failed to delete historical data before timestamp %"PA_PRItime" for group %s.%s",
                                                OS_TIMEW_PRINT(t),
                                                chain->request->partition,
                                                chain->request->topic);
                                }
                            } else {
                                OS_REPORT(OS_ERROR,
                                            "d_sampleChainListenerCheckChainComplete",0,
                                            "Failed to dispose all instances for group %s.%s",
                                            chain->request->partition, chain->request->topic);
                            }
                            break;
                        case D_MERGE_REPLACE:
                            /* Apply the REPLACE merge policy */
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                    "Applying REPLACE merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);

                            d_timestampToTimeW(&t, &chain->request->requestTime, IS_Y2038READY(chain->request));
                            if (equalCount == 0) {
                                /* When no equal link is received dispose all data
                                 * before the requestTime of the sampleRequest
                                 * that lead to the retrieval of the sampleChain.
                                 */
                                writeResult = v_groupDisposeAllMatchingInstances(vgroup, t, L_REPLACED, NULL, NULL);
                            } else {
                                writeResult = V_WRITE_SUCCESS;
                            }
                            if (equalCount != linkCount) {
                                if ( writeResult == V_WRITE_SUCCESS ) {
                                    os_timeE te;

                                    d_printTimedEvent(durability, D_LEVEL_FINER,
                                                        "Samples before timestamp %"PA_PRItime" disposed for group %s.%s\n",
                                                        OS_TIMEW_PRINT(t),
                                                        chain->request->partition,
                                                        chain->request->topic);
                                    /* Mark all reader instances with the L_REPLACED flag to indicate
                                     * that the REPLACE merge policy is about to inject historical samples.
                                     */
                                    v_groupMarkGroupInstanceStates(vgroup, L_REPLACED);
                                    /* Delete all historical data for the group with a writeTime up to and
                                     * including timestamp.
                                     */
                                    te = d_timeWToTimeE(t);
                                    writeResult = v_groupDeleteHistoricalData(vgroup, te);
                                    if ( writeResult == V_WRITE_SUCCESS ) {
                                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                                                            "Historical data before timestamp %"PA_PRItime" deleted for group %s.%s\n",
                                                            OS_TIMEW_PRINT(t),
                                                            chain->request->partition,
                                                            chain->request->topic);
                                        /* Per bead, remove the dispose message that was generated by the
                                         * v_groupDisposeAll() from the reader instance, update the reader
                                         * state, and inject historical data.
                                         */
                                         d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                                    } else {
                                        OS_REPORT(OS_ERROR,
                                                    "d_sampleChainListenerCheckChainComplete",0,
                                                    "Failed to delete historical data before timestamp %"PA_PRItime" for group %s.%s",
                                                    OS_TIMEW_PRINT(t),
                                                    chain->request->partition,
                                                    chain->request->topic);
                                    }
                                    /* Reset the marker. */
                                    v_groupUnmarkReaderInstanceStates(vgroup, L_REPLACED);
                                    /* Assume that the REPLACE always changes the data set */
                                    d_mergeActionSetStateChanged(mergeAction, TRUE);
                                } else {
                                    OS_REPORT(OS_ERROR,
                                                "d_sampleChainListenerCheckChainComplete",0,
                                                "Failed to dispose all instances for group %s.%s",
                                                chain->request->partition, chain->request->topic);
                                }
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                        "No need to do anything for group %s.%s because my current set is equal to the set to inject\n",
                                        chain->request->partition, chain->request->topic);
                            }
                            break;
                        case D_MERGE_CATCHUP:
                            /* Apply the CATCHUP merge policy */
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                    "Applying CATCHUP merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);
                            /* If a different data set is aligned then mark all existing
                             * instances to indicate that a CATCHUP operation is about to
                             * take place. Only if a different data set is aligned (i.e.,
                             * no equal link is received) we need to mark our current set
                             * in order to dispose instances that don't exist later on.
                             */
                            d_timestampToTimeW(&t, &chain->request->requestTime, IS_Y2038READY(chain->request));

                            if (equalCount == 0) {
                                assert(equalCount != linkCount);

                                /* No equal link received */
                                v_groupMarkGroupInstanceStates(vgroup, L_MARK);
                                /* At least one none-equal link has been received, so at
                                 * least one of the aligner fellows has a different set.
                                 * Insert the historical data. While doing so
                                 * clear the L_MARK flag for existing instances,
                                 * and make sure that the instance state is set to L_NEW
                                 * for new instances
                                 */
                                d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                                /* Now dispose all messages before the chain->request->time for
                                 * all instances with the L_MARK flag (these were the instances
                                 * that were untouched by the CATCHUP merge action). Collect
                                 * the instances that are touched and return these in the
                                 * instances argument.
                                 */
                                if ((writeResult = v_groupSweepMarkedInstances(vgroup, t)) != V_WRITE_SUCCESS) {
                                    OS_REPORT(OS_ERROR,
                                                "d_sampleChainListenerCheckChainComplete",0,
                                                "Failed to dispose marked instances for group %s.%s",
                                                chain->request->partition, chain->request->topic);
                                }
                                /* Reset the marker. */
                                v_groupUnmarkGroupInstanceStates(vgroup, L_MARK);
                                /* Assume that the CATCHUP always changes the data set */
                                d_mergeActionSetStateChanged(mergeAction, TRUE);
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                        "No need to do anything for group %s.%s because my current set is equal to the set to inject\n",
                                        chain->request->partition, chain->request->topic);
                            }
                            break;
                        case D_MERGE_MERGE:
                            /* Apply the MERGE merge policy */
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                    "Applying MERGE merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);
                            /* inject the beads */
                            if (equalCount != linkCount) {
                                d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                                /* Assume that MERGE with a non-empty set leads to a state change */
                                if (beadHelper.totalCount != 0) {
                                    d_mergeActionSetStateChanged(mergeAction, TRUE);
                                }
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                        "No need to do anything for group %s.%s because my current set is equal to the set to inject\n",
                                        chain->request->partition, chain->request->topic);
                            }
                            break;
                        case D_MERGE_IGNORE:
                            /* Apply the IGNORE merge policy */
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                    "Applying IGNORE merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);
                            /* IGNORE never leads to a state change */
                            break;
                        default:
                            /* A new merge policy? How exciting...*/
                            assert(FALSE);
                            break;
                        }
                        d_tracegroup(durability, vgroup, "final state");
                    } else {
                        /* The chain is not part of a mergeAction, but it can be
                         * an initial merge action. Simply inject the beads
                         */
                        beadHelper.mergePolicy = D_MERGE_IGNORE;
                        d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                    }

                    /* Messages have been rejected. A resend must be scheduled. */
                    if(resendData->messages != NULL){
                        os_duration sleepTime = OS_DURATION_INIT(1,0);
                        os_timeM actionTime = os_timeMAdd(os_timeMGet(), sleepTime);

                        action = d_actionNew(actionTime, sleepTime, d_resendRejected, resendData);
                        d_actionQueueAdd(listener->resendQueue, action);
                    } else{
                        d_resendActionFree(resendData);
                        d_groupSetComplete(dgroup, admin);
                        updateGroupStatistics(admin, dgroup);

                        d_printTimedEvent(durability, D_LEVEL_FINER,
                            "Group %s.%s is now complete.\n", partition, topic);
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                            "Injected: TOTAL: %d, WRITE: %d, DISPOSE: %d, WRITE_DISPOSE: %d, REGISTER: %d, UNREGISTER: %d, EOT: %d.\n",
                                    beadHelper.totalCount, beadHelper.writeCount,
                                    beadHelper.disposeCount, beadHelper.writeDisposeCount,
                                    beadHelper.registerCount, beadHelper.unregisterCount,
                                    beadHelper.eotCount);

                        as->aligneeSamplesTotalDif          = beadHelper.totalCount;
                        as->aligneeSamplesRegisterDif       = beadHelper.registerCount;
                        as->aligneeSamplesWriteDif          = beadHelper.writeCount;
                        as->aligneeSamplesDisposeDif        = beadHelper.disposeCount;
                        as->aligneeSamplesWriteDisposeDif   = beadHelper.writeDisposeCount;
                        as->aligneeSamplesUnregisterDif     = beadHelper.unregisterCount;
                        as->aligneeTotalSizeDif             = chain->receivedSize;
                        d_sampleChainListenerReportGroup(listener, dgroup);
                    }

                    if (mergeAction) {
                        c_bool success = d_mergeActionRemoveChain(mergeAction, chain);
                        assert(success);
                        (void)success;

                        chainCount = d_mergeActionGetChainCount(mergeAction);

                        if (chainCount == 0) {

                            d_trace(D_TRACE_CHAINS, "Removing mergeAction %p\n", (void *)mergeAction);

                            d_tableRemove(listener->mergeActions, mergeAction);
                            nameSpace = d_mergeActionGetNameSpace(mergeAction);

                            assert (myNameSpace);

                            /* Decide whether or not to update my state */
                            if (!d_nameSpaceMasterIsMe (myNameSpace, admin)) {
                                /* If I am a slave I need to follow my master's merge states */

                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                    "Slaving to my master, set the state of namespace '%s' to '%d' for role '%s'\n",
                                   d_nameSpaceGetName(nameSpace),
                                   d_mergeActionGetNewState(mergeAction)->value,
                                   d_mergeActionGetNewState(mergeAction)->role);

                                d_nameSpaceReplaceMergeStates (myNameSpace, nameSpace);
                                d_nameSpaceSetMergeState(myNameSpace, d_mergeActionGetNewState(mergeAction));
                            } else if (!d_mergeActionGetStateChanged(mergeAction)) {
                                /* I am the master but my state was not changed */

                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                    "No need to update the state of namespace '%s' for role '%s' because I "
                                    "did not change my state while applying my merge policy\n",
                                    d_nameSpaceGetName(nameSpace), d_mergeActionGetNewState(mergeAction)->role);

                            } else {
                                /* I am the master and changed my state, so I need to update my state */
                                d_nameSpaceSetMergeState(myNameSpace, d_mergeActionGetNewState(mergeAction));

                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                     "Updating state of namespace '%s' to '%d' for role '%s'\n",
                                     d_nameSpaceGetName(nameSpace),
                                     d_mergeActionGetNewState(mergeAction)->value,
                                     d_mergeActionGetNewState(mergeAction)->role);
                            }

                            d_mergeActionUpdateMergeCount(mergeAction);

                            /* Indicate that the mergeAction is completed
                             * This function will also republish namespaces when necessary
                             */
                            d_conflictResolverResetConflictInProgress(conflictResolver, mergeAction->conflict);

                            d_mergeActionFree(mergeAction);

                        }
                    }

                    if (myNameSpace) {
                        d_nameSpaceFree (myNameSpace);
                    }
                }
                c_free(vgroup);

                as->aligneeRequestsWaiting = c_iterLength(listener->unfulfilledChains);
                as->aligneeRequestsOpenDif = -1;
                d_durabilityUpdateStatistics(durability, d_statisticsUpdateAlignee, as);
                d_aligneeStatisticsFree(as);

                result = TRUE;
                os_free(partition);
                os_free(topic);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Expecting %d samples, received %u so far (1)\n",
                    chain->samplesExpect, d_tableSize(chain->beads));
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Expecting %d samples, received %u so far (2)\n",
                chain->samplesExpect, d_tableSize(chain->beads));
        }
    }
    return result;
}

void
d_sampleChainListenerReportStatus(
    d_sampleChainListener listener)
{
    d_admin admin;
    d_durability durability;
    d_chain chain;
    c_ulong i;

    assert(d_sampleChainListenerIsValid(listener));

    if(listener){
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);

        d_listenerLock(d_listener(listener));

        d_printTimedEvent(durability, D_LEVEL_FINEST, "The following groups are currently being aligned:\n");

        d_tableWalk(listener->chains, d_chainReportStatus, durability);

        d_printTimedEvent(durability, D_LEVEL_FINEST, "The following groups have no aligner yet:\n");

        for(i=0; i<c_iterLength(listener->unfulfilledChains); i++){
            chain = c_iterObject(listener->unfulfilledChains, i);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                 "- No aligner yet for group: '%s.%s'.\n", chain->request->partition, chain->request->topic);
        }
        d_listenerUnlock(d_listener(listener));
    }
    return;
}

struct processChainsHelper {
    d_sampleChainListener listener;
    d_publisher publisher;
    c_iter fellows;
    d_durability durability;
    d_aligneeStatistics as;
    d_mergeAction mergeAction;
    d_chain chain;
    c_bool equalityCheck;
};


static c_bool
publishSampleRequests(
    d_fellow fellow,
    c_voidp args)
{
    struct processChainsHelper* helper;

    helper = (struct processChainsHelper*)args;

    if (helper->mergeAction) {
        d_printTimedEvent(helper->durability, D_LEVEL_FINE,
            "Going to send sampleRequest to fellow %u as part of merge action %p\n",
            fellow->address->systemId, (void *)(helper->mergeAction));
    } else {
        d_printTimedEvent(helper->durability, D_LEVEL_FINE,
            "Going to send sampleRequest to fellow %u\n",
            fellow->address->systemId, (void *)(helper->mergeAction));
    }

    /* Update statistics */
    helper->as->aligneeRequestsSentDif += 1;
    helper->as->aligneeRequestsOpenDif += 1;
    /* Update the fellow with the number of sampleRequests */
    d_fellowRequestAdd(fellow);
    /* Add the fellow as the recipient of the sampleRequest */
    if (d_tableInsert(helper->chain->fellows, d_objectKeep(d_object(fellow))) != NULL) {
        d_fellowFree(fellow);
    } else {
        /* Calculate hash when fellow supports hash and when not in request already */
        if ((helper->equalityCheck) &&
            (d_fellowHasCapabilityGroupHash(fellow)) &&
            (d_sampleRequestHasHash(helper->chain->request, fellow) == FALSE)) {
            sampleChainAddLocalHash(helper->chain);
        }
        /* Publish the sampleRequest */
        d_sampleChainListenerAddChain(helper->listener, helper->chain, fellow->address);
    }
    return TRUE;
}


/**
 * \brief Send a sampleRequest for the chain to all fellows in the mergeAction
 */
static c_bool
processChains(
    d_chain chain,
    c_voidp args)
{
    struct processChainsHelper* helper;

    assert(d_chainIsValid(chain));

    helper = (struct processChainsHelper*)args;
    helper->chain = chain;
    /* Publish sampleRequests for the chain to all fellows
     * in the mergeAction.
     */
    assert(d_mergeActionIsValid(helper->mergeAction));
    (void)d_tableWalk(helper->mergeAction->fellows, publishSampleRequests, helper);
    helper->chain = NULL;
    return TRUE;
}


static void
remove_non_existent_fellows(
    d_admin admin,
    d_table fellows)
{
    /* Remove the fellows from the table if they do not exist anymore.
     * This is possible because fellows may have left before taking the
     * listenerLock.
     */

    d_durability durability;

    durability = d_adminGetDurability(admin);
    if (fellows) {
        c_iter to_remove = c_iterNew(NULL);
        d_tableIter tableIter;
        d_fellow fellow, found;

        fellow = d_fellow(d_tableIterFirst(fellows, &tableIter));
        /* First get the fellows that do not exist anymore. */
        while (fellow) {
            if ((found = d_adminGetFellow(admin, fellow->address)) == NULL) {

                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    " - NOT going to send sampleRequest to fellow %u, the fellow disappeared\n",
                    fellow->address->systemId);

                to_remove = c_iterAppend(to_remove, fellow);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    " - Going to send sampleRequest to fellow %u\n",
                    fellow->address->systemId);

                d_fellowFree(found);
            }
            fellow = d_fellow(d_tableIterNext(&tableIter));
        }
        /* Next remove the fellows from the table */
        while (((fellow = d_fellow(c_iterTakeFirst(to_remove))) != NULL)) {
           (void)d_tableRemove(fellows, fellow);
            d_fellowFree(fellow);
        }
        c_iterFree(to_remove);
    }
}


c_bool
d_sampleChainListenerInsertMergeAction(
    d_sampleChainListener listener,
    d_mergeAction action)
{
    d_admin admin;
    struct processChainsHelper helper;
    d_mergeAction duplicate;
    c_bool result;
    d_conflictResolver conflictResolver;
    d_nameSpace myNameSpace;
    d_durability durability;

    assert(d_sampleChainListenerIsValid(listener));
    assert(action);
    result = FALSE;

    if (listener && action) {

        /* Lock the sampleChainListener to prevent that chains are removed
         * (because fellows are removed) while at the same time sampleRequests
         * are being sent to the fellow. Taking the lock at this point in time prevents
         * d_sampleChainListenerNotifyFellowRemoved to proceed. Only when the sampleRequests
         * for this mergeAction have been sent the lock is released, and
         * d_sampleChainListenerNotifyFellowRemoved can proceed to remove any outstanding
         * chains for fellows that have been removed.
         */

        d_listenerLock(d_listener(listener));
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        conflictResolver = admin->conflictResolver;

        d_printTimedEvent(durability, D_LEVEL_FINEST,
              "Determining the target fellows for merge action %p\n",
              (void *)action);

        remove_non_existent_fellows(admin, action->fellows);

        if (d_tableSize(action->fellows) == 0) {
            d_printTimedEvent(durability, D_LEVEL_FINE,
                  "All fellows in merge action %p have disappeared, canceling this merge action \n",
                  (void *)action);
        } else {
            duplicate = d_mergeAction(d_tableInsert(listener->mergeActions, action));

            if (duplicate == NULL) {
                /* Samples will be requested. Remember that the conflictResolver
                 * is in progress with handling a conflict to prevent that a new
                 * conflict is started before the previous one is completed.
                 * The conflict handling is completed when conflictInProgress is
                 * set to FALSE (see d_sampleChainListenerCheckChainComplete)
                 * . */
                d_conflictResolverSetConflictInProgress(conflictResolver, action->conflict);

                helper.mergeAction = action;
                helper.as = d_aligneeStatisticsNew();
                helper.durability = d_adminGetDurability(admin);
                helper.publisher = d_adminGetPublisher(admin);
                helper.listener = listener;
                helper.equalityCheck = FALSE;

                d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "Merge action %p applies to %d fellows\n",
                      (void *)action, d_tableSize(action->fellows));

                myNameSpace = d_adminGetNameSpace(admin, d_nameSpaceGetName(action->nameSpace));
                if (myNameSpace) {
                    helper.equalityCheck = d_nameSpaceGetEqualityCheck(myNameSpace);
                }
                d_nameSpaceFree(myNameSpace);

                /* Walk over all chains in the mergeAction to send out sample requests */
                d_mergeActionChainWalk(action, processChains, &helper);

                d_durabilityUpdateStatistics(helper.durability,
                        d_statisticsUpdateAlignee, helper.as);

                d_aligneeStatisticsFree(helper.as);

                result = TRUE;
            }
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}

void
d_sampleChainListenerCheckUnfulfilled(
    d_sampleChainListener listener,
    d_nameSpace nameSpace,
    d_networkAddress fellowAddress) {

    d_admin admin;
    d_chain chain;
    d_groupsRequest request;
    d_publisher publisher;
    c_ulong i;

    if (listener) {
        admin = d_listenerGetAdmin(d_listener(listener));
        publisher = d_adminGetPublisher(admin);

        d_listenerLock(d_listener(listener));

        for(i=0; i<c_iterLength(listener->unfulfilledChains); i++) {
            chain = c_iterObject(listener->unfulfilledChains, i);

            if (d_nameSpaceIsIn(nameSpace, chain->request->partition, chain->request->topic)) {
                /* Re-request group from (master) fellow so we're sure to have the latest group completeness */
                request = d_groupsRequestNew(admin, chain->request->partition, chain->request->topic);

                /* Write request */
                d_publisherGroupsRequestWrite(publisher, request, fellowAddress);

                /* Free request */
                d_groupsRequestFree(request);
            }
        }

        d_listenerUnlock(d_listener(listener));
    }
}


/**
 * \brief Compare two chains.
 *
 * Two chains are considered equal if the partition, topic,
 * durabilityKind and source of their requests are equal.
 *
 * Note:
 * The filter is NOT involved in the sampleChain comparison.
 * This is because a chain lookup is done based on an incoming
 * samplechain that does not carry a filter (see
 * d_sampleChainListenerFindChain). This still works because:
 * - if a reader requests historical data with a filter then
 *   the source of the reader is sufficient to identify the
 *   outstanding chain
 * - for a give partitio/topic there is at most a single
 *   static content filter, so the filter would not do much
 *   in the comparison anyway.
 */
int
d_chainCompare(
    d_chain chain1,
    d_chain chain2)
{
    int result;

    assert(d_chainIsValid(chain1));
    assert(d_chainIsValid(chain2));
    assert(chain1->request);
    assert(chain2->request);

    /* Check if partition/topic match */
    result = strcmp(chain1->request->partition, chain2->request->partition);
    if (result == 0) {
        result = strcmp(chain1->request->topic, chain2->request->topic);
        if (result == 0) {
            /* Check if durabilityKind matches */
            if (chain1->request->durabilityKind == chain2->request->durabilityKind) {
                /* Check if source matches */
                result = d_networkAddressCompare(
                                &chain1->request->source, &chain2->request->source);
            } else if (chain1->request->durabilityKind > chain2->request->durabilityKind) {
                result = 1;
            } else {
                result = -1;
            }
        }
    }
    return result;
}

