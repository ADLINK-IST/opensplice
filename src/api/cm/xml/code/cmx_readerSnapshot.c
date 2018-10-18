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
#include "cmx_readerSnapshot.h"
#include "cmx__readerSnapshot.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "u_observable.h"
#include "u_reader.h"
#include "v_kernel.h"
#include "v_dataReader.h"
#include "v_observer.h"
#include "v_query.h"
#include "v_handle.h"
#include "v_index.h"
#include "v_dataReaderInstance.h"
#include "c_typebase.h"
#include "c_iterator.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "sd_serializerXML.h"
#include "os_abstract.h"

static c_iter readerSnapshots = NULL;

c_char*
cmx_readerSnapshotNew(
    const c_char* reader)
{
    cmx_entity ce;
    c_char* result;
    struct cmx_readerSnapshotArg arg;
    os_mutex m;

    arg.success = FALSE;
    result = NULL;

    ce = cmx_entityClaim(reader);
    if(ce != NULL){
        if (u_observableAction(u_observable(ce->uentity),
                               cmx_readerSnapshotNewAction,
                               &arg) == U_RESULT_OK)
        {
            if(arg.success == TRUE){
                m = cmx_getReaderSnapshotMutex();
                os_mutexLock(&m);
                readerSnapshots = c_iterInsert(readerSnapshots, arg.snapshot);
                os_mutexUnlock(&m);

                result = (c_char*)(os_malloc(60));
                os_sprintf(result,
                           "<readerSnapshot><id>"PA_ADDRFMT"</id></readerSnapshot>",
                           (c_address)(arg.snapshot));
            }
        }
        cmx_entityRelease(ce);
    }
    return result;
}

void
cmx_readerSnapshotNewAction(
    v_public p,
    c_voidp args)
{
    v_dataReader reader;
    c_iter instances;
    v_dataReaderInstance instance;
    v_dataReaderSample sample;
    v_query query;
    c_bool release;
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_readerSnapshotArg* arg;

    release = FALSE;
    arg = (struct cmx_readerSnapshotArg*)args;
    reader = NULL;
    instances = NULL;
    ser = NULL;

    switch(v_object(p)->kind){
    case K_DATAREADER:
        reader = v_dataReader(p);
        arg->success = TRUE;
        arg->snapshot = cmx_readerSnapshot(os_malloc(C_SIZEOF(cmx_readerSnapshot)));
        v_observerLock(v_observer(reader));

        if(reader->index->objects){
            instances = ospl_c_select(reader->index->notEmptyList, 0);
        }
    break;
    case K_QUERY:
    case K_DATAREADERQUERY:
        query = v_query(p);
        reader = v_dataReader(v_querySource(query));

        if(reader != NULL){
            release = TRUE;
            arg->success = TRUE;
            arg->snapshot = cmx_readerSnapshot(os_malloc(C_SIZEOF(cmx_readerSnapshot)));
            v_observerLock(v_observer(reader));

            switch(v_object(query)->kind){
            case K_DATAREADERQUERY:
                if(v_dataReaderQuery(query)->instanceQ){
                    instances = ospl_c_select((c_collection)(v_dataReaderQuery(query)->instanceQ), 0);
                }
            break;
            default:
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "cmx_readerSnapshotNewAction unknown kind (%d).",
                    v_object(query)->kind);
            break;
            }
        }
    break;
    default:
    break;
    }
    if(arg->success == TRUE){
        arg->snapshot->samples = c_iterNew(NULL);
    }
    if(instances != NULL){
        v_dataReaderSample sampleShallowCopy = NULL;
        instance = v_dataReaderInstance(c_iterTakeFirst(instances));

        while(instance != NULL){
            v_state ivState = instance->_parent._parent.state & (L_DISPOSED | L_NOWRITERS | L_NEW);

            sample = v_dataReaderInstanceOldest(instance);
            if (sample != NULL) {
                do {
                    v_state state = ivState | ((sample->_parent.sampleState & (L_READ | L_LAZYREAD)) ? L_READ : 0);

                    if (sampleShallowCopy == NULL) {
                        sampleShallowCopy = c_new(c_getType(c_object(sample)));
                    }
                    memcpy(sampleShallowCopy, sample, c_typeSize(c_getType(sampleShallowCopy)));
                    sampleShallowCopy->newer = NULL;
                    sampleShallowCopy->_parent.sampleState &= ~(L_DISPOSED | L_NOWRITERS | L_NEW | L_READ | L_LAZYREAD);
                    sampleShallowCopy->_parent.sampleState |= state;

                    if(ser == NULL){
                        ser = sd_serializerXMLNewTyped(c_getType(c_object(sampleShallowCopy)));
                    }
                    data = sd_serializerSerialize(ser, c_object(sampleShallowCopy));
                    arg->snapshot->samples = c_iterInsert(arg->snapshot->samples, sd_serializerToString(ser, data));
                    sd_serializedDataFree(data);

                    sample = sample->newer;
                } while (sample != NULL);
            }
            c_free(instance);
            instance = v_dataReaderInstance(c_iterTakeFirst(instances));
        }
        c_iterFree(instances);

        if (sampleShallowCopy != NULL) {
            memset(sampleShallowCopy, 0, c_typeSize(c_getType(sampleShallowCopy)));
            c_free(sampleShallowCopy);
        }
    }
    if(reader != NULL){
        v_observerUnlock(v_observer(reader));

        if(release == TRUE){
            c_free(reader);
        }
    }
    if(ser != NULL){
        sd_serializerFree(ser);
    }
}

void
cmx_readerSnapshotFree(
    c_char* snapshot)
{
    cmx_readerSnapshot s;
    c_char* sample;
    os_mutex m;

    s = cmx_readerSnapshotLookup(snapshot);

    if(s != NULL){
        m = cmx_getReaderSnapshotMutex();
        os_mutexLock(&m);
        c_iterTake(readerSnapshots, s);
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
cmx_readerSnapshotFreeAll()
{
    cmx_readerSnapshot s;
    c_char* sample;
    os_mutex m;

    m = cmx_getReaderSnapshotMutex();
    os_mutexLock(&m);
    s = cmx_readerSnapshot(c_iterTakeFirst(readerSnapshots));

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
        s = cmx_readerSnapshot(c_iterTakeFirst(readerSnapshots));
    }
    os_mutexUnlock(&m);
}

c_char*
cmx_readerSnapshotRead(
    const c_char* snapshot)
{
    cmx_readerSnapshot s;
    c_char* result;
    c_char* temp;
    s = cmx_readerSnapshotLookup(snapshot);
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
cmx_readerSnapshotTake(
    const c_char* snapshot)
{
    cmx_readerSnapshot s;
    c_char* result;
    s = cmx_readerSnapshotLookup(snapshot);
    result = NULL;

    if(s != NULL){
        result = (c_char*)(c_iterTakeFirst(s->samples));
    }
    return result;
}

cmx_readerSnapshot
cmx_readerSnapshotLookup(
    const c_char* snapshot)
{
    c_char* saveptr;
    c_char* copy;
    c_char* temp;
    cmx_readerSnapshot s;
    os_mutex m;

    s = NULL;

    if(snapshot != NULL){
        copy = (c_char*)(os_malloc(strlen(snapshot) + 1));
        if (copy != NULL){
            os_strcpy(copy, snapshot);
            temp = os_strtok_r((c_char*)copy, "</>", &saveptr);    /*<readerSnapshot>*/
            temp = os_strtok_r(NULL, "</>", &saveptr);             /*<id>*/
            temp = os_strtok_r(NULL, "</>", &saveptr);             /*... the pointer*/

            if(temp != NULL){
                (void)sscanf(temp, PA_ADDRFMT, (c_address *)(&s));

                m = cmx_getReaderSnapshotMutex();
                os_mutexLock(&m);

                if(c_iterContains(readerSnapshots, s) == FALSE){
                    s = NULL;
                }
                os_mutexUnlock(&m);
            }
            os_free(copy);
        }
    }
    return s;
}
