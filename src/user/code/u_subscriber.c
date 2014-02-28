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

#include "u__types.h"
#include "u__entity.h"
#include "u__dataReader.h"
#include "u_networkReader.h"
#include "u__dataView.h"
#include "u_groupQueue.h"
#include "u__subscriber.h"
#include "u__participant.h"
#include "u_user.h"
#include "v_participant.h"
#include "v_subscriber.h"
#include "u__dispatcher.h"
#include "v_group.h"
#include "os_report.h"

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
        result = u_entityWriteClaim(u_entity(p),(v_entity*)(&kp));
        if (result == U_RESULT_OK) {
            assert(kp);
            ks = v_subscriberNew(kp,name,qos,enable);
            if (ks != NULL) {
                _this = u_entityAlloc(p,u_subscriber,ks,TRUE);
                if (_this != NULL) {
                    result = u_subscriberInit(_this,p);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_subscriberNew", 0,
                                    "Initialisation failed. "
                                    "For DataReader: <%s>.", name);
                        (void)u_subscriberFree(_this);
                        _this = NULL;
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
            result = u_entityRelease(u_entity(p));
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
    u_subscriber _this,
    u_participant p)
{
    u_result result;

    if (_this != NULL) {
        result = u_dispatcherInit(u_dispatcher(_this));
        if (result == U_RESULT_OK) {
            _this->readers = NULL;
            _this->participant = p;
            result = u_participantAddSubscriber(p,_this);
        }
    } else {
        OS_REPORT_2(OS_ERROR,
                    "u_subscriberInit",0,
                    "Illegal parameter: _this = 0x%x, participant = 0x%x.",
                    _this,p);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_subscriberFree(
    u_subscriber _this)
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
                result = u_subscriberDeinit(_this);
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
                            "u_subscriberFree",0,
                            "Operation u_subscriberDeinit failed: "
                            "Subscriber = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_subscriberFree",0,
                    "Operation u_entityLock failed: "
                    "Subscriber = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_subscriberDeleteContainedEntities (
    u_subscriber _this)
{
    u_result result;
    u_reader reader;
    c_iter list;

    if (_this != NULL) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            list = _this->readers;
            _this->readers = NULL;
            /* Unlock here because following code will take this lock. */
            u_entityUnlock(u_entity(_this));
            reader = c_iterTakeFirst(list);
            while (reader) {
                switch (u_entityKind(u_entity(reader))) {
                case U_READER:
                    result = u_dataReaderDeleteContainedEntities(u_dataReader(reader));
                    result = u_dataReaderFree(u_dataReader(reader));
                break;
                case U_GROUPQUEUE:
                    result = u_groupQueueFree(u_groupQueue(reader));
                break;
                case U_DATAVIEW:
                    result = u_dataViewFree(u_dataView(reader));
                break;
                case U_NETWORKREADER:
                    result = u_networkReaderFree(u_networkReader(reader));
                break;
                default:
                    OS_REPORT_2(OS_WARNING,
                                "u_subscriberDeleteContainedEntities",0,
                                "invalid object type: "
                                "For Subscriber = 0x%x, found Reader type = %s.",
                                _this, u_kindImage(u_entityKind(u_entity(reader))));
                    assert(0);
                break;
                }
                u_entityDereference(u_entity(_this));
                reader = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            OS_REPORT_2(OS_WARNING,
                        "u_subscriberDeleteContainedEntities",0,
                        "Operation u_entityLock failed: "
                        "Subscriber = 0x%x, result = %s.",
                        _this, u_resultImage(result));
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_subscriberDeleteContainedEntities",0,
                  "Invalid Subscriber <NULL>.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_subscriberDeinit(
    u_subscriber _this)
{
    u_result result;
    u_dataReader reader;
    c_iter list;

    if (_this != NULL) {
        result = u_participantRemoveSubscriber(_this->participant,_this);
        if (result == U_RESULT_OK) {
            _this->participant = NULL;
            if (_this->readers) {
                list = _this->readers;
                _this->readers = NULL;
                u_entityUnlock(u_entity(_this));
                reader = c_iterTakeFirst(list);
                while (reader) {
                    /* Readers should not exist at this point!
                     * This loop corrects this erronous state.
                     */
                    result = u_dataReaderFree(reader);
                    u_entityDereference(u_entity(_this));
                    reader = c_iterTakeFirst(list);
                }
                c_iterFree(list);
                result = u_entityLock(u_entity(_this));
            }
            result = u_dispatcherDeinit(u_dispatcher(_this));
        }
    } else {
        OS_REPORT_1(OS_ERROR,
                    "u_subscriberDeinit",0,
                    "Illegal parameter: _this = 0x%x.",
                    _this);
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

    result= u_entityWriteClaim(u_entity(_this),(v_entity*)(&ks));
    if (result == U_RESULT_OK) {
        assert(ks);
        v_subscriberSubscribe(ks,partitionExpr);
        result = u_entityRelease(u_entity(_this));
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

    result= u_entityReadClaim(u_entity(_this),(v_entity*)(&ks));
    if (result == U_RESULT_OK) {
        assert(ks);
        v_subscriberUnSubscribe(ks,partitionExpr);
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING, "u_subscriberUnSubscribe", 0,
                  "Could not claim subscriber.");
    }
    return result;
}

u_result
u_subscriberAddReader(
    u_subscriber _this,
    u_reader reader)
{
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        _this->readers = c_iterInsert(_this->readers, reader);
        u_entityKeep(u_entity(_this));
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_subscriberAddReader",0,
                  "Failed to lock Subscriber.");
    }
    return result;
}

u_result
u_subscriberRemoveReader(
    u_subscriber _this,
    u_reader reader)
{
    u_reader found;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        found = c_iterTake(_this->readers,reader);
        if (found) {
            u_entityDereference(u_entity(_this));
        }
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_subscriberRemoveReader",0,
                  "Failed to lock Subscriber.");
    }
    return result;
}

c_bool
u_subscriberContainsReader(
    u_subscriber _this,
    u_reader reader)
{
    c_bool found = FALSE;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        found = c_iterContains(_this->readers,reader);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_subscriberContainsReader",0,
                  "Failed to lock Subscriber.");
    }
    return found;
}

c_long
u_subscriberReaderCount(
    u_subscriber _this)
{
    c_long length = -1;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        length = c_iterLength(_this->readers);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_subscriberRemoveReader",0,
                  "Failed to lock Subscriber.");
    }
    return length;
}

struct collect_readers_arg {
    const c_char *topic_name;
    c_iter readers;
};

static void
collect_readers(
    c_voidp object,
    c_voidp arg)
{
    struct collect_readers_arg *a = (struct collect_readers_arg *)arg;
    u_reader r = (u_reader)object;
    c_char *name;

    if (a->topic_name == NULL) {
        a->readers = c_iterInsert(a->readers, r);
    } else {
        name = NULL;
        u_dataReaderTopicName(u_dataReader(r),&name);
        if (name) {
            if (strcmp(name, a->topic_name) == 0)
            {
                /* Expect to have a u_entityKeep(r); at this point as
                 * soon as GAPI redesign is finished.
                */
                a->readers = c_iterInsert(a->readers, r);
            }
            os_free(name);
        }
    }
}

c_iter
u_subscriberLookupReaders(
    u_subscriber _this,
    const c_char *topic_name)
{
    struct collect_readers_arg arg;
    u_result result;

    /* topic_name == NULL is treated as wildcard '*' */
    arg.topic_name = topic_name;
    arg.readers = NULL;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalk(_this->readers, collect_readers, &arg);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_subscriberLookupReaders",0,
                  "Failed to lock Subscriber.");
    }
    return arg.readers;
}

u_result
u_subscriberWalkReaders(
    u_subscriber _this,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalkUntil(_this->readers, (c_iterAction)action, actionArg);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_subscriberWalkReaders",0,
                  "Failed to lock Subscriber.");
    }
    return result;
}

u_dataReader
u_subscriberCreateDataReader (
    u_subscriber _this,
    const c_char *name,
    const c_char *expression,
    c_value params[],
    v_readerQos qos,
    c_bool enable)
{
    u_dataReader reader;
    q_expr expr;

    if (_this) {
        if (expression) {
            expr = q_parse(expression);
            if (!expr) {
                OS_REPORT_1(OS_WARNING,
                            "u_subscriberCreateDataReader",0,
                            "Invalid filter expression: '%s'.",
                            expression);
            }
        } else {
            expr = NULL;
        }
        reader = u_dataReaderNew(_this, name, expr, params, qos, enable);
        if (expr) {
            q_dispose(expr);
        }
    } else {
        reader = NULL;
        OS_REPORT(OS_WARNING,
                  "u_subscriberCreateDataReader",0,
                  "Bad parameter: Subscriber = NULL.");
    }
    return reader;
}

