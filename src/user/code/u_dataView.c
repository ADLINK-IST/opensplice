/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "u_dataView.h"
#include "u_dataViewQos.h"
#include "u_dataReader.h"
#include "u__types.h"
#include "u__reader.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__handle.h"
#include "u__instanceHandle.h"
#include "u__user.h"
#include "v_topic.h"
#include "v_public.h"

#include "v_dataView.h"
#include "v_dataReader.h"
#include "os_report.h"

/* ----------------------------------- Private ------------------------------ */

struct instanceActionArg {
    u_dataView view;
    u_readerAction readerAction;
    void *readerActionArg;
};

typedef void (*u_dataViewInstanceActionFunc)(const v_dataViewInstance i, const c_voidp arg, const os_duration timeout);

static u_result
u_dataViewInstanceAction(
    u_instanceHandle handle,
    u_dataViewInstanceActionFunc action,
    void *arg,
    const os_duration timeout)
{
    u_result result;
    v_public instance;
    v_dataView view;
    struct instanceActionArg *a = (struct instanceActionArg *)arg;

    OS_UNUSED_ARG(timeout);

    result = u_observableReadClaim(u_observable(a->view),(v_public*)(&view), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_collection(view));
        u_observableRelease(u_observable(a->view), C_MM_RESERVATION_LOW);
        result = u_instanceHandleClaim(handle, &instance);
        if ((result == U_RESULT_OK) && (instance != NULL)) {
            assert(instance != NULL);
            action((v_dataViewInstance)instance,arg, timeout);
            u_instanceHandleRelease(handle);
        }
    }
    return result;
}

/* ----------------------------------- Protected ---------------------------- */

static u_result
u__dataViewDeinitW(
    void *_this)
{
    return u__entityDeinitW(_this);
}

static void
u__dataViewFreeW(
    void *_this)
{
    u__entityFreeW(_this);
}

static u_result
u_dataViewInit(
    const u_dataView _this,
    const v_dataView view,
    const u_dataReader reader)
{
    return u_entityInit(u_entity(_this), v_entity(view), u_observableDomain(u_observable(reader)));
}

u_dataView
u_dataViewNew(
    const u_dataReader reader,
    const os_char *name,
    const u_dataViewQos qos)
{
    v_dataView view;
    u_dataView _this = NULL;
    v_dataReader kernelReader = NULL;
    u_result result;

    assert(reader);
    assert(qos);

    if (name == NULL) {
        name = "No name specified";
    }

    result = u_observableWriteClaim(u_observable(reader), (v_public*)(&kernelReader), C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        view = v_dataViewNew(kernelReader, name, qos, TRUE);
        if (view != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_DATAVIEW, u__dataViewDeinitW, u__dataViewFreeW);
            if (_this != NULL) {
                result = u_dataViewInit(_this, view, reader);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_dataViewNew", result,
                                "Initialisation failed. "
                                "For DataView: <%s>.", name);
                    u_objectFree(u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_dataViewNew", U_RESULT_INTERNAL_ERROR,
                            "Create proxy failed. "
                            "For DataView: <%s>.", name);
            }
            c_free(view);
        } else {
            OS_REPORT(OS_ERROR, "u_dataViewNew", U_RESULT_INTERNAL_ERROR,
                        "Create kernel entity failed. "
                        "For DataView: <%s>.", name);
        }
        u_observableRelease(u_observable(reader), C_MM_RESERVATION_HIGH);
    } else {
        OS_REPORT(OS_WARNING, "u_dataViewNew", result,
                    "Claim DataReader failed. "
                    "For DataView: <%s>", name);
    }
    return _this;
}

/* ------------------------------------ Public ------------------------------ */

u_result
u_dataViewGetQos (
    const u_dataView _this,
    u_dataViewQos *qos)
{
    u_result result;
    v_dataView vView;
    v_dataViewQos vQos;

    assert(_this);
    assert(qos);

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&vView),C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        vQos = v_dataViewGetQos(vView);
        *qos = u_dataViewQosNew(vQos);
        c_free(vQos);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_dataViewSetQos (
    const u_dataView _this,
    const u_dataViewQos qos)
{
    u_result result;
    v_dataView vView;

    assert(_this);
    assert(qos);

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&vView),C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_dataViewSetQos(vView, qos));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

C_STRUCT(readActionArg) {
    u_readerAction action;
    void *arg;
    v_sampleMask mask;
};

C_CLASS(readActionArg);

static v_actionResult
readAction(
    c_object sample,
    void *arg)
{
    v_actionResult result;
    readActionArg a = (readActionArg)arg;

    /* (sample == NULL) => pass! */
    if (v_sampleMaskPass(a->mask, sample)) {
        result = a->action(sample, a->arg);
    } else {
        result = V_SKIP | V_PROCEED;
    }
    return result;
}

u_result
u_dataViewRead(
    const u_dataView _this,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;
    v_dataView view;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&view),C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        C_STRUCT(readActionArg) arg;
        arg.action = action;
        arg.arg = actionArg;
        arg.mask = mask;
        result = v_dataViewRead(view, readAction, &arg, timeout);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_dataViewRead", result,
                  "dataView could not be claimed.");
    }
    return result;
}

u_result
u_dataViewTake(
    const u_dataView _this,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;
    v_dataView view;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&view), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        C_STRUCT(readActionArg) arg;
        arg.action = action;
        arg.arg = actionArg;
        arg.mask = mask;
        result = v_dataViewTake(view, readAction, &arg, timeout);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_dataViewTake", result,
                  "dataView could not be claimed.");
    }
    return result;
}

static void
u_readInstanceAction(
    const v_dataViewInstance i,
    const c_voidp arg,
    const os_duration timeout)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;

    assert(i);
    assert(arg);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(actionArg->view),(v_public *)(&dataView), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(dataView);
        result = v_dataViewReadInstance(dataView, i,
                                        (v_readerSampleAction)actionArg->readerAction,
                                        actionArg->readerActionArg, timeout);
        u_observableRelease(u_observable(actionArg->view),C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_readInstanceAction", result,
                  "dataView could not be claimed.");
    }
}

u_result
u_dataViewReadInstance(
    const u_dataView _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    struct instanceActionArg instanceActionArg;
    u_result result;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    arg.action = action;
    arg.arg = actionArg;
    arg.mask = mask;

    instanceActionArg.view = _this;
    instanceActionArg.readerAction = readAction;
    instanceActionArg.readerActionArg = &arg;
    result = u_dataViewInstanceAction(handle, u_readInstanceAction, (c_voidp)&instanceActionArg, timeout);
    return result;
}


static void
u_takeInstanceAction(
    const v_dataViewInstance i,
    const c_voidp arg,
    const os_duration timeout)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;

    assert(i);
    assert(arg);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(actionArg->view),(v_public *)(&dataView), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(dataView);
        result = v_dataViewTakeInstance(dataView, i,
                                        (v_readerSampleAction)actionArg->readerAction,
                                        actionArg->readerActionArg, timeout);
        u_observableRelease(u_observable(actionArg->view), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_takeInstanceAction", result,
                  "dataView could not be claimed.");
    }
}

u_result
u_dataViewTakeInstance(
    const u_dataView _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;
    struct instanceActionArg instanceActionArg;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    arg.action = action;
    arg.arg = actionArg;
    arg.mask = mask;

    instanceActionArg.view = _this;
    instanceActionArg.readerAction = readAction;
    instanceActionArg.readerActionArg = &arg;

    result = u_dataViewInstanceAction(handle,
                                      u_takeInstanceAction,
                                      (c_voidp)&instanceActionArg, timeout);
    return result;
}


static void
u_readNextInstanceAction(
    const v_dataViewInstance i,
    const c_voidp arg,
    const os_duration timeout)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;

    assert(i);
    assert(arg);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(actionArg->view),(v_public *)(&dataView), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(dataView);
        result = v_dataViewReadNextInstance(dataView, i,
                                            (v_readerSampleAction)actionArg->readerAction,
                                            actionArg->readerActionArg, timeout);
        u_observableRelease(u_observable(actionArg->view),C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_readNextInstanceAction", result,
                  "dataView could not be claimed.");
    }
}


u_result
u_dataViewReadNextInstance(
    const u_dataView _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    arg.action = action;
    arg.arg = actionArg;
    arg.mask = mask;

    if (u_instanceHandleIsNil(handle)) {
        v_dataView kernelView;

        result = u_observableReadClaim(u_observable(_this),(v_public *)(&kernelView),C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            assert(kernelView);
            result = v_dataViewReadNextInstance(kernelView, NULL, readAction, &arg, timeout);
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        } else {
            OS_REPORT(OS_WARNING, "u_dataViewReadNextInstance", result,
                      "dataView could not be claimed.");
        }
    } else {
        struct instanceActionArg instanceActionArg;

        instanceActionArg.view = _this;
        instanceActionArg.readerAction = readAction;
        instanceActionArg.readerActionArg = &arg;
        result = u_dataViewInstanceAction(handle, u_readNextInstanceAction, &instanceActionArg, timeout);

        if (result == U_RESULT_HANDLE_EXPIRED) {
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
            result = u_observableReadClaim(u_observable(_this),
                                       (v_public *)(&kernelView));
            if ((result == U_RESULT_OK) && (kernelView != NULL)) {
                result = v_dataViewReadNextInstance(kernelView, NULL, readAction, &arg);
                u_observableRelease(u_observable(_this));
            } else {
                OS_REPORT(OS_WARNING, "u_dataViewReadNextInstance", result,
                          "dataView could not be claimed.");
            }
#endif
        }
    }
    return result;
}


static void
u_takeNextInstanceAction(
    const v_dataViewInstance i,
    const c_voidp arg,
    const os_duration timeout)
{
    struct instanceActionArg *actionArg = (struct instanceActionArg *)arg;
    v_dataView dataView;
    u_result result;

    assert(i);
    assert(arg);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(actionArg->view),(v_public *)(&dataView), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(dataView);
        result = v_dataViewTakeNextInstance(dataView, i,
                                            (v_readerSampleAction)actionArg->readerAction,
                                            actionArg->readerActionArg, timeout);
        u_observableRelease(u_observable(actionArg->view), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_takeNextInstanceAction", result,
                  "dataView could not be claimed.");
    }
}


u_result
u_dataViewTakeNextInstance(
    const u_dataView _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    arg.action = action;
    arg.arg = actionArg;
    arg.mask = mask;

    if (u_instanceHandleIsNil(handle)) {
        v_dataView kernelView;

        result = u_observableReadClaim(u_observable(_this),(v_public *)(&kernelView), C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            assert(kernelView);
            result = v_dataViewTakeNextInstance(kernelView, NULL, readAction, &arg, timeout);
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        } else {
            OS_REPORT(OS_WARNING, "u_dataViewTakeNextInstance", result,
                      "dataView could not be claimed.");
        }
    } else {
        struct instanceActionArg instanceActionArg;

        instanceActionArg.view = _this;
        instanceActionArg.readerAction = readAction;
        instanceActionArg.readerActionArg = &arg;
        result = u_dataViewInstanceAction(handle, u_takeNextInstanceAction, &instanceActionArg, timeout);

        if (result == U_RESULT_HANDLE_EXPIRED) {
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
            result = u_observableReadClaim(u_observable(_this),
                                       (v_public *)(&kernelView));
            if ((result == U_RESULT_OK) && (kernelView != NULL)) {
                result = v_dataViewTakeNextInstance(kernelView, NULL, readAction, &arg);
                u_observableRelease(u_observable(_this));
            } else {
                OS_REPORT(OS_WARNING, "u_dataViewTakeNextInstance", result,
                          "dataView could not be claimed.");
            }
#endif
        }
    }
    return result;
}


u_result
u_dataViewLookupInstance(
    const u_dataView _this,
    void *keyTemplate,
    u_copyIn copyIn,
    u_instanceHandle *handle)
{
    v_dataView           view;
    u_result             result;
    v_message            message;
    v_topic              topic;
    c_voidp              to;
    v_dataViewInstance   instance;

    assert(_this);
    assert(keyTemplate);
    assert(copyIn);
    assert(handle);

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&view), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(view);
        topic = v_dataReaderGetTopic(view->reader);

        if(topic){
            message = v_topicMessageNew_s(topic);
            if (message) {
                to = (void *) (message + 1);
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
                result = U_RESULT_OUT_OF_MEMORY;
                OS_REPORT(OS_ERROR,
                            "u_dataViewLookupInstance", result,
                            "Out of memory: unable to create message for Topic '%s'.",
                            name);
            }
            c_free(topic);
        } else {
            result = U_RESULT_ALREADY_DELETED;
        }
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_dataViewLookupInstance", result,
                  "dataView could not be claimed.");
    }
    return result;
}

