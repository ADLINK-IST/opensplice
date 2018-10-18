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
