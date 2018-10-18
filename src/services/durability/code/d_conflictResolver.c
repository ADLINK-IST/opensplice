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

#include "d__conflictResolver.h"
#include "d__conflictMonitor.h"
#include "d__conflict.h"
#include "d__thread.h"
#include "d__conflict.h"
#include "d__nameSpace.h"
#include "d__groupLocalListener.h"
#include "d__mergeState.h"
#include "d__configuration.h"
#include "d__fellow.h"
#include "d__nameSpace.h"
#include "d__misc.h"
#include "d__groupLocalListener.h"
#include "d__subscriber.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__mergeAction.h"
#include "d__eventListener.h"
#include "d__nameSpacesRequestListener.h"
#include "d__sampleChainListener.h"
#include "d_object.h"
#include "os_time.h"
#include "os_heap.h"

/**
 * \brief Helper structure to collect all fellow aligners with the
 *        specified role.
 */
struct collectAlignableFellowsHelper {
    d_durability durability;
    d_nameSpace nameSpace;
    c_iter fellows;
    char *role;
    c_ulong maxValue;
};

/**
 * \brief Collect all fellow aligners that have the same role as specified
 *        in the helper, and collect the maximum value of the merge states.
 *
 * Only fellows are selected that are complete and for which all namespaces
 * have been received.
 */
static c_bool
collectAlignableFellows (
    d_fellow fellow,
    c_voidp userData)
{
    struct collectAlignableFellowsHelper *helper;
    d_serviceState fellowState;
    d_durability durability;
    d_mergeState mergeState;
    d_nameSpace fellowNs;

    helper = (struct collectAlignableFellowsHelper *)userData;
    durability = helper->durability;
    if (fellow->role && strcmp(fellow->role, helper->role) == 0) {
        fellowState = d_fellowGetState(fellow);

        if ((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Fellow %u terminated, so it is not no candidate aligner.\n",
                        fellow->address->systemId);

        } else if (fellowState != D_STATE_COMPLETE) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Fellow %u is not yet complete, so it is no candidate aligner\n",
                        fellow->address->systemId);

        } else if (!d_fellowAreNameSpacesComplete(fellow)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Not all namespaces of fellow %u are received yet, so it is no candidate aligner\n",
                        fellow->address->systemId);

        } else {
            /* Check if the fellow is aligner for the namespace. If so,
             * the fellow is a candidate to request samples from. Add
             * it to the fellows list and determine the maximum mergeState
             * value.  The fellows are refCounted. They are lateron added
             * to the mergeAction and freed when the mergeAction is
             * destroyed.
             */
            fellowNs = d_fellowGetNameSpace(fellow, helper->nameSpace);
            if (d_nameSpaceIsAligner(fellowNs)) {
                if (d_nameSpaceIsAlignable(fellowNs)) {
                    helper->fellows = c_iterAppend(helper->fellows, d_fellow(d_objectKeep(d_object(fellow))));
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Fellow %u is a candidate aligner\n",
                                fellow->address->systemId);
                    /* Calculate the new state value.
                     * Only take non-cleared states into account.
                     */
                    if ((mergeState = d_nameSpaceGetMergeState(fellowNs, fellow->role)) != NULL) {
                        if (mergeState->value > helper->maxValue) {
                            helper->maxValue = mergeState->value;
                        }
                        d_mergeStateFree(mergeState);
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Fellow %u is not marked as alignable, so it is no candidate aligner\n",
                                fellow->address->systemId);
                }
            }
        }
    }
    return TRUE;
}

static void
fellowsFree(
    c_iter fellows)
{
    d_fellow fellow;
    assert(fellows);
    fellow = c_iterTakeFirst(fellows);
    while (fellow) {
        d_fellowFree(fellow);
        fellow = c_iterTakeFirst(fellows);
    }
    c_iterFree(fellows);
}

static c_bool
resolveMasterConflict(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_nameSpace nameSpace, nameSpaceCopy, fellowNameSpace;
    c_iter nameSpaces, fellows;
    d_groupLocalListener listener;
    d_networkAddress myAddr, masterAddr, unAddressed;
    d_admin admin;
    d_durability durability;
    d_configuration config;
    d_fellow fellow;
    d_mergeState newState;
    c_bool reschedule = FALSE;
    struct collectAlignableFellowsHelper helper;
    d_mergeState fellowMergeState;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_MASTER_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    d_printTimedEvent(durability, D_LEVEL_FINER,
            "Resolving master conflict %u for namespace '%s' (run: %u)\n",
            conflict->id, conflict->nameSpaceCopy->name, conflict->reQueueCount);

    myAddr = d_adminGetMyAddress(admin);
    unAddressed = d_networkAddressUnaddressed();
    listener = d_subscriberGetGroupLocalListener(d_adminGetSubscriber(admin));
    nameSpace = d_adminGetNameSpace(admin, conflict->nameSpaceCopy->name);
    if (nameSpace) {
        /* A master conflict needs to be resolved.
         * Determine a new master for the namespace.
         */
        nameSpaces = c_iterNew(nameSpace);
        d_groupLocalListenerDetermineNewMasters(listener, nameSpaces);
        c_iterFree(nameSpaces);

        /* At this point in time we should have a confirmed master, but it may
         * not be me. In fact, in may not even be the fellow, e.g., when
         * multiple nodes have competed for mastership. So let's find out who the
         * confirmed master for the nameSpace is. If it turns out that I have
         * become master then I must request samples from all other fellows that
         * participated in the master conflicts.
         */
        masterAddr = d_nameSpaceGetMaster(nameSpace);
        fellow = d_adminGetFellow(admin, masterAddr);

        /* A fellow has become master */
        if (fellow) {
            if ( (d_networkAddressCompare(masterAddr, myAddr) != 0) &&       /* somebody else is master for the namespace */
                 (d_networkAddressCompare(masterAddr, unAddressed) != 0) &&  /* this somebody is not (0,0,0) */
                 (d_nameSpaceIsMasterConfirmed(nameSpace)) &&                /* the master is confirmed */
                 (strcmp(config->role, fellow->role) == 0) ) {               /* the master has the same role as me */

                if (d_nameSpaceGetMergePolicy(nameSpace, config->role) != D_MERGE_IGNORE) {

                    fellowNameSpace = d_fellowGetNameSpace(fellow, nameSpace);
                    if (fellowNameSpace) {
                        fellowMergeState = d_nameSpaceGetMergeState(fellowNameSpace, fellow->role);
                        /* The fellow might have published transient data without having raised its state.
                         * Therefore, I must send a request for data.
                         * Only retrieve if the fellow state exists
                         */
                        if (fellowMergeState) {
                            fellows = c_iterNew(d_fellow(d_objectKeep(d_object(fellow))));

                            nameSpaceCopy = d_nameSpaceCopy(nameSpace);
                            handleMergeAlignment(listener, conflict, nameSpaceCopy, fellows, fellowMergeState);
                            d_nameSpaceFree(nameSpaceCopy);

                            d_printTimedEvent(durability, D_LEVEL_INFO,
                                 "Fellow %u has become master when resolving conflict %d, I am going to retrieve data.\n",
                                 fellow->address->systemId, conflict->id);

                            fellowsFree(fellows);

                            d_mergeStateFree(fellowMergeState);
                        }
                    }
                } else {
                    /* I have an IGNORE merge policy so I better not request data */
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                        "Fellow %u has become master when resolving conflict %d, but I have an IGNORE merge policy so I will not request any data.\n",
                        fellow->address->systemId, conflict->id);
                }
            } else {
                d_printTimedEvent(durability, D_LEVEL_INFO,
                    "No need to merge with fellow %u as a result of solving conflict %d\n",
                    fellow->address->systemId, conflict->id);
            }
            d_fellowFree(fellow);

        /* I have become master */
        } else if ( (d_networkAddressCompare(masterAddr, myAddr) == 0) &&      /* I have become master */
                    (d_nameSpaceIsMasterConfirmed(nameSpace)) ) {              /* the master is confirmed */

            d_printTimedEvent(durability, D_LEVEL_INFO,
                "I have become master when resolving conflict %d.\n", conflict->id);

            if (d_nameSpaceGetMergePolicy(nameSpace, config->role) != D_MERGE_IGNORE) {
                /* Retrieve all potential fellow aligners that have the same role as me, and
                 * determine the maximum merge state value of all of them.
                 *
                 * Note: currently ALL fellow aligners are collected. An optimized
                 * approach would be to request the former master fellows for the
                 * nameSpace.
                 */

                /* Get a copy of the namespace at this moment in time. */
                nameSpaceCopy = d_nameSpaceCopy(nameSpace);

                /* Collect fellow aligners for the namespace */
                helper.durability = durability;
                helper.fellows = c_iterNew(NULL);
                helper.role = config->role;
                helper.maxValue = (nameSpaceCopy->mergeState == NULL) ? 0 : nameSpaceCopy->mergeState->value;
                helper.nameSpace = nameSpaceCopy;
                d_adminFellowWalk(admin, collectAlignableFellows, &helper);
                if (c_iterLength(helper.fellows) > 0) {
                    /* The new merge state must be higher than the maximum mergeState
                     * value of all potential fellow aligners including myself
                     */
                    newState = d_mergeStateNew(config->role, helper.maxValue + 1);
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                        "I am going to request data for namespace '%s' from %d fellow aligners.\n",
                        d_nameSpaceGetName(nameSpaceCopy), c_iterLength(helper.fellows));
                    /* Request data from the fellows, and set the new merge state when all
                     * fellows have send their data.
                     */
                    handleMergeAlignment(listener, conflict, nameSpaceCopy, helper.fellows, newState);
                    d_mergeStateFree(newState);
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINER, "No fellow aligners available\n");
                }
                d_nameSpaceFree(nameSpaceCopy);
                fellowsFree(helper.fellows);
            } else {
                /* I have an IGNORE merge policy so I better not request data */
                d_printTimedEvent(durability, D_LEVEL_INFO,
                     "I have become master when resolving conflict %d, but I have an IGNORE merge policy so I will not request any data.\n",
                     conflict->id);
            }
        /* No master found */
        } else {
            d_printTimedEvent(durability, D_LEVEL_INFO,
                "No need to request data because there is no master\n");
        }
        d_networkAddressFree(masterAddr);
        d_nameSpaceFree(nameSpace);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINER,
            "Cancelling conflict %d because namespace '%s' could not be found.\n",
            conflict->id, conflict->nameSpaceCopy->name);
    }
    d_networkAddressFree(myAddr);
    d_networkAddressFree(unAddressed);

    return reschedule;
}


static c_bool
resolveNativeStateConflict(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_nameSpace nameSpace, fellowNameSpace, fellowNameSpaceCopy;
    d_fellow fellow;
    d_groupLocalListener listener;
    d_admin admin;
    d_durability durability;
    d_configuration config;
    d_serviceState fellowState;
    d_networkAddress myAddr, masterAddr, fellowAddr, unAddressed;
    d_mergeState newState, mergeState, fellowMergeState;
    d_mergePolicy mergePolicy;
    d_alignmentKind alignmentKind;
    c_bool reschedule = FALSE;
    c_iter fellows;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_NATIVE_STATE_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    d_printTimedEvent(durability, D_LEVEL_FINER,
        "Resolving native state conflict %u for namespace '%s' (run: %u)\n",
        conflict->id, conflict->nameSpaceCopy->name, conflict->reQueueCount);
    /* A native state conflict needs to be resolved.
     * First find out if the native state conflict still exists.
     */
    myAddr = d_adminGetMyAddress(admin);
    unAddressed = d_networkAddressUnaddressed();
    listener = d_subscriberGetGroupLocalListener(d_adminGetSubscriber(admin));
    nameSpace = d_adminGetNameSpace(admin, conflict->nameSpaceCopy->name);
    if (nameSpace) {
        masterAddr = d_nameSpaceGetMaster(nameSpace);
        if (d_networkAddressCompare(masterAddr, conflict->fellowAddr) == 0) {
            /* The fellow is still the master that was detected when the conflict
             * was generated.
             */
            fellow = d_adminGetFellow(admin, conflict->fellowAddr);
            if (fellow) {
                fellowAddr = d_fellowGetAddress(fellow);
                if ( (d_networkAddressCompare(masterAddr, myAddr) != 0) &&       /* somebody else is master for the namespace */
                     (d_networkAddressCompare(masterAddr, unAddressed) != 0) &&  /* this somebody is not (0,0,0) */
                     (d_networkAddressCompare(masterAddr, fellowAddr) == 0)  &&  /* the fellow is master */
                     (strcmp(config->role, fellow->role) == 0) ) {               /* the master has the same role as me */

                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                         "Native state conflict %d for namespace '%s' may still exist, checking further\n",
                         conflict->id, nameSpace->name);

                    fellowState = d_fellowGetState(fellow);
                    fellowNameSpace = d_fellowGetNameSpace(fellow, nameSpace);
                    mergePolicy = d_nameSpaceGetMergePolicy(nameSpace, config->role);
                    alignmentKind = d_nameSpaceGetAlignmentKind(nameSpace);
                    /* Get the merge state for my role */
                    mergeState = d_nameSpaceGetMergeState(nameSpace, config->role);
                    fellowMergeState = d_nameSpaceGetMergeState(fellowNameSpace, config->role);

                    if (!d_nameSpaceIsMasterConfirmed(nameSpace)) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                             "I do not (yet) have a confirmed master, rescheduling conflict %d.\n",
                             conflict->id);
                        reschedule = TRUE;

                    } else if ((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Cancelling conflict %d because fellow %u has terminated.\n",
                            conflict->id, fellow->address->systemId);

                    } else if ((alignmentKind != D_ALIGNEE_INITIAL) && (alignmentKind != D_ALIGNEE_LAZY)) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Cancelling conflict %d because alignee policy does not ask for it.\n",
                            conflict->id);

                    } else if (fellowMergeState == NULL) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "Cancelling conflict %d, unable to merge namespace '%s' of fellow %u because its merge state is NULL\n",
                              conflict->id, nameSpace->name, fellowAddr->systemId);

                    } else if (fellowState < D_STATE_INJECT_PERSISTENT) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "Fellow %u is not yet ready, re-scheduling conflict %d\n",
                              fellow->address->systemId, conflict->id);
                        reschedule = TRUE;

                    } else if (!d_fellowAreNameSpacesComplete(fellow)) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                             "Not all namespaces of fellow %u are received yet, re-scheduling conflict %d\n",
                              fellow->address->systemId, conflict->id);
                        reschedule = TRUE;

                    } else if (nameSpace->mergeState->value != conflict->nameSpaceCopy->mergeState->value) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "The merge state of my namespace '%s' has changed after conflict %d was detected from (%s,%d) to (%s,%d)," \
                            " canceling the conflict",
                            nameSpace->name, conflict->id,
                            conflict->nameSpaceCopy->mergeState->role, conflict->nameSpaceCopy->mergeState->value,
                            nameSpace->mergeState->role, nameSpace->mergeState->value);

                    } else if (fellowNameSpace->mergeState->value != conflict->fellowNameSpaceCopy->mergeState->value) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "The merge state of the namespace '%s' of my master %u has changed after conflict %d was detected from (%s,%d) to (%s,%d)," \
                            " canceling the conflict",
                            fellowNameSpace->name, masterAddr->systemId, conflict->id,
                            conflict->fellowNameSpaceCopy->mergeState->role, conflict->fellowNameSpaceCopy->mergeState->value,
                            fellowNameSpace->mergeState->role, fellowNameSpace->mergeState->value);

                    } else {
                        c_bool doMerge = FALSE;
                        /* Only merge if my state was empty or differs.
                         * There is also no need to merge if the merge policy is IGNORE
                         */
                        if (mergeState != NULL) {
                            doMerge = ((mergeState->value != fellowMergeState->value) && (mergePolicy != D_MERGE_IGNORE));
                        } else {
                            doMerge = TRUE;
                        }
                        if (doMerge) {
                            /* I need to retrieve data from my master and update my
                             * mergestate to that of the master's.
                             */
                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                "Applying merge with my master fellow %u for nameSpace '%s' to solve conflict %d.\n",
                                fellow->address->systemId, d_nameSpaceGetName(nameSpace), conflict->id);
                            /* Get a copy of the masters's nameSpace at this moment in time. */
                            fellowNameSpaceCopy = d_nameSpaceCopy(fellowNameSpace);
                            /* Copy the master's namespace state */
                            newState = d_mergeStateNew(conflict->fellowNameSpaceCopy->mergeState->role, conflict->fellowNameSpaceCopy->mergeState->value);
                            /* A merge is needed with the fellow for the nameSpace. */
                            fellows = c_iterNew(d_fellow(d_objectKeep(d_object(fellow))));
                            handleMergeAlignment(listener, conflict, fellowNameSpaceCopy, fellows, newState);
                            fellowsFree(fellows);
                            d_mergeStateFree(newState);
                            d_nameSpaceFree(fellowNameSpaceCopy);
                        } else if (mergeState->value == fellowMergeState->value) {
                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                "No difference in merge state detected for namespace '%s' from fellow %u, done solving conflict %d\n",
                                nameSpace->name, fellowAddr->systemId, conflict->id);
                        } else {
                            /* The only possibility left for doMerge to be FALSE is that
                             * the merge policy is D_MERGE_IGNORE. In that case there is
                             * no need to align, it is sufficient to take over the fellow's
                             * merge state
                             */
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "No need to merge with fellow %u to solve native state conflict %d because the merge policy is IGNORE\n",
                                fellowAddr->systemId, conflict->id);
                            d_nameSpaceSetMergeState(nameSpace, fellowMergeState);
                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                "Updating merge state for role '%s' to fellow's %u merge state %d, done solving conflict %d\n",
                                fellow->role, fellowAddr->systemId, fellowMergeState->value, conflict->id);
                        }
                    }
                    if (mergeState) {
                        d_mergeStateFree(mergeState);
                    }
                    if (fellowMergeState) {
                        d_mergeStateFree(fellowMergeState);
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "No need to merge with fellow %u as a result of solving conflict %d\n",
                         fellow->address->systemId, conflict->id);
                }
                d_networkAddressFree(fellowAddr);
                d_fellowFree(fellow);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Cancelling conflict %d because no fellow for master %u could be found.\n",
                    conflict->id, masterAddr->systemId);
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "NameSpace '%s' now has changed master from %u to %u, cancelling conflict %d.\n",
                conflict->nameSpaceCopy->name, conflict->fellowAddr->systemId, masterAddr->systemId, conflict->id);
        }
        d_networkAddressFree(masterAddr);
        d_nameSpaceFree(nameSpace);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Cancelling conflict %d because no namespace '%s' could be found.\n",
             conflict->id, conflict->nameSpaceCopy->name);
    }
    d_networkAddressFree(unAddressed);
    d_networkAddressFree(myAddr);

    return reschedule;
}


static c_bool
resolveForeignStateConflict(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_nameSpace nameSpace, fellowNameSpace, fellowNameSpaceCopy;
    d_fellow fellow;
    d_groupLocalListener listener;
    d_admin admin;
    d_durability durability;
    d_configuration config;
    d_serviceState fellowState;
    d_networkAddress myAddr, masterAddr, fellowAddr, fellowMasterAddr, unAddressed;
    d_mergeState newState, mergeState, fellowMergeState;
    d_mergePolicy mergePolicy;
    d_alignmentKind alignmentKind;
    c_bool reschedule = FALSE;
    c_iter fellows;
    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_FOREIGN_STATE_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    d_printTimedEvent(durability, D_LEVEL_FINER,
          "Resolving foreign state conflict %u for namespace '%s' (run: %u)\n",
          conflict->id, conflict->nameSpaceCopy->name, conflict->reQueueCount);
    /* A foreign state conflict needs to be resolved.
     * First find out if the foreign state conflict still exists.
     */
    myAddr = d_adminGetMyAddress(admin);
    unAddressed = d_networkAddressUnaddressed();
    listener = d_subscriberGetGroupLocalListener(d_adminGetSubscriber(admin));
    nameSpace = d_adminGetNameSpace(admin, conflict->nameSpaceCopy->name);
    if (nameSpace) {
        masterAddr = d_nameSpaceGetMaster(nameSpace);
        fellow = d_adminGetFellow(admin, conflict->fellowAddr);
        if (fellow) {
            fellowAddr = d_fellowGetAddress(fellow);
            fellowNameSpace = d_fellowGetNameSpace(fellow, nameSpace);
            if (fellowNameSpace) {
                fellowMasterAddr = d_nameSpaceGetMaster(fellowNameSpace);

                if ( (d_networkAddressCompare(masterAddr, myAddr) == 0) &&                      /* I am master for the namespace */
                     (d_nameSpaceIsMasterConfirmed(nameSpace)) &&                               /* my master is confirmed */
                     (d_networkAddressCompare(fellowMasterAddr, fellowAddr) == 0)  &&           /* the fellow is master for the fellow namespace */
                     (d_nameSpaceIsMasterConfirmed(fellowNameSpace)) &&                         /* the master for the fellow namespace is confirmed */
                     (strcmp(config->role, fellow->role) != 0) ) {                              /* the fellow has a different role than me */

                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                         "Foreign state conflict %d for namespace '%s' may still exist, checking further\n",
                         conflict->id, nameSpace->name);

                    fellowState = d_fellowGetState(fellow);
                    mergePolicy = d_nameSpaceGetMergePolicy(nameSpace, fellow->role);
                    alignmentKind = d_nameSpaceGetAlignmentKind(nameSpace);
                    /* Get the merge state for the fellow role */
                    mergeState = d_nameSpaceGetMergeState(nameSpace, fellow->role);
                    fellowMergeState = d_nameSpaceGetMergeState(fellowNameSpace, fellow->role);

                    if ((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Cancelling conflict %d because fellow %u has terminated.\n",
                            conflict->id, fellowAddr->systemId);

                    } else if ((alignmentKind != D_ALIGNEE_INITIAL) && (alignmentKind != D_ALIGNEE_LAZY)) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                             "Cancelling conflict %d because alignee policy does not ask for it.\n",
                             conflict->id);

                    } else if (fellowMergeState == NULL) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Cancelling conflict %d, unable to merge namespace '%s' of fellow %u because its merge state is NULL\n",
                            conflict->id, nameSpace->name, fellowAddr->systemId);

                    } else if (!d_fellowAreNameSpacesComplete(fellow)) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Not all namespaces of fellow %u are received yet, re-scheduling conflict %d\n",
                            fellowAddr->systemId, conflict->id);
                        reschedule = TRUE;

                    } else if (d_durabilityGetState(durability) != D_STATE_COMPLETE) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "I am not complete, re-scheduleing conflict %d\n",
                            conflict->id);
                        reschedule = TRUE;

                    } else if (nameSpace->mergeState->value != conflict->nameSpaceCopy->mergeState->value) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "The merge state of my namespace '%s' has changed after conflict %d was detected from (%s,%d) to (%s,%d)," \
                            " rescheduling the conflict",
                            nameSpace->name, conflict->id,
                            conflict->nameSpaceCopy->mergeState->role, conflict->nameSpaceCopy->mergeState->value,
                            nameSpace->mergeState->role, nameSpace->mergeState->value);
                        reschedule = TRUE;

                    } else if (fellowNameSpace->mergeState->value != conflict->fellowNameSpaceCopy->mergeState->value) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                             "The merge state of the conflicting namespace '%s' of fellow %u has changed after conflict %d was detected from (%s,%d) to (%s,%d)," \
                             " rescheduling the conflict",
                             fellowNameSpace->name, fellowAddr->systemId, conflict->id,
                             conflict->fellowNameSpaceCopy->mergeState->role, conflict->fellowNameSpaceCopy->mergeState->value,
                             fellowNameSpace->mergeState->role, fellowNameSpace->mergeState->value);
                        reschedule = TRUE;
                    } else if (mergePolicy == D_MERGE_IGNORE) {
                        /* No need to align, it is sufficient to take over the fellow's merge state */
                         d_printTimedEvent(durability, D_LEVEL_FINER,
                              "No need to merge with fellow %u to solve foreign state conflict %d because the merge policy is IGNORE\n",
                              fellowAddr->systemId, conflict->id);
                         d_nameSpaceSetMergeState(nameSpace, fellowMergeState);
                         d_printTimedEvent(durability, D_LEVEL_FINE,
                              "Updating merge state for role '%s' to fellow's %u merge state %d, done solving conflict %d\n",
                              fellow->role, fellowAddr->systemId, fellowMergeState->value, conflict->id);
                    } else if (mergeState && (mergeState->value == fellowMergeState->value)) {
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                             "No difference in merge state detected for namespace '%s' from fellow %u, done solving conflict %d\n",
                             nameSpace->name, fellowAddr->systemId, conflict->id);
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                            "Applying merge with my fellow %u for nameSpace '%s' to solve foreign state conflict %d.\n",
                            fellow->address->systemId,
                            d_nameSpaceGetName(nameSpace),
                            conflict->id);
                        /* Get a copy of the masters's nameSpace at this moment in time. */
                        fellowNameSpaceCopy = d_nameSpaceCopy(fellowNameSpace);
                        /* Copy state from master fellow */
                        newState = d_mergeStateNew(fellow->role, fellowNameSpaceCopy->mergeState->value);
                        /* A merge is needed with the fellow for the nameSpace.
                         * Get a copy of the fellow's namespace at this moment
                         * in time.
                         */
                        fellows = c_iterNew(d_fellow(d_objectKeep(d_object(fellow))));
                        handleMergeAlignment(listener, conflict, fellowNameSpaceCopy, fellows, newState);
                        fellowsFree(fellows);
                        d_mergeStateFree(newState);
                        d_nameSpaceFree(fellowNameSpaceCopy);
                    }
                    if (mergeState) {
                        d_mergeStateFree(mergeState);
                    }
                    if (fellowMergeState) {
                        d_mergeStateFree(fellowMergeState);
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "No need to merge with fellow %u as a result of solving conflict %d\n",
                        fellow->address->systemId, conflict->id);
                }
                d_networkAddressFree(fellowMasterAddr);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Cancelling conflict %d because no fellow for master %u could be found.\n",
                    conflict->id, masterAddr->systemId);
            }
            d_fellowFree(fellow);
            d_networkAddressFree(fellowAddr);
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "NameSpace '%s' now has changed master from %u to %u, cancelling conflict %d.\n",
                conflict->nameSpaceCopy->name, conflict->fellowAddr->systemId, masterAddr->systemId, conflict->id);
        }
        d_networkAddressFree(masterAddr);
        d_nameSpaceFree(nameSpace);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Cancelling conflict %d because no namespace '%s' could be found.\n",
            conflict->id, conflict->nameSpaceCopy->name);
    }
    d_networkAddressFree(unAddressed);
    d_networkAddressFree(myAddr);

    return reschedule;
}


static c_bool
resolveFellowDisconnectConflict(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_admin admin;
    d_durability durability;
    c_bool reschedule = FALSE;
    d_fellow fellow;
    d_networkAddress fellowAddr;
    d_serviceState myState, fellowState;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_FELLOW_DISCONNECTED_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    d_printTimedEvent(durability, D_LEVEL_FINER,
         "Resolving fellow disconnected conflict %u for fellow %u (run: %u)\n",
         conflict->id, conflict->fellowAddr->systemId, conflict->reQueueCount);
    /* To solve a fellow disconnected conflict we need to request data for all
     * namespaces for which I am confirmed master.
     */
    fellow = d_adminGetFellow(admin, conflict->fellowAddr);
    if (fellow) {
        myState = d_durabilityGetState(durability);
        fellowAddr = d_fellowGetAddress(fellow);
        fellowState = d_fellowGetState(fellow);

        d_printTimedEvent(durability, D_LEVEL_FINEST,
             "Fellow disconnected conflict %d may still exist, checking further\n",
             conflict->id);

        if ((myState == D_STATE_TERMINATING) || (myState == D_STATE_TERMINATED)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Canceling conflict %d because I am terminating.\n",
                conflict->id);

        } else if ((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Canceling conflict %d because fellow %u has terminated.\n",
                conflict->id, fellowAddr->systemId);

        } else if (!d_fellowAreNameSpacesComplete(fellow)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                 "Not all namespaces of fellow %"PA_PRIu32" are received yet, re-scheduling conflict %"PA_PRIu32"\n",
                 fellowAddr->systemId, conflict->id);
            reschedule = TRUE;

        } else {
            /* Apparently the fellow experienced a disconnect with me, but I did not notice that.
             * Still I might have lost data when the fellow was publishing data, e.g., because my acks/nacks
             * where not received by the fellow.
             * To ensure a consistent state I must align from my master (assuming some fellow is the master),
             * or I must acquire the data from the fellow if I was master.
             */
            c_iter nameSpaces;
            c_iterIter iter;
            d_nameSpace nameSpace;
            d_networkAddress masterAddr;
            d_networkAddress myAddr;
            d_mergeState newState;
            d_groupLocalListener listener;
            d_networkAddress unAddressed = d_networkAddressUnaddressed();

            nameSpaces = d_adminGetNameSpaces(admin);
            iter = c_iterIterGet(nameSpaces);
            listener = d_subscriberGetGroupLocalListener(d_adminGetSubscriber(admin));

            myAddr = d_adminGetMyAddress(admin);
            while ((nameSpace = (d_nameSpace)c_iterNext(&iter)) != NULL) {
                masterAddr = d_nameSpaceGetMaster(nameSpace);

                /* If I have no confirmed master for the namespace then I am busy negotiating
                 * a master. This will already lead to alignment, so I can safely discard this case.
                 */
                if (!d_nameSpaceIsMasterConfirmed(nameSpace)) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "I have no confirmed master for namespace %s, discarding conflict %"PA_PRIu32"\n", nameSpace->name, conflict->id);
                }

                else if (d_networkAddressCompare(masterAddr, unAddressed) == 0) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "No aligner is available for namespace %s, discarding conflict %"PA_PRIu32"\n", nameSpace->name, conflict->id);
                }

                /* If I was the master myself then I must retrieve the data that I
                 * potentially missed and update my state.
                 */
                else if (d_networkAddressCompare(masterAddr, myAddr) == 0) {
                    c_iter fellows;
                    d_nameSpace fellowNameSpace;

                    fellowNameSpace = d_fellowGetNameSpace(fellow, nameSpace);
                    if (d_nameSpaceIsAligner(fellowNameSpace)) {
                        /* I am the confirmed master for this nameSpace, so I must make sure
                         * to obtain any data that I missed
                         */
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                             "I am the confirmed master for namespace %s, "
                             "I am going to request data from fellow %"PA_PRIu32"\n",
                             nameSpace->name, fellow->address->systemId);
                        /* Set the new state */
                        newState = d_mergeStateNew(fellow->role, nameSpace->mergeState->value + 1);
                        /* A merge is needed with the fellow for the nameSpace. */
                        fellows = c_iterNew(d_fellow(d_objectKeep(d_object(fellow))));
                        handleMergeAlignment(listener, conflict, nameSpace, fellows, newState);
                        fellowsFree(fellows);
                        d_mergeStateFree(newState);
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "I am the confirmed master for namespace %s, "
                            "no need to request data from fellow %"PA_PRIu32" since it can't align\n",
                            nameSpace->name, fellow->address->systemId);
                    }

                /* A fellow was master, I must realign with this fellow to get synchronized again.
                 * Clear my merge state and initiate a native state conflict to get
                 * data from my master again
                 */
                } else {
                    d_fellow master;

                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        " - Fellow %"PA_PRIu32" is the confirmed master for namespace %s, clear the namespace state and realign with my master\n", masterAddr->systemId, nameSpace->name);

                    d_nameSpaceClearMergeState(nameSpace, NULL);
                    if ((master = d_adminGetFellow(admin, masterAddr)) != NULL) {
                        d_fellowSendNSRequest(master);
                        d_fellowFree(master);
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            " - Unable to find the confirmed %"PA_PRIu32" for namespace %s in my administration, I only cleared the namespace state\n", masterAddr->systemId, nameSpace->name);
                    }

                }
                d_networkAddressFree(masterAddr);
                d_nameSpaceFree(nameSpace);
            }
            c_iterFree(nameSpaces);
            d_networkAddressFree(myAddr);
            d_networkAddressFree(unAddressed);

        }
        d_networkAddressFree(fellowAddr);
        d_fellowFree(fellow);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Canceling conflict %d because fellow = <NULL>.\n",
            conflict->id);
    }

    return reschedule;
}


static c_bool
resolveFellowConnectConflict(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_admin admin;
    d_durability durability;
    d_configuration config;
    c_bool reschedule = FALSE;
    d_fellow fellow;
    d_networkAddress fellowAddr;
    d_serviceState myState, fellowState;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_FELLOW_CONNECTED_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    d_printTimedEvent(durability, D_LEVEL_FINER,
        "Resolving fellow connected conflict %u for fellow %u (run: %u)\n",
        conflict->id, conflict->fellowAddr->systemId, conflict->reQueueCount);

    /* A fellowConnect conflict occurs if a fellow is recently connected.
     * This can happen when a fellow just joins, or when an asymmetric disconnect
     * has been discovered. In both cases the fellow may already have published data.
     * This data needs to be retrieved for nodes that are master
     */
    fellow = d_adminGetFellow(admin, conflict->fellowAddr);
    if (fellow) {
        myState = d_durabilityGetState(durability);
        fellowAddr = d_fellowGetAddress(fellow);
        fellowState = d_fellowGetState(fellow);

        d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Fellow connected conflict %d may still exist, checking further\n",
                    conflict->id);

        if ((myState == D_STATE_TERMINATING) || (myState == D_STATE_TERMINATED)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Canceling conflict %d because I am terminating.\n",
                        conflict->id);
            fellow->recently_joined = FALSE;

        } else if ((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Canceling conflict %d because fellow %u has terminated.\n",
                        conflict->id, fellowAddr->systemId);
            fellow->recently_joined = FALSE;


        } else if (!d_fellowAreNameSpacesComplete(fellow)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Not all namespaces of fellow %"PA_PRIu32" are received yet, re-scheduling conflict %"PA_PRIu32"\n",
                        fellowAddr->systemId, conflict->id);
            reschedule = TRUE;

        } else {
            /* The fellow may have published data that I do not know.
             * To receive the data that I potentially missed I should re-align.
             */
            c_iter nameSpaces;
            c_iterIter iter;
            d_nameSpace nameSpace;
            d_networkAddress masterAddr;
            d_networkAddress myAddr;
            d_mergeState newState;
            d_groupLocalListener listener;
            d_networkAddress unAddressed = d_networkAddressUnaddressed();

            nameSpaces = d_adminGetNameSpaces(admin);
            iter = c_iterIterGet(nameSpaces);
            listener = d_subscriberGetGroupLocalListener(d_adminGetSubscriber(admin));

            myAddr = d_adminGetMyAddress(admin);
            while ((nameSpace = (d_nameSpace)c_iterNext(&iter)) != NULL) {
                masterAddr = d_nameSpaceGetMaster(nameSpace);

                /* If I have no confirmed master for the namespace then I am busy negotiating
                 * a master. This will already to alignment, so I can safely discard this case.
                 */
                if (!d_nameSpaceIsMasterConfirmed(nameSpace)) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "I have no confirmed master for namespace %s yet, reschedule conflict %"PA_PRIu32"\n",
                        nameSpace->name, conflict->id);
                    reschedule = TRUE;
                }
                else if (d_networkAddressCompare(masterAddr, unAddressed) == 0) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "No aligner is available for namespace %s, discarding conflict %"PA_PRIu32"\n",
                        nameSpace->name, conflict->id);
                }
                /* If I was the master myself then I must retrieve the data that I
                 * potentially missed and update my state.
                 */
                else if (d_networkAddressCompare(masterAddr, myAddr) == 0) {
                    c_iter fellows;
                    d_nameSpace fellowNameSpace;

                    fellowNameSpace = d_fellowGetNameSpace(fellow, nameSpace);
                    if (d_nameSpaceIsAligner(fellowNameSpace)) {
                        d_mergePolicy mergePolicy = d_nameSpaceGetMergePolicy(nameSpace, d_fellowGetRole(fellow));
                        if (mergePolicy != D_MERGE_IGNORE) {
                            /* I am the confirmed master for this nameSpace, so I must make sure
                             * to obtain any data that I missed
                             */
                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                    "I am the confirmed master for namespace %s, "
                                    "I am going to request data from fellow %"PA_PRIu32"\n",
                                    nameSpace->name, fellow->address->systemId);
                            /* Set the new state */
                            newState = d_mergeStateNew(fellow->role, nameSpace->mergeState->value + 1);
                            /* A merge is needed with the fellow for the nameSpace. */
                            fellows = c_iterNew(d_fellow(d_objectKeep(d_object(fellow))));
                            handleMergeAlignment(listener, conflict, nameSpace, fellows, newState);
                            fellowsFree(fellows);
                            d_mergeStateFree(newState);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                    "I am the confirmed master but configured to ignore the namespace %s, "
                                    "I am NOT going to request data from fellow %"PA_PRIu32"\n",
                                    nameSpace->name, fellow->address->systemId);
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "I am the confirmed master for namespace %s, "
                                "no need to request data from fellow %"PA_PRIu32" since it can't align\n",
                                nameSpace->name, fellow->address->systemId);
                    }
                } else if (d_networkAddressCompare(masterAddr, fellowAddr) == 0) {
                    d_nameSpace fellowNameSpace;
                    fellowNameSpace = d_fellowGetNameSpace(fellow, nameSpace);

                    if (d_nameSpaceIsAligner(fellowNameSpace)) {
                        d_mergePolicy mergePolicy = d_nameSpaceGetMergePolicy(nameSpace, d_fellowGetRole(fellow));

                        if (d_nameSpaceGetMergeCount(fellowNameSpace) == 0) {
                            if (mergePolicy != D_MERGE_IGNORE) {
                                d_nameSpace fellowNameSpaceCopy;
                                c_iter fellows;

                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                    "Applying merge with my master fellow %u for nameSpace '%s' to solve conflict %d.\n",
                                    fellow->address->systemId, d_nameSpaceGetName(nameSpace), conflict->id);

                                /* Get a copy of the masters's nameSpace at this moment in time. */
                                fellowNameSpaceCopy = d_nameSpaceCopy(fellowNameSpace);
                                /* Copy the master's namespace state */
                                newState = d_mergeStateNew(fellowNameSpaceCopy->mergeState->role, fellowNameSpaceCopy->mergeState->value);
                                /* A merge is needed with the fellow for the nameSpace. */
                                fellows = c_iterNew(d_fellow(d_objectKeep(d_object(fellow))));
                                handleMergeAlignment(listener, conflict, fellowNameSpaceCopy, fellows, newState);
                                fellowsFree(fellows);
                                d_mergeStateFree(newState);
                                d_nameSpaceFree(fellowNameSpaceCopy);
                            } else {
                                d_mergeState fellowMergeState;
                                fellowMergeState = d_nameSpaceGetMergeState(fellowNameSpace, config->role);
                                assert(fellowMergeState);
                                /* No need to align, it is sufficient to take over the fellow's merge state */
                                 d_printTimedEvent(durability, D_LEVEL_FINER,
                                      "No need to merge with fellow %"PA_PRIu32" to solve connect conflict %d because the merge policy is IGNORE\n",
                                      fellowAddr->systemId, conflict->id);
                                 d_nameSpaceSetMergeState(nameSpace, fellowMergeState);
                                 d_printTimedEvent(durability, D_LEVEL_FINE,
                                      "Updating merge state for role '%s' to fellow's %u merge state %d, done solving conflict %d\n",
                                      fellow->role, fellowAddr->systemId, fellowMergeState->value, conflict->id);
                                 d_mergeStateFree(fellowMergeState);
                            }
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Already applied merge policy for namespace %s after connect, no need to do anything\n",
                                nameSpace->name);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Neither me nor fellow %"PA_PRIu32" is master for namespace %s when solving conflict %u, no need to do anything\n",
                        masterAddr->systemId, nameSpace->name, conflict->id);
                }

                d_networkAddressFree(masterAddr);
                d_nameSpaceFree(nameSpace);
            }
            c_iterFree(nameSpaces);
            d_networkAddressFree(myAddr);
            d_networkAddressFree(unAddressed);

            if (reschedule != TRUE) {
                fellow->recently_joined = FALSE;
            }
        }
        d_networkAddressFree(fellowAddr);
        d_fellowFree(fellow);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Canceling conflict %d because fellow = <NULL>.\n",
                    conflict->id);
    }

    return reschedule;
}


static c_bool
resolveFederationDisconnectConflict(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_admin admin;
    d_durability durability;
    c_bool reschedule = FALSE;
    d_serviceState myState;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_FEDERATION_DISCONNECTED_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    myState = d_durabilityGetState(durability);
    d_printTimedEvent(durability, D_LEVEL_FINER,
            "Resolving federation disconnected conflict %u (run: %u)\n",
            conflict->id, conflict->reQueueCount);

    if ((myState == D_STATE_TERMINATING) || (myState == D_STATE_TERMINATED)) {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Canceling conflict %d because I am terminating.\n", conflict->id);

    } else if (myState != D_STATE_COMPLETE) {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
             "Rescheduling conflict %d because I have not reached state D_STATE_COMPLETE yet (current state %d).\n",
             conflict->id, myState);
        reschedule = TRUE;

    } else {
        /* No real resolution needed for this conflict.
         * The only purpose is to trigger a conflict so that d_conflictResolverRun
         * marks all aligner groups complete in case there are no pending conflicts
         */
        d_printTimedEvent(durability, D_LEVEL_FINEST,
             "Federation disconnected conflict %d still valid, checking whether to groups needs to become complete\n",
             conflict->id);
    }
    return reschedule;
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

static c_bool
resolveInitialConflict(
    _Inout_ d_conflictResolver conflictResolver,
    _Inout_ d_conflict conflict)
{
    d_admin admin;
    d_durability durability;
    c_bool reschedule = FALSE;
    d_serviceState myState;
    c_bool complete;
    c_bool fellowNameSpacesKnown = FALSE;
    d_subscriber subscriber;
    d_configuration config;
    d_sampleChainListener sampleChainListener;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_INITIAL_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    subscriber = d_adminGetSubscriber(admin);
    sampleChainListener = d_subscriberGetSampleChainListener(subscriber);

    myState = d_durabilityGetState(durability);
    d_printTimedEvent(durability, D_LEVEL_FINER,
            "Resolving initial conflict %u (run: %u)\n",
            conflict->id, conflict->reQueueCount);

    if ((myState == D_STATE_TERMINATING) || (myState == D_STATE_TERMINATED)) {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Canceling conflict %d because I am terminating.\n",
                    conflict->id);
    } else {
        os_timeM now = os_timeMGet();
        c_bool toReport;

        /* Get a copy of the list of currently available fellows.
         * This list will be used when resolving the initial conflict.
         * Any fellow that will appear during the resolution of the initial
         * conflict will not play a role in resolving the initial conflict */ 
        d_adminInitialFellowsCreate(admin);

        /* Check whether or not to report */
        toReport = (os_timeMCompare(conflictResolver->nextReportTime, now) == OS_LESS);
        if (toReport) {
            char *str;

            str = d_adminGetInitialFellowsString(admin);
            d_printTimedEvent(durability, D_LEVEL_INFO,
                        "Checking for completeness of local groups with fellows [%s].\n", str);
            os_free(str);
        }

        /* Reschedule initial conflict if namespaces not yet known */
        d_adminInitialFellowWalk(durability->admin, checkFellowNameSpacesKnown, &fellowNameSpacesKnown);
        if ((fellowNameSpacesKnown == FALSE) && (d_adminGetInitialFellowCount(admin) > 0)) {
            reschedule = TRUE;
            return reschedule;
        }

        if (toReport) {
            d_printTimedEvent(durability, D_LEVEL_FINEST, "Fellow nameSpaces complete.\n");
        }

        /* Start groupLocalListener; cannot be started multiple times due to listener->attached */
        if (durability->splicedRunning == TRUE) {
            d_groupLocalListenerStart(d_subscriberGetGroupLocalListener(subscriber));
        }

        /* Check for group completeness */
        complete = d_adminAreLocalGroupsComplete(durability->admin, FALSE);

        if (!complete) {
            if (toReport && (config->tracingVerbosityLevel <= D_LEVEL_FINER)) {
                d_printTimedEvent(durability, D_LEVEL_FINER, "Waiting for local groups to get complete...\n");
            }

            if (toReport && (config->tracingVerbosityLevel == D_LEVEL_FINEST)) {
                d_sampleChainListenerReportStatus(sampleChainListener);
                d_durabilityReportKernelGroupCompleteness(durability);
            }

        } else {
            c_bool ret;

            /* Wait until the persistent store has marked all relevant
             * namespaces as complete and change the state
             */
            d_durabilityDetermineNameSpaceCompleteness (durability);

            d_printTimedEvent(durability, D_LEVEL_FINER, "Local groups are complete now.\n");
            d_durabilitySetState(durability, D_STATE_COMPLETE);
            ret = u_serviceChangeState(durability->service, STATE_OPERATIONAL);
            if (ret) {
                d_printTimedEvent(durability, D_LEVEL_INFO, "Durability service up and fully operational.\n");
                d_durabilityDoInitialMerge(durability);
            }
        }

        /* Update nextReportTime */
        if (toReport) {
            /* Set next reporting period over 10 seconds */
            os_duration d  = OS_DURATION_INIT(10,0);
            conflictResolver->nextReportTime = os_timeMAdd(conflictResolver->nextReportTime, d);
        }

        d_adminInitialFellowsDestroy(admin);

        /* Reschedule conflict if local groups are not yet complete */
        reschedule = !complete;
    }
    return reschedule;
}

static c_bool
resolveLocalGroupCompleteConflict(
    _Inout_ d_conflictResolver conflictResolver,
    _Inout_ d_conflict conflict)
{
    d_admin admin;
    d_durability durability;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_LOCAL_GROUP_COMPLETE_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Resolving local group complete conflict %u (run: %u)\n",
            conflict->id, conflict->reQueueCount);

    return FALSE; /* Never reschedule local group conflict. */
}

static c_bool
resolveHeartbeatProcessedConflict(
    _Inout_ d_conflictResolver conflictResolver,
    _Inout_ d_conflict conflict)
{
    d_admin admin;
    d_durability durability;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));
    assert(IS_HEARTBEAT_PROCESSED_CONFLICT(conflict));

    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Resolving heartbeat processed (for federation %u) conflict %u (run: %u)\n",
            conflict->fellowAddr->systemId, conflict->id, conflict->reQueueCount);

    d_durabilityHeartbeatProcessed(durability);

    return FALSE; /* Never reschedule local group conflict. */
}

/* Verifies if existing conflict1 can be combined with conflict2.
 *
 * The caller must ensure that the conflictQueueMutex is taken before
 * this function is called
 *
 * Returns TRUE if can be combined, FALSE otherwise */
static c_bool
can_combine_conflict(d_conflict conflict1, d_conflict conflict2)
{
    d_durability durability = d_threadsDurability();
    c_bool result = FALSE;

    if ((conflict1->event) != (conflict2->event)) {
        /* Conflicts for different events can never be combined */\
        return FALSE;
    }

    /* From now on we know that conflict1->event == conflict2->event */

    if (IS_HEARTBEAT_PROCESSED_CONFLICT(conflict1)) {
        /* heartbeat process/federation/initial/local group complete conflicts are never combined */
        return FALSE;
    }

    if (IS_MASTER_CONFLICT(conflict1)) {
        /* master conclicts can be combined if they are for the same namespace independent of the fellow */
        assert(conflict1->nameSpaceCopy && conflict2->nameSpaceCopy && conflict1->fellowNameSpaceCopy && conflict2->fellowNameSpaceCopy);
        result = ((d_nameSpaceCompare(conflict1->nameSpaceCopy, conflict2->nameSpaceCopy) == 0) &&
                  (d_nameSpaceCompare(conflict1->fellowNameSpaceCopy, conflict2->fellowNameSpaceCopy) == 0));

    } else if (IS_FEDERATION_DISCONNECTED_CONFLICT(conflict1) || IS_INITIAL_CONFLICT(conflict1) || IS_LOCAL_GROUP_COMPLETE_CONFLICT(conflict1)) {
        /* federation disconnect/initial/local group complete conflicts are always combined */
        result = TRUE;

    } else if (IS_NATIVE_STATE_CONFLICT(conflict1) || IS_FOREIGN_STATE_CONFLICT(conflict1)) {
        /* native/foreign state conflicts can be combined if they are for the same fellow and same namespace */
        result = (d_networkAddressCompare(conflict1->fellowAddr, conflict2->fellowAddr) == 0);
        if (result) {
            result = ((d_nameSpaceCompare(conflict1->nameSpaceCopy, conflict2->nameSpaceCopy) == 0) &&
                      (d_nameSpaceCompare(conflict1->fellowNameSpaceCopy, conflict2->fellowNameSpaceCopy) == 0));
        }

    } else if (IS_FELLOW_DISCONNECTED_CONFLICT(conflict1) || IS_FELLOW_CONNECTED_CONFLICT(conflict1)) {
        /* fellow connected/disconnect conflicts can be combined if they are for the same fellow */
         result = (d_networkAddressCompare(conflict1->fellowAddr, conflict2->fellowAddr) == 0);

    }

    if (result) {
        d_printTimedEvent(durability, D_LEVEL_FINER,
                "Conflict %d is similar to already existing conflict %d, discarding conflict %d\n",
                conflict2->id, conflict1->id, conflict2->id);
    }
    return result;
}


struct findConflictHelper {
    d_conflict conflict;
    c_bool found;
    d_admin admin;
};

static void
findConflict(
    c_voidp o,
    c_voidp userData)
{
    struct findConflictHelper *helper;
    d_conflict conflict;

    assert(userData);

    helper = (struct findConflictHelper *)userData;
    conflict = d_conflict(o);

    if (helper->conflict) {
        d_lockLock(d_lock(conflict));
        /* Only check for combination if not yet combined */
        if (!helper->found) {
            helper->found = can_combine_conflict(conflict, helper->conflict);
        }
        d_lockUnlock(d_lock(conflict));
    }
}

static void
check_remove_pending_requests(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{

     d_durability durability;
     d_fellow fellow;

     assert(d_conflictResolverIsValid(conflictResolver));

     durability = d_adminGetDurability(conflictResolver->admin);

     /* If my fellow was asymmetrically disconnected from me and I have
      * pending unanswered sampleRequests to this fellow, then the fellow might have
      * missed some of them. To prevent that I will be waiting indefinitely for
      * replies that will never come I must assume that the fellow was disconnected
      * and clean up pending chains for this fellow
      */
     if (IS_FELLOW_DISCONNECTED_CONFLICT(conflict)) {
         fellow = d_adminGetFellow(conflictResolver->admin, conflict->fellowAddr);
         if (fellow) {
             d_printTimedEvent(durability, D_LEVEL_FINER,
                 "Fellow %u was asymmetrically disconnected from me while I have outstanding sample requests to this fellow," \
                 "clearing pending requests to this fellow\n",
                  conflict->fellowAddr->systemId);
             /* Do as if the fellow was removed without actually removing it
              * This will trigger new merges
              */
             d_adminAsymRemoveFellow(durability->admin, fellow, FALSE);
             d_fellowFree(fellow);
         }
     }
}

/**
 * \brief Checks if the conflict exists in the queue of conflicts
 *
 * @return TRUE if the conflict already exists, FALSE otherwise
 */
c_bool
d_conflictResolverConflictExists(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    struct findConflictHelper helper;
    d_durability durability = d_threadsDurability();

    d_printTimedEvent(durability, D_LEVEL_FINER, "Trying fo find similar conflict as conflict %d\n", conflict->id);

    /* Check the current conflict */
    check_remove_pending_requests(conflictResolver, conflict);

    helper.conflict = conflict;
    helper.found = FALSE;
    helper.admin = conflictResolver->admin;

    /* Lock the conflict queue to prevent any update to the queue or
     * the current conflict */
    os_mutexLock(&helper.admin->conflictQueueMutex);

    /* Check if a similar conflict is currently being processed */
    if (conflictResolver->current_conflict != NULL) {
        helper.found = can_combine_conflict(conflictResolver->current_conflict, conflict);
    }

    if (!helper.found) {
        /* Check if a similar conflict is pending in the queue */
        c_iterWalk(helper.admin->conflictQueue, findConflict, &helper);
    }

    /* Unlock the conflict queue*/
    os_mutexUnlock(&helper.admin->conflictQueueMutex);

    return helper.found;
}

static void
set_current_conflict(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));

    /* Only set the current conflict if it was reset previously */
    assert(conflictResolver->current_conflict == NULL);

    os_mutexLock(&conflictResolver->admin->conflictQueueMutex);
    conflictResolver->current_conflict = d_conflict(d_objectKeep(d_object(conflict)));
    os_mutexUnlock(&conflictResolver->admin->conflictQueueMutex);
}


static void
reset_current_conflict(
    d_conflictResolver conflictResolver)
{
    assert(d_conflictResolverIsValid(conflictResolver));

    /* Only reset the current conflict if it was set previously */
    assert(conflictResolver->current_conflict != NULL);

    os_mutexLock(&conflictResolver->admin->conflictQueueMutex);
    d_objectFree(d_object(conflictResolver->current_conflict));
    conflictResolver->current_conflict = NULL;
    os_mutexUnlock(&conflictResolver->admin->conflictQueueMutex);

}

/**
 * \brief Main loop to resolve conflicts.
 *
 * The loop checks every 200ms if there are any conflicts, and if so,
 * handles them.
 *
 * A future extension could be to define various strategies to handle
 * conflicts, such as:
 * 1) handle conflicts on a first come first serve basis
 * 2) wait until no new conflicts appear, then get all pending conflicts,
 *    analyse the pending conflicts, and decide about a optimal order
 *    to resolve them.
 * 3) Blacklist the fellow that causes the conflict.
 *
 * NOTE:
 * The code is NOT thread safe, but this is OK since the conflict resolution
 * code is executed by a single, dedicated thread.
 */
static void*
d_conflictResolverRun(
    void* userData)
{
    d_conflictResolver conflictResolver;
    os_duration sleepTime = OS_DURATION_INIT(0, 200000000);  /* 200 ms */
    d_durability durability = d_threadsDurability();
    d_conflict conflict = NULL, updatedConflict;
    d_admin admin;
    c_iter toReschedule;
    c_ulong startLen, conflictQueueSize;
    d_thread self = d_threadLookupSelf ();

    conflictResolver = (d_conflictResolver)userData;
    admin = conflictResolver->admin;
    durability = d_adminGetDurability(admin);
    toReschedule = c_iterNew(NULL);
    while ( (!conflictResolver->threadTerminate) && (d_durabilityMustTerminate(durability) == FALSE) ) {
        /* Check if a conflict is being resolved. */
        if (!d_conflictResolverHasConflictInProgress(conflictResolver)) {
            /* Take a conflict from the queue (if any). */
            os_mutexLock(&admin->conflictQueueMutex);
            startLen = c_iterLength(admin->conflictQueue);
            conflict = d_conflict(c_iterTakeFirst(admin->conflictQueue));
            os_mutexUnlock(&admin->conflictQueueMutex);
            while (conflict && (d_durabilityMustTerminate(durability) == FALSE)) {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Trying to resolve conflict %u.\n",
                     conflict->id);
                /* admin->conflictMonitor is necessarily initialised by
                 * the time a conflict exists, but currently not at the
                 * time conflictResolverRun starts
                 */
                updatedConflict = d_conflictMonitorEvaluateConflict(admin->conflictMonitor, conflict);
                if (updatedConflict) {
                    c_bool reschedule;

                    set_current_conflict(conflictResolver, updatedConflict);
                    switch (updatedConflict->event) {
                        case D_CONFLICT_NAMESPACE_MASTER :
                            reschedule = resolveMasterConflict(conflictResolver, updatedConflict);
                            break;
                        case D_CONFLICT_NAMESPACE_NATIVE_STATE :
                            reschedule = resolveNativeStateConflict(conflictResolver, updatedConflict);
                            break;
                        case D_CONFLICT_NAMESPACE_FOREIGN_STATE :
                            reschedule = resolveForeignStateConflict(conflictResolver, updatedConflict);
                            break;
                        case D_CONFLICT_FELLOW_DISCONNECTED :
                            reschedule = resolveFellowDisconnectConflict(conflictResolver, updatedConflict);
                            break;
                        case D_CONFLICT_FELLOW_CONNECTED :
                            reschedule = resolveFellowConnectConflict(conflictResolver, updatedConflict);
                            break;
                        case D_CONFLICT_FEDERATION_DISCONNECTED :
                            reschedule = resolveFederationDisconnectConflict(conflictResolver, updatedConflict);
                            break;
                        case D_CONFLICT_INITIAL:
                            reschedule = resolveInitialConflict(conflictResolver, updatedConflict);
                            break;
                        case D_CONFLICT_LOCAL_GROUP_COMPLETE:
                            reschedule = resolveLocalGroupCompleteConflict(conflictResolver, updatedConflict);
                            break;
                        case D_CONFLICT_HEARTBEAT_PROCESSED:
                            reschedule = resolveHeartbeatProcessedConflict(conflictResolver, updatedConflict);
                            break;
                        default:
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Unknown conflict %u (type: %d, nameSpace: '%s', fellow: %u, run: %d). Conflict is dicarded.\n",
                                updatedConflict->id, updatedConflict->event, updatedConflict->nameSpaceCopy->name,
                                updatedConflict->fellowAddr->systemId, updatedConflict->reQueueCount);
                            reschedule = FALSE;
                    }
                    if (reschedule) {
                        reset_current_conflict(conflictResolver);
                        toReschedule = c_iterAppend(toReschedule, updatedConflict);

                        d_printTimedEvent(durability, D_LEVEL_FINER,
                            "Rescheduling conflict %u\n",
                            updatedConflict->id);
                    } else {
                        reset_current_conflict(conflictResolver);

                        d_printTimedEvent(durability, D_LEVEL_FINER,
                            "Finished solving conflict %u\n",
                            updatedConflict->id);

                        d_conflictFree(updatedConflict);
                   }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        "The conditions for conflict %u do not apply anymore, discarding the conflict\n",
                        conflict->id);
                    /* Free the conflict, it is not needed anymore */
                    d_conflictFree(conflict);
                }

                if (!d_conflictResolverHasConflictInProgress(conflictResolver)) {
                    /* Get the next conflict */
                    os_mutexLock(&admin->conflictQueueMutex);
                    conflict = d_conflict(c_iterTakeFirst(admin->conflictQueue));
                    os_mutexUnlock(&admin->conflictQueueMutex);
                } else {
                    /* No next conflicts */
                    conflict = NULL;
                }
            }
            os_mutexLock(&admin->conflictQueueMutex);
            while ((updatedConflict = d_conflict(c_iterTakeFirst(toReschedule))) != NULL) {
                d_conflictUpdateQueueTime(updatedConflict);
                admin->conflictQueue = c_iterAppend(admin->conflictQueue, updatedConflict);
            }
            /* If the conflict queue became empty then mark all incomplete groups complete */
            conflictQueueSize = c_iterLength(admin->conflictQueue);
            os_mutexUnlock(&admin->conflictQueueMutex);

            if(startLen && (conflictQueueSize == 0) && !d_conflictResolverHasConflictInProgress(conflictResolver)) {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Conflict-queue became empty. Marking all namespace kernel groups complete.\n");
                d_adminMarkNameSpaceKernelGroupsCompleteness(admin, TRUE);
                d_admin_sync_mergeStates(admin);
            }
        } /* if */
        d_sleep(self, sleepTime);
    } /* while */
    if (conflict) {
        d_printTimedEvent(durability, D_LEVEL_FINER,
            "Durability is terminating while resolving conflict %u, discarding this conflict\n",
            conflict->id);
        d_conflictFree(conflict);
    }
    c_iterFree(toReschedule);
    return NULL;
}


d_conflictResolver
d_conflictResolverNew(
    d_admin admin)
{
    d_conflictResolver conflictResolver = NULL;
    os_threadAttr attr;
    d_configuration config;

    config = d_durabilityGetConfiguration(admin->durability);
    /* Create conflictResolver object */
    conflictResolver = d_conflictResolver(os_malloc(C_SIZEOF(d_conflictResolver)));
    if (conflictResolver) {
        /* Call super-init */
        d_lockInit(d_lock(conflictResolver), D_CONFLICT_RESOLVER, d_conflictResolverDeinit);
        /* Initialize conflictResolver */
        conflictResolver->admin = admin;
        conflictResolver->lastTimeChecked = OS_TIMEM_ZERO;
        conflictResolver->lastTimeResolved = OS_TIMEM_ZERO;
        conflictResolver->totalConflicts = 0;
        conflictResolver->threadTerminate = FALSE;
        conflictResolver->conflictInProgress = 0;
        conflictResolver->current_conflict = NULL;
        conflictResolver->nextReportTime = config->startMTime;

        os_threadAttrInit(&attr);
        (void) d_threadCreate(&conflictResolver->actionThread, "conflictResolver",
                                 &attr, (void*(*)(void*))d_conflictResolverRun,
                                 (void*)conflictResolver);
    }
    return conflictResolver;
}


void
d_conflictResolverDeinit(
    d_object object)
{
    d_conflictResolver conflictResolver;
    d_admin admin;
    d_durability durability;
    os_result osr;

    assert(d_objectIsValid(object, D_CONFLICT_RESOLVER) == TRUE);

    if (object) {
        conflictResolver = d_conflictResolver(object);
        admin = conflictResolver->admin;
        durability = d_adminGetDurability(admin);

        /* Terminate the conflictResolver thread */
        if (os_threadIdToInteger(conflictResolver->actionThread)) {
            conflictResolver->threadTerminate = TRUE;
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Waiting for resolver thread to terminate...\n");
            osr = d_threadWaitExit(conflictResolver->actionThread, NULL);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Resolver thread destroyed (%s).\n", os_resultImage(osr));
        }
        d_lockDeinit(d_lock(conflictResolver));
    }
}


void d_conflictResolverFree(
    d_conflictResolver conflictResolver)
{
    assert(d_conflictResolverIsValid(conflictResolver));

    d_objectFree(d_object(conflictResolver));
}


void
d_conflictResolverAddConflict(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_admin admin;

    assert(d_conflictResolverIsValid(conflictResolver));
    assert(d_conflictIsValid(conflict));

    admin = conflictResolver->admin;
    d_lockLock(d_lock(conflict));
    conflict->lastQueueTime = os_timeMGet(); /* update the last queue time */
    ++(conflict->reQueueCount);              /* update the number of times the conflict is queued */
    d_lockUnlock(d_lock(conflict));
    os_mutexLock(&admin->conflictQueueMutex);
    admin->conflictQueue = c_iterAppend(admin->conflictQueue, conflict);
    if (c_iterLength(admin->conflictQueue) == 1) {
        d_printTimedEvent(d_adminGetDurability(admin), D_LEVEL_FINER,
            "Added conflict to empty conflict queue; marking groups incomplete.\n");
        d_adminMarkNameSpaceKernelGroupsCompleteness(admin, FALSE);
    }
    os_mutexUnlock(&admin->conflictQueueMutex);
}

void
d_conflictResolverSetConflictInProgress(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_durability durability;
    c_ulong conflict_id;

    assert(d_conflictResolverIsValid(conflictResolver));

    durability = d_adminGetDurability(conflictResolver->admin);
    d_lockLock(d_lock(conflictResolver));
    d_lockLock(d_lock(conflict));
    conflict_id = d_conflictGetId(conflict);
    if (conflict_id == 0) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Cannot resolve conflict 0, invalid config id\n");
    } else if ((conflictResolver->conflictInProgress != 0) && (conflictResolver->conflictInProgress != conflict_id)) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Cannot resolve conflict %u because conflict %u is being resolved\n", conflict_id, conflictResolver->conflictInProgress);
    } else {
       /* Solve the conflict.
        * Only print a message when the first task of the conflict is being resolved */
        assert((conflictResolver->conflictInProgress == 0) || (conflictResolver->conflictInProgress == conflict_id));
        conflict->nr++;
        if (conflictResolver->conflictInProgress == 0) {
            d_printTimedEvent(durability, D_LEVEL_FINER, "Start resolving conflict %u\n", conflict_id);
            conflictResolver->conflictInProgress = conflict_id;
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST, "Resolving subtask of conflict %u (total: %u subtasks pending)\n", conflict_id, conflict->nr);
        }
    }
    d_lockUnlock(d_lock(conflict));
    d_lockUnlock(d_lock(conflictResolver));
}

void
d_conflictResolverResetConflictInProgress(
    d_conflictResolver conflictResolver,
    d_conflict conflict)
{
    d_durability durability;
    c_ulong conflict_id, len;
    d_admin admin;

    assert(d_conflictResolverIsValid(conflictResolver));

    durability = d_adminGetDurability(conflictResolver->admin);
    d_lockLock(d_lock(conflictResolver));
    d_lockLock(d_lock(conflict));
    conflict_id = d_conflictGetId(conflict);
    if (conflictResolver->conflictInProgress == conflict_id) {
        conflict->nr--;
        if (conflict->nr == 0) {
            d_printTimedEvent(durability, D_LEVEL_FINER, "Stop resolving conflict %u\n", conflictResolver->conflictInProgress);
            conflictResolver->conflictInProgress = 0;
            admin = conflictResolver->admin;
            os_mutexLock(&admin->conflictQueueMutex);
            len = c_iterLength(admin->conflictQueue);
            os_mutexUnlock(&admin->conflictQueueMutex);
            if (len == 0) {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Conflict-queue became empty. Marking all namespace kernel groups complete.\n");
                d_adminMarkNameSpaceKernelGroupsCompleteness(admin, TRUE);
                d_admin_sync_mergeStates(admin);
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST, "Subtask of conflict %u resolved, still %u subtasks to go\n", conflictResolver->conflictInProgress, conflict->nr);
        }
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Cannot stop resolving conflict %u because conflict %u is currently being resolved\n",
            conflict_id, conflictResolver->conflictInProgress);
    }
    d_lockUnlock(d_lock(conflict));
    d_lockUnlock(d_lock(conflictResolver));
}

c_bool
d_conflictResolverHasConflictInProgress(
    d_conflictResolver conflictResolver)
{
    c_bool inProgress;

    assert(d_conflictResolverIsValid(conflictResolver));

    d_lockLock(d_lock(conflictResolver));
    inProgress = (conflictResolver->conflictInProgress != 0);
    d_lockUnlock(d_lock(conflictResolver));

    return inProgress;
}

