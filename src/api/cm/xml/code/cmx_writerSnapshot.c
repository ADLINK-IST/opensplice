/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include "cmx_writerSnapshot.h"
#include "cmx__writerSnapshot.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "sd_serializer.h"
#include "sd_serializerXML.h"
#include "u_observable.h"
#include "v_kernel.h"
#include "v_writer.h"
#include "v_writerSample.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"

static c_iter writerSnapshots = NULL;

c_char*
cmx_writerSnapshotNew(
    const c_char* writer)
{
    cmx_entity ce;
    c_char* result;
    os_mutex m;
    struct cmx_writerSnapshotArg arg;

    arg.snapshot = NULL;
    arg.success = FALSE;
    arg.serializer = NULL;
    result = NULL;

    ce = cmx_entityClaim(writer);
    if(ce != NULL){
        if (u_observableAction(u_observable(ce->uentity),
                               cmx_writerSnapshotNewAction,
                               &arg) == U_RESULT_OK)
        {
            if(arg.success == TRUE){
                m = cmx_getWriterSnapshotMutex();
                os_mutexLock(&m);
                writerSnapshots = c_iterInsert(writerSnapshots, arg.snapshot);
                os_mutexUnlock(&m);

                result = (c_char*)(os_malloc(60));
                os_sprintf(result,
                           "<writerSnapshot><id>"PA_ADDRFMT"</id></writerSnapshot>",
                           (c_address)(arg.snapshot));
            }
        }
        cmx_entityRelease(ce);
    }
    return result;
}

void
cmx_writerSnapshotNewAction(
    v_public p,
    c_voidp args)
{
    v_writer writer;
    struct cmx_writerSnapshotArg* arg;

    arg = (struct cmx_writerSnapshotArg*)args;

    switch(v_object(p)->kind){
    case K_WRITER:
        arg->success = TRUE;
        arg->snapshot = cmx_writerSnapshot(os_malloc(C_SIZEOF(cmx_writerSnapshot)));
        arg->snapshot->samples = NULL;
        writer = v_writer(p);
        v_writerRead(writer, cmx_writerHistoryCopy, args);
        if(arg->serializer != NULL){
            sd_serializerFree(arg->serializer);
        }
    break;
    default:
    break;
    }
}

c_bool
cmx_writerHistoryCopy(
    c_object sample,
    c_voidp args)
{
    struct cmx_writerSnapshotArg* arg;
    sd_serializedData data;
    c_char* xml;

    arg = (struct cmx_writerSnapshotArg*)args;

    if(arg->serializer == NULL){
        arg->serializer = sd_serializerXMLNewTyped(c_getType(sample));
    }

    data = sd_serializerSerialize(arg->serializer, sample);
    xml = sd_serializerToString(arg->serializer, data);
    arg->snapshot->samples = c_iterInsert(arg->snapshot->samples, xml);
    sd_serializedDataFree(data);

    return TRUE;
}

void
cmx_writerSnapshotFree(
    c_char* snapshot)
{
    cmx_writerSnapshot s;
    c_char* sample;
    os_mutex m;

    s = cmx_writerSnapshotLookup(snapshot);

    if(s != NULL){
        m = cmx_getWriterSnapshotMutex();
        os_mutexLock(&m);
        c_iterTake(writerSnapshots, s);
        os_mutexUnlock(&m);

        if(s->samples != NULL){
            sample = (c_char*)(c_iterTakeFirst(s->samples));

            while(sample != NULL){
                os_free(sample);
                sample = (c_char*)(c_iterTakeFirst(s->samples));
            }
            c_iterFree(s->samples);
        }
        os_free(s);
        os_free(snapshot);
    }
}

void
cmx_writerSnapshotFreeAll()
{
    cmx_writerSnapshot s;
    c_char* sample;
    os_mutex m;

    m = cmx_getWriterSnapshotMutex();
    os_mutexLock(&m);
    s = cmx_writerSnapshot(c_iterTakeFirst(writerSnapshots));

    while(s != NULL){
        if(s->samples != NULL){
            sample = (c_char*)(c_iterTakeFirst(s->samples));

            while(sample != NULL){
                os_free(sample);
                sample = (c_char*)(c_iterTakeFirst(s->samples));
            }
            c_iterFree(s->samples);
        }
        os_free(s);
        s = cmx_writerSnapshot(c_iterTakeFirst(writerSnapshots));
    }
    os_mutexUnlock(&m);
}

c_char*
cmx_writerSnapshotRead(
    const c_char* snapshot)
{
    cmx_writerSnapshot s;
    c_char* result;
    c_char* temp;
    s = cmx_writerSnapshotLookup(snapshot);
    result = NULL;

    if(s != NULL){
        temp = (c_char*)(c_iterObject(s->samples, 0));

        if(temp != NULL){
            result = (c_char*)(os_strdup(temp));
        }
    }
    return result;
}

c_char*
cmx_writerSnapshotTake(
    const c_char* snapshot)
{
    cmx_writerSnapshot s;
    c_char* result;
    s = cmx_writerSnapshotLookup(snapshot);
    result = NULL;

    if(s != NULL){
        result = (c_char*)(c_iterTakeFirst(s->samples));
    }
    return result;
}

cmx_writerSnapshot
cmx_writerSnapshotLookup(
    const c_char* snapshot)
{
    c_char* saveptr;
    c_char* copy;
    c_char* temp;
    cmx_writerSnapshot s;
    os_mutex m;

    s = NULL;

    if(snapshot != NULL){
        copy = (c_char*)(os_malloc(strlen(snapshot) + 1));
        os_strcpy(copy, snapshot);
        temp = os_strtok_r((c_char*)copy, "</>", &saveptr);    /*<writerSnapshot>*/
        temp = os_strtok_r(NULL, "</>", &saveptr);             /*<id>*/
        temp = os_strtok_r(NULL, "</>", &saveptr);             /*... the pointer*/

        if(temp != NULL){
            (void)sscanf(temp, PA_ADDRFMT, (c_address *)(&s));

            m = cmx_getWriterSnapshotMutex();
            os_mutexLock(&m);

            if(c_iterContains(writerSnapshots, s) == FALSE){
                s = NULL;
            }
            os_mutexUnlock(&m);
        }
        os_free(copy);
    }
    return s;
}
