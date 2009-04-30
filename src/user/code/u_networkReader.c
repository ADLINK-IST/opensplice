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
#include "u_networkReader.h"

#include "u__types.h"
#include "u__entity.h"
#include "u__subscriber.h"
#include "u_user.h"

#include "v_subscriber.h"
#include "v_topic.h"
#include "v_networkReader.h"
#include "v_reader.h"
#include "v_query.h"
#include "v_entity.h"

#include "os_report.h"

u_result
u_networkReaderClaim(
    u_networkReader _this,
    v_networkReader *reader)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (reader != NULL)) {
        *reader = v_networkReader(u_entityClaim(u_entity(_this)));
        if (*reader == NULL) {
            OS_REPORT_2(OS_WARNING, "u_networkReaderClaim", 0,
                        "NetworkReader could not be claimed. "
                        "<_this = 0x%x, reader = 0x%x>.",
                         _this, reader);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_networkReaderClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, reader = 0x%x>.",
                    _this, reader);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_networkReaderRelease(
    u_networkReader _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_networkReaderRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_networkReader
u_networkReaderNew(
    u_subscriber s,
    const c_char *name,
    v_readerQos qos,
    c_bool ignoreReliabilityQoS)
{
    u_participant p;
    u_networkReader _this = NULL;
    v_subscriber ks = NULL;
    v_networkReader kn;
    u_result result;

    if (name == NULL) {
        name = "No name specified";
    }
    if (s != NULL) {
        result = u_subscriberClaim(s,&ks);
        if ((result == U_RESULT_OK) && (ks != NULL)) {
            kn = v_networkReaderNew(ks,name,qos, ignoreReliabilityQoS);
            if (kn != NULL) {
                p = u_entityParticipant(u_entity(s));
                _this = u_entityAlloc(p,u_networkReader,kn,TRUE);
                if (_this != NULL) {
                    result = u_networkReaderInit(_this);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_networkReaderNew", 0,
                                    "Initialisation failed. "
                                    "For NetworkReader: <%s>.", name);
                        u_entityFree(u_entity(_this));
                    }
                } else {
                    OS_REPORT_1(OS_ERROR, "u_networkReaderNew", 0,
                                "Create proxy failed. "
                                "For NetworkReader: <%s>.", name);
                }
                c_free(kn);
            } else {
                OS_REPORT_1(OS_ERROR, "u_networkReaderNew", 0,
                            "Create kernel entity failed. "
                            "For NetworkReader: <%s>.", name);
            }
            result = u_subscriberRelease(s);
        } else {
            OS_REPORT_2(OS_WARNING, "u_networkReaderNew", 0,
                        "Claim Subscriber (0x%x) failed. "
                        "For NetworkReader: <%s>.", s, name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_networkReaderNew",0,
                    "No Subscriber specified. "
                    "For NetworkReader: <%s>", name);
    }
    return _this;
}

u_result
u_networkReaderInit(
    u_networkReader n)
{
    u_result result;

    if (n != NULL) {
        result = u_readerInit(u_reader(n));
        u_entity(n)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_networkReaderInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_networkReaderFree(
    u_networkReader _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_networkReaderDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_networkReaderFree",0,
                  "The specified Network Reader = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_networkReaderDeinit(
    u_networkReader _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_readerDeinit(u_reader(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_networkReaderDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_networkReaderCreateQueue(
    u_networkReader _this,
    c_ulong queueSize,
    c_ulong priority,
    c_bool reliable,
    c_bool P2P,
    c_time resolution,
    c_bool useAsDefault,
    c_ulong *queueId)
{
    v_networkReader kn;
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (queueId != NULL)) {
        result = u_networkReaderClaim(_this,&kn);
        if ((result == U_RESULT_OK) && (kn != NULL)) {
            *queueId = v_networkReaderCreateQueue(kn,queueSize, priority,
                                                  reliable, P2P,
                                                  resolution, useAsDefault);
        } else {
            OS_REPORT(OS_WARNING, "u_networkReaderCreateQueue", 0,
                      "Claim networkReader failed.");
        }
        result = u_networkReaderRelease(_this);
    } else {
        OS_REPORT(OS_ERROR, "u_networkReaderCreateQueue", 0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_networkReaderTrigger(
    u_networkReader _this,
    c_ulong queueId)
{
    v_networkReader kn;
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (queueId != 0)) {
        result = u_networkReaderClaim(_this,&kn);
        if ((result == U_RESULT_OK) && (kn != NULL)) {
            v_networkReaderTrigger(kn,queueId);
        } else {
            OS_REPORT(OS_WARNING, "u_networkReaderTrigger", 0,
                      "Claim networkReader failed.");
        }
        result = u_networkReaderRelease(_this);
    } else {
        OS_REPORT(OS_ERROR, "u_networkReaderTrigger", 0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}


u_result
u_networkReaderRemoteActivityDetected(
    u_networkReader _this)
{
    v_networkReader kn;
    u_result result;

    if (_this != NULL) {
        result = u_networkReaderClaim(_this,&kn);
        if ((result == U_RESULT_OK) && (kn != NULL)) {
            v_networkReaderRemoteActivityDetected(kn);
	} else {
	    OS_REPORT(OS_WARNING, "u_networkReaderRemoteActivityDetected", 0,
		      "Claim networkReader failed.");
	}
	result = u_networkReaderRelease(_this);
    } else {
	OS_REPORT(OS_ERROR, "u_networkReaderRemoteActivityDetected", 0,
		  "Illegal parameter.");
	result = U_RESULT_ILL_PARAM;
    }
    return result;
}


u_result
u_networkReaderRemoteActivityLost(
    u_networkReader _this)
{
    v_networkReader kn;
    u_result result;

    if (_this != NULL) {
        result = u_networkReaderClaim(_this,&kn);
        if ((result == U_RESULT_OK) && (kn != NULL)) {
            v_networkReaderRemoteActivityLost(kn);
	} else {
	    OS_REPORT(OS_WARNING, "u_networkReaderRemoteActivityLost", 0,
		      "Claim networkReader failed.");
	}
	result = u_networkReaderRelease(_this);
    } else {
	OS_REPORT(OS_ERROR, "u_networkReaderRemoteActivityLost", 0,
		  "Illegal parameter.");
	result = U_RESULT_ILL_PARAM;
    }
    return result;
}

