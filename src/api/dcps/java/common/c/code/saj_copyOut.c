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
#include "saj_copyOut.h"
#include "saj_utilities.h"
#include "saj__exception.h"

#include "os_abstract.h"
#include "os_report.h"
#include "os_heap.h"
#include "os_time.h"
#include "c_base.h"

#include "sd_cdr.h"

#include <string.h>

#define STATIC static

#ifdef JNISTATS
STATIC struct jniStats {
    int nrJniTransitions;
    os_time jniTransitionStart;
    os_time jniTotalTransitionTime;
    os_time copyOutStartTime;
} jniFacts;

#define JNI_START_COPY()                                                    \
{                                                                           \
    os_time timeZero = {0, 0};                                              \
    jniFacts.nrJniTransitions = 0;                                          \
    jniFacts.jniTransitionStart = timeZero;                                 \
    jniFacts.jniTotalTransitionTime = timeZero;                             \
    jniFacts.copyOutStartTime = os_timeGet();                               \
}


#define JNI_STAMP_FUNC(methodInvocation)                                    \
{                                                                           \
    os_time jniDuration;                                                    \
    jniFacts.jniTransitionStart = os_timeGet();                             \
    methodInvocation;                                                       \
    jniDuration = os_timeSub(os_timeGet(), jniFacts.jniTransitionStart);    \
    jniFacts.nrJniTransitions++;                                            \
    jniFacts.jniTotalTransitionTime = os_timeAdd(jniFacts.jniTotalTransitionTime, jniDuration);               \
}

#define JNI_STOP_COPY()                                                     \
{                                                                           \
    os_time copyDuration;                                                   \
    os_timeReal jniTime, totalTime;                                         \
    copyDuration = os_timeSub(os_timeGet(), jniFacts.copyOutStartTime);     \
    jniTime = os_timeToReal(jniFacts.jniTotalTransitionTime);               \
    totalTime = os_timeToReal(copyDuration);                                \
    OS_REPORT_4(OS_INFO, "dcpssaj", 0,                                      \
        "jniFacts: %d jniTransitions took %f seconds, which is %.0f percent of the total copyOut time of %f seconds.",\
        jniFacts.nrJniTransitions,                                          \
        jniTime,                                                            \
        jniTime/totalTime * 100,                                            \
        totalTime);                                                         \
}
#else
#define JNI_START_COPY()
#define JNI_STAMP_FUNC(methodInvocation) methodInvocation
#define JNI_STOP_COPY()
#endif

typedef struct {
    void *src;
    os_uint32 offset;
    JNIEnv *javaEnv;
} saj_context;

typedef saj_copyResult (*copyOutFromStruct)(sajCopyHeader *ch, jobject javaObject, jfieldID javaFID, saj_context *ctx);
typedef saj_copyResult (*copyOutFromGenericUnion)(sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
typedef saj_copyResult (*copyOutFromSupportedUnion)(sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
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
STATIC saj_copyResult saj_cfuoBooleanGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoByteGeneric       (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoCharGeneric       (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoShortGeneric      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoIntGeneric        (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoLongGeneric       (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoFloatGeneric      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoDoubleGeneric     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Array of primitive type */
STATIC saj_copyResult saj_cfuoArrBooleanGeneric (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrByteGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrCharGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrCharToBStringGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrShortGeneric   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrIntGeneric     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrLongGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrFloatGeneric   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrDoubleGeneric  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfuoSeqBooleanGeneric (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqByteGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqCharGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqShortGeneric   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqIntGeneric     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqLongGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqFloatGeneric   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqDoubleGeneric  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfuoEnumGeneric       (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfuoStructGeneric     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoUnionGeneric      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfuoStringGeneric     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoBStringGeneric    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoBStringToArrCharGeneric
                                                (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfuoArrayGeneric      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfuoSequenceGeneric   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Reference type */
STATIC saj_copyResult saj_cfuoReferenceGeneric  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopyGenericUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);

STATIC saj_copyResult saj_cfuoBooleanSupported  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoByteSupported     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoCharSupported     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoShortSupported    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoIntSupported      (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoLongSupported     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoFloatSupported    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoDoubleSupported   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Array of primitive type */
STATIC saj_copyResult saj_cfuoArrBooleanSupported(sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrByteSupported  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrCharSupported  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrCharToBStringSupported
                                                (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrShortSupported (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrIntSupported   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrLongSupported  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrFloatSupported (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoArrDoubleSupported(sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Sequence of primitive type */
STATIC saj_copyResult saj_cfuoSeqBooleanSupported(sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqByteSupported  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqCharSupported  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqShortSupported (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqIntSupported   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqLongSupported  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqFloatSupported (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoSeqDoubleSupported(sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Enumeration type */
STATIC saj_copyResult saj_cfuoEnumSupported     (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Structured types */
STATIC saj_copyResult saj_cfuoStructSupported   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoUnionSupported    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* String types */
STATIC saj_copyResult saj_cfuoStringSupported   (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoBStringSupported  (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
STATIC saj_copyResult saj_cfuoBStringToArrCharSupported
                                                (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Array of object type */
STATIC saj_copyResult saj_cfuoArraySupported    (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Sequence of object type */
STATIC saj_copyResult saj_cfuoSequenceSupported (sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);
    /* Reference type */
STATIC saj_copyResult saj_cfuoReferenceSupported(sajCopyHeader *ch, jobject javaObject, sajCopyUnion *cuh, sajCopySupportedUnionCase *unionCase, unsigned long long discrValue, saj_context *ctx);

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

STATIC copyOutFromGenericUnion coFromUnionGeneric[] = {
    saj_cfuoBooleanGeneric,
    saj_cfuoByteGeneric,
    saj_cfuoCharGeneric,
    saj_cfuoShortGeneric,
    saj_cfuoIntGeneric,
    saj_cfuoLongGeneric,
    saj_cfuoFloatGeneric,
    saj_cfuoDoubleGeneric,
    saj_cfuoArrBooleanGeneric,
    saj_cfuoArrByteGeneric,
    saj_cfuoArrCharGeneric,
    saj_cfuoArrCharToBStringGeneric,
    saj_cfuoArrShortGeneric,
    saj_cfuoArrIntGeneric,
    saj_cfuoArrLongGeneric,
    saj_cfuoArrFloatGeneric,
    saj_cfuoArrDoubleGeneric,
    saj_cfuoSeqBooleanGeneric,
    saj_cfuoSeqByteGeneric,
    saj_cfuoSeqCharGeneric,
    saj_cfuoSeqShortGeneric,
    saj_cfuoSeqIntGeneric,
    saj_cfuoSeqLongGeneric,
    saj_cfuoSeqFloatGeneric,
    saj_cfuoSeqDoubleGeneric,
    saj_cfuoEnumGeneric,
    saj_cfuoStructGeneric,
    saj_cfuoUnionGeneric,
    saj_cfuoStringGeneric,
    saj_cfuoBStringGeneric,
    saj_cfuoBStringToArrCharGeneric,
    saj_cfuoArrayGeneric,
    saj_cfuoSequenceGeneric,
    saj_cfuoReferenceGeneric
    };

STATIC copyOutFromSupportedUnion coFromUnionSupported[] = {
    saj_cfuoBooleanSupported,
    saj_cfuoByteSupported,
    saj_cfuoCharSupported,
    saj_cfuoShortSupported,
    saj_cfuoIntSupported,
    saj_cfuoLongSupported,
    saj_cfuoFloatSupported,
    saj_cfuoDoubleSupported,
    saj_cfuoArrBooleanSupported,
    saj_cfuoArrByteSupported,
    saj_cfuoArrCharSupported,
    saj_cfuoArrCharToBStringSupported,
    saj_cfuoArrShortSupported,
    saj_cfuoArrIntSupported,
    saj_cfuoArrLongSupported,
    saj_cfuoArrFloatSupported,
    saj_cfuoArrDoubleSupported,
    saj_cfuoSeqBooleanSupported,
    saj_cfuoSeqByteSupported,
    saj_cfuoSeqCharSupported,
    saj_cfuoSeqShortSupported,
    saj_cfuoSeqIntSupported,
    saj_cfuoSeqLongSupported,
    saj_cfuoSeqFloatSupported,
    saj_cfuoSeqDoubleSupported,
    saj_cfuoEnumSupported,
    saj_cfuoStructSupported,
    saj_cfuoUnionSupported,
    saj_cfuoStringSupported,
    saj_cfuoBStringSupported,
    saj_cfuoBStringToArrCharSupported,
    saj_cfuoArraySupported,
    saj_cfuoSequenceSupported,
    saj_cfuoReferenceSupported
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

#define saj_setGenericUnionBranch(javaObject,cuh,unionCase,jType,src,discrValue,ctx)                                                                    \
    if (unionCase->setterWithDiscrID)                                                                                                                   \
    {                                                                                                                                                   \
        switch (c_baseObject(cuh->discrType)->kind)                                                                                                     \
        {                                                                                                                                               \
        case M_ENUMERATION:                                                                                                                             \
        {                                                                                                                                               \
            jint enumVal = (jint) discrValue;                                                                                                           \
            jobject discrEnumObject;                                                                                                                    \
            saj_cfooEnum((sajCopyHeader*)sajCopyUnionEnumDiscrObject(cuh), &discrEnumObject, &enumVal, ctx);                                            \
            if (saj_copyGetStatus(ctx) == SAJ_COPYRESULT_OK){                                                                                           \
                (*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, discrEnumObject, *((jType*) src));           \
                (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, discrEnumObject);                                                                      \
            }                                                                                                                                           \
            break;                                                                                                                                      \
        }                                                                                                                                               \
        case M_PRIMITIVE:                                                                                                                               \
            switch (c_primitive (cuh->discrType)->kind)                                                                                                 \
            {                                                                                                                                           \
            case P_BOOLEAN:                                                                                                                             \
                JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jboolean) discrValue, *((jType*)src)));\
                break;                                                                                                                                  \
            case P_CHAR:                                                                                                                                \
                JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jchar) discrValue, *((jType*)src)));\
                break;                                                                                                                                  \
            case P_SHORT:                                                                                                                               \
            case P_USHORT:                                                                                                                              \
                JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jshort) discrValue, *((jType*)src)));\
                break;                                                                                                                                  \
            case P_LONG:                                                                                                                                \
            case P_ULONG:                                                                                                                               \
                JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jint) discrValue, *((jType*)src)));\
                break;                                                                                                                                  \
            case P_LONGLONG:                                                                                                                            \
            case P_ULONGLONG:                                                                                                                           \
                JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterWithDiscrID, (jlong) discrValue, *((jType*)src)));\
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
        JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (ctx->javaEnv, javaObject, unionCase->setterID, *((jType*)src)));                              \
    }

#define saj_setSupportedUnionBranch(javaObject,cuh,unionCase,jType,jName,src,discrValue,ctx)                                                            \
    switch (c_baseObject(cuh->discrType)->kind)                                                                                                         \
    {                                                                                                                                                   \
    case M_ENUMERATION:                                                                                                                                 \
    {                                                                                                                                                   \
        jint enumVal = (jint) discrValue;                                                                                                               \
        jobject discrEnumObject;                                                                                                                        \
        saj_cfooEnum((sajCopyHeader*)sajCopyUnionEnumDiscrObject(cuh), &discrEnumObject, &enumVal, ctx);                                                \
        if (saj_copyGetStatus(ctx) == SAJ_COPYRESULT_OK){                                                                                               \
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, discrEnumObject);                  \
            (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, discrEnumObject);                                                                          \
        }                                                                                                                                               \
        break;                                                                                                                                          \
    }                                                                                                                                                   \
    case M_PRIMITIVE:                                                                                                                                   \
        switch (c_primitive (cuh->discrType)->kind)                                                                                                     \
        {                                                                                                                                               \
        case P_BOOLEAN:                                                                                                                                 \
            (*(ctx->javaEnv))->SetBooleanField (ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jboolean) discrValue);           \
            break;                                                                                                                                      \
        case P_CHAR:                                                                                                                                    \
            (*(ctx->javaEnv))->SetCharField (ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jchar) discrValue);                 \
            break;                                                                                                                                      \
        case P_SHORT:                                                                                                                                   \
        case P_USHORT:                                                                                                                                  \
            (*(ctx->javaEnv))->SetShortField (ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jshort) discrValue);               \
            break;                                                                                                                                      \
        case P_LONG:                                                                                                                                    \
        case P_ULONG:                                                                                                                                   \
            (*(ctx->javaEnv))->SetIntField (ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jint) discrValue);                   \
            break;                                                                                                                                      \
        case P_LONGLONG:                                                                                                                                \
        case P_ULONGLONG:                                                                                                                               \
            (*(ctx->javaEnv))->SetLongField (ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jlong) discrValue);                 \
            break;                                                                                                                                      \
        default:                                                                                                                                        \
            assert(FALSE); /* Unknown Primitive Type. */                                                                                                \
        }                                                                                                                                               \
        break;                                                                                                                                          \
    default:                                                                                                                                            \
        assert(FALSE); /* Unknown Discriminator Type. */                                                                                                \
    }                                                                                                                                                   \
    if (saj_copyGetStatus(ctx) == SAJ_COPYRESULT_OK)                                                                                                    \
            (*(ctx->javaEnv))->Set##jName##Field (ctx->javaEnv, javaObject, unionCase->caseID, *((jType*) src));


static void
saj_setGenericUnionDefault(
    jobject javaObject,
    sajCopyUnion *cuh,
    unsigned long long discrValue,
    saj_context *ctx)
{
    switch (c_baseObject(cuh->discrType)->kind)
    {
    case M_ENUMERATION:
    {
        jobject discrEnumObject;
        saj_cfooEnum((sajCopyHeader*)sajCopyUnionEnumDiscrObject(cuh), &discrEnumObject, &discrValue, ctx);
        if (saj_copyGetStatus(ctx) == SAJ_COPYRESULT_OK){
            (*(ctx->javaEnv))->CallVoidMethod (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.genericUnion.__defaultID, discrEnumObject);
            (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, discrEnumObject);
        }
        break;
    }
    case M_PRIMITIVE:
        switch (c_primitive (cuh->discrType)->kind)
        {
        case P_BOOLEAN:
            JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.genericUnion.__defaultID, (jboolean) discrValue));
            break;
        case P_CHAR:
            JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod
                    (ctx->javaEnv, javaObject, cuh->unionAccessors.genericUnion.__defaultID, (jchar) discrValue));
            break;
        case P_SHORT:
        case P_USHORT:
            JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.genericUnion.__defaultID, (jshort) discrValue));
            break;
        case P_LONG:
        case P_ULONG:
            JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod
                    (ctx->javaEnv, javaObject, cuh->unionAccessors.genericUnion.__defaultID, (jint) discrValue));
            break;
        case P_LONGLONG:
        case P_ULONGLONG:
            JNI_STAMP_FUNC((*(ctx->javaEnv))->CallVoidMethod (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.genericUnion.__defaultID, (jlong) discrValue));
            break;
        default:
            assert(FALSE); /* Unknown Primitive Type. */
        }
        break;
    default:
        assert(FALSE); /* Unknown Discriminator Type. */
    }
}

static void
saj_setSupportedUnionDefault(
    jobject javaObject,
    sajCopyUnion *cuh,
    unsigned long long discrValue,
    saj_context *ctx)
{
    switch (c_baseObject(cuh->discrType)->kind)
    {
    case M_ENUMERATION:
    {
        jobject discrEnumObject;
        saj_cfooEnum((sajCopyHeader*)sajCopyUnionEnumDiscrObject(cuh), &discrEnumObject, &discrValue, ctx);
        if (saj_copyGetStatus(ctx) == SAJ_COPYRESULT_OK){
            (*(ctx->javaEnv))->SetObjectField (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, discrEnumObject);
            (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, discrEnumObject);
        }
        break;
    }
    case M_PRIMITIVE:
        switch (c_primitive (cuh->discrType)->kind)
        {
        case P_BOOLEAN:
            (*(ctx->javaEnv))->SetBooleanField (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jboolean) discrValue);
            break;
        case P_CHAR:
            (*(ctx->javaEnv))->SetCharField (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jchar) discrValue);
            break;
        case P_SHORT:
        case P_USHORT:
            (*(ctx->javaEnv))->SetShortField (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jshort) discrValue);
            break;
        case P_LONG:
        case P_ULONG:
            (*(ctx->javaEnv))->SetIntField (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jint) discrValue);
            break;
        case P_LONGLONG:
        case P_ULONGLONG:
            (*(ctx->javaEnv))->SetLongField (
                    ctx->javaEnv, javaObject, cuh->unionAccessors.supportedUnion.discrID, (jlong) discrValue);
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

    OS_UNUSED_ARG(ch);

    src = (c_bool *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetBooleanField (ctx->javaEnv, javaObject, javaFID, (jboolean)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetBooleanField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Boolean = %d @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoBooleanGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_bool *src;
    jboolean tmp;

    OS_UNUSED_ARG(ch);

    src = (c_bool *)ctx->src;
    /* Since the footprint of jboolean and c_bool might not be the same, perform the cast
     * before invoking the saj_setUnionBranch, which assumes it receives a jboolean*, which
     * might have an incorrect size.
     */
    tmp = (jboolean) *src;
    saj_setGenericUnionBranch(javaObject, cuh, unionCase, jboolean, &tmp, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Boolean = %d setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfuoBooleanSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_bool *src;
    jboolean tmp;

    OS_UNUSED_ARG(ch);

    src = (c_bool *)ctx->src;
    /* Since the footprint of jboolean and c_bool might not be the same, perform the cast
     * before invoking the saj_setUnionBranch, which assumes it receives a jboolean*, which
     * might have an incorrect size.
     */
    tmp = (jboolean) *src;
    saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jboolean, Boolean, &tmp, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetBooleanField (0x%x, %d) = %d)\n", javaObject, unionCase->caseID, *src));
    TRACE(printf ("Copied out Boolean = %d caseID = %x\n", *src, unionCase->caseID));

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
    c_octet *src;

    OS_UNUSED_ARG(ch);

    src = (c_octet *)((PA_ADDRCAST)ctx->src + ctx->offset);

    (*(ctx->javaEnv))->SetByteField (ctx->javaEnv, javaObject, javaFID, (jbyte)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetByteField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Byte = %hd @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoByteGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_octet *src;

    OS_UNUSED_ARG(ch);

    src = (c_octet *)ctx->src;
    saj_setGenericUnionBranch(javaObject, cuh, unionCase, jbyte, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Byte = %hd setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfuoByteSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_octet *src;

    OS_UNUSED_ARG(ch);

    src = (c_octet *)ctx->src;
    saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jbyte, Byte, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetByteField (0x%x, %d) = %d)\n", javaObject, unionCase->caseID, *src));
    TRACE(printf ("Copied out Byte = %hd caseID = %x\n", *src, unionCase->caseID));

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

    OS_UNUSED_ARG(ch);

    src = (c_char *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetCharField (ctx->javaEnv, javaObject, javaFID, (jchar)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetCharField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Char = %hd @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoCharGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_char *src;
    jchar tmp;

    OS_UNUSED_ARG(ch);

    src = (c_char *)ctx->src;
    /* Since the footprint of jchar and c_char are not the same, perform the cast before
     * invoking the saj_setUnionBranch, which assumes it receives a jchar*, which has
     * an incorrect size. */
    tmp = (jchar) *src;
    saj_setGenericUnionBranch(javaObject, cuh, unionCase, jchar, &tmp, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, (jchar) *src));
    TRACE(printf ("Copied out Char = %d setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfuoCharSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_char *src;
    jchar tmp;

    OS_UNUSED_ARG(ch);

    src = (c_char *)ctx->src;
    /* Since the footprint of jchar and c_char are not the same, perform the cast before
     * invoking the saj_setUnionBranch, which assumes it receives a jchar*, which has
     * an incorrect size.
     */
    tmp = (jchar) *src;
    saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jchar, Char, &tmp, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetCharField (0x%x, %d) = %d)\n", javaObject, unionCase->caseID, *src));
    TRACE(printf ("Copied out Char = %d caseID = %x\n", *src, unionCase->caseID));

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

    OS_UNUSED_ARG(ch);

    src = (c_short *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetShortField (ctx->javaEnv, javaObject, javaFID, (jshort)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetShortField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Short = %hd @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoShortGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_short *src;

    OS_UNUSED_ARG(ch);

    src = (c_short *)ctx->src;
    saj_setGenericUnionBranch(javaObject, cuh, unionCase, jshort, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Short = %d setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfuoShortSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_short *src;

    OS_UNUSED_ARG(ch);

    src = (c_short *)ctx->src;
    saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jshort, Short, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetShortField (0x%x, %d) = %d)\n", javaObject, unionCase->caseID, *src));
    TRACE(printf ("Copied out Short = %d caseID = %x\n", *src, unionCase->caseID));

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

    OS_UNUSED_ARG(ch);

    src = (c_long *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetIntField (ctx->javaEnv, javaObject, javaFID, (jint)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetIntField (0x%x, %d, %d)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Int = %d @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoIntGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_long *src;

    OS_UNUSED_ARG(ch);

    src = (c_long *)ctx->src;
    saj_setGenericUnionBranch(javaObject, cuh, unionCase, jint, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %d)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Int = %d setterID = %x\n", *src, (int)unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfuoIntSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_long *src;

    OS_UNUSED_ARG(ch);

    src = (c_long *)ctx->src;
    saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jint, Int, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetIntField (0x%x, %d) = %d)\n", javaObject, unionCase->caseID, *src));
    TRACE(printf ("Copied out Int = %d caseID = %x\n", *src, (int)unionCase->caseID));

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

    OS_UNUSED_ARG(ch);

    src = (c_longlong *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetLongField (ctx->javaEnv, javaObject, javaFID, (jlong)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetLongField (0x%x, %d, %lld)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Long = %lld @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoLongGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_longlong *src;

    OS_UNUSED_ARG(ch);

    src = (c_longlong *)ctx->src;
    saj_setGenericUnionBranch(javaObject, cuh, unionCase, jlong, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %lld)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Long = %lld setterID = %x\n", *src, unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfuoLongSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_longlong *src;

    OS_UNUSED_ARG(ch);

    src = (c_longlong *)ctx->src;
    saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jlong, Long, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetLongField (0x%x, %d) = %lld)\n", javaObject, unionCase->caseID, *src));
    TRACE(printf ("Copied out Long = %lld caseID = %x\n", *src, unionCase->caseID));

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

    OS_UNUSED_ARG(ch);

    src = (c_float *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetFloatField (ctx->javaEnv, javaObject, javaFID, (jfloat)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetFloatField (0x%x, %d, %f)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Float = %f @ offset = %d FID = %x\n", *src, ctx->offset, javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoFloatGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_float *src;

    OS_UNUSED_ARG(ch);

    src = (c_float *)ctx->src;
    saj_setGenericUnionBranch(javaObject, cuh, unionCase, jfloat, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %f)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Float = %f setterID = %x\n", *src, (int)unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfuoFloatSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_float *src;

    OS_UNUSED_ARG(ch);

    src = (c_float *)ctx->src;
    saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jfloat, Float, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetFloatField (0x%x, %d) = %f)\n", javaObject, unionCase->caseID, *src));
    TRACE(printf ("Copied out Float = %f caseID = %x\n", *src, (int)unionCase->caseID));

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

    OS_UNUSED_ARG(ch);

    src = (c_double *)((PA_ADDRCAST)ctx->src + ctx->offset);
    (*(ctx->javaEnv))->SetDoubleField (ctx->javaEnv, javaObject, javaFID, (jdouble)*src);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: SetDoubleField (0x%x, %d, %f)\n", javaObject, javaFID, *src));
    TRACE(printf ("Copied out Double = %f @ offset = %d FID = %x\n", *src, ctx->offset, (int)javaFID));

    return result;
}

STATIC saj_copyResult
saj_cfuoDoubleGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_double *src;

    OS_UNUSED_ARG(ch);

    src = (c_double *)ctx->src;
    saj_setGenericUnionBranch(javaObject, cuh, unionCase, jdouble, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, %f)\n", javaObject, unionCase->setterID, *src));
    TRACE(printf ("Copied out Double = %f setterID = %x\n", *src, (int)unionCase->setterID));

    return result;
}

STATIC saj_copyResult
saj_cfuoDoubleSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    saj_copyResult result;
    c_double *src;

    OS_UNUSED_ARG(ch);

    src = (c_double *)ctx->src;
    saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jdouble, Double, src, discrValue, ctx);
    result = saj_copyGetStatus(ctx);

    TRACE(printf ("JNI: GetDoubleField (0x%x, %d) = %f)\n", javaObject, unionCase->caseID, *src));
    TRACE(printf ("Copied out Double = %f caseID = %x\n", *src, (int)unionCase->caseID));

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

    if (copyEnum->enumType == SAJ_SUPPORTED_ENUM)
    {
        *enumObject = (*(ctx->javaEnv))->NewLocalRef (ctx->javaEnv, copyEnum->enumAccessors.supportedEnum.labels[*src]);
    }
    else
    {
        JNI_STAMP_FUNC(*enumObject = (*(ctx->javaEnv))->CallStaticObjectMethod (
                ctx->javaEnv, copyEnum->enumClass, copyEnum->enumAccessors.genericEnum.from_intID, *src));
        TRACE(printf ("JNI: CallStaticObjectMethod (0x%x, %d, %d)\n",
                copyEnum->enumClass, copyEnum->enumAccessors.genericEnum.from_intID, *src));
    }
    result = saj_copyGetStatus(ctx);

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
    result = saj_cfooEnum (ch, &enumObject, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, enumObject);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, enumObject));
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, enumObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoEnumGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
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
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jobject, &enumObject, discrValue, ctx);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, enumObject));
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, enumObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoEnumSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
        saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jobject, Object, &enumObject, discrValue, ctx);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, enumObject));
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, enumObject);
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
    c_bool *src;
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
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewBooleanArray (ctx->javaEnv, ah->size);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: NewBooleanArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = (jobject)array;
    } else {
        result = SAJ_COPYRESULT_OK;
    }

    if(result == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_bool, jboolean);
        (*(ctx->javaEnv))->SetBooleanArrayRegion (ctx->javaEnv, array, 0, ah->size, src);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: SetBooleanArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, src));
        TRACE(printf ("Copied out Boolean array size %d @ offset = %d\n", ah->size, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrBoolean (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrBooleanGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jbooleanArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrBoolean (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jbooleanArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrBooleanSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jbooleanArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
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
    c_octet *src;
    saj_copyResult result;

    src = (c_octet *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jbyteArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewByteArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: array == null -> NewByteArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewByteArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewByteArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_octet, jbyte);
        (*(ctx->javaEnv))->SetByteArrayRegion (ctx->javaEnv, array, 0, ah->size,(jbyte *) src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetByteArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, src));
        TRACE(printf ("Copied out Byte array size %d @ offset = %d\n", ah->size, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrByte (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrByteGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jbyteArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrByte (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jbyteArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrByteSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jbyteArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    saj_copyResult result;

    src = (c_char *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jcharArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK && ah->size){
        if((charArray = os_malloc (sizeof (jchar) * ah->size)) != NULL){
            /* sizeof(c_char) != sizeof(jchar), so need to copy one by one */
#ifndef OSPL_SAJ_NO_LOOP_UNROLLING
            SAJ_LOOP_UNROLL(ah->size, c_char, src, jchar, charArray);
#else /* OSPL_SAJ_NO_LOOP_UNROLLING */
            unsigned int i;
            for (i = 0; i < ah->size; i++) {
                charArray[i] = src[i];
                TRACE(printf ("%d;", src[i]));
            }
#endif /* OSPL_SAJ_NO_LOOP_UNROLLING */
            (*(ctx->javaEnv))->SetCharArrayRegion (ctx->javaEnv, array, 0, ah->size, charArray);
            result = saj_copyGetStatus(ctx);

            TRACE(printf ("JNI: SetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, charArray));
            TRACE(printf ("Copied out Char array size %d @ offset = %d\n", ah->size, ctx->offset));

            os_free (charArray);
        } else {
            result = SAJ_COPYRESULT_ERROR; /* Out of (heap) resources */
        }
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrChar (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrCharToBString (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    saj_copyResult result;

    src = (c_string *)srcBString;
    ah = (sajCopyArray *)ch;
    array = (jcharArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK) {
        if((charArray = os_malloc (sizeof (jchar) * ah->size)) != NULL){
            /* sizeof(c_char) != sizeof(jchar), so need to copy one by one */
#ifndef OSPL_SAJ_NO_LOOP_UNROLLING
            SAJ_LOOP_UNROLL(ah->size, c_char, *src, jchar, charArray);
#else /* OSPL_SAJ_NO_LOOP_UNROLLING */
            unsigned int i;
            for (i = 0; i < ah->size; i++) {
                charArray[i] = (*src)[i];
                TRACE(printf ("%d;", (*src)[i]));
            }
#endif /* OSPL_SAJ_NO_LOOP_UNROLLING */
            (*(ctx->javaEnv))->SetCharArrayRegion (ctx->javaEnv, array, 0, ah->size, charArray);
            result = saj_copyGetStatus(ctx);

            TRACE(printf ("JNI: SetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, charArray));
            TRACE(printf ("Copied out Char array size %d @ offset = %d\n", ah->size, ctx->offset));

            os_free (charArray);
        } else {
            result = SAJ_COPYRESULT_ERROR; /* Out of (heap) resources */
        }
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrCharGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jcharArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrChar (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jcharArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrCharSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
        result = saj_cfooArrChar (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jcharArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrCharToBStringGeneric(
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jcharArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrCharToBString (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jcharArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrCharToBStringSupported(
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
        result = saj_cfooArrCharToBString (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jcharArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    c_short *src;
    saj_copyResult result;

    src = (c_short *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jshortArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewShortArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewShortArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewShortArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewShortArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_short, jshort);
        (*(ctx->javaEnv))->SetShortArrayRegion (ctx->javaEnv, array, 0, ah->size, src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetShortArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, src));
        TRACE(printf ("Copied out Short array size %d @ offset = %d\n", ah->size, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrShort (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrShortGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jshortArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrShort (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jshortArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrShortSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jshortArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    c_long *src;
    saj_copyResult result;

    src = (c_long *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jintArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewIntArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewIntArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewIntArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewIntArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_long, jint);
        (*(ctx->javaEnv))->SetIntArrayRegion (ctx->javaEnv, array, 0, ah->size, src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetIntArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, src));
        TRACE(printf ("Copied out Int array size %d @ offset = %d\n", ah->size, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrInt (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrIntGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jintArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrInt (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jintArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrIntSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jintArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    c_longlong *src;
    saj_copyResult result;

    src = (c_longlong *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jlongArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewLongArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewLongArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewLongArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewLongArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_longlong, jlong);
        (*(ctx->javaEnv))->SetLongArrayRegion (ctx->javaEnv, array, 0, ah->size, src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetLongArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, src));
        TRACE(printf ("Copied out Long array size %d @ offset = %d\n", ah->size, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrLong (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrLongGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jlongArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrLong (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jlongArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrLongSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jlongArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewFloatArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewFloatArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_float, jfloat);
        (*(ctx->javaEnv))->SetFloatArrayRegion (ctx->javaEnv, array, 0, ah->size, src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetFloatArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, src));
        TRACE(printf ("Copied out Float array size %d @ offset = %d\n", ah->size, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrFloat (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrFloatGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jfloatArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrFloat (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jfloatArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrFloatSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jfloatArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
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
    c_double *src;
    saj_copyResult result;

    src = (c_double *)srcArray;
    ah = (sajCopyArray *)ch;
    array = (jdoubleArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewDoubleArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewDoubleArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != (int)ah->size) {
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewDoubleArray (ctx->javaEnv, ah->size);
        TRACE(printf ("JNI: NewDoubleArray (%d) = 0x%x\n", ah->size, array));
        *objectArray = array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_double, jdouble);
        (*(ctx->javaEnv))->SetDoubleArrayRegion (ctx->javaEnv, array, 0, ah->size, src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetDoubleArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, ah->size, src));
        TRACE(printf ("Copied out Double array size %d @ offset = %d\n", ah->size, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooArrDouble (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrDoubleGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jdoubleArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooArrDouble (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jdoubleArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrDoubleSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jdoubleArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
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
    jbooleanArray array;
    c_bool **src;
    jsize seqLen;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    src = (c_bool **)srcSeq;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jbooleanArray)(*objectArray);

    if (array == NULL){
        array = (*(ctx->javaEnv))->NewBooleanArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewBooleanArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewBooleanArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewBooleanArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        if(seqLen > 0) {
            SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_bool, jboolean);
            (*(ctx->javaEnv))->SetBooleanArrayRegion (ctx->javaEnv, array, 0, seqLen, *src);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetBooleanArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, *src));
            TRACE(printf ("Copied out Boolean sequence size %d @ offset = %d\n", seqLen, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooSeqBoolean (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqBooleanGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jbooleanArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooSeqBoolean (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jbooleanArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqBooleanSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jbooleanArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    jbyteArray array;
    c_octet **src;
    int seqLen;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    src = (c_octet **)srcSeq;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jbyteArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewByteArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewByteArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewByteArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewByteArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if ((result == SAJ_COPYRESULT_OK) && (seqLen > 0)) {
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_octet, jbyte);
        (*(ctx->javaEnv))->SetByteArrayRegion (ctx->javaEnv, array, 0,seqLen, (jbyte *) *src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetByteArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, *src));
        TRACE(printf ("Copied out Byte sequence size %d @ offset = %d\n", seqLen, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooSeqByte (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqByteGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jbyteArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooSeqByte (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jbyteArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqByteSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jbyteArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    jcharArray array;
    jchar *charArray;
    c_char **src;
    int seqLen;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    src = (c_char **)srcSeq;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jcharArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewCharArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewCharArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0 )){
        if((charArray = os_malloc (sizeof (jchar) * seqLen)) != NULL){
            /* sizeof(c_char) != sizeof(jchar), so need to copy one by one */
#ifndef OSPL_SAJ_NO_LOOP_UNROLLING
            SAJ_LOOP_UNROLL(seqLen, c_char, *src, jchar, charArray);
#else /* OSPL_SAJ_NO_LOOP_UNROLLING */
            unsigned int i;
            for (i = 0; i < seqLen; i++) {
                charArray[i] = (*src)[i];
                TRACE(printf ("%d;", (*src)[i]));
            }
#endif /* OSPL_SAJ_NO_LOOP_UNROLLING */
            (*(ctx->javaEnv))->SetCharArrayRegion (ctx->javaEnv, array, 0, seqLen, charArray);
            result = saj_copyGetStatus(ctx);

            TRACE(printf ("JNI: SetCharArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, charArray));
            TRACE(printf ("Copied out Char sequence size %d @ offset = %d\n", seqLen, ctx->offset));

            os_free (charArray);
        } else {
            result = SAJ_COPYRESULT_ERROR; /* Out of (heap) resources */
        }
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooSeqChar (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqCharGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jcharArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooSeqChar (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jcharArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqCharSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jcharArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    jshortArray array;
    c_short **src;
    int seqLen;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    src = (c_short **)srcSeq;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jshortArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewShortArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewShortArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewShortArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewShortArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0 )){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_short, jshort);
        (*(ctx->javaEnv))->SetShortArrayRegion (ctx->javaEnv, array, 0, seqLen, *src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetShortArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, *src));
        TRACE(printf ("Copied out Short sequence size %d @ offset = %d\n", seqLen, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooSeqShort (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqShortGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jshortArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooSeqShort (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jshortArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqShortSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jshortArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    jintArray array;
    c_long **src;
    int seqLen;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    src = (c_long **)srcSeq;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jintArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewIntArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewIntArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewIntArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewIntArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0)){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_long, jint);
        (*(ctx->javaEnv))->SetIntArrayRegion (ctx->javaEnv, array, 0, seqLen, *src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetIntArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, *src));
        TRACE(printf ("Copied out Int sequence size %d @ offset = %d\n", seqLen, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooSeqInt (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqIntGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jintArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooSeqInt (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jintArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqIntSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
        saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jintArray, Object, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
    }
    (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);

    return result;
}

STATIC saj_copyResult
saj_cfooSeqLong (
    sajCopyHeader *ch,
    jobject *objectArray,
    void *srcSeq,
    saj_context *ctx)
{
    jlongArray array;
    c_longlong **src;
    int seqLen;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    src = (c_longlong **)srcSeq;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jlongArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewLongArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewLongArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewLongArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewLongArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0)){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_longlong, jlong);
        (*(ctx->javaEnv))->SetLongArrayRegion (ctx->javaEnv, array, 0, seqLen, *src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetLongArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, *src));
        TRACE(printf ("Copied out Long sequence size %d @ offset = %d\n", seqLen, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooSeqLong (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqLongGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jlongArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooSeqLong (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jlongArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqLongSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jlongArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    jfloatArray array;
    c_float **src;
    int seqLen;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    src = (c_float **)srcSeq;
    seqLen = c_arraySize ((c_sequence)*src);

    array = (jfloatArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewFloatArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewFloatArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewFloatArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewFloatArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0)){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_float, jfloat);
        (*(ctx->javaEnv))->SetFloatArrayRegion (ctx->javaEnv, array, 0, seqLen, *src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetFloatArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, src));
        TRACE(printf ("Copied out Float sequence size %d @ offset = %d\n", seqLen, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooSeqFloat (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqFloatGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jfloatArray array = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooSeqFloat (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jfloatArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqFloatSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jfloatArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    jdoubleArray array;
    c_double **src;
    int seqLen;
    saj_copyResult result;

    OS_UNUSED_ARG(ch);

    src = (c_double **)srcSeq;
    seqLen = c_arraySize ((c_sequence)*src);
    array = (jdoubleArray)(*objectArray);

    if (array == NULL) {
        array = (*(ctx->javaEnv))->NewDoubleArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewDoubleArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    } else if ((*(ctx->javaEnv))->GetArrayLength(ctx->javaEnv, array) != seqLen) {
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
        array = (*(ctx->javaEnv))->NewDoubleArray (ctx->javaEnv, seqLen);
        TRACE(printf ("JNI: NewDoubleArray (%d) = 0x%x\n", seqLen, array));
        *objectArray = (jobject)array;
    }
    result = saj_copyGetStatus(ctx);

    if((result == SAJ_COPYRESULT_OK) && (seqLen > 0)){
        SAJ_COMPILE_CONSTRAINT_SIZE_EQ(c_double, jdouble);
        (*(ctx->javaEnv))->SetDoubleArrayRegion (ctx->javaEnv, array, 0, seqLen, *src);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetDoubleArrayRegion (0x%x, %d, %d, 0x%x)\n", array, 0, seqLen, doubleArray));
        TRACE(printf ("Copied out Double sequence size %d @ offset = %d\n", seqLen, ctx->offset));
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    array = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, array));
        result = saj_cfooSeqDouble (ch, &array, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, array);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqDoubleGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jdoubleArray array = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)ctx->src;
    result = saj_cfooSeqDouble (ch, &array, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jdoubleArray, &array, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, array));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSeqDoubleSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jdoubleArray, Object, &array, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, array));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    structObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, structObject));
        result = saj_cfooStruct (ch, &structObject, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, structObject);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, structObject));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, structObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoStructGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject structObject = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)ctx->src;
    result = saj_cfooStruct (ch, &structObject, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jobject, &structObject, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, structObject));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, structObject);
    }
    TRACE(printf ("Copied out Struct setterID = %x\n", unionCase->setterID));
    return result;
}

STATIC saj_copyResult
saj_cfuoStructSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jobject, Object, &structObject, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, structObject));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, structObject);
        TRACE(printf ("Copied out Struct setterID = %x\n", unionCase->caseID));
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
    unsigned long long discrVal = 0;
    sajCopyUnionLabels *defaultLabel = NULL;
    int active_case = 0;
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
            if (cuh->unionType == SAJ_GENERIC_UNION)
            {
                sajCopyGenericUnionCase *unionCase = (sajCopyGenericUnionCase *)lab;
                ch = (sajCopyHeader *)sajCopyGenericUnionCaseDescription (unionCase);

                if (active_case) {
                    result = coFromUnionGeneric[ch->copyType] (ch, *unionObject, cuh, unionCase, discrVal, &context);
                    ci = cuh->nrOfCases;
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
                    result = coFromUnionSupported[ch->copyType] (ch, *unionObject, cuh, unionCase, discrVal, &context);
                    ci = cuh->nrOfCases;
                } else {
                    ci++;
                }
                csl = (sajCopyUnionLabels *)sajCopyHeaderNextObject (ch);
            }
        }
        if ((result == SAJ_COPYRESULT_OK) && (!active_case)) {
            if (defaultLabel) {
                if (cuh->unionType == SAJ_GENERIC_UNION)
                {
                    sajCopyGenericUnionCase *unionCase = (sajCopyGenericUnionCase *)sajCopyUnionLabelObject (defaultLabel);
                    ch = sajCopyGenericUnionCaseDescription (unionCase);
                    result = coFromUnionGeneric[ch->copyType] (ch, *unionObject, cuh, unionCase, discrVal, &context);
                }
                else
                {
                    sajCopySupportedUnionCase *unionCase = (sajCopySupportedUnionCase *)sajCopyUnionLabelObject (defaultLabel);
                    ch = sajCopySupportedUnionCaseDescription (unionCase);
                    result = coFromUnionSupported[ch->copyType] (ch, *unionObject, cuh, unionCase, discrVal, &context);
                }
            } else {
                if (cuh->unionType == SAJ_GENERIC_UNION)
                {
                    assert(cuh->unionAccessors.genericUnion.__defaultID); /* In situation there should be a default modifier. */
                    TRACE(printf ("JNI: CallVoidMethod (0x%x, %d)\n", *unionObject, cuh->unionAccessors.genericUnion.__defaultID));
                    saj_setGenericUnionDefault(*unionObject, cuh, discrVal, &context);
                }
                else
                {
                    TRACE(printf ("JNI: SetXXXField (0x%x, %d)\n", *unionObject, cuh->unionAccessors.supportedUnion.discrID));
                    saj_setSupportedUnionDefault(*unionObject, cuh, discrVal, &context);
                }
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    unionObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, unionObject));
        result = saj_cfooUnion (ch, &unionObject, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, unionObject);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, unionObject));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, unionObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoUnionGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject unionObject = NULL;
    void *src;
    saj_copyResult result = SAJ_COPYRESULT_OK;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    result = saj_cfooUnion (ch, &unionObject, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jobject, &unionObject, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, unionObject));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, unionObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoUnionSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jobject, Object, &unionObject, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, unionObject));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, unionObject);
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
    saj_copyResult result = SAJ_COPYRESULT_OK;

    OS_UNUSED_ARG(ch);

    src = (c_string *)srcString;

    assert(src);

    if (*src == NULL || (*src)[0] == '\0') {
        *stringObject = saj_getEmptyStringRef(ctx->javaEnv);
        TRACE(printf ("Copied out empty string = %s @ offset = %d\n", *src, ctx->offset));
    } else {
        *stringObject = (*(ctx->javaEnv))->NewStringUTF (ctx->javaEnv, *src);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: NewStringUTF (\"%s\")\n", *src));
        TRACE(printf ("Copied out string = %s @ offset = %d\n", *src, ctx->offset));
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
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoStringGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    c_string *src;
    jstring str = NULL;
    saj_copyResult result;

    src = (c_string *)ctx->src;
    result = saj_cfooString (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jstring, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, \"%s\")\n", javaObject, unionCase->setterID, str));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoStringSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    c_string *src;
    jstring str = NULL;
    saj_copyResult result;

    src = (c_string *)ctx->src;
    result = saj_cfooString (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jstring, Object, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetObjectField (0x%x, %d, \"%s\")\n", javaObject, unionCase->caseID, str));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
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
    saj_copyResult result = SAJ_COPYRESULT_OK;

    OS_UNUSED_ARG(ch);

    src = (c_string *)srcString;

    assert(src);

    if(*src == NULL || (*src)[0] == '\0') {
        *stringObject = saj_getEmptyStringRef(ctx->javaEnv);
        TRACE(printf ("Copied out empty bounded string = %s @ offset = %d\n", *src, ctx->offset));
    } else {
       *stringObject = (*(ctx->javaEnv))->NewStringUTF (ctx->javaEnv, *src);
       result = saj_copyGetStatus(ctx);

       TRACE(printf ("JNI: NewStringUTF (\"%s\")\n", *src));
       TRACE(printf ("Copied out bounded string = %s @ offset = %d\n", *src, ctx->offset));
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

    OS_UNUSED_ARG(ch);

    src = (c_char *)srcArray;
    if(*src == NULL || src[0] == '\0') {
        *stringObject = saj_getEmptyStringRef(ctx->javaEnv);
        TRACE(printf ("Copied out empty bounded string = %s @ offset = %d\n", src, ctx->offset));
        result = SAJ_COPYRESULT_OK;
    } else {
        *stringObject = (*(ctx->javaEnv))->NewStringUTF (ctx->javaEnv, src);
        result = saj_copyGetStatus(ctx);

        TRACE(printf ("JNI: NewStringUTF (\"%s\")\n", src));
        TRACE(printf ("Copied out bounded string = %s @ offset = %d\n", src, ctx->offset));
    }
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
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
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
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoBStringGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    c_string *src;
    saj_copyResult result;
    jstring str = NULL;

    src = (c_string *)ctx->src;
    result = saj_cfooBString (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jstring, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, \"%s\")\n", javaObject, unionCase->setterID, str));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoBStringSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    c_string *src;
    saj_copyResult result;
    jstring str = NULL;

    src = (c_string *)ctx->src;
    result = saj_cfooBString (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jstring, Object, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetObjectField (0x%x, %d, \"%s\")\n", javaObject, unionCase->caseID, str));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoBStringToArrCharGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    void *src;
    saj_copyResult result;
    jstring str = NULL;

    src = (void *)ctx->src;
    result = saj_cfooBStringToArrChar (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jstring, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, \"%s\")\n", javaObject, unionCase->setterID, str));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoBStringToArrCharSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    void *src;
    saj_copyResult result;
    jstring str = NULL;

    src = (void *)ctx->src;
    result = saj_cfooBStringToArrChar (ch, &str, src, ctx);

    if(result == SAJ_COPYRESULT_OK){
        saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jstring, Object, &str, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: SetObjectField (0x%x, %d, \"%s\")\n", javaObject, unionCase->caseID, str));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, str);
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
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
            (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, element);
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    arrayObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, arrayObject));
        result = saj_cfooArray (ch, &arrayObject, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, arrayObject);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, arrayObject));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, arrayObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArrayGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject arrayObject = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    result = saj_cfooArray (ch, &arrayObject, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jobject, &arrayObject, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, arrayObject));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, arrayObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoArraySupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jobject, Object, &arrayObject, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, arrayObject));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, arrayObject);
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
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, array);
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
                (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, element);
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
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    arrayObject = (*(ctx->javaEnv))->GetObjectField (ctx->javaEnv, javaObject, javaFID);
    result = saj_copyGetStatus(ctx);

    if(result == SAJ_COPYRESULT_OK){
        TRACE(printf ("JNI: GetObjectField (0x%x, %d) = 0x%x\n", javaObject, javaFID, arrayObject));
        result = saj_cfooSequence (ch, &arrayObject, src, ctx);

        if (result == SAJ_COPYRESULT_OK) {
            (*(ctx->javaEnv))->SetObjectField (ctx->javaEnv, javaObject, javaFID, arrayObject);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, javaFID, arrayObject));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, arrayObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSequenceGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    jobject arrayObject = NULL;
    void *src;
    saj_copyResult result;

    src = (void *)((PA_ADDRCAST)ctx->src + ctx->offset);
    result = saj_cfooSequence (ch, &arrayObject, src, ctx);

    if (result == SAJ_COPYRESULT_OK) {
        saj_setGenericUnionBranch(javaObject, cuh, unionCase, jobject, &arrayObject, discrValue, ctx);
        result = saj_copyGetStatus(ctx);
        TRACE(printf ("JNI: CallVoidMethod (0x%x, %d, 0x%x)\n", javaObject, unionCase->setterID, arrayObject));
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, arrayObject);
    }
    return result;
}

STATIC saj_copyResult
saj_cfuoSequenceSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
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
            saj_setSupportedUnionBranch(javaObject, cuh, unionCase, jobject, Object, &arrayObject, discrValue, ctx);
            result = saj_copyGetStatus(ctx);
            TRACE(printf ("JNI: SetObjectField (0x%x, %d, 0x%x)\n", javaObject, unionCase->caseID, arrayObject));
        }
        (*(ctx->javaEnv))->DeleteLocalRef(ctx->javaEnv, arrayObject);
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
saj_cfuoReferenceGeneric (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopyGenericUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);
    return coFromUnionGeneric[nch->copyType] (nch, javaObject, cuh, unionCase, discrValue, ctx);
}

STATIC saj_copyResult
saj_cfuoReferenceSupported (
    sajCopyHeader *ch,
    jobject javaObject,
    sajCopyUnion *cuh,
    sajCopySupportedUnionCase *unionCase,
    unsigned long long discrValue,
    saj_context *ctx)
{
    sajCopyReference *ref;
    sajCopyHeader *nch;

    ref = (sajCopyReference *)ch;
    nch = sajCopyReferencedObject (ref);
    return coFromUnionSupported[nch->copyType] (nch, javaObject, cuh, unionCase, discrValue, ctx);
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

    JNI_START_COPY();
    ch = saj_copyCacheCache(dstInfo->copyProgram);
    context.src = src;
    context.offset = 0;
    context.javaEnv = dstInfo->javaEnv;
    result = coFromArray[ch->copyType] (ch, &dstInfo->javaObject, src, &context);
    if(result != SAJ_COPYRESULT_OK){
        (*(dstInfo->javaEnv))->ExceptionDescribe(dstInfo->javaEnv);
    }
    JNI_STOP_COPY();

    return;
}

void
saj_CDROutStruct (
    void *src,
    void *dst)
{
    struct sd_cdrSerdata *serdata;
    saj_dstInfo dstInfo = (saj_dstInfo)dst;
    jobject bb;
    os_uint32 cdr_size;
    struct sd_cdrInfo *ci;
    const void *cdr_blob;

    JNI_START_COPY();

    ci = saj_copyCacheCdrInfo (dstInfo->copyProgram);
    if ((serdata = sd_cdrSerialize(ci, src)) == NULL) {
        saj_exceptionThrow (dstInfo->javaEnv, SAJ_EXCEPTION_TYPE_MARSHAL,
                            "CDROut serialization failure");
    } else {
      cdr_size = sd_cdrSerdataBlob(&cdr_blob, serdata);

      bb = (*(dstInfo->javaEnv))->NewDirectByteBuffer(dstInfo->javaEnv, (void *) cdr_blob, cdr_size);
      (*(dstInfo->javaEnv))->DeleteLocalRef(dstInfo->javaEnv, dstInfo->javaObject);
      dstInfo->javaObject = (*(dstInfo->javaEnv))->CallObjectMethod(dstInfo->javaEnv, dstInfo->jreader, GET_CACHED(dataReaderImplClassCDRDeserializeByteBuffer_mid), bb);
      if((*(dstInfo->javaEnv))->ExceptionCheck(dstInfo->javaEnv)){
        (*(dstInfo->javaEnv))->ExceptionDescribe(dstInfo->javaEnv);
      }
      (*(dstInfo->javaEnv))->DeleteLocalRef(dstInfo->javaEnv, bb);

      sd_cdrSerdataFree(serdata);
    }

    JNI_STOP_COPY();

    return;
}
