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

#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "os.h"
#include "c_misc.h"
#include "c_base.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "u_user.h"
#include "v_topic.h"
#include "v_partition.h"
#include "c__scope.h"

#define _METANAME(m) c_metaObject(m)->name
#define iprintf { int i; for (i=0; i<actionData->depth; i++) printf("    "); } printf

typedef struct toolActionData {
    FILE *fin;
    FILE *fout;
    c_long depth;
    c_iter stack;
    c_long size;
} *toolActionData;

static void
printType (
    c_type type,
    toolActionData actionData);


static void
printWalkHistory (
    c_object o,
    c_iterActionArg arg);

#define OBJECT_PUSH(actionData, obj) \
        (actionData)->stack = c_iterInsert((actionData)->stack, obj); \
        (actionData)->depth++

#define OBJECT_POP(actionData) \
        c_iterTakeFirst((actionData)->stack); \
        (actionData)->depth--

static c_type v_handle_t = NULL;
static c_type v_topic_t = NULL;
static c_type v_partition_t = NULL;

static void
printCollectionAction(
    c_metaObject mo,
    c_scopeWalkActionArg arg)
{
    c_type type;
    c_object o = c_metaObject (mo);
    toolActionData actionData = (toolActionData)arg;
    c_char *name;

    if (o != NULL) {
        type = c_getType(o);
        name = c_metaScopedName(c_metaObject(type));
        iprintf("Object <0x"PA_ADDRFMT"> refCount=%d type is: <0x"PA_ADDRFMT"> %s\n",
                (os_address)o, c_refCount(o), (os_address)type, name);
        os_free(name);
        OBJECT_PUSH(actionData, o);
        printType(type, actionData);
        printf("\n");
        OBJECT_POP(actionData);
    }
}

static void
printCollection(
    c_collectionType type,
    toolActionData actionData)
{
    c_long size, i, offset, esize;
    c_object o;
    c_voidp p;
    c_object arrayElement;
    c_type subtype;
    c_bool isRef;

    o = c_iterObject(actionData->stack, 0);
    switch (type->kind) {
    case C_ARRAY:
    case C_SEQUENCE:
        /* Walk over all entries */
        switch (type->kind) {
        case C_ARRAY:
            if (type->maxSize == 0) {
                size = c_arraySize((c_array)o);
            } else {
                size = type->maxSize;
            }
        break;
        case C_SEQUENCE:
            size = c_arraySize((c_array)o);
        break;
        default:
            size = 0;
            assert(FALSE);
        break;
        }

        if (c_typeIsRef(type->subType)) {
            esize = sizeof(c_voidp);
            isRef = TRUE;
        } else {
            esize = type->subType->size;
            isRef = FALSE;
        }
        p = o;
        offset = 0;
        for (i=0; i<size; i++) {
            iprintf("Element (%d) Offset (%d)\n",i,offset);
            arrayElement = isRef ? *((c_object *)p) : (c_object) p;
            if (arrayElement != NULL) {
                OBJECT_PUSH(actionData, arrayElement);
                if (isRef) {
                    subtype = c_getType(arrayElement);
                    printType(subtype, actionData);
                } else {
                    iprintf(" ");
                    printType(type->subType, actionData);
                }
                printf("\n");
                OBJECT_POP(actionData);
            } else {
                iprintf("    <0x0>\n");
            }
            p = C_DISPLACE(p, esize);
            offset += esize;
        }
    break;
    case C_STRING:
        printf(" \"%s\"",(c_char *)o);
    break;
    case C_SET:
    case C_LIST:
    case C_BAG:
    case C_DICTIONARY:
    case C_QUERY:
    {
        if (o != NULL) {
            /* Walk over the elements */
            c_walk(o, (c_action)printCollectionAction, actionData);
            if (c_count(o) == 0) {
                iprintf("<EMPTY>");
            }
        } else {
            iprintf("<NULL>");
        }
    }
    break;
    case C_SCOPE:
        c_scopeWalk(o, printCollectionAction, actionData);
    break;
    default:
        printf("Specified type <0x"PA_ADDRFMT"> is not a valid collection type\n",
               (os_address)type);
    break;
    }
}

static void
printPrimitive(
    c_type _this,
    toolActionData actionData)
{
    c_object o;
    c_value v;
    c_char *str;

    #define _CASE_(l,f,t) \
            case P_##l: \
            { \
                v.is.f = *(t *)o; \
                v.kind = V_##l; \
                str = c_valueImage(v); \
                printf("%s",c_valueImage(v)); \
                os_free(str); \
            } \
            break

    o = c_iterObject(actionData->stack, 0);
    switch(c_primitiveKind(_this)) {
    _CASE_(BOOLEAN,Boolean,c_bool);
    _CASE_(SHORT,Short,c_short);
    _CASE_(LONG,Long,c_long);
    _CASE_(LONGLONG,LongLong,c_longlong);
    _CASE_(OCTET,Octet,c_octet);
    _CASE_(USHORT,UShort,c_ushort);
    _CASE_(ULONG,ULong,c_ulong);
    _CASE_(ULONGLONG,ULongLong,c_ulonglong);
    _CASE_(CHAR,Char,c_char);
    _CASE_(WCHAR,WChar,c_wchar);
    _CASE_(FLOAT,Float,c_float);
    _CASE_(DOUBLE,Double,c_double);
    case P_ADDRESS:
        v.is.Address = *(c_address *)o;
        v.kind = V_ADDRESS;
        str = c_valueImage(v);
        printf("<%s>",c_valueImage(v));
        os_free(str);
    break;
    case P_VOIDP:
        v.is.Voidp = *(c_voidp *)o;
        v.kind = V_VOIDP;
        str = c_valueImage(v);
        printf("<%s>",c_valueImage(v));
        os_free(str);
    break;
    case P_MUTEX:
    case P_LOCK:
    case P_COND:
        printf("<******>");
    break;
    default:
        printf("Specified type <0x"PA_ADDRFMT"> is not a valid primitive type\n",
               (os_address)_this);
    break;
    }
    #undef _CASE_
}

static void
printEnum(
    c_enumeration _this,
    toolActionData actionData)
{
    c_object o;
    c_long i, size;
    c_constant constant;

    o = c_iterObject(actionData->stack, 0);
    size = c_enumerationCount(_this);
    i = *(c_long *)o;
    if ((i < 0) || (i > size)) {
        printf("(%d) \"Bad enumeration value\"\n", i);
    } else {
        constant = c_enumeration(_this)->elements[i];
        printf("%s",_METANAME(constant));
    }
}

static void
printStructure(
    c_structure _this,
    toolActionData actionData)
{
    c_object o;
    c_object object;
    c_long i, size;
    c_member member;

    o = c_iterObject(actionData->stack, 0);

    if (o) {
        if (c_iterLength(actionData->stack) == 1) {
            printf("%s ", _METANAME(_this));
        } else {
        }
        printf("{\n");
        size = c_structureMemberCount(_this);
        for (i=0; i<size; i++) {
            member = c_structureMember(_this, i);
            object = C_DISPLACE(o, (c_address)member->offset);
            if (c_typeIsRef(c_memberType(member))) {
                iprintf("    %s <0x"PA_ADDRFMT">\n",
                       c_specifierName(member),
                       *(os_address *)object);
            } else {
                OBJECT_PUSH(actionData, object);
                iprintf("%s ",c_specifierName(member));
                printType(c_memberType(member), actionData);
                printf("\n");
                OBJECT_POP(actionData);
            }
        }
        if (c_iterLength(actionData->stack) == 1) {
            iprintf("};");
        } else {
            iprintf("} %s;", _METANAME(_this));
        }
        if (c_type(_this) == v_handle_t) {
            v_object vo;
            v_handleClaim(*(v_handle *)o,&vo);
            v_handleRelease(*(v_handle *)o);
            printf(" /* Handle's object: <0x"PA_ADDRFMT"> */", (os_address)vo);
        }
    }
}

static c_bool
walkProperty(
    c_metaObject object,
    c_metaWalkActionArg actionArg)
{
    toolActionData actionData = (toolActionData)actionArg;
    c_object o;
    c_voidp addr;

    o = c_iterObject(actionData->stack, 0);

    if (c_baseObjectKind(object) == M_ATTRIBUTE) {
        addr = C_DISPLACE(o, (c_address)c_property(object)->offset);
        if (c_typeIsRef(c_property(object)->type)) {
            addr = *(c_voidp *)addr;
            if (c_baseObjectKind(c_property(object)->type) == M_COLLECTION) {
                if (addr) {
                    if (c_iterContains(actionData->stack, addr)) {
                        printf("Ignore cyclic reference 0x"PA_ADDRFMT"\n",
                               (os_address)addr);
                    } else {
                        OBJECT_PUSH(actionData, addr);
                        iprintf("%s ",_METANAME(object));
                        printType(c_property(object)->type, actionData);
                        OBJECT_POP(actionData);
                    }
                } else {
                    iprintf("    %s <0x"PA_ADDRFMT">", _METANAME(object), (os_address)addr);
                }
            } else {
                iprintf("    %s <0x"PA_ADDRFMT">", _METANAME(object), (os_address)addr);
                if (addr) {
                    /* Section for code to print additional type specific info. */
                    if (c_property(object)->type == v_topic_t) {
                        printf(" /* topic name is: %s */", v_topicName(addr));
                    }
                    if (c_property(object)->type == v_partition_t) {
                        printf(" /* Partition: %s */", v_partitionName(addr));
                    }
                    if (c_property(object)->type == c_type_t(c_getBase(object))) {
                        printf(" /* Type: %s */", _METANAME(c_type(addr)));
                    }
                }
            }
            printf("\n");
        } else {
            OBJECT_PUSH(actionData, addr);
            iprintf("%s ",_METANAME(object));
            printType(c_property(object)->type, actionData);
            printf("\n");
            OBJECT_POP(actionData);
        }
    }
    return TRUE;
}

static void
printInterface(
    c_interface interf,
    toolActionData actionData)
{
    c_object o;

    o = c_iterObject(actionData->stack, 0);
    /* Only walk over the properties if the reference is valid. */
    if (o != NULL) {
        iprintf("%s {\n", _METANAME(interf));
        c_metaWalk(c_metaObject(interf),
                   (c_metaWalkAction)walkProperty,
                   actionData);
        iprintf("};");
    }
}

static void
printClass(
    c_class class,
    toolActionData actionData)
{
    if (class->extends) {
        printClass(class->extends, actionData);
        printf("\n");
    }
    printInterface(c_interface(class), actionData);
}

static void
printUnion(
    c_union v_union,
    toolActionData actionData)
{
    c_type switchType;
    c_type caseType;
    c_unionCase deflt;
    c_unionCase activeCase;
    c_unionCase currentCase;
    c_object unionData;
    c_value switchValue;
    c_literal label;
    c_object o;
    int i,j, nCases, nLabels;

    c_long dataOffset;

    o = c_iterObject(actionData->stack, 0);
    /* action for the union itself */
    /* No action, but separate actions for the switch and the data */
    /* action(c_type(v_union), o, actionArg); */
    /* action for the switch */
    printType(v_union->switchType, actionData);
    printf("\n");

    switchType = c_typeActualType(v_union->switchType);
    /* Determine value of the switch field */
    switch (c_baseObjectKind(switchType)) {
    case M_PRIMITIVE:
        switch (c_primitive(switchType)->kind) {
#define __CASE__(prim, type) \
        case prim: switchValue = type##Value((*(type *)o)); break;
        __CASE__(P_BOOLEAN,c_bool)
        __CASE__(P_CHAR,c_char)
        __CASE__(P_SHORT,c_short)
        __CASE__(P_USHORT,c_ushort)
        __CASE__(P_LONG,c_long)
        __CASE__(P_ULONG,c_ulong)
        __CASE__(P_LONGLONG,c_longlong)
        __CASE__(P_ULONGLONG,c_ulonglong)
#undef __CASE__
        default:
            switchValue = c_undefinedValue();
            assert(FALSE);
        break;
        }
    break;
    case M_ENUMERATION:
        switchValue = c_longValue(*(c_long *)o);
    break;
    default:
        switchValue = c_undefinedValue();
        assert(FALSE);
    break;
    }

    /* Determine the label corresponding to this field */

    activeCase = NULL;
    deflt = NULL;
    nCases = c_arraySize(v_union->cases);

    for (i=0; (i<nCases) && !activeCase; i++) {
        currentCase = c_unionCase(v_union->cases[i]);
        nLabels = c_arraySize(currentCase->labels);
        if (nLabels > 0) {
            for (j=0; (j<nLabels) && !activeCase; j++) {
                label = c_literal(currentCase->labels[j]);
                if (c_valueCompare(switchValue, label->value) == C_EQ) {
                    activeCase = currentCase;
                }
            }
        } else {
            deflt = currentCase;
        }
    }
    if (!activeCase) {
        activeCase = deflt;
    }
    assert(activeCase);
    if (activeCase) {
        caseType = c_specifier(activeCase)->type;
        if (c_type(v_union)->alignment >= v_union->switchType->size) {
            dataOffset = c_type(v_union)->alignment;
        } else {
            dataOffset = v_union->switchType->size;
        }
        unionData = C_DISPLACE(o, (c_address)dataOffset);
        OBJECT_PUSH(actionData, unionData);
        printType(caseType, actionData);
        printf("\n");
        OBJECT_POP(actionData);
    }
}


static void
printCollectionKind(
    c_collectionType type,
    toolActionData actionData)
{
    c_long size;
    c_object o;

    o = c_iterObject(actionData->stack, 0);
    if (o != NULL) {
        switch (type->kind) {
        case C_ARRAY:
            size = c_collectionType(type)->maxSize;
            if (size == 0) {
                printf("<0x"PA_ADDRFMT"> /* C_ARRAY(%d) */", (os_address)o, c_arraySize(o));
            } else {
                printf("<0x"PA_ADDRFMT"> /* Bounded C_ARRAY(%d) */", (os_address)o, size);
            }
        break;
        case C_SEQUENCE:
            printf("<0x"PA_ADDRFMT"> /* C_SEQUENCE(%d) */", (os_address)o, c_sequenceSize(o));
        break;
        case C_STRING:
            printf(" \"%s\"",(c_char *)o);
        break;
        case C_SET:
            printf("<0x"PA_ADDRFMT"> /* C_SET(%d) */", (os_address)o, c_count(o));
        break;
        case C_LIST:
            printf("<0x"PA_ADDRFMT"> /* C_LIST(%d) */", (os_address)o, c_count(o));
        break;
        case C_BAG:
            printf("<0x"PA_ADDRFMT"> /* C_BAG(%d) */", (os_address)o, c_count(o));
        break;
        case C_DICTIONARY:
            printf("<0x"PA_ADDRFMT"> /* C_DICTIONARY(%d) */", (os_address)o, c_count(o));
        break;
        case C_QUERY:
            printf("<0x"PA_ADDRFMT"> /* C_QUERY(%d) */", (os_address)o, c_count(o));
        break;
        case C_SCOPE:
            printf("<0x"PA_ADDRFMT"> /* C_SCOPE(%d) */", (os_address)o, c_scopeCount(o));
        break;
        default:
            printf("Specified type <0x"PA_ADDRFMT"> is not a valid collection type\n",
                   (os_address)type);
        break;
        }
    } else {
        printf("<0x"PA_ADDRFMT"> \"Unexpected Internal Stack Error\"", (os_address)o);
    }
}

static void
printType (
    c_type type,
    toolActionData actionData)
{
    c_type actualType;

    actualType = c_typeActualType(type);
    /* Determine which action to take */
    switch (c_baseObjectKind(actualType)) {
    case M_PRIMITIVE:
        printPrimitive(actualType, actionData);
    break;
    case M_ENUMERATION:
        printEnum(c_enumeration(actualType), actionData);
    break;
    case M_STRUCTURE:
    case M_EXCEPTION:
        printStructure(c_structure(actualType), actionData);
    break;
    case M_COLLECTION:
        if (c_iterLength(actionData->stack) == 1) {
            printCollection(c_collectionType(actualType), actionData);
        } else {
            printCollectionKind(c_collectionType(actualType), actionData);
        }
    break;
    case M_CLASS:
        printClass(c_class(actualType), actionData);
    break;
    case M_INTERFACE:
        printInterface(c_interface(actualType), actionData);
    break;
    case M_UNION:
        printUnion(c_union(actualType), actionData);
    break;
    default:
        printf("Specified type <0x"PA_ADDRFMT"> is not a valid type\n",
               (os_address)type);
        assert(FALSE); /* Only descendants of type possible */
    break;
    }
}

static void
printStructureMember(
    c_structure _this,
    toolActionData actionData,
    c_address offset)
{
}

static void
printCollectionElement(
    c_collectionType _this,
    toolActionData actionData,
    c_address offset)
{
}


typedef struct lookupMetaData {
    c_long offset;
    c_metaObject metaObject;
} *lookupMetaData;

static c_bool
lookupProperty(
    c_metaObject object,
    c_metaWalkActionArg actionArg)
{
    lookupMetaData actionData = (lookupMetaData)actionArg;
    c_long offset, size;
    c_bool proceed = TRUE;

    if (c_baseObjectKind(object) == M_ATTRIBUTE) {
        offset = actionData->offset - c_property(object)->offset;
        size = c_typeSize(c_property(object)->type);
        if ((offset >= 0) && (offset < size)) {
            assert(actionData->metaObject == NULL);
            actionData->metaObject = object;
            actionData->offset -= offset; /* calculate correct offset */
            proceed = FALSE;
        }
    }
    return proceed;
}

static void
printInterfaceAttribute(
    c_interface _this,
    toolActionData actionData,
    c_address offset)
{
    struct lookupMetaData MetaData;
    c_object o, object;
    c_string name;

    o = c_iterObject(actionData->stack, 0);
    /* Only walk over the properties if the reference is valid. */
    if (o != NULL) {
        MetaData.offset = offset;
        MetaData.metaObject = NULL;
        c_metaWalk(c_metaObject(_this),
                   (c_metaWalkAction)lookupProperty,
                   &MetaData);
        if (MetaData.metaObject != NULL) {
            c_iter stack;

            name = c_metaScopedName(MetaData.metaObject);
            if (MetaData.offset < (c_long)offset) {
                printf("Address is %lu bytes in property %s\n",
                       (offset - MetaData.offset), name);
            } else {
                printf("Address refers to property %s\n",
                       name);
            }
            os_free(name);

            stack = actionData->stack;
            actionData->stack = NULL;
            object = C_DISPLACE(o, (c_address)MetaData.offset);
            OBJECT_PUSH(actionData, object);
            printType(c_property(MetaData.metaObject)->type, actionData);
            printf("\n");
            OBJECT_POP(actionData);
            c_iterFree(actionData->stack);
            actionData->stack = stack;
        }
    }
}

static void
printClassAttribute(
    c_class _this,
    toolActionData actionData,
    c_address offset)
{
    c_long size;

    size = 0;
    if (_this->extends) {
        size = c_typeSize(c_type(_this->extends));
        if (offset < (c_address)size) {
            printClassAttribute(_this->extends, actionData, offset);
        }
    }
    printInterfaceAttribute(c_interface(_this), actionData, offset);
}

static void
printUnionMember(
    c_union _this,
    toolActionData actionData,
    c_address offset)
{
}

static void
tryPrintOffset(
    c_voidp o,
    toolActionData actionData,
    c_address offset)
{
    c_type type, actualType;

    type = c_getType(o);
    actualType = c_typeActualType(type);

    /* Determine which action to take */
    switch (c_baseObjectKind(actualType)) {
    case M_PRIMITIVE:
        printPrimitive(actualType, actionData);
    break;
    case M_ENUMERATION:
        printEnum(c_enumeration(actualType), actionData);
    break;
    case M_STRUCTURE:
    case M_EXCEPTION:
        printStructureMember(c_structure(actualType), actionData, offset);
    break;
    case M_COLLECTION:
        printCollectionElement(c_collectionType(actualType), actionData, offset);
    break;
    case M_CLASS:
        printClassAttribute(c_class(actualType), actionData, offset);
    break;
    case M_INTERFACE:
        printInterfaceAttribute(c_interface(actualType), actionData, offset);
    break;
    case M_UNION:
        printUnionMember(c_union(actualType), actionData, offset);
    break;
    default:
        printf("Specified type <0x"PA_ADDRFMT"> is not a valid type\n",
               (os_address)type);
        assert(FALSE); /* Only descendants of type possible */
    break;
    }
}

static void
printWalkHistory (
    c_object o,
    c_iterActionArg arg)
{
    c_type type;
    c_char *name, *ename;

    if (o) {
        type = c_getType(o);
        name = c_metaScopedName(c_metaObject(type));
        printf("<0x"PA_ADDRFMT"> %s",(os_address)o,name);
        if (c_checkType(o, "v_entity") == o) {
            ename = v_entityName(o);
            if (ename != NULL) {
                printf(" /* %s */", ename);
            }
        } else if (c_checkType(o, "c_metaObject") == o) {
            ename = c_metaScopedName(o);
            if (ename != NULL) {
                printf(" /* %s */", ename);
                os_free(ename);
            }
        }
        printf("\n");
    } else {
        printf("<0x0>\n");
    }
}

