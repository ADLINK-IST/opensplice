/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/** \file services/serialization/code/sd_randomizer.c
 *  \brief Implementation of the \b randomizer class.
 *
 *  The randomizer class is capable of creating pseudorandom instances of
 *  database objects. It uses the deepwalk functionality to walk over the
 *  data elements and randomly fills these elements.
 *
 *  Currently, \b randomizer is used as a helper class for testing purposes
 *  only.
 */

/* interface */
#include "sd_randomizer.h"

/* implementation */
#include "os_heap.h"
#include "c_base.h"
#include "c_collection.h"
#include "sd__confidence.h"
#include "sd_deepwalk.h"


/* ---------------------------------- randomizer --------------------------- */

C_STRUCT(sd_randomizer) {
    c_base base;
    c_ulong randValue;
};

/* ----------------------------------- private ----------------------------- */


/** \brief Get a random unsigned long value using the value previously
 *         returned.
 *
 *  Since this algorithm is very simple, a sequence of random values is
 *  completely determined by the starting value. This starting value can
 *  be set using randomizerInit.
 */

static c_ulong
sd_randomizerNextValue(
    sd_randomizer randomizer)
{
    randomizer->randValue = (randomizer->randValue * 1103515245U) + 12345U;
    return randomizer->randValue;
}


/** \brief Get a random unsigned long long value by concatenating two random
 *         unsigned long values.
 */

static c_ulonglong
sd_randomizerNextULongLongValue(
    sd_randomizer randomizer)
{
    c_ulonglong result;

    result = sd_randomizerNextValue(randomizer);
    result <<= 16;
    result <<= 16;
    result += sd_randomizerNextValue(randomizer);

    return result;
}

#define SD_FIRST_DIGIT     '0'
#define SD_FIRST_UPPERCASE 'A'
#define SD_FIRST_LOWERCASE 'a'
#define SD_NUM_DIGIT       (10U)
#define SD_NUM_LOWERCASE   (26U)
#define SD_NUM_UPPERCASE   (26U)

/** \brief Get a random character value by transforming a random unsigned long
 *         value to one of the characters '0'..'9','A'..'Z','a'..'z'.
 */
static c_char
sd_randomizerNextCharValue(
    sd_randomizer randomizer)
{
    c_ulong result;

    /* Letters and digits only */
    result = (sd_randomizerNextValue(randomizer) %
               (SD_NUM_DIGIT + SD_NUM_UPPERCASE + SD_NUM_LOWERCASE));
    if (result < SD_NUM_DIGIT) {
        /* Digit */
        result += SD_FIRST_DIGIT;
    } else {
        result -= SD_NUM_DIGIT;
        if (result < SD_NUM_UPPERCASE) {
            /* Uppercase */
            result += SD_FIRST_UPPERCASE;
        } else {
            /* Lowercase */
            result -= SD_NUM_UPPERCASE;
            SD_CONFIDENCE(result <= SD_NUM_LOWERCASE);
            result += SD_FIRST_LOWERCASE;
        }
    }

    return (c_char)result;
}

#undef SD_FIRST_DIGIT
#undef SD_FIRST_UPPERCASE
#undef SD_FIRST_LOWERCASE
#undef SD_NUM_DIGIT
#undef SD_NUM_LOWERCASE
#undef SD_NUM_UPPERCASE


/** \brief Get a random boolean value by taking the third to last bit of
 *         a random unsigned long value.
 */
static c_bool
sd_randomizerNextBooleanValue(
    sd_randomizer randomizer)
{
    c_ulong result;

    result = sd_randomizerNextValue(randomizer);
    result >>= 3;
    result &= 1U;

    return (c_bool)result;
}

#define SD_RAND_CHAR(r,dst)      *(c_char *)(dst) = (c_char)sd_randomizerNextCharValue(r)
#define SD_RAND_OCTET(r,dst)     *(c_octet *)(dst) = (c_octet)sd_randomizerNextValue(r)
#define SD_RAND_SHORT(r,dst)     *(c_ushort *)(dst) = (c_ushort)sd_randomizerNextValue(r)
#define SD_RAND_USHORT(r,dst)    *(c_ushort *)(dst) = (c_ushort)sd_randomizerNextValue(r)
#define SD_RAND_LONG(r,dst)      *(c_ulong *)(dst) = sd_randomizerNextValue(r)
#define SD_RAND_ULONG(r,dst)     *(c_ulong *)(dst) = sd_randomizerNextValue(r)
#define SD_RAND_FLOAT(r,dst)     *(c_ulong *)(dst) = sd_randomizerNextValue(r)
#define SD_RAND_DOUBLE(r,dst)    *(c_ulonglong *)(dst) = sd_randomizerNextULongLongValue(r)
#define SD_RAND_ULONGLONG(r,dst) *(c_ulonglong *)(dst) = sd_randomizerNextULongLongValue(r)
#define SD_RAND_LONGLONG(r,dst)  *(c_ulonglong *)(dst) = sd_randomizerNextULongLongValue(r)
#define SD_RAND_BOOLEAN(r,dst)   *(c_bool *)(dst) = sd_randomizerNextBooleanValue(r)
#define SD_RAND(t,r,dst) SD_RAND_##t(r,dst)


/** \brief Maximum length of randomly created collections */
#define SD_COLLECTION_SIZE_MAX 10U


/** \brief Get a random collection (string, array or set).
 *
 *  In case of a variable length type, the length of the collection is
 *  determined using a random unsigned long, modulo SD_COLLECTION_SIZE_MAX.
 *
 *  In case of a string type collection, a new string with this length is
 *  created and filled with random characters.
 *
 *  In case of a variable length array, a new c_array with this length and the
 *  proper subtype is created. Deepwalk will automatically walk over the
 *  array elements.
 *
 *  In case of a set, a new c_set with this length is created and filled with
 *  non-initialized objects. Deepwalk will automatically walk over the set
 *  elements.
 */

static c_bool sd_randCollection(c_collectionType collectionType, c_object *objectPtr, sd_randomizer randomizer) __nonnull_all__ __attribute_warn_unused_result__;

static c_bool
sd_randCollection(
    c_collectionType collectionType,
    c_object *objectPtr,
    sd_randomizer randomizer)
{
    c_ulong colSize;
    c_ulong i;
    c_char *str;
    c_set set;
    c_object object, inserted;
    c_octet randVal;

    /* Different behaviour for reftypes and non-reftypes */
    if (((collectionType->kind == OSPL_C_ARRAY) ||
         (collectionType->kind == OSPL_C_SEQUENCE)) &&
        !c_typeIsRef(c_type(collectionType))) {

        return TRUE; /* Do nothing */

    } else {
        SD_RAND_OCTET(randomizer, &randVal);

        /* Chance of having a valid reference is 240 out of 256 */
        if ((int)randVal > 15) {

            /* Only serialize the collection size in case of list/set/bag/etc */
            switch (collectionType->kind) {
                case OSPL_C_STRING:
                    colSize =  ((c_ulong)sd_randomizerNextValue(randomizer) % (SD_COLLECTION_SIZE_MAX-1U)) + 1U;
                    str = os_malloc(colSize + 1U);
                    for (i=0; i<colSize; i++) {
                        SD_RAND_CHAR(randomizer,&(str[i]));
                    }
                    str[colSize] = '\0';
                    if ((*((c_string *)(*objectPtr)) = c_stringNew_s (c_getBase(collectionType), str)) == NULL) {
                        os_free(str);
                        return FALSE;
                    }
                    os_free(str);
                    break;
                case OSPL_C_ARRAY:
                case OSPL_C_SEQUENCE:
                    /* Deserialize into new array if necessary */
                    /* Only variable length arrays need to be created. */
                    SD_CONFIDENCE(c_typeIsRef(c_type(collectionType)));
                    colSize = (sd_randomizerNextValue(randomizer) % (SD_COLLECTION_SIZE_MAX-1U)) + 1U;
                    if ((*((c_array *)(*objectPtr)) = c_arrayNew_s(collectionType->subType, colSize)) == NULL && colSize) {
                        return FALSE;
                    }
                    break;
                case OSPL_C_SET:
                    /* Deserialize into a set */
                    colSize = (sd_randomizerNextValue(randomizer) % (SD_COLLECTION_SIZE_MAX-1U)) + 1U;
                    /* Create the set */
                    set = c_setNew(collectionType->subType);
                    if (set == NULL) {
                        return FALSE;
                    }
                    /* And initialize it with objects */
                    for (i=0; i<colSize; i++) {
                        if ((object = c_new_s(collectionType->subType)) == NULL) {
                            c_free(set);
                            return FALSE;
                        }
                        SD_CONFIDENCE(object);
                        inserted = ospl_c_insert(set, object);
                        SD_CONFIDENCE(inserted == object);
                        OS_UNUSED_ARG(inserted);
                        /* Local reference is released */
                        c_free(object);
                    }
                    *((c_set *)(*objectPtr)) = set;
                    break;
                case OSPL_C_LIST:
                case OSPL_C_BAG:
                case OSPL_C_DICTIONARY:
                case OSPL_C_QUERY:
                    SD_CONFIDENCE(FALSE);
                    return FALSE;
                default:
                    SD_CONFIDENCE(FALSE); /* No other collection types supported */
                    return FALSE;
            }
        } else { /* not a valid reference */
            *((c_object *)(*objectPtr)) = NULL;
        }
        return TRUE;
    }
}

#undef SD_COLLECTION_SIZE_MAX


/** \brief Fill the address pointed to with random bytes according to the
 *         primitive type requested.
 */

static c_bool sd_randPrimitive(c_primitive primitive, c_object *objectPtr, sd_randomizer randomizer) __nonnull_all__;

static c_bool
sd_randPrimitive(
    c_primitive primitive,
    c_object *objectPtr,
    sd_randomizer randomizer)
{
    switch (primitive->kind) {
#define __CASE__(kind) case P_##kind: SD_RAND(kind,randomizer,*objectPtr); return TRUE;
            __CASE__(CHAR)
            __CASE__(OCTET)
            __CASE__(BOOLEAN)
            __CASE__(USHORT)
            __CASE__(SHORT)
            __CASE__(ULONG)
            __CASE__(LONG)
            __CASE__(ULONGLONG)
            __CASE__(LONGLONG)
            __CASE__(FLOAT)
            __CASE__(DOUBLE)
#undef __CASE__
        default:
            SD_CONFIDENCE(FALSE);
            return FALSE;
    }
    return FALSE;
}

/** \brief Fill the data pointed to with a random enumeration value.
 *
 *  First, the number of elements in this enumeration is determined. Then
 *  a random value within this range is determined.
 */
static c_bool sd_randEnumeration(c_enumeration enumeration, c_object *objectPtr, sd_randomizer randomizer) __nonnull_all__;

static c_bool
sd_randEnumeration(
    c_enumeration enumeration,
    c_object *objectPtr,
    sd_randomizer randomizer)
{
    /* Use the metadata in order to find a correct value */
    c_ulong range;
    c_constant selectedEnum;
    c_value selectedValue;
    c_literal operandValue;

    range = c_arraySize(enumeration->elements);
    selectedEnum = enumeration->elements[sd_randomizerNextValue(randomizer) % range];
    operandValue = c_operandValue(selectedEnum->operand);
    selectedValue = operandValue->value;
    c_free(operandValue);
    SD_CONFIDENCE(selectedValue.kind == V_LONG);
    *((c_long *)(*objectPtr)) = selectedValue.is.Long;
    return TRUE;
}

/** \brief Main dispatcher for filling any data type with random values.*/
static c_bool sd_randType(c_type type, c_object *objectPtr, sd_randomizer randomizer) __nonnull_all__ __attribute_warn_unused_result__;

static c_bool
sd_randType(
    c_type type,
    c_object *objectPtr,
    sd_randomizer randomizer)
{
    c_type actualType;

    actualType = c_typeActualType(type);
    switch (c_baseObject(actualType)->kind) {
        case M_COLLECTION:
            return sd_randCollection(c_collectionType(actualType), objectPtr, randomizer);
        case M_PRIMITIVE:
            return sd_randPrimitive(c_primitive(actualType), objectPtr, randomizer);
        case M_ENUMERATION:
            return sd_randEnumeration(c_enumeration(actualType), objectPtr, randomizer);
        default:
            SD_CONFIDENCE(FALSE); /* No other expected than these */
            return FALSE;
    }
}

/** \brief Callback function for deepwalk.*/
static c_bool sd_randCallback(c_type type, c_object *objectPtr, void *arg) __nonnull_all__ __attribute_warn_unused_result__;

static c_bool
sd_randCallback(
    c_type type,
    c_object *objectPtr,
    void *arg)
{
    sd_randomizer randomizer = arg;
    return sd_randType(type, objectPtr, randomizer);
}


/* --------------------------------- public --------------------------------- */

/** \brief Constructor for the randomizer class.
 *
 *  \param base Database to create instances in.
 *  \return The newly created randomizer, to be released using randomizerFree.
 */

sd_randomizer
sd_randomizerNew(
    c_base base)
{
    sd_randomizer result = os_malloc(sizeof(*result));
    sd_randomizerInit(result, 0);
    result->base = base;
    return result;
}

/** \brief Newly created randomizers will be initialized with this seed value.*/
#define SD_DEFAULT_SEED (144267U)


/** \brief Method for setting the seed value, which determines the
 *         actual random sequence.
 *  Using the default seed value will always result in the same sequence of
 *  random values. By setting the seed to a different value, a different
 *  sequence will be generated. Random sequences can be reproduced by
 *  setting the seed to the same value.
 *
 *  \param randomizer The randomizer instance (self).
 *  \param seed The seed value to use for random number generation.
 */

void
sd_randomizerInit(
    sd_randomizer randomizer,
    c_ulong seed)
{
    if (seed > 0U) {
        randomizer->randValue = seed;
    } else {
        randomizer->randValue = SD_DEFAULT_SEED;
    }
}

#undef SD_DEFAULT_SEED


/** \brief Method for generating a random object instance.
 *
 *  \param randomizer The randomizer instance (self).
 *  \param typeName The scoped name of the type of the instance to be created.
 *  \return A randomly filled object instance, to be released with c_free.
 */

c_object
sd_randomizerRandomInstance(
    sd_randomizer randomizer,
    const c_char *typeName)
{
    c_object result = NULL;
    c_type resultType;

    if ((resultType = c_resolve(randomizer->base, typeName)) == NULL) {
        return NULL;
    } else if ((result = c_new_s(resultType)) == NULL) {
        c_free(resultType);
        return NULL;
    } else {
        if (!sd_deepwalk(resultType, &result, sd_randCallback, randomizer)) {
            c_free(resultType);
            c_free(result);
            return NULL;
        } else {
            c_free(resultType);
            return result;
        }
    }
}


/** \brief Destructor for randomizer instances.
 *
 *  \param randomizer The randomizer instance (self).
 */

void
sd_randomizerFree(
    sd_randomizer randomizer)
{
    os_free(randomizer);
}
