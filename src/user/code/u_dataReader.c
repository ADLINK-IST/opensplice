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

#include "u__dataReader.h"
#include "u__handle.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__dataView.h"
#include "u__subscriber.h"

#include "v_subscriber.h"
#include "v_topic.h"
#include "v_dataReader.h"
#include "v_dataReaderInstance.h"
#include "v_reader.h"
#include "v_query.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_statistics.h"

#include "os_report.h"

#define U_DATAREADER_SIZE sizeof(C_STRUCT(u_dataReader))

u_result
u_dataReaderClaim(
    u_dataReader _this,
    v_dataReader *reader)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (reader != NULL)) {
        *reader = v_dataReader(u_entityClaim(u_entity(_this)));
        if (*reader == NULL) {
            OS_REPORT_2(OS_WARNING, "u_dataReaderClaim", 0,
                        "Claim DataReader failed. "
                        "<_this = 0x%x, reader = 0x%x>.",
                         _this, reader);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_dataReaderClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, reader = 0x%x>.",
                    _this, reader);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dataReaderRelease(
    u_dataReader _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_dataReaderRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

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
        result = u_subscriberClaim(s,&ks);
        if ((result == U_RESULT_OK) && (ks != NULL)) {
            reader = v_dataReaderNew(ks,name, OQLexpr,params,qos,enable);
            if (reader != NULL) {
                p = u_entityParticipant(u_entity(s));
                _this = u_entityAlloc(p,u_dataReader,reader,TRUE);
                if (_this != NULL) {
                    result = u_dataReaderInit(_this);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_dataReaderNew", 0,
                                    "Initialisation failed. "
                                    "For DataReader: <%s>.", name);
                        u_entityFree(u_entity(_this));
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
            result = u_subscriberRelease(s);
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
    u_dataReader _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_readerInit(u_reader(_this));
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_dataReaderInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dataReaderFree(
    u_dataReader _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_dataReaderDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_dataReaderFree",0,
                  "The specified DataReader = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_dataReaderDeinit(
    u_dataReader _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_readerDeinit(u_reader(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_dataReaderDeinit",0, "Illegal parameter.");
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
                    "Unsuitable collection kind (%d)",v_objectKind(c));
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


C_STRUCT(readActionArg) {
    u_readerAction action;
    c_voidp arg;
    c_bool proceed;
};

C_CLASS(readActionArg);

static c_bool
readAction(
    v_readerSample sample,
    c_voidp arg)
{
    readActionArg a = (readActionArg)arg;

    if (a && a->action) {
        if (sample == NULL) {
            a->action(sample,a->arg);
        } else {
            a->proceed = a->action(sample,a->arg);
        }
    }
    return a->proceed;
}

u_result
u_dataReaderRead(
    u_dataReader _this,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    v_dataReader reader;
    C_STRUCT(readActionArg) arg;

    result = u_dataReaderClaim(_this,&reader);

    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.proceed = TRUE;
        v_dataReaderRead(reader,readAction,&arg);
        u_dataReaderRelease(_this);
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

		result = u_dataReaderClaim(_this,&reader);
		if (result == U_RESULT_OK) {
			handle = u_instanceHandleFix(handle,v_collection(reader));
			result = u_instanceHandleClaim(handle, &instance);
			if (result == U_RESULT_OK) {
				if (v_dataReaderContainsInstance(reader,instance)) {
					*userData_out =
							v_dataReaderInstanceGetUserData (instance);
				} else {
					result = U_RESULT_PRECONDITION_NOT_MET;
				}
				u_instanceHandleRelease(handle);
			}
			u_dataReaderRelease(_this);
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

    result = u_dataReaderClaim(_this,&reader);
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_collection(reader));
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            if (v_dataReaderContainsInstance(reader,instance)) {
                v_dataReaderInstanceSetUserData (instance, userData);
            } else {
                result = U_RESULT_PRECONDITION_NOT_MET;
            }
            u_instanceHandleRelease(handle);
        }
        u_dataReaderRelease(_this);
    }
    return result;
}

u_result
u_dataReaderTake(
    u_dataReader _this,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    v_dataReader reader;
    C_STRUCT(readActionArg) arg;

    result = u_dataReaderClaim(_this,&reader);

    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.proceed = TRUE;
        v_dataReaderTake(reader,readAction,&arg);
        u_dataReaderRelease(_this);
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

static c_bool
readListAction(
    v_readerSample sample,
    c_voidp arg)
{
    readListActionArg a = (readListActionArg)arg;

    if (a->spaceLeft == 0U) {
        return FALSE;
    }
    if (sample == NULL) {
        a->result = a->copyAction(NULL,a->iter,a->copyArg);
        return FALSE;
    }
    a->iter = c_iterInsert(a->iter,c_keep(sample));
    a->spaceLeft--;
    if (a->spaceLeft == 0U) {
        a->result = a->copyAction(NULL,a->iter,a->copyArg);
        return FALSE;
    } else {
        return TRUE;
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
    result = u_dataReaderClaim(_this,&reader);
    if (result == U_RESULT_OK) {
        if (max == 0U) {
            arg.spaceLeft = 0x7fffffffU;
        } else {
            arg.spaceLeft = max;
        }
        v_dataReaderRead(reader,readListAction,(c_voidp)&arg);
        list = arg.iter;
        object = c_iterTakeFirst(list);
        while (object != NULL) {
            c_free(object);
            object = c_iterTakeFirst(list);
        }
        c_iterFree(list);
        u_dataReaderRelease(_this);
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
    result = u_dataReaderClaim(_this,&reader);
    if (result == U_RESULT_OK) {
        if (max == 0U) {
            arg.spaceLeft = 0x7fffffffU;
        } else {
            arg.spaceLeft = max;
        }
        v_dataReaderTake(reader,readListAction,(c_voidp)&arg);
        list = arg.iter;
        object = c_iterTakeFirst(list);
        while (object != NULL) {
            c_free(object);
            object = c_iterTakeFirst(list);
        }
        c_iterFree(list);
        u_dataReaderRelease(_this);
    }
    return arg.result;
}

/***************************** read/take_(next)_instance **********************/

u_result
u_dataReaderReadInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_dataReaderClaim(_this,&reader);
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
                result = U_RESULT_PRECONDITION_NOT_MET;
            }
            u_instanceHandleRelease(handle);
        }
        u_dataReaderRelease(_this);
    }
    return result;
}

u_result
u_dataReaderTakeInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_dataReaderClaim(_this,&reader);
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
                result = U_RESULT_PRECONDITION_NOT_MET;
            }
            u_instanceHandleRelease(handle);
        }
        u_dataReaderRelease(_this);
    }
    return result;
}

u_result
u_dataReaderReadNextInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_dataReaderClaim(_this,&reader);
    if (result == U_RESULT_OK) {
        if (u_instanceHandleIsNil(handle)) {
            v_dataReaderReadNextInstance(reader,
                                         NULL,
                                         (v_readerSampleAction)action,
                                         actionArg);
        } else {
            handle = u_instanceHandleFix(handle,v_collection(reader));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_OK) {
                assert(instance != NULL);
                if (v_dataReaderContainsInstance(reader,instance)) {
                    v_dataReaderReadNextInstance(reader,
                                                 instance,
                                                 (v_readerSampleAction)action,
                                                 actionArg);
                } else {
                    result = U_RESULT_PRECONDITION_NOT_MET;
                }
                u_instanceHandleRelease(handle);
            }
        }
        u_dataReaderRelease(_this);
    }
    return result;
}

u_result
u_dataReaderTakeNextInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    result = u_dataReaderClaim(_this,&reader);
    if (result == U_RESULT_OK) {
        if (u_instanceHandleIsNil(handle)) {
            v_dataReaderTakeNextInstance(reader,
                                         NULL,
                                         (v_readerSampleAction)action,
                                         actionArg);
        } else {
            handle = u_instanceHandleFix(handle,v_collection(reader));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_OK) {
                assert(instance != NULL);
                if (v_dataReaderContainsInstance(reader,instance)) {
                    v_dataReaderTakeNextInstance(reader,
                                                 instance,
                                                 (v_readerSampleAction)action,
                                                 actionArg);
                } else {
                    result = U_RESULT_PRECONDITION_NOT_MET;
                }
                u_instanceHandleRelease(handle);
            }
        }
        u_dataReaderRelease(_this);
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

    result = u_dataReaderClaim(_this,&reader);

    if (result == U_RESULT_OK) {
        if(v_readerWaitForHistoricalData(v_reader(reader), timeout) == TRUE) {
            result = U_RESULT_OK;
        } else {
            result = U_RESULT_TIMEOUT;
        }
        u_dataReaderRelease(_this);
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

    result = u_dataReaderClaim(_this,&reader);

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
        u_dataReaderRelease(_this);
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

    result = u_dataReaderClaim(_this,&reader);

    if (result == U_RESULT_OK) {
        topic = v_dataReaderGetTopic(reader);
        message = v_topicMessageNew(topic);
        to = C_DISPLACE(message, v_topicDataOffset(topic));
        copyIn(v_topicDataType(topic), keyTemplate, to);
        instance = v_dataReaderLookupInstance(reader, message);
        *handle = u_instanceHandleNew(v_public(instance));
        c_free(instance);
        c_free(topic);
        c_free(message);
        u_dataReaderRelease(_this);
    }
    return result;
}
