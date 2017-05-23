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

/* Interface */
#include "c_typenames.h"

/* Implementation */
#include "os_if.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "c_metabase.h"
#include "c_base.h"


static c_char *
c_getCollKindName(
    c_collectionType c)
{
    c_char *result;

#define _CASE_(k) case k: result = os_strdup(#k + 5 /* OSPL_ prefix */); break
    switch(c->kind) {
    _CASE_(OSPL_C_ARRAY);
    _CASE_(OSPL_C_SEQUENCE);
    _CASE_(OSPL_C_SET);
    _CASE_(OSPL_C_LIST);
    _CASE_(OSPL_C_BAG);
    _CASE_(OSPL_C_DICTIONARY);
    _CASE_(OSPL_C_STRING);
    _CASE_(OSPL_C_WSTRING);
    default:
        assert(FALSE);
        result = os_strdup("C_UNDEFINED");
    break;
    }
#undef _CASE_

    return result;
}


static c_char *
c_getCollAnonName(
    c_metaObject scope,
    c_collectionType c,
    c_char *separator)
{
    c_char *result;
    c_char *kind;
    c_char *subTypeName;

    kind = c_getCollKindName(c);
    subTypeName = c_getScopedTypeName(scope, c->subType,
                                      separator, C_SCOPE_ALWAYS);
    result = (c_char *)os_malloc(strlen(kind) + strlen(subTypeName) + 32);
    if (c->kind == OSPL_C_STRING) {
        if (c->maxSize > 0) {
            os_sprintf(result,"%s<%d>", kind, c->maxSize);
        } else {
            os_sprintf(result,"%s", kind);
        }
    } else {
        if (c->maxSize > 0) {
            os_sprintf(result,"%s<%s,%d>", kind, subTypeName, c->maxSize);
        } else {
            os_sprintf(result,"%s<%s>", kind, subTypeName);
        }
    }

    return result;
}


c_char *
c_getScopedTypeName(
    c_metaObject scope,
    c_type type,
    c_char *separator,
    c_scopeWhen scopeWhen)
{
    c_char *typeName, *moduleName, *result;
    c_metaObject module;

    typeName = c_metaName(c_metaObject(type));
    if (!typeName && (c_baseObject(type)->kind == M_COLLECTION)) {
        typeName = c_getCollAnonName(scope, c_collectionType(type),
                                     separator);
    }
    assert(typeName);
    module = c_metaModule(c_metaObject(type));
    moduleName = c_metaName(module);
    if ((moduleName != NULL) &&
        ((scopeWhen == C_SCOPE_ALWAYS) ||
         ((scopeWhen == C_SCOPE_SMART) && (scope != module)))) {
        /* the type is defined within another module */
        result = (c_char *)os_malloc(strlen(moduleName)+
                                  strlen(typeName)+
                                  strlen(separator)+
                                  1 /* '0' */);
        os_sprintf(result,"%s%s%s",moduleName,separator,typeName);
    } else {
        result = os_strdup(typeName);
    }

    return result;
}


c_char *
c_getScopedConstName(
    c_metaObject scope,
    c_constant c,
    c_char *separator,
    c_scopeWhen scopeWhen)
{
    c_char *name, *moduleName, *result = NULL;
    c_metaObject module;

    name = c_metaName(c_metaObject(c));
    if (name) {
        module = c_metaModule(c_metaObject(c));
        moduleName = c_metaName(module);
        if ((moduleName != NULL) &&
            ((scopeWhen == C_SCOPE_ALWAYS) ||
             ((scopeWhen == C_SCOPE_SMART) && (scope != module)))) {
            /* the const is defined within another module */
            result = (c_char *)os_malloc(strlen(moduleName)+
                                      strlen(name)+
                                      strlen(separator)+
                                      1 /* '0' */);
            os_sprintf(result,"%s%s%s",moduleName,separator,name);
        } else {
            result = os_strdup(name);
        }
    }

    return result;
}
