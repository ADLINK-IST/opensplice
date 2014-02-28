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
#include "v_filter.h"
#include "v_topic.h"

#include "c_filter.h"

#include "os_report.h"
#include "os.h"
#include "q_helper.h"
#include "v_index.h"

static q_expr
resolveField(
    c_type type,
    const c_char *name,
    c_bool typeIsIndex)
{
    c_property property;

    c_field field;
    c_array path;
    c_long i, length;
    q_list list;
    c_char* metaName;
    c_type fieldType;

    /*In case the filter is created using the instance, parameter type is already a usable type
    and doesn't need to be resolved using c_metaResolve, so skip that lookup.
    */
    if (typeIsIndex==TRUE)
    {
        fieldType = c_keep(type);
    }
    else
    {
        property = c_property(c_metaResolve(c_metaObject(type),"userData"));
        fieldType = c_keep(property->type);
        c_free(property);
    }
    if (strncmp(name, "sample.message.userData",strlen("sample.message.userData")) == 0)
    {
    	field = c_fieldNew(fieldType,name+strlen("sample.message.userData")+1);
    }
    else
    {
        field = c_fieldNew(fieldType,name);
    }
    c_free(fieldType);
    if (field == NULL) {
        metaName = c_metaName(c_metaObject(fieldType));
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_filter::v_filterNew:",0,
                    "Field %s not found in type %s\n",
                    name,metaName);
        c_free(metaName);
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
    if (!typeIsIndex)
    {
        list = q_insert(list,q_newId("userData"));
    }

    return q_newFnc(Q_EXPR_PROPERTY,list);
}

static c_bool
resolveFields (
    c_type type,
    q_expr e,
    c_bool typeIsIndex)
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
                    p = resolveField(type,name, typeIsIndex);
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
                        result = resolveFields(type,p, typeIsIndex);
                        i++;
                    }
                    break;
            }
            break;
        case T_ID:
            name = q_getId(e);
            p = resolveField(type,name, typeIsIndex);
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

v_filter
v_filterNew(
    v_topic t,
    q_expr e,
    c_value params[])
{
    v_kernel kernel;
    c_type type;
    v_filter filter;

    assert(C_TYPECHECK(t,v_topic));

    filter = NULL;
    kernel = v_objectKernel(t);
    type = v_topicMessageType(t);

    if (t) {
        if (type) {
            if (!resolveFields(type,e, FALSE)) {
                OS_REPORT_1(OS_ERROR,
                            "kernel::v_filter::v_filterNew",0,
                            "Failed to resolve fields in filter expression."
                            OS_REPORT_NL "Topic = \"%s\"",
                            v_topicName(t));
            } else {
                filter = c_new(v_kernelType(kernel, K_FILTER));
    
                if (filter) {
                    filter->topic = c_keep(t);
                    filter->predicate = c_filterNew(type,e,params);
                    if (filter->predicate == NULL) {
                        c_free(filter);
                        filter = NULL;
                    }
                } else {
                    OS_REPORT_1(OS_ERROR,
                                "kernel::v_filter::v_filterNew",0,
                                "Failed to allocate a filter."
                                OS_REPORT_NL "Topic = \"%s\"",
                                v_topicName(t));
                    assert(FALSE);
                }
            }
        } else {
            OS_REPORT_1(OS_ERROR,
                        "kernel::v_filter::v_filterNew",0,
                        "Failed to resolve type for Topic \"%s\".",
                        v_topicName(t));
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "kernel::v_filter::v_filterNew",0,
                  "Pre condition failed: Topic is not specified (NULL).");
    }
    return filter;
}

v_filter
v_filterNewFromIndex(
    v_index i,
    q_expr e,
    c_value params[])
{
    v_kernel kernel;
    c_type type;
    v_filter filter;

    assert(C_TYPECHECK(i,v_index));

    filter = NULL;
    if (i) {
        kernel = v_objectKernel(i);

        type = i->objectType;
        assert(type);
        if (type) {
            if (!resolveFields(type,e, TRUE)) {
                OS_REPORT_1(OS_ERROR,
                            "kernel::v_filter::v_filterNewFromIndex",0,
                            "Failed to resolve fields in filter expression."
                            OS_REPORT_NL "Topic = \"%s\"",
                            v_topicName(i));
            } else {
                filter = c_new(v_kernelType(kernel, K_FILTER));

                if (filter) {
                    filter->predicate = c_filterNew(type,e,params);
                    if (filter->predicate == NULL) {
                        c_free(filter);
                        filter = NULL;
                        OS_REPORT_1(OS_ERROR,
                                    "kernel::v_filter::v_filterNewFromIndex",0,
                                    "Failed to allocate a filter expression."
                                    OS_REPORT_NL "Topic = \"%s\"",
                                    v_topicName(i));
                                    assert(FALSE);
                    }
                } else {
                    OS_REPORT_1(OS_ERROR,
                                "kernel::v_filter::v_filterNewFromIndex",0,
                                "Failed to allocate a filter."
                                OS_REPORT_NL "Topic = \"%s\"",
                                v_topicName(i));
                    assert(FALSE);
                }
            }
        } else {
            OS_REPORT_1(OS_ERROR,
                        "kernel::v_filter::v_filterNewFromIndex",0,
                        "Failed to resolve type for Topic \"%s\".",
                        v_topicName(i));
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "kernel::v_filter::v_filterNew",0,
                  "Pre condition failed: Topic is not specified (NULL).");
    }
    return filter;
}

c_bool
v_filterEval(v_filter f, c_object o)
{
    assert(C_TYPECHECK(f,v_filter));
    return c_qPredEval(f->predicate, o);
}

#define PRINT_QUERY (0)

void v_filterSplit(v_topic topic, q_expr where, c_value *params, c_array *instanceQ, c_array *sampleQ, v_index index)
{
    c_iter list;
    c_long i,len;
    q_expr subExpr,keyExpr,progExpr;
    c_array sourceKeyList, indexKeyList;
    q_expr  WithoutWhere = q_getPar(q_exprCopy(where),0);
    v_kernel kernel = v_objectKernel(topic);

    if (WithoutWhere != NULL)
    {

        q_prefixFieldNames(&WithoutWhere,"sample.message.userData");

        q_disjunctify(WithoutWhere);
        WithoutWhere = q_removeNots(WithoutWhere);
        list = deOr(WithoutWhere,NULL);
        len = c_iterLength(list);
        *instanceQ = c_arrayNew(v_kernelType(kernel, K_FILTER),len);
        *sampleQ = c_arrayNew(v_kernelType(kernel, K_FILTER),len);

        for (i=0;i<len;i++) {
            subExpr = c_iterTakeFirst(list);
#if PRINT_QUERY
            printf("v_filterSplit\n");
            printf("deOr term(%d):\n",i);
            q_print(subExpr,0);
            printf("\n\n");
#endif
            assert(subExpr != NULL);

            sourceKeyList = v_indexSourceKeyList(index);
            indexKeyList = v_indexKeyList(index);

            keyExpr = q_takeKey(&subExpr, sourceKeyList);

            if (keyExpr != NULL) {
                translate(keyExpr, sourceKeyList, indexKeyList);
                assert(keyExpr);
            }

            if (keyExpr != NULL) {
#if PRINT_QUERY
                printf("keyExpr[%d]: ",i);
                q_print(keyExpr,12);
                printf("\n");
#endif
	            progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
                (*instanceQ)[i] = v_filterNewFromIndex(index,
                                                       progExpr, params);

                q_dispose(progExpr);
                if ((*instanceQ)[i] == NULL) {
                    c_free(*instanceQ);
                    c_free(*sampleQ);
                    c_iterFree(list);
                    OS_REPORT(OS_ERROR,
                              "v_filterSplit failed",0,
                              "error in expression");
                    return;
                }
            } else {
#if PRINT_QUERY
                printf("keyExpr[%d]: <NULL>\n",i);
#endif
                (*instanceQ)[i] = NULL;
            }
            if (subExpr != NULL) {
#if PRINT_QUERY
                printf("subExpr[%d]: ",i);
                q_print(subExpr,12);
                printf("\n");
#endif
                progExpr = F1(Q_EXPR_PROGRAM,subExpr);
                (*sampleQ)[i] = v_filterNew(topic,
                                            progExpr,
                                            params);
                q_dispose(progExpr);
                if ((*sampleQ)[i] == NULL) {
                    c_free(*instanceQ);
                    c_free(*sampleQ);
                    c_iterFree(list);
                    OS_REPORT(OS_ERROR,
                              "v_filterSplit failed",0,
                              "error in expression");
                    return;
                }
            } else {
#if PRINT_QUERY
                printf("subExpr[%d]: <NULL>\n",i);
#endif
                (*sampleQ)[i] = NULL;
            }
        }
#if PRINT_QUERY
    printf("End v_filterSplit\n\n");
#endif
    c_iterFree(list);
    }
    else
    {
        *instanceQ = c_arrayNew(v_kernelType(kernel, K_FILTER),1);
        *sampleQ = c_arrayNew(v_kernelType(kernel, K_FILTER),1);

        (*instanceQ)[0] = v_filterNew(topic, where, params);
        (*sampleQ)[0] = v_filterNew(topic, where, params);
    }
}
