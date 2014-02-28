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

/***********************************************************************
 *
 * Object-name   : c_gen
 * Component     : Type system
 * Description   : Is able to generate C source code from metadata.
 * Author        : Robert Poth
 * Date          :
 * History       :
 *
 * Algorithm     : output is generated for specified module and contains
 *                 a spec (header file) and body (code file).
 *                 All elements of the module are visited and translated
 *                 into either spec or body code.
 *                 The translation will first generate all dependecies
 *                 and then generate the actual code.
 *                 Dependencies are all types refered by the element
 *                 that is currently processed.
 *                 Dependencies will only be generated if being part of
 *                 the processed module, otherwise if it is the first
 *                 occurence of the external module a load method or
 *                 include statement will be generated.
 ***********************************************************************/
#include "os.h"
#include "os_if.h"
#include "c_genc.h"
#include "c_base.h"
#include "c_collection.h"
#include "c_iterator.h"
#include "c_metafactory.h"
#include "c_typenames.h"
/***********************************************************************
 *
 * Global function implementations
 *
 ***********************************************************************/

typedef struct c_genCArg {
    c_metaObject scope;
    FILE *stream;
    int level;
    c_bool scopedNames;
    c_iter processing;
    void (*action)(c_metaObject o, struct c_genCArg *context);
} *c_genCArg;

void c_moduleSpec         (c_module o,         c_genCArg context);
void c_typeDefSpec        (c_typeDef o,        c_genCArg context);
void c_attributeSpec      (c_attribute o,      c_genCArg context);
void c_classSpec          (c_class o,          c_genCArg context);
void c_interfaceSpec      (c_interface o,      c_genCArg context);
void c_operandSpec        (c_operand o, c_type type, c_genCArg context);
void c_constantSpec       (c_constant o,       c_genCArg context);
void c_unionCaseSpec      (c_unionCase o,      c_genCArg context);
void c_unionSpec          (c_union o,          c_genCArg context);
void c_primitiveSpec      (c_primitive o,      c_genCArg context);
void c_structureSpec      (c_structure o,      c_genCArg context);
void c_enumerationSpec    (c_enumeration o,    c_genCArg context);
void c_operationSpec      (c_operation o,      c_genCArg context);

void c_moduleBody         (c_module o,         c_genCArg context);
void c_typeDefBody        (c_typeDef o,        c_genCArg context);
void c_attributeBody      (c_attribute o,      c_genCArg context);
void c_classBody          (c_class o,          c_genCArg context);
void c_unionCaseBody      (c_unionCase o,      c_genCArg context);
void c_unionBody          (c_union o,          c_genCArg context);
void c_collectionTypeBody (c_collectionType o, c_genCArg context);
void c_structureBody      (c_structure o,      c_genCArg context);
void c_enumerationBody    (c_enumeration o,    c_genCArg context);

static void operandPrint(c_operand operand, c_long scopePrecedence,
                         c_long negLevel, c_primKind primKind,
                         c_genCArg context);
static void c_setLocale(struct lconv *aLocale);

/***********************************************************************
 * Misc
 ***********************************************************************/
static void
c_indent(
    c_genCArg context,
    int subLevel)
{
    int i;
    for (i=0; i<(context->level + subLevel); i++) {
        fprintf(context->stream, "    ");
    }
}

static void
c_out(
    c_genCArg context,
    const char *format, ...)
{
    va_list args;
    va_start(args,format);
    vfprintf(context->stream,format,args);
    va_end(args);
}

static void
c_outi(
    c_genCArg context,
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

int
c_compareProperty(
    const void *ptr1,
    const void *ptr2)
{
    c_property *p1,*p2;

    p1 = (c_property *)ptr1;
    p2 = (c_property *)ptr2;

    if (*p1 == *p2) {
        return 0;
    }
    if (*p1 == NULL) {
        return 1;
    }
    if (*p2 == NULL) {
        return -1;
    }

    return (*p1)->offset - (*p2)->offset;
}

typedef enum c_objectStateKind {
    G_UNKNOWN, G_UNDECLARED, G_DECLARED, G_EXTENDS, G_FINISHED
} c_objectStateKind;

typedef struct c_objectState {
    c_objectStateKind kind;
    c_object object;
} *c_objectState;

static c_equality
c_collectCompare(
    c_objectState o1, c_object o2)
{
    if (o1->object > o2) {
        return C_GT;
    }
    if (o1->object < o2) {
        return C_LT;
    }
    return C_EQ;
}

static c_objectStateKind
c_getObjectState(
    c_genCArg context,
    c_object o)
{
    c_objectState found;

    found = c_iterResolve(context->processing,(c_iterResolveCompare)c_collectCompare,o);
    if (found == NULL) {
        return G_UNKNOWN;
    }
    return found->kind;
}

static void
c_setObjectState(
    c_genCArg context,
    c_object o,
    c_objectStateKind kind)
{
    c_objectState template,found;

    found = c_iterResolve(context->processing,(c_iterResolveCompare)c_collectCompare,o);
    if (found != NULL) {
        found->kind = kind;
    } else {
        template = (c_objectState)os_malloc(sizeof(struct c_objectState));
        template->object = o;
        template->kind = kind;
        c_iterInsert(context->processing,template);
    }
}

/** \brief This method will generate all dependencies of the given object.
 *
 *  Dependencies are only generated if not generated before, if generated
 *  the dependencies are added to a collection of the context.
 *  The context is consulted for the generation state of objects.
 */
static void
c_genDependancies(
    c_baseObject o,
    c_genCArg context)
{
    int i;
    c_objectStateKind state;

#define _TYPEGEN_(type) context->action(c_metaObject(type),context)

    if (o == NULL) {
        return;
    }
    switch(o->kind) {
    case M_MODULE:
        c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genDependancies, context);
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
        state = c_getObjectState(context,c_class(o)->extends);
        _TYPEGEN_(c_class(o)->extends);
        if (c_interface(o)->inherits != NULL) {
            for (i=0; i<c_arraySize(c_interface(o)->inherits); i++) {
                _TYPEGEN_(c_interface(o)->inherits[i]);
            }
        }
        c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genDependancies, context);
    break;
    case M_INTERFACE:
        if (c_interface(o)->inherits != NULL) {
            for (i=0; i<c_arraySize(c_interface(o)->inherits); i++) {
                _TYPEGEN_(c_interface(o)->inherits[i]);
            }
        }
        c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genDependancies, context);
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

static c_char *
c_primitiveImageC(
    c_primitive o)
{
    switch(o->kind) {
    case P_ADDRESS:   return "c_address";
    case P_BOOLEAN:   return "c_bool";
    case P_CHAR:      return "c_char";
    case P_WCHAR:     return "c_wchar";
    case P_OCTET:     return "c_octet";
    case P_SHORT:     return "c_short";
    case P_USHORT:    return "c_ushort";
    case P_LONG:      return "c_long";
    case P_ULONG:     return "c_ulong";
    case P_LONGLONG:  return "c_longlong";
    case P_ULONGLONG: return "c_ulonglong";
    case P_FLOAT:     return "c_float";
    case P_DOUBLE:    return "c_double";
    case P_MUTEX:     return "c_mutex";
    case P_LOCK:      return "c_lock";
    case P_COND:      return "c_cond";
    case P_VOIDP:     return "c_voidp";
    default:
        assert(FALSE);
    }
    return NULL;
}

static c_char *
c_getContextScopedTypeName(
    c_type type,
    c_char *separator,
    c_bool smart,
    c_genCArg context)
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

static c_char *
c_getContextScopedConstName(
    c_constant c,
    c_char *separator,
    c_bool smart,
    c_genCArg context)
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


/* ------------------------------------- Type ---------------------------- */
void
c_genObjectSpec(
    c_metaObject o,
    c_genCArg context)
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

    /* Check if this meta object is a c_base type, if true then skip
       the generation */
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

    switch (c_getObjectState(context,o)) {
    case G_UNKNOWN:
        c_setObjectState(context,o,G_UNDECLARED);
        c_genDependancies(c_baseObject(o),context);

        /* Generate the definition for this meta object. */
#define _CASE_(t,k) case k: t##Spec(t(o),context); break
        switch(c_baseObject(o)->kind) {
        _CASE_(c_attribute,       M_ATTRIBUTE);
        _CASE_(c_class,           M_CLASS);
        _CASE_(c_constant,        M_CONSTANT);
        _CASE_(c_enumeration,     M_ENUMERATION);
        _CASE_(c_interface,       M_INTERFACE);
        _CASE_(c_module,          M_MODULE);
        _CASE_(c_operation,       M_OPERATION);
        _CASE_(c_primitive,       M_PRIMITIVE);
        _CASE_(c_structure,       M_STRUCTURE);
        _CASE_(c_typeDef,         M_TYPEDEF);
        _CASE_(c_union,           M_UNION);
        default:
        break;
        }
#undef _CASE_
        c_setObjectState(context,o,G_FINISHED);
    break;
    case G_UNDECLARED:
        /* The second time this dependency occures => then generate a forward declaration. */
        name = c_metaName(o);
        if (name != NULL) {
            switch(c_baseObject(o)->kind) {
            case M_CLASS:
                c_outi(context,0,"C_CLASS(%s);\n\n",name);
            break;
            case M_TYPEDEF:
                c_typeDefSpec(c_typeDef(o),context);
                c_genObjectSpec(c_metaObject(c_typeDef(o)->alias),context);
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

void
c_genObjectBody(
    c_metaObject o,
    c_genCArg context)
{
    c_metaObject scope, module;
    c_string name;

    /* Get the module that is being processed */
    if (c_baseObject(context->scope)->kind == M_MODULE) {
        module = context->scope;
    } else {
        module = c_metaModule(context->scope);
    }
        /* Get the module that contains the processed object. */
    scope = c_metaModule(o);

    /* Check if this meta object is a c_base type, if true then skip
       the generation */
    if (scope == NULL) {
        return;
    }

    name = c_metaName(scope);
    if (name == NULL) { /* top-level module */
        return;
    }

    /* If this meta object is defined in another module then don't
       generate a definition but instead include that module. */
    if (scope != module) {
        /* If this scope is not processed before then load it */
        if (c_getObjectState(context,scope) == G_UNKNOWN) {
            if (strncmp(name,"c_",2) != 0) {
                c_outi(context,1,"load%s(base);\n",name);
            }
            c_setObjectState(context,scope,G_FINISHED);
        }
        return;
    }

    /* If this meta object is processed before then don't generate
       a definition */
    switch (c_getObjectState(context,o)) {
    case G_UNDECLARED:
        name = c_metaName(o);
        if (name != NULL) {
            switch(c_baseObject(o)->kind) {
            case M_CLASS:
                c_outi(context,1,"c_free(c_metaDeclare(scope,\"%s\",M_CLASS));\n",name);
            break;
            case M_COLLECTION:
                c_outi(context,1,"c_metaDeclare(scope,\"%s\",M_COLLECTION);\n",name);
            break;
            default:
            break;
            }
        } else {
        }
        c_setObjectState(context,o,G_DECLARED);
    break;
    case G_UNKNOWN:
        c_setObjectState(context,o,G_UNDECLARED);
        c_genDependancies(c_baseObject(o),context);

        /* Generate the definition for this meta object. */
#define _CASE_(t,k) case k: t##Body(t(o),context); break
        switch(c_baseObject(o)->kind) {
        _CASE_(c_attribute,       M_ATTRIBUTE);
        _CASE_(c_class,           M_CLASS);
        _CASE_(c_collectionType,  M_COLLECTION);
        _CASE_(c_enumeration,     M_ENUMERATION);
        _CASE_(c_module,          M_MODULE);
        _CASE_(c_structure,       M_STRUCTURE);
        _CASE_(c_typeDef,         M_TYPEDEF);
        _CASE_(c_union,           M_UNION);
        default:
        break;
        }
#undef _CASE_
        c_setObjectState(context,o,G_FINISHED);
    break;
    default:
    break;
    }
}

void
c_specifierSpec(
    c_string name,
    c_type type,
    c_genCArg context)
{
    c_string typeName, commentName;
    c_string newName;

    if (type == NULL) {
        return;
    }
    if ((name != NULL) && (strlen(name) > 0)) {
        newName = (c_string)os_malloc(strlen(name)+2);
        os_sprintf(newName," %s",name);
    } else {
        newName = "";
    }

    commentName = c_getContextScopedTypeName(type, "::", TRUE, context);
    typeName = c_getContextScopedTypeName(type, "_", FALSE, context);

    if (typeName != NULL) {
        switch(c_baseObject(type)->kind) {
        case M_PRIMITIVE:
            c_out(context,"%s%s",c_primitiveImageC(c_primitive(type)),newName);
        break;
        case M_COLLECTION:
            switch (c_collectionType(type)->kind) {
            case C_ARRAY:
                if (c_collectionType(type)->maxSize == 0) {
                    c_out(context,"c_array %s /*%s*/",name, commentName);
                } else {
                    c_specifierSpec(name,c_collectionType(type)->subType,context);
                    c_out(context,"[%d] /*%s*/",
                          c_collectionType(type)->maxSize,commentName);
                }
            break;
            case C_SEQUENCE:
                c_out(context,"c_sequence %s /*%s*/",name, commentName);
            break;
            case C_BAG:        c_out(context,"c_bag %s",name);  break;
            case C_SET:        c_out(context,"c_set %s",name);  break;
            case C_MAP:        c_out(context,"c_map %s",name);  break;
            case C_QUERY:      c_out(context,"c_query %s",name);  break;
            case C_LIST:       c_out(context,"c_list %s",name);  break;
            case C_DICTIONARY: c_out(context,"c_table %s",name); break;
            case C_STRING:
                c_out(context,"c_string %s",name);
                if (c_collectionType(type)->maxSize > 0) {
                    c_out(context, " /*%s*/", commentName);
                }
            break;
            default:
            break;
            }
        break;
        case M_BASE:
        case M_TYPEDEF:
        case M_CLASS:
        case M_INTERFACE:
            c_out(context,"%s%s",typeName,newName);
        break;
        case M_ENUMERATION:
            c_out(context,"enum %s%s",typeName,newName);
        break;
        case M_STRUCTURE:
            c_out(context,"struct %s%s",typeName,newName);
        break;
        case M_UNION:
            c_out(context,"struct %s%s",typeName,newName);
        break;
        default:
        break;
        }
    } else {
        switch(c_baseObject(type)->kind) {
        case M_COLLECTION:
            switch (c_collectionType(type)->kind) {
            case C_ARRAY:
                if (c_collectionType(type)->maxSize == 0) {
                    c_out(context,"c_array %s",name);
                } else {
                    c_specifierSpec(name,c_collectionType(type)->subType,context);
                    c_out(context,"[%d]",c_collectionType(type)->maxSize);
                }
            break;
            case C_SEQUENCE:   c_out(context,"c_sequence %s",name); break;
            case C_BAG:        c_out(context,"c_bag %s",name);  break;
            case C_SET:        c_out(context,"c_set %s",name);  break;
            case C_MAP:        c_out(context,"c_map %s",name);  break;
            case C_QUERY:      c_out(context,"c_query %s",name);  break;
            case C_LIST:       c_out(context,"c_list %s",name);  break;
            case C_DICTIONARY: c_out(context,"c_table %s",name); break;
            case C_STRING:
                c_out(context,"char *%s",name);
            break;
            case C_WSTRING:
                c_out(context,"short *%s",name);
            break;
            default:
            break;
            }
        break;
        case M_ENUMERATION:
        case M_STRUCTURE:
        case M_UNION:
        break;
        default:
        break;
        }
    }
}

void
c_typeBody(
    c_string lh,
    c_type o,
    c_genCArg context)
{
    c_char *typeName;

    typeName = c_getContextScopedTypeName(o, "::", TRUE, context);
    c_outi(context,1,"%sResolveType(scope,\"%s\");\n",lh,typeName);
    os_free(typeName);
}

/***********************************************************************
 * Module and Type generator
 ***********************************************************************/

void
c_gen_C(
    c_module topLevel,
    c_bool scopedNames)
{
    struct c_genCArg context;
    c_type type;

    c_setLocale(localeconv());

    type = c_type(c_metaResolve(c_metaObject(topLevel),"c_object"));
    context.scope       = NULL;
    context.stream      = NULL;
    context.action      = NULL;
    context.level       = 0;
    context.scopedNames = scopedNames;

    context.processing = c_iterNew(NULL);
    context.action = c_genObjectSpec;
    c_metaWalk(c_metaObject(topLevel), (c_metaWalkAction)c_moduleSpec, &context);

    context.processing = c_iterNew(NULL);
    context.action = c_genObjectBody;
    c_metaWalk(c_metaObject(topLevel), (c_metaWalkAction)c_moduleBody, &context);
}

void
c_genIncludesSpec(
    c_metaObject o,
    c_genCArg context)
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
        if (c_getObjectState(context,scope) == G_UNKNOWN) {
            name = c_metaName(c_metaObject(scope));
            if (name) {
              c_out(context,"#include \"%s.h\"\n",name);
            }
            c_setObjectState(context,scope,G_FINISHED);
        }
    }
}

void
c_moduleSpec(
    c_module o,
    c_genCArg context)
{
    char *moduleName;
    char *streamName;
    struct c_genCArg newContext;

    if (o == NULL) {
        return;
    }
    if (c_baseObject(o)->kind != M_MODULE) {
        return;
    }
    moduleName = c_metaName(c_metaObject(o));
    if (strncmp(moduleName,"c_",2) == 0) {
        return; /* internal modules */
    }

    streamName = os_malloc(strlen(moduleName) + 3);
    os_sprintf(streamName, "%s.h", moduleName);

    newContext.scope     = c_metaObject(o);
    newContext.stream    = fopen(streamName,"w");
    newContext.level     = 0;
    newContext.scopedNames = context->scopedNames;

    /* Standard starting lines and includes */
    c_outi(&newContext,0,"\n#ifndef MODULE_%s_HEADER\n",moduleName);
    c_outi(&newContext,0,"#define MODULE_%s_HEADER\n\n",moduleName);

    c_outi(&newContext,0,"#include \"os_if.h\"\n");
    c_outi(&newContext,0,"#include \"c_base.h\"\n");
    c_outi(&newContext,0,"#include \"c_misc.h\"\n");
    c_outi(&newContext,0,"#include \"c_sync.h\"\n");
    c_outi(&newContext,0,"#include \"c_collection.h\"\n");
    c_outi(&newContext,0,"#include \"c_field.h\"\n");

    /* Generate lines for inluding header files */
    /* A new iter is created for this */
    newContext.processing = c_iterNew(NULL);
    newContext.action = c_genIncludesSpec;
    c_genDependancies(c_baseObject(o), &newContext);
    c_out(&newContext,"\n");

    c_outi(&newContext,0,"\n#ifdef MODEL_%s_IMPLEMENTATION\n",moduleName);
    c_outi(&newContext,0,"#define OS_API OS_API_EXPORT\n");
    c_outi(&newContext,0,"#else\n");
    c_outi(&newContext,0,"#define OS_API OS_API_IMPORT\n");
    c_outi(&newContext,0,"#endif\n\n");

    /* Now start the actual generation of types */
    newContext.processing = context->processing;
    newContext.action = c_genObjectSpec;

    c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genObjectSpec, &newContext);

    c_outi(&newContext,0,"OS_API c_bool\n");
    c_outi(&newContext,0,"load%s(\n",moduleName);
    c_outi(&newContext,1,"c_base base);\n\n");

    c_outi(&newContext,0,"#undef OS_API\n\n");

    c_outi(&newContext,0,"\n#endif /* MODULE_%s_HEADER */\n\n",moduleName);

    fclose(newContext.stream);
}

void
c_moduleBody(
    c_module o,
    c_genCArg context)
{
    char *moduleName;
    char *streamName;
    struct c_genCArg newContext;

    if (o == NULL) {
        return;
    }
    if (c_baseObject(o)->kind != M_MODULE) {
        return;
    }
    moduleName = c_metaName(c_metaObject(o));
    if (strncmp(moduleName,"c_",2) == 0) {
        return; /* internal modules */
    }

    streamName = os_malloc(strlen(moduleName) + 3);
    os_sprintf(streamName, "%s.c", moduleName);
    newContext.stream      = fopen(streamName,"w");
    newContext.scope       = c_metaObject(o);

    /* newContext.processing  = context->processing; */
    newContext.processing = c_iterNew(NULL);

    newContext.action      = c_genObjectBody;
    newContext.level       = 0;
    newContext.scopedNames = context->scopedNames;

    c_outi(&newContext,0,
           "#include \"%s.h\"\n\n",moduleName);
    c_outi(&newContext,0,"#define Resolve(s,o) c_metaResolve(c_metaObject(s),o)\n");
    c_outi(&newContext,0,"#define ResolveType(s,t) c_type(c_metaResolve(c_metaObject(s),t))\n");
    c_outi(&newContext,0,"#define ResolveClass(s,c) c_class(c_metaResolve(c_metaObject(s),c))\n\n");

    c_outi(&newContext, 0, "#if defined(__GNUC__)\n");
    c_outi(&newContext, 0, "#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402\n");
    c_outi(&newContext, 0, "#define OSPL_GCC_DIAG_STR(s) #s\n");
    c_outi(&newContext, 0, "#define OSPL_GCC_DIAG_JOINSTR(x,y) OSPL_GCC_DIAG_STR(x ## y)\n");
    c_outi(&newContext, 0, "#define OSPL_GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)\n");
    c_outi(&newContext, 0, "#define OSPL_GCC_DIAG_PRAGMA(x) OSPL_GCC_DIAG_DO_PRAGMA(GCC diagnostic x)\n");
    c_outi(&newContext, 0, "#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406\n");
    c_outi(&newContext, 0, "#define OSPL_DIAG_OFF(x) OSPL_GCC_DIAG_PRAGMA(push) OSPL_GCC_DIAG_PRAGMA(ignored OSPL_GCC_DIAG_JOINSTR(-W,x))\n");
    c_outi(&newContext, 0, "#define OSPL_DIAG_ON(x) OSPL_GCC_DIAG_PRAGMA(pop)\n");
    c_outi(&newContext, 0, "#else\n");
    c_outi(&newContext, 0, "#define OSPL_DIAG_OFF(x) OSPL_GCC_DIAG_PRAGMA(ignored OSPL_GCC_DIAG_JOINSTR(-W,x))\n");
    c_outi(&newContext, 0, "#define OSPL_DIAG_ON(x)  OSPL_GCC_DIAG_PRAGMA(warning OSPL_GCC_DIAG_JOINSTR(-W,x))\n");
    c_outi(&newContext, 0, "#endif\n");
    c_outi(&newContext, 0, "#else\n");
    c_outi(&newContext, 0, "#define OSPL_DIAG_OFF(x)\n");
    c_outi(&newContext, 0, "#define OSPL_DIAG_ON(x)\n");
    c_outi(&newContext, 0, "#endif\n");
    c_outi(&newContext, 0, "#else\n");
    c_outi(&newContext, 0, "#define OSPL_DIAG_OFF(x)\n");
    c_outi(&newContext, 0, "#define OSPL_DIAG_ON(x)\n");
    c_outi(&newContext, 0, "#endif\n\n");

    c_outi(&newContext, 0, "OSPL_DIAG_OFF(unused-variable)\n\n");

    c_outi(&newContext,0,"c_bool\n");
    c_outi(&newContext,0,"load%s(\n",moduleName);
    c_outi(&newContext,1,"c_base base)\n{\n");
    c_outi(&newContext,1,"c_object module;\n");
    c_outi(&newContext,1,"c_object scope;\n");
    c_outi(&newContext,1,"c_object type;\n");
    c_outi(&newContext,1,"c_object *members;\n");
    c_outi(&newContext,1,"c_object *cases;\n");
    c_outi(&newContext,1,"c_object *labels;\n");
    c_outi(&newContext,1,"c_bool result = FALSE;\n");
    c_outi(&newContext,1,"c_object o,found;\n\n");

    c_outi(&newContext,1,
          "module = c_metaDeclare(c_object(base),\"%s\",M_MODULE);\n\n",
           moduleName);
    c_outi(&newContext,1,"scope = module;\n");

    c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_genObjectBody, &newContext);

    c_outi(&newContext,1,"result = TRUE;\n");
    c_outi(&newContext,1,"c_free(module);\n");
    c_outi(&newContext,1,"return result;\n");
    c_outi(&newContext,0,"}\n\n");

    fclose(newContext.stream);
}

void
c_typeDefSpec(
    c_typeDef o,
    c_genCArg context)
{
    c_string name;


    name = c_getContextScopedTypeName(c_type(o), "_", FALSE, context);

    if (strcmp(name,"v_pointer") == 0) {
        c_outi(context,0,"typedef c_object v_pointer;\n\n");
    } else {
        c_outi(context,0,"typedef ");
        c_specifierSpec(name, o->alias, context);
        c_out(context,";\n\n");
    }
}

void
c_typeDefBody(
    c_typeDef o,
    c_genCArg context)
{
    c_string name;

    name = c_getContextScopedTypeName(c_type(o), "::", TRUE, context);

    c_outi(context,1,"o = c_metaDefine(scope,M_TYPEDEF);\n");
    c_typeBody("c_typeDef(o)->alias = ",c_typeDef(o)->alias,context);
    c_outi(context,1,"c_metaObject(o)->definedIn = scope;\n");
    c_outi(context,1,"c_metaFinalize(o);\n");
    c_outi(context,1,"found = c_metaBind(scope,\"%s\",o);\n",name);
    c_outi(context,1,"c_free(o);\n");
    c_outi(context,1,"if (found == NULL) { c_free(module); return result; }\n");
    c_outi(context,1,"c_free(found);\n\n");
}

void
c_constantSpec(
    c_constant o,
    c_genCArg context)
{
    c_char *name;

    if (c_baseObject(o)->kind != M_CONSTANT) {
        return;
    }
    if (c_baseObject(o->type)->kind != M_ENUMERATION) {
        name = c_getContextScopedConstName(o, "_", FALSE, context);
        c_outi(context,0,"#define %s", name);
        c_out(context," (");
        c_operandSpec(o->operand, o->type, context);
        c_out(context,")\n");
    }
}

void
c_unionSpec(
    c_union o,
    c_genCArg context)
{
    int i;
    c_specifier s;
    c_string unionName;

    unionName = c_getContextScopedTypeName(c_type(o), "_", FALSE, context);

    c_outi(context,0,"struct %s {\n", unionName);
    context->level++;
    c_indent(context,0);
    c_specifierSpec("_d", o->switchType, context);
    c_out(context,";\n");
    c_outi(context,0, "union {\n");
    context->level++;
    for (i=0; i<c_arraySize(o->cases); i++) {
        s = c_specifier(o->cases[i]);
        c_indent(context,0);
        c_specifierSpec(s->name, s->type, context);
        c_out(context, ";\n");
    }
    context->level--;
    c_outi(context,0,"} _u;\n");
    context->level--;
    c_outi(context,0,"};\n\n");
}

void
c_unionBody(
    c_union o,
    c_genCArg context)
{
    int i, j, nLabels, ie, index;
    c_specifier s;
    c_unionCase uc;
    c_literal l, l_enum;
    c_bool enumSwitch;
    c_primKind switchKind;
    c_string unionName, typeName, switchTypeName;
    c_enumeration enumerationType;
    c_type sType;

    unionName = c_getContextScopedTypeName(c_type(o), "::", TRUE, context);

    /* For now, enum switch types are not yet allowed */
    sType = c_typeActualType(o->switchType);
    switch (c_baseObject(sType)->kind) {
    case M_PRIMITIVE:
        enumSwitch = FALSE;
        enumerationType = NULL;
        switchKind = c_primitive(sType)->kind;
    break;
    case M_ENUMERATION:
        enumSwitch = TRUE;
        enumerationType = c_enumeration(sType);
        switchKind = P_UNDEFINED;
    break;
    default:
        enumSwitch = FALSE;
        switchKind = NULL;
        enumerationType = NULL;
        assert(0); /* Only primitives and enums as switch types */
    }

    switchTypeName = c_getContextScopedTypeName(sType, "::", TRUE, context);

    c_outi(context,1,"o = c_metaDefine(scope,M_UNION);\n");
    c_outi(context,2,"c_union(o)->switchType = ResolveType(scope,\"%s\");\n",
                      switchTypeName);
    c_outi(context,2,"cases = c_arrayNew(ResolveType(base,\"c_object\"),%d);\n",
                      c_arraySize(o->cases));
    for (i=0; i<c_arraySize(o->cases); i++) {
        uc = o->cases[i];
        nLabels = c_arraySize(uc->labels);
        if (nLabels == 0) {
            c_outi(context,2,"/* The default case */\n");
        }
        s = c_specifier(uc);

        typeName = c_getContextScopedTypeName(s->type, "::", TRUE, context);
        c_outi(context,2,"cases[%d] = ",i);
        c_out(context,"(c_voidp)c_metaDefine(scope,M_UNIONCASE);\n");
        c_outi(context,2,"c_specifier(cases[%d])->name = ",i);
        c_out(context,"c_stringNew(base,\"%s\");\n",s->name);
        c_outi(context,2,"c_specifier(cases[%d])->type = ",i);
        c_out(context,"ResolveType(scope,\"%s\");\n",typeName);
        if (nLabels > 0) {
            c_outi(context,2,"labels = c_arrayNew(ResolveType(base,\"c_object\"),%d);\n",
                              c_arraySize(uc->labels));
            for (j=0; j<c_arraySize(uc->labels); j++) {
                l = c_literal(uc->labels[j]);
                c_outi(context,2,"labels[%d] = ",j);
                c_out(context,"(c_voidp)c_metaDefine(scope,M_LITERAL);\n");
                c_outi(context,2,"c_literal(labels[%d])->value = ",j);
                if (enumSwitch) {
                    /* First determine the index of the enum-value */
                    index = -1;
                    for (ie=0; (ie<c_arraySize(enumerationType->elements)) && (index == -1); ie++) {
                        l_enum = c_operandValue(c_constant(enumerationType->elements[ie])->operand);
                        assert(l_enum->value.kind == V_LONG);
                        if (c_valueCompare(l->value, l_enum->value) == C_EQ) {
                            index = ie;
                        }
                        c_free(l_enum);
                    }
                    assert (index != -1);
                    c_out(context, "c_literal(c_constant(c_enumeration(c_union(o)->switchType)->elements[%d])->operand)->value;\n", index);
                } else {
                    switch (switchKind) {
                    case P_BOOLEAN: c_out(context,"c_boolValue"); break;
                    case P_CHAR: c_out(context,"c_charValue"); break;
                    case P_OCTET: c_out(context,"c_octetValue"); break;
                    case P_SHORT: c_out(context,"c_shortValue"); break;
                    case P_USHORT: c_out(context,"c_ushortValue"); break;
                    case P_LONG: c_out(context,"c_longValue"); break;
                    case P_ULONG: c_out(context,"c_ulongValue"); break;
                    case P_LONGLONG: c_out(context,"c_longlongValue"); break;
                    case P_ULONGLONG: c_out(context,"c_ulonglongValue"); break;
                    default: assert(0);
                    }
                    if (switchKind == P_CHAR) {
                        c_out(context, "('%s');\n",c_valueImage(l->value));
                    } else {
                        c_out(context, "(%s);\n",c_valueImage(l->value));
                    }
                }
            }
            c_outi(context,2,"c_unionCase(cases[%d])->labels = labels;\n",i);
        } else {
            c_outi(context,2,"c_unionCase(cases[%d])->labels = NULL;\n",i);
        }
    }
    c_outi(context,2,"c_union(o)->cases = cases;\n");
    c_outi(context,1,"c_metaObject(o)->definedIn = scope;\n");
    c_outi(context,1,"c_metaFinalize(o);\n");
    c_outi(context,1,"found = c_metaBind(scope,\"%s\",o);\n",unionName);
    c_outi(context,1,"c_free(o);\n");
    c_outi(context,1,"if (found == NULL) { c_free(module); return result; }\n");
    c_outi(context,1,"c_free(found);\n\n");
}

void
c_collectionTypeBody(
    c_collectionType o,
    c_genCArg context)
{
    c_string name, kind;

    name = c_metaName(c_metaObject(o));

    c_outi(context,1,"o = c_metaDefine(scope,M_COLLECTION);\n");
    context->level++;

#define _CASE_(k) case k: kind = #k; break
    switch(o->kind) {
    _CASE_(C_ARRAY);
    _CASE_(C_SEQUENCE);
    _CASE_(C_SET);
    _CASE_(C_LIST);
    _CASE_(C_BAG);
    _CASE_(C_MAP);
    _CASE_(C_QUERY);
    _CASE_(C_SCOPE);
    _CASE_(C_DICTIONARY);
    _CASE_(C_STRING);
    _CASE_(C_WSTRING);
    default:
        assert(FALSE);
        c_outi(context,1,"assert(FALSE); /* Illegal collection kind: */\n");
        kind = "C_UNDEFINED";
    break;
    }
#undef _CASE_

    c_outi(context,1,"c_collectionType(o)->kind = %s;\n",kind);
    c_typeBody("c_collectionType(o)->subType = ",o->subType,context);
    c_outi(context,1,"c_collectionType(o)->maxSize = %d;\n",o->maxSize);
    context->level--;
    c_outi(context,1,"c_metaObject(o)->definedIn = scope;\n");
    c_outi(context,1,"c_metaFinalize(o);\n");
    c_outi(context,1,"found = c_metaBind(scope,\"%s\",o);\n",name);
    c_outi(context,1,"c_free(o);\n");
    c_outi(context,1,"if (found == NULL) { c_free(module); return result; }\n");
    c_outi(context,1,"c_free(found);\n\n");
    c_free(name);
}

void
c_primitiveSpec(
    c_primitive o,
    c_genCArg context)
{
    c_string name = c_metaName(c_metaObject(o));
    c_outi(context,0,"%s %s;\n", c_primitiveImageC(o), name);
}

void
c_structureSpec(
    c_structure o,
    c_genCArg context)
{
    int i;
    c_string structName;
    c_specifier s;

    structName = c_getContextScopedTypeName(c_type(o), "_", FALSE, context);

    c_outi(context,0,"struct %s {\n", structName);
    context->level++;
    for (i=0; i<c_arraySize(o->members); i++) {
        c_indent(context,0);
        s = c_specifier(o->members[i]);
        c_specifierSpec(s->name,s->type,context);
        c_out(context,";\n");
    }
    context->level--;
    c_outi(context,0,"};\n\n");
}

void
c_structureBody(
    c_structure o,
    c_genCArg context)
{
    int i;
    c_string structName;
    c_char *typeName;
    c_specifier s;

    structName = c_getContextScopedTypeName(c_type(o), "::", TRUE, context);

    c_outi(context,1,"o = c_metaDefine(scope,M_STRUCTURE);\n");
    c_outi(context,2,"members = c_arrayNew(ResolveType(base,\"c_object\"),%d);\n",
                      c_arraySize(o->members));
    for (i=0; i<c_arraySize(o->members); i++) {
        s = c_specifier(o->members[i]);
        typeName = c_getContextScopedTypeName(s->type, "::", TRUE, context);
        c_outi(context,2,"members[%d] = ",i);
        c_out(context,"(c_voidp)c_metaDefine(scope,M_MEMBER);\n");
        c_outi(context,2,"c_specifier(members[%d])->name = ",i);
        c_out(context,"c_stringNew(base,\"%s\");\n",s->name);
        c_outi(context,2,"c_specifier(members[%d])->type = ",i);

        c_out(context,"ResolveType(scope,\"%s\");\n",typeName);
    }
    c_outi(context,2,"c_structure(o)->members = members;\n");
    c_outi(context,1,"c_metaObject(o)->definedIn = scope;\n");
    c_outi(context,1,"c_metaFinalize(o);\n");
    c_outi(context,1,"found = c_metaBind(scope,\"%s\",o);\n",structName);
    c_outi(context,1,"c_free(o);\n");
    c_outi(context,1,"if (found == NULL) { c_free(module); return result; }\n");
    c_outi(context,1,"c_free(found);\n\n");
}

void
c_enumerationSpec(
    c_enumeration o,
    c_genCArg context)
{
    int i,size;
    c_string label;
    c_string enumName;

    enumName = c_getContextScopedTypeName(c_type(o), "_", FALSE, context);

    c_outi(context,0,"typedef enum %s {\n",enumName);
    size = c_arraySize(o->elements);
    for (i=0; i<size; i++) {
        label = c_getContextScopedConstName(c_constant(o->elements[i]),
                                            "_", FALSE, context);
        /* label = c_metaName(c_metaObject(o->elements[i])); */
        if (i == (size-1)) {
            c_outi(context,1,"%s\n", label);
        } else {
            c_outi(context,1,"%s,\n", label);
        }
    }
    c_outi(context,0,"} %s;\n\n",enumName);
}

void
c_enumerationBody(
    c_enumeration o,
    c_genCArg context)
{
    int i;
    c_string label;
    c_string enumName;

    enumName = c_getContextScopedTypeName(c_type(o), "::", TRUE, context);
    c_outi(context,1,"o = c_metaDefine(scope,M_ENUMERATION);\n");
    c_outi(context,1,
           "c_enumeration(o)->elements = c_arrayNew(ResolveType(base,\"c_object\"),%d);\n",
            c_arraySize(o->elements));
    for (i=0; i<c_arraySize(o->elements); i++) {
        label = c_metaName(c_metaObject(o->elements[i]));
        c_outi(context,1,"c_enumeration(o)->elements[%d] =\n",i);
        c_outi(context,2,"(c_voidp)c_metaDeclare(scope,\"%s\",M_CONSTANT);\n",label);
    }
    c_outi(context,1,"c_metaObject(o)->definedIn = scope;\n");
    c_outi(context,1,"if (c_metaFinalize(o) == S_ACCEPTED) {\n");
    c_outi(context,1,"    found = c_metaBind(scope,\"%s\",o);\n",enumName);
    c_outi(context,1,"} else {\n");
    c_outi(context,1,"    found = NULL;\n");
    c_outi(context,1,"}\n");
    c_outi(context,1,"c_free(o);\n");
    c_outi(context,1,"if (found == NULL) { c_free(module); return result; }\n");
    c_outi(context,1,"c_free(found);\n\n");
}

void
c_operationSpec(
    c_operation o,
    c_genCArg context)
{
    c_specifier s;
    int i;

    if (c_baseObject(o)->kind != M_OPERATION) {
        return;
    }

    c_indent(context,0);
    c_specifierSpec(c_metaName(c_metaObject(o)),o->result,context);
    if (o->parameters == NULL) {
        c_out(context, "();\n\n");
        return;
    }
    c_out(context,"(\n");
    for (i=0; i<c_arraySize(o->parameters); i++) {
        c_indent(context,2);
        s = c_specifier(o->parameters[i]);
        c_specifierSpec(s->name,s->type,context);
        if (i != c_arraySize(o->parameters)-1) {
            c_out(context, ",\n");
        }
    }
    c_outi(context,1,");\n\n");
}

void
c_attributeBody(
    c_attribute o,
    c_genCArg context)
{
    if (c_baseObject(o)->kind != M_ATTRIBUTE) {
        return;
    }
    c_outi(context,1,"o = c_metaDefine(scope,M_ATTRIBUTE);\n");
    c_typeBody("c_property(o)->type = ",c_property(o)->type,context);
    c_outi(context,1,"found = c_metaBind(scope,\"%s\",o);\n",
                      c_metaName(c_metaObject(o)));
    c_outi(context,1,"c_free(o);\n");
    c_outi(context,1,"if (found == NULL) { c_free(module); return result; }\n");
    c_outi(context,1,"c_free(found);\n\n");
}

void
c_attributeSpec(
    c_attribute o,
    c_genCArg context)
{
    if (c_baseObject(o)->kind != M_ATTRIBUTE) {
        return;
    }
    c_indent(context,0);
    c_specifierSpec(c_metaName(c_metaObject(o)),c_property(o)->type,context);
    c_out(context,";\n");
}

void
c_propertyBody(
    c_property o,
    c_genCArg context)
{
    switch(c_baseObject(o)->kind) {
    case M_ATTRIBUTE: c_attributeBody(c_attribute(o),context); break;
    default:
    break;
    }
}

void
c_propertySpec(
    c_property o,
    c_genCArg context)
{
    switch(c_baseObject(o)->kind) {
    case M_ATTRIBUTE: c_attributeSpec(c_attribute(o),context); break;
    default:
    break;
    }
}

void
getProperties(
    c_metaObject o,
    c_iter iter)
{
    switch(c_baseObject(o)->kind) {
    case M_ATTRIBUTE:
    case M_RELATION:
        c_iterInsert(iter,o);
    break;
    default:
    break;
    }
}

void
c_classBody(
    c_class o,
    c_genCArg context)
{
    int i;
    c_string className;
    c_metaObject scope;
    c_type type;
    c_string typeName;

    className = c_metaName(c_metaObject(o));

    if (c_getObjectState(context,o) == G_FINISHED) {
        return;
    }

    type = c_type(o->extends);
    if (type != NULL) {
        if (c_getObjectState(context,type) == G_DECLARED) {
            c_genDependancies(c_baseObject(type),context);
            c_classBody(c_class(type),context);
            c_setObjectState(context,type,G_FINISHED);
        }
    }

    scope = context->scope;
    context->scope = c_metaObject(o);
    context->level++;

    c_outi(context,1,"o = c_metaDefine(scope,M_CLASS);\n");
    c_outi(context,1,"c_metaObject(o)->definedIn = scope;\n");
    c_outi(context,1,"scope = o;\n");

    type = c_type(o->extends);
    if (type != NULL) {

        while (c_baseObject(type)->kind == M_TYPEDEF) type = c_typeDef(type)->alias;
        typeName = c_getContextScopedTypeName(type, "::", TRUE, context);
        c_outi(context,1,
              "c_class(scope)->extends = ResolveClass(scope,\"%s\");\n",
               typeName);
    } else {
        c_outi(context,1,"c_class(scope)->extends = NULL;\n");
    }

    if (o->keys != NULL) {
        for (i=0; i<c_arraySize(o->keys); i++) {
        }
    }
    c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_propertyBody, context);
    context->scope = scope;
    context->level--;
    c_outi(context,1,"c_metaFinalize(scope);\n");
    c_outi(context,1,"o = scope;\n");
    c_outi(context,1,"scope = c_metaObject(scope)->definedIn;\n");
    c_outi(context,1,"found = c_metaBind(scope,\"%s\",o);\n",className);
    c_outi(context,1,"c_free(o);\n");
    c_outi(context,1,"if (found == NULL) { c_free(module); return result; }\n");
    c_outi(context,1,"c_free(found);\n\n");
}


void
c_classSpec(
    c_class o,
    c_genCArg context)
{
    int i;
    c_string className;
    c_metaObject scope;
    c_type type;
    c_iter iter;
    c_long length;
    c_property *properties;

    className = c_metaName(c_metaObject(o));

    if (!c_isFinal((c_metaObject)c_baseObject(o))) {
        return;
    }

    if (c_getObjectState(context,o) == G_FINISHED) {
        return;
    }

    type = c_type(o->extends);
    if (type != NULL) {
        if (c_getObjectState(context,type) == G_DECLARED) {
            c_classSpec(c_class(type),context);
            c_setObjectState(context,type,G_FINISHED);
        }
    }

    /* If not yet declared then now is the time do do it */
    switch (c_getObjectState(context,o)) {
    case G_UNKNOWN:
    case G_UNDECLARED:
        c_outi(context,0,"C_CLASS(%s);\n\n",className);
    default:
    break;
    }

    scope = context->scope;
    context->scope = c_metaObject(o);
    if (o->keys != NULL) {
        c_outi(context,0,"/* Keys: ");
        for (i=0; i<c_arraySize(o->keys); i++) {
            c_out(context," %s",o->keys[i]);
        }
        c_out(context," */\n");
    }
    c_outi(context,0,"C_STRUCT(%s) {\n", className);
    context->level++;
    if (type != NULL) {
        while (c_baseObject(type)->kind == M_TYPEDEF) type = c_typeDef(type)->alias;
        c_outi(context,0,"C_EXTENDS(%s);\n",c_metaName(c_metaObject(type)));
    }

    iter = c_iterNew(NULL);
    c_metaWalk(c_metaObject(o),(c_metaWalkAction)getProperties,iter);
    length = c_iterLength(iter);
    if (length > 0) {
        properties = (c_property *)os_malloc(sizeof(c_property) * length);
        for (i=0; i<length; i++) {
            properties[i] = c_iterTakeFirst(iter);
        }
        qsort(properties,length,sizeof(c_property),c_compareProperty);
        for (i=0; i<length; i++) {
             c_propertySpec(c_property(properties[i]),context);
        }
        os_free(properties);
    }

    context->level--;
    c_outi(context,0,"};\n\n");

    c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_operationSpec, context);
    context->scope = scope;
}

void
c_interfaceSpec(
    c_interface o,
    c_genCArg context)
{
    c_string name;

    name = c_metaName(c_metaObject(o));

    /* If not yet declared then now is the time do do it */
    switch (c_getObjectState(context,o)) {
    case G_UNKNOWN:
    case G_UNDECLARED:
        c_outi(context,0,"C_CLASS(%s);\n\n",name);
    default:
    break;
    }


    c_outi(context,0,"C_STRUCT(%s) {\n", name);
    context->level++;
    c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_propertySpec, context);
    context->level--;
    c_outi(context,0,"};\n\n");
    c_metaWalk(c_metaObject(o), (c_metaWalkAction)c_operationSpec, context);
}

void
c_operandSpec(
    c_operand operand,
    c_type type,
    c_genCArg context)
{
    c_type actualType;

    actualType = c_typeActualType(type);

    if (c_baseObject(actualType)->kind == M_PRIMITIVE) {
        operandPrint(operand, 0, 0, c_primitive(actualType)->kind, context);
    } else {
        if ((c_baseObject(actualType)->kind == M_COLLECTION) &&
            (c_collectionType(actualType)->kind == C_STRING)) {
            operandPrint(operand, 0, 0, P_UNDEFINED, context);
       } else {
           assert(0);
       }
    }
}

/* ------------------------------ Operand printing ------------------------- */

static struct lconv *locale;
static void
c_setLocale(
    struct lconv *aLocale)
{
    locale = aLocale;
}

static struct lconv*
c_getLocale()
{
    return locale;
}

static c_long
precedence(
    c_exprKind exprKind,
    c_bool unary)
{
    c_long result = 0;

    switch(exprKind){
    case E_NOT:
        result = 6;
    break;
    case E_PLUS:
    case E_MINUS:
        if (unary) {
            result = 6;
        } else {
            result = 4;
        };
    break;
    case E_MUL:
    case E_DIV:
    case E_MOD:
        result = 5;
    break;
    case E_SHIFTRIGHT:
    case E_SHIFTLEFT:
        result = 3;
    break;
    case E_AND:
        result = 2;
    break;
    case E_XOR:
        result = 1;
    break;
    case E_OR:
        result = 0;
    break;
    default:
        assert(0);
    }
    /* Avoid compiler warnings */
    return result;
}

static c_bool
isDigitsOnly(
    c_string num)
{
    while (*num) {
        if ((*num < '0') || (*num > '9')) {
            return FALSE;
        }
        num++;
    }
    return TRUE;
}

static void
operandPrint(
    c_operand operand,
    int scopePrecedence,
    int negLevel,
    c_primKind primKind,
    c_genCArg context)
{
    c_string s;
    int currentPrecedence;
    c_expression expression;
    c_bool parentheses;
    c_value value;

    switch(c_baseObject(operand)->kind) {
    case M_LITERAL:
        value = c_literal(operand)->value;
        switch (value.kind) {
        case V_CHAR:
            s = c_valueImage(value);
            c_out(context,"\'%s\'",s);
        break;
        case V_STRING:
            s = c_valueImage(value);
            c_out(context,"\"%s\"",s);
        break;
        default:
          /* All values are stored as either longlong or double */
            assert((value.kind == V_LONGLONG) ||
                   (value.kind == V_DOUBLE) ||
                   (value.kind == V_BOOLEAN));
            switch(primKind) {
            case P_OCTET:
                s = c_valueImage(c_octetValue((c_octet)value.is.LongLong));
                c_out(context, s);
            break;
            break;
            case P_SHORT:
                s = c_valueImage(c_shortValue((c_short)value.is.LongLong));
                c_out(context, s);
            break;
            case P_USHORT:
                s = c_valueImage(c_ushortValue((c_ushort)value.is.LongLong));
                c_out(context, s);
            break;
            case P_LONG:
                s = c_valueImage(c_ulongValue((c_long)value.is.LongLong));
                c_out(context, s);
            break;
            case P_ULONG:
                s = c_valueImage(c_ulongValue((c_ulong)value.is.LongLong));
                c_out(context, "%sU", s);
            break;
            case P_LONGLONG:
                s = c_valueImage(value);
                c_out(context, "%sLL", s);
            break;
            case P_ULONGLONG:
                s = c_valueImage(c_ulonglongValue(value.is.ULongLong));
                c_out(context, "%sULL", s);
            break;
            case P_FLOAT:
                switch (value.kind) {
                case V_DOUBLE:
                    s = c_valueImage(c_floatValue((c_float)value.is.Double));
                    c_out(context, s);
                    if (isDigitsOnly(s)) {
                        c_out(context, "%s0", c_getLocale()->decimal_point);
                    }
                break;
                case V_LONGLONG:
                    c_out(context, "%s%s0", c_valueImage(value), c_getLocale()->decimal_point);
                break;
                default: assert(0);
                }
                c_out(context, "F");
            break;
            case P_DOUBLE:
                switch (value.kind) {
                case V_DOUBLE:
                    s = c_valueImage(value);
                    c_out(context, s);
                    if (isDigitsOnly(s)) {
                        c_out(context, "%s0", c_getLocale()->decimal_point);
                    }
                break;
                case V_LONGLONG:
                    c_out(context, "%s%s0", c_valueImage(value), c_getLocale()->decimal_point);
                break;
                default: assert(0);
                }
            break;
            case P_BOOLEAN:
                s = c_valueImage(value);
                c_out(context, s);
            break;
            default:
                assert(0);
            }
        break;
        }
    break;
    case M_CONSTANT:
    {
        c_char *name;

        name = c_getContextScopedConstName(
                  c_constant(operand), "_", FALSE, context);
        c_out(context, "%s", name);
        free(name);
    }
    break;
    case M_EXPRESSION:
#define PRINTOPERAND1(operandStr) \
    do \
    { \
        c_out(context, operandStr); \
        operandPrint(c_operand(expression->operands[0]),\
            currentPrecedence, \
            negLevel, \
            primKind, \
            context); \
    } \
    while(0)
#define PRINTOPERAND2(operandStr) \
    do \
    {\
        operandPrint(c_operand(expression->operands[0]),\
            currentPrecedence, \
            negLevel, \
            primKind, \
            context); \
        c_out(context, operandStr); \
        operandPrint(c_operand(expression->operands[1]),\
            currentPrecedence, \
            negLevel, \
            primKind, \
            context); \
    } \
    while(0)

        expression = c_expression(operand);
        currentPrecedence = precedence(expression->kind,
            c_arraySize(expression->operands) == 1);
        parentheses = (currentPrecedence < scopePrecedence) ||
                      /* Parentheses after minus-sign */
                      (negLevel == 1) ||
                      /* Parentheses if several unary operators in a row */
                      ((currentPrecedence == scopePrecedence) &&
                       (currentPrecedence == precedence(E_PLUS, TRUE)));

        if (parentheses) {
            c_out(context, "(");
        }

        if (negLevel > 0) {
            negLevel++;
        }

        switch(expression->kind) {
        case E_OR:
            PRINTOPERAND2(" | ");
        break;
        case E_XOR:
            PRINTOPERAND2(" ^ ");
        break;
        case E_AND:
            PRINTOPERAND2(" & ");
        break;
        case E_SHIFTRIGHT:
            PRINTOPERAND2(" >> ");
        break;
        case E_SHIFTLEFT:
            PRINTOPERAND2(" << ");
        break;
        case E_PLUS:
            switch(c_arraySize(expression->operands)){
            case 1:
                PRINTOPERAND1("+");
            break;
            case 2:
                if (primKind != P_UNDEFINED) {
                    PRINTOPERAND2(" + ");
                } else {
                    /* Special case: concatenated string constant */
                    PRINTOPERAND2(" \\\n");
                }
            break;
            default:
                assert(0);
            };
        break;
        case E_MINUS:
          /* Special case: the last operand of E_MINUS sometimes
             has to be surrounded by parentheses, even if the
             precedence is the same */
            switch(c_arraySize(expression->operands)){
            case 1:
                c_out(context, "-");
                operandPrint(c_operand(expression->operands[0]),
                    currentPrecedence, 1, primKind, context);
            break;
            case 2:
                operandPrint(c_operand(expression->operands[0]),
                    currentPrecedence, negLevel, primKind, context);
                c_out(context, " - ");
                operandPrint(c_operand(expression->operands[1]),
                    currentPrecedence, 1, primKind, context);
            break;
            default:
                assert(0);
            };
        break;
        case E_MUL:
            PRINTOPERAND2(" * ");
        break;
        case E_DIV:
            operandPrint(c_operand(expression->operands[0]),
                currentPrecedence, negLevel, primKind, context);
            c_out(context, " / ");
            operandPrint(c_operand(expression->operands[1]),
                currentPrecedence, 1, primKind, context);
        break;
        case E_MOD:
            operandPrint(c_operand(expression->operands[0]),
                currentPrecedence, negLevel, primKind, context);
            c_out(context, " % ");
            operandPrint(c_operand(expression->operands[1]),
                currentPrecedence, 1, primKind, context);
        break;
        case E_NOT:
            PRINTOPERAND1("!");
        break;
        default:
            assert(0);
        };

        if (parentheses) {
            c_out(context, ")");
        };
#undef PRINTOPERAND1
#undef PRINTOPERAND2

    break;
    default:
        assert(0);
    };
}
