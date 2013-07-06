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
#ifndef GAPI_EXPRESSION_H
#define GAPI_EXPRESSION_H

#include "gapi.h"
#include "gapi_common.h"
#include "u_entity.h"
#include "u_reader.h"

C_CLASS(gapi_expression);

typedef struct gapi_readerMask_s {
    gapi_sampleStateMask    sampleStateMask;
    gapi_viewStateMask      viewStateMask;
    gapi_instanceStateMask  instanceStateMask;
} gapi_readerMask;

q_expr
gapi_parseExpression (
    const char *queryString);

gapi_expression
gapi_expressionNew (
    gapi_char *queryString);

void
gapi_expressionFree (
    gapi_expression e);

void
gapi_expressionInitParser (
    void);

void
gapi_expressionDeinitParser (
    void);

u_query
gapi_expressionCreateQuery (
    gapi_expression expression,
    u_reader        reader,
    const c_char   *queryName,
    gapi_stringSeq *parameters);

gapi_returnCode_t
gapi_expressionSetQueryArgs (
    gapi_expression expression,
    u_query         query,
    const gapi_stringSeq *parameters);

gapi_expression
gapi_createQueryExpression (
    u_entity entity,
    const c_char *query);

c_value
gapi_stringValue (
    const char *s);

#endif /* GAPI_EXPRESSION_H */
