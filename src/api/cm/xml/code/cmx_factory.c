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


#include "cmx_factory.h"
#include "cmx__factory.h"
#include "cmx__entity.h"
#include "cmx__snapshot.h"
#include "cmx__readerSnapshot.h"
#include "cmx__writerSnapshot.h"
#include "c_iterator.h"
#include "u_user.h"
#include "u_service.h"
#include "u_entity.h"
#include "v_entity.h"
#include "os.h"
#include "os_version.h"
#include <string.h>
#include "os_version.h"
#include "os_gitrev.h"

static c_iter       cmx_allocatedEntities   = NULL;
static os_mutex     cmx_allocationMutex;
static c_bool       cmx_initialized         = FALSE;
static c_bool       cmx_mustDetach          = FALSE;
static os_mutex     cmx_readerSnapshotMutex;
static os_mutex     cmx_writerSnapshotMutex;
static u_service    cmx_detachService       = NULL;

const c_char*
cmx_initialise()
{
    u_result ur;
    os_mutexAttr attr;
    const c_char* result;
    os_result osr = os_resultSuccess;

    result = CMX_RESULT_FAILED;

    ur = u_userInitialise();

    if(ur == U_RESULT_OK){
        if(osr == os_resultSuccess){
            osr = os_mutexAttrInit(&attr);
            attr.scopeAttr = OS_SCOPE_PRIVATE;

            if(osr == os_resultSuccess){
                osr = os_mutexInit(&cmx_allocationMutex, &attr);

                if(osr == os_resultSuccess){
                    cmx_allocatedEntities = c_iterNew(NULL);
                    osr = os_mutexInit(&cmx_readerSnapshotMutex, &attr);

                    if(osr == os_resultSuccess){
                        osr = os_mutexInit(&cmx_writerSnapshotMutex, &attr);

                        if(osr == os_resultSuccess){
                            result = CMX_RESULT_OK;
                            cmx_initialized = TRUE;
                        } else {
                            os_mutexDestroy(&cmx_readerSnapshotMutex);
                            os_mutexDestroy(&cmx_allocationMutex);
                            u_userDetach();
                            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                                      "cmx_initialise: mutexInit failed.");
                        }
                    } else {
                        os_mutexDestroy(&cmx_allocationMutex);
                        u_userDetach();
                        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                                  "cmx_initialise: mutexInit failed.");
                    }
                } else {
                    u_userDetach();
                    OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                              "cmx_initialise: mutexInit failed.");
                }
            } else {
                u_userDetach();
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "cmx_initialise: mutexAttrInit failed.");
            }
        } else {
            u_userDetach();
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                      "cmx_initialise: license checkout failed.");
        }
    } else {
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "cmx_initialise: u_userInitialise failed.");
    }
    return result;
}

const c_char*
cmx_detach()
{
    u_result ur;
    const c_char* result;
    u_entity entity;
    os_result osr;

    result = CMX_RESULT_FAILED;

    if((cmx_initialized == TRUE) || (cmx_mustDetach == TRUE)){
        cmx_initialized = FALSE;
        cmx_mustDetach  = FALSE;
        cmx_snapshotFreeAll();

        osr = os_mutexLock(&cmx_allocationMutex);

        if(osr == os_resultSuccess){
            entity = u_entity(c_iterTakeFirst(cmx_allocatedEntities));

            while(entity != NULL){
                cmx_entityFreeUserEntity(entity);
                entity = u_entity(c_iterTakeFirst(cmx_allocatedEntities));
            }
            c_iterFree(cmx_allocatedEntities);
            osr = os_mutexUnlock(&cmx_allocationMutex);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "cmx_detach: mutexUnlock failed.");
            }
        } else {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                      "cmx_detach: mutexLock failed.");
        }
        ur = u_userDetach();

        if(ur == U_RESULT_OK){
            osr = os_mutexDestroy(&cmx_allocationMutex);

            if(osr == os_resultSuccess){
                osr = os_mutexDestroy(&cmx_readerSnapshotMutex);

                if(osr == os_resultSuccess){
                    osr = os_mutexDestroy(&cmx_writerSnapshotMutex);

                    if(osr == os_resultSuccess){
                        result = CMX_RESULT_OK;
                    } else {
                        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                                  "cmx_detach: mutexDestroy failed.");
                    }
                } else {
                    OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                              "cmx_detach: mutexDestroy failed.");
                }
            } else {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "cmx_detach: mutexDestroy failed.");
            }
        }
    } else {
        result = CMX_RESULT_OK;
    }
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
        OS_REPORT_1(OS_ERROR,
                    "cmx_factory.c",78,
                    "cmx_resolveKind: supplied kind unknown: '%s'",
                    kind);
        assert(FALSE);
        vk = K_ENTITY;
    }
    return vk;
}

c_char*
cmx_convertToXMLList(
    c_iter xmlEntities,
    c_long length)
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

void
cmx_registerEntity(
    u_entity entity)
{
    os_result osr;

    if(entity != NULL){
        osr = os_mutexLock(&cmx_allocationMutex);

        if(osr == os_resultSuccess){
            cmx_allocatedEntities = c_iterInsert(cmx_allocatedEntities, entity);
            osr = os_mutexUnlock(&cmx_allocationMutex);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "cmx_registerEntity: mutexUnlock failed.");
            }
        } else {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                      "cmx_registerEntity: mutexLock failed.");
        }
    }
}

u_entity
cmx_deregisterEntity(
    u_entity entity)
{
    u_entity ue;
    os_result osr;
    ue = NULL;

    if(entity != NULL){
        osr = os_mutexLock(&cmx_allocationMutex);

        if(osr == os_resultSuccess){
            ue = u_entity(c_iterTake(cmx_allocatedEntities, entity));
            osr = os_mutexUnlock(&cmx_allocationMutex);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "cmx_deregisterEntity: mutexUnlock failed.");
            }
        } else {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                      "cmx_deregisterEntity: mutexLock failed.");
        }
    }
    return ue;
}

void
cmx_deregisterAllEntities()
{
    os_result osr;
    u_entity entity;

    if(cmx_initialized == TRUE){
        osr = os_mutexLock(&cmx_allocationMutex);

        if(osr == os_resultSuccess){
            entity = u_entity(c_iterTakeFirst(cmx_allocatedEntities));

            while(entity != NULL){
                cmx_entityFreeUserEntity(entity);
                entity = u_entity(c_iterTakeFirst(cmx_allocatedEntities));
            }
            osr = os_mutexUnlock(&cmx_allocationMutex);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "cmx_deregisterAllEntities: mutexUnlock failed.");
            }
        } else {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                      "cmx_deregisterAllEntities: mutexLock failed.");
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

c_bool
cmx_isInitialized()
{
    return cmx_initialized;
}

void
cmx_internalDetach()
{
    if(cmx_initialized == TRUE){
        cmx_initialized = FALSE;
        cmx_mustDetach = TRUE;
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

