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

#include "v_query.h"
#include "v__dataReader.h"
#include "v__dataView.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"
#include "v_entity.h"
#include "v_event.h"
#include "v__observable.h"
#include "v__observer.h"
#include "v_public.h"
#include "v__collection.h"

#include "q_helper.h"
#include "os_report.h"
#include "os_heap.h"



v_query
v_queryNew (
    v_collection source,
    const c_char *name,
    q_expr predicate,
    c_value params[])
{
    v_query q;
    v_dataReader reader;
    v_dataView readerView;

    assert(C_TYPECHECK(source,v_collection));

    q = NULL;

    switch(v_objectKind(source)) {
    case K_DATAREADER:
        reader = v_dataReader(source);
        q = v_query(v_dataReaderQueryNew(
                        reader,
                        name,
                        predicate,
                        params));
    break;
    case K_DATAVIEW:
        readerView = v_dataView(source);
        q = v_query(v_dataViewQueryNew(
                        readerView,
                        name,
                        predicate,
                        params));
    break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_queryNew failed",0,
                    "illegal source kind (%d) specified",
                    v_objectKind(source));
    }
    return q;
}

void
v_queryInit(
    v_query q,
    const c_char *name,
    v_statistics qs,
    v_collection src,
    q_expr predicate,
    c_value params[])
{
    OS_UNUSED_ARG(predicate);
    OS_UNUSED_ARG(params);
    assert(C_TYPECHECK(q,v_query));

    v_collectionInit(v_collection(q), name, qs, TRUE);

    q->source = src;
}

void
v_queryFree(
    v_query q)
{
    if (q != NULL) {
        assert(C_TYPECHECK(q,v_query));

        v_collectionFree(v_collection(q));
    }
}

void
v_queryDeinit(
    v_query q)
{
    if (q != NULL) {
        assert(C_TYPECHECK(q,v_query));
        v_collectionDeinit(v_collection(q));
    }
}

v_collection
v_querySource(
    v_query q)
{
    v_collection c;

    if (q == NULL) {
        return NULL;
    }

    assert(C_TYPECHECK(q,v_query));

    c = v_collection(q->source);
    if (c == NULL) {
        OS_REPORT_1(OS_ERROR,
                    "v_querySource failed",0,
                    "Query (0x%x) without source detected",
                    q);
        assert(FALSE);
        return NULL;
    }

    switch(v_objectKind(c)) {
    case K_DATAREADER:
    case K_DATAVIEW:
        c_keep(c);
    break;
    case K_DATAREADERQUERY:
    case K_DATAVIEWQUERY:
        c = v_querySource(v_query(c));
    break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_querySource failed",0,
                    "illegal source kind (%d) detected",
                    v_objectKind(c));
        assert(FALSE);
        return NULL;
    }
    return c;
}

c_bool
v_queryTest(
    v_query q,
    v_queryAction action,
    c_voidp args)
{
    c_bool result = FALSE;

    if (q == NULL) {
        return FALSE;
    }

    assert(C_TYPECHECK(q,v_query));

    switch (v_objectKind(q)) {
    case K_DATAREADERQUERY:
        result = v_dataReaderQueryTest(v_dataReaderQuery(q), action, args);
    break;
    case K_DATAVIEWQUERY:
        result = v_dataViewQueryTest(v_dataViewQuery(q), action, args);
    break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_queryTest failed",0,
                    "illegal query kind (%d) specified",
                    v_objectKind(q));
        assert(FALSE);
    }

    return result;
}

static c_bool
v_queryReadInternal(
    v_query q,
    v_dataReaderInstance instance,
    c_bool readNext,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;
    v_dataReaderQuery drq;
    v_dataViewQuery dvq;

    if (q == NULL) {
        return FALSE;
    }

    assert(C_TYPECHECK(q,v_query));

    switch (v_objectKind(q)) {
    case K_DATAREADERQUERY:
        drq = v_dataReaderQuery(q);
        if (readNext) {
            proceed = v_dataReaderQueryReadNextInstance(
                        drq,
                        instance,
                        (v_readerSampleAction)action,
                        arg);
        } else {
            if (instance) {
                proceed = v_dataReaderQueryReadInstance(
                              drq,
                              instance,
                              (v_readerSampleAction)action,
                              arg);
            } else {
                proceed = v_dataReaderQueryRead(
                              drq,
                              (v_readerSampleAction)action,
                              arg);
            }
        }
    break;
    case K_DATAVIEWQUERY:
        dvq = v_dataViewQuery(q);
        if (readNext) {
            proceed = v_dataViewQueryReadNextInstance(dvq,
                          (v_dataViewInstance)instance,
                          (v_readerSampleAction)action, arg);
        } else {
            if (instance) {
                proceed = v_dataViewQueryReadInstance(dvq,
                              (v_dataViewInstance)instance,
                              (v_readerSampleAction)action, arg);
            } else {
                proceed = v_dataViewQueryRead(dvq,
                              (v_readerSampleAction)action, arg);
            }
        }
    break;
    default:
        OS_REPORT_1(OS_ERROR,"v_queryRead failed",0,
                    "illegal query kind (%d) specified",v_objectKind(q));
        proceed = FALSE;
        assert(FALSE);
    }

    return proceed;
}

c_bool
v_queryRead(
    v_query q,
    v_readerSampleAction action,
    c_voidp arg)
{
    return v_queryReadInternal(q, NULL, FALSE, action, arg);
}

static c_bool
v_queryTakeInternal(
    v_query q,
    v_dataReaderInstance instance,
    c_bool readNext,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed;
    v_dataReaderQuery drq;
    v_dataViewQuery dvq;

    if (q == NULL) {
        return FALSE;
    }

    assert(C_TYPECHECK(q,v_query));

    switch (v_objectKind(q)) {
    case K_DATAREADERQUERY:
        drq = v_dataReaderQuery(q);
        if (readNext) {
            proceed = v_dataReaderQueryTakeNextInstance(drq, instance,
                          (v_readerSampleAction)action,arg);
        } else {
            if (instance) {
                proceed = v_dataReaderQueryTakeInstance(drq,
                              instance,
                              (v_readerSampleAction)action, arg);
            } else {
                proceed = v_dataReaderQueryTake(drq,
                              (v_readerSampleAction)action, arg);
            }
        }
    break;
    case K_DATAVIEWQUERY:
        dvq = v_dataViewQuery(q);
        if (readNext) {
	  proceed = v_dataViewQueryTakeNextInstance(dvq,
                        (v_dataViewInstance)instance,
                        (v_readerSampleAction)action, arg);
        } else {
            if (instance) {
                proceed = v_dataViewQueryTakeInstance(dvq,
                              (v_dataViewInstance)instance,
                              (v_readerSampleAction)action,
                              arg);
            } else {
                proceed = v_dataViewQueryTake(dvq,
                              (v_readerSampleAction)action, arg);
            }
        }
    break;
    default:
        OS_REPORT_1(OS_ERROR,"v_queryTake failed",0,
                    "illegal query kind (%d) specified",v_objectKind(q));
        proceed = FALSE;
        assert(FALSE);
    }

    return proceed;
}

c_bool
v_queryTake(
    v_query q,
    v_readerSampleAction action,
    c_voidp arg)
{
    return v_queryTakeInternal(q, NULL, FALSE, action, arg);
}

void
v_queryNotify(
    v_query q,
    v_event event,
    c_voidp userData)
{
    OS_UNUSED_ARG(userData);
    if (q && event) {
        if (event->kind != V_EVENT_DATA_AVAILABLE) {
            OS_REPORT_1(OS_WARNING, "v_query", 0,
                        "Unexpected event %d", event->kind);
        }
    }
}

c_bool
v_queryNotifyDataAvailable(
    v_query _this,
    v_event event)
{
    c_bool result = TRUE;

    switch (v_objectKind(_this)) {
    case K_DATAREADERQUERY:
        result = v_dataReaderQueryNotifyDataAvailable(
                     v_dataReaderQuery(_this),
                     event);
    break;
    case K_DATAVIEWQUERY:
        result = v_dataViewQueryNotifyDataAvailable(
                     v_dataViewQuery(_this),
                     event);
    break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_queryNotifyDataAvailable failed",0,
                    "illegal query kind (%d) specified",
                    v_objectKind(_this));
        result = TRUE;
        assert(FALSE);
    }
    return result;
}

c_bool
v_queryReadInstance(
    v_query q,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool result = FALSE;

    if (instance != NULL) {
        result = v_queryReadInternal(q, instance, FALSE, action, arg);
    }
    return result;
}

c_bool
v_queryReadNextInstance(
    v_query q,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool result = FALSE;

    result = v_queryReadInternal(q, instance, TRUE, action, arg);

    return result;
}

c_bool
v_queryTakeInstance(
    v_query q,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool result = FALSE;

    if (instance != NULL) {
        result = v_queryTakeInternal(q, instance, FALSE, action, arg);
    }

    return result;
}

c_bool
v_queryTakeNextInstance(
    v_query q,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool result = FALSE;

    result = v_queryTakeInternal(q, instance, TRUE, action, arg);

    return result;
}

c_bool
v_querySetParams(
    v_query q,
    q_expr predicate,
    c_value params[])
{
    c_bool result = FALSE;

    assert(C_TYPECHECK(q,v_query));

    if (q != NULL) {
        switch (v_objectKind(q)) {
        case K_DATAREADERQUERY:
            result = v_dataReaderQuerySetParams(v_dataReaderQuery(q),
                                                predicate,
                                                params);
        break;
        case K_DATAVIEWQUERY:
            result = v_dataViewQuerySetParams(v_dataViewQuery(q),
                                              predicate,
                                              params);
        break;
        default:
            OS_REPORT_1(OS_ERROR,
                        "v_querySetParams failed",0,
                        "illegal query kind (%d) specified",
                        v_objectKind(q));
            assert(FALSE);
        }
    }

    return result;
}
