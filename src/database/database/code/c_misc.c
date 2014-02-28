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
#include "c_collection.h"
#include "os_report.h"
#include "c__metabase.h"

#define c_array(c) ((c_array)(c))

c_object
c_checkType (
    c_object o,
    const c_char *name)
{
    c_type type;
    c_string str;
    c_bool found = FALSE;
    c_bool stop = FALSE;

    if (o == NULL) {
        return NULL;
    }
    c_assertValidDatabaseObject(o);
    assert(c_refCount(o) > 0);
    assert(name != NULL);
    type = c__getType(o);
    while (!found && !stop) {
        str = c_metaObject(type)->name;
        if (str == NULL) {
            found = TRUE; /** assume TRUE **/
        } else if (strcmp(str,name) != 0) {
            switch (c_baseObject(type)->kind) {
            case M_CLASS:
                type = c_type(c_class(type)->extends);
                if (type == NULL) {
                    if ((strcmp(str,"c_base") == 0) && (strcmp(name,"c_module") == 0)) {
                        found = TRUE;
                    }
                    stop = TRUE;
                }
            break;
            case M_TYPEDEF:
                type = c_typeDef(type)->alias;
                if (type == NULL) {
                    stop = TRUE;
                }
            break;
            default:
              stop = TRUE;
            }
        } else {
            found = TRUE;
        }
    }
    if (!found) {
#ifndef NDEBUG
        if(o != NULL){
            str = c_metaObject(c__getType(o))->name;
            OS_REPORT_2(OS_ERROR, "Database", 0,
                    "Type mismatch: object type is %s but %s was expected\n",
                    str,name);
        }
#endif

        return NULL;
    }
    return o;
}


c_bool
c_instanceOf (
    c_object o,
    const c_char *name)
{
    c_type type;
    c_string str;
    c_bool found = FALSE;
    c_bool stop = FALSE;

    if (o != NULL) {
        c_assertValidDatabaseObject(o);
        assert(c_refCount(o) > 0);
        assert(name != NULL);
        type = c__getType(o);

        while (!found && !stop) {
            str = c_metaObject(type)->name;
            if (str == NULL) {
                found = TRUE; /** assume TRUE **/
            } else if (strcmp(str,name) != 0) {
                switch (c_baseObject(type)->kind) {
                case M_CLASS:
                    type = c_type(c_class(type)->extends);
                    if (type == NULL) {
                        if ((strcmp(str,"c_base") == 0) && (strcmp(name,"c_module") == 0)) {
                            found = TRUE;
                        }
                        stop = TRUE;
                    }
                break;
                case M_TYPEDEF:
                    type = c_typeDef(type)->alias;
                    if (type == NULL) {
                        stop = TRUE;
                    }
                break;
                default:
                  stop = TRUE;
                }
            } else {
                found = TRUE;
            }
        }
    }
    return found;
}

static void
copyReferences(
    c_type module,
    c_voidp dest,
    c_voidp data);

static void
copyStructReferences(
    c_structure m,
    c_voidp dest,
    c_voidp data)
{
    c_long i,length;
    c_member member;
    c_type type;
    c_object *ref;

    if (m->references == NULL) return;
    length = c_arraySize(m->references);
    for (i=0;i<length;i++) {
        member = c_member(m->references[i]);
        type = c_typeActualType(c_specifier(member)->type);
        switch (c_baseObject(type)->kind) {
        case M_CLASS:
        case M_INTERFACE:
        case M_ANNOTATION:
        case M_COLLECTION:
        case M_BASE:
            ref = C_DISPLACE(dest,member->offset);
            c_copyIn(type,C_REFGET(data,member->offset),ref);
        break;
        case M_EXCEPTION:
        case M_STRUCTURE:
        case M_UNION:
            copyReferences(type,C_DISPLACE(dest,member->offset),
                                C_DISPLACE(data,member->offset));
        break;
        default:
            assert(FALSE);
        break;
        }
    }
}

static void
copyInterfaceReferences(
    c_interface m,
    c_voidp dest,
    c_voidp data)
{
    c_long i,length;
    c_property property;
    c_type type;
    c_object *ref;

    if (m->references == NULL) {
        return;
    }
    length = c_arraySize(m->references);
    for (i=0;i<length;i++) {
        property = c_property(m->references[i]);
        type = c_typeActualType(property->type);
        switch (c_baseObject(type)->kind) {
        case M_CLASS:
        case M_INTERFACE:
        case M_ANNOTATION:
        case M_COLLECTION:
        case M_BASE:
            ref = C_DISPLACE(dest,property->offset);
            c_copyIn(type,C_REFGET(data,property->offset),ref);
        break;
        case M_EXCEPTION:
        case M_STRUCTURE:
        case M_UNION:
            copyReferences(type,C_DISPLACE(dest,property->offset),
                                C_DISPLACE(data,property->offset));
        break;
        default:
            assert(FALSE);
        break;
        }
    }
}

static void
copyReferences(
    c_type module,
    c_voidp dest,
    c_voidp data)
{
    switch (c_baseObject(module)->kind) {
    case M_STRUCTURE:
    case M_EXCEPTION:
        copyStructReferences(c_structure(module),dest,data);
    break;
    case M_CLASS:
        if (c_class(module)->extends != NULL) {
            copyReferences(c_type(c_class(module)->extends),dest,data);
        }
    case M_ANNOTATION:
    case M_INTERFACE:
        copyInterfaceReferences(c_interface(module),dest,data);
    break;
    case M_UNION:
    break;
    default:
    break;
    }

}

void
c_copyIn (
    c_type type,
    c_voidp data,
    c_voidp *dest)
{
    c_long size, subSize, i;
    c_type t, refType;

    if (data == NULL) {
        *dest = NULL;
        return;
    }
    t = c_typeActualType(type);
    if (c_baseObject(t)->kind == M_COLLECTION) {
        switch(c_collectionType(t)->kind) {
        case C_STRING:
            *dest = c_stringNew(c_getBase(t),data);
            return;
        case C_LIST:
        case C_BAG:
        case C_SET:
        case C_MAP:
        case C_DICTIONARY:
            OS_REPORT(OS_WARNING,"Database misc",0,
                      "c_copyIn: ODL collections unsupported");
        break;
        case C_ARRAY:
            refType = c_typeActualType(c_collectionType(type)->subType);
            subSize = refType->size;
            size = c_collectionType(t)->maxSize;
            if (size == 0) {
                size = c_arraySize(data);
                *dest = c_newArray(c_collectionType(t), size);
            }
            if (size > 0) {
                c_array ar = c_array(data);
                c_array destar = c_array(*dest);
                if (c_typeIsRef(refType)) {
                    for (i = 0; i < size; i++) {
                        copyReferences(refType, destar[i], ar[i]);
                    }
                } else {
                    memcpy(*dest, data, size * subSize);
                    /* Find indirections */
                    for (i = 0; i < size; i++) {
                        copyReferences(refType, C_DISPLACE(destar, (i*subSize)), C_DISPLACE(ar, (i*subSize)));
                    }
                }
            }
        break;
        case C_SEQUENCE:
            refType = c_typeActualType(c_collectionType(type)->subType);
            subSize = refType->size;
            size = c_sequenceSize(data);
            if (size > 0) {
                *dest = c_newSequence(c_collectionType(t), size);
                if (c_typeIsRef(refType)) {
                    c_sequence seq = c_sequence(data);
                    c_sequence destseq = c_sequence(*dest);
                    for (i = 0; i < size; i++) {
                        copyReferences(refType, destseq[i], seq[i]);
                    }
                } else {
                    memcpy(*dest, data, size * subSize);
                    /* Find indirections */
                    for (i = 0; i < size; i++) {
                        copyReferences(refType, C_DISPLACE(*dest, (i*subSize)), C_DISPLACE(data, (i*subSize)));
                    }
                }
            }
        break;
        default:
            OS_REPORT_1(OS_ERROR,"Database misc",0,
                        "c_copyIn: unknown collection kind (%d)",
                        c_collectionType(t)->kind);
            assert(FALSE);
        break;
        }
    } else if (c_typeIsRef(t)) {
        *dest = c_new(t);
        memcpy(*dest, data, t->size);
        copyReferences(t, *dest, data);
    } else {
        memcpy(*dest, data, t->size);
        copyReferences(t, *dest, data);
    }
}



static c_bool c__cloneReferences (c_type module, c_voidp data, c_voidp dest);

static c_bool
_cloneReference (
    c_type type,
    c_voidp data,
    c_voidp dest)
{
    c_type t = type;

    assert(data);
    assert(type);

    while (c_baseObject(t)->kind == M_TYPEDEF) {
        t = c_typeDef(t)->alias;
    }
    switch (c_baseObject(t)->kind) {
    case M_CLASS:
    case M_INTERFACE:
    case M_ANNOTATION:
        c_cloneIn(t, C_REFGET(data, 0), (c_voidp *) dest);
    break;
    case M_BASE:
    case M_COLLECTION:
        if ((c_collectionType(t)->kind == C_ARRAY) &&
            (c_collectionType(t)->maxSize != 0)) {
            c__cloneReferences(t, data, dest);
        } else {
            c_cloneIn(t, C_REFGET(data, 0), (c_voidp *) dest);
        }
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
    case M_UNION:
        c__cloneReferences(t, data, dest);
    break;
    default:
        OS_REPORT(OS_ERROR,
                  "cloneReference",0,
                  "illegal object detected");
        assert(FALSE);
        return FALSE;
    }
    return TRUE;
}



static c_bool
c__cloneReferences (
    c_type module,
    c_voidp data,
    c_voidp dest)
{
    c_type refType;
    c_class cls;
    c_array references, labels, ar, destar;
    c_sequence seq, destseq;
    c_property property;
    c_member member;
    c_long i,j,length,size;
    c_long nrOfRefs,nrOfLabs;
    c_value v;

    switch (c_baseObject(module)->kind) {
    case M_CLASS:
        cls = c_class(module);
        while (cls) {
            length = c_arraySize(c_interface(cls)->references);
            for (i=0;i<length;i++) {
                property = c_property(c_interface(cls)->references[i]);
                refType = property->type;
                _cloneReference(refType,
                        C_DISPLACE(data, property->offset),
                        C_DISPLACE(dest, property->offset));
            }
            cls = cls->extends;
        }
    break;
    case M_ANNOTATION:
    case M_INTERFACE:
        length = c_arraySize(c_interface(module)->references);
        for (i=0;i<length;i++) {
            property = c_property(c_interface(module)->references[i]);
            refType = property->type;
            _cloneReference(refType,
                    C_DISPLACE(data, property->offset),
                    C_DISPLACE(dest, property->offset));
        }
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
        length = c_arraySize(c_structure(module)->references);
        for (i=0;i<length;i++) {
            member = c_member(c_structure(module)->references[i]);
            refType = c_specifier(member)->type;
            _cloneReference(refType,
                    C_DISPLACE(data, member->offset),
                    C_DISPLACE(dest, member->offset));
        }
    break;
    case M_UNION:
#define _CASE_(k,t) case k: v = t##Value(*((t *)data)); break
        switch (c_metaValueKind(c_metaObject(c_union(module)->switchType))) {
        _CASE_(V_BOOLEAN,   c_bool);
        _CASE_(V_OCTET,     c_octet);
        _CASE_(V_SHORT,     c_short);
        _CASE_(V_LONG,      c_long);
        _CASE_(V_LONGLONG,  c_longlong);
        _CASE_(V_USHORT,    c_ushort);
        _CASE_(V_ULONG,     c_ulong);
        _CASE_(V_ULONGLONG, c_ulonglong);
        _CASE_(V_CHAR,      c_char);
        _CASE_(V_WCHAR,     c_wchar);
        default:
            OS_REPORT(OS_ERROR,
                      "c__cloneReferences",0,
                      "illegal union switch type detected");
            assert(FALSE);
            return FALSE;
        break;
        }
#undef _CASE_
        references = c_union(module)->references;
        if (references != NULL) {
            i=0; refType=NULL;
            nrOfRefs = c_arraySize(references);
            while ((i<nrOfRefs) && (refType == NULL)) {
                labels = c_unionCase(references[i])->labels;
                j=0;
                nrOfLabs = c_arraySize(labels);
                while ((j<nrOfLabs) && (refType == NULL)) {
                    if (c_valueCompare(v,c_literal(labels[j])->value) == C_EQ) {
                        c__cloneReferences(c_type(references[i]),
                                           C_DISPLACE(data, c_type(module)->alignment),
                                           C_DISPLACE(dest, c_type(module)->alignment));
                        refType = c_specifier(references[i])->type;
                    }
                    j++;
                }
                i++;
            }
        }
    break;
    case M_COLLECTION:
        refType = c_typeActualType(c_collectionType(module)->subType);
        switch (c_collectionType(module)->kind) {
        case C_ARRAY:
            ar = c_array(data);
            destar = c_array(dest);
            length = c_collectionType(module)->maxSize;
            if (length == 0) {
                length = c_arraySize(ar);
            }
            if (c_typeIsRef(refType)) {
                for (i=0;i<length;i++) {
                    c_cloneIn(refType, ar[i], &destar[i]);
                }
            } else {
                if (c_typeHasRef(refType)) {
                    size = refType->size;
                    for (i=0;i<length;i++) {
                        _cloneReference(refType, C_DISPLACE(data, (i*size)), C_DISPLACE(dest, (i*size)));
                    }
                }
            }
        break;
        case C_SEQUENCE:
            seq = c_sequence(data);
            destseq = c_sequence(dest);
            length = c_sequenceSize(seq);
            if (c_typeIsRef(refType)) {
                for (i=0;i<length;i++) {
                    c_cloneIn(refType, seq[i], &destseq[i]);
                }
            } else {
                if (c_typeHasRef(refType)) {
                    size = refType->size;
                    for (i=0;i<length;i++) {
                        _cloneReference(refType, C_DISPLACE(seq, (i*size)), C_DISPLACE(dest, (i*size)));
                    }
                }
            }
        break;
        default:
            OS_REPORT(OS_ERROR,
                  "c__cloneReferences",0,
                  "illegal collectionType found");
        break;
        }
    break;
    case M_BASE:
    break;
    case M_TYPEDEF:
        c__cloneReferences(c_type(c_typeDef(module)->alias), data, dest);
    break;
    case M_ATTRIBUTE:
    case M_RELATION:
        refType = c_typeActualType(c_property(module)->type);
        _cloneReference(refType,
                C_DISPLACE(data, c_property(module)->offset),
                C_DISPLACE(dest, c_property(module)->offset));
    break;
    case M_MEMBER:
        refType = c_typeActualType(c_specifier(module)->type);
        _cloneReference(refType,
                C_DISPLACE(data, c_member(module)->offset),
                C_DISPLACE(dest, c_member(module)->offset));
    break;
    case M_UNIONCASE:
        refType = c_typeActualType(c_specifier(module)->type);
        _cloneReference(refType, data, dest);
    break;
    case M_MODULE:
        /* Do nothing */
    break;
    case M_PRIMITIVE:
        /* Do nothing */
    break;
    default:
        OS_REPORT(OS_ERROR,
                  "c__cloneReferences",0,
                  "illegal meta object specified");
        assert(FALSE);
        return FALSE;
    }
    return TRUE;
}


void
c_cloneIn (
    c_type type,
    c_voidp data,
    c_voidp *dest)
{
    c_long size,subSize;
    c_type t;

    if (data == NULL) {
        *dest = NULL;
        return;
    }

    t = c_typeActualType(type);
    if (c_baseObject(t)->kind == M_COLLECTION) {
        switch(c_collectionType(t)->kind) {
        case C_STRING:
            *dest = c_stringNew(c_getBase(t), data);
            break;
        case C_LIST:
        case C_BAG:
        case C_SET:
        case C_MAP:
        case C_DICTIONARY:
            OS_REPORT(OS_WARNING,"Database misc",0,
                      "c_cloneIn: ODL collections unsupported");
        break;
        case C_ARRAY:
            subSize = c_collectionType(t)->subType->size;
            size = c_collectionType(t)->maxSize;
            if (size == 0) {
                size = c_arraySize(data);
                *dest = c_newArray(c_collectionType(t), size);
            }
            if (size > 0) {
                memcpy(*dest, data, size * subSize);
                /* Find indirections */
                c__cloneReferences(t, data, *dest);
            }
            break;
        case C_SEQUENCE:
            subSize = c_collectionType(t)->subType->size;
            size = c_sequenceSize(data);
            *dest = c_newSequence(c_collectionType(t), size);
            if (size > 0) {
                memcpy(*dest, data, size * subSize);
                /* Find indirections */
                c__cloneReferences(t, data, *dest);
            }
            break;
        break;
        default:
            OS_REPORT_1(OS_ERROR,"Database misc",0,
                        "c_cloneIn: unknown collection kind (%d)",
                        c_collectionType(t)->kind);
            assert(FALSE);
        break;
        }
    } else if (c_typeIsRef(t)) {
        *dest = c_new(t);
        memcpy(*dest, data, t->size);
        /* Find indirections */
        c__cloneReferences(t, data, *dest);
    } else {
        memcpy(*dest, data, t->size);
        /* Find indirections */
        c__cloneReferences(t, data, *dest);
    }
}


static void
extractReferences(
    c_type module,
    c_object srcObj,
    void *data);

static void
extractStructReferences(
    c_structure m,
    c_object o,
    void *data)
{
    c_long i,length;
    c_member member;
    c_type type;
    c_object *ref;

    if (m->references == NULL) return;
    length = c_arraySize(m->references);
    for (i=0;i<length;i++) {
        member = c_member(m->references[i]);
        type = c_typeActualType(c_specifier(member)->type);
        while (c_baseObject(type)->kind == M_TYPEDEF) {
            type = c_typeDef(type)->alias;
        }
        switch (c_baseObject(type)->kind) {
        case M_CLASS:
        case M_INTERFACE:
        case M_ANNOTATION:
        case M_COLLECTION:
        case M_BASE:
            ref = C_DISPLACE(data,member->offset);
            *ref = NULL;
            c_copyOut(type,C_REFGET(o,member->offset),ref);
        break;
        case M_EXCEPTION:
        case M_STRUCTURE:
        case M_UNION:
            copyReferences(type,C_DISPLACE(o,member->offset),
                                C_DISPLACE(data,member->offset))
;
        break;
        default:
            assert(FALSE);
        break;
        }
    }
}

static void
extractInterfaceReferences(
    c_interface m,
    c_object o,
    void *data)
{
    c_long i,length;
    c_property property;
    c_type type;
    c_object *ref;

    if (m->references == NULL) return;
    length = c_arraySize(m->references);
    for (i=0;i<length;i++) {
        property = c_property(m->references[i]);
        type = c_typeActualType(property->type);
        switch (c_baseObject(type)->kind) {
        case M_CLASS:
        case M_INTERFACE:
        case M_ANNOTATION:
        case M_COLLECTION:
        case M_BASE:
            ref = C_DISPLACE(data,property->offset);
            *ref = NULL;
            c_copyOut(type,C_REFGET(o,property->offset),ref);
        break;
        case M_EXCEPTION:
        case M_STRUCTURE:
        case M_UNION:
            extractReferences(type,C_DISPLACE(o,property->offset),
                                   C_DISPLACE(data,property->offset));
        break;
        default:
            assert(FALSE);
        break;
        }
    }
}

static void
extractReferences(
    c_type module,
    c_object srcObj,
    c_voidp data)
{
    switch (c_baseObject(module)->kind) {
    case M_STRUCTURE:
    case M_EXCEPTION:
        extractStructReferences(c_structure(module),srcObj,data);
    break;
    case M_CLASS:
        if (c_class(module)->extends != NULL) {
            extractReferences(c_type(c_class(module)->extends),srcObj,data);
        }
    case M_ANNOTATION:
    case M_INTERFACE:
        extractInterfaceReferences(c_interface(module),srcObj,data);
    break;
    case M_UNION:
    break;
    default:
    break;
    }

}

void
c_copyOut (
    c_type type,
    c_object o,
    c_voidp *data)
{
    c_long i,size;
    c_type t,subType;

    if (data == NULL) {
        OS_REPORT(OS_ERROR,"Database misc",0,
                  "c_copyOut: no destination specified");
        return;
    }
    if (o == NULL) {
        *data = NULL;
        return;
    }
    t = c_typeActualType(type);
    size = c_typeSize(t);
    if (size == 0) {
        OS_REPORT(OS_WARNING,"Database misc",0,
                  "c_copyOut: zero sized type specified");
        *data = NULL;
        return;
    }
    if (*data == NULL) {
        *data = (c_voidp)os_malloc(size);
    }
    if (c_baseObject(t)->kind == M_COLLECTION) {
        switch(c_collectionType(t)->kind) {
        case C_STRING:
            *data = os_strdup((c_char *)o);
        break;
        case C_LIST:
        case C_BAG:
        case C_SET:
        case C_MAP:
        case C_DICTIONARY:
            OS_REPORT(OS_WARNING,"Database misc",0,
                      "c_copyOut: ODL collections unsupported");
            assert(FALSE);
        break;
        case C_ARRAY:
            size = c_collectionType(t)->maxSize;
            if (size > 0) {
                subType = c_collectionType(t)->subType;
                for (i=0;i<size;i++) {
                    c_copyIn(subType,
                             ((c_voidp *)o)[i],
                             &((c_voidp *)(*data))[i]);
                }
            } else {
                OS_REPORT(OS_WARNING,"Database misc",0,
                          "c_copyOut: dynamic sized arrays unsupported");
            }
        case C_SEQUENCE:
            OS_REPORT(OS_WARNING,"Database misc",0,
                      "c_copyOut: sequences unsupported");
            assert(FALSE);
        break;
        default:
            OS_REPORT_1(OS_ERROR,"Database misc",0,
                        "c_copyOut: unknown collection kind (%d)",
                        c_collectionType(t)->kind);
            assert(FALSE);
        break;
        }
    } else if (c_typeIsRef(t)) {
        memcpy(*data,*(void**)o,size);
        extractReferences(t,*(void**)o,*data);
    } else {
        memcpy(*data,o,size);
        extractReferences(t,o,*data);
    }
}
