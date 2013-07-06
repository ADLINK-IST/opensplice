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
    cmx_writerSnapshot s;
    const c_char* result;
    
    s = NULL;
    result = NULL;
    
    if(snapshot != NULL){
        copy = (c_char*)(os_malloc(strlen(snapshot) + 1));
        os_strcpy(copy, snapshot);
        temp = strtok((c_char*)copy, "</>");    /*<xxxxxxSnapshot>*/
        
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
