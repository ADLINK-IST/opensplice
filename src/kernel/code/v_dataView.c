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
/* Interface */
#include "v__dataView.h"

/* Implementation */
#include "os.h"
#include "os_report.h"

#include "c_stringSupport.h"

#include "v__entity.h"
#include "v__dataReader.h"
#include "v__query.h"
#include "v_dataViewInstance.h"
#include "v_readerQos.h"
#include "v_dataViewQos.h"
#include "v_collection.h"
#include "v_qos.h"
#include "v_public.h"
#include "v_topic.h"
#include "v__topic.h"
#include "v__index.h"
#include "v_observer.h"
#include "v_observable.h"
#include "v_time.h"
#include "v_state.h"
#include "v__collection.h"

/* ------------------------------- private ---------------------------------- */

static c_type
dataViewSampleTypeNew(
    v_dataReader dataReader)
{
    c_metaObject o;
    c_type sampleType,foundType,readerSampleType;
    c_base base;
    c_char *name;
    c_long length,sres;
    c_char *metaName;
    v_kernel kernel;

    assert(C_TYPECHECK(dataReader,v_dataReader));
    assert(dataReader);

    kernel = v_objectKernel(dataReader);

    base = c_getBase(dataReader);

    if (base == NULL) {
        OS_REPORT(OS_ERROR,
                  "v_dataView::dataViewSampleTypeNew",0,
                  "failed to retrieve base");
        return NULL;
    }

    readerSampleType = v_dataReaderSampleType(dataReader);
    if (readerSampleType == NULL) {
        OS_REPORT(OS_ERROR,
                  "v_dataView::dataViewSampleTypeNew",0,
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
                assert(sres == (length-1));
#undef SAMPLE_FORMAT
#undef SAMPLE_NAME

                foundType = c_type(c_metaBind(c_metaObject(base),name,
                                              c_metaObject(sampleType)));
                os_free(name);
                c_free(o);
            } else {
                OS_REPORT(OS_ERROR,
                          "v_dataView::dataViewSampleTypeNew",0,
                          "failed to declare new sample type sample attribute");
            }
            c_free(sampleType);
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataView::dataViewSampleTypeNew",0,
                      "failed to define new sample type");
        }
        c_free(metaName);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataView::dataViewSampleTypeNew",0,
                  "failed to retrieve sample type name");
    }

    c_free(readerSampleType);

    return foundType;
}

static c_type
dataViewInstanceTypeNew(
    v_kernel kernel,
    c_type viewSampleType)
{
    c_metaObject o;
    c_type instanceType, foundType;
    c_base base;
    c_char *name;
    c_long length,sres;
    c_char *metaName;

    assert(C_TYPECHECK(viewSampleType,c_type));
    assert(viewSampleType);

    base = c_getBase(viewSampleType);

    if (base == NULL) {
        OS_REPORT(OS_ERROR,
                  "v_dataView::dataViewInstanceTypeNew",0,
                  "failed to retrieve base");
        return NULL;
    }

    metaName = c_metaName(c_metaObject(viewSampleType));
    if (metaName == NULL) {
        OS_REPORT(OS_ERROR,
                  "v_dataView::dataViewInstanceTypeNew",0,
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
            assert(sres == (length-1));
#undef INSTANCE_NAME
#undef INSTANCE_FORMAT

            foundType = c_type(c_metaBind(c_metaObject(base),name,
                                          c_metaObject(instanceType)));

            os_free(name);
            c_free(o);
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataView::dataViewInstanceTypeNew",0,
                      "failed to declare sampleType->sample attribute");
        }
        c_free(instanceType);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataView::dataViewInstanceTypeNew",0,
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
    c_long nrOfKeys,totalSize;
    c_char *keyExpr, *prefix;
    v_dataViewQos q;
    v_kernel kernel;

    assert(C_TYPECHECK(dataView,v_dataView));
    assert(C_TYPECHECK(dataReader,v_dataReader));

    kernel = v_objectKernel(dataReader);
    q = v_dataViewQosNew(kernel, qos);
    dataView->qos = q;

    v_collectionInit(v_collection(dataView),name, NULL, enable);

    dataViewSampleType = dataViewSampleTypeNew(dataReader);
    assert(dataViewSampleType != NULL);
    dataViewInstanceType = dataViewInstanceTypeNew(kernel, dataViewSampleType);
    assert(dataViewInstanceType != NULL);
    /* When the view has defined its own keys, then grab the key-definition from
     * the userKeyQosPolicy and the key-values from the messages that are passed.
     */
    if (qos->userKey.enable)
    {
        if (qos->userKey.expression)
        {
            totalSize = strlen(qos->userKey.expression) + 1;
            keyExpr = os_malloc(totalSize);
            os_strncpy(keyExpr, qos->userKey.expression, totalSize);
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
    else if (v_reader(dataReader)->qos->userKey.enable)
    {
        if (v_reader(dataReader)->qos->userKey.expression)
        {
            totalSize = strlen(v_reader(dataReader)->qos->userKey.expression) + 1;
            keyExpr = os_malloc(totalSize);
            os_strncpy(keyExpr, v_reader(dataReader)->qos->userKey.expression, totalSize);
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

    kernel = v_objectKernel(dataReader);
    dataView = v_dataView(c_new(v_kernelType(kernel,K_DATAVIEW)));
    if (dataView) {
        v_object(dataView)->kernel = kernel;
        v_objectKind(dataView) = K_DATAVIEW;
        v_dataViewInit(dataView, dataReader, name, qos, enable);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataViewNew",0,
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
        } else {
            v_publicInit(v_public(instance));
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

C_STRUCT(walkInstanceArg) {
    c_query query;
    v_readerSampleAction action;
    c_voidp arg;
    c_iter emptyList;
    c_time time;
};
C_CLASS(walkInstanceArg);

static c_bool
instanceReadSamples(
    v_dataViewInstance instance,
    c_voidp arg)
{
    walkInstanceArg a = (walkInstanceArg)arg;

    return v_dataViewInstanceReadSamples(instance, a->query,
                                         a->action, a->arg);
}

c_bool
v_dataViewRead(
    v_dataView _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;
    C_STRUCT(walkInstanceArg) argument;

    assert(C_TYPECHECK(_this,v_dataView));

    argument.action = action;
    argument.arg = arg;
    argument.query = NULL;
    argument.emptyList = NULL;
    argument.time = v_timeGet();

    v_dataViewLock(_this);
    v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));
    proceed = c_tableReadCircular(_this->instances,
                           (c_action)instanceReadSamples,
                           &argument);
    action(NULL,arg); /* This triggers the action routine that the
                       * last sample is read. */
    v_dataViewUnlock(_this);

    return proceed;
}

c_bool
v_dataViewReadInstance(
    v_dataView _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;

    assert(C_TYPECHECK(_this,v_dataView));

    if (instance == NULL) {
        return FALSE;
    }

    assert(C_TYPECHECK(instance, v_dataViewInstance));

    v_dataViewLock(_this);
    v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));
    if (!v_dataViewInstanceEmpty(instance)) {
        proceed = v_dataViewInstanceReadSamples(instance,NULL,action,arg);
    } else {
        proceed = FALSE;
    }
    action(NULL,arg); /* This triggers the action routine that the
                       * last sample is read. */
    v_dataViewUnlock(_this);

    return proceed;
}

c_bool
v_dataViewReadNextInstance(
    v_dataView _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataViewInstance nextInstance;
    c_bool proceed = TRUE;

    assert(C_TYPECHECK(_this,v_dataView));
    assert(C_TYPECHECK(instance, v_dataViewInstance));

    v_dataViewLock(_this);
    v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));

    nextInstance = v_dataViewInstance(c_tableNext(_this->instances,instance));
    while ((nextInstance != NULL) && v_dataViewInstanceEmpty(nextInstance)) {
        nextInstance =  v_dataViewInstance(c_tableNext(_this->instances,
                                                       nextInstance));
    }
    if (nextInstance != NULL) {
        proceed = v_dataViewInstanceReadSamples(nextInstance,NULL,action,arg);
    }

    action(NULL,arg); /* This triggers the action routine that the
                       * last sample is read. */
    v_dataViewUnlock(_this);

    return proceed;
}

static c_bool
instanceTakeSamples(
    v_dataViewInstance instance,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    walkInstanceArg a = (walkInstanceArg)arg;

    proceed = v_dataViewInstanceTakeSamples(instance, a->query,
                                            a->action, a->arg);
    if (v_dataViewInstanceEmpty(instance)) {
        a->emptyList = c_iterInsert(a->emptyList,instance);
    }
    return proceed;
}

c_bool
v_dataViewTake(
    v_dataView _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;
    C_STRUCT(walkInstanceArg) argument;
    v_dataViewInstance emptyInstance,found;

    assert(C_TYPECHECK(_this,v_dataView));

    argument.action = action;
    argument.arg = arg;
    argument.query = NULL;
    argument.emptyList = NULL;
    argument.time = v_timeGet();

    v_dataViewLock(_this);
    v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));

    proceed = c_tableReadCircular(_this->instances,
                           (c_action)instanceTakeSamples,
                           &argument);
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
    action(NULL,arg); /* This triggers the action routine that the
                       * last sample is read. */
    v_dataViewUnlock(_this);

    return proceed;
}

c_bool
v_dataViewTakeInstance(
    v_dataView _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataViewInstance found;
    c_bool proceed = FALSE;

    assert(C_TYPECHECK(_this,v_dataView));

    if (instance == NULL) {
        return FALSE;
    }

    assert(C_TYPECHECK(instance, v_dataViewInstance));

    v_dataViewLock(_this);
    v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));

    proceed = v_dataViewInstanceTakeSamples(instance,NULL,action,arg);
    if (v_dataViewInstanceEmpty(instance)) {
        found = c_tableRemove(_this->instances,instance,NULL,NULL);
        assert(found == instance);
        v_publicFree(v_public(instance));
        c_free(found);
    }
    action(NULL,arg); /* This triggers the action routine that the
                       * last sample is read. */
    v_dataViewUnlock(_this);

    return proceed;
}

c_bool
v_dataViewTakeNextInstance(
    v_dataView _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataViewInstance nextInstance, found;
    c_bool proceed = FALSE;

    assert(C_TYPECHECK(_this,v_dataView));
    assert(C_TYPECHECK(instance,v_dataViewInstance ));

    v_dataViewLock(_this);
    v_dataReaderUpdatePurgeLists(v_dataReader(_this->reader));

    nextInstance = v_dataViewInstance(c_tableNext(_this->instances,instance));
    if (nextInstance != NULL) {

        assert(v_dataViewInstanceEmpty(nextInstance) == FALSE);

        proceed = v_dataViewInstanceTakeSamples(nextInstance,NULL,action,arg);
        if (v_dataViewInstanceEmpty(nextInstance)) {
            if (_this->takenInstance != NULL) {
                v_publicFree(v_public(_this->takenInstance));
                c_free(_this->takenInstance);
            }
            found = c_tableRemove(_this->instances,nextInstance,NULL,NULL);
            assert(found == nextInstance);
            _this->takenInstance = nextInstance; /* transfer refcount
                                                  * from c_tableRemove */
        }
    }
    action(NULL,arg); /* This triggers the action routine that the
                       * last sample is read. */
    v_dataViewUnlock(_this);

    return proceed;
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

    v_dataViewLock(_this);
    if ( (v_dataView)(instance->dataView) == _this) {
        found = TRUE;
    }
    v_dataViewUnlock(_this);

    return found;
}

static c_bool
queryNotifyDataAvailable(
    c_object query,
    c_voidp arg)
{
    v_query q = v_query(query);
    v_event event = (v_event)arg;

    event->source = v_publicHandle(v_public(query));
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

    event.kind     = V_EVENT_DATA_AVAILABLE;
    event.source   = V_HANDLE_NIL;
    event.userData = sample;

    v_observerLock(v_observer(_this));
    c_setWalk(v_collection(_this)->queries, queryNotifyDataAvailable, &event);
    v_observerUnlock(v_observer(_this));


    /* Also notify myself, since the user reader might be waiting. */

    event.source = v_publicHandle(v_public(_this));
    v_observerNotify(v_observer(_this), &event, NULL);
    v_observableNotify(v_observable(_this), &event);
}
