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

#include "d__conflict.h"
#include "d__nameSpace.h"
#include "d__nameSpace.h"
#include "d__durability.h"
#include "d_object.h"
#include "os_time.h"
#include "os_heap.h"
#include "os_abstract.h"

d_conflict
d_conflictNew(
    c_ulong conflictEvent,
    d_networkAddress fellowAddr,
    d_nameSpace nameSpaceCopy,
    d_nameSpace fellowNameSpaceCopy)
{
    d_conflict conflict = NULL;

    /* Create conflict object */
    conflict = d_conflict(os_malloc(C_SIZEOF(d_conflict)));
    if (conflict) {
        /* Call super-init */
        d_lockInit(d_lock(conflict), D_CONFLICT, d_conflictDeinit);
        /* Initialize conflict */
        conflict->event = conflictEvent;
        conflict->reQueueCount = 0;
        conflict->id = 0;  /* Initially 0, will be set to the real id when the conflict is added to the resolverQueue. */
        conflict->nr = 0;
        conflict->fellowAddr = (fellowAddr == NULL) ? NULL : d_networkAddressNew(fellowAddr->systemId, fellowAddr->localId, fellowAddr->lifecycleId);
        conflict->creationTime = os_timeMGet();
        conflict->lastQueueTime = OS_TIMEM_INIT(0,0);                                              /* Initially no conflicts queued. */
        conflict->nameSpaceCopy = (nameSpaceCopy == NULL) ? NULL : d_nameSpace(d_objectKeep(d_object(nameSpaceCopy))); /* copy of the namespace at the time the conflict was determined */
        conflict->fellowNameSpaceCopy = (fellowNameSpaceCopy == NULL) ? NULL : d_nameSpace(d_objectKeep(d_object(fellowNameSpaceCopy)));  /* copy of the fellow namespace at the time the conflict was determined. */
        /* Create the list of foreign state conflicts */
        conflict->foreign_stateConflicts = d_nameSpaceGetMergedStatesDiff(conflict->nameSpaceCopy, conflict->fellowNameSpaceCopy);
        /* Verify that the nameSpaceCopy must be set if the fellowNameSpaceCopy is  set */
        assert((conflict->fellowNameSpaceCopy == NULL) || (conflict->nameSpaceCopy != NULL));
    }
    return conflict;
}


void
d_conflictDeinit(
    d_object object)
{
    d_conflict conflict;

    assert(d_objectIsValid(object, D_CONFLICT) == TRUE);

    if (object) {
        conflict = d_conflict(object);

        if (conflict->fellowAddr) {
            d_networkAddressFree(conflict->fellowAddr);
        }

        if (conflict->nameSpaceCopy) {
            d_nameSpaceFree(conflict->nameSpaceCopy);
        }

        if (conflict->fellowNameSpaceCopy) {
            d_nameSpaceFree(conflict->fellowNameSpaceCopy);
        }

        if (conflict->foreign_stateConflicts) {
            c_iterFree(conflict->foreign_stateConflicts);
        }
    }
}


void d_conflictFree(
    d_conflict conflict)
{
    assert(d_conflictIsValid(conflict));

    d_objectFree(d_object(conflict));

}


/**
 * \brief Assign a unique id to the conflict.
 *
 * The id of the conflict is a monotonically increased number starting
 * with 1, and every new id is increased by 1.
 *
 * The id is unique modulo 0xFFFFFFFF, after which a conflict id is
 * wrapped around to 1.
 */
c_ulong
d_conflictSetId(
    d_conflict conflict,
    d_durability durability)
{
    assert(d_conflictIsValid(conflict));
    assert(d_durabilityIsValid(durability));

    conflict->id = d_durabilityGenerateConflictId(durability);

    return conflict->id;
}


c_ulong
d_conflictGetId(
    d_conflict conflict)
{
    assert(d_conflictIsValid(conflict));

    return conflict->id;
}


os_timeM
d_conflictGetCreationTime(
    d_conflict conflict)
{
    assert(d_conflictIsValid(conflict));

    return conflict->creationTime;
}


void
d_conflictUpdate(
    d_conflict conflict,
    d_nameSpace nameSpaceCopy,
    d_nameSpace fellowNameSpaceCopy)
{
    assert(d_conflictIsValid(conflict));

    d_lockLock(d_lock(conflict));
    /* Update the nameSpaceCopy and fellowNameSpaceCopy and foreign_stateConflicts fields */
    if (conflict->nameSpaceCopy) {
        d_nameSpaceFree(conflict->nameSpaceCopy);
    }
    conflict->nameSpaceCopy = (nameSpaceCopy == NULL) ? NULL : d_nameSpace(d_objectKeep(d_object(nameSpaceCopy)));
    if (conflict->fellowNameSpaceCopy) {
        d_nameSpaceFree(conflict->fellowNameSpaceCopy);
    }
    conflict->fellowNameSpaceCopy = (fellowNameSpaceCopy == NULL) ? NULL : d_nameSpace(d_objectKeep(d_object(fellowNameSpaceCopy)));
    c_iterFree(conflict->foreign_stateConflicts);
    conflict->foreign_stateConflicts = d_nameSpaceGetMergedStatesDiff(conflict->nameSpaceCopy, conflict->fellowNameSpaceCopy);
    d_lockUnlock(d_lock(conflict));
}

void
d_conflictUpdateQueueTime(
    d_conflict conflict)
{
    assert(d_conflictIsValid(conflict));

    d_lockLock(d_lock(conflict));
    conflict->lastQueueTime = os_timeMGet(); /* update the last queue time */
    ++(conflict->reQueueCount);              /* update the number of times the conflict is queued */
    d_lockUnlock(d_lock(conflict));
}
