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
        result = u_entityWriteClaim(u_entity(s), (v_entity*)(&ks));
        if(result == U_RESULT_OK)
        {
            assert(ks);
            kn = v_networkReaderNew(ks,name,qos, ignoreReliabilityQoS);
            if (kn != NULL) {
                p = u_entityParticipant(u_entity(s));
                _this = u_entityAlloc(p,u_networkReader,kn,TRUE);
                if (_this != NULL) {
                    result = u_networkReaderInit(_this,s);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_networkReaderNew", 0,
                                    "Initialisation failed. "
                                    "For NetworkReader: <%s>.", name);
                        u_networkReaderDeinit(_this);
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
            result = u_entityRelease(u_entity(s));
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
    u_networkReader _this,
    u_subscriber s)
{
    u_result result;

    if (_this != NULL) {
        result = u_readerInit(u_reader(_this));
        if (result == U_RESULT_OK) {
            _this->subscriber = s;
            result = u_subscriberAddReader(s,u_reader(_this));
        }
        if (result == U_RESULT_OK) {
            u_entity(_this)->flags |= U_ECREATE_INITIALISED;
        }
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
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        /* if refCount becomes zero then this call
         * returns true and destruction can take place
         */
        if (destroy) {
            if (u_entityOwner(u_entity(_this))) {
                result = u_networkReaderDeinit(_this);
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
                            "u_networkReaderFree",0,
                            "Operation u_networkReaderDeinit failed: "
                            "NetworkReader = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_networkReaderFree",0,
                    "Operation u_entityLock failed: "
                    "NetworkReader = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_networkReaderDeinit(
    u_networkReader _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_subscriberRemoveReader(_this->subscriber,
                                          u_reader(_this));
        if (result == U_RESULT_OK) {
            _this->subscriber = NULL;
            result = u_readerDeinit(u_reader(_this));
        }
    } else {
        OS_REPORT_1(OS_ERROR,
                    "u_networkReaderDeinit",0,
                    "Illegal parameter: _this = 0x%x.",
                     _this);
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
    c_ulong *queueId,
    const c_char *name)
{
    v_networkReader kn;
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (queueId != NULL)) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kn));
        if(result == U_RESULT_OK)
        {
            assert(kn);
            *queueId = v_networkReaderCreateQueue(kn,queueSize, priority,
                                                  reliable, P2P,
                                                  resolution, useAsDefault, name);
            result = u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING, "u_networkReaderCreateQueue", 0,
                      "Claim networkReader failed.");
        }
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kn));
        if(result == U_RESULT_OK)
        {
            assert(kn);
            v_networkReaderTrigger(kn,queueId);
            result = u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING, "u_networkReaderTrigger", 0,
                      "Claim networkReader failed.");
         }
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kn));
        if(result == U_RESULT_OK)
        {
            assert(kn);
            v_networkReaderRemoteActivityDetected(kn);
	        result = u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING, "u_networkReaderRemoteActivityDetected", 0,
                  "Claim networkReader failed.");
        }
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kn));
        if(result == U_RESULT_OK)
        {
            assert(kn);
            v_networkReaderRemoteActivityLost(kn);
	        result = u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING, "u_networkReaderRemoteActivityLost", 0,
                  "Claim networkReader failed.");
        }
    } else {
	    OS_REPORT(OS_ERROR, "u_networkReaderRemoteActivityLost", 0,
		  "Illegal parameter.");
	    result = U_RESULT_ILL_PARAM;
    }
    return result;
}

