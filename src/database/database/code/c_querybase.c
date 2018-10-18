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
#include "os_errno.h"

#define c_qKind_t(b)      (b->baseCache.queryCache.c_qKind_t)
#define c_qBoundKind_t(b) (b->baseCache.queryCache.c_qBoundKind_t)
#define c_qConst_t(b)     (b->baseCache.queryCache.c_qConst_t)
#define c_qType_t(b)      (b->baseCache.queryCache.c_qType_t)
#define c_qVar_t(b)       (b->baseCache.queryCache.c_qVar_t)
#define c_qField_t(b)     (b->baseCache.queryCache.c_qField_t)
#define c_qFunc_t(b)      (b->baseCache.queryCache.c_qFunc_t)
#define c_qPred_t(b)      (b->baseCache.queryCache.c_qPred_t)
#define c_qKey_t(b)       (b->baseCache.queryCache.c_qKey_t)
#define c_qRange_t(b)     (b->baseCache.queryCache.c_qRange_t)
#define c_qExpr_t(b)      (b->baseCache.queryCache.c_qExpr_t)

/* returns the head of the iterator as a c_qRange object (non-destructive, i.e. a read, not a take) */
#define c_qRangeIterHead(l) (c_qRange(c_iterObject(l, 0)))

/* a nilRange is a range which contains no elements: e.g. <4..4> */
#define isNilRange(r) \
        (r && \
        (c_valueCompare(r->start, r->end) == C_EQ) && \
        r->startKind == B_EXCLUDE && \
        r->endKind == B_EXCLUDE)

#define c_sscanf(str, ...) (str?sscanf(str, __VA_ARGS__):0)

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
    const c_value params[])
{
    c_qPred filter = NULL;

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

    o = c_metaDefine(module,M_ENUMERATION);
        c_qBoundKind_t(base) = o;
        c_enumeration(o)->elements = c_arrayNew(c_constant_t(base),3);
        c_enumeration(o)->elements[0] = c_metaDeclareEnumElement(module,"B_UNDEFINED");
        c_enumeration(o)->elements[1] = c_metaDeclareEnumElement(module,"B_INCLUDE");
        c_enumeration(o)->elements[2] = c_metaDeclareEnumElement(module,"B_EXCLUDE");
    c_metaFinalize(o);
    c_metaBind(module,"c_qBoundKind",o);
    c_free(o);

    o = c_metaDefine(module,M_ENUMERATION);
        c_qKind_t(base) = o;
        c_enumeration(o)->elements = c_arrayNew(c_constant_t(base),12);
        c_enumeration(o)->elements[0] = c_metaDeclareEnumElement(module,"CQ_FIELD");
        c_enumeration(o)->elements[1] = c_metaDeclareEnumElement(module,"CQ_CONST");
        c_enumeration(o)->elements[2] = c_metaDeclareEnumElement(module,"CQ_AND");
        c_enumeration(o)->elements[3] = c_metaDeclareEnumElement(module,"CQ_OR");
        c_enumeration(o)->elements[4] = c_metaDeclareEnumElement(module,"CQ_EQ");
        c_enumeration(o)->elements[5] = c_metaDeclareEnumElement(module,"CQ_NE");
        c_enumeration(o)->elements[6] = c_metaDeclareEnumElement(module,"CQ_LT");
        c_enumeration(o)->elements[7] = c_metaDeclareEnumElement(module,"CQ_LE");
        c_enumeration(o)->elements[8] = c_metaDeclareEnumElement(module,"CQ_GT");
        c_enumeration(o)->elements[9] = c_metaDeclareEnumElement(module,"CQ_GE");
        c_enumeration(o)->elements[10] = c_metaDeclareEnumElement(module,"CQ_LIKE");
        c_enumeration(o)->elements[11] = c_metaDeclareEnumElement(module,"CQ_NOT");
    c_metaFinalize(o);
    c_metaBind(module,"c_qKind",o);
    c_free(o);

    scope = c_metaDeclare(module,"c_qExpr",M_CLASS);
        c_qExpr_t(base) = scope;
        c_class(scope)->extends = NULL;
        C_META_ATTRIBUTE_(c_qExpr,scope,kind,c_qKind_t(base));
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qConst",M_CLASS);
        c_qConst_t(base) = scope;
        c_class(scope)->extends = c_class(c_keep(c_qExpr_t(base)));
        type = ResolveType(scope,"c_value");
        C_META_ATTRIBUTE_(c_qConst,scope,value,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qType",M_CLASS);
        c_qType_t(base) = scope;
        c_class(scope)->extends = c_class(c_keep(c_qExpr_t(base)));
        C_META_ATTRIBUTE_(c_qType,scope,type,c_type_t(base));
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qField",M_CLASS);
        c_qField_t(base) = scope;
        c_class(scope)->extends = c_class(c_keep(c_qExpr_t(base)));
        C_META_ATTRIBUTE_(c_qField,scope,field,c_field_t(base));
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qFunc",M_CLASS);
        c_qFunc_t(base) = scope;
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_qExpr>");
            c_collectionType(type)->kind = OSPL_C_ARRAY;
            c_collectionType(type)->subType = c_keep(c_qExpr_t(base));
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);

        c_class(scope)->extends = c_keep(c_qExpr_t(base));
        C_META_ATTRIBUTE_(c_qFunc,scope,params,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qRange",M_CLASS);
        c_qRange_t(base) = scope;
        c_class(scope)->extends = NULL;
        C_META_ATTRIBUTE_(c_qRange,scope,startKind,c_qBoundKind_t(base));
        C_META_ATTRIBUTE_(c_qRange,scope,endKind,c_qBoundKind_t(base));
        C_META_ATTRIBUTE_(c_qRange,scope,startExpr,c_qExpr_t(base));
        C_META_ATTRIBUTE_(c_qRange,scope,endExpr,c_qExpr_t(base));
        type = ResolveType(scope,"c_value");
        C_META_ATTRIBUTE_(c_qRange,scope,start,type);
        C_META_ATTRIBUTE_(c_qRange,scope,end,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qKey",M_CLASS);
        c_qKey_t(base) = scope;
        c_class(scope)->extends = NULL;
        C_META_ATTRIBUTE_(c_qKey,scope,expr,c_qExpr_t(base));
        C_META_ATTRIBUTE_(c_qKey,scope,field,c_field_t(base));
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_qRange>");
            c_collectionType(type)->kind = OSPL_C_ARRAY;
            c_collectionType(type)->subType = c_keep(c_qRange_t(base));
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);
        C_META_ATTRIBUTE_(c_qKey,scope,range,type);
        c_free(type);
    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qVar",M_CLASS);
        c_qVar_t(base) = scope;
        c_class(scope)->extends = NULL;
        C_META_ATTRIBUTE_(c_qVar,scope,hasChanged,c_bool_t(base));
        C_META_ATTRIBUTE_(c_qVar,scope,id,c_long_t(base));
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"SET<c_qKey>");
            c_collectionType(type)->kind = OSPL_C_SET;
            c_collectionType(type)->subType = c_keep(c_qKey_t(base));
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);
        C_META_ATTRIBUTE_(c_qVar,scope,keys,type);
        c_free(type);
        C_META_ATTRIBUTE_(c_qVar,scope,variable,c_qConst_t(base));
        C_META_ATTRIBUTE_(c_qVar,scope,type,c_type_t(base));

    c__metaFinalize(scope,FALSE);
    c_free(scope);

    scope = c_metaDeclare(module,"c_qPred",M_CLASS);
        c_qPred_t(base) = scope;
        c_class(scope)->extends = NULL;
        C_META_ATTRIBUTE_(c_qPred,scope,fixed,c_bool_t(base));
        C_META_ATTRIBUTE_(c_qPred,scope,expr,c_qExpr_t(base));
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_qKey>");
            c_collectionType(type)->kind = OSPL_C_ARRAY;
            c_collectionType(type)->subType = c_keep(c_qKey_t(base));
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);
        C_META_ATTRIBUTE_(c_qPred,scope,keyField,type);
        c_free(type);
        type = c_metaDefine(scope,M_COLLECTION);
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_qVar>");
            c_collectionType(type)->kind = OSPL_C_ARRAY;
            c_collectionType(type)->subType = c_keep(c_qVar_t(base));
            c_collectionType(type)->maxSize = 0;
        c_metaFinalize(type);
        C_META_ATTRIBUTE_(c_qPred,scope,varList,type);
        c_free(type);
        C_META_ATTRIBUTE_(c_qPred,scope,next,c_qPred_t(base));
    c__metaFinalize(scope,FALSE);
    c_free(scope);
    c_free(module);

#undef ResolveType
}

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
    c_ulong i,size;

#define _RVAL_(q,o) \
        (c_qValue(c_qFunc(q)->params[0],o))

#define _LVAL_(q,o) \
        (c_qValue(c_qFunc(q)->params[1],o))

#define _CASE_(l,q,o) \
        case CQ_##l: \
            rightValue = _RVAL_(q,o);\
            leftValue = _LVAL_(q,o);\
            v = c_valueCalculate(rightValue,leftValue,O_##l);\
            c_valueFreeRef(rightValue);\
            c_valueFreeRef(leftValue);\
            return v

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
        assert(i>1);
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
             * e.g. [*..5] AND [10..*] ~~> no value satisfies both ranges
             * so the result is <x>, i.e. a list which no variable can satisfy
             */
            r = c_new(c_qRange_t(c__getBase(r1->startExpr)));
            c_setRange(r,r1->start,r1->startExpr,B_EXCLUDE,r1->start,r2->startExpr,B_EXCLUDE);
            result = c_iterAppend(result,r);
            r = NULL;
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
        case C_LE:
            r = c_new(c_qRange_t(c__getBase(r1->endExpr)));
            c_setRange(r,r2->start,r2->startExpr,r2->startKind,r1->end,r1->endExpr,r1->endKind);
            result = c_iterAppend(result,r);
            r = NULL;
            c_free(r1);
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
            range = c_new(c_qRange_t(c__getBase(e)));
            range->startKind = B_UNDEFINED;
            range->endKind   = B_EXCLUDE;
            range->startExpr = c_keep(valueExpr);
            range->endExpr = c_keep(valueExpr);
            range->end   = v;
            range->start = v;
            rangeList = c_iterNew(range);
            range = c_new(c_qRange_t(c__getBase(e)));
            range->startKind = B_EXCLUDE;
            range->endKind   = B_UNDEFINED;
            range->startExpr = c_keep(valueExpr);
            range->endExpr = c_keep(valueExpr);
            range->end   = v;
            range->start = v;
            rangeList = c_iterAppend(rangeList,range);

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
        range = c_new(c_qRange_t(c__getBase(e)));
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
 * If the expression is a string constant then it is the enum label and
 * will be transformed into the enum value.
 */
static c_qResult
transformEnumLabelToValue(
    c_type type,
    c_qExpr expr)
{
    c_qResult result;
    c_type t;
    c_literal l;

    t = c_typeActualType(type);
    if (t != NULL) {
        result = CQ_RESULT_OK;
        switch (c_baseObject(t)->kind) {
        case M_ENUMERATION:
            switch (expr->kind) {
            case CQ_CONST:
                switch (c_qConst(expr)->value.kind) {
                case V_STRING:
                    /* Transform string value into int value */
                    l = c_enumValue(c_enumeration(t),
                                    c_qConst(expr)->value.is.String);
                    if (l != NULL) {
                        c_free(c_qConst(expr)->value.is.String);
                        c_qConst(expr)->value = l->value;
                        c_free(l);
                    } else {
                        OS_REPORT(OS_ERROR,
                                    "c_querybase",0,
                                    "expected enumeration value instead of \"%s\".",
                                    c_qConst(expr)->value.is.String);
                        result = CQ_RESULT_PRECOND_FAIL;
                    }
                break;
                case V_LONGLONG:
                case V_ULONGLONG:
                break;
                case V_UNDEFINED:
                break;
                default:
                    result = CQ_RESULT_PRECOND_FAIL;
                break;
                }
            break;
            case CQ_FIELD:
                if (t == c_typeActualType(c_fieldType(c_qField(expr)->field))) {
                    result = CQ_RESULT_OK;
                } else {
                    result = CQ_RESULT_PRECOND_FAIL;
                }
            break;
            default:
                result = CQ_RESULT_PRECOND_FAIL;
            break;
            }
        break;
        case M_PRIMITIVE:
            if (c_primitive(t)->kind == P_BOOLEAN) {
                switch (expr->kind) {
                case CQ_CONST:
                    switch (c_qConst(expr)->value.kind) {
                    case V_STRING:
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
                            OS_REPORT(OS_ERROR,
                                        "c_querybase",0,
                                        "expected boolean value instead of \"%s\".",
                                        c_qConst(expr)->value.is.String);
                            result = CQ_RESULT_PRECOND_FAIL;
                        }
                    break;
                    case V_UNDEFINED:
                    break;
                    default:
                        result = CQ_RESULT_PRECOND_FAIL;
                    break;
                    }
                break;
                case CQ_FIELD:
                    if (t == c_typeActualType(c_fieldType(c_qField(expr)->field))) {
                        result = CQ_RESULT_OK;
                    } else {
                        result = CQ_RESULT_PRECOND_FAIL;
                    }
                break;
                default:
                    result = CQ_RESULT_PRECOND_FAIL;
                break;
                }
            }
        break;
        default:
            /* nothing to do here */
        break;
        }
    } else {
        result = CQ_RESULT_BAD_PARAMETER;
    }
    return result;
}

/* this is a generic optimise method that will be executed on all node or
 * fnc expression as soon as they are completed. This function will analyse
 * the node parameters and e.g. determine if enum labels exist that must be
 * resolved. Also in future extensions of this code the resulting type of
 * the expression can be determined and cached. This last optimisation
 * enables detailed error reports.
 */
static c_qResult
optimizeExpr(
    c_qExpr e,
    c_bool *fixed)
{
    c_qResult result = CQ_RESULT_OK;
    c_qExpr p1, p2;
    c_type t1, t2;

    OS_UNUSED_ARG(fixed);

    p1 = c_qFunc(e)->params[0];
    p2 = c_qFunc(e)->params[1];
    if (p1 == NULL || p2 == NULL) {
        result = CQ_RESULT_PRECOND_FAIL;
        goto err_precond;
    }

    t1 = t2 = NULL;
    if (p1->kind == CQ_FIELD) {
        t1 = c_fieldType(c_qField(p1)->field);
        result = transformEnumLabelToValue(t1,p2);
        if(result != CQ_RESULT_OK) {
            goto err_transform;
        }
    }
    if (p2->kind == CQ_FIELD) {
        t2 = c_fieldType(c_qField(p2)->field);
        result = transformEnumLabelToValue(t2,p1);
        if(result != CQ_RESULT_OK) {
            goto err_transform;
        }
    }

    if (p1->kind == CQ_FIELD && p2->kind == CQ_FIELD && t1 != t2) {
        OS_REPORT(OS_WARNING,
            "c_querybase::optimizeExpr", 0,
            "Detected incompatible types between "
            "field <%s> and field <%s>",
            c_fieldName(c_qField(p1)->field),
            c_fieldName(c_qField(p2)->field));
    }

err_transform:
    c_free(t1);
    c_free(t2);
err_precond:
    return result;
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
        OS_REPORT(OS_ERROR,
                    "c_querybase::c_qNot",0,
                    "Unknown operator <%d>",
                    (int)e->kind);
        assert(FALSE);
        r = e;
    break;
    }
    return r;
}

static c_qResult
checkValueKindCompatibility (
    c_valueKind k1,
    c_valueKind k2)
{
    c_qResult result;

    switch (k1) {
    case V_OCTET:
    case V_FLOAT: case V_DOUBLE:
    case V_SHORT: case V_LONG: case V_LONGLONG:
    case V_USHORT: case V_ULONG: case V_ULONGLONG:
        switch(k2) {
        case V_OCTET:
        case V_SHORT: case V_LONG: case V_LONGLONG:
        case V_USHORT: case V_ULONG: case V_ULONGLONG:
        case V_FLOAT: case V_DOUBLE:
            result = CQ_RESULT_OK;
        break;
        default:
            result = CQ_RESULT_BAD_PARAMETER;
        break;
        }
    break;
    case V_CHAR: case V_STRING: case V_WCHAR: case V_WSTRING:
    case V_BOOLEAN: case V_FIXED:
    case V_ADDRESS: case V_OBJECT: case V_VOIDP:
        if (k1 == k2) {
            result = CQ_RESULT_OK;
        } else {
            result = CQ_RESULT_BAD_PARAMETER;
        }
    break;
    default :
        result = CQ_RESULT_BAD_PARAMETER;
    }
    return result;
}

static c_qResult
checkTypeCompatiblity (
    c_qExpr e1,
    c_qExpr e2)
{
    c_valueKind k1;
    c_valueKind k2;
    c_qResult result;

    switch (c_qExpr(e1)->kind) {
    case CQ_FIELD:
        k1 = c_fieldValueKind(c_qField(e1)->field);
        switch (c_qExpr(e2)->kind) {
        case CQ_FIELD:
            k2 = c_fieldValueKind(c_qField(e2)->field);
            result = checkValueKindCompatibility(k1, k2);
        break;
        case CQ_CONST:
            k2 = c_qConst(e2)->value.kind;
            if (k2 == V_UNDEFINED) { /* Variable, so for now result = ok. */
                result = CQ_RESULT_OK;
            } else {
                if ((k2 == V_STRING) &&
                    (k1 == V_CHAR) &&
                    (strlen(c_qConst(e2)->value.is.String) < 2))
                {
                    result = CQ_RESULT_OK;
                } else if ((k1 == V_USHORT || k1 == V_ULONG || k1 == V_ULONGLONG) &&
                        k2 == V_LONGLONG && c_qConst(e2)->value.is.LongLong < 0 ) {
                    result = CQ_RESULT_BAD_PARAMETER;
                } else {
                    result = checkValueKindCompatibility(k1, k2);
                }
            }
        break;
        case CQ_AND: case CQ_OR: case CQ_NOT:
        case CQ_EQ: case CQ_NE: case CQ_LT: case CQ_LE: case CQ_GT: case CQ_GE:
        case CQ_LIKE:
            if (k1 == V_BOOLEAN) {
                result = CQ_RESULT_OK;
            } else {
                result = CQ_RESULT_BAD_PARAMETER;
            }
        break;
        default:
            result = CQ_RESULT_BAD_PARAMETER;
        break;
        }
    break;
    case CQ_CONST:
        k1 = c_qConst(e1)->value.kind;
        if (k1 == V_UNDEFINED) { /* Variable, so for now result = ok. */
            result = CQ_RESULT_OK;
        } else {
            switch (c_qExpr(e2)->kind) {
            case CQ_FIELD:
                k2 = c_fieldValueKind(c_qField(e2)->field);
                if ((k1 == V_STRING) &&
                    (k2 == V_CHAR) &&
                    (strlen(c_qConst(e1)->value.is.String) < 2))
                {
                    result = CQ_RESULT_OK;
                } else if ((k2 == V_USHORT || k2 == V_ULONG || k2 == V_ULONGLONG) &&
                        k1 == V_LONGLONG && c_qConst(e1)->value.is.LongLong < 0 ) {
                    result = CQ_RESULT_BAD_PARAMETER;
                } else {
                    result = checkValueKindCompatibility(k1, k2);
                }
            break;
            case CQ_CONST:
                k2 = c_qConst(e2)->value.kind;
                if (k2 == V_UNDEFINED) { /* Variable, so for now result = ok. */
                    result = CQ_RESULT_OK;
                } else {
                    if (((k1 == V_STRING) && (k2 == V_CHAR) &&
                         (strlen(c_qConst(e1)->value.is.String) < 2)) ||
                        ((k2 == V_STRING) && (k1 == V_CHAR) &&
                         (strlen(c_qConst(e2)->value.is.String) < 2)))
                    {
                        result = CQ_RESULT_OK;
                    } else if ((k1 == V_ULONGLONG && k2 == V_LONGLONG) ||
                            (k2 == V_ULONGLONG && k1 == V_LONGLONG)) {
                        result = CQ_RESULT_BAD_PARAMETER;
                    } else {
                        result = checkValueKindCompatibility(k1, k2);
                    }
                }
            break;
            case CQ_AND: case CQ_OR: case CQ_NOT:
            case CQ_EQ: case CQ_NE: case CQ_LT: case CQ_LE: case CQ_GT: case CQ_GE:
            case CQ_LIKE:
                if (k1 == V_BOOLEAN) {
                    result = CQ_RESULT_OK;
                } else {
                    result = CQ_RESULT_BAD_PARAMETER;
                }
            break;
            default:
                result = CQ_RESULT_BAD_PARAMETER;
            break;
            }
        }
    break;
    case CQ_AND: case CQ_OR: case CQ_NOT:
    case CQ_EQ: case CQ_NE: case CQ_LT: case CQ_LE: case CQ_GT: case CQ_GE:
    case CQ_LIKE:
        switch (c_qExpr(e2)->kind) {
        case CQ_FIELD:
            if (c_fieldValueKind(c_qField(e2)->field) == V_BOOLEAN) {
                result = CQ_RESULT_OK;
            } else {
                result = CQ_RESULT_BAD_PARAMETER;
            }
        break;
        case CQ_CONST:
            if (c_qConst(e2)->value.kind == V_BOOLEAN) {
                result = CQ_RESULT_OK;
            } else {
                result = CQ_RESULT_BAD_PARAMETER;
            }
        break;
        case CQ_AND: case CQ_OR: case CQ_NOT:
        case CQ_EQ: case CQ_NE: case CQ_LT: case CQ_LE: case CQ_GT: case CQ_GE:
        case CQ_LIKE:
            result = CQ_RESULT_OK;
        break;
        default:
            result = CQ_RESULT_BAD_PARAMETER;
        break;
        }
    break;
    default:
        result = CQ_RESULT_BAD_PARAMETER;
    break;
    }
    return result;
}

static c_qResult
makeExprQuery(
    q_expr e,
    c_type t,
    c_iter *varList,
    c_bool *fixed,
    c_qExpr *expr);

static c_qResult
makeBooleanExpr(
    q_expr e,
    c_type t,
    c_iter *varList,
    c_bool *fixed,
    c_qExpr *expr)
{
    c_qResult result;
    c_base base;
    c_qExpr q = NULL;

    if ((e == NULL) ||
        (q_getKind(e) != T_FNC) ||
        (t == NULL) ||
        (expr == NULL))
    {
        result = CQ_RESULT_BAD_PARAMETER;
    } else {
        result = CQ_RESULT_OK;
        base = c__getBase(t);
        q = c_qExpr(c_new(c_qFunc_t(base)));
        if (q) {
            c_qFunc(q)->params = c_arrayNew(c_object_t(base),2);
            if (c_qFunc(q)->params == NULL) {
                result = CQ_RESULT_ERROR;
            }
            if (result == CQ_RESULT_OK) {
                result = makeExprQuery(q_leftPar(e),t,varList,fixed, (c_qExpr *)&c_qFunc(q)->params[0]);
                if (result == CQ_RESULT_OK) {
                    result = makeExprQuery(q_rightPar(e),t,varList,fixed, (c_qExpr *)&c_qFunc(q)->params[1]);
                }
            }
            if (result == CQ_RESULT_OK) {
                switch (q_getTag(e)) {
                case Q_EXPR_AND: q->kind=CQ_AND; break;
                case Q_EXPR_OR: q->kind=CQ_OR; break;
                case Q_EXPR_LIKE: q->kind=CQ_LIKE; result = optimizeExpr(q,fixed); break;
                case Q_EXPR_EQ: q->kind=CQ_EQ; result = optimizeExpr(q,fixed); break;
                case Q_EXPR_NE: q->kind=CQ_NE; result = optimizeExpr(q,fixed); break;
                case Q_EXPR_LT: q->kind=CQ_LT; result = optimizeExpr(q,fixed); break;
                case Q_EXPR_LE: q->kind=CQ_LE; result = optimizeExpr(q,fixed); break;
                case Q_EXPR_GT: q->kind=CQ_GT; result = optimizeExpr(q,fixed); break;
                case Q_EXPR_GE: q->kind=CQ_GE; result = optimizeExpr(q,fixed); break;
                default : result = CQ_RESULT_PRECOND_FAIL; break;
                }
            }
            if (result == CQ_RESULT_OK) {
                result = checkTypeCompatiblity(c_qFunc(q)->params[0], c_qFunc(q)->params[1]);
            }
            if (result != CQ_RESULT_OK) {
                c_free(q);
                q = NULL;
            }
        } else {
            result = CQ_RESULT_ERROR;
        }
        *expr = q;
    }
    return result;
}


static c_qResult
makeExprQuery(
    q_expr e,
    c_type t,
    c_iter *varList,
    c_bool *fixed,
    c_qExpr *expr)
{
    c_qResult result;
    c_qExpr r = NULL;
    c_qExpr c = NULL;
    c_qVar var;
    c_field f;
    c_longlong id;
    c_long id_as_long;
    c_base base;
    c_long i;
    q_expr p;
    c_char qn[1024];

    if (e == NULL) {
        *expr = NULL;
        return CQ_RESULT_OK;
    }

    result = CQ_RESULT_OK;

    base = c__getBase(t);
    switch (q_getKind(e)) {
    case T_FNC:
        switch (q_getTag(e)) {
        case Q_EXPR_AND:
        case Q_EXPR_OR:
        case Q_EXPR_LIKE:
        case Q_EXPR_EQ:
        case Q_EXPR_NE:
        case Q_EXPR_LT:
        case Q_EXPR_LE:
        case Q_EXPR_GT:
        case Q_EXPR_GE:
            result = makeBooleanExpr(e,t,varList,fixed,&r);
        break;
        case Q_EXPR_NOT:
            result = makeExprQuery(q_getPar(e,0),t,varList,fixed, &r);
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
                r = c_qExpr(c_new(c_qConst_t(base)));
                if (r) {
                    r->kind = CQ_CONST;
                    c_qConst(r)->value = c_stringValue(c_stringNew(base,qn));
                } else {
                    result = CQ_RESULT_PRECOND_FAIL;
                    (void)result;
                    assert(FALSE);
                }
            } else {
                r = c_qExpr(c_new(c_qField_t(base)));
                if (r) {
                    r->kind = CQ_FIELD;
                    c_qField(r)->field = f;
                } else {
                    result = CQ_RESULT_PRECOND_FAIL;
                    (void)result;
                    assert(FALSE);
                }
            }
        break;
        case Q_EXPR_CALLBACK:
            r = c_qExpr(c_new(c_qFunc_t(base)));
            if (r) {
                r->kind = CQ_CALLBACK;
                c_qFunc(r)->params = c_arrayNew(c_object_t(base),3);
                c = c_qExpr(c_new(c_qType_t(base)));
                if (c) {
                    c->kind = CQ_TYPE;
                    c_qType(c)->type = c_keep(q_getTyp(q_getPar(e,0)));
                }
                c_qFunc(r)->params[0] = c;
                c = c_qExpr(c_new(c_qConst_t(base)));
                if (c) {
                    c->kind = CQ_CONST;
                    c_qConst(c)->value.kind = V_ADDRESS;
                    c_qConst(c)->value.is.Address = (c_address)q_getPar(e,1);
                }
                c_qFunc(r)->params[1] = c;
                result = makeExprQuery(q_getPar(e,2),t,varList,fixed, (c_qExpr *)&c_qFunc(r)->params[2]);
            } else {
                result = CQ_RESULT_PRECOND_FAIL;
                (void)result;
                assert(FALSE);
            }
        break;
        default:
            OS_REPORT(OS_ERROR,
                        "c_querybase::makeExprQuery",0,
                        "Unknown operator <%d>",
                        (int)q_getTag(e));
            result = CQ_RESULT_BAD_PARAMETER;
                (void)result;
            assert(FALSE);
        }
    break;
    case T_INT:
        r = c_qExpr(c_new(c_qConst_t(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_LONGLONG;
            c_qConst(r)->value.is.LongLong = q_getInt(e);
        } else {
            result = CQ_RESULT_PRECOND_FAIL;
            (void)result;
            assert(FALSE);
        }
    break;
    case T_UINT:
        r = c_qExpr(c_new(c_qConst_t(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_ULONGLONG;
            c_qConst(r)->value.is.ULongLong = q_getUInt(e);
        } else {
            result = CQ_RESULT_PRECOND_FAIL;
            (void)result;
            assert(FALSE);
        }
    break;
    case T_DBL:
        r = c_qExpr(c_new(c_qConst_t(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_DOUBLE;
            c_qConst(r)->value.is.Double = q_getDbl(e);
        } else {
            result = CQ_RESULT_PRECOND_FAIL;
            (void)result;
            assert(FALSE);
        }
    break;
    case T_CHR:
        r = c_qExpr(c_new(c_qConst_t(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_CHAR;
            c_qConst(r)->value.is.Char = q_getChr(e);
        } else {
            result = CQ_RESULT_PRECOND_FAIL;
            (void)result;
            assert(FALSE);
        }
    break;
    case T_STR:
        r = c_qExpr(c_new(c_qConst_t(base)));
        if (r) {
            r->kind = CQ_CONST;
            c_qConst(r)->value.kind = V_STRING;
            c_qConst(r)->value.is.String = c_stringNew(base,q_getStr(e));
        } else {
            result = CQ_RESULT_PRECOND_FAIL;
            (void)result;
            assert(FALSE);
        }
    break;
    case T_ID:
        f = c_fieldNew(t,q_getId(e));
        if (f != NULL) {
            r = c_qExpr(c_new(c_qField_t(base)));
            if (r) {
                r->kind = CQ_FIELD;
                c_qField(r)->field = f;
            } else {
                result = CQ_RESULT_PRECOND_FAIL;
                (void)result;
                assert(FALSE);
            }
        } else {
            r = NULL;
        }
    break;
    case T_VAR:
        id = q_getVar(e);
        assert (id <= 0x7fffffff);
        id_as_long = (c_long) id;
        var = c_iterResolve(*varList,(c_iterResolveCompare)c_qVarCompare,&id_as_long);
        if (var == NULL) {
            r = c_qExpr(c_new(c_qConst_t(base)));
            if (r) {
                r->kind = CQ_CONST;
                c_qConst(r)->value = c_undefinedValue();
                var = c_qVar(c_new(c_qVar_t(base)));
                var->type = NULL;
                var->id = id_as_long;
                var->keys = c_setNew(c_qKey_t(base));
                var->variable = c_qConst(c_keep(r));
                *varList = c_iterAppend(*varList,var);
            } else {
                result = CQ_RESULT_PRECOND_FAIL;
                (void)result;
                assert(FALSE);
            }
        } else {
            r = c_keep(c_qExpr(var->variable));
        }
    break;
    case T_TYP:
        r = c_qExpr(c_new(c_qType_t(base)));
        if (r) {
            r->kind = CQ_TYPE;
            c_qType(r)->type = q_getTyp(e);
        } else {
            result = CQ_RESULT_PRECOND_FAIL;
            (void)result;
            assert(FALSE);
        }
    break;
    default:
        OS_REPORT(OS_ERROR,
                    "c_querybase::makeExprQuery",0,
                    "Unknown term type <%d>",
                    (int)q_getKind(e));
        result = CQ_RESULT_BAD_PARAMETER;
        (void)result;
        assert(FALSE);
    break;
    }
    if ((result != CQ_RESULT_OK) && (r != NULL)) {
        c_free(r);
        r = NULL;
    }
    *expr = r;
    return result;
}

static c_bool
optimizeKey(
    c_qKey key)
{
    c_iter rangeList;
    c_ulong i,nrOfRanges;
    c_bool fixed = FALSE;
    c_qRange range,r1,r2;

    rangeList = makeRangeQuery(&key->expr);
    if (rangeList == NULL) {
        return FALSE;
    }
    nrOfRanges = c_iterLength(rangeList);
    key->range = c_arrayNew(c_qRange_t(c__getBase(key)),nrOfRanges);
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
    c_qResult result;

    valueKind = c_fieldValueKind(field);
    keyExpr = q_takeField(term,c_fieldName(field));
    base = c__getBase(field);
    key = c_new(c_qKey_t(base));
    key->field = c_keep(field);
    result = makeExprQuery(keyExpr,type,&KeyVarList,fixed, &key->expr);
    if (result == CQ_RESULT_OK) {
        while ((var = c_iterTakeFirst(KeyVarList)) != NULL) {
            foundVar = c_iterResolve(*varList,(c_iterResolveCompare)c_qVarCompare,&var->id);
            if (foundVar != NULL) {
                foundKey = ospl_c_insert(foundVar->keys,key);
                assert(foundKey == key);
                c_free(var);
            } else {
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
                c_qConst(var->variable)->value.kind = valueKind;
                *varList = c_iterAppend(*varList,var);
                foundKey = ospl_c_insert(var->keys,key);
                assert(foundKey == key);
                OS_UNUSED_ARG(foundKey);
            }
        }
        c_iterFree(KeyVarList);
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
    c_ulong i,length;

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
    const c_value params[])
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

    if (var->id < 0) {
        OS_REPORT(OS_ERROR, "Query expression",0,
                  "Invalid parameter specified [%d]",
                  var->id);
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
                        OS_REPORT(OS_ERROR,
                                    "Query expression",0,
                                    "Value <%s> not part of Enum <%s>",
                                    v.is.String,
                                    metaName);
                        c_free(metaName);
                        parValue.kind = V_UNDEFINED;
                    }
                } else {
                    if (!c_sscanf(v.is.String,"%d",&parValue.is.Long)) {
                        parValue.kind = V_UNDEFINED;
                    }
                }
            } else {
                if (!c_sscanf(v.is.String,"%d",&parValue.is.Long)) {
                    parValue.kind = V_UNDEFINED;
                }
            }
        break;
        case V_ULONG:
            if (!c_sscanf(v.is.String,"%u",&parValue.is.ULong)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_OCTET:
            if (!c_sscanf(v.is.String,"%hd",&s)) {
                parValue.kind = V_UNDEFINED;
            } else {
                parValue.is.Octet = (c_octet)s;
            }
        break;
        case V_SHORT:
            if (!c_sscanf(v.is.String,"%hd",&parValue.is.Short)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_USHORT:
            if (!c_sscanf(v.is.String,"%hu",&parValue.is.UShort)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_FLOAT:
            if (!c_sscanf(v.is.String,"%f",&parValue.is.Float)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_DOUBLE:
            if (!c_sscanf(v.is.String,"%lf",&parValue.is.Double)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_LONGLONG:
            if (!c_sscanf(v.is.String,"%" PA_SCNd64, &parValue.is.LongLong)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_ULONGLONG:
            if (!c_sscanf(v.is.String,"%" PA_SCNu64, &parValue.is.ULongLong)) {
                parValue.kind = V_UNDEFINED;
            }
        break;
        case V_OBJECT:
            if (!c_sscanf(v.is.String,"%p",&parValue.is.Object)) {
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
            if ((v.is.String != NULL) && (strlen(v.is.String) < 2)) {
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
        OS_REPORT(OS_ERROR, "Query expression",0,
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
    /**
     * Negative literal is always of type V_LONGLONG, positive literal is always of type V_ULONGLONG.
     * When comparing a literal with a database field this might cause incompatibilities. Therefore
     * always settle for the most restrictive type.
     */
    if ((k1 == V_LONGLONG) || (k1 == V_ULONGLONG)) {
        if ((k2 == V_LONG) || (k2 == V_SHORT) || (k2 == V_ULONG) || (k2 == V_USHORT)) {
            /* comparison between integer types is allowed. */
            return k2;
        }
    }
    if ((k2 == V_LONGLONG) || (k2 == V_ULONGLONG)) {
        if ((k1 == V_LONG) || (k1 == V_SHORT) || (k1 == V_ULONG) || (k1 == V_USHORT)) {
            /* comparison between integer types is allowed. */
            return k1;
        }
    }
    /* The rest of all combinations are not specified for not yet being used. */
    return V_UNDEFINED;
}

static c_valueKind
c_qExprInitVars (
    c_qExpr e,
    c_array varList,
    const c_value params[])
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
            if (k1 == V_UNDEFINED) {
                return V_UNDEFINED;
            }
        }
        /* If the parameter is an expression variable then set its value. */
        var = lookupVariable(varList,e2);
        if (var != NULL) {
            k2 = c_qVarInit(var,e1,kr,params);
            if (k2 == V_UNDEFINED) {
                return V_UNDEFINED;
            }
        }
        if ((k1 == V_UNDEFINED) || (k2 == V_UNDEFINED)) {
            /* These kinds cannot be combined. */
            return V_UNDEFINED;
        }
        return V_BOOLEAN;
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
            (void) c_qVarInit(var,NULL,V_UNDEFINED,params);

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

static c_qResult
c_qPredInitVars (
    c_qPred p,
    const c_value params[])
{
    c_qResult result = CQ_RESULT_OK;
    c_valueKind kind;
    c_ulong i,nrOfKeys;
    c_qPred orExpr;

    if (p == NULL) {
        OS_REPORT(OS_WARNING,
                  "Database predicate",0,
                  "No predicate specified");
        result = CQ_RESULT_BAD_PARAMETER;
    }
    if (p->varList == NULL) {
        return CQ_RESULT_OK;
    }
    if (params == NULL) {
        OS_REPORT(OS_ERROR,
                  "Database predicate",0,
                  "No arguments specified");
        result = CQ_RESULT_BAD_PARAMETER;
    }
    orExpr = p;
    nrOfKeys = c_arraySize(p->keyField);
    while ((result == CQ_RESULT_OK) && (orExpr != NULL)) {
        if (orExpr->expr != NULL) {
            kind = c_qExprInitVars (orExpr->expr,
                                    p->varList,
                                    params);
            if (kind == V_UNDEFINED) {
                result = CQ_RESULT_BAD_PARAMETER;
            }
        }
        for (i=0; (result == CQ_RESULT_OK) && (i<nrOfKeys); i++)
        {
            if (c_qKey(orExpr->keyField[i])->expr != NULL) {
                kind = c_qExprInitVars (c_qKey(orExpr->keyField[i])->expr,
                                        p->varList,
                                        params);
                if (kind == V_UNDEFINED) {
                    result = CQ_RESULT_BAD_PARAMETER;
                }
            }
        }
        orExpr = orExpr->next;
    }
    return result;
}

c_qResult
c_qPredNew(
    c_type type,
    c_array keyList,
    q_expr pred,
    const c_value params[],
    c_qPred *qPred)
{
    c_qResult result;
    c_base base;
    q_expr e,term;
    c_ulong k,nrOfKeys;
    c_qPred p,*ptr;
    c_ulong i,nrOfVars;
    c_iter varList = NULL;
    c_bool fixed;

    base = c__getBase(type);
    if ((pred == NULL) || (qPred == NULL)) {
        qPred = NULL;
        OS_REPORT(OS_ERROR,
                  "c_querybase::c_qPredNew",0,
                  "predicate == <NULL>");
        return CQ_RESULT_PRECOND_FAIL;
    } else {
        result = CQ_RESULT_OK;
    }
    PRINT_EXPR("Predicate:\n",pred);

    e = q_takePar(pred,0);
    if (q_takePar(pred,0) != NULL) {
        OS_REPORT(OS_ERROR,
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
        p = c_new(c_qPred_t(base));
        p->keyField = NULL;
        result = makeExprQuery(e,type,&varList,&fixed, &p->expr);
        p->next = NULL;
        q_dispose(e);
        if (p->expr == NULL) {
            c_free(p);
            p = NULL;
            result = CQ_RESULT_PRECOND_FAIL;
        }
    } else {
        q_disjunctify(e);
        PRINT_EXPR("Par (disjunctified):\n",e);

        ptr = &p;
        p = NULL;
        while ((term = q_takeTerm(&e)) != NULL) {
            *ptr = c_new(c_qPred_t(base));
            (*ptr)->keyField = c_arrayNew(c_qKey_t(base),nrOfKeys);
            for (k=0; (result == CQ_RESULT_OK) && (k<nrOfKeys); k++)
            {
                (*ptr)->keyField[k] = makeKeyQuery(&term,
                                                   type,
                                                   keyList[k],
                                                   &varList,
                                                   &fixed);
                if ((*ptr)->keyField[k] == NULL) {
                    result = CQ_RESULT_PRECOND_FAIL;
                }
            }
            if (result == CQ_RESULT_OK) {
                result = makeExprQuery(term,type,&varList, &fixed, &(*ptr)->expr);
                (*ptr)->next = NULL;
                ptr = &(*ptr)->next;
            } else {
                c_free(*ptr);
                *ptr = NULL;
            }
            q_dispose(term);
        }
        assert(e == NULL);
        PRINT_PRED("Predicate:\n",p);
    }

    if (result == CQ_RESULT_OK) {
        assert(p);
        nrOfVars = c_iterLength(varList);
        p->varList = c_arrayNew(c_qVar_t(base),nrOfVars);
        for (i=0;i<nrOfVars;i++) {
            p->varList[i] = c_iterTakeFirst(varList);
        }
        c_iterFree(varList);

        result = c_qPredInitVars(p,params);
    }
    if (result == CQ_RESULT_OK) {
        PRINT_PRED("Predicate (vars resolved):\n",p);
        if (p->keyField != NULL) {
            assert(c_refCount(p->keyField) == 1);
        }
        p->fixed = fixed;
        *qPred = c_qPredOptimize(p);
    }
    if ((result != CQ_RESULT_OK) && (p != NULL))
    {
        c_free(p);
    }
    return result;
}

static c_qPred
c_qPredOptimize(
        c_qPred _this)
{
#define nextPred(p) (p ? p->next : NULL)
    c_ulong k,nrOfKeys;
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
    resultPred = _this;

    PRINT_PRED("Predicate (before optimize):\n",pred);
    while (pred != NULL) {
        next = nextPred(pred);
        delete = FALSE;
        nrOfKeys = c_arraySize(pred->keyField);
        assert(pred->keyField || nrOfKeys == 0);
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
                resultPred->expr = c_qExpr(c_new(c_qConst_t(c__getBase(_this))));
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
                 OS_REPORT(OS_ERROR,
                             "c_querybase::setArg",0,
                             "expected enum value instead of \"%s\".",
                             par);
                return FALSE;
            }
            return TRUE;
        case M_PRIMITIVE:
            if (c_primitive(t)->kind == P_BOOLEAN) {
                if (os_strncasecmp(par, "true",5) == 0) {
                    *v = c_boolValue(TRUE);
                } else if (os_strncasecmp(par, "false",6) == 0) {
                    *v = c_boolValue(FALSE);
                } else {
                     OS_REPORT(OS_ERROR,
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
    switch (v->kind) {
    case V_LONG:
        if (!c_sscanf(par,"%d",&v->is.Long)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_ULONG:
        if (!c_sscanf(par,"%u",&v->is.ULong)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_OCTET:
        if (!c_sscanf(par,"%hd",&s)) {
            v->kind = V_UNDEFINED;
        } else {
            v->is.Octet = (c_octet)s;
        }
    break;
    case V_SHORT:
        if (!c_sscanf(par,"%hd",&v->is.Short)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_USHORT:
        if (!c_sscanf(par,"%hu",&v->is.UShort)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_FLOAT:
        if (!c_sscanf(par,"%f",&v->is.Float)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_DOUBLE:
        if (!c_sscanf(par,"%lf",&v->is.Double)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_LONGLONG:
        if (!c_sscanf(par,"%" PA_SCNd64, &v->is.LongLong)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_ULONGLONG:
        if (!c_sscanf(par,"%" PA_SCNu64, &v->is.ULongLong)) {
            v->kind = V_UNDEFINED;
        }
    break;
    case V_OBJECT:
        if(v->is.Object){
            c_free(v->is.Object);
            v->is.Object = NULL;
        }
        if (!c_sscanf(par,"%p",&v->is.Object)) {
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
    c_ulong i,nrOfRanges;

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
    c_ulong i,length;
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
        c_ulong k,nrOfKeys;
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
    c_ulong i;

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
    c_ulong i,length;
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
    c_ulong i,nrOfKeys;
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
        c_ulong i,length;
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
    c_ulong i;
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
    c_ulong i;

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

