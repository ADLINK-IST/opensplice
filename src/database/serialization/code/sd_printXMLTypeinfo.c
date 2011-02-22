/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "os_heap.h"
#include "os_stdlib.h"
#include "c_base.h"
#include "c_metabase.h"
#include "sd__printXMLTypeinfo.h"
#include "sd__serializerXML.h"

static c_ulong
sd_printTaggedString (
    c_char *buffer,
    c_char *tag,
    c_char *str)
{
    c_ulong len;

    len = os_sprintf(buffer, "<%s>", tag);
    len += sd_printCharData(&buffer[len], str);
    len += os_sprintf(&buffer[len], "</%s>", tag);
    return len;
}

static c_ulong
sd_printTagAndName (
    c_char *buffer,
    c_char *tag,
    c_char *str,
    c_bool escapeQuote)
{
    c_ulong len;

    if ( escapeQuote ) {
        len = os_sprintf(buffer, "<%s name=\\\"", tag);
    } else {
        len = os_sprintf(buffer, "<%s name=\"", tag);
    }
    len += sd_printCharData(&buffer[len], str);
    if ( escapeQuote ) {
        len += os_sprintf(&buffer[len], "\\\">");
    } else {
        len += os_sprintf(&buffer[len], "\">");
    }
    return len;
}

typedef void (*sd_printAction)(c_char *data, c_long len, void *userData);

typedef struct sd_printWalkInfo {
    sd_contextItemAction walkAction;
    sd_printAction       printAction;
    void                *actionArg;
    c_bool               escapeQuote;
} sd_printWalkInfo;

static void
sd_printModule (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemModule sitem = (sd_contextItemModule)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    if ( item->name != NULL ) {
        len = sd_printTagAndName(buffer, "Module", item->name, info->escapeQuote);
        info->printAction(buffer, len, info->actionArg);
        sd_contextItemWalkChildren(item, info->walkAction, info);
        len = os_sprintf(buffer, "</Module>");
        info->printAction(buffer, len, info->actionArg);
    } else {
        sd_contextItemWalkChildren(item, info->walkAction, info);
    }

}

static void
sd_printStructure (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemStructure sitem = (sd_contextItemStructure)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    len = sd_printTagAndName(buffer, "Struct", item->name, info->escapeQuote);
    info->printAction(buffer, len, info->actionArg);
    sd_contextItemWalkChildren(item, info->walkAction, info);
    len = os_sprintf(buffer, "</Struct>");
    info->printAction(buffer, len, info->actionArg);

}

static void
sd_printScope (
    c_metaObject scope,
    sd_printWalkInfo *info)
{
    c_long len;
    c_char buffer[256];
    c_metaObject pscope;

    if ( scope ) {
        pscope = c_metaObject(scope->definedIn);
        sd_printScope(pscope, info);
        if ( scope->name ) {
            len = os_sprintf(buffer, "%s::", scope->name);
            info->printAction(buffer, len, info->actionArg);
        }
    }
}

static c_bool
sd_printLiteralIsBoolean (
    sd_contextItemLiteral item)
{
    c_bool isBoolean = FALSE;
    c_type type;

    type = c_typeActualType(item->type);
    if ( c_baseObject(type)->kind == M_PRIMITIVE ) {
        if ( c_primitive(type)->kind == P_BOOLEAN ) {
            isBoolean = TRUE;
        }
    }
    return isBoolean;
}

static c_bool
sd_printLiteralIsEnumeration (
    sd_contextItemLiteral item)
{
    c_bool isEnumeration = FALSE;
    c_type type;

    type = c_typeActualType(item->type);
    if ( c_baseObject(type)->kind == M_ENUMERATION ) {
        isEnumeration = TRUE;
    }
    return isEnumeration;
}

static void
sd_printLiteralBoolean (
    sd_contextItemLiteral item,
    sd_printWalkInfo     *info)
{
    c_long len;
    c_char buffer[256];

    if ( info->escapeQuote ) {
        if(item->value.kind == V_LONG){
            if(item->value.is.Long){
                len = os_sprintf(buffer, "<Label value=\\\"True\\\"/>");
            } else {
                len = os_sprintf(buffer, "<Label value=\\\"False\\\"/>");
            }
        } else if(item->value.kind == V_BOOLEAN){
            if ( item->value.is.Boolean ) {
                len = os_sprintf(buffer, "<Label value=\\\"True\\\"/>");
            } else {
                len = os_sprintf(buffer, "<Label value=\\\"False\\\"/>");
            }
        } else {
            assert(FALSE);
            len = os_sprintf(buffer, "<Label value=\\\"False\\\"/>");
        }
    } else {
        if(item->value.kind == V_LONG){
            if(item->value.is.Long){
                len = os_sprintf(buffer, "<Label value=\"True\"/>");
            } else {
                len = os_sprintf(buffer, "<Label value=\"False\"/>");
            }
        } else if(item->value.kind == V_BOOLEAN){
            if ( item->value.is.Boolean ) {
                len = os_sprintf(buffer, "<Label value=\"True\"/>");
            } else {
                len = os_sprintf(buffer, "<Label value=\"False\"/>");
            }
        } else {
            assert(FALSE);
            len = os_sprintf(buffer, "<Label value=\"False\"/>");
        }
    }
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printLiteralEnumeration (
    sd_contextItemLiteral item,
    sd_printWalkInfo     *info)
{
    c_enumeration enumeration;
    c_constant constant;
    c_long n;
    c_long len;
    c_char buffer[256];

    assert(item->value.kind == V_LONG);

    enumeration = c_enumeration(c_typeActualType(item->type));

    n = item->value.is.Long;

    assert(n < c_arraySize(enumeration->elements));

    constant = enumeration->elements[n];

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<Label value=\\\"%s\\\"/>", c_metaObject(constant)->name);
    } else {
        len = os_sprintf(buffer, "<Label value=\"%s\"/>", c_metaObject(constant)->name);
    }
    info->printAction(buffer, len, info->actionArg);
}


static void
sd_printLiteral (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    c_long len;
    c_char buffer[256];
    sd_contextItemLiteral litem = (sd_contextItemLiteral)item;
    c_char *image;

    assert(item);

    if ( item->name ) {
        if ( sd_printLiteralIsBoolean(litem) ) {
            sd_printLiteralBoolean(litem, info);
        } else if ( sd_printLiteralIsEnumeration(litem) ) {
            sd_printLiteralEnumeration(litem, info);
        } else {
            image = c_valueImage(litem->value);
            if ( info->escapeQuote ) {
                len = os_sprintf(buffer, "<Label value=\\\"%s\\\"/>", image);
            } else {
                len = os_sprintf(buffer, "<Label value=\"%s\"/>", image);
            }
            info->printAction(buffer, len, info->actionArg);
            os_free(image);
        }
    } else {
        len = os_sprintf(buffer, "<Default/>");
        info->printAction(buffer, len, info->actionArg);
    }
}

static void
sd_printLabel (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    c_long len;
    c_char buffer[256];

    assert(item);

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<Element name=\\\"%s\\\"/>", item->name);
    } else {
        len = os_sprintf(buffer, "<Element name=\"%s\"/>", item->name);
    }
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printTyperef (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    c_long len;
    c_char buffer[256];

    assert(item);

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<Type name=\\\"");
    } else {
        len = os_sprintf(buffer, "<Type name=\"");
    }
    info->printAction(buffer, len, info->actionArg);

    sd_printScope(item->scope, info);

    len  = sd_printCharData(buffer, item->name);
    if ( info->escapeQuote ) {
        len += os_sprintf(&buffer[len], "\\\"/>");
    } else {
        len += os_sprintf(&buffer[len], "\"/>");
    }
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printTypedef (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemTypedef sitem = (sd_contextItemTypedef)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<TypeDef name=\\\"%s\\\">", item->name);
    } else {
        len = os_sprintf(buffer, "<TypeDef name=\"%s\">", item->name);
    }
    info->printAction(buffer, len, info->actionArg);
    sd_contextItemWalkChildren(item, info->walkAction, info);
    len = os_sprintf(buffer, "</TypeDef>");
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printMember (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemMember sitem = (sd_contextItemMember)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<Member name=\\\"%s\\\">", item->name);
    } else {
        len = os_sprintf(buffer, "<Member name=\"%s\">", item->name);
    }
    info->printAction(buffer, len, info->actionArg);
    sd_contextItemWalkChildren(item, info->walkAction, info);
    len = os_sprintf(buffer, "</Member>");
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printPrimitive (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemPrimitive sitem = (sd_contextItemPrimitive)item;
    c_long len;
    c_char buffer[256];
    c_char *name;

    assert(sitem);

#define __CASE__(kind,n) case kind: name = #n; break;
    switch ( sitem->kind ) {
        __CASE__(P_ADDRESS,Address)
        __CASE__(P_BOOLEAN,Boolean)
        __CASE__(P_CHAR,Char)
        __CASE__(P_OCTET,Octet)
        __CASE__(P_WCHAR,WChar)
        __CASE__(P_SHORT,Short)
        __CASE__(P_USHORT,UShort)
        __CASE__(P_LONG,Long)
        __CASE__(P_ULONG,ULong)
        __CASE__(P_LONGLONG,LongLong)
        __CASE__(P_ULONGLONG,ULongLong)
        __CASE__(P_FLOAT,Float)
        __CASE__(P_DOUBLE,Double)
        __CASE__(P_VOIDP,an-object-value)
    default:
        break;
    }
#undef __CASE

    len = os_sprintf(buffer, "<%s/>", name);
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printTime (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemTime sitem = (sd_contextItemTime)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    len = os_sprintf(buffer, "<Time/>");
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printString (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemString sitem = (sd_contextItemString)item;
    c_long len;
    c_char buffer[256];
    c_long size;

    assert(sitem);

    size = sd_contextItemCollection(sitem)->maxSize;

    if ( size > 0 ) {
        if ( info->escapeQuote ) {
            len = os_sprintf(buffer, "<String length=\\\"%d\\\"/>", size);
        } else {
            len = os_sprintf(buffer, "<String length=\"%d\"/>", size);
        }
        info->printAction(buffer, len, info->actionArg);
    } else {
        len = os_sprintf(buffer, "<String/>");
        info->printAction(buffer, len, info->actionArg);
    }

}

static void
sd_printArray (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemArray sitem = (sd_contextItemArray)item;
    c_long len;
    c_char buffer[256];
    c_long size;

    assert(sitem);

    size = sd_contextItemCollection(sitem)->maxSize;

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<Array size=\\\"%d\\\">", size);
    } else {
        len = os_sprintf(buffer, "<Array size=\"%d\">", size);
    }
    info->printAction(buffer, len, info->actionArg);
    sd_contextItemWalkChildren(item, info->walkAction, info);
    len = os_sprintf(buffer, "</Array>");
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printSequence (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemSequence sitem = (sd_contextItemSequence)item;
    c_long len;
    c_char buffer[256];
    c_long size;

    assert(sitem);

    size = sd_contextItemCollection(sitem)->maxSize;

    if ( size > 0 ) {
        if ( info->escapeQuote ) {
            len = os_sprintf(buffer, "<Sequence size=\\\"%d\\\">", size);
        } else {
            len = os_sprintf(buffer, "<Sequence size=\"%d\">", size);
        }
        info->printAction(buffer, len, info->actionArg);
    } else {
        len = os_sprintf(buffer, "<Sequence>");
        info->printAction(buffer, len, info->actionArg);
    }

    sd_contextItemWalkChildren(item, info->walkAction, info);

    len = os_sprintf(buffer, "</Sequence>");
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printEnumeration (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemEnumeration sitem = (sd_contextItemEnumeration)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<Enum name=\\\"%s\\\">", item->name);
    } else {
        len = os_sprintf(buffer, "<Enum name=\"%s\">", item->name);
    }
    info->printAction(buffer, len, info->actionArg);
    sd_contextItemWalkChildren(item, info->walkAction, info);
    len = os_sprintf(buffer, "</Enum>");
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printUnionSwitch (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemUnionSwitch sitem = (sd_contextItemUnionSwitch)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    len = os_sprintf(buffer, "<SwitchType>");
    info->printAction(buffer, len, info->actionArg);
    sd_contextItemWalkChildren(item, info->walkAction, info);
    len = os_sprintf(buffer, "</SwitchType>");
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printUnionCase (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemUnionSwitch sitem = (sd_contextItemUnionSwitch)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<Case name=\\\"%s\\\">", item->name);
    } else {
        len = os_sprintf(buffer, "<Case name=\"%s\">", item->name);
    }
    info->printAction(buffer, len, info->actionArg);
    sd_contextItemWalkChildren(item, info->walkAction, info);
    len = os_sprintf(buffer, "</Case>");
    info->printAction(buffer, len, info->actionArg);
}

static void
sd_printUnion (
    sd_contextItem item,
    sd_printWalkInfo *info)
{
    sd_contextItemUnion sitem = (sd_contextItemUnion)item;
    c_long len;
    c_char buffer[256];

    assert(sitem);

    if ( info->escapeQuote ) {
        len = os_sprintf(buffer, "<Union name=\\\"%s\\\">", item->name);
    } else {
        len = os_sprintf(buffer, "<Union name=\"%s\">", item->name);
    }
    info->printAction(buffer, len, info->actionArg);
    sd_contextItemWalkChildren(item, info->walkAction, info);
    len = os_sprintf(buffer, "</Union>");
    info->printAction(buffer, len, info->actionArg);
}

static c_bool
sd_walkItemTree (
    sd_contextItem item,
    void *arg)
{
    sd_printWalkInfo *info = (sd_printWalkInfo *)arg;

    switch ( item->kind ) {
        case SD_CONTEXT_ITEM_SCOPE:
        case SD_CONTEXT_ITEM_MODULE:
            sd_printModule(item, info);
            break;
        case SD_CONTEXT_ITEM_STRUCTURE:
            sd_printStructure(item, info);
            break;
        case SD_CONTEXT_ITEM_MEMBER:
            sd_printMember(item, info);
            break;
        case SD_CONTEXT_ITEM_PRIMITIVE:
            sd_printPrimitive(item, info);
            break;
        case SD_CONTEXT_ITEM_STRING:
            sd_printString(item, info);
            break;
        case SD_CONTEXT_ITEM_TIME:
            sd_printTime(item, info);
            break;
         case SD_CONTEXT_ITEM_ARRAY:
            sd_printArray(item, info);
            break;
        case SD_CONTEXT_ITEM_SEQUENCE:
            sd_printSequence(item, info);
            break;
        case SD_CONTEXT_ITEM_TYPEDEF:
            sd_printTypedef(item, info);
            break;
        case SD_CONTEXT_ITEM_TYPE:
            sd_printTyperef(item, info);
            break;
        case SD_CONTEXT_ITEM_ENUMERATION:
            sd_printEnumeration(item, info);
            break;
        case SD_CONTEXT_ITEM_LABEL:
            sd_printLabel(item, info);
            break;
        case SD_CONTEXT_ITEM_LITERAL:
            sd_printLiteral(item, info);
            break;
        case SD_CONTEXT_ITEM_UNION:
            sd_printUnion(item, info);
            break;
        case SD_CONTEXT_ITEM_UNIONSWITCH:
            sd_printUnionSwitch(item, info);
            break;
        case SD_CONTEXT_ITEM_UNIONCASE:
            sd_printUnionCase(item, info);
            break;
        default:
            break;
    }

    return TRUE;
}

typedef struct sd_lengthInfo {
    c_ulong len;
} sd_lengthInfo;

typedef struct sd_printInfo {
    c_ulong offset;
    c_char *buffer;
} sd_printInfo;


static void
sd_determineLength (
    c_char *data,
    c_long len,
    void *arg)
{
    sd_lengthInfo *info = (sd_lengthInfo *) arg;

    info->len += len;
}

static void
sd_printType (
    c_char *data,
    c_long len,
    void *arg)
{
    sd_printInfo *info = (sd_printInfo *) arg;

    os_sprintf(&info->buffer[info->offset], "%s", data);
    info->offset += len;
}

c_long
sd_printXmlTypeinfoLength (
    sd_contextItem item,
    c_bool escapeQuote)
{
    sd_lengthInfo lengthInfo;
    sd_printWalkInfo walkInfo;
    c_char buffer[64];
    c_long len;

    if ( escapeQuote ) {
        len = os_sprintf(buffer, "<%s version=\\\"1.0.0\\\"></%s>", SPLICE_METADATA_TAG, SPLICE_METADATA_TAG);
    } else {
        len = os_sprintf(buffer, "<%s version=\"1.0.0\"></%s>", SPLICE_METADATA_TAG, SPLICE_METADATA_TAG);
    }

    lengthInfo.len = len + 1;

    walkInfo.walkAction = sd_walkItemTree;
    walkInfo.printAction = sd_determineLength;
    walkInfo.actionArg = &lengthInfo;
    walkInfo.escapeQuote = escapeQuote;

    sd_walkItemTree(item, &walkInfo);

    return lengthInfo.len;
}

void
sd_printXmlTypeinfo (
    sd_contextItem item,
    char *buffer,
    c_bool escapeQuote
    )
{
    sd_printInfo printInfo;
    sd_printWalkInfo walkInfo;
    c_long len;

    if ( escapeQuote ) {
        len = os_sprintf(buffer, "<%s version=\\\"1.0.0\\\">", SPLICE_METADATA_TAG);
    } else {
        len = os_sprintf(buffer, "<%s version=\"1.0.0\">", SPLICE_METADATA_TAG);
    }

    printInfo.offset = len;
    printInfo.buffer = buffer;

    walkInfo.walkAction = sd_walkItemTree;
    walkInfo.printAction = sd_printType;
    walkInfo.actionArg = &printInfo;
    walkInfo.escapeQuote = escapeQuote;

    sd_walkItemTree(item, &walkInfo);

    len = os_sprintf(&buffer[printInfo.offset], "</%s>", SPLICE_METADATA_TAG);
}

