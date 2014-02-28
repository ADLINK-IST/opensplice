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
#include "v__reader.h"
#include "v__dataReader.h"
#include "v_statistics.h"
#include "v_filter.h"
#include "v_readerStatistics.h"
#include "v_dataReaderEntry.h"
#include "v_state.h"
#include "v_event.h"
#include "v__index.h"
#include "v__subscriber.h"
#include "v_projection.h"
#include "v_entity.h"
#include "v_handle.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_groupCache.h"
#include "v__dataReaderInstance.h"
#include "v__status.h"
#include "v__query.h"
#include "v__observable.h"
#include "v__observer.h"
#include "v__builtin.h"
#include "v_participant.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_qos.h"
#include "v_public.h"
#include "v__collection.h"
#include "v_instance.h"
#include "v__deadLineInstanceList.h"
#include "v__leaseManager.h"
#include "v__dataView.h"
#include "v__dataReaderSample.h"
#include "v__statisticsInterface.h"
#include "v__statCat.h"
#include "v__topic.h"
#include "v__kernel.h"
#include "v__policy.h"

#include "c_stringSupport.h"

#include "os.h"
#include "os_report.h"

#include "q_helper.h"

#define v_dataReaderAllInstanceSet(_this) \
        (v_dataReader(_this)->index->objects)

#define v_dataReaderNotEmptyInstanceSet(_this) \
        (v_dataReader(_this)->index->notEmptyList)

static const char* v_dataReaderResultStr[] = {
    "INSERTED",
    "OUTDATED",
    "NOT_OWNER",
    "MAX_SAMPLES",
    "MAX_INSTANCES",
    "INSTANCE_FULL",
    "SAMPLE_LOST",
    "DUPLICATE_SAMPLE",
    "OUT_OF_MEMORY",
    "INTERNAL_ERROR",
    "UNDETERMINED",
    "COUNT"};

const char*
v_dataReaderResultString(
    v_dataReaderResult result)
{
    assert (result <= V_DATAREADER_COUNT);

    return v_dataReaderResultStr[result];
}


static v_dataReaderInstance
dataReaderLookupInstanceUnlocked(
    v_dataReader _this,
    v_message keyTemplate)
{
    v_dataReaderInstance instance, found;

    assert(C_TYPECHECK(_this,v_dataReader));
    assert(C_TYPECHECK(keyTemplate,v_message));

    /* The following line must be removed a.s.a.p.
     * Its not intended to modify the template but there exist a dependency of
     * this side effect that needs to be removed first.
     */
    v_nodeState(keyTemplate) = L_REGISTER;

    instance = v_dataReaderInstanceNew(_this, keyTemplate);

    if (instance == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_dataReader::dataReaderLookupInstanceUnlocked", 0,
                    "Operation v_dataReaderInstanceNew(_this=0x%x, keyTemplate=0x%x) failed.",
                     _this, keyTemplate);
        found = NULL;
        assert(FALSE);
    } else {
        if (v_dataReaderQos(_this)->userKey.enable) {
            /* In case of user defined keys the NotEmpty instance set contains all
             * instances by definition and therefore the objects set that normally
             * contains all instances is not used.
             * So in that case the lookup instance must act on the Not Empty
             * instance set.
             */
            found = c_find(v_dataReaderNotEmptyInstanceSet(_this),instance);
        } else {
            found = c_find(v_dataReaderAllInstanceSet(_this), instance);
        }

        if (found != NULL) {
            if (v_dataReaderInstanceEmpty(found)) {
                if (v_stateTest(found->instanceState, L_NOWRITERS)) {
                    c_free(found);
                    found = NULL;
                }
            }
        }
        c_free(instance);
    }

    return found;
}

static void
dataReaderEntrySubscribe(
    void *o,
    void *arg)
{
    v_partition p = v_partition(o);
    v_dataReaderEntry e = v_dataReaderEntry(arg);
    v_kernel kernel;
    v_group g;

    assert(C_TYPECHECK(e,v_dataReaderEntry));
    assert(C_TYPECHECK(p,v_partition));

    kernel = v_objectKernel(e);
    if (kernel == NULL) {
        OS_REPORT_1(OS_ERROR,
                    "kernel::v_dataReader::dataReaderEntrySubscribe", 0,
                    "Operation v_objectKernel(entity=0x%x) failed.",
                     e);
    } else {
        g = v_groupSetCreate(kernel->groupSet,p,e->topic);
        if (g == NULL) {
            OS_REPORT_3(OS_ERROR,
                        "kernel::v_dataReader::dataReaderEntrySubscribe", 0,
                        "Operation v_groupSetCreate(groupSet=0x%x, partition=0x%x, topic=0x%x) failed.",
                         kernel->groupSet, p, e->topic);
        } else {
            if(v_groupPartitionAccessMode(g) == V_ACCESS_MODE_READ_WRITE ||
               v_groupPartitionAccessMode(g) == V_ACCESS_MODE_READ)
            {
                v_groupAddEntry(g,v_entry(e));
            }
            c_free(g);
        }
    }
}

static c_bool
subscribe(
    c_object entry,
    c_voidp partition)
{
    v_partition p = v_partition(partition);
    v_dataReaderEntry e = v_dataReaderEntry(entry);

    dataReaderEntrySubscribe(p, e);

    return TRUE;
}

static void
dataReaderEntryUnSubscribe(
    void *o,
    void *arg)
{
    v_partition p = v_partition(o);
    v_dataReaderEntry e = v_dataReaderEntry(arg);
    v_kernel kernel;
    v_group g;
    c_value params[2];
    c_iter list;

    assert(C_TYPECHECK(e,v_dataReaderEntry));
    assert(C_TYPECHECK(p,v_partition));

    params[0] = c_objectValue(p);
    params[1] = c_objectValue(e->topic);
    kernel = v_objectKernel(e);
    list = v_groupSetSelect(kernel->groupSet,
                            "partition = %0 and topic = %1",
                            params);
    while ((g = c_iterTakeFirst(list)) != NULL) {
        v_groupRemoveEntry(g,v_entry(e));
        c_free(g);
    }
    c_iterFree(list);
}

static c_bool
unsubscribe(
    c_object entry,
    c_voidp partition)
{
    v_partition p = v_partition(partition);
    v_dataReaderEntry e = v_dataReaderEntry(entry);

    dataReaderEntryUnSubscribe(p, e);

    return TRUE;
}

struct onNewIndexArg {
    v_dataReader dataReader;
    q_expr _where;
    c_value **params;
};



static void
onNewIndex(
    v_index index,
    v_topic topic,
    c_voidp arg)
{
    struct onNewIndexArg *a = (struct onNewIndexArg *)arg;
    v_dataReaderEntry entry = NULL, found;
    c_array filterInstance;
    c_array filterData;

    v_filterSplit(topic, a->_where, *(a->params), &filterInstance, &filterData, index);

    if ((filterInstance == NULL)||(filterData == NULL))
    {
        OS_REPORT_2(OS_ERROR,
                "kernel::v_dataReader::onNewIndex", 0,
                "v_filerSplit failed"
                OS_REPORT_NL "filterInstance = %p, filterData = %p",
                filterInstance, filterData);
    }
    else
    {
        entry = v_dataReaderEntryNew(a->dataReader, topic, filterInstance, filterData);
    }

    c_free(filterData);
    c_free(filterInstance);

    if (entry != NULL) {
        found = v_dataReaderAddEntry(a->dataReader,entry);
        if (found == entry) {
            entry->index = c_keep(index);
            index->entry = entry;
        } else {
            OS_REPORT_4(OS_ERROR,
                        "kernel::v_dataReader::onNewIndex", 0,
                        "Operation v_dataReaderAddEntry(dataReader=0x%x, entry=0x%x) failed"
                        OS_REPORT_NL "Operation returned 0x%x but expected 0x%x",
                        a->dataReader, entry, found, entry);
        }
        c_free(entry);
        c_free(found);
    } else {
        OS_REPORT_4(OS_ERROR,
                    "kernel::v_dataReader::onNewIndex", 0,
                    "Operation v_dataReaderEntryNew(dataReader=0x%x, topic=0x%x, filterInstance=0x%x, filterData=0x%x) failed.",
                     a->dataReader, topic, filterInstance, filterData);
    }
}

static c_bool
dataReaderEntryUpdatePurgeLists(
    c_object o,
    c_voidp arg)
{
    OS_UNUSED_ARG(arg);
    v_dataReaderEntryUpdatePurgeLists(v_dataReaderEntry(o));
    return TRUE; /* process all other entries */
}

/*
 * Precondition: dataReader is locked (v_dataReaderLock).
 */

c_long
v_dataReaderInstanceCount(
    v_dataReader _this)
{
    assert(C_TYPECHECK(_this,v_dataReader));

    return c_tableCount(v_dataReaderAllInstanceSet(_this));
}

void
v_dataReaderUpdatePurgeLists(
    v_dataReader _this)
{
    assert(C_TYPECHECK(_this,v_dataReader));

    v_readerWalkEntries(v_reader(_this),
                        dataReaderEntryUpdatePurgeLists,
                        NULL);
}

void
v_dataReaderUpdatePurgeListsLocked(
    v_dataReader _this)
{
    assert(C_TYPECHECK(_this,v_dataReader));

    c_setWalk(v_reader(_this)->entrySet.entries, dataReaderEntryUpdatePurgeLists, NULL);
}

#ifdef _MSG_STAMP_

#define _LAPSTAMP_(_this,_lap) { \
            if ((_lap > 0) && (_lap < 100000000))  { \
            _this.min = (_this.min < _lap ? _this.min : _lap); \
            _this.max = (_this.max > _lap ? _this.max : _lap); \
            _this.tot += _lap; \
            _this.count++; \
            } \
        }

void
v_dataReaderLogMessage(
    v_dataReader _this,
    v_message msg)
{
    v_hrtime lap;
    c_long i;

    for (i=0; i<=msg->hops; i++) {
        lap = msg->writerCopyTime[i] - msg->writerAllocTime[i];
        _LAPSTAMP_(_this->writerCopyTime[i],lap);
        lap = msg->writerLookupTime[i] - msg->writerCopyTime[i];
        _LAPSTAMP_(_this->writerLookupTime[i],lap);
        lap = msg->groupInsertTime[i] - msg->writerLookupTime[i];
        _LAPSTAMP_(_this->writerWriteTime[i],lap);
        lap = msg->groupLookupTime[i] - msg->groupInsertTime[i];
        _LAPSTAMP_(_this->groupLookupTime[i],lap);
        lap = msg->readerInsertTime[i] - msg->groupLookupTime[i];
        _LAPSTAMP_(_this->groupWriteTime[i],lap);
        lap = msg->readerLookupTime[i] - msg->readerInsertTime[i];
        _LAPSTAMP_(_this->readerLookupTime[i],lap);
        lap = msg->readerDataAvailableTime[i] - msg->readerLookupTime[i];
        _LAPSTAMP_(_this->readerInstanceTime[i],lap);
        lap = msg->readerInstanceTime[i] - msg->readerDataAvailableTime[i];
        _LAPSTAMP_(_this->readerInsertTime[i],lap);
        lap = msg->readerNotifyTime[i] - msg->readerInstanceTime[i];
        _LAPSTAMP_(_this->readerNotifyTime[i],lap);
        lap = msg->readerReadTime[i] - msg->readerNotifyTime[i];
        _LAPSTAMP_(_this->readerReadTime[i],lap);
        lap = msg->readerCopyTime[i] - msg->readerReadTime[i];
        _LAPSTAMP_(_this->readerCopyTime[i],lap);

        lap = msg->nwSerialisationTime[i] - msg->readerDataAvailableTime[i];
        _LAPSTAMP_(_this->nwSerialisationTime[i],lap);
        lap = msg->nwBufferFullTime[i] - msg->readerDataAvailableTime[i];
        _LAPSTAMP_(_this->nwBufferFullTime[i],lap);
        lap = msg->nwFlushBufferTime[i] - msg->nwBufferFullTime[i];
        _LAPSTAMP_(_this->nwFlushBufferTime[i],lap);
        lap = msg->nwSendTime[i] - msg->nwFlushBufferTime[i];
        _LAPSTAMP_(_this->nwSendTime[i],lap);
        if (i > 0) {
            lap = msg->nwReceiveTime[i] - msg->nwSendTime[i-1];
            _LAPSTAMP_(_this->nwReceiveTime[i],lap);
        }
        lap = msg->nwInsertTime[i] - msg->nwReceiveTime[i];
        _LAPSTAMP_(_this->nwInsertTime[i],lap);
    }
}
#undef _LAPSTAMP_

#define _LAPINIT_(_this) { \
            _this.cur = 0ll; \
            _this.min = 0x7fffffffffffffffll; \
            _this.max = 0ll; \
            _this.tot = 0ll; \
            _this.count = 0; \
        }

static void
v_dataReaderLogInit(
    v_dataReader _this)
{
    c_long i;

    for (i=0; i<2; i++) {
        _LAPINIT_(_this->writerCopyTime[i]);
        _LAPINIT_(_this->writerLookupTime[i]);
        _LAPINIT_(_this->writerWriteTime[i]);
        _LAPINIT_(_this->groupLookupTime[i]);
        _LAPINIT_(_this->groupWriteTime[i]);
        _LAPINIT_(_this->readerLookupTime[i]);
        _LAPINIT_(_this->readerInstanceTime[i]);
        _LAPINIT_(_this->readerInsertTime[i]);
        _LAPINIT_(_this->readerNotifyTime[i]);
        _LAPINIT_(_this->readerReadTime[i]);
        _LAPINIT_(_this->readerCopyTime[i]);
        _LAPINIT_(_this->nwSerialisationTime[i]);
        _LAPINIT_(_this->nwBufferFullTime[i]);
        _LAPINIT_(_this->nwFlushBufferTime[i]);
        _LAPINIT_(_this->nwSendTime[i]);
        _LAPINIT_(_this->nwReceiveTime[i]);
        _LAPINIT_(_this->nwInsertTime[i]);
    }
}
#undef _LAPINIT_

static void
_lapReport(
    v_laptime *laptime,
    const char *info)
{
    if (laptime->count > 0) {
        printf("%10d %16d %16d %16d (%s)\n",
            laptime->count,
            (int)(laptime->tot / 1000ll) / laptime->count,
            (int)(laptime->min / 1000ll),
            (int)(laptime->max / 1000ll),
            info);
    }
}

static void
v_dataReaderLogReport(
    v_dataReader _this)
{
    c_char *info = "DataReader message arrival stats";
    c_long i;

    if (_this->writerLookupTime[0].count > 99) {
        printf("==============================================================\n");
        printf("DataReader : %s\n", info);
        printf("--------------------------------------------------------------\n");
        printf("Nr of laps       mean (usec)       min (usec)       max (usec)\n");
        for (i=0; i<2; i++) {
            printf("--------------------------------------------------------------\n");
            _lapReport(&_this->nwReceiveTime[i],"Network: receive time");
            _lapReport(&_this->nwInsertTime[i],"Network: insert time");
            _lapReport(&_this->writerCopyTime[i],"Writer: copy data time");
            _lapReport(&_this->writerLookupTime[i],"Writer: lookup instance time");
            _lapReport(&_this->writerWriteTime[i],"Writer: write to group time");
            _lapReport(&_this->groupLookupTime[i],"Group: lookup instance time");
            _lapReport(&_this->groupWriteTime[i],"Group: write to Reader time");
            _lapReport(&_this->readerLookupTime[i],"Reader: lookup instance time");
            _lapReport(&_this->readerInstanceTime[i],"Reader: insert instance time");
            _lapReport(&_this->readerInsertTime[i],"Reader: insert time");
            _lapReport(&_this->readerNotifyTime[i],"Reader: Notify time");
            _lapReport(&_this->readerReadTime[i],"Reader: read time");
            _lapReport(&_this->readerCopyTime[i],"Reader: copy time");
            _lapReport(&_this->nwSerialisationTime[i],"Network: serialisation time");
            _lapReport(&_this->nwBufferFullTime[i],"Network: buffer full time");
            _lapReport(&_this->nwFlushBufferTime[i],"Network: buffer flush time");
            _lapReport(&_this->nwSendTime[i],"Network: send time");

        }
        printf("==============================================================\n");
    }
}

#endif

v_dataReader
v_dataReaderNew (
    v_subscriber subscriber,
    const c_char *name,
    q_expr OQLexpr,
    c_value params[],
    v_readerQos qos,
    c_bool enable)
{
    v_kernel kernel;
    v_participant participant;
    v_dataReader _this, found;
    v_readerQos q;
    v_readerStatistics readerStatistics;
    struct onNewIndexArg arg;
    q_expr expr, term, _projection, _from, _where;
    c_type instanceType;
    c_property sampleProperty;
    c_long i;
    v_topic topic;

    if (name == NULL) {
        name = "<No Name>";
    }
    assert(C_TYPECHECK(subscriber,v_subscriber));
    kernel = v_objectKernel(subscriber);
    if (!q_isFnc(OQLexpr,Q_EXPR_PROGRAM)) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_dataReader::v_dataReaderNew",0,
                    "Operation for Datareader (name=\"%s\") failed:"
                    OS_REPORT_NL "Reason: expression=0x%x is not a valid view expression.",
                    name, OQLexpr);
        return NULL;
    }
    expr = q_getPar(OQLexpr,0);
    if (!q_isFnc(expr,Q_EXPR_SELECT)) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_dataReader::v_dataReaderNew",0,
                    "Operation for Datareader (name=\"%s\") failed:"
                    OS_REPORT_NL "Reason: expression=0x%x is not a valid select statement.",
                    name, OQLexpr);
        return NULL;
    }
    _projection = NULL;
    _from = NULL;
    _where = NULL;
    i=0;
    term = q_getPar(expr,i++);
    while (term != NULL) {
        switch (q_getTag(term)) {
        case Q_EXPR_PROJECTION:
            _projection = term;
        break;
        case Q_EXPR_FROM:
            _from = q_getPar(term,0);
        break;
        case Q_EXPR_WHERE:
            _where = term;
        break;
        default:
        break;
        }
        term = q_getPar(expr,i++);
    }
    if (_from == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_dataReader::v_dataReaderNew",0,
                    "Operation for Datareader (name=\"%s\") failed:"
                    OS_REPORT_NL "Reason: Missing from clause in expression 0x%x.",
                    name, OQLexpr);
        return NULL;
    }
    topic = v_lookupTopic (kernel, q_getId(_from));
    /* ES, dds1576: Before creating the datareader we have to verify that read
     * access to the topic is allowed. We can accomplish this by checking the
     * access mode of the topic.
     */
    if(!topic)
    {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_dataReader::v_dataReaderNew",0,
                    "DataReader (name=\"%s\") not created: "
                    "Could not locate topic with name \"%s\".",
                    name, q_getId(_from));
        return NULL;
    }
    if(v_topicAccessMode(topic) != V_ACCESS_MODE_READ &&
       v_topicAccessMode(topic) != V_ACCESS_MODE_READ_WRITE)
    {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_dataReader::v_dataReaderNew", 0,
                    "Creation of DataReader (name=\"%s\") failed."
                    OS_REPORT_NL "Topic (name=\"%s\") does not have read access rights.",
                    name, q_getId(_from));
        c_free(topic);
        return NULL;
    }
    c_free(topic);
    topic = NULL;
    q = v_readerQosNew(kernel,qos);
    if (q == NULL) {
        OS_REPORT_1(OS_ERROR,
                    "kernel::v_dataReader::v_dataReaderNew", 0,
                    "DataReader (name=\"%s\") not created: inconsistent qos",
                    name);
        return NULL;
    }
    if (v_isEnabledStatistics(kernel, V_STATCAT_READER)) {
        readerStatistics = v_readerStatisticsNew(kernel);
        if (readerStatistics == NULL) {
            OS_REPORT_1(OS_ERROR,
                        "kernel::v_dataReader::v_dataReaderNew", 0,
                        "Failed to create Statistics for DataReader (name=\"%s\").",
                        name);
            assert(FALSE);
            return NULL;
        }
    } else {
        readerStatistics = NULL;
    }
    _this = v_dataReader(v_objectNew(kernel,K_DATAREADER));
    if (_this == NULL) {
        OS_REPORT_1(OS_ERROR,
                    "kernel::v_dataReader::v_dataReaderNew", 0,
                    "v_objectNew(kernel=0x%x DataReader (name=\"%s\") not created: inconsistent qos",
                    name);
        c_free(readerStatistics);
        return NULL;
    }
    _this->shareCount = 1;
    _this->views = NULL;
    _this->sampleCount = 0;
    _this->notReadCount = 0;
    _this->maxInstances = FALSE;
    _this->depth = 0x7fffffff; /* MAX_INT */
    _this->cachedSample = NULL;
    _this->triggerValue = NULL;
    _this->walkRequired = TRUE;
    _this->updateCnt = 0;
#define _SL_
#ifdef _SL_
    _this->cachedSampleCount = 0;
#endif
    _this->readCnt = 0;

#ifdef _MSG_STAMP_
    v_dataReaderLogInit(_this);
#endif

    v_readerInit(v_reader(_this),name,
                 subscriber,q,
                 v_statistics(readerStatistics),
                 enable);

    if (q->share.enable) {
        if(!subscriber->qos->share.enable){
            OS_REPORT(OS_ERROR,
                      "kernel::v_dataReader::v_dataReaderNew", 0,
                      "Creating a shared dataReader in a non-shared subscriber.");
            assert(FALSE);
        }
        v_subscriberLockShares(subscriber);
        found = v_dataReader(v_subscriberAddShareUnsafe(subscriber,v_reader(_this)));
        if (found != _this) {
           /* Existing shared DataReader so abort reader creation and
            * return existing DataReader.
            */
           /* Make sure to set the index and deadline list to NULL,
            * because v_publicFree will cause a crash in the
            * v_dataReaderDeinit otherwise.
            */
            _this->index = NULL;
            _this->deadLineList = NULL;
            /*v_publicFree to free reference held by the handle server.*/
            v_publicFree(v_public(_this));
            /*Now free the local reference as well.*/
            c_free(_this);
            pa_increment(&(found->shareCount));
            v_subscriberUnlockShares(subscriber);
            c_free(q);
            return c_keep(found);
        }
    }
    c_free(q);

    arg.dataReader = _this;
    arg._where = _where;
    arg.params = &params;
    _this->index = v_indexNew(_this, _from, onNewIndex, &arg);
    if (_this->index) {
        instanceType = v_dataReaderInstanceType(_this);
        sampleProperty = c_property(c_metaResolve(c_metaObject(instanceType),
                                                  "sample"));
        c_free(instanceType);
        /* the sampleExtent is created with the synchronized parameter
         * set to TRUE.
         * So the extent will use a mutex to guarantee reentrancy.
         * This is needed because samples are kept, copied and freed
         * outside the reader lock.
         */
        if (sampleProperty != NULL) {
            _this->sampleType = c_keep (sampleProperty->type);
            _this->projection = v_projectionNew(_this,_projection);

            participant = v_participant(subscriber->participant);
            assert(participant != NULL);
            _this->deadLineList =
            v_deadLineInstanceListNew(c_getBase(c_object(_this)),
                                      participant->leaseManager,
                                      q->deadline.period,
                                      V_LEASEACTION_READER_DEADLINE_MISSED,
                                      v_public(_this));

            if (enable) {
/* TODO : result checking and error propagation */
                v_dataReaderEnable(_this);
            }
        } else {
            OS_REPORT_2(OS_ERROR,
                        "kernel::v_dataReader::v_dataReaderNew", 0,
                        "Creation of DataReader (name=\"%s\") failed."
                        OS_REPORT_NL "Operation c_metaResolve(scope=0x%x, \"sample\") failed.",
                        name, instanceType);
            v_readerDeinit(v_reader(_this));
            c_free(_this);
            _this = NULL;
        }
    } else {
        OS_REPORT_5(OS_ERROR,
                    "kernel::v_dataReader::v_dataReaderNew", 0,
                    "Creation of DataReader (name=\"%s\") failed."
                    OS_REPORT_NL
                    "Operation v_indexNew(_this=0x%x, _from=0x%x, action=0x%x, arg=0x%x) failed.",
                    name, _this, _from, onNewIndex, &arg);
        v_readerDeinit(v_reader(_this));
        c_free(_this);
        _this = NULL;
    }
    if (q->share.enable) {
        v_subscriberUnlockShares(subscriber);
    }
    return _this;
}

v_result
v_dataReaderEnable(
    v_dataReader _this)
{
    v_kernel kernel;
    v_message builtinMsg;
    v_subscriber subscriber;
    v_readerQos qos;
    v_result result;

    if (_this) {
        result = V_RESULT_OK;
        qos = v_reader(_this)->qos;
        subscriber = v_subscriber(v_reader(_this)->subscriber);

        if (qos->history.kind == V_HISTORY_KEEPLAST) {
            if (qos->history.depth >= 0) {
                if (qos->history.depth == 0) {
                    OS_REPORT(OS_WARNING,
                              "instance",0,
                              "history depth is 0");
                }
                _this->depth = qos->history.depth;
            }
        } else {
            if (qos->resource.max_samples_per_instance >= 0) {
                if (qos->resource.max_samples_per_instance == 0) {
                    OS_REPORT(OS_WARNING,
                              "instance",0,
                              "max samples per instance is 0");
                }
                _this->depth = qos->resource.max_samples_per_instance;
            }
        }

/* TODO : result checking and error propagation */
        v_subscriberAddReader(subscriber,v_reader(_this));

        kernel = v_objectKernel(_this);
        builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin, _this);
        v_writeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
        c_free(builtinMsg);
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

static c_bool
instanceFree(
    c_object o,
    c_voidp arg)
{
    OS_UNUSED_ARG(arg);
    v_publicFree(o);
    return TRUE;
}

void
v_dataReaderFree (
    v_dataReader _this)
{
    v_message builtinMsg;
    v_message unregisterMsg;
    v_dataReader found;
    v_subscriber subscriber;
    v_kernel kernel;
    v_dataView view;
    c_iter views;
    long sc;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    sc = pa_decrement(&(_this->shareCount));
    if (sc > 0) return;

    if(sc == 0){
#ifdef _MSG_STAMP_
        v_dataReaderLogReport(_this);
#endif
        /* First create message, only at the end dispose. Applications expect
           the disposed sample to be the last!
        */
        kernel = v_objectKernel(_this);
        builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin,_this);
        unregisterMsg = v_builtinCreateSubscriptionInfo(kernel->builtin,_this);

        if (v_reader(_this)->qos->share.enable) {
            subscriber = v_subscriber(v_reader(_this)->subscriber);
            if (subscriber != NULL) {
                found = v_dataReader(v_subscriberRemoveShare(subscriber,v_reader(_this)));
                assert(found == _this);
                c_free(found);
            }
        }
        v_readerFree(v_reader(_this));
        v_dataReaderLock(_this);
        if(_this->deadLineList){
            v_deadLineInstanceListFree(_this->deadLineList);
        }
        if (_this->views != NULL) {
            views = ospl_c_select(_this->views, 0);
            view = v_dataView(c_iterTakeFirst(views));
            while (view != NULL) {
                v_dataViewFreeUnsafe(view);
                view = v_dataView(c_iterTakeFirst(views));
            }
            c_iterFree(views);
        }
        if (_this->triggerValue) {
            v_dataReaderTriggerValueFree(_this->triggerValue);
            _this->triggerValue = NULL;
        }
        if(_this->index){
            (void) c_tableWalk(v_dataReaderAllInstanceSet(_this),instanceFree,NULL); /* Always returns TRUE. */
            (void) c_tableWalk(v_dataReaderNotEmptyInstanceSet(_this), instanceFree, NULL); /* Always returns TRUE. */
        }
        v_dataReaderUnLock(_this);

        v_writeDisposeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
        v_unregisterBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, unregisterMsg);
        c_free(builtinMsg);
        c_free(unregisterMsg);
    } else {
        OS_REPORT_1(OS_ERROR,  "v_dataReaderFree", 0,
                "dataReader already freed (shareCount is now %d).", sc);
        assert(sc == 0);
    }
}

static c_bool
resetInstanceOwner(
    c_object obj,
    c_voidp arg)
{
    v_dataReaderInstanceResetOwner(v_dataReaderInstance(obj), *((v_gid*)(arg)));

    return TRUE;
}

void
v_dataReaderDeinit (
    v_dataReader _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));
    v_readerDeinit(v_reader(_this));
}

/* Help function for writing into the dataViews */

static v_actionResult
writeSlave(
    c_object sample,
    c_voidp arg)
{
    c_bool result = TRUE;

    if (v_readerSampleTestState(sample, L_VALIDDATA))
    {
        result = v_dataViewWrite(v_dataView(arg), v_readerSample(sample));
    }
    return result;
}

static c_bool
walkInstanceSamples(
    c_object o,
    c_voidp arg)
{
    v_dataReaderInstanceWalkSamples(v_dataReaderInstance(o),
                                    writeSlave, arg);
    return TRUE;
}

void
v_dataReaderInsertView(
    v_dataReader _this,
    v_dataView view)
{
    c_base base;
    c_type type;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));
    assert(view != NULL);
    assert(C_TYPECHECK(view, v_dataView));

    v_dataReaderLock(_this);
    /* Create set if it does not exist yet */
    if (_this->views == NULL) {
        base = c_getBase((c_object)_this);
        type = c_resolve(base, "kernelModule::v_dataView");
        _this->views = c_setNew(type);
    }
    /* Insert the view in the set */
    c_insert(_this->views, view);
    /* Fill the view with initial data */
    c_tableWalk(v_dataReaderNotEmptyInstanceSet(_this),
                walkInstanceSamples,
                view);

    v_dataReaderUnLock(_this);
}

void
v_dataReaderRemoveViewUnsafe(
    v_dataReader _this,
    v_dataView view)
{
    v_dataView found;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));
    assert(view != NULL);
    assert(C_TYPECHECK(view, v_dataView));

    if (_this->views != NULL) {
        found = c_remove(_this->views, view, NULL, NULL);
        assert(found == view);
        if (found == view) {
            c_free(found);
            if (c_count(_this->views) == 0) {
                c_free(_this->views);
                _this->views = NULL;
            }
        }
    }
    /* Remove all data from the view */
    v_dataViewWipeSamples(view);
}

void
v_dataReaderRemoveView(
    v_dataReader _this,
    v_dataView view)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));
    assert(view != NULL);
    assert(C_TYPECHECK(view, v_dataView));

    v_dataReaderLock(_this);
    v_dataReaderRemoveViewUnsafe(_this, view);
    v_dataReaderUnLock(_this);
}

C_STRUCT(readSampleArg) {
    v_dataReader reader;
    c_query query;
    v_readerSampleAction action;
    c_voidp arg;
    c_iter emptyList;
};
C_CLASS(readSampleArg);



c_bool
v_dataReaderWalkInstances (
    v_dataReader _this,
    v_dataReaderInstanceAction action,
    c_voidp arg)
{
    c_bool proceed;

    if (_this != NULL) {
        assert(C_TYPECHECK(_this, v_dataReader));
        proceed = c_readAction(v_dataReaderNotEmptyInstanceSet(_this),
                              (c_action)action,
                              arg);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,"v_dataReaderWalkInstances",0,
                   "dataReader object is NULL");
    }
    assert(_this != NULL);

    return proceed;

}


static c_bool
instanceReadSamples(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    readSampleArg a = (readSampleArg)arg;
    c_bool proceed = TRUE;
    c_long count, oldCount;

    assert(C_TYPECHECK(a->reader, v_dataReader));

    if (!v_dataReaderInstanceEmpty(instance)) {
        oldCount = v_dataReaderInstanceSampleCount(instance);
        assert(oldCount >= 0);
        proceed = v_dataReaderInstanceReadSamples(instance,
                                                  a->query,
                                                  a->action,
                                                  a->arg);
        count = oldCount - v_dataReaderInstanceSampleCount(instance);
        assert(count >= 0);
        v_dataReader(a->reader)->sampleCount -= count;
        assert(v_dataReader(a->reader)->sampleCount >= 0);
        /* Note that if the instance has become empty it is not
         * removed from the not empty list yet!
         * An empty instance will be removed the next time the instance
         * is accessed (see the following else branch).
         * This is an optimization to avoid continuous inserting and
         * removing instances in case samples are continuous written
         * and taken.
         */
    } else {
        if (v_dataReaderInstanceInNotEmptyList(instance)) {
            /* Apparently the instance was already empty and as
             * optimization described above left in the not empty list.
             * And no data is inserted in the meanwhile meaning that
             * the use case for optimization is not the case.
             * So the instance can now be registered to be removed.
             */
            c_keep(instance);
            a->emptyList = c_iterInsert(a->emptyList,instance);
        }
    }

    return proceed;
}

/* The read and take actions */
c_bool
v_dataReaderRead(
    v_dataReader _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;
    C_STRUCT(readSampleArg) argument;
    v_dataReaderInstance emptyInstance;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));

    /* Collect entries for purging outside of observer lock to prevent deadlock
     * between observer, entrySet and group locks with three different threads.
     */

    v_dataReaderLock(_this);
    _this->readCnt++;

    v_dataReaderUpdatePurgeListsLocked(_this);

    argument.reader = _this;
    argument.action = action;
    argument.arg = arg;
    argument.query = NULL;
    argument.emptyList = NULL;

    proceed = c_tableReadCircular(v_dataReaderNotEmptyInstanceSet(_this),
                           (c_action)instanceReadSamples,
                           &argument);

    /* The state of an instance can also change because of a read action
     * in case of an invalid sample.
     */
    if (argument.emptyList != NULL) {
        emptyInstance = c_iterTakeFirst(argument.emptyList);
        while (emptyInstance != NULL) {
            v_dataReaderRemoveInstance(_this,emptyInstance);
            c_free(emptyInstance);
            emptyInstance = c_iterTakeFirst(argument.emptyList);
        }
        c_iterFree(argument.emptyList);
        v_statisticsULongSetValue(v_reader,
                                  numberOfInstances,
                                  _this,
                                  v_dataReaderInstanceCount(_this));
    }
    v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);

    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfReads, _this);
    v_dataReaderUnLock(_this);

    return proceed;
}

c_array
v_dataReaderKeyList(
    v_dataReader _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));

    return v_indexKeyList(_this->index);
}

c_array
v_dataReaderSourceKeyList(
    v_dataReader _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));

    return v_indexSourceKeyList(_this->index);
}

c_bool
v_dataReaderReadInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));

    if (instance == NULL) {
        return FALSE;
    }

    assert(C_TYPECHECK(instance, v_dataReaderInstance));

    /* Collect entries for purging outside of observer lock to prevent deadlock
     * between observer, entrySet and group locks with three different threads.
     */
    v_dataReaderLock(_this);
    _this->readCnt++;
    if (!v_dataReaderInstanceEmpty(instance)) {
        v_dataReaderUpdatePurgeListsLocked(_this);
        proceed = v_dataReaderInstanceReadSamples(instance,NULL,action,arg);
        v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);

        if (v_dataReaderInstanceEmpty(instance)) {
            v_dataReaderRemoveInstance(_this,instance);
        }
    } else {
        proceed = TRUE;
    }
    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfInstanceReads, _this);
    v_dataReaderUnLock(_this);

    return proceed;
}

c_bool
v_dataReaderReadNextInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_dataReaderInstance next, cur;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));
    assert(C_TYPECHECK(instance, v_dataReaderInstance));

    v_dataReaderLock(_this);
    _this->readCnt++;

    v_dataReaderUpdatePurgeListsLocked(_this);

    cur = v_dataReaderNextInstance(_this,instance);
    while ((cur != NULL) && v_dataReaderInstanceEmpty(cur))
    {
        next = v_dataReaderNextInstance(_this,cur);
        v_dataReaderRemoveInstance(_this,cur);
        cur = next;
    }

    if (cur != NULL) {
        proceed = v_dataReaderInstanceReadSamples(cur, NULL,action,arg);
        if (v_dataReaderInstanceEmpty(cur)) {
            v_dataReaderRemoveInstance(_this,cur);
        }
    }

    v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);

    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfNextInstanceReads, _this);
    v_dataReaderUnLock(_this);

    return proceed;
}

C_STRUCT(takeSampleArg) {
    v_dataReader reader;
    c_query query;
    v_readerSampleAction action;
    c_voidp arg;
    c_iter emptyList;
};
C_CLASS(takeSampleArg);

static c_bool
instanceTakeSamples(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    takeSampleArg a = (takeSampleArg)arg;
    c_long count, oldCount;

    assert(C_TYPECHECK(a->reader, v_dataReader));
    assert(v_dataReader(a->reader)->sampleCount >= 0);

    if (v_dataReaderInstanceEmpty(instance)) {
        if (v_dataReaderInstanceInNotEmptyList(instance)) {
            c_keep(instance);
            a->emptyList = c_iterInsert(a->emptyList,instance);
        }
        return proceed;
    }
    oldCount = v_dataReaderInstanceSampleCount(instance);
    assert(oldCount >= 0);
    proceed = v_dataReaderInstanceTakeSamples(instance,
                                              a->query,
                                              a->action,
                                              a->arg);
    count = oldCount - v_dataReaderInstanceSampleCount(instance);
    assert(count >= 0);
    v_dataReader(a->reader)->sampleCount -= count;
    assert(v_dataReader(a->reader)->sampleCount >= 0);

    v_statisticsULongSetValue(v_reader,
                              numberOfSamples,
                              a->reader,
                              v_dataReader(a->reader)->sampleCount);
	v_statisticsULongValueAdd(v_reader,
                              numberOfSamplesTaken,
                              a->reader,
                              count);

#if 0
      /* This code snippet is functionally correct, but in typical
       * pingpong like scenario'sit might result in the same instance
       * being inserted and removed from the non-emptyList continuously.
       * So postponing the decision to move the instance around can be
       * seen as an optimization for these pingpong like scenario's.
       *
       * So this snippet of code intends to avoid leakage, but is also
       * executed by other functions that operate on the v_dataReaderInstance.
       * So if we don't clean-up the instance now, it will be done in
       * a more lazy manner by subsequent operations on the same
       * v_dataReaderInstance.
       *
       * Code like this can permanently be deleted as soon as active
       * garbage collection is implemented (scdds1817)
       */
    if (v_dataReaderInstanceEmpty(instance)) {
        if (!c_iterContains(a->emptyList, instance)) {
             a->emptyList = c_iterInsert(a->emptyList,c_keep(instance));
        }
    }
#endif
    return proceed;
}

static c_bool
only_if_equal (
    c_object found,
    c_object requested,
    c_voidp arg)
{
    *(c_bool *)arg = (found == requested);
    return *(c_bool *)arg;
}

void
v_dataReaderRemoveInstance(
    v_dataReader _this,
    v_dataReaderInstance instance)
{
    v_dataReaderInstance found;
    c_object instanceSet;
    c_bool equal, doFree;

    assert(C_TYPECHECK(_this, v_dataReader));
    assert(v_dataReaderInstanceEmpty(instance));

    doFree = FALSE;

    if (v_dataReaderInstanceInNotEmptyList(instance)) {
        instanceSet = v_dataReaderNotEmptyInstanceSet(_this);
        equal = FALSE;
        found = v_dataReaderInstance(c_remove(instanceSet,
                                              instance,
                                              only_if_equal,
                                              &equal));
        if (equal) {
            v_dataReaderInstanceInNotEmptyList(found) = FALSE;
            c_free(found);
            /* A v_publicFree is needed here as well, but due to the
             * fact that the instance actually might be free by that, do
             * it in the end of the routine for subscriber defined keys
             * readers only.
             */
            doFree = TRUE;
        } else {
            /* The instance apparently isn't a member of the
             * NotEmptySet but there is another instance with equal
             * key values.
             * This is unexpected and should be analyzed if this
             * happens.
             * For now the instance is not removed.
             */
#ifndef NDEBUG
            OS_REPORT(OS_WARNING,
                      "v_dataReaderInstanceRemove",0,
                      "try removed incorrect instance from NotEmptySet");
#endif
        }
    }

    if (!v_reader(_this)->qos->userKey.enable) {
        if (v_dataReaderInstanceNoWriters(instance)) {
            instanceSet = v_dataReaderAllInstanceSet(_this);
            equal = FALSE;
            found = v_dataReaderInstance(c_remove(instanceSet,
                                                  instance,
                                                  only_if_equal,
                                                  &equal));
            if (equal) {
                v_deadLineInstanceListRemoveInstance(_this->deadLineList,
                                                     v_instance(instance));
                /* Remove the instance from the instance-statistics
                 * administration.
                 */
                UPDATE_READER_STATISTICS_REMOVE_INSTANCE(_this->index,
                                                         instance);
                instance->purgeInsertionTime = C_TIME_MIN_INFINITE;
                v_dataReaderInstanceStateSet(instance, L_REMOVED);
                v_publicFree(v_public(instance));
                c_free(found);
            } else {
                /* The instance apparently isn't a member of the
                 * DataReader but there is another instance with equal
                 * key values.
                 * This is unexpected and should be analyzed if this
                 * happens.
                 * For now the instance is not removed.
                 */
#ifndef NDEBUG
                OS_REPORT(OS_WARNING,
                          "v_dataReaderInstanceRemove",0,
                          "try removed incorrect instance");
#endif
            }
        }
    } else if(doFree){
        v_publicFree(v_public(instance));
    }
}

c_bool
v_dataReaderTake(
    v_dataReader _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;
    C_STRUCT(takeSampleArg) argument;
    v_dataReaderInstance emptyInstance;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));

    /* Collect entries for purging outside of observer lock to prevent deadlock
     * between observer, entrySet and group locks with three different threads.
     */
    v_dataReaderLock(_this);
    _this->readCnt++;

    v_dataReaderUpdatePurgeListsLocked(_this);

    argument.action = action;
    argument.arg = arg;
    argument.query = NULL;
    argument.reader = _this;
    argument.emptyList = NULL;

    proceed = c_tableReadCircular(v_dataReaderNotEmptyInstanceSet(_this),
                           (c_action)instanceTakeSamples,
                           &argument);
    if (argument.emptyList != NULL) {
        emptyInstance = c_iterTakeFirst(argument.emptyList);
        while (emptyInstance != NULL) {
            v_dataReaderRemoveInstance(_this,emptyInstance);
            c_free(emptyInstance);
            emptyInstance = c_iterTakeFirst(argument.emptyList);
        }
        c_iterFree(argument.emptyList);
        v_statisticsULongSetValue(v_reader,
                                  numberOfInstances,
                                  _this,
                                  v_dataReaderInstanceCount(_this));
    }
    v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);

    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfTakes, _this);
    v_dataReaderUnLock(_this);

    return proceed;
}

c_bool
v_dataReaderTakeInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    c_long count;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));
    assert(C_TYPECHECK(instance, v_dataReaderInstance));

    if (instance == NULL) {
        return FALSE;
    }

    /* Collect entries for purging outside of observer lock to prevent deadlock
     * between observer, entrySet and group locks with three different threads.
     */

    v_dataReaderLock(_this);
    _this->readCnt++;

    if (!v_dataReaderInstanceEmpty(instance)) {
        v_dataReaderUpdatePurgeListsLocked(_this);
        count = v_dataReaderInstanceSampleCount(instance);
        proceed = v_dataReaderInstanceTakeSamples(instance,NULL,action,arg);
        count -= v_dataReaderInstanceSampleCount(instance);
        assert(count >= 0);
        if (count > 0) {
            _this->sampleCount -= count;
            v_statisticsULongSetValue(v_reader,
                                      numberOfSamples,
                                      _this,
                                      _this->sampleCount);
            assert(_this->sampleCount >= 0);
            v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);

            if (v_dataReaderInstanceEmpty(instance)) {
                v_dataReaderRemoveInstance(_this,instance);
            }

        }
    } else {
        v_dataReaderRemoveInstance(_this,instance);
    }
    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfInstanceTakes, _this);
    v_dataReaderUnLock(_this);

    return proceed;
}

c_bool
v_dataReaderTakeNextInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    v_dataReaderInstance next, cur;
    c_bool proceed = TRUE;
    c_long count;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));
    assert(C_TYPECHECK(instance, v_dataReaderInstance));

    /* Collect entries for purging outside of observer lock to prevent deadlock
     * between observer, entrySet and group locks with three different threads.
     */
    v_dataReaderLock(_this);
    _this->readCnt++;

    v_dataReaderUpdatePurgeListsLocked(_this);

    cur = v_dataReaderNextInstance(_this,instance);

    while ((cur != NULL) && v_dataReaderInstanceEmpty(cur))
    {
        next = v_dataReaderNextInstance(_this,cur);
        v_dataReaderRemoveInstance(_this,cur);
        cur = next;
    }

    if (cur != NULL) {
        count = v_dataReaderInstanceSampleCount(cur);
        proceed = v_dataReaderInstanceTakeSamples(cur,
                                                  NULL,
                                                  action,
                                                  arg);
        count -= v_dataReaderInstanceSampleCount(cur);
        assert(count >= 0);
        if (count > 0) {
            _this->sampleCount -= count;
            assert(_this->sampleCount >= 0);

            if (v_dataReaderInstanceEmpty(cur)) {
                v_dataReaderRemoveInstance(_this,cur);
            }
        }
    }
    v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);

    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfNextInstanceTakes, _this);
    v_dataReaderUnLock(_this);

    return proceed;
}

void
v_dataReaderNotify(
    v_dataReader _this,
    v_event event,
    c_voidp userData)
{
    /* This Notify method is part of the observer-observable pattern.
     * It is designed to be invoked when _this object as observer receives
     * an event from an observable object.
     * It must be possible to pass the event to the subclass of itself by
     * calling <subclass>Notify(_this, event, userData).
     * This implies that _this cannot be locked within any Notify method
     * to avoid deadlocks.
     * For consistency _this must be locked by v_observerLock(_this) before
     * calling this method.
     */
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));

    OS_UNUSED_ARG(userData);

    if (event != NULL) {

#define _NOTIFICATION_MASK_ \
        V_EVENT_INCONSISTENT_TOPIC | \
        V_EVENT_SAMPLE_REJECTED | \
        V_EVENT_SAMPLE_LOST | \
        V_EVENT_DEADLINE_MISSED | \
        V_EVENT_INCOMPATIBLE_QOS | \
        V_EVENT_TOPIC_MATCHED | \
        V_EVENT_LIVELINESS_CHANGED | \
        V_EVENT_DATA_AVAILABLE | \
        V_EVENT_ALL_DATA_DISPOSED

        v_entity(_this)->status->state |= (event->kind & (_NOTIFICATION_MASK_));
#undef _NOTIFICATION_MASK_
    }
}

static c_bool
queryNotifyDataAvailable(
    c_object query,
    c_voidp arg)
{
    v_query q = v_query(query);
    v_event event = (v_event)arg;

    if (v_observableCount(v_observable(q)) > 0) {
        event->source = v_publicHandle(v_public(query));
        v_dataReaderQueryNotifyDataAvailable(v_dataReaderQuery(query),event);
    } else {
        v_dataReaderQuery(query)->walkRequired = TRUE;
    }

    return TRUE;
}

void
v_dataReaderNotifyDataAvailable(
    v_dataReader _this,
    v_dataReaderSample sample)
{
    C_STRUCT(v_event) event;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));
    assert(C_TYPECHECK(sample,v_dataReaderSample));

    v_entity(_this)->status->state |= V_EVENT_DATA_AVAILABLE;

    if (_this->triggerValue) {
        v_dataReaderTriggerValueFree(_this->triggerValue);
        _this->triggerValue = NULL;
    }

    if (sample) {
        _this->triggerValue = v_dataReaderTriggerValueKeep(sample);
    }

    event.kind     = V_EVENT_DATA_AVAILABLE;
    event.source   = V_HANDLE_NIL;
    event.userData = sample;

    c_setWalk(v_collection(_this)->queries,
              queryNotifyDataAvailable,
              &event);

    /* Also notify myself, since the user reader might be waiting. */
    event.source = v_publicHandle(v_public(_this));

    /* Notify myself! However if myself is attached to myself then this
     * call wasn't needed because the next call to v_observableNotify would
     * perform the trigger to myself!
     */
    v_observerNotify(v_observer(_this), &event, NULL);

    v_observableNotify(v_observable(_this), &event);
}

void
v_dataReaderNotifySampleRejected(
    v_dataReader _this,
    v_sampleRejectedKind kind,
    v_gid instanceHandle)
{
    c_bool changed;
    C_STRUCT(v_event) event;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    changed = v_statusNotifySampleRejected(v_entity(_this)->status,
                                           kind,instanceHandle);
    if (changed) {
        /* Also notify myself, since the user reader might be waiting. */
        event.kind = V_EVENT_SAMPLE_REJECTED;
        event.source = v_publicHandle(v_public(_this));
        event.userData = NULL;
        v_observerNotify(v_observer(_this), &event, NULL);
        v_observableNotify(v_observable(_this), &event);
    }
}

void
v_dataReaderNotifySampleLost(
    v_dataReader _this,
    c_ulong nrSamplesLost)
{
    c_bool changed;
    C_STRUCT(v_event) event;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    changed = v_statusNotifySampleLost(v_entity(_this)->status,nrSamplesLost);
    if (changed) {
        /* Also notify myself, since the user reader might be waiting. */
        event.kind = V_EVENT_SAMPLE_LOST;
        event.source = v_publicHandle(v_public(_this));
        event.userData = NULL;
        v_observerNotify(v_observer(_this), &event, NULL);
        v_observableNotify(v_observable(_this), &event);
    }
}

void
v_dataReaderNotifyIncompatibleQos(
    v_dataReader _this,
    v_policyId   id,
    v_gid        writerGID)
{
    c_bool changed;
    C_STRUCT(v_event) event;

    OS_UNUSED_ARG(writerGID);
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    v_observerLock(v_observer(_this));

    changed = v_statusNotifyIncompatibleQos(v_entity(_this)->status, id);

    if (changed) {
        /* Also notify myself, since the user reader might be waiting. */
        event.kind = V_EVENT_INCOMPATIBLE_QOS;
        event.source = v_publicHandle(v_public(_this));
        event.userData = NULL;
        v_observerNotify(v_observer(_this), &event, NULL);
        v_observableNotify(v_observable(_this), &event);
    }

    v_observerUnlock(v_observer(_this));
}

void
v_dataReaderNotifySubscriptionMatched (
        v_dataReader _this,
    v_gid        writerGID,
    c_bool       dispose)
{
    c_bool changed;
    C_STRUCT(v_event) e;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    v_dataReaderLock(_this);

    changed = v_statusNotifySubscriptionMatched(v_entity(_this)->status, writerGID, dispose);
    if (changed) {
        e.kind = V_EVENT_TOPIC_MATCHED;
        e.source = v_publicHandle(v_public(_this));
        e.userData = NULL;
        v_observerNotify(v_observer(_this), &e, NULL);
        v_observableNotify(v_observable(_this), &e);
    }

    v_dataReaderUnLock(_this);
}

static c_bool
updateConnections(
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry e = v_dataReaderEntry(o);
    v_dataReaderConnectionChanges *a = (v_dataReaderConnectionChanges *)arg;

    c_iterWalk(a->addedPartitions, dataReaderEntrySubscribe, e);
    c_iterWalk(a->removedPartitions, dataReaderEntryUnSubscribe, e);

    return TRUE;
}

/* only called by v_subscriberSetQos() and v_readerSetQos(). */
void
v_dataReaderUpdateConnections(
    v_dataReader _this,
    v_dataReaderConnectionChanges *arg)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    if ((arg != NULL) &&
        ((arg->addedPartitions != NULL) || (arg->removedPartitions != NULL))) {
        /* partition policy has changed */
        v_readerWalkEntries(v_reader(_this), updateConnections, arg);
    }
}

/* only called by v_subscriberSetQos() and v_readerSetQos(). */
void
v_dataReaderNotifyChangedQos(
    v_dataReader _this)
{
    v_kernel kernel;
    v_message builtinMsg;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    /* publish subscription info */
    v_dataReaderLock(_this);
    kernel = v_objectKernel(_this);
    builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin, _this);
    v_deadLineInstanceListSetDuration(_this->deadLineList,v_reader(_this)->qos->deadline.period);

    v_dataReaderUnLock(_this);
    v_writeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
    c_free(builtinMsg);
}

void
v_dataReaderNotifyLivelinessChanged(
    v_dataReader _this,
    v_gid wGID,
    enum v_statusLiveliness oldLivState,
    enum v_statusLiveliness newLivState,
    v_message publicationInfo)
{
    c_bool changed;
    C_STRUCT(v_event) event;

    OS_UNUSED_ARG(publicationInfo);
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));
    assert(oldLivState != V_STATUSLIVELINESS_COUNT);
    assert(newLivState != V_STATUSLIVELINESS_COUNT);

    changed = FALSE;
    if (oldLivState != newLivState) {
        v_dataReaderLock(_this);
        /**
         * Transition table:
         *
         *                          old
         *              UNKNOWN  ALIVE NOTALIVE  DELETED
         *
         *    UNKNOWN      -       -       -        -
         *
         * n  ALIVE        B       -       C        B
         * e
         * w  NOTALIVE     B       C       -        B
         *
         *    DELETED      -       A       A        -
         *
         * Actions:
         * -: No action
         * A: the new state is DELETED, so depending on the old state
         *    the active or inactive must be decremented.
         *    And the writer must be unregistered with this reader.
         * B: the old state is UNKNOWN or DELETED, so this is the first
         *    time we see this writer. Depending on the new state this
         *    writer is alive or not alive to us.
         * C: the writer is already known, only it state has changed
         *    ALIVE -> NOTALIVE or NOTALIVE->ALIVE. In case of the first
         *    transition, the writer must also be unregistered.
         */
        if (newLivState == V_STATUSLIVELINESS_DELETED) { /* A */
            switch(oldLivState) {
            case V_STATUSLIVELINESS_ALIVE:
                changed = v_statusNotifyLivelinessChanged(
                        v_entity(_this)->status, -1, 0, wGID);
            break;
            case V_STATUSLIVELINESS_NOTALIVE:
                changed = v_statusNotifyLivelinessChanged(
                        v_entity(_this)->status, 0, -1, wGID);
            break;
            default: /* no action! */
            break;
            }
        } else {
            if ((oldLivState == V_STATUSLIVELINESS_UNKNOWN) ||
                (oldLivState == V_STATUSLIVELINESS_DELETED)) { /* B */
                switch(newLivState) {
                case V_STATUSLIVELINESS_ALIVE:
                    changed = v_statusNotifyLivelinessChanged(
                            v_entity(_this)->status, 1, 0, wGID);
                break;
                case V_STATUSLIVELINESS_NOTALIVE:
                    changed = v_statusNotifyLivelinessChanged(
                            v_entity(_this)->status, 0, 1, wGID);

                    if(v_reader(_this)->qos->ownership.kind == V_OWNERSHIP_EXCLUSIVE){
                        c_tableWalk(v_dataReaderAllInstanceSet(_this),resetInstanceOwner,&wGID);
                    }
                break;
                default: /* no action! */
                break;
                }
            } else {
                if (newLivState != V_STATUSLIVELINESS_UNKNOWN) { /* C: ALIVE or NOTALIVE */
                    assert((newLivState == V_STATUSLIVELINESS_ALIVE) ||
                           (newLivState == V_STATUSLIVELINESS_NOTALIVE));
                    if (newLivState == V_STATUSLIVELINESS_ALIVE) {
                        changed = v_statusNotifyLivelinessChanged(
                                v_entity(_this)->status, 1, -1, wGID);
                    } else {
                        assert(newLivState == V_STATUSLIVELINESS_NOTALIVE);
                        changed = v_statusNotifyLivelinessChanged(
                                v_entity(_this)->status, -1, 1, wGID);

                        if(v_reader(_this)->qos->ownership.kind == V_OWNERSHIP_EXCLUSIVE){
                            c_tableWalk(v_dataReaderAllInstanceSet(_this),resetInstanceOwner,&wGID);
                        }
                    }
                }
            }
        }
        if (changed) {
            event.kind = V_EVENT_LIVELINESS_CHANGED;
            event.source = v_publicHandle(v_public(_this));
            event.userData = NULL;
            v_observerNotify(v_observer(_this), &event, NULL);
            v_observableNotify(v_observable(_this), &event);
        }
        v_dataReaderUnLock(_this);
    }
}

static c_bool
v__dataReaderNotifyOwnershipStrengthChangedCallback (
    c_object obj,
    c_voidp arg)
{
    assert (C_TYPECHECK (obj, v_dataReaderInstance));
    assert (arg != NULL);

    (void)v_determineOwnershipByStrength (
       &(v_dataReaderInstance (obj)->owner),
        (struct v_owner *)(arg),
        (v_dataReaderInstance (obj)->liveliness > 0));

    return TRUE;
}

void
v_dataReaderNotifyOwnershipStrengthChanged (
    v_dataReader _this,
    struct v_owner *ownership)
{
    assert (C_TYPECHECK (_this, v_dataReader));
    assert (ownership != NULL);

    v_dataReaderLock(_this);
    (void)c_tableWalk (
        v_dataReaderAllInstanceSet (_this),
       &v__dataReaderNotifyOwnershipStrengthChangedCallback,
        ownership);
    v_dataReaderUnLock(_this);
}

c_type
v_dataReaderInstanceType(
    v_dataReader _this)
{
    assert(C_TYPECHECK(_this,v_dataReader));
    return c_subType(v_dataReaderAllInstanceSet(_this)); /* pass refCount */
}

c_type
v_dataReaderSampleType(
    v_dataReader _this)
{
    assert(C_TYPECHECK(_this,v_dataReader));
    return c_keep (_this->sampleType);
}

c_char *
v_dataReaderKeyExpr(
    v_dataReader _this)
{
    return v_indexKeyExpr(_this->index);
}

c_field
v_dataReaderIndexField(
    v_dataReader _this,
    const c_char *name)
{
    c_field field;
    c_type  instanceType;
    c_char *fieldName;

    instanceType = v_dataReaderInstanceType(_this);
    /* Try lookup the specified name as an instance field. */
    field = c_fieldNew(instanceType,name);
    if (field == NULL) {
        fieldName = os_alloca(strlen(name) + strlen("sample.message.userData.") + 1);
        /* Try to lookup the specified name as a sample field. */
        os_sprintf(fieldName,"sample.%s",name);
        field = c_fieldNew(instanceType,fieldName);
        if (field == NULL) {
            /* Try to lookup the specified name as a message field. */
            os_sprintf(fieldName,"sample.message.%s",name);
            field = c_fieldNew(instanceType,fieldName);
            if (field == NULL) {
                /* Try to lookup the specified name as a userData field. */
                os_sprintf(fieldName,"sample.message.userData.%s",name);
                field = c_fieldNew(instanceType,fieldName);
            }
        }
        os_freea(fieldName);
    }
    c_free(instanceType);
    return field;
}

c_field
v_dataReaderField(
    v_dataReader _this,
    const c_char *name)
{
    c_field field;

    field = v_projectionSource(_this->projection,name);
    if (field == NULL) {
        field = v_dataReaderIndexField(_this,name);
    } else {
        /* The specified name identified a projection field. */
        /* Need to keep the field because v_projectionSource doesn't. */
        c_keep(field);
    }
    return field;
}

c_bool
v_dataReaderSubscribe(
    v_dataReader _this,
    v_partition d)
{
    assert(C_TYPECHECK(_this,v_dataReader));

    v_readerWalkEntries(v_reader(_this), subscribe, d);

    return TRUE;
}

static c_bool
dataReaderEntrySubscribeGroup(
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry entry = v_dataReaderEntry(o);
    v_group group = v_group(arg);

    assert(C_TYPECHECK(entry,v_entry));
    assert(C_TYPECHECK(group,v_group));

    if (group->topic == entry->topic) {
        v_groupAddEntry(group, v_entry(entry));
    }
    return TRUE;
}

c_bool
v_dataReaderSubscribeGroup(
    v_dataReader _this,
    v_group group)
{
    assert(C_TYPECHECK(_this, v_dataReader));

    return v_readerWalkEntries(v_reader(_this),
                               dataReaderEntrySubscribeGroup,
                               group);
}

c_bool
v_dataReaderUnSubscribe(
    v_dataReader _this,
    v_partition d)
{
    assert(C_TYPECHECK(_this,v_reader));

    return v_readerWalkEntries(v_reader(_this), unsubscribe, d);
}

static c_bool
dataReaderEntryUnSubscribeGroup(
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry entry = v_dataReaderEntry(o);
    v_group group = v_group(arg);

    v_groupRemoveEntry(group, v_entry(entry));

    return TRUE;
}

c_bool
v_dataReaderUnSubscribeGroup(
    v_dataReader _this,
    v_group group)
{
    assert(C_TYPECHECK(_this, v_reader));

    return v_readerWalkEntries(v_reader(_this),
                               dataReaderEntryUnSubscribeGroup,
                               group);
}


static c_bool
getTopic (
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry entry = v_dataReaderEntry(o);
    v_topic *topic = (v_topic *)arg;
    c_bool result = TRUE;

    if (*topic == NULL) {
        *topic = c_keep(entry->topic);
    } else {
        /* Already a topic was found so this must be a Multi Topic reader.
         * In that case abort and clear the topic.
         * Multi Topics are not yet supported by v_builtin.
         * Even Worse: The spec doesn't support builtin definition for
         * Multi Topics.
         */
        c_free(*topic);
        *topic = NULL;
        result = FALSE;
    }
    return result;
}

v_topic
v_dataReaderGetTopic(
    v_dataReader _this)
{
    v_topic topic;
    assert(C_TYPECHECK(_this,v_dataReader));

    topic = NULL;
    v_readerWalkEntries(v_reader(_this), getTopic, &topic);

    return c_keep(topic);
}

v_dataReaderInstance
v_dataReaderLookupInstance(
    v_dataReader _this,
    v_message keyTemplate)
{
    v_dataReaderInstance found;

    assert(C_TYPECHECK(_this,v_dataReader));
    assert(C_TYPECHECK(keyTemplate,v_message));

    v_dataReaderLock(_this);
    found = dataReaderLookupInstanceUnlocked(_this, keyTemplate);
    v_statisticsULongValueInc(v_reader, numberOfInstanceLookups, _this);
    v_dataReaderUnLock(_this);

    return found;
}

c_bool
v_dataReaderContainsInstance (
    v_dataReader _this,
    v_dataReaderInstance instance)
{
    v_dataReader instanceReader;
    c_bool result = FALSE;

    assert(C_TYPECHECK(_this,v_dataReader));
    assert(C_TYPECHECK(instance,v_dataReaderInstance));

    instanceReader = v_dataReaderInstanceReader(instance);
    if (instanceReader != NULL) {
        result = (instanceReader == _this);
    } else {
        OS_REPORT_2(OS_ERROR, "v_dataReaderContainsInstance", 0,
            "Invalid dataReaderInstance: no attached DataReader"
            "<_this = 0x%x instance = 0x%x>", _this, instance);
        result = FALSE;
    }
    return result;
}

void
v_dataReaderCheckDeadlineMissed(
    v_dataReader _this,
    c_time now)
{
    C_STRUCT(v_event) event;
    c_bool changed = FALSE;
    c_iter missed;
    v_dataReaderInstance instance;

    v_dataReaderLock(_this);
    missed = v_deadLineInstanceListCheckDeadlineMissed(_this->deadLineList,
                  v_reader(_this)->qos->deadline.period, now);
    instance = v_dataReaderInstance(c_iterTakeFirst(missed));
    while (instance != NULL) {
        if(instance->owner.exclusive){ /*Exclusive ownership*/
            v_gidSetNil(instance->owner.gid);
            instance->owner.strength = 0;
        }
        if (v_statusNotifyDeadlineMissed(v_entity(_this)->status,
                                         v_publicHandle(v_public(instance))))
        {
            changed = TRUE;
        }
        instance = v_dataReaderInstance(c_iterTakeFirst(missed));
    }
    c_iterFree(missed);

    if (changed) {
        event.kind = V_EVENT_DEADLINE_MISSED;
        event.source = v_publicHandle(v_public(_this));
        event.userData = NULL;
        v_observerNotify(v_observer(_this), &event, NULL);
        v_dataReaderUnLock(_this);
        v_observableNotify(v_observable(_this), &event);
    } else {
        v_dataReaderUnLock(_this);
    }
}

v_dataReaderInstance
v_dataReaderAllocInstance(
    v_dataReader _this)
{
    v_dataReaderInstance instance;
    v_kernel kernel;

    kernel = v_objectKernel(_this);

    instance = v_dataReaderInstance(c_new(_this->index->objectType));

    v_object(instance)->kernel = kernel;
    v_objectKind(instance) = K_DATAREADERINSTANCE;

    instance->index = (c_voidp)_this->index;

    return instance;
}

c_long
v_dataReaderNotReadCount(
    v_dataReader _this)
{
    c_long count;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    v_dataReaderLock(_this);
    count = _this->notReadCount;
    v_dataReaderUnLock(_this);

    return count;
}

v_result
v_dataReaderSetNotReadThreshold(
    v_dataReader _this,
    c_long threshold)
{
    assert(C_TYPECHECK(_this, v_reader));
    if(_this){
      v_dataReaderLock(_this);
        _this->notReadTriggerThreshold = threshold;
        /* FIXME: should trigger if threshold lowered, but can't be
           bothered for this PoC */
        v_dataReaderUnLock(_this);
    }
    return V_RESULT_OK;
}

c_bool
v_dataReaderUpdateSampleLost(
    v_reader _this,
    c_ulong missedSamples)
{
    v_dataReaderNotifySampleLost(v_dataReader(_this), missedSamples);
    v_statisticsULongValueAdd(v_reader, numberOfSamplesLost, _this, missedSamples);
    return TRUE;
}
