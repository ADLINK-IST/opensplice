/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
/* Interface */
#include "v__dataView.h"

/* Implementation */
#include "vortex_os.h"
#include "os_report.h"

#include "c_stringSupport.h"

#include "v__entity.h"
#include "v__dataReader.h"
#include "v__query.h"
#include "v_dataViewInstance.h"
#include "v_readerQos.h"
#include "v_dataViewQos.h"
#include "v_qos.h"
#include "v_public.h"
#include "v__topic.h"
#include "v__index.h"
#include "v__observer.h"
#include "v_observable.h"
#include "v_state.h"
#include "v__collection.h"
#include "v__subscriber.h"
#include "v__kernel.h"

/* ------------------------------- private ---------------------------------- */

c_type
dataViewSampleTypeNew(
    v_dataReader dataReader)
{
    c_metaObject o;
    c_type sampleType,foundType,readerSampleType;
    c_base base;
    c_char *name;
    int sres;
    os_size_t length;
    c_char *metaName;
    v_kernel kernel;

    assert(C_TYPECHECK(dataReader,v_dataReader));
    assert(dataReader);

    kernel = v_objectKernel(dataReader);

    base = c_getBase(dataReader);

    if (base == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "v_dataView::dataViewSampleTypeNew",V_RESULT_INTERNAL_ERROR,
                  "failed to retrieve base");
        return NULL;
    }

    readerSampleType = v_dataReaderSampleType(dataReader);
    if (readerSampleType == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "v_dataView::dataViewSampleTypeNew",V_RESULT_INTERNAL_ERROR,
                  "failed to retrieve sample type from dataReader");
        return NULL;
    }

    foundType = NULL;

    metaName = c_metaName(c_metaObject(readerSampleType));
    if (metaName) {
        sampleType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
        if (sampleType) {
           c_class(sampleType)->extends = c_keep(v_kernelType(kernel,K_DATAVIEWSAMPLE));
            o = c_metaDeclare(c_metaObject(sampleType),"sample",M_ATTRIBUTE);
            if (o) {
                c_property(o)->type = c_keep(readerSampleType);
                c_metaObject(sampleType)->definedIn = c_keep(base);
                c_metaFinalize(c_metaObject(sampleType));

#define SAMPLE_FORMAT "v_dataViewSample<%s>"
#define SAMPLE_NAME   "v_dataViewSample<>"
                /* sizeof contains \0 */
                length = sizeof(SAMPLE_NAME) + strlen(metaName);
                name = os_malloc(length);
                sres = snprintf(name,length,SAMPLE_FORMAT, metaName);
                assert(sres >= 0 && (os_size_t) sres == (length-1));
                OS_UNUSED_ARG(sres);
#undef SAMPLE_FORMAT
#undef SAMPLE_NAME

                foundType = c_type(c_metaBind(c_metaObject(base),name,
                                              c_metaObject(sampleType)));
                os_free(name);
                c_free(o);
            } else {
                OS_REPORT(OS_CRITICAL,
                          "v_dataView::dataViewSampleTypeNew",V_RESULT_INTERNAL_ERROR,
                          "failed to declare new sample type sample attribute");
            }
            c_free(sampleType);
        } else {
            OS_REPORT(OS_CRITICAL,
                      "v_dataView::dataViewSampleTypeNew",V_RESULT_INTERNAL_ERROR,
                      "failed to define new sample type");
        }
        c_free(metaName);
    } else {
        OS_REPORT(OS_CRITICAL,
                  "v_dataView::dataViewSampleTypeNew",V_RESULT_INTERNAL_ERROR,
                  "failed to retrieve sample type name");
    }

    c_free(readerSampleType);

    return foundType;
}

c_type
dataViewInstanceTypeNew(
    v_kernel kernel,
    c_type viewSampleType)
{
    c_metaObject o;
    c_type instanceType, foundType;
    c_base base;
    c_char *name;
    os_size_t length;
    int sres;
    c_char *metaName;

    assert(C_TYPECHECK(viewSampleType,c_type));
    assert(viewSampleType);

    base = c_getBase(viewSampleType);

    if (base == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "v_dataView::dataViewInstanceTypeNew",V_RESULT_INTERNAL_ERROR,
                  "failed to retrieve base");
        return NULL;
    }

    metaName = c_metaName(c_metaObject(viewSampleType));
    if (metaName == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "v_dataView::dataViewInstanceTypeNew",V_RESULT_INTERNAL_ERROR,
                  "failed to retrieve sample type name");
        return NULL;
    }

    foundType = NULL;

    instanceType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    if (instanceType) {
        c_class(instanceType)->extends = c_keep(v_kernelType(kernel,K_DATAVIEWINSTANCE));
        o = c_metaObject(c_metaDeclare(c_metaObject(instanceType),
                                       "sample",
                                       M_ATTRIBUTE));
        if (o) {
            c_property(o)->type = c_keep(viewSampleType);
            c_metaObject(instanceType)->definedIn = c_keep(base);
            c_metaFinalize(c_metaObject(instanceType));

#define INSTANCE_NAME   "v_dataViewInstance<>"
#define INSTANCE_FORMAT "v_dataViewInstance<%s>"
            /* The sizeof contains \0 */
            length = sizeof(INSTANCE_NAME) + strlen(metaName);
            name = os_malloc(length);
            sres = snprintf(name,length,INSTANCE_FORMAT, metaName);
            assert(sres >= 0 && (os_size_t) sres == (length-1));
            OS_UNUSED_ARG(sres);
#undef INSTANCE_NAME
#undef INSTANCE_FORMAT

            foundType = c_type(c_metaBind(c_metaObject(base),name,
                                          c_metaObject(instanceType)));

            os_free(name);
            c_free(o);
        } else {
            OS_REPORT(OS_CRITICAL,
                      "v_dataView::dataViewInstanceTypeNew",V_RESULT_INTERNAL_ERROR,
                      "failed to declare sampleType->sample attribute");
        }
        c_free(instanceType);
    } else {
        OS_REPORT(OS_CRITICAL,
                  "v_dataView::dataViewInstanceTypeNew",V_RESULT_INTERNAL_ERROR,
                  "failed to define instance type name");
    }
    c_free(metaName);

    return foundType;
}

static void
v_dataViewInit(
    v_dataView dataView,
    v_dataReader dataReader,
    const c_char *name,
    v_dataViewQos qos,
    c_bool enable)
{
    c_type dataViewSampleType;
    c_type dataViewInstanceType;
    c_iter keyExprNames;
    c_string fieldName;
    c_ulong nrOfKeys;
    os_size_t totalSize;
    c_char *keyExpr, *prefix;
    v_dataViewQos q;
    v_kernel kernel;

    assert(C_TYPECHECK(dataView,v_dataView));
    assert(C_TYPECHECK(dataReader,v_dataReader));

    kernel = v_objectKernel(dataReader);
    q = v_dataViewQosNew(kernel, qos);
    dataView->qos = q;

    v_collectionInit(v_collection(dataView), name, enable);

    dataViewSampleType = dataViewSampleTypeNew(dataReader);
    assert(dataViewSampleType != NULL);
    dataViewInstanceType = dataViewInstanceTypeNew(kernel, dataViewSampleType);
    assert(dataViewInstanceType != NULL);
    /* When the view has defined its own keys, then grab the key-definition from
     * the userKeyQosPolicy and the key-values from the messages that are passed.
     */
    if (qos->userKey.v.enable)
    {
        if (qos->userKey.v.expression)
        {
            totalSize = strlen(qos->userKey.v.expression) + 1;
            keyExpr = os_malloc(totalSize);
            os_strncpy(keyExpr, qos->userKey.v.expression, totalSize);
        }
        else
        {
            keyExpr = NULL;
        }
    }
    /* When the view-spectrum  is fully slaved to a reader, then grab the key-
     * definition from the corresponding dataReader. These keys can either be
     * the original keys as specified on the topic, or the reader may have
     * overruled the topic keys by means of its own userKeyQosPolicy.
     */
    else if (v_reader(dataReader)->qos->userKey.v.enable)
    {
        if (v_reader(dataReader)->qos->userKey.v.expression)
        {
            totalSize = strlen(v_reader(dataReader)->qos->userKey.v.expression) + 1;
            keyExpr = os_malloc(totalSize);
            os_strncpy(keyExpr, v_reader(dataReader)->qos->userKey.v.expression, totalSize);
        }
        else
        {
            keyExpr = NULL;
        }
    }
    else
    {
        v_topic topic;
        topic = v_dataReaderGetTopic(dataReader);
        if (v_topicKeyExpr(topic) != NULL)
        {
            totalSize = strlen(v_topicKeyExpr(topic)) + 1;
            keyExpr = os_malloc(totalSize);
            os_strncpy(keyExpr, v_topicKeyExpr(topic), totalSize);
        }
        else
        {
            keyExpr = NULL;
        }
        c_free(topic);
    }
    prefix = "sample.sample.message.userData.";
    if (keyExpr != NULL) {
        keyExprNames = c_splitString(keyExpr,", \t");
        nrOfKeys = c_iterLength(keyExprNames);
        totalSize = strlen(keyExpr) + (nrOfKeys * strlen(prefix)) + 1;
        os_free(keyExpr);
        keyExpr = (char *)os_malloc(totalSize);
        keyExpr[0] = 0;
        fieldName = c_iterTakeFirst(keyExprNames);
        while (fieldName != NULL) {
            os_strcat(keyExpr,prefix);
            os_strcat(keyExpr,fieldName);
            os_free(fieldName);
            fieldName = c_iterTakeFirst(keyExprNames);
            if (fieldName != NULL) { os_strcat(keyExpr,","); }
        }
        c_iterFree(keyExprNames);
    }

    /* Initialize baseclass */
    dataView->reader = dataReader;
    dataView->instances = c_tableNew(dataViewInstanceType,keyExpr);
    dataView->sampleType = dataViewSampleType;
    dataView->instanceType = dataViewInstanceType;
    dataView->takenInstance = NULL;
    os_free(keyExpr);

    /* finally add myself to the dataReader views set */
    v_dataReaderInsertView(dataReader, dataView);
}

v_dataView
v_dataViewNew(
    v_dataReader dataReader,
    const c_char *name,
    v_dataViewQos qos,
    c_bool enable)
{
    v_kernel kernel;
    v_dataView dataView;

    assert(C_TYPECHECK(dataReader,v_dataReader));

    if (name == NULL) {
        name = "<No Name>";
    }

    kernel = v_objectKernel(dataReader);
    dataView = v_dataView(c_new(v_kernelType(kernel,K_DATAVIEW)));
    if (dataView) {
        v_object(dataView)->kernel = kernel;
        v_objectKind(dataView) = K_DATAVIEW;
        v_dataViewInit(dataView, dataReader, name, qos, enable);
    } else {
        OS_REPORT(OS_FATAL,
                  "v_dataViewNew",V_RESULT_INTERNAL_ERROR,
                  "Failed to create a v_dataReaderView.");
        assert(FALSE);
    }
    return dataView;
}


void
v_dataViewDeinit(
    v_dataView dataView)
{
    v_dataViewWipeSamples(dataView);
    if (dataView->takenInstance != NULL) {
        v_publicFree(v_public(dataView->takenInstance));
        dataView->takenInstance = NULL;
    }
    v_collectionDeinit(v_collection(dataView));
}


void
v_dataViewFree(
    v_dataView dataView)
{
    v_dataReader reader;

    assert(C_TYPECHECK(dataView,v_dataView));

    /* First remove myself from my parents views-set */
    reader = v_dataReader(dataView->reader);
    if (reader != NULL) {
        v_dataReaderRemoveView(reader, dataView);
        dataView->reader = NULL;
    }
    /* Free parent */
    v_entityFree(v_entity(dataView));
}

void
v_dataViewFreeUnsafe(
    v_dataView dataView)
{
    v_dataReader reader;

    assert(C_TYPECHECK(dataView,v_dataView));

    /* First remove myself from my parents views-set */
    reader = v_dataReader(dataView->reader);
    if (reader != NULL) {
        v_dataReaderRemoveViewUnsafe(reader, dataView);
    }
    /* Free parent */
    v_entityFree(v_entity(dataView));
}

void
v_dataViewWipeSamples(
    v_dataView dataView)
{
    v_dataViewInstance instance;
    assert(C_TYPECHECK(dataView,v_dataView));
    while ((instance = c_take(dataView->instances)) != NULL) {
        v_dataViewInstanceWipe(instance);
        v_publicFree(v_public(instance));
        c_free(instance);
    }
}

v_actionResult
v_dataViewWrite(
    v_dataView _this,
    v_readerSample sample)
{
    v_actionResult result = 0;
    v_dataViewInstance instance, found;

    assert(C_TYPECHECK(_this,v_dataView));

    if (!v_stateTest(v_nodeState(v_dataReaderSampleMessage(sample)),L_REGISTER)) {
        instance = v_dataViewInstanceNew(_this,sample);
        found = c_tableInsert(_this->instances,instance);
        if (found != instance) {
            v_dataViewInstanceWipe(instance);
            v_dataViewInstanceWrite(found,sample);
        }
        c_free(instance);
    }

    v_actionResultSet(result, V_PROCEED);
    return result;
}

/* -------------------------------- public ---------------------------------- */

v_dataReader
v_dataViewGetReader(
    v_dataView _this)
{
    assert(C_TYPECHECK(_this,v_dataView));

    return _this->reader;
}

static v_result
waitForData(
    v_dataView _this,
    os_duration *delay)
{
    v_result result = V_RESULT_OK;
    /* If no data read then wait for data or timeout.
     */
    if (*delay > 0) {
        c_ulong flags = 0;
        os_timeE time = os_timeEGet();
        v_observerLock(_this);
        v__observerSetEvent(v_observer(_this), V_EVENT_DATA_AVAILABLE);
        flags = v__observerTimedWait(v_observer(_this), *delay);
        v_observerUnlock(_this);
        if (flags & V_EVENT_TIMEOUT) {
            result = V_RESULT_TIMEOUT;
        } else {
            *delay -= os_timeEDiff(os_timeEGet(), time);
        }
    } else {
        result = V_RESULT_NO_DATA;
    }
    return result;
}

C_STRUCT(walkInstanceArg) {
    c_query query;
    v_readerSampleAction action;
    c_voidp arg;
    c_iter emptyList;
    c_long count;
};
C_CLASS(walkInstanceArg);

static v_actionResult
instanceSampleAction(
    c_object sample,
    c_voidp arg)
{
    walkInstanceArg a = (walkInstanceArg)arg;
    a->count++;
    return a->action(sample,a->arg);
}

static c_bool
instanceReadSamples(
    v_dataViewInstance instance,
    c_voidp arg)
{
    walkInstanceArg a = (walkInstanceArg)arg;

    return v_dataViewInstanceReadSamples(instance, a->query, V_MASK_ANY,
                                         instanceSampleAction, arg);
}

v_result
v_dataViewRead(
    v_dataView _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    C_STRUCT(walkInstanceArg) argument;

    assert(C_TYPECHECK(_this,v_dataView));

    v_dataViewLock(_this);
    if (v_readerSubscriber(_this->reader) == NULL) {
        v_dataViewUnlock(_this);
        return V_RESULT_ALREADY_DELETED;
    }
    result = v_subscriberTestBeginAccess(v_readerSubscriber(_this->reader));
    if (result == V_RESULT_OK) {
        argument.action = action;
        argument.arg = arg;
        argument.query = NULL;
        argument.emptyList = NULL;
        argument.count = 0;

        v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));
        while ((argument.count == 0) && (result == V_RESULT_OK))
        {
            (void)c_tableReadCircular(_this->instances,
                                      (c_action)instanceReadSamples,
                                      &argument);
            /* If no data read then wait for data or timeout.
             */
            if (argument.count == 0) {
                result = waitForData(_this, &timeout);
            }
        }
        action(NULL,arg); /* This triggers the action routine that the
                           * last sample is read. */
    }
    v_dataViewUnlock(_this);

    return result;
}

v_result
v_dataViewReadInstance(
    v_dataView _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    C_STRUCT(walkInstanceArg) argument;

    assert(C_TYPECHECK(_this,v_dataView));

    if (instance == NULL) {
        return V_RESULT_ILL_PARAM;
    }

    assert(C_TYPECHECK(instance, v_dataViewInstance));

    v_dataViewLock(_this);
    if (v_readerSubscriber(_this->reader) == NULL) {
        v_dataViewUnlock(_this);
        return V_RESULT_ALREADY_DELETED;
    }
    result = v_subscriberTestBeginAccess(v_readerSubscriber(_this->reader));
    if (result == V_RESULT_OK) {
        argument.action = action;
        argument.arg = arg;
        argument.query = NULL;
        argument.emptyList = NULL;
        argument.count = 0;

        v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));
        while ((argument.count == 0) && (result == V_RESULT_OK))
        {
            if (!v_dataViewInstanceEmpty(instance)) {
                (void)v_dataViewInstanceReadSamples(instance,NULL,V_MASK_ANY,instanceSampleAction,&argument);
            }
            /* If no data read then wait for data or timeout.
             */
            if (argument.count == 0) {
                result = waitForData(_this, &timeout);
            }
        }
        action(NULL,arg); /* This triggers the action routine that the
                           * last sample is read. */
    }
    v_dataViewUnlock(_this);

    return result;
}

v_result
v_dataViewReadNextInstance(
    v_dataView _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_dataViewInstance nextInstance;
    v_result result = V_RESULT_OK;
    c_bool proceed = TRUE;
    C_STRUCT(walkInstanceArg) argument;

    assert(C_TYPECHECK(_this,v_dataView));
    assert(C_TYPECHECK(instance, v_dataViewInstance));

    v_dataViewLock(_this);
    if (v_readerSubscriber(_this->reader) == NULL) {
        v_dataViewUnlock(_this);
        return V_RESULT_ALREADY_DELETED;
    }
    if (instance != NULL && ((v_dataView)v_instanceEntity(instance)) != _this) {
        v_dataViewUnlock(_this);
        return V_RESULT_PRECONDITION_NOT_MET;
    }
    result = v_subscriberTestBeginAccess(v_readerSubscriber(_this->reader));
    if (result == V_RESULT_OK) {
        v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));

        argument.action = action;
        argument.arg = arg;
        argument.query = NULL;
        argument.emptyList = NULL;
        argument.count = 0;

        while ((argument.count == 0) && (result == V_RESULT_OK))
        {
            nextInstance = v_dataViewInstance(c_tableNext(_this->instances,instance));
            while ((nextInstance != NULL) && (v_dataViewInstanceEmpty(nextInstance) || proceed)) {
                proceed = v_dataViewInstanceReadSamples(nextInstance,NULL,V_MASK_ANY,instanceSampleAction,&argument);
                nextInstance =  v_dataViewInstance(c_tableNext(_this->instances, nextInstance));
            }
            /* If no data read then wait for data or timeout.
             */
            if (argument.count == 0) {
                result = waitForData(_this, &timeout);
            }
        }

        action(NULL,arg); /* This triggers the action routine that the
                           * last sample is read. */
    }
    v_dataViewUnlock(_this);

    return result;
}

static c_bool
instanceTakeSamples(
    v_dataViewInstance instance,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    walkInstanceArg a = (walkInstanceArg)arg;

    proceed = v_dataViewInstanceTakeSamples(instance, a->query, V_MASK_ANY,
                                            instanceSampleAction, arg);
    if (v_dataViewInstanceEmpty(instance)) {
        a->emptyList = c_iterInsert(a->emptyList,instance);
    }
    return proceed;
}

v_result
v_dataViewTake(
    v_dataView _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    C_STRUCT(walkInstanceArg) argument;
    v_dataViewInstance emptyInstance,found;

    assert(C_TYPECHECK(_this,v_dataView));

    v_dataViewLock(_this);
    if (v_readerSubscriber(_this->reader) == NULL) {
        v_dataViewUnlock(_this);
        return V_RESULT_ALREADY_DELETED;
    }
    result = v_subscriberTestBeginAccess(v_readerSubscriber(_this->reader));
    if (result == V_RESULT_OK) {
        v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));

        argument.action = action;
        argument.arg = arg;
        argument.query = NULL;
        argument.emptyList = NULL;
        argument.count = 0;

        while ((argument.count == 0) && (result == V_RESULT_OK))
        {
            (void)c_tableReadCircular(_this->instances, (c_action)instanceTakeSamples, &argument);
            if (argument.emptyList != NULL) {
                emptyInstance = c_iterTakeFirst(argument.emptyList);
                while (emptyInstance != NULL) {
                    found = c_tableRemove(_this->instances,emptyInstance,NULL,NULL);
                    assert(found == emptyInstance);
                    v_publicFree(v_public(emptyInstance));
                    c_free(found);
                    emptyInstance = c_iterTakeFirst(argument.emptyList);
                }
                c_iterFree(argument.emptyList);
            }
            if (argument.count == 0) {
                result = waitForData(_this, &timeout);
            }
        }
        action(NULL,arg); /* This triggers the action routine that the last sample is read. */
    }
    v_dataViewUnlock(_this);

    return result;
}

v_result
v_dataViewTakeInstance(
    v_dataView _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    v_dataViewInstance found;
    C_STRUCT(walkInstanceArg) argument;

    assert(C_TYPECHECK(_this,v_dataView));

    if (instance == NULL) {
        return V_RESULT_ILL_PARAM;
    }

    assert(C_TYPECHECK(instance, v_dataViewInstance));

    v_dataViewLock(_this);
    if (v_readerSubscriber(_this->reader) == NULL) {
        v_dataViewUnlock(_this);
        return V_RESULT_ALREADY_DELETED;
    }
    result = v_subscriberTestBeginAccess(v_readerSubscriber(_this->reader));
    if (result == V_RESULT_OK) {
        v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));

        argument.action = action;
        argument.arg = arg;
        argument.query = NULL;
        argument.emptyList = NULL;
        argument.count = 0;

        while ((argument.count == 0) && (result == V_RESULT_OK))
        {
            (void)v_dataViewInstanceTakeSamples(instance,NULL,V_MASK_ANY,instanceSampleAction,&argument);
            if (v_dataViewInstanceEmpty(instance)) {
                found = c_tableRemove(_this->instances,instance,NULL,NULL);
                assert(found == instance);
                v_publicFree(v_public(instance));
                c_free(found);
            }
            if (argument.count == 0) {
                result = waitForData(_this, &timeout);
            }
        }
        action(NULL,arg); /* This triggers the action routine that the last sample is read. */
    }
    v_dataViewUnlock(_this);

    return result;
}

v_result
v_dataViewTakeNextInstance(
    v_dataView _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    v_dataViewInstance nextInstance, found;
    C_STRUCT(walkInstanceArg) argument;

    assert(C_TYPECHECK(_this,v_dataView));
    assert(C_TYPECHECK(instance,v_dataViewInstance ));

    v_dataViewLock(_this);
    if (v_readerSubscriber(_this->reader) == NULL) {
        v_dataViewUnlock(_this);
        return V_RESULT_ALREADY_DELETED;
    }
    if (instance != NULL && ((v_dataView)v_instanceEntity(instance)) != _this) {
        v_dataViewUnlock(_this);
        return V_RESULT_PRECONDITION_NOT_MET;
    }
    result = v_subscriberTestBeginAccess(v_readerSubscriber(_this->reader));
    if (result == V_RESULT_OK) {
        v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));

        argument.action = action;
        argument.arg = arg;
        argument.query = NULL;
        argument.emptyList = NULL;
        argument.count = 0;

        while ((argument.count == 0) && (result == V_RESULT_OK))
        {
            nextInstance = v_dataViewInstance(c_tableNext(_this->instances,instance));
            if (nextInstance != NULL) {

                assert(v_dataViewInstanceEmpty(nextInstance) == FALSE);

                (void)v_dataViewInstanceTakeSamples(nextInstance,NULL,V_MASK_ANY,instanceSampleAction,&argument);
                if (v_dataViewInstanceEmpty(nextInstance)) {
                    if (_this->takenInstance != NULL) {
                        v_publicFree(v_public(_this->takenInstance));
                        c_free(_this->takenInstance);
                    }
                    found = c_tableRemove(_this->instances,nextInstance,NULL,NULL);
                    assert(found == nextInstance);
                    OS_UNUSED_ARG(found);
                    _this->takenInstance = nextInstance; /* transfer refcount
                                                          * from c_tableRemove */
                }
            }
            if (argument.count == 0) {
                result = waitForData(_this, &timeout);
            }
        }
        action(NULL,arg); /* This triggers the action routine that the
                           * last sample is read. */
    }
    v_dataViewUnlock(_this);

    return result;
}

v_dataViewQos
v_dataViewGetQos(
    v_dataView _this)
{
    v_dataViewQos qos;

    assert(C_TYPECHECK(_this,v_dataView));

    v_dataViewLock(_this);
    qos = c_keep(_this->qos);
    v_dataViewUnlock(_this);

    return qos;
}

v_result
v_dataViewSetQos(
    v_dataView _this,
    v_dataViewQos qos)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataView));
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(qos);

    /* No need to call v_dataViewQosSet because it is currently not stored. */
    OS_REPORT(OS_ERROR, "v_dataViewSetQos", V_RESULT_PRECONDITION_NOT_MET,
              "Precondition not met: operation unsupported");
    return V_RESULT_IMMUTABLE_POLICY;
}


v_dataViewInstance
v_dataViewLookupInstance(
    v_dataView _this,
    v_message keyTemplate)
{
    v_dataViewInstance instance;
    C_STRUCT(v_dataReaderSampleTemplate) readerSampleTemplate;
    C_STRUCT(v_dataViewSampleTemplate) viewSampleTemplate;
    C_STRUCT(v_dataViewInstanceTemplate) instanceTemplate;

    assert(C_TYPECHECK(_this,v_dataView));
    assert(C_TYPECHECK(keyTemplate,v_message));

    v_dataViewLock(_this);
    readerSampleTemplate.message = keyTemplate;
    viewSampleTemplate.sample = (v_readerSample)(&readerSampleTemplate);
    instanceTemplate.sample = (v_dataViewSample)(&viewSampleTemplate);
    instance = c_find(_this->instances, &instanceTemplate);
    v_dataViewUnlock(_this);

    return instance;
}

c_bool
v_dataViewContainsInstance (
    v_dataView _this,
    v_dataViewInstance instance)
{
    c_bool found = FALSE;

    assert(C_TYPECHECK(_this,v_dataView));
    assert(C_TYPECHECK(instance,v_dataViewInstance));

    if (instance) {
        v_dataViewLock(_this);
        if ( ((v_dataView)v_instanceEntity(instance)) == _this) {
            found = TRUE;
        } else {
            OS_REPORT(OS_ERROR, "v_dataViewContainsInstance", V_RESULT_PRECONDITION_NOT_MET,
                "Invalid dataViewInstance: no attached to DataView"
                "<_this = 0x%"PA_PRIxADDR" instance = 0x%"PA_PRIxADDR">", (os_address)_this, (os_address)instance);
        }
        v_dataViewUnlock(_this);
    }
    return found;
}

static c_bool
queryNotifyDataAvailable(
    c_object query,
    c_voidp arg)
{
    v_query q = v_query(query);
    v_event event = (v_event)arg;

    event->source = v_observable(query);
    v_queryNotifyDataAvailable(q, event);

    return TRUE;
}

void
v_dataViewNotifyDataAvailable(
    v_dataView _this,
    v_dataViewSample sample)
{
    C_STRUCT(v_event) event;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataView));
    assert(C_TYPECHECK(sample,v_dataViewSample));

    event.kind = V_EVENT_DATA_AVAILABLE;
    event.source = NULL;
    event.data = sample;

    v_observerLock(v_observer(_this));
    c_setWalk(v_collection(_this)->queries, queryNotifyDataAvailable, &event);
    v_observerUnlock(v_observer(_this));

    /* Also notify myself, since the user reader might be waiting. */
    event.source = v_observable(_this);

    v_observableNotify(v_observable(_this), &event);
}
