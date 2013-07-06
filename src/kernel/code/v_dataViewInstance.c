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
#include "v_dataViewInstance.h"
#include "v_dataViewSample.h"
#include "v__dataView.h"
#include "v_state.h"
#include "v_instance.h"
#include "v_topic.h"
#include "v_time.h"
#include "v_public.h"

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

    assert(dataView);
    assert(sample);
    assert(C_TYPECHECK(dataView,v_dataView));
    assert(C_TYPECHECK(sample,v_readerSample));

    instance = v_dataViewInstance(c_new(dataView->instanceType));
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
    OS_UNUSED_ARG(instance);
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
    c_query query,
    v_queryAction action,
    c_voidp args)
{
    v_dataViewSample sample, firstSample;
    c_bool sampleSatisfies = FALSE;

    assert(C_TYPECHECK(instance,v_dataViewInstance));

    CHECK_INSTANCE(instance);

    if (instance == NULL) {
        return FALSE;
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
        if (query) {
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
        /* If a sample passed the query, then check whether it matches the
         * optional condition passed by the action routine. (This condition
         * can for example check for matching lifecycle states.)
         */
        if (sampleSatisfies && action != NULL) {
            sampleSatisfies = action(sample, args);
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
    v_actionResult result = V_PROCEED;
    c_bool sampleSatisfies;

    assert(C_TYPECHECK(instance,v_dataViewInstance));
    CHECK_INSTANCE(instance);

    if (instance == NULL) {
        return v_actionResultTest(result, V_PROCEED);
    }
    if (instance->sampleCount == 0) {
        return v_actionResultTest(result, V_PROCEED);
    }
    firstSample = v_dataViewInstanceTemplate(instance)->sample;
    sample = firstSample;
    while ((sample != NULL) && v_actionResultTest(result, V_PROCEED)) {
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
            result = v_dataViewSampleReadTake(sample, action, arg, FALSE);
        }
        sample = sample->prev;
    }
    CHECK_INSTANCE(instance);
    return v_actionResultTest(result, V_PROCEED);
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
            proceed = v_actionResultTest(v_dataViewSampleReadTake(sample, action, actionArg, TRUE), V_PROCEED);
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

v_actionResult
v_dataViewSampleReadTake(
    v_dataViewSample sample,
    v_readerSampleAction action,
    c_voidp arg,
    c_bool consume)
{
    v_dataViewInstance instance;
    v_state state;
    v_state mask;
    v_actionResult result = 0;

    instance = v_dataViewSampleInstance(sample);

    state = v_dataViewInstanceState(instance);
    mask = L_NEW | L_DISPOSED | L_NOWRITERS;


    /* Copy the value of instance state bits specified by the mask
     * to the sample state bits without affecting other bits.
     */
    v_readerSampleSetState(sample,(state & mask));
    v_readerSampleClearState(sample,(~state & mask));

    /* If the status of the sample is READ by the previous read
     * operation and the flag is not yet set (specified by the
     * LAZYREAD flag) then correct the state before executing the
     * read action.
     */
    if (v_readerSampleTestState(sample,L_LAZYREAD))
    {
        v_readerSampleSetState(sample,L_READ);
        v_readerSampleClearState(sample,L_LAZYREAD);
    }


    /* An action routine is provided in case the sample needs to be returned
     * to the user. If an action routine is not provided, it means the sample
     * needs to be removed from the administration, so the reader should be
     * modified accordingly. That means the 'proceed' flag should be set in
     * that case.
     */
    V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample),readerReadTime);
    if (action)
    {
        /* Invoke the action routine with the typed sample. */
        result = action(v_readerSample(sample), arg);
    }
    else
    {
        v_actionResultSet(result, V_PROCEED);
    }

    /* A sample is considered 'skipped' if the action routine invoked above
     * does not want to keep track of the sample (for example because it
     * didn't match its readerMasks). In that case, it sets the 'skip' flag
     * to true, which indicates that those samples should be considered
     * 'untouched' and therefore their instance and sample states should
     * not be modified.
     */
    if (v_actionResultTestNot(result, V_SKIP))
    {
        V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample),readerCopyTime);
        V_MESSAGE_REPORT(v_dataReaderSampleMessage(sample),
                         v_dataReaderInstanceDataReader(instance));

        v_stateClear(instance->instanceState,L_NEW);
        if (!v_stateTest(v_readerSample(sample)->sampleState,L_READ)) {
            v_stateSet(v_readerSample(sample)->sampleState,L_LAZYREAD);
        }
        if (consume) {
            v_dataViewSampleListRemove(v_dataViewSampleList(sample));
            v_dataViewSampleRemove(sample);
        }
    }
    return result;
}

