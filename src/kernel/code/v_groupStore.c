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
#include "v__groupStore.h"
#include "v__groupInstance.h"
#include "v__group.h"
#include "v__kernel.h"
#include "v__topic.h"
#include "v_kernelParser.h"
#include "v_public.h"
#include "v_message.h"
#include "v_messageQos.h"
#include "q_helper.h"
#include "c_base.h"
#include "vortex_os.h"
#include "os_report.h"

C_STRUCT(v_groupStoreQuery) {
    c_array instanceQ;
    c_array sampleQ;
    void *userData;
};

v_groupStore
v_groupStoreNew(
    const v_group group,
    const c_char *keyExpr,
    const c_array messageKeyList)
{
    v_groupStore store;
    v_kernel kernel;
    c_base base;

    base = c_getBase(group);
    kernel = v_objectKernel(group);

    store = c_new(v_kernelType(kernel,K_GROUPSTORE));
    (void)c_mutexInit(base, &store->mutex);
    store->group = group; /* (void *) backref */
    store->instances = c_tableNew(group->instanceType,keyExpr);
    store->messageKeyList = c_keep(messageKeyList);
    store->readid = 0;
    return store;
}

static c_bool
instanceFree (
    c_object o,
    c_voidp arg)
{
    OS_UNUSED_ARG(arg);
    v_groupInstanceFree(v_groupInstance(o));
    return TRUE;
}

void
v_groupStoreDeinit(
    const v_groupStore _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_groupStore));

    c_mutexLock(&_this->mutex);
    (void)c_tableWalk(_this->instances,instanceFree,NULL);
    c_mutexUnlock(&_this->mutex);
}

static c_bool
alwaysFalse(
    c_object found,
    c_object requested,
    c_voidp arg)
{
    v_groupInstance *instance = (v_groupInstance *)arg;

    OS_UNUSED_ARG(requested);
    assert(instance != NULL);
    assert(*instance == NULL); /* out param */

    *instance = c_keep(found);

    return FALSE;
}

static v_groupInstance
lookupInstance(
    v_groupStore _this,
    v_message msg)
{
    v_groupInstance instance = NULL;
    c_value _keyValues[32];
    c_value *keyValues = _keyValues;

    if (_this->gidkey) {
        C_STRUCT(v_groupInstanceGID) template;
        template.gid = msg->writerInstanceGID;
        /* Note: The alwaysFalse function increases the refCount of
         * found, which is the out-parameter of the tableRemove.
         */
        c_tableRemove(_this->instances, &template, alwaysFalse, &instance);
    } else {
        c_ulong i, nrOfKeys;

        nrOfKeys = c_arraySize(_this->messageKeyList);
        if (nrOfKeys > 32) {
            keyValues = os_malloc(sizeof(c_value) * nrOfKeys);
        }
        for (i=0;i<nrOfKeys;i++) {
            keyValues[i] = c_fieldValue(_this->messageKeyList[i], msg);
        }
        instance = c_tableFind(_this->instances, &keyValues[0], nrOfKeys);
        /* Key values have to be freed again */
        for (i=0;i<nrOfKeys;i++) {
            c_valueFreeRef(keyValues[i]);
        }
        if (nrOfKeys > 32) {
            os_free(keyValues);
        }
    }
    return instance;
}

v_groupInstance
v_groupStoreLookupInstance(
    const v_groupStore _this,
    const v_message msg)
{
    v_groupInstance instance = NULL;
    c_mutexLock(&_this->mutex);
    instance = lookupInstance(_this, msg);
    c_mutexUnlock(&_this->mutex);
    return instance;
}

v_groupInstance
v_groupStoreCreateInstance(
    const v_groupStore _this,
    const v_message msg)
{
    v_groupInstance instance;

    c_mutexLock(&_this->mutex);
    instance = lookupInstance(_this, msg);
    if (instance == NULL) {
        v_groupInstance found;
        instance = v_groupInstanceNew(_this->group, msg);
        found = c_tableInsert(_this->instances, instance);
        assert(found == instance);
        OS_UNUSED_ARG(found);
    }
    c_mutexUnlock(&_this->mutex);

    return instance;
}

void
v_groupStoreDispose(
    const v_groupStore _this,
    const v_groupInstance instance)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_groupStore));
    OS_UNUSED_ARG(instance);
    c_mutexLock(&_this->mutex);
    c_mutexUnlock(&_this->mutex);
}

void
v_groupStoreDelete(
    const v_groupStore _this,
    const v_groupInstance instance)
{
    v_groupInstance removed;

    c_mutexLock(&_this->mutex);
    removed = c_remove(_this->instances,instance,NULL,NULL);
    c_mutexUnlock(&_this->mutex);
    /* It is allowed that the instance is already taken and
     * no longer exists in the group->instances set.
     */
    if (removed) {
        v_groupInstanceFree(removed);
    }
}

void
v_groupStoreWalk(
    const v_groupStore _this,
    const c_action action,
    const c_voidp arg)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_groupStore));

    c_mutexLock(&_this->mutex);
    (void)c_tableWalk(_this->instances,action,arg);
    c_mutexUnlock(&_this->mutex);
}

c_iter
v_groupStoreSelect(
    const v_groupStore _this,
    const os_uint32 max)
{
    c_iter list;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_groupStore));

    c_mutexLock(&_this->mutex);
    list = ospl_c_select(_this->instances, (c_long)max);
    c_mutexUnlock(&_this->mutex);

    return list;
}

static c_bool
markGroupInstance(c_object o, c_voidp arg)
{
    v_groupInstance instance = v_groupInstance(o);
    c_ulong *flags = (c_ulong *)arg;

    v_stateSet(instance->state, *flags);
    return TRUE;
}

/* Set the specified flags for all DataReader instance states associated
 * with the specified group.
 */
void
v_groupStoreMarkGroupInstanceStates (
    const v_groupStore _this,
    c_ulong flags)
{
    c_mutexLock(&_this->mutex);
    (void)c_tableWalk(_this->instances,markGroupInstance,&flags);
    c_mutexUnlock(&_this->mutex);
}

static c_bool
unmarkGroupInstance(c_object o, c_voidp arg)
{
    v_groupInstance instance = v_groupInstance(o);
    c_ulong *flags = (c_ulong *)arg;

    v_stateClear(instance->state, *flags);
    return TRUE;
}

/* Set the specified flags for all DataReader instance states associated
 * with the specified group.
 */
void
v_groupStoreUnmarkGroupInstanceStates (
    const v_groupStore _this,
    c_ulong flags)
{
    assert(C_TYPECHECK(_this,v_groupStore));

    if (flags != 0) {
        c_mutexLock(&_this->mutex);
        (void)c_tableWalk(_this->instances, unmarkGroupInstance, &flags);
        c_mutexUnlock(&_this->mutex);
    }
}

c_array
v_groupStoreKeyList(
    const v_groupStore _this)
{
    return c_tableKeyList(_this->instances);
}

static q_expr
resolveField(
    c_type instanceType,
    const c_char *name)
{
    c_field field;
    c_array path;
    c_ulong i, length;
    q_list list;
    c_string str;
    c_char *fieldName;
    q_expr expr;

    field = c_fieldNew(instanceType,name);
    if (!field) {
        fieldName = os_alloca(strlen(name) + strlen("newest.message.userData.") + 1);
        os_sprintf(fieldName,"newest.%s",name);
        field = c_fieldNew(instanceType,fieldName);

        if (!field) {
            os_sprintf(fieldName,"newest.message.%s",name);
            field = c_fieldNew(instanceType,fieldName);
            if (!field) {
                os_sprintf(fieldName,"newest.message.userData.%s",name);
                field = c_fieldNew(instanceType,fieldName);
            }
        }
        os_freea(fieldName);
    }
    if (field) {
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
        expr = q_newFnc(Q_EXPR_PROPERTY,list);
    } else {
        expr = NULL;
    }
    return expr;
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
                OS_REPORT(OS_ERROR, "v_groupStoreQueryNew failed",
                          V_RESULT_ILL_PARAM, "Parsing query expression failed: "
                                              "field '%s' is undefined for type '%s'",
                          name, c_metaName(c_metaObject(type)));
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
            OS_REPORT(OS_ERROR, "v_groupStoreQueryNew failed",
                      V_RESULT_ILL_PARAM, "Parsing query expression failed: field '%s' "\
                                          "is undefined for type '%s'",
                      name, c_metaName(c_metaObject(type)));
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

#define PREFIX "newest.message"
static c_array
createKeyList(
    c_type instanceType,
    c_array keyList)
{
    c_ulong size, i;
    c_array newKeyList = NULL;

    assert(instanceType);

    if(keyList){
        size = c_arraySize(keyList);

        newKeyList = c_arrayNew(c_field_t(c_getBase(instanceType)), size);

        if(newKeyList){
            for(i = 0; i<size; i++){
                c_field f = c_fieldNew(instanceType, PREFIX);
                assert(f);
                if(f){
                    newKeyList[i] = c_fieldConcat(f, keyList[i]);
                    c_free(f);
                } else {
                    OS_REPORT(OS_ERROR,
                                "createKeyList", V_RESULT_INTERNAL_ERROR,
                                "Could not create c_field");
                }
            }
        } else {
            OS_REPORT(OS_ERROR,
                        "createKeyList", V_RESULT_INTERNAL_ERROR,
                        "Could not create array");
        }
    }
    return newKeyList;
}
#undef PREFIX

void
v_groupStoreQueryFree(
    v_groupStoreQuery _this)
{
    if (_this->instanceQ) {
        c_free(_this->instanceQ);
    }
    if (_this->sampleQ) {
        c_free(_this->sampleQ);
    }
    os_free(_this);
}

v_groupStoreQuery
v_groupStoreQueryNew(
    const v_groupStore _this,
    const os_char *expression,
    const c_value params[],
    const os_uint32 nrOfParams)
{
    v_groupStoreQuery query;
    c_bool result = TRUE;
    q_expr progExpr, e, subExpr, keyExpr;
    c_ulong i,len;
    c_iter list;
    c_type type, c_query_t;
    c_array keyList;
    q_expr predicate;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_groupStore));

    if (expression == NULL) {
        return NULL;
    }
    predicate = v_parser_parse(expression);
    if (!predicate) {
        OS_REPORT(OS_ERROR, "DataReader SQL Parser",
                  V_RESULT_ILL_PARAM, "Parse Query expression failed. Query: \"%s\"",
                  expression);
        return NULL;
    }

    if (q_getLastVar(predicate) > nrOfParams) {
        q_dispose(predicate);
        OS_REPORT(OS_ERROR, "DataReader SQL Parser",
                  V_RESULT_ILL_PARAM, "Query parameter count mismatch. Query: \"%s\"",
                  expression);
        return NULL;
    }

    c_mutexLock(&_this->mutex);

    query = os_malloc(C_SIZEOF(v_groupStoreQuery));
    query->instanceQ = NULL;
    query->sampleQ = NULL;
    query->userData = NULL;

    e = q_takePar(predicate,0);
    type = c_subType(_this->instances);
    if (!resolveFields(type,e)) {
        q_dispose(e);
        result = FALSE;
    } else {
        /* Normalize the query to the disjunctive form. */
        q_disjunctify(e);
        e = q_removeNots(e);

        list = q_exprDeOr(e,NULL);
        len = c_iterLength(list);

        c_query_t = c_resolve(c_getBase(c_object(v_objectKernel(_this->group))),"c_query");

        query->instanceQ = c_arrayNew(c_query_t,len);
        query->sampleQ   = c_arrayNew(c_query_t,len);
        c_free(c_query_t);

        keyList = createKeyList(type, _this->messageKeyList);

        for (i=0;i<len && result;i++) {
            subExpr = c_iterTakeFirst(list);
            assert(subExpr != NULL);
            keyExpr = q_takeKey(&subExpr, keyList);

            if (keyExpr != NULL) {
                progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
                query->instanceQ[i] = c_queryNew(_this->instances, progExpr, params);
                q_dispose(progExpr);

                if (query->instanceQ[i] == NULL) {
                    OS_REPORT(OS_ERROR, "calculateCondition failed",
                              V_RESULT_ILL_PARAM, "error in query expression");
                    result = FALSE;
                }
            } else {
                query->instanceQ[i] = NULL;
            }
            if (subExpr != NULL) {
                progExpr   = F1(Q_EXPR_PROGRAM,subExpr);
                query->sampleQ[i] = c_queryNew(_this->instances, progExpr, params);
                q_dispose(progExpr);

                if (query->sampleQ[i] == NULL) {
                    OS_REPORT(OS_ERROR, "calculateCondition failed",
                              V_RESULT_ILL_PARAM, "error in query expression");
                    result = FALSE;
                }
            } else {
                query->sampleQ[i] = NULL;
            }
        }
        c_iterFree(list);
        c_free(keyList);
    }
    c_mutexUnlock(&_this->mutex);

    q_dispose(predicate);

    if (!result){
        v_groupStoreQueryFree(query);
        query = NULL;
    }
    return query;
}

v_groupStoreQuery
v_groupStoreQueryNew2(
    const v_groupStore _this,
    const os_char *expression,
    const os_char *params[],
    const os_uint32 nrOfParams)
{
    c_value values[100]; /* maximum number of parameters */
    os_uint32 i;

    /* The dds specification prescribes a max parameter number of 99 */
    assert(nrOfParams<100);
    for (i=0; i<nrOfParams; i++) {
        values[i] = c_stringValue((const c_string)params[i]);
    }
    return v_groupStoreQueryNew(_this, expression, values, nrOfParams);
}

struct readArg {
    v_groupStoreQuery query;
    os_timeW begin;
    os_timeW end;
    v_groupStoreAction action;
    const void *actionArg;
    c_ulong readid;
    v_result result;
};

static c_bool
checkConditionPeriod(
    v_groupSample sample,
    os_timeW begin,
    os_timeW end)
{
    c_bool pass = TRUE;
    os_timeW writeTime = v_groupSampleTemplate(sample)->message->writeTime;
    if (!OS_TIMEW_ISINVALID(begin)) {
        if (os_timeWCompare(begin, writeTime) == OS_MORE) {
            pass = FALSE; /* produced before begin of period */
        }
    }
    if (pass && !OS_TIMEW_ISINVALID(end)) {
        if (os_timeWCompare(end, writeTime) == OS_LESS) {
            pass = FALSE; /* produced after end of period */
        }
    }
    return pass;
}

static c_bool
checkTransientLocal(
    v_groupSample sample)
{
    v_message vmessage;
    v_groupInstance instance;
    v_registration reg;
    c_bool pass = TRUE;

    vmessage = v_groupSampleTemplate(sample)->message;
    if ( (v_messageQos_durabilityKind(vmessage->qos) == V_DURABILITY_TRANSIENT_LOCAL) ) {
        /* check if the sample is registered; if so, the writer is alive */
        instance = v_groupInstance(sample->instance);
        reg = instance->registrations;
        pass = FALSE;
        while ( (reg != NULL) && ! pass ) {
            if ( v_gidCompare(reg->writerGID,vmessage->writerInstanceGID) == C_EQ) {
                pass = TRUE;
            } else {
                reg = reg->next;
            }
        }
    }
    return pass;
}

static c_bool
walkMatchingSamples(
    c_object obj,
    c_voidp args)
{
    struct readArg *arg = (struct readArg *)args;
    v_groupStoreQuery query;
    c_ulong len, i;
    v_groupSample firstSample, sample;
    c_bool pass, proceed;
    v_groupInstance instance;

    query = arg->query;
    instance = (v_groupInstance)obj;

    proceed = TRUE;

    if (v_stateTest(instance->state, L_EMPTY)) {
        return proceed;
    }
    /* only walk in case the instance queue is not empty */
    if (query) {
        len = c_arraySize(query->instanceQ);
        for (i=0; (i<len) && proceed;i++) {
            if (query->instanceQ[i]) {
                pass = c_queryEval(query->instanceQ[i],instance);
            } else {
                pass = TRUE;
            }
            if (pass) { /* instance matches query*/
                /* Since history is 'replayed' here, the oldest sample should be
                 * processed first. We keep a reference to the first sample and
                 * set the current sample to the tail of the instance (oldest).
                 */
                firstSample = v_groupInstanceHead(instance);
                sample = v_groupInstanceTail(instance);
                while ((sample != NULL) && proceed) {
                    /* if len > 1 then expression contains OR meaning that multiple queries
                     * are executed and results are joined, so samples can be selected multiples times,
                     * therefore check if not already inserted.
                     */
                    if (len == 1 || sample->readid != arg->readid) {
                        sample->readid = arg->readid;
                        /* prevent aligning local-transient data without alive writers */
                        pass = checkTransientLocal(sample);
                        if (pass) {
                            /* prevent aligning data outside request period */
                            pass = checkConditionPeriod(sample, arg->begin, arg->end);
                        }
                        if (pass && query->sampleQ[i]) {
                            /* Evaluate non key query part */
                            if (sample == firstSample) {
                                pass = c_queryEval(query->sampleQ[i], instance);
                            } else {
                                /* Swap sample with first sample during query evaluation since
                                 * it only addresses first sample.
                                 */
                                v_groupInstanceSetHeadNoRefCount(instance,sample);
                                pass = c_queryEval(query->sampleQ[i], instance);
                                v_groupInstanceSetHeadNoRefCount(instance,firstSample);
                            }
                        }
                        if (pass) {
                            proceed = arg->action(sample, arg->actionArg);
                        }
                    }
                    sample = sample->newer;
                }
            }
        }
    } else {
        sample = v_groupInstanceTail(instance);
        while ((sample != NULL) && proceed) {
            /* prevent aligning local-transient data without alive writers */
            pass = checkTransientLocal(sample);
            if (pass) {
                /* prevent aligning data outside request period */
                pass = checkConditionPeriod(sample, arg->begin, arg->end);
            }
            if (pass) {
                proceed = arg->action(sample, arg->actionArg);
            }
            sample = sample->newer;
        }
    }
    return proceed;
}

v_result
v_groupStoreRead(
    const v_groupStore _this,
    const v_groupStoreQuery query,
    const v_groupStoreAction action,
    const void *actionArg)
{
    struct readArg arg;

    arg.query = query;
    arg.begin = OS_TIMEW_INVALID;
    arg.end = OS_TIMEW_INVALID;
    arg.action = action;
    arg.actionArg = actionArg;
    arg.readid = _this->readid++;
    arg.result = V_RESULT_OK;

    c_mutexLock(&_this->mutex);
    (void)c_tableWalk(_this->instances,walkMatchingSamples,&arg);
    c_mutexUnlock(&_this->mutex);

    return arg.result;
}

c_query
v_groupStore_create_query(
    const v_groupStore _this,
    const q_expr expr,
    const c_value *params)
{
    c_query query;
    c_mutexLock(&_this->mutex);
    query = c_queryNew(_this->instances, expr, params);
    c_mutexUnlock(&_this->mutex);
    return query;
}
