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
#ifndef DESIGNBASE_H
#define DESIGNBASE_H

#include "c_metabase.h"
#include "c_iterator.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef struct c_declarator *c_declarator;

OS_API c_array
c_metaArray(
    c_metaObject scope,
    c_iter iterator,
    c_metaKind kind);
    
OS_API c_declarator
c_declaratorNew(
    c_string name,
    c_iter sizes);

OS_API c_type
c_declaratorType(
    c_declarator declaration,
    c_type type);
    
OS_API c_string
c_declaratorName(
    c_declarator declaration);

OS_API c_iter
c_bindMembers(
    c_metaObject scope,
    c_iter declarations,
    c_type type);
    
OS_API c_iter
c_bindAttributes(
    c_metaObject scope,
    c_iter declarations,
    c_type type,
    c_bool isReadOnly);
    
OS_API c_iter
c_bindTypes(
    c_metaObject scope,
    c_iter declarations,
    c_type type);

OS_API c_constant
c_constantNew(
    c_metaObject scope,
    const c_char *name,
    c_value value);
    
OS_API c_unionCase
c_unionCaseNew(
    c_metaObject scope,
    const c_char *name,
    c_type type,
    c_iter labels);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif


