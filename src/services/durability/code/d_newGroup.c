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

#include "d_newGroup.h"
#include "d_message.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "d__misc.h"

_Ret_notnull_
d_newGroup
d_newGroupNew(
    _In_ d_admin admin,
    _In_opt_z_ const c_char* partition,
    _In_opt_z_ const c_char* topic,
    _In_ d_durabilityKind kind,
    _In_ d_completeness completeness,
    _In_ d_quality quality)
{
    d_newGroup newGroup;

    newGroup = os_malloc(sizeof *newGroup);
    d_messageInit(d_message(newGroup), admin);

    newGroup->partition           = partition ? os_strdup(partition) : NULL;
    newGroup->topic               = topic ? os_strdup(topic) : NULL;
    newGroup->durabilityKind      = kind;
    newGroup->completeness        = completeness;
    newGroup->alignerCount        = 0;
    /* Quality conversion */
    d_qualityExtFromQuality(&newGroup->quality, &quality, IS_Y2038READY(newGroup));

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
