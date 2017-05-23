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


#include "cmx_factory.h"
#include "cmx__factory.h"
#include "cmx__entity.h"
#include "cmx__snapshot.h"
#include "cmx__readerSnapshot.h"
#include "cmx__writerSnapshot.h"
#include "c_iterator.h"
#include "u_user.h"
#include "u_types.h"
#include "u_service.h"
#include "u_entity.h"
#include "ut_collection.h"
#include "v_entity.h"
#include "vortex_os.h"
#include "os_version.h"
#include "os_defs.h"
#include "os_version.h"
#include "os_gitrev.h"
#include "os_atomics.h"
#include <string.h>

static ut_collection cmx_uobjects = NULL;
static os_mutex      cmx_mutex;
static os_boolean    cmx_initialized         = OS_FALSE;
static os_boolean    cmx_mustDetach          = OS_FALSE;
static os_mutex      cmx_readerSnapshotMutex;
static os_mutex      cmx_writerSnapshotMutex;
static pa_uint32_t   cmx_initCount = {0};

static os_equality
cmx_factoryObjectCompare(
    void *o1,
    void *o2,
    void *unused)
{
    OS_UNUSED_ARG(unused);

    if(o1 == o2){
        return OS_EQ;
    } else if( o1 > o2){
        return OS_GT;
    } else {
        return OS_LT;
    }
}

static void
cmx_factoryObjectFree(
    void* entity,
    void* arg)
{
    cmx_entity ce = cmx_entity(entity);

    OS_UNUSED_ARG(arg);

    if(ce){
        cmx_factoryReleaseEntity(ce);
    }
    return;
}

const c_char*
cmx_initialise()
{
    u_result ur;
    const c_char* result;
    os_uint32 initCount;
    os_result osr = os_resultSuccess;

    result = CMX_RESULT_FAILED;

    initCount = pa_inc32_nv(&cmx_initCount);

    if(initCount == 1){
        ur = u_userInitialise();

        if(ur == U_RESULT_OK){
            CMX_SHOW_DEFINE(PA_ADDRFMT);

            if(osr == os_resultSuccess){
                osr = os_mutexInit(&cmx_mutex, NULL);

                if(osr == os_resultSuccess){
                    osr = os_mutexInit(&cmx_readerSnapshotMutex, NULL);

                    if(osr == os_resultSuccess){
                        osr = os_mutexInit(&cmx_writerSnapshotMutex, NULL);

                        if(osr == os_resultSuccess){
                            cmx_uobjects = (ut_collection)ut_tableNew(cmx_factoryObjectCompare, NULL,
                                                                      NULL, NULL,
                                                                      cmx_factoryObjectFree, NULL);
                            result = CMX_RESULT_OK;
                            cmx_initialized = OS_TRUE;
                        } else {
                            os_mutexDestroy(&cmx_readerSnapshotMutex);
                            os_mutexDestroy(&cmx_mutex);
                            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                                      "cmx_initialise: mutexInit failed.");
                        }
                    } else {
                        os_mutexDestroy(&cmx_mutex);
                        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                                  "cmx_initialise: mutexInit failed.");
                    }
                } else {
                    OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                              "cmx_initialise: mutexInit failed.");
                }
            } else {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "cmx_initialise: license checkout failed.");
            }
        } else {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                      "cmx_initialise: u_userInitialise failed.");
        }
    } else {
        result = CMX_RESULT_OK;
    }
    return result;
}

const c_char*
cmx_detach()
{
    const c_char* result;
    os_uint32 initCount;

    result = CMX_RESULT_FAILED;

    initCount = pa_dec32_nv(&cmx_initCount);

    if(initCount == 0){
        if((cmx_initialized == OS_TRUE) || (cmx_mustDetach == OS_TRUE)){
            CMX_TRACE(printf("Detaching...\n"));

            cmx_initialized = OS_FALSE;
            cmx_mustDetach  = OS_FALSE;
            cmx_snapshotFreeAll();

            os_mutexLock(&cmx_mutex);
            if (cmx_uobjects) {
                ut_tableFree((ut_table)cmx_uobjects);
                cmx_uobjects = NULL;
            }
            os_mutexUnlock(&cmx_mutex);
            os_mutexDestroy(&cmx_mutex);
            os_mutexDestroy(&cmx_readerSnapshotMutex);
            os_mutexDestroy(&cmx_writerSnapshotMutex);
            result = CMX_RESULT_OK;
        } else {
            result = CMX_RESULT_OK;
        }
    } else {
        result = CMX_RESULT_OK;
    }
    CMX_TRACE(printf("Detaching done.\n"));
    return result;
}

v_kind
cmx_resolveKind(
    const c_char* kind)
{
    v_kind vk;

    if(kind == NULL){
        vk = K_ENTITY;
    } else if(strcmp(kind, "ENTITY") == 0){
        vk = K_ENTITY;
    } else if(strcmp(kind, "KERNEL") == 0){
        vk = K_KERNEL;
    } else if(strcmp(kind, "TOPIC") == 0){
        vk = K_TOPIC;
    } else if(strcmp(kind, "QUERY") == 0){
        vk = K_QUERY;
    } else if(strcmp(kind, "DOMAIN") == 0){
        vk = K_DOMAIN;
    } else if(strcmp(kind, "READER") == 0){
        vk = K_READER;
    } else if(strcmp(kind, "DATAREADER") == 0){
        vk = K_DATAREADER;
    } else if(strcmp(kind, "WRITER") == 0){
        vk = K_WRITER;
    } else if(strcmp(kind, "SUBSCRIBER") == 0){
        vk = K_SUBSCRIBER;
    } else if(strcmp(kind, "PUBLISHER") == 0){
        vk = K_PUBLISHER;
    } else if(strcmp(kind, "PARTICIPANT") == 0){
        vk = K_PARTICIPANT;
    } else if(strcmp(kind, "SERVICE") == 0){
        vk = K_SERVICE;
    } else if(strcmp(kind, "SERVICESTATE") == 0){
        vk = K_SERVICESTATE;
    } else if(strcmp(kind, "NETWORKREADER") == 0){
        vk = K_NETWORKREADER;
    } else if(strcmp(kind, "GROUPQUEUE") == 0){
        vk = K_GROUPQUEUE;
    } else{
        OS_REPORT(OS_ERROR,
                    "cmx_factory.c",78,
                    "cmx_resolveKind: supplied kind unknown: '%s'",
                    kind);
        assert(0);
        vk = K_ENTITY;
    }
    return vk;
}

c_char*
cmx_convertToXMLList(
    c_iter xmlEntities,
    c_ulong length)
{
    c_char* result;
    c_char* temp;

    result = (c_char*)(os_malloc(length + 14));
    memset(result, 0, length + 14);
    os_sprintf(result, "<list>");
    temp = (c_char*)(c_iterTakeFirst(xmlEntities));

    while(temp != NULL){
       result = os_strcat(result, temp);
       os_free(temp);
       temp = (c_char*)(c_iterTakeFirst(xmlEntities));
    }
    c_iterFree(xmlEntities);

    result = os_strcat(result, "</list>");
    return result;
}

cmx_entity
cmx_registerObject(
    u_object object,
    cmx_entity participant)
{
    cmx_entity ce = NULL;
    os_int32 inserted = 0;

    if(object != NULL){
        ce = (cmx_entity)os_malloc(OS_SIZEOF(cmx_entity));

        if(ce){
            pa_st32(&ce->claimCount, 1);
            ce->uentity = object;
            ce->participant = participant;

            os_mutexLock(&cmx_mutex);
            CMX_TRACE(printf("  Allocating entity 0x%x (%s) ...\n",
                        ce->uentity,
                        u_kindImage(u_objectKind(u_object(ce->uentity)))
                      )
                    );
            inserted = ut_tableInsert((ut_table)cmx_uobjects, object, ce);
            os_mutexUnlock(&cmx_mutex);
            if (inserted) {
                if (participant) {
                    os_uint32 claimCount = pa_inc32_nv(&participant->claimCount);
                    assert(claimCount > (claimCount - 1));
                    (void)claimCount;
                }
            } else {
                ce = cmx_factoryClaimEntity(object);
            }
        } else {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "cmx_registerObject: allocation of object failed.");
        }
    }
    return ce;
}

void
cmx_deregisterObject(
    u_object object)
{
    cmx_entity ce;

    if (object) {
        os_mutexLock(&cmx_mutex);
        ce = cmx_entity(ut_remove(cmx_uobjects, object));
        os_mutexUnlock(&cmx_mutex);

        if(ce){
            cmx_factoryReleaseEntity(ce);
        }
    }
    return;
}

cmx_entity
cmx_factoryClaimEntity(
    u_object object)
{
    cmx_entity ce = NULL;

    if (object) {
        os_mutexLock(&cmx_mutex);

        ce = cmx_entity(ut_get(cmx_uobjects, object));

            if(ce){
#ifndef NDEBUG
                os_uint32 claimCount = pa_inc32_nv(&ce->claimCount);
                assert(claimCount > (claimCount - 1));
#else
                pa_inc32(&ce->claimCount);
#endif
            }
            os_mutexUnlock(&cmx_mutex);
    }
    return ce;
}

void
cmx_factoryReleaseEntity(
    cmx_entity entity)
{
    if (entity) {
        os_uint32 claimCount = pa_dec32_nv(&entity->claimCount);
        assert(claimCount < (claimCount + 1));
        if(claimCount == 0){
            assert(cmx_uobjects?ut_get(cmx_uobjects, entity->uentity) == NULL:TRUE);

            if (entity->uentity) {
                CMX_TRACE(printf("  Freeing entity 0x%x (%s) ...\n",
                            entity->uentity,
                            u_kindImage(u_objectKind(u_object(entity->uentity)))
                          )
                        );
                u_objectFree(entity->uentity);
            }
            if(entity->participant){
                cmx_factoryReleaseEntity(entity->participant);
            }
            os_free(entity);
        }
    }
    return;
}

void
cmx_deregisterAllEntities()
{
    if(cmx_initialized == OS_TRUE){
        /*
         * OSPL-7643:
         * Potentially, in SP-mode, a parallel JNI and SOAP connection to
         * the C&M API can exist within the same process. The SOAP service
         * garbage collection mechanism below will clean up all entities
         * managed by the C&M API when all connections are closed (either by
         * normal disconnects by all clients or when their leases expire due
         * to a broken network connection or crash of the client). The SOAP
         * garbage collector instructs the C&M API to destroy all entities
         * that still exist using this operation. This leads to a problem in
         * case their is also still an open JNI connection as the entities
         * created through that connection are also destroyed. When no
         * participants are created through any other API in the same process,
         * this leads to deletion of the last participant for the domain and
         * this causes all services to stop. Therefore only if this API has
         * been initialized once, entities are deregistered for now. This
         * check is not thread-safe, but this operation will only work if at
         * least init has been performed anyway. It also assumes only the SOAP
         * service calls this operation, which is true right now. If this ever
         * changes, something may need to be changed here.
         */
        if(pa_ld32(&cmx_initCount) == 1){
            os_mutexLock(&cmx_mutex);
            if (cmx_uobjects) {
                ut_tableClear((ut_table)cmx_uobjects);
            }
            os_mutexUnlock(&cmx_mutex);
        }
    }
}

os_mutex
cmx_getReaderSnapshotMutex()
{
    return cmx_readerSnapshotMutex;
}

os_mutex
cmx_getWriterSnapshotMutex()
{
    return cmx_writerSnapshotMutex;
}

os_boolean
cmx_isInitialized()
{
    return cmx_initialized;
}

void
cmx_internalDetach()
{
    if(cmx_initialized == OS_TRUE){
        cmx_initialized = OS_FALSE;
        cmx_mustDetach = OS_TRUE;
    }
}

c_char*
cmx_getVersion()
{
    char* result;
#if defined(OSPL_INNER_REV) && defined (OSPL_OUTER_REV)
        result = OSPL_VERSION_STR ", build " OSPL_INNER_REV_STR "/" OSPL_OUTER_REV_STR "";
#else
        result = OSPL_VERSION_STR ", non-PrismTech build";
#endif
    return (c_char*)(os_strdup(result));
}

