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

#include "d__nameSpacesListener.h"
#include "d_nameSpacesListener.h"
#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d_listener.h"
#include "d_fellow.h"
#include "d_networkAddress.h"
#include "d_nameSpaces.h"
#include "d_nameSpacesRequest.h"
#include "d_nameSpace.h"
#include "d_groupsRequest.h"
#include "d_message.h"
#include "d_admin.h"
#include "d_publisher.h"
#include "d_subscriber.h"
#include "d_sampleChainListener.h"
#include "d_misc.h"
#include "d_configuration.h"
#include "v_time.h"
#include "os_heap.h"
#include "os_report.h"

struct compatibilityHelper {
    d_fellow fellow;
    c_bool compatible;
    d_nameSpace ns;
    d_nameSpace fellowNs; /* For easy debugging */
};

/* Walk fellow namespaces */
static c_bool
isFellowNameSpaceCompatible(
    d_nameSpace fellowNs,
    c_voidp args)
{
    struct compatibilityHelper* walkData;
    walkData = (struct compatibilityHelper*)args;

    /* If nameSpace name is equal, policy of namespace must be equal too */
    if (strcmp (d_nameSpaceGetName(fellowNs), d_nameSpaceGetName(walkData->ns)) == 0) {
        walkData->fellowNs = fellowNs;
        return (d_nameSpaceCompatibilityCompare(walkData->ns, fellowNs) == 0);
    }

    return TRUE;
}

/* Walk admin namespaces */
static void
areFellowNameSpacesCompatible(
    d_nameSpace adminNs,
    c_voidp args)
{
    struct compatibilityHelper* walkData;
    d_networkAddress address;
    char* localPartitions;
    char* remotePartitions;

    walkData    = (struct compatibilityHelper*)args;

    walkData->ns = adminNs;

    if (!d_fellowNameSpaceWalk(walkData->fellow, isFellowNameSpaceCompatible, walkData))
    {
        walkData->compatible = FALSE;
        localPartitions = d_nameSpaceGetPartitionTopics(adminNs);
        remotePartitions = d_nameSpaceGetPartitionTopics(walkData->fellowNs);
        address = d_fellowGetAddress (walkData->fellow);

        OS_REPORT_5(OS_ERROR, D_CONTEXT, 0,
            "NameSpace configuration of remote durability service '%u' for NameSpace "\
            "'%s' is incompatible with local NameSpace '%s'. Partition(-Topic) expressions "\
            "are '%s'(local) and '%s'(remote).",
            address->systemId, d_nameSpaceGetName(walkData->fellowNs),
            d_nameSpaceGetName(adminNs), localPartitions,
            remotePartitions);

        d_networkAddressFree(address);
        os_free (localPartitions);
        os_free (remotePartitions);
    }
}

static c_bool
isFellowStateCompatible(
    d_durability durability,
    d_fellow fellow)
{
    d_serviceState state, fellowState;
    c_bool allowed;

    allowed     = FALSE;
    fellowState = d_fellowGetState(fellow);
    state       = d_durabilityGetState(durability);

    switch(state){
        case D_STATE_INIT:
        case D_STATE_DISCOVER_FELLOWS_GROUPS:
            switch(fellowState){
                case D_STATE_INIT:
                case D_STATE_DISCOVER_FELLOWS_GROUPS:
                case D_STATE_INJECT_PERSISTENT:
                case D_STATE_FETCH_INITIAL:
                case D_STATE_FETCH:
                case D_STATE_ALIGN:
                case D_STATE_FETCH_ALIGN:
                case D_STATE_COMPLETE:
                case D_STATE_DISCOVER_LOCAL_GROUPS:
                   allowed = TRUE;
                    break;
                case D_STATE_DISCOVER_PERSISTENT_SOURCE:
                case D_STATE_TERMINATING:
                case D_STATE_TERMINATED:
                    allowed = FALSE;
                    break;
                default:
                    assert(FALSE);
                    allowed = FALSE;
                    break;
            }
            break;
        case D_STATE_INJECT_PERSISTENT:
        case D_STATE_DISCOVER_LOCAL_GROUPS:
        case D_STATE_FETCH_INITIAL:
        case D_STATE_FETCH:
        case D_STATE_ALIGN:
        case D_STATE_FETCH_ALIGN:
        case D_STATE_COMPLETE:
            switch(fellowState){
                case D_STATE_INIT:
                case D_STATE_DISCOVER_FELLOWS_GROUPS:
                case D_STATE_COMPLETE: /* TODO: need to allow other states too? */
                    allowed = TRUE;
                    break;
                default:
                    allowed = FALSE;
                    break;
            }
            break;
        case D_STATE_DISCOVER_PERSISTENT_SOURCE:
        case D_STATE_TERMINATING:
        case D_STATE_TERMINATED:
            allowed = FALSE;
            break;
        default:
            assert(FALSE);
            allowed = FALSE;
            break;
    }

    return TRUE;
}

d_nameSpacesListener
d_nameSpacesListenerNew(
    d_subscriber subscriber)
{
    d_nameSpacesListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_nameSpacesListener(os_malloc(C_SIZEOF(d_nameSpacesListener)));
        d_listener(listener)->kind = D_NAMESPACES_LISTENER;
        d_nameSpacesListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_nameSpacesListenerInit(
    d_nameSpacesListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    os_threadAttrInit(&attr);

    d_readerListenerInit(   d_readerListener(listener),
                            d_nameSpacesListenerAction, subscriber,
                            D_NAMESPACES_TOPIC_NAME,
                            D_NAMESPACES_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            attr,
                            d_nameSpacesListenerDeinit);

}

void
d_nameSpacesListenerFree(
    d_nameSpacesListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_NAMESPACES_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_nameSpacesListenerDeinit(
    d_object object)
{
    OS_UNUSED_ARG(object);
    assert(d_listenerIsValid(d_listener(object), D_NAMESPACES_LISTENER));

    return;
}

c_bool
d_nameSpacesListenerStart(
    d_nameSpacesListener listener)
{
    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_nameSpacesListenerStop(
    d_nameSpacesListener listener)
{
    return d_readerListenerStop(d_readerListener(listener));
}

struct checkFellowMasterHelper
{
    d_admin admin;
    d_fellow fellow;
    d_nameSpace oldNameSpace;
};

static void
checkFellowMasterWalk(
    void* o,
    c_voidp userData)
{
    struct checkFellowMasterHelper* helper;
    d_networkAddress master, fellow;
    d_nameSpace nameSpace;

    helper = (struct checkFellowMasterHelper*)userData;
    nameSpace = d_nameSpace(o);                     /* the namespace of the fellow */
    master = d_nameSpaceGetMaster (nameSpace);      /* the master of the namespace of the fellow */
    fellow = d_fellowGetAddress(helper->fellow);    

    /* Only start checking for conflicts if the fellow is a master
     * for the namespace and the namespace is confirmed
     */
    if (!d_networkAddressCompare (fellow, master) && d_nameSpaceIsMasterConfirmed(nameSpace)) {
        d_adminReportMaster (helper->admin, helper->fellow, nameSpace, helper->oldNameSpace);
    }

    d_networkAddressFree(master);
    d_networkAddressFree(fellow);
}

struct checkDelayAlignmentHelper {
    d_admin admin;
    d_fellow fellow;
};

static void
checkFellowDelayAlignmentWalk(
    void* o,
    c_voidp userData)
{
    d_admin admin;
    d_nameSpace nameSpace;
    d_fellow fellow;
    d_quality q;
    struct checkDelayAlignmentHelper* data;

    data = (struct checkDelayAlignmentHelper*)userData;
    nameSpace = d_nameSpace(o);
    admin = data->admin;
    fellow = data->fellow;

    /* Get quality of fellow namespace */
    q = d_nameSpaceGetInitialQuality(nameSpace);

    /* Report potential delayed initial dataset if quality is non-zero and not infinite */
    if((q.seconds || q.nanoseconds) && ((q.seconds != C_TIME_INFINITE.seconds) && (q.nanoseconds != C_TIME_INFINITE.nanoseconds))) {
        d_adminReportDelayedInitialSet(admin, nameSpace, fellow);
    }
}

static c_bool
collectFellowNsWalk(
    d_nameSpace nameSpace,
    c_voidp userData)
{
    c_iter nsList = (c_iter)userData;
    c_iterAppend (nsList, d_nameSpaceCopy(nameSpace));
    return TRUE;
}

void
d_nameSpacesListenerAction(
    d_listener listener,
    d_message message)
{
    d_durability durability;
    d_admin admin;
    d_publisher publisher;
    d_fellow fellow;
    c_bool allowed;
    d_nameSpace nameSpace, localNameSpace, oldFellowNameSpace, fellowNameSpace;
    c_ulong count;
    d_configuration config;
    d_nameSpacesRequest nsRequest;
    d_networkAddress sender;
    d_subscriber subscriber;
    d_sampleChainListener sampleChainListener;
    struct compatibilityHelper helper;
    d_adminStatisticsInfo info;
    c_bool added;
    os_time srcTime , curTime, difTime, maxDifTime;
    struct checkFellowMasterHelper fellowMasterHelper;
    struct checkDelayAlignmentHelper delayAlignmentHelper;
    d_name role;
    c_iter fellowNameSpaces;
    d_nameSpace ns;

    assert(d_listenerIsValid(d_listener(listener), D_NAMESPACES_LISTENER));

    admin               = d_listenerGetAdmin(listener);
    publisher           = d_adminGetPublisher(admin);
    durability          = d_adminGetDurability(admin);
    config              = d_durabilityGetConfiguration(durability);
    fellowNameSpaces    = NULL;

    nameSpace = d_nameSpaceFromNameSpaces(d_nameSpaces(message));

    d_printTimedEvent         (durability, D_LEVEL_FINE,
                               D_THREAD_NAMESPACES_LISTENER,
                               "Received nameSpace '%s' from fellow %u (his master: %u, confirmed: %d, mergeState: %s, %d, quality: %d.%.9u).\n",
                               d_nameSpaces(message)->name,
                               message->senderAddress.systemId,
                               d_nameSpaces(message)->master.systemId,
                               d_nameSpaces(message)->masterConfirmed,
                               d_nameSpaces(message)->state.role,
                               d_nameSpaces(message)->state.value,
                               d_nameSpaces(message)->initialQuality.seconds,
                               d_nameSpaces(message)->initialQuality.nanoseconds);

    sender = d_networkAddressNew(message->senderAddress.systemId,
                               message->senderAddress.localId,
                               message->senderAddress.lifecycleId);

    fellow = d_adminGetFellow(admin, sender);

    if(!fellow){
        d_printTimedEvent (durability, D_LEVEL_FINE,
                           D_THREAD_NAMESPACES_LISTENER,
                           "Fellow %u unknown, administrating it.\n",
                           message->senderAddress.systemId);
        fellow = d_fellowNew(sender, message->senderState);
        d_fellowUpdateStatus(fellow, message->senderState, v_timeGet());
        added = d_adminAddFellow(admin, fellow);

        if(added == FALSE){
            d_fellowFree(fellow);
            fellow = d_adminGetFellow(admin, sender);
            assert(fellow);
        } else {
            fellow = d_adminGetFellow(admin, sender); /*Do this to allow fellowFree at the end*/
            nsRequest = d_nameSpacesRequestNew(admin);
            d_messageSetAddressee(d_message(nsRequest), sender);
            d_publisherNameSpacesRequestWrite(publisher, nsRequest, sender);
            d_nameSpacesRequestFree(nsRequest);
        }
    }
    d_fellowUpdateStatus(fellow, message->senderState, v_timeGet());

    if(d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED){

        fellowNameSpace = d_fellowGetNameSpace (fellow, nameSpace);

        /* Update master of fellow nameSpace */
        added = d_fellowAddNameSpace(fellow, nameSpace);

        if(fellowNameSpace){
            /* Get old namespace of fellow */
            oldFellowNameSpace = d_nameSpaceCopy (fellowNameSpace);
        } else {
            oldFellowNameSpace = d_nameSpaceCopy(nameSpace);
            d_fellowSetExpectedNameSpaces(fellow, d_nameSpaces(message)->total);
            subscriber = d_adminGetSubscriber(admin);
            sampleChainListener = d_subscriberGetSampleChainListener(subscriber);

            if(sampleChainListener){
                d_sampleChainListenerTryFulfillChains(sampleChainListener, NULL);
            }
        }

        /* Create namespace with local policy (if a match exists) */
        localNameSpace = d_nameSpaceNew (config, d_nameSpaceGetName(nameSpace));

        /* If namespace is created, add to administration */
        if (localNameSpace) {
            /* Copy partitions to local nameSpace */
            d_nameSpaceCopyPartitions (localNameSpace, nameSpace);
            d_adminAddNameSpace (admin, localNameSpace);
            d_nameSpaceFree (localNameSpace);
        }

        /* Check if fellow is a candidate for delayed alignment */
        delayAlignmentHelper.admin = admin;
        delayAlignmentHelper.fellow = fellow;
        checkFellowDelayAlignmentWalk(nameSpace, &delayAlignmentHelper);

        /* If fellow is master for a namespace, report it to admin */
        fellowMasterHelper.admin = admin;
        fellowMasterHelper.fellow = fellow;
        fellowMasterHelper.oldNameSpace = oldFellowNameSpace;
        checkFellowMasterWalk (nameSpace, &fellowMasterHelper);

        /* If the namespace was not added to the fellow (because it already existed there), free it */
        if(!added){
            d_nameSpaceFree(nameSpace);
        }

        d_nameSpaceFree (oldFellowNameSpace);

    } else {
        info = d_adminStatisticsInfoNew();
        d_fellowSetExpectedNameSpaces(fellow, d_nameSpaces(message)->total);
        d_fellowAddNameSpace(fellow, nameSpace);
        count = d_fellowNameSpaceCount(fellow);

        if(count == d_nameSpaces(message)->total){
            allowed = isFellowStateCompatible(durability, fellow);

            if(allowed == TRUE){
                config = d_durabilityGetConfiguration(durability);
                helper.fellow = fellow;
                helper.compatible = TRUE;

                d_adminNameSpaceWalk (admin, areFellowNameSpacesCompatible, &helper);

                if(helper.compatible == TRUE){

                    if(config->timeAlignment == TRUE){
                        curTime.tv_sec     = d_readerListener(listener)->lastInsertTime.seconds;
                        curTime.tv_nsec    = d_readerListener(listener)->lastInsertTime.nanoseconds;
                        srcTime.tv_sec     = d_readerListener(listener)->lastSourceTime.seconds;
                        srcTime.tv_nsec    = d_readerListener(listener)->lastSourceTime.nanoseconds;
                        maxDifTime.tv_sec  = 1; /*1s*/
                        maxDifTime.tv_nsec = 0;
                        difTime            = os_timeAbs(os_timeSub(curTime, srcTime));

                        if(os_timeCompare(difTime, maxDifTime) == OS_MORE){
                            d_printTimedEvent (durability, D_LEVEL_WARNING,
                               D_THREAD_NAMESPACES_LISTENER,
                               "Estimated time difference including latency with " \
                               "fellow %u is %f seconds, which is larger then " \
                               "expected.\n",
                               message->senderAddress.systemId,
                               os_timeToReal(difTime));
                            OS_REPORT_2(OS_WARNING, D_CONTEXT, 0,
                                "Estimated time difference including latency " \
                                "with fellow '%u' is larger then expected " \
                                "(%f seconds). Durability alignment might not be " \
                                "reliable. Please align time between these nodes " \
                                "and restart.",
                                message->senderAddress.systemId,
                                os_timeToReal(difTime));
                        } else {
                            d_printTimedEvent (durability, D_LEVEL_FINER,
                               D_THREAD_NAMESPACES_LISTENER,
                               "Estimated time difference including latency with " \
                               "fellow %u is %f seconds.\n",
                               message->senderAddress.systemId,
                               os_timeToReal(difTime));
                        }
                    }

                    /* Set role of fellow (take native role from namespace) */
                    role = d_nameSpaceGetRole(nameSpace);
                    d_fellowSetRole (fellow, role);
                    os_free (role);

                    d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_APPROVED);
                    info->fellowsApprovedDif += 1;
                    subscriber = d_adminGetSubscriber(admin);
                    sampleChainListener = d_subscriberGetSampleChainListener(subscriber);

                    if(sampleChainListener){
                        d_sampleChainListenerTryFulfillChains(sampleChainListener, NULL);
                    }

                    /* Check if the fellow is master for one or more namespaces and report this to admin */
                    fellowNameSpaces = c_iterNew(NULL);

                    /* Collect fellow namespaces */
                    d_fellowNameSpaceWalk (fellow, collectFellowNsWalk, fellowNameSpaces);

                    /* Check if fellow is a candidate for delayed alignment */
                    delayAlignmentHelper.admin = admin;
                    delayAlignmentHelper.fellow = fellow;
                    c_iterWalk(fellowNameSpaces, checkFellowDelayAlignmentWalk, &delayAlignmentHelper);

                    fellowMasterHelper.admin = admin;
                    fellowMasterHelper.fellow = fellow;
                    fellowMasterHelper.oldNameSpace = NULL;
                    c_iterWalk (fellowNameSpaces, checkFellowMasterWalk, &fellowMasterHelper);

                    while ((ns = c_iterTakeFirst(fellowNameSpaces))) {
                        d_nameSpaceFree(ns);
                    }
                    c_iterFree(fellowNameSpaces);

                    d_printTimedEvent (durability, D_LEVEL_WARNING,
                                       D_THREAD_NAMESPACES_LISTENER,
                                       "Received %u of %u nameSpaces from fellow %u.\n",
                                       count, d_nameSpaces(message)->total,
                                       message->senderAddress.systemId);
                } else {
                    info->fellowsIncompatibleDataModelDif += 1;

                    d_printTimedEvent (durability, D_LEVEL_WARNING,
                                   D_THREAD_NAMESPACES_LISTENER,
                                   "Communication with fellow %u NOT approved, because data model is not compatible\n",
                                   message->senderAddress.systemId);
                    d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL);
                }
            } else {
                info->fellowsIncompatibleStateDif += 1;
                d_printTimedEvent (durability, D_LEVEL_WARNING,
                                   D_THREAD_NAMESPACES_LISTENER,
                                   "Communication with fellow %u NOT approved, because state is not compatible my state: %d, fellow state: %d\n",
                                   message->senderAddress.systemId,
                                   d_durabilityGetState(durability),
                                   message->senderState);
                d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_INCOMPATIBLE_STATE);
            }
        } else {
            d_printTimedEvent (durability, D_LEVEL_WARNING,
                               D_THREAD_NAMESPACES_LISTENER,
                               "Received %u of %u nameSpaces from fellow %u.\n",
                               count, d_nameSpaces(message)->total,
                               message->senderAddress.systemId);
        }
        d_adminUpdateStatistics(admin, info);
        d_adminStatisticsInfoFree(info);
    }
    d_fellowFree(fellow);
    d_networkAddressFree(sender);

    return;
}

