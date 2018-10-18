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

#include "d__nameSpacesListener.h"
#include "d__readerListener.h"
#include "d__listener.h"
#include "d__fellow.h"
#include "d__nameSpace.h"
#include "d__admin.h"
#include "d__publisher.h"
#include "d__subscriber.h"
#include "d__configuration.h"
#include "d__sampleChainListener.h"
#include "d__misc.h"
#include "d__durability.h"
#include "d__mergeState.h"
#include "d__policy.h"
#include "d_networkAddress.h"
#include "d_nameSpaces.h"
#include "d_nameSpacesRequest.h"
#include "d_groupsRequest.h"
#include "d_message.h"
#include "v_time.h"
#include "os_heap.h"
#include "os_report.h"

/**
 * Macro that checks the d_nameSpacesListener validity.
 * Because d_nameSpacesListener is a concrete class typechecking is required.
 */
#define d_nameSpacesListenerIsValid(_this) \
    d_listenerIsValidKind(d_listener(_this), D_NAMESPACES_LISTENER)

/**
 * \brief The <code>d_nameSpacesListener</code> cast macro.
 *
 * This macro casts an object to a <code>d_nameSpacesListener</code> object.
 */
#define d_nameSpacesListener(_this) ((d_nameSpacesListener)(_this))


C_STRUCT(d_nameSpacesListener){
    C_EXTENDS(d_readerListener);
};

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

        OS_REPORT(OS_ERROR, D_CONTEXT, 0,
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
                    break;
                case D_STATE_DISCOVER_PERSISTENT_SOURCE:
                case D_STATE_TERMINATING:
                case D_STATE_TERMINATED:
                    break;
                default:
                    assert(FALSE);
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
                    break;
                default:
                    break;
            }
            break;
        case D_STATE_DISCOVER_PERSISTENT_SOURCE:
        case D_STATE_TERMINATING:
        case D_STATE_TERMINATED:
            break;
        default:
            assert(FALSE);
            break;
    }

    return TRUE;
}

struct checkFellowMasterHelper
{
    d_admin admin;
    d_fellow fellow;
    d_nameSpace oldNameSpace;
};

struct checkDelayAlignmentHelper {
    d_admin admin;
    d_fellow fellow;
};

static void
namespace_compatibility_check(
    d_admin admin,
    struct d_networkAddress_s sender,
    d_nameSpace nameSpace)
{
    d_nameSpace localNameSpace;

    if ((localNameSpace = d_adminGetNameSpace(admin, nameSpace->name)) == NULL) {
        /* No matching local nameSpace, so masterPriorities cannot be incompatible */
        d_durability durability;
        d_configuration config;

        /* Check if local filters overlap namespace.
         * This should eventually result in ignoring the namespace instead of administering it locally,
         * for now only an error is reported
         */
        durability = d_adminGetDurability(admin);
        config = d_durabilityGetConfiguration(durability);
        if (d_configurationCheckFilterInNameSpace(config, nameSpace)) {
            d_printTimedEvent(durability, D_LEVEL_WARNING,
                "Received nameSpace '%s' from fellow %u is incompatible with local configured filters, can not be the master of this namespace.\n",
                nameSpace->name, sender.systemId);
        }
        return;
    }
    if (!localNameSpace->compatibility_check_required) {
        /* No compatibility check required anymore */
        d_nameSpaceFree(localNameSpace);
        return;
    }
    if (!d_nameSpaceIsAligner(localNameSpace) || (!d_nameSpaceIsAligner(nameSpace))) {
        /* One or both of the nameSpaces is not an aligner, so masterPriority cannot be incompatible */
        d_nameSpaceFree(localNameSpace);
        return;
    }
    if (IS_LEGACY_MASTER_SELECTION(d_nameSpaceGetMasterPriority(nameSpace)) && (!IS_LEGACY_MASTER_SELECTION(d_nameSpaceGetMasterPriority(localNameSpace)))) {
        localNameSpace->compatibility_check_required = FALSE;
        OS_REPORT(OS_ERROR, OS_FUNCTION, 0,
            "I use priorities for namespace '%s' while fellow %u uses the legacy master selection algorithm, this is an incompatible and unsupported configuration",
            localNameSpace->name, sender.systemId);
    } else if (!IS_LEGACY_MASTER_SELECTION(d_nameSpaceGetMasterPriority(nameSpace)) && (IS_LEGACY_MASTER_SELECTION(d_nameSpaceGetMasterPriority(localNameSpace)))) {
        localNameSpace->compatibility_check_required = FALSE;
        OS_REPORT(OS_ERROR, OS_FUNCTION, 0,
            "I use the legacy master selection algorithm for namespace '%s' while fellow %u uses priorities, this is an incompatible and unsupported configuration",
            localNameSpace->name, sender.systemId);
    }
    d_nameSpaceFree(localNameSpace);
}

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
    if (!D_QUALITY_ISZERO(q) && !D_QUALITY_ISINFINITE(q)) {
        d_adminReportDelayedInitialSet(admin, nameSpace, fellow);
    }
}

static void
checkFellowMasterWalk(
    void* o,
    c_voidp userData)
{
    struct checkFellowMasterHelper* helper;
    d_networkAddress fellowMasterAddr, fellowAddr;
    d_nameSpace fellowNameSpace, nameSpace;

    helper = (struct checkFellowMasterHelper*)userData;
    fellowNameSpace = d_nameSpace(o);                           /* the namespace that was received as a d_nameSpace message from the fellow */
    fellowMasterAddr = d_nameSpaceGetMaster (fellowNameSpace);  /* the master of the namespace of the fellow */
    fellowAddr = d_fellowGetAddress(helper->fellow);

    /* The nameSpace that was received as a d_nameSpace message may have
     * changed state, so we need to retrieve the current state of the namespace.
     * If the nameSpace that was received from the fellow has a confirmed
     * master we must check for conflicts. We do not require that I have a
     * confirmed master for the nameSpace myself. This is because I may been
     * involved in a master conflict with another fellow for the same
     * nameSpace, and in the process of solving it I might have set the
     * masterConfirmed attribute for the nameSpace to 0
     * (see d_conflictMonitorCheckForConflicts).
     */
    nameSpace = d_adminGetNameSpace(helper->admin, fellowNameSpace->name);
    if (nameSpace) {
        /* When I receive a namespace from a fellow with a confirmed master then check
         * for a possible master conflict
         */
        if ( d_fellowIsConfirmed(helper->fellow) &&            /* The fellow is confirmed */
             d_nameSpaceIsMasterConfirmed(fellowNameSpace) ) { /* The master selected by the fellow is confirmed */
              /* Now check for conflicts */
              d_adminReportMaster(helper->admin, helper->fellow, nameSpace);
         }

        d_nameSpaceFree(nameSpace);
    }

    d_networkAddressFree(fellowMasterAddr);
    d_networkAddressFree(fellowAddr);
}

static void
dynamicNameSpaceWalk(
    void* o,
    c_voidp userData)
{
    d_nameSpace fellowNameSpace = d_nameSpace(o);
    d_admin admin = d_admin(userData);

    d_nameSpace localNameSpace;
    d_durability durability;
    d_configuration config;
    d_policy policy;
    char * name;

    durability          = d_adminGetDurability(admin);
    config              = d_durabilityGetConfiguration(durability);

    name = d_nameSpaceGetName(fellowNameSpace);
    policy = d_configurationFindPolicyForNameSpace(config, name);
    localNameSpace = d_nameSpaceNew(name, policy);
    /* If namespace is created, add to administration */
    if (localNameSpace) {
        /* Copy partitions to local nameSpace */
        d_nameSpaceCopyPartitions (localNameSpace, fellowNameSpace);
        d_adminAddNameSpace (admin, localNameSpace);
        d_nameSpaceFree (localNameSpace);
    }
    d_policyFree(policy);
}

static c_bool
collectFellowNsWalk(
    d_nameSpace nameSpace,
    c_voidp userData)
{
    c_iter nsList = (c_iter)userData;
    assert(nsList != NULL);
    (void)c_iterAppend (nsList, d_nameSpaceCopy(nameSpace));
    return TRUE;
}

static void
d_nameSpacesListenerAction(
    d_listener listener,
    d_message message)
{
    d_durability durability;
    d_admin admin;
    d_fellow fellow = NULL;
    c_bool allowed;
    d_nameSpace nameSpace, oldFellowNameSpace, fellowNameSpace;
    c_ulong count;
    d_configuration config;
    d_subscriber subscriber;
    d_sampleChainListener sampleChainListener;
    struct compatibilityHelper helper;
    d_adminStatisticsInfo info;
    c_bool added;
    struct checkFellowMasterHelper fellowMasterHelper;
    struct checkDelayAlignmentHelper delayAlignmentHelper;
    d_name role;
    c_iter fellowNameSpaces;
    d_nameSpace ns;
    char mergedStatesStr[1024] = "";
    char mergedStatesExtStr[1024] = "";
    d_quality quality;
    d_networkAddress fellowAddr;

    assert(d_nameSpacesListenerIsValid(listener));

#define MERGED_STATES_STRING(mergedStates, str, begin, end) \
    { \
        const struct d_mergeState_s *ms = (const struct d_mergeState_s *) mergedStates; \
        const c_ulong nms = end; \
        size_t pos = 0; \
        c_ulong i; \
        for (i = begin; i < nms && pos < sizeof(str); i++) { \
            int n = snprintf(str + pos, sizeof(str) - pos, "%s%s, %d", (i == 0) ? "" : "; ", ms[i].role, ms[i].value); \
            if (n > 0) { pos += (size_t)n; } else { break; } \
        } \
        if  (i < nms || pos >= sizeof(str)) { \
            assert(sizeof(str) >= 4); \
            if (pos >= sizeof(str) - 4) { \
                pos = sizeof(str) - 4; \
            } \
            strcpy(str + pos, "..."); \
        } \
    }
    MERGED_STATES_STRING(d_nameSpaces(message)->mergedStates, mergedStatesStr, 0, d_nameSpaces(message)->mergedStatesCount);
    MERGED_STATES_STRING(d_nameSpaces(message)->mergedStates, mergedStatesExtStr, d_nameSpaces(message)->mergedStatesCount, c_sequenceSize(d_nameSpaces(message)->mergedStates));

    admin               = d_listenerGetAdmin(listener);
    durability          = d_adminGetDurability(admin);
    config              = d_durabilityGetConfiguration(durability);
    fellowNameSpaces    = NULL;
    nameSpace = d_nameSpaceFromNameSpaces(config, d_nameSpaces(message));
    d_qualityExtToQuality(&quality, &d_nameSpaces(message)->initialQuality, IS_Y2038READY(message));
    d_printTimedEvent         (durability, D_LEVEL_FINE,
                               "Received nameSpace '%s' from fellow %u (his master: %u, confirmed: %d, mergeState: (%s,%d), quality: %"PA_PRItime", mergedStates:{%s}, extensions: {%s}).\n",
                               d_nameSpaces(message)->name,
                               message->senderAddress.systemId,
                               d_nameSpaces(message)->master.systemId,
                               d_nameSpaces(message)->masterConfirmed,
                               d_nameSpaces(message)->state.role,
                               d_nameSpaces(message)->state.value,
                               OS_TIMEW_PRINT(quality),
                               mergedStatesStr,
                               mergedStatesExtStr);
    /* Check compatibility */
    namespace_compatibility_check(admin, message->senderAddress, nameSpace);

    fellowAddr = d_networkAddressNew(
            message->senderAddress.systemId,
            message->senderAddress.localId,
            message->senderAddress.lifecycleId);

    fellow = d_durabilityGetOrCreateFellowFromMessage(admin, fellowAddr, message);
    /* At this point the fellow must be created unless it has recently
     * terminated or is terminating.
     */
    if (fellow) {
        if (d_fellowNameSpaceCount(fellow) < d_nameSpaces(message)->total && message->addressee.systemId == 0)
        {
            /* Ignore broadcasted namesapces until at least one explicit name space
             * request from me has been answered, as that is what completes the
             * capability+namespace handshake
             */
             d_printTimedEvent (durability, D_LEVEL_FINEST,
                                "Ignoring received broadcasted nameSpaces from fellow %u because it hasn't responded to a request from me yet.\n",
                                message->senderAddress.systemId);
             d_nameSpaceFree(nameSpace);
        }
        /* The last reception time has already been updated in
         * d_durabilitySendNSRequestWhenConfirmed. Let´s check whether
         * the fellow is approved.
         */
        else if (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED) {

            fellowNameSpace = d_fellowGetNameSpace(fellow, nameSpace);

            /* Update master of fellow nameSpace */
            added = d_fellowAddNameSpace(fellow, nameSpace);

            if (fellowNameSpace) {
                /* Get old namespace of fellow */
                oldFellowNameSpace = d_nameSpaceCopy(fellowNameSpace);

            } else {
                oldFellowNameSpace = d_nameSpaceCopy(nameSpace);
                d_fellowSetExpectedNameSpaces(fellow, d_nameSpaces(message)->total);
                subscriber = d_adminGetSubscriber(admin);
                sampleChainListener = d_subscriberGetSampleChainListener(subscriber);

                if (sampleChainListener) {
                    d_sampleChainListenerTryFulfillChains(sampleChainListener, NULL);
                }
            }

            /* Dynamically learn the nameSpace */
            dynamicNameSpaceWalk(nameSpace, admin);

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
            if (!added) {
                d_nameSpaceFree(nameSpace);
            }

            d_nameSpaceFree(oldFellowNameSpace);

        } else {
            info = d_adminStatisticsInfoNew();
            d_fellowSetExpectedNameSpaces(fellow, d_nameSpaces(message)->total);

            /* Set role of fellow (take native role from namespace) */
            if (!d_fellowGetRole(fellow)) {
                role = d_nameSpaceGetRole(nameSpace);
                d_fellowSetRole (fellow, role);
                os_free (role);
            }

            d_fellowAddNameSpace(fellow, nameSpace);
            count = d_fellowNameSpaceCount(fellow);

            if (count == d_nameSpaces(message)->total) {
                allowed = isFellowStateCompatible(durability, fellow);

                if (allowed == TRUE) {
                    config = d_durabilityGetConfiguration(durability);
                    helper.fellow = fellow;
                    helper.compatible = TRUE;

                    d_adminNameSpaceWalk (admin, areFellowNameSpacesCompatible, &helper);

                    if (helper.compatible == TRUE) {

                        if(config->timeAlignment == TRUE){
                            os_timeW srcTime , curTime;
                            os_duration delta, max_delta;

                            curTime = d_readerListener(listener)->lastInsertTime;
                            srcTime= d_readerListener(listener)->lastSourceTime;
                            delta = os_durationAbs(os_timeWDiff(curTime, srcTime));
                            max_delta = OS_DURATION_INIT(1,0);

                            if ( os_durationCompare(delta, max_delta) == OS_MORE) {
                                d_printTimedEvent (durability, D_LEVEL_WARNING,
                                   "Estimated time difference including latency with " \
                                   "fellow %u is %f seconds, which is larger then " \
                                   "expected.\n",
                                   message->senderAddress.systemId,
                                   os_durationToReal(delta));
                                OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                                    "Estimated time difference including latency " \
                                    "with fellow '%u' is larger then expected " \
                                    "(%f seconds). Durability alignment might not be " \
                                    "reliable. Please align time between these nodes " \
                                    "and restart.",
                                    message->senderAddress.systemId,
                                    os_durationToReal(delta));
                            } else {
                                d_printTimedEvent (durability, D_LEVEL_FINEST,
                                   "Estimated time difference including latency with " \
                                   "fellow %u is %f seconds.\n",
                                   message->senderAddress.systemId,
                                   os_durationToReal(delta));
                            }
                        }

                        d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_APPROVED);
                        info->fellowsApprovedDif += 1;
                        subscriber = d_adminGetSubscriber(admin);
                        sampleChainListener = d_subscriberGetSampleChainListener(subscriber);

                        if(sampleChainListener){
                            d_sampleChainListenerTryFulfillChains(sampleChainListener, NULL);
                        }

                        d_printTimedEvent (durability, D_LEVEL_FINER,
                                           "Received %u of %u nameSpaces from fellow %u.\n",
                                           count, d_nameSpaces(message)->total,
                                           message->senderAddress.systemId);

                        /* Check if the fellow is master for one or more namespaces and report this to admin */
                        fellowNameSpaces = c_iterNew(NULL);

                        /* Collect fellow namespaces */
                        d_fellowNameSpaceWalk (fellow, collectFellowNsWalk, fellowNameSpaces);

                        /* Dynamically learn the fellow's nameSpaces */
                        c_iterWalk(fellowNameSpaces, dynamicNameSpaceWalk, admin);

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
                    } else {
                        info->fellowsIncompatibleDataModelDif += 1;

                        d_printTimedEvent (durability, D_LEVEL_WARNING,
                                       "Communication with fellow %u NOT approved, because data model is not compatible\n",
                                       message->senderAddress.systemId);
                        d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL);
                    }
                } else {
                    info->fellowsIncompatibleStateDif += 1;
                    d_printTimedEvent (durability, D_LEVEL_WARNING,
                                       "Communication with fellow %u NOT approved, because state is not compatible my state: %d, fellow state: %d\n",
                                       message->senderAddress.systemId,
                                       d_durabilityGetState(durability),
                                       message->senderState);
                    d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_INCOMPATIBLE_STATE);
                }
            } else {
                d_printTimedEvent (durability, D_LEVEL_FINEST,
                                   "Received %u of %u nameSpaces from fellow %u.\n",
                                   count, d_nameSpaces(message)->total,
                                   message->senderAddress.systemId);
            }
            d_adminUpdateStatistics(admin, info);
            d_adminStatisticsInfoFree(info);
        }
        d_fellowFree(fellow);
    } else {
        d_nameSpaceFree(nameSpace);
    }
    d_networkAddressFree(fellowAddr);
    return;
}

static void
d_nameSpacesListenerDeinit(
    d_nameSpacesListener listener)
{
    assert(d_nameSpacesListenerIsValid(listener));

    /* Stop the nameSpacesListener */
    d_nameSpacesListenerStop(listener);
    /* Nothing to deallocated, call super-deinit */
    d_readerListenerDeinit(d_readerListener(listener));
}

static void
d_nameSpacesListenerInit(
    d_nameSpacesListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed
     */

    assert(d_subscriberIsValid(subscriber));

    /* Call super-init */
    os_threadAttrInit(&attr);
    d_readerListenerInit(   d_readerListener(listener),
                            D_NAMESPACES_LISTENER,
                            d_nameSpacesListenerAction,
                            subscriber,
                            D_NAMESPACES_TOPIC_NAME,
                            D_NAMESPACES_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            attr,
                            (d_objectDeinitFunc)d_nameSpacesListenerDeinit);
}

d_nameSpacesListener
d_nameSpacesListenerNew(
    d_subscriber subscriber)
{
    d_nameSpacesListener listener;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate nameSpacesListener object */
    listener = d_nameSpacesListener(os_malloc(C_SIZEOF(d_nameSpacesListener)));
    if (listener) {
        /* Initialize the nameSpacesListener */
        d_nameSpacesListenerInit(listener, subscriber);
    }
    return listener;
}


void
d_nameSpacesListenerFree(
    d_nameSpacesListener listener)
{
    assert(d_nameSpacesListenerIsValid(listener));

    d_objectFree(d_object(listener));
}


c_bool
d_nameSpacesListenerStart(
    d_nameSpacesListener listener)
{
    assert(d_nameSpacesListenerIsValid(listener));

    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_nameSpacesListenerStop(
    d_nameSpacesListener listener)
{
    assert(d_nameSpacesListenerIsValid(listener));

    return d_readerListenerStop(d_readerListener(listener));
}
