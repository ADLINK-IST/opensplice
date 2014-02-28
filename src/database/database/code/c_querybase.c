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
#include "os_report.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include "os_heap.h"
#include "c__base.h"
#include "c__metabase.h"
#include "q_expr.h"
#include "c_misc.h"
#include "c_field.h"
#include "c_filter.h"
#include "c__querybase.h"
#include "c_collection.h"

#include <errno.h>

#define c_qConstType(b) \
        (b->baseCache.queryCache.c_qConst_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qConst_t) : \
         c_keep(b->baseCache.queryCache.c_qConst_t = c_resolve((b),"c_querybase::c_qConst")))

#define c_qTypeType(b) \
        (b->baseCache.queryCache.c_qType_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qType_t) : \
         c_keep(b->baseCache.queryCache.c_qType_t = c_resolve((b),"c_querybase::c_qType")))

#define c_qVarType(b) \
        (b->baseCache.queryCache.c_qVar_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qVar_t) : \
         c_keep(b->baseCache.queryCache.c_qVar_t = c_resolve((b),"c_querybase::c_qVar")))

#define c_qFieldType(b) \
        (b->baseCache.queryCache.c_qField_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qField_t) : \
         c_keep(b->baseCache.queryCache.c_qField_t = c_resolve((b),"c_querybase::c_qField")))

#define c_qFuncType(b) \
        (b->baseCache.queryCache.c_qFunc_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qFunc_t) : \
         c_keep(b->baseCache.queryCache.c_qFunc_t = c_resolve((b),"c_querybase::c_qFunc")))

#define c_qPredType(b) \
        (b->baseCache.queryCache.c_qPred_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qPred_t) : \
         c_keep(b->baseCache.queryCache.c_qPred_t = c_resolve((b),"c_querybase::c_qPred")))

#define c_qKeyType(b) \
        (b->baseCache.queryCache.c_qKey_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qKey_t) : \
         c_keep(b->baseCache.queryCache.c_qKey_t = c_resolve((b),"c_querybase::c_qKey")))

#define c_qRangeType(b) \
        (b->baseCache.queryCache.c_qRange_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qRange_t) : \
         c_keep(b->baseCache.queryCache.c_qRange_t = c_resolve((b),"c_querybase::c_qRange")))

#define c_qExprType(b) \
         (b->baseCache.queryCache.c_qExpr_t != NULL ? \
         c_keep(b->baseCache.queryCache.c_qExpr_t) : \
         c_keep(b->baseCache.queryCache.c_qExpr_t = c_resolve((b),"c_querybase::c_qExpr")))

/* returns the head of the iterator as a c_qRange object (non-destructive, i.e. a read, not a take) */
#define c_qRangeIterHead(l) (c_qRange(c_iterObject(l, 0)))

/* a nilRange is a range which contains no elements: e.g. <4..4> */
#define isNilRange(r) \
        (r && \
        (c_valueCompare(r->start, r->end) == C_EQ) && \
        r->startKind == B_EXCLUDE && \
        r->endKind == B_EXCLUDE)

/*
 * Optimizes the predicate and returns the optimized predicate.
 */
static c_qPred
c_qPredOptimize(
        c_qPred _this);

c_filter
c_filterNew(
    c_type type,
    q_expr predicate,
    c_value params[])
{
    c_qPred filter;

    c_qPredNew(type,NULL,predicate,params,(c_qPred *)&filter);
    return (c_filter)filter;
}

c_bool
c_filterEval(
    c_filter f,
    c_object o)
{
    return c_qPredEval((c_qPred)f, o);
}

void
c_querybaseInit(
    c_base base)
{
#define ResolveType(s,typeName) \
        (c_type(c_metaResolve(c_metaObject(s),typeName)))

    c_object scope,module;
    c_object o;
    c_object type;

    module = c_metaDeclare(c_object(base),"c_querybase",M_MODULE);

    o = c_metaDeclare(module,"c_qBoundKind",M_ENUMERATION);
    type = c_object_t(base);
    c_enumeration(o)->elements = c_arrayNew(type,3);
    c_enumeration(o)->elements[0] =
        (c_voidp)c_metaDeclare(module,"B_UNDEFINED",M_CONSTANT);
    c_enumeration(o)->elements[1] =
        (c_voidp)c_metaDeclare(module,"B_INCLUDE",M_CONSTANT);
    c_enumeration(o)->elements[2] =
        (c_voidp)c_metaDeclare(module,"B_EXCLUDE",M_CONSTANT);
    c_metaFinalize(o);
    c_free(o);

    o = c_metaDeclare(module,"c_qKind",M_ENUMERATION);
    c_enumeration(o)->elements = c_arrayNew(type,12);
    c_enumeration(o)->elements[0] =
        (c_voidp)c_metaDeclare(module,"CQ_FIELD",M_CONSTANT);
    c_enumeration(o)->elements[1] =
        (c_voidp)c_metaDeclare(module,"CQ_CONST",M_CONSTANT);
    c_enumeration(o)->elements[2] =
        (c_voidp)c_metaDeclare(module,"CQ_AND",M_CONSTANT);
    c_enumeration(o)->elements[3] =
        (c_voidp)c_metaDeclare(module,"CQ_OR",M_CONSTANT);
    c_enumeration(o)->elements[4] =
        (c_voidp)c_metaDeclare(module,"CQ_EQ",M_CONSTANT);
    c_enumeration(o)->elements[5] =
        (c_voidp)c_metaDeclare(module,"CQ_NE",M_CONSTANT);
    c_enumeration(o)->elements[6] =
        (c_voidp)c_metaDeclare(module,"CQ_LT",M_CONSTANT);
    c_enumeration(o)->elements[7] =
        (c_voidp)c_metaDeclare(module,"CQ_LE",M_CONSTANT);
    c_enumeration(o)->elements[8] =
        (c_voidp)c_metaDeclare(module,"CQ_GT",M_CONSTANT);
    c_enumeration(o)->elements[9] =
        (c_voidp)c_metaDeclare(module,"CQ_GE",M_CONSTANT);
    c_enumeration(o)->elements[10] =
        (c_voidp)c_metaDeclare(module,"CQ_LIKE",M_CONSTANT);
    c_enumeration(o)->elements[11] =
        (c_voidp)c_metaDeclare(module,"CQ_NOT",M_CONSTANT);
    c_metaFinalize(o);
    c_free(o);

    scope = c_metaDeclare(module,"c_qExpr",M_CLASS);
        c_class(scope)->extends = NULL;
        type = ResolveType(scope,"c_qKind");
        C_META_ATTRIBUTE_(c_qExpr,scope,kind,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qConst",M_CLASS);
        c_class(scope)->extends = c_class(c_qExprType(base));
        type = ResolveType(scope,"c_value");
        C_META_ATTRIBUTE_(c_qConst,scope,value,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qType",M_CLASS);
        c_class(scope)->extends = c_class(c_qExprType(base));
        type = ResolveType(scope,"c_type");
        C_META_ATTRIBUTE_(c_qType,scope,type,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qField",M_CLASS);
        c_class(scope)->extends = c_class(c_qExprType(base));
        type = ResolveType(scope,"c_field");
        C_META_ATTRIBUTE_(c_qField,scope,field,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qFunc",M_CLASS);
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_qExpr>");
            c_collectionType(type)->kind = C_ARRAY;
            c_collectionType(type)->subType = c_qExprType(base);
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);

        c_class(scope)->extends = c_qExprType(base);
        C_META_ATTRIBUTE_(c_qFunc,scope,params,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qRange",M_CLASS);
        c_class(scope)->extends = NULL;
        type = ResolveType(scope,"c_qBoundKind");
        C_META_ATTRIBUTE_(c_qRange,scope,startKind,type);
        C_META_ATTRIBUTE_(c_qRange,scope,endKind,type);
        c_free(type);
        type = c_qExprType(base);
        C_META_ATTRIBUTE_(c_qRange,scope,startExpr,type);
        C_META_ATTRIBUTE_(c_qRange,scope,endExpr,type);
        c_free(type);
        type = ResolveType(scope,"c_value");
        C_META_ATTRIBUTE_(c_qRange,scope,start,type);
        C_META_ATTRIBUTE_(c_qRange,scope,end,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qKey",M_CLASS);
        c_class(scope)->extends = NULL;
        type = c_qExprType(base);
        C_META_ATTRIBUTE_(c_qKey,scope,expr,type);
        c_free(type);
        type = ResolveType(scope,"c_field");
        C_META_ATTRIBUTE_(c_qKey,scope,field,type);
        c_free(type);
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_qRange>");
            c_collectionType(type)->kind = C_ARRAY;
            c_collectionType(type)->subType = c_qRangeType(base);
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);
        C_META_ATTRIBUTE_(c_qKey,scope,range,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qPred",M_CLASS);
        c_class(scope)->extends = NULL;
        type = ResolveType(scope,"c_bool");
        C_META_ATTRIBUTE_(c_qPred,scope,fixed,type);
        c_free(type);
        type = c_qExprType(base);
        C_META_ATTRIBUTE_(c_qPred,scope,expr,type);
        c_free(type);
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_qKey>");
            c_collectionType(type)->kind = C_ARRAY;
            c_collectionType(type)->subType = c_qKeyType(base);
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);
        C_META_ATTRIBUTE_(c_qPred,scope,keyField,type);
        c_free(type);
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_qVar>");
            c_collectionType(type)->kind = C_ARRAY;
            c_collectionType(type)->subType = c_qVarType(base);
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);
        C_META_ATTRIBUTE_(c_qPred,scope,varList,type);
        c_free(type);
        type = c_qPredType(base);
        C_META_ATTRIBUTE_(c_qPred,scope,next,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qVar",M_CLASS);
        c_class(scope)->extends = NULL;
        type = ResolveType(scope,"c_bool");
        C_META_ATTRIBUTE_(c_qVar,scope,hasChanged,type);
        c_free(type);
        type = ResolveType(scope,"c_long");
        C_META_ATTRIBUTE_(c_qVar,scope,id,type);
        c_free(type);
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"SET<c_qKey>");
            c_collectionType(type)->kind = C_SET;
            c_collectionType(type)->subType = c_qKeyType(base);
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);
        C_META_ATTRIBUTE_(c_qVar,scope,keys,type);
        c_free(type);
        type = c_qConstType(base);
        C_META_ATTRIBUTE_(c_qVar,scope,variable,type);
        c_free(type);
#if 1
        type = ResolveType(scope,"c_type");
        C_META_ATTRIBUTE_(c_qVar,scope,type,type);
        c_free(type);
#endif
    c__metaFinalize(scope,FALSE);
    c_free(scope);
    c_free(module);

#undef ResolveType
}

#if 0
c_array
c_keyNew(
    c_type type,
    c_iter fieldNames)
{
    c_array key;
    c_long i, j, n;
    c_string fieldName[1000];
    c_type fieldType;
    c_bool redefined;

    if (fieldNames == NULL) {
        return NULL;
    }

    i = 0; n = 0;
    fieldName[n] = c_iterObject(fieldNames,i);
    while (fieldName[n] != NULL) {
        redefined = FALSE;
        for (j=0;j<n;j++) {
            if (strcmp(fieldName[n],fieldName[j]) == 0) {
                redefined = TRUE;
            }
        }
        if (!redefined) n++;
        i++;
        fieldName[n] = c_iterObject(fieldNames,i);
    }

    fieldType = c_resolve(c__getBase(type),"c_qField");
    key = c_arrayNew(fieldType,n);
    c_free(fieldType);

    for (j=0; j<n; j++) {
        key[j] = c_fieldNew(type,fieldName[j]);
        if (key[j] == NULL) {
            OS_REPORT_1(OS_API_INFO,
                        "c_queribase::c_keyNew",0,
                        "Key %s not found",
                        fieldName[j]);
            for (i=0; i<j; i++) c_free(key[i]);
            c_free(key);
            return NULL;
        }
    }
    return key;
}

c_array
c_keyDcl(
    c_type t,
    c_long size,
    ...)
{
    va_list ap;
    c_array key;
    c_string fieldName;
    c_long i;
    c_type fieldType;

    if (size < 1) {
        return NULL;
    }

    fieldType = c_resolve(c__getBase(t),"c_qField");
    key = c_arrayNew(fieldType,size);
    c_free(fieldType);

    va_start(ap,size);
    for (i=0;i<size;i++) {
        fieldName = va_arg(ap,c_string);
        key[i] = c_fieldNew(t,fieldName);
    }
    va_end(ap);

    return key;
}
#endif

c_bool
c_qRangeEval(
    c_qRange r,
    c_value v)
{
#define _START_ (c_valueCompare(c_qRangeStartValue(r),v))
#define _END_   (c_valueCompare(c_qRangeEndValue(r),v))

    switch (r->startKind) {
    case B_UNDEFINED:
       switch (r->endKind) {
       case B_UNDEFINED: return TRUE;
       case B_INCLUDE:   return ( _END_ != C_GT);
       case B_EXCLUDE:   return ( _END_ != C_GE);
       }
    case B_INCLUDE:
       switch (r->endKind) {
       case B_UNDEFINED: return ( _START_ != C_LT );
       case B_INCLUDE:   return ((_START_ != C_GT) && (_END_ != C_LT));
       case B_EXCLUDE:   return ((_START_ != C_GT) && (_END_ == C_GT));
       }
    case B_EXCLUDE:
       switch (r->endKind) {
       case B_UNDEFINED: return ( _START_ != C_LE);
       case B_INCLUDE:   return ((_START_ == C_LT) && (_END_ != C_LT));
       case B_EXCLUDE:   return ((_START_ == C_LT) && (_END_ == C_GT));
       }
    }
    return FALSE;

#undef _END_
#undef _START_
}

typedef void
(*c_qCallback)(
    c_object o,
    c_value argument,
    c_value *result);

static c_value
qExecute(
    c_qExpr e,
    c_object o)
{
    c_qCallback callback;
    c_value argument, result, v;

    assert(e->kind == CQ_CALLBACK);

    v = c_qValue(c_qFunc(e)->params[1],NULL);
    assert(v.kind == V_ADDRESS);
    callback = (c_qCallback)v.is.Address;
    argument = c_qValue(c_qFunc(e)->params[2],o);
    callback(o,argument,&result);
    return result;
}

c_value
c_qValue(
    c_qExpr q,
    c_object o)
{
    c_value v;
    c_value rightValue;
    c_value leftValue;
    c_long i,size;

#define _RVAL_(q,o) \
        (c_qValue(c_qFunc(q)->params[0],o))

#define _LVAL_(q,o) \
        (c_qValue(c_qFunc(q)->params[1],o))

#if 0
#define _CASE_(l,q,o) \
        case CQ_##l: { \
            c_value lv, rv; \
            lv = _LVAL_(q,o); \
            rv = _RVAL_(q,o); \
            return c_valueCalculate(rv,lv,O_##l); \
        }
#else
#define _CASE_(l,q,o) \
        case CQ_##l: \
            rightValue = _RVAL_(q,o);\
            leftValue = _LVAL_(q,o);\
            v = c_valueCalculate(rightValue,leftValue,O_##l);\
            c_valueFreeRef(rightValue);\
            c_valueFreeRef(leftValue);\
            return v
#endif

    switch(q->kind) {
    case CQ_FIELD:
        v = c_fieldValue(c_qField(q)->field,o);
    break;
    case CQ_CONST:
        v = c_valueKeepRef(c_qConst(q)->value);
    break;
    case CQ_TYPE:
       v = c_objectValue(c_qType(q)->type);
    break;
    _CASE_(EQ,q,o);
    _CASE_(NE,q,o);
    _CASE_(LT,q,o);
    _CASE_(LE,q,o);
    _CASE_(GT,q,o);
    _CASE_(GE,q,o);
    case CQ_AND:
        v = _LVAL_(q,o);

        if (v.is.Boolean) {
            v = _RVAL_(q,o);
        }
    break;
    case CQ_OR:
        v = _LVAL_(q,o);

        if (!v.is.Boolean) {
            v = _RVAL_(q,o);
        }
    break;
    case CQ_NOT:
        v = _RVAL_(q,o);

        if (v.is.Boolean) {
            v.is.Boolean = FALSE;
        } else {
            v.is.Boolean = TRUE;
        }
    break;
    case CQ_LIKE:
        leftValue = _LVAL_(q,o);
        rightValue = _RVAL_(q,o);
        v = c_valueStringMatch(leftValue,rightValue);
        c_valueFreeRef(leftValue);
        c_valueFreeRef(rightValue);
        return v;
    case CQ_CALLBACK: return qExecute(q,o);
    default:
        v = c_qValue(c_qFunc(q)->params[0],o);
        size = c_arraySize(c_qFunc(q)->params);
        for (i=1;i<size;i++) {
            leftValue = v;
            rightValue = c_qValue(c_qFunc(q)->params[i],o);
            v = c_valueCalculate(leftValue,rightValue,(c_operator)q->kind);
            c_valueFreeRef(rightValue);
        }
        c_valueFreeRef(leftValue);
    }
#undef _CASE_
#undef _LVAL_
#undef _RVAL_
    return v;
}

static c_equality
c_qRangeCompare(
    c_qRange r1,
    c_qRange r2)
{
    c_equality e1,e2;

#define r1s (r1->startKind != B_UNDEFINED)
#define r1e (r1->endKind != B_UNDEFINED)
#define r2s (r2->startKind != B_UNDEFINED)
#define r2e (r2->endKind != B_UNDEFINED)

    if (r1s && r2e) {
        e1 = c_valueCompare(c_qRangeStartValue(r1),c_qRangeEndValue(r2));
        if (e1 == C_ER) {
            return C_ER;
        }
    } else {
        e1 = C_LT;
    }
    if (r1e && r2s) {
        e2 = c_valueCompare(c_qRangeEndValue(r1),c_qRangeStartValue(r2));
        if (e2 == C_ER) {
            return C_ER;
        }
    } else {
        e2 = C_GT;
    }

    if (e1 == C_GT) {
        return C_GT;
    }
    if (e2 == C_LT) {
        return C_LT;
    }

    if (r1s) {
        if (r2s) {
            e1 = c_valueCompare(c_qRangeStartValue(r1),c_qRangeStartValue(r2));
            if (e1 == C_ER) {
                return C_ER;
            }
        } else {
            e1 = C_GT;
        }
    } else {
        if (r2s) {
            e1 = C_LT;
        } else {
            e1 = C_EQ;
        }
    }

    if (r1e) {
        if (r2e) {
            e2 = c_valueCompare(c_qRangeEndValue(r1),c_qRangeEndValue(r2));
            if (e2 == C_ER) {
                return C_ER;
            }
        } else {
            e2 = C_LT;
        }
    } else {
        if (r2e) {
            e2 = C_GT;
        } else {
            e2 = C_EQ;
        }
    }

    if (e1 == C_EQ) {
        if (r1->startKind != B_EXCLUDE) {
            if (r2->startKind == B_EXCLUDE) e1 = C_LT;
        } else {
            if (r2->startKind != B_EXCLUDE) e1 = C_GT;
        }
    }

    if (e2 == C_EQ) {
        if (r1->endKind != B_EXCLUDE) {
            if (r2->endKind == B_EXCLUDE) e2 = C_GT;
        } else {
            if (r2->endKind != B_EXCLUDE) e2 = C_LT;
        }
    }

    if (e1 == C_LT) {
        if (e2 == C_GT) {
            return C_NE;
        }
        return C_LE;
    }
    if (e1 == C_GT) {
        if (e2 == C_LT) {
            return C_EQ;
        }
        return C_GE;
    }
    if (e2 == C_GT) {
        return C_GE;
    }
    if (e2 == C_LT) {
        return C_LE;
    }
    return C_EQ;
#undef r1s
#undef r1e
#undef r2s
#undef r2e

}

static void
c_setRange(
    c_qRange range,
    c_value start,
    c_qExpr startExpr,
    c_qBoundKind startKind,
    c_value end,
    c_qExpr endExpr,
    c_qBoundKind endKind)
{
    if ((startKind == B_UNDEFINED) || (endKind == B_UNDEFINED)) {
        range->start = start;
        range->end = end;
        range->startExpr = c_keep(startExpr);
        range->endExpr = c_keep(endExpr);
        range->startKind = startKind;
        range->endKind = endKind;
    } else {
        switch(c_valueCompare(start,end)) {
        case C_GT:
            range->start = end;
            range->end = start;
            range->startExpr = c_keep(endExpr);
            range->endExpr = c_keep(startExpr);
            range->startKind = endKind;
            range->endKind = startKind;
        break;
        case C_LT:
        case C_EQ:
            range->start = start;
            range->end = end;
            range->startExpr = c_keep(startExpr);
            range->endExpr = c_keep(endExpr);
            range->startKind = startKind;
            range->endKind = endKind;
        break;
        default: assert(FALSE);
        }
    }
}

static c_iter
c_qRangeListAnd(
    c_iter list1,
    c_iter list2)
{
    c_iter result,l;
    c_qRange r1, r2, r;
    c_equality eq;

    if (list1 == NULL) {
        return list2;
    }

    if (c_iterLength(list1) == 0) {
        return list2;
    }
    if (c_iterLength(list2) == 0) {
        return list1;
    }

    r1 = c_iterTakeFirst(list1);
    r2 = c_iterTakeFirst(list2);
    result = NULL;

    while (TRUE) {
        eq = c_qRangeCompare(r1,r2);
        switch (eq) {
        case C_LT:
            /* in this case the range is such that no value could ever satisfy it
               e.g. [*..5] AND [10..*] ~~> no value satisfies both ranges
               so the result is <x>, i.e. a list which no variable can satisfy */
            c_setRange(r1,r1->start,r1->startExpr,B_EXCLUDE,
                          r1->start,r2->startExpr,B_EXCLUDE);
            result = c_iterAppend(result,c_keep(r1));
            while (r1 != NULL) {
                c_free(r1);
                r1 = c_iterTakeFirst(list1);
            }
            c_iterFree(list1);
            while (r2 != NULL) {
                c_free(r2);
                r2 = c_iterTakeFirst(list2);
            }
            c_iterFree(list2);
            return result;
        break;
        case C_LE:
            c_setRange(r1,r2->start,r2->startExpr,r2->startKind,
                          r1->end,  r1->endExpr,  r1->endKind);
            result = c_iterAppend(result,r1);
            r1 = c_iterTakeFirst(list1);
        break;
        case C_EQ:
            result = c_iterAppend(result,r1);
            r1 = c_iterTakeFirst(list1);
        break;
        case C_NE:
        case C_GE:
        case C_GT:
            r = r1; r1 = r2; r2 = r;
            l = list1; list1 = list2; list2 = l;
        break;
        default: assert(FALSE);
        }
        if (r1 == NULL) {
            c_iterFree(list1);
            while (r2 != NULL) {
                c_free(r2);
                r2 = c_iterTakeFirst(list2);
            }
            c_iterFree(list2);
            return result;
        }
    }
}

static c_value
getValue(
    c_qExpr e)
{
    c_value v;

    switch(e->kind) {
    case CQ_CONST:
        return c_qConst(e)->value;
    default:
        v = c_undefinedValue();
    }
    return v;
}

static c_iter
makeRange(
    c_qExpr e)
{
    c_valueKind kind;
    c_qRange range;
    c_qBoundKind startKind, endKind;
    c_qField field;
    c_qExpr valueExpr;
    c_value v;
    c_iter rangeList;
    c_bool inverse;

    startKind = B_UNDEFINED;
    endKind = B_UNDEFINED;
    rangeList = NULL;

#define _LEFT_PARAM_(e)  (c_qExpr(c_qFunc(e)->params[0]))
#define _RIGHT_PARAM_(e) (c_qExpr(c_qFunc(e)->params[1]))

    switch (e->kind) {
    case CQ_EQ:
    case CQ_NE:
    case CQ_LT:
    case CQ_LE:
    case CQ_GT:
    case CQ_GE:
        if (_LEFT_PARAM_(e)->kind == CQ_FIELD) {
            if (_RIGHT_PARAM_(e)->kind == CQ_FIELD) {
                return NULL;
            }
            field = c_qField(_LEFT_PARAM_(e));
            valueExpr = _RIGHT_PARAM_(e);
            inverse = FALSE;
        } else if (_RIGHT_PARAM_(e)->kind == CQ_FIELD) {
            field = c_qField(_RIGHT_PARAM_(e));
            valueExpr = _LEFT_PARAM_(e);
            inverse = TRUE;
        } else {
            return NULL;
        }
        v = getValue(valueExpr);
        if (v.kind == V_UNDEFINED) {
            return NULL;
        }
        kind = c_fieldValueKind(field->field);
        v = c_valueCast(v,kind);

        if (v.kind == V_UNDEFINED) {
            return NULL;
        }

        switch (e->kind) {
        case CQ_NE:
            range = c_new(c_qRangeType(c__getBase(e)));
            if (range) {
                range->startKind = B_UNDEFINED;
                range->endKind   = B_EXCLUDE;
                range->startExpr = c_keep(valueExpr);
                range->endExpr = c_keep(valueExpr);
                range->end   = v;
                range->start = v;
                rangeList = c_iterNew(range);
            } else {
                assert(FALSE);
            }
            range = c_new(c_qRangeType(c__getBase(e)));
            if (range) {
                range->startKind = B_EXCLUDE;
                range->endKind   = B_UNDEFINED;
                range->startExpr = c_keep(valueExpr);
                range->endExpr = c_keep(valueExpr);
                range->end   = v;
                range->start = v;
                rangeList = c_iterAppend(rangeList,range);
            } else {
                assert(FALSE);
            }

            /* In case v is a value of a reference type (string or object)
             * it should have been c_keeped when assigned to the 'start' and
             * 'end' attributes of the two ranges above.
             * to avoid a lot of if statements the following correction is
             * performed:
             */
            if (v.kind == V_STRING) {
                c_keep(c_keep(c_keep(c_keep(v.is.String))));
            }
            if (v.kind == V_OBJECT) {
                c_keep(c_keep(c_keep(c_keep(v.is.Object))));
            }

            return rangeList;
        case CQ_EQ: startKind = B_INCLUDE;   endKind = B_INCLUDE;   break;
        case CQ_LT: startKind = B_UNDEFINED; endKind = B_EXCLUDE;   break;
        case CQ_LE: startKind = B_UNDEFINED; endKind = B_INCLUDE;   break;
        case CQ_GT: startKind = B_EXCLUDE;   endKind = B_UNDEFINED; break;
        case CQ_GE: startKind = B_INCLUDE;   endKind = B_UNDEFINED; break;
        default:
            assert(FALSE);
            return NULL;
        }
        range = c_new(c_qRangeType(c__getBase(e)));
        if (range) {
            if (inverse) {
                range->startKind = endKind;
                range->endKind   = startKind;
            } else {
                range->startKind = startKind;
                range->endKind   = endKind;
            }

            range->startExpr = c_keep(valueExpr);
            range->endExpr = c_keep(valueExpr);
            range->end   = v;
            range->start = v;

            if (v.kind == V_STRING) {
                c_keep(c_keep(v.is.String));
            }
            if (v.kind == V_OBJECT) {
                c_keep(c_keep(v.is.Object));
            }
        } else {
            assert(FALSE);
        }
        return c_iterNew(range);
    default:
    break;
    }
    return NULL;
#undef _LEFT_PARAM_
#undef _RIGHT_PARAM_
}

static c_iter
makeRangeQuery(
    c_qExpr *expr)
{
    c_qExpr *leftPar,*rightPar;
    c_iter leftList,rightList;
    c_qExpr e;
    c_iter rangeList;

    if (expr == NULL) {
        return NULL;
    }
    e = *expr;
    if (e == NULL) {
        return NULL;
    }
    switch (e->kind) {
    case CQ_AND:
        leftPar = (c_qExpr *)(&c_qFunc(e)->params[0]);
        rightPar = (c_qExpr *)(&c_qFunc(e)->params[1]);
        leftList  = makeRangeQuery(leftPar);
        rightList = makeRangeQuery(rightPar);
        if((leftList && isNilRange(c_qRangeIterHead(leftList))) ||
           (rightList && isNilRange(c_qRangeIterHead(rightList)))){
            return (leftList && isNilRange(c_qRangeIterHead(leftList))) ? leftList : rightList;
        }
        if (leftList == NULL) {
            if (rightList != NULL) {
                *expr = c_keep(*leftPar);
                c_free(e);
            }
            return rightList;
        } else {
            if (rightList == NULL) {
                *expr = c_keep(*rightPar);
                c_free(e);
                return leftList;
            }
            *expr = NULL;
            c_free(e);
            return c_qRangeListAnd(leftList,rightList);
        }
    case CQ_OR:
        assert(FALSE);
    break;
    default:
        rangeList = makeRange(e);
        if (rangeList != NULL) {
            c_free(e);
            *expr = NULL;
            return rangeList;
        } else {
            return NULL;
        }
    }
    return NULL;
}

static c_equality
c_qVarCompare(
    c_qVar e,
    c_long *id)
{
    if (e->id < *id) {
        return C_LT;
    }
    if (e->id > *id) {
        return C_GT;
    }
    return C_EQ;
}

/* If the given type is an enumeration then the expression is examined.
   If the expression is a string constant then it is the enum label and
   will be transformed into the enum value.
*/
static void
transformEnumLabelToValue(
    c_type type,
    c_qExpr expr)
{
    c_type t;
    c_literal l;

    t = c_typeActualType(type);
    switch (c_baseObject(t)->kind) {
    case M_ENUMERATION:
        if (expr->kind == CQ_CONST) {
            if (c_qConst(expr)->value.kind == V_STRING) {
                /* Transform string value into int value */
                l = c_enumValue(c_enumeration(t),
                                c_qConst(expr)->value.is.String);
                c_free(c_qConst(expr)->value.is.String);
                c_qConst(expr)->value = l->value;
                c_free(l);
            }
        }
    break;
    case M_PRIMITIVE:
        if (c_primitive(t)->kind == P_BOOLEAN) {
            if (expr->kind == CQ_CONST) {
                if (c_qConst(expr)->value.kind == V_STRING) {
                    /* Transform string value into int value */
                    if (os_strncasecmp(c_qConst(expr)->value.is.String,
                                              "true",5) == 0) {
                        c_free(c_qConst(expr)->value.is.String);
                        c_qConst(expr)->value = c_boolValue(TRUE);
                    } else if (os_strncasecmp(c_qConst(expr)->value.is.String,
                                              "false",6) == 0) {
                        c_free(c_qConst(expr)->value.is.String);
                        c_qConst(expr)->value = c_boolValue(FALSE);
                    } else {
                         OS_REPORT_1(OS_API_INFO,
                                     "c_querybase",0,
                                     "expected boolean value instead of \"%s\".",
                                     c_qConst(expr)->value.is.String);
                    }
                }
            }
        }
    break;
    default:
        /* nothing to do here */
    break;
    }
}

/* this is a generic optimise method that will be executed on all node or
 * fnc expression as soon as they are completed. This function will analyse
 * the node parameters and e.g. determine if enum labels exist that must be
 * resolved. Also in future extensions of this code the resulting type of
 * the expression can be determined and cached. This last optimisation
 * enables detailed error reports.
 */
static void
optimizeExpr(
    c_qExpr e,
    c_bool *fixed)
{
    c_qExpr p1,p2;
    c_type type, t1, t2;

    OS_UNUSED_ARG(fixed);

    p1 = c_qFunc(e)->params[0];
    p2 = c_qFunc(e)->params[1];
    if ((p1->kind == CQ_FIELD) && (p2->kind == CQ_FIELD)) {
        t1 = c_fieldType(c_qField(p1)->field);
        t2 = c_fieldType(c_qField(p2)->field);
        if (t1 != t2) {
            OS_REPORT_2(OS_WARNING,
                "c_querybase::optimizeExpr", 0,
                "Detected inclompatible types between "
                "field <%s> and field <%s>",
                c_fieldName(c_qField(p1)->field),
                c_fieldName(c_qField(p2)->field));
        }
        c_free(t1);
        c_free(t2);
    }
    if (p1->kind == CQ_FIELD) {
        type = c_fieldType(c_qField(p1)->field);
        transformEnumLabelToValue(type,p2);
        c_free(type);
    }
    if (p2->kind == CQ_FIELD) {
        type = c_fieldType(c_qField(p2)->field);
        transformEnumLabelToValue(type,p1);
        c_free(type);
    }
}

/* Note: this method will potentinally destroy the given expression */
/* Note: this method will probably become superfluous because nots
         should be removed earlier */
static c_qExpr
c_qNot(
    c_qExpr e)
{
    c_qExpr r;
    switch (e->kind) {
    case CQ_LIKE:                  r = e; break;
    case CQ_EQ:   e->kind = CQ_NE; r = e; break;
    case CQ_NE:   e->kind = CQ_EQ; r = e; break;
    case CQ_LT:   e->kind = CQ_GE; r = e; break;
    case CQ_LE:   e->kind = CQ_GT; r = e; break;
    case CQ_GT:   e->kind = CQ_LE; r = e; break;
    case CQ_GE:   e->kind = CQ_LT; r = e; break;
    case CQ_NOT:
        r = c_keep(c_qFunc(e)->params[0]);
        c_free(e);
    break;
    case CQ_AND:
        e->kind = CQ_OR;
        c_qFunc(e)->params[0] = c_qNot(c_qFunc(e)->params[0]);
        c_qFunc(e)->params[1] = c_qNot(c_qFunc(e)->params[1]);
        r = e;
    break;
    case CQ_OR:
        e->kind = CQ_AND;
        c_qFunc(e)->params[0] = c_qNot(c_qFunc(e)->params[0]);
        c_qFunc(e)->params[1] = c_qNot(c_qFunc(e)->params[1]);
        r = e;
    break;
    default:
        OS_REPORT_1(OS_API_INFO,
                    "c_querybase::c_qNot",0,
                    "Unknown operator <%d>",
                    (int)e->kind);
        assert(FALSE);
        r = e;
    break;
    }
    return r;
}

static c_qExpr
makeExprQuery(
    q_expr e,
    c_type t,
    c_iter *varList,
    c_bool *fixed)
{
    c_qExpr r = NULL;
    c_qExpr c = NULL;
    c_qVar var;
    c_field f;
    c_long id;
    c_base base;
    c_long i;
    q_expr p;
    c_char qn[1024];

    if (e == NULL) {
        return NULL;
    }

#define _CASE_(eq) \
        case Q_EXPR_##eq: \
            r = c_qExpr(c_new(c_qFuncType(base))); \
            if (r) { \
                r->kind=CQ_##eq; \
                c_qFunc(r)->params = c_arrayNew(c_object_t(base),2); \
                c_qFunc(r)->params[0] = makeExprQuery(q_leftPar(e),t,varList,fixed); \
                c_qFunc(r)->params[1] = makeExprQuery(q_rightPar(e),t,varList,fixed); \
            }

    base = c__getBase(t);
    switch (q_getKind(e)) {
    case T_FNC:
        switch (q_getTag(e)) {
        _CASE_(AND);                         break;
        _CASE_(OR);                          break;
        _CASE_(LIKE); optimizeExpr(r,fixed); break;
        _CASE_(EQ);   optimizeExpr(r,fixed); break;
        _CASE_(NE);   optimizeExpr(r,fixed); break;
        _CASE_(LT);   optimizeExpr(r,fixed); break;
        _CASE_(LE);   optimizeExpr(r,fixed); break;
        _CASE_(GT);   optimizeExpr(r,fixed); break;
        _CASE_(GE);   optimizeExpr(r,fixed); break;
        case Q_EXPR_NOT:
            r = makeExprQuery(q_getPar(e,0),t,varList,fixed);
            r = c_qNot(r);
        break;
        case Q_EXPR_PROPERTY:
            i=0; qn[0]=0;
            while ((p = q_getPar(e,i)) != NULL) {
                if (i!=0) os_strcat(qn,".");
                os_strcat(qn,q_getId(p));
                i++;
            }
            f = c_fieldNew(t,qn);
            if (f == NULL) {
                r = c_qExpr(c_new(c_qConstType(base)));
                if (r) {
                    r->kind = CQ_CONST;
                    c_qConst(r)->value = c_stringValue(c_stringNew(base,qn));
                } else {
                    assert(FALSE);
                }
            } else {
                r = c_qExpr(c_new(c_qFieldType(base)));
                if (r) {
                    r->kind = CQ_FIELD;
                    c_qField(r)->field = f;
                } else {
                    assert(FALSE);
                }
            }
        break;
        case Q_EXPR_CALLBACK:
            r = c_qExpr(c_new(c_qFuncType(base)));
            if (r) {
                r->kind = CQ_CALLBACK;
                c_qFunc(r)->params = c_arrayNew(c_object_t(base),3);
                c = c_qExpr(c_new(c_qTypeType(base)));
                if (c) {
                    c->kind = CQ_TYPE;
                    c_qType(c)->type = c_keep(q_getTyp(q_getPar(e,0)));
                }
                c_qFunc(r)->params[0] = c;
                c = c_qExpr(c_new(c_qConstType(base)));
                if (c) {
                    c->kind = CQ_CONST;
                    c_qConst(c)->value.kind = V_ADDRESS;
                    c_qConst(c)->value.is.Address = (c_address)q_getPar(e,1);
                }
                c_qFunc(r)->params[1] = c;
                c_qFunc(r)->params[2] = makeExprQuery(q_getPar(e,2),t,varList,fixed);
            } else {
                assert(FALSE);
            }
        break;
        default:
            OS_REPORT_1(OS_API_INFO,
                        "c_querybase::makeExprQuery",0,
                        "Unknown operator <%d>",
                        (int)q_getTag(e));
            assert(FALSE);
            r = NULL;
        }
#undef _CASE_
    break;
    case T_INT:
        r = c_qExpr(c_new(c_qConstType(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_LONGLONG;
            c_qConst(r)->value.is.LongLong = q_getInt(e);
        } else {
            assert(FALSE);
        }
    break;
    case T_DBL:
        r = c_qExpr(c_new(c_qConstType(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_DOUBLE;
            c_qConst(r)->value.is.Double = q_getDbl(e);
        } else {
            assert(FALSE);
        }
    break;
    case T_CHR:
        r = c_qExpr(c_new(c_qConstType(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_CHAR;
            c_qConst(r)->value.is.Char = q_getChr(e);
        } else {
            assert(FALSE);
        }
    break;
    case T_STR:
        r = c_qExpr(c_new(c_qConstType(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_STRING;
            c_qConst(r)->value.is.String = c_stringNew(base,q_getStr(e));
        } else {
            assert(FALSE);
        }
    break;
    case T_ID:
        f = c_fieldNew(t,q_getId(e));
        if (f != NULL) {
            r = c_qExpr(c_new(c_qFieldType(base)));
            if (r) {
                r->kind = CQ_FIELD;
                c_qField(r)->field = f;
            } else {
                assert(FALSE);
            }
        } else {
            r = NULL;
        }
    break;
    case T_VAR:
        id = q_getVar(e);
        var = c_iterResolve(*varList,(c_iterResolveCompare)c_qVarCompare,&id);
        if (var == NULL) {
            r = c_qExpr(c_new(c_qConstType(base)));
            if (r) {
                r->kind = CQ_CONST;
                c_qConst(r)->value = c_undefinedValue();
                var = c_qVar(c_new(c_qVarType(base)));
#if 1
                var->type = NULL;
#endif
                var->id = id;
                var->keys = c_setNew(c_qKeyType(base));
                var->variable = c_qConst(c_keep(r));
                *varList = c_iterAppend(*varList,var);
            } else {
                assert(FALSE);
            }
        } else {
            r = c_keep(c_qExpr(var->variable));
        }
        return r;
    case T_TYP:
        r = c_qExpr(c_new(c_qTypeType(base)));
        if (r) {
            r->kind = CQ_TYPE;
            c_qType(r)->type = q_getTyp(e);
        } else {
            assert(FALSE);
        }
    break;
    default:
        OS_REPORT_1(OS_API_INFO,
                    "c_querybase::makeExprQuery",0,
                    "Unknown term type <%d>",
                    (int)q_getKind(e));
        assert(FALSE);
        r = NULL;
    break;
    }
    return r;
}

static c_bool
optimizeKey(
    c_qKey key)
{
    c_iter rangeList;
    c_long i,nrOfRanges;
    c_bool fixed = FALSE;
    c_qRange range,r1,r2;

    rangeList = makeRangeQuery(&key->expr);
    if (rangeList == NULL) {
        return FALSE;
    }
    nrOfRanges = c_iterLength(rangeList);
    key->range = c_arrayNew(c_qRangeType(c__getBase(key)),nrOfRanges);
    for (i=0;i<nrOfRanges;i++) {
        key->range[i] = c_iterTakeFirst(rangeList);
    }
    c_iterFree(rangeList);
    /* if at most one open ended range exists then query is not fixed.
     * So parameters can be set dynamically without reevaluation of the
     * expression.
     */
    if (nrOfRanges > 2) {
        fixed = TRUE;
    } else if (nrOfRanges == 1) {
        range = c_qRange(key->range[0]);
        fixed = ((c_valueCompare(range->start,range->end) != C_EQ) &&
                 (range->startKind != B_UNDEFINED) &&
                 (range->endKind != B_UNDEFINED));
    } else if (nrOfRanges == 2) {
        r1 = c_qRange(key->range[0]);
        r2 = c_qRange(key->range[1]);
        if ((c_valueCompare(r1->start,r1->end) == C_EQ) &&
            (c_valueCompare(r2->start,r2->end) == C_EQ) &&
            (c_valueCompare(r1->start,r2->end) == C_EQ)) {
            if (((r1->startKind == B_UNDEFINED) &&
                 (r1->endKind == B_EXCLUDE) &&
                 (r2->startKind == B_EXCLUDE) &&
                 (r2->endKind == B_UNDEFINED)) ||
                ((r2->startKind == B_UNDEFINED) &&
                 (r2->endKind == B_EXCLUDE) &&
                 (r1->startKind == B_EXCLUDE) &&
                 (r1->endKind == B_UNDEFINED))) {
                fixed = FALSE;
            } else {
                fixed = TRUE;
            }
        } else {
            fixed = TRUE;
        }
    } else {
        fixed = FALSE;
    }
    return fixed;
}

static c_qKey
makeKeyQuery (
    q_expr *term,
    c_type type,
    c_field field,
    c_iter *varList,
    c_bool *fixed)
{
    c_base base;
    q_expr keyExpr;
    c_qKey key, foundKey;
    c_iter KeyVarList = NULL;
    c_qVar var, foundVar;
    c_valueKind valueKind;

    valueKind = c_fieldValueKind(field);
    keyExpr = q_takeField(term,c_fieldName(field));
    base = c__getBase(field);
    key = c_new(c_qKeyType(base));
    if (key) {
        key->field = c_keep(field);
        key->expr = makeExprQuery(keyExpr,type,&KeyVarList,fixed);
        while ((var = c_iterTakeFirst(KeyVarList)) != NULL) {
            foundVar = c_iterResolve(*varList,(c_iterResolveCompare)c_qVarCompare,&var->id);
            if (foundVar != NULL) {
                foundKey = c_insert(foundVar->keys,key);
                assert(foundKey == key);
                c_free(var);
            } else {
#if 1
                c_type t;
                t = c_typeActualType(c_fieldType(field));
                assert(var->type == NULL);
                var->type = NULL;
                switch (c_baseObject(t)->kind) {
                case M_ENUMERATION:
                    var->type = c_keep(t);
                break;
                case M_PRIMITIVE:
                    if (c_primitive(t)->kind == P_BOOLEAN) {
                        var->type = c_keep(t);
                    }
                break;
                default:
                break;
                }
#endif
                c_qConst(var->variable)->value.kind = valueKind;
                *varList = c_iterAppend(*varList,var);
                foundKey = c_insert(var->keys,key);
                assert(foundKey == key);
            }
        }
        c_iterFree(KeyVarList);
    } else {
        assert(FALSE);
    }
    q_dispose(keyExpr);
    return key;
}

#if 1

static c_qVar
lookupVariable(
    c_array varList,
    c_qExpr e)
{
    c_long i,length;

    if (e->kind != CQ_CONST) {
        return NULL;
    }
    length = c_arraySize(varList);
    for (i=0;i<length;i++) {
        if (c_qVar(varList[i])->variable == c_qConst(e)) {
            return c_qVar(varList[i]);
        }
    }
    return NULL;
}

static c_valueKind
c_qVarInit(
    c_qVar var,
    c_qExpr expr,
    c_valueKind kind,
    c_value params[])
{
    c_value v,parValue;
    c_base base;
    c_short s;
    c_type type;
    c_literal l;
    c_string metaName;

    if (var == NULL) {
        return V_UNDEFINED;
    }
    base = c__getBase(var);
    parValue.kind = kind;
    v = params[var->id];

    switch (v.kind) {
    case V_STRING:
        switch (kind) {
        case V_LONG:
            if (expr->kind == CQ_FIELD) {
                type = c_typeActualType(c_fieldType(c_qField(expr)->field));
                if (c_baseObject(type)->kind == M_ENUMERATION) {
                    l = c_enumValue(c_enumeration(type),v.is.String);
                    if (l != NULL) {
                        if (var->type == NULL) {
                            var->type = c_keep(type);
                        } else {
                            assert(var->type == type);
                        }
                        parValue = l->value;
                        c_free(l);
                    } else {
                        metaName = c_metaName(c_metaObject(type));
                        OS_REPORT_2(OS_ERROR,
                                    "Database predicate",0,
                                    "Value <%s> not part of Enum <%s>",
                                    v.is.String,
                                    metaName);
                        c_free(metaName);
                        parValue.kind = V_UNDEFINED;
                    }
                } else {
                    if (!sscanf(v.is.String,"%d",&parValue.is.Long)) {
                        parValue.kind = V_UNDEFINED;
                    }
                }
            } else {
                if (!sscanf(v.is.String,"%d",&parValue.is.Long)) {
                    parValue.kind = V_UNDEFINED;
                }
            }
        break;
        case V_ULONG:
            if (!sscanf(v.is.String,"%u",&parValue.is.ULong)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_OCTET:
            if (!sscanf(v.is.String,"%hd",&s)) {
                parValue.kind = V_UNDEFINED;
            }
            parValue.is.Octet = (c_octet)s;
        break;
        case V_SHORT:
            if (!sscanf(v.is.String,"%hd",&parValue.is.Short)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_USHORT:
            if (!sscanf(v.is.String,"%hu",&parValue.is.UShort)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_FLOAT:
            if (!sscanf(v.is.String,"%f",&parValue.is.Float)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_DOUBLE:
            if (!sscanf(v.is.String,"%lf",&parValue.is.Double)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_LONGLONG:
            parValue.is.LongLong = os_atoll (v.is.String);
            if (errno) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_ULONGLONG:
            parValue.is.LongLong = os_atoll (v.is.String);
            if (errno) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_OBJECT:
            if (!sscanf(v.is.String,"%p",&parValue.is.Object)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_BOOLEAN:
            if (os_strncasecmp(v.is.String,"TRUE",5) == 0) {
                parValue.is.Boolean = TRUE;
            } else if (os_strncasecmp(v.is.String,"FALSE",6) == 0) {
                parValue.is.Boolean = FALSE;
            } else {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_CHAR:
            if (v.is.String != NULL) {
                parValue.is.Char = *v.is.String;
            } else {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_STRING:
        default:
            parValue.is.String = c_stringNew(base,v.is.String);
        break;
        }
    break;
    case V_OBJECT:
        c_keep(v.is.Object);
        parValue = v;
    break;
    default:
        parValue = c_valueCast(v,kind);
    break;
    }
    if (parValue.kind == V_UNDEFINED) {
        OS_REPORT_1(OS_ERROR,
                    "Database predicate",0,
                    "Set query parameter[%d] failed",
                    var->id);
    }
    c_qConst(var->variable)->value = parValue;
    return parValue.kind;
}

static c_valueKind
combinedKind(
    c_valueKind k1,
    c_valueKind k2)
{
    if (k1 == k2) {
        return k1;
    }
    if (k1 == V_UNDEFINED) {
        return k2;
    }
    if (k2 == V_UNDEFINED) {
        return k1;
    }
    if (k1 == V_STRING) {
        return k2;
    }
    if (k2 == V_STRING) {
        return k1;
    }
    /* The rest of all combinations are not specified for not yet being used. */
    return V_UNDEFINED;
}

static c_valueKind
c_qExprInitVars (
    c_qExpr e,
    c_array varList,
    c_value params[])
{
    if (e == NULL) {
        return V_UNDEFINED;
    }
    switch (e->kind) {
    case CQ_AND:
    case CQ_OR:
    case CQ_EQ:
    case CQ_NE:
    case CQ_LT:
    case CQ_LE:
    case CQ_GT:
    case CQ_GE:
    case CQ_LIKE:
    {
        c_valueKind k1,k2,kr;
        c_qExpr e1,e2;
        c_qVar var;

        e1 = c_qFunc(e)->params[0];
        e2 = c_qFunc(e)->params[1];
        k1 = c_qExprInitVars (e1,varList,params);
        k2 = c_qExprInitVars (e2,varList,params);
        /* Determine the combined functions result type. */
        kr = combinedKind(k1,k2);
        if (kr == V_UNDEFINED) {
            /* These kinds cannot be combined. */
            return V_UNDEFINED;
        }
        /* If the parameter is an expression variable then set its value. */
        var = lookupVariable(varList,e1);
        if (var != NULL) {
            k1 = c_qVarInit(var,e2,kr,params);
        }
        /* If the parameter is an expression variable then set its value. */
        var = lookupVariable(varList,e2);
        if (var != NULL) {
            k2 = c_qVarInit(var,e1,kr,params);
        }

        if (k1 == k2) {
            /* values are equal, no translation required. */
            return k1;
        }
        /* return the combined set argument result. */
        return combinedKind(k1,k2);

    }
    case CQ_NOT:
        return c_qExprInitVars (c_qFunc(e)->params[0],varList,params);
    case CQ_CALLBACK:
    {
        c_value v;
        c_valueKind result;
        c_qVar var;

        /* PARAMS: [0]=RESULT-TYPE; [1]=FUNCTION-POINTER; [2]=FUNCTION-ARGUMENT. */

        var = lookupVariable(varList,c_qFunc(e)->params[2]);
        if (var != NULL) {
            result = c_qVarInit(var,NULL,V_UNDEFINED,params);

        }
        v = c_qValue(c_qFunc(e)->params[0],NULL);
        assert(v.kind == V_OBJECT);
        result = c_metaValueKind(v.is.Object);
        assert(result == V_BOOLEAN);
        return result;
    }
    case CQ_FIELD:
        return c_fieldValueKind(c_qField(e)->field);
    case CQ_CONST:
        return c_qConst(e)->value.kind;
    default:
        OS_REPORT(OS_ERROR,
                  "Database predicate",0,
                  "Illegal expression kind");
        assert(FALSE);
    break;
    }
    return V_UNDEFINED;
}

#endif

static c_bool
c_qPredInitVars (
    c_qPred p,
    c_value params[])
{
    c_long i,nrOfKeys;
    c_qPred orExpr;

    if (p == NULL) {
        OS_REPORT(OS_WARNING,
                  "Database predicate",0,
                  "No predicate specified");
    }
    if (p->varList == NULL) {
        return TRUE;
    }
    if (params == NULL) {
        OS_REPORT(OS_ERROR,
                  "Database predicate",0,
                  "No arguments specified");
        return FALSE;
    }
    orExpr = p;
    nrOfKeys = c_arraySize(p->keyField);
    while (orExpr != NULL) {
        if (orExpr->expr != NULL) {
            c_qExprInitVars (orExpr->expr,
                                    p->varList,
                                    params);
        }
        for (i=0;i<nrOfKeys;i++) {
            c_qExprInitVars (c_qKey(orExpr->keyField[i])->expr,
                                    p->varList,
                                    params);
        }
        orExpr = orExpr->next;
    }
    return TRUE;
}

c_qResult
c_qPredNew(
    c_type type,
    c_array keyList,
    q_expr pred,
    c_value params[],
    c_qPred *qPred)
{
    c_base base;
    q_expr e,term;
    c_long k,nrOfKeys;
    c_qPred p,*ptr;
    c_long i,nrOfVars;
    c_iter varList = NULL;
    c_bool fixed;

    base = c__getBase(type);
    if ((pred == NULL) || (qPred == NULL)) {
        qPred = NULL;
        OS_REPORT(OS_API_INFO,
                  "c_querybase::c_qPredNew",0,
                  "predicate == <NULL>");
        return CQ_RESULT_PRECOND_FAIL;
    }
    PRINT_EXPR("Predicate:\n",pred);

    e = q_takePar(pred,0);
    if (q_takePar(pred,0) != NULL) {
        OS_REPORT(OS_API_INFO,
                  "c_querybase::c_qPredNew",0,
                  "predicate has unknown extension");
        return CQ_RESULT_PRECOND_FAIL;
    }
    PRINT_EXPR("Par:\n",e);

    if (e == NULL) {
        /* empty condition => always TRUE (equals qPred == NULL) */
        *qPred = NULL;
        return CQ_RESULT_OK;
    }
    if (keyList == NULL) {
        nrOfKeys = 0;
    } else {
        nrOfKeys = c_arraySize(keyList);
    }
    fixed = FALSE;
    if (nrOfKeys == 0) {
        p = c_new(c_qPredType(base));
        if (p) {
            p->keyField = NULL;
            p->expr = makeExprQuery(e,type,&varList,&fixed);
            p->next = NULL;
            q_dispose(e);
        } else {
            assert(FALSE);
        }
    } else {
        q_disjunctify(e);
        PRINT_EXPR("Par (disjunctified):\n",e);

        ptr = &p;
        p = NULL;
        while ((term = q_takeTerm(&e)) != NULL) {
            *ptr = c_new(c_qPredType(base));
            if (*ptr) {
                (*ptr)->keyField = c_arrayNew(c_qKeyType(base),nrOfKeys);
                for (k=0;k<nrOfKeys;k++) {
                    (*ptr)->keyField[k] = makeKeyQuery(&term,
                                                       type,
                                                       keyList[k],
                                                       &varList,
                                                       &fixed);
                }
                (*ptr)->expr = makeExprQuery(term,type,&varList,
                                                       &fixed);
                (*ptr)->next = NULL;
                ptr = &(*ptr)->next;
                q_dispose(term);
            } else {
                assert(FALSE);
            }
        }
        assert(e == NULL);
        PRINT_PRED("Predicate:\n",p);
    }

    nrOfVars = c_iterLength(varList);
    p->varList = c_arrayNew(c_qVarType(base),nrOfVars);
    for (i=0;i<nrOfVars;i++) {
        p->varList[i] = c_iterTakeFirst(varList);
    }
    c_iterFree(varList);

    if (!c_qPredInitVars(p,params)) {
        c_free(p);
        return CQ_RESULT_PRECOND_FAIL;
    }

    PRINT_PRED("Predicate (vars resolved):\n",p);
    if (p->keyField != NULL) {
        assert(c_refCount(p->keyField) == 1);
    }
    p->fixed = fixed;
    *qPred = c_qPredOptimize(p);
    return CQ_RESULT_OK;
}

static c_qPred
c_qPredOptimize(
        c_qPred _this)
{
#define nextPred(p) (p ? p->next : NULL)
    c_long k,nrOfKeys;
    c_qPred pred;
    c_qPred next;
    c_qPred predPrev = NULL;
    c_qPred resultPred;
    c_bool delete;

    /*
     * Note: predicate _this is the first in a list of predicates which together
     * are AND'ed (i.e. _this AND _this->next AND _this->next->next AND ... etc.
     */

    if(_this == NULL)
    {
        return NULL;
    }

    pred = _this;
    next = nextPred(pred);
    resultPred = _this;

    PRINT_PRED("Predicate (before optimize):\n",pred);
    while (pred != NULL) {
        next = nextPred(pred);
        delete = FALSE;
        nrOfKeys = c_arraySize(pred->keyField);
        for (k=0;!delete && k<nrOfKeys;k++) {
            pred->fixed = optimizeKey(pred->keyField[k]) || pred->fixed;

            /* if the 1st field of the pred's range is a nilRange, i.e. a range
             * not containing any values (e.g. <4..4>), then delete it from the
             * list, as it will always result to FALSE. This means that in an or
             * statement it does not contribute.
             */
            if(c_qKey(pred->keyField[k])->range &&
                    (c_arraySize(c_qKey(pred->keyField[k])->range) > 0) &&
                    isNilRange(c_qRange(c_qKey(pred->keyField[k])->range[0]))){
                delete = TRUE;
            }
        }


        if(delete){
            PRINT_PRED("Going to delete predicate (during optimize):\n",pred);
            /* delete pred from the predicate list of queries */
            if(!predPrev){
                /* if predPrev == null then pred is the first pred in the list.
                 * Instead of deleting it, we transform it to a FALSE const predicate.
                 * This way, when all predicates are FALSE, there is at least one
                 * predicate left to indicate that this list of predicates evaluates to
                 * false.
                 */
                c_free(resultPred->expr);
                resultPred->expr = c_qExpr(c_new(c_qConstType(c__getBase(_this))));
                if (resultPred->expr) {
                    resultPred->expr->kind = CQ_CONST;
                    c_qConst(resultPred->expr)->value = c_boolValue(FALSE);
                    predPrev = pred;
                } else {
                    assert(FALSE);
                }
            } else {
                predPrev->next = c_keep(pred->next);
                c_free(pred);
            }
        } else {
            PRINT_PRED("Predicate (during optimize):\n",pred);
            predPrev = pred;
        }

        pred = next;

    }

    PRINT_PRED("Predicate (after optimize):\n",resultPred);

    return resultPred;

#undef nextPred
}


static c_bool
setArg (
    c_qVar var,
    c_char *par)
{
    c_base base;
    c_short s;
    c_value *v;

    base = c__getBase(var);
    v = &c_qConst(var->variable)->value;

#if 1
    if (var->type) {
        c_literal l;
        c_type t;
        t = c_typeActualType(var->type);
        switch (c_baseObject(t)->kind) {
        case M_ENUMERATION:
            l = c_enumValue(c_enumeration(t), par);
            if (l) {
                *v = l->value;
                c_free(l);
            } else {
                 OS_REPORT_1(OS_API_INFO,
                             "c_querybase::setArg",0,
                             "expected enum value instead of \"%s\".",
                             par);
                return FALSE;
            }
            return TRUE;
        break;
        case M_PRIMITIVE:
            if (c_primitive(t)->kind == P_BOOLEAN) {
                if (os_strncasecmp(par, "true",5) == 0) {
                    *v = c_boolValue(TRUE);
                } else if (os_strncasecmp(par, "false",6) == 0) {
                    *v = c_boolValue(FALSE);
                } else {
                     OS_REPORT_1(OS_API_INFO,
                                 "c_querybase::setArg",0,
                                 "expected boolean value instead of \"%s\".",
                                 par);
                    return FALSE;
                }
                return TRUE;
            }
        break;
        default:
            /* nothing to do here */
        break;
        }
    }
#endif
    switch (v->kind) {
    case V_LONG:
        if (!sscanf(par,"%d",&v->is.Long)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_ULONG:
        if (!sscanf(par,"%u",&v->is.ULong)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_OCTET:
        if (!sscanf(par,"%hd",&s)) {
            v->kind = V_UNDEFINED;
        }
        v->is.Octet = (c_octet)s;
    break;
    case V_SHORT:
        if (!sscanf(par,"%hd",&v->is.Short)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_USHORT:
        if (!sscanf(par,"%hu",&v->is.UShort)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_FLOAT:
        if (!sscanf(par,"%f",&v->is.Float)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_DOUBLE:
        if (!sscanf(par,"%lf",&v->is.Double)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_LONGLONG:
        v->is.LongLong = os_atoll (par);
        if (errno) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_ULONGLONG:
        v->is.LongLong = os_atoll (par);
        if (errno) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_OBJECT:
        if(v->is.Object){
            c_free(v->is.Object);
            v->is.Object = NULL;
        }
        if (!sscanf(par,"%p",&v->is.Object)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_BOOLEAN:
        if (os_strncasecmp(par,"TRUE",5) == 0) {
            v->is.Boolean = TRUE;
        } else if (os_strncasecmp(par,"FALSE",6) == 0) {
            v->is.Boolean = FALSE;
        } else {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_CHAR:
        if (par != NULL) {
            v->is.Char = *par;
        } else {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_STRING:
    default:
        if(v->is.String){
            c_free(v->is.String);
        }
        v->is.String = c_stringNew(base,par);
    break;
    }
    return (v->kind != V_UNDEFINED);
}

static c_bool
setKey(
    c_qKey key)
{
    c_qRange r;
    c_value v;
    c_long i,nrOfRanges;

    if (key != NULL) {
        if (key->range != NULL) {
            nrOfRanges = c_arraySize(key->range);
            for (i=0;i<nrOfRanges;i++) {
                r = key->range[i];
                if (r->start.kind != V_UNDEFINED) {
                    if (r->start.kind == V_STRING) {
                        c_free(r->start.is.String);
                    } else if (r->start.kind == V_OBJECT) {
                        c_free(r->start.is.Object);
                    }
                    v = getValue(r->startExpr);
                    r->start = c_valueCast(v,r->start.kind);
                    if (r->start.kind == V_UNDEFINED) {
                        return FALSE;
                    } else if (r->start.kind == V_STRING) {
                        c_keep(r->start.is.String);
                    } else if (r->start.kind == V_OBJECT) {
                        c_keep(r->start.is.Object);
                    }
                }
                if (r->end.kind != V_UNDEFINED) {
                    if (r->end.kind == V_STRING) {
                        c_free(r->end.is.String);
                    } else if (r->end.kind == V_OBJECT) {
                        c_free(r->end.is.Object);
                    }
                    v = getValue(r->endExpr);
                    r->end = c_valueCast(v,r->end.kind);
                    if (r->end.kind == V_UNDEFINED) {
                        return FALSE;
                    } else if (r->end.kind == V_STRING) {
                        c_keep(r->end.is.String);
                    } else if (r->end.kind == V_OBJECT) {
                        c_keep(r->end.is.Object);
                    }
                }
            }
        }
    }
    return TRUE;
}

c_bool
c_qPredSetArguments(
    c_qPred p,
    c_value params[])
{
    c_long i,length;
    c_qVar v;

    if (p == NULL) {
        OS_REPORT(OS_WARNING,
                  "Database predicate set arguments",0,
                  "No predicate specified");
    }
#if 0 /* change to 0 to enable optimization! */
    return FALSE;
#else
    if (p->varList == NULL) {
        return TRUE;
    }
    if (params == NULL) {
        OS_REPORT(OS_ERROR,
                  "Database predicate set arguments",0,
                  "No arguments specified");
        return FALSE;
    }
    if (p->fixed) { /* parameters cannot be modified */
        return FALSE;
    }
    length = c_arraySize(p->varList);
#if 1
    for (i=0;i<length;i++) {
        v = c_qVar(p->varList[i]);
        assert(params[v->id].kind == V_STRING);
        if (setArg(v,params[v->id].is.String) == FALSE) {
            return FALSE;
        }
    }

    {
        c_long k,nrOfKeys;
        c_qPred pred = p;

        while (pred != NULL) {
            nrOfKeys = c_arraySize(pred->keyField);
            for (k=0;k<nrOfKeys;k++) {
                if (setKey(pred->keyField[k]) == FALSE) {
                    return FALSE;
                }
            }
            pred = pred->next;
        }
    }
#else
    for (i=0;i<length;i++) {
        v = c_qVar(p->varList[i]);
        if (c_qConst(v->variable)->value.kind != params[v->id].kind) {
            return FALSE;
        }
    }
    for (i=0;i<length;i++) {
        v = c_qVar(p->varList[i]);
        c_qConst(v->variable)->value = params[v->id];
    }
#endif
    return TRUE;
#endif
}

c_bool
c_qKeyFree(
    c_qKey key)
{
    c_long i;

    if (key == NULL) {
        return TRUE;
    }
    for (i=0;i<c_arraySize(key->range);i++) {
        c_free(key->range[i]);
    }
    c_free(key);
    return TRUE;
}

c_bool
c_qKeyEval(
    c_qKey key,
    c_object o)
{
    c_value v;
    c_long i,length;
    c_qRange r;
    c_bool rangeResult;


    if (key == NULL) {
        return TRUE; /* open range */
    }
    if (key->expr != NULL) {
        v = c_qValue(key->expr,o);
        assert(v.kind == V_BOOLEAN);
        if (!v.is.Boolean) {
            return FALSE;
        }
    }
    if (key->range != NULL) {
        v = c_fieldValue(key->field,o);
        length = c_arraySize(key->range);
        for (i=0;i<length;i++) {
            r = key->range[i];
            rangeResult = TRUE;
            switch (r->startKind) {
            case B_UNDEFINED:
            break;
            case B_INCLUDE:
                if (c_valueCompare(v,r->start) == C_LT) {
                    rangeResult = FALSE;
                }
            break;
            case B_EXCLUDE:
                if (c_valueCompare(v,r->start) != C_GT) {
                    rangeResult = FALSE;
                }
            break;
            default:
                assert(FALSE);
            break;
            }
            if (rangeResult) {
                switch (r->endKind) {
                case B_UNDEFINED:
                break;
                case B_INCLUDE:
                    if (c_valueCompare(v,r->end) == C_GT) {
                        rangeResult = FALSE;
                    }
                break;
                case B_EXCLUDE:
                    if (c_valueCompare(v,r->end) != C_LT) {
                        rangeResult = FALSE;
                    }
                break;
                default:
                    assert(FALSE);
                break;
                }
            }
            if (rangeResult) {
                c_valueFreeRef(v);
                return TRUE;
            }
        }
        c_valueFreeRef(v);
        return FALSE;
    }
    return TRUE;
}

c_bool
c_qPredEval (
    c_qPred q,
    c_object o)
{
    c_long i,nrOfKeys;
    c_bool dontStop;
    c_value v;

    if (q == NULL) {
        return TRUE;
    }
    while (q != NULL) {
        i = 0; dontStop = TRUE;
        nrOfKeys = c_arraySize(q->keyField);
        while ((i<nrOfKeys) && dontStop) {
            dontStop = c_qKeyEval(q->keyField[i],o);
            i++;
        }
        if (dontStop) {
            if (q->expr == NULL) {
              return TRUE;
            }
            v = c_qValue(q->expr,o);
            assert(v.kind == V_BOOLEAN);
            return v.is.Boolean;
        }
        q = q->next;
    }
    return FALSE;
}

void
c_qExprPrint(
    c_qExpr q)
{
    c_char* vi;

    if (q == NULL) {
        return;
    }
#define _LEFT_    (c_qFunc(q)->params[0])
#define _RIGHT_   (c_qFunc(q)->params[1])
#define _FUNC_(o) c_qExprPrint(_LEFT_); printf(o); c_qExprPrint(_RIGHT_)
    switch (q->kind) {
    case CQ_FIELD:
    {
        c_array path;
        c_long i,length;
        c_property property;
        c_member member;
        c_string metaName;

        path = c_fieldPath(c_qField(q)->field);
        if (path != NULL) {
            length = c_arraySize(path);
            for (i=0;i<length;i++) {
                switch(c_baseObject(path[i])->kind) {
                case M_ATTRIBUTE:
                case M_RELATION:
                    property = c_property(path[i]);
                    metaName = c_metaName(c_metaObject(property));
                    printf("%s(" PA_ADDRFMT ")",
                           metaName,
                           (PA_ADDRCAST) property->offset);
                    c_free(metaName);
                break;
                case M_MEMBER:
                    member = c_member(path[i]);
                    printf("%s(" PA_ADDRFMT ")",
                           c_specifier(member)->name,
                           (PA_ADDRCAST) member->offset);
                break;
                default:
                    assert(FALSE);
                break;
                }
            }
        }
        printf("%s",c_fieldName(c_qField(q)->field));
    }
    break;
    case CQ_CONST: vi = c_valueImage(c_qConst(q)->value); printf("%s", vi); os_free(vi); break;
    case CQ_AND:   _FUNC_(" AND "); break;
    case CQ_OR:    _FUNC_(" OR "); break;
    case CQ_NOT:   _FUNC_(" NOT "); break;
    case CQ_EQ:    _FUNC_(" == "); break;
    case CQ_NE:    _FUNC_(" <> "); break;
    case CQ_LT:    _FUNC_(" < "); break;
    case CQ_LE:    _FUNC_(" <= "); break;
    case CQ_GT:    _FUNC_(" > "); break;
    case CQ_GE:    _FUNC_(" >= "); break;
    case CQ_LIKE:  _FUNC_(" like "); break;
    default:
        printf(" <UNKOWN> ");
    break;
    }
#undef _FUNC_
#undef _RIGHT_
#undef _LEFT_
}

void
c_qRangePrint(
    c_qRange q)
{
    c_char* vi;
    if (q == NULL) {
        return;
    }
    switch (q->startKind) {
    case B_UNDEFINED: printf("[*.."); break;
    case B_INCLUDE:   vi = c_valueImage(q->start); printf("[%s..",vi); os_free(vi); break;
    case B_EXCLUDE:   vi = c_valueImage(q->start); printf("<%s..",vi); os_free(vi); break;
    }
    switch (q->endKind) {
    case B_UNDEFINED: printf("*]"); break;
    case B_INCLUDE:   vi = c_valueImage(q->end); printf("%s]",vi); os_free(vi); break;
    case B_EXCLUDE:   vi = c_valueImage(q->end); printf("%s>",vi); os_free(vi); break;
    }
}

void
c_qKeyPrint(
    c_qKey q)
{
    c_long i;
    c_qExprPrint(q->expr);
    if (q->range == NULL) {
        return;
    }
    printf("\n");
    for (i=0;i<c_arraySize(q->range);i++) {
        c_qRangePrint(q->range[i]);
        if ((i+1) < c_arraySize(q->range)) {
            printf(" || ");
        }
    }
}

void
c_qPredPrint(
    c_qPred q)
{
    c_long i;

    printf("Query definition:\n");
    while (q != NULL) {
        printf("    expression: ");
        c_qExprPrint(q->expr);
        printf("\nAND:\n");
        if (q->keyField != NULL) {
            for (i=0;i<c_arraySize(q->keyField);i++) {
                printf("    key[%d]: ",i);
                c_qKeyPrint(q->keyField[i]);
                printf("\n");
            }
        }
        q = q->next;
        if (q != NULL) {
            printf("OR:\n");
        }
    }
    printf("\n");
}

