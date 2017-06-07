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
#ifndef C_QUERYBASE_H
#define C_QUERYBASE_H

#include "c_querybase.h"
#include "c_base.h"
#include "c_collection.h"
#include "c_field.h"
#include "c_iterator.h"
#include "c_filter.h"
#include "q_expr.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef enum c_qResult {
    CQ_RESULT_OK,
    CQ_RESULT_PRECOND_FAIL,
    CQ_RESULT_BAD_PARAMETER,
    CQ_RESULT_ERROR,
    CQ_RESULT_COUNT
} c_qResult;

typedef enum c_qKind {
    CQ_UNDEFINED,
    CQ_FIELD,CQ_CONST,CQ_TYPE,
    CQ_AND, CQ_OR, CQ_NOT,
    CQ_EQ, CQ_NE, CQ_LT, CQ_LE, CQ_GT, CQ_GE,
    CQ_LIKE, CQ_CALLBACK, CQ_COUNT
} c_qKind;

typedef enum c_qBoundKind {
    B_UNDEFINED,
    B_INCLUDE,
    B_EXCLUDE
} c_qBoundKind;

C_STRUCT(c_qExpr) {
    c_qKind kind;
};

C_STRUCT(c_qConst) {
    C_EXTENDS(c_qExpr);
    c_value value;
};

C_STRUCT(c_qField) {
    C_EXTENDS(c_qExpr);
    c_field field;
};

C_STRUCT(c_qVar) {
    c_long id;
    c_bool hasChanged;
    c_collection keys;
    c_qConst variable;
#if 1
    c_type type;
#endif
};

C_STRUCT(c_qType) {
    C_EXTENDS(c_qExpr);
    c_type type;
};

C_STRUCT(c_qFunc) {
    C_EXTENDS(c_qExpr);
    c_array params;
};

C_STRUCT(c_qRange) {
    c_qBoundKind startKind;
    c_qBoundKind endKind;
    c_qExpr startExpr;
    c_qExpr endExpr;
    c_value start;
    c_value end;
};

C_STRUCT(c_qKey) {
    c_qExpr expr;
    c_field field;
    c_array range;     /* ARRAY<c_qRange> */
};

C_STRUCT(c_qPred) {
    c_bool fixed;
    c_qExpr expr;
    c_array keyField;  /* ARRAY<c_qKey> */
    c_array varList;   /* ARRAY<c_qVar> */
    c_qPred next;
};

#if 0
c_array
c_keyNew (
    c_type t,
    c_iter fieldNames);

c_array
c_keyDcl (
    c_type t,
    c_long size,
    ...);
#endif

OS_API c_qResult
c_qPredNew (
    c_type type,
    c_array keyList,
    q_expr predicate,
    const c_value params[],
    c_qPred *qPred);

c_value
c_qValue (
    c_qExpr q,
    c_object o);

c_bool
c_qPredSetArguments (
    c_qPred p,
    c_value params[]);

void
c_qPredPrint (
    c_qPred q);

void
c_querybaseInit (
    c_base base);

#define c_qRangeStartValue(_this) (_this->start)
#define c_qRangeEndValue(_this)   (_this->end)

#define SHOW_EXPR (0)
#if SHOW_EXPR
#define PRINT_EXPR(msg,expr) \
    printf(msg); q_print(expr,0); printf("\n")
#define PRINT_PRED(msg,pred) \
    printf(msg); c_qPredPrint(pred); printf("\n")
#else
#define PRINT_EXPR(msg,expr)
#define PRINT_PRED(msg,pred)
#endif

#undef OS_API

#endif
