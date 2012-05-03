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
#ifndef SAJ_COPYCACHE_H
#define SAJ_COPYCACHE_H

#include "c_metabase.h"
#include "os_abstract.h"

#include <jni.h>

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DCPSSAJ
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define JNI_TRACE 0

#if JNI_TRACE
#define TRACE(function)     function
#else
#define TRACE(function)
#endif

typedef enum saj_copyResult {
    SAJ_COPYRESULT_OK,
    SAJ_COPYRESULT_BAD_PARAMETER,
    SAJ_COPYRESULT_ERROR
} saj_copyResult;

/* Copy cache definitions */
typedef enum {
    /* Primitive types */
    sajBoolean,
    sajByte,
    sajChar,
    sajShort,
    sajInt,
    sajLong,
    sajFloat,
    sajDouble,
    /* Array of primitive type */
    sajArrBoolean,
    sajArrByte,
    sajArrChar,
    sajArrCharToBString,
    sajArrShort,
    sajArrInt,
    sajArrLong,
    sajArrFloat,
    sajArrDouble,
    /* Sequence of primitive type */
    sajSeqBoolean,
    sajSeqByte,
    sajSeqChar,
    sajSeqShort,
    sajSeqInt,
    sajSeqLong,
    sajSeqFloat,
    sajSeqDouble,
    /* Enumeration type */
    sajEnum,
    /* Structured types */
    sajStruct,
    sajUnion,
    /* String types */
    sajString,
    sajBString,
    sajBStringToArrChar,
    /* Array of object type */
    sajArray,
    /* Sequence of object type */
    sajSequence,
    /* Reference to existing type (for recursive definitions) */
    sajRecursive
} sajCopyType;

C_CLASS(saj_copyCache);
#define saj_copyCache(o)	((saj_copyCache)(o))

typedef struct {
    jclass		dataClass;
    jmethodID		dataClass_constructor_mid;
    jclass		dataHolderClass;
    jfieldID		dataHolder_value_fid;
    jclass		dataSeqHolderClass;
    jfieldID		dataSeqHolder_value_fid;
} sajReaderCopyCache;

typedef struct {
    os_uint32           size;
    unsigned short	copyType;
} sajCopyHeader;

#define sajCopyHeaderNextObject(copyHeader) \
    (sajCopyHeader *)((PA_ADDRCAST)copyHeader + copyHeader->size)

typedef struct {
    os_uint32           memberOffset;
    jfieldID		javaFID;
    /* member description is added */
} sajCopyStructMember;

#define sajCopyStructMemberDescription(copyStructMember) \
    (sajCopyHeader *)((PA_ADDRCAST)copyStructMember + sizeof (sajCopyStructMember))

typedef struct {
    sajCopyHeader	header;
    unsigned int	nrOfMembers;
    jclass		structClass;
    jmethodID		constrID;
    /* sequence of sajCopyStructMember follows */
} sajCopyStruct;

#define sajCopyStructMemberObject(copyStruct) \
    (sajCopyStructMember *)((PA_ADDRCAST)copyStruct + sizeof (sajCopyStruct))

#define sajCopyStructNextObject(copyStruct) \
    (sajCopyHeader *)((PA_ADDRCAST)copyStruct + copyStruct->header.size)

typedef struct {
    unsigned long long	labelVal;
} sajCopyUnionLabel;

#define sajCopyUnionLabelNext(copyUnionLabel) \
    (sajCopyUnionLabel *)((PA_ADDRCAST)copyUnionLabel + sizeof (sajCopyUnionLabel))

typedef struct {
    /* offset is always 0 */
    jmethodID		setterID;
    jmethodID       setterWithDiscrID;
    jmethodID		getterID;
    jfieldID		caseID;
    /* case description follows */
} sajCopyUnionCase;

#define sajCopyUnionCaseDescription(copyUnionCase) \
    (sajCopyHeader *)((PA_ADDRCAST)copyUnionCase + sizeof (sajCopyUnionCase))

typedef struct {
    unsigned int	labelCount;
    /* array of sajCopyUnionLabel of length labelCount follows */
    /* sajCopyUnionCase follows */
} sajCopyUnionLabels;

#define sajCopyUnionLabelObject(copyUnionLabels) \
    (sajCopyUnionLabel *)((PA_ADDRCAST)copyUnionLabels + sizeof (sajCopyUnionLabels))

#define sajCopyUnionCaseObject(copyUnionLabels) \
    (sajCopyUnionCase *)((PA_ADDRCAST)copyUnionLabels + sizeof (sajCopyUnionLabels) + \
	sizeof (sajCopyUnionLabel) * copyUnionLabels->labelCount)

typedef struct {
    sajCopyHeader	header;
    unsigned int	nrOfCases;
    jclass	        unionClass;
    jmethodID		constrID;
    jfieldID		discrID;
    jmethodID		getDiscrMethodID;
    c_type	        discrType;
    os_uint32       casesOffset;
    jclass	        discrClass;	/* used for enumeration discriminant */
    jmethodID		valueID;
    jmethodID		from_intID;
    jmethodID       __defaultID;
    /* sequence of sajCopyUnionLabels follows */
} sajCopyUnion;

#define sajCopyUnionLabelsObject(copyUnion) \
    (sajCopyUnionLabels *)((PA_ADDRCAST)copyUnion + sizeof (sajCopyUnion))

#define sajCopyUnionNextObject(copyUnion) \
    (sajCopyHeader *)((PA_ADDRCAST)copyUnion + sajCopyUnion->header.size)

typedef struct {
    sajCopyHeader	header;
    unsigned int	max;
} sajCopyBoundedString;

#define sajCopyBoundedStringNextObject(copyBoundedString) \
    (sajCopyHeader *)((PA_ADDRCAST)copyBoundedString + copyBoundedString->header.size)

typedef struct {
    sajCopyHeader	header;
    unsigned int	size;
} sajCopyArray;

#define sajCopyArrayNextObject(copyArray) \
    (sajCopyHeader *)((PA_ADDRCAST)copyArray + copyArray->header.size)

typedef struct {
    sajCopyHeader	header;
    unsigned int	typeSize;
    unsigned int	arraySize;
    jclass		arrayClass;
    /* array element description follows */
} sajCopyObjectArray;

#define sajCopyObjectArrayDescription(copyObjectArray) \
    (sajCopyHeader *)((PA_ADDRCAST)copyObjectArray + sizeof(sajCopyObjectArray))

#define sajCopyObjectArrayNextObject(copyObjectArray) \
    (sajCopyHeader *)((PA_ADDRCAST)copyObjectArray + copyObjectArray->header.size)

typedef struct {
    sajCopyHeader	header;
    c_type		type;
    unsigned int	size;
} sajCopySequence;

#define sajCopySequenceNextObject(copySequence) \
    (sajCopyHeader *)((PA_ADDRCAST)copySequence + copySequence->header.size)

typedef struct {
    sajCopyHeader	header;
    c_type		type;
    unsigned int	typeSize;
    unsigned int	seqSize;
    jclass		seqClass;
    /* sequence element description follows */
} sajCopyObjectSequence;

#define sajCopyObjectSequenceDescription(copyObjectSequence) \
    (sajCopyHeader *)((PA_ADDRCAST)copyObjectSequence + sizeof(sajCopyObjectSequence))

#define sajCopyObjectSequenceNextObject(copySequence) \
    (sajCopyHeader *)((PA_ADDRCAST)copyObjectSequence + copyObjectSequence->header.size)

typedef struct {
    sajCopyHeader	header;
    unsigned int	nrOfElements;
    jclass		enumClass;
    jmethodID		valueID;
    jmethodID		from_intID;
} sajCopyEnum;

#define sajCopyEnumNextObject(copyEnum) \
    (sajCopyHeader *)((PA_ADDRCAST)copyEnum + copyEnum->header.size)

typedef struct {
    sajCopyHeader	header;
    unsigned int	refIndex;
} sajCopyReference;

#define sajCopyReferenceNextObject(copyReference) \
    (sajCopyHeader *)((PA_ADDRCAST)copyReference + copyReference->header.size)

#define sajCopyReferencedObject(copyReference) \
    (sajCopyHeader *)((PA_ADDRCAST)copyReference - copyReference->refIndex)

OS_API saj_copyCache
saj_copyCacheNew (
    JNIEnv *env,
    c_metaObject object,
    const os_char* orgPName,
    const os_char* tgtPName);

OS_API void
saj_copyCacheFree (
    saj_copyCache copyCache);

OS_API c_long
saj_copyCacheLength (
    saj_copyCache copyCache);

OS_API void *
saj_copyCacheCache (
    saj_copyCache copyCache);

OS_API sajReaderCopyCache *
saj_copyCacheReaderCache (
    saj_copyCache copyCache);

OS_API void
saj_copyCacheDump (
    saj_copyCache copyCache);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* SAJ_COPYCACHE_H */
