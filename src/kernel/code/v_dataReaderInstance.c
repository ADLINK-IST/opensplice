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
#include "c_extent.h"
#include "os_report.h"
#include "v__kernel.h"

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
    if (v_dataReaderInstanceOldest(_this) == NULL && v_dataReaderInstanceStateTest(_this, L_STATECHANGED)){ \
        printf("Warning at line %d empty instance while L_STATECHANGED is set\n", __LINE__); \
    }
#else
#define CHECK_INSTANCE_CONSISTENCY(_this)
#endif

#ifndef NDEBUG
#define CHECK_COUNT(_this) v_dataReaderInstanceCheckCount(_this)
#else
#define CHECK_COUNT(_this)
#endif

static void
v_dataReaderInstanceCheckCount(
    v_dataReaderInstance _this)
{
    c_long writeFound = 0;
    c_long totalFound = 0;
    v_dataReaderSample currentSample;

    assert(C_TYPECHECK(_this, v_dataReaderInstance));

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
                    /* Only set the instance state to NEW, if the newer samples
                     * have not yet been accessed before.
                     */
                    if (!v_readerSampleTestStateOr(sample->newer, L_LAZYREAD | L_READ))
                    {
                        v_dataReaderInstanceStateSet(_this, L_NEW);
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

            /* Intermediate sample, so there is always newer sample. */
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
            /* Only set the instance state to NEW, if the newer samples
             * have not yet been accessed before.
             */
            if (!v_readerSampleTestStateOr(sample->newer, L_LAZYREAD | L_READ))
            {
                v_dataReaderInstanceStateSet(_this, L_NEW);
            }
        }
    }
}

static void
updateFinalInstanceAndSampleState(
    v_dataReaderInstance _this,
    v_message message,
    v_dataReaderSample sample,
    c_bool isTransactionFlush)
{
    v_reader reader;
    v_state msgState = v_nodeState(message);


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
    if (!v_readerSampleTestState(sample,L_TRANSACTION)){
        /* If the instance is empty and the reader has subscriber defined keys,
         * then the instance must raise its L_NEW flag.
         */
        reader = v_reader(v_index(_this->index)->reader);
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
            v_dataReaderInstanceStateSet(_this, L_NEW);
            v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
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
                v_dataReaderInstanceStateSet(_this, L_NEW);
                v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
            }
        }

        /* Assign sample counters. Be aware that only valid samples consume
         * resource limits.
         */
        sample->disposeCount = _this->disposeCount;
        sample->noWritersCount = _this->noWritersCount;


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
                v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
            }
            v_dataReaderInstanceStateSet(_this, L_DISPOSED);
        }
        if (v_stateTest(msgState, L_UNREGISTER))
        {
            /* If the message also has the DISPOSED flag set, then do not
             * set the instance state to NOWRITERS.
             */
            if (!v_dataReaderInstanceStateTest(_this, L_DISPOSED))
            {
                v_dataReaderInstanceStateSet(_this, L_NOWRITERS);
                v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
            }
            v_instanceRemove(v_instance(_this)); /* deadline */
        }
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
     * in case of equal timestamps order is determined by the writerGID.)
     */
    reader = v_reader(v_index(_this->index)->reader);
    qos = reader->qos;
    messageState = v_nodeState(message);
    if (qos->orderby.kind == V_ORDERBY_SOURCETIME)
    {
        if (c_timeCompare(message->writeTime, _this->lastConsumed.sourceTimestamp) == C_LT ||
            (
                c_timeCompare(message->writeTime, _this->lastConsumed.sourceTimestamp) == C_EQ &&
                v_gidCompare(message->writerGID, _this->lastConsumed.gid) == C_LT
            )
        )
        {
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
             *    and a writerGID equal to the current position.
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
            if ( m == message ||
                   (
                       c_timeCompare(message->writeTime, m->writeTime) == C_EQ &&
                       v_gidCompare(message->writerGID, m->writerGID) == C_EQ
                   )
            )
            {
                return V_DATAREADER_DUPLICATE_SAMPLE;
            }
            else
            {
                if (qos->orderby.kind == V_ORDERBY_SOURCETIME)
                {
                    equality = c_timeCompare(message->writeTime, m->writeTime);
                    /* C_GT : message is newer than the current position
                     *        in the history and older than the previous.
                     *        so insert before the current position.
                     * C_LT : message is older than the current position
                     *        in the history and newer than the previous.
                     *        so goto the next iteration.
                     * C_EQ : message has the same timestamp as the current
                     *        position. If both messages have different
                     *        sources, then messages with the same timestamp
                     *        will be sorted by their writerGID to guarantee
                     *        eventual consistency throughout all DataReaders
                     *        in the Domain.
                     */
                    if (equality == C_EQ)
                    {
                        /* Duplicate timestamps with equal GID have
                         * already been filtered out before.
                         */
                        equality = v_gidCompare(message->writerGID, m->writerGID);
                    }
                }
                else
                {
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
                                 * into account for depth (calledsampleCount
                                 * here).
                                 */
                                do
                                {
                                    oldest = v_dataReaderInstanceOldest(_this);
                                    v_dataReaderInstanceSetOldest(_this,
                                                            oldest->newer);
                                    if (oldest->newer)
                                    {
                                        v_dataReaderSample(oldest->newer)->older = oldest->older;
                                    }
                                    else
                                    {
                                        /* oldest = newest */
                                        /* in this use case always set to NULL. */
                                        v_dataReaderInstanceSetNewest(_this,
                                                                oldest->older);
                                    }
                                    oldest->newer = NULL;
                                    oldest->older = NULL;
                                    v_dataReaderSampleWipeViews(oldest);

                                    if (*sample == oldest)
                                    {
                                        *sample = NULL;
                                        proceed = FALSE;
                                    }
                                    else if (v_dataReaderSampleMessageStateTest(
                                                                oldest, L_WRITE))
                                    {
                                        _this->sampleCount--;
                                        proceed = FALSE;
                                    }
                                    v_dataReaderSampleRemoveFromLifespanAdmin(oldest);
                                    v_dataReaderSampleFree(oldest);
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

    /* The first insert will increase to 0 times noWriter state. */
    v_instanceInit(v_instance(_this));
    _this->instanceState                = L_EMPTY | L_NOWRITERS;
    _this->noWritersCount               = -1;
    _this->disposeCount                 = 0;
    _this->sampleCount                  = 0;
    _this->liveliness                   = 0;
    _this->hasBeenAlive                 = FALSE;
    _this->epoch                        = C_TIME_ZERO;
    _this->lastConsumed.sourceTimestamp = C_TIME_ZERO;
    v_gidSetNil(_this->lastConsumed.gid);
    _this->purgeInsertionTime           = C_TIME_ZERO;
    _this->userDataDataReaderInstance   = NULL;
    v_dataReaderInstanceSetOldest(_this,NULL);
    v_dataReaderInstanceSetNewest(_this,NULL);

    qos = v_reader(index->reader)->qos;

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
v_dataReaderInstanceFree(
    v_dataReaderInstance _this)
{
    if (c_refCount(_this) == 1) {
        v_dataReaderInstanceDeinit(_this);
    }
    c_free(_this);
}

void
v_dataReaderInstanceDeinit(
    v_dataReaderInstance _this)
{
    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    CHECK_COUNT(_this);

    v_groupCacheDeinit(_this->sourceCache);
    v_instanceDeinit(v_instance(_this));
}

v_writeResult
v_dataReaderInstanceWrite (
    v_dataReaderInstance _this,
    v_message msg)
{
    v_writeResult result;
    v_dataReaderEntry entry;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    assert(C_TYPECHECK(msg,v_message));

    if ((_this->owner.exclusive) &&
        (v_messageQos_getOwnershipStrength(msg->qos) < _this->owner.strength) &&
        (v_gidIsValid(_this->owner.gid))) {
        result = V_WRITE_SUCCESS;
    } else {
        entry = v_dataReaderEntry(v_index(_this->index)->entry);
        result = v_dataReaderEntryWrite(entry, msg, (v_instance *)&_this);
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
    v_dataReaderSample sample;
    v_index index;
    v_state messageState;
    v_state instanceState;
    c_equality equality;
    v_dataReader reader;
    v_dataReaderEntry entry;
    v_readerQos qos;
    c_long msg_strength;
    v_gid msg_gid;
    v_dataReaderResult result = V_DATAREADER_UNDETERMINED;

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
    qos = v_reader(reader)->qos;
    sample = NULL;

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

    messageState = v_nodeState(message);

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

    /* scdds2146: ES: Get the state of the datareader instance now as it will
     * be possibly modified in the next piece of code to set the dispose bit
     * in the instance state. And to determine correct dispose counters
     * later on we need to know what the instance state was before any dispose
     * information is changed within the instance so we can determine if the
     * instance went from an ALIVE state to a NOT_ALIVE_DISPOSED state.
     */
    instanceState = v_dataReaderInstanceState(_this);

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


    /*
     * Test if ownership is exclusive and whether the writer identified
     * in the message should become owner. In case of an invalid GID
     * ownership is always assumed. (For example in case of disposeAll.)
     */
    msg_gid = message->writerGID;
    if (_this->owner.exclusive && v_gidIsValid(msg_gid))
    {
        /* If the message is an UNREGISTER message, only determine whether
         * it is the owner.
         * If it is the owner the ownership is cleared.
         * When the writer of the message is stronger than the current owner,
         * the current owner remains the owner, since it is the strongest
         * writer we know so far.
         */
        msg_strength = v_messageQos_getOwnershipStrength(message->qos);
        if (v_stateTest(messageState, L_UNREGISTER))
        {
            /* If the writer of the message is owner, then clear the owner */
            if (v_gidEqual(_this->owner.gid, msg_gid))
            {
                v_gidSetNil(_this->owner.gid);
            }
        }
        else
        {
            if (v_gidIsValid(_this->owner.gid))
            {
                equality = v_gidCompare(_this->owner.gid, msg_gid);
                if (equality != C_EQ)
                {
                    if (_this->owner.strength < msg_strength)
                    {
                        _this->owner.gid = msg_gid;
                        _this->owner.strength = msg_strength;
                    }
                    else
                    {
                        if ((_this->owner.strength == msg_strength) &&
                            (equality == C_LT))
                        {
                            /* The current message comes from a writer,
                             * which is not owner AND has a strength that is
                             * equal to the strength of the current owner.
                             * So we must determine which writer should be
                             * the owner. Every reader must determine the
                             * ownership identically, so we determine it by
                             * comparing the identification of the writer.
                             * The writer with the highest gid will be the owner.
                             */
                            _this->owner.gid = msg_gid;
                            _this->owner.strength = msg_strength;
                        }
                        else
                        {
                            CHECK_COUNT(_this);
                            return V_DATAREADER_NOT_OWNER;
                        }
                    }
                } /* else is current owner */
            }
            else
            {
                /* As a workaround for dds1784 I need to make sure that the
                 * owner is not set when there are no more live DataWriters.
                 * Otherwise a new DataWriter with a lower strength would
                 * never be able to take over ownership again!
                 *
                 * THE FOLLOWING CONDITION IS A WORKAROUND AND NEEDS A REAL FIX.
                 */
                if(_this->liveliness > 0){
                    /* instance has no owner yet,
                     * so this writer becomes the owner.
                     */
                    _this->owner.gid = msg_gid;
                    _this->owner.strength = msg_strength;
                }
            }
        }
    }


    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

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

    if ((v_dataReaderInstanceStateTest(_this, L_STATECHANGED) ||
         v_stateTest(messageState,L_WRITE)))
    {

        if ((qos->lifecycle.enable_invalid_samples) || (sample != NULL))
        {
            V_MESSAGE_STAMP(message,readerDataAvailableTime);

            V_MESSAGE_STAMP(message, readerInstanceTime);
            v_dataReaderNotifyDataAvailable(reader, sample);
        }
    }
    v_deadLineInstanceListUpdate(v_dataReader(reader)->deadLineList,
                                 v_instance(_this));

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

void
v_dataReaderInstanceFlushTransaction(
    v_dataReaderInstance _this,
    c_ulong transactionId)
{
    v_dataReaderSample sample, nextSample;
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
                }
            }
        }
        sample = sample->newer;
    }
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
    c_query query)
{
    v_dataReaderSample sample, newestSample;
    c_bool sampleSatisfies = FALSE;

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
    if (query == NULL)
    {
        return TRUE;
    }
    if (v_dataReaderInstanceSampleCount(_this) == 0)
    {
        if (v_dataReaderInstanceStateTest(_this, L_STATECHANGED))
        {
            /* The sample is invalid, but not yet processed. This could
             * indicate a relevant, but not yet processed state change.
             */
            return TRUE;
        }
        else
        {
            /* Sample is invalid, but already processed. */
            return FALSE;
        }
    }
    newestSample = v_dataReaderInstanceNewest(_this);
    sample = v_dataReaderInstanceOldest(_this);
    while ((sample != NULL) && (sampleSatisfies == FALSE))
    {
        /* Invalid samples will not be offered when the instance has valid
         * samples as well. Therefore invalid samples cannot match the query
         * when sampleCount > 0.
         */
        if (v_readerSampleTestState(sample, L_VALIDDATA))
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
        sample = sample->newer;
    }

    CHECK_EMPTINESS(_this);
    CHECK_COUNT(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    return sampleSatisfies;
}

c_bool
v_dataReaderSampleRead(
    v_dataReaderSample sample,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderInstance instance;
    v_message untypedMsg;
    v_state state;
    v_state mask;
    c_bool proceed = TRUE;

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
        untypedMsg = v_dataReaderSampleMessage(sample);
        v_dataReaderSampleTemplate(sample)->message =
            CreateTypedInvalidMessage(instance, untypedMsg);
    }
    V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample), readerReadTime);
    proceed = action(v_readerSample(sample), arg);
    V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample), readerCopyTime);
    V_MESSAGE_REPORT(v_dataReaderSampleMessage(sample),
                     v_dataReaderInstanceDataReader(instance));
    /* If the message was temporarily switched, switch it back. */
    if (!v_readerSampleTestState(sample, L_VALIDDATA))
    {
        c_free(v_dataReaderSampleMessage(sample));
        v_dataReaderSampleTemplate(sample)->message = untypedMsg;
    }

    v_dataReaderInstanceStateClear(instance, L_NEW);
    v_dataReaderInstanceStateClear(instance, L_STATECHANGED);

    if (!v_readerSampleTestState(sample, L_READ))
    {
        v_dataReaderInstanceReader(instance)->notReadCount--;
        v_readerSampleSetState(sample,L_LAZYREAD);
    }

    /* reader internal state of the data has been modified.
     * so increase readers update count.
     * This value is used by queries to determine if a query
     * needs to be re-evaluated.
     */
    v_dataReaderInstanceReader(instance)->updateCnt++;

    /* The instance state can have changed, so update the statistics */
    v_statisticsULongValueInc(v_reader,
                              numberOfSamplesRead,
                              v_dataReaderInstanceReader(instance));

    CHECK_EMPTINESS(instance);
    CHECK_COUNT(instance);
    CHECK_INSTANCE_CONSISTENCY(instance);

    return proceed;
}

c_bool
v_dataReaderInstanceReadSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderSample sample, newestSample, prevSample;
    v_message msg, prevMsg;
    v_state msgState;
    c_bool proceed = TRUE;
    c_bool sampleSatisfies;
    c_ulong readId;
    v_dataReader r;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    /* If no valid nor invalid samples exist, then skip further actions. */
    if (_this && !v_dataReaderInstanceEmpty(_this))
    {
        /* Check the number of L_VALID samples. If there are none, check to
         * see whether any invalid samples need to be communicated.
         */
        if (_this->sampleCount == 0)
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
                        prevMsg = v_dataReaderSampleMessage(prevSample);
                        msgState = v_nodeState(prevMsg);

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
                        proceed = v_dataReaderSampleRead(sample, action, arg);
                        assert(!v_dataReaderInstanceStateTest(_this, L_STATECHANGED));
                    }
                }
            }

            CHECK_EMPTINESS(_this);
            CHECK_COUNT(_this);
            CHECK_INSTANCE_CONSISTENCY(_this);

            return proceed;
        }
        readId = v_dataReaderInstanceDataReader(_this)->readCnt;
        newestSample = v_dataReaderInstanceNewest(_this);
        sample = v_dataReaderInstanceOldest(_this);
        while ((sample != NULL) && (proceed == TRUE)) {
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
                    proceed = v_dataReaderSampleRead(sample, action, arg);
                }
            }
            sample = sample->newer;
        }
    }

    CHECK_EMPTINESS(_this);
    CHECK_COUNT(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    return proceed;
}


c_bool
v_dataReaderInstanceWalkSamples(
    v_dataReaderInstance _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderSample sample;
    c_bool proceed = TRUE;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this != NULL)
    {
        CHECK_COUNT(_this);
        CHECK_EMPTINESS(_this);
        CHECK_INSTANCE_CONSISTENCY(_this);

        if (!v_dataReaderInstanceEmpty(_this))
        {
            sample = v_dataReaderInstanceOldest(_this);
            while ((sample != NULL) && (proceed == TRUE))
            {
                proceed = action(v_readerSample(sample),arg);
                sample = sample->newer;
            }
            CHECK_COUNT(_this);
            CHECK_EMPTINESS(_this);
            CHECK_INSTANCE_CONSISTENCY(_this);
        }
    }
    return proceed;
}

c_bool
v_dataReaderSampleTake(
    v_dataReaderSample sample,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderInstance instance;
    v_message untypedMsg;
    v_state state;
    v_state mask;
    c_bool proceed = TRUE;
    v_dataReader r;

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

    /* Check for an action routine. */
    V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample),readerReadTime);
    untypedMsg = v_dataReaderSampleMessage(sample);
    if (action)
    {
        /* If the sample contains an untyped invalid message, then temporarily
         * replace it with a typed invalid message so that the Reader is able to
         * interpret its contents.
         */
        if (!v_readerSampleTestState(sample, L_VALIDDATA))
        {
            v_dataReaderSampleTemplate(sample)->message =
                CreateTypedInvalidMessage(instance, untypedMsg);
        }
        /* Invoke the action routine with the typed sample. */
        proceed = action(v_readerSample(sample), arg);

        /* If the message was temporarily switched, switch it back. */
        if (!v_readerSampleTestState(sample, L_VALIDDATA))
        {
            c_free(v_dataReaderSampleMessage(sample));
            v_dataReaderSampleTemplate(sample)->message = untypedMsg;
        }
    }
    else
    {
        proceed = TRUE;
    }

    V_MESSAGE_STAMP(v_dataReaderSampleMessage(sample),readerCopyTime);
    V_MESSAGE_REPORT(v_dataReaderSampleMessage(sample),
                     v_dataReaderInstanceDataReader(instance));

    /* If the consumed sample is newer than the previously consumed
     * sample, then update the history bookmark to indicate that
     * all samples prior to the current sample have been consumed.
     */
    if (c_timeCompare(untypedMsg->writeTime, instance->lastConsumed.sourceTimestamp) == C_GT ||
       (
            c_timeCompare(untypedMsg->writeTime, instance->lastConsumed.sourceTimestamp) == C_EQ &&
            v_gidCompare(untypedMsg->writerGID, instance->lastConsumed.gid) == C_GT
       )
    )
    {
        instance->lastConsumed.sourceTimestamp = untypedMsg->writeTime;
        instance->lastConsumed.gid = untypedMsg->writerGID;
    }


    if (r->views != NULL)
    {
        v_dataReaderSampleWipeViews(v_dataReaderSample(sample));
    }
    if(action)
    {
        v_dataReaderInstanceStateClear(instance, L_NEW);
    }
    assert(instance->sampleCount >= 0);
    if (v_readerSampleTestState(sample, L_VALIDDATA))
    {
        instance->sampleCount--;
    }
    /* Remove sample from history. */
    if (sample->older)
    {
        assert(v_dataReaderInstanceOldest(instance) != sample);
        v_dataReaderSample(sample->older)->newer = sample->newer;
    }
    else
    {
        /* sample = oldest */
        assert(v_dataReaderInstanceOldest(instance) == sample);
        v_dataReaderInstanceSetOldest(instance,sample->newer);
    }
    if (sample->newer)
    {
        assert(v_dataReaderInstanceNewest(instance) != sample);
        v_dataReaderSample(sample->newer)->older = sample->older;
    }
    else
    {
        /* sample = newest */
        assert(v_dataReaderInstanceNewest(instance) == sample);
        v_dataReaderInstanceSetNewest(instance,sample->older);
    }
    sample->newer = NULL;
    sample->older = NULL;
    v_dataReaderSampleRemoveFromLifespanAdmin(sample);
    v_dataReaderSampleFree(sample);

    v_dataReaderInstanceStateClear(instance, L_STATECHANGED);

    if (v_dataReaderInstanceEmpty(instance))
    {
        assert(instance->sampleCount == 0);
        v_dataReaderInstanceStateSet(instance, L_EMPTY);
    }

    /* reader internal state of the data has been modified.
     * so increase readers update count.
     * This value is used by queries to determine if a query
     * needs to be re-evaluated.
     */
    v_dataReader(r)->updateCnt++;

    if (r->triggerValue)
    {
        v_dataReaderTriggerValueFree(r->triggerValue);
        r->triggerValue = NULL;
    }

    /* The instance state can have changed, so update the statistics */
    UPDATE_READER_STATISTICS(v_index(instance->index), instance, state);

    CHECK_COUNT(instance);
    CHECK_EMPTINESS(instance);
    CHECK_INSTANCE_CONSISTENCY(instance);
    return proceed;
}

c_bool
v_dataReaderInstanceTakeSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderSample sample, next, newestSample, prevSample;
    c_bool proceed = TRUE;
    c_bool sampleSatisfies;
    c_ulong readId;
    v_message msg, prevMsg;
    v_state msgState;
    v_dataReader r;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if (_this && !v_dataReaderInstanceEmpty(_this))
    {
        /* Check the number of L_VALID samples. If there are none, check to
         * see whether any invalid samples need to be communicated.
         */
        if (_this->sampleCount == 0)
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
                        prevMsg = v_dataReaderSampleMessage(prevSample);
                        msgState = v_nodeState(prevMsg);

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
                        proceed = v_dataReaderSampleRead(sample, action, arg);
                        assert(!v_dataReaderInstanceStateTest(_this, L_STATECHANGED));
                    }
                }
            }

            /* Now trash all the remaining (invalid) samples that are not part
             * of an unfinished transaction.
             */
            sample = v_dataReaderInstanceOldest(_this);
            while (sample != NULL)
            {
                next = sample->newer;

                /* Check whether transaction is in progress for this sample. */
                if(!v_readerSampleTestState(sample,L_TRANSACTION))
                {
                    v_dataReaderSampleTake(sample, NULL, NULL);
                }
                sample = next;
            }
            CHECK_EMPTINESS(_this);
            CHECK_COUNT(_this);
            CHECK_INSTANCE_CONSISTENCY(_this);

            return proceed;
        }
        readId = v_dataReaderInstanceDataReader(_this)->readCnt;
        newestSample = v_dataReaderInstanceNewest(_this);
        sample = v_dataReaderInstanceOldest(_this);
        while ((proceed == TRUE) && (sample != NULL))
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
                        proceed = v_dataReaderSampleTake(sample, action, arg);
                        assert(!v_dataReaderInstanceStateTest(_this, L_STATECHANGED));
                        CHECK_EMPTINESS(_this);
                    }
                    else
                    {
                        /* When taking samples, all invalid samples up till the
                         * first valid sample can be removed.
                         */
                        v_dataReaderSampleTake(sample, NULL, NULL);
                    }
                }
            }
            sample = next;
        }

        if (v_dataReaderInstanceEmpty(_this)) {
              assert(!v_dataReaderInstanceStateTest(_this, L_STATECHANGED));
        }
    }
    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    return proceed;
}

void
v_dataReaderInstancePurge(
    v_dataReaderInstance _this,
    c_long disposedCount,
    c_long noWritersCount)
{
    v_dataReaderSample sample;
    v_dataReaderSample latestToPurge = NULL;
    v_dataReader r;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    /* Algorithm doesn't handle scanning for disposed- AND nowriterscount. */
    assert(disposedCount < 0 || noWritersCount < 0);

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INSTANCE_CONSISTENCY(_this);

    if ((_this != NULL) && !v_dataReaderInstanceEmpty(_this))
    {
        r = v_dataReaderInstanceReader(_this);
        /* Start with the oldest generation, so begin with oldest. */
        sample = v_dataReaderInstanceOldest(_this);

        if (disposedCount >= 0)
        {
            while ((sample != NULL) && (sample->disposeCount <= disposedCount))
            {
                if (v_readerSampleTestState(sample, L_VALIDDATA))
                {
                    /* Do not decrease sample count on invalid sample */
                    _this->sampleCount--;
                }

                if (r->views != NULL)
                {
                    v_dataReaderSampleWipeViews(v_dataReaderSample(sample));
                }

                v_dataReaderSampleRemoveFromLifespanAdmin(sample);

                latestToPurge = sample;
                sample = sample->newer;
            }
        }

        if (noWritersCount >= 0)
        {
            while ((sample != NULL) && (sample->noWritersCount <= noWritersCount))
            {
                if (v_readerSampleTestState(sample, L_VALIDDATA))
                {
                    /* Do not decrease sample count on invalid sample */
                    _this->sampleCount--;
                }

                if (r->views != NULL)
                {
                    v_dataReaderSampleWipeViews(v_dataReaderSample(sample));
                }

                v_dataReaderSampleRemoveFromLifespanAdmin(sample);

                latestToPurge = sample;
                sample = sample->newer;
            }
        }
        /* now latestToPurge points to the sample in the history from where
         * we need to purge.
         */
        if (sample == NULL)
        {   /* instance becomes empty, purge all */
            assert(_this->sampleCount == 0);
            sample = v_dataReaderInstanceNewest(_this);
            v_dataReaderSampleFree(sample);
            v_dataReaderInstanceSetOldest(_this,NULL);
            v_dataReaderInstanceSetNewest(_this,NULL);
            v_dataReaderInstanceStateClear(_this, L_NEW);
            v_dataReaderInstanceStateSet(_this, L_EMPTY);
            v_dataReaderInstanceStateClear(_this, L_STATECHANGED);

            /* reader internal state of the data has been modified.
             * so increase readers update count.
             * This value is used by queries to determine if a query
             * needs to be re-evaluated.
             */
            v_dataReader(r)->updateCnt++;

            if (r->triggerValue)
            {
                v_dataReaderTriggerValueFree(r->triggerValue);
                r->triggerValue = NULL;
            }
        }
        else
        {
            /* Everything older then (including) latestToPurge can be purged */
            if (latestToPurge != NULL)
            {
               v_dataReaderInstanceSetOldest(_this, sample);
               /* newer is c_voidp, but break link anyway */
               latestToPurge->newer = NULL;
               sample->older = NULL; /* reference transfered to latestToPurge */
               /* Free latestToPurge (and all older samples, since older-pointers
                * are managed.
                */
               v_dataReaderSampleFree(latestToPurge);
               /* The instance state is in correct state, so no update needed. */
            } /* else nothing to purge! */
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
        result = c_count(v_index(_this->index)->notEmptyList);
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
            msg->qos = c_keep(unregistration->qos);
            /* Set the nodeState of the message to UNREGISTER. */
            v_stateSet(v_nodeState(msg), L_UNREGISTER);

            if (autoDispose)
            {
                /* Set the nodeState of the message to UNREGISTER. */
                v_stateSet(v_nodeState(msg), L_DISPOSED);
            }

            /* Insert the invalid message into the dataReaderInstance. */
            entry = v_dataReaderEntry(v_index(_this->index)->entry);
            writeResult = v_dataReaderEntryWrite(entry, msg, (v_instance *)&_this);
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
