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
#include "os.h"
#include "d_groupLocalListener.h"
#include "d_nameSpacesRequestListener.h"
#include "d_nameSpacesRequest.h"
#include "d__durability.h"
#include "d__groupLocalListener.h"
#include "d__sampleChainListener.h"
#include "d__group.h"
#include "d_listener.h"
#include "d_admin.h"
#include "d_eventListener.h"
#include "d_configuration.h"
#include "d_durability.h"
#include "d_group.h"
#include "d_nameSpace.h"
#include "d_newGroup.h"
#include "d_deleteData.h"
#include "d_message.h"
#include "d_sampleRequest.h"
#include "d_groupsRequest.h"
#include "d_deleteData.h"
#include "d_sampleChainListener.h"
#include "d_actionQueue.h"
#include "d_fellow.h"
#include "d_networkAddress.h"
#include "d_publisher.h"
#include "d_table.h"
#include "d_misc.h"
#include "d_waitset.h"
#include "d_store.h"
#include "d_readerRequest.h"
#include "u_group.h"
#include "u_waitsetEvent.h"
#include "v_event.h"
#include "v_service.h"
#include "v_participant.h"
#include "v_entity.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_group.h"
#include "v_time.h"
#include "c_time.h"
#include "c_iterator.h"
#include "os_heap.h"
#include "os_report.h"
#include "d_durability.h"
#include "d__mergeAction.h"
#include "d__mergeState.h"

struct createPersistentSnapshotHelper {
    c_char* partExpr;
    c_char* topicExpr;
    c_char* uri;
    d_listener listener;
};

/**
 * TODO: d_groupLocalListenerHandleAlignment: take appropriate action on
 * lazy alignment where injection of data fails.
 */


struct checkNameSpacesHelper {
    d_nameSpacesRequest request;
    d_publisher publisher;
    c_iter retryFellows;
    os_time retryTime;
    os_time retryDelay;
};

static c_bool
checkNameSpaces(
    d_fellow fellow,
    c_voidp args)
{
    struct checkNameSpacesHelper *helper;
    d_communicationState state;
    d_networkAddress fellowAddress;

    helper = (struct checkNameSpacesHelper*)args;
    state = d_fellowGetCommunicationState (fellow);

    /* If an incomplete fellow is found, send a namespaces request to that fellow, and reset the retryTime.
     * The fellow is added to the retryFellows list and removed from the administration (by the caller function),
     * when retryTime eventually exceeds and the fellow is still in the list.
     * */
    if (state == D_COMMUNICATION_STATE_APPROVED) {
        d_fellow retryFellow;
        retryFellow = c_iterTake(helper->retryFellows, fellow);
        if(retryFellow) {
            d_objectFree(d_object(retryFellow), D_FELLOW);
        }
    } else {
        if (!c_iterContains(helper->retryFellows, fellow)) {
            fellowAddress = d_fellowGetAddress(fellow);
            d_messageSetAddressee(d_message(helper->request), fellowAddress);
            d_publisherNameSpacesRequestWrite(helper->publisher, helper->request, fellowAddress);
            /* Increase retrytime, in case this is a new fellow give it a fair chance to respond */
            helper->retryTime = os_timeAdd(os_timeGet(), helper->retryDelay);
            d_objectKeep((d_object)fellow);
            helper->retryFellows = c_iterAppend(helper->retryFellows, fellow);
            d_networkAddressFree(fellowAddress);
        }
    }

    return TRUE;
}

static void
checkNameSpacesRemoveFellowAction(
   void* o, void* userData)
{
    d_adminRemoveFellow((d_admin)userData, (d_fellow)o);
    d_objectFree((d_object)o, D_FELLOW);
}

d_groupLocalListener
d_groupLocalListenerNew(
    d_subscriber subscriber,
    d_sampleChainListener sampleChainListener)
{
    d_groupLocalListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_groupLocalListener(os_malloc(C_SIZEOF(d_groupLocalListener)));
        d_listener(listener)->kind = D_GROUP_LOCAL_LISTENER;
        listener->sampleChainListener = sampleChainListener;
        d_groupLocalListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_groupLocalListenerDeinit(
    d_object object)
{
    d_groupLocalListener listener;

    assert(d_listenerIsValid(d_listener(object), D_GROUP_LOCAL_LISTENER));

    if(object){
        listener = d_groupLocalListener(object);
        d_groupLocalListenerStop(listener);
        d_actionQueueFree(listener->actionQueue);
        d_actionQueueFree(listener->masterMonitor);

        os_mutexDestroy(&(listener->masterLock));
    }
}

void
d_groupLocalListenerFree(
    d_groupLocalListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_GROUP_LOCAL_LISTENER));

    d_listenerFree(d_listener(listener));
}

/*************** START NEW IMPL *************************/

static c_bool
checkFellowGroupsKnown(
    d_fellow fellow,
    c_voidp args)
{
    c_long expected;
    c_ulong actual;
    c_bool* known;
    c_bool requested;

    known = (c_bool*)args;

    requested = d_fellowGetGroupsRequested(fellow);

    if(requested){
        expected = d_fellowGetExpectedGroupCount(fellow);

        if(expected != -1){
            actual = d_fellowGetGroupCount(fellow);

            if(actual >= ((c_ulong)expected)){
                *known = TRUE;
            } else {
                *known = FALSE;
            }
        } else if( (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_STATE) ||
                   (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL))
        {
            *known = TRUE;
        } else {
            *known = FALSE;
        }
    } else {
        *known = TRUE;
    }
    return *known;

}

struct groupInfo {
    c_iter nameSpaces;
    c_bool iAmAMaster;
    d_groupsRequest request;
    d_publisher publisher;
    d_durability durability;
};


static c_bool
requestGroups(
    d_fellow fellow,
    c_voidp args)
{
    c_bool toRequest;
    d_networkAddress addr, master;
    struct groupInfo *info;
    c_long i;
    d_nameSpace ns;
    c_bool notInitial;
    d_admin admin;
    d_subscriber subscriber;
    d_sampleChainListener sampleChainListener;

    info = (struct groupInfo*)args;

    if(info->iAmAMaster){
        toRequest = d_fellowSetGroupsRequested(fellow);
        addr = d_fellowGetAddress(fellow);

        if(toRequest){
            d_printTimedEvent(info->durability, D_LEVEL_FINE,
                              D_THREAD_GROUP_LOCAL_LISTENER,
                              "Requesting all groups from fellow '%u'.\n",
                              addr->systemId);
            d_messageSetAddressee(d_message(info->request), addr);
            d_publisherGroupsRequestWrite(info->publisher, info->request, addr);
        } else {
            d_printTimedEvent(info->durability, D_LEVEL_FINEST,
                              D_THREAD_GROUP_LOCAL_LISTENER,
                              "No need to request all groups from fellow '%u' again\n",
                              addr->systemId);
        }
        d_networkAddressFree(addr);
    } else {
        addr = d_fellowGetAddress(fellow);
        toRequest = FALSE;

        for(i=0; i<c_iterLength(info->nameSpaces) && (!toRequest); i++){
            ns = d_nameSpace(c_iterObject(info->nameSpaces, i));
            master = d_nameSpaceGetMaster(ns);

            if(d_networkAddressEquals(master, addr)){
                notInitial = d_nameSpaceIsAlignmentNotInitial(ns);

                if(notInitial){
                    d_printTimedEvent(info->durability, D_LEVEL_FINER,
                          D_THREAD_GROUP_LOCAL_LISTENER,
                          "I am very lazy and will not request groups from master '%u'.\n",
                          addr->systemId);

                    /* Request groups that are still being aligned */
                    admin = info->durability->admin;
                    subscriber = d_adminGetSubscriber(admin);
                    sampleChainListener = d_subscriberGetSampleChainListener(subscriber);
                    d_sampleChainListenerCheckUnfulfilled(sampleChainListener, ns, master);

                    toRequest = FALSE;
                } else {
                    toRequest = TRUE;
                }
            }
            d_networkAddressFree(master);
        }
        if(toRequest){
            toRequest = d_fellowSetGroupsRequested(fellow);

            if(toRequest){
                d_printTimedEvent(info->durability, D_LEVEL_FINE,
                                  D_THREAD_GROUP_LOCAL_LISTENER,
                                  "Requesting all groups from fellow '%u'.\n",
                                  addr->systemId);
                d_messageSetAddressee(d_message(info->request), addr);
                d_publisherGroupsRequestWrite(info->publisher, info->request, addr);
            } else {
                d_printTimedEvent(info->durability, D_LEVEL_FINEST,
                                  D_THREAD_GROUP_LOCAL_LISTENER,
                                  "No need to request all groups from fellow '%u' again\n",
                                  addr->systemId);
            }
        } else {
            d_printTimedEvent(info->durability, D_LEVEL_FINEST,
                              D_THREAD_GROUP_LOCAL_LISTENER,
                              "No need to request groups from fellow '%u'.\n",
                              addr->systemId);
        }
        d_networkAddressFree(addr);
    }
    return TRUE;
}


static void
doGroupsRequest(
    d_groupLocalListener listener,
    c_iter nameSpaces,
    c_bool iAmAMaster)
{
    d_admin admin;
    struct groupInfo info;

    if(listener){
        admin           = d_listenerGetAdmin(d_listener(listener));

        info.nameSpaces = nameSpaces;
        info.durability = d_adminGetDurability(admin);
        info.iAmAMaster = iAmAMaster;
        info.publisher  = d_adminGetPublisher(admin);
        info.request    = d_groupsRequestNew(admin, NULL, NULL);

        d_adminFellowWalk(admin, requestGroups, &info);

        d_groupsRequestFree(info.request);
    }
    return;
}

/*
struct nsMaster {
    d_nameSpace myNameSpace;
    d_networkAddress masterAddress;
    c_bool conflictsFound;
};

static c_bool
findExistingMaster(
    d_fellow fellow,
    c_voidp userData)
{
    struct nsMaster* nsm;
    d_nameSpace fellowNameSpace;
    d_networkAddress fellowMasterAddress;

    nsm = (struct nsMaster*)userData;
    fellowNameSpace = d_fellowGetNameSpace(fellow, nsm->myNameSpace);

    if(fellowNameSpace){
        fellowMasterAddress = d_nameSpaceGetMaster(fellowNameSpace);


        if(!(d_networkAddressIsUnaddressed(nsm->masterAddress))){
            if( (!d_networkAddressIsUnaddressed(fellowMasterAddress)) &&
                (!d_networkAddressEquals(nsm->masterAddress, fellowMasterAddress)))
            {
                nsm->conflictsFound = TRUE;
            }
            d_networkAddressFree(fellowMasterAddress);
        } else if(!d_networkAddressIsUnaddressed(fellowMasterAddress)){
            nsm->masterAddress = fellowMasterAddress;
        } else {
            //Fellow hasn't determined a master yet
            d_networkAddressFree(fellowMasterAddress);
        }
    } else {
        // This might happen when the fellow has just been added and his
        // namespaces are not complete yet.
        nsm->conflictsFound = TRUE;
    }
    return (!(nsm->conflictsFound));
}

struct nsNewMaster{
    d_nameSpace myNameSpace;
    d_networkAddress bestFellow;
    d_quality fellowQuality;
    d_serviceState bestFellowState;
};

static c_bool
findNewMaster(
    d_fellow fellow,
    c_voidp userData)
{
    struct nsNewMaster* nsm;
    d_nameSpace fellowNameSpace;
    d_networkAddress fellowAddress;
    c_bool isAligner, replace;
    d_serviceState state;
    d_quality quality;

    nsm = (struct nsNewMaster*)userData;
    replace = FALSE;
    fellowNameSpace = d_fellowGetNameSpace(fellow, nsm->myNameSpace);

    if(fellowNameSpace){
        state = d_fellowGetState(fellow);
        isAligner = d_nameSpaceIsAligner(fellowNameSpace);
        quality = d_nameSpaceGetInitialQuality(fellowNameSpace);

        if(isAligner){
            if(!(nsm->bestFellow)){
                replace = TRUE;
            } else if((state > nsm->bestFellowState) &&
                      (state != D_STATE_TERMINATING) &&
                      (state != D_STATE_TERMINATED))
            {
                replace = TRUE;
            } else if(quality.seconds > nsm->fellowQuality.seconds){
                replace = TRUE;
            } else if(quality.seconds == nsm->fellowQuality.seconds){
                if(quality.nanoseconds > nsm->fellowQuality.nanoseconds){
                    replace = TRUE;
                } else if(quality.nanoseconds == nsm->fellowQuality.nanoseconds){
                    fellowAddress = d_fellowGetAddress(fellow);

                    if(d_networkAddressCompare(fellowAddress, nsm->bestFellow) > 0){
                        replace = TRUE;
                    }
                    d_networkAddressFree(fellowAddress);
                }
            }
        }
    }

    if(replace){
        if(nsm->bestFellow){
            d_networkAddressFree(nsm->bestFellow);
        }
        nsm->bestFellow                = d_fellowGetAddress(fellow);
        nsm->fellowQuality.seconds     = quality.seconds;
        nsm->fellowQuality.nanoseconds = quality.nanoseconds;
        nsm->bestFellowState           = state;
    }
    return TRUE;
}


static d_networkAddress
findMaster(
    d_groupLocalListener listener,
    d_nameSpace nameSpace)
{
    d_admin admin;
    d_durability durability;
    d_quality myQuality;
    struct nsMaster nsm;
    struct nsNewMaster nsnm;
    d_networkAddress selectedMaster;

    admin       = d_listenerGetAdmin(d_listener(listener));
    durability  = d_adminGetDurability(admin);

    nsm.myNameSpace    = nameSpace;
    nsm.masterAddress  = d_nameSpaceGetMaster(nameSpace);
    nsm.conflictsFound = FALSE;

    d_adminFellowWalk(admin, findExistingMaster, &nsm);

    if((!d_networkAddressIsUnaddressed(nsm.masterAddress)) && (!nsm.conflictsFound)){
        //Existing master found and found no conflicts
        selectedMaster = nsm.masterAddress;
        d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Found existing master '%d' for nameSpace '%s'.\n",
                        selectedMaster->systemId, d_nameSpaceGetName(nameSpace));

    } else {
        if(nsm.conflictsFound){
            d_printTimedEvent(durability, D_LEVEL_INFO,
                           D_THREAD_GROUP_LOCAL_LISTENER,
                           "Found conflicting masters for nameSpace '%s', " \
                           "determining the master for myself now.\n",
                           d_nameSpaceGetName(nameSpace));
        }
        //Determine master for myself now
        d_networkAddressFree(nsm.masterAddress);
        nsnm.myNameSpace = nameSpace;

        if(d_nameSpaceIsAligner(nameSpace)){
            myQuality                      = d_nameSpaceGetInitialQuality(nameSpace);
            nsnm.bestFellow                = d_adminGetMyAddress(admin);
            nsnm.fellowQuality.seconds     = myQuality.seconds;
            nsnm.fellowQuality.nanoseconds = myQuality.nanoseconds;
            nsnm.bestFellowState           = d_durabilityGetState(durability);
        } else {
            nsnm.bestFellow                = NULL;
            nsnm.fellowQuality.seconds     = 0;
            nsnm.fellowQuality.nanoseconds = 0;
            nsnm.bestFellowState           = D_STATE_INIT;
        }
        d_adminFellowWalk(admin, findNewMaster, &nsnm);

        if(nsnm.bestFellow){
            selectedMaster = nsnm.bestFellow;
            d_printTimedEvent(durability, D_LEVEL_INFO,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "Found new master '%d' for nameSpace '%s'.\n",
                selectedMaster->systemId, d_nameSpaceGetName(nameSpace));
        } else {
            selectedMaster = d_networkAddressUnaddressed();
            d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "No master found for nameSpace '%s'.\n",
                    selectedMaster->systemId, d_nameSpaceGetName(nameSpace));
        }
    }
    return selectedMaster;
}
*/
/*****************************************************************************/

struct addressList {
    d_networkAddress address;
    c_ulong count;
    c_voidp next;
};

struct masterInfo {
    d_nameSpace nameSpace;
    d_networkAddress master;
    d_quality masterQuality;
    c_bool conflicts;
    d_serviceState masterState;
    d_durability durability;
    c_bool conflictsAllowed;
    struct addressList* list;
};

static void
addMajorityMaster(
    struct masterInfo* info,
    d_networkAddress address)
{
    struct addressList *list, *prevList;
    c_bool found;

    assert(info);

    found = FALSE;

    if(info->list){
        list = info->list;

        while(list && !found){
            prevList = list;
            if(d_networkAddressEquals(address, list->address)){
                found = TRUE;
                list->count++;
            }
            list = list->next;
        }
        if(!found){
            prevList->next = (struct addressList*)(os_malloc(sizeof(struct addressList)));
            list = prevList->next;
        }
    } else {
        info->list = (struct addressList*)(os_malloc(sizeof(struct addressList)));
        list = info->list;
    }

    if(!found){
        list->address = d_networkAddressNew(address->systemId, address->localId, address->lifecycleId);
        list->count = 1;
        list->next = NULL;
    }
    return;
}

static void
removeMajorityMaster(
    struct masterInfo* info,
    d_networkAddress address)
{
    struct addressList *list, *prev;
    c_bool found;

    assert(info);

    prev = NULL;
    list = info->list;
    found = FALSE;

    while(list && !found){
        if(d_networkAddressEquals(list->address, address)){
            if(prev){
                prev->next = list->next;
            } else {
                info->list = list->next;
            }
            d_networkAddressFree(list->address);
            os_free(list);
            found = TRUE;
        }

        if (!found) {
            prev = list;
            list = list->next;
        }
    }
    return;
}

static d_networkAddress
getMajorityMaster(
    struct masterInfo* info)
{
    d_networkAddress master;
    c_ulong count;
    c_bool replace;
    struct addressList* list;

    assert(info);

    if(info->list){
        master = d_networkAddressNew(info->list->address->systemId,
                info->list->address->localId, info->list->address->lifecycleId);
        count = info->list->count;

        list = (struct addressList*)(info->list->next);

        while(list){
            replace = FALSE;

            if(list->count > count){
                replace = TRUE;
            } else if (list->count == count){
                if(d_networkAddressCompare(list->address, master) > 0){
                    replace = TRUE;
                }
            }

            if(replace){
                d_networkAddressFree(master);
                master = d_networkAddressNew(list->address->systemId,
                        list->address->localId, list->address->lifecycleId);
                count = list->count;
            }
            list = (struct addressList*)list->next;
        }
    } else {
        master = d_networkAddressUnaddressed();
    }
    return master;
}

static void
freeMajorityMasters(
    struct masterInfo* info)
{
    struct addressList *list, *prevList;

    assert(info);

    list = info->list;

    while(list){
        d_networkAddressFree(list->address);
        prevList = list;
        list = list->next;
        os_free(prevList);
    }
    return;
}

static c_bool
determineExistingMaster(
    d_fellow fellow,
    c_voidp userData)
{
    struct masterInfo* m;
    d_nameSpace fellowNameSpace;
    d_networkAddress fellowMaster, fellowAddress;
    c_bool result;
    char *role, *fellowRole;

    m = (struct masterInfo*)userData;
    fellowNameSpace = d_fellowGetNameSpace(fellow, m->nameSpace);
    role = d_nameSpaceGetRole(m->nameSpace);

        /*Check if fellow has compatible nameSpace. If not go to the next one */
    if(fellowNameSpace){
        fellowMaster = d_nameSpaceGetMaster(fellowNameSpace);
        fellowRole = d_nameSpaceGetRole(fellowNameSpace);

        if (strcmp(fellowRole, role) == 0) {
            /* If I haven't found a potential master so far, check whether the
             * fellow determined a (potential) master already. If so, conform
             * to the selected master.
             */
            if(d_networkAddressIsUnaddressed(m->master)){
                if(!d_networkAddressIsUnaddressed(fellowMaster)){
                    d_networkAddressFree(m->master);
                    m->master = d_networkAddressNew(
                            fellowMaster->systemId,
                            fellowMaster->localId,
                            fellowMaster->lifecycleId);
                    addMajorityMaster(m, fellowMaster);
                }
            } else if(!d_networkAddressIsUnaddressed(fellowMaster)){
                /* If the fellow has determined a (potential) master
                 * that doesn't match my current one, there's a conflict and
                 * I need to drop out of this function.
                 */
                if(!d_networkAddressEquals(m->master, fellowMaster)){
                    m->conflicts = TRUE;

                    fellowAddress = d_fellowGetAddress(fellow);
                    d_printTimedEvent(m->durability, D_LEVEL_INFO,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Fellow '%u' reports master '%u' for nameSpace '%s'. " \
                        "while I found master '%u'.\n",
                        fellowAddress->systemId,
                        fellowMaster->systemId,
                        d_nameSpaceGetName(m->nameSpace),
                        m->master->systemId);
                    d_networkAddressFree(fellowAddress);
                }
                addMajorityMaster(m, fellowMaster);
            }
        }
        if(m->conflictsAllowed){
            result = TRUE;
        } else {
            result = !(m->conflicts);
        }

        d_networkAddressFree(fellowMaster);
        os_free (fellowRole);
    /* If fellow namespace is from other role, it can't be master for this node */
    }else {
        result = TRUE;
    }

    os_free (role);

    return result;
}

static c_bool
determineNewMaster(
    d_fellow fellow,
    c_voidp userData)
{
    struct masterInfo* m;
    d_nameSpace fellowNameSpace;
    d_networkAddress fellowAddress;
    c_bool isAligner, replace;
    d_quality quality;
    d_serviceState fellowState;
    char *role, *fellowRole;

    m = (struct masterInfo*)userData;
    replace = FALSE;
    fellowNameSpace = d_fellowGetNameSpace(fellow, m->nameSpace);


    if(fellowNameSpace){

        role = d_nameSpaceGetRole (m->nameSpace);
        fellowRole = d_nameSpaceGetRole (fellowNameSpace);

        if (strcmp (role, fellowRole) == 0) {
            isAligner = d_nameSpaceIsAligner(fellowNameSpace);
            quality = d_nameSpaceGetInitialQuality(fellowNameSpace);
            fellowState = d_fellowGetState(fellow);

            if(isAligner){
                if(d_networkAddressIsUnaddressed(m->master)){
                    replace = TRUE;

                /* This behavior is undesired when delayedAlignment is active */
                } else if(!d_nameSpaceGetDelayedAlignment(m->nameSpace)) {
                    if((m->masterState <= D_STATE_DISCOVER_PERSISTENT_SOURCE) && (fellowState > D_STATE_DISCOVER_PERSISTENT_SOURCE)) {
                        replace = TRUE;
                    } else if((m->masterState > D_STATE_DISCOVER_PERSISTENT_SOURCE) && (fellowState <= D_STATE_DISCOVER_PERSISTENT_SOURCE)) {
                        replace = FALSE;
                    }

                } else if(quality.seconds > m->masterQuality.seconds){
                    replace = TRUE;
                } else if(quality.seconds == m->masterQuality.seconds){
                    if(quality.nanoseconds > m->masterQuality.nanoseconds){
                        replace = TRUE;
                    } else if(quality.nanoseconds == m->masterQuality.nanoseconds){
                        fellowAddress = d_fellowGetAddress(fellow);

                        if(d_networkAddressCompare(fellowAddress, m->master) > 0){
                            replace = TRUE;
                        }
                        d_networkAddressFree(fellowAddress);
                    }
                }
            }
        }
        os_free (role);
        os_free (fellowRole);
    }

    if(replace){
        if(m->master){
            d_networkAddressFree(m->master);
        }
        m->master = d_fellowGetAddress(fellow);
        m->masterQuality.seconds = quality.seconds;
        m->masterQuality.nanoseconds = quality.nanoseconds;
        m->masterState = fellowState;
    }
    return TRUE;
}

/*
 * PRECONDTION: listener->masterLock is locked
 *
 * @return Returns TRUE if I have become a master for one or more nameSpaces.
 */
static c_bool
d_groupLocalListenerDetermineMasters(
    d_groupLocalListener listener,
    c_iter nameSpaces)
{
    d_admin admin;
    d_durability durability;
    d_configuration configuration;
    c_long length, i;
    d_nameSpace nameSpace;
    d_subscriber subscriber;
    d_publisher publisher;
    d_nameSpacesRequestListener nsrListener;
    d_networkAddress unaddressed, myAddress, master, lastMaster;
    struct checkNameSpacesHelper checkNsHelper;
    struct masterInfo mastership;
    os_time sleepTime, endTime;
    c_bool conflicts, firstTime, cont, proceed;
    d_quality myQuality;
    c_bool iAmAMaster;
    d_serviceState fellowState;
    d_fellow fellow, retryFellow;
    c_ulong tries, maxTries;
    c_bool initialUnaddressed;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    configuration = d_durabilityGetConfiguration(durability);
    length = c_iterLength(nameSpaces);
    subscriber = d_adminGetSubscriber(admin);
    publisher = d_adminGetPublisher(admin);
    nsrListener = d_subscriberGetNameSpacesRequestListener(subscriber);
    unaddressed = d_networkAddressUnaddressed();
    myAddress = d_adminGetMyAddress(admin);
    firstTime = TRUE;
    iAmAMaster = FALSE;
    maxTries = 4;
    tries = 0;

    /*100ms*/
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 100000000;
    mastership.durability = durability;
    mastership.masterQuality.seconds = 0;
    mastership.masterQuality.nanoseconds = 0;

    checkNsHelper.request = d_nameSpacesRequestNew(admin);
    checkNsHelper.retryFellows = NULL;
    checkNsHelper.publisher = publisher;
    checkNsHelper.retryDelay = configuration->heartbeatExpiryTime;

    do {
        conflicts = FALSE;
        tries++;

        /*Check if namespaces still have the same master*/
        for(i=0; i<length && (d_durabilityMustTerminate(durability) == FALSE); i++){
            nameSpace = d_nameSpace(c_iterObject(nameSpaces, i));

            /* Remember the last master I selected. If the 'existing'
             * one I find now is different, I need to do this whole loop again.
             */
            if(firstTime){
                lastMaster = d_nameSpaceGetMaster(nameSpace);

                if (d_networkAddressIsUnaddressed(lastMaster)) {
                    initialUnaddressed = TRUE;
                }else {
                    d_networkAddressFree(lastMaster);
                    lastMaster = d_networkAddressUnaddressed();
                    initialUnaddressed = FALSE;
                }
            } else {
                lastMaster = d_nameSpaceGetMaster(nameSpace);
                if (d_networkAddressIsUnaddressed(lastMaster)) {
                    initialUnaddressed = TRUE;
                }else {
                    initialUnaddressed = FALSE;
                }
            }

            /*
             * Always use un-addressed as my master when looking for an
             * existing master.
             */
            d_nameSpaceSetMaster(nameSpace, unaddressed);
            d_nameSpaceMasterPending (nameSpace);

            mastership.nameSpace = nameSpace;
            mastership.master = d_nameSpaceGetMaster(nameSpace);
            mastership.conflicts = FALSE;
            mastership.list = NULL;

            if(tries >= maxTries){
                mastership.conflictsAllowed = TRUE;
            } else {
                mastership.conflictsAllowed = FALSE;
            }

            if (initialUnaddressed || !firstTime) {
                /*Walk over all fellows that are approved*/
                d_adminFellowWalk(admin, determineExistingMaster, &mastership);
            }

            if((!mastership.conflicts) && (!d_networkAddressIsUnaddressed(mastership.master))){
                /*Check whether the found fellow is still alive and kicking.*/

                /*Some node(s) could already have chosen me as master */
                if(!d_networkAddressEquals(mastership.master, myAddress)){
                    fellow = d_adminGetFellow(admin, mastership.master);

                    /*Fellow may be gone already or is currently terminating*/
                    if(!fellow){
                        d_networkAddressFree(mastership.master);
                        mastership.master = d_networkAddressUnaddressed();
                    } else {
                        fellowState = d_fellowGetState(fellow);

                        if((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)){
                            d_networkAddressFree(mastership.master);
                            mastership.master = d_networkAddressUnaddressed();
                        } else if(!d_networkAddressEquals(lastMaster, mastership.master)){
                            /* Check whether I determined a master already
                             * in the previous loop.
                             */
                            if(!d_networkAddressIsUnaddressed(lastMaster)) {
                                /* Existing master is not the same as the one I found before
                                 * so make sure I'll check once more.
                                 */
                                conflicts = TRUE;
                                d_printTimedEvent(durability, D_LEVEL_INFO,
                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                    "The existing master '%u' I found now for nameSpace '%s'. " \
                                    "doesn't match the one '%u' I found before. " \
                                    "Waiting for confirmation...",
                                    mastership.master->systemId,
                                    d_nameSpaceGetName(nameSpace),
                                    lastMaster->systemId);
                            }
                        }
                        d_fellowFree(fellow);
                    }
                }
            } else if(mastership.conflictsAllowed){
                cont = TRUE;
                mastership.conflicts = FALSE;

                while(cont){
                    if(mastership.master){
                        d_networkAddressFree(mastership.master);
                    }
                    mastership.master = getMajorityMaster(&mastership);

                    if(!d_networkAddressIsUnaddressed(mastership.master)){
                        fellow = d_adminGetFellow(admin, mastership.master);

                        /*Fellow may be gone already or is currently terminating*/
                        if(!fellow){
                            removeMajorityMaster(&mastership, mastership.master);
                        } else {
                            fellowState = d_fellowGetState(fellow);

                            if((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)){
                                removeMajorityMaster(&mastership, mastership.master);
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_INFO,
                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                    "Found majority master '%d' for nameSpace '%s'.\n",
                                    mastership.master->systemId,
                                    d_nameSpaceGetName(nameSpace));
                                cont = FALSE;
                            }
                            d_fellowFree(fellow);
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_INFO,
                            D_THREAD_GROUP_LOCAL_LISTENER,
                            "Tried to find majority master for nameSpace '%s', but found none.\n",
                            d_nameSpaceGetName(nameSpace));
                        cont = FALSE;
                    }
                }
            }
            freeMajorityMasters(&mastership);

            if((mastership.conflicts) || d_networkAddressIsUnaddressed(mastership.master)){
                if(mastership.conflicts){
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Found conflicting masters for nameSpace '%s'. " \
                        "Determining new master now...\n",
                        d_nameSpaceGetName(nameSpace));
                    conflicts = TRUE;
                    mastership.conflicts = FALSE;
                } else {
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Found no existing master for nameSpace '%s'. " \
                        "Determining new master now...\n",
                        d_nameSpaceGetName(nameSpace));
                }
                d_networkAddressFree(mastership.master);

                if(d_nameSpaceIsAligner(nameSpace)){
                    myQuality = d_nameSpaceGetInitialQuality(nameSpace);
                    mastership.master = d_adminGetMyAddress(admin);
                    mastership.masterQuality.seconds = myQuality.seconds;
                    mastership.masterQuality.nanoseconds = myQuality.nanoseconds;
                    mastership.masterState = d_durabilityGetState(durability);
                } else {
                    mastership.master = d_networkAddressUnaddressed();
                    mastership.masterQuality.seconds = 0;
                    mastership.masterQuality.nanoseconds = 0;
                    mastership.masterState = D_STATE_INIT;
                }
                /*7. Walk over all fellows that are approved again.*/
                d_adminFellowWalk(admin, determineNewMaster, &mastership);

                if(d_networkAddressIsUnaddressed(mastership.master)){
                    /* Depending on the configuration setting TimeToWaitForAligner
                     * the service should keep waiting until an aligner becomes
                     * available. If no aligner is available within the specified time
                     * the system should move to state INCOMPATIBLE_CONFIGURATION and
                     * gracefully exit.
                     * The INCOMPATIBLE_CONFIGURATION is then picked up by the splice
                     * daemon that can execute the configured FailureAction.
                     * Note: at the moment only 0 is supported, any other value
                     * will result in waiting indefinitely for an aligner.
                     */
                    if ((configuration->timeToWaitForAligner.tv_sec != 0) ||
                        (configuration->timeToWaitForAligner.tv_nsec != 0)) {
                        /* For the moment it is assumed that in case timeToWaitForAligner != 0
                         * then the service will wait indefinitely until an aligner becomes available.
                         */
                        d_printTimedEvent(durability, D_LEVEL_INFO,
                           D_THREAD_GROUP_LOCAL_LISTENER,
                           "There's no new master available for nameSpace '%s'. " \
                           "Awaiting availability of a new master...\n",
                           d_nameSpaceGetName(nameSpace));
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_INFO,
                           D_THREAD_GROUP_LOCAL_LISTENER,
                           "There's no new master available for nameSpace '%s'. " \
                           "Incompatible configuration.\n",
                           d_nameSpaceGetName(nameSpace));
                        /* Move the state of the durability service to
                         * STATE_INCOMPATIBLE_CONFIGURATION and terminate
                         * gracefully.
                         */
                        d_durabilityTerminate(durability, FALSE);
                    }
                } else if(d_networkAddressEquals(mastership.master, myAddress)){
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                       D_THREAD_GROUP_LOCAL_LISTENER,
                       "I want to be the new master for nameSpace '%s'. " \
                       "Waiting for confirmation...\n",
                       d_nameSpaceGetName(nameSpace));
                } else {
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "I want fellow '%d' to be the new master for nameSpace '%s'. " \
                        "Waiting for confirmation...\n",
                        mastership.master->systemId,
                        d_nameSpaceGetName(nameSpace));
                }
            } else {
                d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "Found existing master '%u' for nameSpace '%s'. " \
                    "Waiting for confirmation...\n",
                    mastership.master->systemId,
                    d_nameSpaceGetName(nameSpace));
            }
            d_nameSpaceSetMaster(nameSpace, mastership.master);
            d_networkAddressFree(mastership.master);
            d_networkAddressFree(lastMaster);
        }

        d_nameSpacesRequestListenerReportNameSpaces(nsrListener);

        /* Do the same thing again if there were conflicts.
         * Since this step must be taken at least once (even if there are
         * no conflicts), 'firstTime' is checked also.
         */
        if(conflicts || firstTime){
            if(firstTime){
                firstTime = FALSE;
                /*To make sure the do-while is done at least once more: */
                conflicts = TRUE;
            }

            /*Wait twice the heartbeat period*/
            endTime = os_timeGet();
            endTime = os_timeAdd(endTime, checkNsHelper.retryDelay);

            d_printTimedEvent(durability, D_LEVEL_INFO,
                       D_THREAD_GROUP_LOCAL_LISTENER,
                       "Waiting the heartbeat expiry period: %f seconds.\n",
                       os_timeToReal(configuration->heartbeatExpiryTime));

            proceed = FALSE;

            assert(!checkNsHelper.retryFellows);

            while((d_durabilityMustTerminate(durability) == FALSE) && (proceed == FALSE)) {
                os_nanoSleep(sleepTime);

                if(os_timeCompare(os_timeGet(), endTime) > 0) {
                    d_adminFellowWalk(admin, checkNameSpaces, &checkNsHelper);

                    if (c_iterLength(checkNsHelper.retryFellows) > 0) {
                        d_printTimedEvent(durability, D_LEVEL_INFO, D_THREAD_GROUP_LOCAL_LISTENER,
                            "Found %d incomplete fellow(s)\n", c_iterLength(checkNsHelper.retryFellows));
                        proceed = FALSE;
                        if(os_timeCompare(os_timeGet(), checkNsHelper.retryTime) > 0) {
                            /* There hasn't been a new incomplete fellow for the last 2*heartbeat period, remove any remaining incomplete fellows */
                            c_iterWalk(checkNsHelper.retryFellows, checkNameSpacesRemoveFellowAction, (void*)admin);
                            c_iterFree(checkNsHelper.retryFellows);
                            checkNsHelper.retryFellows = NULL;
                            proceed = TRUE;
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_INFO, D_THREAD_GROUP_LOCAL_LISTENER,
                            "All fellows' namespaces complete\n");
                        c_iterFree(checkNsHelper.retryFellows);
                        checkNsHelper.retryFellows = NULL;
                        proceed = TRUE;
                    }
                }
            }
            if((d_durabilityMustTerminate(durability) == TRUE) && checkNsHelper.retryFellows){
                retryFellow = d_fellow(c_iterTakeFirst(checkNsHelper.retryFellows));

                while(retryFellow){
                    d_fellowFree(retryFellow);
                    retryFellow = d_fellow(c_iterTakeFirst(checkNsHelper.retryFellows));
                }
                c_iterFree(checkNsHelper.retryFellows);
                checkNsHelper.retryFellows = NULL;
            }

        } else if(d_durabilityMustTerminate(durability) == FALSE){
            assert(conflicts == FALSE);
            /*No more conflicts; all masters have been confirmed*/
            for(i=0; i<length; i++){
                nameSpace = d_nameSpace(c_iterObject(nameSpaces, i));
                master = d_nameSpaceGetMaster(nameSpace);

                if(d_networkAddressEquals(master, myAddress)){
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                       D_THREAD_GROUP_LOCAL_LISTENER,
                       "Confirming master: I am the master for nameSpace '%s'.\n",
                       d_nameSpaceGetName(nameSpace));
                    iAmAMaster = TRUE;
                } else {
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                           D_THREAD_GROUP_LOCAL_LISTENER,
                           "Confirming master: Fellow '%u' is the master for nameSpace '%s'.\n",
                           master->systemId,
                           d_nameSpaceGetName(nameSpace));

                    /* Get masterfellow */
                    fellow = d_adminGetFellow(admin, master);
                    if(fellow) {
                        d_nameSpace fellowNamespace;
                        d_quality q;

                        /* Get fellow namespace */
                        fellowNamespace = d_fellowGetNameSpace(fellow, nameSpace);

                        q = d_nameSpaceGetInitialQuality(fellowNamespace);

                        /* Set quality of namespace to quality of master */
                        d_nameSpaceSetInitialQuality(nameSpace, q);

                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                               D_THREAD_GROUP_LOCAL_LISTENER,
                               "Quality of namespace '%s' set to %d.%.9u.\n",
                               d_nameSpaceGetName(nameSpace),
                               q.seconds,
                               q.nanoseconds);
                        d_fellowFree(fellow);
                    }
                }
                d_networkAddressFree(master);
                d_nameSpaceMasterConfirmed (nameSpace);

            }
        }
    } while ((conflicts == TRUE) && (d_durabilityMustTerminate(durability) == FALSE));

    d_nameSpacesRequestFree(checkNsHelper.request);
    assert(!checkNsHelper.retryFellows);

    d_durabilityUpdateStatistics(durability, d_statisticsUpdateConfiguration, admin);

    /* Re-report namespaces with correct confirmed status */
    d_nameSpacesRequestListenerReportNameSpaces(nsrListener);

    /* Request groups */
    doGroupsRequest(listener, nameSpaces, iAmAMaster);

    d_networkAddressFree(unaddressed);
    d_networkAddressFree(myAddress);

    return iAmAMaster;
}

/*****************************************************************************/

static c_bool
collectNameSpaces(
    d_nameSpace ns, void* userData)
{
    c_iter nameSpaces = (c_iter)userData;
    if (ns)
    {
        d_objectKeep(d_object(ns));
        c_iterInsert (nameSpaces, ns);
    }
    return TRUE;
}

static void
initPersistentData(
    d_groupLocalListener listener)
{
    d_admin admin;
    d_subscriber subscriber;
    d_store store;
    d_durability durability;
    d_group group;
    u_participant participant;
    d_storeResult result;
    d_groupList list, next;
    c_long i, length;
    d_nameSpace nameSpace;
    d_durabilityKind dkind;
    c_iter nameSpaces;
    c_bool attached;
    v_group vgroup;
    os_time waitTime;
    c_ulong count;

    admin         = d_listenerGetAdmin(d_listener(listener));
    durability    = d_adminGetDurability(admin);
    subscriber    = d_adminGetSubscriber(admin);
    store         = d_subscriberGetPersistentStore(subscriber);
    participant   = u_participant(d_durabilityGetService(durability));
    result        = d_storeGroupsRead(store, &list);

    /* Collect namespaces from admin */
    nameSpaces = d_adminNameSpaceCollect(admin);
    length        = c_iterLength(nameSpaces);

    if(result == D_STORE_RESULT_OK){

    	/* Loop namespaces */
        for(i=0; (i<length) && (d_durabilityMustTerminate(durability) == FALSE); i++) {
            nameSpace = d_nameSpace(c_iterObject(nameSpaces, i));

            if(d_nameSpaceMasterIsMe(nameSpace, admin)){
                /* This durability service is master for the nameSpace */
                dkind = d_nameSpaceGetDurabilityKind(nameSpace);

                if((dkind == D_DURABILITY_PERSISTENT) || (dkind == D_DURABILITY_ALL)){
                    os_time t = os_timeGet();

                    next = list;

                    d_durabilitySetState(durability, D_STATE_INJECT_PERSISTENT);

                    /* Loop (persistent) groups, inject data from group */
                    while(next) {
                        if(d_durabilityMustTerminate(durability) == FALSE){
                            if(d_nameSpaceIsIn(nameSpace, next->partition, next->topic) == TRUE) {
                                result = d_storeGroupInject(store, next->partition, next->topic, participant, &group);

                                if(result == D_STORE_RESULT_OK) {
                                    d_printTimedEvent(durability, D_LEVEL_FINE,
                                        D_THREAD_GROUP_LOCAL_LISTENER,
                                        "Group %s.%s locally created\n",
                                        next->partition, next->topic);


                                    d_printTimedEvent(durability, D_LEVEL_FINE,
                                        D_THREAD_GROUP_LOCAL_LISTENER,
                                        "Data from group %s.%s must now be injected\n",
                                        next->partition, next->topic);

                                    vgroup = d_groupGetKernelGroup(group);
                                    attached = d_durabilityWaitForAttachToGroup(durability, vgroup);


                                    waitTime.tv_sec  = 0;
                                    waitTime.tv_nsec = 100000000; /*100ms*/
                                    count = 0;

                                    while(  (c_count(vgroup->streams) == 0) &&
                                            (count < 30)){
                                        os_nanoSleep(waitTime);
                                        count++;
                                    }
                                    c_free(vgroup);

                                    result = d_storeMessagesInject(store, group);

                                    if(result == D_STORE_RESULT_OK) {
                                        d_printTimedEvent(durability, D_LEVEL_FINE,
                                            D_THREAD_GROUP_LOCAL_LISTENER,
                                            "All data for group %s.%s has been injected from local store.\n",
                                            next->partition, next->topic);
                                    } else {
                                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                            D_THREAD_GROUP_LOCAL_LISTENER,
                                            "All data for group %s.%s could not be injected.\n",
                                            next->partition, next->topic);
                                    }
                                    if(!attached){
                                        d_groupSetPrivate(group, TRUE);
                                    }
                                    d_groupSetComplete(group);
                                    d_adminAddLocalGroup(admin, group);
                                    d_sampleChainListenerReportGroup(listener->sampleChainListener, group);
                                } else {
                                    d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                        D_THREAD_GROUP_LOCAL_LISTENER,
                                        "Group %s.%s could NOT be created locally (%d)\n",
                                        next->partition, next->topic, result);
                                }
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                            D_THREAD_GROUP_LOCAL_LISTENER,
                                            "Group %s.%s not in nameSpace.\n",
                                            next->partition, next->topic);
                            }
                        }
                        next = d_groupList(next->next);
                    }
                    d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_GROUP_LOCAL_LISTENER,
                                                  "Initializing persistent data took %f sec\n", os_timeToReal(os_timeSub(os_timeGet(), t)));
                }
            } else {
            	/* If not master, backup old persistent data (would be overwritten otherwise) */
                result = d_storeBackup(store, nameSpace);

                if(result != D_STORE_RESULT_OK) {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Namespace could NOT be backupped in local persistent store (%d)\n",
                        result);
                }

                /* Mark namespace incomplete */
                d_storeNsMarkComplete (store, nameSpace, FALSE);
            }
        }
    } else {
        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                            D_THREAD_GROUP_LOCAL_LISTENER,
                            "Could not read groups from persistent store. Persistent data not injected.\n");
    }

    d_storeGroupListFree(store, list);

    /* Free namespace list */
    d_adminNameSpaceCollectFree(admin, nameSpaces);
}

/*
 * PRECONDTION: listener->masterLock is locked
 */
static void
initMasters(
    d_groupLocalListener listener)
{
    d_admin admin;
    d_durability durability;
    c_bool fellowGroupsKnown, terminate;
    os_time sleepTime;
    c_iter nameSpaces;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);

    /* Collect namespaces from admin */
    nameSpaces = d_adminNameSpaceCollect(admin);

    /* Initialize with namespaces from configuration */
    d_groupLocalListenerDetermineMasters(listener, nameSpaces);

    fellowGroupsKnown = FALSE;
    sleepTime.tv_sec  = 0;
    sleepTime.tv_nsec = 100000000; /* 100 ms */
    terminate = d_durabilityMustTerminate(durability);

    /*now wait for completion of groups*/
    while((fellowGroupsKnown == FALSE) && (!terminate) && (d_adminGetFellowCount(admin) > 0)){
        d_adminFellowWalk(admin, checkFellowGroupsKnown, &fellowGroupsKnown);
        os_nanoSleep(sleepTime);
        terminate = d_durabilityMustTerminate(durability);
    }

    if(!terminate){
        d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN, "Fellow groups complete.\n");
    }

    /* Free namespace list */
    d_adminNameSpaceCollectFree(admin, nameSpaces);
}

struct fellowState {
	d_networkAddress address;
	d_serviceState state;
	d_table nameSpaces; /* Namespace states */
};

struct masterHelper {
    d_groupLocalListener listener;
    c_iter nameSpaces;
    c_iter fellowStates;
};

static c_bool
fellowStateCopyNsWalk  (
    d_nameSpace nameSpace,
    c_voidp userData)
{
    d_table nameSpaces;

    nameSpaces = d_table(userData);

    assert (nameSpaces);

    d_tableInsert (nameSpaces, d_nameSpaceCopy(nameSpace));

    return TRUE;
}

/* Walk fellows, insert fellow state in iterator list */
static c_bool
fellowStateWalk (
    d_fellow fellow,
    c_voidp userData)
{
	/* Create new fellow object */
	struct fellowState* fellowStateObject;

	fellowStateObject = os_malloc (sizeof(struct fellowState));
	if (!fellowStateObject)
	{
		return FALSE;
	}

	/* Set address and state of fellowstate object */
	fellowStateObject->address = d_fellowGetAddress (fellow);
	fellowStateObject->state = d_fellowGetState (fellow);

	/* Copy namespace states */
	fellowStateObject->nameSpaces = d_tableNew(d_nameSpaceNameCompare, d_nameSpaceFree);
	d_fellowNameSpaceWalk (fellow, fellowStateCopyNsWalk, fellowStateObject->nameSpaces);

	/* Insert fellow state */
	c_iterInsert (userData, fellowStateObject);

	return TRUE;
}

/* Clean fellowstate object */
static void
fellowStateCleanWalk (
    void* o,
    c_iterActionArg arg)
{
    struct fellowState* fellowStateObject;

    OS_UNUSED_ARG(arg);
    fellowStateObject = (struct fellowState*)o;

	d_networkAddressFree(fellowStateObject->address);
	d_tableFree (fellowStateObject->nameSpaces);

	os_free (fellowStateObject);
}

/* Clean fellowstates & iterator */
static void
freeFellowStates (
    c_iter fellowStates)
{
	c_iterWalk (fellowStates, fellowStateCleanWalk, NULL);
	c_iterFree (fellowStates);
}

/* Store snapshot of fellow states for future reference */
static c_iter
fetchFellowStates(d_groupLocalListener listener)
{
	c_iter fellowStates;
	d_admin admin;
	d_durability durability;
	struct fellowState* fellowStateObject;

	admin =  d_listenerGetAdmin(d_listener(listener));
	durability = d_adminGetDurability(admin);
	fellowStates = c_iterNew (NULL);

	/* Fill fellow list */
	d_adminFellowWalk (admin, fellowStateWalk, fellowStates);

	/* Create fellow state for own durability service */
	fellowStateObject = os_malloc (sizeof(struct fellowState));
	if (!fellowStateObject)
	{
		freeFellowStates (fellowStates);
		fellowStates = NULL;
	}else
	{
		fellowStateObject->address = d_adminGetMyAddress(admin);
		fellowStateObject->state = d_durabilityGetState (durability);
	    fellowStateObject->nameSpaces = NULL;

        /* Insert own address & state to list */
        c_iterInsert (fellowStates, fellowStateObject);
	}

	return fellowStates;
}

/* Mark namespace with state of fellow */
static void
markNameSpace (
		const d_nameSpace nameSpace,
		const c_iter fellowStates,
		d_admin admin)
{
	d_durability durability;
	d_configuration config;
	d_networkAddress master;
	d_nameSpace fellowNameSpace, dummy;
	struct fellowState* fellowStateObject;
	c_long i;

	durability = d_adminGetDurability(admin);
	config = d_durabilityGetConfiguration (durability);
	master = d_nameSpaceGetMaster(nameSpace);
    i = 0;

	/* Walk fellow states */
	while ((fellowStateObject = (struct fellowState*)c_iterObject(fellowStates, i++)))
	{
		if (d_networkAddressEquals(fellowStateObject->address, master)) {
			/* Only D_STATE_COMPLETE states can be overwritten */
			if (d_nameSpaceGetMasterState(nameSpace) == D_STATE_COMPLETE) {
				d_nameSpaceSetMasterState (nameSpace, fellowStateObject->state);
			}

			dummy = d_nameSpaceNew (config, d_nameSpaceGetName (nameSpace));

			if (fellowStateObject->nameSpaces) {
			    fellowNameSpace = d_tableFind (fellowStateObject->nameSpaces, dummy);

                /* If nameSpace would not exist, fellow could not have been master for this namespace */
                assert (fellowNameSpace);

                /* Copy mergestate of namespace */
                d_nameSpaceSetMergeState(nameSpace, d_nameSpaceGetMergeState(fellowNameSpace, NULL));
			}

			d_nameSpaceFree(dummy);

			break;
		}
	}

	d_networkAddressFree (master);
}

/* Walk function that marks namespace master state */
static void
nameSpaceMarkWalk (
		void* o,
		c_iterActionArg userData)
{
	c_iter fellowStates;
	d_admin admin;
	d_nameSpace nameSpace = d_nameSpace(o);

	fellowStates = (c_iter)((struct masterHelper*)userData)->fellowStates;
	admin = d_listenerGetAdmin(d_listener(((struct masterHelper*)userData)->listener));

	markNameSpace (nameSpace, fellowStates, admin);
}

static void
determineNewMasters(
    d_groupLocalListener listener,
    c_iter fellowStates,
    c_iter nameSpaces)
{
    c_bool tryChains;
    struct masterHelper helper;
    tryChains = FALSE;

    if(d_objectIsValid(d_object(listener), D_LISTENER)){
        os_mutexLock(&listener->masterLock);

        if(c_iterLength(nameSpaces) > 0){
            tryChains = d_groupLocalListenerDetermineMasters(listener, nameSpaces);

            if (fellowStates){
                helper.listener = listener;
                helper.fellowStates = fellowStates;
                helper.nameSpaces = NULL; /* Not used in this walk */

                c_iterWalk (nameSpaces, nameSpaceMarkWalk, &helper);

                /* Cleanup fellow states */
                freeFellowStates (fellowStates);
            }
        }

        os_mutexUnlock(&listener->masterLock);

        if(tryChains){
            d_sampleChainListenerTryFulfillChains(listener->sampleChainListener, NULL);
        }
    }
}

static c_bool
determineNewMastersAction(
    d_action action,
    c_bool terminate)
{
    struct masterHelper* helper;

    helper = (struct masterHelper*)(d_actionGetArgs(action));

    if(terminate == FALSE){
        determineNewMasters (helper->listener, helper->fellowStates, helper->nameSpaces);
    }

    c_iterFree(helper->nameSpaces);
    os_free(helper);

    return FALSE;
}

struct nsGroupAlignWalkData
{
    d_durability durability;
    d_nameSpace nameSpace;
    d_groupLocalListener listener;
    c_iter groups;
};

/* Start alignment for existing groups in specific namespace */
static c_bool
nsCollectGroupWalk(
    d_group group,
    c_voidp userData)
{
    c_bool inNameSpace;
    d_partition partition;
    d_topic topic;
    struct nsGroupAlignWalkData* walkData = (struct nsGroupAlignWalkData*)userData;

    assert(walkData);

    partition = d_groupGetPartition(group);
    topic = d_groupGetTopic(group);

    if(walkData->nameSpace){
        inNameSpace = d_nameSpaceIsIn(walkData->nameSpace, partition, topic);

        if(inNameSpace){
            d_printTimedEvent(walkData->durability, D_LEVEL_FINEST,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "-Group %s.%s.\n",
                partition, topic);
            c_iterAppend (walkData->groups, group);
        }
    } else {
        d_printTimedEvent(walkData->durability, D_LEVEL_FINEST,
               D_THREAD_GROUP_LOCAL_LISTENER,
               "- Group %s.%s.\n",
               partition, topic);
        c_iterAppend (walkData->groups, group);
    }
    os_free(partition);
    os_free(topic);

    return TRUE;
}

static void
handleGroupAlignmentWalk (
    void* o,
    c_voidp userData)
{
    struct nsGroupAlignWalkData* walkData = (struct nsGroupAlignWalkData*)userData;

    d_group group = d_group(o);
    d_groupLocalListener listener;
    d_admin admin;
    d_nameSpace nameSpace;
    d_partition partition;
    d_topic topic;

    admin       = d_listenerGetAdmin(d_listener(walkData->listener));
    partition   = d_groupGetPartition(group);
    topic       = d_groupGetTopic(group);
    listener    = walkData->listener;

    /* Compare namespace in walkdata with namespace from group */
    nameSpace = d_adminGetNameSpaceForGroup(admin, partition, topic);
    if (!d_nameSpaceCompare (walkData->nameSpace, nameSpace))
    {
        /* Start alignment of group when namespaces are equal */
        d_groupLocalListenerHandleAlignment(
            listener,
            group,
            NULL);
    }

    os_free (partition);
    os_free (topic);
}

static void
setGroupIncomplete (
    void* o,
    c_voidp userData)
{
    d_group group;

    OS_UNUSED_ARG(userData);

    group = d_group(o);

    d_groupSetIncomplete(group);
}

static c_bool
collectGroups(
    d_group group,
    c_voidp args)
{
    c_iterInsert((c_iter)args, d_objectKeep(d_object(group)));
    return TRUE;
}

static void
handleMergeAlignment(
    d_groupLocalListener listener,
    d_nameSpace fellowNameSpace,
    d_fellow fellow,
    d_mergeState newState)
{
    d_mergeAction mergeAction;
    d_admin admin;
    d_durability durability;
    d_configuration config;
    c_iter groups;
    d_group group;
    d_partition partition;
    d_topic topic;
    d_durabilityKind dkind;
    d_networkAddress fellowAddress;
    c_bool inNameSpace, success;
    d_chain chain;
    c_time stamp, networkAttachTime, zeroTime;
    d_sampleRequest request;
    os_uint32 groupCount;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    groups = c_iterNew(NULL);
    groupCount = 0;
    fellowAddress = d_fellowGetAddress(fellow);

    d_adminGroupWalk(admin, collectGroups, groups);

    mergeAction = d_mergeActionNew(fellowNameSpace, fellow, newState);

    d_mergeStateFree (newState);

    group = d_group(c_iterTakeFirst(groups));

    /* Create sample requests for all groups that are part of the nameSpace */
    while(group){
        partition = d_groupGetPartition(group);
        topic = d_groupGetTopic(group);
        dkind = d_groupGetKind(group);
        inNameSpace = d_nameSpaceIsIn(fellowNameSpace, partition, topic);

        if(inNameSpace){
            stamp = v_timeGet();

            if(config->timeAlignment){
                networkAttachTime = stamp;
            } else {
                networkAttachTime = C_TIME_INFINITE;
            }
            zeroTime.seconds     = 0;
            zeroTime.nanoseconds = 0;

            request = d_sampleRequestNew(
                            admin, partition, topic,
                            dkind, stamp, FALSE,
                            zeroTime, networkAttachTime);
            chain = d_chainNew(admin, request);

            d_mergeActionAddChain(mergeAction, chain);
            groupCount++;
        }
        os_free(partition);
        os_free(topic);
        d_groupFree(group);
        group = d_group(c_iterTakeFirst(groups));
    }
    c_iterFree(groups);

    d_printTimedEvent(durability, D_LEVEL_INFO,
        D_THREAD_GROUP_LOCAL_LISTENER,
        "Inserting merge request to merge '%d' groups with fellow '%d' for "\
        "nameSpace '%s'\n",
        groupCount,
        fellowAddress->systemId,
        d_nameSpaceGetName(fellowNameSpace));

    success = d_sampleChainListenerInsertMergeAction(
            listener->sampleChainListener, mergeAction);

    if(success == FALSE){
        d_mergeActionFree(mergeAction);

        d_printTimedEvent(durability, D_LEVEL_INFO,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "Merge of '%d' groups with fellow '%d' for "\
                "nameSpace '%s' is already in progress, so not "\
                "issuing new request.\n",
                groupCount,
                fellowAddress->systemId,
                d_nameSpaceGetName(fellowNameSpace));
    }
    d_networkAddressFree(fellowAddress);
    d_nameSpaceFree (fellowNameSpace);

    return;
}

struct applyMergePolicyHelper {
    d_groupLocalListener listener;
    d_fellow fellow;            /* When set this fellow is master for the namespace. */
    d_nameSpace nameSpace;      /* If set, applyMergePolicy will only check this namespace, otherwise all namespaces are evaluated */
    d_table fellowStates;       /* Fellow states at the time of the namespace event */
    c_iter conflictStates;      /* Result of mergeState compare between own namespace and fellow namespace */
    c_ulong event;              /* the event that triggered the merge action */
    d_mergeState oldMergeState; /* the merge state of the namespace before a new master was determined */
};

/* This function determines whether a merge action is needed or not and
 * if so, it forwards the request. */
static c_bool
applyMergePolicy(
    d_action action,
    c_bool terminate)
{
    struct applyMergePolicyHelper* helper;
    d_admin admin;
    d_durability durability;
    d_networkAddress fellowMaster, fellowAddress, myMaster;
    c_iter fellowNameSpaces;
    d_nameSpace fellowNameSpace, nameSpace;
    c_bool nameSpacesKnown;
    c_bool amIMaster;
    d_alignmentKind aKind;
    c_bool callAgain = TRUE;
    d_mergePolicy merge;
    d_name role, fellowRole;
    d_mergeState ownState, fellowState, newState, conflictState;

    helper = (struct applyMergePolicyHelper*)(d_actionGetArgs(action));
    admin = d_listenerGetAdmin(d_listener(helper->listener));
    durability = d_adminGetDurability(admin);
    fellowNameSpaces = NULL;

    /* Set the address of the fellow */
    if (! helper->fellow) {
        fellowAddress = d_networkAddressUnaddressed();
    } else {
        fellowAddress = d_fellowGetAddress(helper->fellow);
    }

    /* Wait until service and fellow reach COMPLETE state (action will be rescheduled) */
    if (!terminate && (d_durabilityGetState(durability) != D_STATE_COMPLETE)) {
        d_printTimedEvent(durability, D_LEVEL_INFO,
            D_THREAD_GROUP_LOCAL_LISTENER,
            "I am not complete. Not yet investigating merge with fellow %u.\n",
            fellowAddress->systemId);

    } else if (!terminate && helper->fellow && (d_fellowGetState(helper->fellow) != D_STATE_COMPLETE)) {
        d_printTimedEvent(durability, D_LEVEL_INFO,
            D_THREAD_GROUP_LOCAL_LISTENER,
            "Fellow %u is not complete. Not yet investigating merge.\n",
            fellowAddress->systemId);

    } else if (!terminate) {
        /* check if a fellow is available */
        if (helper->fellow) {
            /* check if all nameSpaces are known */
            nameSpacesKnown = d_fellowAreNameSpacesComplete(helper->fellow);

            /* Make sure all nameSpaces of the fellow are known before proceeding */
            if(nameSpacesKnown) {

                /* If a namespace is set in the helper, only check if durability
                 * should merge for that namespace */
                if (helper->nameSpace) {
                    fellowNameSpaces = c_iterNew (helper->nameSpace);
                    fellowNameSpace = d_nameSpace(c_iterTakeFirst(fellowNameSpaces));
                /* Otherwise, check for all namespaces in that fellow */
                } else {
                    fellowNameSpaces = c_iterNew(NULL);
                    d_fellowNameSpaceWalk(helper->fellow, collectNameSpaces, fellowNameSpaces);
                    fellowNameSpace = d_nameSpace(c_iterTakeFirst(fellowNameSpaces));
                }

                /* Walk over all nameSpaces of the fellow to determine what
                 * merge activity needs to be performed.
                 */
                while (fellowNameSpace){
                    fellowMaster = d_nameSpaceGetMaster(fellowNameSpace);

                    /* If and only if the fellow is a 'master', a merge action
                     * is necessary. The assumption is that the configuration of
                     * the networking service makes sure, that nodes are only
                     * removed from the reliability protocol after the
                     * durability service has determined the new master already.
                     *
                     * This means the detection time of a disconnection plus the
                     * time to determine a new master needs to be <= the time
                     * after which networking kicks out a node from the reliability
                     * protocol.
                     */

                    d_printTimedEvent(durability, D_LEVEL_INFO,
                            D_THREAD_GROUP_LOCAL_LISTENER,
                            "Fellow '%d' was master for nameSpace "\
                            "%s; investigating merge.\n",
                            fellowAddress->systemId,
                            d_nameSpaceGetName(fellowNameSpace));

                    nameSpace = d_adminGetNameSpace(admin,
                            d_nameSpaceGetName(fellowNameSpace));

                    /* Check if the fellow nameSpace is locally known. */
                    if (nameSpace) {

                        /* Get own role */
                        role = d_nameSpaceGetRole(nameSpace);
                        /* Get the role of the fellow */
                        fellowRole = d_nameSpaceGetRole(fellowNameSpace);
                        /* Get the merge policy for the fellow role */
                        merge = d_nameSpaceGetMergePolicy(nameSpace, fellowRole);
                        /* Get the master for the namespace */
                        myMaster = d_nameSpaceGetMaster (nameSpace);

                        /* Only merge if merge policy is not ignore or if there are conflict states */
                        if (helper->conflictStates || (merge != D_MERGE_IGNORE)) {

                            /* Only perform merge if the alignment policy
                             * is initial or lazy. */
                            aKind = d_nameSpaceGetAlignmentKind(nameSpace);

                            if((aKind == D_ALIGNEE_INITIAL)||
                               (aKind == D_ALIGNEE_LAZY)){
                                /* Check if I am master for the local administrated namespace */
                                amIMaster = d_nameSpaceMasterIsMe (nameSpace, admin);

                               /* There are four conditions when a merge should occur:
                                * 1. I am master and I encounter another (confirmed) master
                                * 2. I am master and I encounter a node without confirmed master
                                * 3. I am not a master and I encounter a different or new namespace state from my master
                                * 4. I am master and I encounter a master from another role with an previously unseen state
                                *
                                * Note: when your master leaves a D_FELLOW_REMOVE event is eventually triggered.
                                * If no alternative master could be found the mergestate is cleared. When a
                                * new master appears case 3 is triggered again.
                                */

                                /* use the state before a new master was determined as the state of the fellow */
                                fellowState = d_nameSpaceGetMergeState (fellowNameSpace, fellowRole);

                                /* The oldMergeState is always for my own role, so when merging with another role
                                 * get mergestate for correct role */
                                if(strcmp(fellowRole,role) != 0) {
                                    ownState = d_nameSpaceGetMergeState(nameSpace, role);
                                } else if (helper->oldMergeState != NULL) {
                                    ownState = d_mergeStateNew(helper->oldMergeState->role, helper->oldMergeState->value);
                                } else {
                                    /* <empty> state because no aligner is available.
                                     * Note that this can only happen if you are not master yourself!
                                     */
                                    ownState = NULL;
                                }
                                /* I am a master */
                                if (amIMaster) {
                                    if (strcmp (fellowRole, role) == 0) {
                                        /* Case 1: check if I am master and I encounter another (confirmed) master
                                         * Case 2: check if I am master encounter a node without confirmed master
                                         */
                                        if ((d_networkAddressEquals(fellowMaster, fellowAddress) && d_nameSpaceIsMasterConfirmed(fellowNameSpace)) ||
                                               !d_nameSpaceIsMasterConfirmed(fellowNameSpace)) {
                                                d_printTimedEvent(durability, D_LEVEL_INFO,
                                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                                    "Applying merge with fellow '%d' for "\
                                                    "nameSpace '%s'.\n",
                                                    fellowAddress->systemId,
                                                    d_nameSpaceGetName(nameSpace));

                                                /* Increase state counter by one when I am master */
                                                newState = d_mergeStateNew (ownState->role, ownState->value + 1);

                                                /* A merge is needed for this nameSpace;
                                                 * taking further action */
                                                handleMergeAlignment(helper->listener,
                                                        d_nameSpaceCopy(fellowNameSpace), helper->fellow, newState);

                                        } else { /* If other fellow never was master, I shouldn't have to do anything */
                                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                                D_THREAD_GROUP_LOCAL_LISTENER,
                                                "No need to merge with fellow '%d'.\n",
                                                fellowAddress->systemId,
                                                d_nameSpaceGetName(nameSpace));
                                        }
                                    } else {
                                        /* Case 4: check if fellow is a confirmed master and the state between my state the fellow state differs */
                                        /* If I am master, fellow is (confirmed) master for other role and native state is different, apply merge */
                                        if ((ownState != fellowState) && (d_networkAddressEquals(fellowMaster, fellowAddress)) && d_nameSpaceIsMasterConfirmed(fellowNameSpace)) {
                                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                                D_THREAD_GROUP_LOCAL_LISTENER,
                                                "Applying merge between roles with fellow '%d' for "\
                                                "nameSpace '%s' from role '%s'.\n",
                                                fellowAddress->systemId,
                                                d_nameSpaceGetName(nameSpace),
                                                fellowRole);

                                            /* When I am not master, copy state from master fellow */
                                            newState = d_mergeStateNew(fellowState->role, fellowState->value);

                                            /* A merge is needed for this nameSpace;
                                             * taking further action */
                                            handleMergeAlignment(helper->listener,
                                                    d_nameSpaceCopy(fellowNameSpace), helper->fellow, newState);
                                        } else {
                                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                                D_THREAD_GROUP_LOCAL_LISTENER,
                                                "No need to merge with fellow '%d' from role '%s'.\n",
                                                fellowAddress->systemId,
                                                d_nameSpaceGetName(nameSpace),
                                                fellowRole);
                                        }
                                    }

                                /* I am not a master */
                                } else {

                                    /* Case 3: I am not a master and I encounter a different or new namespace state from my master */
                                    if ((!ownState || (ownState->value != fellowState->value)) &&
                                        (d_networkAddressEquals(fellowAddress, myMaster) && d_nameSpaceIsMasterConfirmed(fellowNameSpace))) {

                                        if(ownState) {
                                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                                D_THREAD_GROUP_LOCAL_LISTENER,
                                                "Own namespace state '%d' for role '%s'"\
                                                "differs from state '%d' from my master '%d'"\
                                                "for nameSpace '%s'.\n",
                                                ownState->value,
                                                ownState->role,
                                                fellowState->value,
                                                fellowAddress->systemId,
                                                d_nameSpaceGetName(nameSpace));
                                        }else {
                                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                                    "Own namespace state <empty> for role '%s'"\
                                                    "differs from state '%d' from my master '%d'"\
                                                    "for nameSpace '%s'.\n",
                                                    fellowState->role,
                                                    fellowState->value,
                                                    fellowAddress->systemId,
                                                    d_nameSpaceGetName(nameSpace));
                                        }

                                        d_printTimedEvent(durability, D_LEVEL_INFO,
                                            D_THREAD_GROUP_LOCAL_LISTENER,
                                            "Applying merge with fellow '%d' for "\
                                            "nameSpace '%s'.\n",
                                            fellowAddress->systemId,
                                            d_nameSpaceGetName(nameSpace));

                                        /* When I am not master, copy state from master fellow */
                                        newState = d_mergeStateNew(fellowState->role, fellowState->value);

                                        /* A merge is needed for this nameSpace;
                                         * take a copy of the namespace at this moment in time 
                                         */
                                        handleMergeAlignment(helper->listener,
                                                d_nameSpaceCopy(fellowNameSpace), helper->fellow, newState);

                                    /* A master with conflicting states for other roles is detected */
                                    } else if (helper->conflictStates) {
                                        if (c_iterLength(helper->conflictStates) > 1) {
                                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                                D_THREAD_GROUP_LOCAL_LISTENER,
                                                "Multiple role state changes detected for "\
                                                "nameSpace '%s' while own state has not changed. "\
                                                "Not applying merge for now (wait for master update).\n",
                                                fellowAddress->systemId,
                                                d_nameSpaceGetName(nameSpace));
                                        } else {

                                            conflictState = c_iterTakeFirst (helper->conflictStates);

                                            /* Determine merge policy for conflicting role */
                                            merge = d_nameSpaceGetMergePolicy (nameSpace, conflictState->role);
                                            if (merge != D_MERGE_IGNORE) {
                                                d_printTimedEvent(durability, D_LEVEL_INFO,
                                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                                    "Applying merge for conflicting state from role '%s' with fellow '%d' for "\
                                                    "nameSpace '%s'.\n",
                                                    conflictState->role,
                                                    fellowAddress->systemId,
                                                    d_nameSpaceGetName(nameSpace));

                                                handleMergeAlignment (helper->listener,
                                                        d_nameSpaceCopy(fellowNameSpace), helper->fellow, conflictState);
                                            }
                                        }

                                    } else {
                                        d_printTimedEvent(durability, D_LEVEL_INFO,
                                            D_THREAD_GROUP_LOCAL_LISTENER,
                                            "No need to merge with fellow '%d' for now.\n",
                                            fellowAddress->systemId,
                                            d_nameSpaceGetName(nameSpace));
                                    }
                                }
                                d_mergeStateFree (ownState);
                                d_mergeStateFree (fellowState);

                            } else {
                                d_printTimedEvent(durability, D_LEVEL_INFO,
                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                    "Ignoring merge with fellow '%d' for "\
                                    "nameSpace '%s' because alignee policy " \
                                    "does not ask for it.\n",
                                    fellowAddress->systemId,
                                    d_nameSpaceGetName(nameSpace));
                            }
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                    "Ignoring merge with fellow '%d' for "\
                                    "nameSpace '%s' because merge policy is ignore.\n",
                                    fellowAddress->systemId,
                                    d_nameSpaceGetName(nameSpace));
                        }
                        d_nameSpaceFree(nameSpace);
                        d_networkAddressFree(myMaster);
                        os_free (fellowRole);
                        os_free (role);

                    } else {
                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                    "Namespace '%s' not available locally, "\
                                    "skipping merge with fellow '%d' for this namespace.\n",
                                    d_nameSpaceGetName(nameSpace));
                    }
                    if (!helper->nameSpace) {
                        d_nameSpaceFree(fellowNameSpace);
                    }
                    d_networkAddressFree(fellowMaster);

                    fellowNameSpace = d_nameSpace(c_iterTakeFirst(fellowNameSpaces));
                } /* while */
                if(helper->oldMergeState){
                    d_mergeStateFree(helper->oldMergeState);
                    helper->oldMergeState = NULL;
                }
                callAgain = FALSE;
            } else {
                d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "Waiting for nameSpaces of fellow '%d' to become available "\
                    "before taking merge action.\n",
                    fellowAddress->systemId);
            }
        } else {
            /* No fellow is selected as master yet.
             * Reschedule until a fellow for the namespace has been found.
             */
            if (helper->nameSpace) {
                /* get the CURRENT namespace that matches helper->nameSpace */
                nameSpace = d_adminGetNameSpace(admin, d_nameSpaceGetName(helper->nameSpace));
                if (d_nameSpaceIsMasterConfirmed(nameSpace)) {
                    /* the namespace is complete, retrieve the master */
                    myMaster = d_nameSpaceGetMaster(nameSpace);
                    helper->fellow = d_adminGetFellow(admin, myMaster);
                    d_networkAddressFree(myMaster);
                } else {
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Waiting for namespace '%s' to become complete "\
                        "before taking merge action.\n",
                        fellowAddress->systemId);
                }
            } else {
               /* Either a fellow or namespace should be available.
                  If none are specified no merge can be performed*/
                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "Missing fellow and namespace, no merge action cannot be performed.\n");
            }
        } 
    } /* terminate */
    if (fellowAddress) {
        d_free (fellowAddress);
    }

    if (fellowNameSpaces) {
        c_iterFree (fellowNameSpaces);
    }

    if(terminate || !callAgain){

        if (helper->nameSpace) {
            d_nameSpaceFree (helper->nameSpace);
        }

        if (helper->conflictStates) {
            conflictState = c_iterTakeFirst (helper->conflictStates);
             while (conflictState) {
                 d_mergeStateFree (conflictState);
                 conflictState = c_iterTakeFirst (helper->conflictStates);
             }
             c_iterFree (helper->conflictStates);
        }

        d_fellowFree(helper->fellow);
        os_free(helper);
    }

    return callAgain;
}

typedef struct applyDelayedAlignment_t {
    d_groupLocalListener listener;
    d_nameSpace nameSpace;
    d_fellow fellow;
}applyDelayedAlignment_t;

static c_bool
applyDelayedAlignment (
    d_action action,
    c_bool terminate)
{
    c_iter nameSpaces;
    d_admin admin;
    d_durability durability;
    d_groupLocalListener listener;
    d_nameSpace ns;
    applyDelayedAlignment_t* actionData;
    d_networkAddress master;
    d_fellow masterFellow;
    d_quality q;
    c_bool callAgain = TRUE;
    struct nsGroupAlignWalkData walkData;

    OS_UNUSED_ARG(terminate);
    actionData = (applyDelayedAlignment_t*)d_actionGetArgs(action);
    listener = actionData->listener;
    durability  = d_adminGetDurability(d_listenerGetAdmin(d_listener(listener)));
    admin = d_listenerGetAdmin(d_listener(listener));
    ns = actionData->nameSpace;

    /* Only do action when fellow has reached discover_persistent_source state, so master-selection is synced. */
    if(d_fellowGetState(actionData->fellow) >= D_STATE_DISCOVER_PERSISTENT_SOURCE) {

        /* Re-determine master for namespace */
        nameSpaces = c_iterNew(ns);
        determineNewMasters (listener, NULL, nameSpaces);
        c_iterFree(nameSpaces);

        /* If I am not the new master, and the namespace quality of the new master is not infinite, I will
         * request alignment for the namespace. */

        /* Get master */
        master = d_nameSpaceGetMaster(ns);

        /* Get fellow namespace */
        masterFellow = d_adminGetFellow(admin, master);

        if(masterFellow) {
            /* Re-align groups */
            if(d_durabilityMustTerminate(durability) == FALSE){
                walkData.durability = durability;
                walkData.listener = listener;
                walkData.nameSpace = ns;
                walkData.groups = c_iterNew(NULL);
                admin = d_listenerGetAdmin(d_listener(listener));

                d_printTimedEvent(durability, D_LEVEL_FINE,
                   D_THREAD_GROUP_LOCAL_LISTENER,
                   "Collecting groups for namespace %s to apply delayed alignment.\n",
                   d_nameSpaceGetName(ns));
                /* Collect groups */
                d_adminGroupWalk (admin, nsCollectGroupWalk, &walkData);

                /* Set completeness of group back to incomplete again */
                c_iterWalk(walkData.groups, setGroupIncomplete, 0);

                /* Align groups */
                c_iterWalk (walkData.groups, handleGroupAlignmentWalk, &walkData);

                /* Namespace has no longer zero-quality, set it to infinite so it can't be written to anymore. */
                q = C_TIME_INFINITE;
                d_nameSpaceSetInitialQuality(ns, q);
            }

        }else {
            d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Fellow '%d' lost before starting delayed alignment, or I have become master.\n",
                        master->systemId);
        }
        callAgain = FALSE;
        d_networkAddressFree(master);
    }else {
        d_printTimedEvent(durability, D_LEVEL_INFO,
                   D_THREAD_GROUP_LOCAL_LISTENER,
                   "Redo applyDelayedAlignment (namespace %s) - fellow not yet in DISCOVER_PERSISTENT_SOURCE state.\n",
                   d_nameSpaceGetName(ns));
    }

    return callAgain;
}

static c_bool
notifyNameSpaceEvent(
    c_ulong event,
    d_fellow fellow,
    d_nameSpace ns,
    d_group group,
    c_voidp eventUserData,
    c_voidp userData)
{
    struct masterHelper *helper;
    struct nsGroupAlignWalkData walkData;
    struct applyMergePolicyHelper *mergeHelper;
    d_durability durability;
    d_admin admin;
    d_groupLocalListener listener;
    d_fellow masterFellow;
    d_action action;
    d_networkAddress master;
    os_time sleepTime;
    d_nameSpace adminNs;
    char* nameSpaceName;
    c_iter fellowStates;
    c_iter nameSpaces;
    d_mergeState oldMergeState = NULL;

    OS_UNUSED_ARG(fellow);
    OS_UNUSED_ARG(group);


    /* New namespace detected */
    if (event == D_NAMESPACE_NEW)
    {
        listener = d_groupLocalListener(userData);
        durability  = d_adminGetDurability(d_listenerGetAdmin(d_listener(listener)));

        assert (listener);

        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 100000000;

        nameSpaces = c_iterNew(ns);
        helper = (struct masterHelper*)(os_malloc(sizeof(struct masterHelper)));
        helper->listener = listener;
        helper->nameSpaces = nameSpaces;
        fellowStates = fetchFellowStates (listener);

        /* Determine new master for new namespace */
        determineNewMasters (listener, fellowStates, nameSpaces);

        /* Align new groups */
        if(d_durabilityMustTerminate(durability) == FALSE){
            walkData.durability = durability;
            walkData.listener = listener;
            walkData.nameSpace = ns;
            walkData.groups = c_iterNew(NULL);
            admin = d_listenerGetAdmin(d_listener(listener));

            d_printTimedEvent(durability, D_LEVEL_FINE,
                   D_THREAD_GROUP_LOCAL_LISTENER,
                   "Collecting groups to apply alignment for new namespace %s.\n",
                   d_nameSpaceGetName(ns));
            /* Collect groups */
            d_adminGroupWalk (admin, nsCollectGroupWalk, &walkData);

            /* Align groups */
            c_iterWalk (walkData.groups, handleGroupAlignmentWalk, &walkData);
        }

        c_iterFree (nameSpaces);
        os_free (helper);

    /* Detected a conflicting (confirmed) master for a namespace */
    } else if (event & (D_NAMESPACE_MASTER_CONFLICT | D_NAMESPACE_STATE_CONFLICT)) {
        listener = d_groupLocalListener(userData);
        durability  = d_adminGetDurability(d_listenerGetAdmin(d_listener(listener)));
        admin = d_listenerGetAdmin(d_listener(listener));

        /* When a conflicting master is detected, insert merge action */
        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;
        master = d_nameSpaceGetMaster (ns);
        masterFellow = d_adminGetFellow (admin, master);

        nameSpaceName = d_nameSpaceGetName (ns);
        adminNs = d_adminGetNameSpace (admin, nameSpaceName);
        /* remember the current state of adminNS for your own role */
        oldMergeState = d_nameSpaceGetMergeState(adminNs, NULL);

        /* If a master conflict is encountered, re-determine masters */
        if (event == D_NAMESPACE_MASTER_CONFLICT) {
            nameSpaces = c_iterNew(adminNs);
            d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "Master conflict detected, redetermine master for nameSpace '%s'\n",
                    d_nameSpaceGetName(ns));

            /* (no need to re-evaluate fellow states (NULL)) */
            determineNewMasters (listener, NULL, nameSpaces);
            c_iterFree (nameSpaces);
        }
        d_nameSpaceFree (adminNs);

        if (masterFellow) {
            mergeHelper = (struct applyMergePolicyHelper*)(os_malloc(sizeof(struct applyMergePolicyHelper)));
            mergeHelper->event = event;
            mergeHelper->listener = listener;
            mergeHelper->fellow = d_fellow(d_objectKeep(d_object(masterFellow)));
            mergeHelper->nameSpace = ns; /* Only check this namespace if it needs merging */
            mergeHelper->conflictStates = (c_iter)eventUserData;
            mergeHelper->oldMergeState = oldMergeState;
            action = d_actionNew(os_timeGet(), sleepTime, applyMergePolicy, mergeHelper);
            d_actionQueueAdd(listener->actionQueue, action);
            d_fellowFree(masterFellow);
        }else {
            d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Fellow '%d' lost before inserting merge action\n",
                        master->systemId);

            /* Free namespace event object */
            d_nameSpaceFree (ns);
        }
        d_free (master);
    /* Late-joining node with initial data joined */
    } else if (event & D_NAMESPACE_DELAYED_INITIAL) {
        applyDelayedAlignment_t* actionData;
        d_fellow adminFellow;
        d_networkAddress fellowAddr;

        listener = d_groupLocalListener(userData);
        admin = d_listenerGetAdmin(d_listener(listener));

        fellowAddr = d_fellowGetAddress(fellow);
        adminFellow = d_adminGetFellow(admin, fellowAddr);
        d_networkAddressFree(fellowAddr);

        actionData = malloc(sizeof(applyDelayedAlignment_t));
        actionData->listener = listener;
        actionData->nameSpace = ns;
        actionData->fellow = adminFellow;

        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;

        /* Post delayed alignment action */
        action = d_actionNew(os_timeGet(), sleepTime, applyDelayedAlignment, actionData);
        d_actionQueueAdd(listener->actionQueue, action);
    }

    return TRUE;
}


static c_bool
notifyFellowEvent(
    c_ulong event,
    d_fellow fellow,
    d_nameSpace ns,
    d_group group,
    c_voidp eventUserData,
    c_voidp userData)
{
    d_groupLocalListener listener;
    d_admin admin;
    d_durability durability;
    c_long length, i;
    d_nameSpace nameSpace;
    d_action masterAction;
    os_time sleepTime;
    d_networkAddress masterAddress, fellowAddress;
    d_serviceState fellowState, myState;
    c_iter nameSpaces, nsCollect;
    struct masterHelper *helper;

    OS_UNUSED_ARG(ns);
    OS_UNUSED_ARG(group);
    OS_UNUSED_ARG(eventUserData);
    listener      = d_groupLocalListener(userData);
    admin         = d_listenerGetAdmin(d_listener(listener));
    durability    = d_adminGetDurability(admin);
    fellowAddress = d_fellowGetAddress(fellow);


    if (event == D_FELLOW_NEW) {
        fellowState   = d_fellowGetState(fellow);
        myState = d_durabilityGetState(durability);

        d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "New fellow '%d' with state %s\n",
                    fellowAddress->systemId,
                    d_fellowStateText(fellowState));

        if(fellowState >= D_STATE_INJECT_PERSISTENT && myState >= D_STATE_INJECT_PERSISTENT){
            /* Detecting a fellow when it already in this state might be a
             * reason to perform a merge.
             */
            d_printTimedEvent(durability, D_LEVEL_INFO,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Potentially I need merging with fellow '%d' with state %s\n",
                                fellowAddress->systemId,
                                d_fellowStateText(fellowState));
        }

    } else if(event == D_FELLOW_REMOVED){
        nameSpaces    = c_iterNew(NULL);

        d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "Fellow '%d' removed, checking whether new master must be determined.\n",
                    fellowAddress->systemId);

        nsCollect = d_adminNameSpaceCollect(admin);
        length        = c_iterLength(nsCollect);

        for(i=0; (i<length) && (d_durabilityMustTerminate(durability) == FALSE); i++) {
            nameSpace = d_nameSpace(c_iterObject(nsCollect, i));
            masterAddress = d_nameSpaceGetMaster(nameSpace);

            if((d_networkAddressEquals(masterAddress, fellowAddress)) ||
               (d_networkAddressIsUnaddressed(masterAddress)))
            {
                d_printTimedEvent(durability, D_LEVEL_INFO,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Need to find a new master for nameSpace '%s'.\n",
                                d_nameSpaceGetName(nameSpace));

                nameSpaces = c_iterInsert(nameSpaces, nameSpace);
            } else {
                d_printTimedEvent(durability, D_LEVEL_INFO,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Master '%d' still available for nameSpace '%s'.\n",
                                masterAddress->systemId, d_nameSpaceGetName(nameSpace));
            }
            d_networkAddressFree(masterAddress);
        }
        length = c_iterLength(nameSpaces);

        /* TODO
         *
         * Remove merge action from sampleChainListener if it exists
         */

        if(length > 0){
            helper = (struct masterHelper*)(os_malloc(sizeof(struct masterHelper)));
            helper->listener = listener;
            helper->nameSpaces = nameSpaces;
            /* Don't fetch fellowstates (only neccesary during initial alignment) */
            helper->fellowStates = NULL;

            sleepTime.tv_sec = 0;
            sleepTime.tv_nsec = 100000000;
            masterAction = d_actionNew(os_timeGet(), sleepTime, determineNewMastersAction, helper);
            d_actionQueueAdd(listener->masterMonitor, masterAction);
        } else {
            c_iterFree(nameSpaces);
        }

        /* Free namespace list */
        d_adminNameSpaceCollectFree(admin, nsCollect);
    }

    d_networkAddressFree(fellowAddress);

    return TRUE;

}
/*************** END NEW IMPL *************************/

void
d_groupLocalListenerInit(
    d_groupLocalListener listener,
    d_subscriber subscriber)
{
    os_time sleepTime;
    os_mutexAttr attr;
    os_threadAttr ta;

    d_listenerInit(d_listener(listener), subscriber, NULL, d_groupLocalListenerDeinit);
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);

    sleepTime.tv_sec                     = 0;
    sleepTime.tv_nsec                    = 100000000;
    listener->lastSequenceNumber         = D_FLOOR_SEQUENCE_NUMBER;
    listener->initialGroupsAdministrated = FALSE;

    os_threadAttrInit(&ta);
    listener->actionQueue                = d_actionQueueNew(
                                                "groupLocalListenerActionQueue",
                                                sleepTime, ta);

    listener->masterMonitor              = d_actionQueueNew(
                                                "masterMonitor",
                                                 sleepTime, ta);
    os_mutexAttrInit(&attr);
    attr.scopeAttr = OS_SCOPE_PRIVATE;
    os_mutexInit(&(listener->masterLock), &attr);

    listener->fellowListener = d_eventListenerNew(
                                        D_FELLOW_NEW | D_FELLOW_REMOVED,
                                        notifyFellowEvent,
                                        listener);

    listener->nameSpaceListener = d_eventListenerNew(
                                        D_NAMESPACE_NEW | D_NAMESPACE_MASTER_CONFLICT | 
                                        D_NAMESPACE_STATE_CONFLICT | D_NAMESPACE_DELAYED_INITIAL,
                                        notifyNameSpaceEvent,
                                        listener);
}

static c_bool
d_groupLocalAction(
    d_action action,
    c_bool terminate)
{
    d_listener listener;
    d_durability durability;
    d_admin admin;
    u_entity service;

    listener = d_listener(d_actionGetArgs(action));

    if(d_objectIsValid(d_object(listener), D_LISTENER)){
        if(terminate == FALSE){
            admin = d_listenerGetAdmin(listener);
            durability = d_adminGetDurability(admin);
            service = u_entity(d_durabilityGetService(durability));

            u_entityAction(service, d_groupLocalListenerHandleNewGroupsLocal, listener);
        }
    }
    return FALSE;
}


static c_bool
d_groupCreatePersistentSnapshotAction(
    d_action action,
    c_bool terminate)
{
    d_durability durability;
    d_admin admin;
    struct createPersistentSnapshotHelper* cps;
    u_result result;

    cps = (struct createPersistentSnapshotHelper*)(d_actionGetArgs(action));

    if(d_objectIsValid(d_object(cps->listener), D_LISTENER))
    {
        if(terminate == FALSE)
        {
            admin = d_listenerGetAdmin(cps->listener);
            durability = d_adminGetDurability(admin);

            result = d_durabilityTakePersistentSnapshot(
                durability,
                cps->partExpr,
                cps->topicExpr,
                cps->uri);
            if(result != U_RESULT_OK)
            {
                OS_REPORT_4(
                    OS_ERROR,
                    "d_groupCreatePersistentSnapshotAction",
                    0,
                    "Creation of persistent snapshot failed with result "
                    "'%d'. Snapshot was requested for partition expression '%s',"
                    " topic expression '%s' and was to be stored at location '%s'.",
                    result,
                    cps->partExpr,
                    cps->topicExpr,
                    cps->uri);
            }

        }
    }
    os_free(cps->partExpr);
    os_free(cps->topicExpr);
    os_free(cps->uri);
    os_free(cps);

    return FALSE;
}

struct deleteHistoricalDataHelper {
    c_time deleteTime;
    c_char* partExpr;
    c_char* topicExpr;
    d_listener listener;
};

static c_bool
d_groupDeleteHistoricalDataAction(
    d_action action,
    c_bool terminate)
{
    d_durability durability;
    d_admin admin;
    d_publisher publisher;
    d_networkAddress unaddressed;
    d_deleteData delData;
    struct deleteHistoricalDataHelper* dhd;

    dhd = (struct deleteHistoricalDataHelper*)(d_actionGetArgs(action));

    if(d_objectIsValid(d_object(dhd->listener), D_LISTENER)){
        if(terminate == FALSE){
            admin = d_listenerGetAdmin(dhd->listener);
            durability = d_adminGetDurability(admin);
            publisher = d_adminGetPublisher(admin);

            d_printTimedEvent(durability, D_LEVEL_FINER,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "Notified fellows of delete_historical_data action.\n");

            unaddressed = d_networkAddressUnaddressed();
            delData = d_deleteDataNew(admin, dhd->deleteTime, dhd->partExpr, dhd->topicExpr);
            d_publisherDeleteDataWrite(publisher, delData, unaddressed);
            d_networkAddressFree(unaddressed);
            d_deleteDataFree(delData);
        }
    }
    os_free(dhd->partExpr);
    os_free(dhd->topicExpr);
    os_free(dhd);

    return FALSE;
}

struct readerRequestHelper {
    d_readerRequest request;
    d_admin admin;
    d_groupLocalListener listener;
};

static c_bool
d_groupLocalReaderRequestAction(
    d_action action,
    c_bool terminate)
{
    c_bool callAgain, fulfilled;
    d_table groups;
    d_group group, localGroup;
    c_char *partition, *topic;
    d_durabilityKind kind;
    d_durability durability;
    v_handle handle;
    struct readerRequestHelper* helper;

    helper = (struct readerRequestHelper*)d_actionGetArgs(action);

    if(!terminate){
        callAgain  = FALSE;
        groups     = d_readerRequestGetGroups(helper->request);
        durability = d_adminGetDurability(helper->admin);
        handle     = d_readerRequestGetHandle(helper->request);

        group = d_tableTake(groups);

        while(group && !callAgain){
            partition  = d_groupGetPartition(group);
            topic      = d_groupGetTopic(group);
            kind       = d_groupGetKind(group);
            localGroup = d_adminGetLocalGroup(helper->admin, partition, topic, kind);

            if(!localGroup){
                callAgain = TRUE;
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "Handling alignment of group %s.%s as part of "\
                    "historicalDataRequest from reader [%d, %d]\n",
                    partition, topic, handle.index, handle.serial);
                d_groupLocalListenerHandleAlignment(helper->listener,
                        localGroup, helper->request);
            }
            os_free(partition);
            os_free(topic);

            d_groupFree(group);

            if(!callAgain){
                group = d_tableTake(groups);
            } else {
                group = NULL;
            }
        }
        d_tableFree(groups);

        if(!callAgain){
            d_printTimedEvent(durability, D_LEVEL_FINER,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Alignment for historicalDataRequest from "\
                                "reader [%d, %d] in progress\n",
                                handle.index, handle.serial);
            fulfilled = d_adminCheckReaderRequestFulfilled(
                                helper->admin, helper->request);

            if(fulfilled){
                d_printTimedEvent(durability, D_LEVEL_FINER,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "historicalDataRequest from reader [%d, %d] fulfilled.\n",
                         handle.index, handle.serial);
            }
        }
    } else {
        callAgain = FALSE;
    }


    if(!callAgain){
        d_readerRequestFree(helper->request);
        os_free(helper);
    }

    return callAgain;
}

/* Lookup namespace */
typedef struct lookupNameSpace_t {
    v_group group;
    d_nameSpace namespace;
}lookupNameSpace_t;
static void lookupNamespace(d_nameSpace ns, void* data) {
    lookupNameSpace_t* userData;

    userData = (lookupNameSpace_t*)data;

    if(!userData->namespace) {
        if(d_nameSpaceIsIn(ns, v_entity(userData->group->partition)->name, v_entity(userData->group->topic)->name)) {
            userData->namespace = ns;
        }
    }
}

/* Set namespace quality to infinite. This behavior supports delayed alignment functionality. */
static void markGroupNamespaceWritten(d_admin admin, v_group group) {
    lookupNameSpace_t walkData;
    d_quality q;
    d_durability durability;

    durability = d_adminGetDurability(admin);

    /* Lookup namespace */
    walkData.group = group;
    walkData.namespace = 0;
    d_adminNameSpaceWalk(admin, lookupNamespace, &walkData);

    /* Check if namespace is found */
    if(!walkData.namespace) {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "Namespace not found for group '%s.%s' not found in administration (cannot update namespace quality).\n",
                v_entity(group->partition)->name, v_entity(group->topic)->name);
        return;
    }

    /* Set quality to infinite when delayed alignment is enabled. */
    if(d_nameSpaceGetDelayedAlignment(walkData.namespace)) {
        q = d_nameSpaceGetInitialQuality(walkData.namespace);
        if((q.seconds != C_TIME_INFINITE.seconds) && (q.nanoseconds != C_TIME_INFINITE.nanoseconds)) {
            /* Create infinite quality */
            q = C_TIME_INFINITE;

            /* Set quality to infinite */
            d_nameSpaceSetInitialQuality(walkData.namespace, q);

            /* Report that quality of namespace is set to infinite */
            d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "Quality of namespace '%s' is set to infinite.\n",
                    d_nameSpaceGetName(walkData.namespace));
        }
    }
}

static void
d_groupLocalListenerHandleReaderRequest(
    d_groupLocalListener listener,
    v_handle source,
    c_char* filter,
    c_char** filterParams,
    c_long filterParamCount,
    struct v_resourcePolicy* resourceLimits,
    c_time minSourceTimestamp,
    c_time maxSourceTimestamp)
{
    d_admin admin;
    d_durability durability;
    d_readerRequest readerRequest;
    os_time sleepTime;
    struct readerRequestHelper* requestHelper;
    d_action action;
    c_bool added;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);

    readerRequest = d_readerRequestNew(admin, source, filter, filterParams,
                    filterParamCount, *resourceLimits,
                    minSourceTimestamp, maxSourceTimestamp);
    added = d_adminAddReaderRequest(admin, readerRequest);

    if(added){
        d_printTimedEvent(durability, D_LEVEL_FINER,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "Received historicalDataRequest from reader [%d, %d]\n",
                source.index, source.serial);

        sleepTime.tv_sec  = 0;
        sleepTime.tv_nsec = 500 * 1000 * 1000; /* 500ms*/
        requestHelper = (struct readerRequestHelper*)
                            os_malloc(sizeof(struct readerRequestHelper));
        requestHelper->admin = admin;
        requestHelper->listener = d_groupLocalListener(listener);
        requestHelper->request = readerRequest;

        action = d_actionNew(os_timeGet(),
                sleepTime, d_groupLocalReaderRequestAction, requestHelper);
        d_actionQueueAdd(listener->actionQueue, action);
    }
    return;
}

c_ulong
d_groupLocalListenerAction(
    u_dispatcher o,
    u_waitsetEvent event,
    c_voidp userData)
{
    d_listener listener;
    d_admin admin;
    d_durability durability;
    d_actionQueue queue;
    d_action action;
    u_waitsetHistoryDeleteEvent hde;
    u_waitsetHistoryRequestEvent hre;
    u_waitsetPersistentSnapshotEvent pse;
    os_time sleepTime;
    struct deleteHistoricalDataHelper* data;
    struct createPersistentSnapshotHelper* snapshotHelper;

    if (o && userData) {
        listener   = d_listener(userData);
        assert(d_listenerIsValid(d_listener(listener), D_GROUP_LOCAL_LISTENER));
        admin      = d_listenerGetAdmin(listener);
        durability = d_adminGetDurability(admin);
        queue      = d_groupLocalListener(listener)->actionQueue;

        if((event->events & V_EVENT_NEW_GROUP) == V_EVENT_NEW_GROUP){
            sleepTime.tv_sec = 1;
            sleepTime.tv_nsec = 0;
            action = d_actionNew(os_timeGet(), sleepTime, d_groupLocalAction, listener);
            d_actionQueueAdd(queue, action);
        }
        if((event->events & V_EVENT_HISTORY_REQUEST) == V_EVENT_HISTORY_REQUEST){
            hre = u_waitsetHistoryRequestEvent(event);

            d_groupLocalListenerHandleReaderRequest(
                    d_groupLocalListener(listener), hre->source, hre->filter,
                    hre->filterParams, hre->filterParamsCount,
                    &(hre->resourceLimits), hre->minSourceTimestamp,
                    hre->maxSourceTimestamp);
        }

        if((event->events & V_EVENT_HISTORY_DELETE) == V_EVENT_HISTORY_DELETE){
            d_printTimedEvent(durability, D_LEVEL_FINER,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "Received local deleteHistoricalData event. Notifying fellows...\n");

            hde   = u_waitsetHistoryDeleteEvent(event);
            data  = (struct deleteHistoricalDataHelper*)(os_malloc(
                                    sizeof(struct deleteHistoricalDataHelper)));

            data->deleteTime.seconds     = hde->deleteTime.seconds;
            data->deleteTime.nanoseconds = hde->deleteTime.nanoseconds;
            data->partExpr               = os_strdup(hde->partitionExpr);
            data->topicExpr              = os_strdup(hde->topicExpr);
            data->listener               = listener;

            sleepTime.tv_sec             = 1;
            sleepTime.tv_nsec            = 0;

            action = d_actionNew(os_timeGet(), sleepTime, d_groupDeleteHistoricalDataAction, data);
            d_actionQueueAdd(queue, action);
        }
        if((event->events & V_EVENT_PERSISTENT_SNAPSHOT) == V_EVENT_PERSISTENT_SNAPSHOT)
        {
            pse = u_waitsetPersistentSnapshotEvent(event);
            d_printTimedEvent(durability, D_LEVEL_FINER,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "Received a request for a persistent snapshot for partition "
                "expression '%s' and topic expression '%s' to be stored at"
                "destination '%s'.\n", pse->partitionExpr, pse->topicExpr, pse->uri);
            snapshotHelper  = (struct createPersistentSnapshotHelper*)(os_malloc(
                                    sizeof(struct createPersistentSnapshotHelper)));
            snapshotHelper->partExpr = os_strdup(pse->partitionExpr);
            snapshotHelper->topicExpr = os_strdup(pse->topicExpr);
            snapshotHelper->uri = os_strdup(pse->uri);
            snapshotHelper->listener = listener;
            sleepTime.tv_sec = 1;
            sleepTime.tv_nsec = 0;

            action = d_actionNew(os_timeGet(), sleepTime, d_groupCreatePersistentSnapshotAction, snapshotHelper);
            d_actionQueueAdd(queue, action);
        }
        if((event->events & V_EVENT_CONNECT_WRITER) == V_EVENT_CONNECT_WRITER) {
            admin = d_listenerGetAdmin(listener);

            /* Set namespace quality to infinite */
            markGroupNamespaceWritten(admin, u_waitsetConnectWriterEvent(event)->group);
        }
    }

    return event->events;
}

c_bool
d_groupLocalListenerStart(
    d_groupLocalListener listener)
{
    c_bool result;
    u_dispatcher dispatcher;
    u_result ur;
    c_ulong mask;
    d_durability durability;

    c_bool wsResult;
    d_waitset waitset;
    d_admin admin;
    d_subscriber subscriber;
    d_waitsetAction action;
    d_store store;
    os_threadAttr attr;

    result = FALSE;

    assert(d_listenerIsValid(d_listener(listener), D_GROUP_LOCAL_LISTENER));

    /* Setup listener for CONNECT_WRITER, NEW_GROUP, HISTORY_DELETE, HISTORY_REQUEST and PERSISTENT_SNAPSHOT events */
    if(listener){
        d_listenerLock(d_listener(listener));
        durability  = d_adminGetDurability(d_listenerGetAdmin(d_listener(listener)));
        dispatcher  = u_dispatcher( d_durabilityGetService(durability));
        action      = d_groupLocalListenerAction; /* callback function */

        if(d_listener(listener)->attached == FALSE){
            ur = u_dispatcherGetEventMask(dispatcher, &mask);

            if(ur == U_RESULT_OK){
                ur = u_dispatcherSetEventMask(dispatcher,
                        mask | V_EVENT_CONNECT_WRITER | V_EVENT_NEW_GROUP | V_EVENT_HISTORY_DELETE | V_EVENT_HISTORY_REQUEST | V_EVENT_PERSISTENT_SNAPSHOT);

                if(ur == U_RESULT_OK){
                    admin      = d_listenerGetAdmin(d_listener(listener));
                    subscriber = d_adminGetSubscriber(admin);
                    store      = d_subscriberGetPersistentStore(subscriber);
                    waitset    = d_subscriberGetWaitset(subscriber);

                    /* Create and attach waitset for listener */
                    os_threadAttrInit(&attr);
                    listener->waitsetData = d_waitsetEntityNew(
                                    "groupLocalListener",
                                    dispatcher, action,
                                    V_EVENT_CONNECT_WRITER | V_EVENT_NEW_GROUP | V_EVENT_HISTORY_DELETE | V_EVENT_HISTORY_REQUEST | V_EVENT_PERSISTENT_SNAPSHOT,
                                    attr, listener);
                    wsResult = d_waitsetAttach(waitset, listener->waitsetData);

                    if(wsResult == TRUE) {
                        ur = U_RESULT_OK;
                    } else {
                        ur = U_RESULT_ILL_PARAM;
                    }

                    if(listener->initialGroupsAdministrated == FALSE){
                        d_durabilitySetState(durability, D_STATE_DISCOVER_PERSISTENT_SOURCE);
                        os_mutexLock(&listener->masterLock);
                        d_adminAddListener(admin, listener->fellowListener);
                        d_adminAddListener(admin, listener->nameSpaceListener);

                        initMasters(listener);

                        if(store != NULL){
                            initPersistentData(listener);
                            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_GROUP_LOCAL_LISTENER, "Persistency has been enabled...\n");
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_GROUP_LOCAL_LISTENER, "Persistency has not been enabled...\n");
                        }
                        os_mutexUnlock(&listener->masterLock);
                        d_durabilitySetState(durability, D_STATE_DISCOVER_LOCAL_GROUPS);
                        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_GROUP_LOCAL_LISTENER, "Initializing local groups...\n");

                        if(d_durabilityMustTerminate(durability) == FALSE){
                            u_entityAction(u_entity(dispatcher), d_groupLocalListenerInitLocalGroups, listener);
                        }
                        d_durabilitySetState(durability, D_STATE_FETCH_INITIAL);
                        listener->initialGroupsAdministrated = TRUE;
                        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_GROUP_LOCAL_LISTENER, "Local groups initialized.\n");
                    }
                    if(ur == U_RESULT_OK){
                        d_listener(listener)->attached = TRUE;
                        result = TRUE;
                        d_listenerUnlock(d_listener(listener));
                        u_dispatcherNotify(dispatcher);
                    } else {
                        d_listenerUnlock(d_listener(listener));
                    }
                } else {
                    d_listenerUnlock(d_listener(listener));
                }
            } else {
                d_listenerUnlock(d_listener(listener));
            }
        } else {
            d_listenerUnlock(d_listener(listener));
            result = TRUE;
        }
    }
    return result;
}

c_bool
d_groupLocalListenerStop(
    d_groupLocalListener listener)
{
    c_bool result;
    u_result ur;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_listenerIsValid(d_listener(listener), D_GROUP_LOCAL_LISTENER));

    result = FALSE;

    if(listener){
        d_listenerLock(d_listener(listener));

        if(d_listener(listener)->attached == TRUE){
            admin = d_listenerGetAdmin(d_listener(listener));
            d_adminRemoveListener(admin, listener->fellowListener);
            d_eventListenerFree(listener->fellowListener);
            listener->fellowListener = NULL;
            d_adminRemoveListener(admin, listener->nameSpaceListener);
            d_eventListenerFree(listener->nameSpaceListener);
            listener->nameSpaceListener = NULL;
            subscriber = d_adminGetSubscriber(admin);
            waitset = d_subscriberGetWaitset(subscriber);
            result = d_waitsetDetach(waitset, listener->waitsetData);

            if(result == TRUE) {
                d_waitsetEntityFree(listener->waitsetData);
                ur = U_RESULT_OK;
            } else {
                ur = U_RESULT_ILL_PARAM;
            }
            if(ur == U_RESULT_OK){
                d_listener(listener)->attached = FALSE;
                result = TRUE;
            }
        } else {
            result = TRUE;
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}

void
d_groupLocalListenerInitLocalGroups(
    v_entity e,
    c_voidp args)
{
    v_serviceFillNewGroups(v_service(e));
    d_groupLocalListenerHandleNewGroupsLocal(e, args);
}

c_ulong
d_groupLocalListenerNewGroupLocalAction(
    u_dispatcher o,
    c_ulong event,
    c_voidp userData)
{
    d_admin admin;
    d_durability durability;
    d_listener listener;

    if (o && (event & V_EVENT_NEW_GROUP)) {
        if (userData) {
            listener   = d_listener(userData);
            assert(d_listenerIsValid(d_listener(listener), D_GROUP_LOCAL_LISTENER));
            admin = d_listenerGetAdmin(listener);
            durability = d_adminGetDurability(admin);

            u_entityAction(u_entity(d_durabilityGetService(durability)), d_groupLocalListenerHandleNewGroupsLocal, userData);

        }
    }
    return V_EVENT_NEW_GROUP;
}

void
d_groupLocalListenerHandleNewGroupsLocal(
    v_entity entity,
    c_voidp args)
{
    d_listener           listener;
    d_groupLocalListener groupListener;
    v_service            kservice;
    c_iter               groups;
    d_admin              admin;
    v_group              group, group2;
    d_group              dgroup;
    d_durabilityKind     kind;
    d_durability         durability;
    d_quality            quality;
    v_durabilityKind     vkind;
    c_bool               added, attached, groupAlreadyKnown;
    v_topicQos           qos;
    d_adminStatisticsInfo info;

    listener      = d_listener(args);
    groupListener = d_groupLocalListener(args);
    kservice      = v_service(entity);
    admin         = d_listenerGetAdmin(listener);
    durability    = d_adminGetDurability(admin);
    groups        = v_serviceTakeNewGroups(kservice);

    if (groups) {
        group = v_group(c_iterTakeFirst(groups));

        while (group) {
            if(d_durabilityMustTerminate(durability) == FALSE){
                dgroup = NULL;
                qos = v_topicQosRef(group->topic);
                vkind = qos->durability.kind;
                d_reportLocalGroup(durability, D_THREAD_GROUP_LOCAL_LISTENER, group);

                /* Check durability kind. Only transient, transient local
                 * and persistent groups require actions
                 */
                if(vkind == V_DURABILITY_VOLATILE){
                    d_printTimedEvent(durability, D_LEVEL_WARNING,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Ignoring group %s.%s.\n",
                                v_partitionName(v_groupPartition(group)),
                                v_topicName(v_groupTopic(group)));
                    /*update statistics*/
                    info = d_adminStatisticsInfoNew();
                    info->kind = D_ADMIN_STATISTICS_GROUP;
                    info->groupsKnownVolatileDif += 1;
                    info->groupsIgnoredVolatileDif +=1;
                    d_adminUpdateStatistics(admin, info);
                    d_adminStatisticsInfoFree(info);
                }
                if ((vkind == V_DURABILITY_TRANSIENT) ||
                    (vkind == V_DURABILITY_PERSISTENT) ||
                    (vkind == V_DURABILITY_TRANSIENT_LOCAL)) {
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                            D_THREAD_GROUP_LOCAL_LISTENER,
                            "Wait for services to attach.\n");
                    attached = d_durabilityWaitForAttachToGroup(durability, group);
                    d_printTimedEvent(durability, D_LEVEL_FINE,
                            D_THREAD_GROUP_LOCAL_LISTENER,
                            "Administrating group %s.%s.\n",
                            v_partitionName(v_groupPartition(group)),
                            v_topicName(v_groupTopic(group)));

                    if( (vkind == V_DURABILITY_TRANSIENT) ||
                        (vkind == V_DURABILITY_TRANSIENT_LOCAL)){
                        kind = D_DURABILITY_TRANSIENT;
                    } else {
                        kind = D_DURABILITY_PERSISTENT;
                    }
                    /* New groups may be notified multiple times by the kernel.
                     * Therefore no alignment should be done if group is
                     * already known in the durability administration.
                     */
                    groupAlreadyKnown = TRUE;
                    dgroup = d_adminGetLocalGroup(
                                 admin,
                                 v_partitionName(v_groupPartition(group)),
                                 v_topicName(v_groupTopic(group)),
                                 kind);

                    if(!dgroup){
                        quality.seconds = 0;
                        quality.nanoseconds = 0;
                        dgroup = d_groupNew(v_partitionName(v_groupPartition(group)),
                                            v_topicName(v_groupTopic(group)),
                                            kind, D_GROUP_INCOMPLETE, quality);
                        d_groupSetKernelGroup(dgroup, group);

                        if(!attached){
                           d_groupSetPrivate(dgroup, TRUE);
                        }
                        added = d_adminAddLocalGroup(admin, dgroup);

                        if(added == FALSE){
                            d_groupFree(dgroup);
                            dgroup = d_adminGetLocalGroup(
                                         admin,
                                         v_partitionName(v_groupPartition(group)),
                                         v_topicName(v_groupTopic(group)),
                                         kind);
                            if(!attached){
                                d_groupSetPrivate(dgroup, TRUE);
                            }
                        } else {
                            groupAlreadyKnown = FALSE;
                        }
                    } else if(!attached){
                        d_groupSetPrivate(dgroup, TRUE);
                    }

                    if((d_groupGetCompleteness(dgroup) != D_GROUP_COMPLETE) && (!groupAlreadyKnown)) {
                        group2 = d_groupGetKernelGroup(dgroup);

                        if(group2){
                            c_free(group2);
                        } else {
                            d_groupSetKernelGroup(dgroup, group);
                        }
                        if(d_durabilityMustTerminate(durability) == FALSE){
                            d_groupLocalListenerHandleAlignment(groupListener, dgroup, NULL);
                        }
                    } else if(groupAlreadyKnown){
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Group %s.%s already known in admin.\n",
                                v_partitionName(v_groupPartition(group)),
                                v_topicName(v_groupTopic(group)));
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                            D_THREAD_GROUP_LOCAL_LISTENER,
                            "Group %s.%s already complete.\n",
                            v_partitionName(v_groupPartition(group)),
                            v_topicName(v_groupTopic(group)));
                    }
                }
                c_free(group);
                group = (v_group)c_iterTakeFirst(groups);
            } else {
                c_free(group);
                group = (v_group)c_iterTakeFirst(groups);

                while(group){
                    c_free(group);
                    group = (v_group)c_iterTakeFirst(groups);
                }
            }
        }
        c_iterFree(groups);
    }
}

void
d_groupLocalListenerHandleAlignment(
    d_groupLocalListener listener,
    d_group dgroup,
    d_readerRequest readerRequest)
{
    d_timestamp         stamp, zeroTime, networkAttachTime;
    d_sampleRequest     request;
    d_admin             admin;
    d_durability        durability;
    d_subscriber        subscriber;
    d_store             store;
    d_completeness      completeness;
    d_configuration     config;
    d_durabilityKind    dkind, nsdkind;
    c_bool              timeRangeActive, requestRemote, inject;
    d_group             localGroup;
    c_char              *partition, *topic;
    d_nameSpace         nameSpace;
    d_alignmentKind     akind;
    d_storeResult       result;
    d_chain             chain;
    d_adminStatisticsInfo info;
    d_nameSpace          groupNameSpace;

    assert(d_listenerIsValid(d_listener(listener), D_GROUP_LOCAL_LISTENER));

    admin      = d_listenerGetAdmin(d_listener(listener));
    subscriber = d_adminGetSubscriber(admin);
    store      = d_subscriberGetPersistentStore(subscriber);
    durability = d_adminGetDurability(admin);
    config     = d_durabilityGetConfiguration(durability);
    partition  = d_groupGetPartition(dgroup);
    topic      = d_groupGetTopic(dgroup);
    localGroup = d_adminGetLocalGroup(admin, partition, topic, d_groupGetKind(dgroup));

    if(localGroup){
        completeness = d_groupGetCompleteness(localGroup);
        dkind        = d_groupGetKind(localGroup);

        if(readerRequest){
            requestRemote = FALSE;
            nameSpace = d_adminGetNameSpaceForGroup(admin, partition, topic);

            if(nameSpace){
                akind = d_nameSpaceGetAlignmentKind(nameSpace);

                if((akind == D_ALIGNEE_ON_REQUEST) && (completeness != D_GROUP_COMPLETE)){
                    if(dkind == D_DURABILITY_PERSISTENT){
                        inject = d_nameSpaceMasterIsMe(nameSpace, admin);

                        if(inject || d_groupIsPrivate(localGroup)){
                            result = d_storeMessagesInject(store, localGroup);
                            d_readerRequestRemoveGroup(readerRequest, localGroup);
                        } else {
                            requestRemote = TRUE;
                        }
                    } else if(!d_groupIsPrivate(localGroup)){
                        requestRemote = TRUE;
                    } else {
                        /* All data available, so we're done for this group*/
                    }

                } else if(completeness == D_GROUP_COMPLETE){
                    d_readerRequestRemoveGroup(readerRequest, localGroup);
                } else {
                    /* Simply wait for group completeness later on */
                }
            } else {
                /*Not in nameSpace so ignore request for this group*/
                d_readerRequestRemoveGroup(readerRequest, localGroup);

                d_printTimedEvent(durability, D_LEVEL_WARNING,
                    D_THREAD_GROUP_LOCAL_LISTENER,
                    "Received a historicalDataRequest from a reader for group %s.%s, "\
                    "but that is not in the nameSpace and therefore ignored.\n",
                    partition, topic);

            }

            if(requestRemote == TRUE){
                stamp = v_timeGet();

                if(config->timeAlignment){
                    networkAttachTime    = stamp;
                } else {
                    networkAttachTime = C_TIME_INFINITE;
                }
                timeRangeActive      = FALSE;
                zeroTime.seconds     = 0;
                zeroTime.nanoseconds = 0;

                request = d_sampleRequestNew(
                                admin, partition, topic,
                                dkind, stamp, timeRangeActive,
                                zeroTime, networkAttachTime);

                d_sampleRequestSetCondition(request, readerRequest);
                chain = d_chainNew(admin, request);
                d_readerRequestAddChain(readerRequest, chain);
                d_readerRequestRemoveGroup(readerRequest, localGroup);

                d_sampleChainListenerInsertRequest(
                            d_groupLocalListener(listener)->sampleChainListener,
                            chain, TRUE);

            }
        } else if(d_adminGroupInActiveAligneeNS(admin, partition, topic) == TRUE){
            if(completeness != D_GROUP_COMPLETE){
                if(dkind == D_DURABILITY_PERSISTENT){
                    nameSpace = d_adminGetNameSpaceForGroup(admin, partition, topic);
                    assert(nameSpace);
                    akind     = d_nameSpaceGetAlignmentKind(nameSpace);
                    inject    = d_nameSpaceMasterIsMe(nameSpace, admin);

                    if(inject == TRUE){
                        nsdkind = d_nameSpaceGetDurabilityKind(nameSpace);

                        if( (nsdkind == D_DURABILITY_ALL) ||
                            (nsdkind == D_DURABILITY_PERSISTENT)
                        ){
                            d_storeGroupStore(store, localGroup);
                        }
                        if(akind == D_ALIGNEE_LAZY){
                            result = d_storeMessagesInject(store, localGroup);

                            if(result == D_STORE_RESULT_OK){
                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                    "Data for group %s.%s injected from disk. Group is complete now (1).\n",
                                    partition, topic);
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Injecting data from disk for group %s.%s failed (1).\n",
                                partition, topic);
                            }
                        } else if(akind == D_ALIGNEE_ON_REQUEST){
                            result = d_storeMessagesInject(store, localGroup);

                            if(result == D_STORE_RESULT_OK){
                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                    "Data for group %s.%s injected from disk. Group is complete now (2).\n",
                                    partition, topic);
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Injecting data from disk for group %s.%s failed (1).\n",
                                partition, topic);
                            }
                        } else if(akind == D_ALIGNEE_INITIAL){
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Persistent group %s.%s complete, because I am persistent source and already injected data.\n",
                                partition, topic);
                            result = D_STORE_RESULT_OK;
                        } else {
                            assert(FALSE);
                        }
                        if(d_groupIsPrivate(localGroup)){
                            d_groupSetComplete(localGroup);
                            requestRemote = FALSE;
                        } else {
                            requestRemote = TRUE;
                        }
                    } else if(d_groupIsPrivate(localGroup)){
                        /* I am not the master, but this is a persistent group
                         * that exists on this node only, so inject data for
                         * group.
                         */
                        result = d_storeMessagesInject(store, localGroup);

                        if(result == D_STORE_RESULT_OK){
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Data for local group %s.%s injected from disk. Group is complete now (3).\n",
                                partition, topic);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Injecting data from disk for local group %s.%s failed (3).\n",
                                partition, topic);
                        }
                        d_groupSetComplete(localGroup);
                        requestRemote = FALSE;
                    } else {
                        /*Only request if complete.*/
                        d_storeGroupStore(store, localGroup);
                        requestRemote = TRUE;
                    }
                } else if(d_groupIsPrivate(localGroup)){
                    /* This group is private and transient so it is complete */
                    d_groupSetComplete(localGroup);
                    d_printTimedEvent(durability, D_LEVEL_FINE,
                        D_THREAD_GROUP_LOCAL_LISTENER,
                        "Transient group %s.%s complete, because it is local.\n",
                        partition, topic);
                    requestRemote = FALSE;
                } else {
                    requestRemote = TRUE;
                }

                if(requestRemote == TRUE){
                    stamp = v_timeGet();

                    if(config->timeAlignment){
                        networkAttachTime    = stamp;
                    } else {
                        networkAttachTime = C_TIME_INFINITE;
                    }
                    timeRangeActive      = FALSE;
                    zeroTime.seconds     = 0;
                    zeroTime.nanoseconds = 0;

                    if((dkind == D_DURABILITY_PERSISTENT) && store){
                        result = d_storeGroupStore(store, localGroup);

                        if(result == D_STORE_RESULT_OK){
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Persistent group %s.%s stored on disk.\n",
                                partition, topic);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Storing persistent group %s.%s on disk failed (error code: %d).\n",
                                partition, topic, result);
                        }
                    }

                    request = d_sampleRequestNew(admin, partition, topic,
                            dkind, stamp, timeRangeActive, zeroTime, networkAttachTime);
                    chain   = d_chainNew(admin, request);
                    d_sampleChainListenerInsertRequest(
                                d_groupLocalListener(listener)->sampleChainListener,
                                chain, TRUE);
                }
            }
        } else if(d_adminGroupInAligneeNS(admin, partition, topic) == TRUE){
            d_sampleChainListenerReportGroup(listener->sampleChainListener, localGroup);
            /*For those topics in a on_request namespace, they will be marked unaligned*/
            groupNameSpace = d_adminGetNameSpaceForGroup(admin,
                                                         d_groupGetPartition(localGroup),
                                                         d_groupGetTopic(localGroup));

            if((d_nameSpaceGetAlignmentKind(groupNameSpace) == D_ALIGNEE_ON_REQUEST)
                && (d_groupIsBuiltinGroup(localGroup) != TRUE)) {

                d_groupSetUnaligned(localGroup);

                d_printTimedEvent(durability, D_LEVEL_FINE,
                D_THREAD_GROUP_LOCAL_LISTENER,
                "Group %s.%s is transient group in the namespace with " \
                "on_request policy will be marked unaligned.\n",
                partition, topic);
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINE,
                            D_THREAD_GROUP_LOCAL_LISTENER,
                            "Group %s.%s not in alignee namespace, so no " \
                            "alignment action taken.\n",
                            partition, topic);
            d_groupSetUnaligned(localGroup);

            /*update statistics*/
            info = d_adminStatisticsInfoNew();
            info->kind = D_ADMIN_STATISTICS_GROUP;

            switch(dkind){
            case D_DURABILITY_VOLATILE:
                info->groupsIncompleteVolatileDif -= 1;
                info->groupsIgnoredVolatileDif +=1;
                break;
            case D_DURABILITY_TRANSIENT_LOCAL:
            case D_DURABILITY_TRANSIENT:
                info->groupsIncompleteTransientDif -= 1;
                info->groupsIgnoredTransientDif +=1;
                break;
            case D_DURABILITY_PERSISTENT:
                info->groupsIncompletePersistentDif -= 1;
                info->groupsIgnoredPersistentDif +=1;
                break;
            default:
                break;
            }

            d_adminUpdateStatistics(admin, info);
            d_adminStatisticsInfoFree(info);
        }
    }
    os_free(partition);
    os_free(topic);
}
