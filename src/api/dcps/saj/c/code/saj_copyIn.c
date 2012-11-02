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

typedef saj_copyResult (*copyInFromStruct)(sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
typedef saj_copyResult (*copyInFromUnion)(sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
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
STATIC saj_copyResult saj_cfuiBoolean    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiByte       (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiChar       (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiShort      (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiInt        (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiLong       (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiFloat      (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiDouble     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Array of primitive type */
STATIC saj_copyResult saj_cfuiArrBoolean (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrByte    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrChar    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrCharToBString    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrShort   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrInt     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrLong    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrFloat   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiArrDouble  (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfuiSeqBoolean (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqByte    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqChar    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqShort   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqInt     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqLong    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqFloat   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiSeqDouble  (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfuiEnum       (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfuiStruct     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiUnion      (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfuiString     (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiBString    (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
STATIC saj_copyResult saj_cfuiBStringToArrChar
                                         (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfuiArray      (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfuiSequence   (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);
    /* reference to previous defined type */
STATIC saj_copyResult saj_cfuiReference  (sajCopyHeader *ch, jobject javaObject, jmethodID getterID, saj_context *ctx);

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

STATIC copyInFromUnion ciFromUnion[] = {
    saj_cfuiBoolean,
    saj_cfuiByte,
    saj_cfuiChar,
    saj_cfuiShort,
    saj_cfuiInt,
    saj_cfuiLong,
    saj_cfuiFloat,
    saj_cfuiDouble,
    saj_cfuiArrBoolean,
    saj_cfuiArrByte,
    saj_cfuiArrChar,
    saj_cfuiArrCharToBString,
    saj_cfuiArrShort,
    saj_cfuiArrInt,
    saj_cfuiArrLong,
    saj_cfuiArrFloat,
    saj_cfuiArrDouble,
    saj_cfuiSeqBoolean,
    saj_cfuiSeqByte,
    saj_cfuiSeqChar,
    saj_cfuiSeqShort,
    saj_cfuiSeqInt,
    saj_cfuiSeqLong,
    saj_cfuiSeqFloat,
    saj_cfuiSeqDouble,
    saj_cfuiEnum,
    saj_cfuiStruct,
    saj_cfuiUnion,
    saj_cfuiString,
    saj_cfuiBString,
    saj_cfuiBStringToArrChar,
    saj_cfuiArray,
    saj_cfuiSequence,
    saj_cfuiReference
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

    dst = (c_bool *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetBooleanField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetBooleanField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Boolean = %d @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_bool *dst;

    dst = (c_bool *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallBooleanMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallBooleanMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Boolean = %d getterID = %x\n", *dst, (int)getterID));

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

    dst = (c_octet *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetByteField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetByteField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Byte = %hd @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiByte (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_octet *dst;

    dst = (c_octet *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallByteMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallByteMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Byte = %d getterID = %x\n", *dst, (int)getterID));

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

    dst = (c_char *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (c_char)(*(ctx->javaEnv))->GetCharField(ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetCharField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Char = %hd @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_char *dst;

    dst = (c_char *)ctx->dst;
    *dst = (c_char)(*(ctx->javaEnv))->CallCharMethod(ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallCharMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Char = %d getterID = %x\n", *dst, (int)getterID));

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

    dst = (c_short *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetShortField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetShortField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Short = %hd @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiShort (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_short *dst;

    dst = (c_short *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallShortMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallShortMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Short = %d getterID = %x\n", *dst, (int)getterID));

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

    dst = (c_long *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetIntField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetIntField (0x%x, %d) = %d\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Int = %d @ offset = %d FID = %x Result = %d\n", *dst, ctx->offset, (int)javaFID, result));

    return result;
}

STATIC saj_copyResult
saj_cfuiInt (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_long *dst;

    dst = (c_long *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallIntMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallIntMethod (0x%x, %d) = %d\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Int = %d getterID = %x\n", *dst, (int)getterID));

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

    dst = (c_longlong *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetLongField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetLongField (0x%x, %d) = %lld\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Long = %lld @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiLong (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_longlong *dst;

    dst = (c_longlong *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallLongMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallLongMethod (0x%x, %d) = %lld\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Long = %lld getterID = %x\n", *dst, (int)getterID));

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

    dst = (c_float *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetFloatField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetFloatField (0x%x, %d) = %f\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Float = %f @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuiFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_float *dst;

    dst = (c_float *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallFloatMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallFloatMethod (0x%x, %d) = %lld\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Float = %f getterID = %x\n", *dst, (int)getterID));

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

    dst = (c_double *)((PA_ADDRCAST)ctx->dst + ctx->offset);
    *dst = (*(ctx->javaEnv))->GetDoubleField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetDoubleField (0x%x, %d) = %f\n", javaObject, javaFID, *dst));
    TRACE(printf ("Copied in Double = %f @ offset = %d FID = %x\n", *dst, ctx->offset, (int)javaFID));

    return result;

}

STATIC saj_copyResult
saj_cfuiDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_double *dst;

    dst = (c_double *)ctx->dst;
    *dst = (*(ctx->javaEnv))->CallDoubleMethod (ctx->javaEnv, javaObject, getterID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallDoubleMethod (0x%x, %d) = %lld\n", javaObject, getterID, *dst));
    TRACE(printf ("Copied in Double = %f getterID = %x\n", *dst, (int)getterID));

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
    *dst = (*(ctx->javaEnv))->CallIntMethod (ctx->javaEnv, enumObject, copyEnum->valueID);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiEnum (
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
    jboolean *booleanArray;
    c_bool *dst;
    unsigned int i;

    array = (jbooleanArray)objectArray;
    dst = (c_bool *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Boolean array bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Boolean array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            booleanArray = os_alloca (sizeof (jboolean) * ah->size);
            (*(ctx->javaEnv))->GetBooleanArrayRegion (ctx->javaEnv, array, 0, ah->size, booleanArray);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetBooleanArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, booleanArray));

                for (i = 0; i < ah->size; i++) {
                    dst[i] = booleanArray[i];
                    TRACE(printf ("%d @ 0x%x;", dst[i], (unsigned int)&dst[i]));
                }
                TRACE(printf ("Copied in Boolean array size %d @ offset = %d\n",
                                                        ah->size, ctx->offset));
            }
            os_freea (booleanArray);
        }
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrBoolean (
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
    jbyte *byteArray;
    c_octet *dst;
    unsigned int i;

    array = (jbyteArray)objectArray;
    dst = (c_octet *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
    	OS_REPORT(OS_ERROR, "dcpssaj", 0, "Byte array bounds violation (null reference).");
    	result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else  if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "Byte array bounds violation.");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            byteArray = os_alloca (sizeof (jbyte) * ah->size);
            (*(ctx->javaEnv))->GetByteArrayRegion (ctx->javaEnv, array, 0, ah->size, byteArray);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetByteArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, byteArray));

                for (i = 0; i < ah->size; i++) {
                    dst[i] = byteArray[i];
                    TRACE(printf ("%d @ 0x%x;", dst[i], (unsigned int)&dst[i]));
                }
                TRACE(printf ("Copied in Byte array size %d @ offset = %d\n", ah->size, ctx->offset));
            }
            os_freea (byteArray);
        }
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrByte (
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
    c_char *dst;
    unsigned int i;

    array = (jcharArray)objectArray;
    dst = (c_char *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
    	OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char array bounds violation (null reference).");
    	result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
    	OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char array bounds violation.");
    	result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            charArray = os_alloca (sizeof (jchar) * ah->size);
            (*(ctx->javaEnv))->GetCharArrayRegion (ctx->javaEnv, array, 0, ah->size, charArray);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, charArray));

                for (i = 0; i < ah->size; i++) {
                    dst[i] = (c_char)charArray[i];
                    TRACE(printf ("%d @ 0x%x;", dst[i], (unsigned int)&dst[i]));
                }
                TRACE(printf ("Copied in Char array size %d @ offset = %d\n", ah->size, ctx->offset));
            }
            os_freea (charArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrChar (
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrCharToBString (
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
    jshort *shortArray;
    c_short *dst;
    unsigned int i;

    array = (jshortArray)objectArray;
    dst = (c_short *)dstArray;
    ah = (sajCopyArray *)ch;

    if (array == NULL) {
    	OS_REPORT(OS_ERROR, "dcpssaj", 0, "Short array bounds violation (null reference).");
    	result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
    	OS_REPORT(OS_ERROR, "dcpssaj", 0, "Short array bounds violation.");
    	result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            shortArray = os_alloca (sizeof (jshort) * ah->size);
            (*(ctx->javaEnv))->GetShortArrayRegion (ctx->javaEnv, array, 0, ah->size, shortArray);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetShortArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, shortArray));

                for (i = 0; i < ah->size; i++) {
                	dst[i] = shortArray[i];
                	TRACE(printf ("%d @ 0x%x;", dst[i], (unsigned int)&dst[i]));
                }
                TRACE(printf ("Copied in Short array size %d @ offset = %d\n", ah->size, ctx->offset));
            }
            os_freea (shortArray);
        }
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrShort (
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
    jint *intArray;
    c_long *dst;
    unsigned int i;
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
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            intArray = os_alloca (sizeof (jint) * ah->size);
            (*(ctx->javaEnv))->GetIntArrayRegion (ctx->javaEnv, array, 0, ah->size, intArray);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetIntArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, intArray));

                for (i = 0; i < ah->size; i++) {
                	dst[i] = intArray[i];
                	TRACE(printf ("%d @ 0x%x;", dst[i], (unsigned int)&dst[i]));
                }
                TRACE(printf ("Copied in Int array size %d @ offset = %d\n", ah->size, ctx->offset));
            }
            os_freea (intArray);
        }
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrInt (
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
    jlong *longArray;
    c_longlong *dst;
    unsigned int i;
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
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            longArray = os_alloca (sizeof (jlong) * ah->size);
            (*(ctx->javaEnv))->GetLongArrayRegion (ctx->javaEnv, array, 0, ah->size, longArray);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetLongArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, longArray));

                for (i = 0; i < ah->size; i++) {
                	dst[i] = longArray[i];
                	TRACE(printf ("%lld @ 0x%x;", dst[i], (unsigned int)&dst[i]));
                }
                TRACE(printf ("Copied in Long array size %d @ offset = %d\n", ah->size, ctx->offset));
            }
            os_freea (longArray);
        }
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrLong (
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
    jfloat *floatArray;
    c_float *dst;
    unsigned int i;
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
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            floatArray = os_alloca (sizeof (jfloat) * ah->size);
            (*(ctx->javaEnv))->GetFloatArrayRegion (ctx->javaEnv, array, 0, ah->size, floatArray);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetFloatArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, floatArray));

                for (i = 0; i < ah->size; i++) {
                	dst[i] = floatArray[i];
                	TRACE(printf ("%f @ 0x%x;", dst[i], (unsigned int)&dst[i]));
                }
                TRACE(printf ("Copied in Float array size %d @ offset = %d\n", ah->size, ctx->offset));
            }
            os_freea (floatArray);
        }
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrFloat (
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
    jdouble *doubleArray;
    c_double *dst;
    unsigned int i;
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
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            doubleArray = os_alloca (sizeof (jdouble) * ah->size);
            (*(ctx->javaEnv))->GetDoubleArrayRegion (ctx->javaEnv, array, 0, ah->size, doubleArray);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetDoubleArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, doubleArray));

                for (i = 0; i < ah->size; i++) {
                    dst[i] = doubleArray[i];
            	    TRACE(printf ("%f @ 0x%x;", dst[i], (unsigned int)&dst[i]));
                }
                TRACE(printf ("Copied in Double array size %d @ offset = %d\n", ah->size, ctx->offset));
            }
            os_freea (doubleArray);
        }
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArrDouble (
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
    jboolean *booleanArray;
    jsize arrLen;
    c_bool **dst;
    int i;
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
            } else {
                *dst = (c_bool *)c_sequenceNew (sh->type, sh->size, arrLen);
                booleanArray = os_alloca (sizeof (jboolean) * arrLen);
                (*(ctx->javaEnv))->GetBooleanArrayRegion (ctx->javaEnv, array, 0, arrLen, booleanArray);
                result = saj_copyGetStatus(ctx);

                if(result == SAJ_COPYRESULT_OK){
                    TRACE(printf ("JNI: GetBooleanArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, arrLen, booleanArray));

                    for (i = 0; i < arrLen; i++) {
                	    (*dst)[i] = booleanArray[i];
                	    TRACE(printf ("%d;", (*dst)[i]));
                    }
                    TRACE(printf ("Copied in Boolean sequence size %d @ offset = %d\n", arrLen, ctx->offset));
                }
                os_freea (booleanArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqBoolean (
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
    jbyte *byteArray;
    jsize arrLen;
    c_octet **dst;
    int i;
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
            } else {
                *dst = (c_octet *)c_sequenceNew (sh->type, sh->size, arrLen);
                byteArray = os_alloca (sizeof (jbyte) * arrLen);
                (*(ctx->javaEnv))->GetByteArrayRegion (ctx->javaEnv, array, 0, arrLen, byteArray);
                result = saj_copyGetStatus(ctx);

                if(result == SAJ_COPYRESULT_OK){
                    TRACE(printf ("JNI: GetByteArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, arrLen, byteArray));

                    for (i = 0; i < arrLen; i++) {
                	    (*dst)[i] = byteArray[i];
                	    TRACE(printf ("%d;", (*dst)[i]));
                    }
                    TRACE(printf ("Copied in Byte sequence size %d @ offset = %d\n", arrLen, ctx->offset));
                }
                os_freea (byteArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqByte (
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
    int i;
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
            } else {
                *dst = (c_char *)c_sequenceNew (sh->type, sh->size, arrLen);
                charArray = os_alloca (sizeof (jchar) * arrLen);
                (*(ctx->javaEnv))->GetCharArrayRegion (ctx->javaEnv, array, 0, arrLen, charArray);
                result = saj_copyGetStatus(ctx);

                if(result == SAJ_COPYRESULT_OK){
                    TRACE(printf ("JNI: GetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, arrLen, charArray));

                    for (i = 0; i < arrLen; i++) {
                        (*dst)[i] = (c_char)charArray[i];
                        TRACE(printf ("%d;", (*dst)[i]));
                    }
                    TRACE(printf ("Copied in Char sequence size %d @ offset = %d\n", arrLen, ctx->offset));
                }
                os_freea (charArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqChar (
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
    jshort *shortArray;
    jsize arrLen;
    c_short **dst;
    int i;
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
            } else {
                *dst = (c_short *)c_sequenceNew (sh->type, sh->size, arrLen);
                shortArray = os_alloca (sizeof (jshort) * arrLen);
                (*(ctx->javaEnv))->GetShortArrayRegion (ctx->javaEnv, array, 0, arrLen, shortArray);
                result = saj_copyGetStatus(ctx);

                if(result == SAJ_COPYRESULT_OK){
                    TRACE(printf ("JNI: GetShortArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, arrLen, shortArray));

                    for (i = 0; i < arrLen; i++) {
                	    (*dst)[i] = shortArray[i];
                	    TRACE(printf ("%d;", (*dst)[i]));
                    }
                    TRACE(printf ("Copied in Short sequence size %d @ offset = %d\n", arrLen, ctx->offset));
                }
                os_freea (shortArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqShort (
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
    jint *intArray;
    jsize arrLen;
    c_long **dst;
    int i;
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
            } else {
                *dst = (c_long *)c_sequenceNew (sh->type, sh->size, arrLen);
                intArray = os_alloca (sizeof (jint) * arrLen);
                (*(ctx->javaEnv))->GetIntArrayRegion (ctx->javaEnv, array, 0, arrLen, intArray);
                result = saj_copyGetStatus(ctx);

                if(result == SAJ_COPYRESULT_OK){
                    TRACE(printf ("JNI: GetIntArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, arrLen, intArray));

                    for (i = 0; i < arrLen; i++) {
                	    (*dst)[i] = intArray[i];
                	    TRACE(printf ("%d;", (*dst)[i]));
                    }
                    TRACE(printf ("Copied in Int sequence size %d @ offset = %d\n", arrLen, ctx->offset));
                }
                os_freea (intArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqInt (
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
    jlong *longArray;
    jsize arrLen;
    c_longlong **dst;
    int i;
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
            } else {
                *dst = (c_longlong *)c_sequenceNew (sh->type, sh->size, arrLen);
                longArray = os_alloca (sizeof (jlong) * arrLen);
                (*(ctx->javaEnv))->GetLongArrayRegion (ctx->javaEnv, array, 0, arrLen, longArray);
                result = saj_copyGetStatus(ctx);

                if(result == SAJ_COPYRESULT_OK){
                    TRACE(printf ("JNI: GetLongArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, arrLen, longArray));

                    for (i = 0; i < arrLen; i++) {
                	    (*dst)[i] = longArray[i];
                	    TRACE(printf ("%lld;", (*dst)[i]));
                    }
                    TRACE(printf ("Copied in Long sequence size %d @ offset = %d\n", arrLen, ctx->offset));
                }
                os_freea (longArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqLong (
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
    jfloat *floatArray;
    jsize arrLen;
    c_float **dst;
    int i;
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
            } else {
                *dst = (c_float *)c_sequenceNew (sh->type, sh->size, arrLen);
                floatArray = os_alloca (sizeof (jfloat) * arrLen);
                (*(ctx->javaEnv))->GetFloatArrayRegion (ctx->javaEnv, array, 0, arrLen, floatArray);
                result = saj_copyGetStatus(ctx);

                if(result == SAJ_COPYRESULT_OK){
                    TRACE(printf ("JNI: GetFloatArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, arrLen, floatArray));

                    for (i = 0; i < arrLen; i++) {
                	    (*dst)[i] = floatArray[i];
                	    TRACE(printf ("%f;", (*dst)[i]));
                    }
                    TRACE(printf ("Copied in Float sequence size %d @ offset = %d\n", arrLen, ctx->offset));
                }
                os_freea (floatArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqFloat (
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
    jdouble *doubleArray;
    jsize arrLen;
    c_double **dst;
    int i;
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
            } else {
                *dst = (c_double *)c_sequenceNew(sh->type, sh->size, arrLen);
                doubleArray = os_alloca (sizeof (jdouble) * arrLen);
                (*(ctx->javaEnv))->GetDoubleArrayRegion (ctx->javaEnv, array, 0, arrLen, doubleArray);
                result = saj_copyGetStatus(ctx);

                if(result == SAJ_COPYRESULT_OK){
                    TRACE(printf ("JNI: GetDoubleArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, arrLen, doubleArray));

                    for (i = 0; i < arrLen; i++) {
                	    (*dst)[i] = doubleArray[i];
                	    TRACE(printf ("%f;", (*dst)[i]));
                    }
                    TRACE(printf ("Copied in Double sequence size %d @ offset = %d\n", arrLen, ctx->offset));
                }
                os_freea (doubleArray);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSeqDouble (
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

    context.dst = dstStruct;
    context.base = ctx->base;
    context.javaEnv = ctx->javaEnv;
    csh = (sajCopyStruct *)ch;
    result = SAJ_COPYRESULT_OK;
    csm = sajCopyStructMemberObject (csh);

    for (mi = 0; (mi < csh->nrOfMembers) && (result == SAJ_COPYRESULT_OK); mi++) {
        context.offset = csm->memberOffset;
        ch = sajCopyStructMemberDescription (csm);
        result = ciFromStruct[ch->copyType] (ch, structObject, csm->javaFID, &context);
        csm = (sajCopyStructMember *)sajCopyHeaderNextObject (ch);
    }
    TRACE(printf ("Copied in Struct @ offset = %d. Result = %d\n", ctx->offset, result));

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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiStruct (
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
    sajCopyUnionCase *unionCase;
    saj_copyResult result;

    cuh = (sajCopyUnion *)ch;
    switch (c_baseObject(cuh->discrType)->kind) {
    case M_ENUMERATION: {
        jint enumVal;
        jobject enumObject;
        c_long *dst;

        if(cuh->discrID)
        {
            enumObject = (*(ctx->javaEnv))->GetObjectField(ctx->javaEnv, unionObject, cuh->discrID);
        } else
        {
            enumObject = (*(ctx->javaEnv))->CallObjectMethod(ctx->javaEnv, unionObject, cuh->getDiscrMethodID);
        }
        result = saj_copyGetStatus(ctx);

        if (result == SAJ_COPYRESULT_OK) {
            TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", unionObject, cuh->discrID, enumObject));
            enumVal = (*(ctx->javaEnv))->CallIntMethod(ctx->javaEnv,
                    enumObject, cuh->valueID);
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: CallIntMethod (0x%x, %d) = 0x%d\n", enumObject, cuh->valueID, enumVal));
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

            if(cuh->discrID)
            {
                d = (*(ctx->javaEnv))->GetBooleanField(ctx->javaEnv, unionObject, cuh->discrID);
            } else
            {
                d = (*(ctx->javaEnv))->CallBooleanMethod(ctx->javaEnv, unionObject, cuh->getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: GetBooleanField (0x%x, %d) = %d\n", unionObject, cuh->discrID, d));
                discrVal = (unsigned long long)d;
                dst = (c_bool *)dstUnion;
                *dst = (c_bool)d;
            }
        }
            break;
        case P_CHAR: {
            jchar d;
            c_char *dst;

            if(cuh->discrID)
            {
                d = (*(ctx->javaEnv))->GetCharField(ctx->javaEnv, unionObject, cuh->discrID);
            } else
            {
                d = (*(ctx->javaEnv))->CallCharMethod(ctx->javaEnv, unionObject, cuh->getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: GetCharField (0x%x, %d) = %d\n", unionObject, cuh->discrID, d));

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

            if(cuh->discrID)
            {
                d = (*(ctx->javaEnv))->GetShortField(ctx->javaEnv, unionObject, cuh->discrID);
            } else
            {
                d = (*(ctx->javaEnv))->CallShortMethod(ctx->javaEnv, unionObject, cuh->getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: GetShortField (0x%x, %d) = %d\n", unionObject, cuh->discrID, d));
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

            if(cuh->discrID)
            {
                d = (*(ctx->javaEnv))->GetIntField(ctx->javaEnv, unionObject, cuh->discrID);
            } else
            {
                d = (*(ctx->javaEnv))->CallIntMethod(ctx->javaEnv, unionObject, cuh->getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI: GetIntField (0x%x, %d) = %d\n", unionObject, cuh->discrID, d));

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

            if(cuh->discrID)
            {
                d = (*(ctx->javaEnv))->GetLongField(ctx->javaEnv, unionObject, cuh->discrID);
            } else
            {
                d = (*(ctx->javaEnv))->CallLongMethod(ctx->javaEnv, unionObject, cuh->getDiscrMethodID);
            }
            result = saj_copyGetStatus(ctx);

            if (result == SAJ_COPYRESULT_OK) {
                TRACE(printf ("JNI:  (0x%x, %d) = %lld\n", unionObject, cuh->discrID, d));
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
            unionCase = (sajCopyUnionCase *)lab;
            ch = (sajCopyHeader *)sajCopyUnionCaseDescription (unionCase);

            if (active_case) {
                result = ciFromUnion[ch->copyType](ch, unionObject, unionCase->getterID, &context);
                ci = cuh->nrOfCases;
            } else {
                ci++;
            }
            csl = (sajCopyUnionLabels *)sajCopyHeaderNextObject (ch);
        }

        if (!active_case && defaultLabel && (result == SAJ_COPYRESULT_OK)) {
            unionCase = (sajCopyUnionCase *)sajCopyUnionLabelObject (defaultLabel);
            ch = sajCopyUnionCaseDescription (unionCase);
            result = ciFromUnion[ch->copyType](ch, unionObject, unionCase->getterID, &context);
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiUnion (
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiString (
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
    unsigned int length;

    dst = (c_string *)(dstString);

    if(stringObject){
        strNative = (*(ctx->javaEnv))->GetStringUTFChars (ctx->javaEnv, (jstring)stringObject, 0);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            cbs = (sajCopyBoundedString*)ch;
            length = (unsigned int)strlen(strNative);

            if(length <= cbs->max){
                TRACE(printf ("JNI: GetStringUTFChars (0x%x) = 0x%x\n", stringObject, strNative));
                *dst = c_stringNew (ctx->base, (c_char *)strNative);
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
    unsigned int length;

    dst = (c_char *)(dstArray);
    if(stringObject){
        strNative = (*(ctx->javaEnv))->GetStringUTFChars (ctx->javaEnv, (jstring)stringObject, 0);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            cbs = (sajCopyBoundedString*)ch;
            length = (unsigned int)strlen(strNative);
            /* length must be smaller then max because the array internally is 1
             * bigger then the string bounds to accomodate the '/o' char
             */
            if(length < cbs->max){
                TRACE(printf ("JNI: GetStringUTFChars (0x%x) = 0x%x\n", stringObject, strNative));
                strncpy(dst, strNative, cbs->max);/*max already takes the '/0' char into account*/
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
    if (array == NULL)
    {
    	OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char array bounds violation (null reference).");
    	result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size)
    {
    	OS_REPORT(OS_ERROR, "dcpssaj", 0, "Char array bounds violation.");
    	result = SAJ_COPYRESULT_BAD_PARAMETER;
    } else
    {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK)
        {
            charArray = os_alloca (sizeof (jchar) * ah->size);
            (*(ctx->javaEnv))->GetCharArrayRegion (ctx->javaEnv, array, 0, ah->size, charArray);
            result = saj_copyGetStatus(ctx);
            if(result == SAJ_COPYRESULT_OK)
            {
                TRACE(printf ("JNI: GetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, charArray));
                /* Allocate the length of the array (and null terminator) as
                 * a database string
                 */
                *dst = c_stringMalloc(ctx->base, (ah->size + 1));
                if(*dst)
                {
                    os_uint32 i;
                    for(i = 0; i < ah->size; i++)
                    {
                         (*dst)[i] = (c_char)charArray[i];
                    }
                    (*dst)[ah->size] = '\0';
                }
            }
            os_freea (charArray);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiBString (
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
        saj_cfoiBString (ch, str, dst, ctx);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiBStringToArrChar (
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
        saj_cfoiBStringToArrChar (ch, str, dst, ctx);
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
    } else {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiArray (
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
    }
    return result;
}

STATIC saj_copyResult
saj_cfuiSequence (
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
saj_cfuiReference (
    sajCopyHeader *ch,
    jobject javaObject,
    jmethodID getterID,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);
    return ciFromUnion[nch->copyType] (nch, javaObject, getterID, ctx);
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
