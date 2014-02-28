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
#include "os_report.h"
#include "c__metabase.h"
#include "c__base.h"
#include "c__scope.h"
#include "c_misc.h"
#include "c_sync.h"
#include "c_iterator.h"
#include "c_stringSupport.h"
#include "c__collection.h"
#include "c_metafactory.h"
#include <ctype.h>
#include "c_module.h"
#include "ut_collection.h"

extern c_type c_getMetaType(c_base base, c_metaKind kind);

#define MINSTRUCTALIGNMENT (C_ALIGNMENT(struct{c_char c;}))

_TYPE_CACHE_(c_member)
_TYPE_CACHE_(c_literal)
_TYPE_CACHE_(c_constant)
_TYPE_CACHE_(c_unionCase)
_TYPE_CACHE_(c_property)

static c_scope
metaClaim(
    c_metaObject scope)
{
    if (scope == NULL) {
        return NULL;
    }
    switch(c_baseObjectKind(scope)) {
    case M_MODULE:
        c_mutexLock(&c_module(scope)->mtx);
        return c_module(scope)->scope;
    case M_INTERFACE:
    case M_CLASS:
    case M_ANNOTATION:
        return c_interface(scope)->scope;
    case M_STRUCTURE:
    case M_EXCEPTION:
        return c_structure(scope)->scope;
    case M_ENUMERATION:
        return metaClaim(scope->definedIn);
    case M_UNION:
        return c_union(scope)->scope;
    case M_TYPEDEF:
        return metaClaim(scope->definedIn);
    default:
        return NULL;
    }
}

static void
metaRelease(
    c_metaObject scope)
{
    switch(c_baseObjectKind(scope)) {
    case M_MODULE:
        c_mutexUnlock(&c_module(scope)->mtx);
    break;
    case M_ENUMERATION:
        metaRelease(scope->definedIn);
    break;
    case M_TYPEDEF:
        metaRelease(scope->definedIn);
    break;
    default:
        /* nothing to do for others. */
    break;
    }
}

static c_metaObject
metaScopeInsert(
    c_metaObject scope,
    c_metaObject object)
{
    c_scope s;
    c_metaObject o;

    s = metaClaim(scope);
    o = c_scopeInsert(s,object);
    metaRelease(scope);
    return o;
}

static c_metaObject
metaScopeLookup(
    c_metaObject scope,
    const c_char *name,
    c_long metaFilter)
{
    c_scope s;
    c_metaObject o;

    s = metaClaim(scope);
    o = c_metaObject(c_scopeResolve(s,name,metaFilter));
    metaRelease(scope);
    return o;
}

static void
metaScopeWalk(
    c_metaObject scope,
    c_scopeWalkAction action,
    c_scopeWalkActionArg actionArg)
{
    c_scope s;

    s = metaClaim(scope);
    if(s != NULL){
        c_scopeWalk(s, action, actionArg);
    }
    metaRelease(scope);
}

static c_long
metaScopeCount(
    c_metaObject scope)
{
    c_scope s;
    c_long n = 0;

    s = metaClaim(scope);
    if(s != NULL){
        n = c_scopeCount(s);
    }
    metaRelease(scope);
    return n;
}

c_metaObject
c_metaBind(
    c_metaObject scope,
    const c_char *name,
    c_metaObject object)
{
    c_metaObject found;
    c_metaEquality equality;

    assert(name != NULL);
    assert(object != NULL);

    if (object->name != NULL) {
        OS_REPORT_1(OS_ERROR,"c_metaObject::c_metaBind",0,
                    "Object already bound to \"%s\"",object->name);
        return NULL;
    }
    object->name = c_stringNew(c__getBase(scope),name);
    found = metaScopeInsert(scope,object);

    if (found == object) {
        object->definedIn = scope;
        return object;
    } else {
        equality = c_metaCompare(found,object);
        if (equality != E_EQUAL) {
            c_free(found);
            return NULL;
        }
        c_free(object->name);
        object->name = NULL;
        return found;
    }
}

typedef enum c_token {
    TK_ERROR,
    TK_IDENT,
    TK_DOT,
    TK_REF,
    TK_SLASH,
    TK_SCOPE,
    TK_COMMA,
    TK_END
} c_token;


static c_bool
c_getInstantiationType(
    const c_char **str)
{
    c_bool result;

    if (**str != '<') {
        return TRUE;
    }
    (*str)++;
    while (**str != '>') {
        if (**str == '<') {
            /* Nested brackets, recursively scan */
            result = c_getInstantiationType(str);
            if (result == FALSE) {
                return FALSE;
            }
        } else {
            /* Unexpected end of string */
            if (**str == '\0') {
                return FALSE;
            } else {
                /* No bracket, no terminator, so skip to next */
                (*str)++;
            }
        }
    }
    /* We have reached the closing tag '>' */
    (*str)++;
    return TRUE;
}


static c_token
c_getToken(
    const c_char **str)
{
    c_bool stop,result;

    if ((str == NULL) || (*str == NULL)) return TK_ERROR;
    *str = c_skipSpaces(*str);
    if (**str == '.') {
        (*str)++;
        return TK_DOT;
    }
    if (**str == '/') {
        (*str)++;
        return TK_SLASH;
    }
    if (**str == ',') {
        (*str)++;
        return TK_COMMA;
    }
    if (strncmp(*str,"->",2) == 0) {
        (*str) += 2;
        return TK_REF;
    }
    if (strncmp(*str,"::",2) == 0)  {
        (*str) += 2;
        return TK_SCOPE;
    }
    if (c_isLetter(**str) || (**str == '_')) {
        (*str)++;
        stop = FALSE;
        while (!stop) {
            if (c_isLetter(**str) || c_isDigit(**str) || (**str == '_') || (**str == ',')) {
                (*str)++;
            } else if (**str == '<') {
                result = c_getInstantiationType(str);
                if (result == FALSE) {
                    return TK_ERROR;
                }
            } else {
                stop = TRUE;
            }
        }
        return TK_IDENT;
    }
    if (**str == 0) {
        return TK_END;
    }
    return TK_ERROR;
}

typedef enum c_state {
    ST_ERROR,
    ST_START,
    ST_IDENT,
    ST_REFID,
    ST_SCOPE,
    ST_REF,
    ST_END
} c_state;

void
c_metaWalk(
    c_metaObject scope,
    c_metaWalkAction action,
    c_metaWalkActionArg actionArg)
{
    metaScopeWalk(scope,action,actionArg);
}

/*
 * Returns the number of elements in the c_scope of the given c_metaObject.
 */
c_long
c_metaCount(
    c_metaObject scope)
{
    return metaScopeCount(scope);
}

c_object
c_metaDefine(
    c_metaObject scope,
    c_metaKind kind)
{
    c_baseObject o;
    c_base base;

    assert(scope != NULL);

    base = c__getBase(scope);

    switch (kind) {
    case M_ATTRIBUTE:
    case M_CONSTANT:
    case M_CONSTOPERAND:
    case M_EXPRESSION:
    case M_LITERAL:
    case M_MEMBER:
    case M_OPERATION:
    case M_PARAMETER:
    case M_RELATION:
    case M_UNIONCASE:
        o = (c_baseObject)c_new(c_getMetaType(base,kind));
        if (o) {
            o->kind = kind;
        }
    break;
    case M_COLLECTION:
    case M_ENUMERATION:
    case M_PRIMITIVE:
    case M_TYPEDEF:
    case M_BASE:
        o = (c_baseObject)c_new(c_getMetaType(base,kind));
        if (o) {
            o->kind = kind;
            c_type(o)->base = base; /* REMARK c_keep(base); */
        }
    break;
    case M_UNION:
        o = (c_baseObject)c_new(c_getMetaType(base,kind));
        if (o) {
            o->kind = kind;
            c_union(o)->scope = c_scopeNew(base);
            c_type(o)->base = base; /* REMARK c_keep(base); */
        }
    break;
    case M_STRUCTURE:
    case M_EXCEPTION:
        o = (c_baseObject)c_new(c_getMetaType(base,kind));
        if (o) {
            o->kind = kind;
            c_structure(o)->scope = c_scopeNew(base);
            c_type(o)->base = base; /* REMARK c_keep(base); */
        }
    break;
    case M_MODULE:
        o = (c_baseObject)c_new(c_getMetaType(base,kind));
        if (o) {
            o->kind = kind;
            c_module(o)->scope = c_scopeNew(base);
            c_mutexInit(&c_module(o)->mtx, c_baseGetMutexAttr(base));
        }
    break;
    case M_ANNOTATION:
    case M_CLASS:
    case M_INTERFACE:
        o = (c_baseObject)c_new(c_getMetaType(base,kind));
        if (o) {
            o->kind = kind;
            if (kind == M_CLASS) {
                c_class(o)->extends = NULL;
            }else if (kind == M_ANNOTATION) {
                c_annotation(o)->extends = NULL;
            }
            c_interface(o)->scope = c_scopeNew(base);
            c_type(o)->base = base; /* REMARK c_keep(base); */
        }
    break;
    default:
        o = NULL;
    }
    return c_object(o);
}

c_metaObject
c_metaDeclare(
    c_metaObject scope,
    const c_char *name,
    c_metaKind kind)
{
    c_metaObject o,found;

    assert(scope != NULL);
    assert(name != NULL);
    o = c_metaObject(c_metaFindByName (scope, name, CQ_METAOBJECTS | CQ_FIXEDSCOPE));
    if (o == NULL) {
        switch(kind) {
        case M_CLASS:
        case M_COLLECTION:
        case M_CONSTANT:
        case M_ATTRIBUTE:
        case M_ENUMERATION:
        case M_EXCEPTION:
        case M_INTERFACE:
        case M_MODULE:
        case M_OPERATION:
        case M_PARAMETER:
        case M_PRIMITIVE:
        case M_RELATION:
        case M_STRUCTURE:
        case M_TYPEDEF:
        case M_UNION:
        case M_ANNOTATION:
            o = c_metaDefine(scope,kind);
            found = c_metaBind(scope,name,o);
            c_free(o);
            o = found;
            break;
        default:
            OS_REPORT_1(OS_WARNING,"c_metaDeclare failed",0,
                        "illegal meta kind (%d) specified",kind);
            assert(FALSE);
            o = NULL;
            break;
        }
    } else if (c_baseObjectKind(o) != kind) {
        c_free(o);
        o = NULL;
    }

    return o;
}

static void
c_typeInit(
    c_metaObject o,
    c_long alignment,
    c_long size)
{
    c_type(o)->base = c__getBase(o);
    c_type(o)->alignment = alignment;
    c_type(o)->size = size;
}


static void
getProperties(
    c_metaObject o,
    c_iter iter)
{
    assert(iter);
    switch(c_baseObjectKind(o)) {
    case M_ATTRIBUTE:
    case M_RELATION:
    case M_MEMBER:
    case M_UNIONCASE:
        (void)c_iterInsert(iter,o);
    break;
    default:
    break;
    }
}

static void
getPropertiesScopeWalkAction(
    c_metaObject o,
    c_scopeWalkActionArg arg /* c_iter */)
{
    getProperties(o, (c_iter)arg);
}
#if 0
static c_long
c_compareMembers(
    c_member *m1,
    c_member *m2)
{
    c_type t1,t2;
#define _ALIGN_(m) (c_specifier(m)->type->alignment)
#define _KIND_(m)  (c_baseObjectKind(c_specifier(m)->type))
    if (*m1 == *m2) return 0;
    if (*m1 == NULL) return 1;
    if (*m2 == NULL) return -1;
    t1 = c_specifier(*m1)->type;
    while (c_baseObjectKind(t1) == M_TYPEDEF) {
        t1 = c_typeDef(t1)->alias;
    }
    t2 = c_specifier(*m2)->type;
    while (c_baseObjectKind(t2) == M_TYPEDEF) {
        t2 = c_typeDef(t2)->alias;
    }
    if ((c_baseObjectKind(t1) == M_COLLECTION) &&
        (c_baseObjectKind(t2) != M_COLLECTION)) return 1;
    if ((c_baseObjectKind(t1) != M_COLLECTION) &&
        (c_baseObjectKind(t2) == M_COLLECTION)) return -1;
    if (_ALIGN_(*m1) > _ALIGN_(*m2)) return -1;
    if (_ALIGN_(*m1) < _ALIGN_(*m2)) return 1;
    return strcmp(c_specifier(*m1)->name,c_specifier(*m2)->name);
#undef _KIND_
#undef _ALIGN_
}
#endif

static c_long
c_compareProperty(
    const void *ptr1,
    const void *ptr2)
{
    c_type t1,t2;
    const c_property *p1, *p2;

    p1 = (const c_property *)ptr1;
    p2 = (const c_property *)ptr2;

#define _ALIGN_(p) ((p)->type->alignment == 8 ? 4 : (p)->type->alignment)
#define _KIND_(p)  (c_baseObjectKind((p)->type))
#define _NAME_(p)  (c_metaObject(p)->name)
    if (*p1 == *p2) return 0;
    if (*p1 == NULL) return 1;
    if (*p2 == NULL) return -1;
    t1 = (*p1)->type;
    while (c_baseObjectKind(t1) == M_TYPEDEF) {
        t1 = c_typeDef(t1)->alias;
    }
    t2 = (*p2)->type;
    while (c_baseObjectKind(t2) == M_TYPEDEF) {
        t2 = c_typeDef(t2)->alias;
    }
    if (c_typeIsRef(t1) && !c_typeIsRef(t2)) return 1;
    if (!c_typeIsRef(t1) && c_typeIsRef(t2)) return -1;
    if (_ALIGN_(*p1) > _ALIGN_(*p2)) return -1;
    if (_ALIGN_(*p1) < _ALIGN_(*p2)) return 1;
    return strcmp(_NAME_(*p1),_NAME_(*p2));
#undef _NAME_
#undef _KIND_
#undef _ALIGN_
}


c_type
c_typeActualType(
    c_type type)
{
    c_type t = type;

    while (c_baseObjectKind(t) == M_TYPEDEF) {
        t = c_typeDef(t)->alias;
    }

    return t;
}

c_bool
c_typeIsRef (
    c_type type)
{
    c_bool result;

    switch(c_baseObjectKind(type)) {
    case M_CLASS:
    case M_INTERFACE:
    case M_ANNOTATION:
        result = TRUE;
    break;
    case M_COLLECTION:
        switch(c_collectionType(type)->kind) {
        case C_ARRAY:
            if (c_collectionType(type)->maxSize == 0) {
                result = TRUE;
            } else {
                result = FALSE;
            }
        break;
        default:
            result = TRUE;
        }
    break;
    case M_TYPEDEF:
        result = c_typeIsRef(c_typeActualType(type));
    break;
    default:
        result = FALSE;
    break;
    }

    return result;
}

c_long
c_typeSize(
    c_type type)
{
    c_type subType;
    c_long size;

    if (c_baseObjectKind(type) == M_COLLECTION) {
        switch(c_collectionTypeKind(type)) {
        case C_ARRAY:
            subType = c_collectionTypeSubType(type);
            switch(c_baseObjectKind(subType)) {
            case M_INTERFACE:
            case M_CLASS:
            case M_ANNOTATION:
                size = c_collectionTypeMaxSize(type)*sizeof(void *);
            break;
            default:
                if (subType->size == 0) {
                    subType->size = sizeof(void *);
                }
                size = c_collectionTypeMaxSize(type)*subType->size;
            break;
            }
        break;
        case C_LIST:
            size = c_listSize;
        break;
        case C_BAG:
            size = c_bagSize;
        break;
        case C_SET:
            size = c_setSize;
        break;
        case C_DICTIONARY:
            size = c_tableSize;
        break;
        case C_QUERY:
            size = c_querySize;
        break;
        case C_SCOPE:
            size = C_SIZEOF(c_scope);
        break;
        case C_STRING:
            size = sizeof(c_char *);
        break;
        case C_SEQUENCE:
            size = sizeof(c_address);
        break;
        default:
            OS_REPORT(OS_ERROR,
                      "c_typeSize failed",0,
                      "illegal type specified");
            assert(FALSE);
            size = -1;
        break;
        }
    } else {
        size = type->size;
    }

    return size;
}

c_bool
c_typeHasRef (
    c_type type)
{
    switch(c_baseObjectKind(type)) {
    case M_CLASS:
    case M_INTERFACE:
    case M_ANNOTATION:
        return TRUE;
    case M_COLLECTION:
        switch(c_collectionType(type)->kind) {
        case C_ARRAY:
            if (c_collectionType(type)->maxSize == 0) {
                return TRUE;
            } else {
                return c_typeHasRef(c_collectionType(type)->subType);
            }
        default:
            return TRUE;
        }
    case M_EXCEPTION:
    case M_STRUCTURE:
        if (c_structure(type)->references != NULL) {
            return TRUE;
        }
        return FALSE;
    case M_UNION:
        if (c_union(type)->references != NULL) {
            return TRUE;
        }
        return FALSE;
    case M_PRIMITIVE:
        switch (c_primitive(type)->kind) {
        case P_MUTEX:
        case P_LOCK:
        case P_COND:
            return TRUE;
        default:
            return FALSE;
        }
    case M_ENUMERATION:
    case M_BASE:
        return FALSE;
    case M_TYPEDEF:
        return c_typeHasRef(c_typeDef(type)->alias);
    default:
        OS_REPORT(OS_WARNING,
                  "c_typeHasRef failed",0,
                  "specified type is not a type");
        assert(FALSE); /* not a type */
    break;
    }
    return FALSE;
}

static c_long
alignSize(
    c_long size,
    c_long alignment)
{
    c_long gap;

    if (size == 0) return 0;

    gap = size % alignment;
    if (gap != 0) gap = alignment - gap;
    return (size + gap);
}

static c_long
propertySize(
    c_type type)
{
    switch(c_baseObjectKind(type)) {
    case M_COLLECTION:
        if ((c_collectionType(type)->kind == C_ARRAY) &&
            (c_collectionType(type)->maxSize != 0)) {
            assert(type->size > 0);
            return type->size;
        } else {
            return sizeof(void *);
        }
    case M_CLASS:
        return sizeof(void *);
    case M_TYPEDEF:
        return propertySize(c_typeDef(type)->alias);
    default:
        return type->size;
    }
}

static c_metaEquality
_c_metaCompare (
    c_metaObject object,
    c_metaObject obj,
    ut_collection adm)
{
    c_long i, length;
    c_metaEquality result;
    c_metaObject o = obj;

    if(object && o && (o == ut_get(adm, object)))
    {
        /* if the object-obj pair is in the adm-tree, then it is already being compared,
         * so ignore here by returning equal.
         */
        return E_EQUAL;
    }
    else if(object && o)
    {
        ut_tableInsert(ut_table(adm), object, o);
    }

    if (object == o) {
        return E_EQUAL;
    }
    if ((object == NULL) || (o == NULL)) {
        return E_UNEQUAL;
    }
    if (c_baseObjectKind(object) != c_baseObjectKind(o)) {
        return E_UNEQUAL;
    }

    switch(c_baseObjectKind(object)) {
    case M_PRIMITIVE:
        if ( c_primitive(object)->kind == c_primitive(o)->kind) {
            return E_EQUAL;
        } else {
            return E_UNEQUAL;
        }
    case M_EXCEPTION:
    case M_STRUCTURE:
    {
        c_member member,m;
        c_long length;

        length = c_arraySize(c_structure(object)->members);

        if (length != c_arraySize(c_structure(o)->members)) {
            return E_UNEQUAL;
        }
        for (i=0; i<length; i++) {
            member = c_member(c_structure(object)->members[i]);
            m = c_member(c_structure(o)->members[i]);
            if (member == m) {
                return E_EQUAL;
            }
            result = _c_metaCompare(c_metaObject(c_specifier(member)->type),
                                   c_metaObject(c_specifier(m)->type),
                                   adm);
            if (result != E_EQUAL) {
                return result;
            }
            if (strcmp(c_specifier(member)->name,c_specifier(m)->name) != 0) {
                return E_UNEQUAL;
            }
        }
    }
    break;
    case M_UNION:
    {
        c_unionCase c1,c2;

        result = _c_metaCompare(c_metaObject(c_union(object)->switchType),
                               c_metaObject(c_union(o)->switchType),
                               adm);
        if (result != E_EQUAL) {
            return result;
        }
        if (c_union(o)->cases == c_union(o)->cases) {
            return E_EQUAL; /* used to identify both being NULL */
        }
        length = c_arraySize(c_union(object)->cases);
        if (length != c_arraySize(c_union(o)->cases)) {
            return E_UNEQUAL;
        }
        for (i=0; i<length; i++) {
            c1 = c_unionCase(c_union(object)->cases[i]);
            c2 = c_unionCase(c_union(o)->cases[i]);
            result = _c_metaCompare(c_metaObject(c_specifier(c1)->type),
                                   c_metaObject(c_specifier(c2)->type),
                                   adm);
            if (result != E_EQUAL) {
                return result;
            }
            if (strcmp(c_specifier(c1)->name,c_specifier(c2)->name) != 0) {
                return E_UNEQUAL;
            }
        }
    }
    break;
    case M_COLLECTION:
        if (c_collectionType(object)->kind != c_collectionType(o)->kind) {
            return E_UNEQUAL;
        }
        result = _c_metaCompare(c_metaObject(c_collectionType(object)->subType),
                               c_metaObject(c_collectionType(o)->subType),
                               adm);
        if (result != E_EQUAL) {
            return result;
        }
        if (c_collectionType(object)->maxSize != c_collectionType(o)->maxSize) {
            return E_UNEQUAL;
        }
    break;
    case M_ENUMERATION:
    {
        c_constant c;

        length = c_arraySize(c_enumeration(object)->elements);
        if (length != c_arraySize(c_enumeration(o)->elements)) {
            return E_UNEQUAL;
        }

        for (i=0; i<length; i++) {
            c = c_constant(c_enumeration(object)->elements[i]);
            if (strcmp (c_metaObject(c)->name, c_metaObject(c_enumeration(o)->elements[i])->name) != 0) {
                return E_UNEQUAL;
            }
        }
    }
    break;
    case M_INTERFACE:
    case M_CLASS:
    case M_ANNOTATION:
    {
        c_property property,p;
        c_iter iter;

        if (c_baseObjectKind(o) == M_CLASS) {
            result = _c_metaCompare(c_metaObject(c_class(object)->extends),
                                   c_metaObject(c_class(o)->extends),
                                   adm);
            if (result != E_EQUAL) {
                return result;
            }
        }

        if (metaScopeCount(c_metaObject(o)) != metaScopeCount(c_metaObject(object))) {
            return E_UNEQUAL;
        }
        iter = c_iterNew(NULL);
        metaScopeWalk(c_metaObject(o), getPropertiesScopeWalkAction, iter);

        length = c_iterLength(iter);
        if (length > 0) {
            for (i=0;i<length;i++) {
                property = c_iterTakeFirst(iter);
                p = c_property(metaScopeLookup(c_metaObject(o),
                         c_metaObject(property)->name,CQ_METAOBJECTS));
                if (p == NULL) {
                    c_free(property); c_free(p);
                    return E_UNEQUAL;
                }
                result = _c_metaCompare(c_metaObject(property->type),
                                       c_metaObject(p->type),
                                       adm);
                c_free(p);
                if (result != E_EQUAL) {
                    return result;
                }
            }
        }
        c_iterFree(iter);
    }
    break;
    case M_TYPEDEF:
    {
        return _c_metaCompare(c_metaObject(c_typeDef(object)->alias),
                             c_metaObject(c_typeDef(o)->alias),
                             adm);
    }
    default:
    break;
    }
    return E_EQUAL;
}

/*
 * Compares two pointers.
 * Returns:
 *  - OS_EQ when pointers are equal
 *  - OS_GT when o1 is bigger than o2
 *  - OS_LT when o1 is littler than o2
 */
static os_equality
comparePointers(
        void *o1,
        void *o2,
        void *args)
{
    os_equality result = OS_EQ;

    OS_UNUSED_ARG(args);

    if(o1 > o2) {
        result = OS_GT;
    }
    else if(o1 < o2)
    {
        result = OS_LT;
    }
    return result;
}

c_metaEquality
c_metaCompare(
        c_metaObject object,
        c_metaObject obj)
{
    ut_collection adm;
    c_metaEquality result;
    adm = ut_tableNew(comparePointers, NULL);

    result = _c_metaCompare(
            object,
            obj,
            adm);

    ut_tableFree(ut_table(adm), NULL, NULL, NULL, NULL);

    return result;
}

c_result
c__metaFinalize(
    c_metaObject o,
    c_bool normalize)
{
    c_address i, alignment, size;
    c_address objectAlignment = C_ALIGNMENT(c_object);
    c_address ps;

#define _TYPEINIT_(o,t) c_typeInit(o,C_ALIGNMENT(t),sizeof(t))

    switch(c_baseObjectKind(o)) {
    case M_PRIMITIVE:
        switch(c_primitive(o)->kind) {
        case P_ADDRESS:   _TYPEINIT_(o,c_address);   break;
        case P_BOOLEAN:   _TYPEINIT_(o,c_bool);      break;
        case P_CHAR:      _TYPEINIT_(o,c_char);      break;
        case P_WCHAR:     _TYPEINIT_(o,c_wchar);     break;
        case P_OCTET:     _TYPEINIT_(o,c_octet);     break;
        case P_SHORT:     _TYPEINIT_(o,c_short);     break;
        case P_USHORT:    _TYPEINIT_(o,c_ushort);    break;
        case P_LONG:      _TYPEINIT_(o,c_long);      break;
        case P_ULONG:     _TYPEINIT_(o,c_ulong);     break;
        case P_LONGLONG:  _TYPEINIT_(o,c_longlong);  break;
        case P_ULONGLONG: _TYPEINIT_(o,c_ulonglong); break;
        case P_DOUBLE:    _TYPEINIT_(o,c_double);    break;
        case P_FLOAT:     _TYPEINIT_(o,c_float);     break;
        case P_VOIDP:     _TYPEINIT_(o,c_voidp);     break;
        case P_MUTEX:     _TYPEINIT_(o,c_mutex);     break;
        case P_LOCK:      _TYPEINIT_(o,c_lock);      break;
        case P_COND:      _TYPEINIT_(o,c_cond);      break;
        default:          c_typeInit(o,0,0);         break;
        }
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
    {
        c_member member;
        c_type type;
        c_iter refList;
        c_address length;

        alignment = 0; size = 0; refList = NULL;
        if (c_structure(o)->members != NULL) {
            length = c_arraySize(c_structure(o)->members);
            for (i=0; i<length; i++) {
                member = c_member(c_structure(o)->members[i]);
                type = c_specifier(member)->type;
                if (c_typeHasRef(type)) {
                    refList = c_iterInsert(refList,member);
                }
                if (!c_isFinal(o)) {
                    switch(c_baseObjectKind(type)) {
                    case M_INTERFACE:
                    case M_ANNOTATION:
                    case M_CLASS:
                    case M_BASE:
                        if (alignment < objectAlignment) {
                            alignment = objectAlignment;
                        }
                        member->offset = alignSize(size,objectAlignment);
                        size = member->offset + sizeof(void *);
                    break;
                    default:
                        if (type->alignment > alignment) {
                            alignment = type->alignment;
                        }
                        member->offset = alignSize(size,type->alignment);
                        ps = propertySize(type);
                        if (ps == 0) {
                            return S_ILLEGALRECURSION;
                        }
                        size = member->offset + ps;
                    break;
                    }
                }
            }
            if (refList != NULL) {
                i=0;
                type = c_member_t(c__getBase(o));
                length = c_iterLength(refList);
                c_structure(o)->references = c_arrayNew(type,length);
                while ((member = c_iterTakeFirst(refList)) != NULL) {
                    c_structure(o)->references[i++] = c_keep(member);
                }
                c_iterFree(refList);
                c_free(type);
            }
        }
        if (!c_isFinal(o)) {
            size = alignSize(size,alignment);
            c_typeInit(o,alignment,size);
        }
    }
    break;
    case M_UNION:
    {
        c_unionCase unCase;
        c_type type;
        c_iter refList;
        c_address length;

        /*
         * A union has the following C syntax:
         * struct union {
         *   <discriminant> _d;
         *   <the union>    _u;
         * };
         *
         * So a union has a discriminant '_d' and the c-union '_u'.
         * The size of this union is determined by the maximum size of
         * the members of '_u' + the size of the descriminant '_d'. We
         * also need to take into account the alignment of '_u', since
         * we might need to add padding bytes between '_d' and '_u'. Finally
         * we need to align the entire struct.
         *
         * This results in the following algorithm:
         * 1. determine size of '_u', which is equal to the largest member
         * 2. determine alignment which is equal to the maximum of the
         *    alignment of '_d' and '_u'. The alignment of '_u' is equal
         *    to the alignment of the member with the largest alignment
         * 3. The total size of the union is equal to:
         *    sizeof('_d') + sizeof ('_d') + alignSize('_u') + alignSize('struct union')
         *    where the alignSize-function calculates the amount of padding
         *    for the identified member/type.
         */
        alignment = c_union(o)->switchType->alignment;
        size = 0;
        refList = NULL;
        if (c_union(o)->cases != NULL) {
            length = c_arraySize(c_union(o)->cases);
            for (i=0; i<length; i++) {
                unCase = c_unionCase(c_union(o)->cases[i]);
                type = c_specifier(unCase)->type;
                if (c_typeHasRef(type)) {
                    refList = c_iterInsert(refList,unCase);
                }
                if (!c_isFinal(o)) {
                    switch(c_baseObjectKind(type)) {
                    case M_INTERFACE:
                    case M_CLASS:
                    case M_ANNOTATION:
                    case M_BASE:
                        if (type->alignment > alignment) {
                            alignment = type->alignment;
                        }
                        if (objectAlignment > size) {
                            size = objectAlignment;
                        }
                    break;
                    default:
                        if (type->alignment > alignment) {
                            alignment = type->alignment;
                        }
                        ps = propertySize(type);
                        if (ps == 0) {
                            return S_ILLEGALRECURSION;
                        }
                        if (ps > size) {
                            size = ps;
                        }
                    break;
                    }
                }
            }
            /*size += c_union(o)->switchType->size + alignSize(c_union(o)->switchType->size, alignment);*/
            size += alignSize(c_union(o)->switchType->size, alignment);
            if (refList != NULL) {
                i=0;
                type = c_unionCase_t(c__getBase(o));
                c_union(o)->references = c_arrayNew(type,c_iterLength(refList));
                while ((unCase = c_iterTakeFirst(refList)) != NULL) {
                    c_union(o)->references[i++] = c_keep(unCase);
                }
                c_iterFree(refList);
                c_free(type);
            }
        }
        if (!c_isFinal(o)) {
            size = alignSize(size,alignment);
            c_typeInit(o,alignment,size);
        }
    }
    break;
    case M_COLLECTION:
        switch (c_collectionType(o)->kind) {
        case C_ARRAY:
            if (c_collectionType(o)->maxSize != 0) {
                ps = propertySize(c_collectionType(o)->subType);
                if (ps == 0) {
                    return S_ILLEGALRECURSION;
                }
                size = c_collectionType(o)->maxSize * ps;
                alignment = c_collectionType(o)->subType->alignment;
                c_typeInit(o,alignment,size);
            } else {
                _TYPEINIT_(o,void *);
            }
        break;
        default:
            _TYPEINIT_(o,void *);
        break;
        }
    break;
    case M_ENUMERATION:
    {
        c_constant c;
        c_result status = S_COUNT;
        c_address length;
        c_string metaName;

        length = c_arraySize(c_enumeration(o)->elements);
        for (i=0; i<length; i++) {
            /* This loop is added to avoid that an enumeration will affect
               existing constants. Existing constants originate from either
               a previous definition of the enumeration or from another
               definition. If originating from another definition then this
               must lead to an application notification of an incompatible
               type.
            */
            c = c_constant(c_enumeration(o)->elements[i]);
            if (c->type != NULL) {
                if (c_baseObjectKind(c_metaObject(c->type)) != M_ENUMERATION) {
                    metaName = c_metaName(c_metaObject(c));
                    OS_REPORT_1(OS_WARNING,"c_metaFinalize",0,
                                "Enum value <%s> already defined",
                                metaName);
                    c_free(metaName);
                    return S_REDEFINED;
                }
                if (status == S_COUNT) {
                    status = S_REDECLARED;
                }
                if (status != S_REDECLARED) {
                    return S_REDEFINED;
                }
            } else {
                if (status == S_COUNT) {
                    status = S_ACCEPTED;
                }
                if (status != S_ACCEPTED) {
                    return S_REDEFINED;
                }
            }
        }
        if (status != S_REDECLARED) {
            for (i=0; i<length; i++) {
                c = c_constant(c_enumeration(o)->elements[i]);
                c->type = c_keep(o); /* The constant has a managed ref. to type */
                if (c->operand == NULL) { /* typical in the odlpp */
                    c->operand = (c_operand)c_metaDefine(c_metaObject(o),
                                                         M_LITERAL);
                    c_literal(c->operand)->value.kind = V_LONG;
                    c_literal(c->operand)->value.is.Long = i;
                }
            }
            _TYPEINIT_(o,c_metaKind);
        }
    }
    break;
    case M_INTERFACE:
    case M_ANNOTATION:
    case M_CLASS:
    {
        c_property property;
        c_iter iter;
        c_type type;
        c_property *properties;
        c_address length,offset;
        c_iter refList;

        alignment = 0; size = 0;
        if (c_baseObjectKind(o) == M_CLASS) {
            if (c_class(o)->extends != NULL) {
                assert(c_isFinal(c_metaObject(c_class(o)->extends)));
                size = c_type(c_class(o)->extends)->size;
                assert(size > 0);
                alignment = c_type(c_class(o)->extends)->alignment;
                assert(alignment > 0);
            }
        }

        iter = c_iterNew(NULL);
        metaScopeWalk(c_metaObject(o), getPropertiesScopeWalkAction, iter);
        if (c_interface(o)->inherits != NULL) {
            length = c_arraySize(c_interface(o)->inherits);
            for (i=0; i<length; i++) {
                iter = c_iterInsert(iter,c_interface(o)->inherits[i]);
            }
        }

        refList = NULL;
        properties = NULL;
        length = c_iterLength(iter);
        if (length > 0) {
            properties = (c_property *)os_malloc(length*sizeof(c_property));
            if (normalize) {
                for (i=0;i<length;i++) {
                    properties[i] = c_iterTakeFirst(iter);
                }
            } else {
                for (i=length;i>0;i--) {
                    properties[i-1] = c_iterTakeFirst(iter);
                }
            }
        }
        c_iterFree(iter);

        if (properties != NULL) {
            if (!c_isFinal(o)) {
                if (normalize) {
                    qsort(properties,length,sizeof(void *),c_compareProperty);
                }
                for (i=0; i<length; i++) {
                    property = c_property(properties[i]);
                    offset = property->offset;
                    type = property->type;
                    if (c_typeHasRef(type)) {
                        refList = c_iterInsert(refList,property);
                    }
                    while(c_baseObjectKind(type) == M_TYPEDEF) {
                        type = c_typeDef(type)->alias;
                    }
                    switch(c_baseObjectKind(type)) {
                    case M_INTERFACE:
                    case M_ANNOTATION:
                    case M_CLASS:
                    case M_BASE:
                        if (alignment < objectAlignment) {
                            alignment = objectAlignment;
                        }
                        if (normalize) {
                            property->offset = alignSize(size,objectAlignment);
                        }
                        size = property->offset + objectAlignment;
                    break;
                    default:
                        if (property->type->alignment > alignment) {
                            alignment = property->type->alignment;
                        }
                        if (normalize) {
                            property->offset = alignSize(size,property->type->alignment);
                        }
                        ps = propertySize(property->type);
                        if (ps == 0) {
                            return S_ILLEGALRECURSION;
                        }
                        size = property->offset + ps;
                    break;
                    }
#ifndef NDEBUG
if ((offset != 0) && (offset != property->offset)) {
    c_string propertyMetaName = c_metaName(c_metaObject(property));
    c_string objectMetaName = c_metaName(o);
    /* apperantly the new order differs from the existing order */
    printf("property %s of type %s meta offset = "PA_ADDRFMT" but was "PA_ADDRFMT" \n",
            propertyMetaName,objectMetaName,(PA_ADDRCAST)property->offset,(PA_ADDRCAST)offset);
    c_free(propertyMetaName);
    c_free(objectMetaName);
}
#endif

                }
            }
            os_free(properties);
        }

        if (!c_isFinal(o)) {
            size = alignSize(size,alignment);
            c_typeInit(o,alignment,size);
        }

        if (refList != NULL) {
            i=0;
            type = c_property_t(c__getBase(o));
            c_interface(o)->references = c_arrayNew(type,c_iterLength(refList));
            while ((property = c_iterTakeFirst(refList)) != NULL) {
                c_interface(o)->references[i++] = c_keep(property);
            }
            c_iterFree(refList);
            c_free(type);
        }
    }
    break;
    case M_TYPEDEF:
        c_typeInit(o,c_type(c_typeDef(o)->alias)->alignment,
                     c_type(c_typeDef(o)->alias)->size);
    break;
    default:
    break;
    }
    return S_ACCEPTED;

#undef _TYPEINIT_
}

c_result
c_metaFinalize(
    c_metaObject o) {
    return c__metaFinalize(o,TRUE);
}

c_bool
c_isFinal(
    c_metaObject o)
{
    switch(c_baseObjectKind(o)) {
    case M_CLASS:
    case M_COLLECTION:
    case M_ENUMERATION:
    case M_EXCEPTION:
    case M_INTERFACE:
    case M_ANNOTATION:
    case M_PRIMITIVE:
    case M_STRUCTURE:
    case M_TYPEDEF:
    case M_UNION:
        if (c_type(o)->alignment != 0) return TRUE; /* what about c_object ??? */
    break;
    default:
    break;
    }
    return FALSE;
}

c_metaObject
c_metaModule(
    c_metaObject object)
{
    c_metaObject scope;

    if (object == NULL) {
        return NULL;
    }
    scope = object->definedIn;
    while ((scope != NULL) && (c_baseObjectKind(scope) != M_MODULE)) {
        scope = scope->definedIn;
    }
    return c_keep(scope);
}

c_bool
c_objectIs(
    c_baseObject object,
    c_metaKind kind)
{
    if (object->kind == kind) {
        return TRUE;
    }
    return FALSE;
}

c_bool
c_objectIsType(
    c_baseObject o)
{
    if (o == NULL) {
        return FALSE;
    }
    switch(o->kind) {
    case M_TYPEDEF:
    case M_CLASS:
    case M_COLLECTION:
    case M_ENUMERATION:
    case M_EXCEPTION:
    case M_INTERFACE:
    case M_ANNOTATION:
    case M_PRIMITIVE:
    case M_STRUCTURE:
    case M_UNION:
    case M_BASE:     /* meta types are also types */
        return TRUE;
    default:
        return FALSE;
    }
}

c_string
c_metaName(
    c_metaObject o)
{
    if (o == NULL) {
        return NULL;
    }
    switch(c_baseObjectKind(o)) {
    case M_PARAMETER:
    case M_MEMBER:
    case M_UNIONCASE:
        return c_keep(c_specifier(o)->name);
    case M_LITERAL:
    case M_CONSTOPERAND:
    case M_EXPRESSION:
        return NULL;
    default:
        return c_keep(o->name);
    }
}

c_long
c_metaNameLength(
    c_metaObject o)
{
    c_char *name;
    c_long length;

    name = c_metaName(o);
    if (name == NULL) {
        length = 0;
    } else {
        length = strlen(name);
        c_free(name);
    }
    return length;
}

c_char *
c_metaScopedName(
    c_metaObject o)
{
    c_char *scopedName, *ptr;
    c_long length;
    c_iter path = NULL;
    c_metaObject scope,previous;
    c_char *name;

    if (o == NULL) {
        return NULL;
    }

    length = 1; /* \0 */
    scope = o;
    while (scope != NULL) {
        path = c_iterInsert(path,scope);
        length += c_metaNameLength(scope);
        previous = scope->definedIn;
        if (previous != NULL) {
            switch (c_baseObjectKind(scope)) {
            case M_ATTRIBUTE:
            case M_MEMBER:
            case M_RELATION:
            case M_UNIONCASE:
                length += 1; /* .  */
            break;
            default:
                length += 2; /* :: */
            break;
            }
        }
        scope = scope->definedIn;
    }
    scopedName = os_malloc(length);
    ptr = scopedName;
    previous = NULL;
    while ((scope = c_iterTakeFirst(path)) != NULL) {
        length = c_metaNameLength(scope);
        if (length != 0) {
            if (previous != NULL) {
                switch (c_baseObjectKind(scope)) {
                case M_ATTRIBUTE:
                case M_MEMBER:
                case M_RELATION:
                case M_UNIONCASE:
                    os_sprintf(ptr,".");
                    ptr = C_DISPLACE(ptr,1);
                break;
                default:
                    os_sprintf(ptr,"::");
                    ptr = C_DISPLACE(ptr,2);
                break;
                }
            }
            name = c_metaName(scope);
            os_strncpy(ptr,name,length);
            c_free(name);
            ptr = C_DISPLACE(ptr,length);
            previous = scope;
        }
    }
    c_iterFree(path);
    *ptr = 0;
    return scopedName;
}

c_literal
c_expressionValue(
    c_expression expr)
{
    c_literal left, right, result;
    c_metaObject scope;

    left = NULL; right = NULL;
    if (c_arraySize(expr->operands) > 0) {
        left = c_operandValue(c_operand(expr->operands[0]));
    }
    if (c_arraySize(expr->operands) > 1) {
        right = c_operandValue(c_operand(expr->operands[1]));
    }

    scope = c_metaObject(c__getBase(expr));
    result = (c_literal)c_metaDefine(scope,M_LITERAL);

#define _CASE_(l,o) \
    case l: result->value = c_valueCalculate(left->value,right->value,o); \
    break

    switch(expr->kind) {
    _CASE_(E_OR,O_LOR);
    _CASE_(E_XOR,O_LXOR);
    _CASE_(E_AND,O_LAND);
    _CASE_(E_SHIFTRIGHT,O_RIGHT);
    _CASE_(E_SHIFTLEFT,O_LEFT);
    _CASE_(E_MUL,O_MUL);
    _CASE_(E_DIV,O_DIV);
    _CASE_(E_MOD,O_MOD);
    case E_PLUS:
        if (c_arraySize(expr->operands) > 1) {
            result->value = c_valueCalculate(left->value,right->value,O_ADD);
        } else {
            result->value = left->value;
        }
    break;
    case E_MINUS:
        if (c_arraySize(expr->operands) > 1) {
            result->value = c_valueCalculate(left->value,right->value,O_SUB);
        } else {
#define _MINUS_(d,s,t) d.is.t = -s.is.t
            result->value.kind = left->value.kind;
            switch (left->value.kind) {
            case V_OCTET: _MINUS_(result->value,left->value,Octet); break;
            case V_SHORT: _MINUS_(result->value,left->value,Short); break;
            case V_LONG: _MINUS_(result->value,left->value,Long); break;
            case V_LONGLONG: _MINUS_(result->value,left->value,LongLong); break;
            case V_FLOAT: _MINUS_(result->value,left->value,Float); break;
            case V_DOUBLE: _MINUS_(result->value,left->value,Double); break;
            default : assert(FALSE);
            }
#undef _MINUS_
        }
    break;
    case E_NOT:
#define _NOT_(d,s,t) d.is.t = ~s.is.t
        result->value.kind = left->value.kind;
        switch (left->value.kind) {
        case V_OCTET: _NOT_(result->value,left->value,Octet); break;
        case V_SHORT: _NOT_(result->value,left->value,Short); break;
        case V_USHORT: _NOT_(result->value,left->value,UShort); break;
        case V_LONG: _NOT_(result->value,left->value,Long); break;
        case V_ULONG: _NOT_(result->value,left->value,ULong); break;
        case V_LONGLONG: _NOT_(result->value,left->value,LongLong); break;
        case V_ULONGLONG: _NOT_(result->value,left->value,ULongLong); break;
        default : assert(FALSE);
        }
#undef _NOT_
    break;
    default:
      assert(FALSE);
    }
#undef _CASE_
    c_free(left);
    c_free(right);
    return result;
}

c_literal
c_operandValue(
    c_operand operand)
{
    switch (c_baseObjectKind(operand)) {
    case M_CONSTOPERAND:
        return c_operandValue(c_constOperand(operand)->constant->operand);
    case M_EXPRESSION:
        return c_expressionValue(c_expression(operand));
    case M_LITERAL:
        return c_keep(c_literal(operand));
    case M_CONSTANT:
        return c_operandValue(c_constant(operand)->operand);
    default: assert(FALSE);
    }
    return NULL;
}

c_literal
c_enumValue (
    c_enumeration e,
    const c_char *label)
{
    c_long i,length;
    c_string metaName;

    if (e == NULL) {
        return NULL;
    }
    if (label == NULL) {
        return NULL;
    }
    length = c_arraySize(e->elements);
    for (i=0;i<length;i++) {
        metaName = c_metaName(c_metaObject(e->elements[i]));
        if (strcmp(label,metaName) == 0) {
            c_free(metaName);
            return c_operandValue(c_operand(e->elements[i]));
        }
        c_free(metaName);
    }
    return NULL;
}

int
c_alignment(
    c_baseObject o)
{
    if (o == NULL) {
        return 0;
    }
    switch(o->kind) {
    case M_ATTRIBUTE:
    case M_RELATION:
        return c_alignment(c_baseObject(c_property(o)->type));
    case M_CONSTANT:
        return c_alignment(c_baseObject(c_constant(o)->type));
    case M_CONSTOPERAND:
        return c_alignment(c_baseObject(c_constOperand(o)->constant));
    case M_TYPEDEF:
        return c_alignment(c_baseObject(c_typeDef(o)->alias));
    case M_MEMBER:
    case M_PARAMETER:
    case M_UNIONCASE:
        return c_alignment(c_baseObject(c_specifier(o)->type));
    case M_CLASS:
    case M_COLLECTION:
    case M_ENUMERATION:
    case M_EXCEPTION:
    case M_INTERFACE:
    case M_ANNOTATION:
    case M_PRIMITIVE:
    case M_STRUCTURE:
    case M_UNION:
        return c_type(o)->alignment;
    case M_BASE:
    case M_EXPRESSION:
    case M_LITERAL:
    case M_MODULE:
    case M_OPERATION:
    default:
        return 0;
    }
}

int
c_getSize(
    c_baseObject o)
{
    if (o == NULL) {
        return 0;
    }
    switch(o->kind) {
    case M_ATTRIBUTE:
    case M_RELATION:
        return c_getSize(c_baseObject(c_property(o)->type));
    case M_CONSTANT:
        return c_getSize(c_baseObject(c_constant(o)->type));
    case M_CONSTOPERAND:
        return c_getSize(c_baseObject(c_constOperand(o)->constant));
    case M_TYPEDEF:
        return c_getSize(c_baseObject(c_typeDef(o)->alias));
    case M_MEMBER:
    case M_PARAMETER:
    case M_UNIONCASE:
        return c_getSize(c_baseObject(c_specifier(o)->type));
    case M_CLASS:
    case M_COLLECTION:
    case M_ENUMERATION:
    case M_EXCEPTION:
    case M_INTERFACE:
    case M_ANNOTATION:
    case M_PRIMITIVE:
    case M_STRUCTURE:
    case M_UNION:
        return c_type(o)->size;
    case M_BASE:
    case M_EXPRESSION:
    case M_LITERAL:
    case M_MODULE:
    case M_OPERATION:
    default:
        return 0;
    }
}

c_valueKind
c_metaValueKind(
    c_metaObject o)
{
    switch (c_baseObjectKind(o)) {
    case M_PRIMITIVE:
        switch (c_primitive(o)->kind) {
        case P_ADDRESS:   return V_ADDRESS;
        case P_BOOLEAN:   return V_BOOLEAN;
        case P_CHAR:      return V_CHAR;
        case P_WCHAR:     return V_WCHAR;
        case P_OCTET:     return V_OCTET;
        case P_SHORT:     return V_SHORT;
        case P_USHORT:    return V_USHORT;
        case P_LONG:      return V_LONG;
        case P_ULONG:     return V_ULONG;
        case P_LONGLONG:  return V_LONGLONG;
        case P_ULONGLONG: return V_ULONGLONG;
        case P_FLOAT:     return V_FLOAT;
        case P_DOUBLE:    return V_DOUBLE;
        case P_VOIDP:     return V_VOIDP;
        default:          assert(FALSE);
        break;
        }
    break;
    case M_ENUMERATION:
        return V_LONG;
    case M_ATTRIBUTE:
    case M_RELATION:
        return c_metaValueKind(c_metaObject(c_property(o)->type));
    case M_MEMBER:
        return c_metaValueKind(c_metaObject(c_specifier(o)->type));
    case M_TYPEDEF:
        return c_metaValueKind(c_metaObject(c_typeDef(o)->alias));
    case M_CONSTANT:
        return c_metaValueKind(c_metaObject(c_constant(o)->type));
    case M_CONSTOPERAND:
        return c_metaValueKind(c_metaObject(c_constOperand(o)->constant));
    case M_LITERAL:
        return c_literal(o)->value.kind;
    case M_OPERATION:
        return c_metaValueKind(c_metaObject(c_operation(o)->result));
    case M_PARAMETER:
    case M_UNIONCASE:
        return c_metaValueKind(c_metaObject(c_specifier(o)->type));
    case M_COLLECTION:
        if (c_collectionType(o)->kind == C_STRING) return V_STRING;
    case M_CLASS:
    case M_INTERFACE:
    case M_ANNOTATION:
    case M_MODULE:
    case M_BASE:
    case M_STRUCTURE:
    case M_EXCEPTION:
    case M_UNION:
        return V_OBJECT;

    case M_EXPRESSION:
    case M_UNDEFINED:
    default:
        assert(FALSE);
    break;
    }
    return V_UNDEFINED;
}

void
c_metaAttributeNew(
    c_metaObject scope,
    const c_char *name,
    c_type type,
    c_address offset)
{
    c_metaObject o,found;

    o = c_metaObject(c_metaDefine(scope,M_ATTRIBUTE));
    c_property(o)->type = c_type(c_keep(type));
    c_property(o)->offset = offset;
    c_attribute(o)->isReadOnly = TRUE;
    found = c_metaBind(scope,name,o);
    assert(found == o);
    c_free(found);
    c_free(o);
}

c_type
c_metaArrayTypeNew(
    c_metaObject scope,
    const c_char *name,
    c_type subType,
    c_long maxSize)
{
    c_metaObject _this = NULL;
    c_metaObject found;

    if (name) {
        _this = c_metaResolve(scope, name);
    }
    if (_this == NULL) {
        _this = c_metaDefine(c_metaObject(c__getBase(scope)),M_COLLECTION);
        c_collectionType(_this)->kind = C_ARRAY;
        c_collectionType(_this)->subType = c_keep(subType);
        c_collectionType(_this)->maxSize = maxSize;
        c_metaFinalize(_this);
        if (name) {
            found = c_metaBind(scope, name, _this);
            assert(found != NULL);
            c_free(_this);
            if (found != _this) {
                _this = found;
            }
        }
    }
    return c_type(_this);
}

c_type
c_metaSequenceTypeNew(
    c_metaObject scope,
    const c_char *name,
    c_type subType,
    c_long maxSize)
{
    c_metaObject _this = NULL;
    c_metaObject found;

    if (name) {
        _this = c_metaResolve(scope, name);
    }
    if (_this == NULL) {
        _this = c_metaDefine(c_metaObject(c__getBase(scope)),M_COLLECTION);
        c_collectionType(_this)->kind = C_SEQUENCE;
        c_collectionType(_this)->subType = c_keep(subType);
        c_collectionType(_this)->maxSize = maxSize;
        c_metaFinalize(_this);
        if (name) {
            found = c_metaBind(scope, name, _this);
            assert(found != NULL);
            c_free(_this);
            if (found != _this) {
                _this = found;
            }
        }
    }
    return c_type(_this);
}

void
c_metaTypeInit(
    c_object o,
    c_long size,
    c_long alignment)
{
    c_type(o)->base = c__getBase(o);
    c_type(o)->alignment = alignment;
    c_type(o)->size = size;
}

static void
c_specifierCopy(
    c_specifier s,
    c_specifier d)
{
    d->name = c_keep(s->name);
    d->type = c_keep(s->type);
}

static void
c_typeCopy(
    c_type s,
    c_type d)
{
    d->alignment = s->alignment;
    d->base = c_keep(s->base);
    d->size = s->size;
}

static void
copyScopeObject(
    c_metaObject o,
    c_metaObject scope)
{
    c_metaObject found;

    found = metaScopeInsert(scope,o);
    assert(found == o);
    c_free(found);
}

static void
copyScopeObjectScopeWalkAction(
    c_metaObject o,
    c_scopeWalkActionArg arg /* c_metaObject */)
{
    copyScopeObject(o, arg);
}

void
c_metaCopy(
    c_metaObject s,
    c_metaObject d)
{
    if (c_baseObjectKind(s) != c_baseObjectKind(d)) {
        return;
    }
    switch(c_baseObjectKind(s)) {
    case M_CLASS:
        c_class(d)->extends = c_keep(c_class(s)->extends);
        c_class(d)->keys = c_keep(c_class(s)->keys);
    case M_ANNOTATION:
    case M_INTERFACE:
        c_interface(d)->abstract = c_interface(s)->abstract;
        c_interface(d)->inherits = c_keep(c_interface(s)->inherits);
        c_interface(d)->references = c_keep(c_interface(s)->references);
        metaScopeWalk(s, copyScopeObjectScopeWalkAction, d);
        c_typeCopy(c_type(s),c_type(d));
    break;
    case M_COLLECTION:
        c_collectionType(d)->kind = c_collectionType(s)->kind;
        c_collectionType(d)->maxSize = c_collectionType(s)->maxSize;
        c_collectionType(d)->subType = c_keep(c_collectionType(s)->subType);
        c_typeCopy(c_type(s),c_type(d));
    break;
    case M_ATTRIBUTE:
        c_attribute(d)->isReadOnly = c_attribute(s)->isReadOnly;
        c_specifierCopy(c_specifier(s),c_specifier(d));
    break;
    case M_ENUMERATION:
        c_enumeration(d)->elements = c_keep(c_enumeration(s)->elements);
        c_typeCopy(c_type(s),c_type(d));
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
        c_structure(d)->members = c_keep(c_structure(s)->members);
        c_structure(d)->references = c_keep(c_structure(s)->references);
        metaScopeWalk(s, copyScopeObjectScopeWalkAction, d);
    break;
    case M_MODULE:
        metaScopeWalk(s, copyScopeObjectScopeWalkAction, d);
    break;
    case M_PRIMITIVE:
        c_primitive(d)->kind = c_primitive(s)->kind;
        c_typeCopy(c_type(s),c_type(d));
    break;
    case M_TYPEDEF:
        c_typeDef(d)->alias = c_keep(c_typeDef(s)->alias);
        c_typeCopy(c_type(s),c_type(d));
    break;
    case M_UNION:
        c_union(d)->cases = c_keep(c_union(s)->cases);
        c_union(d)->references = c_keep(c_union(s)->references);
        c_union(d)->switchType = c_keep(c_union(s)->switchType);
        c_typeCopy(c_type(s),c_type(d));
    break;
    case M_OPERATION:
        c_operation(d)->parameters = c_keep(c_operation(s)->parameters);
        c_operation(d)->result = c_keep(c_operation(s)->result);
    break;
    case M_PARAMETER:
        c_parameter(d)->mode = c_parameter(s)->mode;
    break;
    case M_CONSTANT:
        c_constant(d)->operand = c_keep(c_constant(s)->operand);
        c_constant(d)->type = c_keep(c_constant(s)->type);
    break;
    default:
    break;
    }
}

void
c_metaPrint(
    c_metaObject o,
    c_scopeWalkActionArg unused)
{
    c_string name,extends;

    OS_UNUSED_ARG(unused);

    name = c_metaName(o);
    if (name == NULL) {
        name = "<anonomous>";
    }
    switch (c_baseObjectKind(o)) {
    case M_CLASS:
        if (c_class(o)->extends != NULL) {
            extends = c_metaName(c_metaObject(c_class(o)->extends));
            if (extends == NULL) {
                extends = "<anonomous>";
            }
            c_metaPrint(c_metaObject(c_class(o)->extends), NULL);
            printf("class %s extends %s {\n",name,extends);
        } else {
            printf("class %s {\n",name);
        }
        c_scopeWalk(c_interface(o)->scope, c_metaPrint, NULL);
        printf("};\n\n");
    break;
    case M_ANNOTATION:
    case M_INTERFACE:
    break;
    case M_STRUCTURE:
    case M_EXCEPTION:
    case M_UNION:
    case M_PRIMITIVE:
    case M_TYPEDEF:
    case M_OPERATION:
    case M_CONSTANT:
    case M_MODULE:
    case M_RELATION:
    case M_ATTRIBUTE:
        printf("    attribute %s %s; ("PA_ADDRFMT")\n",
               c_metaName(c_metaObject(c_property(o)->type)),
               name, (PA_ADDRCAST)c_property(o)->offset);
    case M_UNIONCASE:
    case M_MEMBER:
    case M_PARAMETER:
    case M_EXPRESSION:
    case M_CONSTOPERAND:
    case M_LITERAL:
    default:
    break;
    }
}

#define metaNameCompare(scope,name,filter) \
        _metaNameCompare (c_baseObject(scope),name,filter)

static c_metaEquality
_metaNameCompare (
    c_baseObject baseObject,
    const char *name,
    c_long metaFilter)
{
    c_metaEquality equality = E_UNEQUAL;
    char *objName;

    if (baseObject != NULL) {
        if (CQ_KIND_IN_MASK (baseObject, metaFilter)) {
            if (CQ_KIND_IN_MASK (baseObject, CQ_SPECIFIERS)) {
                objName = c_specifier(baseObject)->name;
            } else if (CQ_KIND_IN_MASK (baseObject, CQ_METAOBJECTS)) {
                objName = c_metaObject(baseObject)->name;
            } else {
                return equality;
            }
            if (metaFilter & CQ_CASEINSENSITIVE) {
                if (os_strcasecmp (objName, name) == 0) {
                    equality = E_EQUAL;
                }
            } else {
                if (strcmp (objName, name) == 0) {
                    equality = E_EQUAL;
                }
            }
        }
    }
    return equality;
}

#define metaResolveName(scope,name,filter) \
        _metaResolveName(c_metaObject(scope),name,filter)

static c_baseObject
_metaResolveName(
    c_metaObject scope,
    const char *name,
    c_long metaFilter)
{
    c_baseObject o;
    c_specifier sp;
    c_unionCase uc;
    c_long i,length;

    if (scope == NULL) {
        return NULL;
    }
    switch (c_baseObjectKind(scope)) {
    case M_EXCEPTION:
    case M_STRUCTURE:
        if (metaFilter & CQ_SPECIFIERS) {
            length = c_arraySize(c_structure(scope)->members);
            for (i=0; i<length; i++) {
                sp = c_specifier(c_structure(scope)->members[i]);
                if (metaNameCompare(sp,name,metaFilter) == E_EQUAL) {
                    return c_keep(c_structure(scope)->members[i]);
                }
            }
        }
        return c_baseObject(metaScopeLookup(scope,name,metaFilter));
    case M_UNION:
        if (metaFilter & CQ_SPECIFIERS) {
            length = c_arraySize(c_union(scope)->cases);
            for (i=0; i<length; i++) {
                uc = c_unionCase(c_union(scope)->cases[i]);
                if (metaNameCompare(uc, name, metaFilter) == E_EQUAL) {
                    return c_keep(c_union(scope)->cases[i]);
                }
            }
        }
        return c_baseObject(metaScopeLookup(scope,name,metaFilter));
    case M_MODULE:
    case M_INTERFACE:
    case M_ANNOTATION:
    case M_CLASS:
        o = c_baseObject(metaScopeLookup(scope,name,metaFilter));
        if (o != NULL) {
            return o;
        } else if (c_baseObjectKind(scope) == M_CLASS) {
            return metaResolveName(c_class(scope)->extends,name,metaFilter);
        }
    break;
    case M_TYPEDEF:
        return metaResolveName(c_typeDef(scope)->alias,name,metaFilter);
    default:
    break;
    }
    return NULL;
}

c_baseObject
c_metaFindByComp (
    c_metaObject scope,
    const char *name,
    c_long metaFilter)
{
    c_baseObject o = NULL;
    const c_char *head,*tail;
    c_char *str;
    c_long length;
    c_state state;
    c_token tok;

    str = NULL;
    tail = name;
    state = ST_START;

    while ((state != ST_END) && (state != ST_ERROR)) {
        head = c_skipSpaces(tail);
        tail = head;
        tok = c_getToken(&tail);
        switch (tok) {
        case TK_DOT:
        case TK_REF:
            switch (state) {
            case ST_IDENT:
            case ST_REFID:
                switch (o->kind) {
                case M_STRUCTURE:
                case M_EXCEPTION:
                case M_INTERFACE:
                case M_CLASS:
                case M_ANNOTATION:
                    scope = c_metaObject(o);
                    state = ST_REF;
                break;
                case M_ATTRIBUTE:
                case M_RELATION:
                    scope = c_metaObject(c_property(o)->type);
                    state = ST_REF;
                break;
                case M_MEMBER:
                    scope = c_metaObject(c_specifier(o)->type);
                    state = ST_REF;
                break;
                default:
                    state = ST_ERROR;
                }
                c_free(o);
            break;
            default:
                state = ST_ERROR;
            break;
            }
        break;
        case TK_SCOPE:
        case TK_SLASH:
            switch (state) {
            case ST_START:
                scope = c_metaObject(c__getBase(scope));
                state = ST_SCOPE;
            break;
            case ST_IDENT:
                if (o->kind == M_MODULE ||
                    o->kind == M_STRUCTURE ||
                    o->kind == M_UNION) {
                    scope = c_metaObject(o);
                    state = ST_SCOPE;
                } else {
                    state = ST_ERROR;
                }
            break;
            default:
                state = ST_ERROR;
            break;
            }
        break;
        case TK_IDENT:
            length = abs((c_address)tail - (c_address)head)+1;
            str = (char *)os_malloc(length);
            os_strncpy(str,head,length);
            str[length-1]=0;

            switch (state) {
            case ST_START:
                o = NULL;
                while ((scope != NULL) && (o == NULL)) {
                    if ((metaFilter & CQ_FIXEDSCOPE) == 0) {
                        o = metaResolveName(scope,str,metaFilter);
                        scope = scope->definedIn;
                    } else {
                        o = metaResolveName(scope,str,metaFilter);
                        scope = NULL;
                    }
                }
                if (o == NULL) {
                    state = ST_ERROR;
                } else {
                    state = ST_IDENT;
                }
            break;
            case ST_SCOPE:
                o = metaResolveName(scope,str,metaFilter);
                if (o == NULL) {
                    state = ST_ERROR;
                } else {
                    state = ST_IDENT;
                }
            break;
            case ST_REF:
                o = metaResolveName(scope,str,metaFilter);
                if (o == NULL) {
                    state = ST_ERROR;
                } else {
                    state = ST_REFID;
                }
            break;
            default:
                state = ST_ERROR;
            break;
            }
            os_free(str);
        break;
        case TK_END:
            switch (state) {
            case ST_IDENT:
            case ST_REFID:
                state = ST_END;
            break;
            default:
                state = ST_ERROR;
            }
        break;
        default:
            state = ST_ERROR;
        break;
        }
    }
    if (state == ST_ERROR) return NULL;

    return o;
}

c_baseObject
c_metaFindByName (
    c_metaObject scope,
    const char *name,
    c_long metaFilter)
{
    return c_metaFindByComp (scope, name, metaFilter);
}

c_metaObject
c_metaResolveType (
    c_metaObject scope,
    const os_char *name)
{
    return c_metaObject(c_metaFindByComp (scope,
                                          name,
                                          CQ_METAOBJECTS));
}

c_specifier
c_metaResolveSpecifier (
    c_metaObject scope,
    const os_char *name)
{
    return c_specifier(c_metaFindByComp (scope,
                                          name,
                                          CQ_SPECIFIERS));
}

c_metaObject
c_metaResolve (
    c_metaObject scope,
    const os_char *name)
{
    return c_metaObject(c_metaFindByComp (scope,
                                          name,
                                          CQ_ALL));
}

c_metaObject
c_metaResolveFixedScope (
    c_metaObject scope,
    const os_char *name)
{
    return c_metaObject(c_metaFindByComp (scope,
                                          name,
                                          CQ_ALL | CQ_FIXEDSCOPE));
}

/*
 * Returns TRUE if an instance of the given type will be a c_baseObject.
 */
c_bool
c_isBaseObjectType(
        c_type type)
{
    c_base base;
    int i = 0;

    assert(type);

    base = type->base;

    for(i = M_ATTRIBUTE; i < M_COUNT; i++){
        assert(base->metaType[i]);
        if(base->metaType[i] == type)
        {
            return TRUE;
        }
    }

    /* the following types are either abstract types or not used, so
     * assert that the type is non of these:
     *  - c_operand
     *  - c_specifier
     *  - c_metaObject
     *  - c_property
     *  - c_type
     *  - c_blob
     *  - c_fixed
     *  - c_valueType
     */

#define _ASSERT_NOT_TYPE_(t) assert(strcmp(c_metaObject(type)->name,  #t) != 0)

    _ASSERT_NOT_TYPE_(c_operand);
    _ASSERT_NOT_TYPE_(c_specifier);
    _ASSERT_NOT_TYPE_(c_metaObject);
    _ASSERT_NOT_TYPE_(c_property);
    _ASSERT_NOT_TYPE_(c_type);
    _ASSERT_NOT_TYPE_(c_blob);
    _ASSERT_NOT_TYPE_(c_fixed);
    _ASSERT_NOT_TYPE_(c_valueType);
#undef _ASSERT_NOT_TYPE_

    return FALSE;
}


/*
 * Returns TRUE is obj is a c_baseObject.
 */
c_bool
c_isBaseObject(c_object obj)
{
    if(obj) {
        return c_isBaseObjectType(c_getType(obj));
    }
    return FALSE;
}
