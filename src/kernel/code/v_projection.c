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

#include "v_projection.h"
#include "v__dataReader.h"
#include "os_heap.h"
#include "os_report.h"
#include "os.h"

static c_char *
getFieldName(
    q_expr fieldExpr)
{
    c_char *name, *str;
    c_long len;

    if (q_isId(fieldExpr)) {
        str = q_getId(fieldExpr);
        len = strlen(str)+1;
        name = (c_char *)os_malloc(len);
        os_strncpy(name,str,len);
        return name;
    } else {
        return q_propertyName(fieldExpr);
    }
}

static c_char *
fieldName(
    c_field field)
{
    c_array path;

    path = c_fieldPath(field);

    return os_strdup(c_metaObject(path[c_arraySize(path)-1])->name);
}

static v_mapping
v_mappingNew(
    v_dataReader reader,
    c_type resultType,
    q_expr ruleExpr)
{
    v_kernel kernel;
    v_mapping rule;
    c_char *resultFieldName, *sourceFieldName;
    c_field resultField, sourceField;

    kernel = v_objectKernel(reader);

    if (q_isFnc(ruleExpr,Q_EXPR_BIND)) {
        /* binding spec of result type field name and source type field name */
        assert(q_getLen(ruleExpr) == 2);
        sourceFieldName = getFieldName(q_getPar(ruleExpr,0));
        resultFieldName = getFieldName(q_getPar(ruleExpr,1));
    } else {
        /* field name, is equal for result and source type */
        sourceFieldName = getFieldName(ruleExpr);
        resultFieldName = NULL;
    }

    sourceField = v_dataReaderIndexField(reader,sourceFieldName);
    os_free(sourceFieldName);

    if (sourceField == NULL) {
        os_free(resultFieldName);
        return NULL;
    }

    if (resultType != NULL) {
        if (resultFieldName == NULL) {
            resultFieldName = fieldName(sourceField);
        }
        if (strcmp(resultFieldName,"userData") == 0) {
            resultField = NULL;
        } else {
            resultField = c_fieldNew(resultType,resultFieldName);
            if (resultField == NULL) {
                os_free(resultFieldName);
                c_free(sourceField);
                return NULL;
            }
        }
    } else {
        resultField = NULL;
    }
    os_free(resultFieldName);

    rule = c_new(v_kernelType(kernel, K_MAPPING));
    if (rule) {
        rule->source = sourceField;
        rule->destination = resultField;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_mappingNew",0,
                  "Failed to allocate mapping rule.");
        assert(FALSE);
    }
    return rule;
}

static c_type
getProjectionType(
    v_kernel kernel,
    q_expr type)
{
    c_metaObject scope, prevScope;
    q_expr term,expr;
    c_long i;

    scope = NULL;

    if (q_getTag(type) != Q_EXPR_CLASS) {
        return NULL;
    }
    term = q_getPar(type,0);
    if (q_getKind(term) == T_TYP) {
        return q_getTyp(term);
    }
    assert(q_getTag(term) == Q_EXPR_SCOPEDNAME);
    prevScope = c_metaObject(c_keep(c_getBase(kernel)));
    i=0;
    while ((expr = q_getPar(term,i)) != NULL) {
        assert(q_isId(expr));
        scope = c_metaResolve(prevScope,q_getId(expr));
        c_free(prevScope);
        prevScope = scope;
        i++;
    }
    assert(C_TYPECHECK(scope,c_type));

    return c_type(scope); /* transfer refCount */
}

#define v_mappingArrayNew(kernel,nrOfRules) \
        ((v_mapping *)c_arrayNew(v_kernelType(kernel,K_MAPPING), nrOfRules))

/**
 *
 * Decsription:
 * Handles the following syntax:
 * a> The default projection:      "*"
 *    Result type is the internal frame type.
 * b> The single field projection: "<field name>"
 *    Result type is the field type.
 * c> The type projection:         "<type name>(<field map list>)"
 *    Result type is the specified type.
 */
v_projection
v_projectionNew(
    v_dataReader reader,
    q_expr projection)
{
    v_kernel kernel;
    v_projection p;
    v_mapping *rules;
    q_expr rule,par;
    c_long i,nrOfRules;
    c_type resultType;
    c_string fieldName;
    c_field field;

    assert(C_TYPECHECK(reader,v_dataReader));
    assert(q_isFnc(projection,Q_EXPR_PROJECTION));

    kernel = v_objectKernel(reader);
    /* list of: field names, bindings field as field or '*' */

    par = q_getPar(projection,0);
    if (par == NULL) {
        /* default mapping */
        rules = NULL;
        resultType = v_dataReaderInstanceType(reader);
    } else {
        switch (q_getKind(par)) {
        case T_FNC:
            switch (q_getTag(par)) {
            case Q_EXPR_CLASS:
                /* object constructor */
                nrOfRules = q_getLen(par)-1;
                resultType = getProjectionType(kernel,par);
                rules = v_mappingArrayNew(kernel,nrOfRules);
                for (i=0;i<nrOfRules;i++) {
                    rule = q_getPar(par,i+1);
                    rules[i] = v_mappingNew(reader,resultType,rule);
                }
            break;
            case Q_EXPR_PROPERTY:
                /* field name */
                fieldName = getFieldName(par);
                nrOfRules = 1;
                field = v_dataReaderIndexField(reader,fieldName);
                resultType = c_fieldType(field);
                c_free(field);
                rules = v_mappingArrayNew(kernel,nrOfRules);
                rules[0] = v_mappingNew(reader,NULL,par);
                os_free(fieldName);
            break;
            default:
                OS_REPORT_1(OS_ERROR,"v_projectionNew failed",0,
                            "illegal mapping kind (%d) specified",
                            q_getTag(par));
                assert(FALSE);
                return NULL;
            }
        break;
        case T_ID:
            /* field name */
            nrOfRules = 1;
            field = v_dataReaderIndexField(reader,q_getId(par));
            resultType = c_fieldType(field);
            c_free(field);
            rules = v_mappingArrayNew(kernel,nrOfRules);
            rules[0] = v_mappingNew(reader,NULL,par);
        break;
        default:
            OS_REPORT_1(OS_ERROR,"v_projectionNew failed",0,
                        "illegal mapping kind (%d) specified",
                        q_getKind(par));
            assert(FALSE);
            return NULL;
        }
    }
    assert(resultType != NULL);

    /*
    p = v_projection(v_objectNew(kernel,K_PROJECTION));
    */
    p = c_new(v_kernelType(kernel, K_PROJECTION));
    if (p) {
        p->rules = (c_array)rules;
        p->resultType = resultType; /* transfer refcount */
    } else {
        OS_REPORT(OS_ERROR,
                  "v_projectionNew",0,
                  "Failed to allocate projection.");
        assert(FALSE);
    }

    return p;
}

c_array
v_projectionRules (
    v_projection p)
{
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_projection));

    return p->rules;
}

c_type
v_projectionType(
    v_projection p)
{
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_projection));

    return p->resultType;
}

c_field
v_projectionSource (
    v_projection p,
    const c_char *fieldName)
{
    v_mapping mapping;
    c_long i, length;

    assert(C_TYPECHECK(p,v_projection));
    assert(fieldName != NULL);

    length = c_arraySize(p->rules);
    for (i=0;i<length;i++) {
        mapping = v_mapping(p->rules[i]);
        if (mapping->destination != NULL) {
            if (strcmp(fieldName,c_fieldName(mapping->destination)) == 0) {
                return mapping->source;
            }
        }
    }
    return NULL;
}
