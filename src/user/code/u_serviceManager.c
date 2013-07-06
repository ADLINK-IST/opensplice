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
#include "u__serviceManager.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__dispatcher.h"
#include "u__participant.h"
#include "u__domain.h"
#include "u_user.h"

#include "v_kernel.h"
#include "v_entity.h"
#include "v_service.h"

#include "os_stdlib.h"
#include "os_heap.h"
#include "os_report.h"

u_serviceManager
u_serviceManagerNew(
    u_participant participant)
{
    u_result result;
    u_serviceManager m;
    u_domain domain;
    v_kernel kk;
    v_serviceManager sm;

    m = NULL;
    if (participant != NULL) {
        domain = u_participantDomain(participant);
        result = u_entityWriteClaim(u_entity(domain),(v_entity*)(&kk));
        if (result == U_RESULT_OK) {
            assert(kk);
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
            result = u_entityRelease(u_entity(domain));
        } else {
            OS_REPORT(OS_WARNING,"u_serviceManagerNew",0,
                      "Claim Domain failed.");
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
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        /* if refCount becomes zero then this call
         * returns true and destruction can take place
         */
        if (destroy) {
            if (u_entityOwner(u_entity(_this))) {
                result = u_serviceManagerDeinit(_this);
            } else {
                /* This user entity is a proxy, meaning that it is not fully
                 * initialized, therefore only the entity part of the object
                 * can be deinitialized.
                 * It would be better to either introduce a separate proxy
                 * entity for clarity or fully initialize entities and make
                 * them robust against missing information.
                 */
                result = u_entityDeinit(u_entity(_this));
            }
            if (result == U_RESULT_OK) {
                u_entityDealloc(u_entity(_this));
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_serviceManagerFree",0,
                            "Operation u_serviceManagerDeinit failed: "
                            "ServiceManager = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_serviceManagerFree",0,
                    "Operation u_entityLock failed: "
                    "ServiceManager = 0x%x, result = %s.",
                    _this, u_resultImage(result));
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kServiceManager));
        if (result == U_RESULT_OK) {
            kind = v_serviceManagerGetServiceStateKind(kServiceManager, serviceName);
            u_entityRelease(u_entity(_this));
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kServiceManager));
        if (result == U_RESULT_OK) {
            vNames = v_serviceManagerGetServices(kServiceManager, kind);
            u_entityRelease(u_entity(_this));
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

c_bool
u_serviceManagerRemoveService(
    u_serviceManager _this,
    const c_char *serviceName)
{
    u_result result = U_RESULT_OK;
    v_serviceManager kServiceManager;
    c_bool retVal = FALSE;

    if (_this == NULL) {
        OS_REPORT_1(OS_ERROR, "u_serviceManagerRemoveService", 0,
                  "No valid serviceManager therefore service %s cannot be removed.",serviceName);
    } else {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kServiceManager));
        if (result == U_RESULT_OK) {
            retVal = v_serviceManagerRemoveService(kServiceManager, serviceName);
            u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT(OS_ERROR, "u_serviceManagerRemoveService", 0,
                      "Could not claim serviceManager.");
        }
    }
    return retVal;
}
