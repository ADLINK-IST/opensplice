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
#include "v__dataViewSample.h"
#include "q_helper.h"
#include "v__statCat.h"
#include "v__kernel.h"
#include "v_statistics.h"
#include "v_queryStatistics.h"
#include "v__statistics.h"

#include "c_stringSupport.h"

#include "os.h"
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
    c_long i, length;
    q_list list;

    c_char *metaName;

    field = c_fieldNew(type,name);

    if (field == NULL) {
        return NULL;
    }
    path = c_fieldPath(field);
    length = c_arraySize(path);
    list = NULL;
    for (i=(length-1);i>=0;i--) {
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
                OS_REPORT_1(OS_ERROR,
                            "v_dataViewQueryNew failed",0,
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
            OS_REPORT_1(OS_ERROR,
                        "v_dataViewQueryNew failed",0,
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
    v_dataView _this,
    const c_char *name,
    q_expr predicate,
    c_value params[])
{
    v_kernel kernel;
    v_dataViewQuery query,found;
    v_queryStatistics queryStatistics;
    c_long i,len;
    q_expr e,subExpr,keyExpr,progExpr;
    c_iter list;
    c_type type, subtype;
    c_array keyList;

    assert(C_TYPECHECK(_this,v_dataView));

    q_prefixFieldNames(&predicate,"sample.sample.message.userData");

    kernel = v_objectKernel(_this);
    if (q_getTag(predicate) !=  Q_EXPR_PROGRAM) {
        assert(FALSE);
        return NULL;
    }
    e = q_takePar(predicate,0);
    subtype = c_subType(_this->instances);
    if (!resolveFields(subtype,e)) {
        q_dispose(e);
        c_free(subtype);
        return NULL;
    }
    c_free(subtype);

    v_dataViewLock(_this);
    query = v_dataViewQuery(v_objectNew(kernel,K_DATAVIEWQUERY));

    if (v_isEnabledStatistics(kernel, V_STATCAT_READER)) {
        queryStatistics = v_queryStatisticsNew(kernel);
    } else {
        queryStatistics = NULL;
    }
    v_queryInit(v_query(query), name, v_statistics(queryStatistics),
                v_collection(_this), predicate, params);
    query->expression   = c_stringNew(c_getBase(_this),
                                      q_exprGetText(predicate));
    query->params       = NULL;
    query->instanceMask = q_exprGetInstanceState(predicate);
    query->sampleMask   = q_exprGetSampleState(predicate);
    query->viewMask     = q_exprGetViewState(predicate);
    query->triggerValue = NULL;
    query->walkRequired = TRUE;
    query->updateCnt    = 0;

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
        keyList = v_dataViewKeyList(_this);
        keyExpr = q_takeKey(&subExpr, keyList);
        c_free(keyList);
        if (keyExpr != NULL) {
#if PRINT_QUERY
            printf("keyExpr[%d]: ",i);
            q_print(keyExpr,12);
            printf("\n");
#endif
            progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
            query->instanceQ[i] = c_queryNew(_this->instances,progExpr,params);
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
            query->sampleQ[i]   = c_queryNew(_this->instances,progExpr,params);
            q_dispose(progExpr);
        } else {
#if PRINT_QUERY
            printf("subExpr[%d]: <NULL>\n",i);
#endif
            query->sampleQ[i]   = NULL;
        }
    }
    c_iterFree(list);
    found = c_insert(v_collection(_this)->queries,query);
    assert(found == query);
    v_dataViewUnlock(_this);
#if PRINT_QUERY
    printf("End v_dataViewQueryNew\n\n");
#endif

    return query;
}

void
v_dataViewQueryFree (
    v_dataViewQuery _this)
{
    assert(C_TYPECHECK(_this,v_dataViewQuery));
    v_queryFree(v_query(_this));
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
                          "v_dataViewQueryDeinit failed", 0,
                          "source is not datareader");
            }
            c_free(src);
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryDeinit failed", 0,
                      "no source");
        }
    }
}

C_STRUCT(testActionArg) {
    c_query query;
    c_bool result;
    v_queryAction *action;
    c_voidp args;
};

C_CLASS(testActionArg);

static c_bool
testAction(
    c_object o,
    c_voidp arg)
{
    v_dataViewInstance inst = v_dataViewInstance(o);
    testActionArg a = (testActionArg)arg;

    a->result = v_dataViewInstanceTest(inst,a->query, a->action, a->args);
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
    c_long len,i;
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
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryTest failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataViewQueryTest failed", 0,
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
};
C_CLASS(walkQueryArg);


/* Read functions */

static c_bool
instanceReadSamples(
    v_dataViewInstance instance,
    c_voidp arg)
{
    walkQueryArg a = (walkQueryArg)arg;

    assert(!v_dataViewInstanceEmpty(instance));
    return v_dataViewInstanceReadSamples(instance,
                                         a->query,
                                         a->action,
                                         a->arg);
}

c_bool
v_dataViewQueryRead (
    v_dataViewQuery _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    int i,len;
    C_STRUCT(walkQueryArg) argument;

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);
            v_dataViewLock(v);
            if (_this->walkRequired == FALSE) {
                if (_this->triggerValue != NULL) {
                   if (!v_dataViewSampleTestState(_this->triggerValue, L_REMOVED)) {
                       proceed = v_actionResultTest(v_dataViewSampleReadTake(_this->triggerValue,action,arg, FALSE), V_PROCEED);
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
                argument.action = action;
                argument.arg = arg;

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
            /* This triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryRead failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataViewQueryRead failed", 0,
                  "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }
    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfReads, _this);
    return proceed;
}

c_bool
v_dataViewQueryReadInstance(
    v_dataViewQuery _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    int i,len;

    if (instance == NULL) {
        return FALSE;
    }
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            if (!v_dataViewInstanceEmpty(instance)) {
                len = c_arraySize(_this->instanceQ);
                i=0;
                while ((i<len) && proceed) {
                    if (_this->instanceQ[i] != NULL) {
                        if (c_queryEval(_this->instanceQ[i],instance)) {
                            proceed = v_dataViewInstanceReadSamples(
                                          instance,
                                          _this->sampleQ[i],
                                          action,
                                          arg);
                        }
                    } else {
                        proceed = v_dataViewInstanceReadSamples(
                                      instance,
                                      _this->sampleQ[i],
                                      action,
                                      arg);
                    }
                    i++;
                }
            }
            /* This triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryReadInstance failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataViewQueryReadInstance failed", 0,
                  "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfInstanceReads, _this);
    return proceed;
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

c_bool
v_dataViewQueryReadNextInstance(
    v_dataViewQuery _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    int i,len;
    v_dataViewInstance nextInstance;
    struct nextInstanceActionArg a;


    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            len = c_arraySize(_this->instanceQ);
            nextInstance = c_tableNext(v->instances,instance);

            a.action = action;
            a.arg = arg;
            a.hasData = FALSE;
            while ((nextInstance != NULL) && (a.hasData == FALSE)) {
                i=0;
                while ((i<len) && proceed) {
                    if (_this->instanceQ[i] != NULL) {
                        if (c_queryEval(_this->instanceQ[i],nextInstance)) {
                            proceed = v_dataViewInstanceReadSamples(
                                          nextInstance,
                                          _this->sampleQ[i],
                                          nextInstanceAction,
                                          &a);
                        }
                    } else {
                        proceed = v_dataViewInstanceReadSamples(
                                      nextInstance,
                                      _this->sampleQ[i],
                                      nextInstanceAction,
                                      &a);
                    }
                    i++;
                }
                nextInstance = c_tableNext(v->instances,nextInstance);
            }
            /* This triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryReadNextInstance failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataViewQueryReadNextInstance failed", 0,
                  "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfNextInstanceReads, _this);
    return proceed;
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
                                            a->action,
                                            a->arg);
    if (v_dataViewInstanceEmpty(instance)) {
        a->emptyList = c_iterInsert(a->emptyList,instance);
    }
    return proceed;
}

c_bool
v_dataViewQueryTake(
    v_dataViewQuery _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    v_dataViewInstance emptyInstance;
    v_dataViewInstance found;
    int len, i;
    C_STRUCT(walkQueryArg) argument;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);

            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            if (_this->walkRequired == FALSE) {
                if (_this->triggerValue != NULL) {
                   if (!v_dataViewSampleTestState(_this->triggerValue, L_REMOVED)) {
                       proceed = v_actionResultTest(v_dataViewSampleReadTake(_this->triggerValue,action,arg, TRUE), V_PROCEED);
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
                argument.dataView = v;
                argument.action = action;
                argument.arg = arg;
                argument.emptyList = NULL;

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
            /* This triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryTake failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataViewQueryTake failed", 0,
                  "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfTakes, _this);
    return proceed;
}

c_bool
v_dataViewQueryTakeInstance(
    v_dataViewQuery _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    v_dataViewInstance found;
    int len, i;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    if (instance == NULL) {
        /* Should fall within a lock on _this */
        v_statisticsULongValueInc(v_query, numberOfInstanceTakes, _this);
        return FALSE;
    }
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            if (!v_dataViewInstanceEmpty(instance)) {
                len = c_arraySize(_this->instanceQ);
                i=0;
                while ((i<len) && proceed) {
                    if (_this->instanceQ[i] != NULL) {
                        if (c_queryEval(_this->instanceQ[i],instance)) {
                            proceed = v_dataViewInstanceTakeSamples(
                                           instance,
                                           _this->sampleQ[i],
                                           action,arg);

                        }
                    } else {
                        proceed = v_dataViewInstanceTakeSamples(
                                           instance,
                                           _this->sampleQ[i],
                                           action,arg);
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
            /* The call to the actioni routine with a NULL parameter
             * triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryTakeInstance failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataViewQueryTakeInstance failed", 0,
                  "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfInstanceTakes, _this);
    return proceed;
}

c_bool
v_dataViewQueryTakeNextInstance(
    v_dataViewQuery _this,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataView v;
    int len, i;
    v_dataViewInstance nextInstance;
    v_dataViewInstance savedInstance;
    v_dataViewInstance found;
    struct nextInstanceActionArg a;

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            v = v_dataView(src);
            v_dataViewLock(v);
            v_dataReaderUpdatePurgeLists(v_dataReader(v->reader));

            len = c_arraySize(_this->instanceQ);
            nextInstance = c_tableNext(v->instances,instance);
            a.action = action;
            a.arg = arg;
            a.hasData = FALSE;
            while ((nextInstance != NULL) && (a.hasData == FALSE)) {
                i=0;
                while ((i<len) && proceed) {
                    if (_this->instanceQ[i] != NULL) {
                        if (c_queryEval(_this->instanceQ[i],nextInstance)) {
                            proceed = v_dataViewInstanceTakeSamples(
                                          nextInstance,
                                          _this->sampleQ[i],
                                          nextInstanceAction,&a);
                        }
                    } else {
                        proceed = v_dataViewInstanceTakeSamples(
                                          nextInstance,
                                          _this->sampleQ[i],
                                          nextInstanceAction,&a);
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
            /* The call to the actioni routine with a NULL parameter
             * triggers the action routine that the last sample is read.
             */
            action(NULL,arg);
            v_dataViewUnlock(v);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataViewQueryTakeNextInstance failed", 0,
                      "source is not dataView");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataViewQueryTakeNextInstance failed", 0,
                  "no source");
    }

    if (!proceed) {
        _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
    }

    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfNextInstanceTakes, _this);
    return proceed;
}

c_bool
v_dataViewQueryNotifyDataAvailable(
    v_dataViewQuery _this,
    v_event e)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_dataViewQuery));
    assert(e);
    assert(C_TYPECHECK(e->userData,v_dataViewSample));

    if ((_this->state & V_STATE_DATA_AVAILABLE) == 0) {

        v_observerLock(v_observer(_this));

        if (e->userData) {
            if (_this->triggerValue == NULL) {
                _this->triggerValue = v_dataViewTriggerValueKeep(e->userData);
            } else {
                _this->walkRequired = TRUE;
            }
            _this->state |= V_STATE_DATA_AVAILABLE;

            v_observerNotify(v_observer(_this), e, NULL);
            v_observableNotify(v_observable(_this),e);
        } else {
            OS_REPORT(OS_WARNING,
                      "v_dataViewQueryNotifyDataAvailable failed", 0,
                      "No triggerValue provided");
            assert(FALSE);
        }
        v_observerUnlock(v_observer(_this));
    }
    return TRUE;
}

c_bool
v_dataViewQuerySetParams(
    v_dataViewQuery _this,
    q_expr expression,
    c_value params[])
{
    v_collection src;
    v_dataView v;
    v_kernel kernel;
    c_long i,len;
    q_expr e,subExpr,keyExpr,progExpr;
    q_expr predicate;
    c_iter list;
    c_type type;
    c_bool result = TRUE;
    c_array keyList;
    c_char *tmp;
    c_base base;
    c_type subtype;
#if 0
    c_long size, strSize, curSize, exprSize, count;
    c_char *paramString;
    c_char character, prevCharacter;
    c_char number[32];
    c_bool inNumber;
#endif

    assert(C_TYPECHECK(_this,v_dataViewQuery));

    if (q_getTag(expression) !=  Q_EXPR_PROGRAM) {
        assert(FALSE);
        return FALSE;
    }

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAVIEW);
        if (v_objectKind(src) == K_DATAVIEW) {
            kernel = v_objectKernel(_this);
            base = c_getBase(c_object(_this));
            v = v_dataView(src);

            v_dataViewLock(v);

            len = c_arraySize(_this->instanceQ);
            for (i=0; (i<len) && (result == TRUE); i++) {
                result = c_querySetParams(_this->instanceQ[i],params) &&
                         c_querySetParams(_this->sampleQ[i],params);
            }
            if (result) {
                v_dataViewUnlock(v);
            } else {
                predicate = q_exprCopy(expression);
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
                    q_dispose(expression);
                    c_free(subtype);
                    return FALSE;
                }
                c_free(subtype);

                _this->instanceMask = q_exprGetInstanceState(predicate);
                _this->sampleMask   = q_exprGetSampleState(predicate);
                _this->viewMask     = q_exprGetViewState(predicate);

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
                                                         params);
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
                                                       params);
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
                if(_this->expression){
                    c_free(_this->expression);
                    _this->expression = NULL;
                }
                tmp = q_exprGetText(predicate);

                if(tmp){
                    _this->expression = c_stringNew(base, tmp);
                    os_free(tmp);
                } else {
                    _this->expression = NULL;
                }
                q_dispose(predicate);
            }
            result = TRUE;
#if 0
            if(_this->params){
                c_free(_this->params);
                _this->params = NULL;
            }

            if(params){
                exprSize = strlen(_this->expression);
                prevCharacter = '\0';
                memset(number, 0, 32);
                size = -1;
                count = 0;
                inNumber = FALSE;

                /* Get the highest parameter number in the expression string.
                 * This number determines the the maximum index value to be
                 * used in the parameter array.
                 */
                for(i=0; i<exprSize; i++){
                    character = _this->expression[i];

                    if(character == '%'){
                        if(prevCharacter != '%'){
                            inNumber = TRUE;
                        }
                    } else if((character == ' ') && (inNumber == TRUE)){
                        curSize = atoi(number);

                        if(curSize > size){
                            size = curSize;
                        }
                        memset(number, 0, 32);
                        inNumber = FALSE;
                        count = 0;
                    } else if(inNumber == TRUE){
                        number[count++] = character;
                    }
                    prevCharacter = character;
                }
                if(inNumber == TRUE){
                    curSize = atoi(number);

                    if(curSize > size){
                        size = curSize;
                    }
                }
                size += 1;

                if(size > 0){
                    strSize = 0;

                    /* Determine the string size of a comma sepparated
                     * parameter list.
                     */
                    for(i=0; i<size; i++){
                        tmp = c_valueImage(params[i]);
                        strSize += strlen(tmp) + 1;
                        os_free(tmp);
                    }
                    /* Allocate and create a comma sepparated parameter list.
                     */
                    paramString = (c_char*)os_malloc(strSize);
                    memset(paramString, 0, strSize);

                    for(i=0; i<size; i++){
                        tmp = c_valueImage(params[i]);
                        os_strcat(paramString, tmp);
                        os_free(tmp);

                        if(i+1 != size){
                            os_strcat(paramString, ",");
                        }
                    }
                    _this->params = c_stringNew(base, paramString);
                    os_free(paramString);
                } else {
                    _this->params = NULL;
                }
            } else {
                _this->params = NULL;
            }
#endif
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataViewQuerySetParams failed", 0,
                      "source is not dataView");
            result = FALSE;
        }
        c_free(src);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataViewQuerySetParams failed", 0,
                  "no source");
        result = FALSE;
    }

    return result;
}
