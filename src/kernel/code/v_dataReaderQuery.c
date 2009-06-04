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
#include "v__dataReader.h"
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
#include "v_statisticsHelpers.h"
#include "v__statisticsInterface.h"
#include "v__statistics.h"
#include "q_helper.h"
#include "v__statCat.h"
#include "v__kernel.h"
#include "v_queryStatistics.h"

#include "c_stringSupport.h"

#include "os.h"
#include "os_report.h"

#define V_STATE_INITIAL        (0x00000000U)       /* 0 */
#define V_STATE_ACTIVE         (0x00000001U)       /* 1 */
#define V_STATE_DATA_AVAILABLE (0x00000001U << 1)  /* 2 */

static q_expr
resolveField(
    v_dataReader _this,
    const c_char *name)
{
    c_field field;
    c_array path;
    c_long i, length;
    q_list list;
    c_string str;

    field = v_dataReaderField(_this,name);
    if (field == NULL) {
        return NULL;
    }
    path = c_fieldPath(field);
    length = c_arraySize(path);
    list = NULL;
    for (i=(length-1);i>=0;i--) {
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
                OS_REPORT_1(OS_ERROR,
                            "v_dataReaderQueryNew failed",0,
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
            OS_REPORT_1(OS_ERROR,
                        "v_dataReaderQueryNew failed",0,
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

static c_iter
deOr(
    q_expr e,
    c_iter list)
{
    c_iter results;

    if (q_getTag(e) == Q_EXPR_OR) {
        results = deOr(q_takePar(e,0),deOr(q_takePar(e,0),list));
        q_dispose(e);
    } else {
        results = c_iterInsert(list,e);
    }
    return results;
}

#define PRINT_QUERY (0)

v_dataReaderQuery
v_dataReaderQueryNew (
    v_dataReader r,
    const c_char *name,
    q_expr predicate,
    c_value params[])
{
    v_kernel kernel;
    v_dataReaderQuery query,found;
    v_queryStatistics queryStatistics;
    c_long i,len;
    q_expr e,subExpr,keyExpr,progExpr;
    c_iter list;
    c_type type;
    c_array keyList;
    c_table instanceSet;
    c_char *pr;

    assert(C_TYPECHECK(r,v_dataReader));

    kernel = v_objectKernel(r);
    if (q_getTag(predicate) !=  Q_EXPR_PROGRAM) {
        assert(FALSE);
        return NULL;
    }
#if PRINT_QUERY
    printf("v_datyaReaderQueryNew\n");
    printf("predicate:\n"); q_print(predicate,0); printf("\n");
#endif
    e = q_takePar(predicate,0);
    if (!resolveFields(r,e)) {
        q_dispose(e);
        return NULL;
    }
    v_dataReaderLock(r);
    query = v_dataReaderQuery(v_objectNew(kernel,K_DATAREADERQUERY));

    if (v_isEnabledStatistics(kernel, V_STATCAT_READER)) {
        queryStatistics = v_queryStatisticsNew(kernel);
    } else {
        queryStatistics = NULL;
    }
    v_queryInit(v_query(query), name, v_statistics(queryStatistics),
                v_collection(r), predicate, params);
    pr = q_exprGetText(predicate);
    query->expression   = c_stringNew(c_getBase(r), pr);
    os_free(pr);
    query->params       = NULL;
    query->instanceMask = q_exprGetInstanceState(predicate);
    query->sampleMask   = q_exprGetSampleState(predicate);
    query->viewMask     = q_exprGetViewState(predicate);
    query->triggerValue = NULL;
    query->updateCnt    = 0;

    /* Normilize the query to the disjunctive form. */
    q_disjunctify(e);
#if PRINT_QUERY
    printf("v_datyaReaderQueryNew\n");
    printf("after disjunctify:\n"); q_print(e,0); printf("\n");
#endif
    e = q_removeNots(e);
#if PRINT_QUERY
    printf("v_datyaReaderQueryNew\n");
    printf("after remove nots:\n"); q_print(e,0); printf("\n");
#endif

    list = deOr(e,NULL);

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
        printf("deOr term(%d):\n",i);
        q_print(subExpr,0);
        printf("\n");
#endif
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
            query->instanceQ[i] = c_queryNew(instanceSet,
                                             progExpr,params);
            q_dispose(progExpr);
            if (query->instanceQ[i] == NULL) {
                v_dataReaderUnLock(r);
                v_queryFree(v_query(query));
                c_iterFree(list);
                if (name) {
                    OS_REPORT_1(OS_ERROR,
                                "v_dataReaderQueryNew failed",0,
                                "error in expression: %s",name);
                } else {
                    OS_REPORT(OS_ERROR,
                              "v_dataReaderQueryNew failed",0,
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
/* The following code generates the intermeadiate non-key query code.
   Unfortunatly c_queryNew creates the query expression relative to the
   given collection's element type. In this case the instance type.
   This means that to performe the query evaluation on each sample within
   an instance the sample must be swapped with the instance sample field and
   reswapped after the evaluation.
*/
            progExpr = F1(Q_EXPR_PROGRAM,subExpr);
            query->sampleQ[i] = c_queryNew(instanceSet,
                                           progExpr,
                                           params);
            q_dispose(progExpr);
            if (query->sampleQ[i] == NULL) {
                v_dataReaderUnLock(r);
                v_queryFree(v_query(query));
                c_iterFree(list);
                if (name) {
                    OS_REPORT_1(OS_ERROR,
                                "v_dataReaderQueryNew failed",0,
                                "error in expression: %s",name);
                } else {
                    OS_REPORT(OS_ERROR,
                              "v_dataReaderQueryNew failed",0,
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
    found = c_setInsert(v_collection(r)->queries,query);
    assert(found == query);

    if (params) {
        c_long size, strSize, curSize, exprSize, count;
        c_char *tmp, *paramString;
        c_char character, prevCharacter;
        c_char number[32];
        c_bool inNumber;

        exprSize = strlen(query->expression);
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
            character = query->expression[i];

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

        if (size > 0) {
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
                strcat(paramString, tmp);
                os_free(tmp);

                if(i+1 != size){
                    strcat(paramString, ",");
                }
            }
            query->params = c_stringNew(c_getBase(r), paramString);
            os_free(paramString);
        } else {
            query->params = NULL;
        }
    }
    v_observerUnlock(v_observer(r));
#if PRINT_QUERY
    printf("End v_dataReaderQueryNew\n\n");
#endif

    return query;
}

void
v_dataReaderQueryFree (
    v_dataReaderQuery _this)
{
    assert(C_TYPECHECK(_this,v_dataReaderQuery));
    v_queryFree(v_query(_this));
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
                v_dataReaderLock(r);
                found = c_setRemove(v_collection(r)->queries,_this,NULL,NULL);
                if (found != NULL) {
                    assert(_this == found);
                    /* Free the query found because it has been removed
                     * from the queries-collection */
                    c_free(found);
                    v_queryDeinit(v_query(_this));
                }
                v_dataReaderUnLock(r);
            } else {
                OS_REPORT(OS_ERROR, "v_dataReaderQueryDeinit failed", 0,
                          "source is not datareader");
            }
            c_free(src);
        } else {
            OS_REPORT(OS_ERROR, "v_dataReaderQueryDeinit failed", 0,
                      "no source");
        }
    }
}

C_STRUCT(testActionArg) {
    c_query query;
    c_bool result;
};

C_CLASS(testActionArg);

static c_bool
testAction(
    c_object o,
    c_voidp arg)
{
    v_dataReaderInstance inst = v_dataReaderInstance(o);
    testActionArg a = (testActionArg)arg;

    a->result = v_dataReaderInstanceTest(inst,a->query);
    return (!a->result);
}

c_bool
v_dataReaderQueryTest(
    v_dataReaderQuery _this)
{
    v_collection src;
    v_dataReader r;
    c_long len,i;
    C_STRUCT(testActionArg) argument;
    c_table instanceSet;
    c_bool pass = FALSE;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    argument.result = FALSE;
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            v_dataReaderLock(r);
            instanceSet = r->index->notEmptyList;
            if (c_tableCount(instanceSet) > 0) {
                len = c_arraySize(_this->instanceQ);
                i = 0;
                while ((i<len) && (pass == FALSE)) {
                    argument.query = _this->sampleQ[i];
                    if (_this->instanceQ[i] != NULL) {
                        c_readAction(_this->instanceQ[i],
                                     testAction,&argument);
                    } else {
                        c_readAction(instanceSet, testAction, &argument);
                    }
                    pass = argument.result;
                    i++;
                }
            }
            if ( !pass ) {
                _this->state = V_STATE_INITIAL;
            }
            v_dataReaderUnLock(r);
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQueryTest failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderQueryTest failed", 0,
                  "no source");
    }
    return pass;
}

c_bool
v_dataReaderQueryTriggerTest(
    v_dataReaderQuery _this)
{
    v_collection src;
    v_dataReader r;
    c_long len,i;
    C_STRUCT(testActionArg) argument;
    c_table instanceSet;
    c_bool pass = FALSE;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    argument.result = FALSE;
    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            v_dataReaderLock(r);
            if (r->updateCnt == _this->updateCnt) {
                pass = _this->lastResult;
            } else {
                instanceSet = r->index->notEmptyList;
                if (c_tableCount(instanceSet) > 0) {
                    if ((r->triggerValue != NULL) &&
                        (r->triggerValue == _this->triggerValue))
                    {
                        v_dataReaderInstance instance;
                        instance = v_dataReaderInstance(v_readerSample(r->triggerValue)->instance);
                        /* This part should be moved to the notify method
                         * as part of the producer query evaluation.
                         */
                        len = c_arraySize(_this->instanceQ);
                        for (i=0;(i<len) && !pass;i++) {
                            pass = TRUE;
                            if (_this->instanceQ[i] != NULL) {
                                pass = c_queryEval(_this->instanceQ[i],instance);
                            }
                            if ((_this->sampleQ[i] != NULL) && pass) {
                                v_dataReaderSample head;
                                head = v_dataReaderInstanceHead(instance);
                                if (_this->triggerValue != head) {
                                    v_dataReaderInstanceSetHead(instance,_this->triggerValue);
                                }
                                pass = c_queryEval(_this->sampleQ[i],instance);
                                if (_this->triggerValue != head) {
                                    v_dataReaderInstanceSetHead(instance,head);
                                }
                            }
                        }
                    } else {
                        len = c_arraySize(_this->instanceQ);
                        i = 0;
                        while ((i<len) && (pass == FALSE)) {
                            argument.query = _this->sampleQ[i];
                            if (_this->instanceQ[i] != NULL) {
                                c_readAction(_this->instanceQ[i],
                                             testAction,&argument);
                            } else {
                                c_readAction(instanceSet, testAction, &argument);
                            }
                            pass = argument.result;
                            i++;
                        }
                    }
                }
                _this->updateCnt = r->updateCnt;
                _this->lastResult = pass;
            }
            if ( !pass ) {
                _this->state = V_STATE_INITIAL;
            }
            v_dataReaderUnLock(r);
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQueryTriggerTest failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderQueryTriggerTest failed", 0,
                  "no source");
    }
    return pass;
}

C_STRUCT(readActionArg) {
    c_query query;
    v_readerSampleAction action;
    c_voidp arg;
};
C_CLASS(readActionArg);


/* Read functions */

static c_bool
instanceReadSamples(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    readActionArg a = (readActionArg)arg;
    c_bool proceed = TRUE;

    if (!v_dataReaderInstanceEmpty(instance)) {
        proceed = v_dataReaderInstanceReadSamples(instance,
                                                  a->query,
                                                  a->action,
                                                  a->arg);
    }
    return proceed;
}

c_bool
v_dataReaderQueryRead (
    v_dataReaderQuery _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    C_STRUCT(readActionArg) argument;
    c_table instanceSet;
    int i,len;

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        if (v_objectKind(src) == K_DATAREADER) {

            r = v_dataReader(src);

            v_dataReaderLock(r);
            r->readCnt++;
#ifdef _MAXPURGE_
            v_dataReaderUpdatePurgeLists(r);
#endif
            if ((r->triggerValue != NULL) &&
                (r->triggerValue == _this->triggerValue)) {
                instanceSet = r->index->notEmptyList;
                if (c_tableCount(instanceSet) > 0) {
                    v_dataReaderInstance instance;
                    c_bool pass = FALSE;
                    instance = v_dataReaderInstance(v_readerSample(r->triggerValue)->instance);

                    /* This part should be moved to the notify method
                     * as part of the producer query evaluation.
                     */
                    len = c_arraySize(_this->instanceQ);
                    for (i=0;(i<len) && !pass;i++) {
                        pass = TRUE;
                        if (_this->instanceQ[i] != NULL) {
                            pass = c_queryEval(_this->instanceQ[i],instance);
                        }
                        if ((_this->sampleQ[i] != NULL) && pass) {
                            v_dataReaderSample head;
                            head = v_dataReaderInstanceHead(instance);
                            if (_this->triggerValue != head) {
                                v_dataReaderInstanceSetHead(instance,_this->triggerValue);
                            }
                            pass = c_queryEval(_this->sampleQ[i],instance);
                            if (_this->triggerValue != head) {
                                v_dataReaderInstanceSetHead(instance,head);
                            }
                        }
                    }

                    if (pass) {
                        proceed = v_dataReaderSampleRead(_this->triggerValue,
                                                         action,arg);
                    }
                    proceed = FALSE;
                }
            } else {

                argument.action = action;
                argument.arg = arg;
                argument.query = NULL;

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

            }
            if (_this->triggerValue) {
                c_free(v_readerSample(_this->triggerValue)->instance);
                c_free(_this->triggerValue);
                _this->triggerValue = NULL;
            }
            v_statisticsULongValueInc(v_query, numberOfReads, _this);

            action(NULL,arg); /* This triggers the action routine that
                               * the last sample is read. */

            if (!proceed) {
                _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
            }

            v_dataReaderUnLock(r);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQueryRead failed", 0,
                      "source is not datareader");
            assert(v_objectKind(src) == K_DATAREADER);
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataReaderQueryRead failed", 0,
                  "no source");
    }
    return proceed;
}

c_bool
v_dataReaderQueryReadInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    c_long i, len;

    if (instance == NULL) {
        /* Should fall within a lock on _this */
        v_statisticsULongValueInc(v_query, numberOfReads, _this);
        return FALSE;
    }
    if (v_dataReaderInstanceEmpty(instance)) {
        action(NULL,arg); /* This triggers the action routine that
                           * the last sample is read. */
    } else {
        src = v_querySource(v_query(_this));
        if (src != NULL) {
            assert(v_objectKind(src) == K_DATAREADER);
            if (v_objectKind(src) == K_DATAREADER) {
                r = v_dataReader(src);
                v_dataReaderLock(r);
                r->readCnt++;
#ifdef _MAXPURGE_
                v_dataReaderUpdatePurgeLists(r);
#endif
                len = c_arraySize(_this->instanceQ);
                i=0;
                while ((i<len) && proceed) {
                    if (_this->instanceQ[i] != NULL) {
                        if (c_queryEval(_this->instanceQ[i],instance)) {
                            proceed = v_dataReaderInstanceReadSamples(
                                              instance,
                                              _this->sampleQ[i],
                                              action,arg);
                        }
                    } else {
                        proceed = v_dataReaderInstanceReadSamples(
                                          instance,
                                          _this->sampleQ[i],
                                          action,arg);
                    }
                    i++;
                }
                action(NULL,arg); /* This triggers the action routine that
                                   * the last sample is read. */

                if (!proceed) {
                   _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
                }

                v_dataReaderUnLock(r);
            } else {
                proceed = FALSE;
                OS_REPORT(OS_ERROR,
                          "v_dataReaderQueryReadInstance failed", 0,
                          "source is not datareader");
            }
            c_free(src);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQueryReadInstance failed", 0,
                      "no source");
        }
    }
    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfReads, _this);
    return proceed;
}

struct nextInstanceActionArg {
    v_readerSampleAction action;
    c_voidp arg;
    c_bool hasData;
};

static c_bool
nextInstanceAction(
    v_readerSample sample,
    c_voidp arg)
{
    struct nextInstanceActionArg *a = (struct nextInstanceActionArg *)arg;
    a->hasData = TRUE;
    return a->action(sample,a->arg);
}

c_bool
v_dataReaderQueryReadNextInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    c_long i,len;
    v_dataReaderInstance nextInstance;
    struct nextInstanceActionArg a;

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            v_dataReaderLock(r);
            r->readCnt++;
#ifdef _MAXPURGE_
            v_dataReaderUpdatePurgeLists(r);
#endif
            len = c_arraySize(_this->instanceQ);
            nextInstance = v_dataReaderNextInstance(r,instance);

            a.action = action;
            a.arg = arg;
            a.hasData = FALSE;
            while ((nextInstance != NULL) && (a.hasData == FALSE)) {
                i=0;
                while ((i<len) && proceed) {
                    if (_this->instanceQ[i] != NULL) {
                        if (c_queryEval(_this->instanceQ[i],nextInstance)) {
                            proceed = v_dataReaderInstanceReadSamples(
                                                       nextInstance,
                                                       _this->sampleQ[i],
                                                       nextInstanceAction,
                                                       &a);
                        }
                    } else {
                        proceed = v_dataReaderInstanceReadSamples(
                                                   nextInstance,
                                                   _this->sampleQ[i],
                                                   nextInstanceAction,
                                                   &a);
                    }
                    i++;
                }
                nextInstance = v_dataReaderNextInstance(r,nextInstance);
            }
            action(NULL,arg); /* This triggers the action routine that
                               * the last sample is read. */

            if (!proceed) {
                _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
            }

            v_dataReaderUnLock(r);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQueryReadNextInstance failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataReaderQueryReadNextInstance failed", 0,
                  "no source");
    }
    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfReads, _this);
    return proceed;
}

C_STRUCT(takeActionArg) {
    v_dataReader reader;
    c_query query;
    v_readerSampleAction action;
    c_voidp arg;
    c_iter emptyList;
};
C_CLASS(takeActionArg);

static c_bool
instanceTakeSamples(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    takeActionArg a = (takeActionArg)arg;
    c_long count, oldCount;

    assert(C_TYPECHECK(a->reader, v_dataReader));
    assert(v_dataReader(a->reader)->sampleCount >= 0);

    if (v_dataReaderInstanceEmpty(instance)) {
        if (!c_iterContains(a->emptyList, instance)) {
             a->emptyList = c_iterInsert(a->emptyList,instance);
        } else {
             printf("instanceTakeSamples: found instance\n");
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
    v_statisticsULongSetValue(v_reader,
                              numberOfSamples,
                              a->reader,
                              v_dataReader(a->reader)->sampleCount);
    v_statisticsULongValueAdd(v_reader,
                              numberOfSamplesTaken,
                              a->reader,
                              count);
    assert(v_dataReader(a->reader)->sampleCount >= 0);

    return proceed;
}

c_bool
v_dataReaderQueryTake(
    v_dataReaderQuery _this,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    c_table instanceSet;
    int len, i;
    C_STRUCT(takeActionArg) argument;
    v_dataReaderInstance instance, emptyInstance;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);

            v_dataReaderLock(r);
            r->readCnt++;
#ifdef _MAXPURGE_
            v_dataReaderUpdatePurgeLists(r);
#endif
            if ((r->triggerValue != NULL) &&
                (r->triggerValue == _this->triggerValue)) {
                instanceSet = r->index->notEmptyList;
                if (c_tableCount(instanceSet) > 0) {
                    c_bool pass = FALSE;
                    instance = v_dataReaderInstance(v_readerSample(r->triggerValue)->instance);

                    /* This part should be moved to the notify method
                     * as part of the producer query evaluation.
                     */
                    len = c_arraySize(_this->instanceQ);
                    for (i=0;(i<len) && !pass;i++) {
                        pass = TRUE;
                        if (_this->instanceQ[i] != NULL) {
                            pass = c_queryEval(_this->instanceQ[i],instance);
                        }
                        if ((_this->sampleQ[i] != NULL) && pass) {
                            v_dataReaderSample head;
                            head = v_dataReaderInstanceHead(instance);
                            if (_this->triggerValue != head) {
                                v_dataReaderInstanceSetHead(instance,_this->triggerValue);
                            }
                            pass = c_queryEval(_this->sampleQ[i],instance);
                            if (_this->triggerValue != head) {
                                v_dataReaderInstanceSetHead(instance,head);
                            }
                        }
                    }

                    if (pass) {
                        proceed = v_dataReaderSampleTake(_this->triggerValue,
                                                         action,arg);
                        if (v_dataReaderInstanceEmpty(instance)) {
                            v_dataReaderRemoveInstance(r,instance);
                        }
                    }
                    proceed = FALSE;
                }
            } else {
                instanceSet = r->index->notEmptyList;
                if (c_tableCount(instanceSet) > 0) {
                    argument.action = action;
                    argument.arg = arg;
                    argument.reader = r;
                    argument.emptyList = NULL;
                    argument.query = NULL;

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
                        v_statisticsULongSetValue(v_reader,
                                                  numberOfInstances,
                                                  r,
                                                  v_dataReaderInstanceCount(r));
                    }
                }
            }
            if (_this->triggerValue) {
                c_free(v_readerSample(_this->triggerValue)->instance);
                c_free(_this->triggerValue);
                _this->triggerValue = NULL;
            }
            v_statisticsULongValueInc(v_query, numberOfTakes, _this);

            if (r->sampleCount == 0) {
                v_statusReset(v_entity(r)->status,V_EVENT_DATA_AVAILABLE);
            }
            action(NULL,arg); /* This triggers the action routine that
                               * the last sample is read. */

            if (!proceed) {
                _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
            }

            v_dataReaderUnLock(r);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQueryTake failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataReaderQueryTake failed", 0,
                  "no source");
    }
    return proceed;
}

c_bool
v_dataReaderQueryTakeInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    c_long i,len,count;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    if (instance == NULL) {
        /* Should fall within a lock on _this */
        v_statisticsULongValueInc(v_query, numberOfTakes, _this);
        return FALSE;
    }
    if (v_dataReaderInstanceEmpty(instance)) {
        action(NULL,arg); /* This triggers the action routine that the
                           * last sample is read. */
    } else {
        src = v_querySource(v_query(_this));
        if (src != NULL) {
            assert(v_objectKind(src) == K_DATAREADER);
            if (v_objectKind(src) == K_DATAREADER) {
                r = v_dataReader(src);
                v_dataReaderLock(r);
                r->readCnt++;
#ifdef _MAXPURGE_
                v_dataReaderUpdatePurgeLists(r);
#endif
                len = c_arraySize(_this->instanceQ);
                i=0;
                while ((i<len) && proceed) {
                    count = v_dataReaderInstanceSampleCount(instance);
                    if (_this->instanceQ[i] != NULL) {
                        if (c_queryEval(_this->instanceQ[i],instance)) {
                            proceed = v_dataReaderInstanceTakeSamples(
                                                   instance,
                                                   _this->sampleQ[i],
                                                   action,
                                                   arg);
                        }
                    } else {
                        proceed = v_dataReaderInstanceTakeSamples(
                                               instance,
                                               _this->sampleQ[i],
                                               action,
                                               arg);
                    }
                    count -= v_dataReaderInstanceSampleCount(instance);
                    assert(count >= 0);
                    r->sampleCount -= count;
                    v_statisticsULongSetValue(v_reader,
                                              numberOfSamples,
                                              r,
                                              r->sampleCount);
                    v_statisticsULongValueAdd(v_reader,
                                              numberOfSamplesTaken,
                                              r,
                                              count);
                    assert(r->sampleCount >= 0);
                    i++;
                }
                if (v_dataReaderInstanceEmpty(instance)) {
                    v_dataReaderRemoveInstance(r,instance);
                }
                if (r->sampleCount == 0) {
                    v_statusReset(v_entity(r)->status,V_EVENT_DATA_AVAILABLE);
                }
                action(NULL,arg); /* This triggers the action routine that the
                                   * last sample is read. */

                if (!proceed) {
                    _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
                }

                v_dataReaderUnLock(r);
            } else {
                proceed = FALSE;
                OS_REPORT(OS_ERROR,
                          "v_dataReaderQueryTakeInstance failed", 0,
                          "source is not datareader");
            }
            c_free(src);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQueryTakeInstance failed", 0,
                      "no source");
        }
    }
    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfTakes, _this);
    return proceed;
}

c_bool
v_dataReaderQueryTakeNextInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    v_collection src;
    v_dataReader r;
    c_long i,len,count;
    v_dataReaderInstance nextInstance;
    struct nextInstanceActionArg a;

    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            r = v_dataReader(src);
            v_dataReaderLock(r);
            r->readCnt++;
#ifdef _MAXPURGE_
            v_dataReaderUpdatePurgeLists(r);
#endif
            len = c_arraySize(_this->instanceQ);
            nextInstance = v_dataReaderNextInstance(r,instance);
            a.action = action;
            a.arg = arg;
            a.hasData = FALSE;
            while ((nextInstance != NULL) && (a.hasData == FALSE)) {
                i=0;
                while ((i<len) && proceed) {
                    count = v_dataReaderInstanceSampleCount(nextInstance);
                    if (_this->instanceQ[i] != NULL) {
                        if (c_queryEval(_this->instanceQ[i],nextInstance)) {
                            proceed = v_dataReaderInstanceTakeSamples(
                                               nextInstance,
                                               _this->sampleQ[i],
                                               nextInstanceAction,
                                               &a);
                        }
                    } else {
                        proceed = v_dataReaderInstanceTakeSamples(
                                           nextInstance,
                                           _this->sampleQ[i],
                                           nextInstanceAction,
                                           &a);
                    }
                    count -= v_dataReaderInstanceSampleCount(nextInstance);
                    assert(count >= 0);
                    r->sampleCount -= count;
                    v_statisticsULongSetValue(v_reader,
                                              numberOfSamples,
                                              r,
                                              r->sampleCount);
                    v_statisticsULongValueAdd(v_reader,
                                              numberOfSamplesTaken,
                                              r,
                                              count);
                    assert(r->sampleCount >= 0);
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
            if (r->sampleCount == 0) {
                v_statusReset(v_entity(r)->status,V_EVENT_DATA_AVAILABLE);
            }
            action(NULL,arg); /* This triggers the action routine that
                               * the last sample is read. */

            if (!proceed) {
                _this->state = _this->state & ~V_STATE_DATA_AVAILABLE;
            }

            v_dataReaderUnLock(r);
        } else {
            proceed = FALSE;
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQueryTakeNextInstance failed", 0,
                      "source is not datareader");
        }
        c_free(src);
    } else {
        proceed = FALSE;
        OS_REPORT(OS_ERROR,
                  "v_dataReaderQueryTakeNextInstance failed", 0,
                  "no source");
    }
    /* Should fall within a lock on _this */
    v_statisticsULongValueInc(v_query, numberOfTakes, _this);
    return proceed;
}

c_bool
v_dataReaderQueryNotifyDataAvailable(
    v_dataReaderQuery _this,
    v_event e)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_dataReaderQuery));
    assert(e);
    assert(C_TYPECHECK(e->userData,v_dataReaderSample));

    v_observerLock(v_observer(_this));
    /* Only store the trigger value and notify observers if no
     * trigger value is set before.
     * The trigger value is reset after a modification of the state
     * of the data in the reader history.
     * Query Read and Take operations can compare the trigger value
     * with the DataReader's last trigger value to decide to use the
     * trigger value instead of executing the query.
     */
    if (e->userData) {
        if (_this->triggerValue == NULL) {
            c_keep(v_readerSample(e->userData)->instance);
            _this->triggerValue = c_keep(v_dataReaderSample(e->userData));
        }
        _this->state |= V_STATE_DATA_AVAILABLE;
        v_observableNotify(v_observable(_this),e);
    } else {
#if 0
        OS_REPORT(OS_WARNING,
                  "v_dataReaderQueryNotifyDataAvailable failed", 0,
                  "No triggerValue provided");
        assert(FALSE);
#endif
    }
    v_observerUnlock(v_observer(_this));

    return TRUE;
}

c_bool
v_dataReaderQuerySetParams(
    v_dataReaderQuery _this,
    q_expr expression,
    c_value params[])
{
    v_collection src;
    v_dataReader r;
    v_kernel kernel;
    c_long i,len;
    q_expr e,subExpr,keyExpr,progExpr;
    q_expr predicate;
    c_iter list;
    c_type type;
    c_bool result = TRUE;
    c_array keyList;
    c_table instanceSet;
    c_long size, strSize, curSize, exprSize, count;
    c_char *tmp, *paramString;
    c_base base;
    c_char character, prevCharacter;
    c_char number[32];
    c_bool inNumber;

  /* first remove the old query */
    assert(C_TYPECHECK(_this,v_dataReaderQuery));

    if (q_getTag(expression) !=  Q_EXPR_PROGRAM) {
        assert(FALSE);
        return FALSE;
    }

    src = v_querySource(v_query(_this));
    if (src != NULL) {
        assert(v_objectKind(src) == K_DATAREADER);
        if (v_objectKind(src) == K_DATAREADER) {
            kernel = v_objectKernel(_this);
            base = c_getBase(c_object(_this));
            r = v_dataReader(src);

            v_dataReaderLock(r);
            _this->updateCnt = 0;
            len = c_arraySize(_this->instanceQ);
            for (i=0; (i<len) && (result == TRUE); i++) {
                result = c_querySetParams(_this->instanceQ[i],params) &&
                         c_querySetParams(_this->sampleQ[i],params);
            }
            if (result) {
                v_dataReaderUnLock(r);
            } else {
                predicate = q_exprCopy(expression);
#if PRINT_QUERY
                printf("v_datyaReaderQuerySetParams\n");
                printf("predicate:\n"); q_print(predicate,0); printf("\n");
#endif

                e = q_takePar(predicate,0);
                if (!resolveFields(r,e)) {
                    v_dataReaderUnLock(r);
                    q_dispose(e);
                    q_dispose(predicate);
                    return FALSE;
                }

                _this->instanceMask = q_exprGetInstanceState(expression);
                _this->sampleMask   = q_exprGetSampleState(expression);
                _this->viewMask     = q_exprGetViewState(expression);

                /* Normalize the query to the disjunctive form. */
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
                                                         progExpr,params);
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
                        /* The following code generates the intermeadiate
                         * non-key query code. Unfortunatly c_queryNew
                         * creates the query expression relative to the given
                         * collection's element type. In this case the instance
                         * type. This means that to performe the query
                         * evaluation on each sample within an instance the
                         * sample must be swapped with the instance sample
                         * field and reswapped after the evaluation.
                         */
                        progExpr = F1(Q_EXPR_PROGRAM,subExpr);
                        _this->sampleQ[i] = c_queryNew(instanceSet,
                                                       progExpr,params);
                        q_dispose(progExpr);
                    } else {
#if PRINT_QUERY
                        printf("subExpr[%d]: <NULL>\n",i);
#endif
                        _this->sampleQ[i]   = NULL;
                    }
                }
                c_iterFree(list);
                v_dataReaderUnLock(r);
#if PRINT_QUERY
                printf("End v_dataReaderQuerySetParams\n\n");
#endif
                if(_this->expression){
                    c_free(_this->expression);
                    _this->expression = NULL;
                }
                tmp = q_exprGetText(predicate);

                if(tmp){
                    _this->expression = c_stringNew(base, tmp);
                    os_free(tmp);
                }
                q_dispose(predicate);
            }
            result = TRUE;

            if(_this->params){
                c_free(_this->params);
                _this->params = NULL;
            }

            if (params) {
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

                if (size > 0) {
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
                        strcat(paramString, tmp);
                        os_free(tmp);

                        if(i+1 != size){
                            strcat(paramString, ",");
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
        } else {
            OS_REPORT(OS_ERROR,
                      "v_dataReaderQuerySetParams failed", 0,
                      "source is not datareader");
            result = FALSE;
        }
        c_free(src);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderQuerySetParams failed", 0,
                  "no source");
        result = FALSE;
    }

    return result;
}
