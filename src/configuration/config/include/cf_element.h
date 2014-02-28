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
#ifndef CF_ELEMENT_H
#define CF_ELEMENT_H

#include "cf_node.h"
#include "cf_attribute.h"

#include "c_iterator.h"

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

#define       cf_element(o)    ((cf_element)(o))

C_CLASS(cf_element);

OS_API cf_element
cf_elementNew (
    const c_char *tagName);
    
OS_API void
cf_elementInit (
    cf_element element,
    const c_char *tagName);
    
OS_API void
cf_elementFree (
    cf_element element);

OS_API void
cf_elementDeinit (
    cf_element element);

OS_API cf_attribute
cf_elementAddAttribute (
    cf_element element,
    cf_attribute attribute);

OS_API cf_node
cf_elementAddChild(
    cf_element element,
    cf_node child);

OS_API c_iter
cf_elementGetAttributes(
    cf_element element);

OS_API c_iter
cf_elementGetChilds(
    cf_element element);

OS_API cf_attribute
cf_elementAttribute(
    cf_element element,
    const c_char *attributeName);

OS_API cf_node
cf_elementChild(
    cf_element element,
    const c_char *childName);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CF_ELEMENT_H */
