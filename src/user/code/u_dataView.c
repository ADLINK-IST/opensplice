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

#include "u__dataView.h"
#include "u__dataReader.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__handle.h"
#include "u_user.h"
#include "v_topic.h"
#include "v_public.h"

#include "v_dataView.h"
#include "v_dataViewInstance.h"
#include "v_dataReader.h"
#include "os_report.h"


/* ----------------------------------- Private ------------------------------ */

u_result
u_dataViewClaim(
    u_dataView _this,
    v_dataView *view)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        *view = v_dataView(u_entityClaim(u_entity(_this)));
        if (*view == NULL) {
            OS_REPORT_2(OS_WARNING, "u_dataViewClaim", 0,
                        "DataView could not be claimed. "
                        "<_this = 0x%x, view = 0x%x>.",
                         _this, view);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_dataViewClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, view = 0x%x>.",
                     _this, view);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dataViewRelease(
    u_dataView _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_dataViewRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}


struct instanceActionArg {
    u_dataView view;
    u_readerAction readerAction;
    c_voidp readerActionArg;
};

typedef void (*u_dataViewInstanceActionFunc)(v_dataViewInstance i, c_voidp arg);

static u_result
u_dataViewInstanceAction(
    u_instanceHandle handle,
    u_dataViewInstanceActionFunc action,
    c_voidp arg)
{
    u_result result;
    v_public instance;
    v_dataView view;
    struct instanceActionArg *a = (struct instanceActionArg *)arg;

    result = u_dataViewClaim(a->view,&view);
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_reader(view));
        u_dataViewRelease(a->view);
        result = u_instanceHandleClaim(handle, &instance);
        if ((result == U_RESULT_OK) && (instance != NULL)) {
            assert(instance != NULL);
            action(v_dataViewInstance(instance),arg);
            u_instanceHandleRelease(handle);
        }
    }
    return result;
}

/* ----------------------------------- Protected ---------------------------- */


u_dataView
u_dataViewNew(
    u_dataReader reader,
    const c_char *name,
    v_dataViewQos qos)
{
    u_participant participant;
    v_dataView view;
    u_dataView _this = NULL;
    v_dataReader kernelReader = NULL;
    u_result result;

    if (name == NULL) {
        name = "No name specified";
    }
    if (reader != NULL) {
        result = u_dataReaderClaim(reader, &kernelReader);
        if ((result == U_RESULT_OK) || (kernelReader != NULL)) {
            view = v_dataViewNew(kernelReader, name, qos, TRUE);
            if (view != NULL) {
                participant = u_entityParticipant(u_entity(reader));
                _this = u_entityAlloc(participant,u_dataView,view,TRUE);
                if (_this != NULL) {
                    result = u_dataViewInit(_this, u_reader(reader));
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_dataViewNew", 0,
                                    "Initialisation failed. "
                                    "For DataView: <%s>.", name);
                        u_entityFree(u_entity(_this));
                    }
                } else {
                    OS_REPORT_1(OS_ERROR, "u_dataViewNew", 0,
                                "Create proxy failed. "
                                "For DataView: <%s>.", name);
                }
                c_free(view);
            } else {
                OS_REPORT_1(OS_ERROR, "u_dataViewNew", 0,
                            "Create kernel entity failed. "
                            "For DataView: <%s>.", name);
            }
            result = u_dataReaderRelease(reader);
        } else {
            OS_REPORT_1(OS_WARNING, "u_dataViewNew", 0,
                        "Claim DataReader failed. "
                        "For DataView: <%s>", name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_dataViewNew",0,
                    "Illegal parameter. "
                    "For DataView: <%s>", name);
    }
    return _this;
}


u_result
u_dataViewFree(
    u_dataView _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_dataViewDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_dataViewFree",0,
                  "The specified DATAVIEW = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_dataViewInit(
    u_dataView _this,
    u_reader reader)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
        _this->source = reader;
    } else {
        OS_REPORT(OS_ERROR,"u_dataViewInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dataViewDeinit(
    u_dataView _this)
{
    u_result result;

    if (_this != NULL) {
        _this->source = NULL;
        result = u_entityDeinit(u_entity(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_dataViewDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}


/* ------------------------------------ Public ------------------------------ */

u_result
u_dataViewRead(
    u_dataView _this,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    v_dataView view;
    
    result = u_dataViewClaim(_this, &view);
    
    if ((result == U_RESULT_OK) && (view != NULL)) {
        v_dataViewRead(view, (v_readerSampleAction)action, actionArg);
        result = u_dataViewRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_dataViewRead", 0,
                  "dataView could not be claimed.");
    }
    return result;
}
                
u_result
u_dataViewTake(
    u_dataView _this,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    v_dataView view;
    
    result = u_dataViewClaim(_this, &view);
    
    if ((result == U_RESULT_OK) && (view != NULL)) {
        v_dataViewTake(view, (v_readerSampleAction)action, actionArg);
        result = u_dataViewRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_dataViewTake", 0,
                  "dataView could not be claimed.");
    }
    return result;
}

static void
u_readInstanceAction(
    v_dataViewInstance i,
    c_voidp arg)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;
    
    result = u_dataViewClaim(actionArg->view, &dataView);
    if ((result == U_RESULT_OK) && (dataView != NULL)) {
        v_dataViewReadInstance(dataView, i,
                               (v_readerSampleAction)actionArg->readerAction,
                               actionArg->readerActionArg);
        result = u_dataViewRelease(actionArg->view);
    } else {
        OS_REPORT(OS_WARNING, "u_readInstanceAction", 0,
                  "dataView could not be claimed.");
    }
}    

u_result
u_dataViewReadInstance(
    u_dataView _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    struct instanceActionArg instanceActionArg;
    
    instanceActionArg.view = _this;
    instanceActionArg.readerAction = action;
    instanceActionArg.readerActionArg = actionArg;
    return u_dataViewInstanceAction(handle,
                                    u_readInstanceAction,
                                    (c_voidp)&instanceActionArg);
}


static void
u_takeInstanceAction(
    v_dataViewInstance i,
    c_voidp arg)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;
    
    result = u_dataViewClaim(actionArg->view, &dataView);
    if ((result == U_RESULT_OK) && (dataView != NULL)) {
        v_dataViewTakeInstance(dataView, i,
                               (v_readerSampleAction)actionArg->readerAction,
                               actionArg->readerActionArg);
        result = u_dataViewRelease(actionArg->view);
    } else {
        OS_REPORT(OS_WARNING, "u_takeInstanceAction", 0,
                  "dataView could not be claimed.");
    }
}    

u_result
u_dataViewTakeInstance(
    u_dataView _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    struct instanceActionArg instanceActionArg;
    
    instanceActionArg.view = _this;
    instanceActionArg.readerAction = action;
    instanceActionArg.readerActionArg = actionArg;
    return u_dataViewInstanceAction(handle,
                                    u_takeInstanceAction,
                                    (c_voidp)&instanceActionArg);
}


static void
u_readNextInstanceAction(
    v_dataViewInstance i,
    c_voidp arg)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;
    
    result = u_dataViewClaim(actionArg->view, &dataView);
    if ((result == U_RESULT_OK) && (dataView != NULL)) {
        v_dataViewReadNextInstance(dataView, i,
                                   (v_readerSampleAction)actionArg->readerAction,
                                   actionArg->readerActionArg);
        result = u_dataViewRelease(actionArg->view);
    } else {
        OS_REPORT(OS_WARNING, "u_readNextInstanceAction", 0,
                  "dataView could not be claimed.");
    }
}    


u_result
u_dataViewReadNextInstance(
    u_dataView _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    
    if (u_instanceHandleIsNil(handle)) {
        v_dataView kernelView;
    
        result = u_dataViewClaim(_this, &kernelView);
    
        if ((result == U_RESULT_OK) && (kernelView != NULL)) {
            v_dataViewReadNextInstance(kernelView,
                                       NULL,
                                       (v_readerSampleAction)action,
                                       actionArg);
            u_dataViewRelease(_this);
        } else {
            OS_REPORT(OS_WARNING, "u_dataViewReadNextInstance", 0,
                      "dataView could not be claimed.");
        }
    } else {
        struct instanceActionArg instanceActionArg;
        
        instanceActionArg.view = _this;
        instanceActionArg.readerAction = action;
        instanceActionArg.readerActionArg = actionArg;
        result = u_dataViewInstanceAction(handle,
                                          u_readNextInstanceAction,
                                          &instanceActionArg);
    }
    return result;
}


static void
u_takeNextInstanceAction(
    v_dataViewInstance i,
    c_voidp arg)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;
    
    result = u_dataViewClaim(actionArg->view, &dataView);
    if ((result == U_RESULT_OK) && (dataView != NULL)) {
        v_dataViewTakeNextInstance(dataView, i,
                                   (v_readerSampleAction)actionArg->readerAction,
                                   actionArg->readerActionArg);
        result = u_dataViewRelease(actionArg->view);
    } else {
        OS_REPORT(OS_WARNING, "u_takeNextInstanceAction", 0,
                  "dataView could not be claimed.");
    }
}    


u_result
u_dataViewTakeNextInstance(
    u_dataView _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    
    if (u_instanceHandleIsNil(handle)) {
        v_dataView kernelView;
    
        result = u_dataViewClaim(_this, &kernelView);
    
        if ((result == U_RESULT_OK) && (kernelView != NULL)) {
            v_dataViewTakeNextInstance(kernelView,
                                       NULL,
                                       (v_readerSampleAction)action,
                                       actionArg);
            u_dataViewRelease(_this);
        } else {
            OS_REPORT(OS_WARNING, "u_dataViewTakeNextInstance", 0, 
                      "dataView could not be claimed.");
        }
    } else {
        struct instanceActionArg instanceActionArg;
    
        instanceActionArg.view = _this;
        instanceActionArg.readerAction = action;
        instanceActionArg.readerActionArg = actionArg;
        result = u_dataViewInstanceAction(handle,
                                          u_takeNextInstanceAction,
                                          &instanceActionArg);
    }
    return result;
}


u_result
u_dataViewLookupInstance(
    u_dataView _this,
    c_voidp keyTemplate,
    u_copyIn copyIn,
    u_instanceHandle *handle)
{
    v_dataView           view;
    u_result             result;
    v_message            message;
    v_topic              topic;
    c_voidp              to;
    v_dataViewInstance   instance;

    if ((_this == NULL) ||
        (keyTemplate == NULL) ||
        (copyIn == NULL) ||
        (handle == NULL)) {
        return U_RESULT_ILL_PARAM;
    }

    result = u_dataViewClaim(_this,&view);
    
    if ((result == U_RESULT_OK) && (view != NULL)) {
        topic = v_dataReaderGetTopic(view->reader);
        message = v_topicMessageNew(topic);
        to = C_DISPLACE(message, v_topicDataOffset(topic));
        copyIn(v_topicDataType(topic), keyTemplate, to);
        instance = v_dataViewLookupInstance(view, message);
        *handle = u_instanceHandleNew(v_public(instance));
        c_free(instance);
        c_free(topic);
        c_free(message);
        u_dataViewRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_dataViewLookupInstance", 0, 
                  "dataView could not be claimed.");
    }
    return result;
}

u_reader
u_dataViewSource(
    u_dataView _this)
{
    assert(_this != NULL);
    assert(u_entityKind(u_entity(_this)) == U_DATAVIEW);

    return _this->source;
}

