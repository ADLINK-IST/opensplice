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
/* Interface */
#include "sd__serializer.h"
#include "sd__resultCodes.h"
#include "sd_serializerXMLTypeinfo.h"

/* Implementation */
#include "os_stdlib.h"
#include "os_heap.h"
#include "c_collection.h"
#include "c_module.h"
#include "sd_misc.h"
#include "sd__serializerXML.h"
#include "sd__confidence.h"
#include "sd_errorReport.h"
#include "sd_stringsXML.h"
#include "sd_deepwalkMeta.h"
#include "sd_list.h"
#include "sd_string.h"
#include "sd__contextItem.h"
#include "sd__printXMLTypeinfo.h"

#include "sd_typeInfoParser.h"

#define SD_FORMAT_ID            0x584DU    /* currently the same as XML */
#define SD_FORMAT_VERSION       0x0001U
#define SD_XML_METADATA_VERSION "1.0.0"

#define ResolveType(s,t) c_type(c_metaResolve(c_metaObject(s),#t))

C_CLASS(sd_serializerXMLTypeinfo);
C_STRUCT(sd_serializerXMLTypeinfo) {
    C_EXTENDS(sd_serializer);
    c_bool escapeQuote;
};


/* -------------------------- checking routines -----------------------*/

/** \brief Check if a serializer is an instance of the serializerXMLTypeinfo
 *         class implemented in this file.
 *
 *  Functions implemented in this file assume that an instance of
 *  the serializerXMLTypeinfo class is sent as first parameter. This routine
 *  can be used as a confidence check to avoid mixing of instances.
 *
 *  \param serializer The serializer object (self).
 *  \return TRUE is serializer is indeed a serializerXMLTypeinfo instance,
            FALSE otherwise.
 */

static c_bool
sd_checkSerializerType(
    sd_serializer serializer)
{
    return (c_bool)(
            ((unsigned int)serializer->formatID == SD_FORMAT_ID) &&
            ((unsigned int)serializer->formatVersion == SD_FORMAT_VERSION));
}


typedef struct {
    sd_contextItemKind kind;
    c_metaKind         metaKind;
    const c_char      *xmlName;
    const c_char      *typeName;
    union {
        c_primKind     primKind;
        c_collKind     collKind;
    } special;
} sd_typeInfo;

static const sd_typeInfo typeInfoMap[] = {
    { SD_CONTEXT_ITEM_MODULE,      M_MODULE,      "Module",      "c_structure",   { P_UNDEFINED } },
    { SD_CONTEXT_ITEM_STRUCTURE,   M_STRUCTURE,   "Struct",      "c_structure",   { P_UNDEFINED } },
    { SD_CONTEXT_ITEM_MEMBER,      M_MEMBER,      "Member",      "c_member",      { P_UNDEFINED } },
    { SD_CONTEXT_ITEM_TYPEDEF,     M_TYPEDEF,     "TypeDef",     "c_typeDef",     { P_UNDEFINED } },
    { SD_CONTEXT_ITEM_ARRAY,       M_COLLECTION,  "Array",       "c_collection",  { C_ARRAY     } },
    { SD_CONTEXT_ITEM_SEQUENCE,    M_COLLECTION,  "Sequence",    "c_collection",  { C_SEQUENCE  } },
    { SD_CONTEXT_ITEM_ENUMERATION, M_ENUMERATION, "Enum",        "c_enumeration", { P_UNDEFINED } },
    { SD_CONTEXT_ITEM_UNION,       M_UNION,       "Union",       "c_union",       { P_UNDEFINED } },
    { SD_CONTEXT_ITEM_UNIONCASE,   M_UNIONCASE,   "UNIONCASE",   "c_unionCase",   { P_UNDEFINED } }
};
static const c_long typeInfoMapSize = sizeof(typeInfoMap)/sizeof(sd_typeInfo);

static const sd_typeInfo specTypeInfoMap[] = {
    { SD_CONTEXT_ITEM_TYPE,      M_UNDEFINED,  "Type",      NULL,           { P_UNDEFINED } },
    { SD_CONTEXT_ITEM_STRING,    M_COLLECTION, "String",    "c_string",     { C_STRING    } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "Address",   "c_address",    { P_ADDRESS   } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "Boolean",   "c_bool",       { P_BOOLEAN   } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "Char",      "c_char",       { P_CHAR      } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "WChar",     "c_wchar",      { P_WCHAR     } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "Octet",     "c_octet",      { P_OCTET     } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "Short",     "c_short",      { P_SHORT     } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "UShort",    "c_ushort",     { P_USHORT    } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "Long",      "c_long",       { P_LONG      } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "ULong",     "c_ulong",      { P_ULONG     } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "LongLong",  "c_longlong",   { P_LONGLONG  } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "ULongLong", "c_ulonglong",  { P_ULONGLONG } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "Float",     "c_float",      { P_FLOAT     } },
    { SD_CONTEXT_ITEM_PRIMITIVE, M_PRIMITIVE,  "Double",    "c_double",     { P_DOUBLE    } }
};
static const c_long specTypeInfoMapSize = sizeof(specTypeInfoMap)/sizeof(sd_typeInfo);

static const sd_typeInfo *
sd_findNormalTypeInfo (
    c_char *name)
{
    const sd_typeInfo *typeInfo = NULL;
    c_long i;

    assert(name);

    for ( i = 0; (typeInfo == NULL) && (i < typeInfoMapSize); i++ ) {
        if ( strcmp(typeInfoMap[i].xmlName, name) == 0 ) {
            typeInfo = &typeInfoMap[i];
        }
    }

    return typeInfo;
}

static const sd_typeInfo *
sd_findSpecialTypeInfo (
    c_char *name)
{
    const sd_typeInfo *typeInfo = NULL;
    c_long i;

    assert(name);

    for ( i = 0; (typeInfo == NULL) && (i < specTypeInfoMapSize); i++ ) {
        if ( strcmp(specTypeInfoMap[i].xmlName, name) == 0 ) {
            typeInfo = &specTypeInfoMap[i];
        }
    }

    return typeInfo;
}

static const sd_typeInfo *
sd_findTypeInfo (
    c_char *name)
{
    const sd_typeInfo *typeInfo = NULL;

    assert(name);

    typeInfo = sd_findSpecialTypeInfo(name);
    if ( !typeInfo ) {
        typeInfo = sd_findNormalTypeInfo(name);
    }


    return typeInfo;
}

static c_bool
scopeIsModule (
    c_metaObject scope)
{
    c_bool result = FALSE;

    if ( c_baseObject(scope)->kind == M_MODULE ) {
        result = TRUE;
    }
    return result;
}

#define SD_SCOPE_IDENTIFIER "::"
static c_char *
sd_concatScoped(
    c_char *scopeName,
    c_char *name)
{
    c_char *result;
    c_ulong len;

    if (scopeName && *scopeName) {
        len = strlen(scopeName) + sizeof(SD_SCOPE_IDENTIFIER) + strlen(name) + 1;
        result = os_malloc(len);
        snprintf(result, len, "%s%s%s", scopeName, SD_SCOPE_IDENTIFIER, name);
    } else {
        len = strlen(name) + 1;
        result = os_malloc(len);
        os_strcpy(result, name);
    }

    return result;
}
#undef SD_SCOPE_IDENTIFIER


/* --------------------- Serialization driving functions -------------------- */

typedef struct sd_specialTypes_s {
    c_object c_string;
    c_object c_time;
} sd_specialTypes;


C_CLASS(sd_context);
C_STRUCT(sd_context) {
    c_metaObject base;
    sd_list scopeList;
    sd_list processed;
    sd_list stack;
    sd_contextItem current;
    sd_specialTypes specialTypes;
};

static void
sd_specialTypesInit (
    c_object base,
    sd_specialTypes *special)
{
   special->c_string = c_resolve(base, "c_string");
   special->c_time   = c_resolve(base, "c_time");
}


static sd_context
sd_contextNew (
    c_object base)
{
    sd_context context;

    context = (sd_context)os_malloc(C_SIZEOF(sd_context));

    if ( context ) {
        context->base      = base;
        context->scopeList = sd_listNew();
        context->processed = sd_listNew();
        context->stack     = sd_listNew();
        context->current   = NULL;
        sd_specialTypesInit(base, &context->specialTypes);

    }

    return context;
}


static void
sd_contextFree (
    sd_context context)
{
    sd_contextItem item;

    item = (sd_contextItem)sd_listTakeFirst(context->scopeList);
    while ( item ) {
        sd_contextItemFree(item);
        item = (sd_contextItem)sd_listTakeFirst(context->scopeList);
    }
    sd_listFree(context->scopeList);

    sd_listFree(context->processed);

    item = (sd_contextItem)sd_listTakeFirst(context->stack);
    while ( item ) {
        sd_contextItemFree(item);
        item = (sd_contextItem)sd_listTakeFirst(context->stack);
    }
    sd_listFree(context->stack);

    if ( context->current ) {
        sd_contextItemFree(context->current);
    }

    os_free(context);


}

static void
sd_contextPushItem (
    sd_context context,
    sd_contextItem item)
{
    assert(context);
    assert(context->stack);
    assert(item);

    sd_listInsert(context->stack, item);
    sd_contextItemKeep(item);
    context->current = item;
}

static sd_contextItem
sd_contextPopItem (
    sd_context context)
{
    sd_contextItem item;

    assert(context);
    assert(context->stack);


    item = (sd_contextItem)sd_listTakeFirst(context->stack);

    assert(item == context->current);

    context->current = (sd_contextItem)sd_listReadFirst(context->stack);
    sd_contextItemFree(item);

    return item;
}


static void
sd_contextAddProcessed (
    sd_context context,
    c_object   object)
{
    assert(context);
    assert(context->processed);
    assert(object);

    sd_listAppend(context->processed, object);
}



static c_bool
sd_contextIsProcessed (
    sd_context context,
     c_object   object)
{
    c_bool found = FALSE;

    assert(context);
    assert(context->processed);
    assert(object);

    if ( sd_listFindObject(context->processed, object) ) {
        found = TRUE;
    }

    return found;
}


static sd_contextItem
sd_contextAddScope (
    sd_context context,
    c_metaObject scope)
{
    sd_contextItemScope item;
    c_module module;

    assert(context);
    assert(context->scopeList);
    assert(scope);

    if ( scope != context->base ) {
        module = c_module(scope);

        item = (sd_contextItemScope)sd_contextItemNew(SD_CONTEXT_ITEM_MODULE);
        sd_contextItem(item)->name = scope->name;
    } else {
        item = (sd_contextItemScope)sd_contextItemNew(SD_CONTEXT_ITEM_SCOPE);
    }

    if ( item ) {
        sd_contextItem(item)->self  = scope;
        sd_contextItem(item)->scope = c_metaObject(scope->definedIn);
        sd_listAppend(context->scopeList, item);
    }


    return sd_contextItem(item);
}

typedef struct {
    c_metaObject   scope;
    sd_contextItem item;
} findScopeItemArg;

static c_bool
findScopeItem (
    void *obj,
    void *arg)
{
    sd_contextItem    item = (sd_contextItem)obj;
    findScopeItemArg *info = (findScopeItemArg *)arg;
    c_bool            retval = TRUE;

    if ( item->self == info->scope ) {
        info->item = item;
        retval = FALSE;
    }

    return retval;
}



static sd_contextItem
sd_contextFindScope (
    sd_context context,
    c_metaObject scope)
{
    findScopeItemArg arg;

    assert(context);
    assert(scope);

    arg.scope = scope;
    arg.item  = NULL;

    sd_listWalk(context->scopeList, findScopeItem, &arg);

    return arg.item;
}

static sd_contextItem
sd_contextFindOrCreateScope (
    sd_context context,
    c_metaObject scope)
{
    sd_contextItem item = NULL;
    sd_contextItem parent;
    c_metaObject   pscope;

    item = sd_contextFindScope(context, scope);
    if ( !item ) {
        if ( scope != context->base ) {
            pscope = c_metaObject(scope->definedIn);
            parent = sd_contextFindOrCreateScope(context, pscope);
            if ( parent ) {
                item = sd_contextAddScope(context, scope);
                if ( item ) {
                    sd_contextItemAddChild(parent, item);
                }
            }
        } else {
            item = sd_contextAddScope(context, scope);
        }
    }
    return item;
}

static c_bool
sd_objectInScope (
    sd_context   context,
    c_metaObject scope)
{
    c_bool result = FALSE;

    if ( scope == context->current->self ) {
        result = TRUE;
    } else if ( scope && (c_baseObject(scope)->kind != M_MODULE) ) {
         result = TRUE;
    } else {
         result = FALSE;
    }

    return result;
}

static c_bool
sd_contextItemIsTop (
    sd_contextItem item)
{
    c_bool result = FALSE;

    switch ( item->kind ) {
        case SD_CONTEXT_ITEM_SCOPE:
        case SD_CONTEXT_ITEM_MODULE:
        case SD_CONTEXT_ITEM_STRUCTURE:
        case SD_CONTEXT_ITEM_TYPEDEF:
        case SD_CONTEXT_ITEM_ENUMERATION:
        case SD_CONTEXT_ITEM_UNION:
            result = TRUE;
            break;
        default:
            result = FALSE;
            break;
    }

    return result;
}

static void
sd_addDependency (
    sd_context     context,
    sd_contextItem item,
    c_metaObject   object)
{
    sd_contextItem root;
    sd_contextItem referred = NULL;
    sd_contextItem ancestor = NULL;

    assert(context);
    assert(item);
    assert(object);

    root = (sd_contextItem)sd_contextFindScope(context, context->base);
    assert(root);

    if ( root ) {
        referred = sd_contextItemFindObject(root, object);
        assert(referred);
        if ( referred && (referred != item) ) {
            ancestor = sd_contextItemFindAncestor(item, referred);
            assert(ancestor);
        }
    }

    if ( ancestor ) {
        while ( item->parent && (item->parent != ancestor) ) {
            item = item->parent;
        }
        while ( referred->parent && (referred->parent != ancestor) ) {
            referred = referred->parent;
        }
        assert(item->parent == ancestor);
        assert(referred->parent == ancestor);

        if ( referred != item ) {
            sd_contextItemAddDependency(item, referred);
        }
    }
}


static void
sd_serializeType (
    sd_context context,
    c_type type,
    c_object object);

static void
sd_serializeLabel (
    sd_context context,
    c_constant label,
    c_object object)
{
    sd_contextItemLabel item;
    c_char *name;

    item = (sd_contextItemLabel)sd_contextItemNew(SD_CONTEXT_ITEM_LABEL);

    if ( item ) {
        name = c_metaObject(label)->name;

        sd_contextItem(item)->name = name;

        sd_contextItemAddChild(context->current, sd_contextItem(item));
        sd_contextItemFree(sd_contextItem(item));
    }
}

static void
sd_serializeLiteral (
    sd_context context,
    c_literal  literal,
    c_type     switchType,
    c_object   object)
{
    sd_contextItemLiteral item;

    item = (sd_contextItemLiteral)sd_contextItemNew(SD_CONTEXT_ITEM_LITERAL);

    if ( item ) {
        sd_contextItem(item)->name = "label";
        item->value = literal->value;
        item->type  = switchType;

        sd_contextItemAddChild(context->current, sd_contextItem(item));
        sd_contextItemFree(sd_contextItem(item));
    }
}


static void
sd_serializeMember (
    sd_context context,
    c_member member,
    c_object object)
{
    sd_contextItemMember item;
    c_char *name;
    c_type type;

    item = (sd_contextItemMember)sd_contextItemNew(SD_CONTEXT_ITEM_MEMBER);

    if ( item ) {
        name = c_specifier(member)->name;
        type = c_specifier(member)->type;

        sd_contextItem(item)->self = context->current->self;
        sd_contextItem(item)->name = name;

        sd_contextItemAddChild(context->current, sd_contextItem(item));

        sd_contextPushItem(context, sd_contextItem(item));

        sd_serializeType(context, c_getType(type), type);

        sd_contextPopItem(context);
        sd_contextItemFree(sd_contextItem(item));
    }
}


static void
sd_serializeStructure (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItemStructure item;
    sd_contextItem parent;
    sd_contextItem typeRef;
    c_property property;
    c_object o;
    c_metaObject scope;
    c_array members;
    c_ulong size;
    c_ulong i;
    c_char *name;

    assert(type);
    assert(object);

    property = (c_property)c_metaResolve(c_object(type), "definedIn");
    o = (c_object)C_DISPLACE(object, property->offset);
    scope = (c_metaObject)*(c_object *)o;

    if ( sd_objectInScope(context, scope) ) {
        sd_contextAddProcessed(context, object);

        item = (sd_contextItemStructure)sd_contextItemNew(SD_CONTEXT_ITEM_STRUCTURE);
        if ( item ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            name = (c_char *)*(c_object *)o;

            property = (c_property)c_metaResolve(c_object(type), "members");
            o = (c_object)C_DISPLACE(object, property->offset);
            members = (c_array)*(c_object *)o;

            sd_contextItem(item)->self = c_metaObject(object);
            sd_contextItem(item)->name = name;
            sd_contextItem(item)->scope = scope;

            sd_contextItemAddChild(context->current, sd_contextItem(item));

            sd_contextPushItem(context, sd_contextItem(item));

            size = c_arraySize(members);

            for ( i = 0; i < size; i++ ) {
                c_member m = (c_member)members[i];
                sd_serializeMember(context, m, object);
            }

            sd_contextPopItem(context);

            sd_contextItemFree(sd_contextItem(item));
        }
    } else {
        if ( !sd_contextIsProcessed(context, object) ) {
            parent = (sd_contextItem)sd_contextFindOrCreateScope(context, scope);
            if ( parent ) {
                sd_contextPushItem(context, parent);
                sd_serializeStructure(context, type, object);
                sd_contextPopItem(context);
            }
        }
        sd_addDependency(context, context->current, object);
        typeRef = sd_contextItemNew(SD_CONTEXT_ITEM_TYPE);
        if ( typeRef ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            typeRef->name = (c_char *)*(c_object *)o;
            typeRef->scope = scope;
            sd_contextItemAddChild(context->current, typeRef);
            sd_contextItemFree(sd_contextItem(typeRef));
        }
    }
}

static void
metaWalkAction_serializeMetaObject (
        c_metaObject metaObject,
        c_metaWalkActionArg arg)
{
    sd_serializeType((sd_context)arg, c_getType(metaObject), metaObject);
}


static void
sd_serializeModule (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItemStructure item;
    sd_contextItem parent;
    sd_contextItem typeRef;
    c_property property;
    c_object o;
    c_metaObject scope;
    c_char *name;

    assert(type);
    assert(object);

    property = (c_property)c_metaResolve(c_object(type), "definedIn");
    o = (c_object)C_DISPLACE(object, property->offset);
    scope = (c_metaObject)*(c_object *)o;
    c_free(property);

    if ( sd_objectInScope(context, scope) ) {
        sd_contextAddProcessed(context, object);

        item = (sd_contextItemStructure)sd_contextItemNew(SD_CONTEXT_ITEM_MODULE);
        if ( item ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            name = (c_char *)*(c_object *)o;
            c_free(property);

            sd_contextItem(item)->self = c_metaObject(object);
            sd_contextItem(item)->name = name;
            sd_contextItem(item)->scope = scope;

            sd_contextItemAddChild(context->current, sd_contextItem(item));

            sd_contextPushItem(context, sd_contextItem(item));

            /* serialize all object contained in this module */
            c_metaWalk(object, metaWalkAction_serializeMetaObject, (void*)context);

            sd_contextPopItem(context);

            sd_contextItemFree(sd_contextItem(item));
        }
        else
        {
            /* todo: OS_REPORT() */
        }
    } else {
        if ( !sd_contextIsProcessed(context, object) ) {
            parent = (sd_contextItem)sd_contextFindOrCreateScope(context, scope);
            if ( parent ) {
                sd_contextPushItem(context, parent);
                sd_serializeModule(context, type, object);
                sd_contextPopItem(context);
            }
        }
        sd_addDependency(context, context->current, object);
        typeRef = sd_contextItemNew(SD_CONTEXT_ITEM_TYPE);
        if ( typeRef ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            typeRef->name = (c_char *)*(c_object *)o;
            typeRef->scope = scope;
            sd_contextItemAddChild(context->current, typeRef);
            sd_contextItemFree(sd_contextItem(typeRef));
        }
    }
}

static void
sd_serializeTyperef (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItem typeRef;
    c_property property;
    c_object o;

    assert(type);
    assert(object);

    typeRef = sd_contextItemNew(SD_CONTEXT_ITEM_TYPE);
    if ( typeRef ) {
        property = (c_property)c_metaResolve(c_object(type), "name");
        o = (c_object)C_DISPLACE(object, property->offset);
        typeRef->name = (c_char *)*(c_object *)o;

        property = (c_property)c_metaResolve(c_object(type), "definedIn");
        o = (c_object)C_DISPLACE(object, property->offset);
        typeRef->scope = (c_metaObject)*(c_object *)o;

        sd_contextItemAddChild(context->current, typeRef);
        sd_contextItemFree(sd_contextItem(typeRef));
    }
}

static void
sd_serializeTypedef (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItemTypedef item;
    sd_contextItem parent;
    sd_contextItem typeRef;
    c_type alias;
    c_property property;
    c_object o;
    c_metaObject scope;
    c_char *name;

    assert(type);
    assert(object);

    property = (c_property)c_metaResolve(c_object(type), "definedIn");
    o = (c_object)C_DISPLACE(object, property->offset);
    scope = (c_metaObject)*(c_object *)o;

    if ( sd_objectInScope(context, scope) ) {
        sd_contextAddProcessed(context, object);
        item = (sd_contextItemTypedef)sd_contextItemNew(SD_CONTEXT_ITEM_TYPEDEF);
        if ( item ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            name = (c_char *)*(c_object *)o;

            property = (c_property)c_metaResolve(c_object(type), "alias");
            o = (c_object)C_DISPLACE(object, property->offset);
            alias = (c_type)*(c_object *)o;

            sd_contextItem(item)->self = c_metaObject(object);
            sd_contextItem(item)->name = name;
            sd_contextItem(item)->scope = scope;

            sd_contextItemAddChild(context->current, sd_contextItem(item));

            sd_contextPushItem(context, sd_contextItem(item));

            sd_serializeType(context, c_getType(alias), alias);

            sd_contextPopItem(context);

            sd_contextItemFree(sd_contextItem(item));
        }
    } else {
        if ( !sd_contextIsProcessed(context, object) ) {
            parent = (sd_contextItem)sd_contextFindOrCreateScope(context, scope);
            if ( parent ) {
                sd_contextPushItem(context, parent);
                sd_serializeTypedef(context, type, object);
                sd_contextPopItem(context);
            }
        }
        sd_addDependency(context, context->current, object);
        typeRef = sd_contextItemNew(SD_CONTEXT_ITEM_TYPE);
        if ( typeRef ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            typeRef->name = (c_char *)*(c_object *)o;
            typeRef->scope = scope;
            sd_contextItemAddChild(context->current, typeRef);
            sd_contextItemFree(sd_contextItem(typeRef));
        }
    }
}

static void
sd_serializePrimitive (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItemPrimitive item;
    c_property property;
    c_object o;
    c_primKind kind;

    assert(type);
    assert(object);

    item = (sd_contextItemPrimitive) sd_contextItemNew(SD_CONTEXT_ITEM_PRIMITIVE);

    if ( item ) {
        sd_contextItemAddChild(context->current, sd_contextItem(item));

        property = (c_property)c_metaResolve(c_object(type), "kind");
        o = (c_object)C_DISPLACE(object, property->offset);
        kind = *(c_primKind *)(c_object *)o;

        item->kind  = kind;
        sd_contextItemFree(sd_contextItem(item));
    }
}

static void
sd_serializeCollection (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItem item;
    c_property property;
    c_object o;
    c_collKind kind;
    c_ulong maxSize;
    c_type subType;
    c_char *name;
    c_metaObject scope;

    property = (c_property)c_metaResolve(c_object(type), "definedIn");
    o = (c_object)C_DISPLACE(object, property->offset);
    scope = (c_metaObject)*(c_object *)o;

        property = (c_property)c_metaResolve(c_object(type), "name");
        o = (c_object)C_DISPLACE(object, property->offset);
        name = (c_char *)*(c_object *)o;

        property = (c_property)c_metaResolve(c_object(type), "kind");
        o = (c_object)C_DISPLACE(object, property->offset);
        kind = *(c_collKind *)(c_object *)o;

        property = (c_property)c_metaResolve(c_object(type), "maxSize");
        o = (c_object)C_DISPLACE(object, property->offset);
        maxSize = *(c_ulong *)(c_object *)o;

        property = (c_property)c_metaResolve(c_object(type), "subType");
        o = (c_object)C_DISPLACE(object, property->offset);
        subType = *(c_type *)(c_object *)o;

        property = (c_property)c_metaResolve(c_object(type), "definedIn");
        o = (c_object)C_DISPLACE(object, property->offset);
        scope = (c_metaObject)*(c_object *)o;


        switch ( kind ) {
            case C_STRING:
                item = sd_contextItemNew(SD_CONTEXT_ITEM_STRING);
                break;
            case C_ARRAY:
                item = sd_contextItemNew(SD_CONTEXT_ITEM_ARRAY);
                break;
            case C_SEQUENCE:
                item = sd_contextItemNew(SD_CONTEXT_ITEM_SEQUENCE);
                break;
            default:
                item = sd_contextItemNew(SD_CONTEXT_ITEM_COLLECTION);
                break;
        }

        if ( item ) {
            sd_contextItem(item)->self = context->current->self;
            sd_contextItem(item)->name = name;
            sd_contextItemCollection(item)->maxSize = maxSize;

            sd_contextItemAddChild(context->current, sd_contextItem(item));

            sd_contextPushItem(context, sd_contextItem(item));
            sd_serializeType(context, c_getType(subType), subType);
            sd_contextPopItem(context);

            sd_contextItemFree(sd_contextItem(item));
        }
}

static void
sd_serializeEnumeration (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItemEnumeration item;
    sd_contextItem parent;
    sd_contextItem typeRef;
    c_property property;
    c_object o;
    c_metaObject scope;
    c_array elements;
    c_ulong size;
    c_ulong i;
    c_char *name;

    assert(type);
    assert(object);

    property = (c_property)c_metaResolve(c_object(type), "definedIn");
    o = (c_object)C_DISPLACE(object, property->offset);
    scope = (c_metaObject)*(c_object *)o;

    if ( sd_objectInScope(context, scope) ) {
        sd_contextAddProcessed(context, object);
        item = (sd_contextItemEnumeration)sd_contextItemNew(SD_CONTEXT_ITEM_ENUMERATION);
        if ( item ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            name = (c_char *)*(c_object *)o;

            property = (c_property)c_metaResolve(c_object(type), "elements");
            o = (c_object)C_DISPLACE(object, property->offset);
            elements = (c_array)*(c_object *)o;

            sd_contextItem(item)->self = c_metaObject(object);
            sd_contextItem(item)->name = name;
            sd_contextItem(item)->scope = scope;

            sd_contextItemAddChild(context->current, sd_contextItem(item));

            sd_contextPushItem(context, sd_contextItem(item));

            size = c_arraySize(elements);

            for ( i = 0; i < size; i++ ) {
                c_constant constant = (c_constant)elements[i];
                sd_serializeLabel(context, constant, object);
            }

            sd_contextPopItem(context);

            sd_contextItemFree(sd_contextItem(item));
        }
    } else {
        if ( !sd_contextIsProcessed(context, object) ) {
            parent = (sd_contextItem)sd_contextFindOrCreateScope(context, scope);
            if ( parent ) {
                sd_contextPushItem(context, parent);
                sd_serializeEnumeration(context, type, object);
                sd_contextPopItem(context);
            }
        }
        sd_addDependency(context, context->current, object);
        typeRef = sd_contextItemNew(SD_CONTEXT_ITEM_TYPE);
        if ( typeRef ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            typeRef->name = (c_char *)*(c_object *)o;
            typeRef->scope = scope;
            sd_contextItemAddChild(context->current, typeRef);
            sd_contextItemFree(sd_contextItem(typeRef));
        }
    }
}

static void
sd_serializeUnionSwitch (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItemUnionSwitch item;

    item = (sd_contextItemUnionSwitch)sd_contextItemNew(SD_CONTEXT_ITEM_UNIONSWITCH);

    if ( item ) {
        sd_contextItem(item)->self = context->current->self;
        sd_contextItem(item)->name = NULL;

        sd_contextItemAddChild(context->current, sd_contextItem(item));

        sd_contextPushItem(context, sd_contextItem(item));

        sd_serializeType(context, type, object);

        sd_contextPopItem(context);
        sd_contextItemFree(sd_contextItem(item));
    }

}

static void
sd_serializeUnionCase (
    sd_context  context,
    c_unionCase unionCase,
    c_type      switchType,
    c_object    object)
{
    sd_contextItemUnionCase item;
    sd_contextItemLiteral deflabel;
    c_char *name;
    c_type type;
    c_ulong i, size;
    c_array labels;

    item = (sd_contextItemUnionCase)sd_contextItemNew(SD_CONTEXT_ITEM_UNIONCASE);

    if ( item ) {
        name = c_specifier(unionCase)->name;
        type = c_specifier(unionCase)->type;
        labels = unionCase->labels;

        sd_contextItem(item)->self = context->current->self;
        sd_contextItem(item)->name = name;

        sd_contextItemAddChild(context->current, sd_contextItem(item));

        sd_contextPushItem(context, sd_contextItem(item));

        sd_serializeType(context, c_getType(type), type);

        if ( labels ) {
            size = c_arraySize(labels);

            for ( i = 0; i < size; i++ ) {
                c_literal literal = (c_literal)labels[i];
                sd_serializeLiteral(context, literal, switchType, object);
            }
        } else {
            deflabel = (sd_contextItemLiteral)sd_contextItemNew(SD_CONTEXT_ITEM_LITERAL);
            if ( deflabel ) {
                sd_contextItemAddChild(context->current, sd_contextItem(deflabel));
                sd_contextItemFree(sd_contextItem(deflabel));
            }
        }

        sd_contextPopItem(context);
        sd_contextItemFree(sd_contextItem(item));
    }
}

static void
sd_serializeUnion (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItemUnion item;
    sd_contextItem parent;
    sd_contextItem typeRef;
    c_property property;
    c_object o;
    c_metaObject scope;
    c_array cases;
    c_ulong size;
    c_ulong i;
    c_char *name;
    c_type switchType;

    assert(type);
    assert(object);

    property = (c_property)c_metaResolve(c_object(type), "definedIn");
    o = (c_object)C_DISPLACE(object, property->offset);
    scope = (c_metaObject)*(c_object *)o;

    if ( sd_objectInScope(context, scope) ) {
        sd_contextAddProcessed(context, object);
        item = (sd_contextItemUnion)sd_contextItemNew(SD_CONTEXT_ITEM_UNION);
        if ( item ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            name = (c_char *)*(c_object *)o;

            property = (c_property)c_metaResolve(c_object(type), "switchType");
            o = (c_object)C_DISPLACE(object, property->offset);
            switchType = (c_type)*(c_object *)o;

            property = (c_property)c_metaResolve(c_object(type), "cases");
            o = (c_object)C_DISPLACE(object, property->offset);
            cases = (c_array)*(c_object *)o;

            sd_contextItem(item)->self = c_metaObject(object);
            sd_contextItem(item)->name = name;
            sd_contextItem(item)->scope = scope;

            sd_contextItemAddChild(context->current, sd_contextItem(item));

            sd_contextPushItem(context, sd_contextItem(item));

            sd_serializeUnionSwitch(context, c_getType(switchType), switchType);

            size = c_arraySize(cases);

            for ( i = 0; i < size; i++ ) {
                c_unionCase unionCase = (c_unionCase)cases[i];
                sd_serializeUnionCase(context, unionCase, switchType, object);
            }

            sd_contextPopItem(context);
            sd_contextItemFree(sd_contextItem(item));
        }
    } else {
        if ( !sd_contextIsProcessed(context, object) ) {
            parent = (sd_contextItem)sd_contextFindOrCreateScope(context, scope);
            if ( parent ) {
                sd_contextPushItem(context, parent);
                sd_serializeUnion(context, type, object);
                sd_contextPopItem(context);
            }
        }
        sd_addDependency(context, context->current, object);
        typeRef = sd_contextItemNew(SD_CONTEXT_ITEM_TYPE);
        if ( typeRef ) {
            property = (c_property)c_metaResolve(c_object(type), "name");
            o = (c_object)C_DISPLACE(object, property->offset);
            typeRef->name = (c_char *)*(c_object *)o;
            typeRef->scope = scope;
            sd_contextItemAddChild(context->current, typeRef);
            sd_contextItemFree(sd_contextItem(typeRef));
        }
    }
}


static void
sd_serializeType (
    sd_context context,
    c_type type,
    c_object object)
{
    sd_contextItem item;
    c_property property;
    c_object o;
    c_ulong maxSize;

    if ( object == context->specialTypes.c_string ) {
        item = sd_contextItemNew(SD_CONTEXT_ITEM_STRING);

        property = (c_property)c_metaResolve(c_object(type), "maxSize");
        o = (c_object)C_DISPLACE(object, property->offset);
        maxSize = *(c_ulong *)(c_object *)o;

        sd_contextItemCollection(item)->maxSize = maxSize;
        sd_contextItemAddChild(context->current, sd_contextItem(item));
        sd_contextItemFree(item);
    } else if ( object == context->specialTypes.c_time ) {
        item = sd_contextItemNew(SD_CONTEXT_ITEM_TIME);
        sd_contextItemAddChild(context->current, sd_contextItem(item));
        sd_contextItemFree(item);
    } else {
        if ( !sd_contextIsProcessed(context, object) ) {
            if ( c_baseObject(type)->kind == M_CLASS) {
                switch ( c_baseObject(object)->kind ) {
                case M_STRUCTURE:
                    sd_serializeStructure(context, type, object);
                    break;
                case M_TYPEDEF:
                    sd_serializeTypedef(context, type, object);
                    break;
                case M_PRIMITIVE:
                    /*
                     * when the container object is a module, then only
                     * modules, typedefs, enums, unions and structures
                     * can be serialized (see S142 spec).
                     */
                    if(context->current->kind != SD_CONTEXT_ITEM_MODULE)
                    {
                        sd_serializePrimitive(context, type, object);
                    }
                    break;
                case M_COLLECTION:
                   /*
                    * do not serialize if container object is a module (see explanation
                    * above).
                    */
                    if(context->current->kind != SD_CONTEXT_ITEM_MODULE)
                    {
                        sd_serializeCollection(context, type, object);
                    }
                    break;
                case M_ENUMERATION:
                    sd_serializeEnumeration(context, type, object);
                    break;
                case M_UNION:
                    sd_serializeUnion(context, type, object);
                    break;
                case M_MODULE:
                    sd_serializeModule(context, type, object);
                    break;
                case M_CONSTANT:
                    /*
                     * In IDL constants can only be declared in modules (toplevel module too) and
                     * these are intrepreted as #define pre-processor definitions.
                     * In the S142 XML definition constants cannot be declared. Therefore
                     * any constants that are encountered are those of enumerations, and
                     * these should be ignored, as they are serialized when
                     * the enumeration is serialized.
                     */
                    break;
                default:
                    OS_REPORT_1(OS_ERROR,
                            "sd_serializeType",
                            0, "sd_serializeType does not support type %d",
                            c_baseObject(type)->kind);
                    assert(0);
                    break;
                }
            } else {
                assert(0);
            }
        } else {
            sd_addDependency(context, context->current, object);
            sd_serializeTyperef(context, type, object);
        }
    }
}

static c_bool
findItemWithoutDependencies (
    void *o,
    void *arg)
{
    c_bool found = FALSE;
    sd_contextItem item = (sd_contextItem) o;

    if ( !sd_contextItemHasDependencies(item) ) {
        found = TRUE;
    }

    return found;
}

static c_bool
removeDependency (
    sd_contextItem item,
    void           *arg)
{
    sd_contextItem referred = (sd_contextItem) arg;

    assert(referred);

    sd_contextItemRemoveDependency(item, referred);

    return TRUE;
}

static c_bool
sd_serializeReorderModules (
    sd_contextItem item,
    void           *arg)
{
    sd_list        modules = NULL;
    sd_contextItem child;
    c_bool         ready   = FALSE;

    assert(item);

    if ( sd_contextItemIsTop(item) ) {
        if ( item->children ) {
            sd_contextItemWalkChildren(item, sd_serializeReorderModules, NULL);
            modules = sd_listNew();
            if ( modules ) {
                while ( !ready && !sd_listIsEmpty(item->children) ) {
                    child = sd_listFind(item->children, findItemWithoutDependencies, NULL);
                    if ( child ) {
                        sd_listRemove(item->children, child);
                        sd_listAppend(modules, child);
                        /* remove dependencies */
                        sd_contextItemWalkChildren(item, removeDependency, child);
                    } else {
                        ready = TRUE;
                    }
                }
                assert(sd_listIsEmpty(item->children));
                sd_listFree(item->children);
                item->children = modules;
            }
        }
    }

    return TRUE;
}

static sd_serializedData
sd_serializerXMLTypeinfoSerialize(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result = NULL;
    c_ulong size;
    c_type type;
    c_type actualType;
    c_metaObject module;
    sd_contextItem mitem;
    sd_context context;
    c_object base;
    c_bool escapeQuote;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    escapeQuote = ((sd_serializerXMLTypeinfo)serializer)->escapeQuote;

    base = (c_object)c_getBase(object);

    context = sd_contextNew(base);

    if ( context ) {
        actualType = (c_type)object;

        type = c_getType(object);
        SD_CONFIDENCE(c_metaObject(type)->name != NULL);

        module = c_metaObject(c_metaObject(actualType)->definedIn);

        mitem = (sd_contextItem)sd_contextFindOrCreateScope(context, module);

        sd_contextPushItem(context, mitem);

        sd_serializeType(context, type, object);

        sd_contextPopItem(context);

        sd_serializeReorderModules(sd_contextFindOrCreateScope(context, base), NULL);

        mitem = (sd_contextItem)sd_contextFindOrCreateScope(context, base);

        size = sd_printXmlTypeinfoLength(mitem, escapeQuote);

        result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size);

        sd_printXmlTypeinfo(mitem, (c_char *)result->data, escapeQuote);

        sd_contextFree(context);
    }

    return result;
}


/* ------------------- Special deserialization action routines -------------- */

static c_bool
sd_handleTypeElement (
    sd_typeInfoKind    kind,
    c_char            *name,
    sd_list            attributes,
    void              *argument,
    sd_typeInfoHandle  handle);

typedef struct sd_contextInfo_s {
    c_metaObject   base;
    c_type         objectType;
    sd_errorReport errorInfo;
} sd_contextInfo;

C_CLASS(sd_elementContext);
C_STRUCT(sd_elementContext) {
    sd_contextInfo   *info;
    c_char           *name;
    c_metaObject      object;
    sd_elementContext parent;
    sd_list           children;
};


static void
sd_elementContextFree (
    sd_elementContext context)
{
    if ( context->name ) {
        os_free(context->name);
    }

    if ( context->children ) {
        while ( !sd_listIsEmpty(context->children) ) {
            sd_elementContext child = (sd_elementContext) sd_listTakeFirst(context->children);
            sd_elementContextFree(child);
        }
        sd_listFree(context->children);
    }

    os_free(context);
}

static sd_elementContext
sd_elementContextNew (
    sd_contextInfo   *info,
    const c_char     *name,
    c_metaObject      object,
    sd_elementContext parent,
    c_bool            hasChildren)
{
    sd_elementContext context;

    context = (sd_elementContext) os_malloc(C_SIZEOF(sd_elementContext));
    if ( context ) {
        context->info = info;
        if ( name ) {
            context->name = sd_stringDup(name);
            if ( !context->name ) {
                sd_elementContextFree(context);
                context = NULL;
            }
        } else {
            context->name = NULL;
        }
    }

    if ( context ) {
        context->object = object;
        context->parent = parent;

        if ( hasChildren ) {
            context->children = sd_listNew();
            if ( !context->children ) {
                sd_elementContextFree(context);
                context = NULL;
            }
        } else {
            context->children = NULL;
        }
    }

    if ( context && parent ) {
        assert(parent->children);
        sd_listAppend(parent->children, context);
    }

    return context;
}

static sd_elementContext
sd_elementContextFindScope (
    sd_elementContext context)
{
    sd_elementContext scope   = NULL;
    sd_elementContext element = context;

    while ( !scope && element ) {
        switch ( c_baseObject(element->object)->kind ) {
            case M_MODULE:
            case M_STRUCTURE:
            case M_UNION:
                scope = element;
                break;
            default:
                element = element->parent;
                break;
        }
    }

    return scope;
}


static c_metaObject
sd_elementContextGetScope (
    sd_elementContext context)
{
    c_metaObject      scope   = NULL;
    sd_elementContext element;

    element = sd_elementContextFindScope(context);
    if ( element ) {
        scope = element->object;
    }

    return scope;
}



static const c_char *
sd_findPrimitiveName (
    sd_typeInfoKind kind)
{
    const c_char *name;

#define _CASE_(k,s) case k: name = #s; break;
    switch ( kind ) {
        _CASE_(SD_TYPEINFO_KIND_STRING,    c_string);
        _CASE_(SD_TYPEINFO_KIND_CHAR,      c_char);
        _CASE_(SD_TYPEINFO_KIND_BOOLEAN,   c_bool);
        _CASE_(SD_TYPEINFO_KIND_OCTET,     c_octet);
        _CASE_(SD_TYPEINFO_KIND_SHORT,     c_short);
        _CASE_(SD_TYPEINFO_KIND_USHORT,    c_ushort);
        _CASE_(SD_TYPEINFO_KIND_LONG,      c_long);
        _CASE_(SD_TYPEINFO_KIND_ULONG,     c_ulong);
        _CASE_(SD_TYPEINFO_KIND_LONGLONG,  c_longlong);
        _CASE_(SD_TYPEINFO_KIND_ULONGLONG, c_ulonglong);
        _CASE_(SD_TYPEINFO_KIND_FLOAT,     c_float);
        _CASE_(SD_TYPEINFO_KIND_DOUBLE,    c_double);
        default:
           name = "";
    }
#undef _CASE_

    return name;
}

static void
sd_deserContextSetError (
    sd_elementContext  context,
    const char        *element,
    const char        *name,
    c_ulong            errorNumber,
    const c_char      *message)
{
    sd_string location;

    if ( context->info->errorInfo ) {
        return;
    }

    location = sd_stringNew(256);

    if ( element ) {
        if ( name ) {
            sd_stringAdd(location, "<%s name=\"%s\">", element, name);
        } else {
            sd_stringAdd(location, "<%s>", element);
        }
    }

    context->info->errorInfo = sd_errorReportNew(errorNumber, message, sd_stringContents(location));

    sd_stringFree(location);
}

static c_bool
sd_deserXmlModule (
    sd_elementContext  parent,
    const c_char      *name,
    sd_typeInfoHandle  handle)
{
    c_bool            result = FALSE;
    sd_elementContext element;
    c_metaObject      o;
    c_metaObject      scope;

    if ( name ) {
        scope = sd_elementContextGetScope(parent);
        o = c_metaResolveFixedScope(scope, name);
        if ( !o ) {
            o = c_metaDeclare(scope, name, M_MODULE);
            if ( !o ) {
                c_char *scopeName = c_metaScopedName(scope);
                OS_REPORT_2(OS_ERROR,"sd_deserXmlModule",0,
                            "c_metaDeclare failed for Module <%s> in scope <%s>.",
                            name, scopeName);
                os_free(scopeName);
            }
        }

        if ( o ) {
            element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
            if ( element ) {
                result = sd_typeInfoParserNext(handle, sd_handleTypeElement, element);
            } else {
                OS_REPORT(OS_ERROR,"sd_deserXmlModule",0,
                          "Internal error (out of resources) : "
                          "Allocation context element failed.");
            }
        }
    } else {
        result = sd_typeInfoParserNext(handle, sd_handleTypeElement, parent);
    }

    return result;
}


static c_bool
sd_deserXmlPrimitive (
    sd_elementContext  parent,
    sd_typeInfoKind    kind)
{
    c_bool            result = FALSE;
    sd_elementContext element;
    c_metaObject      o;
    const c_char     *name;

    name = sd_findPrimitiveName(kind);

    if (name != NULL) {
        o = c_metaResolve(parent->info->base, name);
        if ( o ) {
            element = sd_elementContextNew(parent->info, NULL, o, parent, FALSE);
            if ( element ) {
                result = TRUE;
            } else {
                OS_REPORT(OS_ERROR,"sd_deserXmlPrimitive",0,
                          "Internal error (out of resources) : "
                          "Allocation context element failed");
            }
        } else {
            OS_REPORT_1(OS_ERROR,"sd_deserXmlPrimitive",0,
                        "Internal error : "
                        "resolve meta data for primitive type <%s> failed.",
                        name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"sd_deserXmlPrimitive",0,
                    "Internal error : Primitive type name not found for kind <%d>.",
                    kind);
    }
    return result;
}

static c_bool
sd_deserXmlTime (
    sd_elementContext  parent,
    sd_typeInfoKind    kind)
{
    c_bool            result = FALSE;
    sd_elementContext element;
    c_metaObject      o;

    o = c_metaResolve(parent->info->base, "c_time");
    if ( o ) {
        element = sd_elementContextNew(parent->info, NULL, o, parent, FALSE);
        if ( element ) {
            result = TRUE;
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlTime",0,
                      "Internal error (out of resources) : "
                      "Allocation context element failed.");
        }
    } else {
        OS_REPORT(OS_ERROR,"sd_deserXmlTime",0,
                  "Internal error : "
                  "resolve meta data for primitive type <c_time> failed.");
    }

    return result;
}

static c_bool
sd_deserXmlString (
    sd_elementContext parent,
    sd_list           attributes)
{
    c_bool            result = FALSE;
    sd_elementContext element;
    c_metaObject      o;
    c_type            type = NULL;
    c_ulong           length = 0;
    c_char            name[265];
    c_metaObject      scope;

    if ( !sd_listIsEmpty(attributes) ) {
        length = sd_findAttributeNumber(attributes, "length");
    }

    if ( length > 0 ) {
        scope = sd_elementContextGetScope(parent);
        o = c_metaObject(c_metaDefine(scope, M_COLLECTION));
        if ( o ) {
            c_collectionType(o)->kind = C_STRING;
            c_collectionType(o)->subType = c_keep(c_type(c_metaResolve(parent->info->base, "c_char")));
            c_collectionType(o)->maxSize = length;
            c_metaObject(o)->definedIn   = scope;
            c_metaFinalize(o);

            os_sprintf(name, "C_STRING<%d>", length);
            type = c_type(c_metaBind(scope, name, o));
            if ( !type ) {
                c_char *scopeName = c_metaScopedName(scope);
                OS_REPORT_2(OS_ERROR,"sd_deserXmlString",0,
                            "c_metaBind failed for String <%s> in scope <%s>.",
                            name, scopeName);
                os_free(scopeName);
            }
            c_free(o);
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlString",0,
                      "Internal error (out of resources) : "
                      "Allocation c_metaDefine failed.");
        }
    } else {
        type = c_type(c_metaResolve(parent->info->base, "c_string"));
        if ( !type ) {
            OS_REPORT(OS_ERROR,"sd_deserXmlPrimitive",0,
                      "Internal error : "
                      "resolve meta data for primitive type <c_string> failed.");
        }
    }

    if ( type ) {
        element = sd_elementContextNew(parent->info, NULL, c_metaObject(type), parent, FALSE);
        if ( element ) {
            result = TRUE;
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlString",0,
                      "Internal error (out of resources) : "
                      "Allocation context element failed.");
        }
    }

    return result;
}

static c_bool
sd_deserXmlArray (
    sd_elementContext parent,
    sd_list           attributes,
    sd_typeInfoHandle handle)
{
    c_bool            result  = FALSE;
    sd_elementContext element = NULL;
    c_metaObject      o       = NULL;
    c_metaObject      scope   = NULL;
    c_long            size    = 0;
    c_char            name[265];

    if ( !sd_listIsEmpty(attributes) ) {
        size = sd_findAttributeNumber(attributes, "size");
    }

    if ( size >= 0 ) {
        scope = sd_elementContextGetScope(parent);
        o = c_metaObject(c_metaDefine(scope, M_COLLECTION));
        if ( o ) {
            c_collectionType(o)->kind = C_ARRAY;
            element = sd_elementContextNew(parent->info, NULL, o, parent, TRUE);
            if ( element ) {
                if ( sd_typeInfoParserNext(handle, sd_handleTypeElement, element) ) {
                    c_type            subType;
                    sd_elementContext child = (sd_elementContext) sd_listTakeFirst(element->children);

                    assert(child);
                    subType = c_type(child->object);
                    c_collectionType(o)->subType = c_keep(subType);
                    c_collectionType(o)->maxSize = size;
                    c_metaObject(o)->definedIn   = scope;
                    c_metaFinalize(o);
                    if ( child->name ) {
                        os_sprintf(name, "C_ARRAY<%s,%d>", child->name, size);
                    } else if ( c_metaObject(subType)->name ) {
                        os_sprintf(name, "C_ARRAY<%s,%d>", c_metaObject(subType)->name, size);
                    } else {
                        os_sprintf(name, "C_ARRAY<NULL,%d>", size);
                    }

                    element->object = c_metaBind(scope, name, o);
                    if ( element->object ) {
                        result = TRUE;
                    } else {
                        c_char *scopeName = c_metaScopedName(scope);
                        OS_REPORT_2(OS_ERROR,"sd_deserXmlArray",0,
                                    "c_metaBind failed for Array <%s> in scope <%s>.",
                                    name, scopeName);
                        os_free(scopeName);
                    }
                    sd_elementContextFree(child);
                }
            } else {
                OS_REPORT(OS_ERROR,"sd_deserXmlArray",0,
                          "Internal error (out of resources) : "
                          "Allocation context element failed.");
            }
            c_free(o);
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlArray",0,
                      "Internal error (out of resources) : "
                      "Allocation c_metaDefine failed.");
        }
    } else {
        OS_REPORT_1(OS_ERROR,"sd_deserXmlArray",0,
                    "Illegal array size <%d> specified.",
                    size);
    }

    return result;
}

static c_bool
sd_deserXmlSequence (
    sd_elementContext parent,
    sd_list           attributes,
    sd_typeInfoHandle handle)
{
    c_bool            result  = FALSE;
    sd_elementContext element = NULL;
    c_metaObject      o       = NULL;
    c_metaObject      scope   = NULL;
    c_long            size    = 0;
    c_char            name[265];

    if ( !sd_listIsEmpty(attributes) ) {
        size = sd_findAttributeNumber(attributes, "size");
    }

    if ( size >= 0 ) {
        scope = sd_elementContextGetScope(parent);
        o = c_metaObject(c_metaDefine(scope, M_COLLECTION));
        if ( o ) {
            c_collectionType(o)->kind = C_SEQUENCE;
            element = sd_elementContextNew(parent->info, NULL, o, parent, TRUE);
            if ( element ) {
                if ( sd_typeInfoParserNext(handle, sd_handleTypeElement, element) ) {
                    c_type            subType;
                    sd_elementContext child = (sd_elementContext) sd_listTakeFirst(element->children);

                    assert(child);
                    subType = c_type(child->object);
                    c_collectionType(o)->subType = c_keep(subType);
                    c_collectionType(o)->maxSize = size;
                    c_metaObject(o)->definedIn   = scope;
                    c_metaFinalize(o);

                    if ( size > 0 ) {
                        if ( child->name ) {
                            os_sprintf(name, "C_SEQUENCE<%s,%d>", child->name, size);
                        } else if ( c_metaObject(subType)->name ) {
                            os_sprintf(name, "C_SEQUENCE<%s,%d>", c_metaObject(subType)->name, size);
                        } else {
                            os_sprintf(name, "C_SEQUENCE<NULL,%d>", size);
                        }
                    } else {
                        if ( child->name ) {
                            os_sprintf(name, "C_SEQUENCE<%s>", child->name);
                        } else if ( c_metaObject(subType)->name ) {
                            os_sprintf(name, "C_SEQUENCE<%s>", c_metaObject(subType)->name);
                        } else {
                            os_sprintf(name, "C_SEQUENCE<NULL>");
                        }
                    }

                    element->object = c_metaBind(scope, name, o);
                    if ( element->object ) {
                        result = TRUE;
                    } else {
                        c_char *scopeName = c_metaScopedName(scope);
                        OS_REPORT_2(OS_ERROR,"sd_deserXmlSequence",0,
                                    "c_metaBind failed for Sequence <%s> in scope <%s>.",
                                    name, scopeName);
                        os_free(scopeName);
                    }

                    sd_elementContextFree(child);
                }
            } else {
                OS_REPORT(OS_ERROR,"sd_deserXmlSequence",0,
                          "Internal error (out of resources) : "
                          "Allocation context element failed.");
            }
            c_free(o);
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlSequence",0,
                      "Internal error (out of resources) : "
                      "Allocation c_metaDefine failed.");
        }
    } else {
        OS_REPORT_1(OS_ERROR,"sd_deserXmlSequence",0,
                    "Illegal Sequence size <%d> specified.",
                    size);
    }

    return result;
}


static sd_elementContext
sd_findScopeRoot (
    sd_elementContext context,
    const c_char     *name)
{
    sd_elementContext root = NULL;

    assert(name);

    while ( !root && context ) {
        if ( context->name && (strcmp(context->name, name) == 0) ) {
            root = context;
        } else {
            context = context->parent;
        }
    }
    return root;
}

static c_metaObject
sd_findScopeInContext (
    sd_elementContext  context,
    const c_char       *name);

typedef struct sd_findScopeArg_s {
    const c_char  *name;
    c_metaObject  scope;
} sd_findScopeArg;

static c_bool
sd_findScopeInChild (
    void *obj,
    void *arg)
{
    sd_elementContext context = (sd_elementContext) obj;
    sd_findScopeArg  *info    = (sd_findScopeArg  *)arg;
    c_char           *str     = sd_stringDup(info->name);
    c_char           *cur;
    c_bool            proceed = TRUE;

    cur = strstr(str, "::");
    if ( cur ) {
        *cur = '\0';
        cur += 2;
    }

    if ( cur ) {
        if ( context->name && (strcmp(context->name, str) == 0) ) {
            info->scope = sd_findScopeInContext(context, cur);
            proceed = FALSE;
        }
    } else if ( context->name && (strcmp(context->name, str) == 0) ) {
        info->scope = context->object;
        proceed = FALSE;
    } else {
        c_metaKind kind = c_baseObject(context->object)->kind;
        if ( (kind == M_MEMBER) || (kind == M_UNIONCASE) ) {
            c_type type = c_specifier(context->object)->type;
            if ( type && (strcmp(c_metaObject(type)->name, str) == 0) ) {
                info->scope = c_metaObject(type);
                proceed = FALSE;
            }
        }
    }

    os_free(str);

    return proceed;
}


static c_metaObject
sd_findScopeInContext (
    sd_elementContext  context,
    const c_char       *name)
{
    sd_findScopeArg argument;

    argument.scope = NULL;
    argument.name  = name;

    if ( context->children ) {
        sd_listWalk(context->children, sd_findScopeInChild, &argument);
    }

    return argument.scope;
}


static c_metaObject
sd_findTypeInScope (
    sd_elementContext  scope,
    const c_char      *name)
{
    c_metaObject       o;

    /* First, lookup the name in the current scope in the database. */
    o = c_metaResolve(scope->object, name);
    if ( !o ) {
        /* If not found, look it up in the root scope of the database. */
        o = c_metaResolve(scope->info->base, name);
        if ( !o ) {
            /* If still not found, look it up in the root of the serializer scope. */
            /* This is appropriate in case of recursive references to self, where  */
            /* self is not yet inserted into the database.                         */
            sd_elementContext root = scope;
            while (root->parent != NULL) {
                root = root->parent;
            }
            o = sd_findScopeInContext(root, name);
        }
    }

    return o;
}

static c_bool
sd_deserXmlType (
    sd_elementContext  parent,
    const c_char      *name)
{
    c_bool            result   = FALSE;
    sd_elementContext element  = NULL;
    c_metaObject      o        = NULL;
    sd_elementContext scope    = NULL;

    scope = sd_elementContextFindScope(parent);
    assert(scope);

    o = sd_findTypeInScope(scope, name);
    if ( o ) {
        element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
        if ( element ) {
            result = TRUE;
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlType",0,
                      "Internal error (out of resources) : "
                      "Allocation context element failed.");
            c_free(o);
        }
    }

    return result;
}

static c_bool
sd_deserXmlTypedef (
    sd_elementContext parent,
    const c_char      *name,
    sd_typeInfoHandle  handle)
{
    c_bool            result   = FALSE;
    sd_elementContext element  = NULL;
    c_metaObject      o        = NULL;
    c_metaObject      scope    = NULL;

    scope = sd_elementContextGetScope(parent);
    assert(scope);

    o = c_metaObject(c_metaDefine(scope, M_TYPEDEF));
    if ( o ) {
        o->definedIn = scope;
        element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
        if ( element ) {
            if ( sd_typeInfoParserNext(handle, sd_handleTypeElement, element) ) {
                sd_elementContext child = (sd_elementContext) sd_listTakeFirst(element->children);
                assert(child);

                c_typeDef(o)->alias = c_type(child->object);
                c_metaFinalize(o);

                element->object = c_metaBind(scope, name, o);
                if ( element->object ) {
                    result = TRUE;
                } else {
                }

                sd_elementContextFree(child);
            }
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlTypedef",0,
                      "Internal error (out of resources) : "
                      "Allocation Typedef context element failed.");
        }
        c_free(o);
    } else {
        OS_REPORT(OS_ERROR,"sd_deserXmlTypedef",0,
                  "Internal error (out of resources) : "
                  "Allocation c_metaDefine failed.");
    }

    return result;
}

static c_bool
sd_deserXmlMember (
    sd_elementContext  parent,
    const c_char      *name,
    sd_typeInfoHandle  handle)
{
    c_metaObject      o;
    c_metaObject      scope;
    sd_elementContext element;
    c_bool            result = FALSE;

    scope = sd_elementContextGetScope(parent);
    assert(scope);

    o = c_metaObject(c_metaDefine(scope, M_MEMBER));
    if ( o ) {
        c_specifier(o)->name = c_stringNew(c_object(parent->info->base), name);
        element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
        if ( element ) {
            if ( sd_typeInfoParserNext(handle, sd_handleTypeElement, element) ) {
                sd_elementContext child = (sd_elementContext) sd_listTakeFirst(element->children);
                assert(child);
                c_specifier(o)->type = c_type(child->object);
                c_metaFinalize(o);
                sd_elementContextFree(child);
                result = TRUE;
            }
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlMember",0,
                      "Internal error (out of resources) : "
                      "Allocation Member context element failed.");
        }
    } else {
        OS_REPORT(OS_ERROR,"sd_deserXmlMember",0,
                  "Internal error (out of resources) : "
                  "Allocation c_metaDefine failed.");
    }

    return result;
}


static c_bool
sd_deserXmlStructure (
    sd_elementContext  parent,
    const c_char      *name,
    sd_typeInfoHandle  handle)
{
    c_bool            result  = FALSE;
    c_metaObject      o       = NULL;
    c_metaObject      scope   = NULL;
    sd_elementContext element;
    c_array           members = NULL;
    c_ulong           num     = 0;
    c_ulong           i       = 0;

    scope = sd_elementContextGetScope(parent);
    assert(scope);

    o = c_metaObject(c_metaDefine(scope, M_STRUCTURE));
    if ( o ) {
        o->definedIn = scope;
        element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
        if ( element ) {
            if ( sd_typeInfoParserNext(handle, sd_handleTypeElement, element) ) {
                num = sd_listSize(element->children);
                if ( num > 0 ) {
                    members = c_arrayNew(c_member_t((c_base)element->info->base),num);
                    if ( members ) {
                        while ( !sd_listIsEmpty(element->children) ) {
                            sd_elementContext child = (sd_elementContext)sd_listTakeFirst(element->children);
                            assert(child);
                            members[i] = child->object;
                            sd_elementContextFree(child);
                            i++;
                        }
                    } else {
                        OS_REPORT(OS_ERROR,"sd_deserXmlStructure",0,
                                  "Internal error (out of resources) : "
                                  "Allocation member array failed.");
                    }
                }

                c_structure(o)->members = members;
                c_metaFinalize(o);
                element->object = c_metaBind(scope, name, o);
                if ( element->object ) {
                    result = TRUE;
                } else {
                    c_char *scopeName = c_metaScopedName(scope);
                    OS_REPORT_2(OS_ERROR,"sd_deserXmlStructure",0,
                                "c_metaBind failed for Struct <%s> in scope <%s>.",
                                name, scopeName);
                    os_free(scopeName);
                }
            }
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlStructure",0,
                      "Internal error (out of resources) : "
                      "Allocation context element failed.");
        }
        c_free(o);
    } else {
        OS_REPORT(OS_ERROR,"sd_deserXmlStructure",0,
                  "Internal error (out of resources) : "
                  "Allocation c_metaDefine failed.");
    }

    return result;
}

static c_bool
checkEnumLabel (
    const char  *labelName,
    const char  *enumName,
    c_metaObject scope)
{
    c_bool result = TRUE;

    if ( (c_metaResolve(scope, labelName) != NULL) &&
         (c_metaResolve(scope, enumName)  == NULL) ) {
        result = FALSE;
    }

    return result;
}


static c_bool
sd_deserXmlElement (
    sd_elementContext  parent,
    const c_char      *name,
    sd_list            attributes)
{
    c_bool            result       = FALSE;
    c_metaObject      o            = NULL;
    c_metaObject      scope        = NULL;
    c_literal         l;
    sd_elementContext element      = NULL;
    c_long            value        = 0;
    c_bool            valueDefined = FALSE;

    scope = sd_elementContextGetScope(parent);
    assert(scope);

    if ( !sd_listIsEmpty(attributes) ) {
        value = sd_findAttributeNumber(attributes, "value");
        valueDefined = TRUE;
    }
    if ( checkEnumLabel(name, parent->name, scope) ) {
        o = c_metaObject(c_metaDeclare(scope, name, M_CONSTANT));
        if ( o ) {
            if ( valueDefined && c_constant(o)->operand == NULL) {
                l = c_literal(c_metaDefine(scope, M_LITERAL));
                l->value = c_longValue(value);
                c_constant(o)->operand = c_operand(l);
            }

            element = sd_elementContextNew(parent->info, name, o, parent, FALSE);
            if ( element ) {
                result = TRUE;
            } else {
                OS_REPORT(OS_ERROR,"sd_deserXmlElement",0,
                          "Internal error (out of resources) : "
                          "Allocation EnumLabel context element failed.");
                c_free(o);
            }
        } else {
            c_char *scopeName = c_metaScopedName(scope);
            OS_REPORT_2(OS_ERROR,"sd_deserXmlElement",0,
                      "c_metaDeclare of EnumLabel <%s> in scope <%s> failed.",
                      name, scopeName);
            os_free(scopeName);
        }
    } else {
        c_char *scopeName = c_metaScopedName(scope);
        OS_REPORT_3(OS_ERROR,"sd_deserXmlElement",0,
                  "Illegal EnumLabel <%s> for enum <%s> in scope <%s> failed.",
                  name, parent->name, scopeName);
        os_free(scopeName);
    }

    return result;
}

static c_bool
sd_deserXmlEnumeration (
    sd_elementContext  parent,
    const c_char      *name,
    sd_typeInfoHandle  handle)
{
    c_bool            result   = FALSE;
    c_metaObject      o        = NULL;
    c_metaObject      scope    = NULL;
    sd_elementContext element  = NULL;
    c_array           elements = NULL;
    c_ulong           num      = 0;
    c_ulong           i        = 0;

    scope = sd_elementContextGetScope(parent);
    assert(scope);

    o = c_metaObject(c_metaDefine(scope, M_ENUMERATION));
    if ( o ) {
        o->definedIn = scope;
        element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
        if ( element ) {
            if ( sd_typeInfoParserNext(handle, sd_handleTypeElement, element) ) {
                num = sd_listSize(element->children);
                if ( num > 0 ) {
                    elements = c_arrayNew(c_constant_t((c_base)parent->info->base), num);
                    if ( elements ) {
                        while ( !sd_listIsEmpty(element->children) ) {
                            sd_elementContext child = (sd_elementContext)sd_listTakeFirst(element->children);
                            assert(child);
                            elements[i] = child->object;
                            sd_elementContextFree(child);
                            i++;
                        }
                    } else {
                        OS_REPORT(OS_ERROR,"sd_deserXmlEnumeration",0,
                                  "Internal error (out of resources) : "
                                  "Allocation elements array failed.");
                    }
                }

                c_enumeration(o)->elements = elements;
                c_metaFinalize(o);
                element->object = c_metaBind(scope, name, o);
                if ( element->object ) {
                    result = TRUE;
                } else {
                    c_char *scopeName = c_metaScopedName(scope);
                    OS_REPORT_2(OS_ERROR,"sd_deserXmlEnumeration",0,
                                "c_metaBind failed for Enum <%s> in scope <%s>.",
                                name, scopeName);
                    os_free(scopeName);
                }
            }
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlEnumeration",0,
                      "Internal error (out of resources) : "
                      "Allocation context element failed.");
        }
        c_free(o);
    } else {
        OS_REPORT(OS_ERROR,"sd_deserXmlEnumeration",0,
                  "Internal error (out of resources) : "
                  "Allocation c_metaDefine failed.");
    }

    return result;
}


static c_value
sd_stringToPrimValue (
    const c_char *str,
    c_primKind    kind)
{
    c_value    value;
    c_long     lv;
    c_longlong llv;
    c_bool     lb;
    c_address  av;

    value.kind = V_UNDEFINED;

    switch ( kind ) {
        case P_SHORT:
            if ( sd_stringToLong(str, &lv) ) {
                value = c_shortValue((c_short)lv);
            }
            break;
        case P_LONG:
            if ( sd_stringToLong(str, &lv) ) {
                value = c_longValue((c_long)lv);
            }
            break;
        case P_LONGLONG:
            if ( sd_stringToLongLong(str, &llv) ) {
                value = c_longlongValue((c_longlong)llv);
            }
            break;
        case P_USHORT:
            if ( sd_stringToLong(str, &lv) ) {
                value = c_longValue((c_ushort)lv);
            }
            break;
        case P_ULONG:
            if ( sd_stringToLong(str, &lv) ) {
                value = c_ulongValue((c_ulong)lv);
            }
            break;
        case P_ULONGLONG:
            if ( sd_stringToLongLong(str, &llv) ) {
                value = c_ulonglongValue((c_ulonglong)llv);
            }
            break;
        case P_CHAR:
            if ( strlen(str) == 1 ) {
                value = c_charValue(str[0]);
            }
            break;
        case P_ADDRESS:
            if ( sd_stringToAddress(str, &av) ) {
                value = c_addressValue(av);
            }
            break;
        case P_BOOLEAN:
            if ( sd_stringToBoolean(str, &lb) ) {
                value = c_boolValue(lb);
            }
            break;
        default:
            break;
    }

    return value;
}

static c_value
sd_stringToEnumValue (
    const c_char *str,
    c_array       elements)
{
    c_value      value;
    c_bool       found = FALSE;
    c_ulong      i,len;
    c_metaObject c;

    value.kind = V_UNDEFINED;

    len = c_arraySize(elements);

    for ( i = 0; !found && (i < len); i++ ) {
        c = (c_metaObject)elements[i];
        if ( c->name && (strcmp(c->name, str) == 0) ) {
            value = c_longValue(i);
            found = TRUE;
        }
    }

    return value;
}


static c_bool
sd_deserXmlLabel (
    sd_elementContext  parent,
    sd_list            attributes)
{
    c_bool             result        = FALSE;
    c_metaObject       o             = NULL;
    c_metaObject       scope         = NULL;
    sd_elementContext  element       = NULL;
    sd_elementContext  discriminator = NULL;
    const c_char      *value         = NULL;
    c_value            label;
    c_type             type          = NULL;

    scope = sd_elementContextGetScope(parent);
    assert(scope);

    discriminator = (sd_elementContext) sd_listReadFirst(parent->parent->children);
    if ( discriminator ) {
        type = c_type(discriminator->object);
        type = c_typeActualType(type);
    }

    if ( type ) {
        value = sd_findAttributeValue(attributes, "value");
    } else {
        OS_REPORT(OS_ERROR,"sd_deserXmlLabel",0,
                  "Discriminator type not found.");
    }

    if ( value ) {
        switch ( c_baseObject(type)->kind ) {
            case M_PRIMITIVE:
                label = sd_stringToPrimValue(value, c_primitive(type)->kind);
                break;
            case M_ENUMERATION:
                label = sd_stringToEnumValue(value, c_enumeration(type)->elements);
                break;
            default:
                label = c_undefinedValue();
                break;
        }

        if ( label.kind != V_UNDEFINED ) {
            o = c_metaDefine(scope, M_LITERAL);
            if ( o ) {
                c_literal(o)->value = label;
                element = sd_elementContextNew(parent->info, NULL, o, parent, FALSE);
                if ( element ) {
                    result = TRUE;
                } else {
                    OS_REPORT(OS_ERROR,"sd_deserXmlLabel",0,
                              "Internal error (out of resources) : "
                              "Allocation UnionLabel context element failed.");
                    c_free(o);
                }
            } else {
                OS_REPORT(OS_ERROR,"sd_deserXmlLabel",0,
                          "Internal error (out of resources) : "
                          "Allocation c_metaDefine UnionLabel failed.");
            }
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlLabel",0,
                      "UnionLabel kind is undefined.");
        }
    }
    return result;
}

static c_bool
sd_deserXmlUnionCase (
    sd_elementContext  parent,
    const c_char      *name,
    sd_typeInfoHandle  handle)
{
    c_bool            result   = FALSE;
    c_metaObject      o        = NULL;
    c_metaObject      scope    = NULL;
    sd_elementContext element  = NULL;
    c_array           labels   = NULL;
    c_ulong           num      = 0;
    c_ulong           i        = 0;

    scope = sd_elementContextGetScope(parent);
    assert(scope);

    o = c_metaObject(c_metaDefine(scope, M_UNIONCASE));
    if ( o ) {
        o->definedIn = scope;
        element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
        if ( element ) {
            if ( sd_typeInfoParserNext(handle, sd_handleTypeElement, element) ) {
                sd_elementContext child = (sd_elementContext)sd_listTakeFirst(element->children);
                assert(child);

                c_specifier(o)->type = c_type(child->object);
                c_specifier(o)->name = c_stringNew(c_object(parent->info->base), name);

                sd_elementContextFree(child);

                num = sd_listSize(element->children);
                if ( num > 0 ) {
                    labels = c_arrayNew(c_literal_t((c_base)parent->info->base), num);
                    if ( labels ) {
                        while ( !sd_listIsEmpty(element->children) ) {
                            child = (sd_elementContext)sd_listTakeFirst(element->children);
                            assert(child);

                            labels[i] = c_literal(child->object);
                            sd_elementContextFree(child);
                            i++;
                        }

                        c_unionCase(o)->labels = labels;
                        result = TRUE;
                    } else {
                        OS_REPORT(OS_ERROR,"sd_deserXmlUnionCase",0,
                                  "Internal error (out of resources) : "
                                  "Allocation label array failed.");
                    }
                } else {
                    c_unionCase(o)->labels = NULL;
                    result = TRUE;
                }
            }
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlUnionCase",0,
                      "Internal error (out of resources) : "
                      "Allocation UnionCase context element failed.");
        }
    } else {
        OS_REPORT(OS_ERROR,"sd_deserXmlUnionCase",0,
                  "Internal error (out of resources) : "
                  "Allocation c_metaDefine UnionCase failed.");
    }

    return result;
}



static c_bool
sd_deserXmlUnionSwitch (
    sd_elementContext  parent,
    sd_typeInfoHandle  handle)
{
    return sd_typeInfoParserNext(handle, sd_handleTypeElement, parent);
}


static c_bool
sd_deserXmlUnion (
    sd_elementContext  parent,
    const c_char      *name,
    sd_typeInfoHandle  handle)
{
    c_bool            result   = FALSE;
    c_metaObject      o        = NULL;
    c_metaObject      scope    = NULL;
    sd_elementContext element  = NULL;
    c_array           cases    = NULL;
    c_ulong           num      = 0;
    c_ulong           i        = 0;

    scope = sd_elementContextGetScope(parent);
    assert(scope);

    o = c_metaObject(c_metaDefine(scope, M_UNION));
    if ( o ) {
        o->definedIn = scope;
        element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
        if ( element ) {
            if ( sd_typeInfoParserNext(handle, sd_handleTypeElement, element) ) {
                sd_elementContext child = (sd_elementContext)sd_listTakeFirst(element->children);
                assert(child);

                c_union(o)->switchType = c_type(child->object);
                sd_elementContextFree(child);

                num = sd_listSize(element->children);
                if ( num > 0 ) {
                    cases = c_arrayNew(c_unionCase_t((c_base)parent->info->base), num);
                    if ( cases ) {
                        while ( !sd_listIsEmpty(element->children) ) {
                            child = (sd_elementContext)sd_listTakeFirst(element->children);
                            assert(child);

                            cases[i] = child->object;
                            sd_elementContextFree(child);
                            i++;
                        }
                    } else {
                        OS_REPORT(OS_ERROR,"sd_deserXmlUnion",0,
                                  "Internal error (out of resources) : "
                                  "Allocation UnionCase array failed.");
                    }
                }
                c_union(o)->cases = cases;
                c_metaFinalize(o);
                element->object = c_metaBind(scope, name, o);
                if ( element->object ) {
                    result = TRUE;
                } else {
                    c_char *scopeName = c_metaScopedName(scope);
                    OS_REPORT_2(OS_ERROR,"sd_deserXmlUnion",0,
                                "c_metaBind failed for Union <%s> in scope <%s>.",
                                name, scopeName);
                    os_free(scopeName);
                }
            }
        } else {
            OS_REPORT(OS_ERROR,"sd_deserXmlUnion",0,
                      "Internal error (out of resources) : "
                      "Allocation Union context element failed.");
        }
        c_free(o);
    } else {
        OS_REPORT(OS_ERROR,"sd_deserXmlUnion",0,
                  "Internal error (out of resources) : "
                  "Allocation c_metaDefine Union failed.");
    }

    return result;
}



static c_bool
sd_handleTypeElement (
    sd_typeInfoKind    kind,
    c_char            *name,
    sd_list            attributes,
    void              *argument,
    sd_typeInfoHandle  handle)
{
    c_bool            result = FALSE;
    sd_elementContext context = (sd_elementContext) argument;

    assert(context);

    switch ( kind ) {
        case SD_TYPEINFO_KIND_MODULE:
            result = sd_deserXmlModule(context, name, handle);
            break;
        case SD_TYPEINFO_KIND_STRUCT:
            result = sd_deserXmlStructure(context, name, handle);
            break;
        case SD_TYPEINFO_KIND_MEMBER:
            result = sd_deserXmlMember(context, name, handle);
            break;
        case SD_TYPEINFO_KIND_UNION:
            result = sd_deserXmlUnion(context, name, handle);
            break;
        case SD_TYPEINFO_KIND_UNIONCASE:
            result = sd_deserXmlUnionCase(context, name, handle);
            break;
        case SD_TYPEINFO_KIND_UNIONSWITCH:
            result = sd_deserXmlUnionSwitch(context, handle);
            break;
        case SD_TYPEINFO_KIND_UNIONLABEL:
            result = sd_deserXmlLabel(context, attributes);
            break;
        case SD_TYPEINFO_KIND_TYPEDEF:
            result = sd_deserXmlTypedef(context, name, handle);
            break;
        case SD_TYPEINFO_KIND_ENUM:
            result = sd_deserXmlEnumeration(context, name, handle);
            break;
        case SD_TYPEINFO_KIND_ENUMLABEL:
            result = sd_deserXmlElement(context, name, attributes);
            break;
        case SD_TYPEINFO_KIND_TYPE:
            result = sd_deserXmlType(context, name);
            break;
        case SD_TYPEINFO_KIND_ARRAY:
            result = sd_deserXmlArray(context, attributes, handle);
            break;
        case SD_TYPEINFO_KIND_SEQUENCE:
            result = sd_deserXmlSequence(context, attributes, handle);
            break;
        case SD_TYPEINFO_KIND_STRING:
            result = sd_deserXmlString(context, attributes);
            break;
        case SD_TYPEINFO_KIND_CHAR:
        case SD_TYPEINFO_KIND_BOOLEAN:
        case SD_TYPEINFO_KIND_OCTET:
        case SD_TYPEINFO_KIND_SHORT:
        case SD_TYPEINFO_KIND_USHORT:
        case SD_TYPEINFO_KIND_LONG:
        case SD_TYPEINFO_KIND_ULONG:
        case SD_TYPEINFO_KIND_LONGLONG:
        case SD_TYPEINFO_KIND_ULONGLONG:
        case SD_TYPEINFO_KIND_FLOAT:
        case SD_TYPEINFO_KIND_DOUBLE:
            result = sd_deserXmlPrimitive(context, kind);
            break;
        case SD_TYPEINFO_KIND_TIME:
            result = sd_deserXmlTime(context, kind);
            break;
         case SD_TYPEINFO_KIND_UNIONLABELDEFAULT:
            result = TRUE;
            break;
        default:
            break;
    }

    return result;
}

static void
reportError (
    sd_errorReport report)
{
    if ( report ) {
        if ( report->message ) {
            if ( report->location ) {
                OS_REPORT_2(OS_ERROR,"Type deserialize",0,
                    "Deserialize failed : %s at %s", report->message, report->location);
            } else {
                OS_REPORT_1(OS_ERROR,"Type deserialize",0, "Deserialize failed : %s", report->message);
            }
        } else {
            OS_REPORT(OS_ERROR,"Type deserialize",0, "Deserialize failed");
        }
    } else {
        OS_REPORT(OS_ERROR,"Type deserialize",0, "Deserialize failed");
    }
}



static c_object
sd_serializerXMLTypeinfoDeserialize(
    sd_serializer serializer,
    sd_serializedData serData,
    c_bool doValidation)
{
    c_char *xmlString;
    c_object result = NULL;
    sd_contextInfo info;
    sd_elementContext element;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    sd_serializerSetValidationState(serializer, doValidation);

    xmlString = (c_char *)serData->data;

    info.base       = (c_metaObject)serializer->base;
    info.objectType = c_type(c_metaResolve(info.base, "c_object"));
    info.errorInfo  = NULL;

    c_baseSerLock((c_base)info.base);

    element = sd_elementContextNew(&info, NULL, info.base, NULL, TRUE);

    if ( sd_typeInfoParserParse(xmlString, sd_handleTypeElement, element, &info.errorInfo) ) {
        if ( !sd_listIsEmpty(element->children) ) {
            sd_elementContext child = (sd_elementContext)sd_listReadFirst(element->children);
            result = child->object;
        } else {
            OS_REPORT(OS_ERROR,"sd_serializerXMLTypeinfoDeserialize",0,"Succeeded but no type found");
        }
    } else {
        reportError(info.errorInfo);
        if ( info.errorInfo ) {
            c_char *message  = NULL;
            c_char *location = NULL;
            if ( info.errorInfo->message ) {
                message = sd_stringDup(info.errorInfo->message);
            }
            if ( info.errorInfo->location ) {
                location = sd_stringDup(info.errorInfo->location);
            }
            sd_serializerSetValidationInfo(serializer, info.errorInfo->errorNumber, message, location);
            sd_errorReportFree(info.errorInfo);
        } else {
            sd_serializerSetValidationInfo(serializer, -1, NULL, NULL);
        }
    }

    sd_elementContextFree(element);

    c_baseSerUnlock((c_base)info.base);
    return result;
}

/* ---------------------------- constructor --------------------- */


/** \brief Constructor for the XML MetaData format serializer
 *
 *  The \b serializerXMLMetadata class is a descendant of the
 *  \b serializerXML class. In order to use this class, create it with this
 *  function and call the methods as defined on \b serializer.
 *
 *  \param base The database to serialize from and deserialize to.
 */

sd_serializer
sd_serializerXMLTypeinfoNew(
    c_base base,
    c_bool escapeQuote)
{
    sd_serializerXMLTypeinfo result;
    struct sd_serializerVMT VMT;

    VMT.serialize = sd_serializerXMLTypeinfoSerialize;
    VMT.deserialize = sd_serializerXMLTypeinfoDeserialize;
    VMT.deserializeInto = NULL;
    VMT.toString = sd_serializerXMLToString;
    VMT.fromString = sd_serializerXMLFromString;

    result = (sd_serializerXMLTypeinfo)os_malloc(C_SIZEOF(sd_serializerXMLTypeinfo));

    if ( result ) {
        sd_serializerInitialize((sd_serializer)result, SD_FORMAT_ID, SD_FORMAT_VERSION, base, NULL, VMT);
        result->escapeQuote = escapeQuote;
    }

    return (sd_serializer)result;
}

