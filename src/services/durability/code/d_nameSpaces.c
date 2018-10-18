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

#include "d_nameSpaces.h"
#include "d__nameSpaces.h"
#include "d__nameSpace.h"
#include "d_networkAddress.h"
#include "d__mergeState.h"
#include "d__admin.h"
#include "d__table.h"
#include "d__misc.h"
#include "d_message.h"
#include "vortex_os.h"

struct nsWalkHelper {
    d_mergeState states;
    c_ulong index;
};

static c_bool
addMergeState(
    d_mergeState state,
    struct nsWalkHelper* helper)
{
    d_mergeStateInit(&((helper->states)[helper->index]), state->role, state->value);
    helper->index++;

    return TRUE;
}

d_nameSpaces
d_nameSpacesNew(
    d_admin admin,
    d_nameSpace nameSpace,
    d_quality initialQuality,
    c_ulong total)
{
    d_nameSpaces ns = NULL;
    d_networkAddress master;
    d_mergeState state;
    struct nsWalkHelper helper;
    c_ulong masterPriority;
    c_ulong count;

    if(nameSpace){
        ns = d_nameSpaces(os_malloc(C_SIZEOF(d_nameSpaces)));

        if(ns){
            master     = d_networkAddressUnaddressed();
            d_messageInit(d_message(ns), admin);
            /* Quality conversion */
            ns->aligner                    = d_nameSpaceIsAligner(nameSpace);
            ns->durabilityKind             = d_nameSpaceGetDurabilityKind(nameSpace);
            ns->alignmentKind              = d_nameSpaceGetAlignmentKind(nameSpace);
            /* Convert quality (os_timeW) to qualityExt (c_time) */
            d_qualityExtFromQuality(&ns->initialQuality, &initialQuality, IS_Y2038READY(ns));
            ns->partitions                 = d_nameSpaceGetPartitionTopics(nameSpace);
            masterPriority                 = d_nameSpaceGetMasterPriority(nameSpace);
            ns->total                      = total;
            ns->master.systemId            = master->systemId;
            ns->master.localId             = master->localId;
            ns->master.lifecycleId         = master->lifecycleId;
            ns->isComplete                 = TRUE;
            ns->name                       = os_strdup (d_nameSpaceGetName(nameSpace));
            ns->masterConfirmed            = d_nameSpaceIsMasterConfirmed(nameSpace);

            /* Use the advertisedMergeState as the mergeState to advertise */
            state = d_nameSpaceGetAdvertisedMergeState(nameSpace);
            if(state) {
                d_mergeStateInit(&ns->state, state->role, state->value);
                d_mergeStateFree(state);
            } else {
                d_name role = d_nameSpaceGetRole(nameSpace);
                d_mergeStateInit(&ns->state, role, (c_ulong) -1);
                os_free(role);
            }

            ns->mergedStatesCount = d_tableSize(nameSpace->mergedRoleStates);
            count = ns->mergedStatesCount;

            if (ns->aligner == TRUE) {
                count++;
            }

            if (count > 0) {
                ns->mergedStates = os_malloc(C_SIZEOF(d_mergeState)*count);

                helper.index = 0;
                if (ns->mergedStatesCount > 0) {
                    helper.states = (d_mergeState) ns->mergedStates;

                    d_tableWalk(nameSpace->mergedRoleStates, addMergeState, &helper);
                }
                if (ns->aligner == TRUE) {
                    d_mergeStateInit(&(((d_mergeState)ns->mergedStates)[helper.index]),
                                     "masterPriority",
                                     masterPriority);
                }
            } else {
                ns->mergedStates = NULL;
            }

            d_networkAddressFree(master);
        }
    }
    return ns;
}

d_networkAddress
d_nameSpacesGetMaster(
        d_nameSpaces nameSpaces)
{
    d_networkAddress addr = NULL;

    if(nameSpaces){
        addr = d_networkAddressNew( nameSpaces->master.systemId,
                                    nameSpaces->master.localId,
                                    nameSpaces->master.lifecycleId);
    }
    return addr;
}

void
d_nameSpacesSetMaster(
    d_nameSpaces nameSpaces,
    d_networkAddress master)
{
    if(nameSpaces && master){
        nameSpaces->master.systemId    = master->systemId;
        nameSpaces->master.localId     = master->localId;
        nameSpaces->master.lifecycleId = master->lifecycleId;
    }
    return;
}

char *
d_nameSpacesGetName(
    d_nameSpaces  nameSpaces )
{
    char * name;

    name = NULL;
    if (nameSpaces) {
        name = nameSpaces->name;
    }
    return name;
}


void
d_nameSpacesSetTotal(
    d_nameSpaces nameSpaces,
    c_ulong total)
{
    if (nameSpaces)
    {
        nameSpaces->total = total;
    }
}

d_alignmentKind
d_nameSpacesGetAlignmentKind(
    d_nameSpaces nameSpaces)
{
    d_alignmentKind kind = D_ALIGNEE_INITIAL;

    if(nameSpaces){
        kind = nameSpaces->alignmentKind;
    }
    return kind;
}

d_durabilityKind
d_nameSpacesGetDurabilityKind(
    d_nameSpaces nameSpaces)
{
    d_durabilityKind kind = D_DURABILITY_ALL;

    if(nameSpaces){
        kind = nameSpaces->durabilityKind;
    }
    return kind;
}

c_bool
d_nameSpacesIsAligner(
    d_nameSpaces nameSpaces)
{
    c_bool aligner = FALSE;

    if(nameSpaces){
        aligner = nameSpaces->aligner;
    }
    return aligner;
}

c_ulong
d_nameSpacesGetTotal(
    d_nameSpaces nameSpaces)
{
    c_ulong total = 0;

    if(nameSpaces){
        total = nameSpaces->total;
    }
    return total;
}

c_char*
d_nameSpacesGetPartitions(
    d_nameSpaces nameSpaces)
{
    c_char* partitions = NULL;

    if(nameSpaces){
        if(nameSpaces->partitions){
            partitions = os_strdup(nameSpaces->partitions);
        }
    }
    return partitions;
}

void
d_nameSpacesFree(
    d_nameSpaces nameSpaces)
{
    c_ulong count, i;

    if(nameSpaces){
        if(nameSpaces->name){
            os_free(nameSpaces->name);
        }
        if(nameSpaces->partitions){
            os_free(nameSpaces->partitions);
            nameSpaces->partitions = NULL;
        }
        d_mergeStateDeinit(&nameSpaces->state);

        count = nameSpaces->mergedStatesCount;
        if (nameSpaces->aligner == TRUE) {
            count++;
        }

        if(count > 0){
            d_mergeState ms = (d_mergeState) nameSpaces->mergedStates;
            for(i=0; i<count; i++){
                d_mergeStateDeinit(&ms[i]);
            }
        }
        os_free(nameSpaces->mergedStates);
        d_messageDeinit(d_message(nameSpaces));
        os_free(nameSpaces);
    }
}

d_quality
d_nameSpacesGetInitialQuality(
    d_nameSpaces nameSpaces)
{
    d_quality quality = D_QUALITY_ZERO;

    if (nameSpaces) {
        /* Convert qualityExt (c_time) to quality (os_timeW) */
        d_qualityExtToQuality(&quality, &nameSpaces->initialQuality, IS_Y2038READY(nameSpaces));
    }
    return quality;
}

int
d_nameSpacesCompare(
    d_nameSpaces ns1,
    d_nameSpaces ns2)
{
    int r;

    if((!ns1) && (!ns2)){
        r = 0;
    } else if(!ns1){
        r = 1;
    } else if(!ns2){
        r = -1;
    } else if (ns1->aligner && !(ns2->aligner)){
        r = 1;
    }else if (!(ns1->aligner) && ns2->aligner){
        r = -1;
    } else if(ns1->alignmentKind != ns2->alignmentKind){
        if(((c_ulong)ns1->alignmentKind) > ((c_ulong)ns2->alignmentKind)){
            r = 1;
        } else {
            r = -1;
        }
    } else if(ns1->durabilityKind != ns2->durabilityKind) {
        if(((c_ulong)ns1->durabilityKind) > ((c_ulong)ns2->durabilityKind)){
            r = 1;
        } else {
            r = -1;
        }
    } else if((!ns1->partitions) && (!ns2->partitions)){
        r = 0;
    } else if(!ns1->partitions){
        r = 1;
    } else if(!ns2->partitions){
        r = -1;
    } else {
        r = strcmp(ns1->partitions, ns2->partitions);
    }
    return r;
}
