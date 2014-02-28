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
/** \file services/serialization/code/sd_serializerXML.c
 *  \brief Implementation of the \b serializerXML class.
 *
 *  The \b serializerXML class is a concrete descendant of
 *  the abstract \b serializer class. It can be used to
 *  serialize to and deserialize from an XML representation
 *  of an object.
 *
 *  The XML representation of an object is described by the following
 *  definition:
 *  - Each datafield results in a separate XML tag.
 *  - XML tagnames correspond to the metadata names in the database.
 *  - Primitive values are translated to a string equivalent.
 *  - Enum values are translated to their symbolic representation.
 *  - Unions are representated by a tag named "switch", followed by
 *    a tag for the active label.
 *  - Structures are representated by a list of tags which appear in
 *    order of declaration.
 *  - Sets and variable length arrays are represented by a tag named
 *    "size", followed by a list of tags named "element".
 *  - Fixed length arrays are represented by a list of tags
 *    named "element".
 *  - The toplevel tag has a name corresponding to the scoped typename.
 *    Since double colons are not allowed in XML, the string ".." is
 *    used as scope separator.
 *
 *  An example, formatted for understandability:
\verbatim

  <AvAbsTypes..FlightPlanData>
    <ifp>
      <call_sign>
        <element>C</element>
        <element>H</element>
        <element>k</element>
      </call_sign>
      <state>TERMINATED</state>
      <fp_completion>MINIMAL_FP</fp_completion>
      <flight_rules>VFR_THEN_IFR</flight_rules>
      <acr_info>
        <nb_of_aircraft>16653</nb_of_aircraft>
        <icao_ac_type>
          <size>4</size>
          <element>4</element>
          <element>h</element>
          <element>k</element>
          <element>1</element>
        </icao_ac_type>
        <eobd>
          <switch>FALSE</switch>
          <field>148</field>
        </eobd>
        <eobt>
          <switch>TRUE</switch>
          <value>
            <sec>526</sec>
            <usec>826251133</usec>
          </value>
        </eobt>
      </acr_info>
    </ifp>
  </AvAbsTypes..FlightPlanData>\endverbatim
 */

/* interface */
#include "sd__serializer.h"
#include "sd_serializerXML.h"
#include "sd__serializerXML.h"

/* implementation */
#include "os_abstract.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "c_typebase.h"
#include "c_metabase.h"
#include "c_collection.h"
#include "c__scope.h"
#include "sd_misc.h"
#include "sd_stringsXML.h"
#include "sd_deepwalk.h"
#include "sd__deepwalkMeta.h"
#include "sd__confidence.h"
#include "sd__resultCodesXML.h"

#define SD_FORMAT_ID      0x584DU    /* "XM" */
#define SD_FORMAT_VERSION 0x0001U
#define SD_FORMAT_ID_STRING "XML"
#define SD_FORMAT_VERSION_STRING "v0.1"

#define SD_NULL_REF_IMAGE  "&lt;NULL&gt;"
#define SD_VALID_REF_IMAGE ""


#ifndef NDEBUG
/* -------------------------- checking routines -----------------------*/

/** \brief Check if a serializer is an instance of the serializerXML
 *         class implemented in this file.
 *
 *  Functions implemented in this file assume that an instance of
 *  the serializerXML class is sent as first parameter. This routine
 *  can be used as a confidence check to avoid mixing of instances.
 *
 *  \param serializer The serializer object (self).
 *  \return TRUE is serializer is indeed a serializerXML instance,
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

/* -------------------------------- Serialization ------------------------ */

c_ulong sd_XMLSerType( c_type type, c_object object, c_char *dataPtr);

static c_char *
sd_primValue(
    c_primKind primKind,
    c_object object)
{
    char buf[64];

    switch(primKind) {
        case P_ADDRESS:
        {
            snprintf(buf, sizeof(buf), "0x"PA_ADDRFMT, (PA_ADDRCAST)*((c_address*)object));
            break;
        }
        case P_BOOLEAN:
        {
            if (*((c_bool*)object)) {
                snprintf(buf, sizeof(buf), "TRUE");
            } else {
                snprintf(buf, sizeof(buf), "FALSE");
            }
            break;
        }
        case P_CHAR:
        {
            unsigned char c = *((c_char*)object);
            /* Some characters cannot be serialized to XML using their character representation, octal representation is used instead.
             * - Characters with a value < 32 or >= 127: These fall outside the ASCII printable set, and may be non-printable depending on locale / platform.
             * - Characters that may not appear in XML element content: '&', '>' and '<'
             * - To allow deserializer to recognize characters in octal representation: a literal '\'
             */
            if (c < 32 || c >= 127 || c == '&' || c == '>' || c == '<' || c == '\\') {
                snprintf(buf, sizeof(buf), "\\%03o", c);
            } else {
                *(buf) = c;
                *(buf+1) = '\0';
            }
            break;
        }
        case P_OCTET:
        {
            snprintf(buf, sizeof(buf), "%u", *((c_octet*)object));
            break;
        }
        case P_WCHAR:
        {
            snprintf(buf, sizeof(buf), "(a-wchar-value)");
            break;
        }
        case P_SHORT:
        {
            snprintf(buf, sizeof(buf), "%d", *((c_short*)object));
            break;
        }
        case P_USHORT:
        {
            snprintf(buf, sizeof(buf), "%u", *((c_ushort*)object));
            break;
        }
        case P_LONG:
        {
            snprintf(buf, sizeof(buf), "%d", *((c_long*)object));
            break;
        }
        case P_ULONG:
        {
            snprintf(buf, sizeof(buf), "%u", *((c_ulong*)object));
            break;
        }
        case P_LONGLONG:
        {
            char llstr[36];
            llstr[35] = '\0';
            snprintf(buf, sizeof(buf), "%s", os_lltostr(*((c_longlong*)object), &llstr[35]));
            break;
        }
        case P_ULONGLONG:
        {
            char llstr[36];
            llstr[35] = '\0';
            snprintf(buf, sizeof(buf), "%s", os_ulltostr(*((c_ulonglong*)object), &llstr[35]));
            break;
        }
        case P_FLOAT:
        {
            snprintf(buf, sizeof(buf), "%0.7g", *((c_float*)object));
            break;
        }
        case P_DOUBLE:
        {
            snprintf(buf, sizeof(buf), "%0.15g", *((c_double*)object));
            break;
        }
        case P_VOIDP:
        {
            snprintf(buf, sizeof(buf), "0x"PA_ADDRFMT, (PA_ADDRCAST)object); break;
            break;
        }
        case P_MUTEX:
        case P_LOCK:
        case P_COND:
        case P_COUNT:
        {
            snprintf(buf, sizeof(buf), "(an-undefined-value)");
            break;
        }
        default:
        {
            snprintf(buf, sizeof(buf), "(an-undefined-value)");
            SD_CONFIDENCE(FALSE);
            break;
        }
    }

    return os_strdup(buf);
}

static c_ulong
sd_printPrim(
    c_primKind primKind,
    c_object object,
    c_char *dataPtr)
{
    c_char *valueImage;
    c_ulong result;
    int spRes;

    valueImage = sd_primValue(primKind, object);
    spRes = os_sprintf(dataPtr, "%s", valueImage);
    if (spRes >= 0) {
        result = (c_ulong)spRes;
    } else {
        result = 0;
        SD_CONFIDENCE(spRes >= 0);
    }

    os_free(valueImage);

    return result;
}

static c_ulong
sd_printTaggedPrim(
    const c_char *tagName,
    c_primKind primKind,
    c_object object,
    c_char *dataPtr)
{
    c_char *valueImage;
    c_ulong result;
    int spRes;

    valueImage = sd_primValue(primKind, object);
    spRes = os_sprintf(dataPtr, "<%s>%s</%s>", tagName, valueImage, tagName);
    if (spRes >= 0) {
        result = (c_ulong)spRes;
    } else {
        result = 0;
        SD_CONFIDENCE(spRes >= 0);
    }

    os_free(valueImage);

    return result;
}

static c_bool
sd_isValidReference(
    c_object object)
{
    return (object && (*(c_object *)object));
}


static c_ulong
sd_printReference(
    c_object object,
    c_char *dataPtr)
{
    c_ulong result;
    int snpRes;

    /* Check if this is a null reference. In that case, we should still print
       something */
    if (sd_isValidReference(object)) {
        snpRes = snprintf(dataPtr, (unsigned int)sizeof(SD_VALID_REF_IMAGE),
                          "%s", SD_VALID_REF_IMAGE);
    } else {
        snpRes = snprintf(dataPtr, (unsigned int)sizeof(SD_NULL_REF_IMAGE),
                          "%s", SD_NULL_REF_IMAGE);
    }
    if (snpRes >= 0) {
       result = (c_ulong)snpRes;
    } else {
       result = 0;
       SD_CONFIDENCE(snpRes >= 0);
    }

    return result;
}


static c_ulong
sd_printName(
    c_string dst,
    c_string src)
{
    c_ulong result;
    int spRes;

    if (src == NULL) {
        spRes = 0;
    } else {
        spRes = os_sprintf(dst, src);
    }

    if (spRes >= 0) {
        result = (c_ulong)spRes;
    } else {
        result = 0;
        SD_CONFIDENCE(spRes >= 0);
    }

    return result;
}


#define DO_ESCAPING

#define SD_STRING_OPENER "<![CDATA["
#define SD_STRING_CLOSER "]]>"


#ifndef DO_ESCAPING

c_ulong
sd_printCData(
    c_string dst,
    c_string src)
{
    c_ulong result;
    int spRes;

    SD_CONFIDENCE(src);

    spRes = os_sprintf(dst, SD_STRING_OPENER "%s" SD_STRING_CLOSER, src);

    if (spRes >= 0) {
        result = (c_ulong)spRes;
    } else {
        result = 0;
        SD_CONFIDENCE(spRes >= 0);
    }

    return result;
}

#else

struct escapePair {
    char        token;
    const char *escapeString;
};

static struct escapePair escapePairs[] = {
    {'&', "&amp;"},
    {'<', "&lt;"},
    {'>', "&gt;"},
    {'\0', NULL}  /* Terminator for this array */
};

static void
getFirstIllegalToken(
    const char *src,
    const char **strFound,
    struct escapePair *pairFound)
{
    int index = 0;
    const char *firstIllegal;
    const char *currentIllegal = NULL;
    struct escapePair pair;

    pairFound->token = '\0';
    pairFound->escapeString = NULL;
    firstIllegal = &(src[strlen(src)]);
    pair = escapePairs[index];
    while (pair.escapeString != NULL) {
        currentIllegal = strchr(src, pair.token);
        if (currentIllegal != NULL) {
            if (C_ADDRESS(currentIllegal) < C_ADDRESS(firstIllegal)) {
                firstIllegal = currentIllegal;
                *pairFound = pair;
            }
        }
        index++;
        pair = escapePairs[index];
    }
    *strFound = firstIllegal;
}

static void
getFirstEscapeString(
    const char *src,
    const char **strFound,
    struct escapePair *pairFound)
{
    int index = 0;
    const char *firstEscape;
    const char *currentEscape = NULL;
    struct escapePair pair;

    pairFound->token = '\0';
    pairFound->escapeString = NULL;
    firstEscape = &(src[strlen(src)]);
    pair = escapePairs[index];
    while (pair.escapeString != NULL) {
        currentEscape = strstr(src, pair.escapeString);
        if (currentEscape != NULL) {
            if (C_ADDRESS(currentEscape) < C_ADDRESS(firstEscape)) {
                firstEscape = currentEscape;
                *pairFound = pair;
            }
        }
        index++;
        pair = escapePairs[index];
    }
    *strFound = firstEscape;
}

c_ulong
sd_printCharData(
    c_string dst,
    c_string src)
{
    c_ulong result;
    int spRes;
    unsigned int len;
    struct escapePair pair;
    const char *from;
    char *to;
    const char *toEscape;

    SD_CONFIDENCE(src);

    from = src;
    to = dst;
    spRes = 0;
    do {
        getFirstIllegalToken(from, &toEscape, &pair);
        len = C_ADDRESS(toEscape) - C_ADDRESS(from);
        if (len > 0) {
            os_strncpy(to, from, len);
            from = &(from[len]);
            to = &(to[len]);
            spRes += len;
        }

        if (pair.escapeString != NULL) {
           len = strlen(pair.escapeString);
           os_strncpy(to, pair.escapeString, len);
           from = &(from[1]);
           to = &(to[len]);
           spRes += len;
        }
    } while (pair.escapeString != NULL);

    if (spRes >= 0) {
        result = (c_ulong)spRes;
    } else {
        result = 0;
        SD_CONFIDENCE(spRes >= 0);
    }

    return result;
}

#endif

#define SD_SIZE_TAGNAME "size"
static c_ulong
sd_XMLSerCollection(
    c_collectionType collectionType,
    c_object object,
    c_char *dataPtr)
{
    c_long colSize;
    c_ulong result;

    /* Different behaviour for reftypes and non-reftypes */
    if (((collectionType->kind == C_ARRAY) ||
         (collectionType->kind == C_SEQUENCE)) &&
         !(int)c_typeIsRef(c_type(collectionType))) {

       result = 0;

    }  else {

        /* First serialize validity of this object */
        result = sd_printReference(object, dataPtr);

        if (sd_isValidReference(object)) {
            /* Only serialize the collection size in case of list/set/bag/etc */
            switch (collectionType->kind) {
            case C_STRING:
                result += sd_printCharData((c_string)dataPtr, *((c_string *)(object)));
            break;
            case C_ARRAY:
                SD_CONFIDENCE(c_typeIsRef(c_type(collectionType)));

                if (collectionType->maxSize == 0) {
                    colSize = c_arraySize(*((c_array *)(object)));
                    result += sd_printTaggedPrim(SD_SIZE_TAGNAME, P_LONG, (c_object)&colSize, dataPtr);
                }
            break;
            case C_SEQUENCE:
                SD_CONFIDENCE(c_typeIsRef(c_type(collectionType)));
                colSize = c_arraySize(*((c_array *)(object)));
                result += sd_printTaggedPrim(SD_SIZE_TAGNAME, P_LONG, (c_object)&colSize, dataPtr);
            break;
            case C_SET:
            case C_LIST:
            case C_BAG:
            case C_DICTIONARY:
            case C_QUERY:
                colSize = c_count(*((c_collection *)(object)));
                result += sd_printTaggedPrim(SD_SIZE_TAGNAME, P_LONG, (c_object)&colSize, dataPtr);
            break;
            case C_SCOPE:
                colSize = c_scopeCount(*((c_scope *)(object)));
                result += sd_printTaggedPrim(SD_SIZE_TAGNAME, P_LONG, (c_object)&colSize, dataPtr);
            break;
            default:
                SD_CONFIDENCE(FALSE); /* No other collection types supported */
            break;
            }
        }
    }

    return result;
}


static c_ulong
sd_XMLSerPrimitive(
    c_primitive primitive,
    c_object object,
    c_char *dataPtr)
{
    c_ulong result;

    result = sd_printPrim(primitive->kind, object, dataPtr);

    return result;
}


static c_ulong
sd_XMLSerEnumeration(
    c_enumeration enumeration,
    c_object object,
    c_char *dataPtr)
{
    c_literal literal;
    c_ulong result;
    c_long value, index;
    c_long i, size;

    /* First determine the index of the enum-value */
    value = *(c_long *)object;
    index = -1;
    size = c_arraySize(enumeration->elements);
    for (i=0; (i<size) && (index == -1); i++) {
         literal = c_operandValue(c_constant(enumeration->elements[i])->operand);
         SD_CONFIDENCE(literal->value.kind == V_LONG);
         if (literal->value.is.Long == value) {
             index = i;
         }
         c_free(literal);
    }
    if (index == -1) {
        /* This enumeration was filled with an incorrect value */
        /* Continue anyway if we have to */
        index = 0;
        SD_CONFIDENCE (index != -1);
    }
    result = sd_printName((c_string)dataPtr,
                             c_metaObject(enumeration->elements[index])->name);

    return result;
}

static c_ulong
sd_XMLSerInterface(
    c_interface interf,
    c_object object,
    c_char *dataPtr)
{
    c_ulong result;

    OS_UNUSED_ARG(interf);

     SD_CONFIDENCE((c_baseObject(interf)->kind == M_CLASS) ||
                   (c_baseObject(interf)->kind == M_INTERFACE));

    result = sd_printReference(object, dataPtr);

    return result;
}

/* non-static for reuse by descendents */
c_ulong
sd_XMLSerType(
    c_type type,
    c_object object,
    c_char *dataPtr)
{
    c_ulong result;

    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        result = sd_XMLSerCollection(c_collectionType(type), object, dataPtr);
    break;
    case M_PRIMITIVE:
        result = sd_XMLSerPrimitive(c_primitive(type), object, dataPtr);
    break;
    case M_ENUMERATION:
        result = sd_XMLSerEnumeration(c_enumeration(type), object, dataPtr);
    break;
    case M_UNION:
    case M_STRUCTURE:
        /* Members, switchType and data will be called automatically */
        result = 0;
    break;
    case M_INTERFACE:
    case M_CLASS:
        /* Interfaces and classes are to be treated equally */
        result = sd_XMLSerInterface(c_interface(type), object, dataPtr);
    break;
    default:
        SD_CONFIDENCE(FALSE); /* No other expected than these */
        result = 0;
    break;
    }

    return result;
}



/* --------------------- Serialization driving functions -------------------- */

static void
sd_XMLSerCallbackPre(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    c_char **dataPtrPtr = (c_char **)actionArg;
    c_ulong len;
    c_char *tagName, *typeName = NULL;
    int spRes;

    OS_UNUSED_ARG(errorInfo);
    OS_UNUSED_ARG(userData);

    /* Opening tag */
    tagName = name ? sd_stringDup(name) : sd_stringDup("object");
    if(!name){
        typeName = sd_getScopedTypeName(type, "::");
        sd_strEscapeXML(&typeName);
    }
    if(typeName){
        spRes = os_sprintf(*dataPtrPtr, "<%s type=\"%s\">", tagName, typeName);
        os_free(typeName);
    } else {
        spRes = os_sprintf(*dataPtrPtr, "<%s>", tagName);
    }
    if (spRes > 0) {
        *dataPtrPtr = SD_DISPLACE(*dataPtrPtr, C_ADDRESS(spRes));
    }
    os_free(tagName);

    /* The data */
    len = sd_XMLSerType(type, *objectPtr, *dataPtrPtr);
    *dataPtrPtr = SD_DISPLACE(*dataPtrPtr, len);
}


static void
sd_XMLSerCallbackPost(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    c_char **dataPtrPtr = (c_char **)actionArg;
    c_long len;

    OS_UNUSED_ARG(objectPtr);
    OS_UNUSED_ARG(errorInfo);
    OS_UNUSED_ARG(userData);

    /* Closing tag */
    len = os_sprintf(*dataPtrPtr, "</%s>", name ? name : "object");
    if (len > 0) {
        *dataPtrPtr = SD_DISPLACE(*dataPtrPtr, C_ADDRESS(len));
    }
}


/* An implementation for counting: do the serialization but overwrite
   a static buffer. Count anyway and return the count. */
static void
sd_XMLCountCallback(
    const c_char *name,
    c_type type,
    c_object *objectPtr,
    void *actionArg,
    sd_errorInfo *errorInfo,
    void *userData)
{
    c_ulong *size = (c_ulong *)actionArg;
    c_char *buf, *str;
    c_char *start, *end;
    int len;

    OS_UNUSED_ARG(errorInfo);

    len = 256;
    if ((c_baseObject(type)->kind == M_COLLECTION) &&
        (c_collectionType(type)->kind == C_STRING)) {
       str = *((c_string *)(*objectPtr));
       if (str != NULL) {
           len += 5*strlen(str); /* 5 based on length of longest escapestring */
       }
    }
    buf = os_malloc(len);
    start = buf;
    end = buf;
    sd_XMLSerCallbackPre(name, type, objectPtr, &end, NULL, userData);
    sd_XMLSerCallbackPost(name, type, objectPtr, &end, NULL, userData);
    *size += (C_ADDRESS(end)-C_ADDRESS(start)+10);
    /* add extra 10 bytes for every xml object value to have extra space for number changes because the object may not be locked */
    os_free(buf);
}


static sd_serializedData
sd_serializerXMLSerialize(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result;
    c_ulong size;
    c_char *startPtr;
    c_type type;
    sd_deepwalkMetaContext context;

    OS_UNUSED_ARG(serializer);
    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    type = c_getType(object);
    /* Determine the size */
    size = 1U /* '\0' */;

    context = sd_deepwalkMetaContextNew(sd_XMLCountCallback, NULL, NULL, &size,
        FALSE, NULL);
    sd_deepwalkMeta(type, NULL, &object, context);
    sd_deepwalkMetaContextFree(context);


    /* Instantiate the serialized data */
    result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size);

    /* Then do the walk */
    startPtr =  (c_char *)result->data;

    context = sd_deepwalkMetaContextNew(sd_XMLSerCallbackPre,
        sd_XMLSerCallbackPost, NULL, &startPtr, FALSE, NULL);
    sd_deepwalkMeta(type, NULL, &object, context);
    sd_deepwalkMetaContextFree(context);

    /* Terminator */
    *startPtr = 0;
    startPtr = &(startPtr[1]);

    SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(result->data)) <= size);
    return result;
}


static sd_serializedData
sd_serializerXMLSerializeTyped(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result;
    c_ulong size;
    c_char *startPtr;
    sd_deepwalkMetaContext context;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    /* Determine the size */
    size = 1U /* '\0' */;

    context = sd_deepwalkMetaContextNew(sd_XMLCountCallback, NULL, NULL, &size,
        FALSE, NULL);
    sd_deepwalkMeta(serializer->type, "object", &object, context);
    sd_deepwalkMetaContextFree(context);

    /* Instantiate the serialized data */
    result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size);

    /* Then do the walk */
    startPtr =  (c_char *)result->data;
    context = sd_deepwalkMetaContextNew(sd_XMLSerCallbackPre,
        sd_XMLSerCallbackPost, NULL, &startPtr, FALSE, NULL);
    sd_deepwalkMeta(serializer->type, "object", &object, context);
    sd_deepwalkMetaContextFree(context);

    /* Terminator */
    *startPtr = 0;
    startPtr = &(startPtr[1]);

    if ((C_ADDRESS(startPtr) - C_ADDRESS(result->data)) > size) {
        OS_REPORT_3(OS_ERROR, "sd_serialize", 0,
                          "Startptr: 0x%x result: 0x%x size: %d", startPtr,result->data,size);
        }
    SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(result->data)) <= size);

    return result;
}


/* -------------------------------- Deserialization ------------------------ */

void sd_XMLDeserType(c_type type, c_object *objectPtr,
                            c_char **dataPtrPtr, sd_errorInfo *errorInfo);


#define SD_CHARS_CHAR    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define SD_CHARS_DIGIT   "1234567890"
/* Note: Some characters are serialized using octal representation. These are omitted from the following set,
 * except for '\', required to recognize the octal representations (i.e. '\001') */
#define SD_CHARS_SPECIAL "!\"#$%'()*+,-./:;=?@[\\]^_`{|}~"
#define SD_CHARS_SPACES  " \t\n"

#define SD_SKIP_SPACES   SD_CHARS_SPACES
#define SD_SKIP_CHAR     SD_CHARS_CHAR SD_CHARS_DIGIT SD_CHARS_SPECIAL SD_SKIP_SPACES
#define SD_SKIP_INT      SD_CHARS_DIGIT "-"                            SD_SKIP_SPACES
#define SD_SKIP_UINT     SD_CHARS_DIGIT                                SD_SKIP_SPACES
#define SD_SKIP_FLOAT    SD_CHARS_DIGIT "+-Ee.Nan" /* NaN/nan */       SD_SKIP_SPACES
#define SD_SKIP_STRING   SD_CHARS_CHAR SD_CHARS_DIGIT SD_CHARS_SPECIAL SD_SKIP_SPACES
#define SD_SKIP(x)       SD_SKIP_##x

static void
sd_skipPrim(
    c_primKind primKind,
    c_char **dataPtrPtr)
{
#define __CASE__(kind,skipType) case kind: sd_strSkipChars(dataPtrPtr, SD_SKIP(skipType)); break;
    switch (primKind) {
    __CASE__(P_ADDRESS,UINT)
    __CASE__(P_BOOLEAN,STRING)
    __CASE__(P_CHAR,CHAR)
    __CASE__(P_OCTET,UINT)
    __CASE__(P_WCHAR,CHAR)
    __CASE__(P_SHORT,INT)
    __CASE__(P_USHORT,UINT)
    __CASE__(P_LONG,INT)
    __CASE__(P_ULONG,UINT)
    __CASE__(P_LONGLONG,INT)
    __CASE__(P_ULONGLONG,UINT)
    __CASE__(P_FLOAT,FLOAT)
    __CASE__(P_DOUBLE,FLOAT)
    default:
        SD_CONFIDENCE(FALSE);
    break;
    }
#undef __CASE__
/* QAC EXPECT 5101; cyclomatic complexity is no problem here */
}

#define SD_F_PREF     "%"
#define SD_F_NONE     ""
#define SD_F_BYTE     SD_F_NONE
#define SD_F_SHORT    "h"
#define SD_F_LONG     "l"
#define SD_F_LONGLONG "ll"
#define SD_F_CHAR     "c"
#define SD_F_INT      "d"
#define SD_F_UINT     "u"
#define SD_F_FLOAT    "f"

#define SD_FORMAT_ADDRESS      PA_ADDRFMT
#define SD_FORMAT_CHAR         SD_F_PREF SD_F_BYTE     SD_F_CHAR
#define SD_FORMAT_OCTET        SD_F_PREF SD_F_BYTE     SD_F_CHAR
#define SD_FORMAT_BOOLEAN      SD_F_PREF SD_F_BYTE     SD_F_CHAR
#define SD_FORMAT_SHORT        SD_F_PREF SD_F_SHORT    SD_F_INT
#define SD_FORMAT_USHORT       SD_F_PREF SD_F_SHORT    SD_F_UINT
#define SD_FORMAT_WCHAR        SD_F_PREF SD_F_NONE     SD_F_CHAR
#define SD_FORMAT_LONG         SD_F_PREF SD_F_NONE     SD_F_INT
#define SD_FORMAT_ULONG        SD_F_PREF SD_F_NONE     SD_F_UINT
#define SD_FORMAT_ENUM         SD_F_PREF SD_F_NONE     SD_F_INT
#define SD_FORMAT_LONGLONG     SD_F_PREF SD_F_LONGLONG SD_F_INT
#define SD_FORMAT_ULONGLONG    SD_F_PREF SD_F_LONGLONG SD_F_UINT
#define SD_FORMAT_FLOAT        SD_F_PREF SD_F_NONE     SD_F_FLOAT
#define SD_FORMAT_DOUBLE       SD_F_PREF SD_F_LONG     SD_F_FLOAT
/* #define SD_FORMAT_LONGDOUBLE   SD_F_PREF SD_F_LONGLONG SD_F_FLOAT */
#define SD_FORMAT(x) SD_FORMAT_##x


/* Special routine for scanning booleans */
/* scanf does not support this           */
#define SD_TRUE_STRING "TRUE"
#define SD_FALSE_STRING "FALSE"
static c_long
sd_scanBooleanNoSkip(
    c_char *dataPtr,
    c_bool *resultPtr)
{
    c_long result;
    c_char *helperPtr;
    c_char *helperString;

    result = 0;
    helperPtr = dataPtr;
    sd_strSkipChars(&helperPtr, SD_SKIP_SPACES);
    helperString = sd_strGetChars(&helperPtr, "TRUEFALStruefals");
    if (os_strncasecmp(helperString, SD_TRUE_STRING,
                    (unsigned int)sizeof(SD_TRUE_STRING)) == 0) {
        *resultPtr = TRUE;
        result = 1;
    } else {
        if (os_strncasecmp(helperString, SD_FALSE_STRING,
                        (unsigned int)sizeof(SD_FALSE_STRING)) == 0) {
            *resultPtr = FALSE;
            result = 1;
        }
    }
    os_free(helperString);

    return result;
}
#undef SD_TRUE_STRING
#undef SD_FALSE_STRING

#define SD_FIRST_DIGIT '0'
#define SD_LAST_DIGIT  '9'
/* Special routine for scanning single bytes */
/* scanf does not support this               */
static c_long
sd_scanOctetNoSkip(
    c_char *dataPtr,
    c_octet *resultPtr)
{
    c_octet helperValue;
    c_char *helperPtr;
    c_long resultValue;
    c_long result;
    int i;

    result = 0;
    i = 0;
    *resultPtr = 0;
    resultValue = 0;
    helperPtr = dataPtr;
    sd_strSkipChars(&helperPtr, SD_SKIP_SPACES);
    i = (*helperPtr != 0), helperValue = *helperPtr;
    while (i==1) {
        if (((int)helperValue >= SD_FIRST_DIGIT) &&
            ((int)helperValue <= SD_LAST_DIGIT)) {
             result = 1;
             resultValue = (resultValue * 10) + ((int)helperValue-SD_FIRST_DIGIT);
             helperPtr = &(helperPtr[1]);
             if (resultValue <= 255) {
                 i = (*helperPtr != 0), helperValue = *helperPtr;
             } else {
                 result = 0;
                 i = 0;
             }
        } else {
           i = 0;
        }
    }

    if (result != 0) {
        *resultPtr = (c_octet)resultValue;
    }

    return result;
}

/* Special routine for scanning single characters     */
/* scanf does not support special charaters like \000 */
#define SD_FIRST_OCTAL_DIGIT '0'
#define SD_LAST_OCTAL_DIGIT  '7'

static c_long
sd_scanCharNoSkip(
    c_char *dataPtr,
    c_char *resultPtr)
{
    c_char helperValue;
    c_char *helperPtr;
    c_long resultValue;
    c_long result;
    int count;

    result = 0;
    *resultPtr = 0;
    resultValue = 0;
    helperPtr = dataPtr;
    result = (*helperPtr != 0), helperValue = *helperPtr;

    if (result == 1) {
        if ((int)helperValue == (int)'\\') {
            /* Going into escape mode, read octal digit characters */
            helperPtr = &(helperPtr[1]);
            for (count = 1; (count <= 3) && (result == 1); count++) {
                result = (*helperPtr != 0), helperValue = *helperPtr;
                if ((result == 1) &&
                    ((int)helperValue >= SD_FIRST_OCTAL_DIGIT) &&
                    ((int)helperValue <= SD_LAST_OCTAL_DIGIT)) {
                    resultValue = (resultValue * 8) +
                        ((int)helperValue-SD_FIRST_OCTAL_DIGIT);
                    helperPtr = &(helperPtr[1]);
                } else {
                    /* Failure... */
                    result = 0;
                }
            }
        } else {
           resultValue = (c_long)helperValue;
        }
    }

    if (result != 0) {
        *resultPtr = (c_char)resultValue;
    }

    return result;
}

/* Special routine for scanning references */
/* The value can be either NULL or (empty) */
/* scanf does not support this             */
static c_bool
sd_scanReference(
    c_char **dataPtr,
    c_bool stringRef,
    sd_errorInfo *errorInfo)
{
    c_bool result = FALSE;
    c_char *startPtr = *dataPtr;

    /* Check whether the string pointed to by *dataPtr contains the special
     * NULL-value. */
    sd_strSkipChars(dataPtr, SD_SKIP_SPACES);
    if (strncmp(SD_NULL_REF_IMAGE, *dataPtr, sizeof(SD_NULL_REF_IMAGE)-1) == 0) {
        result = FALSE;
        *dataPtr = &((*dataPtr)[sizeof(SD_NULL_REF_IMAGE)-1]);
    } else {
        if (stringRef) {
            result = TRUE;
            /* Undo skips */
            *dataPtr = startPtr;
        } else {
            if (strncmp("</", startPtr, 2) == 0) {
                /* Nothing enclosed between open- and close-tag, this is NULL as well. */
                result = FALSE;
            } else if (**dataPtr == '<') {
                /* Open tag of the referenced data */
                result = TRUE;
            } else {
                if (SD_VALIDATION_NEEDED(errorInfo)) {
                    SD_VALIDATION_SET_ERROR(errorInfo, INVALID_REFERENCE_FORMAT, NULL, startPtr);
                } else {
                    SD_CONFIDENCE((c_long)result == TRUE);
                }
                result = FALSE;
            }
        }
    }

    return result;
}

static void
sd_scanPrim(
    c_primKind primKind,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    int i;

#define SD_SCAN_PRIM_CHECK(kind)                                               \
        if (i != 1) {                                                          \
            if (SD_VALIDATION_NEEDED(errorInfo)) {                             \
                SD_VALIDATION_SET_ERROR(errorInfo, INVALID_##kind##_FORMAT, NULL, *dataPtrPtr)    \
            } else {                                                           \
               SD_CONFIDENCE(i==1);                                            \
            }                                                                  \
        }

#define SD_SCAN_PRIM_CASE(kind,type)                                           \
    case P_##kind:                                                             \
        i = sscanf(*dataPtrPtr, SD_FORMAT(kind), (type *)(*objectPtr));        \
        SD_SCAN_PRIM_CHECK(kind)                                                        \
    break;

    switch (primKind) {
    /* SD_SCAN_PRIM_CASE(CHAR,c_char) */
    SD_SCAN_PRIM_CASE(ADDRESS,c_address)
    SD_SCAN_PRIM_CASE(WCHAR,c_char)
    SD_SCAN_PRIM_CASE(SHORT,c_short)
    SD_SCAN_PRIM_CASE(USHORT,c_ushort)
    SD_SCAN_PRIM_CASE(LONG,c_long)
    SD_SCAN_PRIM_CASE(ULONG,c_ulong)
    SD_SCAN_PRIM_CASE(LONGLONG,c_longlong)
    SD_SCAN_PRIM_CASE(ULONGLONG,c_ulonglong)
    SD_SCAN_PRIM_CASE(FLOAT,c_float)
    SD_SCAN_PRIM_CASE(DOUBLE,c_double)
    /* SD_SCAN_PRIM_CASE(OCTET,c_octet) */
    /* SD_SCAN_PRIM_CASE(BOOLEAN,c_bool) */
    case P_CHAR:
        /* scanf does not support characters like \000 */
        i = sd_scanCharNoSkip(*dataPtrPtr, (c_char *)(*objectPtr));
        SD_SCAN_PRIM_CHECK(CHAR)
    break;
    case P_BOOLEAN:
        /* TRUE or FALSE */
        i = sd_scanBooleanNoSkip(*dataPtrPtr, (c_bool *)(*objectPtr));
        SD_SCAN_PRIM_CHECK(BOOLEAN)
    break;
    case P_OCTET:
        /* Scanning of octet not supported by scanf */
        i = sd_scanOctetNoSkip(*dataPtrPtr, (c_octet *)(*objectPtr));
        SD_SCAN_PRIM_CHECK(OCTET)
    break;
#if 0
    case P_DOUBLE:
        /* Scanning of double not supported by scanf */
        i = 0;
        SD_CONFIDENCE(FALSE);
    break;
#endif
    default:
        i = 0;
        SD_CONFIDENCE(FALSE);
    break;
    }

    if (i==1) {
        sd_skipPrim(primKind, dataPtrPtr);
    }

#undef SD_SCAN_PRIM_CASE
#undef SD_SCAN_PRIM_CHECK
/* QAC EXPECT 5101; cyclomatic complexity is no problem here */
}


static void
sd_scanTaggedPrim(
    const c_char *tagName,
    c_primKind primKind,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    c_char *currentTag;
    c_char *startPtr;

    startPtr = *dataPtrPtr;
    currentTag = sd_strGetOpeningTag(dataPtrPtr);
    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if ((currentTag == NULL) ||
            (strncmp(tagName, currentTag, strlen(currentTag)) != 0)) {
            SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_OPENING_TAG, tagName, startPtr);
        }
    } else {
        SD_CONFIDENCE((currentTag != NULL) &&
                      (strncmp(tagName, currentTag, strlen(currentTag)) == 0));
    }
    if (currentTag) {
        os_free(currentTag);
    }
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

    sd_scanPrim(primKind, objectPtr, dataPtrPtr, errorInfo);
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

    startPtr = *dataPtrPtr;
    currentTag = sd_strGetClosingTag(dataPtrPtr);
    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if ((currentTag == NULL) ||
            (strncmp(tagName, currentTag, strlen(currentTag)) != 0)) {
            SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_CLOSING_TAG, tagName, startPtr);
        }
    } else {
        SD_CONFIDENCE((currentTag != NULL) &&
                      (strncmp(tagName, currentTag, strlen(currentTag)) == 0));
    }
    if (currentTag) {
        os_free(currentTag);
    }
/* QAC EXPECT 2006; more than one return path is justified here */
}


#ifndef DO_ESCAPING

void
sd_scanCharData(
    c_char **dst,
    c_char **src,
    sd_errorInfo *errorInfo)
{
    c_char *strStart;
    c_char *strEnd;
    int len;

    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if (strncmp(*src, SD_STRING_OPENER, sizeof(SD_STRING_OPENER)-1) != 0) {
            SD_VALIDATION_SET_ERROR(errorInfo, INVALID_STRING_FORMAT, NULL, *src);
        }
    } else {
        SD_CONFIDENCE(strncmp(*src, SD_STRING_OPENER, sizeof(SD_STRING_OPENER)-1) == 0);
    }
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

    strStart = &((*src)[sizeof(SD_STRING_OPENER)-1]);
    strEnd = strstr(strStart, SD_STRING_CLOSER);
    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if (!strEnd) {
            SD_VALIDATION_SET_ERROR(errorInfo, INVALID_STRING_FORMAT, NULL, *src);
        }
    } else {
        SD_CONFIDENCE(strEnd);
    }
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

    len = (C_ADDRESS(strEnd) - C_ADDRESS(strStart));
    *dst = os_malloc(len + 1);
    if (*dst) {
        os_strncpy(*dst, strStart, len);
        (*dst)[len] = 0;
    }
    /* Skip to end of string */
    strEnd = &(strEnd[sizeof(SD_STRING_CLOSER)-1]);
    *src = strEnd;
}


c_char *
sd_peekTaggedCharData(
    c_char *src,
    c_char *tagName)
{
    c_char *result = NULL;
    c_char *strStart;
    c_char *strEnd;
    c_char *strFound;
    c_char *foundOpeningTag;
    c_char *foundClosingTag;
    int len;

    strStart = src;
    sd_strSkipChars(&strStart, SD_SKIP_SPACES);
    foundOpeningTag = sd_strGetOpeningTag(&strStart);
    if (strncmp(foundOpeningTag, tagName, strlen(tagName)) == 0) {
        if (strncmp(strStart, SD_STRING_OPENER, sizeof(SD_STRING_OPENER)-1) == 0) {
            strStart = &(strStart[sizeof(SD_STRING_OPENER)-1]);
            strEnd = strstr(strStart, SD_STRING_CLOSER);
            if (strEnd) {
                strFound = strStart;
                len = (C_ADDRESS(strEnd) - C_ADDRESS(strStart));
                strStart = &(strEnd[sizeof(SD_STRING_CLOSER)-1]);
                sd_strSkipChars(&strStart, SD_SKIP_SPACES);
                foundClosingTag = sd_strGetClosingTag(&strStart);
                if (strncmp(foundClosingTag, tagName, strlen(tagName)) == 0) {
                    result = os_malloc(len + 1);
                    os_strncpy(result, strFound, len);
                    result[len] = 0;
                }
                os_free(foundClosingTag);
            }
        }
    }
    os_free(foundOpeningTag);

    return result;
}

#else

c_char *
sd_scanToken (
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    c_char *result = NULL;

    OS_UNUSED_ARG(errorInfo);
    sd_strSkipChars(dataPtrPtr, SD_SKIP_SPACES);

    result = sd_strGetUptoChars(dataPtrPtr, "<");

    return result;
}


#define SD_LESS_THAN '<'
#define SD_AMPERSAND '&'

void
sd_scanCharData(
    c_char **dst,
    c_char **src,
    sd_errorInfo *errorInfo)
{
   static const c_char _CDATA[] = "<![CDATA[";
   static const c_ulong _CDATAlen = sizeof(_CDATA) - 1;
   static const c_char _CDATAEND[] = "]]>";
   static const c_ulong _CDATAENDlen = sizeof(_CDATAEND) - 1;
   static const c_char _AMP[] = "&amp;";
   static const c_ulong _AMPlen = sizeof(_AMP) - 1;
   static const c_char _GT[] = "&gt;";
   static const c_ulong _GTlen = sizeof(_GT) - 1;
   static const c_char _LT[] = "&lt;";
   static const c_ulong _LTlen = sizeof(_LT) - 1;

   const c_ulong step = 128;
   c_ulong dstLen = 0;
   c_ulong srcPos, dstPos;
   c_char *newDst;
   c_bool inCdata = FALSE;

   srcPos = 0;
   dstPos = 0;

   *dst = NULL;

   do{
       /* Allocate enough memory for result; re-alloc if needed */
       if (dstPos >= dstLen) {
           dstLen += step;
           newDst = (c_char *)os_realloc(*dst, dstLen);
           if (newDst == NULL){
               /* Out of resources */
               os_free(*dst);
               *dst = NULL;
               break;
           }
           *dst = newDst;
       }

       /* Scan the string for character-data */
       if(!inCdata){
           switch((*src)[srcPos]){
               case '<': /* Can mark closing tag or start of C_DATA */
                   switch((*src)[srcPos + 1]){
                       case '/': /* End of charData */
                           (*dst)[dstPos++] = '\0';
                           *src = &((*src)[srcPos]);
                           break;
                       case '!': /* Start of CDATA: <![CDATA[ */
                           assert(strncmp(_CDATA, &((*src)[srcPos]), _CDATAlen) == 0);
                           srcPos += _CDATAlen;
                           inCdata = TRUE;
                           break;
                       default:
                           if (SD_VALIDATION_NEEDED(errorInfo)) {
                               SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_OPENING_TAG, NULL, src[srcPos]);
                           }
                           os_free(*dst);
                           *dst = NULL;
                           break;
                   }
                   break;
               case '>': /* Malformed XML... */
                   if (SD_VALIDATION_NEEDED(errorInfo)) {
                       SD_VALIDATION_SET_ERROR(errorInfo, INVALID_XML_FORMAT, NULL, &((*src)[srcPos]));
                   }
                   os_free(*dst);
                   *dst = NULL;
                   break;
               case '&': /* &amp; &gt; &lt; */
                   switch((*src)[srcPos + 1]){
                       case 'a': /* &amp; */
                           assert(strncmp(_AMP, &((*src)[srcPos]), _AMPlen) == 0);
                           srcPos += _AMPlen;
                           (*dst)[dstPos++] = '&';
                           break;
                       case 'g': /* &gt; */
                           assert(strncmp(_GT, &((*src)[srcPos]), _GTlen) == 0);
                           srcPos += _GTlen;
                           (*dst)[dstPos++] = '>';
                           break;
                       case 'l': /* &lt; */
                           assert(strncmp(_LT, &((*src)[srcPos]), _LTlen) == 0);
                           srcPos += _LTlen;
                           (*dst)[dstPos++] = '<';
                           break;
                       default:
                           if (SD_VALIDATION_NEEDED(errorInfo)) {
                               SD_VALIDATION_SET_ERROR(errorInfo, INVALID_STRING_FORMAT, NULL, &((*src)[srcPos]));
                           }
                           os_free(*dst);
                           *dst = NULL;
                           break;
                   }
                   break;
               default:
                   /* Copy character */
                   (*dst)[dstPos++] = (*src)[srcPos++];
                   break;
           }
       } else {
           if((*src)[srcPos] == _CDATAEND[0]){ /* Potential end of CDATA section */
               if(strncmp(_CDATAEND, &((*src)[srcPos]), _CDATAENDlen) == 0){
                   inCdata = FALSE;
                   srcPos += _CDATAENDlen;
               }
           } else {
               /* Copy character */
               (*dst)[dstPos++] = (*src)[srcPos++];
           }
       }
   } while(*dst && ((dstPos == 0) || ((*dst)[dstPos - 1] != '\0')));

#if 0
   /* We possibly waste step-1 bytes (but in this context only pretty short). If
    * that's not OK, than #if 1 this block. */
   if(*dst){
       assert(dstPos <= dstLen);
       /* Shrinking, so cannot run out of resources... */
       *dst = (c_char *)os_realloc(*dst, dstPos);
       assert(*dst); /* Running out of resource when shrinking? */
   }
#endif
}


/** Peek for string between tags. Note that no escape sequences
 *  are allowed here, nor are mixes of strings and CDATA
 */
c_char *
sd_peekTaggedCharData(
    c_char *src,
    c_char *tagName)
{
    c_char *result = NULL;
    c_char *strStart;
    c_char *strEnd;
    c_char *strFound;
    c_char *foundOpeningTag;
    c_char *foundClosingTag;
    int len;

    strStart = src;
    sd_strSkipChars(&strStart, SD_SKIP_SPACES);
    foundOpeningTag = sd_strGetOpeningTag(&strStart);
    if (strncmp(foundOpeningTag, tagName, strlen(tagName)) == 0) {
        /* Check if this starts with CDATA */
        if (strncmp(strStart, SD_STRING_OPENER, sizeof(SD_STRING_OPENER)-1) == 0) {
            /* This is a CDATA section, process it like that */
            strStart = &(strStart[sizeof(SD_STRING_OPENER)-1]);
            strEnd = strstr(strStart, SD_STRING_CLOSER);
            if (strEnd) {
                strFound = strStart;
                len = (C_ADDRESS(strEnd) - C_ADDRESS(strStart));
                strStart = &(strEnd[sizeof(SD_STRING_CLOSER)-1]);
                sd_strSkipChars(&strStart, SD_SKIP_SPACES);
                foundClosingTag = sd_strGetClosingTag(&strStart);
                if (strncmp(foundClosingTag, tagName, strlen(tagName)) == 0) {
                    result = os_malloc(len + 1);
                    os_strncpy(result, strFound, len);
                    result[len] = 0;
                }
                os_free(foundClosingTag);
            }
        } else {
            strEnd = strchr(strStart, SD_LESS_THAN);
            if (strEnd != NULL) {
                strFound = strStart;
                len = (C_ADDRESS(strEnd) - C_ADDRESS(strStart));
                foundClosingTag = sd_strGetClosingTag(&strEnd);
                if (foundClosingTag != NULL) {
                    if (strncmp(foundClosingTag, tagName, strlen(tagName)) == 0) {
                        result = os_malloc(len + 1);
                        os_strncpy(result, strFound, len);
                        result[len] = 0;
                    }
                    os_free(foundClosingTag);
                }
            }
        }
    }
    os_free(foundOpeningTag);

    return result;
}

#endif

#undef SD_STRING_OPENER
#undef SD_STRING_CLOSER


static void
sd_scanString(
    c_collectionType type,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    c_char *scannedString;
    c_base base;

    sd_scanCharData(&scannedString, dataPtrPtr, errorInfo);
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
    base = c_getBase((c_object)type);
    *((c_string *)(*objectPtr)) = c_stringNew(base, scannedString);
    os_free(scannedString);
/* QAC EXPECT 2006; more than one return path is justified here */
}


static void
sd_XMLDeserCollection(
    c_collectionType collectionType,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    c_long colSize = 0, i;
    c_long *colSizePtr = &colSize;
    c_long** colSizePtrPtr;
    c_set set;
    c_object object, inserted;
    c_bool isValidRef;
    c_base base;
    c_scope scope;

    /* Different behaviour for reftypes and non-reftypes */
    if (((collectionType->kind == C_ARRAY) ||
         (collectionType->kind == C_SEQUENCE)) &&
         !(int)c_typeIsRef(c_type(collectionType))) {
        ; /* Do nothing */
    }  else {
        if (collectionType->kind == C_STRING) {
            isValidRef = sd_scanReference(dataPtrPtr, TRUE, errorInfo);
        } else {
            isValidRef = sd_scanReference(dataPtrPtr, FALSE, errorInfo);
        }
        SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

        if (isValidRef) {
            /* Only serialize the collection size in case of list/set/bag/etc */
            switch (collectionType->kind) {
            case C_STRING:
                sd_scanString(collectionType, objectPtr, dataPtrPtr, errorInfo);
                SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
            break;
            case C_ARRAY:
                SD_CONFIDENCE(c_typeIsRef(c_type(collectionType)));
                colSizePtrPtr = &colSizePtr;
                sd_scanTaggedPrim("size", P_LONG, (c_object*)colSizePtrPtr, dataPtrPtr, errorInfo);

                SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

                /* This function will always return a bounded array, or an (potentially unbounded) empty array */
                *((c_array *)(*objectPtr)) = c_arrayNew_w_header(collectionType, colSize);

            break;
            case C_SEQUENCE:
                SD_CONFIDENCE(c_typeIsRef(c_type(collectionType)));
                colSizePtrPtr = &colSizePtr;
                sd_scanTaggedPrim("size", P_LONG, (c_object *)colSizePtrPtr, dataPtrPtr, errorInfo);

                SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

                /* This function will always return a bounded sequence, or an (potentially unbounded) empty sequence */
                *((c_sequence *)(*objectPtr)) = c_sequenceNew(collectionType->subType, collectionType->maxSize, colSize);

            break;
            case C_SET:
                /* Scan the size */
                colSizePtrPtr = &colSizePtr;
                sd_scanTaggedPrim("size", P_LONG, (c_object *)colSizePtrPtr, dataPtrPtr, errorInfo);
                SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
                /* Create the set */
                set = c_setNew(collectionType->subType);
                *((c_set *)(*objectPtr)) = set;
                /* And initialize it with objects */
                for (i=0; i<colSize; i++) {
                    object = c_new(collectionType->subType);
                    SD_CONFIDENCE(object);
                    inserted = c_insert(set, object);
                    SD_CONFIDENCE(inserted == object);
                    /* Let go of my own reference */
                    c_free(object);
                }
            break;
            case C_SCOPE:
                /* Scan the size */
                colSizePtrPtr = &colSizePtr;
                sd_scanTaggedPrim("size", P_LONG, (c_object *)colSizePtrPtr, dataPtrPtr, errorInfo);
                SD_VALIDATION_RETURN_ON_ERROR(errorInfo);
                /* Currently, only empty scopes can be deserialized */
                SD_CONFIDENCE(colSize == 0);
                base = c_getBase(collectionType);
                scope = c_scopeNew(base);
                *((c_scope *)(*objectPtr)) = scope;
            break;
            case C_LIST:
            case C_BAG:
            case C_DICTIONARY:
            case C_QUERY:
                SD_CONFIDENCE(FALSE); /* Not yet implemented */
            break;
            default:
                SD_CONFIDENCE(FALSE); /* No other collection types supported */
            break;
            }
        } else {
            /* Invalid reference */
            *(c_object *)(*objectPtr) = NULL;
        }
    }

/* QAC EXPECT 2006, 5101; more than one return path is justified here and cyclomatic complexity no problem */
}


static void
sd_XMLDeserPrimitive(
    c_primitive primitive,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    sd_scanPrim(primitive->kind, objectPtr, dataPtrPtr, errorInfo);
}


static void
sd_XMLDeserEnumeration(
    c_enumeration enumeration,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    c_literal enumValue = NULL;
    c_char *enumStr;
    c_char *startPtr = *dataPtrPtr;

    enumStr = sd_strGetChars(dataPtrPtr, SD_SKIP_STRING);

    if (enumStr) {
        enumValue = c_enumValue(enumeration, enumStr);
        os_free(enumStr);

        if (enumValue) {
            SD_CONFIDENCE(enumValue->value.kind == V_LONG);
            *((c_long *)(*objectPtr)) = enumValue->value.is.Long;
        }
    }

    if (!enumValue) {
        /* Parsing of enumeration failed, fill errorInfo */
        SD_VALIDATION_SET_ERROR(errorInfo, INVALID_ENUMERATION, NULL, startPtr);
    }
}


static void
sd_XMLDeserInterface(
    c_interface interf,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    c_bool isValidRef;
    c_object *placeHolder;

    placeHolder = (c_object *)(*objectPtr);

    isValidRef = sd_scanReference(dataPtrPtr, FALSE, errorInfo);
    SD_VALIDATION_RETURN_ON_ERROR(errorInfo);

    if (isValidRef) {
        /* treat a non-instantiated interface the same
                 as a NULL pointer. This avoids memleaks. */
        if (!(*placeHolder)) {
            *placeHolder = c_new(c_type(interf));
        }
    } else {
        *placeHolder = NULL;
    }
/* QAC EXPECT 2006; more than one return path is justified here */
}

/* non-static for reuse by descendents */
void
sd_XMLDeserType(
    c_type type,
    c_object *objectPtr,
    c_char **dataPtrPtr,
    sd_errorInfo *errorInfo)
{
    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        sd_XMLDeserCollection(c_collectionType(type), objectPtr, dataPtrPtr, errorInfo);
    break;
    case M_PRIMITIVE:
        sd_XMLDeserPrimitive(c_primitive(type), objectPtr, dataPtrPtr, errorInfo);
    break;
    case M_ENUMERATION:
        sd_XMLDeserEnumeration(c_enumeration(type), objectPtr, dataPtrPtr, errorInfo);
    break;
    case M_UNION:
    case M_STRUCTURE:
        ; /* Do nothing, members, switchType and data will be
             callbacked automatically */
    break;
    case M_INTERFACE:
    case M_CLASS:
        /* Class and interface are to be treated equally */
        sd_XMLDeserInterface(c_interface(type), objectPtr, dataPtrPtr, errorInfo);
    break;
    default:
        SD_CONFIDENCE(FALSE); /* No other expected than these */
    break;
    }
}

/* -------------------- Deserialization driving functions ------------------ */

void
sd_XMLDeserCallbackPre(
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

    OS_UNUSED_ARG(userData);

    /* Opening tag */
    openingTag = sd_strGetOpeningTag(dataPtrPtr);
    tagName = sd_getTagName(name, type);
    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if ((openingTag == NULL) ||
            (strncmp(openingTag, tagName, strlen(openingTag)) != 0)) {
            SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_OPENING_TAG, tagName, startPtr);
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

    /* The data */
    /* Deserialization also updates dataPtrPtr */
    sd_XMLDeserType(type, objectPtr, dataPtrPtr, errorInfo);

    /* If an error has occurred, fill its name */
    SD_VALIDATION_ERROR_SET_NAME(errorInfo, tagName);
    os_free(tagName);
/* QAC EXPECT 2006; more than one return path is justified here */
}

void
sd_XMLDeserCallbackPost(
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

    OS_UNUSED_ARG(objectPtr);
    OS_UNUSED_ARG(userData);

    /* Closing tag */
    /* Check if it has the correct name */
    closingTag = sd_strGetClosingTag(dataPtrPtr);
    tagName = sd_getTagName(name, type);
    if (SD_VALIDATION_NEEDED(errorInfo)) {
        if ((closingTag == NULL) ||
            (strncmp(closingTag, tagName, strlen(closingTag)) != 0)) {
            SD_VALIDATION_SET_ERROR(errorInfo, UNEXPECTED_CLOSING_TAG, tagName,
                startPtr);
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

    /* No checking, this function is used internally only */

    if (!*objectPtr) {
        if (c_typeIsRef(type)) {
            *objectPtr = NULL;
        } else {
            *objectPtr = c_new(type);
            SD_CONFIDENCE(*objectPtr);
           /* memset(*objectPtr, 0, type->size); */
        }
    }

    sd_serializerSetValidationState(serializer, doValidation);

    context = sd_deepwalkMetaContextNew(sd_XMLDeserCallbackPre,
        sd_XMLDeserCallbackPost, NULL, dataPtrPtr, doValidation, NULL);

    sd_deepwalkMeta(type, name, objectPtr, context);

    if (doValidation) {
        errorOccurred = sd_deepwalkMetaContextGetErrorInfo(context, &errorNumber, &errname,
                                                           &message, &location);
        if (errorOccurred) {
            size = strlen(SD_MESSAGE_FORMAT) + strlen(errname) +
                   strlen(message) - 4U + 1U; /* 4 = 2 x strlen ("%s") */
            XMLmessage = os_malloc(size);
            snprintf(XMLmessage, size, SD_MESSAGE_FORMAT, errname, message);
            os_free(message);
            sd_serializerSetValidationInfo(serializer, errorNumber, XMLmessage, sd_stringDup(location));
            /* Free the partially deserialized data */
            if (*objectPtr) {
                c_free(*objectPtr);
            }
            *objectPtr = NULL;
            result = FALSE;
        }
    }

    sd_deepwalkMetaContextFree(context);

    return result;
}
#undef SD_MESSAGE_FORMAT


static c_object
sd_serializerXMLDeserialize(
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
    typeName = sd_getTypeAttributeFromOpenTag(openingTag);

    resultType = c_resolve(serializer->base, typeName);
    SD_CONFIDENCE(resultType);
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


static c_object
sd_serializerXMLDeserializeTyped(
    sd_serializer serializer,
    sd_serializedData serData,
    c_bool doValidation)
{
    c_char *xmlString;
    c_object result = NULL;
    c_bool deserOK;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    xmlString = (c_char *)serData->data;

    deserOK = sd_serializerXMLDeserializeInternal(serializer, serializer->type,
                  "object", &result, &xmlString, doValidation);

    /* Check if we reached the end */
    if (deserOK) {
        SD_CONFIDENCE((int)*xmlString == '\0');
    }

    return result;
}


static void
sd_serializerXMLDeserializeTypedInto(
    sd_serializer serializer,
    sd_serializedData serData,
    c_object object,
    c_bool doValidation)
{
    c_char *xmlString;
    c_bool deserOK;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    xmlString = (c_char *)serData->data;

    deserOK = sd_serializerXMLDeserializeInternal(serializer, serializer->type,
                  "object", &object, &xmlString, doValidation);

    /* Check if we reached the end */
    if (deserOK) {
        SD_CONFIDENCE((int)*xmlString == '\0');
    }
}

/* --------------------- Conversion to string -------------------- */

c_char *
sd_serializerXMLToString(
    sd_serializer serializer,
    sd_serializedData serData)
{
    OS_UNUSED_ARG(serializer);
    SD_CONFIDENCE(serializer != NULL);

    return sd_stringDup((const char *)serData->data);
}


sd_serializedData
sd_serializerXMLFromString(
    sd_serializer serializer,
    const c_char *str)
{
    sd_serializedData result;
    c_ulong size;

    OS_UNUSED_ARG(serializer);
    SD_CONFIDENCE(serializer != NULL);
    size = strlen(str) + 1U /* '\0' */;
    result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size);
    os_strncpy((char *)result->data, str, size);

    return result;
}

/* ---------------------------- constructor --------------------- */


/** \brief Constructor for the XML format serializer
 *
 *  The \b serializerXML class is a concrete descendant of the abstract
 *  \b serializer class. In order to use this class, create it with this
 *  function and call the methods as defined on \b serializer.
 *
 *  \param base The database to serialize from and deserialize to.
 */

sd_serializer
sd_serializerXMLNew(
    c_base base)
{
    sd_serializer result;
    struct sd_serializerVMT VMT;

    VMT.serialize = sd_serializerXMLSerialize;
    VMT.deserialize = sd_serializerXMLDeserialize;
    VMT.deserializeInto = NULL;
    VMT.toString = sd_serializerXMLToString;
    VMT.fromString = sd_serializerXMLFromString;

    result = sd_serializerNew(SD_FORMAT_ID, SD_FORMAT_VERSION, base, NULL, VMT);

    return result;
}


sd_serializer
sd_serializerXMLNewTyped(
    c_type type)
{
    sd_serializer result;
    c_base base;
    struct sd_serializerVMT VMT;

    VMT.serialize = sd_serializerXMLSerializeTyped;
    VMT.deserialize = sd_serializerXMLDeserializeTyped;
    VMT.deserializeInto = sd_serializerXMLDeserializeTypedInto;
    VMT.toString = sd_serializerXMLToString;
    VMT.fromString = sd_serializerXMLFromString;

    base = c_getBase(type);
    result = sd_serializerNew(SD_FORMAT_ID, SD_FORMAT_VERSION, base, type, VMT);

    return result;
}

#undef SD_FORMAT_ID
#undef SD_FORMAT_VERSION

