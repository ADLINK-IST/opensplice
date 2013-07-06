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

#include "u__publisher.h"
#include "u__writer.h"
#include "u__topic.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__dispatcher.h"
#include "u__participant.h"
#include "u_user.h"

#include "v_participant.h"
#include "v_publisher.h"
#include "v_group.h"

#include "os_report.h"

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
        result = u_entityWriteClaim(u_entity(participant),(v_entity*)(&kp));
        if (result == U_RESULT_OK) {
            assert(kp);
            o = v_publisherNew(kp,name,qos,enable);
            if (o != NULL) {
                _this = u_entityAlloc(participant,u_publisher,o,TRUE);
                if (_this != NULL) {
                    result = u_publisherInit(_this,participant);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_publisherNew", 0,
                                    "Initialisation failed. "
                                    "For Publisher: <%s>.", name);
                        u_publisherFree(_this);
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
            result = u_entityRelease(u_entity(participant));
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
    u_publisher _this,
    u_participant p)
{
    u_result result;

    if (_this && p) {
        result = u_dispatcherInit(u_dispatcher(_this));
        if (result == U_RESULT_OK) {
            _this->writers = NULL;
            _this->participant = p;
            result = u_participantAddPublisher(p,_this);
        }
    } else {
        OS_REPORT_2(OS_ERROR,
                    "u_publisherInit",0,
                    "Illegal parameter: _this = 0x%x, participant = 0x%x.",
                    _this,p);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_publisherFree(
    u_publisher _this)
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
                result = u_publisherDeinit(_this);
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
                            "u_publisherFree",0,
                            "Operation u_publisherDeinit failed: "
                            "Publisher = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_publisherFree",0,
                    "Operation u_entityLock failed: "
                    "Publisher = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_publisherDeleteContainedEntities (
    u_publisher _this)
{
    u_result result;
    u_writer writer;
    c_iter list;

    if (_this != NULL) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            list = _this->writers;
            _this->writers = NULL;
            /* Unlock here because following code will take this lock. */
            u_entityUnlock(u_entity(_this));
            writer = c_iterTakeFirst(list);
            while (writer) {
                result = u_writerFree(writer);
                u_entityDereference(u_entity(_this));
                writer = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            OS_REPORT_2(OS_ERROR,
                        "u_publisherDeleteContainedEntities",0,
                        "Lock Publisher 0x%x failed: result = %s.",
                        _this, u_resultImage(result));
        }
    } else {
        OS_REPORT_1(OS_ERROR,
                    "u_publisherDeleteContainedEntities",0,
                    "Illegal parameter: _this = 0x%x.",
                    _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_publisherDeinit(
    u_publisher _this)
{
    u_result result;
    u_writer writer;
    c_iter list;

    if (_this != NULL) {
        result = u_participantRemovePublisher(_this->participant,_this);
        if (result == U_RESULT_OK) {
            _this->participant = NULL;
            if (_this->writers) {
                list = _this->writers;
                _this->writers = NULL;
                u_entityUnlock(u_entity(_this));
                writer = c_iterTakeFirst(list);
                while (writer) {
                    /* No writer should exist!
                     * This loop is correcting an erronous state.
                     */
                    result = u_writerFree(writer);
                    u_entityDereference(u_entity(_this));
                    writer = c_iterTakeFirst(list);
                }
                c_iterFree(list);
                result = u_entityLock(u_entity(_this));
            }
            result = u_dispatcherDeinit(u_dispatcher(_this));
        }
    } else {
        OS_REPORT_1(OS_ERROR,
                    "u_publisherDeinit",0,
                    "Illegal parameter: _this = 0x%x.",
                    _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_publisherPublish(
    u_publisher _this,
    const c_char *partitionExpr)
{
    v_publisher kp;
    u_result result;

    result = u_entityWriteClaim(u_entity(_this),(v_entity*)(&kp));
    if (result == U_RESULT_OK){
        assert(kp);
        v_publisherPublish(kp,partitionExpr);
        result = u_entityRelease(u_entity(_this));
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR,
                        "u_publisherPublish", 0,
                        "Release Publisher (0x%x) failed.",
                        _this);
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_publisherPublish", 0,
                    "Claim Publisher (0x%x) failed.",
                    _this);
    }
    return result;
}

u_result
u_publisherUnPublish(
    u_publisher _this,
    const c_char *partitionExpr)
{
    v_publisher kp;
    u_result result;

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&kp));
    if (result == U_RESULT_OK){
        assert(kp);
        v_publisherUnPublish(kp,partitionExpr);
        result = u_entityRelease(u_entity(_this));
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

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&kp));
    if (result == U_RESULT_OK){
        assert(kp);
        v_publisherSuspend(kp);
        result = u_entityRelease(u_entity(_this));
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

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&kp));
    if (result == U_RESULT_OK){
        assert(kp);
        resumed = v_publisherResume(kp);
        result = u_entityRelease(u_entity(_this));
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

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&kp));
    if (result == U_RESULT_OK){
        assert(kp);
        v_publisherCoherentBegin(kp);
        result = u_entityRelease(u_entity(_this));
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

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&kp));
    if (result == U_RESULT_OK){
        assert(kp);
        v_publisherCoherentEnd(kp);
        result = u_entityRelease(u_entity(_this));
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

u_result
u_publisherAddWriter(
    u_publisher _this,
    u_writer writer)
{
    u_result result;

    if (writer) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            _this->writers = c_iterInsert(_this->writers, writer);
            u_entityKeep(u_entity(_this));
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT_1(OS_WARNING,
                      "u_publisherAddWriter",0,
                      "Failed to lock Publisher: result = %s.",
                      u_resultImage(result));
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_publisherAddWriter",0,
                    "Given DataWriter (0x%x) is invalid.",
                    writer);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_publisherRemoveWriter(
    u_publisher _this,
    u_writer writer)
{
    u_writer found;
    u_result result;

    if (writer) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterTake(_this->writers,writer);
            if (found) {
                u_entityDereference(u_entity(_this));
            }
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_publisherRemoveWriter",0,
                      "Failed to lock Publisher.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_publisherRemoveWriter",0,
                    "Given DataWriter (0x%x) is invalid.",
                    writer);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_publisherContainsWriter(
    u_publisher _this,
    u_writer writer)
{
    c_bool found = FALSE;
    u_result result;

    if (writer) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterContains(_this->writers,writer);
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_publisherContainsWriter",0,
                      "Failed to lock Publisher.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_publisherContainsWriter",0,
                    "Given DataWriter (0x%x) is invalid.",
                    writer);
    }
    return found;
}

c_long
u_publisherWriterCount(
    u_publisher _this)
{
    c_long length = -1;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        length = c_iterLength(_this->writers);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_1(OS_WARNING,
                  "u_publisherWriterCount",0,
                  "Failed to lock Publisher: result = %s.",
                  u_resultImage(result));
    }
    return length;
}

struct collect_writers_arg {
    const c_char *topic_name;
    c_iter writers;
};

static void
collect_writers(
    c_voidp object,
    c_voidp arg)
{
    struct collect_writers_arg *a = (struct collect_writers_arg *)arg;
    u_writer w = (u_writer)object;
    c_char *name;

    if (a->topic_name == NULL) {
        /* topic_name == NULL is treated as wildcard '*' */
        a->writers = c_iterInsert(a->writers, w);
    } else {
        name = u_writerTopicName(w);
        if (strcmp(name, a->topic_name) == 0)
        {
            a->writers = c_iterInsert(a->writers, w);
        }
        if (name != NULL) {
            os_free(name);
        }
    }
}

c_iter
u_publisherLookupWriters(
    u_publisher _this,
    const c_char *topic_name)
{
    struct collect_writers_arg arg;
    u_result result;

    /* topic_name == NULL is treated as wildcard '*' */
    arg.topic_name = topic_name;
    arg.writers = NULL;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalk(_this->writers, collect_writers, &arg);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_1(OS_WARNING,
                  "u_publisherLookupWriters",0,
                  "Failed to lock Publisher: result = %s.",
                  u_resultImage(result));
    }
    return arg.writers;
}

u_result
u_publisherWalkWriters(
    u_publisher _this,
    u_writerAction action,
    c_voidp actionArg)
{
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalkUntil(_this->writers,
                        (c_iterAction)action,
                        actionArg);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_1(OS_WARNING,
                  "u_publisherWalkWriters",0,
                  "Failed to lock Publisher: result = %s.",
                  u_resultImage(result));
    }
    return result;
}

