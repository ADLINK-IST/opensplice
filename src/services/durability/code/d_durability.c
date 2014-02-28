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
#include "d_durability.h"
#include "d__durability.h"
#include "d_configuration.h"
#include "d_admin.h"
#include "d_publisher.h"
#include "d_subscriber.h"
#include "d_table.h"
#include "d_misc.h"
#include "d_status.h"
#include "d_group.h"
#include "d_fellow.h"
#include "d_actionQueue.h"
#include "d_groupsRequest.h"
#include "d_nameSpacesRequest.h"
#include "d_sampleChainListener.h"
#include "d_nameSpace.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "u_entity.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_time.h"
#include "v_entity.h"
#include "os_report.h"
#include "os_heap.h"
#include "os_time.h"
#include "d_store.h"
#include "os.h"
#include "c_iterator.h"

#ifdef INTEGRITY
#include "include/os_getRSObjects.h"
#endif
/**
 * TODO: \todo
 * 1. The waitForAttachToGroup only supports one service to wait for. This
 *    must be a set of services.
 */

static c_bool
checkNameSpaces(
    d_fellow fellow,
    c_voidp args)
{
    c_bool* ready;
    d_communicationState state;

    ready = (c_bool*)args;
    state = d_fellowGetCommunicationState(fellow);

    if(state == D_COMMUNICATION_STATE_UNKNOWN){
        *ready = FALSE;
    } else {
        *ready = TRUE;
    }
    return *ready;
}

static c_bool
checkFellowNameSpacesKnown(
    d_fellow fellow,
    c_voidp args)
{
    c_bool* known;

    known = (c_bool*)args;
    *known = d_fellowAreNameSpacesComplete(fellow);

    return *known;

}

c_bool
d_durabilityArgumentsProcessing(
    int     argc,
    char *  argv[],
    c_char ** uri,
    c_char ** serviceName )
{
    c_bool result = FALSE;

    *uri = NULL;
    *serviceName = NULL;

    if(argc == 2){
        *serviceName = os_strdup(argv[1]);
        result = TRUE;
        OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                "No URI supplied, using default settings.");
    } else if(argc == 3){
        *serviceName = os_strdup(argv[1]);
        *uri = os_strdup(argv[2]);
        result = TRUE;
    } else {
        OS_REPORT_1(OS_ERROR, D_CONTEXT, 0,
                "Arguments are missing. Usage: %s <serviceName> [<uri>]",
                argv[0]);
    }
    return result;
}

void
d_durabilityWatchSpliceDaemon(
    v_serviceStateKind spliceDaemonState,
    c_voidp            usrData )
{
    d_durability durability;
    d_status status;
    d_publisher publisher;
    d_networkAddress addr;

    switch(spliceDaemonState){
        case STATE_TERMINATING:
        case STATE_TERMINATED:
        case STATE_DIED:
            durability = (d_durability)usrData;
            assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);

            if(durability){
                if(durability->admin){
                    addr = d_networkAddressUnaddressed();
                    publisher = d_adminGetPublisher(durability->admin);
                    status = d_statusNew(durability->admin);
                    d_durabilitySetState(durability, D_STATE_TERMINATING);
                    d_publisherStatusWrite(publisher, status, addr);
                    d_statusFree(status);
                    d_networkAddressFree(addr);
                }
                d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_SPLICED_LISTENER,
                    "Splicedaemon is stopping, terminating durability now...\n");
                durability->splicedRunning = FALSE;
            }
            break;
        default:
            break;
    }
}

c_voidp
d_durabilityUpdateLease(
    c_voidp args)
{
    d_durability durability;
    os_time sleepTime, lag, before, after, accepted;
    v_duration expiryTime;
    durability = d_durability(args);

    if(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE){
        sleepTime = durability->configuration->livelinessUpdateInterval;

        accepted.tv_sec = durability->configuration->livelinessExpiryTime.seconds;
        accepted.tv_nsec = durability->configuration->livelinessExpiryTime.nanoseconds;

        while(durability->splicedRunning){
            before = os_timeGet();
            u_serviceRenewLease(durability->service,
                            durability->configuration->livelinessExpiryTime);
            os_nanoSleep(sleepTime);
            after = os_timeGet();

            lag = os_timeSub(after, before);

            if(os_timeCompare(lag, accepted) == OS_MORE){
                OS_REPORT_4(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                    "Durability failed to renew its lease within "
                    "the configured lease expiry-time (%d.%09d). "
                    "The lease renewal took %d.%09d s.",
                    accepted.tv_sec, accepted.tv_nsec,
                    lag.tv_sec, lag.tv_nsec);
            }
        }
        expiryTime.seconds = 20;
        expiryTime.nanoseconds = 0;
        u_serviceRenewLease(durability->service, expiryTime);
    }
    return NULL;
}

c_voidp
d_durabilityNotifyStatus(
    c_voidp args)
{
    d_durability durability;
    d_status status;
    d_publisher publisher;
    d_admin admin;
    d_networkAddress addressee;

    durability = d_durability(args);
    admin = durability->admin;
    publisher = d_adminGetPublisher(admin);

    if(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE){
        addressee = d_networkAddressUnaddressed();
        status = d_statusNew(admin);

        while(durability->splicedRunning == TRUE){
            d_message(status)->senderState = d_durabilityGetState(durability);
            d_publisherStatusWrite(publisher, status, addressee);
            os_nanoSleep(durability->configuration->heartbeatUpdateInterval);
        }
        d_statusFree(status);
        d_networkAddressFree(addressee);
    }
    return NULL;
}

static void
durabilityRegisterNsWalk(
        void* o,
        c_iterActionArg arg)
{
    d_durability durability;
    d_admin admin;
    d_nameSpace ns;

    durability = d_durability(arg);
    admin = durability->admin;
    ns = d_nameSpace(o);

    /* Add namespace to runtime administration */
    d_adminAddNameSpace (admin, ns);
}

static void
d_durabilityRegisterNameSpaces(
        d_durability durability)
{
    d_configuration config;
    c_iter nameSpaces;

    config = durability->configuration;
    nameSpaces = config->nameSpaces;

    c_iterWalk (nameSpaces, durabilityRegisterNsWalk, durability);
}

void
ospl_durabilityAtExit(
    void)
{
    u_userExit();
}

d_durability
d_durabilityNew(
    const c_char* uri,
    const c_char* serviceName,
    c_long domainId)
{
    u_result result;
    d_durability durability;

    durability = d_durability(NULL);
    result = u_userInitialise();

    if(result == U_RESULT_OK){
        durability = d_durability(os_malloc(C_SIZEOF(d_durability)));
        d_objectInit(d_object(durability), D_DURABILITY, d_durabilityDeinit);

        durability->state          = D_STATE_INIT;
        durability->service        = NULL;
        durability->splicedRunning = TRUE;
        durability->configuration  = NULL;
        durability->admin          = NULL;
        durability->serviceManager = NULL;
        durability->leaseThread    = OS_THREAD_ID_NONE;
        durability->statusThread   = OS_THREAD_ID_NONE;

        d_durabilitySetState(durability, D_STATE_INIT);

        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Creating user service...\n");
        durability->service = u_serviceNew(uri, 30, serviceName, NULL, U_SERVICE_DURABILITY, NULL);

        if(durability->service != NULL){
            u_serviceChangeState(durability->service, STATE_INITIALISING);
            durability->serviceManager  = u_serviceManagerNew(u_participant(durability->service));

            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Loading durability module...\n");
            result = u_entityWriteAction(u_entity(durability->service), d_durabilityLoadModule, NULL);

            if(result == U_RESULT_OK){
                d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Reading configuration...\n");
                if (!(durability->configuration = d_configurationNew(durability, serviceName, domainId))) {
                    /* Error occured in loading configuration. */
                    result = U_RESULT_UNDEFINED;
                }else {
                    d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting splicedaemon listener...\n");
                    result = u_serviceWatchSpliceDaemon(durability->service, d_durabilityWatchSpliceDaemon, durability);
                }
            }

            if((result == U_RESULT_OK) && durability->configuration){
                durability->admin = d_adminNew(durability);

                if(durability->admin){
                    /* Register configured namespaces with admin */
                    d_durabilityRegisterNameSpaces(durability);

                    /* Now ready to initialize protocol subscriber */
                    d_adminInitSubscriber (durability->admin);

                    /* Initialize listeners */
                    d_durabilityInit(durability);
                } else {
                    d_durabilityFree(durability);
                    durability = NULL;
                }
            } else {
                d_durabilityFree(durability);
                durability = NULL;
            }
        } else {
            d_durabilityFree(durability);
            durability = NULL;
        }
    }
    return durability;
}

void
d_durabilityInit(
    d_durability durability)
{
    d_subscriber subscriber;
    c_bool result;
    os_result osr;

    subscriber = d_adminGetSubscriber(durability->admin);

    d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting lease updater...\n");
    osr = os_threadCreate(&(durability->leaseThread), "leaseThread",
                    &(durability->configuration->livelinessScheduling),
                    d_durabilityUpdateLease, durability);

    if(osr != os_resultSuccess){
        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_MAIN, "Failed to start lease updater\n");
        OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Failed to start lease update thread.");
    }


    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing nameSpacesListener...\n");
        d_subscriberInitNameSpacesListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing nameSpacesRequestListener...\n");
        d_subscriberInitNameSpacesRequestListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing statusListener...\n");
        d_subscriberInitStatusListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing groupsRequestListener...\n");
        d_subscriberInitGroupsRequestListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing persistentDataListener...\n");
        d_subscriberInitPersistentDataListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing deleteDataListener...\n");
        d_subscriberInitDeleteDataListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing groupRemoteListener...\n");
        d_subscriberInitGroupRemoteListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing sampleRequestListener...\n");
        d_subscriberInitSampleRequestListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing sampleChainListener...\n");
        d_subscriberInitSampleChainListener(subscriber);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Initializing groupLocalListener...\n");
        d_subscriberInitGroupLocalListener(subscriber);
    }


    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting groupsRequestListener...\n");
        result = d_subscriberSetGroupsRequestListenerEnabled(subscriber, TRUE);
        assert(result == TRUE);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting nameSpacesListener...\n");
        result = d_subscriberSetNameSpacesListenerEnabled(subscriber, TRUE);
        assert(result == TRUE);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting nameSpacesRequestListener...\n");
        result = d_subscriberSetNameSpacesRequestListenerEnabled(subscriber, TRUE);
        assert(result == TRUE);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting statusListener...\n");
        result = d_subscriberSetStatusListenerEnabled(subscriber, TRUE);
        assert(result == TRUE);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting persistentDataListener...\n");
        result = d_subscriberSetPersistentDataListenerEnabled(subscriber, TRUE);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting groupRemoteListener...\n");
        result = d_subscriberSetGroupRemoteListenerEnabled(subscriber, TRUE);
        assert(result == TRUE);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting sampleChainListener...\n");
        result = d_subscriberSetSampleChainListenerEnabled(subscriber, TRUE);
        assert(result == TRUE);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER,  D_THREAD_MAIN, "Starting sampleRequestListener\n");
        result = d_subscriberSetSampleRequestListenerEnabled(subscriber, TRUE);
        assert(result == TRUE);
    }
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting deleteDataListener...\n");
        result = d_subscriberSetDeleteDataListenerEnabled(subscriber, TRUE);
        assert(result == TRUE);
    }


    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting notification of own state...\n");

        osr = os_threadCreate(&(durability->statusThread), "statusThread",
                        &(durability->configuration->heartbeatScheduling),
                        d_durabilityNotifyStatus, durability);

        if(osr != os_resultSuccess){
            d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_MAIN,
                    "Failed to start notifying status\n");
            OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Failed to start notifying status.");
        }
    }
}

d_connectivity
d_durabilityDetermineConnectivity(
    d_durability durability)
{
    d_nameSpacesRequest nsRequest;
    d_networkAddress addr;
    c_bool nsComplete;
    os_time sleepTime, stopTime;
    d_publisher publisher;
    c_ulong incompatibleCount;
    d_connectivity connectivity;
    d_networkAddress myAddr;

    d_durabilitySetState(durability, D_STATE_DISCOVER_FELLOWS_GROUPS);

    if(durability->splicedRunning == TRUE){
	    myAddr = d_adminGetMyAddress(durability->admin);
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "My address is: '%u'\n", myAddr->systemId);
        OS_REPORT_1(OS_INFO, D_SERVICE_NAME, 0, "The durability service can by identified by ID: %u", myAddr->systemId);
        d_networkAddressFree(myAddr);

        addr = d_networkAddressUnaddressed();
        publisher  = d_adminGetPublisher(durability->admin);
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Requesting fellow namespaces...\n");
        nsRequest = d_nameSpacesRequestNew(durability->admin);
        d_publisherNameSpacesRequestWrite(publisher, nsRequest, addr);
        d_nameSpacesRequestFree(nsRequest);
        d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN, "Waiting some time to allow fellows to report...\n");

        stopTime  = os_timeGet();
        stopTime  = os_timeAdd(stopTime, durability->configuration->timingInitialWaitPeriod);

        /* 200 ms */
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 200000000;

        while((durability->splicedRunning == TRUE) &&
              (os_timeCompare(os_timeGet(), stopTime) == OS_LESS))
        {
            os_nanoSleep(sleepTime);
        }
        d_networkAddressFree(addr);
        d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN,
            "I now know %u fellows. Wait for fellow namespaces to get complete...\n",
            d_adminGetFellowCount(durability->admin));
        nsComplete = TRUE;
        d_adminFellowWalk(durability->admin, checkNameSpaces, &nsComplete);

        while((nsComplete == FALSE) && (durability->splicedRunning == TRUE)){
            os_nanoSleep(sleepTime);
            d_adminFellowWalk(durability->admin, checkNameSpaces, &nsComplete);

            if(d_adminGetFellowCount(durability->admin) == 0){
                nsComplete = TRUE;
            }
        }
        incompatibleCount = d_adminGetIncompatibleStateCount(durability->admin);

        if(incompatibleCount > 0){
            connectivity = D_CONNECTIVITY_INCOMPATIBLE_STATE;
            d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN,
                "Unable to start, because %u incompatible fellow states have been detected. " \
                "Restarting now...\n", incompatibleCount);
        } else {
            incompatibleCount = d_adminGetIncompatibleDataModelCount(durability->admin);

            if(incompatibleCount > 0){
                connectivity = D_CONNECTIVITY_INCOMPATIBLE_DATA_MODEL;
                d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN,
                    "Unable to start, because %u incompatible data models have been detected. " \
                    "Shutting down now...\n", incompatibleCount);
            } else {
                connectivity = D_CONNECTIVITY_OK;
            }
        }
    } else {
        connectivity = D_CONNECTIVITY_UNDETERMINED;
    }
    return connectivity;
}

typedef struct nsCompleteWalkData
{
    d_durability durability;
    d_store store;
    c_bool confirmed;
} nsCompleteWalkData;

/* Check if namespaces are confirmed */
static void
nameSpaceConfirmedWalk (
    d_nameSpace nameSpace,
    c_voidp userData)
{
    d_durabilityKind durabilityKind;

    durabilityKind = d_nameSpaceGetDurabilityKind(nameSpace);

    /* If namespace is not persistent, continue */
    if( (durabilityKind == D_DURABILITY_PERSISTENT) ||
                        (durabilityKind == D_DURABILITY_ALL)) {

        /* Check if master is confirmed for namespace */
        if (!d_nameSpaceIsMasterConfirmed(nameSpace)) {
            ((struct nsCompleteWalkData*)userData)->confirmed = FALSE;
        }
    }
}

/* Check if namespace is complete */
static void
nameSpaceCompleteWalk(
    d_nameSpace nameSpace,
    c_voidp userData)
{
    d_durability durability;
    d_durabilityKind durabilityKind;
    d_store store;

    durability = ((struct nsCompleteWalkData*)userData)->durability;
    store = ((struct nsCompleteWalkData*)userData)->store;
    durabilityKind = d_nameSpaceGetDurabilityKind(nameSpace);

    /* If namespace is not persistent, continue */
    if( (durabilityKind == D_DURABILITY_PERSISTENT) ||
                        (durabilityKind == D_DURABILITY_ALL)) {

        /* Check master state of namespace, was it complete during alignment? */
        if (d_nameSpaceGetMasterState(nameSpace) == D_STATE_COMPLETE)
        {
            /* Mark namespace complete */
            if (d_storeNsMarkComplete (store, nameSpace, TRUE) == D_STORE_RESULT_OK)
            {
                d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN,
                     "Local copy of namespace '%s' is complete\n", d_nameSpaceGetName (nameSpace));
            }else
            {
                d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_MAIN,
                     "Failed to mark local copy of namespace '%s' as complete\n", d_nameSpaceGetName (nameSpace));
                OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                     "Failed to mark local copy of namespace '%s' as complete\n",
                     d_nameSpaceGetName (nameSpace));
            }
        }else
        {
            /* Mark namespace incomplete */
            if (d_storeNsMarkComplete (store, nameSpace, FALSE) == D_STORE_RESULT_OK)
            {
                d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_MAIN,
                        "Local copy of namespace '%s' is not complete\n", d_nameSpaceGetName (nameSpace));
            }else
            {
                d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_MAIN,
                        "Failed to mark local copy of namespace '%s' as incomplete\n", d_nameSpaceGetName (nameSpace));
                OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                        "Failed to mark local copy of namespace '%s' as incomplete\n",
                        d_nameSpaceGetName (nameSpace));
            }
        }
    }
}

static void
determineNameSpaceCompleteness(
		d_durability durability,
		d_subscriber subscriber)
{
    d_admin admin;
    d_store store;
    nsCompleteWalkData walkData;
    os_time sleepTime;

    admin = durability->admin;
	store = d_subscriberGetPersistentStore(subscriber);
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = 100000000;

	if (store) {
	    walkData.durability = durability;
	    walkData.store = store;
	    do {
	        os_nanoSleep(sleepTime);

            /* Reset confirmed status to true */
            walkData.confirmed = TRUE;
	        d_adminNameSpaceWalk (admin, nameSpaceConfirmedWalk, &walkData);
	    }while (!walkData.confirmed && durability->splicedRunning);

        d_adminNameSpaceWalk (admin, nameSpaceCompleteWalk, &walkData);
	}
}

static c_bool
collectNameSpaces(
    d_nameSpace nameSpace,
    void* userData)
{
    c_iter nameSpaces;

    nameSpaces = (c_iter)userData;
    nameSpaces = c_iterInsert(nameSpaces, d_objectKeep(d_object(nameSpace)));

    return TRUE;
}

static c_bool
collectFellows(
    d_fellow fellow,
    void* userData)
{
    c_iter fellows;

    fellows = (c_iter)userData;
    fellows = c_iterInsert(fellows, d_objectKeep(d_object(fellow)));

    return TRUE;
}

static void
doInitialMerge(
    d_durability durability)
{
    c_iter fellows, nameSpaces;
    d_fellow fellow;
    d_name fellowRole;
    d_nameSpace nameSpace;
    d_networkAddress fellowAddress, fellowMasterAddress;

    fellows = c_iterNew(NULL);
    d_adminFellowWalk(durability->admin, collectFellows, fellows);

    fellow = d_fellow(c_iterTakeFirst(fellows));

    while(fellow) {
        if(d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED) {
            /* If communication state is approved all fellow namespaces are received and fellowRole must be
             * available. */
            fellowRole = d_fellowGetRole(fellow);
            if(strcmp(durability->configuration->role, fellowRole) != 0) {
                fellowAddress = d_fellowGetAddress(fellow);

                nameSpaces = c_iterNew(NULL);
                d_fellowNameSpaceWalk(fellow, collectNameSpaces, nameSpaces);

                nameSpace = d_nameSpace(c_iterTakeFirst(nameSpaces));

                while(nameSpace){
                    fellowMasterAddress = d_nameSpaceGetMaster(nameSpace);

                    if(d_networkAddressEquals(fellowAddress, fellowMasterAddress)){
                        d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN,
                            "Investigating initial merge with fellow %d in role %s for nameSpace %s.\n",
                            fellowAddress->systemId, fellowRole,
                            d_nameSpaceGetName(nameSpace));

                        d_adminReportMaster(durability->admin, fellow, nameSpace, NULL);
                    }
                    d_networkAddressFree(fellowMasterAddress);
                    d_nameSpaceFree(nameSpace);
                    nameSpace = d_nameSpace(c_iterTakeFirst(nameSpaces));
                }
                c_iterFree(nameSpaces);

                d_networkAddressFree(fellowAddress);
            }
            d_fellowFree(fellow);
            fellow = d_fellow(c_iterTakeFirst(fellows));

        /* If state is unknown, wait until communication is either approved or rejected */
        }else if(d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_UNKNOWN) {
            os_time waitTime;

            /* Wait a little while so loop doesn't take too much resources */
            waitTime.tv_sec = 0;
            waitTime.tv_nsec = 100000000; /* 100msec */
            os_nanoSleep(waitTime);

        /* Communication with fellow is not approved. Move to next fellow. */
        }else {
            d_fellowFree(fellow);
            fellow = d_fellow(c_iterTakeFirst(fellows));
        }
    }
    c_iterFree(fellows);
}

void
d_durabilityHandleInitialAlignment(
    d_durability durability)
{
    d_subscriber subscriber;
    c_bool complete, fellowNameSpacesKnown;
    os_time sleepTime, reportTime, newTime;
    d_sampleChainListener sampleChainListener;
    d_configuration configuration;

    sleepTime.tv_sec        = 0;
    sleepTime.tv_nsec       = 100000000; /* 100 ms */
    subscriber              = d_adminGetSubscriber(durability->admin);
    fellowNameSpacesKnown   = FALSE;
    sampleChainListener     = d_subscriberGetSampleChainListener(subscriber);
    configuration           = d_durabilityGetConfiguration(durability);

    d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN,
            "Waiting for nameSpaces of fellows to get complete...\n");

    /* Find namespaces of fellows */
    while((fellowNameSpacesKnown == FALSE) && (durability->splicedRunning == TRUE) && (d_adminGetFellowCount(durability->admin) > 0)){
        d_adminFellowWalk(durability->admin, checkFellowNameSpacesKnown, &fellowNameSpacesKnown);
        os_nanoSleep(sleepTime);
    }

    /* Start aligning groups */
    if(durability->splicedRunning == TRUE){
        d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN, "Fellow nameSpaces complete.\n");
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Starting groupLocalListener...\n");
        d_subscriberSetGroupLocalListenerEnabled(subscriber, TRUE);
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Waiting for local groups to get complete...\n");
    }
    while( (d_durabilityGetState(durability) != D_STATE_FETCH_INITIAL) &&
           (durability->splicedRunning == TRUE))
    {
        os_nanoSleep(sleepTime);
    }
    reportTime = os_timeGet();

    /* Wait until group alignment has finished */
    do{
        complete = d_adminAreLocalGroupsComplete(durability->admin);

        if((complete == FALSE) && (durability->splicedRunning == TRUE)){
            os_nanoSleep(sleepTime);

            if(configuration->tracingVerbosityLevel == D_LEVEL_FINEST){
                newTime = os_timeGet();

                if(os_timeCompare(newTime, reportTime) != OS_LESS){
                    reportTime.tv_sec = newTime.tv_sec;
                    reportTime.tv_nsec = newTime.tv_nsec;
                    reportTime.tv_sec += 30;
                    d_sampleChainListenerReportStatus(sampleChainListener);
                }
            }
        }
    } while((complete == FALSE) && (durability->splicedRunning == TRUE));

    /* Durability service has finished initial alignment */
    if(durability->splicedRunning == TRUE){
    	determineNameSpaceCompleteness (durability, subscriber);

        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Local groups are complete now.\n");
        d_durabilitySetState(durability, D_STATE_COMPLETE);
        u_serviceChangeState(durability->service, STATE_OPERATIONAL);
        d_printTimedEvent(durability, D_LEVEL_INFO, D_THREAD_MAIN, "Durability service up and fully operational.\n");

        doInitialMerge(durability);
    }
}

void
d_durabilityDeinit(
    d_object object)
{
    d_status status;
    d_networkAddress addr;
    d_durability durability;

    assert(d_objectIsValid(object, D_DURABILITY) == TRUE);

    if(object){
        durability = d_durability(object);
        d_durabilitySetState(durability, D_STATE_TERMINATING);

        if(durability->service){
            u_serviceChangeState(durability->service, STATE_TERMINATING);
            u_serviceWatchSpliceDaemon(durability->service, NULL, durability);
        }
        if(os_threadIdToInteger(durability->statusThread)){
            os_threadWaitExit(durability->statusThread, NULL);
        }
        if(durability->admin){
            status = d_statusNew(durability->admin);
            addr = d_networkAddressUnaddressed();
            d_publisherStatusWrite(d_adminGetPublisher(durability->admin),
                                   status, addr);
            d_networkAddressFree(addr);
            d_statusFree(status);
            d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN, "destroying administration...\n");
            d_adminFree(durability->admin);
            d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN, "administration destroyed\n");
            durability->admin = NULL;
        }
        if(durability->configuration){
            d_configurationFree(durability->configuration);
            durability->configuration = NULL;
        }
        if(durability->serviceManager){
            u_serviceManagerFree(durability->serviceManager);
            durability->serviceManager = NULL;
        }
        if(os_threadIdToInteger(durability->leaseThread)){
            os_threadWaitExit(durability->leaseThread, NULL);
        }
        if(durability->service){
            u_serviceChangeState(durability->service, STATE_TERMINATED);

            u_serviceFree(durability->service);
            durability->service = NULL;
        }
        d_durabilitySetState(durability, D_STATE_TERMINATED);
        u_userDetach();
    }
}

void
d_durabilityFree(
    d_durability durability)
{
    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);

    if(durability){
        d_objectFree(d_object(durability), D_DURABILITY);
    }
}

void
d_durabilityLoadModule(
    v_entity entity,
    c_voidp args)
{
    c_base       base;

    OS_UNUSED_ARG(args);
    assert(!args);
    base = c_getBase((c_object)entity);
    loaddurabilityModule2(base);
}

d_configuration
d_durabilityGetConfiguration(
    d_durability durability)
{
    d_configuration config = NULL;

    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);

    if(durability){
        config = durability->configuration;
    }
    return config;
}

OPENSPLICE_ENTRYPOINT (ospl_durability)
{
    c_char *uri;
    c_char *serviceName;
    d_durability durability;
    os_time time;
    d_connectivity connectivity;
    c_bool stop, success=TRUE;
    c_ulong maxTries, tryCount;
    int result;
    c_long domainId = 1;

    /**
     * Expecting: durability <serviceName> <uri>
     */
#ifdef INTEGRITY
    Error err;
    Semaphore durabilitySvcStartSem = SemaphoreObjectNumber(13);

    serviceName = "durability";

    uri = "file:///ospl.xml";

    err = WaitForSemaphore(durabilitySvcStartSem);
    assert ( err == Success );

#else
    success = d_durabilityArgumentsProcessing(argc, argv, &uri, &serviceName);
#endif

    if(success == TRUE){
        stop = FALSE;
        maxTries = 10;
        tryCount = 0;
        domainId = 1;

#ifdef EVAL_V
        OS_REPORT(OS_INFO,"d_durability", 0,
                  "+++++++++++++++++++++++++++++++++++" OS_REPORT_NL
                  "++ durability EVALUATION VERSION ++" OS_REPORT_NL
                  "+++++++++++++++++++++++++++++++++++\n");
#endif

        while((stop == FALSE) && (tryCount < maxTries) ){
            stop = TRUE;
            tryCount++;

            durability = d_durabilityNew(uri, serviceName, domainId);

            if(durability){
                connectivity = d_durabilityDetermineConnectivity(durability);

                if((connectivity == D_CONNECTIVITY_OK) && (durability->splicedRunning == TRUE)){
                    d_durabilityHandleInitialAlignment(durability);

                    /* 200 ms */
                    time.tv_sec = 0;
                    time.tv_nsec = 200000000;
                    while(durability->splicedRunning == TRUE){
                        os_nanoSleep(time);
                    }
                    d_durabilityFree(durability);
                } else if((connectivity == D_CONNECTIVITY_INCOMPATIBLE_STATE) && (durability->splicedRunning == TRUE)){
                    d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_MAIN, "State is incompatible, restarting now...\n");
                    d_durabilityFree(durability);
                    durability = NULL;
                    stop = FALSE;
                } else {
                    d_durabilityFree(durability);
                }
            } else {
                if(uri){
                    OS_REPORT_1(OS_ERROR, D_CONTEXT, 0,
                        "Could not connect to URI '%s'.",
                        uri);
                } else {
                    OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                        "Could not connect to default URI.");
                }
            }
        }
        if(uri){
            os_free(uri);
        }
        if(serviceName){
            os_free(serviceName);
        }
        result = 0;
    } else {
        result = 1;
    }
    assert(d_objectValidate(0));

    return result;
}

u_service
d_durabilityGetService(
    d_durability durability)
{
    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);
    return durability->service;
}

d_serviceState
d_durabilityGetState(
    d_durability durability)
{
    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);
    return durability->state;
}

c_bool
d_durabilityMustTerminate(
    d_durability durability)
{
    c_bool result;

    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);

    if(durability->splicedRunning == FALSE){
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

void
d_durabilityTerminate(
    d_durability durability,
    c_bool died)
{

    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);

    if (! died) {
        /* The durability service is in a state that is incompatible with
         * its configuration, e.g., because the time to wait for an aligner
         * is exceeded and no aligner is found. Terminate and report the service
         * state of durability as STATE_INCOMPATIBLE_CONFIGURATION.
         */
        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                "An incompatibility with the configuration was detected; terminating and reporting as incompatible configuration.\n");
        u_serviceChangeState(durability->service, STATE_INCOMPATIBLE_CONFIGURATION);
    } else {
        /* In all other cases something bad happened. Terminate the durability
         * service and report as died
         */
        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                "Unrecoverable error occurred; terminating and reporting as died.\n");
        u_serviceChangeState(durability->service, STATE_DIED);
    }
    durability->splicedRunning = FALSE;
}

void
d_durabilitySetState(
    d_durability durability,
    d_serviceState state)
{
    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);
    d_printTimedEvent(durability, D_LEVEL_INFO, D_THREAD_MAIN, "----LEAVING STATE----\n\n\n");
    durability->state = state;
    d_printTimedEvent(durability, D_LEVEL_INFO, D_THREAD_MAIN, "----ENTERING STATE----\n");
}


struct waitForServiceHelper {
    v_group group;
    d_durability durability;
    c_bool attached;
    c_bool proceed;
    os_time endTime;
};


c_bool
d_durabilityWaitForAttachToGroup(
    d_durability durability,
    v_group group)
{
    v_serviceStateKind serviceStateKind;
    c_iter services;
    c_char *name, *partition;
    c_bool result;
    os_time endTime, waitTime;
    v_groupAttachState attachState;

    if(c_iterLength(durability->configuration->services) > 0){
        endTime          = os_timeGet();
        waitTime.tv_sec  = 0;
        waitTime.tv_nsec = 100000000; /*100ms*/
        endTime          = os_timeAdd(endTime,
                                durability->configuration->networkMaxWaitTime);

        result    = FALSE;
        services  = c_iterCopy(durability->configuration->services);
        name      = c_iterTakeFirst(services);

        while(name){
            serviceStateKind = u_serviceManagerGetServiceStateKind(
                                durability->serviceManager, name);

            switch (serviceStateKind) {
            case STATE_INITIALISING:
            case STATE_OPERATIONAL:
            case STATE_NONE: /* also wait with STATE_NONE, because networking might not have been started up yet */
                attachState = v_groupServiceGetAttachState(group, name);

                switch(attachState){
                case V_GROUP_ATTACH_STATE_UNKNOWN:
                    if(os_timeCompare(os_timeGet(), endTime) == OS_LESS){
                        os_nanoSleep(waitTime); /* Try again after waitTime*/
                    } else {
                        d_printTimedEvent(durability,
                              D_LEVEL_WARNING,
                              D_THREAD_GROUP_LOCAL_LISTENER,
                              "Service '%s' did NOT attach to the group within time range.\n",
                              name);
                        name = (c_char*)c_iterTakeFirst(services);
                    }
                    break;
                case V_GROUP_ATTACH_STATE_ATTACHED:
                    result = TRUE;
                    d_printTimedEvent(durability,
                          D_LEVEL_FINER,
                          D_THREAD_GROUP_LOCAL_LISTENER,
                          "Service '%s' has attached to group %s.%s.\n",
                          name,
                          v_entity(group->partition)->name,
                          v_entity(group->topic)->name);
                    name = (c_char*)c_iterTakeFirst(services);
                    break;
                case V_GROUP_ATTACH_STATE_NO_INTEREST:
                    d_printTimedEvent(durability,
                                              D_LEVEL_FINER,
                                              D_THREAD_GROUP_LOCAL_LISTENER,
                                              "Service '%s' has no interest in group %s.%s.\n",
                                              name,
                                              v_entity(group->partition)->name,
                                              v_entity(group->topic)->name);
                    name = (c_char*)c_iterTakeFirst(services);
                    break;
                }
                break;
            case STATE_INCOMPATIBLE_CONFIGURATION:
            case STATE_TERMINATING:
            case STATE_TERMINATED:
            case STATE_DIED:
            default:
                d_printTimedEvent(durability,
                          D_LEVEL_WARNING,
                          D_THREAD_GROUP_LOCAL_LISTENER,
                          "Not waiting for service '%s' to attach to the group\n",
                          name);
                OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                          "Not waiting for service %s to attach to the group.",
                          name);
                name = (c_char*)c_iterTakeFirst(services);
                break;
            }
        }
        c_iterFree(services);
    } else {
        result = TRUE;
    }

    if(result == TRUE) {
        partition = v_entity(group->partition)->name;

        /* If partition string is formatted '__NODE<ID>BUILT-IN PARTITION__'
         * it should be considered a local partition.
         */
        /* Verify whether partition has length to potentially have the format
         * we're looking for.
         */
        if(strlen(partition) > (strlen(D_OSPL_NODE) + strlen(D_OSPL_BUILTIN_PARTITION))){
            /* Check if starts with __NODE */
            if(strncmp(D_OSPL_NODE, partition, strlen(D_OSPL_NODE)) == 0){
                /* Set pointer to the end of the string minus the length of the
                 * partition postfix 'BUILT-IN PARTITION__'
                 */
                partition = partition + strlen(partition) - strlen(D_OSPL_BUILTIN_PARTITION);
                /* Verify whether it ends with 'BUILT-IN PARTITION__' */
                if(strncmp(D_OSPL_BUILTIN_PARTITION, partition, strlen(D_OSPL_BUILTIN_PARTITION)) == 0){
                    /* If so consider it local */
                    result = FALSE;

                    d_printTimedEvent(durability,
                          D_LEVEL_FINEST,
                          D_THREAD_GROUP_LOCAL_LISTENER,
                          "Found group %s.%s which is a built-in local group.\n",
                          v_entity(group->partition)->name,
                          v_entity(group->topic)->name);
                }
            }
        }
    }
    return result;

}

struct updateStatistics {
    d_durabilityStatisticsCallback callback;
    c_voidp userData;
};

static void
d_durabilityUpdateStatisticsCallback(
    v_entity entity,
    c_voidp args)
{
    struct updateStatistics* update;


    if(entity->statistics){
        update = (struct updateStatistics*)args;
        update->callback(v_durabilityStatistics(entity->statistics), update->userData);
    }
    return;
}

void
d_durabilityUpdateStatistics(
    d_durability durability,
    d_durabilityStatisticsCallback callback,
    c_voidp args)
{
    struct updateStatistics update;

    update.callback = callback;
    update.userData = args;

    u_entityAction(u_entity(durability->service), d_durabilityUpdateStatisticsCallback, &update);
}

u_result
d_durabilityTakePersistentSnapshot(
    d_durability durability,
    c_char* partitionExpr,
    c_char* topicExpr,
    c_char* uri)
{
    u_result result;
    d_storeResult storeResult;
    d_admin admin;
    d_subscriber subscriber;
    d_store store;
    d_serviceState state;

    assert(durability);
    assert(partitionExpr);
    assert(topicExpr);
    assert(uri);

    state = d_durabilityGetState(durability);
    if(state == D_STATE_COMPLETE)
    {
        admin = durability->admin;
        subscriber = d_adminGetSubscriber(admin);
        store = d_subscriberGetPersistentStore(subscriber);
        storeResult = d_storeCreatePersistentSnapshot(store, partitionExpr, topicExpr, uri);
        /* Map the store result to a u_result */
        switch(storeResult)
        {
        case D_STORE_RESULT_OK:
            result = U_RESULT_OK;
            break;
        case D_STORE_RESULT_OUT_OF_RESOURCES:
            result = U_RESULT_OUT_OF_MEMORY;
            break;
        case D_STORE_RESULT_ILL_PARAM:
            result = U_RESULT_ILL_PARAM;
            break;
        default:
            result = U_RESULT_PRECONDITION_NOT_MET;
            break;
        }
    } else
    {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}


