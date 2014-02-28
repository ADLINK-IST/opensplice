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
#include "v_groupCache.h"
#include "v_dataView.h"
#include "v_dataReaderEntry.h"
#include "v_dataReaderSample.h"
#include "v__group.h"
#include "v__dataReaderInstance.h"
#include "v__dataReader.h"
#include "v__deadLineInstanceList.h"
#include "v__lifespanAdmin.h"
#include "v_state.h"
#include "v_instance.h"
#include "v_index.h"
#include "v__dataReaderSample.h"
#include "v_topic.h"
#include "v_time.h"
#include "v_public.h"
#include "v__statisticsInterface.h"
#include "v_message.h"
#include "v__messageQos.h"
#include "c_misc.h"
#include "os_report.h"
#include "v__kernel.h"
#include "v_collection.h"
#include "v__policy.h"

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
            if (empty && i->sampleCount != 0) { \
                printf("at line %d sampleCount = %d, isEmpty = %d\n", \
                       __LINE__, i->sampleCount, empty); \
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
    }                                                                                                           \
    if ((v_reader(v_dataReaderInstanceReader(_this))->qos->lifecycle.enable_invalid_samples) &&                 \
        (v_dataReaderInstanceOldest(_this) == NULL && v_dataReaderInstanceStateTest(_this, L_STATECHANGED))) {  \
        printf("Warning at line %d empty instance while STATECHANGE has occurred\n", __LINE__);                 \
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
 * Compares two sequence-/serial numbers according to RFC 1982.
 *
 * @param i1 The first comparand
 * @param i2 The second comparand
 * @return
 *      C_EQ if i1 == i2,
 *      C_LT if (i1 < i2 && i2 - i1 < 2^31) || (i1 > i2 && i1 - i2 > 2^31),
 *      C_GT if (i1 < i2 && i2 - i1 > 2^31) || (i1 > i2 && i1 - i2 < 2^31)
 */
static c_equality
seqNrCompare(
    c_ulong i1,
    c_ulong i2)
{
    c_long distance = (c_long)(i1 - i2);
    if(distance == 0)
        return C_EQ;
    else if(distance < 0)
        return C_LT;
    else /* distance > 0 */
        return C_GT;
}

/**
 * Returns the relative order of a v_message to a v_historyBookmark.
 * @param m The address of a v_message
 * @param b A v_historyBookmark
 * @return C_EQ, C_LT or C_GT if m is respectively equal, less or greater than b
 */
#define v__dataReaderInstanceSampleCompareMessageWithBookmark(m, b) \
    v__dataReaderInstanceSampleCompare(\
            (m)->writeTime, \
            (m)->writerGID, \
            (m)->sequenceNumber, \
            (b).sourceTimestamp, \
            (b).gid, \
            (b).sequenceNumber)

/**
 * Returns the relative order two v_message's.
 * @param m1 The address of a v_message
 * @param m2 The address of a v_message
 * @return C_EQ, C_LT or C_GT if m1 is respectively equal, less or greater than m2
 */
#define v__dataReaderInstanceSampleCompareMessages(m1, m2) \
    ((m1) == (m2) ? C_EQ : v__dataReaderInstanceSampleCompare(\
            (m1)->writeTime, \
            (m1)->writerGID, \
            (m1)->sequenceNumber, \
            (m2)->writeTime, \
            (m2)->writerGID, \
            (m2)->sequenceNumber))

/**
 * Returns the relative order based on a comparison on time, gid and
 * sequenceNumber.
 * @return C_EQ, C_LT or C_GT if parameter 1 (3-tuple) is respectively equal,
 * less or greater than parameter 2 (3-tuple)
 */
static c_equality
v__dataReaderInstanceSampleCompare(
    c_time t1,
    v_gid g1,
    c_ulong i1,
    c_time t2,
    v_gid g2,
    c_ulong i2)
{
    c_equality e;

    if((e = c_timeCompare(t1, t2)) != C_EQ)
        return e;
    else if((e = v_gidCompare(g1, g2)) != C_EQ)
        return e;
    else
        return seqNrCompare(i1, i2);
}

static void
v_dataReaderInstanceCheckCount(
    v_dataReaderInstance _this)
{
    c_long writeFound = 0;
    c_long totalFound = 0;
    v_dataReaderSample currentSample;

    assert(C_TYPECHECK(_this, v_dataReaderInstance));
    assert(_this->sampleCount >= _this->accessibleCount);

    currentSample = v_dataReaderInstanceOldest(_this);

    while (currentSample != NULL) {
        totalFound++;
        if (v_dataReaderSampleMessageStateTest(currentSample, L_WRITE)) {
            writeFound++;
        }
        currentSample = ((v_dataReaderSample)currentSample)->newer;
    }
    assert(writeFound == _this->sampleCount);
}

static c_bool
writeSlave(
    c_object o,
    c_voidp arg)
{
    return v_dataViewWrite(v_dataView(o),v_readerSample(arg));
}

static v_message
CreateTypedInvalidMessage(
    v_dataReaderInstance _this,
    v_message untypedMsg)
{
    v_message typedMsg;

    /* Create a message for the invalid sample to carry. */
    typedMsg = v_dataReaderInstanceCreateMessage(_this);
    if (typedMsg)
    {
        /* Set correct attributes. */
        v_node(typedMsg)->nodeState = v_node(untypedMsg)->nodeState;
        typedMsg->writerGID = untypedMsg->writerGID;
        typedMsg->writeTime = untypedMsg->writeTime;
        typedMsg->writerInstanceGID = untypedMsg->writerInstanceGID;
        typedMsg->qos = c_keep(untypedMsg->qos);
        typedMsg->sequenceNumber = untypedMsg->sequenceNumber;
        typedMsg->transactionId = untypedMsg->transactionId;    }
    else
    {
        OS_REPORT_2(OS_ERROR,
                  "v_dataReaderInstance", 0,
                  "CreateTypedInvalidMessage(_this=0x%x, untypedMsg=0x%x)\n"
                  "        Operation failed to allocate new v_message: result = NULL.",
                  _this, untypedMsg);
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
    while (sample != bookmark)
    {
        /* Remember the next sample (in case the current one gets purged.) */
        next = sample->newer;

        /* Purge samples that are invalid and not part of an unfinished transaction. */
        if(!v_readerSampleTestStateOr(sample,L_TRANSACTION | L_VALIDDATA))
        {
            v_dataReaderSampleTake(sample, NULL, NULL);
        }

        /* Iterate to the next sample. */
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
        v_dataReaderSample sample,
        c_bool isTransactionFlush)
{
    v_message prevMsg, nextMsg;
    v_state prevMsgState, nextMsgState;
    v_dataReaderSample s;
    v_state msgState = v_nodeState(message);


    /* Valid samples that are part of a transaction that is still in progress do
     * count for resource limits, so update sample count no matter if this
     * sample is part of an ongoing transaction.
     */
    if (v_stateTest(msgState, L_WRITE)&& (!isTransactionFlush))
    {
        v_readerSampleSetState(sample, L_VALIDDATA);
        _this->sampleCount++;
        v_checkMaxSamplesPerInstanceWarningLevel(
                v_objectKernel(_this), _this->sampleCount);
    }

    /* Do not update states for an unfinished transaction. The update of
     * the states will be performed when the transaction is completed.
     */
    if (!v_readerSampleTestState(sample,L_TRANSACTION)){
        /* If a valid sample does not belong to a transaction, then
         * increase the accessibleCount variable.
         */
        if (v_stateTest(msgState, L_WRITE))
        {
            _this->accessibleCount++;
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
                                     v_instance(_this), sample->insertTime);
    }
}

static void
updateFinalInstanceAndSampleState(
    v_dataReaderInstance _this,
    v_message message,
    v_dataReaderSample sample,
    c_bool isTransactionFlush)
{
    v_reader reader = v_reader(v_index(_this->index)->reader);
    v_state msgState = v_nodeState(message);
    c_bool generationEnd = FALSE;

    /* Valid samples that are part of a transaction that is still in progress do
     * count for resource limits, so update sample count no matter if this
     * sample is part of an ongoing transaction. If a transaction is being
     * flushed, then the samples were already inserted before so that there
     * will be no change to the resource limits. However, a transaction that
     * is being flushed may still change the instance state.
     */
    if (v_stateTest(msgState, L_WRITE) && (!isTransactionFlush))
    {
        v_readerSampleSetState(sample, L_VALIDDATA);
        _this->sampleCount++;
        v_checkMaxSamplesPerInstanceWarningLevel(
                v_objectKernel(_this), _this->sampleCount);
    }


    /* Do not update states for an unfinished transaction. The update of
     * the states will be performed when the transaction is completed.
     */
    if (!v_readerSampleTestState(sample,L_TRANSACTION))
    {
        /* If a valid sample does not belong to a transaction, then
         * increase the accessibleCount variable.
         */
        if (v_stateTest(msgState, L_WRITE))
        {
            _this->accessibleCount++;
        }

        /* If the instance is empty and the reader has subscriber defined keys,
         * then the instance must raise its L_NEW flag.
         */
        if (v_dataReaderInstanceStateTest(_this, L_EMPTY) &&
                reader->qos->userKey.enable)
        {
            v_dataReaderInstanceStateSet(_this, L_NEW);
        }


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
            if (_this->liveliness > 0 || reader->qos->userKey.enable)
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
        if (!v_stateTestOr(msgState, L_DISPOSED | L_UNREGISTER)) {
            _this->lastInsertionTime = sample->insertTime;
        }

        /* Now process all flags that have currently been raised on the
         * message. Since the sample belongs to the most recent lifecycle,
         * its flags fully determine the instance state.
         */
        if (v_stateTest(msgState, L_DISPOSED))
        {
            /* Re-initialize the time-based filter */
            _this->lastInsertionTime = C_TIME_MIN_INFINITE;

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
                        reader->qos->lifecycle.enable_invalid_samples ||
                        hasValidSampleAccessible(_this))
                {
                    v_dataReaderInstanceStateSet(_this, L_TRIGGER);
                }
                generationEnd = TRUE; /* Indicate that this generation has ended. */
            }
        }
        if (v_stateTest(msgState, L_UNREGISTER))
        {
            /* Unregister messages are not expected to contain valid data. */
            assert(v_stateTestNot(msgState, L_WRITE));

            /* Re-initialize the time-based filter */
            _this->lastInsertionTime = C_TIME_MIN_INFINITE;

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
                        reader->qos->lifecycle.enable_invalid_samples ||
                        hasValidSampleAccessible(_this))
                {
                    v_dataReaderInstanceStateSet(_this, L_TRIGGER);
                }
                generationEnd = TRUE; /* Indicate that this generation has ended. */
            }
            v_instanceRemove(v_instance(_this)); /* deadline */
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
        if (v_dataReaderInstanceStateTest(_this, L_TRIGGER))
        {
            v_dataReader(reader)->notReadCount++;
        }
        else
        {
            v_readerSampleSetState(sample, L_READ);
        }
    }
    /* In case this generation is ended, remove the readerInstance from the deadline list.
     * Otherwise update the instance in the deadline list.
     */
    if (generationEnd) {
        v_instanceRemove(v_instance(_this));
    } else {
        v_deadLineInstanceListUpdate(v_dataReader(reader)->deadLineList,
                                     v_instance(_this), sample->insertTime);
    }
}


static v_dataReaderResult
InsertSample(
    v_dataReaderInstance _this,
    v_message message,
    v_dataReaderSample *sample)
{
    v_dataReaderSample s, oldest;
    v_reader reader;
    v_readerQos qos;
    v_message m;
    v_state messageState;
    c_long depth;
    c_equality equality;
    c_bool proceed;

    /* Only insert the sample if it is newer than any history that has
     * already been consumed so far. This is to prevent old data from
     * re-appearing when newer data has already been consumed. (Note that
     * in case of equal timestamps order is determined by the writerGID and
     * then by sequence-number.)
     */
    reader = v_reader(v_index(_this->index)->reader);
    qos = reader->qos;
    messageState = v_nodeState(message);
    if (qos->orderby.kind == V_ORDERBY_SOURCETIME)
    {
        equality = v__dataReaderInstanceSampleCompareMessageWithBookmark(message, _this->lastConsumed);
        if (equality == C_EQ) {
            CHECK_COUNT(_this);
            return V_DATAREADER_DUPLICATE_SAMPLE;
        } else if (equality == C_LT) {
            CHECK_COUNT(_this);
            return V_DATAREADER_SAMPLE_LOST;
        }
    }

    /* Find the location where the sample needs to be inserted, and change
     * the list accordingly.
     */
    if (v_stateTest(_this->instanceState, L_EMPTY))
    {
        assert(_this->sampleCount == 0);
        assert(v_dataReaderInstanceOldest(_this) == NULL);
        assert(v_dataReaderInstanceNewest(_this) == NULL);
        assert(v_dataReaderInstanceEmpty(_this));

        *sample = v_dataReaderSampleNew(_this, message);
        if (*sample)
        {
            /* This sample is the only sample available, so it fully determines
             * the instance and sample state.
             */
            v_dataReaderInstanceSetOldest(_this, *sample);
            v_dataReaderInstanceSetNewest(_this, *sample);
            updateFinalInstanceAndSampleState(_this, message, *sample, FALSE);
            v_dataReaderInstanceStateClear(_this, L_EMPTY);
        }
        else
        {
            OS_REPORT_3(OS_ERROR,
                      "v_dataReaderInstance", 0,
                      "InsertSample(_this=0x%x, message=0x%x, *sample=0x%x)\n"
                      "        Unable to allocate v_dataReaderSample.",
                      _this, message, *sample);
            return V_DATAREADER_OUT_OF_MEMORY;
        }
    }
    else
    {
        depth = v_dataReader(reader)->depth;
        assert(_this->sampleCount <= depth);

        /* Find insertion point in history.
         * Start at the newest (last inserted) sample and
         * walk to the oldest (first inserted).
         */
        proceed = TRUE;
        s = v_dataReaderInstanceNewest(_this);
        while (s != NULL && proceed)
        {
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
             * timestamps will be sorted by their GID values.
             */
            equality = v__dataReaderInstanceSampleCompareMessages(message, m);
            if ( equality == C_EQ )
            {
                return V_DATAREADER_DUPLICATE_SAMPLE;
            }
            else
            {
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
                if (qos->orderby.kind != V_ORDERBY_SOURCETIME){
                    equality = C_GT;
                    /* Store at the tail (newest message). */
                }
                if (equality == C_GT)
                {
                    /* message is newer than the current position s,
                     * insert message at the newer side of s. So check
                     * whether there are enough resources left to store
                     * the sample. Be aware the invalid samples (samples
                     * without the L_WRITE flag) do not consume resources.
                     */
                    if (_this->sampleCount < depth ||
                            !v_stateTest(messageState, L_WRITE))
                    {
                        *sample = v_dataReaderSampleNew(_this, message);
                        if (*sample) {
                            /* Insert after s. */
                            (*sample)->older = s;
                            (*sample)->newer = s->newer;
                            s->newer = *sample;
                            if ((*sample)->newer)
                            {
                                /* sample != newest, so this sample only partially
                                 * determines the instance state.
                                 */
                                assert(v_dataReaderSample(s->newer)->older == s);
                                v_dataReaderSample((*sample)->newer)->older = *sample;
                                updateIntermediateInstanceAndSampleState(_this, message, *sample, FALSE);
                            }
                            else
                            {
                                /* s = newest, so this sample fully determines the
                                 * instance state.
                                 */
                                v_dataReaderInstanceSetNewest(_this, *sample);
                                updateFinalInstanceAndSampleState(_this, message, *sample, FALSE);
                            }
                        }
                        else
                        {
                            OS_REPORT_3(OS_ERROR,
                                      "v_dataReaderInstance", 0,
                                      "InsertSample(_this=0x%x, message=0x%x, *sample=0x%x)\n"
                                      "        Unable to allocate v_dataReaderSample.",
                                      _this, message, *sample);
                            return V_DATAREADER_OUT_OF_MEMORY;
                        }
                        proceed = FALSE;
                    }
                    else
                    {
                        /* If the history of the reader is already filled up AND
                         * the sample contains valid data (both already determined
                         * above) AND the history kind is KEEP_ALL, then the sample
                         * must be rejected.
                         */
                        if (qos->history.kind == V_HISTORY_KEEPALL)
                        {
                            return V_DATAREADER_INSTANCE_FULL;
                        }
                        else
                        {
                            *sample = v_dataReaderSampleNew(_this, message);
                            if (*sample)
                            {
                                /* Insert after s. */
                                (*sample)->older = s;
                                (*sample)->newer = s->newer;
                                s->newer = *sample;
                                if ((*sample)->newer)
                                {
                                    /* s != newest, so this sample only partially
                                     * determines the instance state.
                                     */
                                    assert(v_dataReaderSample(s->newer)->older == s);
                                    v_dataReaderSample((*sample)->newer)->older = *sample;
                                    updateIntermediateInstanceAndSampleState(_this, message, *sample, FALSE);
                                }
                                else
                                {
                                    /* s = newest, so this sample fully determines the
                                     * instance state.
                                     */
                                    v_dataReaderInstanceSetNewest(_this, *sample);
                                    updateFinalInstanceAndSampleState(_this, message, *sample, FALSE);
                                }

                                /* Push out the oldest sample in the history
                                 * until we've at least removed one L_WRITE
                                 * message, because only those ones are taken
                                 * into account for depth (called sampleCount
                                 * here).
                                 */
                                do
                                {
                                    oldest = v_dataReaderInstanceOldest(_this);
                                    if (*sample == oldest)
                                    {
                                        *sample = NULL;
                                        proceed = FALSE;
                                    }
                                    else if (v_readerSampleTestState(oldest, L_VALIDDATA))
                                    {
                                        proceed = FALSE;
                                    }
                                    v_dataReaderInstanceSampleRemove(_this, oldest);
                                } while(proceed);

                                v_statisticsULongValueInc(v_reader,
                                                          numberOfSamplesDiscarded,
                                                          reader);
                            }
                            else
                            {
                                OS_REPORT_3(OS_ERROR,
                                          "v_dataReaderInstance", 0,
                                          "InsertSample(_this=0x%x, message=0x%x, *sample=0x%x)\n"
                                          "        Unable to allocate v_dataReaderSample.",
                                          _this, message, *sample);
                                return V_DATAREADER_OUT_OF_MEMORY;
                            }
                        }
                    }
                }
                else
                {
                    /* The message is older than s. */
                    if (s->older)
                    {
                        /* more candidate older samples to go... */
                        s = s->older;
                    }
                    else
                    {
                        /*
                         * Check if there is enough storage space for the sample,
                         * or whether the sample is invalid, in which case it
                         * does not consume any of the resource limits.
                         */
                        if ( _this->sampleCount < depth ||
                                !v_stateTest(messageState, L_WRITE))
                        {
                            *sample = v_dataReaderSampleNew(_this, message);
                            if (*sample)
                            {
                                /* Insert before s. */
                                s->older = *sample;
                                (*sample)->newer = s;
                                (*sample)->older = NULL;
                                v_dataReaderInstanceSetOldest(_this, *sample);

                                /* s != newest, so this sample only partially
                                 * determines the instance state.
                                 */
                                updateIntermediateInstanceAndSampleState(_this, message, *sample, FALSE);
                            }
                            else
                            {
                                OS_REPORT_3(OS_ERROR,
                                          "v_dataReaderInstance", 0,
                                          "InsertSample(_this=0x%x, message=0x%x, *sample=0x%x)\n"
                                          "        Unable to allocate v_dataReaderSample.",
                                          _this, message, *sample);
                                return V_DATAREADER_OUT_OF_MEMORY;
                            }
                        }
                        else
                        {
                            /* incoming message is oldest and no history space left, so discard */
                            if (qos->history.kind == V_HISTORY_KEEPALL)
                            {
                                /*
                                 * Since the sample is older than all history currently
                                 * in the queue, it doesn't make sense to retransmit
                                 * this sample. Once data has been consumed and space
                                 * is available, the 'lastConsumed' timestamp will have
                                 * increased, resulting in the sample to be refused
                                 * because of this new value of 'lastConsumed'. Since
                                 * that causes the sample to be considered 'lost', we
                                 * might as well save the trouble of retransmitting and
                                 * mark it as 'lost' right now.
                                 */
                                return V_DATAREADER_SAMPLE_LOST;
                            }
                            else
                            {
                                *sample = NULL; /* prevent instance state update later on */
                                v_statisticsULongValueInc(v_reader,
                                                          numberOfSamplesDiscarded,
                                                          reader);
                            }
                        }
                        proceed = FALSE; /* exit while loop. */
                    }
                }
            }
        }
    }

    return V_DATAREADER_INSERTED;
}

void
v_dataReaderInstanceInit (
    v_dataReaderInstance _this,
    v_message message)
{
    c_array instanceKeyList;
    c_array messageKeyList;
    c_value value;
    c_long i, nrOfKeys;
    v_index index;
    v_readerQos qos;

    assert(_this);
    assert(message);
    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    assert(C_TYPECHECK(message,v_message));

    index = _this->index;

    assert(_this->sampleCount == 0);

    qos = v_reader(index->reader)->qos;

    /* The first insert will increase to 0 times noWriter state. */
    v_instanceInit(v_instance(_this));
    _this->instanceState                = L_EMPTY | L_NOWRITERS;
    _this->noWritersCount               = -1;
    _this->disposeCount                 = 0;
    _this->sampleCount                  = 0;
    _this->accessibleCount              = 0;
    _this->liveliness                   = 0;
    _this->hasBeenAlive                 = FALSE;
    _this->epoch                        = C_TIME_MIN_INFINITE;
    _this->lastConsumed.sourceTimestamp = C_TIME_MIN_INFINITE;
    v_gidSetNil(_this->lastConsumed.gid);
    _this->lastConsumed.sequenceNumber  = 0;
    _this->purgeInsertionTime           = C_TIME_MIN_INFINITE;
    _this->userDataDataReaderInstance   = NULL;
    _this->lastInsertionTime            = C_TIME_MIN_INFINITE;
    v_dataReaderInstanceSetOldest(_this,NULL);
    v_dataReaderInstanceSetNewest(_this,NULL);



    /* only if ownership is exclusive the owner must be set! */
     if (qos->ownership.kind == V_OWNERSHIP_EXCLUSIVE) {
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
v_dataReaderInstanceSetEpoch (
    v_dataReaderInstance _this,
    c_time time)
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
    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    CHECK_COUNT(_this);

    v_instanceDeinit(v_instance(_this));
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
        (v_gidIsValid(_this->owner.gid))) {
        result = V_WRITE_SUCCESS;
    } else {
        entry = v_dataReaderEntry(v_index(_this->index)->entry);
        thisPtr = &_this;
        result = v_dataReaderEntryWrite(entry, msg, (v_instance *)thisPtr);
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
    c_long i, nrOfKeys;

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
        }
        else
        {
            OS_REPORT_1(OS_ERROR,
                      "v_dataReaderInstance", 0,
                      "v_dataReaderInstanceCreateMessage(_this=0x%x)\n"
                      "        Operation failed to allocate new topicMessage: result = NULL.", _this);
            assert(FALSE);
        }
    }
    return message;
}

v_dataReaderResult
v_dataReaderInstanceInsert(
    v_dataReaderInstance _this,
    v_message message)
{
    v_dataReaderSample sample, latestDisposedSample, nextSample;
    v_index index;
    v_state messageState;
    c_equality equality;
    v_dataReader reader;
    v_readerQos qos;
    v_dataReaderResult result = V_DATAREADER_UNDETERMINED;
    c_bool found;
    struct v_owner ownership;

    assert(message != NULL);
    assert(_this != NULL);
    assert(C_TYPECHECK(message,v_message));
    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    index = v_index(_this->index);
    reader = v_dataReader(index->reader);
    qos = v_reader(reader)->qos;
    sample = NULL;

    messageState = v_nodeState(message);

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
    if ( v_stateTest(messageState, L_REPLACED) && 
         v_dataReaderInstanceStateTest(_this, L_REPLACED)) {
        /* The message is first message that in injected in 
         * the reader instance due to the replace merge policy.
         * Now remove the marker from the reader instance state.
         */
        v_dataReaderInstanceStateClear(_this, L_REPLACED);
        /* Now purge all samples up to the latest dispose.
         * First find the latest disposed message marked with the specified flags
         */
        sample = v_dataReaderInstanceNewest(_this);
        found = FALSE;
        while ( (sample != NULL) && ! found ) {
            if (v_readerSampleTestState(sample, L_DISPOSED) &&
                v_readerSampleTestState(sample, L_REPLACED)) {
                found = TRUE;
            } else {
                sample = sample->older;
            }
        }
        if (found) {
            /* The latest DISPOSE message with the L_REPLACE flag has been found.
             * This DISPOSE message represents the moment at which a REPLACE merge
             * policy is applied. By definition of the REPLACE merge policy, all
             * existing historical data needs to be replaced with historical data
             * that it gets aligned. This implies that all existing historical data
             * can be discarded. To discard simpy take the samples, starting with
             * the oldest.
             */
            latestDisposedSample = sample;
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

    /*
     * Filter out messages that still belong to a previous lifecycle of this
     * 'recycled' v_dataReaderInstance object. The epoch time determines when
     * the previous lifecycle ended, so everything older than that can be
     * discarded.
     */
    if (qos->orderby.kind == V_ORDERBY_SOURCETIME) {
        equality = c_timeCompare(message->writeTime,_this->epoch);
        if (equality != C_GT) {
            CHECK_COUNT(_this);
            return V_DATAREADER_OUTDATED;
        }
    }

    /* All kinds of messages need to update alive writers.
     * The alive writers must always be updated even when the writer is
     * not owner, it remains a writer that is alive.
     */
    if (v_stateTest(messageState, L_REGISTER)) {
        _this->liveliness++;
        return V_DATAREADER_INSERTED;
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (v_stateTest(messageState, L_UNREGISTER))
    {
        if (_this->liveliness > 0)
        {
            _this->liveliness--;
            if (_this->liveliness == 0)
            {
                /* Create an invalid sample as sample info carrier
                 * to pass state change at next read or take operation.
                 * An UNREGISTER message can also have its DISPOSE flag
                 * set, but any other combination is illegal. When an
                 * instance gets disconnected from the group (for example
                 * because its writer is lost, or because the reader changes
                 * its partition), then the UNREGISTER message might also
                 * have its DISPOSE flag set in case the owner of the instance
                 * has its auto_dispose_unregisterd_instances set to TRUE.
                 * Since the last to unregister is by definition also
                 * the owner of the instance, it is safe to process this
                 * DISPOSE event as well.
                 */
                result = InsertSample(_this, message, &sample);
            }
            else
            {
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
                if (!v_stateTest(messageState, L_DISPOSED))
                {
                    result = V_DATAREADER_INSERTED;
                }
            }
        }
        else
        {
            /* Unregister messages can be received when liveliness == 0 when
             * the Reader disconnects from a group. In that case each instance
             * will be unregistered again, even the ones that have liveliness
             * set to 0 already. Just ignore the message in that case.
             */
            return V_DATAREADER_INSERTED;
        }
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (v_gidIsValid (message->writerGID)) {
        assert (message->qos != NULL);
        ownership.exclusive = v_messageQos_isExclusive(message->qos);
        ownership.strength = v_messageQos_getOwnershipStrength(message->qos);
    }

    ownership.gid = message->writerGID;

    /*
     * Test if ownership is exclusive and whether the writer identified
     * in the message should become owner. In case of an invalid GID
     * ownership is always assumed. (For example in case of disposeAll.)
     */
    switch (v_determineOwnershipByStrength (
        &_this->owner, &ownership, _this->liveliness > 0))
    {
        case V_OWNERSHIP_INCOMPATIBLE_QOS:
            assert (ownership.exclusive != TRUE);
            /* fall through. A writer with an incompatible QoS is by definition
               never owner of the instance. */
        case V_OWNERSHIP_NOT_OWNER:
            return V_DATAREADER_NOT_OWNER;
            break;
        case V_OWNERSHIP_ALREADY_OWNER:
            /* If the writer indicates will no longer update (own) this
               instance (by sending an unregister message) then the ownership
               is released by resetting the owner gid to nil. */
            if (v_stateTest (messageState, L_UNREGISTER)) {
                v_gidSetNil (_this->owner.gid);
            }
            /* fall through */
        default:
            break;
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    /* Filter out messages that violate the time based filter minimum separation time,
     * if the message state is not disposed or unregister and the time_based_filter QoS is enabled
     * Messages part of a transaction are filtered if/when a transaction flush occurs.
     */
    if (!(v_stateTestOr(messageState, L_DISPOSED | L_UNREGISTER | L_TRANSACTION)) &&
        (c_timeCompare(qos->pacing.minSeperation, C_TIME_ZERO) != C_EQ)) {
#ifdef _NAT_
        equality = c_timeCompare(v_timeGet(),
            c_timeAdd(_this->lastInsertionTime, qos->pacing.minSeparation));
#else
        equality = c_timeCompare(message->allocTime,
            c_timeAdd(_this->lastInsertionTime, qos->pacing.minSeperation));
#endif /* _NAT_ */
        if (equality == C_LT) {
            CHECK_COUNT(_this);
            return V_DATAREADER_FILTERED_OUT;
        }
    }

    /* When no message has been inserted yet, handle all other types of messages. */
    if (result == V_DATAREADER_UNDETERMINED)
    {
        result = InsertSample(_this, message, &sample);
        if (result != V_DATAREADER_INSERTED)
        {
            return result;
        }
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    /* Write the sample into all dataViews attached to this reader, but only
     * if the sample contains valid data.
     */
    if ((reader->views != NULL) &&
            (sample != NULL) &&
            v_stateTest(messageState, L_WRITE))
    {
        c_walk(reader->views,writeSlave,sample);
    }
    CHECK_EMPTINESS(_this);
    CHECK_COUNT(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (v_dataReaderInstanceStateTest(_this, L_TRIGGER) &&
        (v_dataReader (reader)->notReadTriggerThreshold <= 0 ||
         v_dataReader (reader)->notReadTriggerCount++ == v_dataReader (reader)->notReadTriggerThreshold))
    {
        V_MESSAGE_STAMP(message,readerDataAvailableTime);

        V_MESSAGE_STAMP(message, readerInstanceTime);
        v_dataReader (reader)->notReadTriggerCount = 0;
        v_dataReaderInstanceStateClear(_this, L_TRIGGER);
        v_dataReaderNotifyDataAvailable(reader, sample);
    }

    /* reader internal state of the data has been modified.
     * so increase readers update count.
     * This value is used by queries to determine if a query
     * needs to be re-evaluated.
     */
    v_dataReader(reader)->updateCnt++;

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);
    assert(_this->sampleCount <= reader->depth);

    return V_DATAREADER_INSERTED;
}

static c_bool
setWalkRequired(
    c_object query,
    c_voidp arg)
{
    v_dataReaderQuery q = v_dataReaderQuery(query);
    OS_UNUSED_ARG(arg);

    q->walkRequired = TRUE;

    return TRUE;
}

void
v_dataReaderInstanceFlushTransaction(
    v_dataReaderInstance _this,
    c_ulong transactionId)
{
    v_dataReaderSample sample, nextSample;
    v_dataReader reader;
    c_bool found;

    sample = v_dataReaderInstanceOldest(_this);

    while (sample != NULL) {
        if (v_readerSampleTestState(sample,L_TRANSACTION)) {
            if (v_dataReaderSampleMessage(sample)->transactionId == transactionId) {
                v_readerSampleClearState(sample,L_TRANSACTION);
                nextSample = sample->newer;
                found = FALSE;

                /* Determine whether there is a newer sample that is not
                 * part of an (unfinished) transaction to determine
                 * whether the intermediate or final instance state needs to
                 * be applied.
                 */
                while(nextSample && !found)
                {
                    if(!v_readerSampleTestState(nextSample,L_TRANSACTION))
                    {
                        found = TRUE;
                    } else
                    {
                        nextSample = nextSample->newer;
                    }
                }

                if(found){
                    /* Found a newer sample, so apply intermediate state. */
                    updateIntermediateInstanceAndSampleState(_this,
                            v_dataReaderSampleMessage(sample), sample, TRUE);
                } else {
                    /* Apply final state otherwise. */
                    updateFinalInstanceAndSampleState(_this,
                            v_dataReaderSampleMessage(sample), sample, TRUE);

                    /* Obtain the reader so we can set the walkRequired flag
                     * on each of the attached queries - see OSPL-1013 */
                    reader = v_dataReaderInstanceReader(_this);
                    if (reader != NULL)
                    {
                        c_walk(v_collection(reader)->queries,setWalkRequired,NULL);
                    }
                }
            }
        }
        sample = sample->newer;
    }

    /* Ensure that the L_TRIGGER state of the instance is reset, since
     * transactionListUpdate calls v_dataReaderNotifyDataAvailable: */
    v_dataReaderInstanceStateClear(_this, L_TRIGGER);
}

void
v_dataReaderInstanceAbortTransaction(
    v_dataReaderInstance _this,
    c_ulong transactionId)
{
    v_dataReaderSample sample;
    c_ulong tid;

    if (transactionId != 0) {
        sample = v_dataReaderInstanceOldest(_this);
        while (sample != NULL) {
            if (v_readerSampleTestState(sample,L_TRANSACTION)) {
                tid = v_dataReaderSampleMessage(sample)->transactionId;
                if (tid == transactionId) {
                    /* Remove sample from history. */
                    if (sample->older) {
                        assert(v_dataReaderInstanceOldest(_this) != sample);
                        v_dataReaderSample(sample->older)->newer = sample->newer;
                    } else {
                        /* _this = head */
                        assert(v_dataReaderInstanceOldest(_this) == sample);
                        v_dataReaderInstanceSetOldest(_this,sample->newer);
                    }
                    if (sample->newer) {
                        assert(v_dataReaderInstanceNewest(_this) != sample);
                        v_dataReaderSample(sample->newer)->older = sample->older;
                    } else {
                        /* sample = newest */
                        assert(v_dataReaderInstanceNewest(_this) == sample);
                        v_dataReaderInstanceSetNewest(_this,sample->older);
                    }
                    sample->newer = NULL;
                    sample->older = NULL;
                    v_dataReaderSampleRemoveFromLifespanAdmin(sample);
                    v_dataReaderSampleFree(sample);
                }
            }
            sample = sample->newer;
        }
    }
}

c_bool
v_dataReaderInstanceTest(
    v_dataReaderInstance _this,
    c_query query,
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
            assert(!v_readerSampleTestState(sample, L_VALIDDATA) ||
                    v_readerSampleTestState(sample,L_TRANSACTION));

            /* Samples that are part of an unfinished transaction may not
             * be shown. This algorithm must ensure that it provides the
             * latest DISPOSE that is not part of a transaction or the
             * latest UNREGISTER if no DISPOSE is applicable.
             *
             * As a 1st step find the newest sample that is not part
             * of an unfinished transaction.
             */
            while(v_readerSampleTestState(sample,L_TRANSACTION) &&
                    sample->older)
            {
                sample = sample->older;
            }
            /* See if we found an invalid sample that is not part of an
             * unfinished transaction.
             */
            if (!v_readerSampleTestState(sample,L_TRANSACTION))
            {
                msg = v_dataReaderSampleMessage(sample);
                msgState = v_nodeState(msg);

                /* Check if our sample is a DISPOSE event. */
                if((!v_stateTest(msgState, L_DISPOSED)) && (sample->older))
                {
                    prevSample = sample->older;

                    /* Find the previous event that is not part of an
                     * unfinished transaction.
                     */
                    while(v_readerSampleTestState(prevSample,L_TRANSACTION) &&
                            prevSample->older)
                    {
                        prevSample = prevSample->older;
                    }
                    msg = v_dataReaderSampleMessage(prevSample);
                    msgState = v_nodeState(msg);

                    /* If our found event is not part of an unfinished
                     * transaction and is a DISPOSE event, this is the
                     * one we need to present. If not, the originally
                     * found UNREGISTER is the one to present.
                     */
                    if ((!v_readerSampleTestState(prevSample,L_TRANSACTION)) &&
                        v_stateTest(msgState, L_DISPOSED))
                    {
                        sample = prevSample;
                    }
                }

                /* Only pass this sample if it has not been READ before and if
                 * the reader has not disabled invalid samples.
                 */
                r = v_dataReaderInstanceReader(_this);
                if (!v_readerSampleTestStateOr(sample, L_READ | L_LAZYREAD) &&
                        v_reader(r)->qos->lifecycle.enable_invalid_samples)
                {
                    sampleSatisfies = action(sample, args);
                }
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
         * when accessibleCount > 0.
         */
        if (v_readerSampleTestState(sample, L_VALIDDATA))
        {
            /* If a query has been passed, evaluate the sample against the
             * query. If not, then make sure the sample is evaluated against
             * the action routine.
             */
            if (query)
            {
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
            }
            else
            {
                sampleSatisfies = TRUE;
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
    c_type sampleType;
    v_state state;
    v_state mask;
    v_actionResult result;

    orgSample = NULL;
    instance = v_dataReaderSampleInstance(sample);

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
    V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample), readerCopyTime);
    V_MESSAGE_REPORT(v_dataReaderSampleMessage(sample),
                     v_dataReaderInstanceDataReader(instance));

    /* If the message was temporarily switched, switch it back. */
    if (!v_readerSampleTestState(sample, L_VALIDDATA))
    {
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
        /* reader internal state of the data has been modified.
         * so increase readers update count.
         */
        v_dataReaderInstanceReader(instance)->updateCnt++;

        /* The instance state can have changed, so update the statistics */
        v_statisticsULongValueInc(v_reader,
                                  numberOfSamplesRead,
                                  v_dataReaderInstanceReader(instance));

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
                    v_reader(r)->qos->lifecycle.enable_invalid_samples)
            {
                /* No valid samples exist, so there must be at least one invalid
                 * sample that still needs to be processed. So walk through all
                 * samples and skip the ones that are part of an unfinished
                 * transaction or that have been accessed previously.
                 */
                sample = v_dataReaderInstanceOldest(_this);
                assert(sample);
                assert(!v_readerSampleTestState(sample, L_VALIDDATA) ||
                        v_readerSampleTestState(sample, L_TRANSACTION));
                while(sample != NULL &&
                        v_readerSampleTestStateOr(sample,L_TRANSACTION | L_READ | L_LAZYREAD))
                {
                    sample = sample->newer;
                }
                /* If a sample is found matching the criteria, then consume it.
                 */
                if (sample)
                {
                    result = v_dataReaderSampleRead(sample, action, arg);

                    /* Reset the event counters for all remaining invalid samples in this
                     * instance if they have communicated their instance state change.
                     */
                    if (v_actionResultTestNot(result, V_SKIP))
                    {
                        invalidSampleResetEventCounters(v_dataReaderInstanceOldest(_this), r);
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
            if (!v_readerSampleTestState(sample,L_TRANSACTION) &&
                (sample->readId != readId))
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
                }
                else
                {
                    /* queries on invalid data are not applicable,
                     * so set satisfies to TRUE. When no query present
                     * the sample also satisfies.
                     */
                    sampleSatisfies = TRUE;
                }
                /* Only pass samples that match and that also have valid data. */
                if (sampleSatisfies && v_readerSampleTestState(sample, L_VALIDDATA))
                {
                    sample->readId = readId;
                    result = v_dataReaderSampleRead(sample, action, arg);
                    if (v_actionResultTestNot(result, V_SKIP))
                    {
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
    v_dataReaderSample sample)
{
    v_message msg = v_dataReaderSampleMessage(sample);

    assert(_this->sampleCount >= 0);
    if (v_readerSampleTestState(sample, L_VALIDDATA))
    {
        CHECK_COUNT(_this);
        _this->sampleCount--;
        if (!v_readerSampleTestState(sample, L_TRANSACTION))
        {
            /* As long as the L_TRANSACTION bit is set, the accessibleCount has
             * not been increased and therefore it should not be decreased if the
             * sample is removed */
            _this->accessibleCount--;
        }
        assert (_this->accessibleCount >= 0);
    }
    /* Remove sample from history. */
    if (sample->older)
    {
        assert(v_dataReaderInstanceOldest(_this) != sample);
        v_dataReaderSample(sample->older)->newer = sample->newer;
    }
    else
    {
        /* sample = oldest */
        assert(v_dataReaderInstanceOldest(_this) == sample);
        v_dataReaderInstanceSetOldest(_this,sample->newer);
    }
    if (sample->newer)
    {
        assert(v_dataReaderInstanceNewest(_this) != sample);
        v_dataReaderSample(sample->newer)->older = sample->older;
    }
    else
    {
        /* sample = newest */
        assert(v_dataReaderInstanceNewest(_this) == sample);
        v_dataReaderInstanceSetNewest(_this,sample->older);
    }
    CHECK_INSTANCE_CONSISTENCY(_this);

    /* If instance becomes empty, then modify states accordingly. */
    if (v_dataReaderInstanceOldest(_this) == NULL)
    {
        v_dataReaderInstanceStateClear(_this, L_NEW);
        v_dataReaderInstanceStateClear(_this, L_STATECHANGED);
        v_dataReaderInstanceStateSet(_this, L_EMPTY);
    }

    /* Remove the sample from all administrations. */
    sample->newer = NULL;
    sample->older = NULL;
    v_dataReaderSampleWipeViews(sample);
    v_dataReaderSampleRemoveFromLifespanAdmin(sample);
    if (!v_readerSampleTestStateOr(sample, L_READ | L_LAZYREAD | L_TRANSACTION))
    {
        v_dataReader r = v_dataReaderInstanceReader(_this);
        r->notReadCount--;
    }

    /* If the consumed sample is newer than the previously consumed
     * sample, then update the history bookmark to indicate that
     * all samples prior to the current sample have been consumed.
     */
    if (v__dataReaderInstanceSampleCompareMessageWithBookmark(msg, _this->lastConsumed) == C_GT)
    {
        _this->lastConsumed.sourceTimestamp = msg->writeTime;
        _this->lastConsumed.gid = msg->writerGID;
        _this->lastConsumed.sequenceNumber = msg->sequenceNumber;
    }

    /* Free the sample itself. */
    v_readerSampleSetState(sample, L_REMOVED);
    v_dataReaderSampleFree(sample);
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
    c_type sampleType;
    v_state state;
    v_state mask;
    v_actionResult result = 0;
    v_dataReader r;

    orgSample = NULL;
    instance = v_dataReaderSampleInstance(sample);

    CHECK_COUNT(instance);
    CHECK_EMPTINESS(instance);
    CHECK_INSTANCE_CONSISTENCY(instance);

    r = v_dataReaderInstanceReader(instance);
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

        /* If the message was temporarily switched, switch it back. */
        if (!v_readerSampleTestState(sample, L_VALIDDATA))
        {
            c_free(sample);
            sample = orgSample;
        }
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


        if(action)
        {
            v_dataReaderInstanceStateClear(instance, L_NEW);
            v_dataReaderInstanceStateClear(instance, L_STATECHANGED);
        }

        v_dataReaderInstanceSampleRemove(instance, sample);

        if (v_dataReaderInstanceEmpty(instance))
        {
            assert(instance->sampleCount == 0);
            assert(instance->accessibleCount == 0);
            v_dataReaderInstanceStateSet(instance, L_EMPTY);
        }

        /* reader internal state of the data has been modified.
         * so increase readers update count.
         * This value is used by queries to determine if a query
         * needs to be re-evaluated.
         */
        v_dataReader(r)->updateCnt++;

        if (r->triggerValue) {
            v_dataReaderTriggerValueFree(r->triggerValue);
            r->triggerValue = NULL;
        }
    }

    /* The instance state can have changed, so update the statistics */
    UPDATE_READER_STATISTICS(v_index(instance->index), instance, state);

    CHECK_COUNT(instance);
    CHECK_EMPTINESS(instance);
    CHECK_INSTANCE_CONSISTENCY(instance);
    return result;
}

c_bool
v_dataReaderInstanceTakeSamples(
    v_dataReaderInstance _this,
    c_query query,
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
                    v_reader(r)->qos->lifecycle.enable_invalid_samples)
            {
                /* No valid samples exist, so there must be at least one invalid
                 * sample that still needs to be processed. So walk through all
                 * samples and skip the ones that are part of an unfinished
                 * transaction or that have been accessed previously.
                 */
                sample = v_dataReaderInstanceOldest(_this);
                assert(sample);
                assert(!v_readerSampleTestState(sample, L_VALIDDATA) ||
                        v_readerSampleTestState(sample, L_TRANSACTION));
                while(sample != NULL &&
                        v_readerSampleTestStateOr(sample,L_TRANSACTION | L_READ | L_LAZYREAD))
                {
                    sample = sample->newer;
                }
                /* If a sample is found matching the criteria, then consume it.
                 */
                if (sample)
                {
                    result = v_dataReaderSampleTake(sample, action, arg);

                    /* If the invalid sample has been able to communicate its instance
                     * state, then trash all the remaining invalid samples that are not
                     * part of an unfinished transaction (i.e. up to newest->next which
                     * is always NULL).
                     */
                    if (v_actionResultTestNot(result, V_SKIP))
                    {
                        invalidSamplesPurgeUntil(_this, NULL);
                    }
                }
            }
            else
            {
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
            if (!v_readerSampleTestState(sample,L_TRANSACTION) &&
                (sample->readId != readId))
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
                }
                else
                {
                    /* queries on invalid data are not applicable,
                     * so set satisfies to TRUE. When no query present
                     * the sample also satisfies.
                     */
                    sampleSatisfies = TRUE;
                }
                /* Only pass samples that match and that also have valid data. */
                if (sampleSatisfies)
                {
                    if (v_readerSampleTestState(sample, L_VALIDDATA))
                    {
                        sample->readId = readId;
                        result = v_dataReaderSampleTake(sample, action, arg);
                        if (v_actionResultTestNot(result, V_SKIP))
                        {
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
            invalidSampleResetEventCounters(sample, r);
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
                v_dataReaderInstanceSampleRemove(_this, sample);
                sample = nextSample;
            }
        }

        if (noWritersCount >= 0)
        {
            while ((sample != NULL) && (sample->noWritersCount <= noWritersCount))
            {
                v_dataReaderSample nextSample = sample->newer;
                v_dataReaderInstanceSampleRemove(_this, sample);
                sample = nextSample;
            }
        }
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);
}

c_voidp
v_dataReaderInstanceGetUserData(
    v_dataReaderInstance _this)
{
    c_voidp result = NULL;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this != NULL) {
        result = _this->userDataDataReaderInstance;
    }

    return result;
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
        result = (c_ulong) c_count(v_index(_this->index)->notEmptyList);
    }

    return result;
}


void
v_dataReaderInstanceSetUserData(
    v_dataReaderInstance _this,
    c_voidp userDataDataReaderInstance)
{
    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this != NULL) {
        _this->userDataDataReaderInstance = userDataDataReaderInstance;
    }
}

v_dataReaderResult
v_dataReaderInstanceUnregister (
    v_dataReaderInstance _this,
    v_registration unregistration,
    c_time timestamp)
{
    v_kernel kernel;
    v_dataReaderEntry entry;
    v_message msg = NULL;
    v_dataReaderResult result = V_DATAREADER_INSERTED;
    v_writeResult writeResult;
    v_dataReaderInstance* thisPtr;
    c_bool autoDispose = v_messageQos_isAutoDispose(unregistration->qos);
    assert(C_TYPECHECK(_this,v_dataReaderInstance));


    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

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
                /* Set the nodeState of the message to UNREGISTER. */
                v_stateSet(v_nodeState(msg), L_DISPOSED);
            }

            /* Insert the invalid message into the dataReaderInstance. */
            entry = v_dataReaderEntry(v_index(_this->index)->entry);
            thisPtr = &_this;
            writeResult = v_dataReaderEntryWrite(entry, msg, (v_instance *)thisPtr);
            c_free(msg);
            if (writeResult != V_WRITE_SUCCESS)
            {
                OS_REPORT_5(OS_ERROR,
                          "v_dataReaderInstance", 0,
                          "v_dataReaderInstanceUnregister(_this=0x%x, unregistration=0x%x, timestamp={%d,%d})\n"
                          "        Unable to insert invalid sample in v_dataReaderInstance: result = %s.",
                          _this, unregistration, timestamp.seconds, timestamp.nanoseconds,
                          v_dataReaderResultString(result));
                result = V_DATAREADER_INTERNAL_ERROR;
                assert(FALSE);
            }
        }
        else
        {
            OS_REPORT_4(OS_ERROR,
                      "v_dataReaderInstance", 0,
                      "v_dataReaderInstanceUnregister(_this=0x%x, unregistration=0x%x, timestamp={%d,%d})\n"
                      "        Unable to create invalid sample to indicate instance unregistration.",
                      _this, unregistration, timestamp.seconds, timestamp.nanoseconds);
            result = V_DATAREADER_OUT_OF_MEMORY;
            assert(FALSE);
        }
    }
    return result;
}
