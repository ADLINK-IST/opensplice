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
#include "v__dataReaderSample.h"
#include "v_dataReaderEntry.h"
#include "v__dataReaderInstance.h"
#include "v__dataView.h"
#include "v__dataViewInstance.h"
#include "v__dataViewSample.h"
#include "v_messageQos.h"
#include "v__reader.h"
#include "v_state.h"
#include "v_index.h"
#include "v_public.h"
#include "v__lifespanAdmin.h"
#include "os_report.h"

v_dataReaderSample
v_dataReaderSampleNew(
    v_dataReaderInstance instance,
    v_message message)
{
    v_dataReader dataReader;
    v_dataReaderSample sample;
    v_readerQos readerQos;
    v_index index;
    os_timeE msgEpoch;

    assert(instance != NULL);
    assert(C_TYPECHECK(message,v_message));

    index = v_index(instance->index);
    dataReader = v_dataReader(index->reader);
    readerQos = v_reader(dataReader)->qos;
    assert(readerQos);

    sample = v_dataReaderSample(c_new(dataReader->sampleType));
    if (sample != NULL) {
        v_readerSample(sample)->instance = (c_voidp)instance;
        v_readerSample(sample)->viewSamples = NULL;
        v_readerSample(sample)->sampleState = 0;

        sample->insertTime = os_timeWGet();

        /* The expiry time calculation is dependent on the DestinationOrderQos(readerQos->orderby.v.kind):
         * In case of the by_reception_timestamp kind the expiry time is determined based on insertion time(sample->insertTime).
         * In case of the by_source_timestamp kind the expiry time is determined based on source time (message->writeTime).
         */

        msgEpoch = os_timeEGet();
        if (readerQos->orderby.v.kind == V_ORDERBY_SOURCETIME) {
            /* assuming wall clocks of source and destination are aligned!
             * calculate the age of the message and then correct the message epoch.
             */
            os_duration message_age = os_timeWDiff(os_timeWGet(), message->writeTime);
            msgEpoch = os_timeESub(msgEpoch, message_age);
        }
        v_dataReaderSampleTemplate(sample)->message = c_keep(message);
        sample->disposeCount = instance->disposeCount;
        sample->noWritersCount = instance->noWritersCount;
        sample->publicationHandle = message->writerGID;
        sample->readId = 0;
        sample->newer = NULL;
         /* When both ReaderLifespanQos(readerQos->lifespan.used) and the inline LifespanQos (v_messageQos_getLifespanPeriod(message->qos))
          * are set the expiryTime will be set to the earliest time among them.
          */
        if (message->qos) {
            os_duration lifespan = v_messageQos_getLifespanPeriod(message->qos);
            if (readerQos->lifespan.v.used) {
                if (os_durationCompare(readerQos->lifespan.v.duration, lifespan) == OS_LESS) {
                    v_lifespanSample(sample)->expiryTime = os_timeEAdd(msgEpoch, readerQos->lifespan.v.duration);
                } else {
                    v_lifespanSample(sample)->expiryTime = os_timeEAdd(msgEpoch, lifespan);
                }
                v_lifespanAdminInsert(v_dataReaderEntry(index->entry)->lifespanAdmin, v_lifespanSample(sample));
            } else {
                if (OS_DURATION_ISINFINITE(lifespan)) {
                    v_lifespanSample(sample)->expiryTime = OS_TIMEE_INFINITE;
                } else {
                    v_lifespanSample(sample)->expiryTime = os_timeEAdd(msgEpoch, lifespan);
                    v_lifespanAdminInsert(v_dataReaderEntry(index->entry)->lifespanAdmin, v_lifespanSample(sample));
                }
            }
        } else {
            if (readerQos->lifespan.v.used) {
                v_lifespanSample(sample)->expiryTime = os_timeEAdd(msgEpoch,readerQos->lifespan.v.duration);
                v_lifespanAdminInsert(v_dataReaderEntry(index->entry)->lifespanAdmin, v_lifespanSample(sample));
            } else {
                v_lifespanSample(sample)->expiryTime = OS_TIMEE_INFINITE;
            }
        }
    } else {
        OS_REPORT(OS_FATAL, OS_FUNCTION, V_RESULT_INTERNAL_ERROR, "Failed to allocate v_dataReaderSample.");
    }
    return sample;
}

void
v_dataReaderSampleRemoveFromLifespanAdmin(
    v_dataReaderSample sample)
{
    v_dataReaderInstance instance;
    v_index index;

    assert(sample);
    assert(C_TYPECHECK(sample, v_dataReaderSample));

    instance = v_readerSample(sample)->instance;
    index = v_index(instance->index);
    v_lifespanAdminRemove(v_dataReaderEntry(index->entry)->lifespanAdmin, v_lifespanSample(sample));
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

void
v_dataReaderSampleAddViewSample(
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
