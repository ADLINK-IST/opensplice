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
