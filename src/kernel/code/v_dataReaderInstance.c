/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "v_dataView.h"
#include "v_dataReaderEntry.h"
#include "v_dataReaderSample.h"
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

#define v_dataReaderInstanceReader(_this) \
        v_dataReader(v_index(v_dataReaderInstance(_this)->index)->reader)

/* For debugging purposes... */
#ifndef NDEBUG
#define CHECK_EMPTINESS(i) \
        { \
            c_bool empty = v_dataReaderInstanceStateTest(i, L_EMPTY); \
            c_bool stateChanged = v_dataReaderInstanceStateTest(i, L_STATECHANGED); \
            if (!(((i->sampleCount == 0) == empty) || stateChanged)) { \
                printf("at line %d sampleCount = %d, isEmpty = %d, stateChanged = %d\n", \
                       __LINE__, i->sampleCount, empty, stateChanged); \
            } \
        }
#else
#define CHECK_EMPTINESS(i)
#endif

#ifndef NDEBUG
#define CHECK_INVALIDITY(_this) \
        { \
            if (!v_reader(v_dataReaderInstanceReader(_this))->qos->lifecycle.enable_invalid_samples) { \
                if (v_dataReaderInstanceStateTest(_this, L_STATECHANGED)) { \
                    if (v_dataReaderInstanceHead(_this) != NULL) { \
                        printf("Error at line %d enable_invalid_samples = " \
                               "FALSE but invalid sample exists\n", __LINE__); \
                    } else { \
                        printf("Warning at line %d enable_invalid_samples = " \
                               "FALSE but L_STATECHANGED is set\n", __LINE__); \
                    } \
                } \
                if (_this->sampleCount == 0) { \
                    if (v_dataReaderInstanceHead(_this) != NULL) { \
                        printf("Error at line %d enable_invalid_samples = " \
                               "FALSE but invalid sample exists\n", __LINE__); \
                    } \
                } \
            } \
        }
#else
#define CHECK_INVALIDITY(_this)
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

    currentSample = v_dataReaderInstanceHead(_this);

    while (currentSample != NULL) {
        totalFound++;
        if (v_dataReaderSampleMessageStateTest(currentSample, L_WRITE)) {
            writeFound++;
        }
        currentSample = ((v_dataReaderSample)currentSample)->prev;
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
    _this->noWritersCount =             0;
    _this->disposeCount =               0;
    _this->sampleCount =                0;
    _this->liveliness =                 0;
    _this->hasBeenAlive =               FALSE;
    _this->epoch =                      C_TIME_ZERO;
    _this->purgeInsertionTime =         C_TIME_ZERO;
    _this->userDataDataReaderInstance = NULL;
    v_dataReaderInstanceSetHead(_this,NULL);
    v_dataReaderInstanceSetTail(_this,NULL);

    qos = v_reader(index->reader)->qos;

    /* only if ownership is exclusive the owner must be set! */
     if (qos->ownership.kind == V_OWNERSHIP_EXCLUSIVE) {
        _this->owner.exclusive = TRUE;
    } else {
        _this->owner.exclusive = FALSE;
    }

    /* In case of a user defined keys dataReader no instance state
     * control messages i.e. register, unregister and dispose are received.
     * This implies that the instance state will never become alive because
     * a register message is required.
     * Therefore in the instance is only initialized with L_NOWRITERS flag
     * set if user defined keys are NOT set.
     */
    if(qos->userKey.enable){
        _this->instanceState = L_EMPTY | L_NEW ;
    } else {
        _this->instanceState = L_EMPTY | L_NEW | L_NOWRITERS;
    }
    /*
     * copy key value from message into instance.
     */
    messageKeyList = v_indexSourceKeyList(index);
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
    CHECK_INVALIDITY(_this);
    return _this;
}

void
v_dataReaderInstanceSetEpoch (
    v_dataReaderInstance _this,
    c_time time)
{
    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    CHECK_COUNT(_this);
    CHECK_INVALIDITY(_this);

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

    assert(C_TYPECHECK(_this,v_dataReaderInstance));
    assert(C_TYPECHECK(msg,v_message));

    if ((_this->owner.exclusive) &&
        (v_messageQos_getOwnershipStrength(msg->qos) < _this->owner.strength) &&
        (v_gidIsValid(_this->owner.gid))) {
        result = V_WRITE_SUCCESS;
    } else {
        result = v_dataReaderEntryWrite(
                     v_dataReaderEntry(v_index(_this->index)->entry),
                                       msg,
                                       (v_instance *)&_this);
    }

    return result;
}

v_dataReaderResult
v_dataReaderInstanceInsert(
    v_dataReaderInstance _this,
    v_message message)
{
    v_dataReaderSample sample, s, oldest;
    v_message m;
    v_index index;
    v_state messageState;
    c_equality equality;
    c_long depth;
    c_bool proceed = TRUE;
    v_dataReader reader;
    v_dataReaderEntry entry;
    v_readerQos qos;
    c_long msg_strength;
    v_gid msg_gid;

    assert(message != NULL);
    assert(_this != NULL);
    assert(C_TYPECHECK(message,v_message));
    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    index = v_index(_this->index);
    reader = v_dataReader(index->reader);
    entry = v_dataReaderEntry(index->entry);
    qos = v_reader(reader)->qos;
    sample = NULL;

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
        if (_this->liveliness == 0) {
            /* if not user defined keys then L_NOWRITERS must be set. */
            assert((!v_reader(reader)->qos->userKey.enable) ==
                     v_dataReaderInstanceStateTest(_this, L_NOWRITERS));
            /* Reuse this instance.
             * So it must be removed from the purgeListEmpty, which
             * can be achieved by resetting the purgeInsertionTime.
             */
            _this->purgeInsertionTime = C_TIME_ZERO;
            if (v_dataReaderInstanceEmpty(_this)) {
                _this->disposeCount       = 0;
                _this->noWritersCount     = 0;
                _this->sampleCount        = 0;
                _this->instanceState      = L_NEW | L_EMPTY;
                _this->epoch              = C_TIME_ZERO;
                _this->purgeInsertionTime = C_TIME_ZERO;
                _this->hasBeenAlive       = FALSE;
                _this->userDataDataReaderInstance = NULL;
                /* Renew handle so old handles are invalidated. */
                if (v_reader(reader)->qos->userKey.enable == FALSE) {
                    v_publicFree(v_public(_this));
                    v_publicInit(v_public(_this));
                }
            } else {
                v_dataReaderInstanceStateClear(_this, L_NOWRITERS);
                v_dataReaderInstanceStateSet(_this, L_NEW);
                _this->hasBeenAlive = TRUE;
            }
        }
        _this->liveliness++;
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    if (v_stateTest(messageState, L_UNREGISTER)) {
        if (_this->liveliness > 0) {
            _this->liveliness--;
            if (_this->liveliness == 0) {
                v_instanceRemove(v_instance(_this)); /* deadline */

                if (!v_dataReaderInstanceStateTest(_this, L_NOWRITERS)) {
                    if (qos->lifecycle.enable_invalid_samples) {
                    	if (!v_dataReaderInstanceStateTest(_this, L_DISPOSED)) {
                            if (_this->sampleCount > 0) {
                                v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
                            } else {
                                if (v_dataReaderInstanceEmpty(_this)) {
                                    assert(v_stateTest(_this->instanceState,L_EMPTY));
                                    /* No valid nor invalid samples exist,
                                     * so need to create an invalid sample as
                                     * sample info carier to pass state change
                                     * at nest read or take operation.
                                     */
                                    sample = v_dataReaderSampleNew(_this,message);
                                    v_dataReaderInstanceStateClear(_this, L_EMPTY);
                                    v_dataReaderInstanceSetHead(_this,sample);
                                    v_dataReaderInstanceSetTail(_this,sample);
                                    v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
                                }
                            }
                    	}
                    }
                    v_dataReaderInstanceStateSet(_this, L_NOWRITERS);
                }
            }
        }
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    /*
     * Test if ownerhsip is exclusive and whether the writer identified
     * in the message should become owner.
     */
    if (_this->owner.exclusive) {
        /* If the message is a UNREGISTER message, only determine whether
         * it is the owner.
         * If it is the owner the ownership is cleared.
         * When the writer of the message is stronger than the current owner,
         * the current owner remains the owner, since it is the strongest
         * writer we know so far.
         */
        msg_strength = v_messageQos_getOwnershipStrength(message->qos);
        msg_gid = message->writerGID;
        if (v_stateTest(messageState, L_UNREGISTER)) {
            /* If the writer of the message is owner, then clear the owner */
            if (v_gidEqual(_this->owner.gid, msg_gid)) {
                v_gidSetNil(_this->owner.gid);
            }
        } else {
            if (v_gidIsValid(_this->owner.gid)) {
                equality = v_gidCompare(_this->owner.gid, msg_gid);
                if (equality != C_EQ) {
                    if (_this->owner.strength < msg_strength) {
                        _this->owner.gid = msg_gid;
                        _this->owner.strength = msg_strength;
                    } else {
                        if ((_this->owner.strength == msg_strength) &&
                            (equality == C_GT)) {
                            /* The current message comes from a writer,
                             * which is not owner AND has a strength that is
                             * smaller than the strength of the current owner.
                             * So we must determine which writer should be
                             * the owner. Every reader must determine the
                             * ownership identically, so we determine it by
                             * comparing the identification of the writer.
                             * The writer with the lowest id will be the owner.
                             */
                            _this->owner.gid = msg_gid;
                            _this->owner.strength = msg_strength;
                        } else {
                            CHECK_COUNT(_this);
                            return V_DATAREADER_NOT_OWNER;
                        }
                    }
                } /* else is current owner */
            } else {
                /* instance has no owner yet,
                 * so this writer becomes the owneri.
                 */
                _this->owner.gid = msg_gid;
                _this->owner.strength = msg_strength;
            }
        }
    }


    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    /* Only write and dispose messages do insert a message */
    if (v_stateTest(messageState,L_DISPOSED) &&
        (!v_dataReaderInstanceStateTest(_this, L_DISPOSED)) )
    {
        /* In case (messageState == L_WRITE ) resource limits
         * must be taken into account, since the message now contains data!
         * Note:
         * Write-Dispose messages are rejected when the resource-limits
         * are reached. The reason for this is that dispose-messages are not
         * allowed to take-over write-messages so the dispose state cannot be
         * accepted while the date (write part) is rejected and needs to be
         * resend later on.
         */
        depth = reader->depth;
        assert(_this->sampleCount <= depth);
        if ((_this->sampleCount == depth) &&
            (qos->history.kind == V_HISTORY_KEEPALL) &&
            (v_stateTest(messageState, L_WRITE))) {
            CHECK_COUNT(_this);
            return V_DATAREADER_INSTANCE_FULL;
        }

        if (_this->sampleCount == 0) {
            v_dataReaderInstanceStateSet(_this, L_DISPOSED);
            /* No valid nor invalid samples exist,
             * so if invalid samples are applicable then there is a need toi
             * create an invalid sample as sample info carier to pass the
             * state change at next read or take operation.
             */
            if (!v_stateTest(messageState,L_WRITE)) {
                if (qos->lifecycle.enable_invalid_samples) {
                    if (v_dataReaderInstanceEmpty(_this)) {
                        sample = v_dataReaderSampleNew(_this,message);
                        v_dataReaderInstanceStateClear(_this, L_EMPTY);
                        v_dataReaderInstanceSetHead(_this,sample);
                        v_dataReaderInstanceSetTail(_this,sample);
                        v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
                    }
                }
            }
        } else {
            /* instance is not empty, valid samples exist.
             */
            if (qos->orderby.kind == V_ORDERBY_SOURCETIME) {
                /* If order by source time then only set disposed if
                 * message is most recent!
                 */
                s = v_dataReaderInstanceTail(_this);
                m = v_dataReaderSampleMessage(s);
                if (c_timeCompare(message->writeTime, m->writeTime) != C_LT) {
                    v_dataReaderInstanceStateSet(_this, L_DISPOSED);
                    if (!v_dataReaderInstanceStateTest(_this, L_NOWRITERS)) {
                        if (qos->lifecycle.enable_invalid_samples) {
                            v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
                        }
                    }
                }
            } else {
                v_dataReaderInstanceStateSet(_this, L_DISPOSED);
                if (!v_dataReaderInstanceStateTest(_this, L_NOWRITERS)) {
                    if (qos->lifecycle.enable_invalid_samples) {
                        v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
                    }
                }
            }
        }
    }


    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    if (v_stateTest(messageState,L_WRITE)) {
        if (_this->hasBeenAlive) {
            _this->noWritersCount++;
            _this->hasBeenAlive = FALSE;
        }
        if (_this->sampleCount == 0) {
            /* No valid data available. */
            if (v_stateTest(_this->instanceState,L_EMPTY)) {
                assert(v_dataReaderInstanceHead(_this) == NULL);
                assert(v_dataReaderInstanceTail(_this) == NULL);
                assert(v_dataReaderInstanceEmpty(_this));
            } else {
                /* An invalid data sample exist.
                 * Remove it as the incomming sample will provide
                 * a sample info carier for read and take operations.
                 */
                s = v_dataReaderInstanceHead(_this);
                assert(s);
                assert(s == v_dataReaderInstanceTail(_this));
                v_dataReaderSampleRemoveFromLifespanAdmin(s); // Maybe not needed here, but shouldn't harm.
                v_dataReaderSampleFree(s);
            }
            sample = v_dataReaderSampleNew(_this,message);
            v_dataReaderInstanceStateClear(_this, L_EMPTY);
            v_dataReaderInstanceSetHead(_this,sample);
            v_dataReaderInstanceSetTail(_this,sample);
            _this->sampleCount = 1;
        } else {
            depth = reader->depth;
            assert(_this->sampleCount <= depth);


            /* Find insertion point in history.
             * Start at the tail (newest, last inserted) and
             * walk to the head (oldest, first inserted).
             */
            proceed = TRUE;
            s = v_dataReaderInstanceTail(_this);
            while ((s!=NULL)&&(proceed)) {
                m = v_dataReaderSampleMessage(s);
                if (m == message) {
                    /*
                     * This equality check only works for local produced
                     * messages.
                     * The best manner to check is to base equality on writer
                     * id and writer sequence number.
                     */
                    proceed = FALSE;
                } else {
                    if (qos->orderby.kind == V_ORDERBY_SOURCETIME) {
                        equality = c_timeCompare(message->writeTime,
                                                 m->writeTime);
                        /* C_GT : message is newer than the current position
                         *        in the history and older than the previous.
                         *        so insert before the current position.
                         * C_LT : message is older than the current position
                         *        in the history and newer than the previous.
                         *        so goto the next iteration.
                         */
                    } else {
                        equality = C_GT;
                        /* Store at the tail (newest message). */
                    }
                    if (equality == C_GT) {
                        /* message is newer than the current position s,
                         * insert message at the tail side of s.
                         */
                        if (_this->sampleCount < depth) {
                            sample = v_dataReaderSampleNew(_this,message);
                            /* Insert after s. */
                            if (s->prev) {
                                assert(v_dataReaderSample(s->prev)->next == s);
                                v_dataReaderSample(s->prev)->next = sample;
                            } else {
                                /* s = tail */
                                v_dataReaderInstanceSetTail(_this,sample);
                            }
                            sample->next = s;
                            sample->prev = s->prev;
                            s->prev = sample;

                            /* update internal instance state. */
                            _this->sampleCount++;
                            v_dataReaderInstanceStateClear(_this, L_EMPTY);
                            proceed = FALSE;
                        } else {
                            if (qos->history.kind == V_HISTORY_KEEPALL) {
                                return V_DATAREADER_INSTANCE_FULL;
                            } else {
                                sample = v_dataReaderSampleNew(_this,message);
                                /* Insert after s. */
                                if (s->prev) {
                                    assert(v_dataReaderSample(s->prev)->next == s);
                                    v_dataReaderSample(s->prev)->next = sample;
                                } else {
                                    /* s = tail */
                                    v_dataReaderInstanceSetTail(_this,sample);
                                }
                                sample->next = s;
                                sample->prev = s->prev;
                                s->prev = sample;

                                /* Push out the oldest sample in the history
                                 * until we've at least removed one L_WRITE
                                 * message, because only those ones are taken
                                 * into account for depth (called
                                 * sampleCount here).
                                 */
                                do {
                                    oldest = v_dataReaderInstanceHead(_this);
                                    v_dataReaderInstanceSetHead(_this,
                                                            oldest->prev);
                                    if (oldest->prev) {
                                        oldest->prev->next = oldest->next;
                                    } else {
                                        /* oldest = tail */
                                        /* in this use case always set to NULL. */
                                        v_dataReaderInstanceSetTail(_this,
                                                                oldest->next);
                                    }
                                    oldest->prev = NULL;
                                    oldest->next = NULL;
                                    v_dataReaderSampleWipeViews(oldest);

                                    if (sample == oldest) {
                                        sample = NULL;
                                        proceed = FALSE;
                                    } else if (
                                        v_dataReaderSampleMessageStateTest(oldest,
                                            L_WRITE))
                                    {
                                        proceed = FALSE;
                                    }
                                    v_dataReaderSampleRemoveFromLifespanAdmin(oldest);
                                    v_dataReaderSampleFree(oldest);
                                } while(proceed);

                                v_statisticsULongValueInc(v_reader,
                                                          numberOfSamplesDiscarded,
                                                          reader);
                            }
                        }
                    } else {
                        /* The message is older than s. */
                        if (s->next) {
                            /* more candidate older samples to go... */
                            s = s->next;
                        } else {
                            if (_this->sampleCount < depth) {
                                sample = v_dataReaderSampleNew(_this,message);
                                /* Insert before s. */
                                s->next = sample;
                                sample->prev = s;
                                sample->next = NULL;
                                v_dataReaderInstanceSetHead(_this,sample);
                                /* update internal instance state. */
                                _this->sampleCount++;
                            }  else { /* incoming message is oldest and no history space left, so discard */
                                sample = NULL; /* prevent instance state update later on */
                                v_statisticsULongValueInc(v_reader,
                                                          numberOfSamplesDiscarded,
                                                          reader);
                            }
                            proceed = FALSE;
                        }
                    }
                }
            }
        }

        CHECK_COUNT(_this);
        CHECK_EMPTINESS(_this);
        CHECK_INVALIDITY(_this);

        /* At this point the message is inserted into the history.
         * Now update internal instance state.
         */

        if (sample != NULL) {
            if (v_dataReaderInstanceStateTest(_this, L_DISPOSED) &&
                !v_stateTest(messageState, L_DISPOSED) &&
                !v_stateTest(messageState, L_UNREGISTER)) {
                /* only when message state has not L_DISPOSED set
                   (dispose message with data) the administration must be updated
                 */
                v_dataReaderInstanceStateClear(_this, L_DISPOSED);
                _this->disposeCount++;
                v_dataReaderInstanceStateSet(_this, L_NEW);
            }

            sample->disposeCount = _this->disposeCount;
            sample->noWritersCount = _this->noWritersCount;
        }
    }


    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    if ((reader->views != NULL) && (sample != NULL)) {
        c_walk(reader->views,writeSlave,sample);
    }
    CHECK_EMPTINESS(_this);
    CHECK_COUNT(_this);
    CHECK_INVALIDITY(_this);

    if ((v_dataReaderInstanceStateTest(_this, L_STATECHANGED) ||
         v_stateTest(messageState,L_WRITE))) {

        if ((qos->lifecycle.enable_invalid_samples) ||
            (sample != NULL)) {
            V_MESSAGE_STAMP(message,readerDataAvailableTime);

            V_MESSAGE_STAMP(message,readerInstanceTime);
            v_dataReaderNotifyDataAvailable(reader,sample);
        }
    }
    v_deadLineInstanceListUpdate(v_dataReader(reader)->deadLineList,
                                 v_instance(_this));

    /* reader internal state of the data has been modified.
     * so increase readers update count.
     * This value is used by queries to determine if a query
     * needs to be reëvaluated.
     */
    v_dataReader(reader)->updateCnt++;

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);
    assert(_this->sampleCount <= reader->depth);

    return V_DATAREADER_INSERTED;
}

c_bool
v_dataReaderInstanceTest(
    v_dataReaderInstance _this,
    c_query query)
{
    v_dataReaderSample sample, firstSample;
    c_bool sampleSatisfies = FALSE;

    assert(_this);
    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this == NULL) {
        return FALSE;
    }
    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    if (v_dataReaderInstanceEmpty(_this)) {
        return FALSE;
    }
    if (query == NULL) {
        return TRUE;
    }
    if (v_dataReaderInstanceSampleCount(_this) == 0) {
        assert(!v_dataReaderInstanceEmpty(_this));
        /* The sample is invalid.
         * So evaluation of non key fields is not applicable.
         */
        return TRUE;
    }
    firstSample = v_dataReaderInstanceHead(_this);
    sample = firstSample;
    while ((sample != NULL) && (sampleSatisfies == FALSE)) {
        /* The history samples are swapped with the first sample to make
           sample-evaluation on instance level work.
        */
        if (sample != firstSample) {
            v_dataReaderInstanceSetHead(_this,sample);
        }
        sampleSatisfies = c_queryEval(query,_this);
        if (sample != firstSample) {
            v_dataReaderInstanceSetHead(_this,firstSample);
        }
        sample = sample->prev;
    }

    CHECK_EMPTINESS(_this);
    CHECK_COUNT(_this);
    CHECK_INVALIDITY(_this);

    return sampleSatisfies;
}

c_bool
v_dataReaderSampleRead(
    v_dataReaderSample _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderInstance instance;
    v_state state;
    v_state mask;
    c_bool proceed;

    instance = v_dataReaderSampleInstance(_this);

    CHECK_EMPTINESS(instance);
    CHECK_COUNT(instance);
    CHECK_INVALIDITY(instance);

    state = v_dataReaderInstanceState(instance);
    mask = L_NEW | L_DISPOSED | L_NOWRITERS;

    /* Copy the value of instance state bits specified by the mask
     * to the sample state bits without affecting other bits.
     */
    v_readerSampleSetState(_this,(state & mask));
    v_readerSampleClearState(_this,(~state & mask));

    /* If the status of the sample is READ by the previous read
     * operation and the flag is not yet set (specified by the
     * LAZYREAD flag) then correct the state before executing the
     * read action.
     */
    if (v_readerSampleTestState(_this,L_LAZYREAD)) {
        v_readerSampleSetState(_this,L_READ);
        v_readerSampleClearState(_this,L_LAZYREAD);
    }
    if (instance->sampleCount > 0) {
        v_readerSampleSetState(_this,L_VALIDDATA);
    } else {
        v_readerSampleClearState(_this,L_VALIDDATA);
    }

    V_MESSAGE_STAMP(v_dataReaderSampleMessage(_this),readerReadTime);
    proceed = action(v_readerSample(_this),arg);
    V_MESSAGE_STAMP(v_dataReaderSampleMessage(_this),readerCopyTime);
    V_MESSAGE_REPORT(v_dataReaderSampleMessage(_this),
                     v_dataReaderInstanceDataReader(instance));
    v_dataReaderInstanceStateClear(instance, L_NEW);
    v_dataReaderInstanceStateClear(instance, L_STATECHANGED);
    if (!v_readerSampleTestState(_this,L_READ)) {
        v_readerSampleSetState(_this,L_LAZYREAD);
    }

    /* reader internal state of the data has been modified.
     * so increase readers update count.
     * This value is used by queries to determine if a query
     * needs to be reëvaluated.
     */
    v_dataReaderInstanceReader(instance)->updateCnt++;

    v_statisticsULongValueInc(v_reader,
                              numberOfSamplesRead,
                              v_dataReaderInstanceReader(instance));

    CHECK_EMPTINESS(instance);
    CHECK_COUNT(instance);
    CHECK_INVALIDITY(instance);

    return proceed;
}

c_bool
v_dataReaderInstanceReadSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderSample sample, firstSample;
    c_bool proceed = TRUE;
    c_bool sampleSatisfies;
    c_ulong readId;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    if (_this == NULL) {
        return proceed;
    }
    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    if (v_dataReaderInstanceEmpty(_this)) {
        /* No valid nor invalid samples exist,
         * so skip further actions.
         */
        return proceed;
    }
    if (_this->sampleCount == 0) {
        /* No valid samples exist,
         * so there must be one invalid sample.
         */
        assert(v_dataReaderInstanceStateTest(_this, L_STATECHANGED));
        sample = v_dataReaderInstanceHead(_this);
        assert(sample);
        assert(sample == v_dataReaderInstanceTail(_this));
        proceed = v_dataReaderSampleTake(sample,action,arg);
        assert(!v_dataReaderInstanceStateTest(_this, L_STATECHANGED));

        CHECK_EMPTINESS(_this);
        CHECK_COUNT(_this);
        CHECK_INVALIDITY(_this);

        return proceed;
    }
    readId = v_dataReaderInstanceDataReader(_this)->readCnt;
    firstSample = v_dataReaderInstanceHead(_this);
    sample = firstSample;
    while ((sample != NULL) && (proceed == TRUE)) {
        if (sample->readId != readId) {
            if (query != NULL) {
                /* The history samples are swapped with the first sample
                 * to make sample-evaluation on instance level work.
                 */
                if (sample != firstSample) {
                    v_dataReaderInstanceSetHead(_this,sample);
                }
                sampleSatisfies = c_queryEval(query,_this);
                if (sample != firstSample) {
                    v_dataReaderInstanceSetHead(_this,firstSample);
                }
            } else {
                sampleSatisfies = TRUE;
            }
            if (sampleSatisfies) {
                sample->readId = readId;
                proceed = v_dataReaderSampleRead(sample,action,arg);
            }
        }
        sample = sample->prev;
    }

    CHECK_EMPTINESS(_this);
    CHECK_COUNT(_this);
    CHECK_INVALIDITY(_this);

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

    if (_this != NULL) {
        CHECK_COUNT(_this);
        CHECK_EMPTINESS(_this);
        CHECK_INVALIDITY(_this);

        if (!v_dataReaderInstanceEmpty(_this)) {
            sample = v_dataReaderInstanceHead(_this);
            while ((sample != NULL) && (proceed == TRUE)) {
                proceed = action(v_readerSample(sample),arg);
                sample = sample->prev;
            }
            CHECK_COUNT(_this);
            CHECK_EMPTINESS(_this);
            CHECK_INVALIDITY(_this);
        }
    }
    return proceed;
}

c_bool
v_dataReaderSampleTake(
    v_dataReaderSample _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderInstance instance;
    v_state state;
    v_state mask;
    c_bool proceed;
    v_dataReader r;

    instance = v_dataReaderSampleInstance(_this);

    CHECK_COUNT(instance);
    CHECK_EMPTINESS(instance);
    CHECK_INVALIDITY(instance);

    state = v_dataReaderInstanceState(instance);
    mask = L_NEW | L_DISPOSED | L_NOWRITERS;

    v_readerSampleSetState(_this,(state & mask));
    v_readerSampleClearState(_this,(~state & mask));

    if (v_readerSampleTestState(_this,L_LAZYREAD)) {
        v_readerSampleSetState(_this,L_READ);
        v_readerSampleClearState(_this,L_LAZYREAD);
    }
    if (instance->sampleCount > 0) {
        v_readerSampleSetState(_this,L_VALIDDATA);
    } else {
        v_readerSampleClearState(_this,L_VALIDDATA);
    }

    V_MESSAGE_STAMP(v_dataReaderSampleMessage(_this),readerReadTime);
    if (action) {
        proceed = action(v_readerSample(_this),arg);
    } else {
        proceed = TRUE;
    }

    V_MESSAGE_STAMP(v_dataReaderSampleMessage(_this),readerCopyTime);
    V_MESSAGE_REPORT(v_dataReaderSampleMessage(_this),
                     v_dataReaderInstanceDataReader(instance));

    r = v_dataReaderInstanceReader(instance);
    if (r->views != NULL) {
        v_dataReaderSampleWipeViews(v_dataReaderSample(_this));
    }
    v_dataReaderInstanceStateClear(instance, L_NEW);
    assert(instance->sampleCount >= 0);
#ifndef NDEBUG
    if (instance->sampleCount == 0) {
        assert(v_dataReaderInstanceStateTest(instance, L_STATECHANGED));
    }
#endif
    if (instance->sampleCount > 0) {
        instance->sampleCount--;
    } else {
        assert(v_reader(r)->qos->lifecycle.enable_invalid_samples);
    }
    /* Remove sample from history. */
    if (_this->next) {
        assert(v_dataReaderInstanceHead(instance) != _this);
        v_dataReaderSample(_this->next)->prev = _this->prev;
    } else {
        /* _this = head */
        assert(v_dataReaderInstanceHead(instance) == _this);
        v_dataReaderInstanceSetHead(instance,_this->prev);
    }
    if (_this->prev) {
        assert(v_dataReaderInstanceTail(instance) != _this);
        _this->prev->next = _this->next;
    } else {
        /* sample = tail */
        assert(v_dataReaderInstanceTail(instance) == _this);
        v_dataReaderInstanceSetTail(instance,_this->next);
    }
    _this->prev = NULL;
    _this->next = NULL;
    v_dataReaderSampleRemoveFromLifespanAdmin(_this);
    v_dataReaderSampleFree(_this);

    v_dataReaderInstanceStateClear(instance, L_STATECHANGED);

    if (instance->sampleCount == 0) {
        assert(v_dataReaderInstanceEmpty(instance));
        v_dataReaderInstanceStateSet(instance, L_EMPTY);
        if (v_reader(r)->qos->userKey.enable) {
            v_dataReaderInstanceStateSet(instance, L_NEW);
        }
    }

    /* reader internal state of the data has been modified.
     * so increase readers update count.
     * This value is used by queries to determine if a query
     * needs to be reëvaluated.
     */
    v_dataReader(r)->updateCnt++;

    if (r->triggerValue) {
        c_free(v_readerSample(r->triggerValue)->instance);
        v_dataReaderSampleFree(r->triggerValue);
        r->triggerValue = NULL;
    }

    CHECK_COUNT(instance);
    CHECK_EMPTINESS(instance);
    CHECK_INVALIDITY(instance);

    /* The instance state can have changed, so update the statistics */
    UPDATE_READER_STATISTICS(v_index(instance->index), instance, state);
    return proceed;
}

c_bool
v_dataReaderInstanceTakeSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderSample sample, previous, firstSample;
    c_bool proceed = TRUE;
    c_bool sampleSatisfies;
    c_ulong readId;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    if (!v_dataReaderInstanceEmpty(_this)) {
        readId = v_dataReaderInstanceDataReader(_this)->readCnt;
        sample = v_dataReaderInstanceHead(_this);
        while ((proceed == TRUE) && (sample != NULL)) {
            previous = sample->prev;
            if (sample->readId != readId) {
                if (query != NULL) {
                    if (_this->sampleCount > 0) {
                        firstSample = v_dataReaderInstanceHead(_this);
                        if (sample != firstSample) {
                            v_dataReaderInstanceSetHead(_this,sample);
                        }
                        sampleSatisfies = c_queryEval(query,_this);
                        if (sample != firstSample) {
                            v_dataReaderInstanceSetHead(_this,firstSample);
                        }
                    } else {
                        /* conditions on invalid data is not applicable,
                         * so set satisfies to TRUE */
                        sampleSatisfies = TRUE;
                    }
                } else {
                    sampleSatisfies = TRUE;
                }
                if (sampleSatisfies) {
                    sample->readId = readId;
                    proceed = v_dataReaderSampleTake(sample,action,arg);
                    assert(!v_dataReaderInstanceStateTest(_this, L_STATECHANGED));
                    CHECK_EMPTINESS(_this);
                }
            }
            sample = previous;
        }

        if (v_dataReaderInstanceEmpty(_this)) {
              assert(!v_dataReaderInstanceStateTest(_this, L_STATECHANGED));
        }
    }
    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    return proceed;
}

void
v_dataReaderInstancePurge(
    v_dataReaderInstance _this,
    c_long disposedCount,
    c_long noWritersCount)
{
    v_dataReaderSample sample, next;
    v_dataReader r;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    if ((_this != NULL) && !v_dataReaderInstanceEmpty(_this)) {
        r = v_dataReaderInstanceReader(_this);
        sample = v_dataReaderInstanceHead(_this);
        if (disposedCount >= 0) {
            next = NULL;
            while ((sample != NULL) && (sample->disposeCount == disposedCount)) {
                if (_this->sampleCount) {
                    /* Do not decrease sample count on invalid sample */
                    _this->sampleCount--;
                } else {
                    assert(v_reader(r)->qos->lifecycle.enable_invalid_samples);
                }
    
                if (r->views != NULL) {
                    v_dataReaderSampleWipeViews(v_dataReaderSample(sample));
                }
                next = sample;
                sample = sample->prev;
            }
        }
        if (noWritersCount >= 0) {
            while ((sample != NULL) && (sample->noWritersCount == noWritersCount)) {
                if (_this->sampleCount) {
                    /* Do not decrease sample count on invalid sample */
                    _this->sampleCount--;
                } else {
                    assert(v_reader(r)->qos->lifecycle.enable_invalid_samples);
                }
    
                if (r->views != NULL) {
                    v_dataReaderSampleWipeViews(v_dataReaderSample(sample));
                }
                next = sample;
                sample = sample->prev;
            }
        }
        /* now next points to the sample in the history from where
         * we need to purge.
         */
        if (sample == NULL) { /* instance becomes empty, purge all */
            assert(_this->sampleCount == 0);
            sample = v_dataReaderInstanceHead(_this);
            v_dataReaderSampleRemoveFromLifespanAdmin(sample);
            v_dataReaderSampleFree(sample);
            v_dataReaderInstanceSetHead(_this,NULL);
            v_dataReaderInstanceSetTail(_this,NULL);
            v_dataReaderInstanceStateClear(_this, L_NEW);
            v_dataReaderInstanceStateSet(_this, L_EMPTY);
            v_dataReaderInstanceStateClear(_this, L_STATECHANGED);

            /* reader internal state of the data has been modified.
             * so increase readers update count.
             * This value is used by queries to determine if a query
             * needs to be reëvaluated.
             */
            v_dataReader(r)->updateCnt++;

            if (r->triggerValue) {
//                c_free(v_readerSample(r->triggerValue)->instance);
                v_dataReaderSampleFree(r->triggerValue);
                r->triggerValue = NULL;
            }
        } else {
           if (next != NULL) {
               v_dataReaderInstanceSetHead(_this, sample);
               /* break link */
               sample->next = NULL;
               next->prev = NULL; /* ref kept by local var. next */
               v_dataReaderSampleRemoveFromLifespanAdmin(next);
               v_dataReaderSampleFree(next);
               /* The instance state is in correct state,
                * so no update needed.
                */
           } /* else nothing to purge! */
        }
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);
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

void
v_dataReaderInstanceUnregister (
    v_dataReaderInstance _this,
    c_long count)
{
    v_dataReaderInstance found;
    c_bool doFree = FALSE;

    assert(C_TYPECHECK(_this,v_dataReaderInstance));

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);

    if (_this->liveliness < count) {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderInstance", 0,
                  "Incorrect instance liveliness state detected ");
        assert(FALSE);
        /* Corrective measure */
        _this->liveliness = 0;
    } else {
        _this->liveliness -= count;
    }
    /* reader internal state of the data has been modified.
     * so increase readers update count.
     * This value is used by queries to determine if a query
     * needs to be reëvaluated.
     */
    v_dataReaderInstanceReader(_this)->updateCnt++;

    if (_this->liveliness == 0) {
        if (v_dataReaderInstanceEmpty(_this)) {
            if (!v_dataReaderInstanceStateTest(_this, L_NOWRITERS)) {
                v_readerQos qos = v_reader(v_index(_this->index)->reader)->qos;
                if (qos->lifecycle.enable_invalid_samples) {
                    if (_this->sampleCount > 0) {
                        if (!v_dataReaderInstanceStateTest(_this, L_DISPOSED)) {
                            v_dataReaderInstanceStateSet(_this, L_STATECHANGED);
                        }
                    }
                }
                v_dataReaderInstanceStateSet(_this, L_NOWRITERS);
            }
            v_dataReaderRemoveInstance(v_dataReaderInstanceReader(_this),
                                       _this);
        }
    }

    CHECK_COUNT(_this);
    CHECK_EMPTINESS(_this);
    CHECK_INVALIDITY(_this);
}
