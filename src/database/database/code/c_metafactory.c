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
#include "c__base.h"
#include "c_misc.h"
#include "c_metafactory.h"
#include "c_collection.h"

struct c_declarator {
    c_string name;
    c_iter sizes;
};

c_string
c_declaratorName(
    c_declarator declaration)
{
    return declaration->name;
}

struct copyToArrayArg {
    c_array a;
    c_long index;
};

static void
copyToArray(
    c_object object,
    c_voidp arg)
{
    struct copyToArrayArg *a = (struct copyToArrayArg *)arg;

    a->a[a->index++] = object;
}

c_array
c_metaArray(
    c_metaObject scope,
    c_iter iter,
    c_metaKind kind)
{
    c_long i;
    struct copyToArrayArg arg;
    c_type type;

    if (kind) {
        /* suppress warnings */
    }
    i = c_iterLength(iter);
    if (i == 0) {
        arg.a = NULL;
    } else {
        type = c_object_t(c__getBase(scope));
        arg.a = c_arrayNew(type,i);
        arg.index = 0;
        c_iterWalk(iter,copyToArray,&arg);
        c_iterFree(iter);
        c_free(type);
    }
    return arg.a;
}

c_type
c_declaratorType(
    c_declarator declaration,
    c_type type)
{
    c_literal l;
    c_metaObject scope;
    c_collectionType o;

    if (declaration->sizes != NULL) {
        scope = c_metaObject(c__getBase(type));
        l = c_iterTakeFirst(declaration->sizes);
        while (l != NULL) {
            o = (c_collectionType)c_metaDefine(scope,M_COLLECTION);
            o->kind = C_ARRAY;
            if (l->value.kind == V_LONGLONG) {
                o->maxSize = (c_long)(l->value.is.LongLong);
            } else {
                o->maxSize = l->value.is.Long;
            }
            o->subType = type;
            c_metaFinalize(c_metaObject(o));
            type = (c_type)o;
            c_free(l);
            l = c_iterTakeFirst(declaration->sizes);
        }
    }
    return type;
}

c_declarator
c_declaratorNew(
    c_string name,
    c_iter sizes)
{
    c_declarator o = (c_declarator)os_malloc(sizeof(struct c_declarator));
    o->name = name;
    o->sizes = sizes;
    return o;
}

c_iter
c_bindTypes(
    c_metaObject scope,
    c_iter declarations,
    c_type type)
{
    c_iter typeDefs;
    c_declarator d;
    c_metaObject o;

    typeDefs = NULL;
    d = (c_declarator)c_iterTakeFirst(declarations);
    while (d != NULL) {
        o = c_metaDeclare(scope,d->name,M_TYPEDEF);
        c_typeDef(o)->alias = c_declaratorType(d,type);
        c_metaFinalize(o);
        typeDefs = c_iterInsert(typeDefs,o);
        os_free(d);
        d = (c_declarator)c_iterTakeFirst(declarations);
    }
    c_iterFree(declarations);
    return typeDefs;
}

c_iter
c_bindAttributes(
    c_metaObject scope,
    c_iter declarations,
    c_type type,
    c_bool isReadOnly)
{
    c_iter attributes;
    c_declarator d;
    c_metaObject o;

    if (isReadOnly) {
        /* suppress warnings */
    }
    attributes = NULL;
    d = (c_declarator)c_iterTakeFirst(declarations);
    while (d != NULL) {
        o = c_metaDefine(scope,M_ATTRIBUTE);
        c_property(o)->type = c_declaratorType(d,type);
        c_metaFinalize(o);
        c_metaBind(scope,d->name,o);
        attributes = c_iterInsert(attributes,o);
        os_free(d);
        d = (c_declarator)c_iterTakeFirst(declarations);
    }
    c_iterFree(declarations);
    return attributes;
}

c_iter
c_bindMembers(
    c_metaObject scope,
    c_iter declarations,
    c_type type)
{
    c_iter members;
    c_baseObject o;
    c_declarator d;

    members = NULL;
    d = c_iterTakeFirst(declarations);
    while (d != NULL) {
        o = c_metaDefine(scope,M_MEMBER);
        c_specifier(o)->name = d->name;
        c_specifier(o)->type = c_declaratorType(d,type);
        members = c_iterInsert(members,o);
        os_free(d);
        d = c_iterTakeFirst(declarations);
    }
    c_iterFree(declarations);
    return members;
}

c_constant
c_constantNew(
    c_metaObject scope,
    const c_char *name,
    c_value value)
{
    c_constant o;
    c_literal l;

    o = c_constant(c_metaDeclare(scope,name,M_CONSTANT));
    l = c_literal(c_metaDefine(scope,M_LITERAL));
    l->value = value;
    o->operand = c_operand(l);

    return o;
}

c_unionCase
c_unionCaseNew (
    c_metaObject scope,
    const c_char *name,
    c_type type,
    c_iter labels)
{
    c_unionCase o;
    c_long nrOfLabels;
    c_type subType;

    nrOfLabels = c_iterLength(labels);
    o = c_unionCase(c_metaDefine(scope,M_UNIONCASE));
    subType = c_type(c_metaResolve(scope,"c_literal"));
    o->labels = c_arrayNew(subType,nrOfLabels);
    c_free(subType);
    c_iterArray(labels,o->labels);
    c_specifier(o)->name = c_stringNew(c__getBase(scope),name);
    /* Do not keep type as usage expects transferral of refcount.
     * If changed then odlpp and idlpp must be adapted accordingly.
     */
    c_specifier(o)->type = type;
    return o;
}

