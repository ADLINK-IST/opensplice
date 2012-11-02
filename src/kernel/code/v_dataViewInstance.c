/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "v_dataViewInstance.h"
#include "v_dataViewSample.h"
#include "v__dataView.h"
#include "v_state.h"
#include "v_instance.h"
#include "v_topic.h"
#include "v_time.h"
#include "v_public.h"
#define _EXTENT_
#ifdef _EXTENT_
#include "c_extent.h"
#endif

#include "os_report.h"

#ifdef EXTENDED_CHECKING
#define CHECK_INSTANCE(instance)  checkInstance(instance, TRUE)
#define CHECK_ZERO_INSTANCE(instance)  checkInstance(instance, FALSE)
#define REL_ASSERT(condition) \
    if (!(condition)) { \
        int _i, *_j; \
        _j = NULL; \
        _i = *_j; \
    }

static void
checkSample(
    v_dataViewSample sample)
{
    if (((v_dataViewInstance)(((v_readerSample)sample)->instance))->sampleCount != 0) {
        v_readerSample readerSample = (v_readerSample)(v_dataViewSampleTemplate(sample)->sample);
        v_dataViewSampleList sampleList = (v_dataViewSampleList)(sample);
        REL_ASSERT(c_refCount(readerSample) > 0);
        REL_ASSERT(C_TYPECHECK(readerSample, v_readerSample));
        REL_ASSERT(c_refCount(sampleList) > 0);
        if (sampleList->prev == NULL) {
            REL_ASSERT(readerSample->viewSamples == sample);
        } else {
            REL_ASSERT(readerSample->viewSamples != sample);
        }
    }
}

void
checkInstance(
    v_dataViewInstance instance,
    c_bool isNotEmpty)
{
    int i = 0;
    v_dataViewSample current;
    v_dataViewSample next;

    if (instance) {
        if (!isNotEmpty) {
            REL_ASSERT(instance->sampleCount == 0);
        }
        if (!v_reader(v_dataViewReader(instance->dataView))->qos->userKey.enable && isNotEmpty) {
            REL_ASSERT(instance->sampleCount == 1);
        }
        REL_ASSERT(c_refCount(instance) > 0);
        REL_ASSERT(C_TYPECHECK(instance, v_dataViewInstance));
        next = NULL;
        current = v_dataViewInstanceTemplate(instance)->sample;
        while (current != NULL) {
            if (!isNotEmpty) {
                checkSample(current);
            }
            REL_ASSERT(current->next == next);
            if (next != NULL) {
                REL_ASSERT(next->prev == current);
            }
            next = current;
            current = current->prev;
            i++;
        }
        if (next != NULL) {
            REL_ASSERT(next->prev == current);
        }
        if (isNotEmpty) {
            REL_ASSERT(i == instance->sampleCount);
        } else {
            REL_ASSERT(i == 1);
        }
    }
}
#else
#define CHECK_INSTANCE(instance)
#define CHECK_ZERO_INSTANCE(instance)
#endif


static void
v_readerSampleAddViewSample(
    v_readerSample sample,
    v_dataViewSample viewSample);

v_dataViewInstance
v_dataViewInstanceNew(
    v_dataView dataView,
    v_readerSample sample)
{
    v_dataViewInstance instance;
    v_dataViewSample viewSample;
    v_dataReader reader;
    v_readerQos qos;

    assert(dataView);
    assert(sample);
    assert(C_TYPECHECK(dataView,v_dataView));
    assert(C_TYPECHECK(sample,v_readerSample));

    reader = dataView->reader;
    qos = v_reader(reader)->qos;

#ifdef _EXTENT_
    instance = v_dataViewInstance(c_extentCreate(dataView->instanceExtent));
#else
    {
        c_type subtype;
        subtype = c_subType(dataView->instances);
        instance = v_dataViewInstance(c_new(subtype));
        c_free(subtype);
    }
#endif
    if (instance) {
        v_object(instance)->kernel = v_objectKernel(dataView);
        v_objectKind(instance) = K_DATAVIEWINSTANCE;
        instance->dataView = (c_voidp)dataView;

        viewSample = v_dataViewSampleNew(instance,sample);
        if (viewSample) {
            v_dataViewInstanceTemplate(instance)->sample = viewSample;
            viewSample->next = NULL;
            viewSample->prev = NULL;
            v_readerSampleAddViewSample(sample,viewSample);
            instance->sampleCount = 1;

            v_stateSet(instance->instanceState,L_NEW);
            v_stateClear(v_readerSample(viewSample)->sampleState,L_READ);

            assert(C_TYPECHECK(instance,v_dataViewInstance));
            v_dataViewNotifyDataAvailable(dataView, viewSample);
        }
        CHECK_INSTANCE(instance);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataViewInstanceNew",0,
                  "Failed to allocate v_dataViewInstancem");
        assert(FALSE);
    }

    return instance;
}

void
v_dataViewInstanceDeinit(
    v_dataViewInstance instance)
{
    assert(C_TYPECHECK(instance,v_dataViewInstance));

/*  v_instanceDeinit(v_instance(instance)); */
}

static void
v_readerSampleAddViewSample(
    v_readerSample sample,
    v_dataViewSample viewSample)
{
    v_dataViewSampleList listSample;

    listSample = v_dataViewSampleList(viewSample);
    listSample->next = sample->viewSamples;
    if (sample->viewSamples != NULL) {
        v_dataViewSampleList(sample->viewSamples)->prev = listSample;
    }
    listSample->prev = NULL;
    sample->viewSamples = listSample;
}

void
v_dataViewInstanceWipe(
    v_dataViewInstance instance)
{
    v_dataViewSample sample,prev,firstSample;

    assert(C_TYPECHECK(instance,v_dataViewInstance));

    if (instance == NULL) {
        return;
    }

    CHECK_INSTANCE(instance);

    sample = v_dataViewInstanceTemplate(instance)->sample;
    firstSample = c_keep(sample);
    while (sample != NULL) {
        v_dataViewSampleListRemove(v_dataViewSampleList(sample));
        prev = sample->prev;
        sample->prev = NULL;
        c_free(sample);
        sample = prev;
    }
    instance->sampleCount = 0;
    v_dataViewInstanceTemplate(instance)->sample = firstSample;
    CHECK_ZERO_INSTANCE(instance);

}

v_writeResult
v_dataViewInstanceWrite (
    v_dataViewInstance instance,
    v_readerSample sample)
{
    v_dataViewSample viewSample;
    v_dataViewSample *prev;
    v_dataViewSample next;

    assert(C_TYPECHECK(instance,v_dataViewInstance));
    assert(C_TYPECHECK(sample,v_readerSample));

    CHECK_INSTANCE(instance);
   
    viewSample = v_dataViewSampleNew(instance,sample);
    if (viewSample) {
        viewSample->next = NULL;

        prev = &v_dataViewInstanceTemplate(instance)->sample;
        next = NULL;

        while (*prev) {
            next = *prev;
            prev = &(*prev)->prev;
        }
        *prev = viewSample;
        viewSample->next = next;
    
        v_readerSampleAddViewSample(sample,viewSample);
        instance->sampleCount++;
        assert(c_refCount(viewSample) == 1);
        v_dataViewNotifyDataAvailable(v_dataView(instance->dataView), viewSample);
    }
    CHECK_INSTANCE(instance);
    return V_WRITE_SUCCESS;   
}

void
v_dataViewInstanceRemove(
    v_dataViewInstance instance)
{
    v_dataViewInstance found;

    assert(C_TYPECHECK(instance,v_dataViewInstance));


    if (instance->sampleCount == 0) {
        CHECK_ZERO_INSTANCE(instance);
        found = c_remove(v_dataView(instance->dataView)->instances,instance,NULL,NULL);
        assert(found == instance);
        instance->dataView  = NULL;
        v_publicFree(v_public(instance));
        c_free(instance);
    } else {
        CHECK_INSTANCE(instance);
    }
}

c_bool
v_dataViewInstanceTest(
    v_dataViewInstance instance,
    c_query query)
{
    v_dataViewSample sample, firstSample;
    c_bool sampleSatisfies = FALSE;

    assert(C_TYPECHECK(instance,v_dataViewInstance));

    CHECK_INSTANCE(instance);

    if (instance == NULL) {
        return FALSE;
    }
    if (query == NULL) {
        return TRUE;
    }
    if (instance->sampleCount == 0) {
        return TRUE;
    }

    firstSample = v_dataViewInstanceTemplate(instance)->sample;
    sample = firstSample;
    while ((sample != NULL) && (sampleSatisfies == FALSE)) {
        /* The history samples are swapped with the first sample to make
           sample-evaluation on instance level work.
        */
        if (sample != firstSample) {
            v_dataViewInstanceTemplate(instance)->sample = sample;
        }
        sampleSatisfies = c_queryEval(query,instance);
        if (sample != firstSample) {
            v_dataViewInstanceTemplate(instance)->sample = firstSample;
        }
        sample = sample->prev;
    }
    CHECK_INSTANCE(instance);
    return sampleSatisfies;
}

c_bool
v_dataViewInstanceReadSamples(
    v_dataViewInstance instance,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataViewSample sample, firstSample;
    c_bool proceed = TRUE;
    c_bool sampleSatisfies;

    assert(C_TYPECHECK(instance,v_dataViewInstance));
    CHECK_INSTANCE(instance);

    if (instance == NULL) {
        return proceed;
    }
    if (instance->sampleCount == 0) {
        return proceed;
    }
    firstSample = v_dataViewInstanceTemplate(instance)->sample;
    sample = firstSample;
    while ((sample != NULL) && (proceed == TRUE)) {
        if (query != NULL) {
            /* The history samples are swapped with the first sample to make
               sample-evaluation on instance level work.
            */
            if (sample != firstSample) {
                v_dataViewInstanceTemplate(instance)->sample = sample;
            }
            sampleSatisfies = c_queryEval(query,instance);
            if (sample != firstSample) {
                v_dataViewInstanceTemplate(instance)->sample = firstSample;
            }
        } else {
            sampleSatisfies = TRUE;
        }
        if (sampleSatisfies) {
            if (v_stateTest(instance->instanceState,L_NEW)) {
                v_stateSet(v_readerSample(sample)->sampleState,L_NEW);
            } else {
                v_stateClear(v_readerSample(sample)->sampleState,L_NEW);
            }
            if (v_stateTest(v_readerSample(sample)->sampleState,L_LAZYREAD)) {
                v_stateSet(v_readerSample(sample)->sampleState,L_READ);
                v_stateClear(v_readerSample(sample)->sampleState,L_LAZYREAD);
            }
            proceed = action(v_readerSample(sample),arg);
            v_stateClear(instance->instanceState,L_NEW);
            if (!v_stateTest(v_readerSample(sample)->sampleState,L_READ)) {
                v_stateSet(v_readerSample(sample)->sampleState,L_LAZYREAD);
            }
        }
        sample = sample->prev;
    }
    CHECK_INSTANCE(instance);
    return proceed;
}


void
v_dataViewInstanceWalkSamples(
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataViewSample sample;
    c_bool proceed = TRUE;

    assert(C_TYPECHECK(instance,v_dataViewInstance));
    CHECK_INSTANCE(instance);

    if (instance == NULL) {
        return;
    }
    if (instance->sampleCount == 0) {
        return;
    }
    sample = v_dataViewInstanceTemplate(instance)->sample;
    while ((sample != NULL) && (proceed == TRUE)) {
        proceed = action(v_readerSample(sample),arg);
        sample = sample->prev;
    }
    CHECK_INSTANCE(instance);
}


typedef c_bool (*v_sampleCondition)(v_dataViewSample sample, c_voidp arg);

static c_bool
v_dataViewInstanceTakeWithCondition(
    v_dataViewInstance instance,
    v_sampleCondition condition,
    c_voidp conditionArg,
    v_readerSampleAction action,
    c_voidp actionArg)
{
    v_dataViewSample sample, previous;
    c_bool proceed = TRUE;
    c_bool sampleSatisfies;

    assert(C_TYPECHECK(instance,v_dataViewInstance));
    CHECK_INSTANCE(instance);

    if (instance == NULL) {
        return proceed;
    }
    if (instance->sampleCount == 0) {
        return proceed;
    }
    sample = v_dataViewInstanceTemplate(instance)->sample;
    while ((proceed == TRUE) && (sample != NULL)) {
        if (condition != NULL) {
            sampleSatisfies = condition(sample, conditionArg);
        } else {
            sampleSatisfies = TRUE;
        }
        previous = sample->prev;
        if (sampleSatisfies) {
            if (v_stateTest(instance->instanceState,L_NEW)) {
                v_stateSet(v_readerSample(sample)->sampleState,L_NEW);
            } else {
                v_stateClear(v_readerSample(sample)->sampleState,L_NEW);
            }
            if (v_stateTest(v_readerSample(sample)->sampleState,L_LAZYREAD)) {
                v_stateSet(v_readerSample(sample)->sampleState,L_READ);
                v_stateClear(v_readerSample(sample)->sampleState,L_LAZYREAD);
            }

            proceed = action(v_readerSample(sample),actionArg);
            v_stateClear(instance->instanceState,L_NEW);

            v_dataViewSampleListRemove(v_dataViewSampleList(sample));
            v_dataViewSampleRemove(sample);
        }
        sample = previous;
    }

    if (instance->sampleCount > 0) {
        CHECK_INSTANCE(instance);
    } else {
        CHECK_ZERO_INSTANCE(instance);
    }
    return proceed;
}

typedef struct v_instanceQueryArg_s {
    c_query query;
    v_dataViewInstance instance;
} *v_instanceQueryArg;

static c_bool
evalInstanceQuery(
    v_dataViewSample sample,
    c_voidp arg)
{
    v_instanceQueryArg instanceQueryArg = (v_instanceQueryArg)arg;
    c_query query = instanceQueryArg->query;
    v_dataViewInstance instance = instanceQueryArg->instance;
    v_dataViewSample firstSample;
    c_bool result;
    
    assert(query != NULL);
    /* The history samples are swapped with the first sample to make
       sample-evaluation on instance level work. */
    firstSample = v_dataViewInstanceTemplate(instance)->sample;
    if (sample != firstSample) {
        v_dataViewInstanceTemplate(instance)->sample = sample;
    }
    result = c_queryEval(query,instance);
    if (sample != firstSample) {
        v_dataViewInstanceTemplate(instance)->sample = firstSample;
    }
    
    return result;
}

c_bool
v_dataViewInstanceTakeSamples(
    v_dataViewInstance instance,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;
    struct v_instanceQueryArg_s instanceQueryArg_s;

    assert(C_TYPECHECK(instance,v_dataViewInstance));
    /* No check, already done in TakeWithCondition */
    /* CHECK_INSTANCE(instance); */

    if (query != NULL) {

        instanceQueryArg_s.query = query;
        instanceQueryArg_s.instance = instance;
        proceed = v_dataViewInstanceTakeWithCondition(
            instance, evalInstanceQuery, &instanceQueryArg_s, action, arg);
    } else {
        proceed = v_dataViewInstanceTakeWithCondition(instance,NULL,NULL,action,arg);
    }

    /* No check, already done in TakeWithCondition */
    /* CHECK_INSTANCE(instance); */
    return proceed;
}

