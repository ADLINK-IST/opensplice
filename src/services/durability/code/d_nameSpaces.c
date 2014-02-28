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

#include "d_nameSpaces.h"
#include "d__nameSpaces.h"
#include "d_nameSpace.h"
#include "d_networkAddress.h"
#include "d__nameSpace.h"
#include "d__mergeState.h"
#include "d_admin.h"
#include "d_message.h"
#include "d_table.h"
#include "os.h"

struct nsWalkHelper {
    d_mergeState* states;
    c_ulong index;
};

static c_bool
addMergeState(
    d_mergeState state,
    struct nsWalkHelper* helper)
{
    ((helper->states)[helper->index])->role = os_strdup(state->role);
    ((helper->states)[helper->index])->value = state->value;
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
    c_sequence *mergedStatesPtr;
    struct nsWalkHelper helper;

    if(nameSpace){
        ns = d_nameSpaces(os_malloc(C_SIZEOF(d_nameSpaces)));

        if(ns){
            master     = d_networkAddressUnaddressed();
            d_messageInit(d_message(ns), admin);

            ns->aligner                    = d_nameSpaceIsAligner(nameSpace);
            ns->durabilityKind             = d_nameSpaceGetDurabilityKind(nameSpace);
            ns->alignmentKind              = d_nameSpaceGetAlignmentKind(nameSpace);
            ns->partitions                 = d_nameSpaceGetPartitionTopics(nameSpace);
            ns->total                      = total;
            ns->initialQuality.seconds     = initialQuality.seconds;
            ns->initialQuality.nanoseconds = initialQuality.nanoseconds;
            ns->master.systemId            = master->systemId;
            ns->master.localId             = master->localId;
            ns->master.lifecycleId         = master->lifecycleId;
            ns->isComplete                 = TRUE;
            ns->name                       = os_strdup (d_nameSpaceGetName(nameSpace));
            ns->masterConfirmed            = d_nameSpaceIsMasterConfirmed(nameSpace);

            state = d_nameSpaceGetMergeState(nameSpace, NULL);
            if(state) {
                ns->state.role                 = os_strdup(state->role);
                ns->state.value                = state->value;
                d_mergeStateFree(state);
            } else {
                ns->state.role                 = d_nameSpaceGetRole(nameSpace);
                ns->state.value                = -1;
            }

            ns->mergedStatesCount          = d_tableSize(nameSpace->mergedRoleStates);

            if(ns->mergedStatesCount > 0){
                ns->mergedStates               = os_malloc(C_SIZEOF(d_mergeState)*ns->mergedStatesCount);

                helper.states = (d_mergeState*)(mergedStatesPtr = &(ns->mergedStates));
                helper.index = 0;

                d_tableWalk(nameSpace->mergedRoleStates, addMergeState, &helper);
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
    c_ulong i;

    if(nameSpaces){
        if(nameSpaces->name){
            os_free(nameSpaces->name);
        }
        if(nameSpaces->partitions){
            os_free(nameSpaces->partitions);
            nameSpaces->partitions = NULL;
        }
        if(nameSpaces->state.role){
            os_free(nameSpaces->state.role);
            nameSpaces->state.role = NULL;
        }
        if(nameSpaces->mergedStatesCount > 0){
            for(i=0; i<nameSpaces->mergedStatesCount; i++){
                os_free(d_mergeState(&(nameSpaces->mergedStates[i]))->role);
            }
            os_free(nameSpaces->mergedStates);
        }
        d_messageDeinit(d_message(nameSpaces));
        os_free(nameSpaces);
    }
}

d_quality
d_nameSpacesGetInitialQuality(
    d_nameSpaces nameSpaces)
{
    d_quality quality;
    quality.seconds = 0;
    quality.nanoseconds = 0;

    if(nameSpaces){
        quality.seconds     = nameSpaces->initialQuality.seconds;
        quality.nanoseconds = nameSpaces->initialQuality.nanoseconds;
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
