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
#include "d_durability.h"
#include "d__durability.h"
#include "d__remoteReaderListener.h"
#include "d__groupLocalListener.h"
#include "d__sampleChainListener.h"
#include "d__sampleRequestListener.h"
#include "d__groupRemoteListener.h"
#include "d__deleteDataListener.h"
#include "d__persistentDataListener.h"
#include "d__groupsRequestListener.h"
#include "d__statusListener.h"
#include "d__nameSpacesRequestListener.h"
#include "d__dcpsHeartbeatListener.h"
#include "d__nameSpacesListener.h"
#include "d__historicalDataRequestListener.h"
#include "d__durabilityStateRequestListener.h"
#include "d__dcpsPublicationListener.h"
#include "d__capabilityListener.h"
#include "d__configuration.h"
#include "d__admin.h"
#include "d__publisher.h"
#include "d__subscriber.h"
#include "d__table.h"
#include "d__misc.h"
#include "d__fellow.h"
#include "d__actionQueue.h"
#include "d__group.h"
#include "d__nameSpace.h"
#include "d__conflictMonitor.h"
#include "d__conflictResolver.h"
#include "d__capability.h"
#include "d__mergeState.h"
#include "d_status.h"
#include "d_groupsRequest.h"
#include "d_nameSpacesRequest.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "d__thread.h"
#include "u_observable.h"
#include "u_entity.h"
#include "u_durability.h"
#include "u_serviceManager.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_durability.h"
#include "v_time.h"
#include "v_entity.h"
#include "os_process.h"
#include "v_builtin.h"
#include "v_groupSet.h"
#include "os_report.h"
#include "os_heap.h"
#include "os_time.h"
#include "d_store.h"
#include "c_iterator.h"
#ifdef INTEGRITY
#include "include/os_getRSObjects.h"
#endif
#ifdef OSPL_ENTRY_OVERRIDE
#include "ospl_entry_override.h"
#endif
/* TODO: The waitForAttachToGroup only supports one service to wait for. This
 * must be a set of services.
 */

static void
durabilityTerminate(
    d_durability durability)
{
    d_status status;
    d_publisher publisher;
    d_networkAddress addr;

    /* Code cloned from d_durabilityWatchSpliceDaemon(). */
    if(durability->admin){
        addr = d_networkAddressUnaddressed();
        publisher = d_adminGetPublisher(durability->admin);
        status = d_statusNew(durability->admin);
        d_durabilitySetState(durability, D_STATE_TERMINATING);
        if (!d_publisherStatusWrite(publisher, status, addr)) {
            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                    "Failed to send d_status message, because durability is terminating.\n");
            OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                    "Failed to send d_status message, because durability is terminating.");
        }
        d_statusFree(status);
        d_networkAddressFree(addr);
    }
    os_mutexLock(&durability->terminateMutex);
    durability->splicedRunning = FALSE;
    (void)os_condBroadcast(&durability->terminateCondition);
    os_mutexUnlock(&durability->terminateMutex);
}


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
        OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                "Arguments are missing. Usage: %s <serviceName> [<uri>]",
                argv[0]);
    }
    return result;
}

void
d_durabilityWatchSpliceDaemon(
    v_serviceStateKind spliceDaemonState,
    c_voidp usrData )
{
    d_durability durability;

    switch(spliceDaemonState){
        case STATE_TERMINATING:
        case STATE_TERMINATED:
        case STATE_DIED:
            durability = (d_durability)usrData;
            assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);

            if (durability) {
                d_printTimedEvent(durability, D_LEVEL_INFO,
                    "Splicedaemon is stopping, terminating durability now...\n");
                durabilityTerminate(durability);
            }
            break;
        default:
            break;
    }
}

static c_voidp
d_durabilityUpdateLeaseAndNotifyStatus(
    c_voidp args)
{
    d_thread self = d_threadLookupSelf ();
    d_durability durability;
    d_configuration config;
    d_admin admin;
    os_duration expiryTime;
    os_duration intvCheck, terminateCheck;
    os_timeM tNextCheck, tNextRenew, tNextStatus, tNextTerminate;
    os_boolean all_is_well = OS_TRUE;
    d_status status;
    d_publisher publisher;
    d_networkAddress addressee;
    int resendCount;
    os_result waitResult;
    int noDeadlockDetection;
    c_bool forceStatusUpdate;

    durability = d_durability(args);
    if (!d_objectIsValid (d_object (durability), D_DURABILITY)) {
        return NULL;
    }
    config = d_durabilityGetConfiguration(durability);
    admin = durability->admin;
    publisher = d_adminGetPublisher(admin);
    addressee = d_networkAddressUnaddressed ();
    status = d_statusNew (admin);
    resendCount = 0;

    intvCheck = OS_DURATION_INIT(0, 100000000); /* 100 ms */
    expiryTime = durability->configuration->livelinessExpiryTime;
    noDeadlockDetection = (durability->configuration->deadlockDetection==FALSE)?1:0;

    d_threadLivelinessInit(expiryTime);
    tNextCheck = tNextRenew = tNextStatus = tNextTerminate = os_timeMGet ();
    while (!d_durabilityMustTerminate(durability)) {
        os_timeM tNow = os_timeMGet ();

        /* Report thread status every 10 sec. */
        if (os_timeMCompare (tNow, tNextCheck) != OS_LESS) {
            tNextCheck = os_timeMAdd (tNextCheck, intvCheck);
            all_is_well = d_threadLivelinessCheck (tNow, config->threadLivelinessReportPeriod);
        }

        /* Renew lease every livelinessUpdateInterval */
        if (os_timeMCompare (tNow, tNextRenew) != OS_LESS) {
            tNextRenew = os_timeMAdd (tNextRenew, durability->configuration->livelinessUpdateInterval);
            if (all_is_well || noDeadlockDetection) {
                (void)u_serviceRenewLease (durability->service, durability->configuration->livelinessExpiryTime);
            } else {
                OS_REPORT(OS_WARNING, "d_durabilityUpdateLeaseAndNotifyStatus", 0,
                          "One or more threads did not proceed, liveliness not asserted");
            }
        }

        /* Note this the only location where forceStatusWrite is decremented */
        forceStatusUpdate = (pa_ld32(&durability->forceStatusWrite) > 0);
        if (forceStatusUpdate) {
            pa_dec32(&durability->forceStatusWrite);
        }

        /* Publish status every heartbeatUpdateInterval */
        if (resendCount > 0) {
            if (all_is_well || noDeadlockDetection) {
                if (d_publisherStatusResend (publisher, status)) {
                    resendCount = 0;
                    tNextStatus = os_timeMAdd (tNextStatus, durability->configuration->heartbeatUpdateInterval);
                } else {
                    resendCount++;
                    if (d_message(status)->senderState == D_STATE_TERMINATING) {
                         d_printTimedEvent(durability, D_LEVEL_FINEST,
                                 "Failed to resend d_status message, because durability is terminating.\n");
                         OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                 "Failed to send d_status message, because durability is terminating.");
                     } else if ((resendCount % 5) == 0) {
                         d_printTimedEvent(durability, D_LEVEL_WARNING,
                                 "Already tried to resend d_status message '%d' times.\n", resendCount);
                     }
                }
            }
        } else if (os_timeMCompare (tNow, tNextStatus) != OS_LESS || forceStatusUpdate) {
            if (all_is_well || noDeadlockDetection) {
                if (d_publisherStatusWrite (publisher, status, addressee)) {
                    tNextStatus = os_timeMAdd (tNextStatus, durability->configuration->heartbeatUpdateInterval);
                } else {
                    resendCount = 1;
                    if (d_message(status)->senderState == D_STATE_TERMINATING) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to resend d_status message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                "Failed to send d_status message, because durability is terminating.");
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Already tried to resend d_status message '%d' times.\n", resendCount);
                    }
                }
            }
        }

        /* Check for cleanup of terminateFellows every second. */
        terminateCheck = OS_DURATION_INIT(1, 0); /* 1 sec */
        if (os_timeMCompare (tNow, tNextTerminate) != OS_LESS) {
            tNextTerminate = os_timeMAdd (tNextTerminate, terminateCheck);
            if (all_is_well || noDeadlockDetection) {
                d_adminCleanupTerminateFellows(admin);
            }
        }

        /* splicedRunning flag is also accessed outside the mutex
         * lock. This is no problem, protection guarantees no wait
         * entry while setting the flag.
         */
        os_mutexLock (&durability->terminateMutex);
        if (durability->splicedRunning) {
            os_timeM tNext = tNextCheck;
            os_duration sleepTime;

            if (os_timeMCompare (tNextRenew, tNext) == OS_LESS) {
                tNext = tNextRenew;
            }
            if (os_timeMCompare (tNextStatus, tNext) == OS_LESS) {
                tNext = tNextStatus;
            }
            sleepTime = os_timeMDiff (tNext, tNow);
            if (os_durationCompare (sleepTime, OS_DURATION_ZERO) == OS_MORE) {
                waitResult = d_condTimedWait(self,
                                             &durability->terminateCondition,
                                             &durability->terminateMutex, sleepTime);
                if (waitResult == os_resultFail) {
                    OS_REPORT(OS_CRITICAL, "d_durabilityUpdateLeaseAndNotifyStatus", 0,
                              "d_condTimedWait failed");
                    os_mutexUnlock (&durability->terminateMutex);
                    d_durabilityTerminate(durability, TRUE);
                    os_mutexLock (&durability->terminateMutex);
                }
            }
        }
        os_mutexUnlock (&durability->terminateMutex);
    } /* while */

    d_statusFree (status);
    d_networkAddressFree (addressee);

    if (all_is_well) {
        u_serviceRenewLease (durability->service, 20*OS_DURATION_SECOND);
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

d_durability
d_durabilityNew(
    const c_char* uri,
    const c_char* serviceName,
    c_long domainId)
{
    u_result uresult;
    d_durability durability;
    os_result osr;
    c_bool result, ret;

    durability = NULL;
    uresult = u_userInitialise();
    if (uresult == U_RESULT_OK) {
        /* Allocate durability object */
        durability = d_durability(os_malloc(C_SIZEOF(d_durability)));
        if (durability) {
            /* Call super-init */
            d_objectInit(d_object(durability), D_DURABILITY,
                         (d_objectDeinitFunc)d_durabilityDeinit);
            /* Initialize durability */
            durability->state               = D_STATE_INIT;
            durability->service             = NULL;
            durability->splicedRunning      = TRUE;
            durability->configuration       = NULL;
            durability->admin               = NULL;
            durability->serviceManager      = NULL;
            durability->statusThread        = OS_THREAD_ID_NONE;
            durability->died                = FALSE;
            durability->myVersion.major     = D_DURABILITY_VERSION_MAJOR;
            durability->myVersion.minor     = D_DURABILITY_VERSION_MINOR;
            {
                struct _DDS_VendorId_t vendorId = D_DURABILITY_VERSION_VENDOR_ID;
                durability->myVersion.vendorId  = vendorId;
            }
            pa_st32(&durability->requestId,0);
            pa_st32(&durability->conflictId, 0);
            pa_st32(&durability->incarnation, 0);
            pa_st32(&durability->forceStatusWrite,0);
            /* Create terminate mutex and terminate condition */
            osr = os_mutexInit(&durability->terminateMutex, NULL);
            if (osr == os_resultSuccess) {
                osr = os_condInit(&durability->terminateCondition, &durability->terminateMutex, NULL );
            }
            if (osr == os_resultSuccess) {
                d_durabilitySetState(durability, D_STATE_INIT);
                /* Has to happen before any thread gets created -- at the time
                 * of writing this, d_adminNew is the first to do that.
                 */
                osr = d_threadsInit(durability);
                /* From now on threads liveliness is available */
            }
            if (osr == os_resultSuccess) {

                /* Note: The following message is never printed because
                 * durability has not yet read the configuration which
                 * specifies the location of log file. It would be better
                 * to buffer the messages until the location of the log file
                 * has been read and flush the buffer as soon as the log
                 * file is available. A similar approach is implemented in
                 * ddsi.
                 */
                d_printTimedEvent(durability, D_LEVEL_FINER, "Creating user service...\n");
                durability->service = u_durabilityNew(uri, U_DOMAIN_ID_ANY, 30, serviceName, NULL, TRUE);
                if (durability->service != NULL) {
                    ret = u_serviceChangeState(durability->service, STATE_INITIALISING);
                    if (ret) {
                        durability->serviceManager = u_serviceManagerNew(u_participant(durability->service));
                        d_printTimedEvent(durability, D_LEVEL_FINER, "Loading durability module...\n");
                        uresult = u_observableWriteAction(u_observable(durability->service), d_durabilityLoadModule, (c_voidp)&result);
                        if (uresult == U_RESULT_OK) {
                            d_printTimedEvent(durability, D_LEVEL_FINER, "Reading configuration...\n");
                            if (!(durability->configuration = d_configurationNew(durability, serviceName, domainId))) {
                                /* Error occurred in loading configuration. */
                                uresult = U_RESULT_UNDEFINED;
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_FINER, "Starting splicedaemon listener...\n");
                                uresult = u_serviceWatchSpliceDaemon(durability->service, d_durabilityWatchSpliceDaemon, durability);
                            }
                        }
                        if ((uresult == U_RESULT_OK) && durability->configuration) {
                            durability->admin = d_adminNew(durability);
                            if (durability->admin) {
                                /* Register configured namespaces with admin */
                                d_durabilityRegisterNameSpaces(durability);
                                /* Now ready to initialize protocol subscriber */
                                d_adminInitSubscriber(durability->admin);
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
                } else {
                    if (uri) {
                        OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Could not connect to URI '%s'.", uri);
                    } else {
                        OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Could not connect to default URI.");
                    }
                    d_durabilityFree(durability);
                    durability = NULL;
                }
            } else {
                d_durabilityFree(durability);
                durability = NULL;
            }
        } else {
            OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Memory allocation failure, unable to start durability.");
        }
    }
    return durability;
}


void
d_durabilityInit(
    d_durability durability)
{
    d_subscriber subscriber;
    os_result osr;

    subscriber = d_adminGetSubscriber(durability->admin);

    if (d_subscriberStartListeners(subscriber)) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Starting subscriber listeners ...\n");
    } else {
        d_printTimedEvent(durability, D_LEVEL_SEVERE, "Starting subscriber listeners failed\n");
    }

    if (durability->splicedRunning == TRUE) {
        os_threadAttr tattr = durability->configuration->heartbeatScheduling;

        /* Create the status thread that is responsible for sending status
         * messages and updating leases
         */
        d_printTimedEvent (durability, D_LEVEL_FINER, "Starting lease update and notification of own state...\n");
        if (tattr.schedClass != OS_SCHED_REALTIME && durability->configuration->livelinessScheduling.schedClass == OS_SCHED_REALTIME) {
            tattr = durability->configuration->livelinessScheduling;
        }
        osr = d_threadCreate(&(durability->statusThread), "statusThread",
                             &tattr,
                             d_durabilityUpdateLeaseAndNotifyStatus, durability);
        if (osr != os_resultSuccess) {
            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                "Failed to start updating lease and notifying status\n");
            OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Failed to start updating lease and notifying status.");
        }
    }
}


d_connectivity
d_durabilityDetermineConnectivity(
    d_durability durability)
{
    d_thread self = d_threadLookupSelf ();
    d_nameSpacesRequest nsRequest;
    d_networkAddress addr;
    c_bool nsComplete;
    os_duration sleepTime = OS_DURATION_INIT(0, 200000000);    /* 200 ms */
    os_timeM stopTime;
    d_publisher publisher;
    c_ulong incompatibleCount;
    d_connectivity connectivity;
    d_networkAddress myAddr;
    struct _DDS_Gid_t myServerId;

    d_durabilitySetState(durability, D_STATE_DISCOVER_FELLOWS_GROUPS);

    /* Initially set connectivity to undetermined. Only when
     * connectivity can be determined it is assigned another value.
     */
    connectivity = D_CONNECTIVITY_UNDETERMINED;
    if (durability->splicedRunning == TRUE) {
        myAddr = d_adminGetMyAddress(durability->admin);
        myServerId = d_durabilityGetMyServerId(durability);
        d_printTimedEvent(durability, D_LEVEL_INFO, "My address is: '%u'\n", myAddr->systemId);
        d_printTimedEvent(durability, D_LEVEL_FINER, "My server Id is: %lld.%lld\n", myServerId.prefix, myServerId.suffix);
        OS_REPORT(OS_INFO, D_SERVICE_NAME, 0, "The durability service can be identified by ID: %u", myAddr->systemId);
        d_networkAddressFree(myAddr);
        d_printTimedEvent(durability, D_LEVEL_FINE, "Requesting fellow namespaces...\n");
        addr = d_networkAddressUnaddressed();
        publisher  = d_adminGetPublisher(durability->admin);
        nsRequest = d_nameSpacesRequestNew(durability->admin);
        d_publisherNameSpacesRequestWrite(publisher, nsRequest, addr, d_durabilityGetState(durability));
        d_nameSpacesRequestFree(nsRequest);
        d_printTimedEvent(durability, D_LEVEL_FINER, "Waiting some time to allow fellows to report...\n");

        /* Set the InitialWaitingPeriod to allow fellows to report. */
        stopTime  = os_timeMGet();
        stopTime  = os_timeMAdd(stopTime, durability->configuration->timingInitialWaitPeriod);
        /* Wait the InitialWaitingPeriod to allow fellows to report. */
        while ((durability->splicedRunning == TRUE) && (os_timeMCompare(os_timeMGet(), stopTime) == OS_LESS)) {
            d_sleep(self, sleepTime);
        }
        /* Free the network address */
        d_networkAddressFree(addr);

        d_printTimedEvent(durability, D_LEVEL_FINE,
            "I now know %u fellows. Wait for fellow namespaces to get complete...\n",
            d_adminGetFellowCount(durability->admin));
        nsComplete = TRUE;
        d_adminFellowWalk(durability->admin, checkNameSpaces, &nsComplete);

        /* Wait until all fellows are completed. */
        while ((nsComplete == FALSE) &&
               (durability->splicedRunning == TRUE)) {
            d_sleep(self,sleepTime);
            /* Check if all fellows are complete */
            d_adminFellowWalk(durability->admin, checkNameSpaces, &nsComplete);
            if (d_adminGetFellowCount(durability->admin) == 0) {
                nsComplete = TRUE;
            }
        }
        incompatibleCount = d_adminGetIncompatibleStateCount(durability->admin);

        if (incompatibleCount > 0) {
            connectivity = D_CONNECTIVITY_INCOMPATIBLE_STATE;
            d_printTimedEvent(durability, D_LEVEL_WARNING,
                "Unable to start, because %u incompatible fellow states have been detected. " \
                "Restarting now...\n", incompatibleCount);
        } else {
            incompatibleCount = d_adminGetIncompatibleDataModelCount(durability->admin);

            if (incompatibleCount > 0) {
                connectivity = D_CONNECTIVITY_INCOMPATIBLE_DATA_MODEL;
                d_printTimedEvent(durability, D_LEVEL_WARNING,
                    "Unable to start, because %u incompatible data models have been detected. " \
                    "Shutting down now...\n", incompatibleCount);
            } else {
                connectivity = D_CONNECTIVITY_OK;
            }
        }
    }
    return connectivity;
}


typedef struct nsCompleteWalkData
{
    c_bool confirmed;
} nsCompleteWalkData;


/* Check if namespaces are confirmed */
static void
nameSpaceConfirmedWalk (
    d_nameSpace nameSpace,
    c_voidp userData)
{
    /* Check if master is confirmed for namespace */
    if (!d_nameSpaceIsMasterConfirmed(nameSpace)) {
        ((struct nsCompleteWalkData*)userData)->confirmed = FALSE;
    }
}

static void
storeNsMarkComplete (
    d_durability durability,
    d_store store,
    d_nameSpace nameSpace,
    c_bool isComplete)
{
    if (d_storeNsMarkComplete (store, nameSpace, isComplete) == D_STORE_RESULT_OK) {
        d_printTimedEvent(durability, D_LEVEL_FINER,
                "Local copy of namespace '%s' is %scomplete\n",
                d_nameSpaceGetName (nameSpace),
                isComplete ? "" : "not ");
    } else {
        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                "Failed to mark local copy of namespace '%s' as %scomplete\n",
                d_nameSpaceGetName (nameSpace),
                isComplete ? "" : "in");
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                "Failed to mark local copy of namespace '%s' as %scomplete\n",
                d_nameSpaceGetName (nameSpace),
                isComplete ? "" : "in");
    }
}

static void
nameSpaceMarkCompleteness(
    d_durability durability,
    d_nameSpace nameSpace)
{
    d_durabilityKind durabilityKind;
    d_subscriber subscriber;
    d_store store = NULL;
    d_networkAddress masterAddr;
    d_fellow masterFellow;
    d_mergeState masterMergeState = NULL;

    durabilityKind = d_nameSpaceGetDurabilityKind(nameSpace);
    if ((durabilityKind == D_DURABILITY_PERSISTENT) ||
        (durabilityKind == D_DURABILITY_ALL)) {
        subscriber = d_adminGetSubscriber(durability->admin);
        store = d_subscriberGetPersistentStore(subscriber);
    }

    /* Check master state of namespace, was it complete during alignment? */
    if (d_nameSpaceGetMasterState(nameSpace) == D_STATE_COMPLETE) {
        masterAddr = d_nameSpaceGetMaster(nameSpace);
        masterFellow = d_adminGetFellow(durability->admin, masterAddr);
        if (masterFellow) {
            d_nameSpace masterNameSpace;
            masterNameSpace = d_fellowGetNameSpace(masterFellow, nameSpace);
            if (masterNameSpace) {
                masterMergeState = d_nameSpaceGetMergeState(masterNameSpace, NULL);
            }
            d_fellowFree(masterFellow);
        }
        d_networkAddressFree(masterAddr);

        if (masterMergeState) {
            d_nameSpaceSetMergeState(nameSpace, masterMergeState);
            d_printTimedEvent(durability, D_LEVEL_FINER,
                 "Namespace '%s' complete, set state to %d\n", nameSpace->name, masterMergeState->value);
            d_mergeStateFree(masterMergeState);
        }

        if (store) {
            storeNsMarkComplete(durability, store, nameSpace, TRUE);
        }
    } else if (store) {
        /* Mark namespace incomplete */
        storeNsMarkComplete(durability, store, nameSpace, FALSE);
    }
}

void
d_durabilityDetermineNameSpaceCompleteness(
    d_durability durability)
{
    d_thread self = d_threadLookupSelf ();
    d_admin admin = durability->admin;
    nsCompleteWalkData walkData;
    os_duration sleepTime = OS_DURATION_INIT(0, 100000000);  /* 100 ms */
    c_iter nameSpaces;
    d_nameSpace nameSpace;

    do {
        d_sleep(self, sleepTime);
        /* Reset confirmed status to true */
        walkData.confirmed = TRUE;
        d_adminNameSpaceWalk (admin, nameSpaceConfirmedWalk, &walkData);
    } while (!walkData.confirmed && durability->splicedRunning);

    nameSpaces = d_adminNameSpaceCollect(admin);
    while ((nameSpace = c_iterTakeFirst(nameSpaces))) {
        nameSpaceMarkCompleteness(durability, nameSpace);
        d_nameSpaceFree(nameSpace);
    }
    c_iterFree(nameSpaces);
}


static c_bool
collectNameSpaces(
    d_nameSpace nameSpace,
    void* userData)
{
    c_iter nameSpaces;

    nameSpaces = (c_iter)userData;
    assert(nameSpaces != NULL);
    (void) c_iterInsert(nameSpaces, d_objectKeep(d_object(nameSpace)));

    return TRUE;
}


static c_bool
collectFellows(
    d_fellow fellow,
    void* userData)
{
    c_iter fellows;

    fellows = (c_iter)userData;
    (void) c_iterInsert(fellows, d_objectKeep(d_object(fellow)));

    return TRUE;
}


void
d_durabilityDoInitialMerge(
    d_durability durability)
{
    d_thread self = d_threadLookupSelf ();
    c_iter fellows, fellowNameSpaces;
    d_fellow fellow;
    d_name fellowRole;
    d_nameSpace nameSpace, fellowNameSpace;
    d_networkAddress fellowAddress, fellowMasterAddress;
    d_admin admin;

    admin = durability->admin;
    fellows = c_iterNew(NULL);
    d_adminFellowWalk(admin, collectFellows, fellows);
    fellow = d_fellow(c_iterTakeFirst(fellows));
    while (fellow) {
        if (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED) {
            /* If communication state is approved all fellow namespaces are received and fellowRole must be
             * available.
             */
            fellowRole = d_fellowGetRole(fellow);
            if (fellowRole && strcmp(durability->configuration->role, fellowRole) != 0) {
                fellowAddress = d_fellowGetAddress(fellow);
                fellowNameSpaces = c_iterNew(NULL);
                (void)d_fellowNameSpaceWalk(fellow, collectNameSpaces, fellowNameSpaces);
                fellowNameSpace = d_nameSpace(c_iterTakeFirst(fellowNameSpaces));
                while (fellowNameSpace) {
                    fellowMasterAddress = d_nameSpaceGetMaster(fellowNameSpace);
                    if (d_networkAddressEquals(fellowAddress, fellowMasterAddress)) {
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                            "Investigating initial merge with fellow %u in role %s for nameSpace %s.\n",
                            fellowAddress->systemId, fellowRole,
                            d_nameSpaceGetName(fellowNameSpace));
                        /* Get the nameSpace with the same name as the fellow's nameSpace */
                        nameSpace = d_adminGetNameSpace(admin, d_nameSpaceGetName(fellowNameSpace));
                        if (nameSpace) {
                            d_adminReportMaster(admin, fellow, nameSpace);
                            d_nameSpaceFree(nameSpace);
                        }
                    }
                    d_networkAddressFree(fellowMasterAddress);
                    d_nameSpaceFree(fellowNameSpace);
                    fellowNameSpace = d_nameSpace(c_iterTakeFirst(fellowNameSpaces));
                }
                c_iterFree(fellowNameSpaces);
                d_networkAddressFree(fellowAddress);
            }
            d_fellowFree(fellow);
            fellow = d_fellow(c_iterTakeFirst(fellows));
        /* If state is unknown, wait until communication is either approved or rejected */
        } else if (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_UNKNOWN &&
                   d_fellowGetState(fellow) != D_STATE_TERMINATED) {
            os_duration waitTime = OS_DURATION_INIT(0, 100000000);  /* 100 ms */
            d_sleep(self, waitTime);
        /* Communication with fellow is not approved. Move to next fellow. */
        } else {
            d_fellowFree(fellow);
            fellow = d_fellow(c_iterTakeFirst(fellows));
        }
    }
    c_iterFree(fellows);
}

/* Below code assumes the V_ALIGNSTATE enum to start with 0. V_ALIGNSTATE_COUNT
 * should be the last (unused) label. No easy way to assert on that.
 */
struct alignstate_order_constraint {
    char require_V_ALIGNSTATE_INCOMPLETE_eq_0 [V_ALIGNSTATE_INCOMPLETE == 0];
    char non_empty_dummy_last_member[1];
};

struct completeness {
    c_iter alignState[V_ALIGNSTATE_COUNT];
    os_uint32 psc;
};

static c_bool
collectGroups (
        c_object o,
        c_voidp arg)
{
    v_group g;
    struct completeness *c = (struct completeness *)arg;

    g = v_group(o);
    if(v_topicGetQos(v_groupTopic(g))->durability.v.kind != V_DURABILITY_VOLATILE) {
        c->alignState[g->alignState] = c_iterAppend(c->alignState[g->alignState], os_strdup(v_groupName(g)));
    }

    return TRUE;
}

static void
d__durabilityCollectCompleteness (
    v_public _this,
    void *varg)
{
    v_kernel k;
    struct completeness *c = (struct completeness *)varg;

    k = v_objectKernel(_this);
    assert(k != NULL);
    assert(v_kernelHasDurabilityService(k));

    c->psc = pa_ld32(&k->purgeSuppressCount);

    v_groupSetWalk(k->groupSet, &collectGroups, varg);
}

void
d_durabilityReportKernelGroupCompleteness (
    _In_ d_durability durability)
{
    u_result result;
    struct completeness completeness;
    static const struct completeness zeroed;
    char *gn;
    char *str[V_ALIGNSTATE_COUNT];
    size_t gnLen, len;
    int pos;
    unsigned int i;

    completeness = zeroed;
    result = u_observableAction (u_observable (durability->service), &d__durabilityCollectCompleteness, &completeness);
    if(result == U_RESULT_OK) {
        for(i = 0; i < V_ALIGNSTATE_COUNT; i++ ) {
            len = strlen(v_alignStateImage(i)) + strlen(": ");
            str[i]  = os_malloc(len + 1);
            pos = snprintf(str[i], len + 1, "%s: ", v_alignStateImage(i));
            while((gn = c_iterTakeFirst(completeness.alignState[i])) != NULL) {
                gnLen = strlen(gn) + 2;
                len = gnLen + len;
                str[i] = os_realloc(str[i], len + 1);
                pos += snprintf(str[i] + pos, gnLen + 1, "%s, ", gn) ;
                os_free(gn);
            }
            c_iterFree(completeness.alignState[i]);
            str[i][pos - 2] = '\0';
        }

        for(i = 0; i < V_ALIGNSTATE_COUNT; i++) {
            d_printTimedEvent(durability, D_LEVEL_FINER, "%s\n", str[i]);
            os_free(str[i]);
        }
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Kernel purgeSuppressionCount = %u\n", completeness.psc);
    } else {
        d_printTimedEvent(durability, D_LEVEL_SEVERE, "d_durabilityReportKernelGroupCompleteness failed to access kernel: %s. \n", u_resultImage(result));
    }
    return;
}

void
d_durabilityDeinit(
    d_durability durability)
{
    d_status status;
    d_networkAddress addr;
    d_subscriber subscriber;
    d_admin admin;

    assert(d_durabilityIsValid(durability));

    /* Durability is about to terminate.
     * To clean up resources we first stop as many threads as possible
     * before actually freeing resources. In this way we avoid that
     * a resources is freed while another tread still holds references
     * to this this resources. Once all threads are stopped it is safe
     * to clean up.
     * The persistenDataListener() is an exception to this rule. In case
     * there is still persistent data available in the persistent data
     * queue when durability is about to terminate, then part of the
     * ServiceTerminatePeriod is used to persist as many samples as
     * possible. To prevent that other listeners will not be freed
     * while persisting data we free the persistentDataListener
     * as the last listener.
     */

    /* Set state to terminating and make sure that threads testing
     * splicedRunning will stop
     */
    d_durabilitySetState(durability, D_STATE_TERMINATING);
    os_mutexLock(&durability->terminateMutex);
    durability->splicedRunning = FALSE;
    (void)os_condBroadcast(&durability->terminateCondition);
    os_mutexUnlock(&durability->terminateMutex);

    /* Stop the heartbeat publication thread */
    if (durability->service) {
        u_serviceChangeState(durability->service, STATE_TERMINATING);
        u_serviceWatchSpliceDaemon(durability->service, NULL, durability);
    }
    if (os_threadIdToInteger(durability->statusThread)) {
        d_threadWaitExit(durability->statusThread, NULL);
    }
    if (durability->admin) {
        admin = durability->admin;
        assert(d_adminIsValid(admin));
        subscriber = d_adminGetSubscriber(admin);
        assert(d_subscriberIsValid(subscriber));

        /* Publish the last known status to the fellows */
        status = d_statusNew(admin);
        addr = d_networkAddressUnaddressed();
        if (!d_publisherStatusWrite(d_adminGetPublisher(admin), status, addr)) {
             d_printTimedEvent(durability, D_LEVEL_SEVERE,
                 "Failed to send d_status message, because durability is terminating.\n");
             OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                 "Failed to send d_status message, because durability is terminating.");
         }

        d_networkAddressFree(addr);
        d_statusFree(status);

        /* we need to disable and free the heartbeat listener first as it is dependent on the
         * conflictresolver and its not useful to process heartbeats during termination */
        if (subscriber->dcpsHeartbeatListener) {
            d_dcpsHeartbeatListenerStop(subscriber->dcpsHeartbeatListener);
            d_dcpsHeartbeatListenerFree(subscriber->dcpsHeartbeatListener);
            subscriber->dcpsHeartbeatListener = ((void *)0);
        }

        d_subscriberStopListeners(subscriber);

        /* Destroy the admin */
        d_printTimedEvent(durability, D_LEVEL_FINE, "destroying administration...\n");
        d_adminFree(durability->admin);
        d_printTimedEvent(durability, D_LEVEL_FINE, "administration destroyed\n");
        durability->admin = NULL;
    }
    /* Destroy serviceManager */
    if (durability->serviceManager) {
        u_objectFree(durability->serviceManager);
        durability->serviceManager = NULL;
    }
    /* Signal the service state change to the splice daemon.
     * From this point onwards the splice daemon may clean up
     * the service, and the service should not access shared
     * memory any more state to splice daemon
     */
    if (durability->service) {
        if (!durability->died) {
            /* Apparently the service has terminated gracefully */
            u_serviceChangeState(durability->service, STATE_TERMINATED);
            d_durabilitySetState(durability, D_STATE_TERMINATED);
        } else {
            /* Apparently something bad has happened.*/
            u_serviceChangeState(durability->service, STATE_DIED);
        }
        u_objectFree(u_object(durability->service));
        durability->service = NULL;
    }
    /* Clear the threads management. */
    d_threadsDeinit();
    /* Clean up the configuration */
    if (durability->configuration) {
        d_configurationFree(durability->configuration);
        durability->configuration = NULL;
    }
    /* Destroy mutex and condition */
    (void)os_condDestroy(&durability->terminateCondition);
    (void)os_mutexDestroy(&durability->terminateMutex);
    /* Print message BEFORE the super class is destroyed,
     * because that frees the durability object
     */
    d_printTimedEvent(durability, D_LEVEL_FINE, "Durability destroyed\n");
    /* Call super-deinit */
    d_objectDeinit(d_object(durability));
}


void
d_durabilityFree(
    d_durability durability)
{
    assert(d_durabilityIsValid(durability));

    d_objectFree(d_object(durability));
}


void
d_durabilityLoadModule(
    v_public entity,
    c_voidp args /* c_bool */)
{
    c_base base;
    c_bool *result = (c_bool *)args;
    c_metaObject hd_obj;

    base = c_getBase((c_object)entity);
    /* Load the durability odl definitions. */
    *result = loaddurabilityModule2(base);
    if (*result) {
        /* The client durability idl definitions should already
         * have been loaded by the kernel.
         */
        hd_obj = c_metaResolve(c_metaObject(base),"DDS::HistoricalData");
        if (hd_obj == NULL) {
            *result = FALSE;
            OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                "Failed to resolve client durability module");
        }
        c_free(hd_obj);
    } else {
        OS_REPORT(OS_ERROR, D_CONTEXT, 0,
            "Failed to load durability module");
    }
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


static os_result
exitRequestHandler(
    os_callbackArg ignore,
    void *callingThreadContext,
    void * arg)
{
    d_durability durability = (d_durability)arg;

    OS_UNUSED_ARG(callingThreadContext);
    OS_UNUSED_ARG(ignore);

    assert(durability);

    /* Terminate durability */
    os_mutexLock(&durability->terminateMutex);
    durability->splicedRunning = FALSE;
    (void)os_condBroadcast(&durability->terminateCondition);
    os_mutexUnlock(&durability->terminateMutex);

    OS_REPORT(OS_INFO, "Durability Exit Handler", 0, "Initiated Durability Service termination...");

    return os_resultSuccess;
}

#if 0 /* Need to review if this is still required. */
static os_result
ExceptionRequestHandler(
        void * arg)
{
    d_durability durability = (d_durability)arg;

    assert(durability);

    u_serviceChangeState(durability->service, STATE_DIED);

    if (durability->oldExceptionHandlerInfo.callback) {
        return durability->oldExceptionHandlerInfo.callback(durability->oldExceptionHandlerInfo.arg);
    } else {
        return os_resultSuccess;
    }
}
#endif

OPENSPLICE_SERVICE_ENTRYPOINT (ospl_durability, durability)
{
    c_char *uri = NULL;
    c_char *serviceName;
    d_connectivity connectivity;
    c_bool stop, success=TRUE;
    c_ulong maxTries, tryCount;
    int result = 0;
    c_long domainId = 1;
    d_durability durability;
    int enable_allocation_report = 0;
    d_conflict conflict;

    OS_UNUSED_ARG(enable_allocation_report);  /* to avoid build failure in case of release builds using -Werror */

    {
        char *p;
        enable_allocation_report = ((p = os_getenv ("OSPL_DURABILITY_ALLOCATION_REPORT")) != NULL && atoi (p) != 0) ? 1 : 0;
    }

    /**
     * Expecting: durability <serviceName> <uri>
     */

    success = d_durabilityArgumentsProcessing(argc, argv, &uri, &serviceName);

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
                os_signalHandlerExitRequestHandle erh = os_signalHandlerExitRequestHandleNil;
                d_thread self = d_threadLookupSelf ();

                if(!os_serviceGetSingleProcess()){
                     erh = os_signalHandlerRegisterExitRequestCallback(exitRequestHandler, NULL, NULL, NULL, durability);
                }

                connectivity = d_durabilityDetermineConnectivity(durability);
                if((connectivity == D_CONNECTIVITY_OK) && (durability->splicedRunning == TRUE)){
                    conflict = d_conflictNew(D_CONFLICT_INITIAL, NULL, NULL, NULL);
                    d_conflictSetId(conflict, durability);
                    d_printTimedEvent(durability, D_LEVEL_FINER, "Initial conflict created, conflict %d created\n",
                            d_conflictGetId(conflict));
                    d_printTimedEvent(durability, D_LEVEL_FINER, "Adding conflict %d to the conflict resolver queue\n",
                            d_conflictGetId(conflict));

                    d_conflictResolverAddConflict(durability->admin->conflictResolver, conflict);

                    os_mutexLock(&durability->terminateMutex);
                    while(durability->splicedRunning == TRUE){
                        d_condWait(self, &durability->terminateCondition, &durability->terminateMutex);
                    }
                    os_mutexUnlock(&durability->terminateMutex);
                } else if((connectivity == D_CONNECTIVITY_INCOMPATIBLE_STATE) && (durability->splicedRunning == TRUE)){
                    d_printTimedEvent(durability, D_LEVEL_WARNING, "State is incompatible, restarting now...\n");
                    stop = FALSE;
                }

                os_signalHandlerUnregisterExitRequestCallback(erh);

                d_durabilityFree(durability);
            } else {
                result = 2;
            }
        }
        if(uri){
            os_free(uri);
        }
        if(serviceName){
            os_free(serviceName);
        }
    } else {
        result = 1;
    }
    assert(d_objectValidate(0, enable_allocation_report));

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
    assert(d_durabilityIsValid(durability));

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
        d_printTimedEvent(durability, D_LEVEL_SEVERE,
             "An incompatibility with the configuration was detected; terminating and reporting as incompatible configuration.\n");
        u_serviceChangeState(durability->service, STATE_INCOMPATIBLE_CONFIGURATION);
    } else {
        /* In all other cases something bad happened. Terminate the durability
         * service and report as died. We cannot set the service state of
         * durability to DIED yet because this will signal the splice daemon
         * to free shared memory while durability may not have finished
         * cleaning up! Therefore we postpone setting the service state
         * state to died when durability is de-initialized (see
         * d_durabilityDeinit).
         */
        d_printTimedEvent(durability, D_LEVEL_SEVERE,
             "Unrecoverable error occurred; terminating and reporting as died.\n");
        durability->died = TRUE;
    }
    os_mutexLock(&durability->terminateMutex);
    durability->splicedRunning = FALSE;
    (void)os_condBroadcast(&durability->terminateCondition);
    os_mutexUnlock(&durability->terminateMutex);
}

void
d_durabilitySetState(
    d_durability durability,
    d_serviceState state)
{
    d_serviceState curState;

    assert(d_durabilityIsValid(durability));

    /* Log and broadcast state changes, including the
     * initial state change to D_STATE_INIT that occurs
     * when durability starts
     */
    curState = d_durabilityGetState(durability);
    if ((state == D_STATE_INIT) || (curState != state)) {
        d_printTimedEvent(durability, D_LEVEL_INFO, "----LEAVING STATE----\n\n\n");
        durability->state = state;
        d_printTimedEvent(durability, D_LEVEL_INFO, "----ENTERING STATE----\n");
        /* Broadcast the state, but only if all listeners are
         * initialized and the publisher is not destroyed yet */
        if ((state > D_STATE_INIT) && (state < D_STATE_TERMINATING)) {
             d_status status;
             d_networkAddress addr;
             d_publisher publisher;

             addr = d_networkAddressUnaddressed();
             status = d_statusNew(durability->admin);
             publisher = d_adminGetPublisher(durability->admin);
             if (!d_publisherStatusWrite(publisher, status, addr)) {
                 d_printTimedEvent(durability, D_LEVEL_SEVERE,
                     "Failed to write d_status message when updating state.\n");
                 pa_inc32(&durability->forceStatusWrite);
             }
             d_statusFree(status);
             d_networkAddressFree(addr);
         }
    }
}

void
d_durabilityHeartbeatProcessed(
        _In_ d_durability durability)
{
    os_uint32 newCount;
    u_result r;

    r = u_durabilityHeartbeatProcessed(durability->service, &newCount);
    if(r == U_RESULT_OK) {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "PurgeSuppressionCount lowered to %u.\n", newCount);
    } else {
        d_printTimedEvent(durability, D_LEVEL_SEVERE,
            "Failed to lower purgeSuppressionCount, u_durabilityHeartbeatProcessed(...) returned %s.\n",
            u_resultImage(r));
        OS_REPORT(OS_FATAL, D_CONTEXT_DURABILITY, r,
            "Failed to lower purgeSuppressionCount, u_durabilityHeartbeatProcessed(...) returned %s.",
            u_resultImage(r));
        d_durabilityTerminate(durability, TRUE);
    }
}



c_bool
d_durabilityWaitForAttachToGroup(
    d_durability durability,
    v_group group)
{
    d_thread self = d_threadLookupSelf ();
    v_serviceStateKind serviceStateKind;
    c_iter services;
    c_char *name, *partition, *topic;
    c_bool result;
    os_timeM endTime;
    os_duration waitTime = OS_DURATION_INIT(0, 100000000);
    v_groupAttachState attachState;

    if(c_iterLength(durability->configuration->services) > 0){
        /* Wait for networkMaxWaitTime. */
        endTime = os_timeMAdd(os_timeMGet(), durability->configuration->networkMaxWaitTime);
        result    = FALSE;
        services  = c_iterCopy(durability->configuration->services);
        name      = c_iterTakeFirst(services);
        while ((name) && (!d_durabilityMustTerminate(durability))) {
            serviceStateKind = u_serviceManagerGetServiceStateKind(
                                durability->serviceManager, name);

            switch (serviceStateKind) {
            case STATE_INITIALISING:
            case STATE_OPERATIONAL:
            case STATE_NONE: /* also wait with STATE_NONE, because networking might not have been started up yet */
                attachState = v_groupServiceGetAttachState(group, name);

                switch(attachState){
                case V_GROUP_ATTACH_STATE_UNKNOWN:
                    if(os_timeMCompare(os_timeMGet(), endTime) == OS_LESS){
                        d_sleep(self, waitTime); /* Try again after waitTime*/
                    } else {
                        OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                            "Service '%s' did NOT attach to the group '%s.%s' within time range.\n",
                            name, v_entity(group->partition)->name, v_entity(group->topic)->name);
                        d_printTimedEvent(durability, D_LEVEL_WARNING,
                              "Service '%s' did NOT attach to the group within time range.\n",
                              name);
                        name = (c_char*)c_iterTakeFirst(services);
                    }
                    break;
                case V_GROUP_ATTACH_STATE_ATTACHED:
                    result = TRUE;
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                          "Service '%s' has attached to group %s.%s.\n",
                          name, v_entity(group->partition)->name, v_entity(group->topic)->name);
                    name = (c_char*)c_iterTakeFirst(services);
                    break;
                case V_GROUP_ATTACH_STATE_NO_INTEREST:
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                         "Service '%s' has no interest in group %s.%s.\n",
                         name, v_entity(group->partition)->name, v_entity(group->topic)->name);
                    name = (c_char*)c_iterTakeFirst(services);
                    break;
                }
                break;
            case STATE_INCOMPATIBLE_CONFIGURATION:
            case STATE_TERMINATING:
            case STATE_TERMINATED:
            case STATE_DIED:
            default:
                d_printTimedEvent(durability, D_LEVEL_WARNING,
                    "Not waiting for service '%s' to attach to the group\n", name);
                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                          "Not waiting for service %s to attach to the group.", name);
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
        topic = v_entity(group->topic)->name;
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

                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Found group %s.%s which is a built-in local group.\n",
                        v_entity(group->partition)->name,
                        v_entity(group->topic)->name);
                }
            }
        } else if((strlen(partition) == strlen(V_BUILTIN_PARTITION)) &&
                (strlen(topic) == strlen(V_HEARTBEATINFO_NAME))){

            if(strncmp(partition, V_BUILTIN_PARTITION, strlen(V_BUILTIN_PARTITION)) == 0){
                if(strncmp(topic, V_HEARTBEATINFO_NAME, strlen(V_HEARTBEATINFO_NAME)) == 0){
                    result = FALSE;

                    d_printTimedEvent(durability, D_LEVEL_FINEST,
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
    v_public entity,
    c_voidp args)
{
    struct updateStatistics* update;
    v_durability durability = v_durability(entity);


    if(durability->statistics){
        update = (struct updateStatistics*)args;
        update->callback(durability->statistics, update->userData);
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

    if (durability != NULL) {
        (void)u_observableAction(
            u_observable(durability->service),
            d_durabilityUpdateStatisticsCallback, &update);
    }
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


/**
 * \brief Get the durability version
 */
struct _DDS_DurabilityVersion_t
d_durabilityGetMyVersion (
    d_durability durability)
{
    assert(d_durabilityIsValid(durability));

    return durability->myVersion;
}


/**
 * \brief Retrieve the serverId of this durability service.
 */
struct _DDS_Gid_t
d_durabilityGetMyServerId(
    d_durability durability)
{
    assert(d_durabilityIsValid(durability));

    return durability->myServerId;
}

/**
 * \brief Check if the list of serverIds contains an ID that matches
 *        my serverID.
 *
 * This function retuns TRUE if I am addressed in the list of serverIds
 * in the request, or if the list of serverIds in the request is empty.
 * In the latter case the request is meant for everybody.
 *
 * The boolean forMe will be set if the request was explicitly addressed
 * to me. The boolean forEveryBody will be addressed if the request was
 * addressed to everybody.
 *
 * @return TRUE, if the request is for me or for everybody,
 *         FALSE, otherwise
 */
c_bool
d_durabilityRequestIsForMe(
    d_durability durability,
    c_iter serverIds,
    c_bool *forMe,
    c_bool *forEverybody)
{
    struct _DDS_Gid_t myServerId;
    struct _DDS_Gid_t *serverId;
    c_iterIter iter;

    assert(d_durabilityIsValid(durability));

    *forMe = FALSE;
    myServerId = d_durabilityGetMyServerId(durability);
    /* If the list of serverIds is empty then the request is meant for every
     * aligner, but only the aligner who is master for the group is allowed
     * to respond.
     */
    *forEverybody = (c_iterLength(serverIds) == 0);
    iter = c_iterIterGet(serverIds);
    while ((!*forMe) && ((serverId = (struct _DDS_Gid_t *)c_iterNext(&iter)) != NULL)) {
        *forMe = ((serverId->prefix == myServerId.prefix) && (serverId->suffix == myServerId.suffix));
    }
    return *forMe || *forEverybody;
}


/**
 * \brief Get a unique request Id
 *
 * @return A unique request Id (module 2^64)
 */
struct _DDS_RequestId_t
d_durabilityGetRequestId(
    d_durability durability)
{
    struct _DDS_RequestId_t requestId;

    assert(d_durabilityIsValid(durability));

    requestId.clientId = d_durabilityGetMyServerId(durability);
    requestId.requestId = pa_inc32_nv(&durability->requestId);
    return requestId;
}

/**
 * \brief Get a unique conflict Id
 *
 * @return A unique conflict Id. Never 0.
 */
c_ulong
d_durabilityGenerateConflictId(
    _Inout_ d_durability durability)
{
    c_ulong newId;

    do {
        newId = pa_inc32_nv(&durability->conflictId);
    } while (newId == 0);

    return newId;
}

/* This function is called from the following listeners threads
 *     capabilityListener
 *     nameSpacesListener
 *     nameSpacesRequestListener
 *     statusListener
 * Each of these may create a confirmed fellow.
 */
d_fellow
d_durabilityGetOrCreateFellowFromMessage(
    d_admin admin,
    d_networkAddress fellowAddr,
    d_message message)
{
    d_fellow fellow, fellow2;
    os_uint32 seqnum;
    d_durability durability = d_threadsDurability();

    d_productionTimestampToSeqnum(&seqnum, &d_message(message)->productionTimestamp);
    fellow = d_adminGetFellow(admin, fellowAddr);
    if (!fellow) {
        if ((message->senderState != D_STATE_TERMINATING) &&
            (message->senderState != D_STATE_TERMINATED)) {
            fellow2 = d_fellowNew(fellowAddr, message->senderState, TRUE);
            d_fellowUpdateStatus(fellow2, message->senderState, seqnum);
            fellow = d_adminAddFellow(admin, fellow2);
            if (fellow) {
                if (fellow != fellow2) {
                    /* There must have been another confirmed fellow
                     * with the same address, use that one.
                     */
                    d_fellowFree(fellow2);
                    d_fellowUpdateStatus(fellow, message->senderState, seqnum);
                }

            } else {
                /* d_adminAddFellow returns NULL when the fellow has recently
                 * terminated. Ignore the fellow for now.
                 */
                d_fellowFree(fellow2);
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Fellow %u unknown but terminating, so ignoring the message.\n",
                message->senderAddress.systemId);
        }
    } else {
        /* The fellow already exist, update the status of the last report time */
        d_fellowUpdateStatus(fellow, message->senderState, seqnum);
    }

    /* A fellow's state change may cause that capabilities must be send
     * and may affect my responsiveness, so let's check.
     */
    if (fellow) {
        /* Set capability support */
        d_fellowSetCapabilitySupport(fellow, d_messageHasCapabilitySupport(message));
        /* Check whether to send my capabilities initially to the fellow */
        d_fellowCheckSendCapabilities(fellow, TRUE);
        /* The fellow's capability setting support may have affected the responsiveness, so check */
        d_fellowCheckInitialResponsiveness(fellow);
    }
    return fellow;
}


c_ulong
d_durabilityGetNewIncarnation(
    d_durability durability)
{
    c_ulong newIncarnation;

    do {
        newIncarnation = pa_inc32_nv(&durability->incarnation);
    } while (newIncarnation == 0);

    return newIncarnation;
}
