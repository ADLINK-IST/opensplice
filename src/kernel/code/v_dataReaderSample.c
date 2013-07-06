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
#include "v__dataReaderSample.h"
#include "v_dataReaderEntry.h"
#include "v__dataReaderInstance.h"
#include "v_dataViewInstance.h"
#include "v_dataViewSample.h"
#include "v__messageQos.h"
#include "v__reader.h"
#include "v_time.h"
#include "v_state.h"
#include "v_index.h"
#include "v_public.h"
#include "v__lifespanAdmin.h"

v_dataReaderSample
v_dataReaderSampleNew(
    v_dataReaderInstance instance,
    v_message message)
{
    v_dataReader dataReader;
    v_dataReaderSample sample;
    v_readerQos readerQos;
    v_index index;
    c_time insertTime;

    assert(instance != NULL);
    assert(C_TYPECHECK(message,v_message));

    index = v_index(instance->index);
    dataReader = v_dataReader(index->reader);
    readerQos = v_reader(dataReader)->qos;
    assert(readerQos);
    if (dataReader->cachedSample != NULL) {
        sample = dataReader->cachedSample;

/* Unfinished proto for cache size control
 * #define _SL_
 */
#ifdef _SL_
        dataReader->cachedSample = sample->newer;
        sample->newer = NULL;
        dataReader->cachedSampleCount--;
#else
        dataReader->cachedSample = NULL;
#endif
    } else {
        sample = v_dataReaderSample(c_new(dataReader->sampleType));
    }
    v_readerSample(sample)->instance = (c_voidp)instance;
    v_readerSample(sample)->viewSamples = NULL;
    if ((message->transactionId != 0) &&
        (v_reader(dataReader)->subQos->presentation.coherent_access))
    {
        v_readerSample(sample)->sampleState = L_TRANSACTION;
    } else {
        v_readerSample(sample)->sampleState = 0;
    }
#ifdef _NAT_
    sample->insertTime = v_timeGet();
#else
#define _INCORRECT_BUT_LOW_INSERT_LATENCY_
#ifdef _INCORRECT_BUT_LOW_INSERT_LATENCY_
    if (v_timeIsZero(v_reader(dataReader)->qos->latency.duration)) {
        sample->insertTime = v_timeGet();
    } else {
        sample->insertTime = message->allocTime;
    }
#else
    sample->insertTime = v_timeGet();
#endif
#endif

    /* The expiry time calculation is dependent on the DestinationOrderQos(readerQos->orderby.kind):
     * In case of the by_reception_timestamp kind the expiry time is determined based on insertion time(sample->insertTime).
     * In case of the by_source_timestamp kind the expiry time is determined based on source time (message->writeTime).
     * see OSPL-871
     */

    if (readerQos->orderby.kind == V_ORDERBY_SOURCETIME) {
       insertTime = message->writeTime;
    } else {
        insertTime = sample->insertTime;
    }

     /* When both ReaderLifespanQos(readerQos->lifespan.used) and the inline LifespanQos (v_messageQos_getLifespanPeriod(message->qos))
      * are set the expiryTime will be set to the earliest time among them.
      */
    if (readerQos->lifespan.used && message->qos != NULL) {
        if (c_timeCompare(readerQos->lifespan.duration, v_messageQos_getLifespanPeriod(message->qos)) == C_LT) {
            v_lifespanSample(sample)->expiryTime =
                        c_timeAdd(insertTime, readerQos->lifespan.duration);
        } else {
            v_lifespanSample(sample)->expiryTime =
                        c_timeAdd(insertTime,
                                  v_messageQos_getLifespanPeriod(message->qos));
        }
    } else if (readerQos->lifespan.used){
        v_lifespanSample(sample)->expiryTime =
            c_timeAdd(insertTime,readerQos->lifespan.duration);
    } else if (message->qos != NULL){
        v_lifespanSample(sample)->expiryTime =
            c_timeAdd(insertTime,
                      v_messageQos_getLifespanPeriod(message->qos));
    } else {
        v_lifespanSample(sample)->expiryTime = c_timeAdd(insertTime, C_TIME_INFINITE);
    }
    sample->disposeCount = instance->disposeCount;
    sample->noWritersCount = instance->noWritersCount;
    sample->publicationHandle = message->writerGID;
    sample->readId = 0;
    sample->newer = NULL;
    assert(message);
    v_dataReaderSampleTemplate(sample)->message = c_keep(message);
    v_lifespanAdminInsert(v_dataReaderEntry(index->entry)->lifespanAdmin,
                          v_lifespanSample(sample));

    return sample;
}


void
v_dataReaderSampleFree(
    v_dataReaderSample sample)
{
    v_dataReaderInstance instance;
    v_index index;
    v_dataReader dataReader;
    v_message message;

    if (sample) {
        assert(C_TYPECHECK(sample, v_dataReaderSample));
        if (c_refCount(sample) == 1) {
            /* Free the slave-samples as well */
            instance = v_readerSample(sample)->instance;
            index = v_index(instance->index);
            dataReader = v_dataReader(index->reader);
#ifdef _SL_
            if (dataReader->cachedSampleCount < 1000) {
                message = v_dataReaderSampleMessage(sample);
                c_free(message);
                v_dataReaderSampleTemplate(sample)->message = NULL;
                sample->newer = dataReader->cachedSample;
                dataReader->cachedSample = sample;
                dataReader->cachedSampleCount++;
#else
            if (dataReader->cachedSample == NULL) {
                dataReader->cachedSample = sample;
                v_dataReaderSampleFree(sample->older);
                sample->older = NULL;
                sample->newer = NULL;
                message = v_dataReaderSampleMessage(sample);
                c_free(message);
                v_dataReaderSampleTemplate(sample)->message = NULL;
#endif
            } else {
                c_free(sample);
            }
        } else {
            c_free(sample);
        }
    }
}

void
v_dataReaderSampleRemoveFromLifespanAdmin(
    v_dataReaderSample sample)
{
    v_dataReaderInstance instance;
    v_index index;

    if (sample) {
        assert(C_TYPECHECK(sample, v_dataReaderSample));
        instance = v_readerSample(sample)->instance;
        index = v_index(instance->index);
        v_lifespanAdminRemove(v_dataReaderEntry(index->entry)->lifespanAdmin,
                              v_lifespanSample(sample));
    }
}


void
v_dataReaderSampleWipeViews(
    v_dataReaderSample sample)
{
    v_dataViewSampleList viewSample;
    v_dataViewInstance instance;

    assert(sample != NULL);
    assert(C_TYPECHECK(sample, v_dataReaderSample));

    viewSample = v_readerSample(sample)->viewSamples;
    while (viewSample != NULL) {
        instance = v_readerSample(viewSample)->instance;
        v_dataViewSampleListRemove(viewSample);
        v_dataViewSampleRemove(v_dataViewSample(viewSample));
        v_dataViewInstanceRemove(instance);
        viewSample = v_readerSample(sample)->viewSamples;
    }
}

void
v_dataReaderSampleEmptyViews(
    v_dataReaderSample sample)
{
    v_dataViewSampleList viewSample;

    assert(sample != NULL);
    assert(C_TYPECHECK(sample, v_dataReaderSample));

    viewSample = v_readerSample(sample)->viewSamples;
    while (viewSample != NULL) {
        v_dataViewSampleListRemove(viewSample);
        v_dataViewSampleRemove(v_dataViewSample(viewSample));
        viewSample = v_readerSample(sample)->viewSamples;
    }
}
