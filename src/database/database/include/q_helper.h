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
#ifndef Q_HELPER_H
#define Q_HELPER_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

OS_API void 
q_exprSetText(
    q_expr expr, const c_char* text);
    
OS_API c_char*    
q_exprGetText(
    q_expr expr);

OS_API void
q_exprSetInstanceState(
    q_expr expr,
    c_ulong state);

OS_API void
q_exprSetSampleState(
    q_expr expr,
    c_ulong state);

OS_API void
q_exprSetViewState(
    q_expr expr,
    c_ulong state);

OS_API c_ulong
q_exprGetInstanceState(
    q_expr expr);

OS_API c_ulong
q_exprGetSampleState(
    q_expr expr);

OS_API c_ulong
q_exprGetViewState(
    q_expr expr);


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /*Q_PARSER_H */
