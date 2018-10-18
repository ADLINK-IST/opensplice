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
