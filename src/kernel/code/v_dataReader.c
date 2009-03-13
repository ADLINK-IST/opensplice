#include "v__reader.h"
#include "v__dataReader.h"
#include "v_statistics.h"
#include "v_filter.h"
#include "v_readerStatistics.h"
#include "v__dataReader.h"
#include "v_dataReaderEntry.h"
#include "v_state.h"
#include "v_event.h"
#include "v__index.h"
#include "v__subscriber.h"
#include "v_subscriber.h"
#include "v_projection.h"
#include "v_entity.h"
#include "v_handle.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v__dataReaderInstance.h"
#include "v__status.h"
#include "v__query.h"
#include "v__observable.h"
#include "v__observer.h"
#include "v__builtin.h"
#include "v_participant.h"
#include "v_topic.h"
#include "v_domain.h"
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
#define _EXTENT_
#ifdef _EXTENT_
#include "c_extent.h"
#endif
#include "v__kernel.h"

#include "c_stringSupport.h"

#include "os.h"
#include "os_report.h"

static v_dataReaderInstance
dataReaderLookupInstanceUnlocked(
    v_dataReader _this,
    v_message keyTemplate)
{
    v_dataReaderInstance instance, found;

    assert(C_TYPECHECK(_this,v_dataReader));
    assert(C_TYPECHECK(keyTemplate,v_message));

    v_nodeState(keyTemplate) = L_REGISTER;

    instance = v_dataReaderInstanceNew(_this, keyTemplate);

    if (!v_reader(_this->index->reader)->qos->userKey.enable) {
        found = c_find(_this->index->objects, instance);
    } else {
        found = c_find(_this->index->notEmptyList, instance);
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

    return found;
}

static void
dataReaderEntrySubscribe(
    void *o,
    void *arg)
{
    v_domain d = v_domain(o);
    v_dataReaderEntry e = v_dataReaderEntry(arg);
    v_kernel kernel;
    v_group g;

    assert(C_TYPECHECK(e,v_dataReaderEntry));
    assert(C_TYPECHECK(d,v_domain));

    kernel = v_objectKernel(e);
    g = v_groupSetCreate(kernel->groupSet,d,e->topic);
    v_groupAddEntry(g,v_entry(e));
}

static c_bool
subscribe(
    c_object entry,
    c_voidp domain)
{
    v_domain d = v_domain(domain);
    v_dataReaderEntry e = v_dataReaderEntry(entry);

    dataReaderEntrySubscribe(d, e);

    return TRUE;
}

static void
dataReaderEntryUnSubscribe(
    void *o,
    void *arg)
{
    v_domain d = v_domain(o);
    v_dataReaderEntry e = v_dataReaderEntry(arg);
    v_kernel kernel;
    v_group g;
    c_value params[2];
    c_iter list;

    assert(C_TYPECHECK(e,v_dataReaderEntry));
    assert(C_TYPECHECK(d,v_domain));

    params[0] = c_objectValue(d);
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
    c_voidp domain)
{
    v_domain d = v_domain(domain);
    v_dataReaderEntry e = v_dataReaderEntry(entry);

    dataReaderEntryUnSubscribe(d, e);

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
    v_dataReaderEntry entry, found;
    v_filter filter;

    filter = v_filterNew(topic, a->_where, *a->params);
    entry = v_dataReaderEntryNew(a->dataReader, topic, filter);
    c_free(filter);
    found = v_dataReaderAddEntry(a->dataReader,entry);
    assert(entry == found);
    c_free(found);
    entry->index = c_keep(index);
    index->entry = entry;
    c_free(entry);
}

static c_bool
onSampleDumpedAction(
    v_readerSample sample,
    c_voidp arg)
{
    /* We should only be here if the reader has view(s) */
    v_dataReader r = v_dataReader(arg);

    if (r->views != NULL) {
        v_dataReaderSampleWipeViews(v_dataReaderSample(sample));
    }
    return TRUE;
}

static c_bool
dataReaderEntryUpdatePurgeLists(
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntryUpdatePurgeLists(v_dataReaderEntry(o));
    return TRUE; /* process all other entries */
}

c_long
v_dataReaderInstanceCount(
    v_dataReader _this)
{
    assert(C_TYPECHECK(_this,v_dataReader));

    return c_tableCount(_this->index->objects);
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


#define STAT_INIT_ULONG(member) \
        v_statisticsULongInit(v_reader, member, _this)

#define STAT_INIT_MAXVALUE(member) \
        v_statisticsMaxValueInit(v_reader, member, _this)

#define STAT_INIT_FULLCOUNTER(member) \
        v_statisticsFullCounterInit(v_reader, member, _this)

static void
v_dataReaderStatisticsInit(
    v_dataReader _this)
{
    STAT_INIT_MAXVALUE(maxSampleSize);
    STAT_INIT_MAXVALUE(maxSamplesPerInstance);
    STAT_INIT_MAXVALUE(maxNumberOfSamples);
    STAT_INIT_MAXVALUE(maxNumberOfInstances);

    STAT_INIT_ULONG(numberOfSamples);
    STAT_INIT_ULONG(numberOfInstances);

    STAT_INIT_FULLCOUNTER(readLatency);
    STAT_INIT_FULLCOUNTER(transportLatency);

    STAT_INIT_ULONG(numberOfInstancesWithStatusNew);
    STAT_INIT_ULONG(numberOfInstancesWithStatusAlive);
    STAT_INIT_ULONG(numberOfInstancesWithStatusDisposed);
    STAT_INIT_ULONG(numberOfInstancesWithStatusNoWriters);

    STAT_INIT_ULONG(numberOfSamplesWithStatusRead);
    STAT_INIT_ULONG(numberOfSamplesExpired);
    STAT_INIT_ULONG(numberOfSamplesPurgedByDispose);
    STAT_INIT_ULONG(numberOfSamplesPurgedByNoWriters);

    STAT_INIT_ULONG(numberOfSamplesArrived);
    STAT_INIT_ULONG(numberOfSamplesInserted);
    STAT_INIT_ULONG(numberOfSamplesDiscarded);
    STAT_INIT_ULONG(numberOfSamplesRead);
    STAT_INIT_ULONG(numberOfSamplesTaken);

    STAT_INIT_ULONG(numberOfReads);
    STAT_INIT_ULONG(numberOfTakes);
}
#undef STAT_INIT_ULONG
#undef STAT_INIT_MAXVALUE
#undef STAT_INIT_FULLCOUNTER

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
    v_filter filter;
    c_type instanceType;
    c_property sampleProperty;
    c_long i;

    assert(C_TYPECHECK(subscriber,v_subscriber));

    kernel = v_objectKernel(subscriber);

    if (!q_isFnc(OQLexpr,Q_EXPR_PROGRAM)) {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderNew",0,
                  "Not a valid view expression.");
        return NULL;
    }
    expr = q_getPar(OQLexpr,0);
    if (!q_isFnc(expr,Q_EXPR_SELECT)) {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderNew",0,
                  "Not a valid select statement.");
        return NULL;
    }
    _projection = NULL;
    _from = NULL;
    _where = NULL;
    filter = NULL;
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
        OS_REPORT(OS_ERROR,
                  "v_dataReaderNew",0,
                  "Missing from clause.");
        return NULL;
    }
    q = v_readerQosNew(kernel,qos);
    if (q == NULL) {
        OS_REPORT(OS_ERROR, "v_dataReaderNew", 0,
            "DataReader not created: inconsistent qos");
        return NULL;
    }
    _this = v_dataReader(v_objectNew(kernel,K_DATAREADER));
    _this->shareCount = 1;
    _this->views = NULL;
    _this->sampleCount = 0;
    _this->maxInstances = FALSE;
    _this->depth = 0x7fffffff; /* MAX_INT */
    _this->cachedSample = NULL;
#define _SL_
#ifdef _SL_
    _this->cachedSampleCount = 0;
#endif
    _this->readCnt = 0;

#ifdef _MSG_STAMP_
    v_dataReaderLogInit(_this);
#endif

    if (v_isEnabledStatistics(kernel, V_STATCAT_READER)) {
        readerStatistics = v_readerStatisticsNew(kernel);
    } else {
        readerStatistics = NULL;
    }
    v_readerInit(v_reader(_this),name,
                 subscriber,q,
                 v_statistics(readerStatistics),
                 enable);

    if (q->share.enable) {
        v_subscriberLockShares(subscriber);
        found = v_dataReader(v_subscriberAddShareUnsafe(subscriber,v_reader(_this)));
        if (found != _this) {
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

#ifdef _EXTENT_
#define _COUNT_ (128)
        /* the sampleExtent is created with the synchronized parameter
         * set to TRUE.
         * So the extent will use a mutex to guarantee reentrancy.
         * This is needed because samples are kept, copied and freed
         * outside the reader lock.
         */
        _this->sampleExtent = c_extentSyncNew(sampleProperty->type,_COUNT_,TRUE);
#endif
        _this->projection = v_projectionNew(_this,_projection);


        if (q->share.enable) {
            v_subscriberUnlockShares(subscriber);
        }

        participant = v_participant(subscriber->participant);
        assert(participant != NULL);
        _this->deadLineList =
            v_deadLineInstanceListNew(c_getBase(c_object(_this)),
                                      participant->leaseManager,
                                      q->deadline.period,
                                      V_LEASEACTION_READER_DEADLINE_MISSED,
                                      v_public(_this));

        if (enable) {
            v_dataReaderEnable(_this);
        }
    } else {
        v_readerDeinit(v_reader(_this));
        c_free(_this);
        _this = NULL;
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

        v_subscriberAddReader(subscriber,v_reader(_this));

        kernel = v_objectKernel(_this);
        builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin, _this);
        v_writeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
        c_free(builtinMsg);
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
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
    assert(sc == 0);

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

        }
    }
    v_readerFree(v_reader(_this));
    v_dataReaderLock(_this);
    if (_this->views != NULL) {
        views = c_select(_this->views, 0);
        view = v_dataView(c_iterTakeFirst(views));
        while (view != NULL) {
            v_dataViewFreeUnsafe(view);
            view = v_dataView(c_iterTakeFirst(views));
        }
        c_iterFree(views);
    }
    if (_this->triggerValue) {
        c_free(v_readerSample(_this->triggerValue)->instance);
    }
    v_dataReaderUnLock(_this);

    v_writeDisposeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
    v_unregisterBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, unregisterMsg);
    c_free(builtinMsg);
    c_free(unregisterMsg);
}

static c_bool
instanceFree(
    c_object o,
    c_voidp arg)
{
    v_publicFree(o);
    return TRUE;
}

void
v_dataReaderDeinit (
    v_dataReader _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));
    v_readerDeinit(v_reader(_this));
    c_tableWalk(_this->index->objects,instanceFree,NULL);
    c_tableWalk(_this->index->notEmptyList, instanceFree, NULL);
    v_deadLineInstanceListFree(_this->deadLineList);
}

/* Helpfunc for writing into the dataViews */

static c_bool
writeSlave(
    v_readerSample sample,
    c_voidp arg)
{
    return v_dataViewWrite(v_dataView(arg),sample);
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
    c_tableWalk(_this->index->notEmptyList, walkInstanceSamples, view);

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
        if (v_statisticsValid(a->reader)) {
            *(v_statisticsGetRef(v_reader,
                                 numberOfSamplesRead,
                                 a->reader)) += count;
        }
        assert(v_dataReader(a->reader)->sampleCount >= 0);
		/*A read behaves like a take for invalid samples*/
#ifndef _DELAYED_NOT_EMPTYLIST_INSERT_
        if (v_dataReaderInstanceEmpty(instance)) {
            a->emptyList = c_iterInsert(a->emptyList,instance);
        }
#endif
    } else {
#ifndef _DELAYED_NOT_EMPTYLIST_INSERT_
        assert(FALSE);
#else
        if (v_dataReaderInstanceInNotEmptyList(instance)) {
            a->emptyList = c_iterInsert(a->emptyList,instance);
        }
#endif
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

    v_dataReaderLock(_this);
    _this->readCnt++;
    v_readerWalkEntries(v_reader(_this),
                        dataReaderEntryUpdatePurgeLists,
                        NULL);
    argument.reader = _this;
    argument.action = action;
    argument.arg = arg;
    argument.query = NULL;
    argument.emptyList = NULL;

    proceed = c_readAction(_this->index->notEmptyList,
                           (c_action)instanceReadSamples,
                           &argument);

    /* The state of an instance can also change because of a read action
     * in case of an invalid sample.
     */
    if (argument.emptyList != NULL) {
        emptyInstance = c_iterTakeFirst(argument.emptyList);
        while (emptyInstance != NULL) {
            v_dataReaderRemoveInstance(_this,emptyInstance);
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

    v_dataReaderLock(_this);
    _this->readCnt++;
    if (!v_dataReaderInstanceEmpty(instance)) {
        v_readerWalkEntries(v_reader(_this),
                            dataReaderEntryUpdatePurgeLists,
                            NULL);
        proceed = v_dataReaderInstanceReadSamples(instance,NULL,action,arg);

        v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);
    } else {
        proceed = TRUE;
    }
    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfReads, _this);
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
    v_dataReaderInstance next;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));
    assert(C_TYPECHECK(instance, v_dataReaderInstance));

    v_dataReaderLock(_this);
    _this->readCnt++;
    v_readerWalkEntries(v_reader(_this), dataReaderEntryUpdatePurgeLists, NULL);
    next = v_dataReaderNextInstance(_this,instance);
    while ((next != NULL) &&
           v_dataReaderInstanceEmpty(next)) {
        next = v_dataReaderNextInstance(_this,next);
    }
    if (next != NULL) {
        proceed = v_dataReaderInstanceReadSamples(next, NULL,action,arg);
    }


    v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);

    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfReads, _this);
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
#ifdef _DELAYED_NOT_EMPTYLIST_INSERT_
        if (v_dataReaderInstanceInNotEmptyList(instance)) {
            a->emptyList = c_iterInsert(a->emptyList,instance);
        }
#endif
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
    if (v_statisticsValid(a->reader)) {
        *(v_statisticsGetRef(v_reader,
                             numberOfSamplesTaken,
                             a->reader)) += count;
    }
    assert(v_dataReader(a->reader)->sampleCount >= 0);

#ifndef _DELAYED_NOT_EMPTYLIST_INSERT_
    if (v_dataReaderInstanceEmpty(instance)) {
        a->emptyList = c_iterInsert(a->emptyList,instance);
    }
#endif
    return proceed;
}

void
v_dataReaderRemoveInstance(
    v_dataReader _this,
    v_dataReaderInstance instance)
{
    v_dataReaderInstance found;

    assert(C_TYPECHECK(_this, v_dataReader));
    assert(v_dataReaderInstanceEmpty(instance));

#ifdef _DELAYED_NOT_EMPTYLIST_INSERT_
    if (v_dataReaderInstanceInNotEmptyList(instance)) {
        found = v_dataReaderInstance(c_remove(_this->index->notEmptyList,
                                              instance, NULL, NULL));
        v_dataReaderInstanceInNotEmptyList(instance) = FALSE;
        c_free(found);
    }
#else
    found = v_dataReaderInstance(c_remove(_this->index->notEmptyList,
                                          instance, NULL, NULL));
    c_free(found);
#endif

    if (!v_reader(_this)->qos->userKey.enable) {
        if (v_dataReaderInstanceNoWriters(instance)) {
            found = v_dataReaderInstance(c_remove(_this->index->objects,
                                                  instance, NULL, NULL));
            v_deadLineInstanceListRemoveInstance(_this->deadLineList,
                                                 v_instance(instance));
            v_statisticsULongValueDec(v_reader,
                                      numberOfInstancesWithStatusNoWriters,
                                      _this);
            instance->purgeInsertionTime = C_TIME_ZERO;
            v_publicFree(v_public(instance));
            c_free(found);
        }
    } else {
        found = v_dataReaderInstance(c_remove(_this->index->objects,
                                              instance, NULL, NULL));
        v_publicFree(v_public(instance));
        c_free(found);
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

    v_dataReaderLock(_this);
    _this->readCnt++;
    v_readerWalkEntries(v_reader(_this),
                        dataReaderEntryUpdatePurgeLists,
                        NULL);

    argument.action = action;
    argument.arg = arg;
    argument.query = NULL;
    argument.reader = _this;
    argument.emptyList = NULL;

    proceed = c_readAction(_this->index->notEmptyList,
                           (c_action)instanceTakeSamples,
                           &argument);
    if (argument.emptyList != NULL) {
        emptyInstance = c_iterTakeFirst(argument.emptyList);
        while (emptyInstance != NULL) {
            v_dataReaderRemoveInstance(_this,emptyInstance);
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

    if (instance == NULL) {
        return FALSE;
    }

    assert(C_TYPECHECK(instance, v_dataReaderInstance));

    v_dataReaderLock(_this);
    _this->readCnt++;

    if (!v_dataReaderInstanceEmpty(instance)) {
        v_readerWalkEntries(v_reader(_this),
                            dataReaderEntryUpdatePurgeLists,
                            NULL);
        count = v_dataReaderInstanceSampleCount(instance);
        proceed = v_dataReaderInstanceTakeSamples(instance,NULL,action,arg);
        count -= v_dataReaderInstanceSampleCount(instance);
        assert(count >= 0);
        if (count > 0) {
            _this->sampleCount -= count;
            if (v_statisticsValid(_this)) {
                *(v_statisticsGetRef(v_reader,
                                     numberOfSamplesTaken,
                                     _this)) += count;
            }
            assert(_this->sampleCount >= 0);
#ifndef _DELAYED_NOT_EMPTYLIST_INSERT_
            if (v_dataReaderInstanceEmpty(instance)) {
                v_dataReaderRemoveInstance(_this,instance);
            }
#endif
            v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);
        }
#ifdef _DELAYED_NOT_EMPTYLIST_INSERT_
    } else {
        v_dataReaderRemoveInstance(_this,instance);
#endif
    }
    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfTakes, _this);
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
    v_dataReaderInstance next;
    c_bool proceed = TRUE;
    c_long count;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));
    assert(C_TYPECHECK(instance, v_dataReaderInstance));

    v_dataReaderLock(_this);
    _this->readCnt++;

    v_readerWalkEntries(v_reader(_this),
                        dataReaderEntryUpdatePurgeLists,
                        NULL);
    next = v_dataReaderNextInstance(_this,instance);
    while ((next != NULL) &&
            v_dataReaderInstanceEmpty(next)) {
        next = v_dataReaderNextInstance(_this,next);
    }
    if (next != NULL) {
        count = v_dataReaderInstanceSampleCount(next);
        proceed = v_dataReaderInstanceTakeSamples(next,
                                                  NULL,
                                                  action,
                                                  arg);
        count -= v_dataReaderInstanceSampleCount(next);
        assert(count >= 0);
        if (count > 0) {
            _this->sampleCount -= count;
            if (v_statisticsValid(_this)) {
                *(v_statisticsGetRef(v_reader,
                                     numberOfSamplesTaken,
                                     _this)) += count;
            }
            assert(_this->sampleCount >= 0);
            if (v_dataReaderInstanceEmpty(next)) {
                v_dataReaderRemoveInstance(_this,next);
            }
        }
    }
    v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);

    /* Now trigger the action routine that the last sample is read. */
    action(NULL,arg);
    v_statisticsULongValueInc(v_reader, numberOfTakes, _this);
    v_dataReaderUnLock(_this);

    return proceed;
}

void
v_dataReaderNotify(
    v_dataReader _this,
    v_event event,
    c_voidp userData)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_dataReader));

    if (event != NULL) {
        switch(event->kind) {
        case V_EVENT_INCONSISTENT_TOPIC:
        case V_EVENT_SAMPLE_REJECTED:
        case V_EVENT_SAMPLE_LOST:
        case V_EVENT_DEADLINE_MISSED:
        case V_EVENT_INCOMPATIBLE_QOS:
        case V_EVENT_LIVELINESS_CHANGED:
        case V_EVENT_DATA_AVAILABLE:
            v_entity(_this)->status->state |= event->kind;
        break;
        default:
            OS_REPORT_1(OS_WARNING,
                        "DataReader",0,
                        "Notify encountered unknown event kind (%d)",
                        event->kind);
        break;
        }
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
        c_free(v_readerSample(_this->triggerValue)->instance);
        c_free(_this->triggerValue);
    }
    if (sample) {
        c_keep(v_readerSample(sample)->instance);
    }
    _this->triggerValue = c_keep(sample);

    event.kind     = V_EVENT_DATA_AVAILABLE;
    event.source   = V_HANDLE_NIL;
    event.userData = sample;

    c_setWalk(v_collection(_this)->queries, queryNotifyDataAvailable, &event);

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
v_dataReaderNotifyIncompatibleQos(
    v_dataReader _this,
    v_policyId   id,
    v_gid        writerGID)
{
    c_bool changed;
    C_STRUCT(v_event) event;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    changed = v_statusNotifyIncompatibleQos(v_entity(_this)->status, id);

    if (changed) {
        /* Also notify myself, since the user reader might be waiting. */
        event.kind = V_EVENT_INCOMPATIBLE_QOS;
        event.source = v_publicHandle(v_public(_this));
        event.userData = NULL;
        v_observerNotify(v_observer(_this), &event, NULL);
        v_observableNotify(v_observable(_this), &event);
    }
}

static c_bool
updateConnections(
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry e = v_dataReaderEntry(o);
    v_dataReaderNotifyChangedQosArg *a = (v_dataReaderNotifyChangedQosArg *)arg;

    c_iterWalk(a->addedDomains, dataReaderEntrySubscribe, e);
    c_iterWalk(a->removedDomains, dataReaderEntryUnSubscribe, e);

    return TRUE;
}

/* only called by v_subscriberSetQos() and v_readerSetQos(). */
void
v_dataReaderNotifyChangedQos(
    v_dataReader _this,
    v_dataReaderNotifyChangedQosArg *arg)
{
    v_kernel kernel;
    v_message builtinMsg;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));

    /* publish subscription info */

    if ((arg != NULL) &&
        ((arg->addedDomains != NULL) || (arg->removedDomains != NULL))) {
      /* partition policy has changed */
/**
 * Now the builtin topic is published, after all connections are updated.
 * Depending on the outcome of the RTPS protocol standardisation, this
 * solution is subject to change.
 */
        v_readerWalkEntries(v_reader(_this), updateConnections, arg);
    }
    v_dataReaderLock(_this);
    kernel = v_objectKernel(_this);
    builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin, _this);
    v_deadLineInstanceListSetDuration(_this->deadLineList,
                                      v_reader(_this)->qos->deadline.period);
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
    v_participant participant;
    c_iter readers;
    v_dataReader publicationReader;
    v_dataReaderInstance instance;
    v_gid instanceHandle;
    v_topic topic;
    v_message message;
    v_subscriber subscriber;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_dataReader));
    assert(oldLivState != V_STATUSLIVELINESS_COUNT);
    assert(newLivState != V_STATUSLIVELINESS_COUNT);

    changed = FALSE;
    if (oldLivState != newLivState) {
        v_dataReaderLock(_this);
        subscriber = v_subscriber(v_reader(_this)->subscriber);

        if(subscriber){ /* subscriber may have become null when reader is freed */
            participant = v_participant(subscriber->participant);

            if(participant->builtinSubscriber && publicationInfo){
                readers = v_subscriberLookupReadersByTopic(
                            participant->builtinSubscriber,
                            V_PUBLICATIONINFO_NAME);

                assert(c_iterLength(readers) <= 1);
                publicationReader = c_iterTakeFirst(readers);

                if(publicationReader){
                    topic = v_dataReaderGetTopic(publicationReader);
                    message = v_topicMessageNew(topic);
                    v_topicMessageCopyKeyValues(topic, message, publicationInfo);

                    /* Splicedaemon receives its own notifications, so to prevent
                     * deadlock use unlocked version here.
                     */
                    if(_this == publicationReader){
                        instance = dataReaderLookupInstanceUnlocked(
                                publicationReader, message);
                    } else {
                        instance = v_dataReaderLookupInstance(
                            publicationReader, message);
                    }

                    instanceHandle = v_publicGid(v_public(instance));

                    c_free(topic);
                    c_free(message);
                    c_free(instance);
                    c_free(publicationReader);
                } else {
                    instanceHandle = v_publicGid(NULL);
                }
                c_iterFree(readers);

            } else {
                instanceHandle = v_publicGid(NULL);
            }
        } else {
            instanceHandle = v_publicGid(NULL);
        }
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
         *    And the writer must be unregisted with this reader.
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
                        v_entity(_this)->status, -1, 0, instanceHandle);
            break;
            case V_STATUSLIVELINESS_NOTALIVE:
                changed = v_statusNotifyLivelinessChanged(
                        v_entity(_this)->status, 0, -1, instanceHandle);
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
                            v_entity(_this)->status, 1, 0, instanceHandle);
                break;
                case V_STATUSLIVELINESS_NOTALIVE:
                    changed = v_statusNotifyLivelinessChanged(
                            v_entity(_this)->status, 0, 1, instanceHandle);
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
                                v_entity(_this)->status, 1, -1, instanceHandle);
                    } else {
                        assert(newLivState == V_STATUSLIVELINESS_NOTALIVE);
                        changed = v_statusNotifyLivelinessChanged(
                                v_entity(_this)->status, -1, 1, instanceHandle);
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

c_type
v_dataReaderInstanceType(
    v_dataReader _this)
{
    assert(C_TYPECHECK(_this,v_dataReader));
    return c_subType(_this->index->objects);
}

c_type
v_dataReaderSampleType(
    v_dataReader _this)
{
    assert(C_TYPECHECK(_this,v_dataReader));
    return c_extentType(_this->sampleExtent);
}

c_char *
v_dataReaderKeyExpr(
    v_dataReader _this)
{
    return v_indexKeyExpr(_this->index);
}

static void
getPathNameSize (
    c_voidp name,
    c_voidp length)
{
    (*(c_long *)length) = (*(c_long *)length + strlen((c_char *)name) + 1);
}

static void
getPathName (
    c_voidp name,
    c_voidp str)
{
    strcat((c_char *)str,(c_char *)name);
    strcat((c_char *)str,".");
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
        fieldName = os_alloca(strlen(name) + strlen("sample.message.userData.."));
        /* Try to lookup the specified name as a sample field. */
        sprintf(fieldName,"sample.%s",name);
        field = c_fieldNew(instanceType,fieldName);
        if (field == NULL) {
            /* Try to lookup the specified name as a message field. */
            sprintf(fieldName,"sample.message.%s",name);
            field = c_fieldNew(instanceType,fieldName);
            if (field == NULL) {
                /* Try to lookup the specified name as a userData field. */
                sprintf(fieldName,"sample.message.userData.%s",name);
                field = c_fieldNew(instanceType,fieldName);
            }
        }
        os_freea(fieldName);
    }
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
    v_domain d)
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
    v_domain d)
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
         * Even Worse: The spec doesn support builtin definition for
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
    v_dataReaderUnLock(_this);

    return found;
}

c_bool
v_dataReaderContainsInstance (
    v_dataReader _this,
    v_dataReaderInstance instance)
{
    c_bool found = FALSE;

    assert(C_TYPECHECK(_this,v_dataReader));
    assert(C_TYPECHECK(instance,v_dataReaderInstance));

    if (v_dataReader(v_index(instance->index)->reader) == _this) {
        found = TRUE;
    }

    return found;
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
        if (v_statusNotifyDeadlineMissed(v_entity(_this)->status, instance)) {
            changed = TRUE;
        }
        instance = v_dataReaderInstance(c_iterTakeFirst(missed));
    }
    c_iterFree(missed);
    v_dataReaderUnLock(_this);

    if (changed) {
        event.kind = V_EVENT_DEADLINE_MISSED;
        event.source = v_publicHandle(v_public(_this));
        event.userData = NULL;
        v_observerNotify(v_observer(_this), &event, NULL);
        v_observableNotify(v_observable(_this), &event);
    }
}

v_dataReaderInstance
v_dataReaderAllocInstance(
    v_dataReader _this)
{
    v_dataReaderInstance instance;

#ifdef _EXTENT_
    instance = v_dataReaderInstance(c_extentCreate(_this->index->objectExtent));
#else
    instance = v_dataReaderInstance(c_new(_this->index->description->type));
#endif

    v_object(instance)->kernel = v_objectKernel(_this);
    v_objectKind(instance) = K_DATAREADERINSTANCE;

    v_instanceInit(v_instance(instance));

    instance->index = (c_voidp)_this->index;

    return instance;
}

void
v_dataReaderDeallocInstance(
    v_dataReaderInstance _this)
{
    c_free(_this);
}



