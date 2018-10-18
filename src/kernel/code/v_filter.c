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
#include "v_filter.h"
#include "v_topic.h"

#include "c_filter.h"
#include "c_metabase.h"

#include "os_report.h"
#include "vortex_os.h"
#include "q_helper.h"
#include "v_index.h"

#define HAS_KEY (1)
#define HAS_NONKEY (2)

static q_expr
resolveField(
    c_type type,
    const c_char *name)
{
    c_field field;
    c_array path;
    c_ulong i, length;
    q_list list;
    c_char* metaName;

    field = c_fieldNew(type,name);
    if (field == NULL) {
        metaName = c_metaName(c_metaObject(type));
        OS_REPORT(OS_ERROR,
                  "kernel::v_filter::v_filterNew:",V_RESULT_ILL_PARAM,
                  "Field %s not found in type %s\n",
                   name,metaName);
        c_free(metaName);
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
    c_bool result;

    result = TRUE;
    switch(q_getKind(e)) {
        case T_FNC:
            switch(q_getTag(e)) {
                case Q_EXPR_PROPERTY:
                    name = q_propertyName(e);
                    p = resolveField(type,name);
                    os_free(name);
                    if (p != NULL) {
                        q_swapExpr(e,p);
                        q_dispose(p);
                    } else {
                        result = FALSE;
                    }
                    break;
                default: /* process sub-expression, fail if a field cannot be resolved */
                    i=0;
                    while ((result) && ((p = q_getPar(e,i)) != NULL)) {
                        result = resolveFields(type,p);
                        i++;
                    }
                    break;
            }
            break;
        case T_ID:
            name = q_getId(e);
            p = resolveField(type,name);
            if (p != NULL) {
                q_swapExpr(e,p);
                q_dispose(p);
            } else {
                result = FALSE;
            }
            break;
        default:
            break;
    }
    return result;
}

#define PRINT_QUERY (0)

static c_filter
createFilter(
    c_type type,
    q_expr subExpr,
    const c_value params[])
{
    c_filter filter = NULL;
    q_expr progExpr;
#if PRINT_QUERY
    printf("subExpr[%d]: ",i);
    q_print(subExpr,12);
    printf("\n");
#endif
    progExpr = F1(Q_EXPR_PROGRAM,subExpr);
    if (resolveFields(type, progExpr)) {
        filter = c_filterNew(type,progExpr,params);
    } else {
        OS_REPORT(OS_ERROR, "kernel::v_filter::v_filterNew",V_RESULT_ILL_PARAM,
                  "Failed to resolve fields in filter expression." OS_REPORT_NL "Type = \"%s\"",
                  c_metaName(c_metaObject(type)));
    }
    q_dispose(progExpr);
    return filter;
}

v_filter
v_filterNew(
    c_type type,
    c_array keyList,
    q_expr e,
    const c_value params[],
    os_uint32 nrOfParams)
{
    c_base base;
    c_type filtertype;
    v_filter filter = NULL;
    c_bool error = FALSE;
    c_iter list;
    c_ulong i,len;
    q_expr expr,subExpr,keyExpr;

    OS_UNUSED_ARG(nrOfParams);

    assert(type != NULL);
    assert(C_TYPECHECK(type,c_type));

    if (q_getLastVar(e) > nrOfParams) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_filter::v_filterNew",V_RESULT_ILL_PARAM,
                  "Requested param %%%d not supplied, only %d parameters where passed",
                  q_getLastVar(e), nrOfParams);
        return NULL;
    }
    base = c_getBase(type);
    filtertype = c_resolve(base,"kernelModuleI::v_filter");
    filter = c_new(filtertype);
    filter->key = NULL;
    filter->nonkey = NULL;
    filter->flags = 0;
    c_free(filtertype);

    expr = q_exprCopy(q_getPar(e,0));
    if (expr != NULL) {
        q_prefixFieldNames(&expr,"userData");

        q_disjunctify(expr);
        expr = q_removeNots(expr);
        list = q_exprDeOr(expr,NULL);
        len = c_iterLength(list);
        filter->key = c_arrayNew(c_object_t(base),len);
        filter->nonkey = c_arrayNew(c_object_t(base),len);

        for (i=0;i<len;i++) {
            subExpr = c_iterTakeFirst(list);
            assert(subExpr != NULL);

            keyExpr = q_takeKey(&subExpr, keyList);

            filter->key[i] = NULL;
            if (keyExpr != NULL) {
                filter->flags |= HAS_KEY;
                filter->key[i] = createFilter(type, keyExpr, params);
                if (filter->key[i] == NULL) {
                    error = TRUE;
                }
            }
            filter->nonkey[i] = NULL;
            if (subExpr != NULL) {
                filter->flags |= HAS_NONKEY;
                filter->nonkey[i] = createFilter(type, subExpr, params);
                if (filter->nonkey[i] == NULL) {
                    error = TRUE;
                }
            }
        }
        c_iterFree(list);
    }
    if (error) {
        c_free(filter);
        filter = NULL;
    }
    return filter;
}

static c_bool
filterEval(v_filter _this, c_object o, c_bool onlykey)
{
    os_uint32 index, length;
    c_bool pass = TRUE;

    assert(C_TYPECHECK(_this,v_filter));

    length = c_arraySize(_this->key);
    for (index=0; index<length; index++) {
        pass = TRUE;
        if (!onlykey && _this->nonkey[index]) {
            pass = c_qPredEval(_this->nonkey[index],o);
        }
        if ((pass == TRUE)&&(_this->key[index])) {
            pass = c_qPredEval(_this->key[index],o);
        }
        if (pass == TRUE) {
            break;
        }
    }
    return pass;
}

c_bool
v_filterEvalKey(v_filter _this, c_object o)
{
    return filterEval(_this, o, TRUE);
}

c_bool
v_filterEval(v_filter _this, c_object o)
{
    return filterEval(_this, o, FALSE);
}

c_bool
v_filterHasKey (
    v_filter _this)
{
    return ((_this->flags & HAS_KEY) != 0);
}

c_bool
v_filterHasNonKey (
    v_filter _this)
{
    return ((_this->flags & HAS_NONKEY) != 0);
}
