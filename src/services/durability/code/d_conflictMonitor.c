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

#include "d__conflictMonitor.h"
#include "d__conflict.h"
#include "d__nameSpace.h"
#include "d__policy.h"
#include "d__thread.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__fellow.h"
#include "d__conflictResolver.h"
#include "d__mergeState.h"
#include "d__subscriber.h"
#include "d__nameSpacesRequestListener.h"
#include "d__configuration.h"
#include "d__table.h"
#include "d__durability.h"
#include "d__admin.h"
#include "d__nameSpace.h"
#include "d__fellow.h"
#include "d__misc.h"
#include "d_networkAddress.h"
#include "d_object.h"
#include "os_time.h"
#include "os_heap.h"


d_conflictMonitor
d_conflictMonitorNew(
    d_admin admin)
{
    d_conflictMonitor conflictMonitor = NULL;

    /* Create conflictMonitor object */
    conflictMonitor = d_conflictMonitor(os_malloc(C_SIZEOF(d_conflictMonitor)));
    if (conflictMonitor) {
        /* Call super-init */
        d_lockInit(d_lock(conflictMonitor), D_CONFLICT_MONITOR, (d_objectDeinitFunc)d_conflictMonitorDeinit);
        /* Initialize conflictMonitor */
        conflictMonitor->admin = admin;
        conflictMonitor->lastTimeChecked = OS_TIMEM_ZERO;
    }
    return conflictMonitor;
}


void
d_conflictMonitorDeinit(
    d_conflictMonitor conflictMonitor)
{
    assert(d_conflictMonitorIsValid(conflictMonitor));

    /* Call super-deinit */
    d_lockDeinit(d_lock(conflictMonitor));
}


void d_conflictMonitorFree(
    d_conflictMonitor conflictMonitor)
{
    assert(d_conflictMonitorIsValid(conflictMonitor));

    d_objectFree(d_object(conflictMonitor));
}

static void traceNetworkAddress (d_durability dur, const char *label, d_networkAddress a)
{
    d_printTimedEvent (dur, D_LEVEL_FINEST, "%s {%u,%u,%u}\n", label, a->systemId, a->localId, a->lifecycleId);
}

static void traceFellow (d_durability dur, const char *label, d_fellow f)
{
    d_printTimedEvent (dur, D_LEVEL_FINEST, "%s %p {%u,%u,%u} state %d commstate %d role %s isConfirmed %d hasConfirmed %d \n", label, f, f->address->systemId, f->address->localId, f->address->lifecycleId, (int)f->state, (int)f->communicationState, f->role, f->isConfirmed, f->hasConfirmed);
}

static void traceNameSpace (d_durability dur, const char *label, d_nameSpace ns)
{
    struct d_policy_s dpst;
    struct d_networkAddress_s mst;
    d_policy dp = ns->policy ? ns->policy : &dpst;
    d_networkAddress m = ns->master ? ns->master : &mst;
    memset (&dpst, 0, sizeof (dpst));
    memset (&mst, 0, sizeof (mst));
    d_printTimedEvent (dur, D_LEVEL_FINEST,
        "%s %p %s [policy %s %d %d %d %d] mergeState (%s,%d) master {%u,%u,%u} st %d conf %d alignable %d\n",
        label, ns, ns->name ? ns->name : "(null)", dp->nameSpace ? dp->nameSpace : "(null)",
        (int)dp->aligner, (int)dp->alignmentKind, (int)dp->delayedAlignment, (int)dp->durabilityKind,
        ns->mergeState->role, ns->mergeState->value, m->systemId, m->localId, m->lifecycleId,
        (int)ns->masterState, (int)ns->masterConfirmed, (int)ns->alignable);
}

static void traceConflict (d_durability dur, const char *label, d_conflict c)
{
    d_printTimedEvent (dur, D_LEVEL_FINEST, "%s %p id %u event %u requeuecnt %u fellowAddr {%u,%u,%u} nameSpaceCopy %p fellowNameSpaceCopy %p\n", label, c, c->id, c->event, c->reQueueCount, c->fellowAddr->systemId, c->fellowAddr->localId, c->fellowAddr->lifecycleId, (void*)c->nameSpaceCopy, (void*)c->fellowNameSpaceCopy);
    if (c->nameSpaceCopy) {
        traceNameSpace (dur, "conflict nameSpaceCopy", c->nameSpaceCopy);
    }
}

/**
 * \brief Verify if there is a master conflict between the namespace and the fellowNameSpace.
 *
 * @return TRUE if there is a master conflict, FALSE otherwise.
 */
static c_bool
d_conflictMonitorHasMasterConflict(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr,
    d_nameSpace nameSpaceCopy,
    d_nameSpace fellowNameSpaceCopy)
{
    d_networkAddress masterAddr, fellowMasterAddr, unAddressed;
    d_durability durability;
    d_admin admin;
    d_fellow fellow;
    d_serviceState myState, fellowState;
    c_bool result = FALSE;

    assert(d_conflictMonitorIsValid(conflictMonitor));

    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    myState = d_durabilityGetState(durability);
    unAddressed = d_networkAddressUnaddressed();

    /* Get the (confirmed) fellow with the fellowAddr */
    fellow = d_adminGetFellow(admin, fellowAddr);
    if (fellow) {
        d_lockLock(d_lock(fellow));
        traceFellow (durability, "HasMasterConflict fellow", fellow);
        fellowState = d_fellowGetState(fellow);
        if ( (fellowState != D_STATE_TERMINATING) && (fellowState != D_STATE_TERMINATED) ) {
            masterAddr = d_nameSpaceGetMaster(nameSpaceCopy);
            fellowMasterAddr = d_nameSpaceGetMaster(fellowNameSpaceCopy);

            traceNetworkAddress (durability, "masterAddr", masterAddr);
            traceNetworkAddress (durability, "fellowMasterAddr", fellowMasterAddr);
            d_printTimedEvent (durability, D_LEVEL_FINEST, "myState %u fellowState %u\n", myState, fellowState);

            /* Only check for conflicts if I'm complete and fellow is past injecting persistent data. */
            if ( ( (myState >= D_STATE_DISCOVER_PERSISTENT_SOURCE) && (fellowState >= D_STATE_INJECT_PERSISTENT) ) ) {

                /* A master conflict is detected when a fellow that has a
                 * different master than me, and my master is confirmed.
                 *
                 * Note that this criterion will result in a master
                 * conflict when there is no aligner (master:0, confirmed: 1)
                 * and suddenly an aligner appears that becomes master.
                 *
                 * Also note that we do NOT require that the master of the
                 * fellow is confirmed!  This is to ensure that master
                 * selection is triggered with a fellow that may been master
                 * a little while before, but has recently unconfirmed its
                 * mastership because it may be involved in master selection
                 * with another node. */
                d_printTimedEvent (durability, D_LEVEL_FINEST,
                    "myrole %s fellowrole %s isMasterConfirmed %d\n",
                    nameSpaceCopy->mergeState->role, fellowNameSpaceCopy->mergeState->role,
                    d_nameSpaceIsMasterConfirmed(nameSpaceCopy));

                result = ( (strcmp(nameSpaceCopy->mergeState->role, fellowNameSpaceCopy->mergeState->role) == 0)  &&   /* my role equals fellow role */
                    (d_networkAddressCompare(masterAddr, fellowMasterAddr) != 0)   &&   /* fellow has a different master than me */
                    (d_nameSpaceIsMasterConfirmed(nameSpaceCopy)) );                    /* my master is confirmed */

                d_printTimedEvent (durability, D_LEVEL_FINEST, "-=- hasMasterConflict evaluates to %d\n", result);


            } else {
                d_printTimedEvent (durability, D_LEVEL_FINEST,
                    "-=- state progress mimatch prior to master checking: " \
                    "myState=%d, fellowState=%d (%d,%d)\n", myState, fellowState, (myState >= D_STATE_DISCOVER_PERSISTENT_SOURCE), (fellowState >= D_STATE_INJECT_PERSISTENT));
            }
            d_networkAddressFree(fellowMasterAddr);
            d_networkAddressFree(masterAddr);

        } else {
            d_printTimedEvent (durability, D_LEVEL_FINEST, "-=- fellow state: %d\n", fellowState);
        }
        d_lockUnlock(d_lock(fellow));
        d_fellowFree(fellow);
    } else {
            d_printTimedEvent (durability, D_LEVEL_FINEST, "-=- fellow with address %u not found\n", fellowAddr->systemId);
    }
    d_networkAddressFree(unAddressed);
    d_printTimedEvent (durability, D_LEVEL_FINER, "HasMasterConflict returns %d\n", (int)result);
    return result;
}


/**
 * \brief Verify if there is a native state conflict between the namespace and the fellowNameSpace.
 *
 * @return TRUE if there is a native state conflict, FALSE otherwise.
 */
static c_bool
d_conflictMonitorHasNativeStateConflict(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr,
    d_nameSpace nameSpaceCopy,
    d_nameSpace fellowNameSpaceCopy)
{
    d_networkAddress masterAddr, fellowMasterAddr;
    d_durability durability;
    d_admin admin;
    d_fellow fellow;
    d_serviceState myState, fellowState;
    d_configuration config;
    c_bool result = FALSE;
    d_mergeState myMergeState, fellowMergeState;

    assert(d_conflictMonitorIsValid(conflictMonitor));

    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    myState = d_durabilityGetState(durability);
    config = d_durabilityGetConfiguration(durability);

    /* Get the confirmed fellow with the fellowAddr */
    fellow = d_adminGetFellow(admin, fellowAddr);
    if (fellow) {
        d_lockLock(d_lock(fellow));
        fellowState = d_fellowGetState(fellow);
        if ( (fellowState != D_STATE_TERMINATING) && (fellowState != D_STATE_TERMINATED) ) {
            masterAddr = d_nameSpaceGetMaster(nameSpaceCopy);
            fellowMasterAddr = d_nameSpaceGetMaster(fellowNameSpaceCopy);
            /* Only check for conflicts if I'm complete and fellow is past injecting persistent data. */
            if ( ( (myState >= D_STATE_DISCOVER_PERSISTENT_SOURCE) && (fellowState >= D_STATE_INJECT_PERSISTENT) ) ) {

                myMergeState = d_nameSpaceGetMergeState(nameSpaceCopy, config->role);
                fellowMergeState = d_nameSpaceGetMergeState(fellowNameSpaceCopy, fellow->role);
                /* A native state conflict occurs if the fellow and myself
                 * have the same role and the fellow is a confirmed
                 * master for the namespace, but the fellow has advertised
                 * a higher namespace state than I am aware of. In that case
                 * I need to request data from my master in order to update
                 * my namespace state.
                 *
                 * There is no need to require that my master is confirmed.
                 * In fact, doing so might lead to the situation that the
                 * fellow send its new confirmed namespace state while I
                 * still have not confirmed the fellow. As a consequence
                 * no native state conflict would be detected until the
                 * fellow re-sends it namespace state and I have confirmed
                 * that the fellow is master. To prevent this we do not
                 * require that I have confirmed the fellow as master. */

                result = ( (d_networkAddressCompare(masterAddr, fellowAddr) == 0) &&                          /* fellow is my master for the namespace */
                           (d_networkAddressCompare(fellowMasterAddr, fellowAddr) == 0) &&                    /* the fellow's master is the fellow itself */
                           (d_nameSpaceIsMasterConfirmed(fellowNameSpaceCopy)) &&                             /* the fellow's master is confirmed */
                           (strcmp(config->role, fellow->role) == 0) &&                                       /* fellow has the same role as me */
                           (d_mergeStateRoleValueCompare(myMergeState, fellowMergeState) == -1) );            /* fellow has a higher native merge state than me */

                d_mergeStateFree(myMergeState);
                d_mergeStateFree(fellowMergeState);
            }
            d_networkAddressFree(fellowMasterAddr);
            d_networkAddressFree(masterAddr);
        }
        d_lockUnlock(d_lock(fellow));
        d_fellowFree(fellow);
    }
    return result;
}


/**
 * \brief Verify if there is a foreign state conflict between the namespace and the fellowNameSpace.
 *
 * @return TRUE if there is a foreign state conflict, FALSE otherwise.
 */
static c_bool
d_conflictMonitorHasForeignStateConflict(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr,
    d_nameSpace nameSpaceCopy,
    d_nameSpace fellowNameSpaceCopy)
{
    d_networkAddress masterAddr, fellowMasterAddr, myAddr;
    d_durability durability;
    d_admin admin;
    d_fellow fellow;
    d_serviceState myState, fellowState;
    d_configuration config;
    d_mergeState mergeState, fellowMergeState;
    c_bool result = FALSE;

    assert(d_conflictMonitorIsValid(conflictMonitor));

    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    myState = d_durabilityGetState(durability);
    config = d_durabilityGetConfiguration(durability);
    myAddr = d_adminGetMyAddress(admin);

    /* Get the confirmed fellow with the fellowAddr */
    fellow = d_adminGetFellow(admin, fellowAddr);
    if (fellow) {
        d_lockLock(d_lock(fellow));
        fellowState = d_fellowGetState(fellow);
        if ( (fellowState != D_STATE_TERMINATING) && (fellowState != D_STATE_TERMINATED) ) {
            masterAddr = d_nameSpaceGetMaster(nameSpaceCopy);
            fellowMasterAddr = d_nameSpaceGetMaster(fellowNameSpaceCopy);
             /* Only check for conflicts if I'm complete and fellow is past injecting persistent data. */
            if ( ( (myState >= D_STATE_DISCOVER_PERSISTENT_SOURCE) && (fellowState >= D_STATE_INJECT_PERSISTENT) ) ) {

                /* A foreign state conflict occurs if I am master confirmed
                 * master for the namespace in my own role, the fellow is
                 * confirmed master for the namespace in a different role,
                 * and the native state of the namespace as advertised by the
                 * fellow differs from my knowledge of the namespace state
                 * in the fellow's role. */

                result = ( (d_networkAddressCompare(masterAddr, myAddr) == 0) &&             /* I am master for the namespace */
                           (d_nameSpaceIsMasterConfirmed(nameSpaceCopy)) &&                  /* I have a confirmed master */
                           (d_networkAddressCompare(fellowMasterAddr, fellowAddr) == 0) &&   /* the fellow is also master for the namespace */
                           (d_nameSpaceIsMasterConfirmed(fellowNameSpaceCopy)) &&            /* the fellow master is confirmed */
                           (strcmp(config->role, fellow->role) != 0) );                      /* fellow has a different role than me */

                if (result) {
                    /* Now check if my mergeState for the namespace in the fellow's role
                     * differs from the one advertised by the fellow. */
                    mergeState = d_nameSpaceGetMergeState(nameSpaceCopy, fellow->role);
                    fellowMergeState = d_nameSpaceGetMergeState(fellowNameSpaceCopy, fellow->role);
                    if ( (mergeState != NULL) && (fellowMergeState != NULL) ) {
                        result = (mergeState->value < fellowMergeState->value);             /* Fellow merge state is higher than my merge state */
                    } else {
                        /* Only foreign conflict if the fellow has a real merge state
                         * and I have not. */
                        result = (fellowMergeState != NULL);
                    }
                    d_mergeStateFree(mergeState);
                    d_mergeStateFree(fellowMergeState);
                }
            }
            d_networkAddressFree(fellowMasterAddr);
            d_networkAddressFree(masterAddr);
        }
        d_lockUnlock(d_lock(fellow));
        d_fellowFree(fellow);
    }
    d_networkAddressFree(myAddr);

    return result;
}


static c_bool
d_conflictMonitorHasFellowDisconnectedConflict(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr)
{
    d_durability durability;
    d_admin admin;
    d_fellow fellow;
    d_serviceState myState, fellowState;
    c_bool result = FALSE;

    assert(d_conflictMonitorIsValid(conflictMonitor));

    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    myState = d_durabilityGetState(durability);
    fellow = d_adminGetFellow(admin, fellowAddr);
    if (fellow) {
        d_lockLock(d_lock(fellow));
        fellowState = d_fellowGetState(fellow);
        if ( (fellowState != D_STATE_TERMINATING) && (fellowState != D_STATE_TERMINATED) &&
             (myState != D_STATE_TERMINATING) && (myState != D_STATE_TERMINATED)   ) {

            /* A fellow disconnect conflict has occurred if a fellow has been disconnected
             * from me. */
            result = TRUE;

        }
        d_lockUnlock(d_lock(fellow));
        d_fellowFree(fellow);
    }
    return result;
}


static c_bool
d_conflictMonitorHasFellowConnectedConflict(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr)
{
    d_durability durability;
    d_admin admin;
    d_fellow fellow;
    d_serviceState myState, fellowState;
    c_bool result = FALSE;

    assert(d_conflictMonitorIsValid(conflictMonitor));

    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    myState = d_durabilityGetState(durability);
    fellow = d_adminGetFellow(admin, fellowAddr);
    if (fellow) {
        d_lockLock(d_lock(fellow));
        fellowState = d_fellowGetState(fellow);
        if ( (fellowState != D_STATE_TERMINATING) && (fellowState != D_STATE_TERMINATED) &&
             (myState != D_STATE_TERMINATING) && (myState != D_STATE_TERMINATED)   ) {

            /* A fellow connect conflict has occurred if a new fellow has recently arrived */
            result = fellow->recently_joined;

        }
        d_lockUnlock(d_lock(fellow));
        d_fellowFree(fellow);
    }
    return result;
}


static c_bool
d_conflictMonitorHasFederationDisconnectedConflict(
    d_conflictMonitor conflictMonitor)
{
    assert(d_conflictMonitorIsValid(conflictMonitor));

    OS_UNUSED_ARG(conflictMonitor);

    /* Always try to resolve this conflict when it appears */
    return TRUE;
}

/**
 * \brief Verify if a master conflict for the namespace and the fellowNameSpace still exists.
 *
 * @return TRUE if the master conflict still exists, FALSE otherwise.
 */
static c_bool
d_conflictMonitorMasterConflictStillExists(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr,
    d_nameSpace nameSpaceCopy,
    d_nameSpace fellowNameSpaceCopy)
{
    d_networkAddress masterAddr, fellowMasterAddr;
    d_durability durability;
    d_admin admin;
    d_fellow fellow;
    d_serviceState myState, fellowState;
    c_bool result = FALSE;

    assert(d_conflictMonitorIsValid(conflictMonitor));

    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    myState = d_durabilityGetState(durability);
    fellow = d_adminGetFellow(admin, fellowAddr);
    if (fellow) {
        d_lockLock(d_lock(fellow));
        fellowState = d_fellowGetState(fellow);
        if ( (fellowState != D_STATE_TERMINATING) && (fellowState != D_STATE_TERMINATED) ) {
            masterAddr = d_nameSpaceGetMaster(nameSpaceCopy);
            fellowMasterAddr = d_nameSpaceGetMaster(fellowNameSpaceCopy);
            /* Only check for conflicts if I'm complete and fellow is past injecting persistent data. */
            if ( ( (myState >= D_STATE_DISCOVER_PERSISTENT_SOURCE) && (fellowState >= D_STATE_INJECT_PERSISTENT) ) ) {

                /* The master conflict still exists if the nameSpaces and the fellowNameSpace
                 * do not have the same confirmed master. */
                result = ( ! ( (d_networkAddressCompare(masterAddr, fellowMasterAddr) == 0) &&   /* fellow has the same master than me */
                               (d_nameSpaceIsMasterConfirmed(nameSpaceCopy)) &&                  /* my master is confirmed */
                               (d_nameSpaceIsMasterConfirmed(fellowNameSpaceCopy)) ) );          /* the fellow's master is confirmed */
            }
            d_networkAddressFree(fellowMasterAddr);
            d_networkAddressFree(masterAddr);
        }
        d_lockUnlock(d_lock(fellow));
        d_fellowFree(fellow);
    } else {
        /* The fellow left, but because my namespace has been set to unconfirmed
         * so I still must determine my master */
        result = TRUE;
    }
    return result;
}


/**
 * \brief Verify if the native state conflict for the namespace and the fellowNameSpace still exists.
 *
 * @return TRUE if the native state conflict still exists, FALSE otherwise.
 */
static c_bool
d_conflictMonitorNativeStateConflictStillExists(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr,
    d_nameSpace nameSpaceCopy,
    d_nameSpace fellowNameSpaceCopy)
{
    return  d_conflictMonitorHasNativeStateConflict(conflictMonitor, fellowAddr, nameSpaceCopy, fellowNameSpaceCopy);
}


/**
 * \brief Verify if the foreign state conflict between the namespace and the fellowNameSpace still exists.
 *
 * @return TRUE if the foreign state conflict still exists, FALSE otherwise.
 */
static c_bool
d_conflictMonitorForeignStateConflictStillExists(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr,
    d_nameSpace nameSpaceCopy,
    d_nameSpace fellowNameSpaceCopy)
{
    return d_conflictMonitorHasForeignStateConflict(conflictMonitor, fellowAddr, nameSpaceCopy, fellowNameSpaceCopy);
}



/**
 * \brief Verify if a federation disconnected conflict still exists.
 *
 * @return TRUE if the federation disconnected conflict still exists, FALSE otherwise.
 */
static c_bool
d_conflictMonitorFederationDisconnectedConflictStillExists(
    d_conflictMonitor conflictMonitor)
{
    return d_conflictMonitorHasFederationDisconnectedConflict(conflictMonitor);
}



/**
 * \brief Verify if a fellow disconnected conflict still exists.
 *
 * @return TRUE if the fellow disconnected conflict still exists, FALSE otherwise.
 */
static c_bool
d_conflictMonitorFellowDisconnectedConflictStillExists(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr)
{
    return d_conflictMonitorHasFellowDisconnectedConflict(conflictMonitor, fellowAddr);
}


/**
 * \brief Verify if a fellow connected conflict still exists.
 *
 * @return TRUE if the fellow connected conflict still exists, FALSE otherwise.
 */
static c_bool
d_conflictMonitorFellowConnectedConflictStillExists(
    d_conflictMonitor conflictMonitor,
    d_networkAddress fellowAddr)
{
    return d_conflictMonitorHasFellowConnectedConflict(conflictMonitor, fellowAddr);
}


/**
 * \brief Check whether I have a conflict with the fellow for the namespace
 *
 * The namespace provided is the namespace as received from the fellow
 *
 * @param conflictMonitor    This monitor used to check for conflicts
 * @param fellow             The fellow in conflict
 * @param nameSpace          The namespace in conflict
 */
void
d_conflictMonitorCheckForConflicts(
    d_conflictMonitor conflictMonitor,
    d_fellow fellow,
    d_nameSpace nameSpace)
{
    d_conflict conflict = NULL;
    d_admin admin;
    d_durability durability;
    d_subscriber subscriber;
    d_nameSpacesRequestListener nsrListener;
    d_networkAddress fellowAddr, masterAddr, unAddressed;
    d_nameSpace fellowNameSpace, nameSpaceCopy, fellowNameSpaceCopy;
    c_bool use_legacy;

    assert(d_conflictMonitorIsValid(conflictMonitor));

    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    subscriber = d_adminGetSubscriber(admin);
    nsrListener = d_subscriberGetNameSpacesRequestListener(subscriber);
    if (fellow && nameSpace) {
        /* We do NOT want to check for conflicts immediately after the fellow
         * has joined, because in that case we want the fellow to slave to
         * existing masters. Only when the fellow has progressed "far enough"
         * we may conclude that a conflict has occurred. We also do not want
         * to check for conflicts if I have not progressed far enough. */
        masterAddr = d_nameSpaceGetMaster(nameSpace);
        unAddressed = d_networkAddressUnaddressed();

        traceFellow (durability, "fellow", fellow);
        traceNameSpace (durability, "nameSpace", nameSpace);

        if ( ( (d_durabilityGetState(durability) >= D_STATE_INJECT_PERSISTENT) &&   /* I have progressed far enough */
               (d_fellowGetState(fellow) >= D_STATE_DISCOVER_PERSISTENT_SOURCE) &&  /* The fellow has progressed far enough */
               (d_fellowIsConfirmed(fellow) ) ) ||                                  /* The fellow is confirmed                   or */
             ( (d_networkAddressCompare(masterAddr, unAddressed) == 0) &&           /* I have an unaddressed master */
               (d_nameSpaceIsMasterConfirmed(nameSpace)) &&                         /* My master is confirmed */
               (d_fellowIsConfirmed(fellow)) ) ) {                                  /* The fellow is confirmed */
            d_printTimedEvent(durability, D_LEVEL_FINER,
                "Checking for conflicts in namespace '%s' with confirmed fellow %u\n",
                nameSpace->name, fellow->address->systemId);
            d_lockLock(d_lock(conflictMonitor));
            conflictMonitor->lastTimeChecked = os_timeMGet();
            fellowAddr = d_fellowGetAddress(fellow);
            /* Get a copy of nameSpace and the corresponding nameSpace for the fellow */
            nameSpaceCopy = d_nameSpaceCopy(nameSpace);
            fellowNameSpace = d_fellowGetNameSpace(fellow, nameSpace);
            if (fellowNameSpace) {
                fellowNameSpaceCopy = d_nameSpaceCopy(fellowNameSpace);

                traceNameSpace (durability, "fellowNameSpaceCopy", fellowNameSpaceCopy);
                /* Check for a master conflict. */
                if  (d_conflictMonitorHasMasterConflict(conflictMonitor, fellowAddr, nameSpaceCopy, fellowNameSpaceCopy)) {
                    conflict = d_conflictNew(D_CONFLICT_NAMESPACE_MASTER, fellowAddr, nameSpaceCopy, fellowNameSpaceCopy);
                    if (!d_conflictResolverConflictExists(admin->conflictResolver, conflict)) {
                        traceConflict (durability, "new conflict", conflict);
                        d_conflictSetId(conflict, durability);
                        use_legacy = IS_LEGACY_MASTER_SELECTION(d_nameSpaceGetMasterPriority(nameSpace));
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                            "Master conflict in namespace '%s' for fellow %u detected, conflict %d created (use_legacy: %d)\n",
                            nameSpace->name, fellow->address->systemId, conflict->id, use_legacy);
                        if (use_legacy) {
                            /* Re-publish my namespaces to let other fellows know that
                             * there was a conflict, and unconfirm mastership for the
                             * conflicting namespace in case of the legacy algorithm.
                             * This prevents that the same conflict
                             * is generated again.
                             * TODO: An optimization would be to broadcast the conflicting
                             * namespace iso all namespaces. */
                            d_nameSpacesRequestListenerReportNameSpaces(nsrListener);
                            d_nameSpaceMasterPending(nameSpace);
                        }
                    } else {
                        traceConflict (durability, "dropping temp conflict: already pending", conflict);
                        d_conflictFree(conflict);
                        conflict = NULL;
                    }
                }

                /* Check for a native state conflict */
                else if (d_conflictMonitorHasNativeStateConflict(conflictMonitor, fellowAddr, nameSpaceCopy, fellowNameSpaceCopy)) {
                    conflict = d_conflictNew(D_CONFLICT_NAMESPACE_NATIVE_STATE, fellowAddr, nameSpaceCopy, fellowNameSpaceCopy);
                    if (!d_conflictResolverConflictExists(admin->conflictResolver, conflict)) {
                        d_conflictSetId(conflict, durability);
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                            "Native state conflict in namespace '%s' for fellow %u detected, conflict %d created\n",
                            nameSpace->name, fellow->address->systemId, conflict->id);
                    } else {
                        d_conflictFree(conflict);
                        conflict = NULL;
                    }
                }

                /* Check for a foreign state conflict */
                else if (d_conflictMonitorHasForeignStateConflict(conflictMonitor, fellowAddr, nameSpaceCopy, fellowNameSpaceCopy)) {
                    conflict = d_conflictNew(D_CONFLICT_NAMESPACE_FOREIGN_STATE, fellowAddr, nameSpaceCopy, fellowNameSpaceCopy);
                    if (!d_conflictResolverConflictExists(admin->conflictResolver, conflict)) {
                        d_conflictSetId(conflict, durability);
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                            "Foreign state conflict in namespace '%s' for fellow %u detected, conflict %d created\n",
                            nameSpace->name, fellow->address->systemId, conflict->id);
                    } else {
                        d_conflictFree(conflict);
                        conflict = NULL;
                    }
                }

                /* Add conflict to the conflictResolver */
                if (conflict) {
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Adding conflict %d to the conflict resolver queue\n",
                        conflict->id);
                    d_conflictResolverAddConflict(admin->conflictResolver, conflict);
                }
                d_nameSpaceFree(fellowNameSpaceCopy);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST, "no matching fellow nameSpace\n");
            }

            d_nameSpaceFree(nameSpaceCopy);
            d_networkAddressFree(fellowAddr);
            d_lockUnlock(d_lock(conflictMonitor));
        }
        d_networkAddressFree(unAddressed);
        d_networkAddressFree(masterAddr);
    }
}


void d_conflictMonitorCheckFellowDisconnected(
    d_conflictMonitor conflictMonitor,
    d_fellow fellow)
{
    d_networkAddress fellowAddr;
    d_conflict conflict;
    d_admin admin;
    d_durability durability;

    assert(d_conflictMonitorIsValid(conflictMonitor));
    assert(d_fellowIsValid(fellow));

    /* Schedule the conflict.
     * When the conflict is about to be resolved we are going to check whether the conditions
     * for the conflict apply. */
    fellowAddr = d_fellowGetAddress(fellow);
    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    if (d_conflictMonitorHasFellowDisconnectedConflict(conflictMonitor, fellowAddr)) {
        conflict = d_conflictNew(D_CONFLICT_FELLOW_DISCONNECTED, fellowAddr, NULL, NULL);
        if (!d_conflictResolverConflictExists(admin->conflictResolver, conflict)) {
            d_conflictSetId(conflict, durability);
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Fellow disconnect conflict for fellow %u detected, conflict %d created\n",
                fellow->address->systemId, conflict->id);

            d_printTimedEvent(durability, D_LEVEL_FINER,
                "Adding conflict %d to the conflict resolver queue\n",
                conflict->id);

            d_conflictResolverAddConflict(admin->conflictResolver, conflict);
        } else {
            d_conflictFree(conflict);
            conflict = NULL;
        }
    }
    d_networkAddressFree(fellowAddr);
}


void d_conflictMonitorCheckFellowConnected(
    d_conflictMonitor conflictMonitor,
    d_fellow fellow)
{
    d_networkAddress fellowAddr;
    d_conflict conflict;
    d_admin admin;
    d_durability durability;

    assert(d_conflictMonitorIsValid(conflictMonitor));
    assert(d_fellowIsValid(fellow));

    /* Schedule the conflict.
     * When the conflict is about to be resolved we are going to check whether the conditions
     * for the conflict apply. */
    fellowAddr = d_fellowGetAddress(fellow);
    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);

    if (d_conflictMonitorHasFellowConnectedConflict(conflictMonitor, fellowAddr)) {
        conflict = d_conflictNew(D_CONFLICT_FELLOW_CONNECTED, fellowAddr, NULL, NULL);
        if (!d_conflictResolverConflictExists(admin->conflictResolver, conflict)) {
            d_conflictSetId(conflict, durability);
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Fellow connect conflict for fellow %u detected, conflict %d created\n",
                fellow->address->systemId, conflict->id);

            d_printTimedEvent(durability, D_LEVEL_FINER,
                "Adding conflict %d to the conflict resolver queue\n",
                conflict->id);

            d_conflictResolverAddConflict(admin->conflictResolver, conflict);
        } else {
            d_conflictFree(conflict);
            conflict = NULL;
        }
    }
    d_networkAddressFree(fellowAddr);
}


void d_conflictMonitorCheckFederationDisconnected(
    d_conflictMonitor conflictMonitor)
{
    d_conflict conflict;
    d_admin admin;
    d_durability durability;

    assert(d_conflictMonitorIsValid(conflictMonitor));

    /* Schedule the conflict.
     * When the conflict is about to be resolved we are going to check whether the conditions
     * for the conflict apply. */
    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);
    if (d_conflictMonitorHasFederationDisconnectedConflict(conflictMonitor)) {
        conflict = d_conflictNew(D_CONFLICT_FEDERATION_DISCONNECTED, NULL, NULL, NULL);
        if (!d_conflictResolverConflictExists(admin->conflictResolver, conflict)) {
            d_conflictSetId(conflict, durability);
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Federation disconnect conflict detected, conflict %d created\n",
                conflict->id);

            d_printTimedEvent(durability, D_LEVEL_FINER,
                "Adding conflict %d to the conflict resolver queue\n",
                conflict->id);

            d_conflictResolverAddConflict(admin->conflictResolver, conflict);
        } else {
            d_conflictFree(conflict);
            conflict = NULL;
        }
    }
}


/**
 * \brief Re-evaluate the conflict.
 *
 * It is checked if the conflict still exists. During conflict evaluation
 * the nameSpaceCopy field and fellowNameSpaceCopy field is updated with
 * the latest information.
 *
 * @return The updated conflict if the conflict still exists, or NULL otherwise
 */
d_conflict
d_conflictMonitorEvaluateConflict(
    d_conflictMonitor conflictMonitor,
    d_conflict conflict)
{
    d_admin admin;
    d_durability durability;
    d_fellow fellow;
    d_nameSpace nameSpace, fellowNameSpace, nameSpaceCopy, fellowNameSpaceCopy;
    c_bool updated = FALSE;
    c_bool result = TRUE;

    assert(d_conflictMonitorIsValid(conflictMonitor));
    assert(d_conflictIsValid(conflict));

    admin = conflictMonitor->admin;
    durability = d_adminGetDurability(admin);

    if (conflict->fellowAddr == NULL) {
        /* The conflict is independent of a fellow.
         * This conflict is never discarded and by default updated */
        return conflict;
    }
    fellow = d_adminGetFellow(admin, conflict->fellowAddr);
    if (fellow) {
        /* Get a copy of the latest state of my namespace and the fellow's namespace */
        nameSpace = (conflict->nameSpaceCopy == NULL) ? NULL : d_adminGetNameSpace(admin, conflict->nameSpaceCopy->name);
        fellowNameSpace = (nameSpace == NULL) ? NULL : d_fellowGetNameSpace(fellow, nameSpace);
        nameSpaceCopy = (nameSpace == NULL) ? NULL : d_nameSpaceCopy(nameSpace);
        fellowNameSpaceCopy = (fellowNameSpace == NULL) ? NULL : d_nameSpaceCopy(fellowNameSpace);

        if (IS_NAMESPACE_CONFLICT(conflict)) {
            /* For namespace related conflicts both copies must exist
             * in order to resolve the conflict */
            if (nameSpaceCopy == NULL) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Could not find a local namespace with name '%s', discarding conflict %d\n",
                    conflict->nameSpaceCopy->name, conflict->id);
                result = FALSE;
            } else if (fellowNameSpaceCopy == NULL) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Could not find namespace '%s' for fellow %u, discarding conflict %d\n",
                    conflict->nameSpaceCopy->name, conflict->fellowAddr->systemId, conflict->id);
                result = FALSE;
            }
        }

        if (result) {
            /* Now check if the conflict still exists */
            d_lockLock(d_lock(conflictMonitor));
            conflictMonitor->lastTimeChecked = os_timeMGet();
            switch (conflict->event) {
                case D_CONFLICT_FEDERATION_DISCONNECTED:
                    if (d_conflictMonitorFederationDisconnectedConflictStillExists(conflictMonitor)) {
                        d_conflictUpdate(conflict, NULL, NULL);
                        updated = TRUE;
                    }
                    break;
                case D_CONFLICT_HEARTBEAT_PROCESSED:
                    updated = TRUE;
                    break;
                case D_CONFLICT_FELLOW_DISCONNECTED:
                    if (d_conflictMonitorFellowDisconnectedConflictStillExists(conflictMonitor, conflict->fellowAddr)) {
                        d_conflictUpdate(conflict, NULL, NULL);
                        updated = TRUE;
                    }
                    break;
                case D_CONFLICT_FELLOW_CONNECTED:
                    if (d_conflictMonitorFellowConnectedConflictStillExists(conflictMonitor, conflict->fellowAddr)) {
                        d_conflictUpdate(conflict, NULL, NULL);
                        updated = TRUE;
                    }
                    break;
                case D_CONFLICT_NAMESPACE_MASTER :
                    if (d_conflictMonitorMasterConflictStillExists(conflictMonitor, conflict->fellowAddr, nameSpaceCopy, fellowNameSpaceCopy)) {
                        d_conflictUpdate(conflict, nameSpaceCopy, fellowNameSpaceCopy);
                        updated = TRUE;
                    }
                    break;
                case D_CONFLICT_NAMESPACE_NATIVE_STATE :
                    if (d_conflictMonitorNativeStateConflictStillExists(conflictMonitor, conflict->fellowAddr, nameSpaceCopy, fellowNameSpaceCopy)) {
                        d_conflictUpdate(conflict, nameSpaceCopy, fellowNameSpaceCopy);
                        updated = TRUE;
                    }
                    break;
                case D_CONFLICT_NAMESPACE_FOREIGN_STATE :
                    if (d_conflictMonitorForeignStateConflictStillExists(conflictMonitor, conflict->fellowAddr, nameSpaceCopy, fellowNameSpaceCopy)) {
                        d_conflictUpdate(conflict, nameSpaceCopy, fellowNameSpaceCopy);
                        updated = TRUE;
                    }
                    break;
                default :
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Invalid conflict event (%d), discarding the conflict\n", conflict->event);
            }
            d_lockUnlock(d_lock(conflictMonitor));
        }

        if (nameSpaceCopy) {
            d_nameSpaceFree(nameSpaceCopy);
        }
        if (fellowNameSpaceCopy) {
            d_nameSpaceFree(fellowNameSpaceCopy);
        }
        if (nameSpace) {
            d_nameSpaceFree(nameSpace);
        }
        d_fellowFree(fellow);
    } else {
        /* The fellow does not exist any more. In case of a master conflict I have
         * unconfirmed my master, so I must look for a new master anyway */

        /* Get a copy of the latest state of my namespace and the fellow's namespace */
        nameSpace = (conflict->nameSpaceCopy == NULL) ? NULL : d_adminGetNameSpace(admin, conflict->nameSpaceCopy->name);
        nameSpaceCopy = (nameSpace == NULL) ? NULL : d_nameSpaceCopy(nameSpace);

        /* Now check if the conflict still exists */
        d_lockLock(d_lock(conflictMonitor));
        conflictMonitor->lastTimeChecked = os_timeMGet();
        switch (conflict->event) {
            case D_CONFLICT_NAMESPACE_MASTER :
                if (d_conflictMonitorMasterConflictStillExists(conflictMonitor, conflict->fellowAddr, nameSpaceCopy, NULL)) {
                    d_conflictUpdate(conflict, nameSpaceCopy, NULL);
                    updated = TRUE;
                }
                break;
                /* For the following conflicts the disappearing of the fellow is not relevant  */
            case D_CONFLICT_FELLOW_DISCONNECTED:
            case D_CONFLICT_FELLOW_CONNECTED:
            case D_CONFLICT_NAMESPACE_NATIVE_STATE :
            case D_CONFLICT_NAMESPACE_FOREIGN_STATE :
                break;
            case D_CONFLICT_HEARTBEAT_PROCESSED :
                updated = TRUE;
                break;
            default :
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Invalid conflict event (%d), discarding the conflict\n", conflict->event);
        }
        d_lockUnlock(d_lock(conflictMonitor));

        if (nameSpaceCopy) {
            d_nameSpaceFree(nameSpaceCopy);
        }
        if (nameSpace) {
            d_nameSpaceFree(nameSpace);
        }
    }
    return (updated ? conflict : NULL);
}
