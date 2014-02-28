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

#include "u__dataReader.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__dataView.h"
#include "u__subscriber.h"
#include "u__user.h"

#include "v_subscriber.h"
#include "v_topic.h"
#include "v_dataReader.h"
#include "v_dataReaderInstance.h"
#include "v_reader.h"
#include "v_query.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_statistics.h"
#include "c_iterator.h"

#include "os_report.h"

#define U_DATAREADER_SIZE sizeof(C_STRUCT(u_dataReader))

u_dataReader
u_dataReaderNew(
    u_subscriber s,
    const c_char *name,
    q_expr OQLexpr,
    c_value params[],
    v_readerQos qos,
    c_bool enable)
{
    u_participant p;
    u_dataReader _this = NULL;
    v_subscriber ks = NULL;
    v_dataReader reader;
    u_result result;

    if (name == NULL) {
        name = "No name specified";
    }
    if (s != NULL) {
        result = u_entityWriteClaim(u_entity(s), (v_entity*)(&ks));
        if (result == U_RESULT_OK) {
            assert(ks);
            reader = v_dataReaderNew(ks,name, OQLexpr,params,qos,enable);
            if (reader != NULL) {
                p = u_entityParticipant(u_entity(s));
                _this = (u_dataReader)u_entityNew(v_entity(reader),p,TRUE);

                if (_this != NULL) {
                    result = u_dataReaderInit(_this,s);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_dataReaderNew", 0,
                                    "Initialisation failed. "
                                    "For DataReader: <%s>.", name);
                        u_dataReaderFree(_this);
                    }
                } else {
                    OS_REPORT_1(OS_ERROR, "u_dataReaderNew", 0,
                                "Create user proxy failed. "
                                "For DataReader: <%s>.", name);
                }
                c_free(reader);
            } else {
                OS_REPORT_1(OS_ERROR, "u_dataReaderNew", 0,
                            "Create kernel entity failed. "
                            "For DataReader: <%s>.", name);
            }
            result = u_entityRelease(u_entity(s));
        } else {
            OS_REPORT_2(OS_WARNING, "u_dataReaderNew", 0,
                        "Claim Subscriber (0x%x) failed. "
                        "For DataReader: <%s>.", s, name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_dataReaderNew",0,
                    "No Subscriber specified. "
                    "For DataReader: <%s>", name);
    }
    return _this;
}

u_result
u_dataReaderInit(
    u_dataReader _this,
    u_subscriber s)
{
    u_result result;

    if (_this && s) {
        result = u_readerInit(u_reader(_this));
        if (result == U_RESULT_OK) {
            _this->subscriber = s;
            _this->views = NULL;
            result = u_subscriberAddReader(s,u_reader(_this));
        }
    } else {
        OS_REPORT_2(OS_ERROR,
                    "u_dataReaderInit",0,
                    "Illegal parameter: _this = 0x%x, subscriber = 0x%x.",
                    _this,s);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dataReaderFree(
    u_dataReader _this)
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
                result = u_dataReaderDeinit(_this);
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
                /* free memory */
                u_entityDealloc(u_entity(_this));
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_dataReaderFree",0,
                            "Operation u_dataReaderDeinit failed: "
                            "DataReader = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_dataReaderFree",0,
                    "Operation u_entityLock failed: "
                    "DataReader = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_dataReaderDeleteContainedEntities (
    u_dataReader _this)
{
    u_result result;
    u_dataView view;
    c_iter list;

    if (_this != NULL) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            list = _this->views;
            _this->views = NULL;
            /* Unlock here because following code will take this lock. */
            u_entityUnlock(u_entity(_this));
            view = c_iterTakeFirst(list);
            while (view) {
                result = u_dataViewFree(view);
                u_entityDereference(u_entity(_this));
                view = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            OS_REPORT_3(OS_WARNING,
                        "u_dataReaderDeleteContainedEntities",0,
                        "Operation u_entityLock DataReader failed: "
                        "Participant = 0x%x, DataReader = 0x%x, result = %s.",
                        u_entityParticipant(u_entity(_this)),
                        _this, u_resultImage(result));
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_dataReaderDeleteContainedEntities",0,
                  "Operations failed on invalid DataReader."
                  "DataReader = NULL");
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

u_subscriber
u_dataReaderSubscriber(
    u_dataReader _this)
{
    return _this->subscriber;
}

u_result
u_dataReaderDeinit(
    u_dataReader _this)
{
    u_result result;
    u_dataView view;
    c_iter list;

    if (_this != NULL) {
        result = u_subscriberRemoveReader(_this->subscriber,u_reader(_this));
        if (result == U_RESULT_OK) {
            _this->subscriber = NULL;
            if (_this->views) {
                list = _this->views;
                _this->views = NULL;
                u_entityUnlock(u_entity(_this));
                view = c_iterTakeFirst(list);
                while (view) {
                    result = u_dataViewFree(view);
                    view = c_iterTakeFirst(list);
                }
                c_iterFree(list);
                result = u_entityLock(u_entity(_this));
            }
            if (result == U_RESULT_OK) {
                result = u_readerDeinit(u_reader(_this));
            }
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "u_dataReaderDeinit",0,
                  "Operation failed on invalid DataReader: "
                  "Datareader = NULL");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_dataReaderDefaultCopy(
    v_collection c,
    c_object o,
    c_voidp actionArg)
{
    v_dataReader dataReader;
    v_dataReaderSample sample;
    v_topic topic;
    v_message message;
    c_object userData;
    c_type type;

    switch (v_objectKind(c)) {
    case K_DATAREADER:
        type = v_dataReaderInstanceType(v_dataReader(c));
        topic = v_dataReaderGetTopic(v_dataReader(c));
    break;
    case K_QUERY:
        dataReader = v_dataReader(v_querySource(v_query(c)));
        type = v_dataReaderInstanceType(dataReader);
        topic = v_dataReaderGetTopic(dataReader);
        c_free(dataReader);
    break;
    default:
        OS_REPORT_1(OS_WARNING,"u_dataReaderDefaultCopy",0,
                    "Unsuitable collection kind (%d)",
                    v_objectKind(c));
        type = NULL;
    break;
    }

    if (type != NULL) {

        sample = (v_dataReaderSample)o;
        message = v_dataReaderSampleMessage(o);
        userData = (c_voidp)C_DISPLACE(message,v_topicDataOffset(topic));

        c_copyOut(type,userData,&actionArg);

        c_free(topic);
        c_free(type);
    }
    return FALSE;
}

c_bool
u_dataReaderDataAvailableTest(
    u_dataReader _this)
{
    u_result result;
    v_dataReader reader;
    c_bool avail = FALSE;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
    if (result == U_RESULT_OK) {
        avail = (v_dataReaderNotReadCount(reader) > 0);
        u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_2(OS_ERROR,
                    "u_dataReaderDataAvailableTest", 0,
                    "Claim of DataReader failed: "
                    "DataReader = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return avail;
}

u_result
u_dataReaderWalkInstances (
    u_dataReader _this,
    u_dataReaderInstanceAction action,
    c_voidp arg)
{
    v_dataReader reader;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

        if (result == U_RESULT_OK) {
            v_dataReaderWalkInstances(reader,action,arg);
            u_entityRelease(u_entity(_this));
        }
   return result;
}


C_STRUCT(readActionArg) {
    u_dataReaderAction action;
    c_voidp arg;
    v_actionResult result;
};

C_CLASS(readActionArg);

static v_actionResult
readAction(
    c_object sample,
    c_voidp arg)
{
    readActionArg a = (readActionArg)arg;

    if (a && a->action) {
        if (sample == NULL) {
            a->action(sample,a->arg);
        } else {
            a->result = a->action(sample, a->arg);
        }
    }
    return a->result;
}

u_result
u_dataReaderRead(
    u_dataReader _this,
    u_dataReaderAction action,
    c_voidp actionArg)
{
    u_result result;
    v_dataReader reader;
    C_STRUCT(readActionArg) arg;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.result = 0;
        v_actionResultSet(arg.result, V_PROCEED);
        v_dataReaderRead(reader, readAction, &arg);
        u_entityRelease(u_entity(_this));
    }
    return result;
}

/* Get instance user data */
u_result
u_dataReaderGetInstanceUserData (
        u_dataReader _this,
        u_instanceHandle handle,
        c_voidp* userData_out)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    if (!userData_out) {
        result = U_RESULT_ILL_PARAM;
    }else
    {
        *userData_out = NULL;

        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK) {
            handle = u_instanceHandleFix(handle,v_collection(reader));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_OK) {
                if (v_dataReaderContainsInstance(reader,instance)) {
                    *userData_out =
                            v_dataReaderInstanceGetUserData (instance);
                } else {
                    result = U_RESULT_ILL_PARAM;
                }
                u_instanceHandleRelease(handle);
            }
            u_entityRelease(u_entity(_this));
        }
    }

    return result;
}

/* Set instance user data */
u_result
u_dataReaderSetInstanceUserData (
        u_dataReader _this,
        u_instanceHandle handle,
        c_voidp userData)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_collection(reader));
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            if (v_dataReaderContainsInstance(reader,instance)) {
                v_dataReaderInstanceSetUserData (instance, userData);
            } else {
                result = U_RESULT_ILL_PARAM;
            }
            u_instanceHandleRelease(handle);
        }
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_dataReaderTake(
    u_dataReader _this,
    u_dataReaderAction action,
    c_voidp actionArg)
{
    u_result result;
    v_dataReader reader;
    C_STRUCT(readActionArg) arg;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.result = 0;
        v_actionResultSet(arg.result, V_PROCEED);
        v_dataReaderTake(reader, readAction, &arg);
        u_entityRelease(u_entity(_this));
    }
    return result;
}

C_STRUCT(readListActionArg) {
    c_iter iter;
    c_ulong spaceLeft;
    u_readerCopyList copyAction;
    c_voidp copyArg;
    c_voidp result;
};

C_CLASS(readListActionArg);

static v_actionResult
readListAction(
    c_object sample,
    c_voidp arg)
{
    v_actionResult result = 0;
    readListActionArg a = (readListActionArg)arg;

    if (a->spaceLeft == 0U) {
        return result;
    }
    if (sample == NULL) {
        a->result = a->copyAction(NULL, a->iter, a->copyArg);
        return result;
    }
    a->iter = c_iterInsert(a->iter, c_keep(sample));
    a->spaceLeft--;
    if (a->spaceLeft == 0U) {
        a->result = a->copyAction(NULL, a->iter, a->copyArg);
        return result;
    } else {
        v_actionResultSet(result, V_PROCEED);
        return result;
    }
}

void *
u_dataReaderReadList(
    u_dataReader _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg)
{
    v_dataReader reader;
    c_iter list;
    u_result result;
    C_STRUCT(readListActionArg) arg;
    c_object object;

    if (copy == NULL) {
        return NULL;
    }
    arg.copyAction = copy;
    arg.copyArg = copyArg;
    arg.iter = NULL;
    arg.result = NULL;
    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
    if (result == U_RESULT_OK) {
        if (max == 0U) {
            arg.spaceLeft = 0x7fffffffU;
        } else {
            arg.spaceLeft = max;
        }
        v_dataReaderRead(reader, readListAction, &arg);
        list = arg.iter;
        object = c_iterTakeFirst(list);
        while (object != NULL) {
            c_free(object);
            object = c_iterTakeFirst(list);
        }
        c_iterFree(list);
        u_entityRelease(u_entity(_this));
    }
    return arg.result;
}

void *
u_dataReaderTakeList(
    u_dataReader _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg)
{
    v_dataReader reader;
    c_iter list;
    u_result result;
    C_STRUCT(readListActionArg) arg;
    c_object object;

    if (copy == NULL) {
        return NULL;
    }
    arg.copyAction = copy;
    arg.copyArg = copyArg;
    arg.iter = NULL;
    arg.result = NULL;
    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
    if (result == U_RESULT_OK) {
        if (max == 0U) {
            arg.spaceLeft = 0x7fffffffU;
        } else {
            arg.spaceLeft = max;
        }
        v_dataReaderTake(reader, readListAction, &arg);
        list = arg.iter;
        object = c_iterTakeFirst(list);
        while (object != NULL) {
            c_free(object);
            object = c_iterTakeFirst(list);
        }
        c_iterFree(list);
        u_entityRelease(u_entity(_this));
    }
    return arg.result;
}

/***************************** read/take_(next)_instance **********************/

u_result
u_dataReaderReadInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_dataReaderAction action,
    c_voidp actionArg)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_collection(reader));
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            if (v_dataReaderContainsInstance(reader,instance)) {
                v_dataReaderReadInstance(reader,
                                         instance,
                                         (v_readerSampleAction)action,
                                         actionArg);
            } else {
                result = U_RESULT_ILL_PARAM;
            }
            u_instanceHandleRelease(handle);
        }
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_dataReaderTakeInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_dataReaderAction action,
    c_voidp actionArg)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_collection(reader));
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            if (v_dataReaderContainsInstance(reader,instance)) {
                v_dataReaderTakeInstance(reader,
                                         instance,
                                         (v_readerSampleAction)action,
                                         actionArg);
            } else {
                result = U_RESULT_ILL_PARAM;
            }
            u_instanceHandleRelease(handle);
        }
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_dataReaderReadNextInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_dataReaderAction action,
    c_voidp actionArg)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
    if (result == U_RESULT_OK) {
        if (u_instanceHandleIsNil(handle)) {
            v_dataReaderReadNextInstance(reader,
                                         NULL,
                                         (v_readerSampleAction)action,
                                         actionArg);
        } else {
            handle = u_instanceHandleFix(handle,v_collection(reader));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_ALREADY_DELETED) {
#if 0
                /* The handle has become invalid and no instance including
                 * the key value can be found. Therefore set the instance
                 * to null and start reading from scratch.
                 * Doing an automatic correction of already deleted handles
                 * hides information for the user but can be a convenience
                 * for iterating over all instances.
                 * Conceptual this should be left out.
                 */
                v_dataReaderReadNextInstance(reader,
                                             NULL,
                                             (v_readerSampleAction)action,
                                             actionArg);
                result = U_RESULT_OK;
#endif
            } else if (result == U_RESULT_OK) {
                assert(instance != NULL);
                if (v_dataReaderContainsInstance(reader,instance)) {
                    v_dataReaderReadNextInstance(reader,
                                                 instance,
                                                 (v_readerSampleAction)action,
                                                 actionArg);
                } else {
                    result = U_RESULT_ILL_PARAM;
                }
                u_instanceHandleRelease(handle);
            }
        }
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_dataReaderTakeNextInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_dataReaderAction action,
    c_voidp actionArg)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
    if (result == U_RESULT_OK) {
        if (u_instanceHandleIsNil(handle)) {
            v_dataReaderTakeNextInstance(reader,
                                         NULL,
                                         (v_readerSampleAction)action,
                                         actionArg);
        } else {
            handle = u_instanceHandleFix(handle,v_collection(reader));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_ALREADY_DELETED) {
#if 0
                /* The handle has become invalid and no instance including
                 * the key value can be found. Therefore set the instance
                 * to null and start reading from scratch.
                 * Doing an automatic correction of already deleted handles
                 * hides information for the user but can be a convenience
                 * for iterating over all instances.
                 * Conceptual this should be left out.
                 */
                v_dataReaderTakeNextInstance(reader,
                                             NULL,
                                             (v_readerSampleAction)action,
                                             actionArg);
                result = U_RESULT_OK;
#endif
            } else if (result == U_RESULT_OK) {
                assert(instance != NULL);
                if (v_dataReaderContainsInstance(reader,instance)) {
                    v_dataReaderTakeNextInstance(reader,
                                                 instance,
                                                 (v_readerSampleAction)action,
                                                 actionArg);
                } else {
                    result = U_RESULT_ILL_PARAM;
                }
                u_instanceHandleRelease(handle);
            }
        }
        u_entityRelease(u_entity(_this));
    }
    return result;
}


u_result
u_dataReaderWaitForHistoricalData(
    u_dataReader _this,
    c_time timeout)
{
    v_dataReader reader;
    u_result     result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

    if (result == U_RESULT_OK) {
        if(v_readerWaitForHistoricalData(v_reader(reader), timeout) == TRUE) {
            result = U_RESULT_OK;
        } else {
            result = U_RESULT_TIMEOUT;
        }
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_dataReaderWaitForHistoricalDataWithCondition(
    u_dataReader _this,
    c_char* filter,
    c_char* params[],
    c_ulong paramsLength,
    c_time minSourceTime,
    c_time maxSourceTime,
    struct v_resourcePolicy* resourceLimits,
    c_time timeout)
{
    v_dataReader reader;
    u_result     result;
    v_historyResult hresult;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

    if (result == U_RESULT_OK) {
        hresult = v_readerWaitForHistoricalDataWithCondition(
            v_reader(reader), filter, params, paramsLength,
            minSourceTime, maxSourceTime, resourceLimits, timeout);

        switch(hresult){
        case V_HISTORY_RESULT_OK:
            result = U_RESULT_OK;
            break;
        case V_HISTORY_RESULT_UNDEFINED:
            result = U_RESULT_UNDEFINED;
            break;
        case V_HISTORY_RESULT_ERROR:
            result = U_RESULT_INTERNAL_ERROR;
            break;
        case V_HISTORY_RESULT_BAD_PARAM:
            result = U_RESULT_ILL_PARAM;
            break;
        case V_HISTORY_RESULT_TIMEOUT:
            result = U_RESULT_TIMEOUT;
            break;
        case V_HISTORY_RESULT_PRE_NOT_MET:
            result = U_RESULT_PRECONDITION_NOT_MET;
            break;
        }
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_dataReaderLookupInstance(
    u_dataReader _this,
    c_voidp keyTemplate,
    u_copyIn copyIn,
    u_instanceHandle *handle)
{
    v_dataReader         reader;
    u_result             result;
    v_message            message;
    v_topic              topic;
    c_voidp              to;
    v_dataReaderInstance instance;

    if ((_this == NULL) ||
        (keyTemplate == NULL) ||
        (copyIn == NULL) ||
        (handle == NULL)) {
        return U_RESULT_ILL_PARAM;
    }

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

    if (result == U_RESULT_OK) {
        topic = v_dataReaderGetTopic(reader);
        message = v_topicMessageNew(topic);
        if (message) {
            to = C_DISPLACE(message, v_topicDataOffset(topic));
            copyIn(v_topicDataType(topic), keyTemplate, to);
            instance = v_dataReaderLookupInstance(reader, message);
            *handle = u_instanceHandleNew(v_public(instance));
            c_free(instance);
            c_free(message);
        } else {
            c_char *name = v_topicName(topic);
            if (name == NULL) {
                name = "No Name";
            }
            OS_REPORT_2(OS_ERROR,
                        "u_dataReaderLookupInstance", 0,
                        "Out of memory: unable to create message for Topic: "
                        "Participant = 0x%x, Topic = '%s'.",
                        u_entityParticipant(u_entity(_this)),
                        name);
            result = U_RESULT_OUT_OF_MEMORY;
        }
        c_free(topic);
        u_entityRelease(u_entity(_this));
    }
    return result;
}

u_result
u_dataReaderTopicName(
    u_dataReader _this,
    c_char **name)
{
    u_result result;
    v_dataReader reader;
    v_topic topic;
    c_char*n;

    *name = NULL;
    if ((_this) && (name)) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

        if (result == U_RESULT_OK) {
            topic = v_dataReaderGetTopic(reader);
            n = v_topicName(topic);
            *name = os_strdup(n);
            c_free(topic);
            u_entityRelease(u_entity(_this));
        }
    } else {
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dataReaderAddView(
    u_dataReader _this,
    u_dataView view)
{
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        if (view) {
            _this->views = c_iterInsert(_this->views, view);
            u_entityKeep(u_entity(_this));
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT_2(OS_WARNING,
                        "u_dataReaderAddView",0,
                        "Invalid DataReaderView: "
                        "Participant = 0x%x, DataReader = 0x%x, DataReaderView = NULL.",
                        u_entityParticipant(u_entity(_this)),
                        _this);
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_dataReaderAddView",0,
                    "Failed to lock DataReader: "
                    "DataReader = 0x%x, result = %s.",
                    _this, u_resultImage(result));
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dataReaderRemoveView(
    u_dataReader _this,
    u_dataView view)
{
    u_dataView found;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        if (view) {
            found = c_iterTake(_this->views,view);
            if (found) {
                u_entityDereference(u_entity(_this));
            }
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT_2(OS_WARNING,
                        "u_dataReaderRemoveView",0,
                        "Given DataReaderView is invalid: "
                        "Participant = 0x%x, DataReader = 0x%x, DataReaderView = NULL",
                        u_entityParticipant(u_entity(_this)),
                        _this);
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_dataReaderRemoveView",0,
                    "Failed to lock DataReader: "
                    "DataReader = 0x%x, result = %s",
                    _this, u_resultImage(result));
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_dataReaderContainsView(
    u_dataReader _this,
    u_dataView view)
{
    c_bool found = FALSE;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        if (view) {
            found = c_iterContains(_this->views,view);
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT_2(OS_WARNING,
                        "u_dataReaderContainsView",0,
                        "Given DataReaderView is invalid: "
                        "Participant = 0x%x, DataReader = 0x%x, DataReaderView = NULL",
                        u_entityParticipant(u_entity(_this)),
                        _this);
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_dataReaderContainsView",0,
                    "Failed to lock DataReader: "
                    "DataReader = 0x%x, result = %s",
                    _this, u_resultImage(result));
    }
    return found;
}

c_long
u_dataReaderViewCount(
    u_dataReader _this)
{
    c_long length = -1;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        length = c_iterLength(_this->views);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_dataReaderViewCount",0,
                    "Failed to lock DataReader: "
                    "DataReader = 0x%x, result = %s",
                    _this, u_resultImage(result));
    }
    return length;
}

static void
collect_views(
    c_voidp object,
    c_voidp arg)
{
    c_iter *views;
    u_dataView view;

    view = u_dataView(object);
    views = (c_iter *)arg;

    *views = c_iterInsert(*views, view);
}

c_iter
u_dataReaderLookupViews(
    u_dataReader _this)
{
    c_iter views = NULL;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalk(_this->views, collect_views, &views);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_dataReaderLookupViews",0,
                    "Failed to lock DataReader: "
                    "DataReader = 0x%x, result = %s",
                    _this, u_resultImage(result));
    }
    return views;
}

u_result
u_dataReaderWalkViews(
    u_dataReader _this,
    u_readerAction action,
    c_voidp actionArg)
{
    c_bool result = FALSE;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalkUntil(_this->views, (c_iterAction)action, actionArg);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_dataReaderWalkViews",0,
                    "Failed to lock DataReader: "
                    "DataReader = 0x%x, result = %s",
                    _this, u_resultImage(result));
    }
    return result;
}

c_bool
u_dataReaderDataAvailable(
    u_dataReader _this)
{
    v_dataReader reader;
    v_status status;
    u_result result;
    c_bool availability = FALSE;

    if (_this) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK) {
            status = v_entityStatus(v_entity(reader));
            availability = (v_statusGetMask(status) & V_EVENT_DATA_AVAILABLE);
            u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT_2(OS_WARNING,
                        "u_dataReaderDataAvailable",0,
                        "Failed to lock DataReader: "
                        "DataReader = 0x%x, result = %s",
                        _this, u_resultImage(result));
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_dataReaderDataAvailable",0,
                  "Given DataReader is invalid: "
                  "DataReader = NULL");
        result = U_RESULT_ILL_PARAM;
    }
    return availability;
}

u_result
u_dataReaderCopyKeysFromInstanceHandle(
    u_dataReader _this,
    u_instanceHandle handle,
    u_copyOut action,
    void *copyArg)
{
    v_dataReaderInstance instance;
    u_result result;
    v_dataReader reader;
    v_message message;
    void *from;

    result = u_instanceHandleClaim(handle, &instance);
    if ((result == U_RESULT_OK) && (instance != NULL)) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK) {
            if (v_dataReaderContainsInstance(reader, instance)) {
                message = v_dataReaderInstanceCreateMessage(instance);
                if (message) {
                    from = C_DISPLACE(message, v_topicDataOffset(v_dataReaderGetTopic(reader)));
                    action(from, copyArg);
                    c_free(message);
                } else {
                    OS_REPORT_1(OS_WARNING, "u_dataReaderCopyKeysFromInstanceHandle", 0,
                        "Failed to create keytemplate message"
                        "<dataReaderInstance = 0x%x>", instance);
                    result = U_RESULT_ILL_PARAM;
                }
            } else {
                OS_REPORT_2(OS_WARNING, "u_dataReaderCopyKeysFromInstanceHandle", 0,
                    "Instance handle does not belong to reader"
                    "<_this = 0x%x handle = %lld>", _this, handle);
                result = U_RESULT_ILL_PARAM;
            }
            u_entityRelease(u_entity(_this));
        }
        u_instanceHandleRelease(handle);
    }
    return result;
}

u_result
u_dataReaderSetNotReadThreshold(
    u_reader _this,
    c_long threshold)
{
    v_dataReader reader;
    u_result result;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_entityWriteClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK){
            result = u_resultFromKernel(
                    v_dataReaderSetNotReadThreshold(reader,
                                                    threshold));
            u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT(OS_ERROR, "u_readerSetNotReadThreshold", 0,
                      "Illegal handle detected");
        }
    }
    return result;
}
