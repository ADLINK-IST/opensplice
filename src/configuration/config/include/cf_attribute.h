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
#ifndef CF_ATTRIBUTE_H
#define CF_ATTRIBUTE_H

#include "cf_node.h"

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

#define       cf_attribute(o)    ((cf_attribute)(o))

C_CLASS(cf_attribute);
C_STRUCT(cf_attribute) {
    C_EXTENDS(cf_node);
    c_value value;
};

OS_API cf_attribute
cf_attributeNew (
    const c_char *name,
    c_value value);

OS_API void
cf_attributeInit (
    cf_attribute attribute,
    const c_char *name,
    c_value value);

OS_API void
cf_attributeDeinit (
    cf_attribute attribute);

OS_API c_value
cf_attributeValue(
    cf_attribute attribute);
    
#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CF_ATTRIBUTE_H */
