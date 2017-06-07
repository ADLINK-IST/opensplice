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

#ifndef C_GENCOMMON_H
#define C_GENCOMMON_H

#include "c_iterator.h"
#include "c_metabase.h"
#include "c_module.h"

typedef struct c_genArg {
    c_metaObject scope;
    FILE *stream;
    int level;
    c_bool scopedNames;
    c_iter processing;
    void (*action)(c_metaObject o, struct c_genArg *context);
} *c_genArg;

typedef enum c_objectStateKind {
    G_UNKNOWN,
    G_UNDECLARED,
    G_DECLARED,
    G_EXTENDS,
    G_FINISHED
} c_objectStateKind;

typedef struct c_objectState {
    c_objectStateKind kind;
    c_object object;
} *c_objectState;

void
c_setObjectState(
    c_genArg context,
    c_object o,
    c_objectStateKind kind);

c_objectStateKind
c_getObjectState(
    c_genArg context,
    c_object o);

void
c_genDependencies(
    c_baseObject o,
    c_genArg context);

void
c_out(
    c_genArg context,
    const char *format, ...)
        __nonnull((1,2))
        __attribute_format__((printf,2,3));

void
c_outi(
    c_genArg context,
    int indent,
    const char *format, ...)
        __nonnull((1,3))
        __attribute_format__((printf,3,4));

void
c_indent(
    c_genArg context,
    int subLevel);

c_char*
c_getContextScopedTypeName(
    c_type type,
    c_char *separator,
    c_bool smart,
    c_genArg context);

c_char*
c_getContextScopedConstName(
    c_constant c,
    c_char *separator,
    c_bool smart,
    c_genArg context);

c_ulong
c_getClassProperties(
    c_class o,
    c_property **p);

int
c_compareProperty(
    const void *ptr1,
    const void *ptr2);

os_size_t
c_typeMaxSize(
    c_type type);

#endif /* C_GENCOMMON_H */
