
#include "u__publisher.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__dispatcher.h"
#include "u__participant.h"
#include "u_user.h"

#include "v_participant.h"
#include "v_publisher.h"
#include "v_group.h"

#include "os_report.h"

u_result
u_publisherClaim(
    u_publisher _this,
    v_publisher *publisher)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (publisher != NULL)) {
        *publisher = v_publisher(u_entityClaim(u_entity(_this)));
        if (*publisher == NULL) {
            OS_REPORT_2(OS_WARNING, "u_publisherClaim", 0,
                        "Claim Publisher failed. "
                        "<_this = 0x%x, publisher = 0x%x>.",
                         _this, publisher);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_publisherClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, publisher = 0x%x>.",
                    _this, publisher);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_publisherRelease(
    u_publisher _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_publisherRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_publisher
u_publisherNew(
    u_participant participant,
    const c_char *name,
    v_publisherQos qos,
    c_bool enable)
{
    u_publisher _this = NULL;
    v_publisher o;
    v_participant kp = NULL;
    u_result result;

    if (name == NULL) {
        name = "No name specified";
    }
    if (participant!= NULL) {
        result = u_participantClaim(participant,&kp);
        if ((result == U_RESULT_OK) && (kp != NULL)) {
            o = v_publisherNew(kp,name,qos,enable);
            if (o != NULL) {
                _this = u_entityAlloc(participant,u_publisher,o,TRUE);
                if (_this != NULL) {
                    result = u_publisherInit(_this);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_publisherNew", 0,
                                    "Initialisation failed. "
                                    "For Publisher: <%s>.", name);
                        u_entityFree(u_entity(_this));
                    }
                } else {
                    OS_REPORT_1(OS_ERROR, "u_publisherNew", 0,
                                "Create proxy failed. "
                                "For Publisher: <%s>.", name);
                }
                c_free(o);
            } else {
                OS_REPORT_1(OS_ERROR, "u_publisherNew", 0,
                            "Create kernel entity failed. "
                            "For Publisher: <%s>.", name);
            }
            result = u_participantRelease(participant);
        } else {
            OS_REPORT_2(OS_WARNING, "u_publisherNew", 0,
                        "Claim Participant (0x%x) failed. "
                        "For Publisher: <%s>.", participant, name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_publisherNew",0,
                    "No Participant specified. "
                    "For Publisher: <%s>", name);
    }
    return _this;
}

u_result
u_publisherInit(
    u_publisher _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_dispatcherInit(u_dispatcher(_this));
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_publisherInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_publisherFree(
    u_publisher _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_publisherDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_publisherFree",0,
                  "The specified Publisher = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_publisherDeinit(
    u_publisher _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_dispatcherDeinit(u_dispatcher(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_publisherDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_publisherPublish(
    u_publisher _this,
    const c_char *domainExpr)
{
    v_publisher kp;
    u_result result;

    result = u_publisherClaim(_this,&kp);
    if ((result == U_RESULT_OK) && (kp != NULL)) {
        v_publisherPublish(kp,domainExpr);
        result = u_publisherRelease(_this);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "u_publisherPublish", 0,
                        "Release Publisher (0x%x) failed.", _this);
        }
    } else {
        OS_REPORT_1(OS_WARNING, "u_publisherPublish", 0,
                    "Claim Publisher (0x%x) failed.", _this);
    }
    return result;
}

u_result
u_publisherUnPublish(
    u_publisher _this,
    const c_char *domainExpr)
{
    v_publisher kp;
    u_result result;

    result = u_publisherClaim(_this,&kp);
    if ((result == U_RESULT_OK) && (kp != NULL)) {
        v_publisherUnPublish(kp,domainExpr);
        result = u_publisherRelease(_this);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "u_publisherUnPublish", 0,
                        "Release Publisher (0x%x) failed.", _this);
        }
    } else {
        OS_REPORT_1(OS_WARNING, "u_publisherUnPublish", 0,
                    "Claim Publisher (0x%x) failed.", _this);
    }
    return result;
}

u_result
u_publisherSuspend(
    u_publisher _this)
{
    v_publisher kp;
    u_result result;

    result = u_publisherClaim(_this,&kp);
    if ((result == U_RESULT_OK) && (kp != NULL)) {
        v_publisherSuspend(kp);
        result = u_publisherRelease(_this);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "u_publisherSuspend", 0,
                        "Release Publisher (0x%x) failed.", _this);
        }
    } else {
        OS_REPORT_1(OS_WARNING, "u_publisherSuspend", 0,
                    "Claim Publisher (0x%x) failed.", _this);
    }
    return result;
}

u_result
u_publisherResume(
    u_publisher _this)
{
    v_publisher kp;
    u_result result;
    c_bool resumed;

    result = u_publisherClaim(_this,&kp);
    if ((result == U_RESULT_OK) && (kp != NULL)) {
        resumed = v_publisherResume(kp);
        result = u_publisherRelease(_this);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "u_publisherResume", 0,
                        "Release Publisher (0x%x) failed.", _this);
        }
        if (resumed == FALSE) {
            result = U_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT_1(OS_ERROR, "u_publisherResume", 0,
                        "Resume Publisher (0x%x) failed.", _this);
        }
    } else {
        OS_REPORT_1(OS_WARNING, "u_publisherResume", 0,
                    "Claim Publisher (0x%x) failed.", _this);
    }
    return result;
}

u_result
u_publisherCoherentBegin(
    u_publisher _this)
{
    v_publisher kp;
    u_result result;

    result = u_publisherClaim(_this,&kp);
    if ((result == U_RESULT_OK) && (kp != NULL)) {
        v_publisherCoherentBegin(kp);
        result = u_publisherRelease(_this);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "u_publisherCoherentBegin", 0,
                        "Release Publisher (0x%x) failed.", _this);
        }
    } else {
        OS_REPORT_1(OS_WARNING, "u_publisherCoherentBegin", 0,
                    "Claim Publisher (0x%x) failed.", _this);
    }
    return result;
}

u_result
u_publisherCoherentEnd(
    u_publisher _this)
{
    v_publisher kp;
    u_result result;

    result = u_publisherClaim(_this,&kp);
    if ((result == U_RESULT_OK) && (kp != NULL)) {
        v_publisherCoherentEnd(kp);
        result = u_publisherRelease(_this);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "u_publisherCoherentEnd", 0,
                        "Release Publisher (0x%x) failed.", _this);
        }
    } else {
        OS_REPORT_1(OS_WARNING, "u_publisherCoherentEnd", 0,
                    "Claim Publisher (0x%x) failed.", _this);
    }
    return result;
}

