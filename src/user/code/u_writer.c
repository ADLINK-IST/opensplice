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

#include "u__writer.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__topic.h"
#include "u__publisher.h"
#include "u__participant.h"
#include "u__dispatcher.h"
#include "u__user.h"

#include "v_publisher.h"
#include "v_topic.h"
#include "v_time.h"
#include "v_writer.h"
#include "v_writerInstance.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_message.h"

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

/**
 * This method invokes the u_entityClaim method to gain access to the writers
 * kernel entity and handles exceptions and protects against termination.
 */
u_result
u_writerClaim(
    u_writer _this,
    v_writer *writer)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (writer != NULL)) {
        *writer = v_writer(u_entityClaim(u_entity(_this)));
        if (*writer == NULL) {
            OS_REPORT_2(OS_WARNING, "u_writerClaim", 0,
                        "Claim Writer failed. "
                        "<_this = 0x%x, writer = 0x%x>.",
                         _this, writer);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_writerClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, writer = 0x%x>.",
                    _this, writer);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

/**
 * This method invokes the u_entityRelease method to release a claim to the writers
 * kernel entity and handles exceptions and restores the protection level.
 */
u_result
u_writerRelease(
    u_writer _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_writerRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

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
        result = u_publisherClaim(p,&kp);
        if ((result == U_RESULT_OK) && (kp != NULL)) {
            result = u_topicClaim(t,&kt);
            if ((result == U_RESULT_OK) && (kt != NULL)) {
                kw = v_writerNew(kp,name,kt,qos,enable);
                if (kw != NULL) {
                    pa = u_entityParticipant(u_entity(p));
                    _this = u_entityAlloc(pa,u_writer,kw,TRUE);
                    if (_this != NULL) {
                        result = u_writerInit(_this);
                        _this->copy = copy;
                        _CREATE_LAPTIMER(name,_this->t1);
                        _CREATE_LAPTIMER(name,_this->t2);
                        _CREATE_LAPTIMER(name,_this->t3);
                        _CREATE_LAPTIMER(name,_this->t4);
                        _CREATE_LAPTIMER(name,_this->t5);
                        _CREATE_LAPTIMER(name,_this->t6);
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
                result = u_topicRelease(t);
            } else {
                OS_REPORT_2(OS_ERROR, "u_writerNew", 0,
                            "Claim Topic (0x%x) failed. "
                            "For DataWriter: <%s>.", t, name);
            }
            result = u_publisherRelease(p);
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

    result = u_writerClaim(_this,&writer);

    if (result == U_RESULT_OK) {
        result = v_writerWaitForAcknowledgments(writer, timeout);
        u_writerRelease(_this);
    }
    return result;
}

u_result
u_writerInit(
    u_writer _this)
{
    u_result result;
    if (_this != NULL) {
        result = u_dispatcherInit(u_dispatcher(_this));
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_writerInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_writerFree(
    u_writer _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            _REPORT_LAPTIMER(_this->t1,"user layer: Claim handle");
            _REPORT_LAPTIMER(_this->t2,"user layer: Message New");
            _REPORT_LAPTIMER(_this->t3,"user layer: Copy data");
            _REPORT_LAPTIMER(_this->t4,"user layer: Kernel write");
            _REPORT_LAPTIMER(_this->t5,"user layer: Translate result");
            _REPORT_LAPTIMER(_this->t6,"user layer: Release handle");
            _DELETE_LAPTIMER(_this->t1);
            _DELETE_LAPTIMER(_this->t2);
            _DELETE_LAPTIMER(_this->t3);
            _DELETE_LAPTIMER(_this->t4);
            _DELETE_LAPTIMER(_this->t5);
            _DELETE_LAPTIMER(_this->t6);
            result = u_writerDeinit(_this);
            os_free(_this);
        } else { /* not initialised so free entity */
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_writerFree",0,
                  "The specified Writer = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_writerDeinit(
    u_writer _this)
{
    u_result result;

    if (_this != NULL) {
        return u_dispatcherDeinit(u_dispatcher(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_writerDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_writerDefaultCopy(
    c_type type,
    void *data,
    void *to)
{
    c_copyIn(type,data,to);

    return TRUE;
}

static u_result
u_resultFromKernelWriteResult (
    v_writeResult vr)
{
    u_result result;

    switch (vr) {
    case V_WRITE_SUCCESS:
    case V_WRITE_INTRANSIT:
    case V_WRITE_REJECTED:
        result = U_RESULT_OK;
    break;
    case V_WRITE_ERROR:
        result = U_RESULT_INTERNAL_ERROR;
    break;
    case V_WRITE_TIMEOUT:
        result = U_RESULT_TIMEOUT;
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
    u_handleWriteAction action)
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
    result = u_writerClaim(_this, &writer);
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
            }
        }

        if (result == U_RESULT_OK) {
            if (data != NULL) {
                _START_LAPTIMER(_this->t2);
                message = v_topicMessageNew(writer->topic);
                _STOP_LAPTIMER(_this->t2);
                to = C_DISPLACE(message, v_topicDataOffset(writer->topic));
                _START_LAPTIMER(_this->t3);
                copyResult = _this->copy(v_topicDataType(writer->topic), data, to);
                V_MESSAGE_STAMP(message,writerAllocTime);
                _STOP_LAPTIMER(_this->t3);

                if(!copyResult){
                    OS_REPORT(OS_ERROR,
                            "u_writeWithHandleAction", 0,
                            "Unable to copy data, because it is invalid.");
                    result = U_RESULT_ILL_PARAM;
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
        u_writerRelease(_this);
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
    if (data != NULL) {
        result = u_writeWithHandleAction(_this,
                                         data,
                                         timestamp,
                                         handle,
                                         v_writerWrite);
    } else {
        result = U_RESULT_ILL_PARAM;
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
    return u_writeWithHandleAction(_this,
                                   data,
                                   timestamp,
                                   handle,
                                   v_writerDispose);
}

/* -------------------------------- u_writerWriteDispose ------------------------- */

u_result
u_writerWriteDispose(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle)
{
    return u_writeWithHandleAction(_this,
                                   data,
                                   timestamp,
                                   handle,
                                   v_writerWriteDispose);
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
    result = u_writerClaim(_this, &writer);
    if ((result == U_RESULT_OK) && (writer != NULL)) {
        v_writerResend(writer);
        result = u_writerRelease(_this);
    }
    *nextPeriod = _this->resendPeriod;
}

u_result
u_writerAssertLiveliness(
    u_writer _this)
{
    u_result result;
    v_writer writer;

    result = u_writerClaim(_this, &writer);
    if ((result == U_RESULT_OK) && (writer != NULL)) {
        v_writerAssertLiveliness(writer);
        result = u_writerRelease(_this);
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

    result = u_writerClaim(_this, &writer);
    if ((result == U_RESULT_OK) && (writer != NULL)) {
        message = v_topicMessageNew(writer->topic);
        to = C_DISPLACE(message, v_topicDataOffset(writer->topic));
        copyResult = _this->copy(v_topicDataType(writer->topic), data, to);

        if(copyResult){
            writeResult = v_writerRegister(writer, message, timestamp, &instance);
            c_free(message);
            wresult = u_resultFromKernelWriteResult(writeResult);
            if (wresult == U_RESULT_OK) {
                *handle = v_publicHandle(v_public(instance));
            }
            c_free(instance);
            result = u_writerRelease(_this);
            if (result == U_RESULT_OK) {
                result = wresult;
            }
        } else {
            result = U_RESULT_ILL_PARAM;

            OS_REPORT(OS_ERROR,
                "u_writerRegisterInstance", 0,
                "Unable to register instance, because the instance data is invalid.");
        }
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

    result = u_writerClaim(_this, &writer);
    if ((result == U_RESULT_OK) && (writer != NULL)) {
        message = v_topicMessageNew(writer->topic);
        to = C_DISPLACE(message, v_topicDataOffset(writer->topic));
        copyResult = copyFunc(v_topicDataType(writer->topic), data, to);

        if(copyResult){
            writeResult = v_writerRegister(writer, message, timestamp, &instance);
            wresult = u_resultFromKernelWriteResult(writeResult);
            if (wresult == U_RESULT_OK) {
                *handle = v_publicHandle(v_public(instance));
            }
            c_free(instance);
            result = u_writerRelease(_this);
            if (result == U_RESULT_OK) {
                result = wresult;
            }
        } else {
            result = U_RESULT_ILL_PARAM;

            OS_REPORT(OS_ERROR,
                "u_writerRegisterInstanceTMP", 0,
                "Unable to register instance, because the instance data is invalid.");
        }
        c_free(message);
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
    return u_writeWithHandleAction(_this,
                                   data,
                                   timestamp,
                                   handle,
                                   v_writerUnregister);
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
    result = u_writerClaim(_this, &writer);

    if ((result == U_RESULT_OK) && (writer != NULL)) {
        message = v_topicMessageNew(writer->topic);
        to = C_DISPLACE(message, v_topicDataOffset(writer->topic));
        copyResult = _this->copy(v_topicDataType(writer->topic), keyTemplate, to);

        if(copyResult){
            instance = v_writerLookupInstance(writer, message);
            c_free(message);

            if (instance) {
                *handle = v_publicHandle(v_public(instance));
                c_free(instance);
            } else {
                *handle = U_HANDLE_NIL;
            }
            result = u_writerRelease(_this);
        } else {
            result = U_RESULT_ILL_PARAM;

            OS_REPORT(OS_ERROR,
                "u_writerLookupInstance", 0,
                "Unable to lookup instance, because the instance data is invalid.");
        }
    }
    return result;
}

/* ---------------------- u_writerInstanceCopyKeys -------------------------- */

u_result
u_writerInstanceCopyKeys (
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
        assert(instance != NULL);
        result = u_writerClaim(_this, &writer);
        if (result == U_RESULT_OK) {
            message = v_writerInstanceCreateMessage(instance);
            if (message) {
                from = C_DISPLACE(message, v_topicDataOffset(writer->topic));
                action(from, copyArg);
                c_free(message);
            } else {
                result = U_RESULT_PRECONDITION_NOT_MET;
            }
            u_writerRelease(_this);
        }
        u_instanceHandleRelease(handle);
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

    result = u_writerClaim(_this, &writer);

    if ((result == U_RESULT_OK) && (writer != NULL)) {
        result = u_resultFromKernel(
                     v_writerGetLivelinessLostStatus(writer,reset,action,arg));
        u_writerRelease(_this);
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

    result = u_writerClaim(_this, &writer);

    if ((result == U_RESULT_OK) && (writer != NULL)) {
        result = u_resultFromKernel(
                     v_writerGetDeadlineMissedStatus(writer,reset,action,arg));
        u_writerRelease(_this);
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

    result = u_writerClaim(_this, &writer);

    if ((result == U_RESULT_OK) && (writer != NULL)) {
        result = u_resultFromKernel(
                     v_writerGetIncompatibleQosStatus(writer,reset,action,arg));
        u_writerRelease(_this);
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

    result = u_writerClaim(_this, &writer);

    if ((result == U_RESULT_OK) && (writer != NULL)) {
        result = u_resultFromKernel(
                     v_writerGetTopicMatchStatus(writer,reset,action,arg));
        u_writerRelease(_this);
    }
    return result;
}

