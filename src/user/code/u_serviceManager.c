/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "u__serviceManager.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__dispatcher.h"
#include "u__participant.h"
#include "u__kernel.h"
#include "u_user.h"

#include "v_kernel.h"
#include "v_entity.h"
#include "v_service.h"

#include "os_stdlib.h"
#include "os_heap.h"
#include "os_report.h"

u_result
u_serviceManagerClaim(
    u_serviceManager _this,
    v_serviceManager *serviceManager)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (serviceManager != NULL)) {
        *serviceManager = v_serviceManager(u_entityClaim(u_entity(_this)));
        if (*serviceManager == NULL) {
            OS_REPORT_2(OS_WARNING, "u_serviceManagerClaim", 0,
                        "Claim ServiceManager failed. "
                        "<_this = 0x%x, serviceManager = 0x%x>.",
                         _this, serviceManager);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_serviceManagerClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, serviceManager = 0x%x>.",
                    _this, serviceManager);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_serviceManagerRelease(
    u_serviceManager _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_serviceManagerRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_serviceManager 
u_serviceManagerNew(
    u_participant participant)
{
    u_result result;
    u_serviceManager m;
    u_kernel k;
    v_kernel kk;
    v_serviceManager sm;

    m = NULL;    
    if (participant != NULL) {
        k = u_participantKernel(participant);
        result = u_kernelClaim(k,&kk);
        if ((result == U_RESULT_OK) && (kk != NULL)) {
            sm = v_getServiceManager(kk);
            if (sm != NULL) {
                m = u_entityAlloc(participant,u_serviceManager,sm,TRUE);
                if (m != NULL) {
                    u_serviceManagerInit(m);
                } else {
                    OS_REPORT(OS_ERROR,"u_serviceManagerNew",0,
                              "Allocation Service Manager proxy failed.");
                }
            } else {
                OS_REPORT(OS_ERROR,"u_serviceManagerNew",0,
                          "Retrieval Service Manager failed.");
            }
            result = u_kernelRelease(k);
        } else {
            OS_REPORT(OS_WARNING,"u_serviceManagerNew",0,
                      "Claim Kernel failed.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_serviceManagerNew",0,
                  "No Participant specified.");
    }
    return m;
}

u_result
u_serviceManagerInit(
    u_serviceManager _this)
{
    u_result result;
    
    if (_this != NULL) {
        result = u_dispatcherInit(u_dispatcher(_this));
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
    } else {
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_serviceManagerFree(
    u_serviceManager _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            /* The following two statements are not according to
               the guidelines of how an entity should be deleted
               because the service manager is shared and the owner
               is the kernel.
             */
            result = u_serviceManagerDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_serviceManagerFree",0,
                  "The specified Service Manager = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_serviceManagerDeinit(
    u_serviceManager _this)
{
    u_result result;

    if (_this == NULL) {
        result = U_RESULT_ILL_PARAM;
    }
    return u_dispatcherDeinit(u_dispatcher(_this));
}

v_serviceStateKind
u_serviceManagerGetServiceStateKind(
    u_serviceManager _this,
    const c_char *serviceName)
{
    u_result result = U_RESULT_OK;
    v_serviceManager kServiceManager;
    v_serviceStateKind kind;

    if (_this == NULL) {
        kind = STATE_NONE;
    } else {
        result = u_serviceManagerClaim(_this, &kServiceManager);
        if (result == U_RESULT_OK) {
            kind = v_serviceManagerGetServiceStateKind(kServiceManager, serviceName);
            u_serviceManagerRelease(_this);
        } else {
            kind = STATE_NONE;
            OS_REPORT(OS_WARNING, "u_serviceManagerGetServiceStateKind", 0, 
                      "Could not claim serviceManager.");
        }
    }
    return kind;
}

c_iter
u_serviceManagerGetServices(
    u_serviceManager _this,
    v_serviceStateKind kind)
{
    u_result result = U_RESULT_OK;
    v_serviceManager kServiceManager;
    c_iter names;
    c_iter vNames;
    c_string str;
    c_char *n;

    names = c_iterNew(NULL);
    if (_this != NULL) {
        result = u_serviceManagerClaim(_this, &kServiceManager);
        if (result == U_RESULT_OK) {
            vNames = v_serviceManagerGetServices(kServiceManager, kind);
            u_serviceManagerRelease(_this);
            str = (c_string)c_iterTakeFirst(vNames);
            while (str != NULL) {
                n = os_strdup(str);
                names = c_iterInsert(names, (void *)n);
                str = (c_string)c_iterTakeFirst(vNames);
            }
            c_iterFree(vNames);
        } else {
            OS_REPORT(OS_WARNING, "u_serviceManagerGetServices", 0, 
                      "Could not claim serviceManager.");
        }
    }
    return names;
}
