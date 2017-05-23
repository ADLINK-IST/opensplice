/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "cmx__snapshot.h"
#include "cmx_snapshot.h"
#include "cmx_readerSnapshot.h"
#include "cmx__readerSnapshot.h"
#include "cmx_writerSnapshot.h"
#include "cmx__writerSnapshot.h"
#include "cmx__entity.h"
#include "os_heap.h"
#include "os_stdlib.h"

void
cmx_snapshotFree(
    c_char* snapshot)
{
    const c_char* kind;

    kind = cmx_snapshotKind(snapshot);

    if(strcmp(kind, "READERSNAPSHOT") == 0){
        cmx_readerSnapshotFree(snapshot);
    } else if(strcmp(kind, "WRITERSNAPSHOT") == 0){
         cmx_writerSnapshotFree(snapshot);
    }
}

c_char*
cmx_snapshotRead(
    const c_char* snapshot)
{
    c_char* result;
    const c_char* kind;

    result = NULL;
    kind = cmx_snapshotKind(snapshot);

    if(strcmp(kind, "READERSNAPSHOT") == 0){
        result = cmx_readerSnapshotRead(snapshot);
    } else if(strcmp(kind, "WRITERSNAPSHOT") == 0){
         result = cmx_writerSnapshotRead(snapshot);
    }
    return result;
}

c_char*
cmx_snapshotTake(
    const c_char* snapshot)
{
    c_char* result;
    const c_char* kind;

    result = NULL;
    kind = cmx_snapshotKind(snapshot);

    if(strcmp(kind, "READERSNAPSHOT") == 0){
        result = cmx_readerSnapshotTake(snapshot);
    } else if(strcmp(kind, "WRITERSNAPSHOT") == 0){
         result = cmx_writerSnapshotTake(snapshot);
    }
    return result;
}


void
cmx_snapshotFreeAll()
{
    cmx_readerSnapshotFreeAll();
    cmx_writerSnapshotFreeAll();
}

const c_char*
cmx_snapshotKind(
    const c_char* snapshot)
{
    c_char* copy;
    c_char* temp;
    c_char* saveptr;
    const c_char* result;

    result = NULL;

    if(snapshot != NULL){
        copy = (c_char*)(os_malloc(strlen(snapshot) + 1));
        os_strcpy(copy, snapshot);
        temp = os_strtok_r((c_char*)copy, "</>", &saveptr);    /*<xxxxxxSnapshot>*/

        if(temp != NULL){
            if(strcmp(temp, "readerSnapshot") == 0){
                result = "READERSNAPSHOT";
            } else if(strcmp(temp, "writerSnapshot") == 0){
                result = "WRITERSNAPSHOT";
            }
        }
        os_free(copy);
    }
    return result;
}
