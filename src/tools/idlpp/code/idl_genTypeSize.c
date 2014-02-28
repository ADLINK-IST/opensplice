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
/*
   This module generates Splice type definitions related to
   an IDL input file.
*/

#include "idl_program.h"
/**
 * @file
 * This module generates Standalone C data types
 * related to an IDL input file.
*/

#include "os.h"
#include "c_base.h"
#include "c_sync.h"
#include "c_iterator.h"
#include "c_stringSupport.h"
#include "c_module.h"
#include "c_typebase.h"
#include "c_metabase.h"
#include "idl_fileMap.h"
#include "idl_keyDef.h"
#include "idl_walk.h"

#define HEADERSIZE (8)
#define ARRAYHEADERSIZE (16)

#define DEFAULT_WORST_CASE_STRING_SIZE (20)
#define DEFAULT_WORST_CASE_SEQUENCE_SIZE (1)

static c_long nr_of_unbounded_sequences = 0;
static c_long nr_of_unbounded_strings = 0;

static const c_char *
metaKindImage (
    c_metaKind kind)
{
#define _CASE_(o) case o: return #o
    switch (kind) {
    _CASE_(M_UNDEFINED);
    _CASE_(M_ANNOTATION);
    _CASE_(M_ATTRIBUTE);
    _CASE_(M_CLASS);
    _CASE_(M_COLLECTION);
    _CASE_(M_CONSTANT);
    _CASE_(M_CONSTOPERAND);
    _CASE_(M_ENUMERATION);
    _CASE_(M_EXCEPTION);
    _CASE_(M_EXPRESSION);
    _CASE_(M_INTERFACE);
    _CASE_(M_LITERAL);
    _CASE_(M_MEMBER);
    _CASE_(M_MODULE);
    _CASE_(M_OPERATION);
    _CASE_(M_PARAMETER);
    _CASE_(M_PRIMITIVE);
    _CASE_(M_RELATION);
    _CASE_(M_BASE);
    _CASE_(M_STRUCTURE);
    _CASE_(M_TYPEDEF);
    _CASE_(M_UNION);
    _CASE_(M_UNIONCASE);
    _CASE_(M_COUNT);
    default:
        return "Unknown metaKind specified";
            }
#undef _CASE_
}

c_long
c_typeMaxSize(
    c_type type);

c_long
c_collectionType_maxSize(
    c_collectionType o)
{
    c_long maxSize;
    c_type subType;
    c_char *scopedName = NULL;

    maxSize = c_collectionTypeMaxSize(o);
    if (maxSize == 0) {
        switch (c_collectionType(o)->kind) {
        case C_STRING:
        case C_WSTRING:
            nr_of_unbounded_strings++;
            maxSize = DEFAULT_WORST_CASE_STRING_SIZE;
        break;
        case C_ARRAY:
        case C_SEQUENCE:
            nr_of_unbounded_sequences++;
            maxSize = DEFAULT_WORST_CASE_SEQUENCE_SIZE;
        break;
        default:
            scopedName = c_metaScopedName(c_metaObject(o));
            printf("Found unexpected unbounded collection type: %s\n", scopedName);
            os_free(scopedName);
        break;
        }
    }
    subType = c_collectionTypeSubType(o);
    maxSize *= c_typeSize(subType);
    return maxSize;
}

c_long
c_structure_maxSize(
    c_structure o)
{
    c_long i;
    c_specifier s;
    c_long maxSize = 0;

    maxSize = c_typeSize(c_type(o));
    for (i=0; i<c_arraySize(o->references); i++) {
        s = c_specifier(o->references[i]);
        maxSize += c_typeMaxSize(s->type);
    }
    return maxSize;
}

c_long
c_typeDef_maxSize(
    c_typeDef o)
{
    return c_typeMaxSize(o->alias);
}

c_long
c_union_maxSize(
    c_union o)
{
    c_long i;
    c_specifier s;
    c_long maxSize = 0;

    c_long size = 0;

    for (i=0; i<c_arraySize(o->cases); i++) {
        s = c_specifier(o->cases[i]);
        size = c_typeMaxSize(s->type);
        maxSize = (size > maxSize ? size : maxSize);
    }
    maxSize += c_typeSize(c_type(o));
    return maxSize;
}

c_long
c_typeMaxSize(
    c_type type)
{
    c_long maxSize = 0;

#define _CASE_(t,k) case k: maxSize = t##_maxSize(t(type)); break
    switch(c_baseObject(type)->kind) {
    _CASE_(c_structure,       M_STRUCTURE);
    _CASE_(c_collectionType,  M_COLLECTION);
    _CASE_(c_typeDef,         M_TYPEDEF);
    _CASE_(c_union,           M_UNION);
#undef _CASE_
#define _CASE_(t,k) case k: maxSize = c_typeSize(type); break
    _CASE_(c_primitive,       M_PRIMITIVE);
    _CASE_(c_enumeration,     M_ENUMERATION);
    _CASE_(c_constant,        M_CONSTANT);
    _CASE_(c_interface,       M_INTERFACE);
    default:
        printf("c_typeMaxSize kind = %s\n", metaKindImage(c_baseObject(type)->kind));
    break;
    }
#undef _CASE_

    return maxSize;
}

void
idl_genTypeSize(
    idl_scope scope,
    c_type type)
{
    c_long minSize = 0;
    c_long maxSize = 0;
    const c_char *key_list = NULL;
    c_char *fullyScopedTypeName;
    c_char *key = NULL;
    c_baseObject o;
    c_iter keyIter;
    c_long keySize = 0;
    c_type t;

    printf("\n");
    if (type != NULL) {
        minSize = c_typeSize(type);
        maxSize = c_typeMaxSize(type);
        key_list = idl_keyResolve(idl_keyDefDefGet(), scope, c_metaObject(type)->name);
        if (key_list == NULL) { key_list = "No keys"; }
        fullyScopedTypeName = c_metaScopedName(c_metaObject(type));
        printf("Type size calculation for \"%s\" :\n\n",fullyScopedTypeName);
        os_free(fullyScopedTypeName);
        printf("    Minimum data size =           \t%d\n"
               "    Estimated maximum data size = \t%d\n", minSize, maxSize);
        keyIter = c_splitString(key_list,",");
        printf("    Key expression length =      \t%d\n"
               "    Number of keys =             \t%d\n",
               strlen(key_list), c_iterLength(keyIter));
        key = c_iterTakeFirst(keyIter);
        while (key) {
            o = c_baseObject(c_metaResolve(c_metaObject(type), key));
            switch (o->kind) {
            case M_MEMBER:
                t = c_memberType(o);
                keySize += c_typeSize(t);
                break;
            default:
                assert(0);
            }
            key = c_iterTakeFirst(keyIter);
        }
        printf("    Total key size =             \t%d\n\n", keySize);
    } else {
        printf("Type size calculation failed, no type specified!\n\n");
    }
}


static void
getMetaSize(
    void *object,
    c_iterActionArg arg)
{
    idl_scope scope;
    c_long *size = (c_long *)arg;
    c_baseObject o = (c_baseObject)object;
    c_long i, length;

    switch (o->kind) {
    case M_MODULE:
        *size += C_SIZEOF(c_module);
        *size += strlen(c_metaObject(o)->name) + HEADERSIZE;
    break;
    case M_CONSTANT:
        *size += C_SIZEOF(c_constant);
        getMetaSize(c_constant(o)->operand, size);
        *size += strlen(c_metaObject(o)->name) + HEADERSIZE;
    break;
    case M_ENUMERATION:
        *size += C_SIZEOF(c_enumeration);
        length = c_enumerationCount(o);
        *size += ARRAYHEADERSIZE + length * sizeof(c_constant);
        /* enumeration elements are already taken into account as sepparate constants. */
        *size += strlen(c_metaObject(o)->name) + HEADERSIZE;
    break;
    case M_STRUCTURE:
        *size += C_SIZEOF(c_structure);
        length = c_structureMemberCount(o);
        *size += ARRAYHEADERSIZE + length * sizeof(c_member); /* member list */
        length = c_arraySize(c_structure(o)->references);
        *size += ARRAYHEADERSIZE + length * sizeof(c_member); /* reference list */
        for (i=0; i<length; i++) {
            getMetaSize(c_structureMember(o,i),size);
        }
        *size += strlen(c_metaObject(o)->name) + HEADERSIZE;

        scope = idl_buildScope(c_metaObject(o));
        if (idl_keyResolve(idl_keyDefDefGet(), scope, c_metaObject(o)->name) != NULL) {
            idl_genTypeSize(scope, c_type(o));
            if (nr_of_unbounded_strings) {
                printf("    Type contains %d unbounded strings,"
                       " assumed a default worst case string size of %d\n\n",
                       nr_of_unbounded_strings, DEFAULT_WORST_CASE_STRING_SIZE);
                nr_of_unbounded_strings = 0;
            }
            if (nr_of_unbounded_sequences) {
                printf("    Type contains %d unbounded sequences,"
                       " assumed a default worst case sequences size of %d\n\n",
                       nr_of_unbounded_sequences, DEFAULT_WORST_CASE_SEQUENCE_SIZE);
                nr_of_unbounded_sequences = 0;
            }
        }
        idl_scopeFree(scope);
    break;
    case M_UNION:
        *size += C_SIZEOF(c_union);
        length = c_unionUnionCaseCount(o);
        *size += ARRAYHEADERSIZE + length * sizeof(c_unionCase);
        for (i=0; i<length; i++) {
            getMetaSize(c_unionUnionCase(o,i),size);
        }
        *size += strlen(c_metaObject(o)->name) + HEADERSIZE;
    break;
    case M_COLLECTION:
        *size += C_SIZEOF(c_collectionType);
        *size += strlen(c_metaObject(o)->name) + HEADERSIZE;
    break;
    case M_TYPEDEF:
        *size += C_SIZEOF(c_typeDef);
        *size += strlen(c_metaObject(o)->name) + HEADERSIZE;
    break;
    case M_LITERAL:
        *size += C_SIZEOF(c_literal);
        switch (c_literal(o)->value.kind) {
        case V_STRING:
            *size += strlen(c_literal(o)->value.is.String) + HEADERSIZE;
        break;
        case V_OBJECT:
            getMetaSize(c_literal(o)->value.is.Object, size);
        break;
        default:
        break;
        }
    break;
    case M_CONSTOPERAND:
        *size += C_SIZEOF(c_constOperand);
    break;
    case M_MEMBER:
        *size += C_SIZEOF(c_member);
        *size += strlen(c_specifierName(o)) + HEADERSIZE;
    break;
    case M_UNIONCASE:
        *size += C_SIZEOF(c_unionCase);
        *size += strlen(c_specifierName(o)) + HEADERSIZE;
        length = c_arraySize(c_unionCase(o)->labels);
        *size += ARRAYHEADERSIZE + length * sizeof(c_literal);
        for (i=0; i<length; i++) {
            getMetaSize(c_unionCase(o)->labels[i], size);
        }
    break;
    case M_EXPRESSION:
        *size += C_SIZEOF(c_expression);
        length = c_arraySize(c_expression(o)->operands);
        *size += ARRAYHEADERSIZE + length * sizeof(c_operand);
        for (i=0; i<length; i++) {
            getMetaSize(c_expression(o)->operands[i], size);
        }
    break;
    default:
        printf("Ran into unexpected meta data kind (%s) (%d)\n",
                metaKindImage(o->kind), o->kind);
    break;
    }
}

void
idl_genMetaSize(
    char *filename)
{
    c_iter source;
    c_long size = 0;
    /* Create source for filename */
    printf("\nFootprint metrics file %s: (sizes in bytes)\n", filename);
    source = idl_fileMapGetObjects(idl_fileMapDefGet(), filename);
    if (source) {
        c_iterWalk(source, getMetaSize, &size);
    }
    printf("Total metadata footprint generated by the idl file = %d\n\n", size);
}

