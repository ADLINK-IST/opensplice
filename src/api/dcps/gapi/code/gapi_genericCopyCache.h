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
#ifndef GAPI_COPYCACHE_H
#define GAPI_COPYCACHE_H

#include "c_metabase.h"
#include "os_abstract.h"
#include "gapi.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* Copy cache definitions */
typedef enum {
    gapiBlackBox,
    /* Primitive types */
    gapiBoolean,
    gapiByte,
    gapiChar,
    gapiShort,
    gapiInt,
    gapiLong,
    gapiFloat,
    gapiDouble,
    /* Array of primitive type */
    gapiArrBoolean,
    gapiArrByte,
    gapiArrChar,
    gapiArrShort,
    gapiArrInt,
    gapiArrLong,
    gapiArrFloat,
    gapiArrDouble,
    /* Sequence of primitive type */
    gapiSeqBoolean,
    gapiSeqByte,
    gapiSeqChar,
    gapiSeqShort,
    gapiSeqInt,
    gapiSeqLong,
    gapiSeqFloat,
    gapiSeqDouble,
    /* Enumeration type */
    gapiEnum,
    /* Structured types */
    gapiStruct,
    gapiUnion,
    /* String types */
    gapiString,
    gapiBString,
    /* Array of object type */
    gapiArray,
    /* Sequence of object type */
    gapiSequence,
    /* Reference to existing type (for recursive definitions) */
    gapiRecursive
} gapiCopyType;

#define gapi_copyCache(o)        ((gapi_copyCache)(o))

typedef struct {
    unsigned short size;
    gapiCopyType copyType;
} gapiCopyHeader;

#define gapiCopyHeaderNextObject(copyHeader) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyHeader + copyHeader->size)

typedef struct {
    gapiCopyHeader header;
    unsigned int   size;
} gapiCopyBlackBox;

#define gapiCopyBlackBoxNextObject(copyBlackBox) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyBlackBox + copyBlackBox->header.size)

typedef struct {
    unsigned int memberOffset;
    /* member description is added */
} gapiCopyStructMember;

#define gapiCopyStructMemberDescription(copyStructMember) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyStructMember + sizeof (gapiCopyStructMember))

typedef struct {
    gapiCopyHeader header;
    unsigned int   nrOfMembers;
    unsigned int   size;
    unsigned int   userSize;
    /* sequence of gapiCopyStructMember follows */
} gapiCopyStruct;

#define gapiCopyStructMemberObject(copyStruct) \
    (gapiCopyStructMember *)((PA_ADDRCAST)copyStruct + sizeof (gapiCopyStruct))

#define gapiCopyStructNextObject(copyStruct) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyStruct + copyStruct->header.size)

typedef struct {
    unsigned long long labelVal;
} gapiCopyUnionLabel;

#define gapiCopyUnionLabelNext(copyUnionLabel) \
    (gapiCopyUnionLabel *)((PA_ADDRCAST)copyUnionLabel + sizeof (gapiCopyUnionLabel))

typedef struct {
    unsigned int labelCount;
    /* array of gapiCopyUnionLabel of length labelCount follows */
    /* gapiCopyUnionCase follows */
} gapiCopyUnionLabels;

#define gapiCopyUnionLabelObject(copyUnionLabels) \
    (gapiCopyUnionLabel *)((PA_ADDRCAST)copyUnionLabels + sizeof (gapiCopyUnionLabels))

#define gapiCopyUnionCaseObject(copyUnionLabels) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyUnionLabels + sizeof (gapiCopyUnionLabels) + \
        sizeof (gapiCopyUnionLabel) * copyUnionLabels->labelCount)

typedef struct {
    gapiCopyHeader header;
    unsigned int   nrOfCases;
    c_type         discrType;
    unsigned int   casesOffset;
    unsigned int   size;
    unsigned int   userSize;
    /* sequence of gapiCopyUnionLabels follows */
} gapiCopyUnion;

#define gapiCopyUnionLabelsObject(copyUnion) \
    (gapiCopyUnionLabels *)((PA_ADDRCAST)copyUnion + sizeof (gapiCopyUnion))

#define gapiCopyUnionNextObject(copyUnion) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyUnion + gapiCopyUnion->header.size)

typedef struct {
    gapiCopyHeader header;
    unsigned int   max;
} gapiCopyBoundedString;

#define gapiCopyBoundedStringNextObject(copyBoundedString) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyBoundedString + copyBoundedString->header.size)

typedef struct {
    gapiCopyHeader header;
    unsigned int   size;
} gapiCopyArray;

#define gapiCopyArrayNextObject(copyArray) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyArray + copyArray->header.size)

typedef struct {
    gapiCopyHeader header;
    unsigned int   typeSize;
    unsigned int   arraySize;
    /* array element description follows */
} gapiCopyObjectArray;

#define gapiCopyObjectArrayDescription(copyObjectArray) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyObjectArray + sizeof(gapiCopyObjectArray))

#define gapiCopyObjectArrayNextObject(copyObjectArray) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyObjectArray + copyObjectArray->header.size)

typedef struct {
    gapiCopyHeader header;
    c_type         type;
    unsigned int   size;
} gapiCopySequence;

#define gapiCopySequenceNextObject(copySequence) \
    (gapiCopyHeader *)((PA_ADDRCAST)copySequence + copySequence->header.size)

typedef struct {
    gapiCopyHeader header;
    c_type         type;
    unsigned int   baseTypeSize;
    unsigned int   userTypeSize;
    unsigned int   refCount;
    unsigned int   seqSize;
    /* sequence element description follows */
} gapiCopyObjectSequence;

#define gapiCopyObjectSequenceDescription(copyObjectSequence) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyObjectSequence + sizeof(gapiCopyObjectSequence))

#define gapiCopyObjectSequenceNextObject(copySequence) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyObjectSequence + copyObjectSequence->header.size)

typedef struct {
    gapiCopyHeader header;
    unsigned int   nrOfElements;
} gapiCopyEnum;

#define gapiCopyEnumNextObject(copyEnum) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyEnum + copyEnum->header.size)

typedef struct {
    gapiCopyHeader header;
    unsigned int   refIndex;
} gapiCopyReference;

#define gapiCopyReferenceNextObject(copyReference) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyReference + copyReference->header.size)

#define gapiCopyReferencedObject(copyReference) \
    (gapiCopyHeader *)((PA_ADDRCAST)copyReference - copyReference->refIndex)

OS_API gapi_copyCache
gapi_copyCacheNew (
    c_metaObject object);

OS_API gapi_copyCache
gapi_copyCacheDup (
    gapi_copyCache copyCache);

OS_API void
gapi_copyCacheFree (
    gapi_copyCache copyCache);

OS_API void 
gapi_copyCacheIncRef (
    gapi_copyCache copyCache);

OS_API c_long
gapi_copyCacheLength (
    gapi_copyCache copyCache);

OS_API void *
gapi_copyCacheCache (
    gapi_copyCache copyCache);

OS_API void
gapi_copyCacheDump (
    gapi_copyCache copyCache);

OS_API unsigned long long
gapi_getUnionDescriptor (
    gapiCopyType ct,
    void         *src);

OS_API unsigned long 
gapi_copyCacheGetUserSize (
    gapi_copyCache copyCache);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* GAPI_COPYCACHE_H */
