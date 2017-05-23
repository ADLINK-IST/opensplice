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

#include "vortex_os.h"
#include "os_report.h"
#include "v_kernel.h"
#include "v_public.h"
#include "v_instance.h"
#include "v_state.h"
#include "v_dataViewInstance.h"
#include "v__dataReaderInstance.h"
#include "v__dataReaderSample.h"
#include "v__orderedInstance.h"


C_STRUCT(v__orderedInstanceGetDataReadersArgument) {
    v_sampleMask mask;
    c_iter samples;
};

C_CLASS(v__orderedInstanceGetDataReadersArgument);


#define v__orderedInstanceSampleDataReader(s)               \
    v_dataReader(                                           \
        v_dataReaderInstanceReader(                         \
            v_dataReaderSampleInstance(                     \
                v_dataViewSampleTemplate(s)->sample)))

#define v__orderedInstanceSampleDataReaderSample(s)         \
    v_dataReaderSample(                                     \
        v_dataViewSampleTemplate(s)->sample)

#define v__orderedInstanceSampleDataReaderSampleState(s)    \
    (v_readerSample(                                        \
         v_dataViewSampleTemplate(s)->sample)->sampleState)

static v_orderedInstanceSample
v__orderedInstanceSampleNew (
    v_orderedInstance instance,
    v_readerSample sample)
{
    v_orderedInstanceSample _this;

    assert(instance != NULL && C_TYPECHECK (instance, v_orderedInstance));
    if (sample != NULL) {
        /* Specified sample might be a dummy. */
        assert(C_TYPECHECK (sample, v_readerSample));
    }

    _this = (v_orderedInstanceSample)c_new (
        v_kernelType (v_objectKernel (instance), K_ORDEREDINSTANCESAMPLE));
    if (_this != NULL) {
        v_readerSample(_this)->instance = (c_voidp)instance;
        v_readerSample(_this)->sampleState = L_VALIDDATA;
        v_dataViewSampleList(_this)->next = NULL;
        v_dataViewSampleList(_this)->prev = NULL;
        ((v_dataViewSample)_this)->next = NULL;
        ((v_dataViewSample)_this)->prev = NULL;
        v_dataViewSampleTemplate(_this)->sample = c_keep(sample);
    } else {
        OS_REPORT(OS_FATAL, OS_FUNCTION ,V_RESULT_INTERNAL_ERROR,
            "Failed to allocate v_orderedInstanceSample");
        assert (_this != NULL);
    }

    return _this;
}

/* v_dataViewInstance objects are created only when a sample with the key
   value is available. Since a v_orderedInstance might be created by the
   subscriber (when a v_dataReader is created), the creation of a
   v_orderedViewInstance cannot be delayed until a sample is available. */
v_orderedInstance
v_orderedInstanceNew (
    v_entity entity, /* Subscriber or DataReader based on access_scope */
    v_presentationKind presentation,
    v_orderbyKind orderby)
{
    v_kernel kernel;
    v_orderedInstance _this;

    assert (entity != NULL);
#ifndef NDEBUG
    if (presentation == V_PRESENTATION_GROUP) {
        assert (C_TYPECHECK (entity, v_subscriber));
    } else {
        assert (C_TYPECHECK (entity, v_dataReader));
    }
#endif

    kernel = v_objectKernel (entity);
    _this = v_orderedInstance (v_objectNew (kernel, K_ORDEREDINSTANCE));
    if (_this != NULL) {
        _this->samples = NULL;
        _this->bookmark = NULL;
        _this->presentation = presentation;
        _this->orderby = orderby;
        _this->mask = V_MASK_ANY;
        _this->lazynew = c_listNew(v_kernelType(kernel, K_TRANSACTIONGROUP));
        v_instanceInit (v_instance(_this), entity);
		v_dataViewInstance(_this)->sampleCount = 0;
		v_dataViewInstanceTemplate(_this)->sample = NULL;
    } else {
        OS_REPORT (OS_FATAL, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
            "Failed to create v_orderedInstance");
        assert(_this != NULL);
    }

    return _this;
}

static void
v__orderedInstanceUpdateLazyNewInstances (
    v_orderedInstance _this)
{
    v_dataReaderInstance instance;

    while ((instance = v_dataReaderInstance(c_removeAt(_this->lazynew, 0)))) {
        v_dataReaderInstanceStateClear (instance, L_LAZYNEW);
        c_free (instance);
    }
}

static void
v__orderedInstanceReset (
    v_orderedInstance _this)
{
    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));

    v__orderedInstanceUpdateLazyNewInstances (_this);

    c_iterFree (_this->samples);

    _this->samples = NULL;
    _this->bookmark = v_dataViewInstanceTemplate(_this)->sample;
    _this->mask = V_MASK_ANY;
}

void
v_orderedInstanceReset (
    v_orderedInstance _this)
{
    if (_this != NULL) {
        assert (C_TYPECHECK (_this, v_orderedInstance));
        v__orderedInstanceReset (_this);
    }
}

void
v_orderedInstanceRemove (
    v_orderedInstance _this,
    v_entity entity)
{
    assert (entity != NULL);

    if (_this != NULL) {
        assert (C_TYPECHECK (_this, v_orderedInstance));

        /* Wipe and free v_orderedInstance if the entity that owns it invokes
           this function, otherwise only the reference count needs to be
           lowered, but that happens automatically. */
        if (v_instance(_this)->entity == entity) {
            assert (c_refCount (_this) > 0);
            v__orderedInstanceReset (_this);
            v_dataViewInstanceWipe (v_dataViewInstance(_this));
            v_publicFree (v_public(_this));
        } else {
            assert (c_refCount (_this) > 1);
            assert (C_TYPECHECK (entity, v_dataReader));
        }
    }
}

static v_actionResult
v__orderedInstanceGetDataReaders (
    c_object object,
    c_voidp data)
{
    v_dataReaderInstance instance;
    v_dataReaderSample sample;
    v__orderedInstanceGetDataReadersArgument argument =
        (v__orderedInstanceGetDataReadersArgument)data;

    sample = v__orderedInstanceSampleDataReaderSample (object);
    if ((v_sampleMaskTest (argument->mask, V_MASK_ANY_SAMPLE))          ||
        (v_sampleMaskTest (argument->mask, V_MASK_READ_SAMPLE) &&
             v_readerSampleTestStateOr (sample, L_READ | L_LAZYREAD))   ||
        (v_sampleMaskTest (argument->mask, V_MASK_NOT_READ_SAMPLE) &&
            !v_readerSampleTestStateOr (sample, L_READ | L_LAZYREAD)))
    {
        instance = v_dataReaderSampleInstance (sample);
        if (!v_readerSampleTestState(sample, L_VALIDDATA) &&
                (
                    v_reader(v_dataReaderInstanceReader (instance))->qos->lifecycle.v.enable_invalid_samples == FALSE ||
                    hasValidSampleAccessible(instance) ||
                    !v_dataReaderInstanceStateTest(instance, L_STATECHANGED) ||
                    v_readerSampleTestStateOr(sample, L_READ | L_LAZYREAD)
                )
            )
        {
            instance = NULL;
        }
        if (instance) {
            if (v_dataReaderInstanceMatchesSampleMask (instance, argument->mask)) {
                (void)c_iterAppend (argument->samples, object);
            }
        }
    }

    return V_PROCEED;
}

c_iter
v_orderedInstanceGetDataReaders (
    v_orderedInstance _this,
    v_sampleMask mask)
{
    c_iter list = NULL;
    v_actionResult proceed;
    C_STRUCT(v__orderedInstanceGetDataReadersArgument) argument;

    if (_this == NULL) {
        return NULL;
    }

    assert (C_TYPECHECK (_this, v_orderedInstance));
    /* v_orderedInstanceGetDataReaders is only invoked if, and only if,
     * presentation is V_PRESENTATION_GROUP.
     */
    assert (_this->presentation == V_PRESENTATION_GROUP);

    argument.mask = mask;
    argument.samples = c_iterNew (NULL);
    if (argument.samples != NULL) {
        c_iterIter iterator;
        v_dataReader reader;
        v_dataReaderInstance instance;
        v_dataReaderSample sample;
        v_orderedInstanceSample next;

        /* First complete list of samples for v_orderedInstance, then iterate
           over that list to generate the list of readers. Only destroy the
           (possibly) already created current list of samples to ensure the
           list that is already there remains valid even if creation of new
           list fails. */
        proceed = v_dataViewInstanceReadSamples (v_dataViewInstance (_this), NULL, V_MASK_ANY,
                                                 &v__orderedInstanceGetDataReaders,
                                                 (c_voidp)&argument);
        /* c_iterAppend returns the c_iter whether the operation was successful
           or not, so proceed must test positive for the V_PROCEED flag. */
        assert (v_actionResultTest (proceed, V_PROCEED));
        OS_UNUSED_ARG(proceed);
        iterator = c_iterIterGet (argument.samples);

        while ((next = c_iterNext (&iterator)) != NULL) {
            sample = v__orderedInstanceSampleDataReaderSample (next);
            instance = v_dataReaderSampleInstance (sample);
            reader = v_dataReaderInstanceReader (instance);
            list = c_iterAppend(list, c_keep(reader));
        }

        /* v_orderedInstanceGetDataReaders might have been invoked already.
         * Reset before creating a new list.
         */
        v__orderedInstanceReset (_this);
        _this->samples = argument.samples;
        _this->mask = mask;
    }
    return list;
}

static v_orderedInstanceSample
v__orderedInstanceWrite (
    v_orderedInstance _this,
    v_readerSample sample)
{
    v_dataViewSample head, next, previous, tail, position = NULL;
    v_message msgPrev, msgNext;

    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));
    assert (sample != NULL && C_TYPECHECK (sample, v_dataReaderSample));

    previous = v_dataViewSample (v__orderedInstanceSampleNew (_this, sample));

    if (v_dataViewInstance(_this)->sampleCount > 0) {
        head = v_dataViewInstanceTemplate(_this)->sample;
        if (head != NULL) {
            c_equality eq = C_NE;

            next = tail = head->next; /* head->next is tail */

            msgPrev = v_dataReaderSampleTemplate(v_dataReaderSample(v_dataViewSampleTemplate(previous)->sample))->message;
            do {
                msgNext = v_dataReaderSampleTemplate(v_dataReaderSample(v_dataViewSampleTemplate(next)->sample))->message;
                if (_this->orderby == V_ORDERBY_SOURCETIME) {
                    eq = v_messageCompare(msgPrev, msgNext);
                } else {
                    eq = v_messageCompareAllocTime(msgPrev, msgNext);
                }
                if (eq == C_LT) {
                    position = next;
                    next = next->next;
                }
            } while (eq == C_LT && next != tail);
        }
    }

    v__dataViewInstanceWrite (v_dataViewInstance (_this), previous, position);
    v_dataReaderSampleAddViewSample (sample, previous);

    return (v_orderedInstanceSample)previous;
}

v_writeResult
v_orderedInstanceWrite(
    v_orderedInstance _this,
    v_readerSample sample)
{
    v_dataViewSample current;

    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));
    assert (sample != NULL && C_TYPECHECK (sample, v_dataReaderSample));

    if (!v_stateTest (v_nodeState(v_dataReaderSampleMessage(sample)), L_REGISTER)) {
        current = v_dataViewSample (v__orderedInstanceWrite (_this, sample));
        assert (current != NULL);
        /* For instance and topic scopes updates are not blocked, because
           begin_access and end_access are optional. Samples older than last
           read sample should be read in a second pass. Samples that are newer
           must be offered to the user in the first pass though. The bookmark
           points to the sample that should be read next. If the newly inserted
           sample is inserted just before the bookmark, the bookmark must
           updated to point to the newly inserted sample. */
        if (_this->bookmark == current->prev) {
            _this->bookmark = (c_voidp)current;
        }
    }

    return V_WRITE_SUCCESS;
}

v_dataReaderSample
v_orderedInstanceFirstSample (
    v_orderedInstance _this)
{
    v_dataReaderSample sample = NULL;

    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));

    if (v_dataViewInstance (_this)->sampleCount > 0) {
        if (_this->presentation == V_PRESENTATION_GROUP) {
            if (c_iterLength(_this->samples) > 0) {
                sample = v__orderedInstanceSampleDataReaderSample (
                    c_iterObject (_this->samples, 0));
            }
        } else {
            assert(c_iterLength(_this->samples) == 0);
            sample = v__orderedInstanceSampleDataReaderSample (
                v_dataViewInstanceTemplate (_this)->sample);
        }
    }

    return sample;
}

static v_dataViewSample
v__orderedInstanceReadSample (
    v_orderedInstance _this)
{
    v_dataViewSample current;

    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));
    assert (_this->samples == NULL);

    /* Bookmark is the sample to read next (if the mask matches), because the
       the sample that is currently operated upon might be taken, which would
       then also invalidate the bookmark. */

    current = _this->bookmark;
    if (current != NULL) {
        _this->bookmark = current->prev;
    } else {
        v__orderedInstanceUpdateLazyNewInstances (_this);
        _this->bookmark = v_dataViewInstanceTemplate(_this)->sample;
    }

    return current;
}

static v_dataViewSample
v__orderedInstanceListReadSample(
    v_orderedInstance _this)
{
    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));
    assert (_this->samples != NULL);

    return v_dataViewSample (c_iterTakeFirst (_this->samples));
}

/* This function treats the sample list in the view as a circular buffer. Only
   one sample per invocation is read, which means that the piece of code that
   invokes this function should check if the sample that is returned was not
   read before. */
v_dataReaderSample
v_orderedInstanceReadSample (
    v_orderedInstance _this,
    v_sampleMask mask)
{
    v_dataViewSample next;
    v_dataReaderSample sample = NULL;

    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));

    /* Read must be circular, therefore v_dataViewInstanceReadSamples cannot
       be used. */

    if (v_dataViewInstance(_this)->sampleCount > 0) {
        if (_this->presentation == V_PRESENTATION_GROUP) {
            next = v__orderedInstanceListReadSample (_this);
        } else {
            /* Bookmark must be reset for topic and instance scopes, because
               using a different mask is interpreted as the start of a new
               read batch. */
            if (mask != _this->mask) {
                v__orderedInstanceReset (_this);
                _this->mask = mask;
            }
            next = v__orderedInstanceReadSample (_this);
        }

        if (next != NULL) {
            v_dataReaderInstance instance;

            sample = v__orderedInstanceSampleDataReaderSample (next);
            instance = v_dataReaderSampleInstance (sample);
            if (v_dataReaderInstanceStateTest (instance, L_NEW)) {
                v_dataReaderInstanceStateSet (instance, L_LAZYNEW);
                (void)c_listInsert(_this->lazynew, instance);
            }
        }
    }

    return sample;
}

#if 0
static void
v__orderedInstanceUnreadSample (
    v_orderedInstance _this,
    v_dataReaderSample sample)
{
    v_dataViewSample bookmark, next;

    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));
    assert (sample != NULL && C_TYPECHECK (sample, v_dataReaderSample));

    bookmark = _this->bookmark;
    if (bookmark != NULL) {
        /* Something is seriously broken if bookmark is the sample that was
           just operated upon. */
        assert (sample != v__orderedInstanceSampleDataReaderSample (bookmark));
    } else {
        bookmark = v_dataViewInstanceTemplate(_this)->sample;
    }

    /* Quickest mode of operation is to iterate the list in opposite
       read order. */
    for (next  = bookmark->next;
         next != bookmark &&
             v__orderedInstanceSampleDataReaderSample (next) != sample;
         next  = next->next)
    {
        /* do nothing */
    }

    /* Something is seriously broken if sample cannot be unread! */
    assert (v__orderedInstanceSampleDataReaderSample (next) == sample);
    _this->bookmark = next;
}

/* Rewinding the bookmark for group scope access is expensive and although it
   can easily be optimized, it is not worth the trouble because the code is
   never used in normal use cases (only one sample is read during a read
   operation). */
static void
v__orderedInstanceListUnreadSample (
    v_orderedInstance _this,
    v_dataReaderSample sample)
{
    v_dataViewSample previous;

    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));
    assert (sample != NULL && C_TYPECHECK (sample, v_dataReaderSample));

    previous = v_dataViewSample (v_dataViewInstanceTemplate (_this)->sample);
    /* First should never be a null pointer. */
    assert (previous != NULL);
    while (sample != v__orderedInstanceSampleDataReaderSample (previous)) {
        previous = previous->prev;
    }

    /* Something is seriously broken if sample is not in the list. */
    assert (previous != NULL);

    c_iterInsert (_this->samples, (c_voidp)previous);
}

void
v_orderedInstanceUnreadSample (
    v_orderedInstance _this,
    v_dataReaderSample sample)
{
    assert (_this != NULL && C_TYPECHECK (_this, v_orderedInstance));
    assert (sample != NULL && C_TYPECHECK (sample, v_dataReaderSample));
    assert (v_dataViewInstance(_this)->sampleCount > 0);

    if (_this->presentation == V_PRESENTATION_GROUP) {
        v__orderedInstanceListUnreadSample (_this, sample);
    } else {
        v__orderedInstanceUnreadSample (_this, sample);
    }
}
#endif

c_bool
v_orderedInstanceIsAligned (
    v_orderedInstance _this)
{
    c_bool aligned = FALSE;

    if (_this != NULL) {
        assert (C_TYPECHECK (_this, v_orderedInstance));
        if (_this->presentation == V_PRESENTATION_GROUP) {
            aligned = (_this->samples != NULL);
        } else {
            aligned = TRUE;
        }
    }

    return aligned;
}

void
v_orderedInstanceUnaligned (
    v_orderedInstance _this)
{
    if (_this != NULL) {
        assert (C_TYPECHECK (_this, v_orderedInstance));
        v__orderedInstanceReset (_this);
    }
}
