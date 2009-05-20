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

#include "u_domain.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__kernel.h"
#include "u_user.h"

#include "v_kernel.h"
#include "v_domain.h"
#include "v_entity.h"
#include "os_report.h"

u_result
u_domainClaim (
    u_domain _this,
    v_domain *partition)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (partition != NULL)) {
        *partition = v_domain(u_entityClaim(u_entity(_this)));
        if (*partition == NULL) {
            OS_REPORT_2(OS_WARNING,"u_domainClaim",0,
                        "Partition could not be claimed. "
                        "<_this = 0x%x, partition = 0x%x>.",
                        _this, partition);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT(OS_ERROR,"u_domainClaim",0,"Illegal parameter");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_domainRelease(
    u_domain _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_domainRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_domain
u_domainNew(
    u_participant p,
    const c_char *name,
    v_domainQos qos)
{
    u_domain _this = NULL;
    v_kernel ke = NULL;
    v_domain kd;
    u_result result;

    if (name == NULL) {
        name = "No partition specified";
    }
    if (p != NULL) {
        result = u_kernelClaim(u_participantKernel(p),&ke);
        if ((result == U_RESULT_OK) && (ke != NULL)) {
            kd = v_domainNew(ke,name,qos);
            if (kd != NULL) {
                _this = u_entityAlloc(p,u_domain,kd,FALSE);
                if (_this != NULL) {
                    result = u_domainInit(_this);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_domainNew", 0,
                                    "Initialisation failed. "
                                    "For Partition: <%s>.", name);
                        u_entityFree(u_entity(_this));
                    }
                } else {
                    OS_REPORT_1(OS_ERROR, "u_domainNew", 0,
                                "Create proxy failed. "
                                "For Partition: <%s>.", name);
                }
                c_free(kd);
            } else {
                OS_REPORT_1(OS_ERROR, "u_domainNew", 0,
                            "Create kernel entity failed. "
                            "For Partition: <%s>", name);
            }
            result = u_kernelRelease(u_participantKernel(p));
        } else {
            OS_REPORT_1(OS_WARNING, "u_domainNew", 0,
                        "Claim Participant failed. "
                        "For Partition: <%s>", name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_domainNew",0,
                    "No Participant specified. "
                    "For Partition: <%s>", name);
    }
    return _this;
}

u_result
u_domainInit(
    u_domain _this)
{
    u_result result;

    if (_this != NULL) {
        result = U_RESULT_OK;
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_domainInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_domainFree(
    u_domain _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_domainDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_domainFree",0,
                  "The specified Partition = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_domainDeinit (
    u_domain _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_entityDeinit(u_entity(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_domainDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

