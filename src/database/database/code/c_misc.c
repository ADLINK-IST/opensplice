#include "os.h"
#include "c__base.h"
#include "c_collection.h"
#include "os_report.h"

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
            assert(FALSE);
        }
#endif        

        return NULL;
    }
    return o;
}

static void
copyReferences(
    c_type type,
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
    c_type type,
    c_voidp dest,
    c_voidp data)
{
    switch (c_baseObject(type)->kind) {
    case M_STRUCTURE:
    case M_EXCEPTION:
        copyStructReferences(c_structure(type),dest,data);
    break;
    case M_CLASS:
        if (c_class(type)->extends != NULL) {
            copyReferences(c_type(c_class(type)->extends),dest,data);
        }
    case M_INTERFACE:
        copyInterfaceReferences(c_interface(type),dest,data);
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
    c_long size,i;
    c_type t,subType;

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
            size = c_collectionType(t)->maxSize;
            if (size > 0) {
                subType = c_collectionType(t)->subType;
                for (i=0;i<size;i++) {
                    c_copyIn(subType,
                             ((c_voidp *)data)[i],
                             &((c_voidp *)(*dest))[i]);
                }
            } else {
                OS_REPORT(OS_WARNING,"Database misc",0,
                          "c_copyIn: dynamic sized arrays unsupported");
            }
        case C_SEQUENCE:
            OS_REPORT(OS_WARNING,"Database misc",0,
                      "c_copyIn: sequence unsupported");
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
        memcpy(*dest,data,t->size);
        copyReferences(t,*dest,data);
    } else {
        memcpy(dest,data,t->size);
        copyReferences(t,dest,data);
    }
}

static void
extractReferences(
    c_type type,
    c_object o,
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
    c_type type,
    c_object o,
    c_voidp data)
{
    switch (c_baseObject(type)->kind) {
    case M_STRUCTURE:
    case M_EXCEPTION:
        extractStructReferences(c_structure(type),o,data);
    break;
    case M_CLASS:
        if (c_class(type)->extends != NULL) {
            extractReferences(c_type(c_class(type)->extends),o,data);
        }
    case M_INTERFACE:
        extractInterfaceReferences(c_interface(type),o,data);
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
    size = c_typeSize(t,o);
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

