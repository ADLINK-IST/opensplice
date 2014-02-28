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
#include "saj_copyIn.h"
#include "saj_utilities.h"
#include "saj__exception.h"

#include "c_base.h"

#include "os.h"
#include "os_report.h"

#include <string.h>
#include <stdarg.h>

#define STATIC static

typedef struct {
    void *dst;
    c_base base;
    os_uint32 offset;
    JNIEnv *javaEnv;
} saj_context;

/* The threshold for obtaining a copy or directly accessing the char-
 * array has been heuristically determined to get the highest average
 * throughput. */
static const unsigned SAJ_CHAR_ARRAY_CRITICAL_HEURISTIC = 384;

typedef saj_copyResult (*copyInFromStruct)(sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
typedef saj_copyResult (*copyInFromGenericUnion)(sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
typedef saj_copyResult (*copyInFromSupportedUnion)(sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
typedef saj_copyResult (*copyInFromArray)(sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);

STATIC saj_copyResult saj_copyGetStatus  (saj_context* context);

    /* Primitive types */
STATIC saj_copyResult saj_cfsiBoolean    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiByte       (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiChar       (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiShort      (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiInt        (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiLong       (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiFloat      (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiDouble     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Array of primitive type */
STATIC saj_copyResult saj_cfsiArrBoolean (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiArrByte    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiArrChar    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiArrCharToBString    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiArrShort   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiArrInt     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiArrLong    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiArrFloat   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiArrDouble  (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfsiSeqBoolean (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiSeqByte    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiSeqChar    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiSeqShort   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiSeqInt     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiSeqLong    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiSeqFloat   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiSeqDouble  (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfsiEnum       (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfsiStruct     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiUnion      (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfsiString     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiBString    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsiBStringToArrChar
                                         (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfsiArray      (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfsiSequence   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* reference to previous defined type */
STATIC saj_copyResult saj_cfsiReference  (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);

    /* Primitive types */
STATIC saj_copyResult saj_cfuiBooleanGeneric    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiByteGeneric       (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiCharGeneric       (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiShortGeneric      (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiIntGeneric        (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiLongGeneric       (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiFloatGeneric      (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiDoubleGeneric     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Array of primitive type */
STATIC saj_copyResult saj_cfuiArrBooleanGeneric (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrByteGeneric    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrCharGeneric    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrCharToBStringGeneric
                                                (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrShortGeneric   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrIntGeneric     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrLongGeneric    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrFloatGeneric   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrDoubleGeneric  (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfuiSeqBooleanGeneric (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqByteGeneric    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqCharGeneric    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqShortGeneric   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqIntGeneric     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqLongGeneric    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqFloatGeneric   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqDoubleGeneric  (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfuiEnumGeneric       (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfuiStructGeneric     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiUnionGeneric      (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfuiStringGeneric     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiBStringGeneric    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiBStringToArrCharGeneric
                                                (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfuiArrayGeneric      (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfuiSequenceGeneric   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* reference to previous defined type */
STATIC saj_copyResult saj_cfuiReferenceGeneric  (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);

/* Primitive types */
STATIC saj_copyResult saj_cfuiBooleanSupported  (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiByteSupported     (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiCharSupported     (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiShortSupported    (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiIntSupported      (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiLongSupported     (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiFloatSupported    (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiDoubleSupported   (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
    /* Array of primitive type */
STATIC saj_copyResult saj_cfuiArrBooleanSupported(sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrByteSupported  (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrCharSupported  (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrCharToBStringSupported
                                                (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrShortSupported (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrIntSupported   (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrLongSupported  (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrFloatSupported (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrDoubleSupported(sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfuiSeqBooleanSupported(sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqByteSupported  (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqCharSupported  (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqShortSupported (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqIntSupported   (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqLongSupported  (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqFloatSupported (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqDoubleSupported(sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfuiEnumSupported     (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfuiStructSupported   (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiUnionSupported    (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfuiStringSupported   (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiBStringSupported  (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiBStringToArrCharSupported
                                                (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfuiArraySupported    (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfuiSequenceSupported (sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);
    /* reference to previous defined type */
STATIC saj_copyResult saj_cfuiReferenceSupported(sajCopyHeader *ch, jobject javaObject, jfieldID caseID, saj_context *ctx);

    /* Array of primitive type */
STATIC saj_copyResult saj_cfoiArrBoolean (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
STATIC saj_copyResult saj_cfoiArrByte    (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
STATIC saj_copyResult saj_cfoiArrChar    (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
STATIC saj_copyResult saj_cfoiArrCharToBString    (sajCopyHeader *ch, jobject objectArray, void *dst, saj_context *ctx);
STATIC saj_copyResult saj_cfoiArrShort   (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
STATIC saj_copyResult saj_cfoiArrInt     (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
STATIC saj_copyResult saj_cfoiArrLong    (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
STATIC saj_copyResult saj_cfoiArrFloat   (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
STATIC saj_copyResult saj_cfoiArrDouble  (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfoiSeqBoolean (sajCopyHeader *ch, jobject objectArray, void *dstSeq, saj_context *ctx);
STATIC saj_copyResult saj_cfoiSeqByte    (sajCopyHeader *ch, jobject objectArray, void *dstSeq, saj_context *ctx);
STATIC saj_copyResult saj_cfoiSeqChar    (sajCopyHeader *ch, jobject objectArray, void *dstSeq, saj_context *ctx);
STATIC saj_copyResult saj_cfoiSeqShort   (sajCopyHeader *ch, jobject objectArray, void *dstSeq, saj_context *ctx);
STATIC saj_copyResult saj_cfoiSeqInt     (sajCopyHeader *ch, jobject objectArray, void *dstSeq, saj_context *ctx);
STATIC saj_copyResult saj_cfoiSeqLong    (sajCopyHeader *ch, jobject objectArray, void *dstSeq, saj_context *ctx);
STATIC saj_copyResult saj_cfoiSeqFloat   (sajCopyHeader *ch, jobject objectArray, void *dstSeq, saj_context *ctx);
STATIC saj_copyResult saj_cfoiSeqDouble  (sajCopyHeader *ch, jobject objectArray, void *dstSeq, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfoiEnum       (sajCopyHeader *ch, jobject enumObject, void *dstEnum, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfoiStruct     (sajCopyHeader *ch, jobject structObject, void *dstStruct, saj_context *ctx);
STATIC saj_copyResult saj_cfoiUnion      (sajCopyHeader *ch, jobject unionObject,  void *dstUnion,  saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfoiString     (sajCopyHeader *ch, jobject stringObject, void *dstString, saj_context *ctx);
STATIC saj_copyResult saj_cfoiBString    (sajCopyHeader *ch, jobject stringObject, void *dstString, saj_context *ctx);
STATIC saj_copyResult saj_cfoiBStringToArrChar
                                         (sajCopyHeader *ch, jobject stringObject, void *dstString, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfoiArray      (sajCopyHeader *ch, jobject objectArray, void *dstArray, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfoiSequence   (sajCopyHeader *ch, jobject objectSequence, void *dstSeq, saj_context *ctx);
    /* reference to previous defined type */
STATIC saj_copyResult saj_cfoiReference  (sajCopyHeader *ch, jobject objectSequence, void *dstSeq, saj_context *ctx);

STATIC copyInFromStruct ciFromStruct[] = {
    saj_cfsiBoolean,
    saj_cfsiByte,
    saj_cfsiChar,
    saj_cfsiShort,
    saj_cfsiInt,
    saj_cfsiLong,
    saj_cfsiFloat,
    saj_cfsiDouble,
    saj_cfsiArrBoolean,
    saj_cfsiArrByte,
    saj_cfsiArrChar,
    saj_cfsiArrCharToBString,
    saj_cfsiArrShort,
    saj_cfsiArrInt,
    saj_cfsiArrLong,
    saj_cfsiArrFloat,
    saj_cfsiArrDouble,
    saj_cfsiSeqBoolean,
    saj_cfsiSeqByte,
    saj_cfsiSeqChar,
    saj_cfsiSeqShort,
    saj_cfsiSeqInt,
    saj_cfsiSeqLong,
    saj_cfsiSeqFloat,
    saj_cfsiSeqDouble,
    saj_cfsiEnum,
    saj_cfsiStruct,
    saj_cfsiUnion,
    saj_cfsiString,
    saj_cfsiBString,
    saj_cfsiBStringToArrChar,
    saj_cfsiArray,
    saj_cfsiSequence,
    saj_cfsiReference
    };

STATIC copyInFromGenericUnion ciFromGenericUnion[] = {
    saj_cfuiBooleanGeneric,
    saj_cfuiByteGeneric,
    saj_cfuiCharGeneric,
    saj_cfuiShortGeneric,
    saj_cfuiIntGeneric,
    saj_cfuiLongGeneric,
    saj_cfuiFloatGeneric,
    saj_cfuiDoubleGeneric,
    saj_cfuiArrBooleanGeneric,
    saj_cfuiArrByteGeneric,
    saj_cfuiArrCharGeneric,
    saj_cfuiArrCharToBStringGeneric,
    saj_cfuiArrShortGeneric,
    saj_cfuiArrIntGeneric,
    saj_cfuiArrLongGeneric,
    saj_cfuiArrFloatGeneric,
    saj_cfuiArrDoubleGeneric,
    saj_cfuiSeqBooleanGeneric,
    saj_cfuiSeqByteGeneric,
    saj_cfuiSeqCharGeneric,
    saj_cfuiSeqShortGeneric,
    saj_cfuiSeqIntGeneric,
    saj_cfuiSeqLongGeneric,
    saj_cfuiSeqFloatGeneric,
    saj_cfuiSeqDoubleGeneric,
    saj_cfuiEnumGeneric,
    saj_cfuiStructGeneric,
    saj_cfuiUnionGeneric,
    saj_cfuiStringGeneric,
    saj_cfuiBStringGeneric,
    saj_cfuiBStringToArrCharGeneric,
    saj_cfuiArrayGeneric,
    saj_cfuiSequenceGeneric,
    saj_cfuiReferenceGeneric
    };

STATIC copyInFromSupportedUnion ciFromSupportedUnion[] = {
    saj_cfuiBooleanSupported,
    saj_cfuiByteSupported,
    saj_cfuiCharSupported,
    saj_cfuiShortSupported,
    saj_cfuiIntSupported,
    saj_cfuiLongSupported,
    saj_cfuiFloatSupported,
    saj_cfuiDoubleSupported,
    saj_cfuiArrBooleanSupported,
    saj_cfuiArrByteSupported,
    saj_cfuiArrCharSupported,
    saj_cfuiArrCharToBStringSupported,
    saj_cfuiArrShortSupported,
    saj_cfuiArrIntSupported,
    saj_cfuiArrLongSupported,
    saj_cfuiArrFloatSupported,
    saj_cfuiArrDoubleSupported,
    saj_cfuiSeqBooleanSupported,
    saj_cfuiSeqByteSupported,
    saj_cfuiSeqCharSupported,
    saj_cfuiSeqShortSupported,
    saj_cfuiSeqIntSupported,
    saj_cfuiSeqLongSupported,
    saj_cfuiSeqFloatSupported,
    saj_cfuiSeqDoubleSupported,
    saj_cfuiEnumSupported,
    saj_cfuiStructSupported,
    saj_cfuiUnionSupported,
    saj_cfuiStringSupported,
    saj_cfuiBStringSupported,
    saj_cfuiBStringToArrCharSupported,
    saj_cfuiArraySupported,
    saj_cfuiSequenceSupported,
    saj_cfuiReferenceSupported
    };

STATIC copyInFromArray ciFromArray[] = {
    NULL, /* saj_cfoiBoolean */
    NULL, /* saj_cfoiByte */
    NULL, /* saj_cfoiChar */
    NULL, /* saj_cfoiShort */
    NULL, /* saj_cfoiInt */
    NULL, /* saj_cfoiLong */
    NULL, /* saj_cfoiFloat */
    NULL, /* saj_cfoiDouble */
    saj_cfoiArrBoolean,
    saj_cfoiArrByte,
    saj_cfoiArrChar,
    saj_cfoiArrCharToBString,
    saj_cfoiArrShort,
    saj_cfoiArrInt,
    saj_cfoiArrLong,
    saj_cfoiArrFloat,
    saj_cfoiArrDouble,
    saj_cfoiSeqBoolean,
    saj_cfoiSeqByte,
    saj_cfoiSeqChar,
    saj_cfoiSeqShort,
    saj_cfoiSeqInt,
    saj_cfoiSeqLong,
    saj_cfoiSeqFloat,
    saj_cfoiSeqDouble,
    saj_cfoiEnum,
    saj_cfoiStruct,
    saj_cfoiUnion,
    saj_cfoiString,
    saj_cfoiBString,
    saj_cfoiBStringToArrChar,
    saj_cfoiArray,
    saj_cfoiSequence,
    saj_cfoiReference
    };

#ifdef NDEBUG
STATIC saj_copyResult
saj_copyGetStatus(
        saj_context* context)
{
    OS_UNUSED_ARG(context);
    return SAJ_COPYRESULT_OK;
}
#else

STATIC saj_copyResult
saj_copyGetStatus(
        saj_context* context)
{
    jboolean errorOccurred;
    saj_copyResult result;

    errorOccurred = ((*(context->javaEnv))->ExceptionCheck (context->javaEnv));

    if(errorOccurred == JNI_TRUE){
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "copyIn failed, because application data is invalid.");
        result = SAJ_COPYRESULT_ERROR;
    } else {
        result = SAJ_COPYRESULT_OK;
    }
    return result;
}
#endif

/* Obtain fieldname */
STATIC jstring
saj_fieldNameGet(jobject obj, jfieldID javaFID, saj_context *ctx) {
    jobject field;
    jclass cls, fieldCls;
    jmethodID mid;

    /* Get class of object */
    cls = (*(ctx->javaEnv))->GetObjectClass(ctx->javaEnv, obj);
    assert(cls);

    /* Translate fieldID to (non-static) field object */
    field = (*(ctx->javaEnv))->ToReflectedField(ctx->javaEnv, cls, javaFID, JNI_FALSE);
    assert(field);

    /* Get class of field object - is Field */
    fieldCls = (*(ctx->javaEnv))->GetObjectClass(ctx->javaEnv, field);
    assert(fieldCls);

    /* Get method 'GetFieldName' of class 'Field'. */
    mid = (*(ctx->javaEnv))->GetMethodID(ctx->javaEnv, fieldCls, "getName", "()Ljava/lang/String;");
    assert(mid);

    /* Call method to obtain fieldname */
    return (jstring)(*(ctx->javaEnv))->CallObjectMethod(ctx->javaEnv, field, mid);
}

/* Obtain methodname */
STATIC jstring
saj_methodNameGet(jobject obj, jmethodID javaMID, saj_context *ctx) {
    jobject method;
    jclass cls, methodCls;
    jmethodID mid;

    /* Get class of object */
    cls = (*(ctx->javaEnv))->GetObjectClass(ctx->javaEnv, obj);
    assert(cls);

    /* Translate fieldID to (non-static) field object */
    method = (*(ctx->javaEnv))->ToReflectedMethod(ctx->javaEnv, cls, javaMID, JNI_FALSE);
    assert(method);

    /* Get class of field object - is Field */
    methodCls = (*(ctx->javaEnv))->GetObjectClass(ctx->javaEnv, method);
    assert(methodCls);

    /* Get method 'GetFieldName' of class 'Field'. */
    mid = (*(ctx->javaEnv))->GetMethodID(ctx->javaEnv, methodCls, "getName", "()Ljava/lang/String;");
    assert(mid);

    /* Call method to obtain fieldname */
    return (jstring)(*(ctx->javaEnv))->CallObjectMethod(ctx->javaEnv, method, mid);
}

/* Report failed copyIn for specific field */
STATIC void
saj_reportCopyInFail(jobject obj, jfieldID javaFID, saj_context *ctx) {
    jstring stringJ;
    const char* stringC;

    /* Report fieldname where error occurred. */
    stringJ = saj_fieldNameGet(obj, javaFID, ctx);
    stringC = (*(ctx->javaEnv))->GetStringUTFChars(ctx->javaEnv, stringJ, NULL);
    OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "copyIn failed for field '%s'.", stringC);
    (*(ctx->javaEnv))->ReleaseStringUTFChars(ctx->javaEnv, stringJ, stringC);
}

/* Report failed copyIn for specific field */
STATIC void
saj_reportCopyInFail_generic(jobject obj, jmethodID javaMID, saj_context *ctx) {
    jstring stringJ;
    const char* stringC;

    /* Report fieldname where error occurred. */
    stringJ = saj_methodNameGet(obj, javaMID, ctx);
    stringC = (*(ctx->javaEnv))->GetStringUTFChars(ctx->javaEnv, stringJ, NULL);
    OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "copyIn failed for method '%s'.", stringC);
    (*(ctx->javaEnv))->ReleaseStringUTFChars(ctx->javaEnv, stringJ, stringC);
}

    /* Primitive types */
STATIC saj_copyResult
saj_cfsiBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_bool *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_bool *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetBooleanField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetBooleanField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Boolean = %d @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiBooleanGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_bool *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_bool *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallBooleanMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallBooleanMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Boolean = %d getterID = %x\n", *dst, (int)getterID));

    return result;
}

STATIC saj_copyResult
saj_cfuiBooleanSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_bool *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_bool *)ctx->dst;
    *dst = (*(ctx->javaEnv))->GetBooleanField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetBooleanField (0x%x, %d) = %d\n", javaObject, caseID, *dst));
    TRACE(printf ("Copied in Boolean = %d caseID = %x\n", *dst, (int)caseID));

    return result;
}

STATIC saj_copyResult
saj_cfsiByte (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_octet *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_octet *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetByteField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetByteField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Byte = %hd @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiByteGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_octet *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_octet *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallByteMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallByteMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Byte = %d getterID = %x\n", *dst, (int)getterID));

    return result;
}

STATIC saj_copyResult
saj_cfuiByteSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_octet *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_octet *)ctx->dst;
    *dst = (*(ctx->javaEnv))->GetByteField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetByteField (0x%x, %d) = %d\n", javaObject, caseID, *dst));
    TRACE(printf ("Copied in Byte = %d caseID = %x\n", *dst, (int)caseID));

    return result;
}

STATIC saj_copyResult
saj_cfsiChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_char *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_char *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (c_char)(*(ctx->javaEnv))->GetCharField(ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetCharField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Char = %hd @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiCharGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_char *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_char *)ctx->dst;
    *dst = (c_char)(*(ctx->javaEnv))->CallCharMethod(ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallCharMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Char = %d getterID = %x\n", *dst, (int)getterID));

    return result;
}

STATIC saj_copyResult
saj_cfuiCharSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_char *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_char *)ctx->dst;
    *dst = (c_char)(*(ctx->javaEnv))->GetCharField(ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetCharField (0x%x, %d) = %d\n", javaObject, caseID, *dst));
    TRACE(printf ("Copied in Char = %d caseID = %x\n", *dst, (int)caseID));

    return result;
}

STATIC saj_copyResult
saj_cfsiShort (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_short *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_short *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetShortField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetShortField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Short = %hd @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiShortGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_short *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_short *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallShortMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallShortMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Short = %d getterID = %x\n", *dst, (int)getterID));

    return result;
}

STATIC saj_copyResult
saj_cfuiShortSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_short *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_short *)ctx->dst;
    *dst = (*(ctx->javaEnv))->GetShortField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetShortField (0x%x, %d) = %d\n", javaObject, caseID, *dst));
    TRACE(printf ("Copied in Short = %d caseID = %x\n", *dst, (int)caseID));

    return result;
}

STATIC saj_copyResult
saj_cfsiInt (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_long *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_long *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetIntField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetIntField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Int = %d @ offset = %d FID = %x Result = %d\n", *dst, ctx->offset, (int)javaFID, result));

    return result;
}

STATIC saj_copyResult
saj_cfuiIntGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_long *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_long *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallIntMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallIntMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Int = %d getterID = %x\n", *dst, (int)getterID));

    return result;
}

STATIC saj_copyResult
saj_cfuiIntSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_long *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_long *)ctx->dst;
    *dst = (*(ctx->javaEnv))->GetIntField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetIntField (0x%x, %d) = %d\n", javaObject, caseID, *dst));
    TRACE(printf ("Copied in Int = %d caseID = %x\n", *dst, (int)caseID));

    return result;
}

STATIC saj_copyResult
saj_cfsiLong (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_longlong *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_longlong *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetLongField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetLongField (0x%x, %d) = %lld\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Long = %lld @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiLongGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_longlong *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_longlong *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallLongMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallLongMethod (0x%x, %d) = %lld\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Long = %lld getterID = %x\n", *dst, (int)getterID));

    return result;
}

STATIC saj_copyResult
saj_cfuiLongSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_longlong *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_longlong *)ctx->dst;
    *dst = (*(ctx->javaEnv))->GetLongField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetLongField (0x%x, %d) = %lld\n", javaObject, caseID, *dst));
    TRACE(printf ("Copied in Long = %lld caseID = %x\n", *dst, (int)caseID));

    return result;
}

STATIC saj_copyResult
saj_cfsiFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_float *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_float *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetFloatField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetFloatField (0x%x, %d) = %f\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Float = %f @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiFloatGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_float *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_float *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallFloatMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallFloatMethod (0x%x, %d) = %lld\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Float = %f getterID = %x\n", *dst, (int)getterID));

    return result;

}

STATIC saj_copyResult
saj_cfuiFloatSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_float *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_float *)ctx->dst;
    *dst = (*(ctx->javaEnv))->GetFloatField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetFloatField (0x%x, %d) = %lld\n", javaObject, caseID, *dst));
    TRACE(printf ("Copied in Float = %f caseID = %x\n", *dst, (int)caseID));

    return result;

}

STATIC saj_copyResult
saj_cfsiDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_double *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_double *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetDoubleField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetDoubleField (0x%x, %d) = %f\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Double = %f @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;

}

STATIC saj_copyResult
saj_cfuiDoubleGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_double *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_double *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallDoubleMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallDoubleMethod (0x%x, %d) = %lld\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Double = %f getterID = %x\n", *dst, (int)getterID));

    return result;
}

STATIC saj_copyResult
saj_cfuiDoubleSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_double *dst;

    OS_UNUSED_ARG(ch);

    dst = (c_double *)ctx->dst;
    *dst = (*(ctx->javaEnv))->GetDoubleField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetDoubleField (0x%x, %d) = %lld\n", javaObject, caseID, *dst));
    TRACE(printf ("Copied in Double = %f caseID = %x\n", *dst, (int)caseID));

    return result;
}

    /* Enumeration type */
STATIC saj_copyResult
saj_cfoiEnum (
    sajCopyHeader *ch,
    jobject enumObject,
    void *dstEnum,
    saj_context *ctx)
{
    saj_copyResult result;
    sajCopyEnum *copyEnum;
    c_long *dst;

    copyEnum = (sajCopyEnum *)ch;
    dst = (c_long *)dstEnum;
    if (copyEnum->enumType == SAJ_SUPPORTED_ENUM)
    {
        *dst = (*(ctx->javaEnv))->GetIntField (ctx->javaEnv, enumObject, copyEnum->enumAccessors.supportedEnum.valueID);
    }
    else
    {
        *dst = (*(ctx->javaEnv))->CallIntMethod (ctx->javaEnv, enumObject, copyEnum->enumAccessors.genericEnum.valueID);
    }
    result = saj_copyGetStatus(ctx);

    if((*dst < 0) || (((unsigned int)*dst) >= copyEnum->nrOfElements)){
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Invalid enumeration label.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    }

    TRACE(printf ("JNI: CallIntMethod (0x%x, %d) = 0x%x\n", enumObject, copyEnum->valueID, *dst));
    TRACE(printf ("Copied in in Enum = %d @ offset = %d\n", *dst, ctx->offset));

    return result;
}

STATIC saj_copyResult
saj_cfsiEnum (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    jobject enumObject;
    c_long *dst;

    enumObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);
    TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, enumObject));

    if(result == SAJ_COPYRESULT_OK){
        dst = (c_long *)((PA_ADDRCAST)ctx->dst + ctx->offset);
        result = saj_cfoiEnum (ch, enumObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiEnumGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    jobject enumObject;
    c_long *dst;

    enumObject = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, enumObject));
        dst = (c_long *)ctx->dst;
        result = saj_cfoiEnum (ch, enumObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiEnumSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    jobject enumObject;
    c_long *dst;

    enumObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, enumObject));
        dst = (c_long *)ctx->dst;
        result = saj_cfoiEnum (ch, enumObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

    /* Array of primitive type */
STATIC saj_copyResult
saj_cfoiArrBoolean (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    saj_copyResult result;
    sajCopyArray *ah;
    jbooleanArray array;
    c_bool *dst;

    array = (jbooleanArray)objectArray;
    dst = (c_bool *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Boolean array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Boolean array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        /* Due to the below constraint and the fact that for jboolean as well as
         * for c_bool every non-0 value is considered true, GetBooleanArrayRegion
         * can directly copy to the target array.   */
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_bool, jboolean);
        (*(ctx->javaEnv))->GetBooleanArrayRegion (ctx->javaEnv, array, 0, ah->size, dst);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetBooleanArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, dst, saj_copyResultImage(result)));
        TRACE(
        if(result == SAJ_COPYRESULT_OK){
            printf ("Copied in Boolean array size %d @ offset = %d\n", ah->size, ctx->offset);
        })
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrBoolean (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrBooleanGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);
    TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));

    if(result == SAJ_COPYRESULT_OK){
        result = saj_cfoiArrBoolean (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrBooleanSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);
    TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));

    if(result == SAJ_COPYRESULT_OK){
        result = saj_cfoiArrBoolean (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiArrByte (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    saj_copyResult result;
    sajCopyArray *ah;
    jbyteArray array;
    c_octet *dst;

    array = (jbyteArray)objectArray;
    dst = (c_octet *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Byte array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else  if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Byte array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_octet, jbyte);
        (*(ctx->javaEnv))->GetByteArrayRegion (ctx->javaEnv, array, 0, ah->size, (jbyte *)dst);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetByteArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, dst, saj_copyResultImage(result)));
        TRACE(
        if(result == SAJ_COPYRESULT_OK){
            printf ("Copied in Byte array size %d @ offset = %d\n", ah->size, ctx->offset);
        })
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrByte (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrByte (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrByteGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArrByte (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrByteSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArrByte (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiArrChar (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    saj_copyResult result;
    sajCopyArray *ah;
    jcharArray array;
    jchar *charArray;

    array = (jcharArray)objectArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK && ah->size){
        /* Access to the char-array is obtained in two ways. When the size of
         * an array is below a certain threshold, access is gained by means
         * of GetPrimitiveArrayCritical, which is very likely to not perform
         * an intermediate copy. In the other case, the array is copied into
         * an intermediate buffer on heap. */
        if(ah->size <= SAJ_CHAR_ARRAY_CRITICAL_HEURISTIC){
            jboolean isCopy;

            charArray = (*(ctx->javaEnv))->GetPrimitiveArrayCritical (ctx->javaEnv, array, &isCopy);
            TRACE(printf ("JNI: GetPrimitiveArrayCritical (%p, %p, isCopy=%s)\n", ctx->javaEnv, array, isCopy ? "yes" : "no"));
        } else {
            if((charArray = os_malloc (sizeof (jchar) * ah->size)) != NULL){
                (*(ctx->javaEnv))->GetCharArrayRegion (ctx->javaEnv, array, 0, ah->size, charArray);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetCharArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, charArray, saj_copyResultImage(result)));
            } else {
                result = SAJ_COPYRESULT_ERROR; /* Out of (heap) resources */
            }
        }

        if(result == SAJ_COPYRESULT_OK){
#ifndef OSPL_SAJ_NO_LOOP_UNROLLING
            SAJ_LOOP_UNROLL(ah->size, jchar, charArray, c_char, dstArray);
#else /* OSPL_SAJ_NO_LOOP_UNROLLING */
            unsigned int i;
            c_char *dst = (c_char *) dstArray;
            for(i = 0; i < ah->size; i++){
                dst[i] = (c_char)charArray[i];
                TRACE(printf ("%d;", dst[i]));
            }
#endif /* OSPL_SAJ_NO_LOOP_UNROLLING */
            TRACE(printf ("Copied in Char array size %d @ offset = %d\n", ah->size, ctx->offset));

            if(ah->size <= SAJ_CHAR_ARRAY_CRITICAL_HEURISTIC){
                /* Since we only read data, release the array with JNI_ABORT */
                (*(ctx->javaEnv))->ReleasePrimitiveArrayCritical (ctx->javaEnv, array, charArray, JNI_ABORT);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: ReleasePrimitiveArrayCritical (%p, %p, %p, JNI_ABORT) -> %s\n", ctx->javaEnv, ctx->javaEnv, charArray, saj_copyResultImage(result)));
            } else {
                os_free(charArray);
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrChar (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrCharGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArrChar (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrCharSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArrChar (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrCharToBStringGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArrCharToBString (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrCharToBStringSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    saj_copyResult result;
    void *dst;
    jobjectArray array;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArrCharToBString (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiArrShort (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    saj_copyResult result;
    sajCopyArray *ah;
    jshortArray array;
    c_short *dst;

    array = (jshortArray)objectArray;
    dst = (c_short *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Short array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Short array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_short, jshort);
        (*(ctx->javaEnv))->GetShortArrayRegion (ctx->javaEnv, array, 0, ah->size, dst);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetShortArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, dst, saj_copyResultImage(result)));
        TRACE(
        if(result == SAJ_COPYRESULT_OK){
            printf ("Copied in Short array size %d @ offset = %d\n", ah->size, ctx->offset);
        })
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrShort (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrShort (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrShortGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArrShort (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrShortSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArrShort (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiArrInt (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jintArray array;
    c_long *dst;
    saj_copyResult result;

    array = (jintArray)objectArray;
    dst = (c_long *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Int array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Int array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_long, jint);
        (*(ctx->javaEnv))->GetIntArrayRegion (ctx->javaEnv, array, 0, ah->size, dst);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetIntArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, dst, saj_copyResultImage(result)));
        TRACE(
        if(result == SAJ_COPYRESULT_OK){
            printf ("Copied in Int array size %d @ offset = %d\n", ah->size, ctx->offset);
        })
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrInt (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrInt (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrIntGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArrInt (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrIntSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArrInt (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiArrLong (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jlongArray array;
    c_longlong *dst;
    saj_copyResult result;

    array = (jlongArray)objectArray;
    dst = (c_longlong *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Long array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Long array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_longlong, jlong);
        (*(ctx->javaEnv))->GetLongArrayRegion (ctx->javaEnv, array, 0, ah->size, dst);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetLongArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, dst, saj_copyResultImage(result)));
        TRACE(
        if(result == SAJ_COPYRESULT_OK){
            printf ("Copied in Long array size %d @ offset = %d\n", ah->size, ctx->offset);
        })
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrLong (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrLong (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrLongGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArrLong (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrLongSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArrLong (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiArrFloat (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jfloatArray array;
    c_float *dst;
    saj_copyResult result;

    array = (jfloatArray)objectArray;
    dst = (c_float *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Float array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Float array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_float, jfloat);
        (*(ctx->javaEnv))->GetFloatArrayRegion (ctx->javaEnv, array, 0, ah->size, dst);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetFloatArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, dst, saj_copyResultImage(result)));
        TRACE(
        if(result == SAJ_COPYRESULT_OK){
            printf ("Copied in Float array size %d @ offset = %d\n", ah->size, ctx->offset);
        })
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrFloat (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrFloatGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArrFloat (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrFloatSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArrFloat (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiArrDouble (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jdoubleArray array;
    c_double *dst;
    saj_copyResult result;

    array = (jdoubleArray)objectArray;
    dst = (c_double *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Double array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Double array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_double, jdouble);
        (*(ctx->javaEnv))->GetDoubleArrayRegion (ctx->javaEnv, array, 0, ah->size, dst);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetDoubleArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, dst, saj_copyResultImage(result)));
        TRACE(
        if(result == SAJ_COPYRESULT_OK){
            printf ("Copied in Double array size %d @ offset = %d\n", ah->size, ctx->offset);
        })
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrDouble (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrDoubleGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArrDouble (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrDoubleSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray array;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArrDouble (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

    /* Sequence of primitive type */
STATIC saj_copyResult
saj_cfoiSeqBoolean (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jbooleanArray array;
    jsize arrLen;
    c_bool **dst;
    saj_copyResult result;

    dst = (c_bool **)dstSeq;
    sh = (sajCopySequence *)ch;
    array = (jbooleanArray)objectArray;

    if (array == NULL) {
        (*dst) = (c_bool *)c_sequenceNew (sh->type, sh->size, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        arrLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->size && (arrLen > (int)sh->size)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Boolean sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else if((*dst = (c_bool *)c_sequenceNew (sh->type, sh->size, arrLen)) != NULL){
                SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_bool, jboolean);
                (*(ctx->javaEnv))->GetBooleanArrayRegion (ctx->javaEnv, array, 0, arrLen, *dst);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetBooleanArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, arrLen, dst, saj_copyResultImage(result)));
                TRACE(
                if(result == SAJ_COPYRESULT_OK){
                    printf ("Copied in Boolean sequence size %d @ offset = %d\n", arrLen, ctx->offset);
                })
            } else {
                OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_sequenceNew<c_bool> failed for length %d.", arrLen);
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSeqBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, sequence));
        result = saj_cfoiSeqBoolean (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqBooleanGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, sequence));
        result = saj_cfoiSeqBoolean (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqBooleanSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, sequence));
        result = saj_cfoiSeqBoolean (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiSeqByte (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jbyteArray array;
    jsize arrLen;
    c_octet **dst;
    saj_copyResult result;

    dst = (c_octet **)dstSeq;
    sh = (sajCopySequence *)ch;
    array = (jbyteArray)objectArray;

    if (array == NULL) {
        (*dst) = (c_octet *)c_sequenceNew (sh->type, sh->size, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        arrLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->size && (arrLen > (int)sh->size)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Byte sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else if((*dst = (c_octet *)c_sequenceNew (sh->type, sh->size, arrLen)) != NULL){
                SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_octet, jbyte);
                (*(ctx->javaEnv))->GetByteArrayRegion (ctx->javaEnv, array, 0, arrLen, (jbyte *)*dst);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetByteArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, arrLen, dst, saj_copyResultImage(result)));
                TRACE(
                if(result == SAJ_COPYRESULT_OK){
                    printf ("Copied in Byte sequence size %d @ offset = %d\n", arrLen, ctx->offset);
                })
            } else {
                OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_sequenceNew<c_octet> failed for length %d.", arrLen);
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSeqByte (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, sequence));
        result = saj_cfoiSeqByte (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqByteGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, sequence));
        result = saj_cfoiSeqByte (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqByteSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, sequence));
        result = saj_cfoiSeqByte (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiSeqChar (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jcharArray array;
    jchar *charArray;
    jsize arrLen;
    c_char **dst;
    saj_copyResult result;

    dst = (c_char **)dstSeq;
    sh = (sajCopySequence *)ch;
    array = (jcharArray)objectArray;

    if (array == NULL) {
        (*dst) = (c_char *)c_sequenceNew (sh->type, sh->size, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        arrLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->size && (arrLen > (int)sh->size)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else if((*dst = (c_char *)c_sequenceNew (sh->type, sh->size, arrLen)) != NULL && arrLen){
                /* Access to the char-array is obtained in two ways. When the size of
                 * an array is below a certain threshold, access is gained by means
                 * of GetPrimitiveArrayCritical, which is very likely to not perform
                 * an intermediate copy. In the other case, the array is copied into
                 * an intermediate buffer on heap. */
                if((unsigned)arrLen <= SAJ_CHAR_ARRAY_CRITICAL_HEURISTIC){
                    jboolean isCopy;

                    charArray = (*(ctx->javaEnv))->GetPrimitiveArrayCritical (ctx->javaEnv, array, &isCopy);
                    TRACE(printf ("JNI: GetPrimitiveArrayCritical (%p, %p, isCopy=%s)\n", ctx->javaEnv, array, isCopy ? "yes" : "no"));
                } else {
                    if((charArray = os_malloc (sizeof (jchar) * arrLen)) != NULL){
                        (*(ctx->javaEnv))->GetCharArrayRegion (ctx->javaEnv, array, 0, arrLen, charArray);
                        result = saj_copyGetStatus(ctx);
                        TRACE(printf ("JNI: GetCharArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, arrLen, charArray, saj_copyResultImage(result)));
                    } else {
                        result = SAJ_COPYRESULT_ERROR; /* Out of (heap) resources */
                    }
                }

                if(result == SAJ_COPYRESULT_OK){
#ifndef OSPL_SAJ_NO_LOOP_UNROLLING
                    SAJ_LOOP_UNROLL(arrLen, jchar, charArray, c_char, *dst);
#else /* OSPL_SAJ_NO_LOOP_UNROLLING */
                    unsigned int i;
                    for (i = 0; i < arrLen; i++) {
                        (*dst)[i] = (c_char)charArray[i];
                        TRACE(printf ("%d;", (*dst)[i]));
                    }
#endif /* OSPL_SAJ_NO_LOOP_UNROLLING */
                   TRACE(printf ("Copied in Char sequence size %d @ offset = %d\n", arrLen, ctx->offset));

                    if((unsigned)arrLen <= SAJ_CHAR_ARRAY_CRITICAL_HEURISTIC){
                        /* Since we only read data, release the array with JNI_ABORT */
                        (*(ctx->javaEnv))->ReleasePrimitiveArrayCritical (ctx->javaEnv, array, charArray, JNI_ABORT);
                        result = saj_copyGetStatus(ctx);
                        TRACE(printf ("JNI: ReleasePrimitiveArrayCritical (%p, %p, %p, JNI_ABORT) -> %s\n", ctx->javaEnv, ctx->javaEnv, charArray, saj_copyResultImage(result)));
                    } else {
                        os_free(charArray);
                    }
                }
            } else if (arrLen) {
                OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_sequenceNew<c_char> failed for length %d.", arrLen);
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSeqChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, sequence));
        result = saj_cfoiSeqChar (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqCharGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, sequence));
        result = saj_cfoiSeqChar (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqCharSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, sequence));
        result = saj_cfoiSeqChar (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiSeqShort (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jshortArray array;
    jsize arrLen;
    c_short **dst;
    saj_copyResult result;

    dst = (c_short **)dstSeq;
    sh = (sajCopySequence *)ch;
    array = (jshortArray)objectArray;

    if (array == NULL) {
        (*dst) = (c_short *)c_sequenceNew (sh->type, sh->size, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        arrLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->size && (arrLen > (int)sh->size)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Short sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else if((*dst = (c_short *)c_sequenceNew (sh->type, sh->size, arrLen)) != NULL){
                SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_short, jshort);
                (*(ctx->javaEnv))->GetShortArrayRegion (ctx->javaEnv, array, 0, arrLen, *dst);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetShortArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, arrLen, dst, saj_copyResultImage(result)));
                TRACE(
                if(result == SAJ_COPYRESULT_OK){
                    printf ("Copied in Short sequence size %d @ offset = %d\n", arrLen, ctx->offset);
                })
            } else {
                OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_sequenceNew<c_short> failed for length %d.", arrLen);
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSeqShort (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, sequence));
        result = saj_cfoiSeqShort (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqShortGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, sequence));
        result = saj_cfoiSeqShort (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqShortSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, sequence));
        result = saj_cfoiSeqShort (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiSeqInt (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jintArray array;
    jsize arrLen;
    c_long **dst;
    saj_copyResult result;

    dst = (c_long **)dstSeq;
    sh = (sajCopySequence *)ch;
    array = (jintArray)objectArray;

    if (array == NULL) {
        (*dst) = (c_long *)c_sequenceNew (sh->type, sh->size, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        arrLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->size && (arrLen > (int)sh->size)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Int sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else if((*dst = (c_long *)c_sequenceNew (sh->type, sh->size, arrLen)) != NULL){
                SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_long, jint);
                (*(ctx->javaEnv))->GetIntArrayRegion (ctx->javaEnv, array, 0, arrLen, *dst);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetIntArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, arrLen, dst, saj_copyResultImage(result)));
                TRACE(
                if(result == SAJ_COPYRESULT_OK){
                    printf ("Copied in Int sequence size %d @ offset = %d\n", arrLen, ctx->offset);
                })
            } else {
                OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_sequenceNew<c_long> failed for length %d.", arrLen);
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSeqInt (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, sequence));
        result = saj_cfoiSeqInt (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqIntGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, sequence));
        result = saj_cfoiSeqInt (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqIntSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, sequence));
        result = saj_cfoiSeqInt (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiSeqLong (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jlongArray array;
    jsize arrLen;
    c_longlong **dst;
    saj_copyResult result;

    dst = (c_longlong **)dstSeq;
    sh = (sajCopySequence *)ch;
    array = (jlongArray)objectArray;

    if (array == NULL) {
        (*dst) = (c_longlong *)c_sequenceNew (sh->type, sh->size, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        arrLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->size && (arrLen > (int)sh->size)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Long sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else if((*dst = (c_longlong *)c_sequenceNew (sh->type, sh->size, arrLen)) != NULL){
                SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_longlong, jlong);
                (*(ctx->javaEnv))->GetLongArrayRegion (ctx->javaEnv, array, 0, arrLen, *dst);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetLongArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, arrLen, dst, saj_copyResultImage(result)));
                TRACE(
                if(result == SAJ_COPYRESULT_OK){
                    printf ("Copied in Long sequence size %d @ offset = %d\n", arrLen, ctx->offset);
                })
            } else {
                OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_sequenceNew<c_longlong> failed for length %d.", arrLen);
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSeqLong (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, sequence));
        result = saj_cfoiSeqLong (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqLongGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, sequence));
        result = saj_cfoiSeqLong (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqLongSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, sequence));
        result = saj_cfoiSeqLong (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiSeqFloat (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jfloatArray array;
    jsize arrLen;
    c_float **dst;
    saj_copyResult result;

    dst = (c_float **)dstSeq;
    sh = (sajCopySequence *)ch;
    array = (jfloatArray)objectArray;

    if (array == NULL) {
        (*dst) = (c_float *)c_sequenceNew (sh->type, sh->size, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        arrLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->size && (arrLen > (int)sh->size)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Float sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else if((*dst = (c_float *)c_sequenceNew (sh->type, sh->size, arrLen)) != NULL){
                SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_float, jfloat);
                (*(ctx->javaEnv))->GetFloatArrayRegion (ctx->javaEnv, array, 0, arrLen, *dst);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetFloatArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, arrLen, dst, saj_copyResultImage(result)));
                TRACE(
                if(result == SAJ_COPYRESULT_OK){
                    printf ("Copied in Float sequence size %d @ offset = %d\n", arrLen, ctx->offset);
                })
            } else {
                OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_sequenceNew<c_float> failed for length %d.", arrLen);
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSeqFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, sequence));
        result = saj_cfoiSeqFloat (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqFloatGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, sequence));
        result = saj_cfoiSeqFloat (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqFloatSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, sequence));
        result = saj_cfoiSeqFloat (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiSeqDouble (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jdoubleArray array;
    jsize arrLen;
    c_double **dst;
    saj_copyResult result;

    dst = (c_double **)dstSeq;
    sh = (sajCopySequence *)ch;
    array = (jdoubleArray)objectArray;

    if (array == NULL) {
        (*dst) = (c_double *)c_sequenceNew(sh->type, sh->size, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        arrLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->size && (arrLen > (int)sh->size)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Double sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else if((*dst = (c_double *)c_sequenceNew (sh->type, sh->size, arrLen)) != NULL){
                SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_double, jdouble);
                (*(ctx->javaEnv))->GetDoubleArrayRegion (ctx->javaEnv, array, 0, arrLen, *dst);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetDoubleArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, arrLen, dst, saj_copyResultImage(result)));
                TRACE(
                if(result == SAJ_COPYRESULT_OK){
                    printf ("Copied in Double sequence size %d @ offset = %d\n", arrLen, ctx->offset);
                })
            } else {
                OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_sequenceNew<c_double> failed for length %d.", arrLen);
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSeqDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, sequence));
        result = saj_cfoiSeqDouble (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqDoubleGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, sequence));
        result = saj_cfoiSeqDouble (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqDoubleSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    void *dst;
    jobjectArray sequence;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    sequence = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, sequence));
        result = saj_cfoiSeqDouble (ch, sequence, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

/* Structured types */
STATIC saj_copyResult
saj_cfoiStruct (
    sajCopyHeader *ch,
    jobject structObject,
    void *dstStruct,
    saj_context *ctx)
{
    saj_context context;
    unsigned long mi;
    sajCopyStruct *csh;
    sajCopyStructMember *csm;
    saj_copyResult result;

    if (structObject == NULL) {
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else {
        context.dst = dstStruct;
        context.base = ctx->base;
        context.javaEnv = ctx->javaEnv;
        csh = (sajCopyStruct *)ch;
        result = SAJ_COPYRESULT_OK;
        csm = sajCopyStructMemberObject (csh);

        for (mi = 0; (mi < csh->nrOfMembers) && (result == SAJ_COPYRESULT_OK); mi++) {
            context.offset = csm->memberOffset;
            ch = sajCopyStructMemberDescription (csm);
            assert(ch != NULL);
            result = ciFromStruct[ch->copyType] (ch, structObject, csm->javaFID, &context);
            if(result != SAJ_COPYRESULT_OK) {
                saj_reportCopyInFail(structObject, csm->javaFID, ctx);
            }
            csm = (sajCopyStructMember *)sajCopyHeaderNextObject (ch);
        }
        TRACE(printf ("Copied in Struct @ offset = %d. Result = %d\n", ctx->offset, result));
    }

    return result;
}

STATIC saj_copyResult
saj_cfsiStruct (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobject structObject;
    void *dst;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    structObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, structObject));
        result = saj_cfoiStruct (ch, structObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiStructGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    jobject structObject;
    void *dst;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    structObject = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, structObject));
        result = saj_cfoiStruct (ch, structObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiStructSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    jobject structObject;
    void *dst;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    structObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, structObject));
        result = saj_cfoiStruct (ch, structObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiUnion (
    sajCopyHeader *ch,
    jobject unionObject,
    void *dstUnion,
    saj_context *ctx)
{
    saj_context context;
    unsigned long ci;
    sajCopyUnion *cuh;
    sajCopyUnionLabels *csl;
    unsigned long long discrVal;
    sajCopyUnionLabels *defaultLabel = NULL;
    int active_case = 0;
    saj_copyResult result;

    cuh = (sajCopyUnion *)ch;
    switch (c_baseObject(cuh->discrType)->kind) {
    case M_ENUMERATION: {
        jint enumVal;
        jobject enumObject;
        c_long *dst;

        if(cuh->unionType == SAJ_SUPPORTED_UNION)
        {
            enumObject = (*(ctx->javaEnv))->GetObjectField(
                    ctx->javaEnv, unionObject, cuh->unionAccessors.supportedUnion.discrID);
        }
        else
        {
            enumObject = (*(ctx->javaEnv))->CallObjectMethod(
                    ctx->javaEnv, unionObject, cuh->unionAccessors.genericUnion.getDiscrMethodID);
        }
        result = saj_copyGetStatus(ctx);

        if (result == SAJ_COPYRESULT_OK) {
            sajCopyEnum *copyEnum = sajCopyUnionEnumDiscrObject(cuh);
            context.base = ctx->base;
            context.javaEnv = ctx->javaEnv;
            context.dst = dstUnion;
            context.offset = 0;
            result = saj_cfoiEnum((sajCopyHeader*) copyEnum, enumObject, &enumVal, &context);

            if (result == SAJ_COPYRESULT_OK) {
                discrVal = (unsigned long long)enumVal;
                dst = (c_long *)dstUnion;
                *dst = (c_long)enumVal;
            }
        }
    }
        break;
    case M_PRIMITIVE:
        switch (c_primitive (cuh->discrType)->kind) {
        case P_BOOLEAN: {
            jboolean d;
            c_bool *dst;

            if(cuh->unionType == SAJ_SUPPORTED_UNION)
            {
                d = (*(ctx->javaEnv))->GetBooleanField(ctx->javaEnv, unionObject, cuh->unionAccessors.supportedUnion.discrID);
            }
            else
            {
                d = (*(ctx->javaEnv))->CallBooleanMethod(ctx->javaEnv, unionObject, cuh->unionAccessors.genericUnion.getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: GetBooleanField (0x%x, %d) = %d\n", unionObject, cuh->unionAccessors.supportedUnion.discrID, d));
                discrVal = (unsigned long long)d;
                dst = (c_bool *)dstUnion;
                *dst = (c_bool)d;
            }
        }
            break;
        case P_CHAR: {
            jchar d;
            c_char *dst;

            if(cuh->unionType == SAJ_SUPPORTED_UNION)
            {
                d = (*(ctx->javaEnv))->GetCharField(ctx->javaEnv, unionObject, cuh->unionAccessors.supportedUnion.discrID);
            }
            else
            {
                d = (*(ctx->javaEnv))->CallCharMethod(ctx->javaEnv, unionObject, cuh->unionAccessors.genericUnion.getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: GetCharField (0x%x, %d) = %d\n", unionObject, cuh->unionAccessors.supportedUnion.discrID, d));

                discrVal = (unsigned long long)d;
                dst = (c_char *)dstUnion;
                *dst = (c_char)d;
            }
        }
            break;
        case P_SHORT:
        case P_USHORT: {
            jshort d;
            c_short *dst;

            if(cuh->unionType == SAJ_SUPPORTED_UNION)
            {
                d = (*(ctx->javaEnv))->GetShortField(ctx->javaEnv, unionObject, cuh->unionAccessors.supportedUnion.discrID);
            }
            else
            {
                d = (*(ctx->javaEnv))->CallShortMethod(ctx->javaEnv, unionObject, cuh->unionAccessors.genericUnion.getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: GetShortField (0x%x, %d) = %d\n", unionObject, cuh->unionAccessors.supportedUnion.discrID, d));
                discrVal = (unsigned long long)d;
                dst = (c_short *)dstUnion;
                *dst = (c_short)d;
            }
        }
            break;
        case P_LONG:
        case P_ULONG: {
            jint d;
            c_long *dst;

            if(cuh->unionType == SAJ_SUPPORTED_UNION)
            {
                d = (*(ctx->javaEnv))->GetIntField(ctx->javaEnv, unionObject, cuh->unionAccessors.supportedUnion.discrID);
            }
            else
            {
                d = (*(ctx->javaEnv))->CallIntMethod(ctx->javaEnv, unionObject, cuh->unionAccessors.genericUnion.getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: GetIntField (0x%x, %d) = %d\n", unionObject, cuh->unionAccessors.supportedUnion.discrID, d));

                discrVal = (unsigned long long)d;
                dst = (c_long *)dstUnion;
                *dst = (c_long)d;
            }
        }
            break;
        case P_LONGLONG:
        case P_ULONGLONG: {
            jlong d;
            c_longlong *dst;

            if(cuh->unionType == SAJ_SUPPORTED_UNION)
            {
                d = (*(ctx->javaEnv))->GetLongField(ctx->javaEnv, unionObject, cuh->unionAccessors.supportedUnion.discrID);
            }
            else
            {
                d = (*(ctx->javaEnv))->CallLongMethod(ctx->javaEnv, unionObject, cuh->unionAccessors.genericUnion.getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI:  (0x%x, %d) = %lld\n", unionObject, cuh->unionAccessors.supportedUnion.discrID, d));
                discrVal = (unsigned long long)d;
                dst = (c_longlong *)dstUnion;
                *dst = (c_longlong)d;
            }
        }
            break;
        default:
            result = SAJ_COPYRESULT_ERROR;
            OS_REPORT (OS_ERROR, "dcpssaj", 0,
                    "saj_cfoiUnion: Unexpected primitive kind\n");
            assert (0);
        }
        break;
    default:
        result = SAJ_COPYRESULT_ERROR;
        OS_REPORT (OS_ERROR, "dcpssaj", 0, "saj_cfoiUnion: Unexpected switch kind\n");
        assert (0);
    }


    if(result == SAJ_COPYRESULT_OK){
        dstUnion = (void *)((PA_ADDRCAST)dstUnion + cuh->casesOffset);
        context.base = ctx->base;
        context.javaEnv = ctx->javaEnv;
        context.dst = dstUnion;
        context.offset = 0;
        csl = sajCopyUnionLabelsObject (cuh);
        ci = 0;

        while ((ci < cuh->nrOfCases) && (result == SAJ_COPYRESULT_OK)) {
            unsigned int label;
            sajCopyUnionLabel *lab;

            lab = sajCopyUnionLabelObject (csl);
            if (csl->labelCount) {
                for (label = 0; label < csl->labelCount; label++) {
                    if (lab->labelVal == discrVal) {
                        active_case = 1;
                    }
                    lab++;
                }
            } else {
                defaultLabel = csl;
            }
            if (cuh->unionType == SAJ_GENERIC_UNION)
            {
                sajCopyGenericUnionCase *unionCase = (sajCopyGenericUnionCase *)lab;
                ch = (sajCopyHeader *)sajCopyGenericUnionCaseDescription (unionCase);

                if (active_case) {
                    result = ciFromGenericUnion[ch->copyType](ch, unionObject, unionCase->getterID, &context);
                    ci = cuh->nrOfCases; /* break out of for-loop. */
                } else {
                    ci++;
                }
                csl = (sajCopyUnionLabels *)sajCopyHeaderNextObject (ch);
            }
            else
            {
                sajCopySupportedUnionCase *unionCase = (sajCopySupportedUnionCase *)lab;
                ch = (sajCopyHeader *)sajCopySupportedUnionCaseDescription (unionCase);

                if (active_case) {
                    result = ciFromSupportedUnion[ch->copyType](ch, unionObject, unionCase->caseID, &context);
                    ci = cuh->nrOfCases; /* break out of for-loop. */
                } else {
                    ci++;
                }
                csl = (sajCopyUnionLabels *)sajCopyHeaderNextObject (ch);
            }
        }

        if (!active_case && defaultLabel && (result == SAJ_COPYRESULT_OK)) {
            if (cuh->unionType == SAJ_GENERIC_UNION)
            {
                sajCopyGenericUnionCase *unionCase = (sajCopyGenericUnionCase *)sajCopyUnionLabelObject (defaultLabel);
                ch = sajCopyGenericUnionCaseDescription (unionCase);
                result = ciFromGenericUnion[ch->copyType](ch, unionObject, unionCase->getterID, &context);
            }
            else
            {
                sajCopySupportedUnionCase *unionCase = (sajCopySupportedUnionCase *)sajCopyUnionLabelObject (defaultLabel);
                ch = sajCopySupportedUnionCaseDescription (unionCase);
                result = ciFromSupportedUnion[ch->copyType](ch, unionObject, unionCase->caseID, &context);
            }
        }
        TRACE(printf ("Copied in Union\n"));
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiUnion (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobject unionObject;
    void *dst;
    saj_copyResult result;

    dst = (c_string *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    unionObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, unionObject));
        result = saj_cfoiUnion (ch, unionObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiUnionGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    jobject unionObject;
    void *dst;
    saj_copyResult result;

    dst = (c_string *)ctx->dst;
    unionObject = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, unionObject));
        result = saj_cfoiUnion (ch, unionObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiUnionSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    jobject unionObject;
    void *dst;
    saj_copyResult result;

    dst = (c_string *)ctx->dst;
    unionObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, unionObject));
        result = saj_cfoiUnion (ch, unionObject, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

    /* String types */
STATIC saj_copyResult
saj_cfoiString (
    sajCopyHeader *ch,
    jobject stringObject,
    void *dstString,
    saj_context *ctx)
{
    c_string *dst;
    const char *strNative;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    dst = (c_string *)(dstString);

    if(stringObject){
        strNative = (*(ctx->javaEnv))->GetStringUTFChars (ctx->javaEnv, (jstring)stringObject, 0);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            TRACE(printf ("JNI: GetStringUTFChars (0x%x) = 0x%x\n", stringObject, strNative));
            *dst = c_stringNew (ctx->base, (c_char *)strNative);
            (*(ctx->javaEnv))->ReleaseStringUTFChars (ctx->javaEnv, (jstring)stringObject, strNative);
            result = saj_copyGetStatus(ctx);

            TRACE(printf ("JNI: ReleaseStringUTFChars (0x%x, 0x%x)\n", stringObject, strNative));
            TRACE(printf ("Copied in string = %s @ offset = %d\n",  *dst, ctx->offset));
        }
    } else {
        result = SAJ_COPYRESULT_BAD_PARAMETER;
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "String bounds violation (null reference)");
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiString (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jstring str;
    c_string *dst;
    saj_copyResult result;

    dst = (c_string *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    str = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, str));
        result = saj_cfoiString (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiStringGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    jstring str;
    c_string *dst;
    saj_copyResult result;

    dst = (c_string *)ctx->dst;
    str = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, str));
        result = saj_cfoiString (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiStringSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    jstring str;
    c_string *dst;
    saj_copyResult result;

    dst = (c_string *)ctx->dst;
    str = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, str));
        result = saj_cfoiString (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiBString (
    sajCopyHeader *ch,
    jobject stringObject,
    void *dstString,
    saj_context *ctx)
{
    c_string *dst;
    const char *strNative;
    saj_copyResult result;
    sajCopyBoundedString* cbs;
    size_t length;

    dst = (c_string *)(dstString);

    if(stringObject){
        strNative = (*(ctx->javaEnv))->GetStringUTFChars (ctx->javaEnv, (jstring)stringObject, 0);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetStringUTFChars (0x%x) = 0x%x -> %s\n", stringObject, strNative, saj_copyResultImage(result)));

        if(result == SAJ_COPYRESULT_OK){
            cbs = (sajCopyBoundedString*)ch;
            length = strlen(strNative);

            if(length <= cbs->max){
                if((*dst = c_stringMalloc (ctx->base, length + 1)) != NULL){
                    if(length){
                        /* strNative is guaranteed to be nul-terminated, so copy
                         * the nul-character in this step as well. */
                        memcpy(*dst, strNative, length + 1);
                    }
                    /* c_stringMalloc(1) should return a nul-terminated string as well */
                    assert((*dst)[length] == '\0');
                } else {
                    result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
                    OS_REPORT_1(OS_ERROR, "dcpssaj", 0, "Out of resources; c_stringMalloc failed for length %d.", length);
                }
                (*(ctx->javaEnv))->ReleaseStringUTFChars (ctx->javaEnv, (jstring)stringObject, strNative);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: ReleaseStringUTFChars (0x%x) = 0x%x -> %s\n", stringObject, strNative, saj_resultImage(result)));
                TRACE(printf ("Copied in string (len=%d) = %s @ offset = %d\n", length, *dst, ctx->offset));
            } else {
                result = SAJ_COPYRESULT_BAD_PARAMETER;
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "String bounds violation; length > maximum.");
            }
        }
    } else {
        result = SAJ_COPYRESULT_BAD_PARAMETER;
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "String bounds violation (null reference)");
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiBStringToArrChar (
    sajCopyHeader *ch,
    jobject stringObject,
    void *dstArray,
    saj_context *ctx)
{
    c_char *dst;
    const char *strNative;
    saj_copyResult result;
    sajCopyBoundedString* cbs;
    size_t length;

    dst = (c_char *)(dstArray);
    if(stringObject){
        strNative = (*(ctx->javaEnv))->GetStringUTFChars (ctx->javaEnv, (jstring)stringObject, 0);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            cbs = (sajCopyBoundedString*)ch;
            length = strlen(strNative);
            /* length must be smaller then max because the array internally is 1
             * bigger then the string bounds to accomodate the '\0' char */
            if(length < cbs->max){
                TRACE(printf ("JNI: GetStringUTFChars (0x%x) = 0x%x\n", stringObject, strNative));
                memcpy(dst, strNative, length + 1);
                assert(dst[length] == '\0');
                (*(ctx->javaEnv))->ReleaseStringUTFChars (ctx->javaEnv, (jstring)stringObject, strNative);
                result = saj_copyGetStatus(ctx);

                TRACE(printf ("JNI: ReleaseStringUTFChars (0x%x) = 0x%x\n", stringObject, strNative));
                TRACE(printf ("Copied in string = %s @ offset = %d\n", *dst, ctx->offset));
            } else {
                result = SAJ_COPYRESULT_BAD_PARAMETER;
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "String bounds violation; length > maximum.");
            }
        }
    } else {
        result = SAJ_COPYRESULT_BAD_PARAMETER;
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "String bounds violation (null reference)");
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiBString (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jstring str;
    c_string *dst;
    saj_copyResult result;

    dst = (c_string *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    str = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, str));
        result = saj_cfoiBString (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiBStringToArrChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jstring str;
    void *dst;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    str = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, str));
        result = saj_cfoiBStringToArrChar (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArrCharToBString(
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobjectArray array;
    c_string *dst;
    saj_copyResult result;

    dst = (c_string *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK)
    {
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArrCharToBString (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfoiArrCharToBString (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstString,
    saj_context *ctx)
{
    saj_copyResult result;
    jcharArray array;
    sajCopyArray *ah;
    jchar *charArray;
    c_string *dst;

    ah = (sajCopyArray *)ch;
    dst = (c_string *)(dstString);
    array = (jcharArray)objectArray;
    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if (ah->size == 0){
         *dst = c_stringMalloc(ctx->base, ah->size + 1);
         assert(*dst); /* Intern is returned, so will not fail on out of resources */
         result = SAJ_COPYRESULT_OK;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        /* Access to the char-array is obtained in two ways. When the size of
         * an array is below a certain threshold, access is gained by means
         * of GetPrimitiveArrayCritical, which is very likely to not perform
         * an intermediate copy. In the other case, the array is copied into
         * an intermediate buffer on heap. */
        if(ah->size <= SAJ_CHAR_ARRAY_CRITICAL_HEURISTIC){
            jboolean isCopy;

            charArray = (*(ctx->javaEnv))->GetPrimitiveArrayCritical (ctx->javaEnv, array, &isCopy);
            TRACE(printf ("JNI: GetPrimitiveArrayCritical (%p, %p, isCopy=%s)\n", ctx->javaEnv, array, isCopy ? "yes" : "no"));
        } else {
            if((charArray = os_malloc (sizeof (jchar) * ah->size)) != NULL){
                (*(ctx->javaEnv))->GetCharArrayRegion (ctx->javaEnv, array, 0, ah->size, charArray);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: GetCharArrayRegion (0x%x, %d, %d, 0x%x) -> %s\n", array, 0, ah->size, charArray, saj_copyResultImage(result)));
            } else {
                result = SAJ_COPYRESULT_ERROR; /* Out of (heap) resources */
            }
        }

        if(result == SAJ_COPYRESULT_OK){
            /* Allocate the length of the array (and null terminator) as a
             * database string */
            if((*dst = c_stringMalloc(ctx->base, (ah->size + 1))) != NULL) {
#ifndef OSPL_SAJ_NO_LOOP_UNROLLING
                SAJ_LOOP_UNROLL(ah->size, jchar, charArray, c_char, *dst);
#else /* OSPL_SAJ_NO_LOOP_UNROLLING */
                unsigned int i;
                for(i = 0; i < ah->size; i++){
                    (*dst)[i] = (c_char)charArray[i];
                    TRACE(printf ("%d @ %p;", (*dst)[i], &(*dst)[i]));
                }
                assert(i == ah->size);
#endif /* OSPL_SAJ_NO_LOOP_UNROLLING */
                (*dst)[ah->size] = '\0';
                TRACE(printf ("Copied in Char array size %d @ offset = %d\n", ah->size, ctx->offset));
            } else {
                result = SAJ_COPYRESULT_ERROR; /* Out of (database) resources */
            }

            if(ah->size <= SAJ_CHAR_ARRAY_CRITICAL_HEURISTIC){
                /* Since we only read data, release the array with JNI_ABORT */
                (*(ctx->javaEnv))->ReleasePrimitiveArrayCritical (ctx->javaEnv, array, charArray, JNI_ABORT);
                /* An eventual error shouldn't be overwritten by a succesful
                 * release of resources. As long as there is no copyresult for
                 * out-of-resources, dont'r return result of saj_copyGetStatus. */
                saj_copyGetStatus(ctx); /* For logging */
                TRACE(printf ("JNI: ReleasePrimitiveArrayCritical (%p, %p, %p, JNI_ABORT)\n", ctx->javaEnv, ctx->javaEnv, charArray));
            } else {
                os_free(charArray);
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiBStringGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    jstring str;
    c_string *dst;
    saj_copyResult result;

    dst = (c_string *)ctx->dst;
    str = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, str));
        result = saj_cfoiBString (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiBStringSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    jstring str;
    c_string *dst;
    saj_copyResult result;

    dst = (c_string *)ctx->dst;
    str = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, str));
        result = saj_cfoiBString (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiBStringToArrCharGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    jstring str;
    void* dst;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    str = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, str));
        result = saj_cfoiBStringToArrChar (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiBStringToArrCharSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    jstring str;
    void* dst;
    saj_copyResult result;

    dst = (void *)ctx->dst;
    str = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, str));
        saj_cfoiBStringToArrChar (ch, str, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

    /* Array of object type */
STATIC saj_copyResult
saj_cfoiArray (
    sajCopyHeader *ch,
    jobject objectArray,
    void *dstArray,
    saj_context *ctx)
{
    sajCopyObjectArray *ah;
    sajCopyHeader *aech;
    jobjectArray array;
    jobject element;
    void *dst;
    unsigned int i;
    saj_copyResult result;

    dst = dstArray;
    ah = (sajCopyObjectArray *)ch;
    aech = sajCopyObjectArrayDescription (ah);
    array = (jobjectArray)objectArray;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Object array bounds violation(null reference)");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->arraySize) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Object array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if((result = saj_copyGetStatus(ctx)) == SAJ_COPYRESULT_OK){
        for (i = 0; (i < ah->arraySize) && (result == SAJ_COPYRESULT_OK); i++) {
            element = (*(ctx->javaEnv))->GetObjectArrayElement (ctx->javaEnv, array, i);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: GetObjectArrayElement (0x%x, %d) = 0x%x\n", array, i, element));

            if(result == SAJ_COPYRESULT_OK){
                result = ciFromArray[aech->copyType] (aech, element, dst, ctx);
                dst = (void *)((PA_ADDRCAST)dst + ah->typeSize);
            }
        }
        TRACE(printf ("Copied in Object array size %d @ offset = %d\n", ah->typeSize, ctx->offset));
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiArray (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobjectArray array;
    void *dst;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiArray (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrayGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    jobjectArray array;
    void *dst;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiArray (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArraySupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    jobjectArray array;
    void *dst;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiArray (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

    /* Sequence of object type */
STATIC saj_copyResult
saj_cfoiSequence (
    sajCopyHeader *ch,
    jobject objectSequence,
    void *dstSeq,
    saj_context *ctx)
{
    sajCopyObjectSequence *sh;
    sajCopyHeader *sech;
    jobjectArray array;
    jobject element;
    void *dst;
    c_long i;
    c_long seqLen;
    saj_copyResult result;

    sh = (sajCopyObjectSequence *)ch;
    sech = sajCopyObjectSequenceDescription (sh);
    array = (jobjectArray)objectSequence;

    if (array == NULL) {
        *(c_sequence *)dstSeq = c_sequenceNew (sh->type, sh->seqSize, 0);
        result = SAJ_COPYRESULT_OK;
    } else {
        seqLen = (*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            if (sh->seqSize && (seqLen > (int)sh->seqSize)) {
                OS_REPORT(OS_ERROR, "dcpssaj", 0, "Object sequence bounds violation.");
                result = SAJ_COPYRESULT_BAD_PARAMETER;
            } else {
                TRACE(printf ("JNI: GetArrayLength (0x%x) = %d\n", array, seqLen));
                *(c_sequence *)dstSeq = c_sequenceNew (sh->type, sh->seqSize, seqLen);
                dst = (void *)*(c_sequence *)dstSeq;

                for (i = 0; (i < seqLen) && (result ==SAJ_COPYRESULT_OK); i++) {
                    element = (*(ctx->javaEnv))->GetObjectArrayElement (ctx->javaEnv, array, i);
                    result = saj_copyGetStatus(ctx);
                    TRACE(printf ("JNI: GetObjectArrayElement (0x%x, %d) = 0x%x\n", array, i, element));

                    if(result == SAJ_COPYRESULT_OK){
                        result = ciFromArray[sech->copyType] (sech, element, dst, ctx);
                        dst = (void *)((PA_ADDRCAST)dst + sh->typeSize);
                    }
                }
                TRACE(printf ("Copied in Object sequence size %d @ offset = %d\n", sh->typeSize, ctx->offset));
            }
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsiSequence (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobjectArray array;
    void *dst;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfoiSequence (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, javaFID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSequenceGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    jobjectArray array;
    void *dst;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->CallObjectMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: CallObjectMethod (0x%x, %d) = 0x%x\n", javaObject, getterID, array));
        result = saj_cfoiSequence (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail_generic(javaObject, getterID, ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSequenceSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    jobjectArray array;
    void *dst;
    saj_copyResult result;

    dst = (void *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, caseID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, caseID, array));
        result = saj_cfoiSequence (ch, array, dst, ctx);
        if(result != SAJ_COPYRESULT_OK) {
            saj_reportCopyInFail(javaObject, caseID, ctx);
        }
    }
    return result;
}

    /* backward referenced type */
STATIC saj_copyResult
saj_cfoiReference (
    sajCopyHeader *ch,
    jobject object,
    void *dst,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);

    return ciFromArray[nch->copyType] (nch, object, dst, ctx);
}

STATIC saj_copyResult
saj_cfsiReference (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);

    return ciFromStruct[nch->copyType] (nch, javaObject, javaFID, ctx);
}

STATIC saj_copyResult
saj_cfuiReferenceGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);
    return ciFromGenericUnion[nch->copyType] (nch, javaObject, getterID, ctx);
}

STATIC saj_copyResult
saj_cfuiReferenceSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID caseID,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);
    return ciFromSupportedUnion[nch->copyType] (nch, javaObject, caseID, ctx);
}

c_bool
saj_copyInStruct (
    c_base base,
    void *src,
    void *dst)
{
    saj_context context;
    saj_srcInfo srcInfo = (saj_srcInfo)src;
    sajCopyHeader *ch;
    saj_copyResult result;
    c_bool cresult;

    ch = saj_copyCacheCache(srcInfo->copyProgram);
    context.dst = dst;
    context.offset = 0;
    context.base = base;
    context.javaEnv = srcInfo->javaEnv;
    result = ciFromArray[ch->copyType] (ch, srcInfo->javaObject, dst, &context);
    TRACE(printf ("Result = %d\n", result));

    if(result == SAJ_COPYRESULT_OK){
        cresult = TRUE;
    } else {
        ((*(srcInfo->javaEnv))->ExceptionClear (srcInfo->javaEnv));
        cresult = FALSE;
    }
    return cresult;
}
