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
#include "vortex_os.h"

#include "c_base.h"

#include "c_gencommon.h"
#include "c_typenames.h"

static c_equality c_collectCompare(c_objectState o1, c_object o2);
static os_size_t c_union_maxSize(c_union o);
static os_size_t c_structure_maxSize(c_structure o);
static os_size_t c_typeDef_maxSize(c_typeDef o);
static int compareProperty(const void *ptr1, const void *ptr2);

static c_long nr_of_unbounded_sequences = 0;
static c_long nr_of_unbounded_strings = 0;
#define DEFAULT_WORST_CASE_STRING_SIZE (20)
#define DEFAULT_WORST_CASE_SEQUENCE_SIZE (1)

static c_equality
c_collectCompare(
    c_objectState o1,
    c_object o2)
{
    if (o1->object > o2) {
        return C_GT;
    }
    if (o1->object < o2) {
        return C_LT;
    }
    return C_EQ;
}

void
c_setObjectState(
    c_genArg context,
    c_object o,
    c_objectStateKind kind)
{
    c_objectState template, found;

    found = c_iterResolve(context->processing, (c_iterResolveCompare)c_collectCompare, o);
    if (found != NULL) {
        found->kind = kind;
    } else {
        template = (c_objectState)os_malloc(sizeof(struct c_objectState));
        template->object = o;
        template->kind = kind;
        c_iterInsert(context->processing, template);
    }
}

c_objectStateKind
c_getObjectState(
    c_genArg context,
    c_object o)
{
    c_objectState found;

    found = c_iterResolve(context->processing,(c_iterResolveCompare)c_collectCompare,o);
    if (found == NULL) {
        return G_UNKNOWN;
    }
    return found->kind;
}



/** \brief This method will generate all dependencies of the given object.
 *
 *  Dependencies are only generated if not generated before, if generated
 *  the dependencies are added to a collection of the context.
 *  The context is consulted for the generation state of objects.
 */
void
c_genDependencies(
    c_baseObject o,
    c_genArg context)
{
    c_ulong i;

#define _TYPEGEN_(type) context->action(c_metaObject(type), context)

    if (o == NULL) {
        return;
    }
    switch(o->kind) {
    case M_MODULE:
        c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genDependencies, context);
    break;
    case M_ATTRIBUTE:
    case M_RELATION:
        _TYPEGEN_(c_property(o)->type);
    break;
    case M_MEMBER:
        _TYPEGEN_(c_specifier(o)->type);
    break;
    case M_CONSTANT:
        _TYPEGEN_(c_constant(o)->type);
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
        if (c_structure(o)->members != NULL) {
            for (i=0; i<c_arraySize(c_structure(o)->members); i++) {
                _TYPEGEN_(c_specifier(c_structure(o)->members[i])->type);
            }
        }
    break;
    case M_CLASS:
        _TYPEGEN_(c_class(o)->extends);
        if (c_interface(o)->inherits != NULL) {
            for (i=0; i<c_arraySize(c_interface(o)->inherits); i++) {
                _TYPEGEN_(c_interface(o)->inherits[i]);
            }
        }
        c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genDependencies, context);
    break;
    case M_INTERFACE:
        if (c_interface(o)->inherits != NULL) {
            for (i=0; i<c_arraySize(c_interface(o)->inherits); i++) {
                _TYPEGEN_(c_interface(o)->inherits[i]);
            }
        }
        c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genDependencies, context);
    break;
    case M_COLLECTION:
        _TYPEGEN_(c_collectionType(o)->subType);
    break;
    case M_OPERATION:
        _TYPEGEN_(c_operation(o)->result);
        if (c_operation(o)->parameters != NULL) {
            for (i=0; i<c_arraySize(c_operation(o)->parameters); i++) {
                _TYPEGEN_(c_specifier(c_operation(o)->parameters[i])->type);
            }
        }
    break;
    case M_UNION:
        _TYPEGEN_(c_union(o)->switchType);
        if (c_union(o)->cases != NULL) {
            for (i=0; i<c_arraySize(c_union(o)->cases); i++) {
                _TYPEGEN_(c_specifier(c_union(o)->cases[i])->type);
            }
        }
    break;
    case M_TYPEDEF:
        _TYPEGEN_(c_typeDef(o)->alias);
    break;
    default:
    break;
    }
#undef _TYPEGEN_
}

void
c_out(
    c_genArg context,
    const char *format, ...)
{
    va_list args;
    va_start(args,format);
    vfprintf(context->stream,format,args);
    va_end(args);
}

void
c_outi(
    c_genArg context,
    int indent,
    const char *format, ...)
{
    va_list args;
    int i;
    for (i=0; i<(context->level + indent); i++) {
        fprintf(context->stream, "    ");
    }
    va_start(args,format);
    vfprintf(context->stream,format,args);
    va_end(args);
}

void
c_indent(
    c_genArg context,
    int subLevel)
{
    int i;
    for (i = 0; i < (context->level + subLevel); i++) {
        fprintf(context->stream, "    ");
    }
}

c_char*
c_getContextScopedTypeName(
    c_type type,
    c_char *separator,
    c_bool smart,
    c_genArg context)
{
    c_metaObject scope;
    c_char *result;
    c_scopeWhen scopeWhen;

    if (c_baseObject(context->scope)->kind == M_MODULE) {
        scope = context->scope;
    } else {
        scope = c_metaModule(context->scope);
    }

    if (smart) {
        scopeWhen = C_SCOPE_SMART;
    } else {
        scopeWhen = (context->scopedNames ? C_SCOPE_ALWAYS : C_SCOPE_NEVER);
    }

    result = c_getScopedTypeName(scope, type, separator, scopeWhen);

    return result;
}

c_char*
c_getContextScopedConstName(
    c_constant c,
    c_char *separator,
    c_bool smart,
    c_genArg context)
{
    c_metaObject scope;
    c_char *result;
    c_scopeWhen scopeWhen;

    if (c_baseObject(context->scope)->kind == M_MODULE) {
        scope = context->scope;
    } else {
        scope = c_metaModule(context->scope);
    }

    if (smart) {
        scopeWhen = C_SCOPE_SMART;
    } else {
        scopeWhen = (context->scopedNames ? C_SCOPE_ALWAYS : C_SCOPE_NEVER);
    }

    result = c_getScopedTypeName(scope, c_type(c), separator, scopeWhen);

    return result;
}

static void
getPropertyAction(
    c_metaObject o,
    c_iter *iter)
{
    switch(c_baseObject(o)->kind) {
        case M_ATTRIBUTE:
        case M_RELATION:
            *iter = c_iterInsert(*iter, o);
            break;
        default:
            break;
    }
}

static int
compareProperty(
    const void *ptr1,
    const void *ptr2)
{
    c_property *p1,*p2;

    p1 = (c_property *)ptr1;
    p2 = (c_property *)ptr2;

    if (*p1 == *p2) {
        return 0;
    } else if (*p1 == NULL) {
        return 1;
    } else if (*p2 == NULL) {
        return -1;
    } else if ((*p1)->offset < (*p2)->offset) {
        return -1;
    } else if ((*p1)->offset > (*p2)->offset) {
        return 1;
    } else {
        return 0;
    }
}

c_ulong
c_getClassProperties(
    c_class o,
    c_property **p)
{
    c_iter propIter = NULL;
    c_ulong len, i;

    c_metaWalk(c_metaObject(o), (c_metaWalkAction)getPropertyAction, &propIter);
    len = c_iterLength(propIter);
    if (len > 0) {
        assert(p && *p == NULL);
        *p = os_malloc(sizeof(c_property) * len);
        for (i = 0; i < len; i++) {
            (*p)[i] = c_iterTakeFirst(propIter);
        }
        c_iterFree(propIter);
        qsort(*p, len, sizeof(c_property), compareProperty);
    } else {
        assert(FALSE);
    }
    return len;
}

static os_size_t
c_structure_maxSize(
    c_structure o)
{
    c_ulong i;
    c_specifier s;
    os_size_t maxSize = 0;

    maxSize = c_typeSize(c_type(o));
    for (i=0; i<c_arraySize(o->references); i++) {
        s = c_specifier(o->references[i]);
        maxSize += c_typeMaxSize(s->type);
    }
    return maxSize;
}

static c_ulong
c_collectionType_maxSize(
    c_collectionType o)
{
    c_ulong maxSize;
    c_type subType;
    c_char *scopedName = NULL;

    maxSize = c_collectionTypeMaxSize(o);
    if (maxSize == 0) {
        switch (c_collectionType(o)->kind) {
            case OSPL_C_STRING:
            case OSPL_C_WSTRING:
                nr_of_unbounded_strings++;
                maxSize = DEFAULT_WORST_CASE_STRING_SIZE;
                break;
            case OSPL_C_ARRAY:
            case OSPL_C_SET:
            case OSPL_C_SEQUENCE:
                nr_of_unbounded_sequences++;
                maxSize = DEFAULT_WORST_CASE_SEQUENCE_SIZE;
                break;
            default:
                scopedName = c_metaScopedName(c_metaObject(o));
                fprintf(stderr, "Found unexpected unbounded collection type: %s, kind %d\n", scopedName, c_collectionType(o)->kind);
                os_free(scopedName);
                break;
        }
    }
    subType = c_collectionTypeSubType(o);
    maxSize *= (c_ulong) c_typeSize(subType);
    return maxSize;
}

static os_size_t
c_typeDef_maxSize(
    c_typeDef o)
{
    return c_typeMaxSize(o->alias);
}

static os_size_t
c_union_maxSize(
    c_union o)
{
    c_ulong i;
    c_specifier s;
    os_size_t maxSize = 0;
    os_size_t size = 0;

    for (i=0; i<c_arraySize(o->cases); i++) {
        s = c_specifier(o->cases[i]);
        size = c_typeMaxSize(s->type);
        maxSize = (size > maxSize ? size : maxSize);
    }
    maxSize += c_typeSize(c_type(o));
    return maxSize;
}

os_size_t
c_typeMaxSize(
    c_type type)
{
    os_size_t maxSize = 0;
    os_char *scopedName;

#define _CASE_(t,k) case k: maxSize = t##_maxSize(t(type)); break
    switch(c_baseObject(type)->kind) {
        _CASE_(c_structure, M_STRUCTURE);
        _CASE_(c_collectionType, M_COLLECTION);
        _CASE_(c_typeDef, M_TYPEDEF);
        _CASE_(c_union, M_UNION);
#undef _CASE_

#define _CASE_(t,k) case k: maxSize = c_typeSize(type); break
        _CASE_(c_primitive, M_PRIMITIVE);
        _CASE_(c_enumeration, M_ENUMERATION);
        _CASE_(c_constant, M_CONSTANT);
        _CASE_(c_interface, M_INTERFACE);
        _CASE_(c_class, M_CLASS);
        default:
            scopedName = c_metaScopedName(c_metaObject(type));
            fprintf(stderr, "Found unexpected type: %s, kind %d\n", scopedName, c_baseObject(type)->kind);
            os_free(scopedName);
            assert(FALSE);
            break;
    }
#undef _CASE_

    return maxSize;
}
