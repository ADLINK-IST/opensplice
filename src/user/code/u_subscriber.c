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

#include "u__types.h"
#include "u__entity.h"
#include "u__subscriber.h"
#include "u__participant.h"
#include "u_user.h"
#include "v_participant.h"
#include "v_subscriber.h"
#include "u__dispatcher.h"
#include "v_group.h"
#include "os_report.h"

u_result
u_subscriberClaim(
    u_subscriber _this,
    v_subscriber *subscriber)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (subscriber != NULL)) {
        *subscriber = v_subscriber(u_entityClaim(u_entity(_this)));
        if (*subscriber == NULL) {
            OS_REPORT_2(OS_WARNING, "u_subscriberClaim", 0,
                        "Claim Subscriber failed. "
                        "<_this = 0x%x, subscriber = 0x%x>.",
                         _this, subscriber);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_subscriberClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, subscriber = 0x%x>.",
                    _this, subscriber);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_subscriberRelease(
    u_subscriber _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_subscriberRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_subscriber
u_subscriberNew(
    u_participant p,
    const c_char *name,
    v_subscriberQos qos,
    c_bool enable)
{
    u_subscriber _this = NULL;
    v_subscriber ks;
    v_participant kp = NULL;
    u_result result;

    if (name == NULL) {
        name = "No name specified";
    }
    if (p != NULL) {
        result = u_participantClaim(p,&kp);
        if ((result == U_RESULT_OK) && (kp != NULL)) {
            ks = v_subscriberNew(kp,name,qos,enable);
            if (ks != NULL) {
                _this = u_entityAlloc(p,u_subscriber,ks,TRUE);
                if (_this != NULL) {
                    result = u_subscriberInit(_this);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_subscriberNew", 0,
                                    "Initialisation failed. "
                                    "For DataReader: <%s>.", name);
                        u_entityFree(u_entity(_this));
                    }
                } else {
                    OS_REPORT_1(OS_ERROR, "u_subscriberNew", 0,
                                "Create user proxy failed. "
                                "For Subscriber: <%s>.", name);
                }
                c_free(ks);
            } else {
                OS_REPORT_1(OS_ERROR, "u_subscriberNew", 0,
                            "Create kernel entity failed. "
                            "For Subscriber: <%s>.", name);
            }
            result = u_participantRelease(p);
            if (result != U_RESULT_OK) {
                OS_REPORT_1(OS_WARNING, "u_subscriberNew", 0,
                            "Could not release participant."
                            "However subscriber <%s> is created.", name);
            }
        } else {
            OS_REPORT_1(OS_WARNING, "u_subscriberNew", 0,
                        "Claim Participant failed. "
                        "For Subscriber: <%s>.", name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_subscriberNew",0,
                    "No Participant specified. "
                    "For Subscriber: <%s>", name);
    }
    return _this;
}

u_result
u_subscriberInit(
    u_subscriber s)
{
    u_result ur;
    
    if (s != NULL) {
        ur = u_dispatcherInit(u_dispatcher(s));
        u_entity(s)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_subscriberInit",0, "Illegal parameter.");
        ur = U_RESULT_ILL_PARAM;
    }
    return ur;
}

u_result
u_subscriberFree(
    u_subscriber _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_subscriberDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_subscriberFree",0,
                  "The specified Subscriber = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_subscriberDeinit(
    u_subscriber _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_dispatcherDeinit(u_dispatcher(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_subscriberDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_subscriberSubscribe(
    u_subscriber _this,
    const c_char *partitionExpr)
{
    v_subscriber ks = NULL;
    u_result result;

    result = u_subscriberClaim(_this,&ks);
    if ((result == U_RESULT_OK) && (ks != NULL)) {
        v_subscriberSubscribe(ks,partitionExpr);
        result = u_subscriberRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_subscriberSubscribe", 0, 
                  "Could not claim subscriber.");
    }
    return result;
}

u_result
u_subscriberUnSubscribe(
    u_subscriber _this,
    const c_char *partitionExpr)
{
    v_subscriber ks = NULL;
    u_result result;

    result= u_subscriberClaim(_this,&ks);
    if ((result == U_RESULT_OK) && (ks != NULL)) {
        v_subscriberUnSubscribe(ks,partitionExpr);
        result = u_subscriberRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_subscriberUnSubscribe", 0, 
                  "Could not claim subscriber.");
    }
    return result;
}

c_iter
u_subscriberLookupReaders(
    u_subscriber _this,
    const c_char *topicName)
{
    u_result r;
    v_subscriber ks = NULL;
    v_reader kr;
    u_dataReader ur;
    c_iter readers = NULL;
    c_iter kReaders = NULL;

    r = u_subscriberClaim(_this,&ks);
    if ((r == U_RESULT_OK) && (ks != NULL)) {
        kReaders = v_subscriberLookupReadersByTopic(ks, topicName);
        kr = c_iterTakeFirst(kReaders);
        while (kr != NULL) {
            ur = u_entityAlloc(u_entity(_this)->participant,
                               u_dataReader, kr, FALSE);
            u_readerInit(u_reader(ur));
            v_entitySetUserData(v_entity(kr), ur);
            c_free(kr);
            readers = c_iterInsert(readers, ur);
            kr = c_iterTakeFirst(kReaders);
        }
        c_iterFree(kReaders);
        r = u_subscriberRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_subscriberLookupReaders", 0, 
                  "Could not claim subscriber.");
    }
    return readers;
}
