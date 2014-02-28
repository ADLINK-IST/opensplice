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
#include "sd__deepwalkMeta.h"
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


#ifndef NDEBUG
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
#endif

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

/* --------------------- Serialization driving functions -------------------- */

typedef struct sd_context_s* sd_context;
struct sd_context_s {
    c_base base;
    c_iter items; /* c_iter<sd_item> - Dependency ordered list of contextItems. */
    c_iter declarations; /* c_iter<sd_item> - Declarations - for cycle detection. */

    /* Variables used for printing */
    c_metaObject module; /* Used for keeping track of current scope while printing. */
    c_iter inlineProcessed; /* c_iter<c_type> - Keeps a list of the inline types defined in the current top-level type. This list is needed
                         because types can be used more than once within a given type-scope and the algorithm must be able
                         to determine whether a type should be defined or whether it already is. */
    c_bool escapeQuotes;
    c_iter xmlbuff;  /* c_iter<c_char*> This contains a list of string-buffers, which together form the metadescriptor. The
                        final step is to aggegrate these buffers into one string. */

    /* Module optimization */
    c_iter modules; /* c_iter<sd_moduleItem> - This list will be populated with used modules when types are parsed with modules. Per module
                       the number of types is counted. At print-time, arrays are allocated of size typeCount * sizeof(sd_item).
                       These arrays will at any time hold the unprinted root-type items with dependency count 0. */
};

/* This structure is only used in the final module optimization step. */
typedef struct sd_item_s* sd_item;
typedef struct sd_moduleItem_s* sd_moduleItem;
struct sd_moduleItem_s {
    c_metaObject self;
    c_long typeCount; /* Worst case amount of types that can be stored at once in this module. */
    sd_item* array; /* Array which at any time contains the unprinted root-type items with dependency count 0 */
    sd_item* start; /* Pointer to the next item that should be printed. */
    sd_item* end; /* Pointer which indicates the next free element in the list. */
};

struct sd_item_s {
    c_type self;
    c_long refcount; /* This is the number of unresolved(unprinted) dependencies */
    c_iter dependees; /* c_iter<sd_item> */
    sd_moduleItem module; /* Pointer to module-item. This overcomes the need for doing
                             module-lookups at print-time, when a type must be added
                             to the type-array of a module when it reaches dependency-count 0. */
};


/* --------------------- Serialization utilities --------------------- */

/* Find the root-type (NULL if type is not an inlined type) */
c_type
sd_utilRootType(
    c_type type)
{
    c_metaObject result, prev;

    prev = NULL;
    result = c_metaObject(type)->definedIn;
    while(result && c_baseObject(result)->kind != M_MODULE) {
        prev = result;

        /* If result isn't a module, there must be higher levels in the hierarchy. */
        result = result->definedIn;
        assert(result);
    }

    return c_type(prev);
}

/* Check whether an object is inlined.
 * Collections and primitives are conceptually always inlined,
 * though the implementation might place them in a scope.
 */
static c_bool
sd_utilIsInlined(
    c_type type)
{
    /* Under some arcane conditions types can arrive here that have an
     * empty 'definedIn' pointer which previously crashed this function,
     * because the check on whether the parent was a module was first.
     * By reversing the checks the code is robust against this issue,
     * but it would be nice to know why it is occurring.
     */
    return (c_baseObject(type)->kind == M_COLLECTION) ||
           (c_baseObject(type)->kind == M_PRIMITIVE) ||
           (c_baseObject(c_metaObject(type)->definedIn)->kind != M_MODULE);
}

static sd_item
sd_itemNew(
    c_type self)
{
    sd_item result;

    result = os_malloc(sizeof(struct sd_item_s));
    result->self = self;
    result->refcount = 0;
    result->dependees = NULL;
    result->module = NULL;

    return result;
}

static void
sd_itemFree(
    sd_item item)
{
    if(item->dependees) {
        c_iterFree(item->dependees);
    }
    os_free(item);
}

static void
sd_moduleItemFree(
    sd_moduleItem item)
{
    if(item->array) {
        os_free(item->array);
    }
    os_free(item);
}

static sd_moduleItem
sd_moduleItemNew(
    c_metaObject module)
{
    sd_moduleItem result;

    result = os_malloc(sizeof(struct sd_moduleItem_s));
    result->array = NULL;
    result->start = NULL;
    result->end = NULL;
    result->typeCount = 0;
    result->self = module;

    return result;
}

static sd_context
sd_contextNew(
    c_base base,
    c_bool escapeQuotes)
{
    sd_context result;

    result = os_malloc(sizeof(struct sd_context_s));
    result->items = c_iterNew(NULL);
    result->declarations = c_iterNew(NULL);
    result->base = base;
    result->module = NULL;
    result->escapeQuotes = escapeQuotes;
    result->xmlbuff = NULL;
    result->inlineProcessed = NULL;
    result->modules = NULL;

    return result;
}

static void
sd_freeItems(
    void* o,
    void* udata)
{
    sd_item item;
    OS_UNUSED_ARG(udata);

    item = o;
    sd_itemFree(item);
}

static void
sd_freeModuleItems(
    void* o,
    void* udata)
{
    sd_moduleItem item;
    OS_UNUSED_ARG(udata);

    item = o;
    sd_moduleItemFree(item);
}

static void
sd_contextFree(
    sd_context context)
{
    /* Free items */
    c_iterWalk(context->items, sd_freeItems, NULL);
    c_iterFree(context->items);

    /* Free declarations */
    c_iterFree(context->declarations);

    /* Free modules */
    c_iterWalk(context->modules, sd_freeModuleItems, NULL);
    c_iterFree(context->modules);

    assert(context->xmlbuff == NULL);

    os_free(context);
}

/* Add a dependee to the dependee list of an item */
static void
sd_itemAddDependee(
    sd_item item,
    sd_item dependee)
{
    item->dependees = c_iterInsert(item->dependees, dependee);

    /* Increase the refcount of the dependee. */
    dependee->refcount++;
    assert(item->dependees);
}

/* Add item to module array */
static void
sd_itemAddToModule(
    sd_item item) {
    sd_moduleItem module = item->module;

    if(!module->array) {
        module->array = os_malloc(module->typeCount * sizeof(sd_item));
        module->start = module->array;
        module->end = module->array;
    }

    *(module->end) = item;
    module->end++;

    assert(module->end <= (module->array + module->typeCount));
}

struct sd_itemLookup_t {
    c_type type;
    sd_item result;
};

static void
sd_itemDerefDependee(
    void* o, void* udata)
{
    if(!--((sd_item)o)->refcount) {
        sd_itemAddToModule(o);
    }
}

/* Deref dependees. Dependees are deref'd when a dependency (the item passed to this function) is
 * resolved. When a dependee reaches refcount zero, it can be processed itself. */
static void
sd_itemDerefDependees(
    sd_item item)
{
    c_iterWalk(item->dependees, sd_itemDerefDependee, NULL);
}

c_bool
sd_contextLookupAction(
    void* o,
    void* userData)
{
    struct sd_itemLookup_t* data;

    data = userData;
    if(((sd_item)o)->self == data->type) {
        data->result = o;
    }

    return data->result == NULL;
}

/* Check whether an item is already processed */
static sd_item
sd_contextIsProcessed(
    sd_context context,
    c_type type)
{
    struct sd_itemLookup_t walkData;

    walkData.type = type;
    walkData.result = NULL;
    c_iterWalkUntil(context->items, sd_contextLookupAction, &walkData);

    return walkData.result;
}

/* Check for cycles */
static sd_item
sd_contextCheckCycles(
    sd_context context,
    c_type type)
{
    struct sd_itemLookup_t walkData;

    walkData.type = type;
    walkData.result = NULL;
    c_iterWalkUntil(context->declarations, sd_contextLookupAction, &walkData);

    return walkData.result;
}

struct sd_contextFindModule_t {
    c_metaObject find;
    sd_moduleItem result;
};

static c_bool
sd_contextFindModule(
    void* o,
    void* userData)
{
    struct sd_contextFindModule_t* data;

    data = userData;
    if(((sd_moduleItem)o)->self == data->find) {
        data->result = o;
    }

    return data->result == NULL;
}

/* Mark an item as processed, add to the context->items list */
static void
sd_contextProcessed(
    sd_context context,
    sd_item item)
{
    struct sd_contextFindModule_t walkData;
    assert(!sd_contextIsProcessed(context, item->self));
    c_iterAppend(context->items, item);

    /* Find corresponding module */
    walkData.find = c_metaObject(item->self)->definedIn;
    walkData.result = NULL;
    c_iterWalkUntil(context->modules, sd_contextFindModule, &walkData);

    /* If module is not found, create new module object. */
    if(!walkData.result) {
        walkData.result = sd_moduleItemNew(walkData.find);
        context->modules = c_iterInsert(context->modules, walkData.result);
    }

    /* Administrate extra type for module. */
    walkData.result->typeCount++;

    /* Store pointer in item to module, so that during printing no lookups are required. */
    item->module = walkData.result;
}

/* Mark an item as declared, add to the context->declarations list */
static void
sd_contextDeclare(
    sd_context context,
    sd_item item)
{
    assert(!sd_contextIsProcessed(context, item->self));
    c_iterAppend(context->declarations, item);
}

/* Serialize a type */
static int
sd_serializeType(
    sd_context context,
    sd_item rootType,
    c_type type,
    c_bool allowCycles,
    sd_item* out);

/* Serialize dependencies of a typedef */
static int
sd_serializeTypedefDependencies(
    sd_context context,
    sd_item rootType,
    c_bool allowCycles,
    c_typeDef type)
{
    sd_item alias;

    alias = NULL;

    /* Resolve dependency */
    if(sd_serializeType(context, rootType, type->alias, allowCycles, &alias)) {
        goto error;
    }

    /* Add typedef to dependee list of dependency */
    if(alias) {
        sd_itemAddDependee(alias, rootType);
    }

    return 0;
error:
    return -1;
}

/* Serialize dependencies of a collection. */
static int
sd_serializeCollectionDependencies(
    sd_context context,
    sd_item rootType,
    c_collectionType type)
{
    sd_item subType;
    c_bool allowCycles;

    subType = NULL;
    allowCycles = FALSE;

    /* Check if type is a sequence or array to find out if cycles are allowed. */
    if(type->kind == C_SEQUENCE) {
        allowCycles = TRUE;
    }

    /* Resolve dependency */
    if(sd_serializeType(context, rootType, type->subType, allowCycles, &subType)) {
        goto error;
    }

    /* Add collection to dependee list of subType */
    if(subType) {
        sd_itemAddDependee(subType, rootType);
    }

    return 0;
error:
    return -1;
}

/* Serialize dependencies for a struct */
static int
sd_serializeStructureDependencies(
    sd_context context,
    sd_item rootType,
    c_structure type)
{
    c_member member;
    sd_item memberType;
    c_long i;

    memberType = NULL;

    /* Walk members to resolve dependencies of struct. */
    for(i=0; i<c_arraySize(type->members); i++) {
        member = type->members[i];
        /* Serialize memberType, do not allow cycles. */
        if(sd_serializeType(context, rootType, c_specifier(member)->type, FALSE, &memberType)) {
            goto error;
        }
        if(memberType) {
            sd_itemAddDependee(memberType, rootType);
        }
    }

    return 0;
error:
    return -1;
}

/* Serialize dependencies for union */
static int
sd_serializeUnionDependencies(
    sd_context context,
    sd_item rootType,
    c_union type)
{
    c_unionCase _case;
    sd_item caseType, switchType;
    c_long i;

    switchType = NULL;
    caseType = NULL;

    /* Serialize switchType, no cycles allowed. */
    if(sd_serializeType(context, rootType, type->switchType, FALSE, &switchType)) {
        goto error;
    }
    if(switchType) {
        sd_itemAddDependee(switchType, rootType);
    }

    /* Walk cases to resolve dependencies of union */
    for(i=0; i<c_arraySize(type->cases); i++) {
        _case = type->cases[i];
        /* Serialize caseType, no cycles allowed. */
        if(sd_serializeType(context, rootType, c_specifier(_case)->type, FALSE, &caseType)) {
            goto error;
        }
        if(caseType) {
            sd_itemAddDependee(caseType, rootType);
        }
    }


    return 0;
error:
    return -1;
}

/* Serialize dependencies of a type.
 *
 * Parameter 'rootType' is typically the same as the 'type' parameter.
 * However, for inline types these two differ. The distinction
 * enables that dependencies are always added to the
 * top-level type. */
static int
sd_serializeTypeDependencies(
    sd_context context,
    sd_item rootType,
    c_bool allowCycles,
    c_type type)
{
    int result;

    result = 0;

    /* Forward to the correct dependency-resolve function, depending on metaKind */
    switch(c_baseObject(type)->kind) {
    case M_TYPEDEF:
        /* Allowance of cycles is transparently forwarded to typedefs. */
        result = sd_serializeTypedefDependencies(context, rootType, allowCycles, c_typeDef(type));
        break;
    case M_COLLECTION:
        result = sd_serializeCollectionDependencies(context, rootType, c_collectionType(type));
        break;
    case M_ENUMERATION:
        /* Enumerations have no dependencies */
        break;
    case M_STRUCTURE:
        result = sd_serializeStructureDependencies(context, rootType, c_structure(type));
        break;
    case M_UNION:
        result = sd_serializeUnionDependencies(context, rootType, c_union(type));
        break;
    default:
        assert(0);
        break;
    }

    return result;
}

/* Serialize a type */
static int
sd_serializeType(
    sd_context context,
    sd_item rootType,
    c_type type,
    c_bool allowCycles,
    sd_item* out)
{
    sd_item item;

    item = NULL;

    switch(c_baseObject(type)->kind) {
    case M_PRIMITIVE:
        /* Primitives and collections are always treated as inline types, and are generated when a type is printed. */
        break;
    case M_STRUCTURE:
        /* Check if type is the c_time type in the rootscope which is not handled as a seperate type, but will result in a <Time/>. */
        if((c_metaObject(type)->definedIn == c_metaObject(context->base)) && !strcmp("c_time", c_metaObject(type)->name)) {
            break; /* Don't handle c_time */
        }
    /* Fallthrough on purpose. */
    case M_TYPEDEF:
    case M_COLLECTION: /* Collections are always treated as inline types, but do have dependencies. */
    case M_ENUMERATION:
    case M_UNION:
        /* Check if item is already processed */
        if(!(item = sd_contextIsProcessed(context, type))) {
            /* Check for invalid cycles. Don't process type if a cycle is detected. */
            if(((c_baseObject(type)->kind == M_STRUCTURE) || (c_baseObject(type)->kind == M_UNION)) && sd_contextCheckCycles(context, type)) {
                if(!allowCycles) {
                    OS_REPORT(OS_ERROR, "sd_serializerXMLTypeInfoSerialize", 0, "unsupported cycle detected!");
                    goto error;
                }
            }else {
                /* Don't process inlined types in graph. These are resolved when the parent-type is printed, which is always in correct
                 * dependency order, which is the order of members(structs) or cases(unions). Collections are also considered
                 * inlined, because the actual location of a collection type is implementation specific. */
                if(!sd_utilIsInlined(type)) {
                    /* Here, the usage of 'rootType' is prohibited since this is not an inlined type. */

                    /* Create contextItem */
                    item = sd_itemNew(type);

                    /* Add forward-declaration marker for structs and unions, to allow references to self, so that
                     * the cycles can be detected. IDL does not allow other types to be forward-declared, thus
                     * cannot introduce cyclic references. */
                    if((c_baseObject(type)->kind == M_STRUCTURE) || (c_baseObject(type)->kind == M_UNION)) {
                        sd_contextDeclare(context, item);
                    }

                    /* Resolve dependencies of type */
                    if(sd_serializeTypeDependencies(context, item, allowCycles, type)) {
                        /* An error occurred, most likely an unsupported cyclic dependency */
                        sd_itemFree(item);
                        goto error;
                    }

                    /* If success, add item to processed list */
                    if(item) {
                        sd_contextProcessed(context, item);
                    }
                    break;
                }else {
                    c_type typeRoot;

                    assert(rootType);

                    /* Still need to resolve dependencies for inlined types */
                    if(sd_serializeTypeDependencies(context, rootType, allowCycles, type)) {
                        goto error;
                    }

                    /* If rootType is referencing an inlined type that is defined in the scope of another type,
                     * rootType is implicitly dependent on the rootType of the inlined type. */
                    if(rootType->self != (typeRoot = sd_utilRootType(type))) {
                        sd_item typeRootItem = NULL;

                        /* An empty typeRoot means that the type is not stored as inlined object, but conceptually it is.
                         * This will typically occur for intern collectiontypes like c_string, who are defined
                         * in the root. */
                        if(typeRoot) {
                            /* Serialize rootType of type */
                            if(sd_serializeType(context, NULL, typeRoot, allowCycles, &typeRootItem)) {
                                goto error;
                            }

                            /* Because the 'typeRoot' is the root type of 'type', this can never be an inlined
                             * type, neither can it be a primitive type (which is not serialized). Thus, typeRootItem
                             * must always be set.
                             */
                            assert(typeRootItem);

                            /* Add the root-type of the type to 'rootType' */
                            sd_itemAddDependee(typeRootItem, rootType);
                        }
                    }
                }
            }
        }
        break;
    default:
        /* The serializer shouldn't call serializeType with metaKinds other than the ones described above. */
        assert(0);
        break;
    }

    /* Return item if out is set */
    if(out) {
        *out = item;
    }

    return 0;
error:
    return -1;
}

#define SD_XML_BUFFER (512)

/* Append to buffer-list. Parameter 'str' is never larger than SD_XML_BUFFER bytes. */
static void
sd_printXmlAppend(
    sd_context context,
    char* str /* char[SD_XML_BUFFER] */)
{
    char* lastBuffer;
    c_ulong strLen, buffLen, spaceLeft;

    /* Get first buffer from list, create if it didn't exist */
    lastBuffer = c_iterObject(context->xmlbuff, 0);
    if(!lastBuffer) {
        lastBuffer = os_malloc(SD_XML_BUFFER + 1);
        *lastBuffer = '\0';
        context->xmlbuff = c_iterInsert(context->xmlbuff, lastBuffer);
    }

    strLen = strlen(str);
    buffLen = strlen(lastBuffer);
    spaceLeft = SD_XML_BUFFER - buffLen;

    /* If length of string is larger than the space left in the buffer, allocate new buffer. */
    if(strLen >= spaceLeft) {
        char* nextBuffer;
        /* Fill remaining bytes of current buffer. */
        memcpy(lastBuffer + buffLen, str, spaceLeft);
        lastBuffer[SD_XML_BUFFER] = '\0';

        /* Do the remaining string in the next buffer. */
        nextBuffer = os_malloc(SD_XML_BUFFER + 1);
        memcpy(nextBuffer, str + spaceLeft, strLen - spaceLeft + 1 /* 0-terminator */);

        /* Insert new buffer in iterator */
        context->xmlbuff = c_iterInsert(context->xmlbuff, nextBuffer);

    /* ... otherwise, just append the string to the current buffer. */
    }else {
        strcat(lastBuffer, str);
    }
}

static void
sd_printXmlAggegrate(
    sd_context context,
    sd_serializedData* result)
{
    c_iterIter iter;
    c_long size;
    c_char* chunk;

    size = c_iterLength(context->xmlbuff);

    /* Allocate memory for string */
    *result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size * (SD_XML_BUFFER) + 1);

    /* Copy buffers in final structure. */
    iter = c_iterIterGet(context->xmlbuff);
    while((chunk = c_iterNext(&iter))) {
#ifdef SER_DEBUG
        printf("   ### %d: %s\n", strlen(chunk), chunk);
#endif
        memcpy((c_char*)((*result)->data) + SD_XML_BUFFER * (size-1), chunk, SD_XML_BUFFER);
        os_free(chunk);
        size--;
    }

}

/* Escape '"' */
static void
sd_printXmlEscapeQuotes(
    c_char* buffer,
    c_char* escapedBuffer)
{
    c_char *ptr, *quotePtr, *escapedPtr;
    c_long ptrLen;

    ptr = buffer;
    escapedPtr = escapedBuffer;

    /* Copy in chunks between quotes, replace quotes with escaped quotes. */
    while((quotePtr = strchr(ptr, '\"'))) {
        memcpy(escapedPtr, ptr, quotePtr - ptr);
        escapedPtr += quotePtr - ptr;
        *escapedPtr = '\\';
        escapedPtr++;
        *escapedPtr = '\"';
        escapedPtr++;
        ptr = quotePtr+1;
    }

    /* Copy last chunk */
    ptrLen = strlen(ptr);
    memcpy(escapedPtr, ptr, ptrLen);

    /* Add 0-terminator */
    escapedPtr += ptrLen;
    *escapedPtr = '\0';
}

/* Print xml */
static void
sd_printXml(
    sd_context context,
    char* fmt,
    ...)
{
    va_list args;
    c_char buffer[SD_XML_BUFFER]; /* Large buffer on stack, safe since function is never called recursively. */
    int length;

    va_start(args, fmt);
    length = vsnprintf(buffer, SD_XML_BUFFER, fmt, args); /* C99 function */
    if(length >= SD_XML_BUFFER) {
        OS_REPORT(OS_ERROR, "sd_serializerXMLTypeInfoSerialize", 0, "buffer too small!");
        assert(0);
    }

    /* Escape quotes if needed */
    if(context->escapeQuotes) {
        c_char escapedBuffer[SD_XML_BUFFER];
        sd_printXmlEscapeQuotes(buffer, escapedBuffer);
        sd_printXmlAppend(context, escapedBuffer);
    }else {
        sd_printXmlAppend(context, buffer);
    }

    va_end(args);
}

/* Print type. */
static void
sd_printXmlType(
    sd_context context,
    c_type current,
    c_type type);

/* Print typedef */
static void
sd_printXmlTypedef(
    sd_context context,
    c_typeDef type)
{
    sd_printXml(context, "<TypeDef name=\"%s\">", c_metaObject(type)->name);
    sd_printXmlType(context, c_type(type), type->alias);
    sd_printXml(context, "</TypeDef>");
}

/* Print primitive */
static void
sd_printXmlPrimitive(
    sd_context context,
    c_primitive type)
{
    switch(type->kind) {
    case P_BOOLEAN:
        sd_printXml(context, "<Boolean/>");
        break;
    case P_CHAR:
        sd_printXml(context, "<Char/>");
        break;
    case P_WCHAR:
        sd_printXml(context, "<WChar/>");
        break;
    case P_OCTET:
        sd_printXml(context, "<Octet/>");
        break;
    case P_SHORT:
        sd_printXml(context, "<Short/>");
        break;
    case P_USHORT:
        sd_printXml(context, "<UShort/>");
        break;
    case P_LONG:
        sd_printXml(context, "<Long/>");
        break;
    case P_ULONG:
        sd_printXml(context, "<ULong/>");
        break;
    case P_LONGLONG:
        sd_printXml(context, "<LongLong/>");
        break;
    case P_ULONGLONG:
        sd_printXml(context, "<ULongLong/>");
        break;
    case P_FLOAT:
        sd_printXml(context, "<Float/>");
        break;
    case P_DOUBLE:
        sd_printXml(context, "<Double/>");
        break;
    default:
        /* Use this serializer only to serialize userdata */
        assert(0);
        break;
    }
}

/* Print enumeration */
static void
sd_printXmlEnumeration(
    sd_context context,
    c_enumeration type)
{
    c_long i;
    c_constant constant;
    c_long value;

    sd_printXml(context, "<Enum name=\"%s\">", c_metaObject(type)->name);

    /* Walk constants */
    for(i=0; i<c_arraySize(type->elements); i++) {
        constant = type->elements[i];
        value = (c_long)(c_literal(c_constant(constant)->operand)->value.is.Long);
        sd_printXml(context, "<Element name=\"%s\" value=\"%d\"/>", c_metaObject(constant)->name, value);
    }

    sd_printXml(context, "</Enum>");
}

/* Print structure */
static void
sd_printXmlStructure(
    sd_context context,
    c_structure type)
{
    c_long i;
    c_member member;

    sd_printXml(context, "<Struct name=\"%s\">", c_metaObject(type)->name);

    /* Walk members of struct */
    for(i=0; i<c_arraySize(type->members); i++) {
        member = type->members[i];

        /* Serialize member and member type */
        sd_printXml(context, "<Member name=\"%s\">", c_specifier(member)->name);
        sd_printXmlType(context, c_type(type), c_specifier(member)->type);
        sd_printXml(context, "</Member>");
    }

    sd_printXml(context, "</Struct>");
}

/* Print union */
static void
sd_printXmlUnion(
    sd_context context,
    c_union type)
{
    c_long i, j;
    c_unionCase _case;
    c_char* image;
    c_literal literal;
    c_type switchType;

    sd_printXml(context, "<Union name=\"%s\">", c_metaObject(type)->name);

    /* Serialize switch-type */
    sd_printXml(context, "<SwitchType>");
    sd_printXmlType(context, c_type(type), type->switchType);
    sd_printXml(context, "</SwitchType>");

    switchType = c_typeActualType(type->switchType);

    /* Walk cases of union */
    for(i=0; i<c_arraySize(type->cases); i++) {
        _case = type->cases[i];

        /* Serialize member and member type */
        sd_printXml(context, "<Case name=\"%s\">", c_specifier(_case)->name);
        sd_printXmlType(context, c_type(type), c_specifier(_case)->type);

        /* Walk case labels */
        if(!_case->labels || !c_arraySize(_case->labels)) {
            sd_printXml(context, "<Default/>");
        }else {
            for(j=0; j<c_arraySize(_case->labels); j++) {
                literal = _case->labels[j];
                /* Serialize label */
                sd_printXml(context, "<Label value=\"");

                /* Obtain and print string for value */
                if(c_baseObject(switchType)->kind == M_ENUMERATION) {
                    c_enumeration enumeration;
                    c_long n;

                    assert(literal->value.kind == V_LONG);
                    enumeration = c_enumeration(switchType);

                    n = literal->value.is.Long;

                    assert(n < c_arraySize(enumeration->elements));

                    sd_printXml(context, c_metaObject(enumeration->elements[n])->name);
                }else {
                    if(literal->value.kind == V_BOOLEAN) {
                        if(literal->value.is.Boolean) {
                            sd_printXml(context, "True");
                        }else {
                            sd_printXml(context, "False");
                        }
                    }else {
                        image = c_valueImage(literal->value);
                        sd_printXml(context, image);
                        os_free(image);
                    }
                }
                sd_printXml(context, "\"/>");
            }
        }
        sd_printXml(context, "</Case>");
    }

    sd_printXml(context, "</Union>");
}

/* Print collection */
static void
sd_printXmlCollection(
    sd_context context,
    c_type current,
    c_collectionType type)
{
    c_char* elementName;

    elementName = NULL;

    /* Print collection header */
    switch(type->kind) {
    case C_SEQUENCE:
        elementName = "Sequence";
        break;
    case C_ARRAY:
        elementName = "Array";
        break;
    case C_STRING:
        elementName = "String";
        break;
    default:
        OS_REPORT(OS_ERROR, "sd_printXmlCollection", 0, "invalid collectionkind for serializer.");
        assert(0);
        break;
    }

    sd_printXml(context, "<%s", elementName);

    /* Print collection size, subType and footer. Pass current rootType to
     * sd_printXmlType so inlined types that are used as subtype are defined within
     * the inline collection. */
    switch(type->kind) {
    case C_SEQUENCE:
    case C_ARRAY:
        if(type->maxSize) {
            sd_printXml(context, " size=\"%d\">", type->maxSize);
        }else {
            sd_printXml(context, ">");
        }
        sd_printXmlType(context, current, type->subType);
        sd_printXml(context, "</%s>", elementName);
        break;
    case C_STRING:
        if(type->maxSize) {
            sd_printXml(context, " length=\"%d\"/>", type->maxSize);
        }else {
            sd_printXml(context, "/>");
        }
        break;
    default:
        OS_REPORT(OS_ERROR, "sd_printXmlCollection", 0, "invalid collectionkind for serializer(2).");
        assert(0);
        break;
    }
}

#define SD_MAX_SCOPE_DEPTH (64) /* Should be on the safe side */

/* Function builds a scope-stack from root to module */
static void
sd_utilModuleStack(
    c_metaObject module,
    c_metaObject* stack /* c_metaObject[SD_MAX_SCOPE_DEPTH] */)
{
    c_long count;
    c_metaObject ptr;

    assert(module);

    /* Count scope depth */
    ptr = module;
    count = 1; /* For self */
    while((ptr = ptr->definedIn)) {
        count++;
    }

    if(count > SD_MAX_SCOPE_DEPTH) {
        OS_REPORT_2(OS_ERROR, "sd_printXmlCollection", 0, "unsupported scope-depth (depth=%d, max=%d).", count, SD_MAX_SCOPE_DEPTH);
    }
    assert(count <= SD_MAX_SCOPE_DEPTH);

    /* Fill module stack */
    ptr = module;
    while(count) {
        stack[count-1] = ptr;
        ptr = ptr->definedIn;
        count--;
    }

    /* ptr should be NULL */
    assert(!ptr);
}

/* Find first common module in two module-stacks */
static c_metaObject
sd_utilFirstCommonModule(
    c_metaObject from,
    c_metaObject to,
    c_metaObject* fromStack,
    c_metaObject* toStack,
    c_long* i_out)
{
    c_metaObject fromPtr, toPtr;
    c_long i;

    /* fromPtr and toPtr will initially point to base */
    i = 0;
    do {
        fromPtr = c_metaObject(fromStack[i]);
        toPtr = c_metaObject(toStack[i]);
        i++;
    }while((fromPtr != from) && (toPtr != to) && (fromStack[i] == toStack[i]));

    /* Common module is now stored in fromPtr and toPtr. */

    if(i_out) {
        *i_out = i;
    }

    return fromPtr;
}

/* Print typeref */
static void
sd_printXmlTyperef(
    sd_context context,
    c_type type)
{
    /* Print typeref. Use relative names (if possible) to obtain the shortest possible reference to another type. */
    if(c_metaObject(type)->definedIn != context->module) {
        c_metaObject fromStack[SD_MAX_SCOPE_DEPTH], toStack[SD_MAX_SCOPE_DEPTH];
        c_metaObject from, to, common;
        c_long i;

        /* Get first common module between current module and the referenced type. */
        from = context->module;
        to = c_metaObject(type)->definedIn;
        sd_utilModuleStack(from, fromStack);
        sd_utilModuleStack(to, toStack);
        sd_utilFirstCommonModule(from, to, fromStack, toStack, &i);

        sd_printXml(context, "<Type name=\"");

        /* Print modules from the common module until the current */
        i--;
        do {
            common = toStack[i];
            i++;
            if(common->name) {
                sd_printXml(context, "%s", common->name);
            }
            sd_printXml(context, "::");
        }while(common != to);

        /* Print the typename */
        sd_printXml(context, "%s\"/>", c_metaObject(type)->name);
    }else {
        /* If module of type is equal to the current, just print the typename. */
        sd_printXml(context, "<Type name=\"%s\"/>", c_metaObject(type)->name);
    }
}

/* Open module. This function finds the shortest path from the current module to the next,
 * and opens and closes modules where necessary. */
static void
sd_printXmlModuleOpen(
    sd_context context,
    c_metaObject to)
{
    c_metaObject from;

    /* If context->module is NULL, start from root */
    from = context->module;
    if(!from) {
        from = c_metaObject(c_getBase(to));
    }

    /* If from and to are not equal, find shortest path between modules. */
    if(from != to) {
        c_metaObject fromStack[SD_MAX_SCOPE_DEPTH], toStack[SD_MAX_SCOPE_DEPTH];
        c_metaObject fromPtr, toPtr;
        c_long i;

        /* Find common module. First build up a scope-stack for the two modules which
         * are ordered base -> <module>. Then walk through these stacks to find the
         * last common module. */
        sd_utilModuleStack(from, fromStack);
        sd_utilModuleStack(to, toStack);
        fromPtr = toPtr = sd_utilFirstCommonModule(from, to, fromStack, toStack, &i);

        /* Walk down from module 'from' to 'toPtr' */
        fromPtr = from;
        while(fromPtr != toPtr) {
            sd_printXml(context, "</Module>");
            fromPtr = fromPtr->definedIn;
        }

        /* Walk from toPtr to 'to' */
        while(toPtr != to) {
            toPtr = toStack[i];
            sd_printXml(context, "<Module name=\"%s\">", toPtr->name);
            i++;
        }

        /* Update context->module */
        context->module = to;
    }
}

/* Close module */
static void sd_printXmlModuleClose(
    sd_context context)
{
    c_metaObject ptr;

    if(context->module) {
        ptr = context->module;
        while((ptr = ptr->definedIn)) {
            sd_printXml(context, "</Module>");
        }

        context->module = NULL;
    }
}

/* Print type. */
static void
sd_printXmlType(
    sd_context context,
    c_type current,
    c_type type)
{
    if((c_baseObject(type)->kind == M_STRUCTURE) &&
            !(c_metaObject(type)->definedIn && c_metaObject(type)->definedIn->definedIn) &&
            !strcmp("c_time", c_metaObject(type)->name)) {
        sd_printXml(context, "<Time/>");
    }else {
        /* If object is defined outside the current scope and is not a collection or primitive, serialize a typeref.
         * Also, if the type is already defined serialize a typeref. This prevents inline types to be defined multiple
         * times if they are used multiple times within a module-scoped type. */
        if(!((c_baseObject(type)->kind == M_COLLECTION) || (c_baseObject(type)->kind == M_PRIMITIVE) ||
                (!c_iterContains(context->inlineProcessed, type) &&
                (c_metaObject(type)->definedIn == c_metaObject(current))))) {
            sd_printXmlTyperef(context, type);
        }else {
            /* Serialize type-definition. */
            switch(c_baseObject(type)->kind) {
            case M_TYPEDEF:
                sd_printXmlTypedef(context, c_typeDef(type));
                break;
            case M_ENUMERATION:
                sd_printXmlEnumeration(context, c_enumeration(type));
                break;
            case M_PRIMITIVE:
                sd_printXmlPrimitive(context, c_primitive(type));
                break;
            case M_COLLECTION:
                sd_printXmlCollection(context, current, c_collectionType(type));
                break;
            case M_STRUCTURE:
                /* Make an exception for c_time. */
                sd_printXmlStructure(context, c_structure(type));
                break;
            case M_UNION:
                sd_printXmlUnion(context, c_union(type));
                break;
            default:
                /* This may not happen. Types other than the ones listed
                 * above cannot be printed. */
                assert(0);
                break;
            }

            /* Mark type as processed. */
            context->inlineProcessed = c_iterInsert(context->inlineProcessed, type);
        }
    }
}

/* Top-level printroutine. */
static void
sd_printXmlItem(
    sd_item item,
    sd_context context)
{
    if(context->inlineProcessed) {
        c_iterFree(context->inlineProcessed);
        context->inlineProcessed = NULL;
    }

#ifdef SER_DEBUG
    printf("%s\n", c_metaScopedName(c_metaObject(item->self)));
#endif

    switch(c_baseObject(item->self)->kind) {
    case M_TYPEDEF:
    case M_STRUCTURE:
    case M_UNION:
    case M_ENUMERATION:
        sd_printXmlModuleOpen(context, c_metaObject(item->self)->definedIn);
        sd_printXmlType(context, c_type(c_metaObject(item->self)->definedIn), item->self);
        break;
    default:
        /* This may not happen. Types other than the ones listed
         * above cannot be directly printed. */
        OS_REPORT(OS_ERROR, "sd_printXmlItem", 0, "invalid typeKind for serializer.");
        assert(0);
        break;
    }

    /* Dereference dependees of item. This will populate module-objects with types
     * that reach refcount 0. */
    sd_itemDerefDependees(item);
}

static void
sd_addInitialItems(
    void* o,
    void* udata)
{
    sd_item item;

    item = o;

    /* If refcount of item is zero, it means that it has no unresolved dependencies, thus that it can
     * be processed. */
    if(!item->refcount){
        sd_itemAddToModule(item);
    }
}

struct sd_findLargestModule_t {
    sd_moduleItem largest;
};

static void
sd_findLargestModule(
    void* o,
    void* userData)
{
    sd_moduleItem module, largest;
    struct sd_findLargestModule_t* data;

    module = o;
    data = userData;
    largest = data->largest;

    /* Find largest module. */
    if(!largest) {
        /* Only set largest if module has types. */
        if(module->end - module->start) {
            data->largest = module;
        }
    }else {
        if((largest->end - largest->start) < (module->end - module->start)) {
            data->largest = module;
        }
    }
}

/* Output types in optimized module order. The algorithm attempts to reduce the number
 * of module transitions by selecting the module with the most types in each iteration.
 * The algorithm stops when all modules are empty.
 */
static void
sd_printModules(
    sd_context context)
{
    struct sd_findLargestModule_t walkData;
    sd_moduleItem module;

    /* There must be at least one type, and as such at least one module. */
    assert(context->modules);

    /* Find initial largest module */
    walkData.largest = NULL;
    c_iterWalk(context->modules, sd_findLargestModule, &walkData);

    /* There must be at least one module with types. */
    assert(walkData.largest);

    do {
        module = walkData.largest;

        /* Walk types */
        while(module->start != module->end) {
            /* If the printing of an item causes other types to be 'unlocked' (refcount becomes 0) in the current module,
             * the type will be added to the 'module' object, causing the module->end pointer to shift. This
             * automatically causes these types to be processed within the current iteration.
             */
            sd_printXmlItem(*(module->start), context);
            module->start++;
        }

        /* Lookup next largest module */
        walkData.largest = NULL;
        c_iterWalk(context->modules, sd_findLargestModule, &walkData);
    }while(walkData.largest);
}

#ifdef SER_DEBUG
static void
sd_printTypes(
    void* o,
    void* udata)
{
    printf("%s\n", c_metaScopedName(c_metaObject(((sd_item)o)->self)));
}
#endif

static void
sd_printXmlDescriptor(
    sd_context context,
    sd_serializedData* result)
{
#ifdef SER_DEBUG
    printf("=== Dependency ordered: (%d types).\n", c_iterLength(context->items));
    c_iterWalk(context->items, sd_printTypes, NULL);
#endif

    /* Set initial module to base */
    context->module = c_metaObject(context->base);

    /* Insert initial types with refcount 0 in module objects */
    c_iterWalk(context->items, sd_addInitialItems, NULL);

    /* Print xml */
    sd_printXml(context, "<MetaData version=\"1.0.0\">");

    /* Print types, ordered by modules. */
#ifdef SER_DEBUG
    printf("=== Module ordered output:\n");
#endif
    sd_printModules(context);

    /* Clean inlineProcessed. */
    if(context->inlineProcessed) {
        c_iterFree(context->inlineProcessed);
        context->inlineProcessed = NULL;
    }

    /* Close last used module */
    sd_printXmlModuleClose(context);
    sd_printXml(context, "</MetaData>");

    /* Aggegrate xml-buffers */
#ifdef SER_DEBUG
    printf("\n=== XML buffers:\n");
#endif
    sd_printXmlAggegrate(context, result);

    /* Free xmlbuff list */
    c_iterFree(context->xmlbuff);
    context->xmlbuff = NULL;

#ifdef SER_DEBUG
    printf("\n=== XML MetaDescriptor:\n");
    printf("'%s'\n", (c_char*)(*result)->data);
#endif
}

static sd_serializedData
sd_serializerXMLTypeinfoSerialize(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result;
    c_base base;
    c_bool escapeQuote;
    c_type type;
    sd_context context;
#ifdef SER_DEBUG
    os_time t, start, stop;

    start = os_timeGet();
    printf("=== Start serializing..\n");
#endif

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    escapeQuote = ((sd_serializerXMLTypeinfo)serializer)->escapeQuote;
    type = c_type(object);
    base = (c_object)c_getBase(object);
    context = sd_contextNew(base, escapeQuote);
    result = NULL;

    /* Serialize type */
    if(sd_serializeType(context, NULL, type, FALSE, NULL)) {
        goto error;
    }

    /* Print XML */
    sd_printXmlDescriptor(context, &result);

    /* Free context */
    sd_contextFree(context);

#ifdef SER_DEBUG
    stop = os_timeGet();
    t = os_timeSub(stop, start);
    printf("=== Serializing finished in %d.%09d seconds.\n", t.tv_sec, t.tv_nsec);
#endif

    return result;
error:
    return NULL;
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

typedef struct sd_elementContextCompareHelper_s {
        const c_char *name;
        sd_elementContext context;
} sd_elementContextCompareHelper;

c_bool
sd_elementContextCompareAction (
    void* o /* sd_elementContext */,
    void* arg /* sd_elementContextCompareHelper */)
{
    sd_elementContext context = (sd_elementContext)o;
    sd_elementContextCompareHelper *helper = (sd_elementContextCompareHelper*)arg;
    c_bool proceed = TRUE;

    if (context->name && (strcmp(context->name, helper->name) == 0)) {
        helper->context = context;
        proceed = FALSE;
    }

    return proceed;
}

static sd_elementContext
sd_elementContextLookup (
    sd_elementContext parent,
    const c_char *name)
{
    sd_elementContextCompareHelper helper;
    helper.context = NULL;
    helper.name = name;

    if (parent->children) {
        sd_listWalk(parent->children, sd_elementContextCompareAction, &helper);
    }
    return helper.context;
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
            /* Lookup existing module or create a new one */
            if ( (element = sd_elementContextLookup(parent, name)) == NULL ) {
                element = sd_elementContextNew(parent->info, name, o, parent, TRUE);
            }

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

    OS_UNUSED_ARG(kind);
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

#if 0
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
#endif

static c_metaObject
sd_findScopeInContext (
    sd_elementContext  context,
    c_char            *name);

typedef struct sd_findScopeArg_s {
    c_char       *name;
    c_metaObject  scope;
} sd_findScopeArg;

static c_bool
sd_findScopeInChild (
    void *obj,
    void *arg)
{
    sd_elementContext context = (sd_elementContext) obj;
    sd_findScopeArg  *info    = (sd_findScopeArg  *)arg;
    c_char           *str     = info->name;
    c_char           *cur;
    c_bool            proceed = TRUE;

    str = sd_stringDup(info->name);

    cur = strstr(str, "::");
    if ( cur ) {
        if(cur == str) {
            while(context->parent) {
                context = context->parent;
            }
        }
        *cur = '\0';
        cur += 2;
    }

    if ( cur ) {
        if ( !context->name || (context->name && (strcmp(context->name, str) == 0))) {
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
    c_char            *name)
{
    sd_findScopeArg argument;

    argument.scope = NULL;
    argument.name  = name;

    if ( context->children ) {
        sd_listWalk(context->children, sd_findScopeInChild, &argument);
    }

    if(!argument.scope && context->parent) {
        argument.scope = sd_findScopeInContext(context->parent, name);
    }

    assert(argument.scope);

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
            o = sd_findScopeInContext(scope, (c_char*)name);
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

