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

#include "d_newGroup.h"
#include "d_message.h"
#include "os_heap.h"
#include "os_stdlib.h"

d_newGroup
d_newGroupNew(
    d_admin admin,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind,
    d_completeness completeness,
    d_quality quality)
{
    d_newGroup newGroup = NULL;

    if(admin){
        newGroup = d_newGroup(os_malloc(C_SIZEOF(d_newGroup)));
        d_messageInit(d_message(newGroup), admin);

        if(partition){
            newGroup->partition = (c_char*)(os_malloc(strlen(partition) + 1));
            os_sprintf(newGroup->partition, partition);
        } else {
            newGroup->partition = NULL;
        }
        if(topic){
            newGroup->topic = (c_char*)(os_malloc(strlen(topic) + 1));
            os_sprintf(newGroup->topic, topic);
        } else {
            newGroup->topic = NULL;
        }
        newGroup->durabilityKind      = kind;
        newGroup->completeness        = completeness;
        newGroup->quality.seconds     = quality.seconds;
        newGroup->quality.nanoseconds = quality.nanoseconds;
        newGroup->alignerCount        = 0;
    }
    return newGroup;
}

void
d_newGroupSetAlignerCount(
    d_newGroup newGroup,
    c_ulong count)
{
    if(newGroup){
        newGroup->alignerCount = count;
    }
}

void
d_newGroupFree(
    d_newGroup newGroup)
{
    if(newGroup){
        if(newGroup->partition){
            os_free(newGroup->partition);
        }
        if(newGroup->topic){
            os_free(newGroup->topic);
        }
        d_messageDeinit(d_message(newGroup));
        os_free(newGroup);
    }
}

int
d_newGroupCompare(
    d_newGroup g1,
    d_newGroup g2)
{
    int r;

    if(g1 && g2){
        r = strcmp(g1->partition, g2->partition);

        if(r == 0){
            r = strcmp(g1->topic, g2->topic);

            if(r == 0){
                if(g1->durabilityKind != g2->durabilityKind){
                    if(g1->durabilityKind == D_DURABILITY_PERSISTENT){
                        r = 1;
                    } else if(g2->durabilityKind == D_DURABILITY_PERSISTENT){
                        r = -1;
                    } else if(g1->durabilityKind == D_DURABILITY_TRANSIENT){
                        r = 1;
                    } else if(g2->durabilityKind == D_DURABILITY_TRANSIENT){
                        r = -1;
                    } else if(g1->durabilityKind == D_DURABILITY_TRANSIENT_LOCAL){
                        r = 1;
                    } else if(g2->durabilityKind == D_DURABILITY_TRANSIENT_LOCAL){
                        r = -1;
                    } else {
                        assert(FALSE);
                    }
                }
            }
        }
    } else if(!g1 && !g2){
        r = 0;
    } else if(g1){
        r = 1;
    } else {
        r = -1;
    }
    return r;
}
