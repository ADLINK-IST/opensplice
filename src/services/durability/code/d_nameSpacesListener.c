/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
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
    c_iter nameSpaces;
    c_bool compatible;
};

static c_bool
areFellowNameSpacesCompatible(
    d_nameSpace fellowNS,
    c_voidp args)
{
    struct compatibilityHelper* helper;
    d_nameSpace nameSpace;
    c_ulong i, length;
    int result;

    helper    = (struct compatibilityHelper*)args;
    length    = c_iterLength(helper->nameSpaces);
    nameSpace = NULL;

    helper->compatible = FALSE;

    for(i=0; i<length && !nameSpace; i++){
        nameSpace = c_iterObject(helper->nameSpaces, i);
        result = d_nameSpaceCompatibilityCompare(nameSpace, fellowNS);

        if(result == 0){
            helper->compatible = TRUE;
            c_iterTake(helper->nameSpaces, nameSpace);
        } else {
            nameSpace = NULL;
        }
    }

    return TRUE;
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
    d_nameSpace nameSpace;
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

    assert(d_listenerIsValid(d_listener(listener), D_NAMESPACES_LISTENER));

    admin      = d_listenerGetAdmin(listener);
    publisher  = d_adminGetPublisher(admin);
    durability = d_adminGetDurability(admin);

    d_printTimedEvent         (durability, D_LEVEL_FINE,
                               D_THREAD_NAMESPACES_LISTENER,
                               "Received nameSpaces from fellow %d.\n",
                               message->senderAddress.systemId);

    sender = d_networkAddressNew(message->senderAddress.systemId,
                               message->senderAddress.localId,
                               message->senderAddress.lifecycleId);

    fellow = d_adminGetFellow(admin, sender);

    if(!fellow){
        d_printTimedEvent (durability, D_LEVEL_FINE,
                           D_THREAD_NAMESPACES_LISTENER,
                           "Fellow %d unknown, administrating it.\n",
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
        /*Update master of fellow nameSpace...*/
        nameSpace = d_nameSpaceFromNameSpaces(d_nameSpaces(message));
        added = d_fellowAddNameSpace(fellow, nameSpace);

        if(!added){
            d_nameSpaceFree(nameSpace);
        }
        d_printTimedEvent (durability, D_LEVEL_FINE,
                           D_THREAD_NAMESPACES_LISTENER,
                           "Fellow %d already approved.\n",
                           message->senderAddress.systemId);
    } else {
        info = d_adminStatisticsInfoNew();
        nameSpace = d_nameSpaceFromNameSpaces(d_nameSpaces(message));
        d_fellowSetExpectedNameSpaces(fellow, d_nameSpaces(message)->total);
        d_fellowAddNameSpace(fellow, nameSpace);
        count = d_fellowNameSpaceCount(fellow);

        if(count == d_nameSpaces(message)->total){
            allowed = isFellowStateCompatible(durability, fellow);

            if(allowed == TRUE){
                config = d_durabilityGetConfiguration(durability);
                helper.nameSpaces = c_iterCopy(config->nameSpaces);
                helper.compatible = FALSE;
                d_fellowNameSpaceWalk(fellow, areFellowNameSpacesCompatible, &helper);
                c_iterFree(helper.nameSpaces);

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
                               "fellow %d is %f seconds, which is larger then " \
                               "expected.\n",
                               message->senderAddress.systemId,
                               os_timeToReal(difTime));
                            OS_REPORT_2(OS_WARNING, D_CONTEXT, 0,
                                "Estimated time difference including latency " \
                                "with fellow '%d' is larger then expected " \
                                "(%f seconds). Durability alignment might not be " \
                                "reliable. Please align time between these nodes " \
                                "and restart.",
                                message->senderAddress.systemId,
                                os_timeToReal(difTime));
                        } else {
                            d_printTimedEvent (durability, D_LEVEL_FINER,
                               D_THREAD_NAMESPACES_LISTENER,
                               "Estimated time difference including latency with " \
                               "fellow %d is %f seconds.\n",
                               message->senderAddress.systemId,
                               os_timeToReal(difTime));
                        }
                    }
                    d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_APPROVED);
                    info->fellowsApprovedDif += 1;
                    subscriber = d_adminGetSubscriber(admin);
                    sampleChainListener = d_subscriberGetSampleChainListener(subscriber);

                    if(sampleChainListener){
                        d_sampleChainListenerTryFulfillChains(sampleChainListener, NULL);
                    }
                } else {
                    info->fellowsIncompatibleDataModelDif += 1;

                    d_printTimedEvent (durability, D_LEVEL_WARNING,
                                   D_THREAD_NAMESPACES_LISTENER,
                                   "Communication with fellow %d NOT approved, because data model is not compatible\n",
                                   message->senderAddress.systemId);
                    d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL);
                }
            } else {
                info->fellowsIncompatibleStateDif += 1;
                d_printTimedEvent (durability, D_LEVEL_WARNING,
                                   D_THREAD_NAMESPACES_LISTENER,
                                   "Communication with fellow %d NOT approved, because state is not compatible my state: %d, fellow state: %d\n",
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

