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

#include "u__writer.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__topic.h"
#include "u__publisher.h"
#include "u__participant.h"
#include "u__dispatcher.h"
#include "u__user.h"

#include "v_time.h"
#include "v_writerInstance.h"
#include "v_writer.h"
#include "v_public.h"
#include "v_message.h"
#include "v_spliced.h"
#include "v_participant.h"

#include "os_report.h"

#ifdef PROFILER
#define _CREATE_LAPTIMER(name,t) (t = c_laptimeCreate(name))
#define _DELETE_LAPTIMER(t) c_laptimeDelete(t)
#define _REPORT_LAPTIMER(t,msg) c_laptimeReport(t,msg)
#define _START_LAPTIMER(t) c_laptimeStart(t)
#define _STOP_LAPTIMER(t) c_laptimeStop(t)
#else
#define _CREATE_LAPTIMER(name,t)
#define _DELETE_LAPTIMER(t)
#define _REPORT_LAPTIMER(t,msg)
#define _START_LAPTIMER(t)
#define _STOP_LAPTIMER(t)
#endif

u_writer
u_writerNew(
    u_publisher p,
    const c_char *name,
    u_topic t,
    u_writerCopy copy,
    v_writerQos qos,
    c_bool enable)
{
    u_writer _this = NULL;
    u_participant pa;
    v_publisher kp;
    v_topic kt;
    v_writer kw;
    u_result result;

    if (name == NULL) {
        name = "No name specified";
    }
    if (p != NULL) {
        result = u_entityWriteClaim(u_entity(p),(v_entity*)(&kp));
        if (result == U_RESULT_OK){
            assert(kp);
            result = u_entityWriteClaim(u_entity(t),(v_entity*)(&kt));
            if (result == U_RESULT_OK) {
                assert(kt);
                kw = v_writerNew(kp,name,kt,qos,enable);
                if (kw != NULL) {
                    pa = u_entityParticipant(u_entity(p));
                    _this = u_entityAlloc(pa,u_writer,kw,TRUE);
                    if (_this != NULL) {
                        result = u_writerInit(_this,p,name,copy);
                        if (result != U_RESULT_OK) {
                            OS_REPORT_1(OS_ERROR, "u_writerNew", 0,
                                        "Writer initialization failed. "
                                        "For DataWriter: <%s>.", name);
                            u_writerFree(_this);
                            _this = NULL;
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, "u_writerNew", 0,
                                    "Create user proxy failed. "
                                    "For DataWriter: <%s>.", name);
                    }
                    c_free(kw);
                } else {
                    OS_REPORT_1(OS_ERROR, "u_writerNew", 0,
                                "Create kernel entity failed. "
                                "For DataWriter: <%s>.", name);
                }
                result = u_entityRelease(u_entity(t));
            } else {
                OS_REPORT_2(OS_ERROR, "u_writerNew", 0,
                        "Claim Topic (0x%x) failed. "
                        "For DataWriter: <%s>.", t, name);
            }
            result = u_entityRelease(u_entity(p));
        } else {
            OS_REPORT_2(OS_WARNING, "u_writerNew", 0,
                        "Claim Publisher (0x%x) failed. "
                        "For DataWriter: <%s>.", p, name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_writerNew",0,
                    "No Publisher specified. "
                    "For DataWriter: <%s>", name);
    }
    return _this;
}

u_result
u_writerWaitForAcknowledgments(
    u_writer _this,
    c_time timeout)
{
    v_writer writer;
    u_result result;

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&writer));

    if (result == U_RESULT_OK) {
        result = v_writerWaitForAcknowledgments(writer, timeout);
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_writerInit(
    u_writer _this,
    u_publisher publisher,
    const c_char *name,
    u_writerCopy copy)
{
    u_result result;

    if (_this && publisher) {
        result = u_dispatcherInit(u_dispatcher(_this));
        if (result == U_RESULT_OK) {
            _this->copy = copy;
            _this->publisher = publisher;
            _CREATE_LAPTIMER(name,_this->t1);
            _CREATE_LAPTIMER(name,_this->t2);
            _CREATE_LAPTIMER(name,_this->t3);
            _CREATE_LAPTIMER(name,_this->t4);
            _CREATE_LAPTIMER(name,_this->t5);
            _CREATE_LAPTIMER(name,_this->t6);
            result = u_publisherAddWriter(publisher,_this);
        }
    } else {
        OS_REPORT_2(OS_ERROR,
                   "u_writerInit",0,
                   "Illegal parameter: _this = 0x%x, publisher = 0x%x.",
                   _this, publisher);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_writerFree(
    u_writer _this)
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
                result = u_writerDeinit(_this);
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
                            "u_writerFree",0,
                            "Operation u_writerDeinit failed: "
                            "DataWriter = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_writerFree",0,
                    "Operation u_entityLock failed: "
                    "DataWriter = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_writerDeinit(
    u_writer _this)
{
    u_result result;

    result = u_publisherRemoveWriter(_this->publisher,_this);
    if (result == U_RESULT_OK) {
        _this->publisher = NULL;
        _REPORT_LAPTIMER(found->t1,"user layer: Claim handle");
        _REPORT_LAPTIMER(found->t2,"user layer: Message New");
        _REPORT_LAPTIMER(found->t3,"user layer: Copy data");
        _REPORT_LAPTIMER(found->t4,"user layer: Kernel write");
        _REPORT_LAPTIMER(found->t5,"user layer: Translate result");
        _REPORT_LAPTIMER(found->t6,"user layer: Release handle");
        _DELETE_LAPTIMER(found->t1);
        _DELETE_LAPTIMER(found->t2);
        _DELETE_LAPTIMER(found->t3);
        _DELETE_LAPTIMER(found->t4);
        _DELETE_LAPTIMER(found->t5);
        _DELETE_LAPTIMER(found->t6);
        result = u_dispatcherDeinit(u_dispatcher(_this));
    }
    return result;
}

u_publisher
u_writerPublisher(
    u_writer _this)
{
    return _this->publisher;
}

c_bool
u_writerDefaultCopy(
    c_type type,
    void *data,
    void *to)
{
    c_copyIn(type,data,&to);

    return TRUE;
}

static u_result
u_resultFromKernelWriteResult (
    v_writeResult vr)
{
    u_result result;

    switch (vr) {
    case V_WRITE_SUCCESS:
    case V_WRITE_REJECTED:
        result = U_RESULT_OK;
    break;
    case V_WRITE_ERROR:
        result = U_RESULT_INTERNAL_ERROR;
    break;
    case V_WRITE_TIMEOUT:
        result = U_RESULT_TIMEOUT;
    break;
    case V_WRITE_OUT_OF_RESOURCES:
        result = U_RESULT_OUT_OF_RESOURCES;
    break;
    case V_WRITE_PRE_NOT_MET:
        result = U_RESULT_PRECONDITION_NOT_MET;
    break;
    default:
        result = U_RESULT_UNDEFINED;
    break;
    }
    return result;
}

/* General write-with-handle algorithm used by all writer-functions */

typedef v_writeResult (*u_handleWriteAction)(v_writer writer,
                                             v_message message,
                                             c_time timestamp,
                                             v_writerInstance instance);

static u_result
u_writeWithHandleAction(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle,
    u_handleWriteAction action,
    os_boolean getWriteClaim)
{
    v_writerInstance instance;
    u_result result;
    v_writer writer;
    v_message message;
    c_voidp to;
    c_bool copyResult;
    v_writeResult writeResult;

    assert(action != NULL);

    _START_LAPTIMER(_this->t1);
    if(getWriteClaim)
    {
        result = u_entityWriteClaim(u_entity(_this), (v_entity*)(&writer));
    } else
    {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
    }
    _STOP_LAPTIMER(_this->t1);

    if ((result == U_RESULT_OK) && (writer != NULL)) {
        if (u_instanceHandleIsNil(handle)) {
            instance = NULL;
        } else {
            result = u_instanceHandleClaim(handle, &instance);
            if ((result == U_RESULT_OK) &&
                 (instance != NULL) &&
                 (instance->writer != writer)) {

                result = U_RESULT_PRECONDITION_NOT_MET;
                u_instanceHandleRelease(handle);
            } else if(result == U_RESULT_ALREADY_DELETED){ /* if the instance is already deleted, then the result of an unregister is PRECONDITION_NOT_MET */
                result = U_RESULT_PRECONDITION_NOT_MET;
                assert(instance == NULL);
            }
        }

        if (result == U_RESULT_OK) {
            if (data != NULL) {
                _START_LAPTIMER(_this->t2);
                message = v_topicMessageNew(writer->topic);
                if (message) {
                    _STOP_LAPTIMER(_this->t2);
                    to = C_DISPLACE(message, v_topicDataOffset(writer->topic));
                    _START_LAPTIMER(_this->t3);
                    copyResult = _this->copy(v_topicDataType(writer->topic),
                                             data, to);
                    V_MESSAGE_STAMP(message,writerAllocTime);
                    _STOP_LAPTIMER(_this->t3);

                    if(!copyResult){
                        OS_REPORT(OS_ERROR,
                                "u_writeWithHandleAction", 0,
                                "Unable to copy data, because it is invalid.");
                        result = U_RESULT_ILL_PARAM;
                    }
                } else {
                    c_char *name = v_topicName(writer->topic);
                    if (name == NULL) {
                        name = "No Name";
                    }
                    OS_REPORT_1(OS_ERROR,
                                "u_writeWithHandleAction", 0,
                                "Out of memory: unable to create message for Topic '%s'.",
                                name);
                    result = U_RESULT_OUT_OF_MEMORY;
                }
            } else {
                assert(instance != NULL);
                message = v_writerInstanceCreateMessage(instance);
            }
            if (result == U_RESULT_OK) {
                /* Execute the context-specific action */
                _START_LAPTIMER(_this->t4);
                writeResult = action(writer, message, timestamp,instance);
                _STOP_LAPTIMER(_this->t4);
                _START_LAPTIMER(_this->t5);
                result = u_resultFromKernelWriteResult(writeResult);
                _STOP_LAPTIMER(_this->t5);
            }
            u_instanceHandleRelease(handle);
            c_free(message);
        }
        _START_LAPTIMER(_this->t6);
        u_entityRelease(u_entity(_this));
        _STOP_LAPTIMER(_this->t6);
    }
    return result;
}

/* -------------------------------- u_writerWrite --------------------------- */

u_result
u_writerWrite(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle)
{
    u_result result;

    if (u_entityEnabled(u_entity(_this))) {
        if (data != NULL) {
            result = u_writeWithHandleAction(_this,
                                             data,
                                             timestamp,
                                             handle,
                                             v_writerWrite,
                                             OS_TRUE);
        } else {
            result = U_RESULT_ILL_PARAM;
        }
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

/* -------------------------------- u_writerDispose ------------------------- */

u_result
u_writerDispose(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle)
{
    u_result result;

    if (u_entityEnabled(u_entity(_this))) {
        result = u_writeWithHandleAction(_this,
                                         data,
                                         timestamp,
                                         handle,
                                         v_writerDispose,
                                         OS_TRUE);
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

/* -------------------------------- u_writerWriteDispose ------------------------- */

u_result
u_writerWriteDispose(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle)
{
    u_result result;

    if (u_entityEnabled(u_entity(_this))) {
       result = u_writeWithHandleAction(_this,
                                        data,
                                        timestamp,
                                        handle,
                                        v_writerWriteDispose,
                                        OS_TRUE);
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

void
u_writerResend(
    u_writer _this,
    os_time *nextPeriod)
{
    u_result result;
    v_writer writer;

    assert(nextPeriod != NULL);
    assert((_this->resendPeriod.tv_sec != 0) || (_this->resendPeriod.tv_nsec != 0));

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
    if (result == U_RESULT_OK) {
        assert(writer);
        v_writerResend(writer);
        result = u_entityRelease(u_entity(_this));
    }
    *nextPeriod = _this->resendPeriod;
}

u_result
u_writerAssertLiveliness(
    u_writer _this)
{
    u_result result;
    v_writer writer;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
    if (result == U_RESULT_OK) {
        assert(writer);
        v_writerAssertLiveliness(writer);
        result = u_entityRelease(u_entity(_this));
    }
    return result;
}

/* --------------------------- u_writerRegisterInstance --------------------- */


u_result
u_writerRegisterInstance(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle *handle)
{
    v_writer writer;
    v_message message;
    void *to;
    c_bool copyResult;
    u_result result, wresult;
    v_writeResult writeResult;
    v_writerInstance instance;

    if (u_entityEnabled(u_entity(_this))) {
        result = u_entityWriteClaim(u_entity(_this), (v_entity*)(&writer));
        if (result == U_RESULT_OK) {
            assert(writer);
            message = v_topicMessageNew(writer->topic);
            if (message) {
                to = C_DISPLACE(message, v_topicDataOffset(writer->topic));
                copyResult = _this->copy(v_topicDataType(writer->topic),
                                         data, to);

                if(copyResult){
                    writeResult = v_writerRegister(writer,
                                                   message,
                                                   timestamp,
                                                   &instance);
                    c_free(message);
                    wresult = u_resultFromKernelWriteResult(writeResult);
                    if (wresult == U_RESULT_OK) {
                        *handle = u_instanceHandleNew(v_public(instance));
                    }
                    c_free(instance);
                    result = u_entityRelease(u_entity(_this));
                    if (result == U_RESULT_OK) {
                        result = wresult;
                    }
                } else {
                    result = U_RESULT_ILL_PARAM;

                    OS_REPORT(OS_ERROR,
                        "u_writerRegisterInstance", 0,
                        "Unable to register instance, "
                        "because the instance data is invalid.");
                }
            } else {
                c_char *name = v_topicName(writer->topic);
                if (name == NULL) {
                    name = "No Name";
                }
                OS_REPORT_1(OS_ERROR,
                            "u_writerRegisterInstance", 0,
                            "Out of memory: unable to create message for Topic '%s'.",
                            name);
                result = U_RESULT_OUT_OF_MEMORY;
            }
        }
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

u_result
u_writerRegisterInstanceTMP(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle *handle,
    u_writerCopy copyFunc)
{
    v_writer writer;
    v_message message;
    void *to;
    c_bool copyResult;
    u_result result, wresult;
    v_writeResult writeResult;
    v_writerInstance instance;

    if (u_entityEnabled(u_entity(_this))) {
        result = u_entityWriteClaim(u_entity(_this), (v_entity*)(&writer));
        if (result == U_RESULT_OK) {
            assert(writer);
            message = v_topicMessageNew(writer->topic);
            if (message) {
                to = C_DISPLACE(message, v_topicDataOffset(writer->topic));
                copyResult = copyFunc(v_topicDataType(writer->topic),
                                      data, to);

                if(copyResult){
                    writeResult = v_writerRegister(writer, message,
                                                   timestamp, &instance);
                    wresult = u_resultFromKernelWriteResult(writeResult);
                    if (wresult == U_RESULT_OK) {
                        *handle = u_instanceHandleNew(v_public(instance));
                    }
                    c_free(instance);
                    result = u_entityRelease(u_entity(_this));
                    if (result == U_RESULT_OK) {
                        result = wresult;
                    }
                } else {
                    result = U_RESULT_ILL_PARAM;

                    OS_REPORT(OS_ERROR,
                        "u_writerRegisterInstanceTMP", 0,
                        "Unable to register instance, "
                        "because the instance data is invalid.");
                }
                c_free(message);
            } else {
                c_char *name = v_topicName(writer->topic);
                if (name == NULL) {
                    name = "No Name";
                }
                OS_REPORT_1(OS_ERROR,
                            "u_writerRegisterInstanceTMP", 0,
                            "Out of memory: unable to create message for Topic '%s'.",
                            name);
                result = U_RESULT_OUT_OF_MEMORY;
            }
        }
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

/* ---------------------- u_writerUnregisterInstance ------------------------ */

u_result
u_writerUnregisterInstance(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle)
{
    u_result result;

    if (u_entityEnabled(u_entity(_this))) {
        result = u_writeWithHandleAction(_this,
                                         data,
                                         timestamp,
                                         handle,
                                         v_writerUnregister,
                                         OS_TRUE);
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

u_result
u_writerLookupInstance(
    u_writer _this,
    void* keyTemplate,
    u_instanceHandle *handle)
{
    v_writer writer;
    v_message message;
    void *to;
    c_bool copyResult;
    u_result result;
    v_writerInstance instance;

    if ((_this == NULL) ||
        (keyTemplate == NULL) ||
        (handle == NULL)) {
        return U_RESULT_ILL_PARAM;
    }
    if (u_entityEnabled(u_entity(_this))) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
        if (result == U_RESULT_OK) {
            assert(writer);
            message = v_topicMessageNew(writer->topic);
            if (message) {
                to = C_DISPLACE(message, v_topicDataOffset(writer->topic));
                copyResult = _this->copy(v_topicDataType(writer->topic), keyTemplate, to);

                if(copyResult){
                    instance = v_writerLookupInstance(writer, message);
                    *handle = u_instanceHandleNew(v_public(instance));
                    c_free(message);
                    c_free(instance);
                    result = u_entityRelease(u_entity(_this));
                } else {
                    result = U_RESULT_ILL_PARAM;

                    OS_REPORT(OS_ERROR,
                        "u_writerLookupInstance", 0,
                        "Unable to lookup instance, "
                        "because the instance data is invalid.");
                }
            } else {
                c_char *name = v_topicName(writer->topic);
                if (name == NULL) {
                    name = "No Name";
                }
                OS_REPORT_1(OS_ERROR,
                            "u_writerLookupInstance", 0,
                            "Out of memory: unable to create message for Topic '%s'.",
                            name);
                result = U_RESULT_OUT_OF_MEMORY;
            }
        }
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

/* ---------------------- u_writerCopyKeysFromInstanceHandle -------------------------- */

u_result
u_writerCopyKeysFromInstanceHandle (
    u_writer _this,
    u_instanceHandle handle,
    u_writerAction action,
    void *copyArg)
{
    v_writerInstance instance;
    u_result result;
    v_writer writer;
    v_message message;
    void *from;

    result = u_instanceHandleClaim(handle, &instance);

    if ((result == U_RESULT_OK) && (instance != NULL)) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
        if (result == U_RESULT_OK) {
            if (v_writerContainsInstance(writer, instance)) {
                message = v_writerInstanceCreateMessage(instance);
                if (message) {
                    from = C_DISPLACE(message, v_topicDataOffset(writer->topic));
                    action(from, copyArg);
                    c_free(message);
                } else {
                    OS_REPORT_1(OS_WARNING, "u_dataWriterCopyKeysFromInstanceHandle", 0,
                        "Failed to create keytemplate message"
                        "<dataWriterInstance = 0x%x>", instance);
                    result = U_RESULT_ILL_PARAM;
                }
            } else {
                OS_REPORT_2(OS_WARNING, "u_dataWriterCopyKeysFromInstanceHandle", 0,
                    "Instance handle does not belong to writer"
                    "<_this = 0x%s handle = %lld>", _this, handle);
                result = U_RESULT_ILL_PARAM;
            }
            u_entityRelease(u_entity(_this));
        }
        u_instanceHandleRelease(handle);
    } else if (result == U_RESULT_ALREADY_DELETED) {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }

    return result;
}

u_result
u_writerGetLivelinessLostStatus (
    u_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_writer writer;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
    if (result == U_RESULT_OK) {
        assert(writer);
        result = u_resultFromKernel(
                     v_writerGetLivelinessLostStatus(writer,reset,action,arg));
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_writerGetDeadlineMissedStatus (
    u_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_writer writer;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));

    if (result == U_RESULT_OK) {
        assert(writer);
        result = u_resultFromKernel(
                     v_writerGetDeadlineMissedStatus(writer,reset,action,arg));
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_writerGetIncompatibleQosStatus (
    u_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_writer writer;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
    if (result == U_RESULT_OK) {
        assert(writer);
        result = u_resultFromKernel(
                     v_writerGetIncompatibleQosStatus(writer,reset,action,arg));
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_writerGetPublicationMatchStatus (
    u_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_writer writer;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
    if (result == U_RESULT_OK) {
        assert(writer);
        result = u_resultFromKernel(
                     v_writerGetTopicMatchStatus(writer,reset,action,arg));
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_writerGetMatchedSubscriptions (
    u_writer _this,
    v_statusAction action,
    c_voidp arg)
{
    v_writer writer;
    v_spliced spliced;
    v_kernel kernel;
    u_result result;
    c_iter participants;
    v_participant participant;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));

    if (result == U_RESULT_OK) {
        assert(writer);
        kernel = v_objectKernel(writer);

        participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
        assert(c_iterLength(participants) == 1);
        participant = v_participant(c_iterTakeFirst(participants));
        spliced = v_spliced(participant);
        c_free(participant);
        c_iterFree(participants);

        result = u_resultFromKernel(
                     v_splicedGetMatchedSubscriptions(spliced, writer, action, arg));
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_writerGetMatchedSubscriptionData (
    u_writer _this,
    u_instanceHandle subscription_handle,
    v_statusAction action,
    c_voidp arg)
{
    v_writer writer;
    v_spliced spliced;
    v_kernel kernel;
    u_result result;
    c_iter participants;
    v_participant participant;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));

    if (result == U_RESULT_OK) {
        assert(writer);
        kernel = v_objectKernel(writer);

        participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
        assert(c_iterLength(participants) == 1);
        participant = v_participant(c_iterTakeFirst(participants));
        spliced = v_spliced(participant);
        c_free(participant);
        c_iterFree(participants);

        result = u_resultFromKernel(
                     v_splicedGetMatchedSubscriptionData(spliced, writer, u_instanceHandleToGID(subscription_handle), action, arg));
        u_entityRelease(u_entity(_this));
    }
    return result;

}

c_char *
u_writerTopicName (
    u_writer _this)
{
    v_writer writer;
    u_result result;
    c_char *name = NULL;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&writer));
    if (result == U_RESULT_OK) {
        assert(writer);
        name = os_strdup(v_topicName(v_writerTopic(writer)));
        u_entityRelease(u_entity(_this));
    }
    return name;
}

