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
/** \file database/serialization/include/sd_serializerXMLMetadata.h
 *  \brief Declaration of the \b serializerXMLMetadata class.
 */

/* Interface */
#include "sd__serializer.h"
#include "sd_serializerXMLMetadata.h"

/* Implementation */
#include "os_stdlib.h"
#include "os_heap.h"
#include "c_collection.h"
#include "sd_misc.h"
#include "sd__serializerXML.h"
#include "sd__confidence.h"
#include "sd__resultCodesXMLMetadata.h"
#include "sd_stringsXML.h"
#include "sd__deepwalkMeta.h"
#include "c_stringSupport.h"

#define SD_FORMAT_ID      0x584DU    /* currently the same as XML */
#define SD_FORMAT_VERSION 0x0001U

#ifndef NDEBUG
/* -------------------------- checking routines -----------------------*/

/** \brief Check if a serializer is an instance of the serializerXMLMetadata
 *         class implemented in this file.
 *
 *  Functions implemented in this file assume that an instance of
 *  the serializerXMLMetadata class is sent as first parameter. This routine
 *  can be used as a confidence check to avoid mixing of instances.
 *
 *  \param serializer The serializer object (self).
 *  \return TRUE is serializer is indeed a serializerXMLMetadata instance,
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


/* ---------- Helper struct for special treatment of some attributes -------- */


typedef struct sd_specialAddresses_s {
    struct baseObject {
        c_object kind;
    } baseObject;
    struct metaObject {
        c_object definedIn;
        c_object name;
    } metaObject;
    struct ignore {
        struct type {
            c_object alignment;
            c_object base;
            c_object size;
            c_object objectCount;
        } type;
        struct structure {
            c_object references;
            c_object scope;
        } structure;
        struct v_union {
            c_object references;
            c_object scope;
        } v_union;
        struct constant {
            c_object operand;
            c_object type;
        } constant;
        struct member {
            c_object offset;
        } member;
    } ignore;
    struct abstractTypes {
        c_object structure;
        c_object type;
        c_object typeDef;
    } abstractTypes;
    struct specialTypes {
        c_object enumeration;
        c_object constant;
    } specialTypes;
    struct currentContext {
        c_set processedTypes;
        c_type existingType;
        c_bool doDefinedIn;
        c_bool doEnumeration;
        c_metaObject enumerationScope;
        c_bool doCompare;
    } currentContext;
} *sd_specialAddresses;

#define SD_CBASEOBJECTNAME  "c_baseObject"
#define SD_KINDNAME         "kind"
#define SD_CMETAOBJECTNAME  "c_metaObject"
#define SD_DEFINEDINNAME    "definedIn"
#define SD_NAMENAME         "name"
#define SD_CTYPENAME        "c_type"
#define SD_ALIGNMENTNAME    "alignment"
#define SD_BASENAME         "base"
#define SD_SIZENAME         "size"
#define SD_COUNTNAME        "objectCount"
#define SD_CSTRUCTURENAME   "c_structure"
#define SD_SCOPENAME        "scope"
#define SD_REFERENCESNAME   "references"
#define SD_CUNIONNAME       "c_union"
#define SD_CCONSTANTNAME    "c_constant"
#define SD_OPERANDNAME      "operand"
#define SD_TYPENAME         "type"
#define SD_CMEMBERNAME      "c_member"
#define SD_OFFSETNAME       "offset"
#define SD_CTYPEDEFNAME     "c_typeDef"
#define SD_CENUMERATIONNAME "c_enumeration"

static sd_specialAddresses
sd_getSpecialAddresses(
    c_base base)
{
    sd_specialAddresses result;
    c_metaObject metaObject;

    result = (sd_specialAddresses)os_malloc(sizeof(*result));
    if (result) {
        metaObject = (c_metaObject)c_resolve(base, SD_CBASEOBJECTNAME);
        SD_CONFIDENCE(metaObject);
        result->baseObject.kind = c_metaResolve(metaObject, SD_KINDNAME);
        SD_CONFIDENCE(result->baseObject.kind);
        c_free(metaObject);

        metaObject = (c_metaObject)c_resolve(base, SD_CMETAOBJECTNAME);
        SD_CONFIDENCE(metaObject);
        result->metaObject.definedIn = c_metaResolve(metaObject, SD_DEFINEDINNAME);
        SD_CONFIDENCE(result->metaObject.definedIn);
        result->metaObject.name = c_metaResolve(metaObject, SD_NAMENAME);
        SD_CONFIDENCE(result->metaObject.name);
        c_free(metaObject);

        metaObject = (c_metaObject)c_resolve(base, SD_CTYPENAME);
        SD_CONFIDENCE(metaObject);
        result->abstractTypes.type = c_keep(metaObject);
        result->ignore.type.alignment = c_metaResolve(metaObject, SD_ALIGNMENTNAME);
        SD_CONFIDENCE(result->ignore.type.alignment);
        result->ignore.type.base = c_metaResolve(metaObject, SD_BASENAME);
        SD_CONFIDENCE(result->ignore.type.base);
        result->ignore.type.size = c_metaResolve(metaObject, SD_SIZENAME);
        SD_CONFIDENCE(result->ignore.type.size);
        result->ignore.type.objectCount = c_metaResolve(metaObject, SD_COUNTNAME);
        SD_CONFIDENCE(result->ignore.type.objectCount);
        c_free(metaObject);

        metaObject = (c_metaObject)c_resolve(base, SD_CSTRUCTURENAME);
        SD_CONFIDENCE(metaObject);
        result->abstractTypes.structure = c_keep(metaObject);
        result->ignore.structure.references = c_metaResolve(metaObject, SD_REFERENCESNAME);
        SD_CONFIDENCE(result->ignore.structure.references);
        result->ignore.structure.scope = c_metaResolve(metaObject, SD_SCOPENAME);
        SD_CONFIDENCE(result->ignore.structure.scope);
        c_free(metaObject);

        metaObject = (c_metaObject)c_resolve(base, SD_CUNIONNAME);
        SD_CONFIDENCE(metaObject);
        result->ignore.v_union.references = c_metaResolve(metaObject, SD_REFERENCESNAME);
        SD_CONFIDENCE(result->ignore.v_union.references);
        result->ignore.v_union.scope = c_metaResolve(metaObject, SD_SCOPENAME);
        SD_CONFIDENCE(result->ignore.v_union.scope);
        c_free(metaObject);

        metaObject = (c_metaObject)c_resolve(base, SD_CTYPEDEFNAME);
        SD_CONFIDENCE(metaObject);
        result->abstractTypes.typeDef = c_keep(metaObject);
        c_free(metaObject);

        metaObject = (c_metaObject)c_resolve(base, SD_CENUMERATIONNAME);
        SD_CONFIDENCE(metaObject);
        result->specialTypes.enumeration = c_keep(metaObject);
        c_free(metaObject);

        metaObject = (c_metaObject)c_resolve(base, SD_CCONSTANTNAME);
        SD_CONFIDENCE(metaObject);
        result->specialTypes.constant = c_keep(metaObject);
        result->ignore.constant.operand = c_metaResolve(metaObject, SD_OPERANDNAME);
        SD_CONFIDENCE(result->ignore.constant.operand);
        result->ignore.constant.type = c_metaResolve(metaObject, SD_TYPENAME);
        SD_CONFIDENCE(result->ignore.constant.type);
        c_free(metaObject);

        metaObject = (c_metaObject)c_resolve(base, SD_CMEMBERNAME);
        SD_CONFIDENCE(metaObject);
        result->ignore.member.offset = c_metaResolve(metaObject, SD_OFFSETNAME);
        SD_CONFIDENCE(result->ignore.member.offset);
        c_free(metaObject);

        result->currentContext.existingType = NULL;
        result->currentContext.doDefinedIn = FALSE;
        result->currentContext.doEnumeration = FALSE;
        result->currentContext.enumerationScope = NULL;
        result->currentContext.doCompare = FALSE;
        result->currentContext.processedTypes =
            c_setNew((c_type)result->abstractTypes.type);
    }

    return result;
}

#undef SD_METAOBJECTNAME
#undef SD_DEFINEDINNAME
#undef SD_NAMENAME
#undef SD_TYPENAME
#undef SD_ALIGNMENTNAME
#undef SD_BASENAME
#undef SD_SIZENAME
#undef SD_STRUCTURENAME
#undef SD_SCOPENAME
#undef SD_REFERENCESNAME
#undef SD_UNIONNAME
#undef SD_MEMBERNAME
#undef SD_OFFSETNAME

static void
sd_releaseSpecialAddresses(
    sd_specialAddresses addresses)
{
    c_object *current;
    c_object *end;

    current = (c_object *)addresses;
    end = (c_object *)&(addresses->currentContext.existingType);

    while (current != end) {
        c_free(*current);
        current = &current[1];
    }
    os_free(addresses);
}

static c_bool
sd_matchesIgnoreAddresses(
    sd_specialAddresses addresses,
    c_object object)
{
    c_bool result = FALSE;
    c_object *current;
    c_object *end;

    current = (c_object *)&(addresses->ignore);
    end = (c_object *)SD_DISPLACE(current, C_ADDRESS(sizeof(addresses->ignore)));

    while (!result && (current != end)) {
        result = result || (C_ADDRESS(*current) == C_ADDRESS(object));
        current = &current[1];
    }

    return result;
}


static c_bool
sd_matchesAbstractTypeAddresses(
    sd_specialAddresses addresses,
    c_object object)
{
    c_bool result = FALSE;
    c_object *current;
    c_object *end;

    current = (c_object *)&(addresses->abstractTypes);
    end = (c_object *)SD_DISPLACE(current, C_ADDRESS(sizeof(addresses->abstractTypes)));

    while (!result && (current != end)) {
        result = result || (C_ADDRESS(*current) == C_ADDRESS(object));
        current = &current[1];
    }

    return result;
}

/* --------------- action argument for (de)serialiazation actions ---------- */

struct sd_metaActionSerArg {
    c_ulong *sizePtr;
    c_char **dataPtrPtr;
};

/* -------------------- more string helper routines -------------------*/

static c_ulong
sd_printTaggedString(
    c_string dst,
    c_string src,
    const c_char *tagName)
{
    int len;
    c_ulong result = 0;
    c_char *current;

    current = dst;
    len = os_sprintf(current, "<%s>", tagName);
    result += (c_ulong)len;
    current = SD_DISPLACE(current, C_ADDRESS(len));
    len = sd_printCharData(current, src);
    result += (c_ulong)len;
    current = SD_DISPLACE(current, C_ADDRESS(len));
    len = os_sprintf(current, "</%s>", tagName);
    result += (c_ulong)len;

    return result;
}


static void
sd_scanTaggedString(
    c_char **dst,
    c_char **src,
    c_char *tagName,
    sd_errorInfo *errorInfo)
{
    c_char *foundTagName;

    foundTagName = sd_strGetOpeningTag(src);
    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if ((foundTagName == NULL) ||
            (strncmp(foundTagName, tagName, strlen(tagName)) != 0)) {
            SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_OPENING_TAG,
                tagName, *src);
        }
    } else {
        SD_CONFIDENCE((foundTagName != NULL) &&
                      (strncmp(foundTagName, tagName, strlen(tagName)) == 0));
    }
    if (foundTagName) {
        os_free(foundTagName);
    }
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
    sd_scanCharData(dst, src, errorInfo);

    /* If an error has occurred, fill its name */
    SD_VALIDATION_ERROR_SET_NAME(errorInfo, tagName);
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

    foundTagName = sd_strGetClosingTag(src);
    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if ((foundTagName == NULL) ||
            (strncmp(foundTagName, tagName, strlen(tagName)) != 0)) {
            SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_CLOSING_TAG,
                tagName, *src);
        }
    } else {
        SD_CONFIDENCE((foundTagName != NULL) &&
                      (strncmp(foundTagName, tagName, strlen(tagName)) == 0));
    }
    if (foundTagName) {
        os_free(foundTagName);
    }

/* QAC EXPECT 2006; more than one return path is justified here */
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


static void
sd_printScopedMetaObjectName(
    c_metaObject object,
    c_char **dataPtrPtr)
{
    c_char *scopeName;
    int len;

    scopeName = c_metaScopedName(object);
    if (scopeName) {
        len = sd_printCharData(*dataPtrPtr, scopeName);
        *dataPtrPtr = SD_DISPLACE(*dataPtrPtr, C_ADDRESS(len));
        os_free(scopeName);
    }
}


static void
sd_printDefinedIn(
    c_object object,
    c_char **dataPtrPtr)
{
    c_metaObject scope;

    scope = *(c_metaObject *)object;
    sd_printScopedMetaObjectName(scope, dataPtrPtr);
}




#define SD_CHARS_SPACES  " \t\n"
static c_bool
sd_stringSkipIgnoreSpaces(
    c_char **dataPtrPtr,
    c_char *buffer)
{
    c_char *currentPtr;
    c_bool result = TRUE;

    currentPtr = buffer;

    while  (result && *currentPtr) {
        sd_strSkipChars(dataPtrPtr, SD_CHARS_SPACES);
        result = (*currentPtr == **dataPtrPtr);
        currentPtr = &(currentPtr[1]);
        *dataPtrPtr = &((*dataPtrPtr)[1]);
    }

    return result;
}
#undef SD_CHARS_SPACES

/* --------------------- Serialization driving functions -------------------- */

static void
sd_XMLMetadataSerCallbackPre(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    struct sd_metaActionSerArg *metaActionArg = (struct sd_metaActionSerArg *)actionArg;
    c_char **dataPtrPtr = metaActionArg->dataPtrPtr;
    c_ulong len;
    c_char *tagName;
    int spRes;
    sd_specialAddresses special;

    OS_UNUSED_ARG(errorInfo);
    special = (sd_specialAddresses)userData;

    /* Opening tag */
    tagName = sd_getTagName(name, type);
    spRes = os_sprintf(*dataPtrPtr, "<%s>", tagName);
    if (spRes > 0) {
        *dataPtrPtr = SD_DISPLACE(*dataPtrPtr, C_ADDRESS(spRes));
    }
    os_free(tagName);

    /* Special treatment for c_voidp definedIn */
    if (special->currentContext.doDefinedIn) {
        sd_printDefinedIn(*objectPtr, dataPtrPtr);
        special->currentContext.doDefinedIn = FALSE;
    } else {
        /* Normal situation */
        len = sd_XMLSerType(type, *objectPtr, *dataPtrPtr);
        *dataPtrPtr = SD_DISPLACE(*dataPtrPtr, len);
    }
}


static void
sd_XMLMetadataSerCallbackPost(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    struct sd_metaActionSerArg *metaActionArg = (struct sd_metaActionSerArg *)actionArg;
    c_char **dataPtrPtr = metaActionArg->dataPtrPtr;
    c_long len;
    c_char *tagName;
    c_object found;
    sd_specialAddresses special;
    c_metaObject typeInstance;

    OS_UNUSED_ARG(errorInfo);
    SD_CONFIDENCE(objectPtr);

    special = (sd_specialAddresses)userData;

    /* Closing tag */
    tagName = sd_getTagName(name, type);
    len = os_sprintf(*dataPtrPtr, "</%s>", tagName);
    if (len > 0) {
        *dataPtrPtr = SD_DISPLACE(*dataPtrPtr, C_ADDRESS(len));
    }
    os_free(tagName);

    if (sd_matchesAbstractTypeAddresses(special, c_typeActualType(type))) {
        typeInstance = *(c_metaObject *)(*objectPtr);
        found = c_remove(special->currentContext.processedTypes, typeInstance, NULL, NULL);
        SD_CONFIDENCE(found == typeInstance);
    }
}


/* An implementation for counting: do the serialization but overwrite
 * a static buffer. Count anyway and return the count.
 */
static void
sd_XMLMetadataCountCallbackPre(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    struct sd_metaActionSerArg *metaActionArg = (struct sd_metaActionSerArg *)actionArg;
    c_ulong *size = metaActionArg->sizePtr;
    c_char *start, *end;

    OS_UNUSED_ARG(errorInfo);

    start = *metaActionArg->dataPtrPtr;
    sd_XMLMetadataSerCallbackPre(name, type, objectPtr, actionArg, NULL, userData);
    end = *metaActionArg->dataPtrPtr;
    *size += (C_ADDRESS(end)-C_ADDRESS(start));
    *metaActionArg->dataPtrPtr = start;
}


static void
sd_XMLMetadataCountCallbackPost(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    struct sd_metaActionSerArg *metaActionArg = (struct sd_metaActionSerArg *)actionArg;
    c_ulong *size = metaActionArg->sizePtr;
    c_char *start, *end;

    OS_UNUSED_ARG(errorInfo);

    start = *metaActionArg->dataPtrPtr;
    sd_XMLMetadataSerCallbackPost(name, type, objectPtr, actionArg, NULL, userData);
    end = *metaActionArg->dataPtrPtr;
    *size += (C_ADDRESS(end)-C_ADDRESS(start));
    *metaActionArg->dataPtrPtr = start;
}


static c_bool
sd_XMLMetadataSerHook(
    const c_char *name,
    c_baseObject propOrMem,
    c_object *objectPtr,
    void *actionArg,
    void *userData)
{
    struct sd_metaActionSerArg *metaActionArg = (struct sd_metaActionSerArg *)actionArg;
    c_char **dataPtrPtr = metaActionArg->dataPtrPtr;
    c_ulong *size = metaActionArg->sizePtr;
    c_bool result;
    sd_specialAddresses special;
    c_object found;
    c_metaObject typeInstance;
    c_type type;
    c_char *start;
    c_char *scopedName;
    c_ulong len;

    special = (sd_specialAddresses)userData;
    SD_CONFIDENCE(propOrMem->kind == M_ATTRIBUTE);

    result = !sd_matchesIgnoreAddresses(special, (c_object)propOrMem);

    if (result) {
        switch(propOrMem->kind) {
        case M_MEMBER:
            type = c_specifier(propOrMem)->type;
        break;
        case M_ATTRIBUTE:
            type = c_property(propOrMem)->type;
        break;
        default:
            SD_CONFIDENCE(FALSE);
            type = NULL;
        }
        if (sd_matchesAbstractTypeAddresses(special, c_typeActualType(type))) {
            typeInstance = *(c_metaObject *)(*objectPtr);
            found = c_replace(special->currentContext.processedTypes,
                typeInstance, NULL, NULL);
            if (found) {
                /* Recursive type definition, do special things and stop */
                start = *dataPtrPtr;
                scopedName = c_metaScopedName(typeInstance);
                SD_CONFIDENCE(scopedName);
                if (scopedName) {
                    len = sd_printTaggedString(*dataPtrPtr, scopedName, name);
                    if (size != NULL) {
                        (*size) += len;
                    } else {
                        *dataPtrPtr = SD_DISPLACE(*dataPtrPtr, C_ADDRESS(len));
                    }
                    os_free(scopedName);
                }
                result = FALSE;
            }
        }
    }

    if (C_ADDRESS(propOrMem) == C_ADDRESS(special->metaObject.definedIn)) {
        special->currentContext.doDefinedIn = TRUE;
    }

    return result;
}


static sd_serializedData
sd_serializerXMLMetadataSerialize(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result;
    c_ulong size;
    c_char *startPtr;
    c_type type;
    c_type actualType;
    sd_deepwalkMetaContext context;
    sd_specialAddresses specialAddresses;
    struct sd_metaActionSerArg metaActionArg;
    c_char *buffer;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    specialAddresses = sd_getSpecialAddresses(c_getBase(object));
    c_insert(specialAddresses->currentContext.processedTypes, object);

    /* Can serialize structures only */
    actualType = (c_type)object;
    actualType = c_typeActualType(actualType);

    type = c_getType(object);
    SD_CONFIDENCE(c_metaObject(type)->name != NULL);
    /* Determine the size */
    size = 1U /* '\0' */;

    buffer = os_malloc(256);
    metaActionArg.sizePtr = &size;
    metaActionArg.dataPtrPtr = &buffer;

    context = sd_deepwalkMetaContextNew(sd_XMLMetadataCountCallbackPre,
        sd_XMLMetadataCountCallbackPost, sd_XMLMetadataSerHook, &metaActionArg,
        FALSE, specialAddresses);
    sd_deepwalkMeta(type, c_metaObject(type)->name, &object, context);
    SD_CONFIDENCE(c_count(specialAddresses->currentContext.processedTypes) == 0);
    sd_deepwalkMetaContextFree(context);

    os_free(buffer);


    /* Instantiate the serialized data */
    result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size);

    /* Fill the block, then do the walk */

    startPtr =  (c_char *)result->data;
    metaActionArg.sizePtr = NULL;
    metaActionArg.dataPtrPtr = &startPtr;
    c_insert(specialAddresses->currentContext.processedTypes, object);

    context = sd_deepwalkMetaContextNew(sd_XMLMetadataSerCallbackPre,
        sd_XMLMetadataSerCallbackPost, sd_XMLMetadataSerHook, &metaActionArg, FALSE,
        specialAddresses);
    sd_deepwalkMeta(type, c_metaObject(type)->name, &object, context);
    SD_CONFIDENCE(c_count(specialAddresses->currentContext.processedTypes) == 0);
    sd_deepwalkMetaContextFree(context);

    /* Terminator */
    *startPtr = 0;
    startPtr = &(startPtr[1]);

    SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(result->data)) == size);

    sd_releaseSpecialAddresses(specialAddresses);

    return result;
}


/* ----------------- Special deserialization action routines ------------- */

static c_metaObject
sd_createOrLookupScope(
    c_base base,
    c_char *scopeName)
{
    c_metaObject foundScope;
    c_metaObject prevScope = NULL;
    c_metaObject result;
    c_char *helperString, *lastName;
    c_char *currentLastName;
    c_char *currentSrc, *currentDst;
    int len;
    c_object o;

    /* if the scopeName is the empty string, then the scope is the general scope, i.e.
     * the general module, thus the base.
     */
    if(c_compareString(scopeName, "") == C_EQ)
    {
        return c_keep(c_metaObject(base));
    }
    len = strlen(scopeName) + 1;
    helperString = os_malloc(len);
    memset(helperString, 0, len);
    lastName = os_malloc(len);
    currentSrc = scopeName;
    currentDst = helperString;
    foundScope = c_keep(base);
    do {
        memset(lastName, 0, len);
        currentLastName = lastName;
        while ((*currentSrc != ':') && (*currentSrc != 0)) {
           *currentDst = *currentSrc;
           *currentLastName = *currentSrc;
           currentSrc = &(currentSrc[1]);
           currentDst = &(currentDst[1]);
           currentLastName = &(currentLastName[1]);
        }
        c_free(prevScope);
        prevScope = foundScope;
        o = c_resolve(base, helperString);
        if (o) {
            SD_CONFIDENCE((c_baseObject(o)->kind == M_MODULE) ||
                          (c_baseObject(o)->kind == M_STRUCTURE) ||
                          (c_baseObject(o)->kind == M_UNION));
        }
        foundScope = c_metaObject(o);
        while (*currentSrc == ':') {
           *currentDst = *currentSrc;
           currentSrc = &(currentSrc[1]);
           currentDst = &(currentDst[1]);
        }
    } while ((*currentSrc != 0) && (foundScope != NULL));

    if (foundScope == NULL) {
        /* Resolving failed, create all modules needed from here */
        result = c_metaDeclare(prevScope, lastName, M_MODULE);
        while (*currentSrc != 0) {
            memset(lastName, 0, len);
            currentLastName = lastName;
            while ((*currentSrc != ':') && (*currentSrc != 0)) {
               *currentDst = *currentSrc;
               *currentLastName = *currentSrc;
               currentSrc = &(currentSrc[1]);
               currentDst = &(currentDst[1]);
               currentLastName = &(currentLastName[1]);
            }
            /* result is already kept in the prevScope, so can already be freed */
            c_free(result);
            result = c_metaDeclare(result, lastName, M_MODULE);
            while (*currentSrc == ':') {
               *currentDst = *currentSrc;
               currentSrc = &(currentSrc[1]);
               currentDst = &(currentDst[1]);
            }
            /* Do not free result as it is returned by this function */
        }
    } else {
        result = c_keep(foundScope);
    }

    c_free(prevScope);
    c_free(foundScope);
    os_free(helperString);
    os_free(lastName);

    return result;
}


#define SD_BASEOBJECT_KIND_NAME      "kind"
#define SD_METAOBJECT_NAME_NAME      "name"
#define SD_METAOBJECT_DEFINEDIN_NAME "definedIn"

static void
sd_createOrLookupType(
    c_object *objectPtr,
    c_char *dataPtr,
    c_type *actualType,
    c_bool *typeExisted,
    sd_errorInfo *errorInfo,
    sd_specialAddresses special)
{
    c_metaKind kind;
    c_metaKind *kindPtr = &kind;
    c_metaKind **kindPtrPtr;
    c_char *scopeName;
    c_char *name;
    c_char *fullName;
    c_base base;
    c_type existingType;
    c_object newObject, found;
    c_metaObject scope;
    c_char **helperPtrPtr;

    /* Initialize out-params */
    *actualType = NULL;
    *typeExisted = FALSE;

    /* Free the abstract type that was created before */
    /* c_free(*objectPtr); */
    helperPtrPtr = &dataPtr;

    kindPtrPtr = &kindPtr;
    sd_XMLDeserCallbackPre(SD_BASEOBJECT_KIND_NAME,
        c_property(special->baseObject.kind)->type, (c_object *)kindPtrPtr,
        helperPtrPtr, errorInfo, NULL);
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
    sd_XMLDeserCallbackPost(SD_BASEOBJECT_KIND_NAME,
        c_property(special->baseObject.kind)->type, (c_object *)kindPtrPtr,
        helperPtrPtr, errorInfo, NULL);
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);


    sd_scanTaggedString(&name, helperPtrPtr, SD_METAOBJECT_NAME_NAME, errorInfo);
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
    sd_scanTaggedString(&scopeName, helperPtrPtr, SD_METAOBJECT_DEFINEDIN_NAME,
        errorInfo);
    if (SD_VALIDATION_ERROR(errorInfo)) {
        os_free(name);
        return;
    }

    fullName = sd_concatScoped(scopeName, name);

    /* Check if this type already exists */
    base = c_getBase(special->baseObject.kind);
    existingType = c_resolve(base, fullName);
    if (existingType != NULL) {
        /* Type already existed in the database */
        *typeExisted = TRUE;
        /* Keep a reference to the found type */
        *actualType = c_keep(existingType);
        /* transfer refcount from c_resolve */
        *(c_object *)(*objectPtr) = existingType;
    } else {
        /* Type did not yet exist, check if modules exist */
        *typeExisted = FALSE;
        /* Make an instance of the metadata of the found type */
        newObject = c_metaDefine(c_metaObject(base), kind);
        *(c_object *)(*objectPtr) = newObject; /* transfer refcount */
        *actualType = c_keep(c_getType(newObject));
        /* Bind it now */
        scope = c_metaResolve(c_metaObject(base), scopeName);
        if (!scope) {
            /* Scope not yet found, so this is a module. Create it. */
            scope = sd_createOrLookupScope(base, scopeName);
        }
        SD_CONFIDENCE(scope != NULL);
        found = c_metaBind(scope, name, (c_metaObject)newObject);
        assert(found == newObject);
        c_free(found);
        c_free(scope);
    }

    os_free(scopeName);
    os_free(name);
    os_free(fullName);
}

#undef SD_BASEOBJECT_KIND_NAME
#undef SD_METAOBJECT_NAME_NAME
#undef SD_METAOBJECT_DEFINEDIN_NAME


/* --------------------- Deserialization driving functions ----------------- */

static void
sd_XMLMetadataDeserCallbackPre(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    c_char **dataPtrPtr = (c_char **)actionArg;
    c_char *startPtr = *dataPtrPtr;
    c_char *tagName;
    c_char *openingTag;
    c_char *scopeName;
    c_bool typeAction;
    c_type actualType;
    c_bool typeExisted;
    c_metaObject scope;
    sd_specialAddresses special;
    c_char *enumName;
    c_type insertedType;

    special = (sd_specialAddresses)userData;

    if (special->currentContext.doCompare) {
        c_char *buffer;
        c_char *start;
        c_char *toFree;
        c_bool equals;
        struct sd_metaActionSerArg metaActionArg;

        start = *dataPtrPtr;
        buffer = os_malloc(256);
        toFree = buffer;
        memset(buffer, 0, 256);
        metaActionArg.dataPtrPtr = &buffer;
        metaActionArg.sizePtr = NULL;
        sd_XMLMetadataSerCallbackPre(name, type, objectPtr, &metaActionArg,
            errorInfo, userData);
        equals = sd_stringSkipIgnoreSpaces(dataPtrPtr, toFree);
        if (SD_VALIDATION_NEEDED(errorInfo)) {
            if (!equals) {
                SD_VALIDATION_SET_ERROR(errorInfo, UNMATCHING_TYPE, name,
                    start);
            }
        } else {
            SD_CONFIDENCE(equals);
        }
        os_free(toFree);
        return;
    }


    /* Opening tag */
    openingTag = sd_strGetOpeningTag(dataPtrPtr);
    tagName = sd_getTagName(name, type);
    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if ((openingTag == NULL) ||
            (strncmp(openingTag, tagName, strlen(openingTag)) != 0)) {
            SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_OPENING_TAG, tagName,
                startPtr);
        }
    } else {
        SD_CONFIDENCE((openingTag != NULL) &&
                      (strncmp(openingTag, tagName, strlen(openingTag)) == 0));
    }
    if (openingTag) {
        os_free(openingTag);
    }

    if (SD_VALIDATION_ERROR(errorInfo)) {
        os_free(tagName);
        return;
    }

    /* Determine the deserialization action */
    /* definedIn requires special actions */
    if (special->currentContext.doDefinedIn) {
        sd_scanCharData(&scopeName, dataPtrPtr, errorInfo);
        /* If an error has occurred, fill its name */
        SD_VALIDATION_ERROR_SET_NAME(errorInfo, tagName);
        SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
        if (*(c_metaObject *)(*objectPtr) == NULL) {
            scope = sd_createOrLookupScope(c_getBase(type), scopeName);
            *(c_metaObject *)(*objectPtr) = scope;
        } else {
#ifndef NDEBUG
            char *foundName;
#endif
            scope = *(c_metaObject *)(*objectPtr);
#ifndef NDEBUG
            foundName = c_metaScopedName(scope);
            SD_CONFIDENCE(strcmp(foundName, scopeName) == 0);
            os_free(foundName);
#endif
        }
        special->currentContext.doDefinedIn = FALSE;
        if (special->currentContext.doEnumeration) {
           special->currentContext.enumerationScope = c_keep(scope);
        }
        os_free(scopeName);
    } else {
        if (C_ADDRESS(type) == C_ADDRESS(special->specialTypes.constant)) {
            SD_CONFIDENCE(special->currentContext.doEnumeration);
            SD_CONFIDENCE(special->currentContext.enumerationScope);
            sd_scanCharData(&enumName, dataPtrPtr, errorInfo);
            /* If an error has occurred, fill its name */
            SD_VALIDATION_ERROR_SET_NAME(errorInfo, tagName);
            SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
            *(c_metaObject *)(*objectPtr) = c_metaDeclare(
                special->currentContext.enumerationScope,
                enumName, M_CONSTANT);
            os_free(enumName);
        } else {
            /* The data */
            /* Deserialization also updates dataPtrPtr */
            sd_XMLDeserType(type, objectPtr, dataPtrPtr, errorInfo);
        }
    }

    /* If an error has occurred, fill its name */
    SD_VALIDATION_ERROR_SET_NAME(errorInfo, tagName);
    os_free(tagName);

    /* Now determine if we start comparing with existing types from here on */
    typeAction = sd_matchesAbstractTypeAddresses(special, c_typeActualType(type));
    if (typeAction) {
        /* This is an abstract type. Lookup the concrete instance type and
         * create it, if it does not exist */
        sd_createOrLookupType(objectPtr, *dataPtrPtr, &actualType, &typeExisted,
            errorInfo, special);
        if (typeExisted) {
            special->currentContext.existingType = actualType;
            special->currentContext.doCompare = TRUE;
        } else {
            if (actualType == special->specialTypes.enumeration) {
                special->currentContext.doEnumeration = TRUE;
            }
        }

        insertedType = *(c_type *)(*objectPtr);
        c_replace(special->currentContext.processedTypes,
                  insertedType, NULL, NULL);
    }

/* QAC EXPECT 2006; more than one return path is justified here */
}

static void
sd_XMLMetadataDeserCallbackPost(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    c_char **dataPtrPtr = (c_char **)actionArg;
    c_char *startPtr = *dataPtrPtr;
    c_char *tagName;
    c_char *closingTag;
    sd_specialAddresses special;
    c_metaObject metaObject;
    c_bool typeAction;
    c_metaObject typeInstance;
    c_metaObject found;

    SD_CONFIDENCE(objectPtr);

    special = (sd_specialAddresses)userData;

    /* Do the DB finalization actions */
    typeAction = sd_matchesAbstractTypeAddresses(special, c_typeActualType(type));
    if (typeAction) {
        typeInstance = *(c_metaObject *)(*objectPtr);
        if (special->currentContext.doCompare) {
            if (c_type(typeInstance) == special->currentContext.existingType) {
                c_free(special->currentContext.existingType);
                special->currentContext.existingType = NULL;
                special->currentContext.doCompare = FALSE;
            }
        } else {
            metaObject = *(c_metaObject *)(*objectPtr);
            c_metaFinalize(metaObject);
            if (special->currentContext.doEnumeration) {
                special->currentContext.doEnumeration = FALSE;
                c_free(special->currentContext.enumerationScope);
                special->currentContext.enumerationScope = NULL;
            }
        }
        if (!special->currentContext.doCompare) {
            found = c_remove(special->currentContext.processedTypes, typeInstance, NULL, NULL);
            SD_CONFIDENCE(found == typeInstance);
            }
    }

    if (special->currentContext.doCompare) {
        c_char *buffer;
        c_char *toFree;
        c_bool equals;
        struct sd_metaActionSerArg metaActionArg;

        buffer = os_malloc(256);
        memset(buffer, 0, 256);
        toFree = buffer;
        metaActionArg.dataPtrPtr = &buffer;
        metaActionArg.sizePtr = NULL;
        sd_XMLMetadataSerCallbackPost(name, type, objectPtr, &metaActionArg,
            errorInfo, userData);
        equals = sd_stringSkipIgnoreSpaces(dataPtrPtr, toFree);
        if (SD_VALIDATION_NEEDED(errorInfo)) {
            if (!equals) {
                SD_VALIDATION_SET_ERROR(errorInfo, UNMATCHING_TYPE, name,
                    startPtr);
            }
        } else {
            SD_CONFIDENCE(equals);
        }
        os_free(toFree);
    } else {

        /* Closing tag */
        /* Check if it has the correct name */
        closingTag = sd_strGetClosingTag(dataPtrPtr);
        tagName = sd_getTagName(name, type);
        if (SD_VALIDATION_NEEDED(errorInfo)) {
            if ((closingTag == NULL) ||
                (strncmp(closingTag, tagName, strlen(closingTag)) != 0)) {
                SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_CLOSING_TAG,
                    tagName, startPtr);
            }
        } else {
            SD_CONFIDENCE((closingTag != NULL) &&
                          (strncmp(closingTag, tagName, strlen(closingTag)) == 0));
        }
        if (closingTag) {
            os_free(closingTag);
        }
        os_free(tagName);
    }
}

static c_bool
doSerHook(
    const c_char *name,
    c_baseObject propOrMem,
    c_object *objectPtr,
    char **dataPtrPtr,
    sd_specialAddresses special)
{
    struct sd_metaActionSerArg metaActionArg;
    c_bool result;
    c_char *buffer;
    c_char *toFree;
    c_bool equals;

    buffer = os_malloc(256);
    memset(buffer, 0, 256);
    toFree = buffer;
    metaActionArg.dataPtrPtr = &buffer;
    metaActionArg.sizePtr = NULL;
    result = sd_XMLMetadataSerHook(name, propOrMem, objectPtr, &metaActionArg,
        (void *)special);
    equals = sd_stringSkipIgnoreSpaces(dataPtrPtr, toFree);
    SD_CONFIDENCE(equals);
    os_free(toFree);

    return result;
}

static c_bool
sd_XMLMetadataDeserHook(
    const c_char *name,
    c_baseObject propOrMem,
    c_object *objectPtr,
    void *actionArg,
    void *userData)
{
    c_char **dataPtrPtr = (c_char **)actionArg;
    c_bool result;
    sd_specialAddresses special;
    c_metaObject typeInstance;
    c_type type;
    c_string memberName;
    c_char *scopedName;

    special = (sd_specialAddresses)userData;
    SD_CONFIDENCE(propOrMem->kind == M_ATTRIBUTE);

    if (special->currentContext.doCompare) {
        result = doSerHook(name, propOrMem, objectPtr, dataPtrPtr, special);
    } else {
        result = !sd_matchesIgnoreAddresses(special, (c_object)propOrMem);

        if (result) {
            switch(propOrMem->kind) {
            case M_MEMBER:
                type = c_specifier(propOrMem)->type;
                memberName = c_specifier(propOrMem)->name;
            break;
            case M_ATTRIBUTE:
                type = c_property(propOrMem)->type;
                memberName = c_metaObject(propOrMem)->name;
            break;
            default:
                SD_CONFIDENCE(FALSE);
                type = NULL;
                memberName = NULL;
            }
            if (sd_matchesAbstractTypeAddresses(special, c_typeActualType(type))) {
                typeInstance = *(c_metaObject *)(*objectPtr);
                if (typeInstance == NULL) {
                    scopedName = sd_peekTaggedCharData(*dataPtrPtr, memberName);
                    if (scopedName) {
                        typeInstance = c_metaResolve(
                            c_metaObject(c_getBase(propOrMem)), scopedName);
                        SD_CONFIDENCE(typeInstance);
                        *(c_metaObject *)(*objectPtr) = c_keep(typeInstance);
                        os_free(scopedName);
                        sd_scanTaggedString(&scopedName, dataPtrPtr, memberName, NULL);
                        os_free(scopedName);
                        result = FALSE;
                        c_free(typeInstance);
                    }
                } else {
                    /* This is a special case. The type has already been set
                     * before the deserializer has processed it. The only known
                     * situation for this is c_string: c_stringNew sets the
                     * subtype. */
                    special->currentContext.existingType = c_type(typeInstance);
                    special->currentContext.doCompare = TRUE;
                    result = doSerHook(name, propOrMem, objectPtr, dataPtrPtr, special);
                }
            }
        }

        if (C_ADDRESS(propOrMem) == C_ADDRESS(special->metaObject.definedIn)) {
            special->currentContext.doDefinedIn = TRUE;
        }
    }

    return result;
}




#define SD_MESSAGE_FORMAT "Error in tag %s: %s"
static c_bool
sd_serializerXMLDeserializeInternal(
    sd_serializer serializer,
    c_type type,
    const c_char *name,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    c_bool doValidation)
{
    sd_deepwalkMetaContext context;
    c_ulong errorNumber;
    c_char *errname;
    c_char *message;
    c_char *location;
    c_bool errorOccurred;
    c_char *XMLmessage;
    c_ulong size;
    c_bool result = TRUE;
    sd_specialAddresses specialAddresses;

    /* No checking, this function is used internally only */

    if (!*objectPtr) {
        if (c_typeIsRef(type)) {
            *objectPtr = NULL;
        } else {
            *objectPtr = c_new(type);
            SD_CONFIDENCE(*objectPtr);
        }
    }

    sd_serializerSetValidationState(serializer, doValidation);
    specialAddresses = sd_getSpecialAddresses(c_getBase(type));

    context = sd_deepwalkMetaContextNew(sd_XMLMetadataDeserCallbackPre,
        sd_XMLMetadataDeserCallbackPost, sd_XMLMetadataDeserHook, dataPtrPtr,
        doValidation, specialAddresses);

    sd_deepwalkMeta(type, name, objectPtr, context);

    if (doValidation) {
        errorOccurred = sd_deepwalkMetaContextGetErrorInfo(context,
            &errorNumber, &errname, &message, &location);

        if (errorOccurred) {
            size = strlen(SD_MESSAGE_FORMAT) + strlen(errname) + strlen(message) -
                   4U + 1U; /* 4 = 2strlen ("%s")*/
            XMLmessage = os_malloc(size);
            snprintf(XMLmessage, size, SD_MESSAGE_FORMAT, errname, message);
            sd_serializerSetValidationInfo(serializer, errorNumber, XMLmessage,
                sd_stringDup(location));

            os_free(message);
            /* Free the partially deserialized data */
            if (*objectPtr) {
                c_free(*objectPtr);
            }
            *objectPtr = NULL;
            result = FALSE;
        } else {
           SD_CONFIDENCE(c_count(specialAddresses->currentContext.processedTypes) == 0);
        }
    }

    sd_deepwalkMetaContextFree(context);
    sd_releaseSpecialAddresses(specialAddresses);

    return result;
}
#undef SD_MESSAGE_FORMAT


static c_object
sd_serializerXMLMetadataDeserialize(
    sd_serializer serializer,
    sd_serializedData serData,
    c_bool doValidation)
{
    c_char *xmlString, *dummy;
    c_char *typeName;
    c_char *openingTag;
    c_type resultType;
    c_object result = NULL;
    c_bool deserOK;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    xmlString = (c_char *)serData->data;

    /* First determine type from data */
    dummy = xmlString;
    openingTag = sd_strGetOpeningTag(&dummy);
    typeName = os_strdup(openingTag);
    SD_CONFIDENCE((strcmp(typeName, "c_structure") == 0) ||
                  (strcmp(typeName, "c_typeDef") == 0));

    resultType = c_resolve(serializer->base, typeName);
    /* resultType is c_class or c_struct or whatever metatype */

    if (resultType) {
        deserOK = sd_serializerXMLDeserializeInternal(serializer, resultType,
                      openingTag, &result, &xmlString, doValidation);
        /* Check if we reached the end */
        if (deserOK) {
            SD_CONFIDENCE((int)*xmlString == '\0');
        }
        c_free(resultType);
    }
    os_free(openingTag);
    os_free(typeName);

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
sd_serializerXMLMetadataNew(
    c_base base)
{
    sd_serializer result;
    struct sd_serializerVMT VMT;

    VMT.serialize = sd_serializerXMLMetadataSerialize;
    VMT.deserialize = sd_serializerXMLMetadataDeserialize;
    VMT.deserializeInto = NULL;
    VMT.toString = sd_serializerXMLToString;
    VMT.fromString = sd_serializerXMLFromString;

    result = sd_serializerNew(SD_FORMAT_ID, SD_FORMAT_VERSION, base, NULL, VMT);

    return result;
}
