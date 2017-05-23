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
#include "v_groupCache.h"
#include "v_dataView.h"
#include "v__dataReaderEntry.h"
#include "v_dataReaderSample.h"
#include "v__lease.h"
#include "v__group.h"
#include "v__dataReaderInstance.h"
#include "v__dataReader.h"
#include "v__deadLineInstanceList.h"
#include "v__lifespanAdmin.h"
#include "v__transaction.h"
#include "v__observer.h"
#include "v_state.h"
#include "v_instance.h"
#include "v__deadLineInstance.h"
#include "v_index.h"
#include "v__dataReaderSample.h"
#include "v_topic.h"
#include "v_public.h"
#include "v_message.h"
#include "v_messageQos.h"
#include "c_misc.h"
#include "os_report.h"
#include "os_abstract.h"
#include "v__subscriber.h"
#include "v__kernel.h"
#include "v_collection.h"
#include "v__policy.h"
#include "v__orderedInstance.h"
#include "v__reader.h"
#include "v__leaseManager.h"

/* For debugging purposes... */
#ifndef NDEBUG
/*
 * The L_EMPTY flag indicates that the instance has no VALID nor INVALID samples.
 * In that case, both the instance HEAD and instance TAIL should be NULL.
 * The sampleCount variable only accounts for VALID samples, to be able to
 * validate whether any resource limit is exceeded, since resource limits only
 * apply to VALID samples.
 */
#define CHECK_EMPTINESS(i) \
        { \
            c_bool empty = v_dataReaderInstanceStateTest(i, L_EMPTY); \
            if (empty && i->historySampleCount != 0) { \
                printf("at line %d sampleCount = %d, isEmpty = %d\n", \
                       __LINE__, i->historySampleCount, empty); \
            } \
        }
#else
#define CHECK_EMPTINESS(i)
#endif

#ifndef NDEBUG
#define CHECK_INSTANCE_CONSISTENCY(_this) \
    if (v_dataReaderInstanceOldest(_this) == NULL && v_dataReaderInstanceNewest(_this) != NULL) {               \
        printf("Warning at line %d v_dataReaderInstanceOldest = NULL, "                                         \
               "but v_dataReaderInstanceNewest != NULL\n", __LINE__);                                           \
    }                                                                                                           \
    if (v_dataReaderInstanceNewest(_this) == NULL && v_dataReaderInstanceOldest(_this) != NULL) {               \
        printf("Warning at line %d v_dataReaderInstanceNewest = NULL, "                                         \
               "but v_dataReaderInstanceOldest != NULL\n", __LINE__);                                           \
    }


#else
#define CHECK_INSTANCE_CONSISTENCY(_this)
#endif

#ifndef NDEBUG
#define CHECK_COUNT(_this) v_dataReaderInstanceCheckCount(_this)

#else
#define CHECK_COUNT(_this)
#endif

/**
 * Returns the relative order of a v_message to a v_historyBookmark.
 * @param _this A v_historyBookmark
 * @param msg The address of a v_message
 * @return C_EQ, C_LT or C_GT if msg is respectively equal, less or greater than _this bookmark
 */
static c_equality
v_historyBookmarkMessageCompare(
    struct v_historyBookmark *_this,
    v_message msg)
{
    C_STRUCT(v_message) template;

    template.writeTime = _this->sourceTimestamp;
    template.writerGID = _this->gid;
    template.sequenceNumber = _this->sequenceNumber;
    ((v_node)&template)->nodeState = _this->isImplicit ? L_IMPLICIT : 0;
    return v_messageCompare(&template, msg);
}

#ifndef NDEBUG
static void
v_dataReaderInstanceCheckCount(
    v_dataReaderInstance _this)
{
    c_long writeFound = 0;
    v_dataReaderSample currentSample;

    assert(C_TYPECHECK(_this, v_dataReaderInstance));
    assert(_this->resourceSampleCount >= _this->historySampleCount);

    currentSample = v_dataReaderInstanceOldest(_this);

    while (currentSample != NULL) {
        if (v_dataReaderSampleMessageStateTest(currentSample, L_WRITE) &&
            v_dataReaderSampleStateTestNot(currentSample, L_INMINSEPTIME)) {
            writeFound++;
        }
        currentSample = currentSample->newer;
    }
    assert(writeFound == _this->historySampleCount);
}
#endif

static c_bool
writeSlave(
    c_object o,
    c_voidp arg)
{
    v_actionResult res = v_dataViewWrite(v_dataView(o),v_readerSample(arg));
    return v_actionResultTest(res, V_PROCEED);
}

static v_message
CreateTypedInvalidMessage(
    v_dataReaderInstance _this,
    v_message untypedMsg)
{
    v_message typedMsg;

    /* Create a message for the invalid sample to carry. */
    typedMsg = v_dataReaderInstanceCreateMessage(_this);
    if (typedMsg) {
        /* Set correct attributes. */
        v_node(typedMsg)->nodeState = v_node(untypedMsg)->nodeState;
        typedMsg->writerGID = untypedMsg->writerGID;
        typedMsg->writeTime = untypedMsg->writeTime;
        typedMsg->writerInstanceGID = untypedMsg->writerInstanceGID;
        typedMsg->qos = c_keep(untypedMsg->qos);
        typedMsg->sequenceNumber = untypedMsg->sequenceNumber;
        typedMsg->transactionId = untypedMsg->transactionId;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderInstance", V_RESULT_INTERNAL_ERROR,
                  "CreateTypedInvalidMessage(_this=0x%"PA_PRIxADDR", untypedMsg=0x%"PA_PRIxADDR")\n"
                  "        Operation failed to allocate new v_message: result = NULL.",
                  (os_address)_this, (os_address)untypedMsg);
        assert(FALSE);
    }

    return typedMsg;
}

static void
invalidSamplesPurgeUntil(
    v_dataReaderInstance _this,
    v_dataReaderSample bookmark)
{
    v_dataReaderSample sample, next;

    /* Loop through all samples of the instance. */
    sample = v_dataReaderInstanceOldest(_this);
    while (sample != bookmark) {
        /* Remember the next sample (in case the current one gets purged.) */
        next = sample->newer;

        /* Purge samples that are invalid. */
        if(!v_readerSampleTestState(sample, L_VALIDDATA)) {
            v_dataReaderSampleTake(sample, NULL, NULL);
        }

        /* Iterate to the next sample. */
        sample = next;
    }

    /* Now purge more aggressively and also purge all invalid samples up to the next valid sample. */
    sample = bookmark;
    while (sample != NULL && !v_readerSampleTestState(sample, L_VALIDDATA)) {
        next = sample->newer;
        v_dataReaderSampleTake(sample, NULL, NULL);
        sample = next;
    }
}

static void
invalidSampleResetEventCounters(
    v_dataReaderSample sample,
    v_dataReader reader)
{
    /* Loop through all remaining samples starting with the current one. */
    while (sample)
    {
        /* Invalid samples that have not yet been read but have already
         * communicated the instance state change should no longer be
         * represented in the notReadCount. So for all remaining invalid
         * samples that are NOT_READ, we decrease the notReadCount and
         * and raise the L_READ flag to indicate that these samples
         * should no longer influence the notReadCount.
         */
        if (!v_readerSampleTestStateOr(sample, L_VALIDDATA | L_READ | L_LAZYREAD))
        {
            reader->notReadCount--;
            v_readerSampleSetState(sample, L_READ);
        }
        sample = v_dataReaderSample(sample->newer);
    }
}

static void
updateIntermediateInstanceAndSampleState(
    v_dataReaderInstance _this,
    v_message message,
    v_dataReaderSample sample)
{
    v_dataReader reader = v_dataReaderInstanceReader(_this);
    v_message prevMsg, nextMsg;
    v_state prevMsgState, nextMsgState;
    v_dataReaderSample s;
    v_state msgState = v_nodeState(message);
    assert(sample->newer);

    /* Valid samples that are part of a transaction that is still in progress do
     * count for resource limits, so update sample count no matter if this
     * sample is part of an ongoing transaction.
     */
    if (v_stateTest(msgState, L_WRITE))
    {
        v_readerSampleSetState(sample, L_VALIDDATA);
        _this->historySampleCount++;
    }

    /* If the sample is a an UNREGISTER, it might need to set increase the
     * noWritersCount, but only if the older sample was not a dispose.
     * However, if the previous sample was both a DISPOSE and an UNREGISTER,
     * then it does need to increase the noWritersCount.
     */
    if (v_stateTest(msgState, L_UNREGISTER))
    {
        if (sample->older)
        {
            prevMsg = v_dataReaderSampleMessage(sample->older);
            prevMsgState = v_nodeState(prevMsg);
            if (!v_stateTest(prevMsgState, L_DISPOSED) ||
                    v_stateTest(prevMsgState, L_DISPOSED | L_UNREGISTER))
            {
                _this->noWritersCount++;
                s = sample;
                while (s->newer)
                {
                    s = s->newer;
                    s->noWritersCount++;
                }
                v_dataReaderInstanceStateSetMask(_this, L_NEW | L_STATECHANGED);
                if (hasValidSampleAccessible(_this))
                {
                    v_dataReaderInstanceStateSet(_this, L_TRIGGER);
                }
            }
        }
    }

    /* If the sample is a a DISPOSE, it needs to increase the disposeCount.
     * If the newer sample was an UNREGISTER, then it needs to decrease the
     * noWritersCount as well. However, if the newer sample was both a DISPOSE
     * and an UNREGISTER, then it can skip the last step.
     */
    if (v_stateTest(msgState, L_DISPOSED))
    {
        _this->disposeCount++;
        s = sample;
        while (s->newer)
        {
            s = s->newer;
            s->disposeCount++;
        }

        /* Intermediate sample, so there is always a newer sample. */
        nextMsg = v_dataReaderSampleMessage(sample->newer);
        nextMsgState = v_nodeState(nextMsg);
        if (v_stateTest(nextMsgState, L_UNREGISTER) &&
                !v_stateTest(nextMsgState, L_DISPOSED | L_UNREGISTER))
        {
            _this->noWritersCount--;
            s = sample;
            while (s->newer)
            {
                s = s->newer;
                s->noWritersCount--;
            }
        }
        v_dataReaderInstanceStateSetMask(_this, L_NEW | L_STATECHANGED);
        if (hasValidSampleAccessible(_this))
        {
            v_dataReaderInstanceStateSet(_this, L_TRIGGER);
        }
    }
    v_deadLineInstanceListUpdate(v_dataReader(v_index(_this->index)->reader)->deadLineList,
                                 v_deadLineInstance(_this),
                                 message->allocTime);

    if (v_dataReaderInstanceStateTest(_this, L_TRIGGER) ||
        v_stateTest(msgState, L_WRITE)) {
        reader->notReadCount++;
    }
}

static void
updateFinalInstanceAndSampleState(
    v_dataReaderInstance _this,
    v_message message,
    v_dataReaderSample sample)
{
    v_dataReader reader = v_dataReaderInstanceReader(_this);
    v_readerQos qos = v_reader(reader)->qos;
    v_state msgState = v_nodeState(message);
    c_bool generationEnd = FALSE;

    assert(!v_readerSampleTestState(sample,L_INMINSEPTIME));
    if (v_stateTest(msgState, L_WRITE))
    {
        v_readerSampleSetState(sample, L_VALIDDATA);
        _this->historySampleCount++;
    }

    /* If the instance is empty and the reader has subscriber defined keys,
     * then the instance must raise its L_NEW flag.
     */
    if (v_dataReaderInstanceStateTest(_this, L_EMPTY) &&
            qos->userKey.v.enable)
    {
        v_dataReaderInstanceStateSet(_this, L_NEW);
    }

    /* Since a new sample was added to the instance, clear its L_EMPTY flag. */
    v_dataReaderInstanceStateClear(_this, L_EMPTY);

    /* If the instance is currently in a NOWRITERS state, then inserting this
     * sample should bring it back into the NEW state and potentially increase
     * its noWritersCount.
     */
    if (v_dataReaderInstanceStateTest(_this, L_NOWRITERS))
    {
        _this->noWritersCount++;
        /* Only clear the NOWRITERS state when there are actual registrations,
         * or when the dataReader has defined its own subscription keys in
         * which case the readerInstance state is no longer bound to the
         * groupInstance state. Be aware that when data comes from the
         * durability store, the registrations might no longer exist, so
         * that the NOWRITERS state may still be applicable.
         */
        if (_this->liveliness > 0 || qos->userKey.v.enable)
        {
            v_dataReaderInstanceStateClear(_this, L_NOWRITERS);
        }
        v_dataReaderInstanceStateSetMask(_this, L_NEW | L_STATECHANGED);
    }

    /* If the instance is currently in a DISPOSED state, then inserting this
     * sample should increase its disposeCount and potentially bring it back
     * into the NEW state.
     */
    if (v_dataReaderInstanceStateTest(_this, L_DISPOSED))
    {
        /* In case the instance was in a DISPOSED state while a DISPOSE message
         * arrives, the disposeCount should not change. However, the message
         * is kept because of its timestamp, and to be able to correctly handle
         * any future intermediate samples. For WriteDispose messages, the
         * disposeCount should be increased.
         * Don't indicate a new lifecycle when the current sample is only an
         * unregister message, since the instance is already in a DISPOSED
         * state and an UNREGISTER message doesn't override that state.
         */
        if (v_stateTest(msgState, L_WRITE))
        {
            _this->disposeCount++;
            /* Clear the current DISPOSE state. A WriteDispose will switch it
             * back on at the end of this function when its own DISPOSE flag
             * is being processed.
             */
            v_dataReaderInstanceStateClear(_this, L_DISPOSED);
            v_dataReaderInstanceStateSetMask(_this, L_NEW | L_STATECHANGED);
        }
    }

    /* Assign sample counters. Be aware that only valid samples consume
     * resource limits.
     */
    sample->disposeCount = _this->disposeCount;
    sample->noWritersCount = _this->noWritersCount;

    /*
     * Update the last insertion time if time_based_filter QoS is set,
     * and message does not have disposed or unregister flags
     */
    if (v_stateTestOr(msgState, L_DISPOSED | L_UNREGISTER)) {
        _this->lastInsertionTime = OS_TIMEE_ZERO;
    } else {
        _this->lastInsertionTime = message->allocTime;
    }

    /* Now process all flags that have currently been raised on the
     * message. Since the sample belongs to the most recent lifecycle,
     * its flags fully determine the instance state.
     */
    if (v_stateTest(msgState, L_DISPOSED))
    {
        /* If the instance was already in a DISPOSED state, then this
         * new DISPOSE message cannot be considered a state change.
         */
        if (!v_dataReaderInstanceStateTest(_this, L_DISPOSED))
        {
            v_dataReaderInstanceStateSetMask(_this, (L_DISPOSED | L_STATECHANGED));

            /* If the sample is valid, the update constitutes a readable
             * state change for which observers need to be triggered.
             *
             * If the sample is invalid, it may become piggy-backed
             * onto a valid sample, triggering the observers to
             * communicate its state change, but masking the invalid
             * sample itself. The sample should increase notReadCount,
             * but decrease it back when its instance state change has
             * been processed.
             *
             * If the sample is invalid and cannot be piggy-backed
             * onto a valid sample, then it may only be communicated
             * to the observers when the reader has indicated that
             * it is prepared to receive invalid samples.
             */
            if (v_stateTest(msgState, L_WRITE) ||
                    qos->lifecycle.v.enable_invalid_samples ||
                    hasValidSampleAccessible(_this))
            {
                /* Don't set the trigger flag when this is a dispose all
                 * and the instance needs to be purge immediately.
                 * Dispose all is indicated by the lack of a writerGID. */
                if (!(!v_gidIsValid(message->writerGID) &&
                        qos->lifecycle.v.autopurge_dispose_all))
                {
                    v_dataReaderInstanceStateSet(_this, L_TRIGGER);
                }
            }
        }
        generationEnd = TRUE; /* Indicate that this generation has ended. */
    }
    if (v_stateTest(msgState, L_UNREGISTER))
    {
        /* Unregister messages are not expected to contain valid data. */
        assert(v_stateTestNot(msgState, L_WRITE));

        /* If the message also has the DISPOSED flag set, then do not
         * set the instance state to NOWRITERS.
         */
        if (!v_dataReaderInstanceStateTest(_this, L_DISPOSED))
        {
            v_dataReaderInstanceStateSetMask(_this, (L_NOWRITERS | L_STATECHANGED));
            /* If the sample is valid, the update constitutes a readable
             * state change for which observers need to be triggered.
             *
             * If the sample is invalid, it may become piggy-backed
             * onto a valid sample, triggering the observers to
             * communicate its state change, but masking the invalid
             * sample itself. The sample should increase notReadCount,
             * but decrease it back when its instance state change has
             * been processed.
             *
             * If the sample is invalid and cannot be piggy-backed
             * onto a valid sample, then it may only be communicated
             * to the observers when the reader has indicated that
             * it is prepared to receive invalid samples.
             */
            if (v_stateTest(msgState, L_WRITE) ||
                    qos->lifecycle.v.enable_invalid_samples ||
                    hasValidSampleAccessible(_this))
            {
                v_dataReaderInstanceStateSet(_this, L_TRIGGER);
            }
        }
        if (_this->liveliness == 0) {
            generationEnd = TRUE; /* Indicate that this generation has ended. */
        }
    }
    /* If the sample is a readable sample and it does not belong to an
     * unfinished transaction, then it should always trigger its observers.
     */
    if (v_stateTest(msgState, L_WRITE))
    {
        v_dataReaderInstanceStateSet(_this, L_TRIGGER);
    }

    /* If a sample is responsible for setting a trigger, then it should
     * also increase the notReadCount. Note that a sample can trigger
     * the instance for multiple reasons at the same time, yet the
     * notReadCount may only be increased once.
     * If the sample does not increase the notReadCount, then it should
     * also not decrease it afterwards. For that reason we set its L_READ
     * flag, to indicate that it does not impact the notReadCount.
     */
    if (v_dataReaderInstanceStateTest(_this, L_TRIGGER)){
        reader->notReadCount++;
    } else {
        v_readerSampleSetState(sample, L_READ);
    }

    /* In case this generation is ended, remove the readerInstance from the deadline list.
     * Otherwise update the instance in the deadline list.
     */
    if (generationEnd) {
        /* v_instanceRemove(v_instance(_this)); */
        v_deadLineInstanceListRemoveInstance(
            v_dataReaderDeadLineInstanceList(reader),
            v_deadLineInstance(_this));
    } else {
        v_deadLineInstanceListUpdate(
            v_dataReaderDeadLineInstanceList(reader),
            v_deadLineInstance(_this), message->allocTime);
    }
}

v_dataReaderResult
v_dataReaderInstanceClaimResource(
    v_dataReaderInstance _this,
    v_message message,
    v_messageContext context)
{
    v_dataReaderResult result = V_DATAREADER_INSERTED;
    v_dataReader reader = v_dataReaderInstanceReader(_this);
    v_readerQos qos = v_reader(reader)->qos;

    /* Valid samples that are part of a transaction that is still in progress do
     * count for resource limits, so update sample count no matter if this
     * sample is part of an ongoing transaction. If a transaction is being
     * flushed, then the samples were already inserted before so that there
     * will be no change to the resource limits.
     */
    if (v_messageStateTest(message, L_WRITE) && context != V_CONTEXT_TRANSACTIONFLUSH) {
        if ((qos->resource.v.max_samples != V_LENGTH_UNLIMITED) &&
            (reader->resourceSampleCount >= qos->resource.v.max_samples)) {
            result = V_DATAREADER_MAX_SAMPLES;
        }
        if (result == V_DATAREADER_INSERTED && qos->resource.v.max_samples_per_instance != V_LENGTH_UNLIMITED &&
            _this->resourceSampleCount >= qos->resource.v.max_samples_per_instance) {
            result = V_DATAREADER_INSTANCE_FULL;
        }
        if (result == V_DATAREADER_INSERTED) {
            reader->resourceSampleCount++;
            _this->resourceSampleCount++;
            v_checkMaxSamplesPerInstanceWarningLevel(v_objectKernel(_this), (c_ulong) _this->resourceSampleCount);
        }
    }

    return result;
}

void
v_dataReaderInstanceReleaseResource(
    v_dataReaderInstance _this)
{
    v_dataReader reader = v_dataReaderInstanceReader(_this);

    assert(reader->resourceSampleCount);
    assert(_this->resourceSampleCount);

    reader->resourceSampleCount--;
    _this->resourceSampleCount--;
}

static v_dataReaderResult
InsertPending(
    v_dataReaderInstance _this,
    v_message message,
    v_messageContext context)
{
    v_dataReaderSample s;
    v_dataReader reader;
    v_readerQos qos;
    v_message m;
    c_equality equality;
    v_dataReaderResult result;
    v_dataReaderSample sample;
    v_dataReaderSample oldest;

    reader = v_dataReaderInstanceReader(_this);
    qos = v_reader(reader)->qos;

    s = v_dataReaderSample(_this->pending);
    if (s) {
        oldest = v_dataReaderSample(_this->pending)->newer;
    } else {
        oldest = NULL;
    }

    /* Find insertion point in pending history.
     * Start at the newest (last inserted) sample and
     * walk to the oldest (first inserted).
     */
    if ((s) && (qos->orderby.v.kind == V_ORDERBY_SOURCETIME)) {
        do {
            m = v_dataReaderSampleMessage(s);
            equality = v_messageCompare(message, m);
            if ( equality == C_EQ ) {
                return V_DATAREADER_DUPLICATE_SAMPLE;
            }
            if ( equality == C_LT) {
                s = s->older;
            }
        } while ( s!= NULL && equality == C_LT);
    }

    /* If max_samples_per_instance is then handle resource issue.
     */
    if (v_messageStateTest(message, L_WRITE) &&
        (qos->resource.v.max_samples_per_instance != V_LENGTH_UNLIMITED) &&
        (qos->resource.v.max_samples_per_instance <= _this->resourceSampleCount))
    {
        if (s == NULL) {
            if (oldest) {
                /* The message is older than the history samples so it will be discarded. */
                if (qos->history.v.kind == V_HISTORY_KEEPLAST) {
                    return V_DATAREADER_OUTDATED;
                } else if (qos->history.v.kind == V_HISTORY_KEEPALL) {
                    return V_DATAREADER_SAMPLE_LOST;
                }
            } else {
                return V_DATAREADER_INSTANCE_FULL;
            }
        } else {
            /* If the sample requires resources, the history policy is KEEP_LAST and the
             * maximum depth is already achieved, then try to push out samples to make room
             * for this one.
             */
            if (v_messageStateTest(message, L_WRITE) && qos->history.v.kind == V_HISTORY_KEEPLAST)
            {
                v_dataReaderSample oldest = v_dataReaderSample(_this->pending)->newer;
                while ((_this->resourceSampleCount >= qos->resource.v.max_samples_per_instance) &&
                       (oldest != NULL))
                {
                    if (s == oldest) {
                        s = NULL;
                    }
                    if (v_messageStateTest(v_dataReaderSampleMessage(oldest), L_WRITE)) {
                        v_dataReaderInstanceReleaseResource(_this);
                    }
                    if(reader->statistics){
                        reader->statistics->numberOfSamplesDiscarded++;
                    }
                    if (oldest == v_dataReaderSample(_this->pending)) {
                        _this->pending = NULL;
                        c_free(oldest);
                        oldest = NULL;
                    } else {
                        v_dataReaderSample(oldest->newer)->older = NULL;
                        v_dataReaderSample(_this->pending)->newer = oldest->newer;
                        c_free(oldest);
                        oldest = v_dataReaderSample(_this->pending)->newer;
                    }
                }
            }
        }
    }
    result = v_dataReaderInstanceClaimResource(_this, message, context);
    if (result == V_DATAREADER_INSERTED) {
        sample = v_dataReaderSampleNew(_this, message);
        if (sample) {
            (sample)->older = s;
            if (s) {
                if (s == v_dataReaderSample(_this->pending)) {
                    sample->newer = v_dataReaderSample(_this->pending)->newer;
                    _this->pending = v_dataReaderSampleTemplate(sample);
                } else {
                    assert(s->newer != NULL);
                    v_dataReaderSample(s->newer)->older = sample;
                    sample->newer = s->newer;
                }
                s->newer = sample;
            } else {
                if (_this->pending) {
                    sample->newer = v_dataReaderSample(_this->pending)->newer;
                    v_dataReaderSample(sample->newer)->older = sample;
                    v_dataReaderSample(_this->pending)->newer = sample;
                } else {
                    _this->pending = v_dataReaderSampleTemplate(sample);
                    sample->newer = sample;
                }
            }
            sample = NULL;
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataReaderInstance", V_DATAREADER_OUT_OF_MEMORY,
                      "InsertPending(_this=0x%"PA_PRIxADDR", message=0x%"PA_PRIxADDR", sample=0x%"PA_PRIxADDR")\n"
                      "        Unable to allocate v_dataReaderSample.",
                      (os_address)_this, (os_address)message, (os_address)sample);
            result = V_DATAREADER_OUT_OF_MEMORY;
        }
    }
    /* Return FILTERED_OUT so that the reader skips purge actions. */
    if (result == V_DATAREADER_INSERTED) { result = V_DATAREADER_FILTERED_OUT; }
    return result;
}

static void
MakeSampleAvailable (
    v_dataReaderInstance _this,
    v_dataReaderSample sample)
{
    v_dataReader reader;

    reader = v_dataReaderInstanceReader(_this);
    /* If ReaderQos has orderedAccess set to TRUE, then report the sample
     * to the orderedInstance.
     */
    if (reader->orderedInstance) {
        if (v_readerSampleTestState(sample, L_VALIDDATA) ||
            v_reader(reader)->qos->lifecycle.v.enable_invalid_samples)
        {
            v_orderedInstanceWrite(reader->orderedInstance, v_readerSample(sample));
        }
    }
    /* Write the sample into all dataViews attached to this reader, but only
     * if the sample contains valid data.
     */
    if ((reader->views != NULL) && v_messageStateTest(v_dataReaderSampleMessage(sample), L_WRITE)) {
        (void)c_walk(reader->views,writeSlave,sample);
    }
    if (v_dataReaderInstanceStateTest(_this, L_TRIGGER) &&
        (reader->notReadTriggerThreshold <= 0 ||
         reader->notReadTriggerCount++ == reader->notReadTriggerThreshold))
    {
        V_MESSAGE_STAMP(message,readerDataAvailableTime);
        V_MESSAGE_STAMP(message, readerInstanceTime);

        reader->notReadTriggerCount = 0;
        v_dataReaderInstanceStateClear(_this, L_TRIGGER);
        v_dataReaderNotifyDataAvailable(reader, sample);
    }
}

static v_dataReaderResult
FindHistoryPosition(
    v_dataReaderInstance _this,
    v_message message,
    v_dataReaderSample *sampleHolder)
{
    v_message m;
    c_equality equality;
    v_dataReader reader;
    v_readerQos qos;
    v_dataReaderSample s;
    v_dataReaderResult result = V_DATAREADER_INSERTED;
    c_long depthBookmark = 0;

    reader = v_dataReaderInstanceReader(_this);
    qos = v_reader(reader)->qos;

    /* Find insertion point in history.
     * Start at the newest (last inserted) sample and
     * walk to the oldest (first inserted).
     */
    *sampleHolder = NULL;
    s = v_dataReaderInstanceNewest(_this);
    if (qos->orderby.v.kind == V_ORDERBY_SOURCETIME) {
        equality = C_LT;
        while ( s!= NULL && equality == C_LT && result == V_DATAREADER_INSERTED) {
            m = v_dataReaderSampleMessage(s);
            /*
             * Messages can be ignored when:
             * 1) they have the same pointer as the current position.
             *    In this case the message is a local duplicate of an
             *    already existing message. (Probably delivered over
             *    multiple matching logical partitions.)
             * 2) they need to be ordered by source timestamp while they
             *    have a timestamp equal to the current position AND
             *    and a writerGID equal to the current position AND
             *    a sequenceNumber equal to the current position.
             *    In this case the message is a network duplicate of an
             *    already existing message. (Probably delivered over
             *    multiple matching network partitions).
             *
             * Messages will be inserted when they have a timestamp equal
             * to the current position AND a writerGID unequal to the current
             * position. In this case 2 messages from different sources happen
             * to be written simultaneously. To guarantee eventual consistency
             * throughout the system in such a case, messages with equal
             * timestamps will be sorted by their GID values, and then by
             * their sequenceNumber.
             */
            equality = v_messageCompare(message, m);
            /* C_GT : message is newer than the current position
             *        in the history and older than the previous.
             *        so insert before the current position.
             * C_LT : message is older than the current position
             *        in the history and newer than the previous.
             *        so goto the next iteration.
             * C_EQ : message has the same timestamp as the current
             *        position. If both messages have different
             *        sources, then messages with the same timestamp
             *        will be sorted by their writerGID and
             *        sequenceNumber to guarantee eventual consistency
             *        throughout all DataReaders in the Domain. */
            if ( equality == C_EQ ) {
                result = V_DATAREADER_DUPLICATE_SAMPLE;
            } else if ( equality == C_LT) {
                if (v_messageStateTest(m, L_WRITE)) {
                    depthBookmark++;
                }
                *sampleHolder = s;
                s = s->older;
            }
        }

        if (result == V_DATAREADER_INSERTED) {
            if (qos->history.v.kind == V_HISTORY_KEEPLAST && depthBookmark >= qos->history.v.depth) {
                /* If this sample is older than all VALID samples in the current
                 * history, and the history has already reached its full depth,
                 * then the sample can discarded. */
                if(reader->statistics){
                    reader->statistics->numberOfSamplesDiscarded++;
                }
                result = V_DATAREADER_OUTDATED;
            } else if (qos->history.v.kind == V_HISTORY_KEEPALL &&
                    qos->resource.v.max_samples_per_instance != V_LENGTH_UNLIMITED &&
                    depthBookmark >= qos->resource.v.max_samples_per_instance) {
                /*
                 * Since the sample is older than all samples currently
                 * in the queue, it doesn't make sense to retransmit
                 * this sample. Once data has been consumed and space
                 * is available, the 'lastConsumed' timestamp will have
                 * increased, resulting in the sample to be refused
                 * because of this new value of 'lastConsumed'. Since
                 * that causes the sample to be considered 'lost', we
                 * might as well save the trouble of retransmitting and
                 * mark it as 'lost' right now.
                 */
                result = V_DATAREADER_SAMPLE_LOST;
            }
        }
    } else {
        /* In case of reception time insert we still need to verify if the message has not been received before.
         * For performance reason we decided not to scan the whole history for each sample but just the first
         * in the history because a duplicate is likely to be the last received.
         */
        if (s != NULL) {
            m = v_dataReaderSampleMessage(s);
            if (v_messageCompare(message, m) == C_EQ) {
                result = V_DATAREADER_DUPLICATE_SAMPLE;
            }
        }
    }
    return result;
}

static void
ClaimHistorySample(
    v_dataReaderInstance _this,
    v_dataReaderSample insertionpoint)
{
    v_dataReader reader;
    v_readerQos qos;

    reader = v_dataReaderInstanceReader(_this);
    qos = v_reader(reader)->qos;

    while (_this->historySampleCount >= qos->history.v.depth) {
        /* We have reached max history depth for this instance.
         * So the oldest samples must be removed until we have removed a
         * valid sample.
         */
        if (insertionpoint) {
            if (!insertionpoint->older) break;
        } else {
            if (!v_dataReaderInstanceNewest(_this)) break;
        }
        v_dataReaderInstanceSampleRemove(_this, v_dataReaderInstanceOldest(_this), TRUE);
        if(reader->statistics){
            reader->statistics->numberOfSamplesDiscarded++;
        }
    }
}

static v_dataReaderResult
InsertHistory(
    v_dataReaderInstance _this,
    v_message message,
    v_messageContext context)
{
    v_dataReaderSample s;
    v_dataReader reader;
    v_readerQos qos;
    os_int32 equality;
    v_dataReaderResult result;
    v_dataReaderSample sample;
    v_state messageState;
    os_boolean filteredOut = OS_FALSE;

    messageState = v_nodeState(message);
    reader = v_dataReaderInstanceReader(_this);
    qos = v_reader(reader)->qos;
    CHECK_COUNT(_this);

    result = V_DATAREADER_INSERTED;
    /* Filter out messages that violate the time based filter minimum separation time,
     * if the message state is not disposed or unregister and the time_based_filter QoS is enabled
     * Messages part of a transaction are filtered if/when a transaction flush occurs.
     * This will not work at this point because message->allocTime is compared with _this->lastInsertionTime,
     * which expect ordering but currently no ordering is guaranteed, for now accept old messages.
     */
    if (!v_stateTestOr(messageState, L_DISPOSED | L_UNREGISTER) &&
        (!OS_DURATION_ISZERO(qos->pacing.v.minSeperation)))
    {
        /* In case this instance is already in the minimum separation list then replace previous pending sample. */
        if (v_dataReaderInstanceStateTest(_this, L_INMINSEPTIME)) {
            /* Instance is already in minimum separation list so lookup pending sample. */
            s = v_dataReaderInstanceNewest(_this);
            while (s && !v_readerSampleTestState(s, L_INMINSEPTIME)) { s = s->older; }
            if (s) {
                v_message m = v_dataReaderSampleMessage(s);
                if (v_messageCompare(message, m) == C_GT) {
                    /* Message is newer than existing pending message so replace existing. */
                    if (s->older) {
                        v_dataReaderSample(s->older)->newer = s->newer;  /* Transfer ownership to the newer sample */
                    } else {
                        v_dataReaderInstanceSetOldest(_this, s->newer);
                    }
                    if (s->newer) {
                        v_dataReaderSample(s->newer)->older = s->older;  /* Transfer ownership to the newer sample */
                    } else {
                        v_dataReaderInstanceSetNewest(_this, s->older);
                    }
                    s->older = NULL;
                    s->newer = NULL;
                    c_free(s);
                    v_dataReaderInstanceReleaseResource(_this);
                } else {
                    result = V_DATAREADER_OUTDATED;
                }
            }
        }
        if (result == V_DATAREADER_INSERTED) {
            equality = os_timeECompare(message->allocTime,
                                       os_timeEAdd(_this->lastInsertionTime, qos->pacing.v.minSeperation));

            if (equality == OS_LESS) {
                /* if message is within the minimun separation window then filter out if not reliable or
                 * store in minimum separation list if reliable.
                 */
                if (qos->reliability.v.kind != V_RELIABILITY_RELIABLE) {
                    result = V_DATAREADER_FILTERED_OUT;
                } else {
                    filteredOut = OS_TRUE;
                }
            }
        }
    }
    CHECK_COUNT(_this);
    if (result == V_DATAREADER_INSERTED) {
        /* Find the location where the sample needs to be inserted, and change
         * the list accordingly.
         */
        result = FindHistoryPosition(_this, message, &s);
        /* Find insertion point in history */
        /* If the sample requires resources, the history policy is KEEP_LAST and the
         * maximum depth is already achieved, then try to push out samples to make room
         * for this one.
         */
        if (result == V_DATAREADER_INSERTED &&
            v_messageStateTest(message, L_WRITE) &&
            qos->history.v.kind == V_HISTORY_KEEPLAST &&
            !filteredOut)
        {
            ClaimHistorySample(_this, s);
        }
    }

    CHECK_COUNT(_this);
    if (result == V_DATAREADER_INSERTED) {
        result = v_dataReaderInstanceClaimResource(_this, message, context);
    }
    if (result == V_DATAREADER_INSERTED) {
        sample = v_dataReaderSampleNew(_this, message);
        if (sample) {
            if (filteredOut) {
                v_readerSampleSetState(sample,L_INMINSEPTIME);
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataReaderInstance", V_DATAREADER_OUT_OF_MEMORY,
                      "InsertSample(_this=0x%"PA_PRIxADDR", message=0x%"PA_PRIxADDR", sample=0x%"PA_PRIxADDR")\n"
                      "        Unable to allocate v_dataReaderSample.",
                      (os_address)_this, (os_address)message, (os_address)sample);
            result = V_DATAREADER_OUT_OF_MEMORY;
        }
    }

    if (result == V_DATAREADER_INSERTED) {
        sample->newer = s;
        if (s == NULL) {
            sample->older = v_dataReaderInstanceNewest(_this);
            if (sample->older) {
                v_dataReaderSample(sample->older)->newer = sample;
            } else {
                v_dataReaderInstanceSetOldest(_this, sample);
            }
            v_dataReaderInstanceSetNewest(_this, sample);
        } else {
            sample->older = s->older;
            if (!s->older) {
                v_dataReaderInstanceSetOldest(_this, sample);
            } else {
                v_dataReaderSample(s->older)->newer = sample;
            }
            s->older = sample;
        }
        if (filteredOut) {
            v_dataReaderMinimumSeparationListRegister(reader, sample);
            v_deadLineInstanceListUpdate(v_dataReaderDeadLineInstanceList(reader),
                                         v_deadLineInstance(_this),
                                         message->allocTime);

        } else {
            v_dataReaderMinimumSeparationListRemove(reader, _this);
            /* Check if inserted sample is the newest one and update states accordingly. */
            if(v_dataReaderInstanceNewest(_this) == sample){
                updateFinalInstanceAndSampleState(_this, message, sample);
            } else {
                updateIntermediateInstanceAndSampleState(_this, message, sample);
            }
            if (((v_observerEventMask(reader) & V_EVENT_PREPARE_DELETE) == 0)) {
               MakeSampleAvailable(_this, sample);
            }
        }
    }
    CHECK_COUNT(_this);
    return result;
}

static v_dataReaderResult
InsertSample(
    v_dataReaderInstance _this,
    v_message message,
    v_messageContext context)
{
    v_dataReader reader;
    v_readerQos qos;
    c_equality equality;
    v_dataReaderResult result;
    c_bool accessLock = FALSE;
    c_bool insert = TRUE;
    v_subscriber subscriber;

    reader = v_dataReaderInstanceReader(_this);
    qos = v_reader(reader)->qos;
    /* Only insert the sample if it is newer than any history that has
     * already been consumed so far. This is to prevent old data from
     * re-appearing when newer data has already been consumed. (Note that
     * in case of equal timestamps order is determined by the writerGID and
     * then by sequence-number.)
     */
    result = V_DATAREADER_INSERTED;
    if (qos->orderby.v.kind == V_ORDERBY_SOURCETIME) {
        equality = v_historyBookmarkMessageCompare(&_this->lastConsumed, message);
        if (equality == C_EQ) {
            result = V_DATAREADER_DUPLICATE_SAMPLE;
        } else if (equality == C_GT) {
            result = V_DATAREADER_SAMPLE_LOST;
        }
    }
    if (result == V_DATAREADER_INSERTED) {
        subscriber = v_readerSubscriber(v_reader(reader));
        assert(subscriber);

        if (context != V_CONTEXT_TRANSACTIONFLUSH) {
            if (v__readerIsGroupOrderedNonCoherent(v_reader(reader))) {
                /* accessLock only required for group, none coherent, ordered subscribers.  */
                v_subscriberLock(subscriber);
                insert = accessLock = v_subscriberTryLockAccess(subscriber);
                if (accessLock == FALSE) {
                   result = InsertPending(_this, message, context);
                }
                v_subscriberUnlock(subscriber);
            }
        }
        if (insert) {
            result = InsertHistory(_this, message, context);
        }
        if (accessLock) {
            v_subscriberLock(subscriber);
            v_subscriberUnlockAccess(subscriber);
            v_subscriberUnlock(subscriber);
        }
    }
    if (result != V_DATAREADER_INSERTED &&
        context == V_CONTEXT_TRANSACTIONFLUSH &&
        v_messageStateTest(message, L_WRITE))
    {
        v_dataReaderInstanceReleaseResource(_this);
    }
    return result;
}

void
v_dataReaderInstanceInit (
    v_dataReaderInstance _this,
    v_message message)
{
    c_array instanceKeyList;
    c_array messageKeyList;
    c_value value;
    c_ulong i, nrOfKeys;
    v_index index;
    v_readerQos qos;

    assert(_this);
    assert(message);
    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    assert(C_TYPECHECK(message,v_message));

    index = _this->index;

    assert(_this->resourceSampleCount == 0);

    qos = v_reader(index->reader)->qos;

    /* The first insert will increase to 0 times noWriter state. */
    v_instanceSetState(_this, (L_EMPTY | L_NOWRITERS));
    v_deadLineInstanceInit(v_deadLineInstance(_this), v_entity(index->reader));
    _this->noWritersCount               = -1;
    _this->disposeCount                 = 0;
    _this->resourceSampleCount          = 0;
    _this->historySampleCount           = 0;
    _this->liveliness                   = 0;
    _this->hasBeenAlive                 = FALSE;
    _this->epoch                        = OS_TIMEW_ZERO;
    _this->lastConsumed.sourceTimestamp = OS_TIMEW_ZERO;
    v_gidSetNil(_this->lastConsumed.gid);
    _this->lastConsumed.sequenceNumber  = 0;
    _this->lastConsumed.isImplicit      = FALSE;
    _this->purgeInsertionTime           = OS_TIMEM_ZERO;
    _this->lastInsertionTime            = OS_TIMEE_ZERO;
    _this->pending                      = NULL;
    v_dataReaderInstanceSetOldest(_this,NULL);
    v_dataReaderInstanceSetNewest(_this,NULL);

    /* only if ownership is exclusive the owner must be set! */
     if (qos->ownership.v.kind == V_OWNERSHIP_EXCLUSIVE) {
        _this->owner.exclusive = TRUE;
    } else {
        _this->owner.exclusive = FALSE;
    }

    /*
     * copy key value from message into instance.
     */
    messageKeyList = v_indexMessageKeyList(index);
    instanceKeyList = v_indexKeyList(index);
    assert(c_arraySize(messageKeyList) == c_arraySize(instanceKeyList));
    nrOfKeys = c_arraySize(messageKeyList);
    for (i=0;i<nrOfKeys;i++) {
        value = c_fieldValue(messageKeyList[i],message);
        c_fieldAssign(instanceKeyList[i],_this,value);
        c_valueFreeRef(value);
    }

    c_free(messageKeyList);
    c_free(instanceKeyList);
}

v_dataReaderInstance
v_dataReaderInstanceNew(
    v_dataReader dataReader,
    v_message message)
{
    v_dataReaderInstance _this;

    assert(dataReader);
    assert(message);
    assert(C_TYPECHECK(dataReader,v_dataReader));
    assert(C_TYPECHECK(message,v_message));

    _this = v_dataReaderAllocInstance(dataReader);
    v_dataReaderInstanceInit(_this,message);

    CHECK_COUNT(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);
    return _this;
}

void
v_dataReaderInstanceFree(
    v_dataReaderInstance instance)
{
    v_dataReader reader;
    assert(C_TYPECHECK(instance,v_dataReaderInstance));

    reader = v_dataReaderInstanceReader(instance);
    if (v_dataReaderInstanceStateTest(instance, L_INMINSEPTIME)) {
        v_dataReaderMinimumSeparationListRemove(reader, instance);
    }
    v_publicFree(v_public(instance));
    c_free(instance);
}


void
v_dataReaderInstanceSetEpoch (
    v_dataReaderInstance _this,
    os_timeW time)
{
    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    CHECK_COUNT(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    _this->epoch = time;
}

void
v_dataReaderInstanceDeinit(
    v_dataReaderInstance _this)
{
    v_index index;
    v_dataReader reader;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    CHECK_COUNT(_this);

    index = v_index(_this->index);
    reader = v_dataReader(index->reader);

    v_deadLineInstanceListRemoveInstance(reader->deadLineList, v_deadLineInstance(_this));
    v_deadLineInstanceDeinit(v_deadLineInstance(_this));
}

v_writeResult
v_dataReaderInstanceWrite (
    v_dataReaderInstance _this,
    v_message msg)
{
    v_writeResult result;
    v_dataReaderEntry entry;
    v_dataReaderInstance* thisPtr;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    assert(C_TYPECHECK(msg,v_message));

    if ((_this->owner.exclusive) &&
        (v_messageQos_getOwnershipStrength(msg->qos) < _this->owner.strength) &&
        (v_gidIsValid(_this->owner.gid)) &&
        (!v_gidEqual(_this->owner.gid, msg->writerGID)) /* also choose else-branch in case current owner lowered strength */
        ) {
        if (v_messageStateTest(msg, L_UNREGISTER)) {
            /*
             * An unregister message should decrease the liveliness.
             * This is normally done in the v_dataReaderInstanceInsert() function.
             * But, because this message is not forwarded and will not reach that
             * function, the liveliness has the be decreased here.
             *
             * The register message takes a different route. It will not come here and
             * will always reach v_dataReaderInstanceInsert() to increase liveliness.
             */
            if (_this->liveliness > 0)
            {
                _this->liveliness--;
            }
        }
        result = V_WRITE_SUCCESS;
    } else {
        entry = v_dataReaderEntry(v_index(_this->index)->entry);
        thisPtr = &_this;
        result = v_dataReaderEntryWrite(entry, msg, (v_instance *)thisPtr, V_CONTEXT_GROUPWRITE);
    }

    return result;
}

void
v_dataReaderInstanceResetOwner(
    v_dataReaderInstance _this,
    v_gid wgid)
{
    if (_this->owner.exclusive) {
        if (v_gidEqual(_this->owner.gid, wgid)) {
            v_gidSetNil(_this->owner.gid);
        }
    }
}

v_message
v_dataReaderInstanceCreateMessage(
    v_dataReaderInstance _this)
{
    v_message message = NULL;
    v_index index = v_index(_this->index);
    c_array instanceKeyList, messageKeyList;
    c_field srcField, trgtField;
    c_ulong i, nrOfKeys;

    if (_this != NULL) {
        message = v_topicMessageNew(v_indexTopic(_this->index));
        if (message != NULL)
        {
            messageKeyList = v_indexMessageKeyList(index);
            instanceKeyList = v_indexKeyList(index);
            assert(c_arraySize(messageKeyList) == c_arraySize(instanceKeyList));
            nrOfKeys = c_arraySize(messageKeyList);
            /* copy key value(s) from instance into message */
            for (i = 0; i < nrOfKeys; i++)
            {
                srcField = (c_field)instanceKeyList[i];
                trgtField = (c_field)messageKeyList[i];
                c_fieldCopy(srcField, (c_object)_this, trgtField, (c_object)message);
            }
            c_free(instanceKeyList);
            c_free(messageKeyList);
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataReaderInstance", V_RESULT_INTERNAL_ERROR,
                      "v_dataReaderInstanceCreateMessage(_this=0x%"PA_PRIxADDR")\n"
                      "        Operation failed to allocate new topicMessage: result = NULL.", (os_address)_this);
            assert(FALSE);
        }
    }
    return message;
}

static void
CheckAndProcessReplacePolicyMarker(
    v_dataReaderInstance _this,
    v_message message)
{
    v_dataReaderSample sample, latestDisposedSample, nextSample;

    /* The first message that is inserted due to the REPLACE
     * merge policy triggers the purging of historical data
     * that is not needed anymore. To recognise the first message
     * due to a REPLACE merge policy the following algorithm is used:
     *
     * If the message is labelled with the L_REPLACED flag, and
     *    the reader instance state also contains the L_REPLACED marker,
     * then
     *    - remove the L_REPLACED flag from the reader instance
     *    - purge all samples up to and including the latest dispose
     *      marked with L_REPLACED (this basically cleans up all
            historical data before the replace merge policy was applied)
     *    - insert the message in the reader instance.
     *
     * In all other cases insert the message in the reader instance as usual.
     */
    if ( v_stateTest(v_nodeState(message), L_REPLACED) ) {
        latestDisposedSample = NULL;
        /* The message is first message that in injected in
         * the reader instance due to the replace merge policy.
         * Now remove the marker from the reader instance state.
         */
        v_dataReaderInstanceStateClear(_this, L_REPLACED);
        /* Now purge all samples up to the latest dispose.
         * First find the latest disposed message marked with the specified flags
         */
        sample = v_dataReaderInstanceNewest(_this);
        while ( (sample != NULL) && (latestDisposedSample == NULL) ) {
            if (v_readerSampleTestState(sample, L_DISPOSED) &&
                v_readerSampleTestState(sample, L_REPLACED)) {
                latestDisposedSample = sample;
            } else {
                sample = sample->older;
            }
        }
        if (latestDisposedSample) {
            /* The latest DISPOSE message with the L_REPLACE flag has been found.
             * This DISPOSE message represents the moment at which a REPLACE merge
             * policy is applied. By definition of the REPLACE merge policy, all
             * existing historical data needs to be replaced with historical data
             * that it gets aligned. This implies that all existing historical data
             * can be discarded. To discard simpy take the samples, starting with
             * the oldest.
             */
            sample = v_dataReaderInstanceOldest(_this);
            while ( (sample != latestDisposedSample->newer) && (sample != NULL) ) {
                /* Remember the next sample (in case the current one gets purged.) */
                nextSample = sample->newer;
                /* Take the sample */
                v_dataReaderSampleTake(sample, NULL, NULL);
                /* Iterate to the next sample. */
                sample = nextSample;
            }
        }
        /* Now follow the normal insertion path. */
    }
}

v_dataReaderResult
v_dataReaderInstanceInsert(
    v_dataReaderInstance _this,
    v_message message,
    v_messageContext context)
{
    v_state messageState;
#if 0
    os_compare equality;
#endif
    v_index index;
    v_dataReader reader;
    v_dataReaderEntry entry;
#if 0
    v_readerQos qos;
#endif
    v_dataReaderResult result = V_DATAREADER_UNDETERMINED;
    struct v_owner ownership;
    v_ownershipResult ownershipResult = V_OWNERSHIP_OWNER;

    assert(message != NULL);
    assert(_this != NULL);
    assert(C_TYPECHECK(message,v_message));
    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    index = v_index(_this->index);
    reader = v_dataReader(index->reader);
    entry = v_dataReaderEntry(index->entry);
    messageState = v_nodeState(message);

#if 0
    qos = v_reader(reader)->qos;

    /*
     * Filter out messages that still belong to a previous lifecycle of this
     * 'recycled' v_dataReaderInstance object. The epoch time determines when
     * the previous lifecycle ended, so everything older than that can be
     * discarded.
     */
    if (qos->orderby.v.kind == V_ORDERBY_SOURCETIME) {
        equality = os_timeWCompare(message->writeTime, _this->epoch);
        if (equality != OS_MORE) {
            CHECK_COUNT(_this);
            /* TODO: in case of a transaction message should we look for
             * existing transaction and abort it or ignore it and leave it
             * to garbage collection or maybe this cannot occur?
             */
            return V_DATAREADER_OUTDATED;
        }
    }
#endif

    /* Replace merge policy code:
     * During a replace action all existing instances are marked with the replace flag
     * This flag will be reset at the end of the replace action or if a replace message
     * is received.
     * Following operation will check and process incomming replace messages.
     */
    if (v_dataReaderInstanceStateTest(_this, L_REPLACED)) {
        CheckAndProcessReplacePolicyMarker(_this, message);
    }

    /* The first message carrying the L_MARK flag that is
     * added to an existing instance will clear the L_MARK
     * from the instance to indicate that the instance has
     * been touched by the CATCHUP merge policy */
    if ( v_stateTest(messageState, L_MARK) &&
         v_dataReaderInstanceStateTest(_this, L_MARK)) {
        v_dataReaderInstanceStateClear(_this, L_MARK);
        /* Now follow the normal insertion path. */
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (message->qos) {
        /*
         * Test if ownership is exclusive and whether the writer identified
         * in the message should become owner. In case of an invalid GID
         * ownership is always assumed. (For example in case of disposeAll.)
         */
        ownership.exclusive = v_messageQos_isExclusive(message->qos);
        if (ownership.exclusive == TRUE) {
            c_bool claim = !v_stateTest(messageState, L_REGISTER);
            ownership.strength = v_messageQos_getOwnershipStrength(message->qos);
            ownership.gid = message->writerGID;
            ownershipResult = v_determineOwnershipByStrength(&_this->owner, &ownership, claim);
        }
    } else {
        ownership.exclusive = FALSE;
    }

    if (context != V_CONTEXT_TRANSACTIONFLUSH) {
        /* All kinds of messages need to update alive writers.
         * The alive writers must always be updated even when the writer is
         * not owner, it remains a writer that is alive.
         */
        if (v_stateTest(messageState, L_REGISTER)) {
            _this->liveliness++;
        }
        if (v_message_isTransaction(message) && entry->transactionAdmin && context != V_CONTEXT_GETHISTORY )
        {
            /* Exclude finished transaction messages (V_CONTEXT_TRANSACTIONFLUSH) and
             * exclude finished historical transaction messages (V_CONTEXT_GETHISTORY).
             * These should not go into the transaction admin but instead go into the
             * readers history, i.e. made available to the reader.
             */
            /* If the sample belongs to an unfinished transaction, then insert it into the
             * transactional administration. Since this is a newly arriving sample, it still
             * needs to make a resource claim.
             */
            v_writeResult wres;
            v_instance instance = NULL;
            if (ownershipResult != V_OWNERSHIP_NOT_OWNER) {
                instance = v_instance(_this);
            }
            wres = v_transactionAdminInsertMessage(entry->transactionAdmin, message, instance, FALSE, NULL);
            if (wres == V_WRITE_SUCCESS) {
                result = V_DATAREADER_FILTERED_OUT;
            } else if (wres == V_WRITE_REJECTED) {
                result = V_DATAREADER_MAX_SAMPLES;
            } else if (wres == V_WRITE_SUCCESS_NOT_STORED) {
                result = V_DATAREADER_INSERTED;
            } else {
                assert(0);
            }
            return result;
        }
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (v_stateTest(messageState, L_REGISTER)) {
        result = V_DATAREADER_INSERTED;
    } else if (v_stateTest(messageState, L_UNREGISTER)) {
        if (_this->liveliness == 0) {
            /* Unregister messages can be received when liveliness == 0 when
             * the Reader disconnects from a group. In that case each instance
             * will be unregistered again, even the ones that have liveliness
             * set to 0 already. Just ignore the message in that case.
             */
            result = V_DATAREADER_INSERTED;
        } else {
            _this->liveliness--;
            /* It is possible (f.i. by deadline QoS) that a lower strength
             * writer has already taken over before this unregister message
             * of a possible higher strength writer is received/triggered.
             *
             * If the current unregister write has a higher strength and would
             * claim this instance, then the lower strength writer (that has
             * taken over and actually is providing data) will be ignored again,
             * until the next deadline occurs.
             * This will cause a 'gap' within sample delivery to the reader.
             * The duration of this gap is the duration of deadline QoS.
             */
            if ((ownership.exclusive == TRUE) && (ownershipResult == V_OWNERSHIP_ALREADY_OWNER)) {
                /* If the writer indicates will no longer update (own) this
                 * instance (by sending an unregister message) then the ownership
                 * is released by resetting the owner gid to nil.
                 */
                v_gidSetNil (_this->owner.gid);
            }
            if (_this->liveliness > 0 && !v_stateTest(messageState, L_DISPOSED)) {
                /* In case the UNREGISTER did not result in a NOWRITERS state,
                 * because the liveliness counter is still > 0, then prevent
                 * any sample from being inserted by indicating this sample
                 * has already been processed. However, if the message also has
                 * a DISPOSE attached, then process the DISPOSE using the normal
                 * route (i.e. first determine ownership). The UNREGISTER
                 * flag will not have any further negative impact when
                 * processing the DISPOSE using this normal route (since an
                 * UNREGISTER following a DISPOSE does not result in any
                 * further state change).
                 */
                result = V_DATAREADER_INSERTED;
            }
        }
    }

    /* When no message has been inserted yet, handle all other types of messages. */
    if (result == V_DATAREADER_UNDETERMINED) {
        if (v_observerEventMask(reader) & V_EVENT_PREPARE_DELETE) {
            /* No need to insert incomming messages when the reader is in process of being deleted. */
            result = V_DATAREADER_INSERTED;
        } else if (ownership.exclusive == TRUE && (ownershipResult == V_OWNERSHIP_INCOMPATIBLE_QOS ||
                                                   ownershipResult == V_OWNERSHIP_NOT_OWNER))
        {
            /* No need to insert incomming messages form lower strength writers. */
            result = V_DATAREADER_NOT_OWNER;
        }
    }
    /* When message state is not determined then it can be inserted. */
    CHECK_COUNT(_this);
    if (result == V_DATAREADER_UNDETERMINED) {
        result = InsertSample(_this, message, context);
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    assert(_this->resourceSampleCount >= _this->historySampleCount);
    return result;
}

c_bool
v_dataReaderInstanceTest(
    v_dataReaderInstance _this,
    c_query query,
    v_state sampleMask,
    v_queryAction action,
    c_voidp args)
{
    v_dataReaderSample sample, newestSample, prevSample;
    c_bool sampleSatisfies = FALSE;
    v_dataReader r;
    v_state msgState;
    v_message msg;

    assert(_this);
    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this == NULL)
    {
        return FALSE;
    }
    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (v_dataReaderInstanceEmpty(_this))
    {
        return FALSE;
    }

    newestSample = v_dataReaderInstanceNewest(_this);
    sample = v_dataReaderInstanceOldest(_this);
    if (!hasValidSampleAccessible(_this))
    {
        /* Only pass invalid samples in case the L_STATECHANGED is set. */
        if (v_dataReaderInstanceStateTest(_this, L_STATECHANGED))
        {
            /* No valid samples exist, so there must be at least one invalid
             * sample. So pick the the most recent one at newest, unless
             * that's an UNREGISTER which is preceded by a DISPOSE without
             * UNREGISTER, since then the DISPOSE should be communicated.
             */
            sample = v_dataReaderInstanceNewest(_this);
            assert(sample);
            assert(!v_readerSampleTestState(sample, L_VALIDDATA));

            msg = v_dataReaderSampleMessage(sample);
            msgState = v_nodeState(msg);

            /* Check if our sample is a DISPOSE event. */
            if((!v_stateTest(msgState, L_DISPOSED)) && (sample->older))
            {
                prevSample = sample->older;

                msg = v_dataReaderSampleMessage(prevSample);
                msgState = v_nodeState(msg);

                /* If our found event is a DISPOSE event, this is the
                 * one we need to present. If not, the originally
                 * found UNREGISTER is the one to present.
                 */
                if (v_stateTest(msgState, L_DISPOSED))
                {
                    sample = prevSample;
                }
            }

            /* Only pass this sample if it has not been READ before and if
             * the reader has not disabled invalid samples.
             */
            r = v_dataReaderInstanceReader(_this);
            if (!v_readerSampleTestStateOr(sample, L_READ | L_LAZYREAD) &&
                     v_reader(r)->qos->lifecycle.v.enable_invalid_samples &&
                     v_sampleMaskPass(sampleMask, sample))
            {
                sampleSatisfies = action(sample, args);
            }
        }

        CHECK_EMPTINESS(_this);
        CHECK_COUNT(_this);
        CHECK_INSTANCE_CONSISTENCY(_this);

        return sampleSatisfies;
    }

    while ((sample != NULL) && (sampleSatisfies == FALSE))
    {
        /* Invalid samples will not be offered when the instance has valid
         * samples as well. Therefore invalid samples cannot match the query
         * when historySampleCount > 0.
         */
        if (v_readerSampleTestState(sample, L_VALIDDATA))
        {
            /* If a query has been passed, evaluate the sample against the
             * query. If not, then make sure the sample is evaluated against
             * the action routine.
             */
            if (v_sampleMaskPass(sampleMask, sample)) {
                if (query) {
                    /* The history samples are swapped with the first sample to make
                       sample-evaluation on instance level work.
                    */
                    if (sample != newestSample) {
                        v_dataReaderInstanceSetNewest(_this,sample);
                    }
                    sampleSatisfies = c_queryEval(query,_this);
                    if (sample != newestSample) {
                        v_dataReaderInstanceSetNewest(_this,newestSample);
                    }
                } else {
                    sampleSatisfies = TRUE;
                }
            }

            /* If a sample passed the query, then check whether it matches the
             * optional condition passed by the action routine. (This condition
             * can for example check for matching lifecycle states.)
             */
            if (sampleSatisfies && action != NULL) {
                sampleSatisfies = action(sample, args);
            }
        }
        sample = sample->newer;
    }

    CHECK_EMPTINESS(_this);
    CHECK_COUNT(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    return sampleSatisfies;
}

v_actionResult
v_dataReaderSampleRead(
    v_dataReaderSample sample,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderInstance instance;
    v_dataReaderSample orgSample;
    v_dataReader reader;
    c_type sampleType;
    v_state state;
    v_state mask;
    v_actionResult result;

    if (v_readerSampleTestState(sample, L_INMINSEPTIME)) {
        return V_SKIP; /* ignore message if before minimum separation time. */
    }
    orgSample = NULL;
    instance = v_dataReaderSampleInstance(sample);
    reader = v_dataReaderInstanceReader(instance);

    CHECK_EMPTINESS(instance);
    CHECK_COUNT(instance);
    CHECK_INSTANCE_CONSISTENCY(instance);

    state = v_dataReaderInstanceState(instance);
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

    /* If the sample contains an untyped invalid message, then temporarily
     * replace it with a typed invalid message so that the Reader is able to
     * interpret its contents.
     */
    if (!v_readerSampleTestState(sample, L_VALIDDATA))
    {
        /* This is a hack to make a shallow copy of the sample. This only works
         * because we manually keep all known references of v_dataReaderSample
         * except for its v_message field, which will be replaced. Of course
         * this is hard to maintain when v_dataReaderSample will be extended to
         * contain other references in the future. Since v_dataReaderSample expects
         * a v_message with userData from its meta-data, copying the sample using
         * c_copyIn will result in an illegal attempt to copy userData from a
         * v_message that doesn't have any.
         * TODO: This code should be removed as soon as scdds2975 has been solved.
         */
        orgSample = sample;
        sampleType = c_typeActualType(c_getType(orgSample));
        sample = c_new(sampleType);
        memcpy(sample, orgSample, sampleType->size);
        c_keep(sample->_parent._parent.next);
        c_keep(sample->older);
        /* Original message was memcopied and thus not kept. Therefore do not use c_free. */
        v_dataReaderSampleTemplate(sample)->message =
            CreateTypedInvalidMessage(instance, v_dataReaderSampleMessage(orgSample));
        /* Hack ends here. */
    }
    V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample), readerReadTime);
    result = action(v_readerSample(sample), arg);
    /* In case of ordered access and group access scope the reader may read at most
     * one sample, even if the max samples specifies more than one.
     * The following stops reading more than one for this use case.
     */
    if (reader->orderedInstance && reader->orderedInstance->presentation == V_PRESENTATION_GROUP && v_actionResultTest(result, V_PROCEED)) {
        result = V_STOP;
    }

    V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample), readerCopyTime);
    V_MESSAGE_REPORT(v_dataReaderSampleMessage(sample),
                     v_dataReaderInstanceDataReader(instance));

    /* If the message was temporarily switched, switch it back. */
    if (!v_readerSampleTestState(sample, L_VALIDDATA))
    {
        assert(orgSample);
        c_free(sample);
        sample = orgSample;
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
        V_MESSAGE_REPORT(v_dataReaderSampleMessage(sample),
                         v_dataReaderInstanceDataReader(instance));
        v_dataReaderInstanceStateClear(instance, L_NEW);
        v_dataReaderInstanceStateClear(instance, L_STATECHANGED);
        if (!v_readerSampleTestState(sample, L_READ)) {
            v_dataReaderInstanceReader(instance)->notReadCount--;
            v_readerSampleSetState(sample, L_LAZYREAD);
        }

        /* The instance state can have changed, so update the statistics */
        if (reader->statistics) {
            reader->statistics->numberOfSamplesDiscarded++;
        }

        CHECK_EMPTINESS(instance);
        CHECK_COUNT(instance);
        CHECK_INSTANCE_CONSISTENCY(instance);
    }

    return result;
}

c_bool
v_dataReaderInstanceReadSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_state sampleMask,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderSample sample, newestSample;
    v_actionResult result = V_PROCEED;
    c_bool sampleSatisfies;
    int nrSamplesRead = 0;
    c_ulong readId;
    v_dataReader r;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    /* If no valid nor invalid samples exist, then skip further actions. */
    if (_this && !v_dataReaderInstanceEmpty(_this))
    {
        r = v_dataReaderInstanceReader(_this);

        /* Check the number of accessible L_VALID samples. If there are none,
         * check to see whether any invalid samples need to be communicated.
         */
        if (!hasValidSampleAccessible(_this))
        {
            /* Only pass invalid samples in case the L_STATECHANGED is set and
             * in case the reader is allowed to return invalid samples.
             */
            if (v_dataReaderInstanceStateTest(_this, L_STATECHANGED) &&
                    v_reader(r)->qos->lifecycle.v.enable_invalid_samples)
            {
                /* No valid samples exist, so there must be at least one invalid
                 * sample that still needs to be processed. So walk through all
                 * samples and that have been accessed previously.
                 */
                sample = v_dataReaderInstanceOldest(_this);
                assert(sample);
                assert(!v_readerSampleTestState(sample, L_VALIDDATA));
                while(sample != NULL &&
                        v_readerSampleTestStateOr(sample, L_READ | L_LAZYREAD))
                {
                    sample = sample->newer;
                }
                /* If a sample is found matching the criteria, then consume it.
                 */
                if (sample)
                {
                    if (v_sampleMaskPass(sampleMask, sample)) {
                        result = v_dataReaderSampleRead(sample, action, arg);

                        /* Reset the event counters for all remaining invalid samples in this
                         * instance if they have communicated their instance state change.
                         */
                        if (v_actionResultTestNot(result, V_SKIP)) {
                            invalidSampleResetEventCounters(v_dataReaderInstanceOldest(_this), r);
                        }
                    }
                }
            }

            CHECK_EMPTINESS(_this);
            CHECK_COUNT(_this);
            CHECK_INSTANCE_CONSISTENCY(_this);

            return v_actionResultTest(result, V_PROCEED);
        }
        readId = v_dataReaderInstanceDataReader(_this)->readCnt;
        newestSample = v_dataReaderInstanceNewest(_this);
        sample = v_dataReaderInstanceOldest(_this);
        while (sample != NULL && v_actionResultTest(result, V_PROCEED)) {
            if (sample->readId != readId)
            {
                if (query != NULL && v_readerSampleTestState(sample, L_VALIDDATA))
                {
                    /* The history samples are swapped with the newest sample
                     * to make sample-evaluation on instance level work.
                     */
                    if (sample != newestSample)
                    {
                        v_dataReaderInstanceSetNewest(_this,sample);
                    }
                    sampleSatisfies = c_queryEval(query,_this);

                    /* Now swap back the original sample. */
                    if (sample != newestSample)
                    {
                        v_dataReaderInstanceSetNewest(_this,newestSample);
                    }
                } else {
                    /* queries on invalid data are not applicable,
                     * so set satisfies to TRUE. When no query present
                     * the sample also satisfies.
                     */
                    sampleSatisfies = TRUE;
                }
                /* Only pass samples that match and that also have valid data. */
                if (sampleSatisfies &&
                    v_readerSampleTestState(sample, L_VALIDDATA) &&
                    v_sampleMaskPass(sampleMask, sample))
                {
                    sample->readId = readId;
                    result = v_dataReaderSampleRead(sample, action, arg);
                    if (v_actionResultTestNot(result, V_SKIP)) {
                        nrSamplesRead++;
                    }
                }
            }
            sample = sample->newer;
        }

        /* Reset the event counters for all remaining invalid samples in this
         * instance if they have communicated their instance state change.
         */
        if (nrSamplesRead > 0)
        {
            invalidSampleResetEventCounters(v_dataReaderInstanceOldest(_this), r);
        }
    }

    CHECK_EMPTINESS(_this);
    CHECK_COUNT(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    return v_actionResultTest(result, V_PROCEED);
}


v_actionResult
v_dataReaderInstanceWalkSamples(
    v_dataReaderInstance _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderSample sample;
    v_actionResult result = V_PROCEED;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this != NULL)
    {
        CHECK_COUNT(_this);
        CHECK_EMPTINESS(_this);
        CHECK_INSTANCE_CONSISTENCY(_this);

        if (!v_dataReaderInstanceEmpty(_this))
        {
            sample = v_dataReaderInstanceOldest(_this);
            while ((sample != NULL) && v_actionResultTest(result, V_PROCEED))
            {
                result = action(v_readerSample(sample), arg);
                sample = sample->newer;
            }
            CHECK_COUNT(_this);
            CHECK_EMPTINESS(_this);
            CHECK_INSTANCE_CONSISTENCY(_this);
        }
    }
    return result;
}

void
v_dataReaderInstanceSampleRemove(
    v_dataReaderInstance _this,
    v_dataReaderSample sample,
    c_bool pushedOutByNewer)
{
    v_dataReader reader = v_dataReaderInstanceReader(_this);
    v_message msg = v_dataReaderSampleMessage(sample);

    assert(_this->resourceSampleCount >= 0);
    if (v_readerSampleTestState(sample, L_VALIDDATA))
    {
        CHECK_COUNT(_this);
        _this->resourceSampleCount--;
        if (!v_readerSampleTestState(sample, L_INMINSEPTIME)) {
            _this->historySampleCount--;
            reader->resourceSampleCount--;
        }
        assert (_this->historySampleCount >= 0);
        assert (_this->resourceSampleCount >= 0);
        assert(v_dataReader(reader)->resourceSampleCount >= 0);
    }
    /* Remove sample from history. */
    if (sample->older)
    {
        assert(v_dataReaderInstanceOldest(_this) != sample);
        v_dataReaderSample(sample->older)->newer = sample->newer;
    } else {
        /* sample = oldest */
        assert(v_dataReaderInstanceOldest(_this) == sample);
        v_dataReaderInstanceSetOldest(_this,sample->newer);
    }
    if (sample->newer)
    {
        assert(v_dataReaderInstanceNewest(_this) != sample);
        v_dataReaderSample(sample->newer)->older = sample->older;
    } else {
        /* sample = newest */
        assert(v_dataReaderInstanceNewest(_this) == sample);
        v_dataReaderInstanceSetNewest(_this,sample->older);
    }

    /* If instance becomes empty, and is not pushed out by a newer sample,
     * then modify states accordingly.
     */
    if (v_dataReaderInstanceOldest(_this) == NULL && !pushedOutByNewer)
    {
        v_dataReaderInstanceStateSet(_this, L_EMPTY);
    }
    CHECK_INSTANCE_CONSISTENCY(_this);

    /* Remove the sample from all administrations. */
    sample->newer = NULL;
    sample->older = NULL;
    v_dataReaderSampleWipeViews(sample);
    v_dataReaderSampleRemoveFromLifespanAdmin(sample);
    if (!v_readerSampleTestStateOr(sample, L_READ | L_LAZYREAD))
    {
        reader->notReadCount--;
    }

    /* Unregister messages should not affect the lastConsumed time. */
    if (!v_messageStateTest(msg,L_UNREGISTER)) {
        /* If the consumed sample is newer than the previously consumed
         * sample, then update the history bookmark to indicate that
         * all samples prior to the current sample have been consumed.
         */
        if (v_historyBookmarkMessageCompare(&_this->lastConsumed, msg) != C_GT)
        {
            _this->lastConsumed.sourceTimestamp = msg->writeTime;
            _this->lastConsumed.gid = msg->writerGID;
            _this->lastConsumed.sequenceNumber = msg->sequenceNumber;
            _this->lastConsumed.isImplicit = v_messageStateTest(msg,L_IMPLICIT);
        }
    }

    /* Free the sample itself. */
    v_readerSampleSetState(sample, L_REMOVED);
    c_free(sample);
}

/*
 * This function checks whether a sample is still contained in the
 * dataReaderInstance. It is not enough to check the instance pointer
 * of the dataReaderSample, because the sample under investigation
 * might just have been taken from the dataReaderInstance but still
 * have its its instance pointer pointing towards the instance.
 * This is for example the case when data is taken but the language
 * specific copyOut functions still need to copy the data outside
 * the reader locks.
 */
c_bool
v_dataReaderInstanceContainsSample(
        v_dataReaderInstance _this,
        v_dataReaderSample sample)
{
    c_bool result = FALSE;

    assert(v_dataReaderSampleInstance(sample) == _this);
    if (v_dataReaderSampleInstance(sample) == _this &&
            !v_readerSampleTestState(sample, L_REMOVED))
    {
        result = TRUE;
    }
    return result;
}


v_actionResult
v_dataReaderSampleTake(
    v_dataReaderSample sample,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderInstance instance;
    v_dataReaderSample orgSample;
    v_dataReader reader;
    c_type sampleType;
    v_state state;
    v_state mask;
    v_actionResult result = 0;

    if (v_readerSampleTestState(sample, L_INMINSEPTIME)) {
        return V_SKIP; /* ignore message if before minimum separation time. */
    }
    orgSample = NULL;
    instance = v_dataReaderSampleInstance(sample);

    CHECK_COUNT(instance);
    CHECK_EMPTINESS(instance);
    CHECK_INSTANCE_CONSISTENCY(instance);

    reader = v_dataReaderInstanceReader(instance);
    state = v_dataReaderInstanceState(instance);
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
        /* If the sample contains an untyped invalid message, then temporarily
         * replace it with a typed invalid message so that the Reader is able to
         * interpret its contents.
         */
        if (!v_readerSampleTestState(sample, L_VALIDDATA))
        {
            /* This is a hack to make a shallow copy of the sample. This only works
             * because we manually keep all known references of v_dataReaderSample
             * except for its v_message field, which will be replaced. Of course
             * this is hard to maintain when v_dataReaderSample will be extended to
             * contain other references in the future. Since v_dataReaderSample expects
             * a v_message with userData from its meta-data, copying the sample using
             * c_copyIn will result in an illegal attempt to copy userData from a
             * v_message that doesn't have any.
             * TODO: This code should be removed as soon as scdds2975 has been solved.
             */
            orgSample = sample;
            sampleType = c_typeActualType(c_getType(orgSample));
            sample = c_new(sampleType);
            memcpy(sample, orgSample, sampleType->size);
            c_keep(sample->_parent._parent.next);
            c_keep(sample->older);
            /* Original message was memcopied and thus not kept. Therefore do not use c_free. */
            v_dataReaderSampleTemplate(sample)->message =
                CreateTypedInvalidMessage(instance, v_dataReaderSampleMessage(orgSample));
            /* Hack ends here. */
        }
        /* Invoke the action routine with the typed sample. */
        result = action(v_readerSample(sample), arg);
        /* In case of ordered access and group access scope the reader may read at most
         * one sample, even if the max samples specifies more than one.
         * The following stops reading more than one for this use case.
         */
        if (reader->orderedInstance && reader->orderedInstance->presentation == V_PRESENTATION_GROUP && v_actionResultTest(result, V_PROCEED)) {
            result = V_STOP;
        }

        /* If the message was temporarily switched, switch it back. */
        if (!v_readerSampleTestState(sample, L_VALIDDATA))
        {
            c_free(sample);
            sample = orgSample;
        }
    } else {
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


        if(action)
        {
            v_dataReaderInstanceStateClear(instance, L_NEW);
            v_dataReaderInstanceStateClear(instance, L_STATECHANGED);
        }

        v_dataReaderInstanceSampleRemove(instance, sample, FALSE);

        if (v_dataReaderInstanceEmpty(instance))
        {
            assert((instance->historySampleCount == 0));
            v_dataReaderInstanceStateSet(instance, L_EMPTY);
        }

        if (reader->triggerValue) {
            v_dataReaderTriggerValueFree(reader->triggerValue);
            reader->triggerValue = NULL;
        }
    }

    /* The instance state can have changed, so update the statistics */
    UPDATE_READER_STATISTICS(reader, instance, state);

    CHECK_COUNT(instance);
    CHECK_EMPTINESS(instance);
    CHECK_INSTANCE_CONSISTENCY(instance);
    return result;
}

c_bool
v_dataReaderInstanceTakeSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_state sampleMask,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderSample sample, next, newestSample;
    v_actionResult result = V_PROCEED;
    c_bool sampleSatisfies;
    int nrSamplesTaken = 0;
    c_ulong readId;
    v_dataReader r;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (_this && !v_dataReaderInstanceEmpty(_this))
    {
        r = v_dataReaderInstanceReader(_this);

        /* Check the number of accessible L_VALID samples. If there are none,
         * check to see whether any invalid samples need to be communicated.
         */
        if (!hasValidSampleAccessible(_this))
        {
            /* Only pass invalid samples in case the L_STATECHANGED is set and
             * in case the reader is allowed to return invalid samples.
             */
            if (v_dataReaderInstanceStateTest(_this, L_STATECHANGED) &&
                    v_reader(r)->qos->lifecycle.v.enable_invalid_samples)
            {
                /* No valid samples exist, so there must be at least one invalid
                 * sample that still needs to be processed. So walk through all
                 * samples and skip the ones that have been accessed previously.
                 */
                sample = v_dataReaderInstanceOldest(_this);
                assert(sample);
                assert(!v_readerSampleTestState(sample, L_VALIDDATA));
                while(sample != NULL &&
                        v_readerSampleTestStateOr(sample, L_READ | L_LAZYREAD))
                {
                    sample = sample->newer;
                }
                /* If a sample is found matching the criteria, then consume it.
                 */
                if (sample)
                {
                    if (v_sampleMaskPass(sampleMask, sample)) {
                        result = v_dataReaderSampleTake(sample, action, arg);

                        /* If the invalid sample has been able to communicate its instance
                         * state, then trash all the remaining invalid samples that are not
                         * part of an unfinished transaction (i.e. up to newest->next which
                         * is always NULL).
                         */
                        if (v_actionResultTestNot(result, V_SKIP)) {
                            invalidSamplesPurgeUntil(_this, NULL);
                        }
                    }
                }
            } else {
                /* If the instance contains only invalid samples whose state-change has
                 * already been communicated, then these invalid samples have no further
                 * purpose and can be purged.
                 */
                invalidSamplesPurgeUntil(_this, NULL);
            }

            CHECK_EMPTINESS(_this);
            CHECK_COUNT(_this);
            CHECK_INSTANCE_CONSISTENCY(_this);

            return v_actionResultTest(result, V_PROCEED);
        }
        readId = v_dataReaderInstanceDataReader(_this)->readCnt;
        newestSample = v_dataReaderInstanceNewest(_this);
        sample = v_dataReaderInstanceOldest(_this);
        while (sample != NULL && v_actionResultTest(result, V_PROCEED))
        {
            next = sample->newer;
            if (sample->readId != readId)
            {
                if (query != NULL && v_readerSampleTestState(sample, L_VALIDDATA))
                {
                    /* The history samples are swapped with the newest sample
                     * to make sample-evaluation on instance level work.
                     */
                    if (sample != newestSample)
                    {
                        v_dataReaderInstanceSetNewest(_this,sample);
                    }
                    sampleSatisfies = c_queryEval(query,_this);

                    /* Now swap back the original sample. */
                    if (sample != newestSample)
                    {
                        v_dataReaderInstanceSetNewest(_this,newestSample);
                    }
                } else {
                    /* queries on invalid data are not applicable,
                     * so set satisfies to TRUE. When no query present
                     * the sample also satisfies.
                     */
                    sampleSatisfies = TRUE;
                }
                /* Only pass samples that match and that also have valid data. */
                if (sampleSatisfies)
                {
                    if (v_readerSampleTestState(sample, L_VALIDDATA) &&
                        v_sampleMaskPass(sampleMask, sample))
                    {
                        sample->readId = readId;
                        result = v_dataReaderSampleTake(sample, action, arg);
                        if (v_actionResultTestNot(result, V_SKIP)) {
                            nrSamplesTaken++;
                        }
                        CHECK_EMPTINESS(_this);
                    }
                }
            }
            sample = next;
        }

        if (v_dataReaderInstanceEmpty(_this)) {
              assert(!v_dataReaderInstanceStateTest(_this, L_STATECHANGED));
        }

        /* If invalid samples have been able to communicate their state change,
         * then purge all invalid samples up to the last consumed, and reset
         * the event counter for all invalid samples that follow it.
         */
        if (nrSamplesTaken > 0)
        {
            invalidSamplesPurgeUntil(_this, sample);
            if (!v_dataReaderInstanceEmpty(_this)) {
                invalidSampleResetEventCounters(v_dataReaderInstanceOldest(_this), r);
            }
        }

    }
    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    return v_actionResultTest(result, V_PROCEED);
}

void
v_dataReaderInstancePurge(
    v_dataReaderInstance _this,
    c_long disposedCount,
    c_long noWritersCount)
{
    v_dataReaderSample sample;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    /* Algorithm doesn't handle scanning for disposed- AND nowriterscount. */
    assert(disposedCount < 0 || noWritersCount < 0);

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if ((_this != NULL) && !v_dataReaderInstanceEmpty(_this))
    {
        /* Start with the oldest generation, so begin with oldest. */
        sample = v_dataReaderInstanceOldest(_this);

        if (disposedCount >= 0)
        {
            while ((sample != NULL) && (sample->disposeCount <= disposedCount))
            {
                v_dataReaderSample nextSample = sample->newer;
                v_dataReaderInstanceSampleRemove(_this, sample, FALSE);
                sample = nextSample;
            }
        }

        if (noWritersCount >= 0)
        {
            while ((sample != NULL) && (sample->noWritersCount <= noWritersCount))
            {
                v_dataReaderSample nextSample = sample->newer;
                v_dataReaderInstanceSampleRemove(_this, sample, FALSE);
                sample = nextSample;
            }
        }
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);
}

c_collection
v_dataReaderInstanceGetNotEmptyInstanceSet(
    v_dataReaderInstance _this)
{
    c_collection result = NULL;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this != NULL) {
        result = v_index(_this->index)->notEmptyList;
    }

    return result;
}

c_ulong
v_dataReaderInstanceGetNotEmptyInstanceCount(
    v_dataReaderInstance _this)
{
    c_ulong result = 0;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this != NULL) {
        result = c_count(v_index(_this->index)->notEmptyList);
    }

    return result;
}

v_dataReaderResult
v_dataReaderInstanceUnregister (
    v_dataReaderInstance _this,
    v_registration unregistration,
    os_timeW timestamp)
{
    v_kernel kernel;
    v_dataReaderEntry entry;
    v_message msg = NULL;
    v_dataReaderResult result = V_DATAREADER_INSERTED;
    v_writeResult writeResult;
    v_dataReaderInstance* thisPtr;
    c_bool autoDispose = FALSE;
    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (unregistration->qos != NULL) {
        autoDispose = v_messageQos_isAutoDispose(unregistration->qos);
    }

    /* If there are no other registrations, or if the Writer had an autodispose
     * policy set for this instance, then insert an unregister message explicitly. */
    if (_this->liveliness == 1 || autoDispose)
    {
        /* Create an invalid sample as holder for the dispose. */
        kernel = v_objectKernel(_this);
        msg = v_groupCreateInvalidMessage(kernel,
                unregistration->writerGID, unregistration->qos, timestamp);
        if (msg)
        {
            /* Set the nodeState of the message to UNREGISTER. */
            v_stateSet(v_nodeState(msg), L_UNREGISTER);

            if (autoDispose)
            {
                /* Set the nodeState of the message to DISPOSE. */
                v_stateSet(v_nodeState(msg), L_DISPOSED);
            }

            /* Insert the invalid message into the dataReaderInstance. */
            entry = v_dataReaderEntry(v_index(_this->index)->entry);
            thisPtr = &_this;
            writeResult = v_dataReaderEntryWrite(entry, msg, (v_instance *)thisPtr, V_CONTEXT_GROUPWRITE);
            c_free(msg);
            if (writeResult != V_WRITE_SUCCESS)
            {
                result = V_DATAREADER_INTERNAL_ERROR;
                OS_REPORT(OS_CRITICAL,
                          "v_dataReaderInstance", result,
                          "v_dataReaderInstanceUnregister(_this=0x%"PA_PRIxADDR", unregistration=0x%"PA_PRIxADDR", timestamp={%"PA_PRItime"})\n"
                          "        Unable to insert invalid sample in v_dataReaderInstance: result = %s.",
                          (os_address)_this, (os_address)unregistration, OS_TIMEW_PRINT(timestamp), v_dataReaderResultString(result));
                assert(FALSE);
            }
        } else {
            result = V_DATAREADER_OUT_OF_MEMORY;
            OS_REPORT(OS_ERROR,
                      "v_dataReaderInstance", result,
                      "v_dataReaderInstanceUnregister(_this=0x%"PA_PRIxADDR", unregistration=0x%"PA_PRIxADDR", timestamp={%"PA_PRItime"})\n"
                      "        Unable to create invalid sample to indicate instance unregistration.",
                      (os_address)_this, (os_address)unregistration, OS_TIMEW_PRINT(timestamp));
            assert(FALSE);
        }
    }
    return result;
}

static v_dataReaderResult
InsertPendingSample(
    v_dataReaderInstance _this,
    v_dataReaderSample sample)
{
    v_dataReaderResult result = V_DATAREADER_INSERTED;
    v_dataReaderSample s;
    v_dataReader reader;
    v_readerQos qos;
    c_equality equality;
    v_message message = v_dataReaderSampleTemplate(sample)->message;

    /* Only insert the sample if it is newer than any history that has
     * already been consumed so far. This is to prevent old data from
     * re-appearing when newer data has already been consumed. (Note that
     * in case of equal timestamps order is determined by the writerGID and
     * then by sequence-number.)
     */
    reader = v_dataReaderInstanceReader(_this);
    qos = v_reader(reader)->qos;
    if (qos->orderby.v.kind == V_ORDERBY_SOURCETIME)
    {
        equality = v_historyBookmarkMessageCompare(&_this->lastConsumed, message);
        if (equality == C_EQ)
        {
            CHECK_COUNT(_this);
            return V_DATAREADER_DUPLICATE_SAMPLE;
        } else if (equality == C_GT) {
            CHECK_COUNT(_this);
            return V_DATAREADER_SAMPLE_LOST;
        }
    }

    /* Find the location where the sample needs to be inserted, and change
     * the list accordingly.
     */
    result = FindHistoryPosition(_this, message, &s);
    if (result == V_DATAREADER_INSERTED) {
        if (v_messageStateTest(message, L_WRITE) && qos->history.v.kind == V_HISTORY_KEEPLAST) {
            ClaimHistorySample(_this, s);
        }
        sample->newer = s;
        if (s == NULL) {
            sample->older = v_dataReaderInstanceNewest(_this);
            if (sample->older) {
                v_dataReaderSample(sample->older)->newer = sample;
            } else {
                v_dataReaderInstanceSetOldest(_this, sample);
            }
            v_dataReaderInstanceSetNewest(_this, sample);
            updateFinalInstanceAndSampleState(_this, message, sample);
        } else {
            sample->older = s->older;
            if (!s->older) {
                v_dataReaderInstanceSetOldest(_this, sample);
            } else {
                v_dataReaderSample(s->older)->newer = sample;
            }
            s->older = sample;
            updateIntermediateInstanceAndSampleState(_this, message, sample);
        }
        if ((v_observerEventMask(reader) & V_EVENT_PREPARE_DELETE) == 0) {
            MakeSampleAvailable(_this, sample);
        }
    }
    return result;
}

void
v_dataReaderInstanceFlushPending(
    v_dataReaderInstance _this)
{
    v_dataReaderResult result;
    v_dataReaderSample newest;
    v_dataReaderSample oldest;

    while (_this->pending) {
        newest = v_dataReaderSample(_this->pending);
        oldest = newest->newer;
        assert(oldest->older == NULL);
        if (oldest == newest) {
            _this->pending = NULL;
        } else {
            v_dataReaderSample(oldest->newer)->older = NULL;
            newest->newer = oldest->newer;
        }
        oldest->newer = NULL;
        result = InsertPendingSample(_this, oldest);
        if (result != V_DATAREADER_INSERTED) {
            if (v_dataReaderSampleMessageStateTest(oldest, L_WRITE)) {
                v_dataReaderInstanceReleaseResource(_this);
            }
        }
    }
}

c_bool
v_dataReaderInstanceMatchesSampleMask (
    v_dataReaderInstance _this,
    v_sampleMask mask)
{
    /* This function ignores sample state flags and only checks if the instance
       flags specified match. */
    v_sampleMask state = V_MASK_ALIVE_INSTANCE;

    assert (_this != NULL && C_TYPECHECK (_this, v_dataReaderInstance));

    if (v_stateTest (v_dataReaderInstanceState (_this), L_NEW)) {
        v_sampleMaskSet (state, V_MASK_NEW_VIEW);
    } else {
        v_sampleMaskSet (state, V_MASK_NOT_NEW_VIEW);
    }

    if (v_stateTest (v_dataReaderInstanceState (_this), L_DISPOSED)) {
        v_sampleMaskSet (state, V_MASK_DISPOSED_INSTANCE);
        v_sampleMaskClear (state, V_MASK_ALIVE_INSTANCE);
    }
    if (v_stateTest (v_dataReaderInstanceState (_this), L_NOWRITERS)) {
        v_sampleMaskSet (state, V_MASK_NOWRITERS_INSTANCE);
        v_sampleMaskClear (state, V_MASK_ALIVE_INSTANCE);
    }

    return v_sampleMaskTest (mask, state);
}

/* This operation is used by the v_dataReaderCheckMinimumSeparation and verifies if this instance
 * still contains a sample within the minimum separation window that by now has expired.
 * If true the sample will be made available.
 */
c_bool
v_dataReaderInstanceCheckMinimumSeparation(
    v_dataReaderInstance _this,
    os_timeE now)
{
    v_dataReaderSample sample;
    c_bool processed = TRUE;

    sample = v_dataReaderInstanceNewest(_this);
    /* lookup sample currently registered being in the minimum separation window. */
    while (sample && !v_readerSampleTestState(sample, L_INMINSEPTIME)) { sample = sample->older; }
    if (sample && v_readerSampleTestState(sample, L_INMINSEPTIME)) {
        /* found a registered sample, so check if it is still within the window. */
        v_dataReader reader = v_dataReaderInstanceReader(_this);
        v_message msg = v_dataReaderSampleMessage(sample);
        if (os_timeEDiff(now, _this->lastInsertionTime) < reader->maximumSeparationTime) {
            /* still in, so don't update anything. */
            processed = FALSE;
        } else {
            /* make sample available in the history when not expired and
             * only use v_timeGet when expiryTime < C_TIME_INFINITE.
             */
            if (OS_TIMEE_ISINFINITE(v_lifespanSample(sample)->expiryTime) ||
                (os_timeECompare(os_timeEGet(), v_lifespanSample(sample)->expiryTime) == OS_LESS)) {
                /* no longer within the window so make sample available, update states. */
                v_readerQos qos = v_reader(reader)->qos;
                /* no longer within the window so make sample available, update states. */
                if (qos->history.v.kind == V_HISTORY_KEEPLAST) {
                    ClaimHistorySample(_this,NULL); /* Pushes out oldest sample in case of history is full */
                }
                v_readerSampleClearState(sample, L_INMINSEPTIME);
                updateFinalInstanceAndSampleState(_this, msg, sample);
                /* The last insertion time is set by the updateFinalInstanceAndSampleState to the message alloc time
                 * This is not correct because the sample is made available lazy after expiry of the minimum separation lease.
                 * therefore correct the last insertion time to the current time.
                 */
                _this->lastInsertionTime = now;
                MakeSampleAvailable(_this, sample);
            } else {
                v_dataReaderInstanceSampleRemove(_this, sample, FALSE);
                if(reader->statistics){
                    reader->statistics->numberOfSamplesDiscarded++;
                }
            }
        }
    }
    return processed;
}
