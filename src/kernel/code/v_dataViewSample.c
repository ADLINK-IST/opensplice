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


#include "v_dataViewSample.h"
#include "v_dataViewInstance.h"
#include "v_dataView.h"
#include "v__reader.h"
#include "v_time.h"
#include "v_state.h"
#include "os_report.h"

#define PRINT_REFCOUNT(functionName, sample)


#ifdef EXTENDED_CHECKING
void checkInstance(v_dataViewInstance instance, c_bool isNotEmpty);
#define CHECK_INSTANCE(instance) checkInstance(instance, TRUE)
#define CHECK_ZERO_INSTANCE(instance) checkInstance(instance, FALSE)
#else
#define CHECK_INSTANCE(instance)
#define CHECK_ZERO_INSTANCE(instance)
#endif


v_dataViewSample
v_dataViewSampleNew(
    v_dataViewInstance instance,
    v_readerSample masterSample)
{
    v_dataView dataView;
    v_dataViewSample sample;

    assert(instance != NULL);
    assert(masterSample != NULL);
    assert(C_TYPECHECK(masterSample,v_readerSample));

    dataView = v_dataView(instance->dataView);
    sample = v_dataViewSample(c_new(dataView->sampleType));
    if (sample) {
        v_readerSample(sample)->instance = (c_voidp)instance;
        v_readerSample(sample)->sampleState = L_VALIDDATA;
        v_dataViewSampleList(sample)->next = NULL;
        v_dataViewSampleList(sample)->prev = NULL;
        sample->prev = NULL;
        v_dataViewSampleTemplate(sample)->sample = c_keep(masterSample);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataViewSampleNew",0,
                  "Failed to allocate v_dataViewSample object.");
        assert(FALSE);
    }
    return sample;
}


void
v_dataViewSampleFree(
    v_dataViewSample sample)
{
    OS_UNUSED_ARG(sample);
    assert(sample != NULL);
    assert(C_TYPECHECK(sample, v_dataViewSample));

 PRINT_REFCOUNT(v_dataViewSampleFree, sample);
    /* Free the slave-samples as well */

 PRINT_REFCOUNT(v_dataViewSampleFree, sample);
}

void
v_dataViewSampleRemove(
    v_dataViewSample sample)
{
    v_dataViewInstance instance;

    assert(C_TYPECHECK(sample,v_dataViewSample));

    instance = v_dataViewInstance(v_readerSample(sample)->instance);
    CHECK_INSTANCE(instance);

    if (instance->sampleCount > 1) {
        if (sample->next != NULL) {
            v_dataViewSample(sample->next)->prev = sample->prev;
        } else {
            v_dataViewInstanceTemplate(instance)->sample = sample->prev;
        }
        if (sample->prev != NULL) {
            v_dataViewSample(sample->prev)->next = sample->next;
        }
        sample->prev = NULL;
        sample->next = NULL;
        v_readerSampleSetState(sample, L_REMOVED);
        c_free(sample);
    } else {
        v_readerSampleSetState(sample, L_REMOVED);
    }
    instance->sampleCount--;
    if (instance->sampleCount > 0) {
        CHECK_INSTANCE(instance);
    } else {
        CHECK_ZERO_INSTANCE(instance);
    }
}

void
v_dataViewSampleListRemove(
    v_dataViewSampleList sample)
{
    assert(C_TYPECHECK(sample,v_dataViewSampleList));
    assert(v_dataViewInstance(v_readerSample(sample)->instance)->sampleCount > 0);
    CHECK_INSTANCE(v_dataViewInstance(v_readerSample(sample)->instance));

    if (sample->next != NULL) {
        v_dataViewSampleList(sample->next)->prev = sample->prev;
    }
    if (sample->prev != NULL) {
        v_dataViewSampleList(sample->prev)->next = sample->next;
    } else {
        assert(v_dataViewSampleTemplate(sample)->sample->viewSamples == sample);
        v_dataViewSampleTemplate(sample)->sample->viewSamples = sample->next;
    }
    sample->prev = NULL;
    sample->next = NULL;
    CHECK_INSTANCE(v_dataViewInstance(v_readerSample(sample)->instance));
}

