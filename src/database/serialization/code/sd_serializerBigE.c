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
/** \file services/serialization/code/sd_serializerBigE.c
 *  \brief Implementation of the \b serializerBigE class
 *
 *  The BigE networking format is a platform independent format
 *  where datafields are represented in the big endian format.
 *  No padding is inserted, so the resulting serializedData object
 *  can be interpreted as a stream of bytes. This stream can
 *  be used for networking, but also for persistent storage.
 *
 * ---------------------- Serialization format ---------------------------
 *
 *  Below, the exact definition for version 0.1 of the BigE format is given.
 *  Here, MSB means Most Significant Byte and LSB means Least Significant Byte
 *
\verbatim

  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |      'S'      |      '2'      |       0       |       1       |
  +---------------+---------------+---------------+---------------+
  | MSB                        dataSize                       LSB |
  +---------------+---------------+---------------+---------------+
  |      'S'      |      'c'      |      'o'      |      'p'      |
  +---------------+---------------+---------------+---------------+
  |      'e'      |      'n'      |      'a'      |      'm'      |
  +---------------+---------------+---------------+---------------+
  |      'e'      |      ':'      |      ':'      |      'T'      |
  +---------------+---------------+---------------+---------------+
  |      'y'      |      'p'      |      'e'      |      'n'      |
  +---------------+---------------+---------------+---------------+
  |      'a'      |      'm'      |      'e'      |       0       |
  +---------------+---------------+---------------+---------------+
  |                                                               |
  |      Serialized data       .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+\endverbatim
 *  The first four bytes represent the format ID and version number of
 *  the serializer used. The next four bytes are a big endian representation
 *  of the number of bytes of the serialization data. This is followed by a
 *  zero terminated string containing the scoped name of the type of the
 *  serialized object. This is a variable length part. Then the stream of
 *  databytes starts, which has a variable length as well.
 *
 *  <b>Primitive types</b> are equal to the big endian representation
 *  of the corresponding IDL types.
 *
\verbatim

  c_char, c_octet

  ....2...........8
  +-+-+-+-+-+-+-+-+
  |7|6|5|4|3|2|1|0|
  +---------------+\endverbatim

\verbatim

  c_bool

  TRUE
  ....2...........8
  +-+-+-+-+-+-+-+-+
  |0|0|0|0|0|0|0|1|
  +---------------+

  FALSE
  ....2...........8
  +-+-+-+-+-+-+-+-+
  |0|0|0|0|0|0|0|0|
  +---------------+\endverbatim

\verbatim

  c_short, c_ushort

  ....2...........8..............16
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |MSB            |            LSB|
  +---------------+---------------+\endverbatim

\verbatim

  c_long, c_ulong

  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |MSB            |               |               |            LSB|
  +---------------+---------------+---------------+---------------+\endverbatim

\verbatim

  c_longlong, c_ulonglong

  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |MSB            |               |               |               |
  +---------------+---------------+---------------+---------------+
  |               |               |               |            LSB|
  +---------------+---------------+---------------+---------------+\endverbatim

For floats and doubles, the standard IEEE big endian format is used. S indicates
the sign-bit, E indicates the mantissa and F indicates the float-value.
\verbatim

  float

  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |S|     E1      |E|     F1      |       F3      |       F3      |
  +---------------+---------------+---------------+---------------+\endverbatim

\verbatim

  double

  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |S|     E1      |   E2  |  F1   |       F2      |       F3      |
  +---------------+---------------+---------------+---------------+
  |      F4       |       F5      |       F6      |       F7      |
  +---------------+---------------+---------------+---------------+\endverbatim

 *  <b>Enumeration types</b> have the same representation as the
 *  \b c_long type.
\verbatim

  enumeration

  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |MSB                        Enum value                       LSB|
  +---------------+---------------+---------------+---------------+\endverbatim
 *
 * The representation of <b>Collection types</b> is different for
 * fixed length and variable length arrays. Fixed length arrays of
 * size <b>n</b> are formatted like <b>n</b> sequential elements.
 * Variable sized arrays are formatted like <b>n</b> sequential
 * elements with the integer value of <b>n</b> prepended. Since such
 * an array can be a NULL-pointer, an extra TRUE/FALSE byte is
 * prepended first. FALSE, which is represented by a zero, indicates
 * a NULL pointer. TRUE, which is represented by a one, indicates a
 * valid array.
\verbatim

  Set or variable length array

  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                                  | TRUE / FALSE  |
  +---------------+---------------+---------------+---------------+
  |MSB             Set size or array length n                  LSB|
  +---------------+---------------+---------------+---------------+
  |                                                               |
  |      Element 1             .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+
  |                                                               |
  |      Element 2             .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+
  |                                                               |
  |                            .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+
  |                                                               |
  |      Element n             .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+\endverbatim

\verbatim

   Fixed length array with length n

  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  |      Element 1             .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+
  |                                                               |
  |      Element 2             .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+
  |                                                               |
  |                            .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+
  |                                                               |
  |      Element n             .                                  |
  |                            .                                  |
  |                            .                                  |
  |                                                               |
  +---------------+---------------+---------------+---------------+\endverbatim
 * \b Strings are incorporated as a zero-terminated concatenation of characters.
 * Like the variable sized array, a string can be NULL. This is
 * indicated by the leading TRUE/FALSE byte.
\verbatim

  String of length n
  ....2...........8..............16..............24..............32
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | TRUE / FALSE  |      c_1      |      c_2      |      c_3      |
  +---------------+---------------+---------------+---------------+
  |             . . .             |      c_n      |       0       |
  +---------------+---------------+---------------+---------------+\endverbatim
 *
 *  <b>Union types</b> are represented as a concatenation of the switch
 *  element and the active case element.
 *
 *  <b>Structure types</b> are represented as a concatenation of their
 *  individual members.
 *
 *
 * ---------------------- General mechanism ---------------------------
 *
 * Both serialization and deserialization make use of the deepwalk function.
 * This functions interprets metadata information and with this information
 * it walks recursively over all elements in a class. During this walk,
 * a callback function is called. This callback function will dispatch to a
 * type-specific function which executes the actual (de)serialization. These
 * functions are the core of the algorithm. They return a c_ulong value
 * indicating how many bytes have been (de)serialized.
 */

/* interface */
#include "sd__serializer.h"
#include "sd_serializerBigE.h"

/* implementation */
#include "os_abstract.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "c_metabase.h"
#include "c_collection.h"
#include "sd__byteswapping.h"
#include "sd__confidence.h"
#include "sd_deepwalk.h"
#include "sd_misc.h"

#define SD_FORMAT_ID              (0x5332U)    /* "S2" */
#define SD_FORMAT_VERSION         (0x0001U)
#define SD_FORMAT_ID_STRING       "BigEndian"
#define SD_FORMAT_VERSION_STRING  "v0.1"

#define SD_FORMAT_USER_BYTE       ((unsigned char)0)
#define SD_FORMAT_INTERNAL_BYTE   ((unsigned char)1)
#define SD_INTERNAL_INDICATOR_SIZE (1)


C_CLASS(sd_serializerBigEInternal);

C_STRUCT(sd_serializerBigEInternal) {
    C_EXTENDS(sd_serializer);
    c_type internalType;
};

#define sd_serializerBigEInternal(o) ((sd_serializerBigEInternal)(o))

#ifndef NDEBUG
/* -------------------------- checking routines -------------------------- */

/** \brief Private function for typechecking the current serializer. This avoids
 * deserialization of incompatible formats
 */
static c_bool
sd_checkSerializerType(
    sd_serializer serializer)
{
    return ((c_bool)
            ((unsigned int)serializer->formatID == SD_FORMAT_ID) &&
            ((unsigned int)serializer->formatVersion == SD_FORMAT_VERSION));
}
#endif


/* ----------------------- serialization routines ----------------------- */

#define sd_isValidReference(_this) ((c_bool)(_this && (*(c_object *)_this)))

/** \brief Function for serializing any collection type. Currently supported
 * types are string, array, sequence and set.
 */
static c_ulong
sd_bigESerCollection(
    c_collectionType collectionType,
    c_object object,
    c_octet *dataPtr)
{
    c_long colSize;
    c_ulong len;
    c_octet *dataPtrHelper;

    /* Different behaviour for reftypes and non-reftypes */
    if (((collectionType->kind == C_ARRAY) ||
         (collectionType->kind == C_SEQUENCE)) &&
         !(int)c_typeIsRef(c_type(collectionType))) {

        len = 0U;

    } else {

        /* first serialize the validity of the object. We have to do this in
         * all situations, whether the reference is valid or not */
        *dataPtr = sd_isValidReference(object);
        len = 1;

        /* Now check if this was a valid reference. In that case, we have to
         * handle the data itself as well */
        if (sd_isValidReference(object)) {

            /* Skip to the data itself */
            dataPtrHelper = C_DISPLACE(dataPtr, C_ADDRESS(len));

            /* Only serialize the collection size in case of list/set/bag/etc */
            switch (collectionType->kind) {
            case C_STRING:
                {
                    c_char *src = *(c_char **)object;
                    c_char *dst = (c_char *)dataPtrHelper;
                    if ((src != NULL) && (dst != NULL)) {
                        while (*src != (c_char)0) {
                            *dst++ = *src++;
                            len++;
                        }
                        *dst = (c_char)0;
                        len++;
                    }
                }
            break;
            case C_ARRAY:
            case C_SEQUENCE:
                SD_CONFIDENCE(c_typeIsRef(c_type(collectionType)));
                /* Serialize the size */
                colSize = c_arraySize(*((c_array *)(object)));
                len += 4;
                SD_COPY2BIG_4(dataPtrHelper,&colSize);
            break;
            case C_SET:
                /* Serialize the size */
                colSize = c_setCount(*(c_collection *)object);
                len += 4;
                SD_COPY2BIG_4(dataPtrHelper,&colSize);
            break;
            case C_LIST:
            case C_BAG:
            case C_DICTIONARY:
            case C_QUERY:
            default:
                SD_CONFIDENCE(FALSE); /* No other collection types supported */
            break;
            }
        }
    }
    return len;
}

/** \brief Callback function for deepwalk when serializing */
static void
sd_bigESerCallback(
    c_type type,
    c_object *objectPtr,
    void *actionArg)
{
    c_octet **dataPtrPtr = (c_octet **)actionArg;
    c_ulong len;

    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        len = sd_bigESerCollection(c_collectionType(type),
                                   *objectPtr,
                                   *dataPtrPtr);
    break;
    case M_PRIMITIVE:
        SD_COPY2BIG_N(*dataPtrPtr,*objectPtr,type->size);
        len = type->size;
    break;
    case M_ENUMERATION:
        SD_COPY2BIG_4(*dataPtrPtr,*objectPtr);
        len = 4;
    break;
    case M_INTERFACE:
    case M_CLASS:
        **dataPtrPtr = sd_isValidReference(*objectPtr);
        len = 1;
    break;
    default:
        SD_CONFIDENCE(FALSE); /* No other expected than these */
        len = 0;
    break;
    }
    *dataPtrPtr = C_DISPLACE(*dataPtrPtr, C_ADDRESS(len));
}


/* ------------------- counting the number of bytes needed -------------------*/


/** \brief Callback function for deepwalk when determining the number of
 *         bytes needed
 */
static void
sd_bigECountCallback(
    c_type type,
    c_object *objectPtr,
    void *actionArg)
{
    c_ulong *size = (c_ulong *)actionArg;

    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        switch (c_collectionType(type)->kind) {
        case C_STRING:
            /* A boolean indicating if the reference is valid */
            *size += (c_ulong)sizeof(c_bool);
            if (sd_isValidReference(*objectPtr)) {
                 *size += strlen(*(c_string *)(*objectPtr)) + 1U /* '\0' */;
            }
        break;
        case C_ARRAY:
        case C_SEQUENCE:
            if (c_typeIsRef(type)) {
                /* A boolean indicating if the reference is valid */
                *size += (c_ulong)sizeof(c_bool);
                if (sd_isValidReference(*objectPtr)) {
                    /* contains size of stretchy array */
                    *size += (c_ulong)sizeof(c_ulong);
                }
            }
        break;
        case C_LIST:
        case C_SET:
        case C_BAG:
        case C_DICTIONARY:
        case C_QUERY:
            /* A boolean indicating if the reference is valid */
            *size += (c_ulong)sizeof(c_bool);
            if (sd_isValidReference(*objectPtr)) {
                /* contains size of the collection */
                *size += (c_ulong)sizeof(c_ulong);
            }
        break;
        default:
            /* Not yet implemented */
            SD_CONFIDENCE(FALSE);
        break;
        }
    break;
    case M_PRIMITIVE:
    case M_ENUMERATION:
        *size += (c_ulong)type->size;
    break;
    case M_CLASS:
    case M_INTERFACE:
        /* A boolean indicating if the reference is valid */
        *size += (c_ulong)sizeof(c_bool);
    break;
    default:
        SD_CONFIDENCE(FALSE);
    break;
    }
/* QAC EXPECT 5101; cyclomatic complexity is no problem here */
}


/* --------------- deserialization callbacks ------------------ */

/** \brief Function for deserializing any collection type.
 * Currently supported types are string, array, sequence and set.
 */
static c_ulong
sd_bigEDeserCollection(
    c_collectionType collectionType,
    c_object *objectPtr,
    c_octet *dataPtr)
{
    c_long colSize;
    c_ulong len = 0;
    c_set set;
    c_object object, inserted;
    c_long i;
    c_bool isValidRef;
    c_octet *dataPtrHelper;

    /* Different behaviour for reftypes and non-reftypes */
    if (((collectionType->kind == C_ARRAY) ||
         (collectionType->kind == C_SEQUENCE)) &&
         !(int)c_typeIsRef(c_type(collectionType))) {

        len = 0;

    } else {
        SD_COPY2BIG_1(&isValidRef, dataPtr);
        len = 1;

        if (isValidRef) {

            /* Skip to the data itself */
            dataPtrHelper = C_DISPLACE(dataPtr, C_ADDRESS(len));

            /* Only serialize the collection size in case of list/set/bag/etc */
            switch (collectionType->kind) {
            case C_STRING:
                *((c_string *)(*objectPtr)) = c_stringNew(c_getBase(collectionType),
                                                  (const c_char *)dataPtrHelper);
                len += strlen((const char *)dataPtrHelper) + 1U /* '\0' */;
            break;
            case C_ARRAY:
            case C_SEQUENCE:
                /* Deserialize into new array if necessary */
                SD_CONFIDENCE(c_typeIsRef(c_type(collectionType)));
                SD_COPY2BIG_4(&colSize,dataPtrHelper);
                len += 4;
#if 1
                *((c_array *)(*objectPtr)) = c_newBaseArrayObject(collectionType, colSize);
#else
                *((c_array *)(*objectPtr)) = c_arrayNew(collectionType->subType, colSize);
#endif
            break;
            /* Other collection types not yet implemented */
            case C_SET:
                /* Get the size */
                SD_COPY2BIG_4(&colSize,dataPtrHelper);
                len += 4;
                /* Create the set */
                set = c_setNew(collectionType->subType);
                *((c_set *)(*objectPtr)) = set;
                /* And initialize it with objects */
                for (i=0; i<colSize; i++) {
                    object = c_new(collectionType->subType);
                    SD_CONFIDENCE(object);
                    inserted = c_insert(set, object);
                    SD_CONFIDENCE(inserted == object);
                    /* Let go of this instance */
                    c_free(object);
                }
            break;
            case C_LIST:
            case C_BAG:
            case C_DICTIONARY:
            case C_QUERY:
                SD_CONFIDENCE(FALSE);
            break;
            default:
                SD_CONFIDENCE(FALSE);
                /* No other collection types supported */
            break;
            }
        }
    }

    return len;
}

/** \brief Deserialization function for interfaces and classes */
static c_ulong
sd_bigEDeserInterface(
    c_interface interf,
    c_object *objectPtr,
    c_octet *dataPtr)
{
    c_bool isValidRef;
    c_ulong result;
    c_object *placeHolder;

    SD_CONFIDENCE((c_baseObject(interf)->kind == M_INTERFACE) ||
                  (c_baseObject(interf)->kind == M_CLASS));

    placeHolder = (c_object *)(*objectPtr);

    /* Deserialize the boolean in order to find out if this is
       a valid reference */
    SD_COPY2BIG_1(&isValidRef, dataPtr);
    result = 1;
    if (isValidRef) {
        if (!(*placeHolder)) {
            /* A new instance has to be created, since this object is still a
             * NULL pointer */
            *placeHolder = c_new(c_type(interf));
        }
    } else {
        *placeHolder = 0;
    }

    return result;
}

/** \brief Callback function for deepwalk when deserializing */
static void
sd_bigEDeserCallback(
    c_type type,
    c_object *objectPtr,
    void *actionArg)
{
    c_octet **dataPtrPtr =  (c_octet **)actionArg;
    c_ulong len;

    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        len = sd_bigEDeserCollection(c_collectionType(type),
                                     objectPtr,
                                     *dataPtrPtr);
    break;
    case M_PRIMITIVE:
        SD_COPY2BIG_N(*objectPtr,*dataPtrPtr,type->size);
        len = type->size;
    break;
    case M_ENUMERATION:
        SD_COPY2BIG_4(*objectPtr,*dataPtrPtr);
        len = 4;
    break;
    case M_INTERFACE:
    case M_CLASS:
        len = sd_bigEDeserInterface(c_interface(type),
                                    objectPtr,
                                    *dataPtrPtr);
    break;
    default:
        SD_CONFIDENCE(FALSE); /* No other expected than these */
        len = 0;
    break;
    }

    *dataPtrPtr = C_DISPLACE(*dataPtrPtr, C_ADDRESS(len));
}


/* --------------------------- driving functions ---------------------------- */


/* ------------------------------ serialization ----------------------------- */

/** \brief The actual BigE implementation of the virtual abstract serialize
 * function as introduced by sd_serializer */
static sd_serializedData
sd_serializerBigESerialize(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result;
    c_ulong size;
    c_long start;
    c_char *typeName;
    c_type type;
    c_char *startPtr;

    OS_UNUSED_ARG(serializer);

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    type = c_getType(object);
    typeName = sd_getScopedTypeName(type, "::");

    /* Determine the size */
    size = strlen(typeName) + 1U; /* sep */
    sd_deepwalk(type, &object, sd_bigECountCallback, &size);

    /* Instantiate the serialized data */
    result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size);

    /* Fill the block */
    /* First print the name */
    /* Note: the return value of snprintf does not count the terminator */
    start = snprintf((c_char *)result->data, size, "%s", typeName) + 1;

    /* Then do the walk */
    startPtr =  C_DISPLACE(result->data,  C_ADDRESS(start));
    sd_deepwalk(type, &object, sd_bigESerCallback, &startPtr);

    /* Double-check if the size was calculated correctly */
    SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(result->data)) ==
                   sd_serializedDataGetDataSize(result));

    os_free(typeName);
    return result;
}

/** \brief The actual BigE typed implementation of the virtual abstract
 * serialize function as introduced by sd_serializer
 */
static sd_serializedData
sd_serializerBigESerializeTyped(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result;
    c_ulong size;
    c_char *startPtr;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));
    SD_CONFIDENCE((serializer->type == c_getType(object)) ||
                  (c_baseObject(c_typeActualType(serializer->type))->kind ==
                   M_COLLECTION));

    /* Determine the size */
    size = 0;
    sd_deepwalk(serializer->type, &object, sd_bigECountCallback, &size);

    /* Instantiate the serialized data */
    result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size);

    /* Fill the block */
    startPtr =  (c_char *)(result->data);
    sd_deepwalk(serializer->type, &object, sd_bigESerCallback, &startPtr);

    /* Double-check if the size was calculated correctly */
    SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(result->data)) ==
                   sd_serializedDataGetDataSize(result));

    return result;
}


/** \brief The actual BigE typed internal implementation of the virtual abstract
 * serialize function as introduced by sd_serializer
 */

static sd_serializedData
sd_serializerBigESerializeTypedInternal(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result;
    c_ulong size;
    c_char *startPtr;
    c_bool isInternalType;
    c_type typeToUse;
    c_char internalFormatByte;

    SD_CONFIDENCE(sd_checkSerializerType(serializer));
    SD_CONFIDENCE((object == NULL) ||
                  (serializer->type == c_getType(object)) ||
                  (sd_serializerBigEInternal(serializer)->internalType == c_getType(object)) ||
                  (c_baseObject(c_typeActualType(serializer->type))->kind ==
                   M_COLLECTION));

    /* First check if this is the internal type */
    isInternalType = (object != NULL) &&
        ((sd_serializerBigEInternal(serializer)->internalType ==
                         c_getType(object)));

    if (isInternalType) {
        /* Only continue if this is correctly constructed */
        SD_CONFIDENCE(sd_serializerBigEInternal(serializer)->internalType != NULL);
        typeToUse = sd_serializerBigEInternal(serializer)->internalType;
        internalFormatByte = SD_FORMAT_INTERNAL_BYTE;
    } else {
        typeToUse = serializer->type;
        internalFormatByte = SD_FORMAT_USER_BYTE;
    }

    /* Determine the size */
    size = SD_INTERNAL_INDICATOR_SIZE; /* Starting byte is counted as well */
    sd_deepwalk(typeToUse, &object, sd_bigECountCallback, &size);

    /* Instantiate the serialized data */
    result = sd_serializedDataNew(SD_FORMAT_ID, SD_FORMAT_VERSION, size);

    /* Set the pointer to the correct place */
    startPtr =  (c_char *)(result->data);
    *startPtr = internalFormatByte;
    /* skip to the actual startingpoint of the serialized data */
    startPtr = &(startPtr[SD_INTERNAL_INDICATOR_SIZE]);

    /* And do the serialization */
    sd_deepwalk(typeToUse, &object, sd_bigESerCallback, &startPtr);

    /* Double-check if the size was calculated correctly */
    SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(result->data)) ==
                   sd_serializedDataGetDataSize(result));

    return result;
}


/* ----------------------------- deserialization ---------------------------- */

/** \brief Function implementing common functionalities for all bigE
 *  deserialization routines */
static void
sd_serializerBigEDeserializeInternal(
    c_type type,
    c_object *objectPtr,
    c_octet **dataPtrPtr)
{
    if (!(int)c_typeIsRef(type)) {
        *objectPtr = c_new(type);
        SD_CONFIDENCE(*objectPtr);
        if (*objectPtr) {
            sd_deepwalk(type, objectPtr, sd_bigEDeserCallback, dataPtrPtr);
        }
    } else {
        /* Class and interface reference created by deser algorithm */
        *objectPtr = NULL;
        sd_deepwalk(type, objectPtr, sd_bigEDeserCallback, dataPtrPtr);
    }
}

/** \brief The implementation of the virtual abstract deserialize function
 * as introduced by sd_serializer, for the BigE class */
static c_object
sd_serializerBigEDeserialize(
    sd_serializer serializer,
    sd_serializedData serData,
    c_bool doValidation)
{
    c_object result = NULL;
    c_type resultType;
    c_char *name;
    c_octet *startPtr;

    OS_UNUSED_ARG(doValidation);

    SD_CONFIDENCE(sd_checkSerializerType(serializer));
    /* Validation not supported for this class */
    SD_CONFIDENCE((int)doValidation == FALSE);

    /* The deserialized data starts with the scoped name of the type */
    /* Create the object using this name */
    name = (c_char *)serData->data;
    resultType = c_resolve(serializer->base, name);

    /* Only continue if the resolution was succesful */
    SD_CONFIDENCE(resultType);

    if (resultType) {

        /* Find the starting point of the actual data */
        startPtr = C_DISPLACE(serData->data, C_ADDRESS(strlen(name) + 1U));

        sd_serializerBigEDeserializeInternal(resultType,
                                             &result, &startPtr);
        /* Double-check if the size was calculated correctly */
        SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(serData->data)) ==
                      sd_serializedDataGetDataSize(serData));

        c_free(resultType);
    }


    return result;
}

/** \brief The implementation of the virtual abstract deserialize function
 * as introduced by sd_serializer, for the BigETyped class */
static c_object
sd_serializerBigEDeserializeTyped(
    sd_serializer serializer,
    sd_serializedData serData,
    c_bool doValidation)
{
    c_object result = NULL;
    c_octet *startPtr;

    OS_UNUSED_ARG(doValidation);

    SD_CONFIDENCE(sd_checkSerializerType(serializer));
    /* Validation not supported for this class */
    SD_CONFIDENCE((int)doValidation == FALSE);

    /* Only continue if this is constructed using BigENewTyped */
    SD_CONFIDENCE(serializer->type != NULL);

    startPtr = serData->data;
    sd_serializerBigEDeserializeInternal(serializer->type,
                                         &result, &startPtr);

    /* Double-check if the size was calculated correctly */
    SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(serData->data)) ==
                   sd_serializedDataGetDataSize(serData));

    return result;
}


/** \brief The implementation of the virtual abstract deserialize function
 * as introduced by sd_serializer, for the BigETypedInternal class */
static c_object
sd_serializerBigEDeserializeTypedInternal(
    sd_serializer serializer,
    sd_serializedData serData,
    c_bool doValidation)
{
    c_object result = NULL;
    c_octet *startPtr;
    c_bool isInternalType;

    OS_UNUSED_ARG(doValidation);

    SD_CONFIDENCE(sd_checkSerializerType(serializer));
    /* Validation not supported for this class */
    SD_CONFIDENCE((int)doValidation == FALSE);


    startPtr = serData->data;
    /* First check if this is user data or internal data */
    isInternalType = (*startPtr == SD_FORMAT_INTERNAL_BYTE);
    startPtr = &(startPtr[SD_INTERNAL_INDICATOR_SIZE]);

    if (isInternalType) {
        /* Only continue if this is correctly constructed */
        SD_CONFIDENCE(sd_serializerBigEInternal(serializer)->internalType != NULL);
        sd_serializerBigEDeserializeInternal(
            sd_serializerBigEInternal(serializer)->internalType, &result, &startPtr);
    } else {
        /* Only continue if this is correctly constructed */
        SD_CONFIDENCE(serializer->type != NULL);
        sd_serializerBigEDeserializeInternal(serializer->type,
                                             &result, &startPtr);
    }

    /* Double-check if the size was calculated correctly */
    SD_CONFIDENCE((C_ADDRESS(startPtr) - C_ADDRESS(serData->data)) ==
                   sd_serializedDataGetDataSize(serData));

    return result;
}



/* ----------------------- conversion to/from string ------------------------ */

/** \brief The actual implementation of the virtual abstract toString function
 * as introduced by sd_serializer */
static c_char *
sd_serializerBigEToString(
    sd_serializer serializer,
    sd_serializedData serData)
{
#define SD_ASCII_SPACE          (32U)  /* SP: Lowest printable character      */
#define SD_ASCII_TILDE         (126U)  /* ~: Highest printable character      */
#define SD_ASCII_PERCENT        (37U)  /* %: Avoid printf confusion           */
#define SD_ASCII_DOT            (46U)  /* .: Replacer for non-printable chars */
#define SD_ASCII_ZERO           (48U)  /* 0: Base for digits                  */
#define SD_ASCII_A              (65U)  /* A: Base for hex-digits > 10         */
#define SD_SEP_REPLACER         (c_octet)'\n'
#define SD_ASCII(i)             (c_octet)(                 \
                                  ((unsigned int)(i)<10U)?  \
                                  ((unsigned int)(i)+SD_ASCII_ZERO):  \
                                  ((unsigned int)(i)+ SD_ASCII_A - 10U))
#define SD_ASCII_MSB(i)         (c_octet)(SD_ASCII(((unsigned int)(i)>>4)))
#define SD_ASCII_LSB(i)         (c_octet)(SD_ASCII(((unsigned int)(i)&15U)))
#define SD_ASCII_PRINTABLE(i)   (c_octet)(                      \
                                  ( ((unsigned int)(i)>= SD_ASCII_SPACE) && \
                                    ((unsigned int)(i)<= SD_ASCII_TILDE) && \
                                    ((unsigned int)(i)!= SD_ASCII_PERCENT) )? \
                                  (unsigned int)(i):            \
                                  SD_ASCII_DOT)
#define SD_LINE_WIDTH           (16U)
#define SD_BUFSIZE              (32U)
#define SD_BYTESEP              (c_octet)' '
#define SD_LINESEP              (c_octet)'\n'

    c_ulong dataSize, totSize, nLines, start;
    c_char *result;
    c_octet *currentDest, *currentSrc;
    c_ulong line, i, nThisLine;
    c_char sizeBuf[SD_BUFSIZE];
    int snpRes;
    OS_UNUSED_ARG(serializer);

    SD_CONFIDENCE(sd_checkSerializerType(serializer));

    dataSize = sd_serializedDataGetDataSize(serData);

    nLines = ((dataSize-1U) / SD_LINE_WIDTH) + 1U;
    totSize = ((4U*SD_LINE_WIDTH) + 1U) * nLines;
    totSize += strlen(SD_FORMAT_ID_STRING) + 1U;
    totSize += strlen(SD_FORMAT_VERSION_STRING) + 1U;
    snpRes = snprintf(sizeBuf, SD_BUFSIZE, "%d%c", dataSize, '\0');
    if (snpRes >= 0) {
        totSize += (c_ulong)snpRes;
    } else {
       SD_CONFIDENCE(snpRes >= 0);
    }
    totSize++; /* '\0' */
    result = (c_char *)os_malloc(totSize);

    /* Prepend version info */
    snpRes = snprintf(result, totSize, "%s%c%s%c%s%c",
                    SD_FORMAT_ID_STRING, SD_SEP_REPLACER,
                    SD_FORMAT_VERSION_STRING, SD_SEP_REPLACER,
                    sizeBuf, SD_SEP_REPLACER);
    if (snpRes >= 0) {
        start = (c_ulong)snpRes;
    } else {
        start = 0;
        SD_CONFIDENCE(snpRes >= 0);
    }

    /* The actual serialized data */
    currentSrc = serData->data;
    currentDest = (c_octet *)C_DISPLACE(result, C_ADDRESS(start));
    for (line=0; line < nLines; line++) {
        nThisLine = (((line+1U) < nLines) ?
                     SD_LINE_WIDTH :
                     (((dataSize - 1U) % SD_LINE_WIDTH) + 1U));
        for (i=0; i < SD_LINE_WIDTH; i++) {
            if (i < nThisLine) {
                currentDest[(3U*i)] = SD_ASCII_MSB(currentSrc[i]);
                currentDest[(3U*i) + 1U] = SD_ASCII_LSB(currentSrc[i]);
            } else {
                currentDest[(3U*i)] = SD_BYTESEP;
                currentDest[(3U*i) + 1U] = SD_BYTESEP;
            }
            currentDest[(3U*i) + 2U] = SD_BYTESEP;
        }
        currentDest = &(currentDest[3U*SD_LINE_WIDTH]);
        for (i=0; i < nThisLine; i++) {
            currentDest[i] = SD_ASCII_PRINTABLE(currentSrc[i]);
        }
        currentDest = &(currentDest[nThisLine]);
        currentSrc = &(currentSrc[nThisLine]);

        *currentDest = SD_LINESEP;
        currentDest = &(currentDest[1]);
    }
    currentDest = &(currentDest[-1]);
    *currentDest = 0;

    return result;

#undef SD_ASCII
#undef SD_ASCII_PRINTABLE
#undef SD_ASCII_LSB
#undef SD_ASCII_MSB
#undef SD_SEP_REPLACER
#undef SD_LINE_WIDTH
#undef SD_BUFSIZE
#undef SD_BYTESEP
#undef SD_LINESEP
}


/** \brief Constructor for the big endian format serializer for any type
 *
 *  The \b serializerBigE class is a concrete descendant of the abstract
 *  \b serializer class. In order to use this class, create it with this
 *  function and call the methods as defined on \b serializer. This class
 *  is capable of (de)serializing objects of any type.
 *
 *  This class is supposed to do quick (de)serialization and does not do any
 *  checking on format errors.
 *
 *  \param base The database to serialize to and deserialize from.
 */

sd_serializer
sd_serializerBigENew(
    c_base base)
{
    sd_serializer result;
    struct sd_serializerVMT VMT;

    VMT.serialize = sd_serializerBigESerialize;
    VMT.deserialize = sd_serializerBigEDeserialize;
    VMT.deserializeInto = NULL;
    VMT.toString = sd_serializerBigEToString;
    VMT.fromString = NULL;

    result = sd_serializerNew(SD_FORMAT_ID, SD_FORMAT_VERSION, base, NULL, VMT);

    return result;
}

/** \brief Constructor for the big endian format serializer for a fixed type.
 *
 *  The \b serializerBigETyped class is a concrete descendant of the abstract
 *  \b serializer class. In order to use this class, create it with this
 *  function and call the methods as defined on \b serializer. This class is
 *  capable of (de)serializing objects of one specific type only.
 *
 *  This class is supposed to do quick (de)serialization and does not do any
 *  checking on format errors.
 *
 *  \param type The type that will be (de)serialized by this object.
 */
sd_serializer
sd_serializerBigENewTyped(
    c_type type)
{
    sd_serializer result;
    c_base base;
    struct sd_serializerVMT VMT;

    base = c_getBase((c_object)type);

    VMT.serialize = sd_serializerBigESerializeTyped;
    VMT.deserialize = sd_serializerBigEDeserializeTyped;
    VMT.deserializeInto = NULL;
    VMT.toString = sd_serializerBigEToString;
    VMT.fromString = NULL;

    result = sd_serializerNew(SD_FORMAT_ID, SD_FORMAT_VERSION, base, type, VMT);

    return result;
}

/** \brief Constructor for the big endian format serializer for a fixed type
 *         which can also handle an internal type
 *
 *  The \b serializerBigETyped class is a concrete descendant of the abstract
 *  \b serializer class. In order to use this class, create it with this
 *  function and call the methods as defined on \b serializer. This class is
 *  capable of (de)serializing objects of two specific types: the type given
 *  to the constructor and the v_networkMessageType
 *
 *  This class is supposed to do quick (de)serialization and does not do any
 *  checking on format errors.
 *
 *  Note: this class could be changed so that it will be able to (de)serialize
 *  several internal types.
 *
 *  \param type The type that will be (de)serialized by this object.
 */

/* Note: hard-coding the name of the type here decreases reusability. Another
 * choice would have been giving the internal type to the constructor. For now,
 * this dedicated solution is chosen for simplicity */
#define SD_INTERNAL_TYPE_NAME "kernelModule::v_networkMessage"

sd_serializer
sd_serializerBigENewTypedInternal(
    c_type type)
{
    sd_serializerBigEInternal result;
    c_base base;
    struct sd_serializerVMT VMT;

    base = c_getBase((c_object)type);

    VMT.serialize = sd_serializerBigESerializeTypedInternal;
    VMT.deserialize = sd_serializerBigEDeserializeTypedInternal;
    VMT.deserializeInto = NULL;
    VMT.toString = sd_serializerBigEToString;
    VMT.fromString = NULL;

    result = (sd_serializerBigEInternal)os_malloc((os_uint32)sizeof(*result));

    if (result) {
        /* Initialize self */
        result->internalType = c_resolve(base, SD_INTERNAL_TYPE_NAME);
        /* Initialize parent */
        sd_serializerInitialize((sd_serializer)result,
            SD_FORMAT_ID, SD_FORMAT_VERSION, base, type, VMT);
    }

    return (sd_serializer)result;
}

#undef SD_INTERNAL_TYPE_NAME

#undef SD_FORMAT_ID
#undef SD_FORMAT_VERSION
#undef SD_FORMAT_ID_STRING
#undef SD_FORMAT_VERSION_STRING
#undef SD_FORMAT_USER_BYTE
#undef SD_FORMAT_INTERNAL_BYTE
