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
#include "v_dataViewQuery.h"
#include "v_dataView.h"
#include "v_state.h"
#include "v_event.h"
#include "v_index.h"
#include "v_projection.h"
#include "v_entity.h"
#include "v_handle.h"
#include "v_dataViewInstance.h"
#include "v__query.h"
#include "v__observable.h"
#include "v__observer.h"
#include "v_public.h"
#include "v__collection.h"
#include "v__deadLineInstanceList.h"
#include "v__dataView.h"
#include "q_helper.h"
#include "v__statCat.h"
#include "v__kernel.h"
#include "v_listener.h"
#include "v_queryStatistics.h"
#include "v_kernelParser.h"

#include "c_stringSupport.h"

#include "vortex_os.h"
#include "os_report.h"

#define V_STATE_INITIAL        (0x00000000U)       /* 0 */
#define V_STATE_ACTIVE         (0x00000001U)       /* 1 */
#define V_STATE_DATA_AVAILABLE (0x00000001U << 1)  /* 2 */

static q_expr
resolveField(
    c_type type,
    const c_char *name)
{
    c_field field;
    c_array path;
    c_ulong i, length;
    q_list list;

    c_char *metaName;

    field = c_fieldNew(type,name);

    if (field == NULL) {
        return NULL;
    }
    path = c_fieldPath(field);
    length = c_arraySize(path);
    list = NULL;
    i = length;
    while (i-- > 0) {
        metaName = c_metaName(path[i]);
        list = q_insert(list,q_newId(metaName));
        c_free(metaName);
    }
    c_free(field);

    return q_newFnc(Q_EXPR_PROPERTY,list);
}

static c_bool
resolveFields (
    c_type type,
    q_expr e)
{
    /* search fields in result, data or info type. */

    q_expr p;
    c_long i;
    c_char *name;

    switch(q_getKind(e)) {
    case T_FNC:
        switch(q_getTag(e)) {
        case Q_EXPR_PROPERTY:
            name = q_propertyName(e);
            p = resolveField(type,name);
            if (p == NULL) {
                OS_REPORT(OS_ERROR,
                            "v_dataViewQueryNew failed",V_RESULT_ILL_PARAM,
                            "field %s undefined",
                            name);
                os_free(name);
                return FALSE;
            }
            os_free(name);
            q_swapExpr(e,p);
            q_dispose(p);
        break;
        default: /* process sub-expression */
            i=0;
            while ((p = q_getPar(e,i)) != NULL) {
                if (!resolveFields(type,p)) {
                    return FALSE;
                }
                i++;
            }
        }
    break;
    case T_ID:
        name = q_getId(e);
        p = resolveField(type,name);
        if (p == NULL) {
            OS_REPORT(OS_ERROR,
                        "v_dataViewQueryNew failed",V_RESULT_ILL_PARAM,
                        "field %s undefined",
                        name);
            return FALSE;
        } else {
            q_swapExpr(e,p);
            q_dispose(p);
        }
    break;
    default:
    break;
    }
    return TRUE;
}

#define PRINT_QUERY (0)

v_dataViewQuery
v_dataViewQueryNew (
    v_dataView view,
    const os_char *name,
    const os_char *expression,
    const os_char *params[],
    const os_uint32 nrOfParams,
    const os_uint32 sampleMask)
{
    v_kernel kernel;
    v_dataViewQuery query,found;
    v_result result;
    c_ulong i,len;
    q_expr e,subExpr,keyExpr,progExpr;
    q_expr predicate;
    c_iter list;
    c_type type, subtype;
    c_array keyList;
    c_value *values;

    assert(C_TYPECHECK(view,v_dataView));

    kernel = v_objectKernel(view);

    query = NULL;
    if (!expression) {
        OS_REPORT(OS_ERROR,
                      "DataReaderView SQL Parser",V_RESULT_ILL_PARAM,
                      "Parse Query expression failed. Query is <NULL>");
    } else {
        predicate = v_parser_parse(expression);
        if (!predicate) {
            OS_REPORT(OS_ERROR,
                        "DataReaderView SQL Parser",V_RESULT_ILL_PARAM,
                        "Parse Query expression failed. Query: \"%s\"",
                        expression);
        } else {
            query = v_dataViewQuery(v_objectNew(kernel,K_DATAVIEWQUERY));
            result = v_queryInit(v_query(query), v_collection(view), name, expression);
            if (result != V_RESULT_OK) {
                c_free(query);
                q_dispose(predicate);
                query = NULL;
            }
        }
    }
    if (query == NULL) {
        return NULL;
    }
    if (q_getLastVar(predicate) > nrOfParams)
    {
        c_free(query);
        q_dispose(predicate);
        return NULL;
    }

    v_queryEnableStatistics(v_query(query), v_isEnabledStatistics(kernel, V_STATCAT_READER));
    query->triggerValue = NULL;
    query->walkRequired = TRUE;
    query->sampleMask = sampleMask;

    q_prefixFieldNames(&predicate,"sample.sample.message.userData");

    e = q_takePar(predicate,0);

    if (nrOfParams > 0) {
        values = (c_value *)os_malloc(nrOfParams * sizeof(c_value));
        for (i=0; i<nrOfParams; i++) {
            values[i] = c_stringValue((const c_string)params[i]);
        }
    } else {
        values = NULL;
    }

    v_dataViewLock(view);
    subtype = c_subType(view->instances);
    if (!resolveFields(subtype,e)) {
        v_dataViewUnlock(view);
        q_dispose(e);
        q_dispose(predicate);
        c_free(subtype);
        c_free(query);
        v_dataViewQueryFree(query);
        if (values) {
            os_free(values);
        }
        return NULL;
    }
    c_free(subtype);

#if PRINT_QUERY
    printf("v_datyaReaderQueryNew\n");
    printf("predicate:\n");
    q_print(predicate,0);
    printf("\n");
#endif

    /* Normilize the query to the disjunctive form. */
    q_disjunctify(e);
    e = q_removeNots(e);

    list = deOr(e,NULL);

    len = c_iterLength(list);
    type = c_resolve(c_getBase(c_object(kernel)),"c_query");
    query->instanceQ = c_arrayNew(type,len);
    query->sampleQ = c_arrayNew(type,len);
    c_free(type);
    for (i=0;i<len;i++) {
        subExpr = c_iterTakeFirst(list);
        assert(subExpr != NULL);
        keyList = v_dataViewKeyList(view);
        keyExpr = q_takeKey(&subExpr, keyList);
        c_free(keyList);
        if (keyExpr != NULL) {
#if PRINT_QUERY
            printf("keyExpr[%d]: ",i);
            q_print(keyExpr,12);
            printf("\n");
#endif
            progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
            query->instanceQ[i] = c_queryNew(view->instances,progExpr,values);
            q_dispose(progExpr);
        } else {
#if PRINT_QUERY
            printf("keyExpr[%d]: <NULL>\n",i);
#endif
            query->instanceQ[i] = NULL;
        }
        if (subExpr != NULL) {
#if PRINT_QUERY
            printf("subExpr[%d]: ",i);
            q_print(subExpr,12);
            printf("\n");
#endif
            progExpr = F1(Q_EXPR_PROGRAM,subExpr);
            query->sampleQ[i]   = c_queryNew(view->instances,progExpr,values);
            q_dispose(progExpr);
        } else {
#if PRINT_QUERY
            printf("subExpr[%d]: <NULL>\n",i);
#endif
            query->sampleQ[i]   = NULL;
        }
    }
    c_iterFree(list);
    q_dispose(predicate);

    found = ospl_c_insert(v_collection(view)->queries,query);
    assert(found == query);
    OS_UNUSED_ARG(found);
    v_dataViewUnlock(view);
    if (values) {
        os_free(values);
    }

#if PRINT_QUERY
    printf("End v_dataViewQueryNew\n\n");
#endif

    return query;
}

void
v_dataViewQueryFree (
    v_dataViewQuery _this)
{
    v_collection src;
    v_dataView v;

    assert(C_TYPECHECK(_this,v_dataViewQuery));
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);
            v_dataViewLock(v);
            if (_this->triggerValue) {
                v_dataViewTriggerValueFree(_this->triggerValue);
                _this->triggerValue = NULL;
            }
            v_dataViewUnlock(v);
            v_queryFree(v_query(_this));
        } else {
            OS_REPORT(OS_ERROR, "v_dataViewQueryFree failed", 0,
                      "source is not a dataView");
        }
    } else {
        OS_REPORT(OS_ERROR, "v_dataViewQueryFree failed", 0,
                  "no source");
    }
}

void
v_dataViewQueryDeinit (
    v_dataViewQuery _this)
{
    v_collection src;
    v_dataView v;
    v_dataViewQuery found;

    if (_this != NULL) {
        assert(C_TYPECHECK(_this,v_dataViewQuery));

        src = v_querySource(v_query(_this));
        if (src != NULL) {
            assert(v_objectKind(src) == K_DATAVIEW);
            if (v_objectKind(src) == K_DATAVIEW) {
                v = v_dataView(src);
                v_dataViewLock(v);
                found = c_remove(v_collection(v)->queries,_this,NULL,NULL);
                assert(_this == found);
                /* Free the query found because it has been removed from the
                 * queries-collection */
                c_free(found);
                v_queryDeinit(v_query(_this));
                v_dataViewUnlock(v);
            } else {
                OS_REPORT(OS_ERROR,
                          "v_dataViewQueryDeinit failed", V_RESULT_ILL_PARAM,
                          "source is not datareader");
            }
            c_free(src);
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryDeinit failed", V_RESULT_ILL_PARAM,
                      "no source");
        }
    }
}

c_bool
v_dataViewQueryTestSample(
    v_dataViewQuery _this,
    v_dataViewSample sample)
{
    c_bool pass = FALSE;
    c_ulong len, i;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    pass = v_sampleMaskPass(_this->sampleMask, sample);
    if (pass) {
        len = c_arraySize(_this->instanceQ);
        if (len > 0) {
            pass = FALSE;
            for (i=0; (i<len) && !pass; i++)
            {
                if (_this->instanceQ[i] != NULL)
                {
                    pass = c_queryEval(_this->instanceQ[i],v_readerSample(sample)->instance);
                }
                if (pass && _this->sampleQ[i] != NULL) {
                    pass = c_queryEval(_this->sampleQ[i],v_readerSample(sample)->instance);
                }
            }
        }
    }
    return pass;
}

C_STRUCT(testActionArg) {
    c_query query;
    c_bool result;
    v_queryAction *action;
    c_voidp args;
    v_state sampleMask;
};

C_CLASS(testActionArg);

static c_bool
testAction(
    c_object o,
    c_voidp arg)
{
    v_dataViewInstance inst = v_dataViewInstance(o);
    testActionArg a = (testActionArg)arg;

    a->result = v_dataViewInstanceTest(inst, a->query, a->sampleMask, a->action, a->args);
    return (!a->result);
}

c_bool
v_dataViewQueryTest(
    v_dataViewQuery _this,
    v_queryAction action,
    c_voidp args)
{
    v_collection src;
    v_dataView v;
    c_ulong len,i;
    C_STRUCT(testActionArg) argument;
    c_table instanceSet;
    c_bool pass = FALSE;
    v_dataViewInstance instance;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    argument.result = FALSE;
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            instanceSet = v->instances;
            if (c_tableCount(instanceSet) > 0)
            {
                if (_this->triggerValue)
                {
                    /* The trigger value still correctly represents the query's trigger state
                     * if it still belongs to its former instance.
                     */
                    if (!v_dataViewSampleTestState(_this->triggerValue, L_REMOVED))
                    {
                        instance = v_dataViewInstance(v_readerSample(_this->triggerValue)->instance);

                        /* This part should be moved to the notify method
                         * as part of the producer query evaluation.
                         */
                        len = c_arraySize(_this->instanceQ);

                        /* Walk over the individual terms of the query
                         * that together make up the logical OR of the
                         * original SQL expression.
                         * The individual terms are separated in the terms
                         * that apply on key-values (instanceQ) and the terms
                         * that apply on non key-values (sampleQ). The indexes
                         * in both term lists should always correspond to the
                         * same term.
                         * TODO: Check whether masks are evaluated correctly.
                         */
                        if (v_sampleMaskPass(_this->sampleMask, _this->triggerValue)) {
                            for (i=0; (i<len) && !pass; i++)
                            {
                                pass = TRUE;
                                if (_this->instanceQ[i] != NULL)
                                {
                                    pass = c_queryEval(_this->instanceQ[i],instance);
                                }
                                if (pass && (_this->sampleQ[i] != NULL)) {
                                    v_dataViewSample firstSample;
                                    firstSample = v_dataViewInstanceTemplate(instance)->sample;
                                    if (_this->triggerValue != firstSample) {
                                        v_dataViewInstanceTemplate(instance)->sample = _this->triggerValue;
                                    }
                                    pass = c_queryEval(_this->sampleQ[i],instance);
                                    if (_this->triggerValue != firstSample) {
                                        v_dataViewInstanceTemplate(instance)->sample = firstSample;
                                    }
                                }
                            }
                        }

                        /* If the sample passed the query, then check whether
                         * it passes the action routine as well.
                         * Note: this action routine is used by the gapi to
                         * match the sample and instance state with the masks
                         * provided.
                         */
                        if (pass)
                        {
                            pass = action(_this->triggerValue, args);
                        }
                        if (!pass)
                        {
                            /* The trigger_value no longer satisfies the Query.
                             * It can therefore be reset.
                             */
                            v_dataViewTriggerValueFree(_this->triggerValue);
                            _this->triggerValue = NULL;
                        }
                    }
                    else
                    {
                        /* The trigger value is no longer available in the DataReader.
                         * It can therefore be reset.
                         */
                        v_dataViewTriggerValueFree(_this->triggerValue);
                        _this->triggerValue = NULL;
                    }
                }
                /* If the trigger value does not satisfy the Query, but other
                 * available samples could, then walk over all available samples
                 * until one is found that does satisfy the Query.
                 */
                if (_this->triggerValue == NULL && _this->walkRequired) {
                    argument.result = FALSE;
                    argument.action = action;
                    argument.args = args;
                    argument.sampleMask = _this->sampleMask;

                    len = c_arraySize(_this->instanceQ);
                    i = 0;
                    while ((i<len) && (pass == FALSE)) {
                        argument.query = _this->sampleQ[i];
                        if (_this->instanceQ[i] != NULL) {
                            c_readAction(_this->instanceQ[i],testAction,&argument);
                        } else {
                            c_readAction(instanceSet,testAction,&argument);
                        }
                        pass = argument.result;
                        i++;
                    }
                    if (!pass) {
                        /* None of the available samples satisfy the Query.
                         * That means the next query evaluation no longer
                         * requires us to walk over all samples.
                         */
                        _this->walkRequired = FALSE;
                    }
                }
            }
            if ( !pass ) {
                _this->state = V_STATE_INITIAL;
            }
            v_dataViewUnlock(v);
        } else {
            OS_REPORT(OS_CRITICAL,
                      "v_dataViewQueryTest failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        OS_REPORT(OS_CRITICAL,
                  "v_dataViewQueryTest failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
    return pass;
}

C_STRUCT(walkQueryArg) {
    v_dataView dataView;
    c_query query;
    v_readerSampleAction action;
    c_voidp arg;
    c_iter emptyList;
    v_state sampleMask;
    c_long count;
};
C_CLASS(walkQueryArg);

/* Read functions */

static v_actionResult
instanceSampleAction(
    c_object sample,
    c_voidp arg)
{
    walkQueryArg a = (walkQueryArg)arg;
    a->count++;
    return a->action(sample,a->arg);
}

static c_bool
instanceReadSamples(
    v_dataViewInstance instance,
    c_voidp arg)
{
    walkQueryArg a = (walkQueryArg)arg;

    assert(!v_dataViewInstanceEmpty(instance));
    return v_dataViewInstanceReadSamples(instance,
                                         a->query,
                                         a->sampleMask,
                                         instanceSampleAction,
                                         arg);
}

static v_result
waitForData(
    v_dataViewQuery _this,
    os_duration *delay)
{
    v_result result = V_RESULT_OK;
    /* If no data read then wait for data or timeout.
     */
    if (*delay > 0) {
        c_ulong flags = 0;
        os_timeE time = os_timeEGet();
        v__observerSetEvent(v_observer(_this), V_EVENT_DATA_AVAILABLE);
        flags = v__observerTimedWait(v_observer(_this), *delay);
        if (flags & V_EVENT_TIMEOUT) {
            result = V_RESULT_TIMEOUT;
        } else {
            *delay -= os_timeEDiff(os_timeEGet(), time);
        }
    } else {
        result = V_RESULT_NO_DATA;
    }
    return result;
}

v_result
v_dataViewQueryRead (
    v_dataViewQuery _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    c_ulong i,len;
    C_STRUCT(walkQueryArg) argument;
    v_result result = V_RESULT_OK;

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);
            v_dataViewLock(v);
            argument.action = action;
            argument.arg = arg;
            argument.sampleMask = _this->sampleMask;
            argument.count = 0;

            while ((argument.count == 0) && (result == V_RESULT_OK))
            {
                if (_this->walkRequired == FALSE) {
                    if (_this->triggerValue != NULL) {
                       if (!v_dataViewSampleTestState(_this->triggerValue, L_REMOVED)) {
                           proceed = v_actionResultTest(v_dataViewSampleReadTake(_this->triggerValue,
                                                        instanceSampleAction, &argument, FALSE), V_PROCEED);
                       } else {
                           proceed = FALSE;
                       }
                       /* The trigger_value may no longer satisfies the Query if the query addresses the
                        * read state, therefore reset the trigger value.
                        * A more optimal approach is to make this conditional depending whether the query
                        * addresses the read state.
                        */
                       v_dataViewTriggerValueFree(_this->triggerValue);
                       _this->triggerValue = NULL;
                    } else {
                        proceed = FALSE;
                    }
                } else {
                    v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));
                    len = c_arraySize(_this->instanceQ);
                    for (i=0;(i<len) && proceed;i++) {
                        argument.query = _this->sampleQ[i];
                        if (_this->instanceQ[i] != NULL) {
                            proceed = c_walk(_this->instanceQ[i],
                                             (c_action)instanceReadSamples,
                                             &argument);
                        } else {
                            proceed = c_tableWalk(v->instances,
                                                  (c_action)instanceReadSamples,
                                                  &argument);
                        }
                    }
                }
                if (argument.count == 0) {
                    result = waitForData(_this, &timeout);
                }
            }
            /* This triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            result = V_RESULT_ILL_PARAM;
            proceed = FALSE;
            OS_REPORT(OS_CRITICAL, "v_dataViewQueryRead failed", result, "source is not dataview");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        proceed = FALSE;
        OS_REPORT(OS_CRITICAL, "v_dataViewQueryRead failed", result, "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }
    /* Should fall within a lock on _this */
    if (v_query(_this)->statistics) {
        v_query(_this)->statistics->numberOfReads++;
    }
    return result;
}

v_result
v_dataViewQueryReadInstance(
    v_dataViewQuery _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    c_ulong i,len;
    v_result result = V_RESULT_OK;

    if (instance == NULL) {
        return V_RESULT_ILL_PARAM;
    }
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            C_STRUCT(walkQueryArg) argument;
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            argument.action = action;
            argument.arg = arg;
            argument.count = 0;

            while ((argument.count == 0) && (result == V_RESULT_OK))
            {
                if (!v_dataViewInstanceEmpty(instance)) {
                    len = c_arraySize(_this->instanceQ);
                    i=0;
                    while ((i<len) && proceed) {
                        if (_this->instanceQ[i] != NULL) {
                            if (c_queryEval(_this->instanceQ[i],instance)) {
                                proceed = v_dataViewInstanceReadSamples(
                                              instance,
                                              _this->sampleQ[i],
                                              _this->sampleMask,
                                              instanceSampleAction,
                                              &argument);
                            }
                        } else {
                            proceed = v_dataViewInstanceReadSamples(
                                          instance,
                                          _this->sampleQ[i],
                                          _this->sampleMask,
                                          instanceSampleAction,
                                          &argument);
                        }
                        i++;
                    }
                }
                if (argument.count == 0) {
                    result = waitForData(_this, &timeout);
                }
            }
            /* This triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            result = V_RESULT_ILL_PARAM;
            proceed = FALSE;
            OS_REPORT(OS_CRITICAL, "v_dataViewQueryReadInstance failed", result, "source is not dataview");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        proceed = FALSE;
        OS_REPORT(OS_CRITICAL, "v_dataViewQueryReadInstance failed", result, "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    if (v_query(_this)->statistics) {
        v_query(_this)->statistics->numberOfInstanceReads++;
    }
    return result;
}

struct nextInstanceActionArg {
    v_readerSampleAction action;
    c_voidp arg;
    c_bool hasData;
};

static v_actionResult
nextInstanceAction(
    c_object sample,
    c_voidp arg)
{
    struct nextInstanceActionArg *a = (struct nextInstanceActionArg *)arg;
    v_actionResult result;
    result = a->action(sample,a->arg);
    a->hasData = v_actionResultTestNot(result, V_SKIP);
    return result;
}

v_result
v_dataViewQueryReadNextInstance(
    v_dataViewQuery _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    c_ulong i,len;
    v_dataViewInstance nextInstance;
    struct nextInstanceActionArg a;
    v_result result = V_RESULT_OK;

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            C_STRUCT(walkQueryArg) argument;
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            len = c_arraySize(_this->instanceQ);
            nextInstance = c_tableNext(v->instances,instance);

            a.action = action;
            a.arg = arg;
            a.hasData = FALSE;

            argument.action = nextInstanceAction;
            argument.arg = &a;
            argument.count = 0;

            while ((argument.count == 0) && (result == V_RESULT_OK))
            {
                while ((nextInstance != NULL) && (a.hasData == FALSE)) {
                    i=0;
                    while ((i<len) && proceed) {
                        if (_this->instanceQ[i] != NULL) {
                            if (c_queryEval(_this->instanceQ[i],nextInstance)) {
                                proceed = v_dataViewInstanceReadSamples(
                                              nextInstance,
                                              _this->sampleQ[i],
                                              _this->sampleMask,
                                              instanceSampleAction,
                                              &argument);
                            }
                        } else {
                            proceed = v_dataViewInstanceReadSamples(
                                          nextInstance,
                                          _this->sampleQ[i],
                                          _this->sampleMask,
                                          instanceSampleAction,
                                          &argument);
                        }
                        i++;
                    }
                    nextInstance = c_tableNext(v->instances,nextInstance);
                }
                if (argument.count == 0) {
                    result = waitForData(_this, &timeout);
                }
            }
            /* This triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            result = V_RESULT_ILL_PARAM;
            proceed = FALSE;
            OS_REPORT(OS_CRITICAL, "v_dataViewQueryReadNextInstance failed", result, "source is not dataview");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        proceed = FALSE;
        OS_REPORT(OS_CRITICAL, "v_dataViewQueryReadNextInstance failed", result, "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    if (v_query(_this)->statistics) {
        v_query(_this)->statistics->numberOfNextInstanceReads++;
    }
    return result;
}


/* Take functions */

static c_bool
instanceTakeSamples(
    v_dataViewInstance instance,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    walkQueryArg a = (walkQueryArg)arg;

    proceed = v_dataViewInstanceTakeSamples(instance,
                                            a->query,
                                            a->sampleMask,
                                            instanceSampleAction,
                                            arg);
    if (v_dataViewInstanceEmpty(instance)) {
        a->emptyList = c_iterInsert(a->emptyList,instance);
    }
    return proceed;
}

v_result
v_dataViewQueryTake(
    v_dataViewQuery _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    v_dataViewInstance emptyInstance;
    v_dataViewInstance found;
    c_ulong len, i;
    C_STRUCT(walkQueryArg) argument;
    v_result result = V_RESULT_OK;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);

            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            argument.dataView = v;
            argument.action = action;
            argument.arg = arg;
            argument.emptyList = NULL;
            argument.sampleMask = _this->sampleMask;
            argument.count = 0;

            while ((argument.count == 0) && (result == V_RESULT_OK))
            {
                if (_this->walkRequired == FALSE) {
                    if (_this->triggerValue != NULL) {
                       if (!v_dataViewSampleTestState(_this->triggerValue, L_REMOVED)) {
                           proceed = v_actionResultTest(v_dataViewSampleReadTake(_this->triggerValue,
                                                        instanceSampleAction, &argument, TRUE), V_PROCEED);
                       } else {
                           proceed = FALSE;
                       }
                       /* The trigger_value no longer satisfies the Query or
                        * has been taken. It can therefore be reset.
                        */
                       v_dataViewTriggerValueFree(_this->triggerValue);
                       _this->triggerValue = NULL;

                    } else {
                        proceed = FALSE;
                    }
                } else {
                    len = c_arraySize(_this->instanceQ);
                    for (i=0;(i<len) && proceed;i++) {
                        argument.query = _this->sampleQ[i];
                        if (_this->instanceQ[i] != NULL) {
                            proceed = c_walk(_this->instanceQ[i],
                                             (c_action)instanceTakeSamples,
                                             &argument);
                        } else {
                            proceed = c_tableWalk(v->instances,
                                                  (c_action)instanceTakeSamples,
                                                  &argument);
                        }
                    }
                    if (argument.emptyList != NULL) {
                        emptyInstance = c_iterTakeFirst(argument.emptyList);
                        while (emptyInstance != NULL) {
                            found = c_remove(v->instances,emptyInstance,NULL,NULL);
                            assert(found == emptyInstance);
                            v_publicFree(v_public(found));
                            c_free(found);
                            emptyInstance = c_iterTakeFirst(argument.emptyList);
                        }
                        c_iterFree(argument.emptyList);
                    }
                }
                if (argument.count == 0) {
                    result = waitForData(_this, &timeout);
                }
            }
            /* This triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            result = V_RESULT_ILL_PARAM;
            proceed = FALSE;
            OS_REPORT(OS_CRITICAL, "v_dataViewQueryTake failed", result, "source is not dataview");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        proceed = FALSE;
        OS_REPORT(OS_CRITICAL, "v_dataViewQueryTake failed", result, "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    if (v_query(_this)->statistics) {
        v_query(_this)->statistics->numberOfTakes++;
    }
    return result;
}

v_result
v_dataViewQueryTakeInstance(
    v_dataViewQuery _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    v_dataViewInstance found;
    c_ulong len, i;
    v_result result = V_RESULT_OK;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    if (instance == NULL) {
        /* Should fall within a lock on _this */
        if (v_query(_this)->statistics) {
            v_query(_this)->statistics->numberOfInstanceTakes++;
        }
        return V_RESULT_ILL_PARAM;
    }
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            C_STRUCT(walkQueryArg) argument;
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            argument.dataView = v;
            argument.action = action;
            argument.arg = arg;
            argument.emptyList = NULL;
            argument.sampleMask = _this->sampleMask;
            argument.count = 0;

            while ((argument.count == 0) && (result == V_RESULT_OK))
            {
                if (!v_dataViewInstanceEmpty(instance)) {
                    len = c_arraySize(_this->instanceQ);
                    i=0;
                    while ((i<len) && proceed) {
                        if (_this->instanceQ[i] != NULL) {
                            if (c_queryEval(_this->instanceQ[i],instance)) {
                                proceed = v_dataViewInstanceTakeSamples(
                                               instance,
                                               _this->sampleQ[i],
                                               _this->sampleMask,
                                               instanceSampleAction, &argument);
                            }
                        } else {
                            proceed = v_dataViewInstanceTakeSamples(
                                               instance,
                                               _this->sampleQ[i],
                                               _this->sampleMask,
                                               instanceSampleAction, &argument);
                        }
                        i++;
                    }
                    if (v_dataViewInstanceEmpty(instance)) {
                        found = c_remove(v->instances,instance,NULL,NULL);
                        assert(found == instance);
                        v_publicFree(v_public(found));
                        c_free(found);
                    }
                }
                if (argument.count == 0) {
                    result = waitForData(_this, &timeout);
                }
            }
            /* The call to the actioni routine with a NULL parameter
             * triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            result = V_RESULT_ILL_PARAM;
            proceed = FALSE;
            OS_REPORT(OS_CRITICAL, "v_dataViewQueryTakeInstance failed", result, "source is not dataview");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        proceed = FALSE;
        OS_REPORT(OS_CRITICAL, "v_dataViewQueryTakeInstance failed", result, "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    if (v_query(_this)->statistics) {
        v_query(_this)->statistics->numberOfInstanceTakes++;
    }
    return result;
}

v_result
v_dataViewQueryTakeNextInstance(
    v_dataViewQuery _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    c_ulong len, i;
    v_dataViewInstance nextInstance;
    v_dataViewInstance savedInstance;
    v_dataViewInstance found;
    struct nextInstanceActionArg a;
    v_result result = V_RESULT_OK;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            C_STRUCT(walkQueryArg) argument;
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            len = c_arraySize(_this->instanceQ);
            nextInstance = c_tableNext(v->instances,instance);
            a.action = action;
            a.arg = arg;
            a.hasData = FALSE;

            argument.action = nextInstanceAction;
            argument.arg = &a;
            argument.sampleMask = _this->sampleMask;
            argument.count = 0;

            while ((argument.count == 0) && (result == V_RESULT_OK))
            {
                while ((nextInstance != NULL) && (a.hasData == FALSE)) {
                    i=0;
                    while ((i<len) && proceed) {
                        if (_this->instanceQ[i] != NULL) {
                            if (c_queryEval(_this->instanceQ[i],nextInstance)) {
                                proceed = v_dataViewInstanceTakeSamples(
                                              nextInstance,
                                              _this->sampleQ[i],
                                               _this->sampleMask,
                                              instanceSampleAction,&argument);
                            }
                        } else {
                            proceed = v_dataViewInstanceTakeSamples(
                                              nextInstance,
                                              _this->sampleQ[i],
                                               _this->sampleMask,
                                              instanceSampleAction,&argument);
                        }
                        i++;
                    }
                    savedInstance = c_tableNext(v->instances,nextInstance);
                    if (v_dataViewInstanceEmpty(nextInstance)) {
                        found = c_remove(v->instances,nextInstance,NULL,NULL);
                        assert(found == nextInstance);
                        v_publicFree(v_public(found));
                        c_free(found);
                    }
                    nextInstance = savedInstance;
                }
                if (argument.count == 0) {
                    result = waitForData(_this, &timeout);
                }
            }
            /* The call to the actioni routine with a NULL parameter
             * triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            result = V_RESULT_ILL_PARAM;
            proceed = FALSE;
            OS_REPORT(OS_CRITICAL, "v_dataViewQueryTakeNextInstance failed", result, "source is not dataView");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        proceed = FALSE;
        OS_REPORT(OS_CRITICAL, "v_dataViewQueryTakeNextInstance failed", result, "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    if (v_query(_this)->statistics) {
        v_query(_this)->statistics->numberOfNextInstanceTakes++;
    }
    return result;
}

c_bool
v_dataViewQueryNotifyDataAvailable(
    v_dataViewQuery _this,
    v_event e)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_dataViewQuery));
    assert(e);
    assert(C_TYPECHECK(e->data,v_dataViewSample));

    v_observerLock(v_observer(_this));

    if (e->data) {
        if (_this->triggerValue == NULL) {
            _this->triggerValue = v_dataReaderTriggerValueKeep(e->data);
        } else {
            _this->walkRequired = TRUE;
        }
        _this->state |= V_STATE_DATA_AVAILABLE;
        v_observableNotify(v_observable(_this),e);
        /* Notify for internal use only, result can be ignored */
        (void)v_entityNotifyListener(v_entity(_this), e);
    } else {
        OS_REPORT(OS_WARNING,
                  "v_dataViewQueryNotifyDataAvailable failed", V_RESULT_ILL_PARAM,
                  "No triggerValue provided");
        assert(FALSE);
    }
    v_observerUnlock(v_observer(_this));

    return TRUE;
}

c_bool
v_dataViewQuerySetParams(
    v_dataViewQuery _this,
    const os_char *params[],
    const os_uint32 nrOfParams)
{
    v_collection src;
    v_dataView v;
    v_kernel kernel;
    os_uint32 i,len;
    q_expr e,subExpr,keyExpr,progExpr;
    q_expr predicate;
    c_iter list;
    c_type type;
    c_bool result = TRUE;
    c_array keyList;
    c_type subtype;
    c_value *values;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {

            if (nrOfParams > 0) {
                values = os_malloc(nrOfParams * sizeof(c_value));
                for (i=0; i<nrOfParams; i++) {
                    values[i] = c_stringValue((const c_string)params[i]);
                }
            } else {
                values = NULL;
            }

            kernel = v_objectKernel(_this);
            v = v_dataView(src);

            v_dataViewLock(v);

            len = c_arraySize(_this->instanceQ);
            for (i=0; (i<len) && (result == TRUE); i++) {
                result = c_querySetParams(_this->instanceQ[i],values) &&
                         c_querySetParams(_this->sampleQ[i],values);
            }
            if (result) {
                v_dataViewUnlock(v);
            } else {
                predicate = v_queryGetPredicate(v_query(_this));
                q_prefixFieldNames(&predicate,"sample.sample.message.userData");

#if PRINT_QUERY
                printf("v_datyaViewQuerySetParams\n");
                printf("predicate:\n"); q_print(predicate,0); printf("\n");
#endif

                e = q_takePar(predicate,0);
                subtype = c_subType(v->instances);
                if (!resolveFields(subtype,e)) {
                    v_dataViewUnlock(v);
                    q_dispose(e);
                    q_dispose(predicate);
                    c_free(subtype);
                    os_free(values);
                    return FALSE;
                }
                c_free(subtype);

                /* Normilize the query to the disjunctive form. */
                q_disjunctify(e);
                e = q_removeNots(e);

                list = deOr(e,NULL);

                len = c_iterLength(list);
                type = c_resolve(c_getBase(c_object(kernel)),"c_query");
                c_free(_this->instanceQ);
                c_free(_this->sampleQ);
                _this->instanceQ = c_arrayNew(type,len);
                _this->sampleQ = c_arrayNew(type,len);
                c_free(type);
                for (i=0;i<len;i++) {
                    subExpr = c_iterTakeFirst(list);
                    assert(subExpr != NULL);
                    keyList = v_dataViewKeyList(v);
                    keyExpr = q_takeKey(&subExpr, keyList);
                    c_free(keyList);
                    if (keyExpr != NULL) {
#if PRINT_QUERY
                        printf("keyExpr[%d]: ",i);
                        q_print(keyExpr,12);
                        printf("\n");
#endif
                        progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
                        _this->instanceQ[i] = c_queryNew(v->instances,
                                                         progExpr,
                                                         values);
                        q_dispose(progExpr);
                    } else {
#if PRINT_QUERY
                        printf("keyExpr[%d]: <NULL>\n",i);
#endif
                        _this->instanceQ[i] = NULL;
                    }
                    if (subExpr != NULL) {
#if PRINT_QUERY
                        printf("subExpr[%d]: ",i);
                        q_print(subExpr,12);
                        printf("\n");
#endif
                        /* The following code generates the intermeadiate non-key
                         * query code. Unfortunatly c_queryNew creates the query
                         * expression relative to the given collection's element
                         * type. In this case the instance type.
                         * This means that to performe the query evaluation on
                         * each sample within an instance the sample must be
                         * swapped with the instance sample field and reswapped
                         * after the evaluation.
                         */
                        progExpr = F1(Q_EXPR_PROGRAM,subExpr);
                        _this->sampleQ[i] = c_queryNew(v->instances,
                                                       progExpr,
                                                       values);
                        q_dispose(progExpr);
                    } else {
#if PRINT_QUERY
                        printf("subExpr[%d]: <NULL>\n",i);
#endif
                        _this->sampleQ[i]   = NULL;
                    }
                }
                c_iterFree(list);
                v_dataViewUnlock(v);
#if PRINT_QUERY
                printf("End v_dataViewQuerySetParams\n\n");
#endif
                q_dispose(predicate);
            }
            result = TRUE;
            _this->walkRequired = TRUE;
            if (values) {
                os_free(values);
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataViewQuerySetParams failed", V_RESULT_ILL_PARAM,
                      "source is not dataView");
            result = FALSE;
        }
        c_free(src);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataViewQuerySetParams failed", V_RESULT_ILL_PARAM,
                  "no source");
        result = FALSE;
    }
    return result;
}
