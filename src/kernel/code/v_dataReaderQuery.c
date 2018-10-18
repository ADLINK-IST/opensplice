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
#include "v__dataReader.h"
#include "v__reader.h"
#include "v_state.h"
#include "v_event.h"
#include "v_index.h"
#include "v_projection.h"
#include "v_entity.h"
#include "v_handle.h"
#include "v__dataReaderInstance.h"
#include "v__query.h"
#include "v__observable.h"
#include "v__observer.h"
#include "v_public.h"
#include "v__collection.h"
#include "v__deadLineInstanceList.h"
#include "v__dataView.h"
#include "v__dataReaderSample.h"
#include "q_helper.h"
#include "v_kernelParser.h"
#include "v__statCat.h"
#include "v__subscriber.h"
#include "v__kernel.h"
#include "v_listener.h"
#include "v_queryStatistics.h"
#include "v__orderedInstance.h"

#include "c_stringSupport.h"

#include "vortex_os.h"
#include "os_report.h"

#define V_STATE_INITIAL        (0x00000000U)       /* 0 */
#define V_STATE_ACTIVE         (0x00000001U)       /* 1 */
#define V_STATE_DATA_AVAILABLE (0x00000001U << 1)  /* 2 */

#define MAX_PARAM_ID_SIZE (32)

static q_expr
resolveField(
    v_dataReader _this,
    const c_char *name)
{
    c_field field;
    c_array path;
    c_ulong i, length;
    q_list list;
    c_string str;

    field = v_dataReaderField(_this,name);
    if (field == NULL) {
        return NULL;
    }
    path = c_fieldPath(field);
    length = c_arraySize(path);
    list = NULL;
    i = length;
    while (i-- > 0) {
        str = c_metaName(path[i]);
        list = q_insert(list,q_newId(str));
        c_free(str);
    }
    c_free(field);

    return q_newFnc(Q_EXPR_PROPERTY,list);
}

static c_bool
resolveFields (
    v_dataReader _this,
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
            p = resolveField(_this,name);
            if (p == NULL) {
                OS_REPORT(OS_ERROR,
                            "v_dataReaderQueryNew failed",V_RESULT_ILL_PARAM,
                            "field %s undefined",name);
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
                if (!resolveFields(_this,p)) {
                    return FALSE;
                }
                i++;
            }
        }
    break;
    case T_ID:
        name = q_getId(e);
        p = resolveField(_this,name);
        if (p == NULL) {
            OS_REPORT(OS_ERROR,
                        "v_dataReaderQueryNew failed",V_RESULT_ILL_PARAM,
                        "field %s undefined",name);
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

static void
translate(
    q_expr expr,
    c_array sourceKeyList, /* c_array<c_field> */
    c_array indexKeyList)  /* c_array<c_field> */
{
    assert(expr);
    assert(sourceKeyList);
    assert(indexKeyList);

    if(q_getKind(expr) == T_FNC){
        if(q_isFnc(expr, Q_EXPR_PROPERTY)) {
            /* first get the string representation of the id's in this expr */
            c_field f;
            c_ulong i, index, size = 0;
            c_char *name;

            name = q_propertyName(expr);
            if(name) {
                /* Now find the matching key in the sourceKeyList */
                index = size = c_arraySize(sourceKeyList);

                assert(size == c_arraySize(indexKeyList));

                if(size == c_arraySize(indexKeyList)){
                    for(i=0; i<size; i++) {
                        f = (c_field)(sourceKeyList[i]);
                        if(strcmp(c_fieldName(f), name) == 0) {
                            index = i;
                            break;
                        }
                    }

                    assert(index < size);

                    if(index < size) {
                        /* now replace the Q_EXPR_PROPERTY id's by the indexKeyList ones */
                        q_expr e;
                        c_char *fieldNameStr;
                        c_char *str;
                        c_iter ids;

                        f = (c_field)(indexKeyList[index]);
                        fieldNameStr = c_fieldName(f);

                        /* clear current list */
                        e = q_takePar(expr, 0);
                        while(e){
                            q_dispose(e);
                            e = q_takePar(expr, 0);
                        }

                        ids = c_splitString(fieldNameStr, ".");
                        if(ids){
                            str = (c_char*)c_iterTakeFirst(ids);
                            while(str){
                                e = q_newId(str);
                                q_addPar(expr, e);
                                os_free(str);
                                str = (c_char*)c_iterTakeFirst(ids);
                            }
                            c_iterFree(ids);
                        }
                    } else {
                        OS_REPORT(OS_WARNING,"v_dataReaderQuery_translate failed", 0,
                                  "Cannot find key '%s' in key list.", name);
                    }
                } else {
                    OS_REPORT(OS_ERROR,"v_dataReaderQuery_translate failed", 0,
                              "sizes of indexKeyList (size %d) and sourceKeyList (size %d) do not match.",
                              c_arraySize(indexKeyList), size);
                }
                os_free(name);
            }
        } else if (!q_isFnc(expr, Q_EXPR_CALLBACK)) {
            q_list l = q_getLst(expr, 0);
            while(l) {
                translate(q_element(l), sourceKeyList, indexKeyList);
                l = q_next(l);
            }
        }
    }
}

#define PRINT_QUERY (0)

v_dataReaderQuery
v_dataReaderQueryNew (
    v_dataReader r,
    const os_char *name,
    const os_char *expression,
    const os_char *params[],
    const os_uint32 nrOfParams,
    const os_uint32 sampleMask)
{
    v_kernel kernel;
    v_dataReaderQuery query,found;
    v_result result;
    os_uint32 i,len;
    q_expr e,subExpr,keyExpr,progExpr;
    q_expr predicate;
    c_iter list;
    c_type type;
    c_array sourceKeyList, indexKeyList;
    c_table instanceSet;
    c_value *values;

    assert(C_TYPECHECK(r,v_dataReader));

    kernel = v_objectKernel(r);

    query = NULL;
    if (!expression) {
        OS_REPORT(OS_ERROR,
                      "DataReader SQL Parser",V_RESULT_ILL_PARAM,
                      "Parse Query expression failed. Query is <NULL>");
    } else {
        predicate = v_parser_parse(expression);
        if (!predicate) {
            OS_REPORT(OS_ERROR,
                        "DataReader SQL Parser",V_RESULT_ILL_PARAM,
                        "Parse Query expression failed. Query: \"%s\"",
                        expression);
        } else {
            query = v_dataReaderQuery(v_objectNew(kernel,K_DATAREADERQUERY));
            result = v_queryInit(v_query(query), v_collection(r), name, expression);
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

    q_prefixFieldNames(&predicate,"sample.message.userData");

#if PRINT_QUERY
    printf("v_datyaReaderQueryNew\n");
    printf("predicate:\n"); q_print(predicate,0); printf("\n");
#endif
    e = q_takePar(predicate,0);

    if (nrOfParams > 0) {
        values = (c_value *)os_malloc(nrOfParams * sizeof(c_value));
        for (i=0; i<nrOfParams; i++) {
            values[i] = c_stringValue((const c_string)params[i]);
        }
    } else {
        values = NULL;
    }

    OSPL_LOCK(r);
    if (!resolveFields(r,e)) {
        c_char *rname;
        if (name == NULL) {
           name = "<NULL>";
        }
        rname = v_entityName(r);
        if (rname == NULL) {
            rname = "<NoName>";
        }
        OS_REPORT(OS_ERROR,
                    "kernel::v_dataReaderQuery::v_dataReaderQueryNew",V_RESULT_ILL_PARAM,
                    "Operation failed: unable to resolve dataReader type fields for query=\"%s\""
                    OS_REPORT_NL "DataReader = \"%s\"",
                    name, rname);
        OSPL_UNLOCK(r);
        q_dispose(e);
        q_dispose(predicate);
        c_free(query);
        v_dataReaderQueryFree(query);
        if (values) {
            os_free(values);
        }
        return NULL;
    }

    /* Normalize the query to the disjunctive form. */
    q_disjunctify(e);
#if PRINT_QUERY
    printf("v_datyaReaderQueryNew\n");
    printf("after disjunctify:\n=============================================\n");
    q_print(e,0);
    printf("\n=============================================\n");
#endif
    e = q_removeNots(e);
#if PRINT_QUERY
    printf("v_datyaReaderQueryNew\n");
    printf("after remove nots:\n"); q_print(e,0); printf("\n");
#endif

    list = q_exprDeOr(e,NULL);

    len = c_iterLength(list);
    type = c_resolve(c_getBase(c_object(kernel)),"c_query");
    query->instanceQ = c_arrayNew(type,len);
    query->sampleQ = c_arrayNew(type,len);
    c_free(type);
    instanceSet = r->index->notEmptyList;
    for (i=0;i<len;i++) {
        subExpr = c_iterTakeFirst(list);
#if PRINT_QUERY
        printf("v_datyaReaderQueryNew\n");
        printf("q_exprDeOr term(%d):\n",i);
        q_print(subExpr,0);
        printf("\n\n");
#endif
        assert(subExpr != NULL);

        sourceKeyList = v_dataReaderSourceKeyList(r);
        indexKeyList = v_dataReaderKeyList(r);

        keyExpr = q_takeKey(&subExpr, sourceKeyList);

        if (keyExpr != NULL) {
            translate(keyExpr, sourceKeyList, indexKeyList);
            assert(keyExpr);
        }

        c_free(sourceKeyList);
        c_free(indexKeyList);
        if (keyExpr != NULL) {
#if PRINT_QUERY
            printf("keyExpr[%d]: ",i);
            q_print(keyExpr,12);
            printf("\n");
#endif
            progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
            query->instanceQ[i] = c_queryNew(instanceSet,
                                             progExpr,values);

            q_dispose(progExpr);
            if (query->instanceQ[i] == NULL) {
                OSPL_UNLOCK(r);
                v_queryFree(v_query(query));
                c_free(query);
                c_iterFree(list);
                os_free(values);
                if (name) {
                    OS_REPORT(OS_ERROR,
                                "v_dataReaderQueryNew failed",V_RESULT_ILL_PARAM,
                                "error in expression: %s",name);
                } else {
                    OS_REPORT(OS_ERROR,
                              "v_dataReaderQueryNew failed",V_RESULT_ILL_PARAM,
                              "error in expression");
                }
                return NULL;
            }
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
/* The following code generates the intermediate non-key query code.
 * Unfortunately c_queryNew creates the query expression relative to the
 * given collection's element type. In this case the instance type.
 * This means that to perform the query evaluation on each sample within
 * an instance the sample must be swapped with the instance sample field and
 * re-swapped after the evaluation.
 */
            progExpr = F1(Q_EXPR_PROGRAM,subExpr);
            query->sampleQ[i] = c_queryNew(instanceSet,
                                           progExpr,
                                           values);
            q_dispose(progExpr);
            if (query->sampleQ[i] == NULL) {
                OSPL_UNLOCK(r);
                v_queryFree(v_query(query));
                c_free(query);
                c_iterFree(list);
                q_dispose(predicate);
                os_free(values);
                if (name) {
                    OS_REPORT(OS_ERROR,
                                "v_dataReaderQueryNew failed",V_RESULT_ILL_PARAM,
                                "error in expression: %s",name);
                } else {
                    OS_REPORT(OS_ERROR,
                              "v_dataReaderQueryNew failed",V_RESULT_ILL_PARAM,
                              "error in expression");
                }
                return NULL;
            }
        } else {
#if PRINT_QUERY
            printf("subExpr[%d]: <NULL>\n",i);
#endif
            query->sampleQ[i] = NULL;
        }
    }
    c_iterFree(list);
    q_dispose(predicate);

    found = c_setInsert(v_collection(r)->queries,query);
    assert(found == query);
    OS_UNUSED_ARG(found);

    OSPL_UNLOCK(r);
    if (values) {
        os_free(values);
    }

#if PRINT_QUERY
    printf("End v_dataReaderQueryNew\n\n");
#endif

    return query;
}

void
v_dataReaderQueryFree (
    v_dataReaderQuery _this)
{
    v_collection src;
    v_dataReader r;
    v_dataReaderQuery drQ;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            drQ = v_dataReaderQuery(_this);
            OSPL_LOCK(r);
            if (drQ->triggerValue) {
                v_dataReaderTriggerValueFree(drQ->triggerValue);
                drQ->triggerValue = NULL;
            }
            OSPL_UNLOCK(r);
            v_queryFree(v_query(_this));
        } else {
            OS_REPORT(OS_ERROR, "v_dataReaderQueryFree failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        OS_REPORT(OS_ERROR, "v_dataReaderQueryFree failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
}

void
v_dataReaderQueryDeinit (
    v_dataReaderQuery _this)
{
    v_collection src;
    v_dataReader r;
    v_dataReaderQuery found;

    if (_this != NULL) {
        assert(C_TYPECHECK(_this,v_dataReaderQuery));

        src = v_querySource(v_query(_this));
        if (src != NULL) {
            assert(v_objectKind(src) == K_DATAREADER);
            if (v_objectKind(src) == K_DATAREADER) {
                r = v_dataReader(src);
                OSPL_LOCK(r);
                found = c_setRemove(v_collection(r)->queries,_this,NULL,NULL);
                if (found != NULL) {
                    assert(_this == found);
                    /* Free the query found because it has been removed
                     * from the queries-collection
                     */
                    c_free(found);
                    v_queryDeinit(v_query(_this));
                }
                OSPL_UNLOCK(r);
            } else {
                OS_REPORT(OS_ERROR, "v_dataReaderQueryDeinit failed", V_RESULT_ILL_PARAM,
                          "source is not datareader");
            }
            c_free(src);
        } else {
            OS_REPORT(OS_ERROR, "v_dataReaderQueryDeinit failed", V_RESULT_ILL_PARAM,
                      "no source");
        }
    }
}

c_bool
v_dataReaderQueryTestSample(
    v_dataReaderQuery _this,
    v_dataReaderSample sample)
{
    c_bool pass = FALSE;
    c_ulong len, i;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

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
    v_dataReaderInstance inst = v_dataReaderInstance(o);
    testActionArg a = (testActionArg)arg;

    a->result = v_dataReaderInstanceTest(inst,a->query, a->sampleMask, a->action, a->args);
    return (!a->result);
}


c_bool
v_dataReaderQueryTest(
    v_dataReaderQuery _this,
    v_queryAction action,
    c_voidp args)
{
    v_collection src;
    v_dataReader r;
    c_ulong len, i;
    C_STRUCT(testActionArg) argument;
    c_table instanceSet;
    c_bool pass = FALSE;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    argument.result = FALSE;
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER)
        {
            r = v_dataReader(src);
            OSPL_LOCK(r);
            instanceSet = r->index->notEmptyList;
            if (c_tableCount(instanceSet) > 0)
            {
                if (_this->triggerValue)
                {
                    v_dataReaderInstance instance;
                    instance = v_dataReaderInstance(v_readerSample(_this->triggerValue)->instance);

                    /* The trigger value still correctly represents the query's trigger state
                     * if it still belongs to its former instance, and if it is not an invalid
                     * sample that is now masked by another, valid, sample.
                     */
                    if (v_dataReaderInstanceContainsSample(instance, _this->triggerValue))
                    {
                        if (!v_readerSampleTestState(_this->triggerValue, L_VALIDDATA) &&
                                hasValidSampleAccessible(instance))
                        {
                            /* The triggerValie is an invalid sample, but it is masked
                             * by a valid sample. That means the query cannot be applied
                             * to the triggerValue itself, so reset the cached sample and
                             * set walkRequired to true, as to re-evaluate the valid sample
                             * in a subsequent pass.
                             */
                            v_dataReaderTriggerValueFree(_this->triggerValue);
                            _this->triggerValue = NULL;
                            _this->walkRequired = TRUE;
                        }
                        else
                        {
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
                                    if (pass && (_this->sampleQ[i] != NULL) && (v_readerSampleTestState(_this->triggerValue, L_VALIDDATA))) {
                                        v_dataReaderSample newest;
                                        newest = v_dataReaderInstanceNewest(instance);
                                        if (_this->triggerValue != newest) {
                                            v_dataReaderInstanceSetNewest(instance,_this->triggerValue);
                                        }
                                        pass = c_queryEval(_this->sampleQ[i],instance);
                                        if (_this->triggerValue != newest) {
                                            v_dataReaderInstanceSetNewest(instance,newest);
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
                                v_dataReaderTriggerValueFree(_this->triggerValue);
                                _this->triggerValue = NULL;
                            }
                        }
                    }
                    else
                    {
                        /* When the triggerValue was an invalid sample, then it could
                         * be that it was a invalid dispose sample that was just replaced
                         * by an unregister sample. Set walk reaquired to true to catch
                         * those situations.
                         */
                        if(!v_readerSampleTestState(_this->triggerValue, L_VALIDDATA)) {
                            _this->walkRequired = TRUE;
                        }
                        /* The trigger value is no longer available in the DataReader.
                         * It can therefore be reset.
                         */
                        v_dataReaderTriggerValueFree(_this->triggerValue);
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
                            c_readAction(_this->instanceQ[i],
                                         testAction, &argument);
                        } else {
                            c_readAction(instanceSet, testAction, &argument);
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
            OSPL_UNLOCK(r);
        } else {
            OS_REPORT(OS_CRITICAL,
                      "v_dataReaderQueryTest failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        OS_REPORT(OS_CRITICAL,
                  "v_dataReaderQueryTest failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
    return pass;
}

C_STRUCT(sampleActionArg) {
    v_dataReader reader;
    c_query query;
    v_readerSampleAction action;
    c_voidp arg;
    c_iter emptyList;
    v_state sampleMask;
    c_long count;
};
C_CLASS(sampleActionArg);


/* Read functions */

static v_actionResult
instanceSampleAction(
    c_object sample,
    c_voidp arg)
{
    sampleActionArg a = (sampleActionArg)arg;
    a->count++;
    return a->action(sample,a->arg);
}

static c_bool
instanceReadSamples(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    sampleActionArg a = (sampleActionArg)arg;
    c_bool proceed = TRUE;

    if (!v_dataReaderInstanceEmpty(instance)) {
        proceed = v_dataReaderInstanceReadSamples(instance,
                                                  a->query,
                                                  a->sampleMask,
                                                  instanceSampleAction,
                                                  arg);
    } else {
        if (!c_iterContains(a->emptyList, instance)) {
             a->emptyList = c_iterInsert(a->emptyList,instance);
        }
    }
    return proceed;
}

static v_result
v__dataReaderQueryOrderedReadOrTake(
    v_dataReaderQuery _this,
    v__dataReaderAction readOrTake,
    v_readerSampleAction action,
    c_voidp argument)
{
    v_result result = V_RESULT_OK;
    v_actionResult proceed = V_PROCEED;
    v_dataReader source, reader;
    v_dataReaderInstance instance;
    v_dataReaderSample bookmark, first, head, sample;
    c_ulong count, length;

    assert (_this != NULL && C_TYPECHECK (_this, v_dataReaderQuery));
    assert (action != NULL);
    assert (argument != NULL);

    source = v_dataReader(v_query(_this)->source);
    assert (source != NULL && C_TYPECHECK (source, v_dataReader));

    length = c_arraySize (_this->instanceQ);
    first = v_orderedInstanceFirstSample (source->orderedInstance);
    sample = bookmark = v_orderedInstanceReadSample (
        source->orderedInstance, _this->sampleMask);
    while (v_actionResultTest (proceed, V_PROCEED) && sample != NULL) {
        instance = v_dataReaderSampleInstance (sample);
        reader = v_dataReaderInstanceReader (instance);

        if (reader != source) {
            v_orderedInstanceUnaligned (source->orderedInstance);
            v_actionResultClear (proceed, V_PROCEED);
            result = V_RESULT_PRECONDITION_NOT_MET;
        } else {
            if (v_sampleMaskPass (_this->sampleMask, sample)) {
                v_actionResultSet (proceed, V_SKIP);

                head = v_dataReaderInstanceNewest (instance);
                if (sample != head) {
                    v_dataReaderInstanceSetNewest (instance, sample);
                }

                for (count = 0;
                     count < length && v_actionResultTest (proceed, V_SKIP);
                     count++)
                {
                    if ((_this->sampleQ[count] == NULL ||
                             c_queryEval (_this->sampleQ[count], instance)) &&
                        (_this->instanceQ[count] == NULL ||
                             c_queryEval (_this->instanceQ[count], instance)))
                    {
                        v_actionResultClear (proceed, V_SKIP);
                    }
                }

                if (sample != head) {
                    v_dataReaderInstanceSetNewest (instance, head);
                }

                if (v_actionResultTest (proceed, V_SKIP)) {
                    /* The trigger_value no longer satisfies the Query and can
                       therefore be reset. */
                    if (_this->triggerValue == sample) {
                        v_dataReaderTriggerValueFree(_this->triggerValue);
                            _this->triggerValue = NULL;
                    }
                } else {
                    proceed = readOrTake (sample, action, argument);
                }
            }

            if (v_actionResultTest (proceed, V_PROCEED)) {
                if (v_readerAccessScope(source) != V_PRESENTATION_GROUP) {
                    sample = v_orderedInstanceReadSample (
                        source->orderedInstance, _this->sampleMask);
                } else {
                    v_actionResultClear (proceed, V_PROCEED);
                }
            }
        }
    }

    if (sample == NULL && bookmark == first) {
        v_orderedInstanceReset (source->orderedInstance);
    }

    return result;
}

static v_result
waitForData(
    v_dataReaderQuery _this,
    os_duration *delay)
{
    v_result result = V_RESULT_OK;
    /* If no data read then wait for data or timeout.
     */
    if (*delay > 0) {
        c_ulong flags = 0;
        os_timeE time = os_timeEGet();
        v_observerSetEvent(v_observer(_this), V_EVENT_DATA_AVAILABLE);
        flags = OSPL_CATCH_EVENT(_this, *delay);
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
v_dataReaderQueryRead (
    v_dataReaderQuery _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    C_STRUCT(sampleActionArg) argument;
    c_table instanceSet;
    c_ulong i,len;
    c_bool unordered = TRUE;

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        if (v_objectKind(src) == K_DATAREADER) {

            r = v_dataReader(src);

            OSPL_LOCK(r);
            if (v_readerSubscriber(r) == NULL) {
                OSPL_UNLOCK(r);
                return V_RESULT_ALREADY_DELETED;
            }
            result = v_dataReaderAccessTest(r);
            if (result == V_RESULT_OK) {
                r->readCnt++;

                if (v_readerAccessScope(r) != V_PRESENTATION_GROUP) {
                    v_dataReaderUpdatePurgeLists(r);
                }
                if (v_orderedInstanceIsAligned (r->orderedInstance)) {
                    result = v__dataReaderQueryOrderedReadOrTake (
                        _this, &v_dataReaderSampleRead, action, arg);
                    if (result == V_RESULT_PRECONDITION_NOT_MET) {
                        result = V_RESULT_OK;
                    } else {
                        unordered = FALSE;
                    }
                }
                if (unordered) {
                    argument.action = action;
                    argument.arg = arg;
                    argument.reader = r;
                    argument.query = NULL;
                    argument.emptyList = NULL;
                    argument.sampleMask = _this->sampleMask;
                    argument.count = 0;

                    while ((argument.count == 0) && (result == V_RESULT_OK))
                    {
                        if (_this->walkRequired == FALSE) {
                            if (_this->triggerValue != NULL) {
                                instanceSet = r->index->notEmptyList;
                                if (c_tableCount(instanceSet) > 0) {
                                    v_dataReaderInstance instance;
                                    c_bool pass = FALSE;
                                    instance = v_dataReaderInstance(v_readerSample(_this->triggerValue)->instance);
                                    if (v_dataReaderInstanceContainsSample(instance, _this->triggerValue)) {

                                        /* This part should be moved to the notify method
                                         * as part of the producer query evaluation.
                                         */
                                        if (v_sampleMaskPass(_this->sampleMask, _this->triggerValue)) {
                                            len = c_arraySize(_this->instanceQ);
                                            for (i=0;(i<len) && !pass;i++) {
                                                pass = TRUE;
                                                if (_this->instanceQ[i] != NULL) {
                                                    pass = c_queryEval(_this->instanceQ[i],instance);
                                                }
                                                if (pass && (_this->sampleQ[i] != NULL) && (v_readerSampleTestState(_this->triggerValue, L_VALIDDATA))) {
                                                    v_dataReaderSample newest;
                                                    newest = v_dataReaderInstanceNewest(instance);
                                                    if (_this->triggerValue != newest) {
                                                        v_dataReaderInstanceSetNewest(instance,_this->triggerValue);
                                                    }
                                                    pass = c_queryEval(_this->sampleQ[i],instance);
                                                    if (_this->triggerValue != newest) {
                                                        v_dataReaderInstanceSetNewest(instance,newest);
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (pass) {
                                        if (instance->historySampleCount == 0) {
                                            /* No valid samples exist,
                                             * so there must be one invalid sample.
                                             * Dcps-Spec. demands a Desctructive read -> v_dataReaderSampleTake()
                                             * TODO: Leave invalid sample as is.
                                             */
                                            assert(v_dataReaderInstanceStateTest(instance, L_STATECHANGED));
                                            (void) v_dataReaderSampleTake(_this->triggerValue,instanceSampleAction,&argument);
                                            assert(!v_dataReaderInstanceStateTest(instance, L_STATECHANGED));
                                        } else {
                                            (void) v_dataReaderSampleRead(_this->triggerValue, action,arg);
                                        }
                                    } else {
                                        /* The trigger_value no longer satisfies the Query.
                                         * It can therefore be reset.
                                         */
                                        v_dataReaderTriggerValueFree(_this->triggerValue);
                                        _this->triggerValue = NULL;
                                    }
                                }
                            }
                            proceed = FALSE;
                        } else {
                            instanceSet = r->index->notEmptyList;
                            len = c_arraySize(_this->instanceQ);
                            for (i=0;(i<len) && proceed;i++) {
                                argument.query = _this->sampleQ[i];
                                if (_this->instanceQ[i] != NULL) {
                                    proceed = c_walk(_this->instanceQ[i],
                                                     (c_action)instanceReadSamples,
                                                     &argument);
                                } else {
                                    proceed = c_readAction(instanceSet,
                                                           (c_action)instanceReadSamples,
                                                           &argument);
                                }
                            }
                            if (argument.emptyList != NULL) {
                                v_dataReaderInstance emptyInstance;

                                emptyInstance = c_iterTakeFirst(argument.emptyList);
                                while (emptyInstance != NULL) {
                                    v_dataReaderRemoveInstance(r,emptyInstance);
                                    emptyInstance = c_iterTakeFirst(argument.emptyList);
                                }
                                c_iterFree(argument.emptyList);
                                if (r->statistics) {
                                    r->statistics->numberOfInstances = v_dataReaderInstanceCount_nl(r);
                                }
                            }
                        }
                        if (argument.count == 0) {
                            result = waitForData(_this, &timeout);
                        }
                    }
                }
                if (_this->_parent.statistics) {
                    _this->_parent.statistics->numberOfReads++;
                }

                action(NULL,arg); /* This triggers the action routine that
                                   * the last sample is read. */

                if (!proceed) {
                    _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
                }
            }
            OSPL_UNLOCK(r);
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_CRITICAL,
                      "v_dataReaderQueryRead failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
            assert(v_objectKind(src) == K_DATAREADER);
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_CRITICAL,
                  "v_dataReaderQueryRead failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
    return result;
}

v_result
v_dataReaderQueryReadInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    c_ulong i, len;

    if (instance == NULL) {
        /* Should fall within a lock on _this */
        if (_this->_parent.statistics) {
            _this->_parent.statistics->numberOfInstanceReads++;
        }
        return V_RESULT_ILL_PARAM;
    }
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            OSPL_LOCK(r);
            if (v_readerSubscriber(r) == NULL) {
                OSPL_UNLOCK(r);
                return V_RESULT_ALREADY_DELETED;
            }
            result = v_dataReaderAccessTest(r);
            if (result == V_RESULT_OK) {
                C_STRUCT(sampleActionArg) argument;
                v_orderedInstanceUnaligned (r->orderedInstance);
                r->readCnt++;
                v_dataReaderUpdatePurgeLists(r);

                argument.action = action;
                argument.arg = arg;
                argument.reader = r;
                argument.query = NULL;
                argument.emptyList = NULL;
                argument.sampleMask = _this->sampleMask;
                argument.count = 0;

                while ((argument.count == 0) && (result == V_RESULT_OK))
                {
                    if (v_dataReaderInstanceEmpty(instance)) {
                        action(NULL,arg); /* This triggers the action routine that
                                           * the last sample is read. */
                        v_dataReaderRemoveInstance(r,instance);
                    } else {
                        len = c_arraySize(_this->instanceQ);
                        i=0;
                        while ((i<len) && proceed) {
                            if (_this->instanceQ[i] != NULL) {
                                if (c_queryEval(_this->instanceQ[i],instance)) {
                                    proceed = v_dataReaderInstanceReadSamples(
                                                      instance,
                                                      _this->sampleQ[i],
                                                      _this->sampleMask,
                                                      instanceSampleAction,&argument);
                                }
                            } else {
                                proceed = v_dataReaderInstanceReadSamples(
                                                  instance,
                                                  _this->sampleQ[i],
                                                  _this->sampleMask,
                                                  instanceSampleAction,&argument);
                            }
                            i++;
                        }
                    }
                    if (argument.count == 0) {
                        result = waitForData(_this, &timeout);
                    }
                }
                action(NULL,arg); /* This triggers the action routine that
                                   * the last sample is read. */

                if (!proceed) {
                   _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
                }
            }
            OSPL_UNLOCK(r);
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_CRITICAL,
                      "v_dataReaderQueryReadInstance failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_CRITICAL,
                  "v_dataReaderQueryReadInstance failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
    /* Should fall within a lock on _this */
    if (_this->_parent.statistics) {
        _this->_parent.statistics->numberOfInstanceReads++;
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
v_dataReaderQueryReadNextInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    c_bool fromStart = (instance == NULL);
    v_collection src;
    v_dataReader r;
    c_ulong i,len;
    v_dataReaderInstance nextInstance, cur;
    struct nextInstanceActionArg a;

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            OSPL_LOCK(r);
            if (v_readerSubscriber(r) == NULL) {
                OSPL_UNLOCK(r);
                return V_RESULT_ALREADY_DELETED;
            }
            result = v_dataReaderAccessTest(r);
            if (result == V_RESULT_OK) {
                C_STRUCT(sampleActionArg) argument;
                v_orderedInstanceUnaligned (r->orderedInstance);

                r->readCnt++;
                v_dataReaderUpdatePurgeLists(r);

                a.action = action;
                a.arg = arg;
                a.hasData = FALSE;

                argument.action = nextInstanceAction;
                argument.arg = &a;
                argument.reader = r;
                argument.query = NULL;
                argument.emptyList = NULL;
                argument.sampleMask = _this->sampleMask;
                argument.count = 0;

                while ((argument.count == 0) && (result == V_RESULT_OK))
                {
                    if (_this->walkRequired || (_this->triggerValue != NULL)) {
                        c_bool proceed = TRUE;

                        len = c_arraySize(_this->instanceQ);
                        nextInstance = v_dataReaderNextInstance(r,instance);

                        while ((nextInstance != NULL) && (a.hasData == FALSE)){
                            i=0;
                            if (v_dataReaderInstanceEmpty(nextInstance)) {
                                cur = nextInstance;
                                nextInstance = v_dataReaderNextInstance(r,nextInstance);
                                v_dataReaderRemoveInstance(r,cur);
                            } else {
                                while ((i<len) && proceed) {
                                    if (_this->instanceQ[i] != NULL) {
                                        if (c_queryEval(_this->instanceQ[i],nextInstance)) {
                                            proceed = v_dataReaderInstanceReadSamples(
                                                    nextInstance,
                                                    _this->sampleQ[i],
                                                    _this->sampleMask,
                                                    instanceSampleAction,
                                                    &argument);
                                        }
                                    } else {
                                        proceed = v_dataReaderInstanceReadSamples(
                                                nextInstance,
                                                _this->sampleQ[i],
                                                _this->sampleMask,
                                                instanceSampleAction,
                                                &argument);
                                    }
                                    i++;
                                }
                                nextInstance = v_dataReaderNextInstance(r,nextInstance);
                            }
                        }
                        /* When no samples matching the query are found (a,hasData == FALSE)
                         * and the complete tree has been evaluated (fromStart == TRUE && nextInstance == NULL),
                         * then the walkRequired and triggerValue properties of the query can be cleared.
                         */
                        if (proceed && fromStart && (nextInstance == NULL) && !a.hasData) {
                            _this->walkRequired = FALSE;
                            if (_this->triggerValue) {
                                v_dataReaderTriggerValueFree(_this->triggerValue);
                                _this->triggerValue = NULL;
                            }
                            _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
                        }
                    }
                    if (argument.count == 0) {
                        result = waitForData(_this, &timeout);
                    }
                }
                action(NULL, arg); /* This triggers the action routine that the last sample is read. */
            }
            OSPL_UNLOCK(r);
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_CRITICAL, "v_dataReaderQueryReadNextInstance failed", V_RESULT_ILL_PARAM,
                    "source is not datareader");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_CRITICAL,
                  "v_dataReaderQueryReadNextInstance failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
    /* Should fall within a lock on _this */
    if (_this->_parent.statistics) {
        _this->_parent.statistics->numberOfNextInstanceReads++;
    }
    return result;
}

static c_bool
instanceTakeSamples(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    sampleActionArg a = (sampleActionArg)arg;
#ifndef NDEBUG
    c_long count, oldCount;
#endif
    assert(C_TYPECHECK(a->reader, v_dataReader));
    assert(v_dataReader(a->reader)->resourceSampleCount >= 0);

    if (v_dataReaderInstanceEmpty(instance)) {
        if (!c_iterContains(a->emptyList, instance)) {
             a->emptyList = c_iterInsert(a->emptyList,instance);
        }
        return proceed;
    }
#ifndef NDEBUG
    oldCount = v_dataReaderInstanceSampleCount(instance);
    assert(oldCount >= 0);
#endif
    proceed = v_dataReaderInstanceTakeSamples(instance,
                                              a->query,
                                              a->sampleMask,
                                              instanceSampleAction,
                                              arg);
#ifndef NDEBUG
    count = oldCount - v_dataReaderInstanceSampleCount(instance);
    assert(count >= 0);
#endif
    assert(v_dataReader(a->reader)->resourceSampleCount >= 0);

    if (a->reader->statistics) {
        a->reader->statistics->numberOfSamples = (c_ulong) a->reader->resourceSampleCount;
    }
#if 1 /* This snippet of code exists to avoid leakage.
       * This code can be deleted as soon as active garbage collection
       * is implemented (scdds1817)
       */
    if (v_dataReaderInstanceEmpty(instance)) {
        if (!c_iterContains(a->emptyList, instance)) {
             a->emptyList = c_iterInsert(a->emptyList,instance);
        }
    }
#endif
    return proceed;
}

v_result
v_dataReaderQueryTake(
    v_dataReaderQuery _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    c_table instanceSet;
    c_ulong len, i;
    C_STRUCT(sampleActionArg) argument;
    v_dataReaderInstance instance, emptyInstance;
    c_bool unordered = TRUE;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);

            OSPL_LOCK(r);
            if (v_readerSubscriber(r) == NULL) {
                OSPL_UNLOCK(r);
                return V_RESULT_ALREADY_DELETED;
            }
            result = v_dataReaderAccessTest(r);
            if (result == V_RESULT_OK) {
                r->readCnt++;

                if (v_readerAccessScope(r) != V_PRESENTATION_GROUP) {
                    v_dataReaderUpdatePurgeLists(r);
                }
                if (v_orderedInstanceIsAligned (r->orderedInstance)) {
                    result = v__dataReaderQueryOrderedReadOrTake (
                        _this, &v_dataReaderSampleTake, action, arg);
                    if (result == V_RESULT_PRECONDITION_NOT_MET) {
                        result = V_RESULT_OK;
                    } else {
                        unordered = FALSE;
                    }
                }
                if (unordered) {
                    argument.action = action;
                    argument.arg = arg;
                    argument.reader = r;
                    argument.emptyList = NULL;
                    argument.query = NULL;
                    argument.sampleMask = _this->sampleMask;
                    argument.count = 0;

                    while ((argument.count == 0) && (result == V_RESULT_OK))
                    {
                        if (_this->walkRequired == FALSE) {
                            if (_this->triggerValue != NULL) {
                                instanceSet = r->index->notEmptyList;
                                if (c_tableCount(instanceSet) > 0) {
                                    c_bool pass = FALSE;
                                    instance = v_dataReaderInstance(v_readerSample(_this->triggerValue)->instance);
                                    if (v_dataReaderInstanceContainsSample(instance, _this->triggerValue)) {

                                        /* This part should be moved to the notify method
                                         * as part of the producer query evaluation.
                                         */
                                        if (v_sampleMaskPass(_this->sampleMask, _this->triggerValue)) {
                                            len = c_arraySize(_this->instanceQ);
                                            for (i=0;(i<len) && !pass;i++) {
                                                pass = TRUE;
                                                if (_this->instanceQ[i] != NULL) {
                                                    pass = c_queryEval(_this->instanceQ[i],instance);
                                                }
                                                if (pass && (_this->sampleQ[i] != NULL) && (v_readerSampleTestState(_this->triggerValue, L_VALIDDATA))) {
                                                    v_dataReaderSample newest;
                                                    newest = v_dataReaderInstanceNewest(instance);
                                                    if (_this->triggerValue != newest) {
                                                        v_dataReaderInstanceSetNewest(instance,_this->triggerValue);
                                                    }
                                                    pass = c_queryEval(_this->sampleQ[i],instance);
                                                    if (_this->triggerValue != newest) {
                                                        v_dataReaderInstanceSetNewest(instance,newest);
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (pass) {
                                        v_dataReaderSampleTake(_this->triggerValue,instanceSampleAction,&argument);
                                        if (v_dataReaderInstanceEmpty(instance)) {
                                            v_dataReaderRemoveInstance(r,instance);
                                        }
                                    }
                                    /* The trigger_value no longer satisfies the Query or
                                     * has been taken. It can therefore be reset.
                                     */
                                    v_dataReaderTriggerValueFree(_this->triggerValue);
                                    _this->triggerValue = NULL;
                                }
                            }
                            proceed = FALSE;
                        } else {
                            instanceSet = r->index->notEmptyList;
                            if (c_tableCount(instanceSet) > 0) {

                                len = c_arraySize(_this->instanceQ);
                                for (i=0;(i<len) && proceed;i++) {
                                    argument.query = _this->sampleQ[i];
                                    if (_this->instanceQ[i] != NULL) {
                                        proceed = c_walk(_this->instanceQ[i],
                                                         (c_action)instanceTakeSamples,
                                                         &argument);
                                    } else {
                                        proceed = c_readAction(instanceSet,
                                                               (c_action)instanceTakeSamples,
                                                               &argument);
                                    }
                                }
                                if (argument.emptyList != NULL) {
                                    emptyInstance = c_iterTakeFirst(argument.emptyList);
                                    while (emptyInstance != NULL) {
                                        v_dataReaderRemoveInstance(r,emptyInstance);
                                        emptyInstance = c_iterTakeFirst(argument.emptyList);
                                    }
                                    c_iterFree(argument.emptyList);
                                    if (r->statistics) {
                                        r->statistics->numberOfInstances = v_dataReaderInstanceCount_nl(r);
                                    }
                                }
                            }
                        }
                        if (argument.count == 0) {
                            result = waitForData(_this, &timeout);
                        }
                    }
                }
                if (_this->_parent.statistics) {
                    _this->_parent.statistics->numberOfTakes++;
                }

                if (r->resourceSampleCount == 0) {
                    v_statusReset(v_entity(r)->status,V_EVENT_DATA_AVAILABLE);
                }
                action(NULL,arg); /* This triggers the action routine that
                                   * the last sample is read. */

                if (!proceed) {
                    _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
                }
            }
            OSPL_UNLOCK(r);
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_CRITICAL,
                      "v_dataReaderQueryTake failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_CRITICAL,
                  "v_dataReaderQueryTake failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
    return result;
}

v_result
v_dataReaderQueryTakeInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    c_ulong i,len;
    c_long count;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    if (instance == NULL) {
        /* Should fall within a lock on _this */
        if (_this->_parent.statistics) {
            _this->_parent.statistics->numberOfInstanceTakes++;
        }
        return V_RESULT_ILL_PARAM;
    }
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            OSPL_LOCK(r);
            if (v_readerSubscriber(r) == NULL) {
                OSPL_UNLOCK(r);
                return V_RESULT_ALREADY_DELETED;
            }
            result = v_dataReaderAccessTest(r);
            if (result == V_RESULT_OK) {
                C_STRUCT(sampleActionArg) argument;
                v_orderedInstanceUnaligned (r->orderedInstance);

                r->readCnt++;
                v_dataReaderUpdatePurgeLists(r);

                argument.action = action;
                argument.arg = arg;
                argument.reader = r;
                argument.query = NULL;
                argument.emptyList = NULL;
                argument.sampleMask = _this->sampleMask;
                argument.count = 0;

                len = c_arraySize(_this->instanceQ);
                while ((argument.count == 0) && (result == V_RESULT_OK))
                {
                    i=0;
                    while ((i<len) && proceed) {
                        count = v_dataReaderInstanceSampleCount(instance);
                        if (_this->instanceQ[i] != NULL) {
                            if (c_queryEval(_this->instanceQ[i],instance)) {
                                proceed = v_dataReaderInstanceTakeSamples(
                                                       instance,
                                                       _this->sampleQ[i],
                                                       _this->sampleMask,
                                                       instanceSampleAction,
                                                       &argument);
                            }
                        } else {
                            proceed = v_dataReaderInstanceTakeSamples(
                                                   instance,
                                                   _this->sampleQ[i],
                                                   _this->sampleMask,
                                                   instanceSampleAction,
                                                   &argument);
                        }
                        count -= v_dataReaderInstanceSampleCount(instance);
                        assert(count >= 0);
                        assert(r->resourceSampleCount >= 0);
                        if (r->statistics) {
                            r->statistics->numberOfSamples = (c_ulong) r->resourceSampleCount;
                        }
                        i++;
                    }
                    if (v_dataReaderInstanceEmpty(instance)) {
                        v_dataReaderRemoveInstance(r,instance);
                    }
                    if (argument.count == 0) {
                        result = waitForData(_this, &timeout);
                    }
                }
                if (r->resourceSampleCount == 0) {
                    v_statusReset(v_entity(r)->status,V_EVENT_DATA_AVAILABLE);
                }
                if (!proceed) {
                    _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
                }
            }
            OSPL_UNLOCK(r);
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_CRITICAL,
                      "v_dataReaderQueryTakeInstance failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_CRITICAL,
                  "v_dataReaderQueryTakeInstance failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
    action(NULL,arg); /* This triggers the action routine that the last sample is read. */
    /* Should fall within a lock on _this */
    if (_this->_parent.statistics) {
        _this->_parent.statistics->numberOfInstanceTakes++;
    }
    return result;
}

v_result
v_dataReaderQueryTakeNextInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    c_bool proceed = TRUE;
    c_bool fromStart = (instance == NULL);
    v_collection src;
    v_dataReader r;
    c_ulong i,len;
    c_long count;
    v_dataReaderInstance nextInstance;
    struct nextInstanceActionArg a;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            OSPL_LOCK(r);
            if (v_readerSubscriber(r) == NULL) {
                OSPL_UNLOCK(r);
                return V_RESULT_ALREADY_DELETED;
            }
            result = v_dataReaderAccessTest(r);
            if (result == V_RESULT_OK) {
                C_STRUCT(sampleActionArg) argument;
                v_orderedInstanceUnaligned (r->orderedInstance);

                r->readCnt++;
                v_dataReaderUpdatePurgeLists(r);

                a.action = action;
                a.arg = arg;
                a.hasData = FALSE;

                argument.action = nextInstanceAction;
                argument.arg = &a;
                argument.reader = r;
                argument.query = NULL;
                argument.emptyList = NULL;
                argument.sampleMask = _this->sampleMask;
                argument.count = 0;

                while ((argument.count == 0) && (result == V_RESULT_OK))
                {
                    if (_this->walkRequired || (_this->triggerValue != NULL)) {
                        len = c_arraySize(_this->instanceQ);
                        nextInstance = v_dataReaderNextInstance(r,instance);
                        while ((nextInstance != NULL) && (a.hasData == FALSE)) {
                            i=0;
                            while ((i<len) && proceed) {
                                count = v_dataReaderInstanceSampleCount(nextInstance);
                                if (_this->instanceQ[i] != NULL) {
                                    if (c_queryEval(_this->instanceQ[i],nextInstance)) {
                                        proceed = v_dataReaderInstanceTakeSamples(
                                                           nextInstance,
                                                           _this->sampleQ[i],
                                                           _this->sampleMask,
                                                           instanceSampleAction,
                                                           &argument);
                                    }
                                } else {
                                    proceed = v_dataReaderInstanceTakeSamples(
                                                       nextInstance,
                                                       _this->sampleQ[i],
                                                       _this->sampleMask,
                                                       instanceSampleAction,
                                                       &argument);
                                }
                                count -= v_dataReaderInstanceSampleCount(nextInstance);
                                assert(count >= 0);
                                assert(r->resourceSampleCount >= 0);
                                if (r->statistics) {
                                    r->statistics->numberOfSamples = (c_ulong) r->resourceSampleCount;
                                }
                                i++;
                            }
                            if (v_dataReaderInstanceEmpty(nextInstance)) {
                                /* The keep is necessary because the instance is
                                 * removed from the index after this, but might be
                                 * used later in this function to determine the next
                                 * instance in the index.
                                 */
                                instance = c_keep(nextInstance);
                                v_dataReaderRemoveInstance(r,nextInstance);
                            } else {
                                instance = NULL;
                            }

                            /**
                             * Do not determine the next instance if data has been taken.
                             * This saves processing...
                             */
                            if(!(a.hasData)){
                                nextInstance = v_dataReaderNextInstance(r,nextInstance);
                            } else {
                                nextInstance = NULL;
                            }
                            c_free(instance);
                        }
                        if (r->resourceSampleCount == 0) {
                            v_statusReset(v_entity(r)->status,V_EVENT_DATA_AVAILABLE);
                        }
                        /* When no samples matching the query are found (a,hasData == FALSE)
                         * and the complete tree has been evaluated (fromStart == TRUE && nextInstance == NULL),
                         * then the walkRequired and triggerValue properties of the query can be cleared.
                         */
                        if (proceed && fromStart && (nextInstance == NULL) && !a.hasData) {
                            _this->walkRequired = FALSE;
                            if (_this->triggerValue) {
                                v_dataReaderTriggerValueFree(_this->triggerValue);
                                _this->triggerValue = NULL;
                            }
                            _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
                        }
                    }
                    if (argument.count == 0) {
                        result = waitForData(_this, &timeout);
                    }
                }
                action(NULL, arg); /* This triggers the action routine that the last sample is read. */
            }
            OSPL_UNLOCK(r);
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_CRITICAL,
                      "v_dataReaderQueryTakeNextInstance failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_CRITICAL,
                  "v_dataReaderQueryTakeNextInstance failed", V_RESULT_ILL_PARAM,
                  "no source");
    }
    /* Should fall within a lock on _this */
    if (_this->_parent.statistics) {
        _this->_parent.statistics->numberOfNextInstanceTakes++;
    }
    return result;
}

c_bool
v_dataReaderQueryNotifyDataAvailable(
    v_dataReaderQuery _this,
    v_event e)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_dataReaderQuery));
    assert(e);
    assert(C_TYPECHECK(e->data,v_dataReaderSample));

    EVENT_TRACE("v_dataReaderQueryNotifyDataAvailable(_this = 0x%x, event = 0x%x)\n", _this, e);

    OSPL_LOCK(_this);
    /* Only store the trigger value and notify observers if no
     * trigger value is set before.
     * The trigger value is reset when it no longer satisfies the Query.
     * Query Read and Take operations can examine the walkRequired value
     * to decide to use the trigger value instead of executing the query.
     * If a trigger value was already selected, inserting a new sample
     * no longer allows you use just the trigger value: when executing the
     * query you will need to do a full walk.
     */
    if (e->data) {
        if (_this->triggerValue == NULL) {
            _this->triggerValue = v_dataReaderTriggerValueKeep(e->data);
        } else {
            _this->walkRequired = TRUE;
        }
        /* Notify for internal use only, result can be ignored */
        (void)v_entityNotifyListener(v_entity(_this), e);
    } else {
        _this->walkRequired = TRUE;
    }
    _this->state |= V_STATE_DATA_AVAILABLE;
    OSPL_THROW_EVENT(_this, e);
    OSPL_UNLOCK(_this);

    return TRUE;
}

c_bool
v_dataReaderQuerySetParams(
    v_dataReaderQuery _this,
    const os_char *params[],
    const os_uint32 nrOfParams)
{
    v_collection src;
    v_dataReader r;
    v_kernel kernel;
    c_ulong i,len;
    q_expr e,subExpr,keyExpr,progExpr;
    q_expr predicate;
    c_iter list;
    c_type type;
    c_bool result = TRUE;
    c_array keyList;
    c_table instanceSet;
    c_value *values;

  /* first remove the old query */
    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    r = NULL;

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {

            if (nrOfParams > 0) {
                values = (c_value *)os_malloc(nrOfParams * sizeof(c_value));
                for (i=0; i<nrOfParams; i++) {
                    values[i] = c_stringValue((const c_string)params[i]);
                }
            } else {
                values = NULL;
            }

            kernel = v_objectKernel(_this);
            r = v_dataReader(src);

            OSPL_LOCK(r);
            len = c_arraySize(_this->instanceQ);
            /* Try to assign parameter values to all sub-queries.
             * If one or more of the assignments fails then it indicates that
             * optimisations have become invalid due to the change and
             * that the whole query needs to be rebuild.
             */
            for (i=0; (i<len) && (result == TRUE); i++) {
                result = c_querySetParams(_this->instanceQ[i],values) &&
                         c_querySetParams(_this->sampleQ[i],values);
            }
            if (!result) {
                /* One or more of the assignments failed so rebuild the
                 * query from the expression with the new parameter values.
                 */
                predicate = v_queryGetPredicate(v_query(_this));
#if PRINT_QUERY
                printf("v_datyaReaderQuerySetParams\n");
                printf("predicate:\n"); q_print(predicate,0); printf("\n");
#endif

                e = q_takePar(predicate,0);
                if (!resolveFields(r,e)) {
                    OSPL_UNLOCK(r);
                    q_dispose(e);
                    q_dispose(predicate);
                    os_free(values);
                    return FALSE;
                }

                /* Normalize the query to the disjunctive form. */
                q_disjunctify(e);
                e = q_removeNots(e);

                list = q_exprDeOr(e,NULL);

                len = c_iterLength(list);
                type = c_resolve(c_getBase(c_object(kernel)),"c_query");
                c_free(_this->instanceQ);
                c_free(_this->sampleQ);
                _this->instanceQ = c_arrayNew(type,len);
                _this->sampleQ = c_arrayNew(type,len);
                c_free(type);
                instanceSet = r->index->notEmptyList;
                for (i=0;i<len;i++) {
                    subExpr = c_iterTakeFirst(list);
                    assert(subExpr != NULL);
                    keyList = v_dataReaderKeyList(r);
                    keyExpr = q_takeKey(&subExpr, keyList);
                    c_free(keyList);
                    if (keyExpr != NULL) {
#if PRINT_QUERY
                        printf("keyExpr[%d]: ",i);
                        q_print(keyExpr,12);
                        printf("\n");
#endif
                        progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
                        _this->instanceQ[i] = c_queryNew(instanceSet,
                                                         progExpr,values);
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
                        /* The following code generates the intermediate
                         * non-key query code. Unfortunately c_queryNew
                         * creates the query expression relative to the given
                         * collection's element type. In this case the instance
                         * type. This means that to perform the query
                         * evaluation on each sample within an instance the
                         * sample must be swapped with the instance sample
                         * field and re-swapped after the evaluation.
                         */
                        progExpr = F1(Q_EXPR_PROGRAM,subExpr);
                        _this->sampleQ[i] = c_queryNew(instanceSet,
                                                       progExpr,values);
                        q_dispose(progExpr);
                    } else {
#if PRINT_QUERY
                        printf("subExpr[%d]: <NULL>\n",i);
#endif
                        _this->sampleQ[i]   = NULL;
                    }
                }
                c_iterFree(list);
#if PRINT_QUERY
                printf("End v_dataReaderQuerySetParams\n\n");
#endif
                q_dispose(predicate);
            }
            result = TRUE;
            _this->walkRequired = TRUE;

            OSPL_UNLOCK(r);
            if (values) {
                os_free(values);
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQuerySetParams failed", V_RESULT_ILL_PARAM,
                      "source is not datareader");
            result = FALSE;
        }
        c_free(src);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderQuerySetParams failed", V_RESULT_ILL_PARAM,
                  "no source");
        result = FALSE;
    }

    if (result == TRUE) {
        C_STRUCT(v_event) event;

        event.kind = V_EVENT_TRIGGER;
        event.source = v_observable(_this);
        event.data = NULL;
        event.handled = FALSE;

        OSPL_THROW_EVENT(_this, &event);
    }

    return result;
}
