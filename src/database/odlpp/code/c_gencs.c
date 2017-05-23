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
#include "os_defs.h"
#include "os_abstract.h"
#include "os_heap.h"

#include "c_gencs.h"
#include "c_gencommon.h"
#include "c_typenames.h"

#include "c_base.h"
#include "c_collection.h"
#include "c_iterator.h"
#include "c_metafactory.h"
#include "c_module.h"

#include "ctype.h"

#define DEBUG 0

#ifndef _WIN32
#define dbg(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, \
        __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
#else
#define dbg(fmt, ...)
#endif

static struct lconv *locale;

static void c_genIncludesSpec(c_metaObject o, c_genArg context);
static void c_genObjectSpec(c_metaObject o, c_genArg context);

static void c_moduleSpec(c_module o, c_genArg context);
static void c_classSpec(c_class o, c_genArg context);
static void c_classFwdSpec(c_class o, c_genArg  context);
static void c_attributeSpec(c_attribute o, c_genArg context);
static void c_specifierSpec(c_string name, c_type type, c_genArg context);
static void c_constantSpec(c_constant o, c_genArg context);
static void c_enumerationSpec(c_enumeration o, c_genArg context);
static void c_interfaceSpec(c_interface o, c_genArg context);
static void c_operationSpec(c_operation o, c_genArg context);
static void c_primitiveSpec(c_primitive o, c_genArg context);
static void c_structureSpec(c_structure o, c_genArg context);
static void c_typeDefSpec(c_typeDef o, c_genArg context);
static void c_unionSpec(c_union o, c_genArg context);

static void primitiveHelper(c_primitive p, c_string name, c_genArg context);
static c_string enumNameHelper(c_string name);


/*****************************************************/

void
c_gen_CS(
    c_module topLevel,
    c_bool scopedNames)
{
    struct c_genArg context;

    locale = localeconv();
    /* c_setLocale(localeconv()); */

    context.scope = NULL;
    context.stream = NULL;
    context.action = NULL;
    context.level = 0;
    context.scopedNames = scopedNames;

    context.processing = c_iterNew(NULL);
    context.action = c_genObjectSpec;
    c_metaWalk(c_metaObject(topLevel), (c_metaWalkAction)c_moduleSpec, &context);
}

static void
c_moduleSpec(
    c_module o,
    c_genArg context)
{
    char *moduleName;
    char *streamName;
    struct c_genArg newContext;

    if (o == NULL) {
        return;
    }

    if (c_baseObject(o)->kind != M_MODULE) {
        return;
    }

    moduleName = c_metaName(c_metaObject(o));
    if (strncmp(moduleName, "c_", 2) == 0) {
        return; /* internal modules */
    }

    streamName = os_malloc(strlen(moduleName) + strlen(".cs") + 1);
    os_sprintf(streamName, "%s.cs", moduleName);

    newContext.stream = fopen(streamName, "w");
    newContext.scope = c_metaObject(o);
    newContext.level = 0;
    newContext.scopedNames = context->scopedNames;

    c_outi(&newContext, 0, "using System;\n");
    c_outi(&newContext, 0, "using System.Runtime.InteropServices;\n");
    c_outi(&newContext, 0, "using DDS.OpenSplice.Database;\n");
    c_outi(&newContext, 0, "\n");

    newContext.processing = c_iterNew(NULL);
    newContext.action = c_genIncludesSpec;
    c_genDependencies(c_baseObject(o), &newContext);
    c_outi(&newContext, 0, "\n");

    c_outi(&newContext, 0, "namespace DDS.OpenSplice.%s {\n", moduleName);
    c_outi(&newContext, 0, "\n");
    c_outi(&newContext, 0, "/* Ignore \"Field `xxx' is never assigned to, and will always have its default value\" warnings */\n");
    c_outi(&newContext, 0, "#pragma warning disable 649\n");
    c_outi(&newContext, 0, "\n");

    newContext.processing = context->processing;
    newContext.action = c_genObjectSpec;

    newContext.level++;
    c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genObjectSpec, &newContext);
    newContext.level--;

    c_outi(&newContext, 0, "\n");
    c_outi(&newContext, 0, "#pragma warning restore 649\n");
    c_outi(&newContext, 0, "}\n");
    c_outi(&newContext, 0, "\n");

    fclose(newContext.stream);
}

static void
c_genObjectSpec(
    c_metaObject o,
    c_genArg context)
{
    c_metaObject scope, module, defIn;
    c_string name;

    /* Get the module that is being processed */
    if (c_baseObject(context->scope)->kind == M_MODULE) {
        module = context->scope;
    } else {
        module = c_metaModule(context->scope);
    }

    /* Get the module that contains the processed object. */
    scope = c_metaModule(o);

    /* Check if this meta object is a c_base type, if true then skip the generation */
    if (scope == NULL) {
        return;
    }
    name = c_metaName(scope);
    if (name == NULL) {
        return;
    }

    /* If this meta object is defined in another module then don't
       generate a definition but instead include that module. */
    if (scope != module) {
        /* If this scope is not processed before then include it */
        if (c_getObjectState(context,scope) == G_UNKNOWN) {
            /* Do not output here, this has been done in c_genIncludesSpec */
            if (c_baseObject(o)->kind == M_MODULE) {
                defIn = o->definedIn;
                while (defIn && (defIn != context->scope)) {
                    defIn = defIn->definedIn;
                }
                if (defIn == context->scope) {
                    c_moduleSpec(c_module(o), context);
                }
            }
            c_setObjectState(context,scope,G_FINISHED);
        }
        return;
    }

    switch (c_getObjectState(context, o)) {
        case G_UNKNOWN:
            c_setObjectState(context,o,G_UNDECLARED);
            c_genDependencies(c_baseObject(o),context);

            /* Generate the definition for this meta object. */
#define _CASE_(t,k) case k: t##Spec(t(o),context); break
            switch(c_baseObject(o)->kind) {
                _CASE_(c_attribute, M_ATTRIBUTE);
                _CASE_(c_class, M_CLASS);
                _CASE_(c_constant, M_CONSTANT);
                _CASE_(c_enumeration, M_ENUMERATION);
                _CASE_(c_interface, M_INTERFACE);
                _CASE_(c_module, M_MODULE);
                _CASE_(c_operation, M_OPERATION);
                _CASE_(c_primitive, M_PRIMITIVE);
                _CASE_(c_structure, M_STRUCTURE);
                _CASE_(c_typeDef, M_TYPEDEF);
                _CASE_(c_union, M_UNION);
                default:
                    break;
            }
#undef _CASE_
            c_setObjectState(context, o, G_FINISHED);
            break;
        case G_UNDECLARED:
            /* The second time this dependency occurs, generate a forward declaration. */
            name = c_metaName(o);
            if (name != NULL) {
                switch(c_baseObject(o)->kind) {
                    case M_CLASS:
                        c_classFwdSpec(c_class(o), context);
                        /* c_outi(context,0,"C_CLASS(%s);\n\n",name); */
                        break;
                    case M_TYPEDEF:
                        c_typeDefSpec(c_typeDef(o),context);
                        /* c_genObjectSpec(c_metaObject(c_typeDef(o)->alias),context); */
                        break;
                    default:
                        break;
                }
            }
            c_setObjectState(context,o,G_DECLARED);
            break;
        default:
            break;
    }
}

static void
c_genIncludesSpec(
    c_metaObject o,
    c_genArg context)
{
    c_metaObject scope, module;
    c_string name;

    if (o == NULL) {
        return;
    }

    /* Get the module that is being processed */
    if (c_baseObject(context->scope)->kind == M_MODULE) {
        module = context->scope;
    } else {
        module = c_metaModule(context->scope);
    }

    scope = c_metaModule(o);

    if (scope != module) {
        /* If this scope is not processed before then include it */
        if (c_getObjectState(context, scope) == G_UNKNOWN) {
            name = c_metaName(c_metaObject(scope));
            if (name) {
                c_out(context, "using DDS.OpenSplice.%s;\n", name);
            }
            c_setObjectState(context, scope, G_FINISHED);
        }
    }
}

static void
c_classSpec(
    c_class o,
    c_genArg context)
{
    c_string className;
    c_type type;
    c_metaObject scope;
    c_ulong i, len;
    c_property p, *properties = NULL;

    className = c_metaName(c_metaObject(o));
    dbg("Processing '%s'\n", className);
    if (!c_isFinal((c_metaObject)c_baseObject(o))) {
        dbg("class '%s' is not final\n", className);
        return;
    }

    if (c_getObjectState(context, o) == G_FINISHED) {
        dbg("class '%s' obj state is finished\n", className);
        return;
    }

    type = c_type(o->extends);
    if (type != NULL) {
        if (c_getObjectState(context, type) == G_DECLARED) {
            c_classSpec(c_class(type), context);
            c_setObjectState(context, type, G_FINISHED);
        }
    }

    scope = context->scope;
    context->scope = c_metaObject(o);
    if (o->keys != NULL) {
        c_outi(context, 0, "/* Keys: ");
        for (i = 0; i < c_arraySize(o->keys); i++) {
            c_out(context, " %s", (c_string)o->keys[i]);
        }
        c_out(context, " */\n");
    }

    c_outi(context, 0, "[StructLayoutAttribute(LayoutKind.Sequential)]\n");
    c_outi(context, 0, "internal class %s {\n", className);
    context->level++;
    if (type != NULL) {
        /* Insert parent class as member */
        c_outi(context, 0, "public %s _parent;\n", c_metaName(c_metaObject(type)));
    }

    len = c_getClassProperties(o, &properties);
    for (i = 0; i < len; i++) {
        p = properties[i];
        assert(c_baseObjectKind(p) == M_ATTRIBUTE);
        c_specifierSpec(c_metaName(c_metaObject(p)), p->type, context);
    }
    os_free(properties);
    context->level--;
    c_outi(context, 0, "}\n\n");
    context->scope = scope;
}

static void
c_attributeSpec(
    c_attribute o,
    c_genArg context)
{
    assert(c_baseObject(o)->kind == M_ATTRIBUTE);
    dbg("Processing attribute %p\n", o);
    c_specifierSpec(c_metaName(c_metaObject(o)), c_property(o)->type, context);
    c_out(context, ";\n");
}

static c_string
enumNameHelper(
    c_string name)
{
    os_size_t i;
    os_size_t len = strlen(name);
    c_string newName = os_malloc(len + 1);
    for (i = 0; i < len; i++) {
        newName[i] = (c_char)toupper(name[i]);
    }
    newName[i] = '\0';

    return newName;
}

static void
primitiveHelper(
    c_primitive p,
    c_string name,
    c_genArg context)
{
    switch(c_primitiveKind(p)) {
        case P_ADDRESS:
        case P_VOIDP:
        case P_PA_UINTPTR: /* "pa_uintptr_t" */
        case P_PA_VOIDP:  /* "pa_voidp_t"; */
            c_outi(context, 0, "public IntPtr %s;\n", name);
            break;
        case P_BOOLEAN:
            c_outi(context, 0, "[MarshalAs(UnmanagedType.U1)]\n");
            c_outi(context, 0, "public bool %s;\n", name);
            break;
        case P_CHAR:
        case P_WCHAR:
            c_outi(context, 0, "public char %s;\n", name);
            break;
        case P_OCTET:
            c_outi(context, 0, "public byte %s;\n", name);
            break;
        case P_SHORT:
            c_outi(context, 0, "public short %s;\n", name);
            break;
        case P_USHORT:
            c_outi(context, 0, "public ushort %s;\n", name);
            break;
        case P_LONG:
            c_outi(context, 0, "public int %s;\n", name);
            break;
        case P_PA_UINT32:
        case P_ULONG:
            c_outi(context, 0, "public uint %s;\n", name);
            break;
        case P_LONGLONG:
            c_outi(context, 0, "public long %s;\n", name);
            break;
        case P_ULONGLONG:
            c_outi(context, 0, "public ulong %s;\n", name);
            break;
        case P_FLOAT:
            c_outi(context, 0, "public float %s;\n", name);
            break;
        case P_DOUBLE:
            c_outi(context, 0, "public double %s;\n", name);
            break;
        case P_MUTEX:
        case P_LOCK:
        case P_COND:
            /* generate a stub of the same size */
            c_outi(context, 0, "[MarshalAs(UnmanagedType.ByValArray, SizeConst=%" PA_PRIuSIZE ")]\n",
                c_typeSize(c_type(p)));
            c_outi(context, 0, "public byte[] %s; /* stub for %s type */\n",
                name, c_metaName(c_metaObject(p)));
            break;
        case P_UNDEFINED: /* Fall-through on purpose */
        case P_COUNT:
        default:
            dbg("Skipping primitive '%s' of kind %d\n", name, c_primitiveKind(p));
            c_outi(context, 0, "** PRIM %s KIND %d NOT SUPPORTED **\n", name, c_primitiveKind(p));
            break;
    }
}

static void
c_specifierSpec(
    c_string name,
    c_type type,
    c_genArg context)
{
    c_string typeName, newName;

    if (type == NULL) {
        return;
    }

    if ((name != NULL) && (strlen(name) > 0)) {
        /* escape C# keywords by prefixing '_' */
        if (strcmp(name, "value") == 0) {
            newName = os_malloc(strlen("_value") + 1);
            os_sprintf(newName, "_value");
        } else if (strcmp(name, "lock") == 0) {
            newName = os_malloc(strlen("_lock") + 1);
            os_sprintf(newName, "_lock");
        } else if (strcmp(name, "orderby") == 0) {
            newName = os_malloc(strlen("_orderby") + 1);
            os_sprintf(newName, "_orderby");
        } else {
            newName = os_malloc(strlen(name) + 1);
            os_sprintf(newName, "%s", name);
        }
    } else {
        newName = os_strdup("(undefined)");
        assert(FALSE);
    }

    dbg("Processing specifier '%s' -> '%s'\n", name, newName);

    type = c_typeActualType(type);
    typeName = c_getContextScopedTypeName(type, ".", FALSE, context);
    if (typeName != NULL) {
        switch(c_baseObject(type)->kind) {
            case M_PRIMITIVE:
                primitiveHelper(c_primitive(type), newName, context);
                break;
            case M_TYPEDEF:
                /* should have been resolved to actual-type */
                assert(FALSE);
                break;
            case M_STRUCTURE:
                c_outi(context, 0, "public %s %s;\n", typeName, newName);
                break;
            case M_CLASS:
                c_outi(context, 0, "public IntPtr %s;\n", newName);
                break;
            case M_COLLECTION:
                if ((c_collectionType(type)->kind == OSPL_C_ARRAY) &&
                    (c_collectionType(type)->maxSize != 0)) {
                    os_size_t size = c_typeSize(c_collectionType(type)->subType) * c_collectionType(type)->maxSize;
                    c_string arrName = os_malloc(strlen(newName) + 4);
                    os_sprintf(arrName, "[] %s", newName);
                    c_outi(context, 0, "[MarshalAs(UnmanagedType.ByValArray, SizeConst=%" PA_PRIuSIZE ")]\n", size);
                    c_specifierSpec(arrName,c_collectionType(type)->subType,context);
                    os_free(arrName);
                } else {
                    c_outi(context, 0, "public IntPtr %s;\n", newName);
                }
                break;
            case M_ENUMERATION: {
                c_string enumName = enumNameHelper(typeName);
                c_outi(context, 0, "public %s %s;\n", enumName, newName);
                os_free(enumName);
                break;
            }
            case M_UNION: {
                /* generate a stub of the same size */
                c_outi(context, 0, "[MarshalAs(UnmanagedType.ByValArray, SizeConst=%" PA_PRIuSIZE ")]\n",
                    c_typeMaxSize(type));
                c_outi(context, 0, "public byte[] %s; /* stub for union '%s' */\n",
                    newName, c_metaName(c_metaObject(type)));
                break;
            }
            default:
                dbg("Unsupported specifier '%s', kind '%d', type '%s'", newName, c_baseObject(type)->kind, typeName);
                assert(FALSE);
                break;
        }
    } else {
        dbg("Specifier '%s' type-name %p is NULL\n\n", newName, type);
    }
    os_free(newName);
}

static void
c_constantSpec(
    c_constant o,
    c_genArg context)
{
    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(context);
    assert(c_baseObject(o)->kind == M_CONSTANT);
    dbg("Skipping const '%s'\n", c_getContextScopedConstName(o, ".", FALSE, context));
}

static void
c_enumerationSpec(
    c_enumeration o,
    c_genArg context)
{
    c_string enumName, newName;
    c_string label, sep;
    os_size_t sz, i;
    c_longlong val;
    c_literal literal;

    enumName = c_getContextScopedTypeName(c_type(o), ".", FALSE, context);
    newName = enumNameHelper(enumName);

    dbg("Processing enum '%s' -> '%s'\n", enumName, newName);

    c_outi(context, 0, "enum %s {\n", newName);
    sz = c_arraySize(o->elements);
    for (i = 0, val = 0; i < sz; i++, val++) {
        label = c_getContextScopedConstName(c_constant(o->elements[i]), ".", FALSE, context);
        literal = c_literal(c_constant(o->elements[i])->operand);
        assert(literal->value.kind == V_LONG || literal->value.kind == V_LONGLONG);

        if (i == (sz - 1)) {
            sep = "\n";
        } else {
            sep = ",\n";
        }

        if ((literal->value.kind == V_LONG) &&
            (literal->value.is.Long != val)) {
            c_outi(context, 1, "%s = %" PA_PRId32 "%s", label, literal->value.is.Long, sep);
            val = literal->value.is.Long;
        } else if ((literal->value.kind == V_LONGLONG) &&
            (literal->value.is.LongLong != val)) {
            c_outi(context, 1, "%s = %" PA_PRId64 "%s", label, literal->value.is.LongLong, sep);
            val = literal->value.is.LongLong;
        } else {
            c_outi(context, 1, "%s%s", label, sep);
        }
    }
    os_free(newName);
    c_outi(context, 0, "}\n\n");
}

static void
c_interfaceSpec(
    c_interface o,
    c_genArg context)
{
    dbg("Skipping %p\n", o);
    c_outi(context, 0, "** INTERFACE NOT IMPLEMENTED **\n");
}

static void
c_operationSpec(
    c_operation o,
    c_genArg context)
{
    dbg("Skipping %p\n", o);
    c_outi(context, 0, "** OPERATION NOT IMPLEMENTED **\n");
}

static void
c_primitiveSpec(
    c_primitive o,
    c_genArg context)
{
    c_string name = c_metaName(c_metaObject(o));
    dbg("Processing '%s'\n", name);
    primitiveHelper(o, name, context);
}

static void
c_structureSpec(
    c_structure o,
    c_genArg context)
{
    c_ulong i;
    c_string structName;
    c_specifier s;

    structName = c_getContextScopedTypeName(c_type(o), ".", FALSE, context);
    dbg("Processing '%s' (%p)\n", structName, o);

    c_outi(context, 0, "[StructLayoutAttribute(LayoutKind.Sequential)]\n");
    c_outi(context, 0, "internal struct %s {\n", structName);
    context->level++;
    for (i = 0; i < c_arraySize(o->members); i++) {
        s = c_specifier(o->members[i]);
        c_specifierSpec(s->name, s->type, context);
    }
    context->level--;
    c_outi(context, 0, "}\n\n");
}

static void
c_typeDefSpec(
    c_typeDef o,
    c_genArg context)
{
    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(context);
    /* Struct/class members that are a typedef are resolved to their actual type. Typedef itself is ignored */
    dbg("Skipping typedef '%s'\n", c_getContextScopedTypeName(c_type(o), ".", FALSE, context));
}

static void
c_classFwdSpec(
    c_class o,
    c_genArg context)
{
    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(context);
    /* Forward declarations not required for C# */
    dbg("Skipping forward-declaration '%s'\n", c_metaName(c_metaObject(o)));
}

static void
c_unionSpec(
    c_union o,
    c_genArg context)
{
    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(context);
    /* Union is not (yet) supported */
    assert(c_baseObject(o)->kind == M_UNION);
    dbg("Skipping union '%s'\n", c_getContextScopedTypeName(c_type(o), ".", FALSE, context));
}
