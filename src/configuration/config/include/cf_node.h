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
#ifndef CF_NODE_H
#define CF_NODE_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define       cf_node(o)    ((cf_node)(o))

typedef enum cf_kindEnum {
    CF_NODE,
    CF_ATTRIBUTE,
    CF_ELEMENT,
    CF_DATA,
    CF_COUNT
} cf_kind;

C_CLASS(cf_node);
C_STRUCT(cf_node) {
    cf_kind kind;
    c_char *name;
};

OS_API void
cf_nodeInit (
    cf_node node,
    cf_kind kind,
    const c_char *name);

OS_API void
cf_nodeFree (
    cf_node node);

OS_API void
cf_nodeDeinit (
    cf_node node);

OS_API const c_char *
cf_nodeGetName (
    cf_node node);

OS_API cf_kind
cf_nodeKind (
    cf_node node);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CF_NODE_H */
