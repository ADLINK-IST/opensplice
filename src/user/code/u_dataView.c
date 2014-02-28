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

#include "u__dataView.h"
#include "u__dataReader.h"
#include "u__types.h"
#include "u__entity.h"
#include "u_user.h"
#include "v_topic.h"
#include "v_public.h"

#include "v_dataView.h"
#include "v_dataViewInstance.h"
#include "v_dataReader.h"
#include "os_report.h"


/* ----------------------------------- Private ------------------------------ */

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

    result = u_entityReadClaim(u_entity(a->view),(v_entity*)(&view));
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_collection(view));
        u_entityRelease(u_entity(a->view));
        result = u_instanceHandleClaim(handle, &instance);
        if ((result == U_RESULT_OK) && (instance != NULL)) {
            assert(instance != NULL);
            action(v_dataViewInstance(instance),arg);
            u_instanceHandleRelease(handle);
        } else if (result == U_RESULT_ALREADY_DELETED){
            /* if the instance is already deleted, then the result of
             * any dataViewInstanceAction is PRECONDITION_NOT_MET */
            result = U_RESULT_PRECONDITION_NOT_MET;
            assert(instance == NULL);
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
        result = u_entityWriteClaim(u_entity(reader), (v_entity*)(&kernelReader));
        if (result == U_RESULT_OK) {
            view = v_dataViewNew(kernelReader, name, qos, TRUE);
            if (view != NULL) {
                participant = u_entityParticipant(u_entity(reader));
                _this = u_entityAlloc(participant,u_dataView,view,TRUE);
                if (_this != NULL) {
                    result = u_dataViewInit(_this, reader);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_dataViewNew", 0,
                                    "Initialisation failed. "
                                    "For DataView: <%s>.", name);
                        u_dataViewFree(_this);
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
            result = u_entityRelease(u_entity(reader));
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
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        /* if refCount becomes zero then this call
         * returns true and destruction can take place
         */
        if (destroy) {
            if (u_entityOwner(u_entity(_this))) {
                result = u_dataViewDeinit(_this);
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
                            "u_dataViewFree",0,
                            "Operation u_dataViewDeinit failed: "
                            "DataView = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_dataViewFree",0,
                    "Operation u_entityLock failed: "
                    "DataView = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_dataViewInit(
    u_dataView _this,
    u_dataReader source)
{
    u_result result;

    if (_this && source) {
        result = u_readerInit(u_reader(_this));
        if (result == U_RESULT_OK) {
            _this->source = source;
            result = u_dataReaderAddView(_this->source, _this);
        }
    } else {
        OS_REPORT_2(OS_ERROR,
                    "u_dataViewInit",0,
                    "Illegal parameter: _this = 0x%x, source = 0x%x.",
                    _this,source);
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
        result = u_dataReaderRemoveView(_this->source, _this);
        if (result == U_RESULT_OK) {
            _this->source = NULL;
            result = u_entityDeinit(u_entity(_this));
        }
    } else {
        OS_REPORT_1(OS_ERROR,
                    "u_dataViewDeinit",0,
                    "Illegal parameter: _this = 0x%x.",
                    _this);
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

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&view));
    if (result == U_RESULT_OK) {
        assert(view);
        v_dataViewRead(view, (v_readerSampleAction)action, actionArg);
        result = u_entityRelease(u_entity(_this));
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

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&view));
    if (result == U_RESULT_OK) {
        assert(view);

        v_dataViewTake(view, (v_readerSampleAction)action, actionArg);
        result = u_entityRelease(u_entity(_this));
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

    result = u_entityReadClaim(u_entity(actionArg->view),(v_entity*)(&dataView));
    if (result == U_RESULT_OK) {
        assert(dataView);
        v_dataViewReadInstance(dataView, i,
                               (v_readerSampleAction)actionArg->readerAction,
                               actionArg->readerActionArg);
        result = u_entityRelease(u_entity(actionArg->view));
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
    u_result result;

    instanceActionArg.view = _this;
    instanceActionArg.readerAction = action;
    instanceActionArg.readerActionArg = actionArg;
    result = u_dataViewInstanceAction(handle,
                                      u_readInstanceAction,
                                      (c_voidp)&instanceActionArg);
    if (result == U_RESULT_ALREADY_DELETED) {
        /* Error propagation moves the role of the handle from
         * being the object to being a parameter, this affect the
         * already deleted status into precondition not met status.
         */
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}


static void
u_takeInstanceAction(
    v_dataViewInstance i,
    c_voidp arg)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;

    result = u_entityReadClaim(u_entity(actionArg->view),(v_entity*)(&dataView));
    if (result == U_RESULT_OK) {
        assert(dataView);
        v_dataViewTakeInstance(dataView, i,
                               (v_readerSampleAction)actionArg->readerAction,
                               actionArg->readerActionArg);
        result = u_entityRelease(u_entity(actionArg->view));
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
    u_result result;
    struct instanceActionArg instanceActionArg;

    instanceActionArg.view = _this;
    instanceActionArg.readerAction = action;
    instanceActionArg.readerActionArg = actionArg;
    result = u_dataViewInstanceAction(handle,
                                      u_takeInstanceAction,
                                      (c_voidp)&instanceActionArg);
    if (result == U_RESULT_ALREADY_DELETED) {
        /* Error propagation moves the role of the handle from
         * being the object to being a parameter, this affect the
         * already deleted status into precondition not met status.
         */
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}


static void
u_readNextInstanceAction(
    v_dataViewInstance i,
    c_voidp arg)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;

    result = u_entityReadClaim(u_entity(actionArg->view),(v_entity*)(&dataView));
    if (result == U_RESULT_OK) {
        assert(dataView);
        v_dataViewReadNextInstance(dataView, i,
                                   (v_readerSampleAction)actionArg->readerAction,
                                   actionArg->readerActionArg);
        result = u_entityRelease(u_entity(actionArg->view));
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

        result = u_entityReadClaim(u_entity(_this),(v_entity*)(&kernelView));
        if (result == U_RESULT_OK) {
            assert(kernelView);
            v_dataViewReadNextInstance(kernelView,
                                       NULL,
                                       (v_readerSampleAction)action,
                                       actionArg);
            u_entityRelease(u_entity(_this));
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
        if (result == U_RESULT_ALREADY_DELETED) {
#if 0
            v_dataView kernelView;
            /* The handle has become invalid and no instance including
             * the key value can be found. Therefore set the instance
             * to null and start reading from scratch.
             * Doing an automatic correction of already deleted handles
             * hides information for the user but can be a convenience
             * for iterating over all instances.
             * Conceptual this should be left out.
             */
            result = u_entityReadClaim(u_entity(_this),
                                       (v_entity*)(&kernelView));
            if ((result == U_RESULT_OK) && (kernelView != NULL)) {
                v_dataViewReadNextInstance(kernelView,
                                           NULL,
                                           (v_readerSampleAction)action,
                                           actionArg);
                result = u_entityRelease(u_entity(_this));
            } else {
                OS_REPORT(OS_WARNING, "u_dataViewReadNextInstance", 0,
                          "dataView could not be claimed.");
            }
#else
            /* This result value is expected by the testcases but if
             * we want to be inline with the behavior of DataReader's
             * read_next_instance then we should return bad parameter.
             */
            result = U_RESULT_PRECONDITION_NOT_MET;
#endif
        }
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

    result = u_entityReadClaim(u_entity(actionArg->view),(v_entity*)(&dataView));
    if (result == U_RESULT_OK) {
        assert(dataView);
        v_dataViewTakeNextInstance(dataView, i,
                                   (v_readerSampleAction)actionArg->readerAction,
                                   actionArg->readerActionArg);
        result = u_entityRelease(u_entity(actionArg->view));
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

        result = u_entityReadClaim(u_entity(_this),(v_entity*)(&kernelView));
        if (result == U_RESULT_OK) {
            assert(kernelView);
            v_dataViewTakeNextInstance(kernelView,
                                       NULL,
                                       (v_readerSampleAction)action,
                                       actionArg);
            u_entityRelease(u_entity(_this));
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
        if (result == U_RESULT_ALREADY_DELETED) {
#if 0
            v_dataView kernelView;
            /* The handle has become invalid and no instance including
             * the key value can be found. Therefore set the instance
             * to null and start reading from scratch.
             * Doing an automatic correction of already deleted handles
             * hides information for the user but can be a convenience
             * for iterating over all instances.
             * Conceptual this should be left out.
             */
            result = u_entityReadClaim(u_entity(_this),
                                       (v_entity*)(&kernelView));
            if ((result == U_RESULT_OK) && (kernelView != NULL)) {
                v_dataViewTakeNextInstance(kernelView,
                                           NULL,
                                           (v_readerSampleAction)action,
                                           actionArg);
                result = u_entityRelease(u_entity(_this));
            } else {
                OS_REPORT(OS_WARNING, "u_dataViewTakeNextInstance", 0,
                          "dataView could not be claimed.");
            }
#else
            /* This result value is expected by the testcases but if
             * we want to be inline with the behavior of DataReader's
             * read_next_instance then we should return bad parameter.
             */
            result = U_RESULT_PRECONDITION_NOT_MET;
#endif
        }
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

    result = u_entityReadClaim(u_entity(_this),(v_entity*)(&view));
    if (result == U_RESULT_OK) {
        assert(view);
        topic = v_dataReaderGetTopic(view->reader);
        message = v_topicMessageNew(topic);
        if (message) {
            to = C_DISPLACE(message, v_topicDataOffset(topic));
            copyIn(v_topicDataType(topic), keyTemplate, to);
            instance = v_dataViewLookupInstance(view, message);
            *handle = u_instanceHandleNew(v_public(instance));
            c_free(instance);
            c_free(message);
        } else {
            c_char *name = v_topicName(topic);
            if (name == NULL) {
                name = "No Name";
            }
            OS_REPORT_1(OS_ERROR,
                        "u_dataViewLookupInstance", 0,
                        "Out of memory: unable to create message for Topic '%s'.",
                        name);
            result = U_RESULT_OUT_OF_MEMORY;
        }
        c_free(topic);
        u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING, "u_dataViewLookupInstance", 0,
                  "dataView could not be claimed.");
    }
    return result;
}

u_dataReader
u_dataViewSource(
    u_dataView _this)
{
    assert(_this != NULL);
    assert(u_entityKind(u_entity(_this)) == U_DATAVIEW);

    return _this->source;
}

