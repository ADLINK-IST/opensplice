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
#include "os.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <locale.h>
#include <assert.h>

#include "c_base.h"
#include "c_collection.h"
#include "c_iterator.h"
#include "c_module.h"

#include "idl_scope.h"
#include "idl_walk.h"
#include "idl_program.h"
#include "idl_fileMap.h"
#include "idl_genLanguageHelper.h"
#include "idl_unsupported.h"

/***********************************************************************
 *
 * Global function implementations
 *
 ***********************************************************************/

C_CLASS(idl_context);

C_STRUCT(idl_context) {
    c_bool traceWalk;
    idl_scope ownScope;
    idl_program program;
    c_char *fileName;
    c_char *baseName;
    c_iterIter* sortedListIter;
};

C_CLASS(idl_switchVal);

C_STRUCT(idl_switchVal) {
    c_type type;
    c_value value;
};

c_bool idl_contextTrace(idl_context context);

void idl_metaobject        (c_metaObject o, idl_context context);
void idl_module        (c_module o, idl_context context);
void idl_typedef       (c_typeDef o, idl_context context);
void idl_union         (c_union o, idl_context context);
void idl_primitive     (c_primitive o, idl_context context);
void idl_structure     (c_structure o, idl_context context);
void idl_enumeration   (c_enumeration o, idl_context context);
void idl_boundedString (c_collectionType o, idl_context context);
void idl_sequence      (c_collectionType o, idl_context context);
void idl_constant      (c_constant o, idl_context context);

void idl_unionCase          (c_union u, c_unionCase o, idl_switchVal defaultLabelVal, idl_context context);
void idl_structureMember    (c_structure s, c_member o, idl_context context);
void idl_unionSwitch        (c_type o, idl_context context);
void idl_unionLabel         (c_type t, c_literal o, idl_context context);
void idl_enumerationElement (c_enumeration e, c_constant o, idl_context context);

static idl_operand idl_makeConstExpression (c_expression operand, c_type type);
static idl_operand idl_makeConstLiteral (c_literal operand, c_type type);
static idl_constSpec idl_makeConstSpec (c_constant o);
static idl_operand idl_makeConstOperand (c_constant operand);
static idl_typeSpec idl_makeTypeSpec (c_type type);
static c_value idl_scaledLiteral ( c_primitive prim, c_value value);

static c_iter presetModules = NULL;
static c_char *presetFilename = NULL;
static c_long presetScope = 0;

/* Not likely that these values will be used as meta-object address */
#define SCOPE_OPEN ((void*)-1)
#define SCOPE_CLOSE ((void*)-2)

/* Walk objects unconditionally until SCOPE_CLOSE is encountered. */
static void c_walkIterScopeDo(c_iterIter* iterator, c_iterWalkAction action, void* userData) {
    void* o;

    while((o = c_iterNext(iterator)) && (o != SCOPE_CLOSE)) {
        if(o == SCOPE_OPEN) {
            /* Scope not handled by idl_program */
            c_walkIterScopeDo(iterator, action, userData);
        } else {
            if (action) {
                action(o, userData);
            }
        }
    }
}

/* Util function to walk over scope */
static void c_walkIterScope(c_iterIter* iterator, c_iterWalkAction action, void* userData) {
    void* o;
    c_iterIter i;

    /* Test if a scopeWalk is needed */
    i = *iterator;
    if((o = c_iterNext(&i)) != SCOPE_OPEN) {
        return;
    }else {
        /* Walk objects from SCOPE_OPEN */
        c_iterNext(iterator);
    }

    c_walkIterScopeDo(iterator, action, userData);
}


idl_switchVal
idl_switchValNew(
    c_type type)
{
    idl_switchVal val;

    val = os_malloc (C_SIZEOF(idl_switchVal));
    val->type = type;
    if (c_baseObject(c_typeActualType(type))->kind == M_PRIMITIVE) {
        switch (c_primitive(c_typeActualType(type))->kind) {
        case P_BOOLEAN:
            val->value = c_boolValue(FALSE);
            break;
        case P_SHORT:
            val->value = c_shortValue(-32768);
            break;
        case P_LONG:
            val->value = c_longValue(-2147483647L);
            break;
        case P_LONGLONG:
            val->value = c_longlongValue(-9223372036854775807LL);
            break;
        case P_USHORT:
            val->value = c_ushortValue(0);
            break;
        case P_ULONG:
            val->value = c_ulongValue(0UL);
            break;
        case P_ULONGLONG:
            val->value = c_ulonglongValue(0ULL);
            break;
        case P_CHAR:
            val->value = c_charValue(0);
            break;
        default:
            assert (FALSE);
        }
    } else {
        if (c_baseObject(c_typeActualType(type))->kind == M_ENUMERATION) {
            val->value = c_longValue(0UL);
        }
    }
    return val;
}

unsigned long long
idl_switchValMaxRange(
    c_type type)
{
    unsigned long long max = 0;

    if (c_baseObject(c_typeActualType(type))->kind == M_PRIMITIVE) {
        switch (c_primitive(c_typeActualType(type))->kind) {
        case P_BOOLEAN:
            max = 2ULL;
            break;
        case P_SHORT:
            max = 65536ULL;
            break;
        case P_LONG:
            max = 4294967296ULL;
            break;
        case P_LONGLONG:
            max = 18446744073709551615ULL;
            break;
        case P_USHORT:
            max = 65536ULL;
            break;
        case P_ULONG:
            max = 4294967296ULL;
            break;
        case P_ULONGLONG:
            max = 18446744073709551615ULL;
            break;
        case P_CHAR:
            max = 256ULL;
            break;
        default:
            assert (FALSE);
        }
    } else {
        if (c_baseObject(c_typeActualType(type))->kind == M_ENUMERATION) {
            max = c_arraySize (c_enumeration(c_typeActualType(type))->elements);
        }
    }
    return max;
}

idl_labelVal
idl_switchLabel(
    idl_switchVal val)
{
    idl_labelVal newVal;
    c_enumeration e;

    if (c_baseObject(c_typeActualType(val->type))->kind == M_ENUMERATION) {
        e = c_enumeration(c_typeActualType(val->type))->elements[val->value.is.Long];
        newVal = idl_labelVal(idl_labelEnumNew (idl_buildScope(c_metaObject(c_typeActualType(val->type))), c_metaObject(c_constant(e))->name));
    } else {
        newVal = idl_labelVal(idl_labelValueNew(val->value));
    }
    return newVal;
}

void
idl_switchValFree(
    idl_switchVal val)
{
    os_free (val);
}

void
idl_switchValInc(
    idl_switchVal val)
{
    switch (val->value.kind) {
    case V_BOOLEAN:
        val->value.is.Boolean++;
        break;
    case V_SHORT:
        val->value.is.Short++;
        break;
    case V_LONG:
        val->value.is.Long++;
        break;
    case V_LONGLONG:
        val->value.is.LongLong++;
        break;
    case V_USHORT:
        val->value.is.UShort++;
        break;
    case V_ULONG:
        val->value.is.ULong++;
        break;
    case V_ULONGLONG:
        val->value.is.ULongLong++;
        break;
    case V_CHAR:
        val->value.is.Char++;
        break;
    default:
        assert (FALSE);
    }
}

c_bool
idl_switchValEq(
    idl_switchVal swVal,
    c_value value)
{
    c_bool comp = FALSE;

    switch (swVal->value.kind) {
    case V_BOOLEAN:
        comp = (swVal->value.is.Boolean == value.is.Boolean);
        break;
    case V_SHORT:
        comp = (swVal->value.is.Short == value.is.Short);
        break;
    case V_LONG:
        comp = (swVal->value.is.Long == value.is.Long);
        break;
    case V_LONGLONG:
        comp = (swVal->value.is.LongLong == value.is.LongLong);
        break;
    case V_USHORT:
        comp = (swVal->value.is.UShort == value.is.UShort);
        break;
    case V_ULONG:
        comp = (swVal->value.is.ULong == value.is.ULong);
        break;
    case V_ULONGLONG:
        comp = (swVal->value.is.ULongLong == value.is.ULongLong);
        break;
    case V_CHAR:
        comp = (swVal->value.is.Char == value.is.Char);
        break;
    default:
        assert (FALSE);
    }
    return comp;
}

void
idl_walkPresetFile(
    const char *fileName)
{
    presetFilename = os_strdup(fileName);
}

void
idl_walkPresetModule(
    const char *moduleName)
{
    presetModules = c_iterInsert(presetModules, os_strdup(moduleName));
}

static c_equality
idl_moduleCompare(
    void *_modName,
    void *_compareName)
{
    char *modName;
    char *compareName;
    c_equality result = C_NE;

    modName = _modName;
    compareName = _compareName;

    if (strcmp(modName, compareName) == 0) {
        result = C_EQ;
    }

    return result;
}

c_bool
idl_contextTrace(
    idl_context context)
{
    return context->traceWalk;
}

c_char *
idl_basename(
    const char *filename)
{
    const char *fname_start;
    const char *fname_end;
    c_long len;
    c_long i;
    static c_char basename[200];

    fname_start = os_rindex(filename, OS_FILESEPCHAR);
    if (fname_start == NULL) {
        fname_start = filename;
    } else {
        fname_start++;
    }
    fname_end = os_rindex (filename, '.');
    if (fname_end == NULL) {
        /* implicit defined meta objects will have no filename attached */
        /* e.g. "C_ARRAY<c_long>,10" or "C_SEQUENCE<c_char>"            */
        basename[0] = '\0';
        return basename;
    }
    len = (fname_end - fname_start);
    for (i = 0; i < len; i++) {
        basename[i] = *fname_start;
        fname_start++;
    }
    basename[len] = '\0';

    return basename;
}

idl_scope
idl_buildScope(
    c_metaObject o)
{
    idl_scope rscope;
    idl_scope scope;
    c_char *filename;
    c_char *basename;

    if (presetFilename) {
        filename = presetFilename;
    } else {
        filename = os_strdup(idl_fileMapResolve (idl_fileMapDefGet(), c_baseObject(o)));
    }
    basename = idl_basename(filename);
    scope = idl_scopeNew(basename);
    rscope = idl_scopeNew(basename);

    while (o->definedIn && o->definedIn->name) {
        if (c_baseObject(o->definedIn)->kind == M_STRUCTURE) {
            idl_scopePush(rscope, idl_scopeElementNew(o->definedIn->name, idl_tStruct));
        } else {
            if (c_baseObject(o->definedIn)->kind == M_UNION) {
                idl_scopePush(rscope, idl_scopeElementNew(o->definedIn->name, idl_tUnion));
            } else {
                idl_scopePush(rscope, idl_scopeElementNew(o->definedIn->name, idl_tModule));
            }
        }
        o = o->definedIn;
    }
    while (idl_scopeStackSize(rscope)) {
        idl_scopePush(scope, idl_scopeCur(rscope));
        idl_scopePop(rscope);
    }
    idl_scopeFree(rscope);
    return scope;
}

c_bool
idl_objectInScope(
    c_metaObject scope,
    c_metaObject o)
{
    c_metaObject q;

    if (scope == NULL) {
        /* Top level passed and object not found */
        return FALSE;
    }
    /* find object on current scope level */
    q = c_metaResolve (scope, o->name);
    if (q) {
        /* object found */
        if (q != o) {
            /* object other than referenced object */
            /* thus obect with same name but in    */
            /* different scope                     */
            return FALSE;
        }
        /* object is the correct one, thus same scope */
        return TRUE;
    }
    /* no object with this name found, try */
    /* next higher level                   */
    return idl_objectInScope(scope->definedIn, o);
}

char *
idl_scopedName(
    c_metaObject o,
    c_metaObject t)
{
    idl_scope scope;
    static char *scopedName = NULL;

    if (idl_objectInScope(o->definedIn, t) == FALSE) {
        if (scopedName) {
            os_free(scopedName);
        }
        scope = idl_buildScope(t);
        scopedName = idl_scopeStack(scope, "::", t->name);
        return scopedName;
    }
    return t->name;
}

c_char *
idl_labelImage(
    c_type t,
    c_value value)
{
    static c_char val[40];

    val[0] = '\0';
    if (c_baseObject(t)->kind != M_ENUMERATION) {
        switch (value.kind) {
        case V_UNDEFINED:
        case V_OCTET:
        case V_FLOAT:
        case V_DOUBLE:
        case V_STRING:
        case V_WCHAR:
        case V_WSTRING:
        case V_FIXED:
        case V_VOIDP:
        case V_OBJECT:
        case V_COUNT:
            /* Invalid types for union switch*/
            break;
        case V_BOOLEAN:
            if (value.is.Boolean) {
                snprintf(val, sizeof(val), "TRUE");
            } else {
                snprintf(val, sizeof(val), "FALSE");
            }
            break;
        case V_SHORT:
            snprintf(val, sizeof(val), "%hd", value.is.Short);
            break;
        case V_LONG:
            snprintf(val, sizeof(val), "%dL", value.is.Long);
            break;
        case V_LONGLONG:
            switch (c_primitive(t)->kind) {
            case P_SHORT:
                snprintf(val, 40, "%hd", (c_short)value.is.LongLong);
            break;
            case P_USHORT:
                snprintf(val, 40, "%huU", (c_ushort)value.is.LongLong);
            break;
            case P_LONG:
                snprintf(val, 40, "%dL", (c_long)value.is.LongLong);
            break;
            case P_ULONG:
                snprintf(val, 40, "%uUL", (c_ulong)value.is.LongLong);
            break;
            case P_LONGLONG:
                snprintf(val, 40, "%lldLL", (c_longlong)value.is.LongLong);
            break;
            case P_ULONGLONG:
                snprintf(val, 40, "%lluULL", (c_ulonglong)value.is.LongLong);
            break;
            case P_CHAR:
                snprintf(val, 40, "%hu", (unsigned char)value.is.LongLong);
            break;
            case P_ADDRESS:
                snprintf(val, 40, PA_ADDRFMT, (PA_ADDRCAST)value.is.LongLong);
            break;
            case P_OCTET:
            case P_UNDEFINED:
            case P_BOOLEAN:
            case P_WCHAR:
            case P_FLOAT:
            case P_DOUBLE:
            case P_VOIDP:
            case P_MUTEX:
            case P_LOCK:
            case P_COND:
            case P_COUNT:
                /* Do nothing */
            break;
            }
            break;
        case V_ADDRESS:
            snprintf(val, sizeof(val), PA_ADDRFMT, value.is.Address);
            break;
        case V_USHORT:
            snprintf(val, sizeof(val), "%huU", value.is.UShort);
            break;
        case V_ULONG:
            snprintf(val, sizeof(val), "%uUL", value.is.ULong);
            break;
        case V_ULONGLONG:
            snprintf(val, sizeof(val), "%lluULL", value.is.ULongLong);
            break;
        case V_CHAR:
            snprintf(val, sizeof(val), "%hu", (unsigned char)value.is.Char);
            break;
        }
    } else {
        snprintf(val, sizeof(val), "%s",
            c_metaObject(c_constant(c_enumeration(t)->elements[value.is.Long]))->name);
    }
    return (val);
}

static idl_typeBasic
idl_makeTypeBasic(
    c_type type)
{
    idl_typeBasic typeBasic = NULL;

    if (c_baseObject(type)->kind == M_PRIMITIVE) {
        switch (c_primitive(type)->kind) {
        case P_BOOLEAN:
            typeBasic = idl_typeBasicNew(idl_boolean);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_OCTET:
            typeBasic = idl_typeBasicNew(idl_octet);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_FLOAT:
            typeBasic = idl_typeBasicNew(idl_float);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_DOUBLE:
            typeBasic = idl_typeBasicNew(idl_double);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_SHORT:
            typeBasic = idl_typeBasicNew(idl_short);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_LONG:
            typeBasic = idl_typeBasicNew(idl_long);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_LONGLONG:
            typeBasic = idl_typeBasicNew(idl_longlong);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_USHORT:
            typeBasic = idl_typeBasicNew(idl_ushort);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_ULONG:
            typeBasic = idl_typeBasicNew(idl_ulong);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_ULONGLONG:
            typeBasic = idl_typeBasicNew(idl_ulonglong);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        case P_CHAR:
            typeBasic = idl_typeBasicNew(idl_char);
            idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
            break;
        default:
            printf("Unexpected primitive kind %d for %s\n", c_primitive(type)->kind, c_metaObject(type)->name);
        }
        if (typeBasic != NULL) {
            idl_typeSpecSetDef(idl_typeSpec(typeBasic),type);
        }
    } else {
        if ((c_baseObject(type)->kind == M_COLLECTION) &&
            (c_collectionType(type)->kind == C_STRING)) {
                typeBasic = idl_typeBasicNew(idl_string);
                idl_typeBasicSetMaxlen(typeBasic, c_collectionType(type)->maxSize);
                idl_typeSpecSetName(idl_typeSpec(typeBasic), c_metaObject(type)->name);
                idl_typeSpecSetRef(idl_typeSpec(typeBasic), TRUE);
                idl_typeSpecSetDef(idl_typeSpec(typeBasic), type);
        }
    }
    return typeBasic;
}

void
idl_setUserType(
    idl_typeUser typeUser,
    c_type type)
{
    if (presetFilename) {
        idl_typeUserSetFileName(typeUser, os_strdup (presetFilename));
    } else {
        idl_typeUserSetFileName(typeUser, idl_fileMapResolve (idl_fileMapDefGet(), c_baseObject(type)));
    }
    idl_typeUserSetScope(typeUser, idl_buildScope(c_metaObject(type)));
    idl_typeSpecSetRef(idl_typeSpec(typeUser), c_typeHasRef (type));
    idl_typeSpecSetDef(idl_typeSpec(typeUser), type);
}

idl_typeSpec
idl_makeTypeCollection(
    c_collectionType type)
{
    idl_typeSpec typeSpec;
    idl_typeSpec subType;
    c_char typeName[1024];

    switch (c_collectionType(type)->kind) {
    case C_STRING:
        typeSpec = idl_typeSpec(idl_makeTypeBasic(c_type(type)));
        if (idl_typeBasicMaxlen(idl_typeBasic(typeSpec)) > 0) {
            idl_setUserType(idl_typeUser(typeSpec), c_type(type));
        }
        break;
    case C_ARRAY:
        subType = idl_makeTypeSpec(type->subType);
        typeSpec = idl_typeSpec(idl_typeArrayNew (subType, type->maxSize));
        snprintf(typeName, sizeof(typeName), "C_ARRAY<%s,%d>",
            idl_typeSpecName(subType), type->maxSize);
        idl_typeSpecSetName(typeSpec, typeName);
        idl_setUserType(idl_typeUser(typeSpec), c_type(type));
        break;
    case C_SEQUENCE:
        subType = idl_makeTypeSpec(type->subType);
        typeSpec = idl_typeSpec(idl_typeSeqNew(subType, type->maxSize));
        idl_setUserType(idl_typeUser(typeSpec), c_type(type));
        if (type->maxSize == 0) {
            /* unbounded sequence */
            snprintf(typeName, sizeof(typeName), "C_SEQUENCE<%s>",
                idl_typeSpecName(subType));
        } else {
            /* bounded sequence */
            snprintf(typeName,
                sizeof(typeName),
                "C_SEQUENCE<%s,%d>",
                idl_typeSpecName(subType),
                type->maxSize);
        }
        idl_typeSpecSetName(typeSpec, typeName);
        break;
    default:
        typeSpec = NULL;
        printf("Unexpected collection kind %d\n", c_collectionType(type)->kind);
    }
    return typeSpec;
}

idl_typeUnion
idl_makeTypeUnion(
    c_union type)
{
    idl_typeUnion typeUnion = NULL;
    idl_typeSpec switchKind;

    switchKind = idl_makeTypeSpec(c_type(type->switchType));
    typeUnion = idl_typeUnionNew(switchKind, c_arraySize(type->cases));
    idl_setUserType(idl_typeUser(typeUnion), c_type(type));
    idl_typeSpecSetType(idl_typeSpec(typeUnion), idl_tunion);
    idl_typeSpecSetName(idl_typeSpec(typeUnion), c_metaObject(type)->name);

    return typeUnion;
}

idl_typeStruct
idl_makeTypeStruct(
    c_structure type)
{
    idl_typeStruct typeStruct = NULL;

    typeStruct = idl_typeStructNew(c_arraySize(type->members));
    idl_setUserType(idl_typeUser(typeStruct), c_type(type));
    idl_typeSpecSetType(idl_typeSpec(typeStruct), idl_tstruct);
    idl_typeSpecSetName(idl_typeSpec(typeStruct), c_metaObject(type)->name);

    return typeStruct;
}

idl_typeEnum
idl_makeTypeEnum(
    c_enumeration type)
{
    idl_typeEnum typeEnum = NULL;

    typeEnum = idl_typeEnumNew(c_arraySize(type->elements));
    idl_setUserType(idl_typeUser(typeEnum), c_type(type));
    idl_typeSpecSetType(idl_typeSpec(typeEnum), idl_tenum);
    idl_typeSpecSetName(idl_typeSpec(typeEnum), c_metaObject(type)->name);

    return typeEnum;
}

idl_typeDef
idl_makeTypeTypedef(
    c_typeDef type)
{
    idl_typeDef typeDef = NULL;
    idl_typeSpec referedDef;
    idl_typeSpec actualDef;
    c_type referedType;
    c_type actualType;

    referedType = type->alias;
    actualType = c_typeActualType(c_type(type));
    if (referedType == actualType) {
        referedDef = idl_makeTypeSpec(referedType);
        actualDef = referedDef;
    } else {
        referedDef = idl_makeTypeSpec(referedType);
        actualDef = idl_makeTypeSpec(actualType);
    }
    typeDef = idl_typeDefNew(referedDef, actualDef);
    idl_setUserType(idl_typeUser(typeDef), c_type(type));
    idl_typeSpecSetType(idl_typeSpec(typeDef), idl_ttypedef);
    idl_typeSpecSetName(idl_typeSpec(typeDef), c_metaObject(type)->name);

    return typeDef;
}

static idl_typeSpec
idl_makeTypeSpec(
    c_type type)
{
    idl_typeSpec typeSpec = NULL;

    if (c_baseObject(type)->kind == M_PRIMITIVE) {
        typeSpec = idl_typeSpec(idl_makeTypeBasic(type));
    } else if (c_baseObject(type)->kind == M_COLLECTION) {
        typeSpec = idl_makeTypeCollection(c_collectionType(type));
    } else if (c_baseObject(type)->kind == M_UNION) {
        typeSpec = idl_typeSpec(idl_makeTypeUnion(c_union(type)));
    } else if (c_baseObject(type)->kind == M_STRUCTURE) {
        typeSpec = idl_typeSpec(idl_makeTypeStruct(c_structure(type)));
    } else if (c_baseObject(type)->kind == M_ENUMERATION) {
        typeSpec = idl_typeSpec(idl_makeTypeEnum(c_enumeration(type)));
    } else if (c_baseObject(type)->kind == M_TYPEDEF) {
        typeSpec = idl_typeSpec(idl_makeTypeTypedef(c_typeDef(type)));
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC messages */
    }
    return typeSpec;
}

void
idl_freeTypeSpec(
    idl_typeSpec typeSpec)
{
    switch (idl_typeSpecType(typeSpec)) {
    case idl_tbasic:
        idl_typeBasicFree(idl_typeBasic(typeSpec));
        break;
    case idl_ttypedef:
        idl_typeDefFree(idl_typeDef(typeSpec));
        break;
    case idl_tenum:
        idl_typeEnumFree(idl_typeEnum(typeSpec));
        break;
    case idl_tstruct:
        idl_typeStructFree(idl_typeStruct(typeSpec));
        break;
    case idl_tunion:
        idl_typeUnionFree(idl_typeUnion(typeSpec));
        break;
    case idl_tarray:
        idl_typeArrayFree(idl_typeArray(typeSpec));
        break;
    case idl_tseq:
        idl_typeSeqFree(idl_typeSeq(typeSpec));
        break;
    }
}

static c_value
idl_scaledLiteral(
    c_primitive prim,
    c_value value)
{
    c_value new_value;

    if (value.kind == V_LONGLONG) {
        switch (prim->kind) {
        case P_BOOLEAN:
            new_value.kind = V_BOOLEAN;
            new_value.is.Boolean = (c_bool)value.is.LongLong;
            break;
        case P_CHAR:
            new_value.kind = V_CHAR;
            new_value.is.Char = (c_char)value.is.LongLong;
            break;
        case P_SHORT:
            new_value.kind = V_SHORT;
            new_value.is.Short = (c_short)value.is.LongLong;
            break;
        case P_USHORT:
            new_value.kind = V_USHORT;
            new_value.is.UShort = (c_ushort)value.is.LongLong;
            break;
        case P_LONG:
            new_value.kind = V_LONG;
            new_value.is.Long = (c_long)value.is.LongLong;
            break;
        case P_ULONG:
            new_value.kind = V_ULONG;
            new_value.is.ULong = (c_ulong)value.is.LongLong;
            break;
        case P_LONGLONG:
            new_value.kind = V_LONGLONG;
            new_value.is.LongLong = value.is.LongLong;
            break;
        case P_ULONGLONG:
            new_value.kind = V_ULONGLONG;
            new_value.is.ULongLong = value.is.LongLong;
            break;
        default:
            new_value.kind = V_UNDEFINED;
            new_value.is.LongLong = value.is.LongLong;
            break;
        }
    } else {
        new_value = value;
    }
    return new_value;
}

long
idl_enumElementValue(
    c_literal o)
{
    assert (o->value.kind == V_LONG);

    return o->value.is.Long;
}

void
idl_iterTrace(
    c_metaObject o,
    void* udata)
{
    const char *scopeName;
    c_char *name;
    const char *metaKind = NULL;

    assert(o);

    name = o->name;
    if (o->definedIn && o->definedIn->name) {
        scopeName = o->definedIn->name;
    } else {
        scopeName = "";
    }
    switch (c_baseObject(o)->kind) {
    case M_UNDEFINED:
        metaKind = "M_UNDEFINED";
    break;
    case M_ATTRIBUTE:
        metaKind = "M_ATTRIBUTE";
    break;
    case M_ANNOTATION:
        metaKind = "M_ANNOTATION";
        break;
    case M_CLASS:
        metaKind = "M_CLASS";
    break;
    case M_COLLECTION:
        metaKind = "M_COLLECTION";
    break;
    case M_CONSTANT:
        metaKind = "M_CONSTANT";
    break;
    case M_CONSTOPERAND:
        metaKind = "M_CONSTOPERAND";
    break;
    case M_ENUMERATION:
        metaKind = "M_ENUMERATION";
    break;
    case M_EXCEPTION:
        metaKind = "M_EXCEPTION";
    break;
    case M_EXPRESSION:
        metaKind = "M_EXPRESSION";
    break;
    case M_INTERFACE:
        metaKind = "M_INTERFACE";
    break;
    case M_LITERAL:
        metaKind = "M_LITERAL";
    break;
    case M_MEMBER:
        metaKind = "M_MEMBER";
    break;
    case M_MODULE:
        metaKind = "M_MODULE";
    break;
    case M_OPERATION:
        metaKind = "M_OPERATION";
    break;
    case M_PARAMETER:
        metaKind = "M_PARAMETER";
    break;
    case M_PRIMITIVE:
        metaKind = "M_PRIMITIVE";
    break;
    case M_RELATION:
        metaKind = "M_RELATION";
    break;
    case M_BASE:
        metaKind = "M_BASE";
    break;
    case M_STRUCTURE:
        metaKind = "M_STRUCTURE";
    break;
    case M_TYPEDEF:
        metaKind = "M_TYPEDEF";
    break;
    case M_UNION:
        metaKind = "M_UNION";
    break;
    case M_UNIONCASE:
        metaKind = "M_UNIONCASE";
    break;
    case M_COUNT:
        metaKind = "M_COUNT";
    break;
    }

    if (strlen (scopeName)) {
        printf("c_base content: %s in %s with kind: %s\n", name, scopeName, metaKind);
    } else {
        printf("c_base content: %s with kind: %s\n",name, metaKind);
    }
}

void
idl_metaObjectTrace(
    c_metaObject o,
    const char *proc,
    idl_context context)
{
    const char *scopeName;
    c_char *name;
    const char *metaKind = NULL;

    assert(o);
    assert(proc);
    assert(context);

    name = o->name;
    if (idl_contextTrace(context) == FALSE) {
        return;
    }
    if (o->definedIn && o->definedIn->name) {
        scopeName = o->definedIn->name;
    } else {
        scopeName = "";
    }
    switch (c_baseObject(o)->kind) {
    case M_UNDEFINED:
        metaKind = "M_UNDEFINED";
        break;
    case M_ATTRIBUTE:
        metaKind = "M_ATTRIBUTE";
        break;
    case M_CLASS:
        metaKind = "M_CLASS";
        break;
    case M_COLLECTION:
        metaKind = "M_COLLECTION";
        break;
    case M_CONSTANT:
        metaKind = "M_CONSTANT";
        break;
    case M_CONSTOPERAND:
        metaKind = "M_CONSTOPERAND";
        break;
    case M_ENUMERATION:
        metaKind = "M_ENUMERATION";
        break;
    case M_EXCEPTION:
        metaKind = "M_EXCEPTION";
        break;
    case M_EXPRESSION:
        metaKind = "M_EXPRESSION";
        break;
    case M_INTERFACE:
        metaKind = "M_INTERFACE";
        break;
    case M_ANNOTATION:
        metaKind = "M_ANNOTATION";
        break;
    case M_LITERAL:
        metaKind = "M_LITERAL";
        break;
    case M_MEMBER:
        metaKind = "M_MEMBER";
        break;
    case M_MODULE:
        metaKind = "M_MODULE";
        break;
    case M_OPERATION:
        metaKind = "M_OPERATION";
        break;
    case M_PARAMETER:
        metaKind = "M_PARAMETER";
        break;
    case M_PRIMITIVE:
        metaKind = "M_PRIMITIVE";
        break;
    case M_RELATION:
        metaKind = "M_RELATION";
        break;
    case M_BASE:
        metaKind = "M_BASE";
        break;
    case M_STRUCTURE:
        metaKind = "M_STRUCTURE";
        break;
    case M_TYPEDEF:
        metaKind = "M_TYPEDEF";
        break;
    case M_UNION:
        metaKind = "M_UNION";
        break;
    case M_UNIONCASE:
        metaKind = "M_UNIONCASE";
        break;
    case M_COUNT:
        metaKind = "M_COUNT";
        break;
    }
    if (strlen (scopeName)) {
        printf("%s: %s (%s in %s)\n", proc, metaKind, name, scopeName);
    } else {
        printf("%s: %s (%s)\n", proc, metaKind, name);
    }
}

static idl_exprKind
idl_expressionKind (
    c_expression expr)
{
    idl_exprKind kind = idl_or; /* Randomly selected item to fix compiler warning -Wuninitialized */

    switch (expr->kind) {
    case E_OR:
        kind = idl_or;
        break;
    case E_XOR:
        kind = idl_xor;
        break;
    case E_AND:
        kind = idl_and;
        break;
    case E_SHIFTRIGHT:
        kind = idl_shiftright;
        break;
    case E_SHIFTLEFT:
        kind = idl_shiftleft;
        break;
    case E_PLUS:
        kind = idl_plus;
        break;
    case E_MINUS:
        kind = idl_minus;
        break;
    case E_MUL:
        kind = idl_mul;
        break;
    case E_DIV:
        kind = idl_div;
        break;
    case E_MOD:
        kind = idl_mod;
        break;
    case E_NOT:
        kind = idl_not;
        break;
    default:
        printf("idl_expressionKind: Unknown expression kind %d\n", expr->kind);
    }
    return kind;
}

static idl_operand
idl_makeConstExpression(
    c_expression expr,
    c_type type)
{
    idl_constExpression constExpr;
    int i;
    c_operand exprOper;
    idl_operand operand = NULL;

    constExpr = idl_constExpressionNew(idl_expressionKind(expr));
    for (i = 0; i < c_arraySize (expr->operands); i++) {
        exprOper = c_operand(expr->operands[i]);
        switch (c_baseObject(exprOper)->kind) {
        case M_CONSTANT:
            operand = idl_makeConstOperand(c_constant(exprOper));
            break;
        case M_CONSTOPERAND:
            operand = idl_makeConstOperand(c_constOperand(exprOper)->constant);
            break;
        case M_LITERAL:
            operand = idl_makeConstLiteral(c_literal(exprOper), type);
            break;
        case M_EXPRESSION:
            operand = idl_makeConstExpression(c_expression(exprOper), type);
            break;
        default:
            printf("idl_makeConstExpression: Unexpected %d operand\n", c_baseObject(exprOper)->kind);
        }
        idl_constExpressionAdd(constExpr, operand);
    }

    return (idl_operand)constExpr;
}

static idl_operand
idl_makeConstLiteral(
    c_literal operand,
    c_type type)
{
    idl_constLiteral constLit;
    char *val = NULL;
    char *val2;
    int i;

    if (c_baseObject(type)->kind != M_ENUMERATION) {
        switch (operand->value.kind) {
        case V_UNDEFINED:
        case V_WCHAR:
        case V_WSTRING:
        case V_FIXED:
        case V_VOIDP:
        case V_OBJECT:
        case V_COUNT:
            /* Invalid types for literal constants*/
            break;
        case V_OCTET:
            val = os_malloc (40);
            if (idl_getLanguage() == IDL_LANG_JAVA || idl_getLanguage() == IDL_LANG_CS) {
                snprintf(val, 40, "%d", operand->value.is.Octet);
            } else {
                snprintf(val, 40, "%x", operand->value.is.Octet);
            }
            break;
        case V_FLOAT:
        case V_DOUBLE:
            val2 = os_malloc(45);
            val = os_malloc(45);
            snprintf(val2, 45, "%40.17g", operand->value.is.Double);
            i = 0;
            while (val2[i] == ' ') {
                i++;
            }
           os_strncpy(val, &val2[i], 40);
            os_free(val2);
            if ((strchr(val, '.') == NULL) && (strchr(val, 'E') == NULL)) {
                strcat(val, ".0");
            }
            break;
        case V_STRING:
            val = os_malloc(strlen(operand->value.is.String)+3);
            snprintf(val, strlen(operand->value.is.String)+3, "\"%s\"", operand->value.is.String);
            break;
        case V_BOOLEAN:
            val = os_malloc(40);
            if (idl_getLanguage() == IDL_LANG_JAVA || idl_getLanguage() == IDL_LANG_CS) {
                if (operand->value.is.Boolean) {
                    snprintf(val, 40, "true");
                } else {
                    snprintf (val, 40, "false");
                }
            } else {
                if (operand->value.is.Boolean) {
                    snprintf(val, 40, "TRUE");
                } else {
                    snprintf (val, 40, "FALSE");
                }
            }
            break;
        case V_LONGLONG:
            val = os_malloc(40);
            if (idl_getLanguage() == IDL_LANG_JAVA || idl_getLanguage() == IDL_LANG_CS) {
                switch (c_primitive(type)->kind) {
                case P_SHORT:
                    snprintf(val, 40, "%hd", (c_short)operand->value.is.LongLong);
                    break;
                case P_USHORT:
                    snprintf(val, 40, "%hd", (c_short)operand->value.is.LongLong);
                    break;
                case P_LONG:
                    snprintf(val, 40, "%d", (c_long)operand->value.is.LongLong);
                    break;
                case P_ULONG:
                    snprintf(val, 40, "%d", (c_long)operand->value.is.LongLong);
                    break;
                case P_LONGLONG:
                    snprintf(val, 40, "%lld", (c_longlong)operand->value.is.LongLong);
                    break;
                case P_ULONGLONG:
                    snprintf(val, 40, "%lld", (c_longlong)operand->value.is.LongLong);
                    break;
                case P_CHAR:
                    snprintf(val, 40, "%hd", (unsigned char)operand->value.is.LongLong);
                    break;
                case P_OCTET:
                    snprintf(val, 40, "%hd", (unsigned char)operand->value.is.LongLong);
                    break;
                case P_ADDRESS:
                    snprintf(val, 40, PA_ADDRFMT, (PA_ADDRCAST)operand->value.is.LongLong);
                    break;
                case P_UNDEFINED:
                case P_BOOLEAN:
                case P_WCHAR:
                case P_FLOAT:
                case P_DOUBLE:
                case P_VOIDP:
                case P_MUTEX:
                case P_LOCK:
                case P_COND:
                case P_COUNT:
                    /* Do nothing */
                break;
                }
            } else {
                switch (c_primitive(type)->kind) {
                case P_SHORT:
                    snprintf(val, 40, "%hu", (c_short)operand->value.is.LongLong);
                    break;
                case P_USHORT:
                    snprintf(val, 40, "%huU", (c_ushort)operand->value.is.LongLong);
                    break;
                case P_LONG:
                    snprintf(val, 40, "%uL", (c_long)operand->value.is.LongLong);
                break;
                case P_ULONG:
                    snprintf(val, 40, "%uUL", (c_ulong)operand->value.is.LongLong);
                break;
                case P_LONGLONG:
                    snprintf(val, 40, "%lluLL", (c_longlong)operand->value.is.LongLong);
                    break;
                case P_ULONGLONG:
                    snprintf(val, 40, "%lluULL", (c_ulonglong)operand->value.is.LongLong);
                break;
                case P_CHAR:
                    snprintf(val, 40, "%hu", (unsigned char)operand->value.is.LongLong);
                    break;
                case P_OCTET:
                    snprintf(val, 40, "%hu", (unsigned char)operand->value.is.LongLong);
                break;
                case P_ADDRESS:
                    snprintf(val, 40, PA_ADDRFMT, (PA_ADDRCAST)operand->value.is.LongLong);
                    break;
                case P_UNDEFINED:
                case P_BOOLEAN:
                case P_WCHAR:
                case P_FLOAT:
                case P_DOUBLE:
                case P_VOIDP:
                case P_MUTEX:
                case P_LOCK:
                case P_COND:
                case P_COUNT:
                    /* Do nothing */
                    break;
                }
            }
            break;
        case V_SHORT:
            val = os_malloc(40);
            snprintf(val, sizeof(val), "%hd", operand->value.is.Short);
            break;
        case V_LONG:
            val = os_malloc(40);
            if (idl_getLanguage() == IDL_LANG_JAVA || idl_getLanguage() == IDL_LANG_CS) {
                snprintf(val, sizeof(val), "%d", (c_long)operand->value.is.Long);
            } else {
                snprintf(val, sizeof(val), "%dL", operand->value.is.Long);
            }
            break;
        case V_USHORT:
            val = os_malloc(40);
            if (idl_getLanguage() == IDL_LANG_JAVA || idl_getLanguage() == IDL_LANG_CS) {
                snprintf(val, sizeof(val), "%hd", (c_short)operand->value.is.UShort);
            } else {
                snprintf(val, sizeof(val), "%huU", operand->value.is.UShort);
            }
            break;
        case V_ULONG:
            val = os_malloc(40);
            if (idl_getLanguage() == IDL_LANG_JAVA || idl_getLanguage() == IDL_LANG_CS) {
                snprintf(val, sizeof(val), "%d", (c_long)operand->value.is.ULong);
            } else {
                snprintf(val, sizeof(val), "%uUL", operand->value.is.ULong);
            }
            break;
        case V_ULONGLONG:
            val = os_malloc(40);
            if (idl_getLanguage() == IDL_LANG_JAVA || idl_getLanguage() == IDL_LANG_CS) {
                snprintf(val, sizeof(val), "%lld", (c_longlong)operand->value.is.ULongLong);
            } else {
                snprintf(val, sizeof(val), "%lluULL", operand->value.is.ULongLong);
            }
            break;
        case V_ADDRESS:
            val = os_malloc(40);
            snprintf(val, sizeof(val), PA_ADDRFMT, (PA_ADDRCAST)operand->value.is.Address);
            break;
        case V_CHAR:
            val = os_malloc(40);
            snprintf(val, 40, "%hu", (unsigned char)operand->value.is.Char);
            break;
        }
    } else {
        char *name;

        name = os_strdup(c_metaObject(c_constant(c_enumeration(type)->elements[operand->value.is.Long]))->name);
        val = os_malloc(strlen(name) + 1);
        snprintf(val, strlen(name) + 1, "%s", name);
    }
    constLit = idl_constLiteralNew(val);
    return (idl_operand)constLit;
}

static idl_operand
idl_makeConstOperand(
    c_constant operand)
{
    idl_constOperand constOper;
    idl_constSpec constSpec;

    constSpec = idl_makeConstSpec(operand);
    constOper = idl_constOperandNew(constSpec);
    return (idl_operand)constOper;
}

static idl_constSpec
idl_makeConstSpec(
    c_constant o)
{
    idl_constSpec constSpec;
    idl_typeSpec typeSpec;
    idl_operand operand = NULL;

    typeSpec = idl_makeTypeSpec(o->type);
    constSpec = idl_constSpecNew(c_metaObject(o)->name, typeSpec, idl_buildScope(c_metaObject(o)));
    switch (c_baseObject(o->operand)->kind) {
    case M_CONSTANT:
        operand = idl_makeConstOperand(c_constant(o->operand));
        break;
    case M_CONSTOPERAND:
        operand = idl_makeConstOperand(c_constOperand(o->operand)->constant);
        break;
    case M_LITERAL:
        if (c_baseObject(o->operand)->kind == M_ENUMERATION) {
            operand = idl_makeConstOperand(c_constant(o->operand));
        } else {
            operand = idl_makeConstLiteral(c_literal(o->operand), c_typeActualType(o->type));
        }
        break;
    case M_EXPRESSION:
        operand = idl_makeConstExpression(c_expression(o->operand), c_typeActualType(o->type));
        break;
    default:
        printf("idl_makeConstSpec: Unexpected %d operand\n", c_baseObject(o->operand)->kind);
    }
    idl_constSpecOperandSet(constSpec, operand);

    return constSpec;
}

int
idl_unsupportedType(
    c_type type)
{
    int retval = 0;

    assert(type);

    /* if name starts with IDL_UNSUP_PREFIX, it is by definition unsupported! */
    if ((c_metaObject(type)->name == NULL) ||
        (strncmp(c_metaObject(type)->name, IDL_UNSUP_PREFIX, strlen(IDL_UNSUP_PREFIX)) != 0)) {
        switch (c_baseObject(type)->kind) {
        case M_COLLECTION:
                if (c_collectionType(type)->kind == C_SEQUENCE) {
                    retval = idl_unsupportedType(c_collectionType(type)->subType);
            } else if (c_collectionType(type)->kind == C_ARRAY) {
                retval = idl_unsupportedType(c_collectionType(type)->subType);
            } else if (!(c_collectionType(type)->kind == C_STRING) &&
                       !(c_collectionType(type)->kind == C_WSTRING)) {
                retval = 1;
            }
            break;
        case M_UNDEFINED:
        case M_ATTRIBUTE:
        case M_CLASS:
        case M_CONSTANT:
        case M_CONSTOPERAND:
        case M_EXCEPTION:
        case M_EXPRESSION:
        case M_INTERFACE:
        case M_LITERAL:
        case M_MEMBER:
        case M_MODULE:
        case M_OPERATION:
        case M_PARAMETER:
        case M_RELATION:
        case M_BASE:
        case M_UNIONCASE:
        case M_COUNT:
            retval = 1;
        break;
        case M_TYPEDEF:
            retval = idl_unsupportedType (c_typeDef(type)->alias);
        break;
        default:
        break;
        }
    } else {
        retval = 1;
    }
    return retval;
}

void
idl_metaobject(
    c_metaObject o,
    idl_context context)
{
    assert(o);
    assert(context);

    if (presetModules && (c_baseObject(o)->kind == M_MODULE)) {
        if (c_iterResolve (presetModules, idl_moduleCompare, o->name) != NULL) {
            presetScope++;
            idl_metaObjectTrace(o, "idl_metaobject", context);
            idl_module (c_module(o), context);
            presetScope--;
            return;
        } else if (presetScope) {
            presetScope++;
            idl_metaObjectTrace(o, "idl_metaobject", context);
            idl_module (c_module(o), context);
            presetScope--;
            return;
        } else {
            /* Do nothing, only to prevent dangling else-ifs QAC messages */
        }
    }
    idl_metaObjectTrace(o, "idl_metaobject", context);
    switch (c_baseObject(o)->kind) {
        /* Illegal object types for application*/
        /* IDL input files*/
    case M_ANNOTATION:
    case M_UNDEFINED:
    case M_ATTRIBUTE:
    case M_CLASS:
    case M_EXCEPTION:
    case M_INTERFACE:
    case M_EXPRESSION:
    case M_OPERATION:
    case M_RELATION:
    case M_PARAMETER:
    case M_BASE:
    case M_COUNT:
    case M_CONSTOPERAND:
    case M_LITERAL:
    case M_MEMBER:
    case M_PRIMITIVE:
    case M_UNIONCASE:
        /* Object types not handled at this level*/
        break;
        /* Object types to handle here */
    case M_CONSTANT:
        idl_constant (c_constant(o), context);
        break;
    case M_COLLECTION:
        if (c_collectionType(o)->kind == C_STRING &&
            c_collectionType(o)->maxSize > 0) {
            idl_boundedString (c_collectionType(o), context);
        }
        if (c_collectionType(o)->kind == C_SEQUENCE) {
            if (idl_unsupportedType (c_type(o)) == 0) {
                idl_sequence (c_collectionType(o), context);
            }
        }
        break;
    case M_ENUMERATION:
        idl_enumeration(c_enumeration(o), context);
        break;
    case M_MODULE:
        idl_module(c_module(o), context);
        break;
    case M_STRUCTURE:
        idl_structure(c_structure(o), context);
        break;
    case M_TYPEDEF:
        if (idl_unsupportedType(c_type(o)) == 0) {
            idl_typedef(c_typeDef(o), context);
        }
        break;
    case M_UNION:
        idl_union((c_union)o, context);
        break;
    }
}

idl_action
idl_moduleOpen(
    idl_context context,
    c_module o)
{
    idl_action next = idl_abort;

    if (context->program->moduleOpen) {
        next = context->program->moduleOpen(context->ownScope, c_metaObject (o)->name, context->program->userData);
    }
    return next;
}

void
idl_moduleClose(
    idl_context context,
    c_module o)
{
    if (context->program->moduleClose) {
        context->program->moduleClose(context->program->userData);
    }
}

void
idl_constant(
    c_constant o,
    idl_context context)
{
    idl_constSpec constSpec;

    if ((c_baseObject(o->type)->kind != M_ENUMERATION) ||
        ((c_baseObject(o->type)->kind == M_ENUMERATION) && (c_baseObject(o->operand)->kind == M_CONSTANT))) {
        /* Ignore enumeration elements */
        constSpec = idl_makeConstSpec (o);
        if (context->program->constantOpenClose) {
            context->program->constantOpenClose(context->ownScope, constSpec, context->program->userData);
        }
    }
}

void
idl_module(
    c_module o,
    idl_context context)
{
    idl_action next;

    if (idl_contextTrace(context)) {
        printf("idl_module: %s\n", c_metaObject(o)->name);
    }
    next = idl_moduleOpen(context, o);
    if (next == idl_explore) {
        idl_scopePush(context->ownScope, idl_scopeElementNew(c_metaObject(o)->name, idl_tModule));
        c_walkIterScope(context->sortedListIter, (c_iterWalkAction)idl_metaobject, context);
        idl_scopePopFree(context->ownScope);
        idl_moduleClose(context, o);
    }
}

void
idl_typedef(
    c_typeDef o,
    idl_context context)
{
    if (idl_contextTrace(context)) {
        printf("idl_typedef: %s (alias for %s)\n", c_metaObject(o)->name,
            idl_scopedName(c_metaObject(o), c_metaObject(o->alias)));
    }
    if (context->program->typedefOpenClose) {
        idl_typeDef typeDef = idl_makeTypeTypedef(o);
        context->program->typedefOpenClose(context->ownScope, c_metaObject(o)->name, typeDef, context->program->userData);
    }
}

void
idl_boundedString(
    c_collectionType o,
    idl_context context)
{
    if (context->program->boundedStringOpenClose) {
        idl_typeBasic typeBasic = idl_makeTypeBasic(c_type(o));
        context->program->boundedStringOpenClose(context->ownScope, typeBasic, context->program->userData);
    }
}

void
idl_sequence(
    c_collectionType o,
    idl_context context)
{
    if (context->program->sequenceOpenClose) {
        idl_typeSeq typeSeq = idl_typeSeq(idl_makeTypeCollection (c_collectionType(c_type(o))));
        context->program->sequenceOpenClose(context->ownScope, typeSeq, context->program->userData);
    }
}

void
idl_union(
    c_union o,
    idl_context context)
{
    int ci;
    idl_switchVal sv;
    idl_action next = idl_abort;
    idl_typeSpec labelType;
    c_bool defFound;
    c_bool userDefFound;
    c_bool eqFound;
    c_ulong labelCount;
    c_ulong labelTotal = 0;
    c_ulong li;

    if (idl_contextTrace(context)) {
        printf("   idl_union: %s\n", c_metaObject(o)->name);
        printf("      idl_unionSwitch: of type %s\n",
            idl_scopedName(c_metaObject(o), c_metaObject(o)));
    }
    if (context->program->idl_getControl) {
        idl_scopePush(context->ownScope, idl_scopeElementNew(c_metaObject(o)->name, idl_tUnion));
        if (context->program->idl_getControl(context->program->userData)->structScopeWalk == idl_prior) {
            c_walkIterScope(context->sortedListIter, (c_iterWalkAction)idl_metaobject, context);
        }
        idl_scopePopFree(context->ownScope);
    }
    if (context->program->unionOpen) {
        idl_typeUnion typeUnion = idl_makeTypeUnion (o);
        next = context->program->unionOpen (context->ownScope, c_metaObject(o)->name, typeUnion, context->program->userData);
        idl_typeUnionFree (typeUnion);
    }
    if (next == idl_explore) {
        idl_scopePush(context->ownScope, idl_scopeElementNew(c_metaObject(o)->name, idl_tUnion));
        if (context->program->idl_getControl) {
            if (context->program->idl_getControl(context->program->userData)->structScopeWalk == idl_inline) {
                c_walkIterScope(context->sortedListIter, (c_iterWalkAction)idl_metaobject, context);
            }
        }
        /* Determine alternative label value for the default label */
        /* The lowest index if not used labels is determined       */
        /* This is for instance required for Java                  */
        {
            defFound = FALSE;
            userDefFound = FALSE;
            sv = idl_switchValNew (o->switchType);
            for (ci = 0; ci < c_arraySize(o->cases); ci++) {
                labelCount = c_arraySize(c_unionCase(o->cases[ci])->labels);
                labelTotal += labelCount;
            }
            while (defFound == FALSE) {
                eqFound = FALSE;
                for (ci = 0; ci < c_arraySize(o->cases); ci++) {
                    labelCount = c_arraySize(c_unionCase(o->cases[ci])->labels);
                    if (labelCount == 0) {
                        userDefFound = TRUE;
                    }
                    for (li = 0; li < labelCount; li++) {
                        if (idl_switchValEq(sv, c_literal (c_unionCase(o->cases[ci])->labels[li])->value) == TRUE) {
                            eqFound = TRUE;
                        }
                    }
                }
                if (eqFound) {
                    idl_switchValInc(sv);
                } else {
                    defFound = TRUE;
                }
            }
            if ((idl_switchValMaxRange (o->switchType)) == labelTotal) {
                /* complete range of values is used */
                defFound = FALSE;
            }
        }
        for (ci = 0; ci < c_arraySize(o->cases); ci++) {
            idl_unionCase (o, o->cases[ci], sv, context);
        }
        /* Make an artificial union default case if the user did   */
        /* not specify a default case and not all values are used  */
        if ((defFound == TRUE) && (userDefFound == FALSE)) {
            if (context->program->artificialDefaultLabelOpenClose) {
                labelType = idl_makeTypeSpec(o->switchType);
                context->program->artificialDefaultLabelOpenClose(context->ownScope,
                    idl_switchLabel(sv),
                    labelType,
                    context->program->userData);
            }
        }
        idl_switchValFree(sv);
        idl_scopePopFree(context->ownScope);
    } else {
        /* Skip structscope */
        c_walkIterScope(context->sortedListIter, 0, context);
    }
    if (context->program->unionClose) {
        context->program->unionClose(c_metaObject(o)->name, context->program->userData);
    }
}

void
idl_unionCase(
    c_union u,
    c_unionCase o,
    idl_switchVal defaultLabelVal,
    idl_context context)
{
    int li;
    int labelCount;
    idl_typeSpec labelType;
    c_type switchType;
    c_enumeration e;

    switchType = u->switchType;
    if (c_baseObject(switchType)->kind == M_TYPEDEF) {
        switchType = c_typeActualType(switchType);
    }
    assert((c_baseObject(switchType)->kind == M_PRIMITIVE) ||
           (c_baseObject(switchType)->kind == M_ENUMERATION));

    labelCount = c_arraySize(o->labels);
    labelType = idl_makeTypeSpec(u->switchType);
    if (context->program->unionLabelsOpenClose) {
        idl_labelSpec labelSpec = idl_labelSpecNew(labelType, labelCount);
        context->program->unionLabelsOpenClose(context->ownScope, labelSpec, context->program->userData);
    }
    if (labelCount == 0) {
        if (idl_contextTrace(context)) {
            printf("      idl_unionCase: default\n");
        }
        if (context->program->unionLabelOpenClose) {
            idl_labelVal alternativeVal = idl_switchLabel(defaultLabelVal);
            idl_labelVal labelVal = idl_labelVal(idl_labelDefaultNew(alternativeVal));

            context->program->unionLabelOpenClose(context->ownScope, labelVal, context->program->userData);
        }
    } else {
        for (li = 0; li < labelCount; li++) {
            if (idl_contextTrace(context)) {
                printf("      idl_unionLabel: %s\n",
                    idl_labelImage(u->switchType, c_literal(o->labels[li])->value));
            }
            if (context->program->unionLabelOpenClose) {
                if (c_baseObject(c_typeActualType(u->switchType))->kind == M_ENUMERATION) {
                    idl_labelEnum labelVal;

                    e = c_enumeration(c_typeActualType(u->switchType))->elements[c_literal(o->labels[li])->value.is.Long];
                    labelVal = idl_labelEnumNew(idl_buildScope(c_metaObject(u->switchType)),
                                   c_metaObject(c_constant(e))->name);
                    context->program->unionLabelOpenClose(context->ownScope, idl_labelVal(labelVal), context->program->userData);
                } else {
                    idl_labelValue labelVal = idl_labelValueNew(c_literal(o->labels[li])->value);
                    context->program->unionLabelOpenClose(context->ownScope, idl_labelVal(labelVal), context->program->userData);
                }
            }
        }
    }

    if (idl_contextTrace(context)) {
        printf("         idl_unionCase: %s (of type %s)\n",
            c_specifier(o)->name,
            idl_scopedName(c_metaObject(u), c_metaObject(c_specifier(o)->type)));
    }
    if (context->program->unionCaseOpenClose) {
        idl_typeSpec caseType = idl_makeTypeSpec(c_specifier(o)->type);
        context->program->unionCaseOpenClose(context->ownScope, c_specifier(o)->name, caseType, context->program->userData);
    }
}

void
idl_primitive(
    c_primitive o,
    idl_context context)
{
}

void
idl_structureMember(
    c_structure s,
    c_member o,
    idl_context context)
{
    if (idl_contextTrace(context)) {
        printf("      idl_structureMember: %s (of type %s)\n",
            c_specifier(o)->name,
            idl_scopedName(c_metaObject(s),
            c_metaObject(c_specifier(o)->type)));
    }
    if (context->program->structureMemberOpenClose) {
        idl_typeSpec typeSpec = idl_makeTypeSpec(c_specifier(o)->type);
        context->program->structureMemberOpenClose(context->ownScope, c_specifier(o)->name, typeSpec, context->program->userData);
    }
}

idl_action
idl_structureOpen(
    idl_context context,
    c_structure o)
{
    idl_action next = idl_abort;

    if (context->program->structureOpen) {
        idl_typeStruct structSpec = idl_makeTypeStruct(o);
        next = context->program->structureOpen(context->ownScope, c_metaObject (o)->name, structSpec, context->program->userData);
    }
    return next;
}

void
idl_structureClose(
    idl_context context,
    c_structure o)
{
    if (context->program->structureClose) {
        context->program->structureClose(c_metaObject(o)->name, context->program->userData);
    }
}

void
idl_structure(
    c_structure o,
    idl_context context)
{
    int mi;
    idl_action next;

    if (idl_contextTrace(context)) {
        printf("   idl_structure: %s\n", c_metaObject(o)->name);
    }
    if (context->program->idl_getControl) {
        if (context->program->idl_getControl(context->program->userData)->structScopeWalk == idl_prior) {
            idl_scopePush(context->ownScope, idl_scopeElementNew(c_metaObject(o)->name, idl_tStruct));
            c_walkIterScope(context->sortedListIter, (c_iterWalkAction)idl_metaobject, context);
            idl_scopePopFree(context->ownScope);
        }
    }
    next = idl_structureOpen(context, o);
    if (next == idl_explore) {
        idl_scopePush(context->ownScope, idl_scopeElementNew(c_metaObject(o)->name, idl_tStruct));
        if (context->program->idl_getControl) {
            if (context->program->idl_getControl(context->program->userData)->structScopeWalk == idl_inline) {
                c_walkIterScope(context->sortedListIter, (c_iterWalkAction)idl_metaobject, context);
            }
        }
        for (mi = 0; mi < c_arraySize(o->members); mi++) {
            idl_structureMember(o, o->members[mi], context);
        }
        idl_scopePopFree(context->ownScope);
        idl_structureClose(context, o);
    } else {
       /* Skip structscope */
        c_walkIterScope(context->sortedListIter, 0, context);
    }

}

void
idl_enumerationElementOpenClose (
    idl_context context,
    c_enumeration e,
    c_constant o)
{
    if (context->program->enumerationElementOpenClose) {
        context->program->enumerationElementOpenClose(context->ownScope, c_metaObject(o)->name, context->program->userData);
    }
}

void
idl_enumerationElement(
    c_enumeration e,
    c_constant o,
    idl_context context)
{
    if (idl_contextTrace(context)) {
        printf("     idl_enumerationElement: %s (%d)\n",
               c_metaObject(o)->name,
               (int)idl_enumElementValue(c_literal(o->operand)));
    }
    idl_enumerationElementOpenClose(context, e, o);
}

idl_action
idl_enumerationOpen(
    idl_context context,
    c_enumeration o,
    c_long noElements)
{
    idl_action next = idl_abort;

    if (context->program->enumerationOpen) {
        idl_typeEnum enumSpec = idl_makeTypeEnum(o);
        next = context->program->enumerationOpen(context->ownScope, c_metaObject (o)->name, enumSpec, context->program->userData);
    }

    return next;
}

void
idl_enumerationClose(
    idl_context context,
    c_enumeration o)
{
    if (context->program->enumerationClose) {
        context->program->enumerationClose(c_metaObject(o)->name, context->program->userData);
    }
}

typedef struct idl_emulateScopesWalk_t {
    c_metaObject scope;
    c_iter objects;
}idl_emulateScopesWalk_t;

static void
idl_emulateScopesAdd(c_iter objects, c_metaObject from, c_metaObject to)
{
    if(to != from) {
        idl_emulateScopesAdd(objects, from, to->definedIn);
        c_iterAppend(objects, to);
        c_iterAppend(objects, SCOPE_OPEN);
    }
}

static void
idl_emulateScopesEnter(c_metaObject from, c_metaObject to, idl_emulateScopesWalk_t* userData) {
    c_metaObject scope, sFrom, sTo;

    /* Find lowest common scope (if !sFrom, from = root) */
    sFrom = from;
    sTo = to;

    /* No need to test for NULL since there is always a common scope. */
    while(sFrom != sTo) {
        sTo = to;
        while(sTo && (sTo != sFrom)) {
            sTo = sTo->definedIn;
        }
        if(sFrom != sTo) {
            sFrom = sFrom->definedIn;
        }
    }

    /* Common scope is now stored in both sFrom and sTo */

    /* Add zero-markers from 'from' to the common scope (exit scope) */
    scope = from;
    while(scope != sFrom) {
        c_iterAppend(userData->objects, SCOPE_CLOSE);
        scope = scope->definedIn;
    }

    /* Recursively add scopes from the common scope to 'to' to the list. */
    idl_emulateScopesAdd(userData->objects, sTo, to);

    /* Set scope to 'to'. */
    userData->scope = to;
}

typedef struct idl_alreadyAdded_t {
    c_metaObject o;
    int found;
}idl_alreadyAdded_t;

static void
idl_alreadyAddedWalk(
    c_metaObject o,
    idl_alreadyAdded_t* udata)
{
    if (o == udata->o) {
        udata->found = 1;
    }
}

/* Emulate a scopewalk by adding scope objects */
static void
idl_emulateScopesWalk(
    c_metaObject o,
    idl_emulateScopesWalk_t* userData)
{
    switch(o->_parent.kind) {
    case M_MODULE:
        if(o->definedIn != userData->scope) {
            idl_emulateScopesEnter(userData->scope, o->definedIn, userData);
        }

        c_iterAppend(userData->objects, o);
        c_iterAppend(userData->objects, SCOPE_OPEN);
        userData->scope = o;
        break;
    case M_STRUCTURE:
    case M_UNION:
    {
        idl_alreadyAdded_t udata;
        udata.o = o;
        udata.found = 0;

        /* Walk objects */
        c_iterWalk(userData->objects, (c_iterWalkAction)idl_alreadyAddedWalk, &udata);
        if(!udata.found) {
            if(o->definedIn != userData->scope) {
                idl_emulateScopesEnter(userData->scope, o->definedIn, userData);
            }

            c_iterAppend(userData->objects, o);
            c_iterAppend(userData->objects, SCOPE_OPEN);
            userData->scope = o;
        }

        break;
    }
    default:
        if(o->definedIn != userData->scope) {
            idl_emulateScopesEnter(userData->scope, o->definedIn, userData);
        }
        c_iterAppend(userData->objects, o);
        break;
    }
}

/* Add scopes to list of meta-objects */
static c_iter
idl_emulateScopes(
    c_base base,
    c_iter objects)
{
    idl_emulateScopesWalk_t walkData;

    /* Start with base */
    walkData.scope = c_metaObject(base);
    walkData.objects = c_iterNew(SCOPE_OPEN);
    c_iterWalk(objects, (c_iterWalkAction)idl_emulateScopesWalk, &walkData);

    return walkData.objects;
}

void showObjects(c_metaObject o, void* udata) {
    if(o == SCOPE_OPEN) {
        printf(" > <scope open>\n");
    }else
    if(o != SCOPE_CLOSE) {
        printf(" > %s.\n", c_metaScopedName(o));
    }else {
        printf(" > <scope close>\n");
    }
}

void
idl_enumeration(
    c_enumeration o,
    idl_context context)
{
    int ei;
    idl_action next;

    if (idl_contextTrace(context)) {
        printf ("   idl_enumeration: %s\n", c_metaObject(o)->name);
    }
    next = idl_enumerationOpen(context, o, c_arraySize(o->elements));
    if (next == idl_explore) {
        for (ei = 0; ei < c_arraySize(o->elements); ei++) {
            idl_enumerationElement(o, c_constant(o->elements[ei]), context);
        }
    }
    idl_enumerationClose(context, o);
}

/* Walk preset modules */
struct idl_presetModuleWalk_t {
    c_base base;
    c_iter objects;
};

static void idl_collectModuleObjects(c_metaObject o, c_iter objects);

static void idl_insertModuleObject(c_metaObject o, c_iter objects) {

    switch(c_baseObject(o)->kind) {
    case M_MODULE:
    case M_STRUCTURE:
    case M_INTERFACE:
    case M_CLASS:
        idl_collectModuleObjects(o, objects);

    /* Fallthrough on purpose */
    default:
        c_iterInsert(objects, o);
        break;
    }
}

static void idl_collectModuleObjects(c_metaObject o, c_iter objects) {
    c_metaWalk(o, (c_metaWalkAction)idl_insertModuleObject, objects);
}

static void idl_presetModuleWalk(c_string module, struct idl_presetModuleWalk_t* udata) {
    c_metaObject o;

    o = c_metaResolve(c_metaObject(udata->base), module);
    if(o) {
        idl_collectModuleObjects(o, udata->objects);
        c_iterInsert(udata->objects, o);
    }
}

void
idl_walk(
    c_base base,
    const char *fileName,
    c_iter objects,
    c_bool traceWalk,
    idl_program program)
{
    struct idl_context_s context;
    idl_action action;
    c_iter i;
    c_iterIter iterator;
    struct idl_presetModuleWalk_t walkData;

    memset(&context, 0, sizeof(struct idl_context_s));

    if(!objects) {
        objects = c_iterNew(NULL);
    }

    /* Insert objects from preset modules */
    if(presetModules) {
        walkData.base = base;
        walkData.objects = objects;
        c_iterWalk(presetModules, (c_iterWalkAction)idl_presetModuleWalk, &walkData);
    }

    /* Emulate scopes */
    i = idl_emulateScopes(base, objects);

    /* Obtain iterator */
    iterator = c_iterIterGet(i);

    context.program = program;
    context.traceWalk = traceWalk;
    context.fileName = os_strdup(fileName);
    context.baseName = os_strdup(idl_basename(fileName));
    context.ownScope = idl_scopeNew (context.baseName);
    context.sortedListIter = &iterator;

    if (traceWalk) {
        /* show all objects in the database */
        c_iterWalk(objects, (c_iterWalkAction)idl_iterTrace, NULL);
    }

    if (context.program->fileOpen) {
        action = context.program->fileOpen(context.ownScope, context.baseName, context.program->userData);
        if (action == idl_explore) {
            c_walkIterScope(&iterator, (c_iterWalkAction)idl_metaobject, &context);
            if (context.program->fileClose) {
               context.program->fileClose(context.program->userData);
            }
        }
    }

    idl_scopeFree(context.ownScope);
    os_free(context.baseName);
}
