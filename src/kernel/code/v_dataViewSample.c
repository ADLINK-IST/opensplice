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


#include "v_dataViewSample.h"
#include "v_dataViewInstance.h"
#include "v__orderedInstance.h"
#include "v_dataView.h"
#include "v__reader.h"
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

    dataView = v_dataView(v_instanceEntity(instance));
    sample = v_dataViewSample(c_new(dataView->sampleType));
    if (sample) {
        v_readerSample(sample)->instance = (c_voidp)instance;
        v_readerSample(sample)->sampleState = L_VALIDDATA;
        v_dataViewSampleList(sample)->next = NULL;
        v_dataViewSampleList(sample)->prev = NULL;
        sample->next = NULL;
        sample->prev = NULL;
        v_dataViewSampleTemplate(sample)->sample = c_keep(masterSample);
    } else {
        OS_REPORT(OS_FATAL, OS_FUNCTION ,V_RESULT_INTERNAL_ERROR,
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
    v_dataViewSample head;

    assert(C_TYPECHECK(sample,v_dataViewSample));

    instance = v_dataViewInstance(v_readerSample(sample)->instance);
    CHECK_INSTANCE(instance);

    /* sampleCount is a signed integer, hence the greater than */
    assert (instance->sampleCount > 0);

    v_readerSampleSetState(sample, L_REMOVED);

    instance->sampleCount--;
    if (instance->sampleCount == 0) {
        CHECK_ZERO_INSTANCE(instance);
        assert (sample->next == sample);
        assert (sample->prev == NULL);

        if (v_objectKind (instance) == K_ORDEREDINSTANCE) {
            /* Set v_orderedInstance bookmark to NULL by default if "no"
               examples exist. */
            v_orderedInstance(instance)->bookmark = NULL;
            v_dataViewInstanceTemplate(instance)->sample = NULL;
            c_free(sample);
            CHECK_INSTANCE(instance);
        }
    } else {
        assert (sample->next != NULL);
        assert (sample->next != sample);

        if (sample->prev != NULL) {
            v_dataViewSample(sample->prev)->next = sample->next;
        }

        head = v_dataViewInstanceTemplate(instance)->sample;
        if (head == sample) {
            /* Upon removing the head of the list the previous pointer of the
             * next sample should never be set, because that would set the
             * previous pointer of the tail sample and thus cause undefined
             * behavior. */
            v_dataViewInstanceTemplate(instance)->sample = sample->prev;
        } else {
            v_dataViewSample(sample->next)->prev = sample->prev;
            if (head->next == sample) {
                head->next = sample->next;
            }
        }

        if (v_objectKind (instance) == K_ORDEREDINSTANCE &&
            v_orderedInstance (instance)->bookmark == sample)
        {
            /* Bookmark should not need to be updated on a take operation, it
               might need to be updated when samples are purged for example. */
            v_orderedInstance (instance)->bookmark = sample->prev;
        }

        sample->prev = NULL;
        sample->next = NULL;
        c_free(sample);
        CHECK_INSTANCE(instance);
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

