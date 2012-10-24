/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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

static q_expr
resolveField(
    c_type type,
    const c_char *name)
{
    c_property property;

    c_field field;
    c_array path;
    c_long i, length;
    q_list list;
    c_char* metaName;

    property = c_property(c_metaResolve(c_metaObject(type),"userData"));

    field = c_fieldNew(property->type,name);
    if (field == NULL) {
        metaName = c_metaName(c_metaObject(property->type));
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_filter::v_filterNew:",0,
                    "Field %s not found in type %s\n",
                    name,metaName);
        c_free(metaName);
        c_free(property);
        return NULL;
    }
    c_free(property);
    path = c_fieldPath(field);
    length = c_arraySize(path);
    list = NULL;
    for (i=(length-1);i>=0;i--) {
        metaName = c_metaName(path[i]);
        list = q_insert(list,q_newId(metaName));
        c_free(metaName);
    }
    c_free(field);
    list = q_insert(list,q_newId("userData"));

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

    kernel = v_objectKernel(t);
    type = v_topicMessageType(t);

    if (t) {
        if (type) {
            if (!resolveFields(type,e)) {
                OS_REPORT_1(OS_ERROR,
                            "kernel::v_filter::v_filterNew",0,
                            "Failed to resolve fields in filter expression."
                            OS_REPORT_NL "Topic = \"%s\"",
                            v_topicName(t));
                filter = NULL;
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

c_bool
v_filterEval(v_filter f, c_object o)
{
    assert(C_TYPECHECK(f,v_filter));
    return c_qPredEval(f->predicate, o);
}

