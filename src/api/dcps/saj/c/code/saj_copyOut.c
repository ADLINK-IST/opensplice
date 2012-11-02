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
#include "saj_copyOut.h"
#include "saj_utilities.h"
#include "saj__exception.h"

#include "os_abstract.h"
#include "os_report.h"
#include "os_heap.h"
#include "c_base.h"

#include <string.h>

#define STATIC static

typedef struct {
    void *src;
    os_uint32 offset;
    JNIEnv *javaEnv;
} saj_context;

typedef saj_copyResult (*copyOutFromStruct)(sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
typedef saj_copyResult (*copyOutFromUnion)(sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
typedef saj_copyResult (*copyOutFromArray)(sajCopyHeader *ch, jobject *objectArray, void *srcArray, saj_context *ctx);

    /* Primitive types */
STATIC saj_copyResult saj_cfsoBoolean    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoByte       (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoChar       (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoShort      (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoInt        (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoLong       (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoFloat      (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoDouble     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Array of primitive type */
STATIC saj_copyResult saj_cfsoArrBoolean (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoArrByte    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoArrChar    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoArrCharToBString    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoArrShort   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoArrInt     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoArrLong    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoArrFloat   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoArrDouble  (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfsoSeqBoolean (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoSeqByte    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoSeqChar    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoSeqShort   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoSeqInt     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoSeqLong    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoSeqFloat   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoSeqDouble  (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfsoEnum       (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfsoStruct     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoUnion      (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfsoString     (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoBString    (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
STATIC saj_copyResult saj_cfsoBStringToArrChar
                                         (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfsoArray      (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfsoSequence   (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
    /* Reference type */
STATIC saj_copyResult saj_cfsoReference  (sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);

    /* Primitive types */
STATIC saj_copyResult saj_cfuoBoolean    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoByte       (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoChar       (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoShort      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoInt        (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoLong       (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoFloat      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoDouble     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Array of primitive type */
STATIC saj_copyResult saj_cfuoArrBoolean (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrByte    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrChar    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrCharToBString    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrShort   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrInt     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrLong    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrFloat   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrDouble  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfuoSeqBoolean (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqByte    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqChar    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqShort   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqInt     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqLong    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqFloat   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqDouble  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfuoEnum       (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfuoStruct     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoUnion      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfuoString     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoBString    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoBStringToArrChar
                                         (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfuoArray      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfuoSequence   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Reference type */
STATIC saj_copyResult saj_cfuoReference  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);

    /* Array of primitive type */
STATIC saj_copyResult saj_cfooArrBoolean (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooArrByte    (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooArrChar    (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooArrCharToBString    (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooArrShort   (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooArrInt     (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooArrLong    (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooArrFloat   (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooArrDouble  (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfooSeqBoolean (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooSeqByte    (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooSeqChar    (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooSeqShort   (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooSeqInt     (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooSeqLong    (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooSeqFloat   (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
STATIC saj_copyResult saj_cfooSeqDouble  (sajCopyHeader *ch, jobject *objectArray,  void *src, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfooEnum       (sajCopyHeader *ch, jobject *enumObject,   void *srcEnum, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfooStruct     (sajCopyHeader *ch, jobject *structObject, void *srcStruct, saj_context *ctx);
STATIC saj_copyResult saj_cfooUnion      (sajCopyHeader *ch, jobject *unionObject,  void *srcUnion,  saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfooString     (sajCopyHeader *ch, jobject *stringObject, void *srcString, saj_context *ctx);
STATIC saj_copyResult saj_cfooBString    (sajCopyHeader *ch, jobject *stringObject, void *srcString, saj_context *ctx);
STATIC saj_copyResult saj_cfooBStringToArrChar
                                         (sajCopyHeader *ch, jobject *stringObject, void *srcString, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfooArray      (sajCopyHeader *ch, jobject *arrayObject,  void *srcArray, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfooSequence   (sajCopyHeader *ch, jobject *seqObject,    void *srcArray, saj_context *ctx);
    /* Reference type */
STATIC saj_copyResult saj_cfooReference  (sajCopyHeader *ch, jobject *seqObject,    void *srcArray, saj_context *ctx);

STATIC copyOutFromStruct coFromStruct[] = {
    saj_cfsoBoolean,
    saj_cfsoByte,
    saj_cfsoChar,
    saj_cfsoShort,
    saj_cfsoInt,
    saj_cfsoLong,
    saj_cfsoFloat,
    saj_cfsoDouble,
    saj_cfsoArrBoolean,
    saj_cfsoArrByte,
    saj_cfsoArrChar,
    saj_cfsoArrCharToBString,
    saj_cfsoArrShort,
    saj_cfsoArrInt,
    saj_cfsoArrLong,
    saj_cfsoArrFloat,
    saj_cfsoArrDouble,
    saj_cfsoSeqBoolean,
    saj_cfsoSeqByte,
    saj_cfsoSeqChar,
    saj_cfsoSeqShort,
    saj_cfsoSeqInt,
    saj_cfsoSeqLong,
    saj_cfsoSeqFloat,
    saj_cfsoSeqDouble,
    saj_cfsoEnum,
    saj_cfsoStruct,
    saj_cfsoUnion,
    saj_cfsoString,
    saj_cfsoBString,
    saj_cfsoBStringToArrChar,
    saj_cfsoArray,
    saj_cfsoSequence,
    saj_cfsoReference
    };

STATIC copyOutFromUnion coFromUnion[] = {
    saj_cfuoBoolean,
    saj_cfuoByte,
    saj_cfuoChar,
    saj_cfuoShort,
    saj_cfuoInt,
    saj_cfuoLong,
    saj_cfuoFloat,
    saj_cfuoDouble,
    saj_cfuoArrBoolean,
    saj_cfuoArrByte,
    saj_cfuoArrChar,
    saj_cfuoArrCharToBString,
    saj_cfuoArrShort,
    saj_cfuoArrInt,
    saj_cfuoArrLong,
    saj_cfuoArrFloat,
    saj_cfuoArrDouble,
    saj_cfuoSeqBoolean,
    saj_cfuoSeqByte,
    saj_cfuoSeqChar,
    saj_cfuoSeqShort,
    saj_cfuoSeqInt,
    saj_cfuoSeqLong,
    saj_cfuoSeqFloat,
    saj_cfuoSeqDouble,
    saj_cfuoEnum,
    saj_cfuoStruct,
    saj_cfuoUnion,
    saj_cfuoString,
    saj_cfuoBString,
    saj_cfuoBStringToArrChar,
    saj_cfuoArray,
    saj_cfuoSequence,
    saj_cfuoReference
    };

STATIC copyOutFromArray coFromArray[] = {
    NULL, /* saj_cfooBoolean */
    NULL, /* saj_cfooByte */
    NULL, /* saj_cfooChar */
    NULL, /* saj_cfooShort */
    NULL, /* saj_cfooInt */
    NULL, /* saj_cfooLong */
    NULL, /* saj_cfooFloat */
    NULL, /* saj_cfooDouble */
    saj_cfooArrBoolean,
    saj_cfooArrByte,
    saj_cfooArrChar,
    saj_cfooArrCharToBString,
    saj_cfooArrShort,
    saj_cfooArrInt,
    saj_cfooArrLong,
    saj_cfooArrFloat,
    saj_cfooArrDouble,
    saj_cfooSeqBoolean,
    saj_cfooSeqByte,
    saj_cfooSeqChar,
    saj_cfooSeqShort,
    saj_cfooSeqInt,
    saj_cfooSeqLong,
    saj_cfooSeqFloat,
    saj_cfooSeqDouble,
    saj_cfooEnum,
    saj_cfooStruct,
    saj_cfooUnion,
    saj_cfooString,
    saj_cfooBString,
    saj_cfooBStringToArrChar,
    saj_cfooArray,
    saj_cfooSequence,
    saj_cfooReference
    };

STATIC saj_copyResult
saj_copyGetStatus(
    saj_context* context)
{
    jboolean errorOccurred;
    saj_copyResult result;

    errorOccurred = ((*(context->javaEnv))->ExceptionCheck (context->javaEnv));

    if(errorOccurred == JNI_TRUE){
        result = SAJ_COPYRESULT_ERROR;
    } else {
        result = SAJ_COPYRESULT_OK;
    }
    return result;
}

#define saj_setUnionBranch(javaObject,cuh,unionCase,jType,src,discrValue,ctx)                                                                           \
    if (unionCase->setterWithDiscrID)                                                                                                                   \
    {                                                                                                                                                   \
        switch (c_baseObject(cuh->discrType)->kind)                                                                                                     \
        {                                                                                                                                               \
        case M_ENUMERATION:                                                                                                                             \
        {                                                                                                                                               \
            jint enumVal = (jint) discrValue;                                                                                                           \
            jobject discrEnumObject = (*(ctx->javaEnv))->CallStaticObjectMethod (ctx->javaEnv, cuh->discrClass, cuh->from_intID, enumVal);              \
            if (saj_copyGetStatus(ctx) == SAJ_COPYRESULT_OK)                                                                                            \
                (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, discrEnumObject, *((jType*) src));           \
            break;                                                                                                                                      \
        }                                                                                                                                               \
        case M_PRIMITIVE:                                                                                                                               \
            switch (c_primitive (cuh->discrType)->kind)                                                                                                 \
            {                                                                                                                                           \
            case P_BOOLEAN:                                                                                                                             \
                (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jboolean) discrValue, *((jType*)src));      \
                break;                                                                                                                                  \
            case P_CHAR:                                                                                                                                \
                (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jchar) discrValue, *((jType*)src));         \
                break;                                                                                                                                  \
            case P_SHORT:                                                                                                                               \
            case P_USHORT:                                                                                                                              \
                (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jshort) discrValue, *((jType*)src));        \
                break;                                                                                                                                  \
            case P_LONG:                                                                                                                                \
            case P_ULONG:                                                                                                                               \
                (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jint) discrValue, *((jType*)src));          \
                break;                                                                                                                                  \
            case P_LONGLONG:                                                                                                                            \
            case P_ULONGLONG:                                                                                                                           \
                (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jlong) discrValue, *((jType*)src));         \
                break;                                                                                                                                  \
            default:                                                                                                                                    \
                assert(FALSE); /* Unknown Primitive Type. */                                                                                            \
            }                                                                                                                                           \
            break;                                                                                                                                      \
        default:                                                                                                                                        \
            assert(FALSE); /* Unknown Discriminator Type. */                                                                                            \
        }                                                                                                                                               \
    }                                                                                                                                                   \
    else                                                                                                                                                \
    {                                                                                                                                                   \
        (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterID, *((jType*)src));                                              \
    }


static void
saj_setUnionDefault(
    jobject javaObject,
    sajCopyUnion *cuh,
    unsigned long long discrValue,
    saj_context *ctx)
{
    switch (c_baseObject(cuh->discrType)->kind)
    {
    case M_ENUMERATION:
    {
        jint enumVal = (jint) discrValue;
        jobject discrEnumObject = (*(ctx->javaEnv))->CallStaticObjectMethod (ctx->javaEnv, cuh->discrClass, cuh->from_intID, enumVal);
        if (saj_copyGetStatus(ctx) == SAJ_COPYRESULT_OK)
            (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, cuh->__defaultID, discrEnumObject);
        break;
    }
    case M_PRIMITIVE:
        switch (c_primitive (cuh->discrType)->kind)
        {
        case P_BOOLEAN:
            (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, cuh->__defaultID, (jboolean) discrValue);
            break;
        case P_CHAR:
            (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, cuh->__defaultID, (jchar) discrValue);
            break;
        case P_SHORT:
        case P_USHORT:
            (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, cuh->__defaultID, (jshort) discrValue);
            break;
        case P_LONG:
        case P_ULONG:
            (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, cuh->__defaultID, (jint) discrValue);
            break;
        case P_LONGLONG:
        case P_ULONGLONG:
            (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, cuh->__defaultID, (jlong) discrValue);
            break;
        default:
            assert(FALSE); /* Unknown Primitive Type. */
        }
        break;
    default:
        assert(FALSE); /* Unknown Discriminator Type. */
    }
}

    /* Primitive types */
STATIC saj_copyResult
saj_cfsoBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_bool *src;

    src = (c_bool *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetBooleanField (ctx->javaEnv, javaObject, javaFID, (jboolean)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetBooleanField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Boolean = %d @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_bool *src;
    jboolean tmp;

    src = (c_bool *)ctx->src;
    /* Since the footprint of jboolean and c_bool might not be the same, perform the cast
     * before invoking the saj_setUnionBranch, which assumes it receives a jboolean*, which
     * might have an incorrect size.
     */
    tmp = (jboolean) *src;
    saj_setUnionBranch(javaObject, cuh, unionCase, jboolean, &tmp, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Boolean = %d setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfsoByte (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;

    c_octet *src = (c_octet *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetByteField (ctx->javaEnv, javaObject, javaFID, (jbyte)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetByteField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Byte = %hd @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoByte (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_octet *src;

    src = (c_octet *)ctx->src;
    saj_setUnionBranch(javaObject, cuh, unionCase, jbyte, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Byte = %hd setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfsoChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_char *src;

    src = (c_char *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetCharField (ctx->javaEnv, javaObject, javaFID, (jchar)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetCharField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Char = %hd @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoChar (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_char *src;
    jchar tmp;

    src = (c_char *)ctx->src;
    /* Since the footprint of jchar and c_char are not the same, perform the cast before
     * invoking the saj_setUnionBranch, which assumes it receives a jchar*, which has
     * an incorrect size.
     */
    tmp = (jchar) *src;
    saj_setUnionBranch(javaObject, cuh, unionCase, jchar, &tmp, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, (jchar) *src));
    TRACE(printf ("Copied out Char = %d setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfsoShort (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_short *src;

    src = (c_short *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetShortField (ctx->javaEnv, javaObject, javaFID, (jshort)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetShortField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Short = %hd @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoShort (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_short *src;

    src = (c_short *)ctx->src;
    saj_setUnionBranch(javaObject, cuh, unionCase, jshort, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Short = %d setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfsoInt (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_long *src;

    src = (c_long *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetIntField (ctx->javaEnv, javaObject, javaFID, (jint)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetIntField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Int = %d @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoInt (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_long *src;

    src = (c_long *)ctx->src;
    saj_setUnionBranch(javaObject, cuh, unionCase, jint, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Int = %d setterID = %x\n", *src, (int)unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfsoLong (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_longlong *src;

    src = (c_longlong *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetLongField (ctx->javaEnv, javaObject, javaFID, (jlong)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetLongField (0x%x, %d, %lld)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Long = %lld @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoLong (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_longlong *src;

    src = (c_longlong *)ctx->src;
    saj_setUnionBranch(javaObject, cuh, unionCase, jlong, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %lld)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Long = %lld setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfsoFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_float *src;

    src = (c_float *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetFloatField (ctx->javaEnv, javaObject, javaFID, (jfloat)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetFloatField (0x%x, %d, %f)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Float = %f @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_float *src;

    src = (c_float *)ctx->src;
    saj_setUnionBranch(javaObject, cuh, unionCase, jfloat, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %f)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Float = %f setterID = %x\n", *src, (int)unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfsoDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    c_double *src;

    src = (c_double *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetDoubleField (ctx->javaEnv, javaObject, javaFID, (jdouble)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetDoubleField (0x%x, %d, %f)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Double = %f @ offset = %d FID = %x\n", *src, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_double *src;

    src = (c_double *)ctx->src;
    saj_setUnionBranch(javaObject, cuh, unionCase, jdouble, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %f)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Double = %f setterID = %x\n", *src, (int)unionCase->setterID));

    return result;
}

    /* Enumeration type */
STATIC saj_copyResult
saj_cfooEnum (
    sajCopyHeader *ch,
    jobject *enumObject,
    void *srcEnum,
    saj_context *ctx)
{
    saj_copyResult result;
    sajCopyEnum *copyEnum;
    c_long *src;

    copyEnum = (sajCopyEnum *)ch;
    src = (c_long *)srcEnum;
    *enumObject = (*(ctx->javaEnv))->CallStaticObjectMethod (ctx->javaEnv, copyEnum->enumClass, copyEnum->from_intID, *src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallStaticObjectMethod (0x%x, %d, %d)\n", copyEnum->enumClass, copyEnum->from_intID, *src));
    TRACE(printf ("Copied out Enum = %d @ offset = %d\n", *src, ctx->offset));

    return result;
}

STATIC saj_copyResult
saj_cfsoEnum (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    saj_copyResult result;
    jobject enumObject;
    void *src;

    src = (c_long *)((PA_ADDRCAST)ctx->src + ctx->offset);
    enumObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, enumObject));

    if(result == SAJ_COPYRESULT_OK){
        result = saj_cfooEnum (ch, &enumObject, src, ctx);
        (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, enumObject);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, enumObject));
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoEnum (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject enumObject;
    void *src;
    saj_copyResult result;

    src = (c_long *)ctx->src;
    enumObject = NULL;
    result = saj_cfooEnum (ch, &enumObject, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setUnionBranch(javaObject, cuh, unionCase, jobject, &enumObject, discrValue, ctx);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, enumObject));
    }
    return result;
}

    /* Array of primitive type */
STATIC saj_copyResult
saj_cfooArrBoolean (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jbooleanArray array;
    jboolean *booleanArray;
    c_bool *src;
    unsigned int i;
    saj_copyResult result;

    src = (c_bool *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jbooleanArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewBooleanArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewBooleanArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = (jobject)array;
        result = saj_copyGetStatus(ctx);
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            array = (*(ctx->javaEnv))->NewBooleanArray (ctx->javaEnv, ah->size);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: NewBooleanArray (%d) = 0x%x\n", ah->size, array));
            *objectArray = (jobject)array;
        }
    } else {
        result = SAJ_COPYRESULT_OK;
    }

    if(result == SAJ_COPYRESULT_OK){
        booleanArray = os_alloca (sizeof (jboolean) * ah->size);

        for (i = 0; i < ah->size; i++) {
            booleanArray[i] = src[i];
            TRACE(printf ("%d;", src[i]));
        }
        (*(ctx->javaEnv))->SetBooleanArrayRegion (ctx->javaEnv, array, 0, ah->size, booleanArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetBooleanArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, booleanArray));
        TRACE(printf ("Copied out Boolean array size %d @ offset = %d\n", ah->size, ctx->offset));
        os_freea (booleanArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jbooleanArray array;
    jbooleanArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        saj_cfooArrBoolean (ch, &array, src, ctx);

        if (arr != array) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jbooleanArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrBoolean (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jbooleanArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooArrByte (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jbyteArray array;
    jbyte *byteArray;
    c_octet *src;
    unsigned int i;
    saj_copyResult result;

    src = (c_octet *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jbyteArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewByteArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: array == null -> NewByteArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        array = (*(ctx->javaEnv))->NewByteArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewByteArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        byteArray = os_alloca (sizeof (jbyte) * ah->size);

        for (i = 0; i < ah->size; i++) {
            byteArray[i] = src[i];
            TRACE(printf ("%d;", src[i]));
        }
        (*(ctx->javaEnv))->SetByteArrayRegion (ctx->javaEnv, array, 0, ah->size, byteArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetByteArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, byteArray));
        TRACE(printf ("Copied out Byte array size %d @ offset = %d\n", ah->size, ctx->offset));

        os_freea (byteArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrByte (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jbyteArray array;
    jbyteArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooArrByte (ch, &array, src, ctx);

        if ((arr != array) && (result == SAJ_COPYRESULT_OK)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrByte (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jbyteArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrByte (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jbyteArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooArrChar (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jcharArray array;
    jchar *charArray;
    c_char *src;
    unsigned int i;
    saj_copyResult result;

    src = (c_char *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jcharArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        charArray = os_alloca (sizeof (jchar) * ah->size);

        for (i = 0; i < ah->size; i++) {
            charArray[i] = src[i];
            TRACE(printf ("%d;", src[i]));
        }
        (*(ctx->javaEnv))->SetCharArrayRegion (ctx->javaEnv, array, 0, ah->size, charArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, charArray));
        TRACE(printf ("Copied out Char array size %d @ offset = %d\n", ah->size, ctx->offset));

        os_freea (charArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jcharArray array;
    jcharArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooArrChar (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrCharToBString (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jcharArray array;
    jcharArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooArrCharToBString (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooArrCharToBString (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcBString,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jcharArray array;
    jchar *charArray;
    c_string *src;
    unsigned int i;
    saj_copyResult result;

    src = (c_string *)srcBString;
    ah = (sajCopyArray *)ch;
    array = (jcharArray)(*objectArray);

    if (array == NULL)
    {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size)
    {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK)
    {
        charArray = os_alloca (sizeof (jchar) * ah->size);

        for (i = 0; i < ah->size; i++)
        {
            charArray[i] = (*src)[i];
            TRACE(printf ("%d;", (*src)[i]));
        }
        (*(ctx->javaEnv))->SetCharArrayRegion (ctx->javaEnv, array, 0, ah->size, charArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, charArray));
        TRACE(printf ("Copied out Char array size %d @ offset = %d\n", ah->size, ctx->offset));

        os_freea (charArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrChar (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jcharArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrChar (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jcharArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrCharToBString(
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jcharArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrCharToBString (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jcharArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooArrShort (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jshortArray array;
    jshort *shortArray;
    c_short *src;
    unsigned int i;
    saj_copyResult result;

    src = (c_short *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jshortArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewShortArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewShortArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        array = (*(ctx->javaEnv))->NewShortArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewShortArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        shortArray = os_alloca (sizeof (jshort) * ah->size);

        for (i = 0; i < ah->size; i++) {
            shortArray[i] = src[i];
            TRACE(printf ("%d;", src[i]));
        }

        (*(ctx->javaEnv))->SetShortArrayRegion (ctx->javaEnv, array, 0, ah->size, shortArray);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetShortArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, shortArray));
        TRACE(printf ("Copied out Short array size %d @ offset = %d\n", ah->size, ctx->offset));
        os_freea (shortArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrShort (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jshortArray array;
    jshortArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooArrShort (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrShort (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jshortArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrShort (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jshortArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooArrInt (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jintArray array;
    jint *intArray;
    c_long *src;
    unsigned int i;
    saj_copyResult result;

    src = (c_long *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jintArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewIntArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewIntArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        array = (*(ctx->javaEnv))->NewIntArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewIntArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        intArray = os_alloca (sizeof (jint) * ah->size);

        for (i = 0; i < ah->size; i++) {
            intArray[i] = src[i];
            TRACE(printf ("%d;", src[i]));
        }
        (*(ctx->javaEnv))->SetIntArrayRegion (ctx->javaEnv, array, 0, ah->size, intArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetIntArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, intArray));
        TRACE(printf ("Copied out Int array size %d @ offset = %d\n", ah->size, ctx->offset));

        os_freea (intArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrInt (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jintArray array;
    jintArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooArrInt (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrInt (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jintArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrInt (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jintArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooArrLong (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jlongArray array;
    jlong *longArray;
    c_longlong *src;
    unsigned int i;
    saj_copyResult result;

    src = (c_longlong *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jlongArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewLongArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewLongArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        array = (*(ctx->javaEnv))->NewLongArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewLongArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        longArray = os_alloca (sizeof (jlong) * ah->size);

        for (i = 0; i < ah->size; i++) {
        	longArray[i] = src[i];
        	TRACE(printf ("%lld;", src[i]));
        }
        (*(ctx->javaEnv))->SetLongArrayRegion (ctx->javaEnv, array, 0, ah->size, longArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetLongArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, longArray));
        TRACE(printf ("Copied out Long array size %d @ offset = %d\n", ah->size, ctx->offset));

        os_freea (longArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrLong (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jlongArray array;
    jlongArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooArrLong (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrLong (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jlongArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrLong (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jlongArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooArrFloat (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jfloatArray array;
    jfloat *floatArray;
    jboolean isCopy;
    c_float *src;
    saj_copyResult result;
    /*unsigned int i;*/

    src = (c_float *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jfloatArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewFloatArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewFloatArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        array = (*(ctx->javaEnv))->NewFloatArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewFloatArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        floatArray = (*(ctx->javaEnv))->GetFloatArrayElements(ctx->javaEnv, array, &isCopy);
        result = saj_copyGetStatus(ctx);

        if(result == SAJ_COPYRESULT_OK){
            memcpy(floatArray, src, (ah->size * sizeof(jfloat)));
            (*(ctx->javaEnv))->ReleaseFloatArrayElements(ctx->javaEnv, array, floatArray, 0);
            TRACE(printf ("Copied out Float array size %d @ offset = %d\n", ah->size, ctx->offset));
            result = saj_copyGetStatus(ctx);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jfloatArray array;
    jfloatArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooArrFloat (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jfloatArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrFloat (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jfloatArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooArrDouble (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyArray *ah;
    jdoubleArray array;
    jdouble *doubleArray;
    c_double *src;
    unsigned int i;
    saj_copyResult result;

    src = (c_double *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jdoubleArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewDoubleArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewDoubleArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        array = (*(ctx->javaEnv))->NewDoubleArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewDoubleArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        doubleArray = os_alloca (sizeof (jdouble) * ah->size);

        for (i = 0; i < ah->size; i++) {
            doubleArray[i] = src[i];
            TRACE(printf ("%f;", src[i]));
        }
        (*(ctx->javaEnv))->SetDoubleArrayRegion (ctx->javaEnv, array, 0, ah->size, doubleArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetDoubleArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, doubleArray));
        TRACE(printf ("Copied out Double array size %d @ offset = %d\n", ah->size, ctx->offset));

        os_freea (doubleArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoArrDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jdoubleArray array;
    jdoubleArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooArrDouble (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jdoubleArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooArrDouble (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jdoubleArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

    /* Sequence of primitive type */
STATIC saj_copyResult
saj_cfooSeqBoolean (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jbooleanArray array;
    jboolean *booleanArray;
    c_bool **src;
    int i;
    jsize seqLen;
    saj_copyResult result;

    src = (c_bool **)srcSeq;
    sh = (sajCopySequence *)ch;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jbooleanArray)(*objectArray);

    if (array == NULL){
	    array = (*(ctx->javaEnv))->NewBooleanArray (ctx->javaEnv, seqLen);
	    TRACE(printf ("JNI: NewBooleanArray (%d) = 0x%x\n", seqLen, array));
	    *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
	    array = (*(ctx->javaEnv))->NewBooleanArray (ctx->javaEnv, seqLen);
	    TRACE(printf ("JNI: NewBooleanArray (%d) = 0x%x\n", seqLen, array));
	    *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        if(seqLen > 0)
        {
            booleanArray = os_alloca (sizeof (jboolean) * seqLen);
            for (i = 0; i < seqLen; i++){
    	        TRACE(printf ("\n-%d-\n", (*src)[i]));
    	        booleanArray[i] = (*src)[i];
    	        TRACE(printf ("%d;", (*src)[i]));
            }
            (*(ctx->javaEnv))->SetBooleanArrayRegion (ctx->javaEnv, array, 0, seqLen, booleanArray);
            result = saj_copyGetStatus(ctx);

            TRACE(printf ("JNI: SetBooleanArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, booleanArray));
            TRACE(printf ("Copied out Boolean sequence size %d @ offset = %d\n", seqLen, ctx->offset));

            os_freea (booleanArray);
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSeqBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jbooleanArray array;
    jbooleanArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooSeqBoolean (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqBoolean (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jbooleanArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooSeqBoolean (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jbooleanArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooSeqByte (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jbyteArray array;
    jbyte *byteArray;
    c_octet **src;
    int i;
    int seqLen;
    saj_copyResult result;

    src = (c_octet **)srcSeq;
    sh = (sajCopySequence *)ch;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jbyteArray)(*objectArray);

    if (array == NULL) {
	    array = (*(ctx->javaEnv))->NewByteArray (ctx->javaEnv, seqLen);
	    TRACE(printf ("JNI: NewByteArray (%d) = 0x%x\n", seqLen, array));
	    *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
	    array = (*(ctx->javaEnv))->NewByteArray (ctx->javaEnv, seqLen);
	    TRACE(printf ("JNI: NewByteArray (%d) = 0x%x\n", seqLen, array));
	    *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if ((result == SAJ_COPYRESULT_OK) && (seqLen > 0)) {
        byteArray = os_alloca (sizeof (jbyte) * seqLen);

        for (i = 0; i < seqLen; i++) {
            byteArray[i] = (*src)[i];
            TRACE(printf ("%d;", (*src)[i]));
        }
        (*(ctx->javaEnv))->SetByteArrayRegion (ctx->javaEnv, array, 0, seqLen, byteArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetByteArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, byteArray));
        TRACE(printf ("Copied out Byte sequence size %d @ offset = %d\n", seqLen, ctx->offset));

        os_freea (byteArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSeqByte (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jbyteArray array;
    jbyteArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooSeqByte (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqByte (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jbyteArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooSeqByte (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jbyteArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooSeqChar (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jcharArray array;
    jchar *charArray;
    c_char **src;
    int i;
    int seqLen;
    saj_copyResult result;

    src = (c_char **)srcSeq;
    sh = (sajCopySequence *)ch;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jcharArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0 )){
        charArray = os_alloca (sizeof (jchar) * seqLen);

        for (i = 0; i < seqLen; i++) {
            charArray[i] = (*src)[i];
            TRACE(printf ("%d;", (*src)[i]));
        }
        (*(ctx->javaEnv))->SetCharArrayRegion (ctx->javaEnv, array, 0, seqLen, charArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, charArray));
        TRACE(printf ("Copied out Char sequence size %d @ offset = %d\n", seqLen, ctx->offset));

        os_freea (charArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSeqChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jcharArray array;
    jcharArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooSeqChar (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqChar (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jcharArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooSeqChar (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jcharArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooSeqShort (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jshortArray array;
    jshort *shortArray;
    c_short **src;
    int i;
    int seqLen;
    saj_copyResult result;

    src = (c_short **)srcSeq;
    sh = (sajCopySequence *)ch;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jshortArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewShortArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewShortArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        array = (*(ctx->javaEnv))->NewShortArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewShortArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0 )){
        shortArray = os_alloca (sizeof (jshort) * seqLen);

        for (i = 0; i < seqLen; i++) {
            shortArray[i] = (*src)[i];
            TRACE(printf ("%d;", (*src)[i]));
        }
        (*(ctx->javaEnv))->SetShortArrayRegion (ctx->javaEnv, array, 0, seqLen, shortArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetShortArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, shortArray));
        TRACE(printf ("Copied out Short sequence size %d @ offset = %d\n", seqLen, ctx->offset));

        os_freea (shortArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSeqShort (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jshortArray array;
    jshortArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooSeqShort (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqShort (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jshortArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooSeqShort (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jshortArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooSeqInt (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jintArray array;
    jint *intArray;
    c_long **src;
    int i;
    int seqLen;
    saj_copyResult result;

    src = (c_long **)srcSeq;
    sh = (sajCopySequence *)ch;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jintArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewIntArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewIntArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        array = (*(ctx->javaEnv))->NewIntArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewIntArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0)){
        intArray = os_alloca (sizeof (jint) * seqLen);

        for (i = 0; i < seqLen; i++) {
            intArray[i] = (*src)[i];
            TRACE(printf ("%d;", (*src)[i]));
        }
        (*(ctx->javaEnv))->SetIntArrayRegion (ctx->javaEnv, array, 0, seqLen, intArray);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetIntArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, intArray));
        TRACE(printf ("Copied out Int sequence size %d @ offset = %d\n",
        seqLen, ctx->offset));
        os_freea (intArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSeqInt (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jintArray array;
    jintArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooSeqInt (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqInt (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jintArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
    result = saj_cfooSeqInt (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setUnionBranch(javaObject, cuh, unionCase, jintArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
    }
    return result;
}

STATIC saj_copyResult
saj_cfooSeqLong (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jlongArray array;
    jlong *longArray;
    c_longlong **src;
    int i;
    int seqLen;
    saj_copyResult result;

    src = (c_longlong **)srcSeq;
    sh = (sajCopySequence *)ch;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jlongArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewLongArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewLongArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        array = (*(ctx->javaEnv))->NewLongArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewLongArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0)){
        longArray = os_alloca (sizeof (jlong) * seqLen);

        for (i = 0; i < seqLen; i++) {
            longArray[i] = (*src)[i];
            TRACE(printf ("%lld;", (*src)[i]));
        }
        (*(ctx->javaEnv))->SetLongArrayRegion (ctx->javaEnv, array, 0, seqLen, longArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetLongArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, longArray));
        TRACE(printf ("Copied out Long sequence size %d @ offset = %d\n", seqLen, ctx->offset));

        os_freea (longArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSeqLong (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jlongArray array;
    jlongArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooSeqLong (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);

            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqLong (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jlongArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooSeqLong (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jlongArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooSeqFloat (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jfloatArray array;
    jfloat *floatArray;
    c_float **src;
    int i;
    int seqLen;
    saj_copyResult result;

    src = (c_float **)srcSeq;
    sh = (sajCopySequence *)ch;
    seqLen = c_arraySize ((c_sequence)*src);

    array = (jfloatArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewFloatArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewFloatArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        array = (*(ctx->javaEnv))->NewFloatArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewFloatArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0)){
        floatArray = os_alloca (sizeof (jfloat) * seqLen);

        for (i = 0; i < seqLen; i++) {
            floatArray[i] = (*src)[i];
            TRACE(printf ("%f;", (*src)[i]));
        }
        (*(ctx->javaEnv))->SetFloatArrayRegion (ctx->javaEnv, array, 0, seqLen, floatArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetFloatArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, floatArray));
        TRACE(printf ("Copied out Float sequence size %d @ offset = %d\n", seqLen, ctx->offset));

        os_freea (floatArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSeqFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jfloatArray array;
    jfloatArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));

        arr = array;
        result = saj_cfooSeqFloat (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqFloat (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jfloatArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooSeqFloat (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jfloatArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfooSeqDouble (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopySequence *sh;
    jdoubleArray array;
    jdouble *doubleArray;
    c_double **src;
    int i;
    int seqLen;
    saj_copyResult result;

    src = (c_double **)srcSeq;
    sh = (sajCopySequence *)ch;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jdoubleArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewDoubleArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewDoubleArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        array = (*(ctx->javaEnv))->NewDoubleArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewDoubleArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0)){
        doubleArray = os_alloca (sizeof (jdouble) * seqLen);

        for (i = 0; i < seqLen; i++) {
            doubleArray[i] = (*src)[i];
            TRACE(printf ("%f;", (*src)[i]));
        }
        (*(ctx->javaEnv))->SetDoubleArrayRegion (ctx->javaEnv, array, 0, seqLen, doubleArray);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetDoubleArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, doubleArray));
        TRACE(printf ("Copied out Double sequence size %d @ offset = %d\n", seqLen, ctx->offset));

        os_freea (doubleArray);
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSeqDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jdoubleArray array;
    jdoubleArray arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        arr = array;
        result = saj_cfooSeqDouble (ch, &array, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != array)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqDouble (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jdoubleArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, array));
        result = saj_cfooSeqDouble (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jdoubleArray, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        }
    }
    return result;
}

    /* Structured types */
STATIC saj_copyResult
saj_cfooStruct (
    sajCopyHeader *ch,
    jobject *structObject,
    void *srcStruct,
    saj_context *ctx)
{
    saj_context context;
    unsigned long mi;
    sajCopyStructMember *csm;
    sajCopyStruct *sh = (sajCopyStruct *)ch;
    jobject jObject;
    saj_copyResult result;

    context.src = srcStruct;
    csm = sajCopyStructMemberObject (ch);
    context.javaEnv = ctx->javaEnv;
    jObject = *structObject;

    if (jObject == NULL) {
        /* Create new object */
        jObject = (*(ctx->javaEnv))->AllocObject (ctx->javaEnv, sh->structClass);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: NewObject (0x%x, %d) = 0x%x\n", sh->structClass, sh->constrID, jObject));
        *structObject = jObject;
    } else {
        result = SAJ_COPYRESULT_OK;
    }

    for (mi = 0; (mi < sh->nrOfMembers) && (result == SAJ_COPYRESULT_OK); mi++) {
        context.offset = csm->memberOffset;
        ch = sajCopyStructMemberDescription (csm);
        result = coFromStruct[ch->copyType] (ch, jObject, csm->javaFID, &context);
        csm = (sajCopyStructMember *)sajCopyHeaderNextObject (ch);
    }
    TRACE(printf ("Copied out Struct @ offset = %d\n", ctx->offset));

    return result;
}

STATIC saj_copyResult
saj_cfsoStruct (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobject structObject;
    jobject so;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    structObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, structObject));
        so = structObject;
        result = saj_cfooStruct (ch, &structObject, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (so != structObject)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, structObject);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, structObject));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoStruct (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject structObject = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    if(unionCase->caseID)
    {
        structObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, structObject));
        result = saj_cfooStruct (ch, &structObject, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jobject, &structObject, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, structObject));
        }
        TRACE(printf ("Copied out Struct setterID = %x\n", unionCase->setterID));
    }
    return result;
}

STATIC saj_copyResult
saj_cfooUnion (
    sajCopyHeader *ch,
    jobject *unionObject,
    void *srcUnion,
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
    c_baseObject discrType;

    cuh = (sajCopyUnion *)ch;
    discrType = c_baseObject(cuh->discrType);

    if (*unionObject == NULL) {
        /* Create new object */
        *unionObject = (*(ctx->javaEnv))->AllocObject (ctx->javaEnv, cuh->unionClass);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: NewObject (0x%x, %d) = 0x%x\n", cuh->unionClass, cuh->constrID, *unionObject));
    } else {
        result = SAJ_COPYRESULT_OK;
    }
    if(result == SAJ_COPYRESULT_OK){
        switch (discrType->kind) {
        case M_ENUMERATION:
    	{
    	    c_long *src;

    	    src = (c_long *)srcUnion;
    	    discrVal = (unsigned long long)*src;
    	}
    	break;
        case M_PRIMITIVE:
    	switch (c_primitive (discrType)->kind) {
    	case P_BOOLEAN:
    	    {
    	        c_bool *src;

    	        src = (c_bool *)srcUnion;
    	        discrVal = (unsigned long long)*src;
    	    }
    	    break;
    	case P_CHAR:
    	    {
    	        c_char *src;
    	        src = (c_char *)srcUnion;
    	        discrVal = (unsigned long long)*src;
    	    }
    	    break;
    	case P_SHORT:
    	case P_USHORT:
    	    {
    	        c_short *src;

    	        src = (c_short *)srcUnion;
    	        discrVal = (unsigned long long)*src;
    	    }
    	    break;
    	case P_LONG:
    	case P_ULONG:
    	    {
    	        c_long *src;

    	        src = (c_long *)srcUnion;
    	        discrVal = (unsigned long long)*src;
    	    }
    	    break;
    	case P_LONGLONG:
    	case P_ULONGLONG:
    	    {
    	        c_longlong *src;

    	        src = (c_longlong *)srcUnion;
    	        discrVal = (unsigned long long)*src;
    	    }
    	    break;
            default:
            saj_exceptionThrow (ctx->javaEnv, SAJ_EXCEPTION_TYPE_MARSHAL, "Invalid discriminator type");
            OS_REPORT(OS_ERROR, "dcpssaj", 0, "saj_cfooUnion: Unexpected primitive kind");
            result = SAJ_COPYRESULT_BAD_PARAMETER;
    	    assert (0);
    	}
    	break;
        default:
            saj_exceptionThrow (ctx->javaEnv, SAJ_EXCEPTION_TYPE_MARSHAL, "Invalid discriminator type");
            OS_REPORT(OS_ERROR, "dcpssaj", 0, "saj_cfooUnion: Unexpected switch kind");
            result = SAJ_COPYRESULT_BAD_PARAMETER;
            assert (0);
        }

        srcUnion = (void *)((PA_ADDRCAST)srcUnion + cuh->casesOffset);
        context.javaEnv = ctx->javaEnv;
        context.src = srcUnion;
        context.offset = 0;
        csl = sajCopyUnionLabelsObject (cuh);
        ci = 0;

        while ((result == SAJ_COPYRESULT_OK) && (ci < cuh->nrOfCases)) {
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
        	    result = coFromUnion[ch->copyType] (ch, *unionObject, cuh, unionCase, discrVal, &context);
        	    ci = cuh->nrOfCases;
        	} else {
        	    ci++;
        	}
        	csl = (sajCopyUnionLabels *)sajCopyHeaderNextObject (ch);
        }
        if ((result == SAJ_COPYRESULT_OK) && (!active_case)) {
            if (defaultLabel) {
                unionCase = (sajCopyUnionCase *)sajCopyUnionLabelObject (defaultLabel);
                ch = sajCopyUnionCaseDescription (unionCase);
                result = coFromUnion[ch->copyType] (ch, *unionObject, cuh, unionCase, discrVal, &context);
            } else {
                assert(cuh->__defaultID); /* In situation there should be a default modifier. */
                TRACE(printf ("JNI: CallVoidMethod (0x%x, %d)\n", *unionObject, cuh->__defaultID));
                saj_setUnionDefault(*unionObject, cuh, discrVal, &context);
                result = saj_copyGetStatus(ctx);
            }
        }
        TRACE(printf ("Copied out Union\n"));
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoUnion (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobject unionObject;
    jobject uo;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    unionObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, unionObject));
        uo = unionObject;
        result = saj_cfooUnion (ch, &unionObject, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (uo != unionObject)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, unionObject);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, unionObject));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoUnion (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject unionObject = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    if(unionCase->caseID)
    {
        unionObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, unionObject));
        result = saj_cfooUnion (ch, &unionObject, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jobject, &unionObject, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, unionObject));
        }
    }
    return result;
}

    /* String types */
STATIC saj_copyResult
saj_cfooString (
    sajCopyHeader *ch,
    jobject *stringObject,
    void *srcString,
    saj_context *ctx)
{
    c_string *src;
    saj_copyResult result;

    src = (c_string *)srcString;

    if(src){
        *stringObject = (*(ctx->javaEnv))->NewStringUTF (ctx->javaEnv, *src);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: NewStringUTF (\"%s\")\n", *src));
        TRACE(printf ("Copied out string = %s @ offset = %d\n", *src, ctx->offset));
    } else {
        saj_exceptionThrow (ctx->javaEnv, SAJ_EXCEPTION_TYPE_BAD_PARAM, "String bounds violation (null reference)");
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "String bounds violation (null reference)");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoString (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    c_string *src;
    jstring str = NULL;
    saj_copyResult result;

    src = (c_string *)((PA_ADDRCAST)ctx->src + ctx->offset);
    result = saj_cfooString (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, str);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetObjectField (0x%x, %d, \"%s\")\n", javaObject, javaFID, str));
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoString (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    c_string *src;
    jstring str = NULL;
    saj_copyResult result;

    src = (c_string *)ctx->src;
    result = saj_cfooString (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setUnionBranch(javaObject, cuh, unionCase, jstring, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, \"%s\")\n", javaObject, unionCase->setterID, str));
    }
    return result;
}

STATIC saj_copyResult
saj_cfooBString (
    sajCopyHeader *ch,
    jobject *stringObject,
    void *srcString,
    saj_context *ctx)
{
    c_string *src;
    saj_copyResult result;

    src = (c_string *)srcString;

    if(src){
        *stringObject = (*(ctx->javaEnv))->NewStringUTF (ctx->javaEnv, *src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: NewStringUTF (\"%s\")\n", *src));
        TRACE(printf ("Copied out bounded string = %s @ offset = %d\n", *src, ctx->offset));
    } else {
        saj_exceptionThrow (ctx->javaEnv, SAJ_EXCEPTION_TYPE_MARSHAL,
            "String bounds violation (null reference)");
        OS_REPORT(OS_ERROR, "dcpssaj", 0, "String bounds violation (null reference).");
        result = SAJ_COPYRESULT_BAD_PARAMETER;
    }
    return result;
}

STATIC saj_copyResult
saj_cfooBStringToArrChar (
    sajCopyHeader *ch,
    jobject *stringObject,
    void *srcArray,
    saj_context *ctx)
{
    c_char *src;
    saj_copyResult result;

    src = (c_char *)srcArray;
    *stringObject = (*(ctx->javaEnv))->NewStringUTF (ctx->javaEnv, src);
    result = saj_copyGetStatus(ctx);
    TRACE(printf ("JNI: NewStringUTF (\"%s\")\n", src));
    TRACE(printf ("Copied out bounded string = %s @ offset = %d\n", src, ctx->offset));
    return result;
}

STATIC saj_copyResult
saj_cfsoBString (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    c_string *src;
    jstring str = NULL;
    saj_copyResult result;

    src = (c_string *)((PA_ADDRCAST)ctx->src + ctx->offset);
    result = saj_cfooBString (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, str);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetObjectField (0x%x, %d, \"%s\")\n", javaObject, javaFID, str));
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoBStringToArrChar (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    void *src;
    jstring str = NULL;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    result = saj_cfooBStringToArrChar (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, str);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetObjectField (0x%x, %d, \"%s\")\n", javaObject, javaFID, str));
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoBString (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    c_string *src;
    saj_copyResult result;
    jstring str = NULL;

    src = (c_string *)ctx->src;
    result = saj_cfooBString (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setUnionBranch(javaObject, cuh, unionCase, jstring, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, \"%s\")\n", javaObject, unionCase->setterID, str));
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoBStringToArrChar (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    void *src;
    saj_copyResult result;
    jstring str = NULL;

    src = (void *)ctx->src;
    result = saj_cfooBStringToArrChar (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setUnionBranch(javaObject, cuh, unionCase, jstring, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, \"%s\")\n", javaObject, unionCase->setterID, str));
    }
    return result;
}

    /* Array of object type */
STATIC saj_copyResult
saj_cfooArray (
    sajCopyHeader *ch,
    jobject *arrayObject,
    void *srcArray,
    saj_context *ctx)
{
    sajCopyObjectArray *ah;
    sajCopyHeader *aech;
    jobjectArray array;
    jobject element;
    void *src;
    unsigned int i;
    saj_copyResult result;

    src = (void *)srcArray;
    ah = (sajCopyObjectArray *)ch;
    aech = sajCopyObjectArrayDescription (ah);
    array = *arrayObject;

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewObjectArray (ctx->javaEnv, ah->arraySize, ah->arrayClass, NULL);
        TRACE(printf ("JNI: NewObjectArray (%d, 0x%x) = 0x%x\n", ah->arraySize, ah->arrayClass, array));
        *arrayObject = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->arraySize) {
        array = (*(ctx->javaEnv))->NewObjectArray (ctx->javaEnv, ah->arraySize, ah->arrayClass, NULL);
        TRACE(printf ("JNI: NewObjectArray (%d, 0x%x) = 0x%x\n", ah->arraySize, ah->arrayClass, array));
        *arrayObject = array;
    }
    result = saj_copyGetStatus(ctx);

    for (i = 0; (i < ah->arraySize) && (result == SAJ_COPYRESULT_OK); i++) {
        element = (*(ctx->javaEnv))->GetObjectArrayElement (ctx->javaEnv, array, i);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: GetObjectArrayElement (0x%x, %d) = 0x%x\n", array, i, element));

        if(result == SAJ_COPYRESULT_OK){
            result = coFromArray[aech->copyType] (aech, &element, src, ctx);

            if(result == SAJ_COPYRESULT_OK){
                (*(ctx->javaEnv))->SetObjectArrayElement (ctx->javaEnv, array, i, element);
                result = saj_copyGetStatus(ctx);
                TRACE(printf ("JNI: SetObjectArrayElement (0x%x, %d, 0x%x)\n", array, i, element));
            }
        }
        src = (void *)((PA_ADDRCAST)src + ah->typeSize);
    }
    TRACE(printf ("Copied out Object array size %d @ offset = %d\n", ah->typeSize, ctx->offset));

    return result;
}

STATIC saj_copyResult
saj_cfsoArray (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobject arrayObject;
    jobject arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    arrayObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, arrayObject));
        arr = arrayObject;
        result = saj_cfooArray (ch, &arrayObject, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != arrayObject)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, arrayObject);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, arrayObject));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArray (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject arrayObject = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    if(unionCase->caseID)
    {
        arrayObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, arrayObject));
        result = saj_cfooArray (ch, &arrayObject, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jobject, &arrayObject, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, arrayObject));
        }
    }
    return result;
}

    /* Sequence of object type */
STATIC saj_copyResult
saj_cfooSequence (
    sajCopyHeader *ch,
    jobject *seqObject,
    void *srcSeq,
    saj_context *ctx)
{
    sajCopyObjectSequence *sh;
    sajCopyHeader *sech;
    jobjectArray array;
    jobject element;
    c_sequence *srcSequence;
    void *src;
    int i;
    c_long seqLen;
    saj_copyResult result;

    srcSequence = (c_sequence *)srcSeq;
    seqLen = c_arraySize (*srcSequence);
    src = *srcSequence;
    sh = (sajCopyObjectSequence *)ch;
    sech = sajCopyObjectSequenceDescription (sh);
    array = *seqObject;

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewObjectArray (ctx->javaEnv, seqLen, sh->seqClass, NULL);
        TRACE(printf ("JNI: NewObjectArray (%d, 0x%x) = 0x%x\n", seqLen, sh->seqClass, array));
        *seqObject = array;
    } else if (seqLen != ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array))) {
        array = (*(ctx->javaEnv))->NewObjectArray (ctx->javaEnv, seqLen, sh->seqClass, NULL);
        TRACE(printf ("JNI: NewObjectArray (%d, 0x%x) = 0x%x\n", seqLen, sh->seqClass, array));
        *seqObject = array;
    }
    result = saj_copyGetStatus(ctx);

    if(seqLen > 0){
        for (i = 0;(result == SAJ_COPYRESULT_OK) && (i < seqLen); i++) {
            element = (*(ctx->javaEnv))->GetObjectArrayElement (ctx->javaEnv, array, i);
            result = saj_copyGetStatus(ctx);

            if(result == SAJ_COPYRESULT_OK){
                TRACE(printf ("JNI: GetObjectArrayElement (0x%x, %d) = 0x%x\n", array, i, element));
                result = coFromArray[sech->copyType] (sech, &element, src, ctx);

                if(result == SAJ_COPYRESULT_OK){
                    (*(ctx->javaEnv))->SetObjectArrayElement (ctx->javaEnv, array, i, element);
                    result = saj_copyGetStatus(ctx);
                    TRACE(printf ("JNI: SetObjectArrayElement (0x%x, %d, 0x%x)\n", array, i, element));
                }
            }
            src = (void *)((PA_ADDRCAST)src + sh->typeSize);
        }
        TRACE(printf ("Copied out Object sequence size %d @ offset = %d\n", sh->typeSize, ctx->offset));
    }
    return result;
}

STATIC saj_copyResult
saj_cfsoSequence (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    jobject arrayObject;
    jobject arr;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    arrayObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, arrayObject));
        arr = arrayObject;
        result = saj_cfooSequence (ch, &arrayObject, src, ctx);

        if ((result == SAJ_COPYRESULT_OK) && (arr != arrayObject)) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, arrayObject);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, arrayObject));
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSequence (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject arrayObject = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    if(unionCase->caseID)
    {
        arrayObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, unionCase->caseID);
        result = saj_copyGetStatus(ctx);
    }

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, unionCase->caseID, arrayObject));
        result = saj_cfooSequence (ch, &arrayObject, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setUnionBranch(javaObject, cuh, unionCase, jobject, &arrayObject, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, arrayObject));
        }
    }
    return result;
}

    /* backward referenced type */
STATIC saj_copyResult
saj_cfooReference (
    sajCopyHeader *ch,
    jobject *object,
    void *dst,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);
    return coFromArray[nch->copyType] (nch, object, dst, ctx);

}

STATIC saj_copyResult
saj_cfsoReference (
    sajCopyHeader *ch,
    jobject javaObject,
    jfieldID javaFID,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);
    return coFromStruct[nch->copyType] (nch, javaObject, javaFID, ctx);
}

STATIC saj_copyResult
saj_cfuoReference (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);
    return coFromUnion[nch->copyType] (nch, javaObject, cuh, unionCase, discrValue, ctx);
}

void
saj_copyOutStruct (
    void *src,
    void *dst)
{
    saj_context context;
    saj_dstInfo dstInfo = (saj_dstInfo)dst;
    sajCopyHeader *ch;
    saj_copyResult result;

    ch = saj_copyCacheCache(dstInfo->copyProgram);
    context.src = src;
    context.offset = 0;
    context.javaEnv = dstInfo->javaEnv;
    result = coFromArray[ch->copyType] (ch, &dstInfo->javaObject, src, &context);

    if(result != SAJ_COPYRESULT_OK){
        (*(dstInfo->javaEnv))->ExceptionDescribe(dstInfo->javaEnv);
    }

    return;
}
