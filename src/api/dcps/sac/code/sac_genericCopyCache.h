/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef DDS_COPYCACHE_H
#define DDS_COPYCACHE_H

#include "c_metabase.h"
#include "os_abstract.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSSAC
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(DDS_copyCache);

/* Copy cache definitions */
typedef enum {
    DDSBlackBox,
    /* Primitive types */
    DDSBoolean,
    DDSByte,
    DDSChar,
    DDSShort,
    DDSInt,
    DDSLong,
    DDSFloat,
    DDSDouble,
    /* Array of primitive type */
    DDSArrBoolean,
    DDSArrByte,
    DDSArrChar,
    DDSArrShort,
    DDSArrInt,
    DDSArrLong,
    DDSArrFloat,
    DDSArrDouble,
    /* Sequence of primitive type */
    DDSSeqBoolean,
    DDSSeqByte,
    DDSSeqChar,
    DDSSeqShort,
    DDSSeqInt,
    DDSSeqLong,
    DDSSeqFloat,
    DDSSeqDouble,
    /* Enumeration type */
    DDSEnum,
    /* Structured types */
    DDSStruct,
    DDSUnion,
    /* String types */
    DDSString,
    DDSBString,
    /* Array of object type */
    DDSArray,
    /* Sequence of object type */
    DDSSequence,
    /* Reference to existing type (for recursive definitions) */
    DDSRecursive
} DDSCopyType;

#define DDS_copyCache(o)        ((DDS_copyCache)(o))

typedef struct {
    unsigned short size;
    DDSCopyType copyType;
} DDSCopyHeader;

#define DDSCopyHeaderNextObject(copyHeader) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyHeader + copyHeader->size)

typedef struct {
    DDSCopyHeader header;
    unsigned int   size;
} DDSCopyBlackBox;

#define DDSCopyBlackBoxNextObject(copyBlackBox) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyBlackBox + copyBlackBox->header.size)

typedef struct {
    unsigned int memberOffset;
    /* member description is added */
} DDSCopyStructMember;

#define DDSCopyStructMemberDescription(copyStructMember) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyStructMember + sizeof (DDSCopyStructMember))

typedef struct {
    DDSCopyHeader header;
    unsigned int   nrOfMembers;
    unsigned int   size;
    unsigned int   userSize;
    /* sequence of DDSCopyStructMember follows */
} DDSCopyStruct;

#define DDSCopyStructMemberObject(copyStruct) \
    (DDSCopyStructMember *)((PA_ADDRCAST)copyStruct + sizeof (DDSCopyStruct))

#define DDSCopyStructNextObject(copyStruct) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyStruct + copyStruct->header.size)

typedef struct {
    unsigned long long labelVal;
} DDSCopyUnionLabel;

#define DDSCopyUnionLabelNext(copyUnionLabel) \
    (DDSCopyUnionLabel *)((PA_ADDRCAST)copyUnionLabel + sizeof (DDSCopyUnionLabel))

typedef struct {
    unsigned int labelCount;
    /* array of DDSCopyUnionLabel of length labelCount follows */
    /* DDSCopyUnionCase follows */
} DDSCopyUnionLabels;

#define DDSCopyUnionLabelObject(copyUnionLabels) \
    (DDSCopyUnionLabel *)((PA_ADDRCAST)copyUnionLabels + sizeof (DDSCopyUnionLabels))

#define DDSCopyUnionCaseObject(copyUnionLabels) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyUnionLabels + sizeof (DDSCopyUnionLabels) + \
        sizeof (DDSCopyUnionLabel) * copyUnionLabels->labelCount)

typedef struct {
    DDSCopyHeader header;
    unsigned int   nrOfCases;
    c_type         discrType;
    unsigned int   casesOffset;
    unsigned int   size;
    unsigned int   userSize;
    /* copy header for discriminant follows (primitive or enum) */
    /* sequence of DDSCopyUnionLabels follows */
} DDSCopyUnion;

#define DDSCopyUnionDiscriminantObject(copyUnion) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyUnion + sizeof (DDSCopyUnion))

#define DDSCopyUnionLabelsObject(copyUnionDiscriminant)                 \
    (DDSCopyUnionLabels *)((PA_ADDRCAST)copyUnionDiscriminant + copyUnionDiscriminant->size)

#define DDSCopyUnionNextObject(copyUnion) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyUnion + DDSCopyUnion->header.size)

typedef struct {
    DDSCopyHeader header;
    unsigned int   max;
} DDSCopyBoundedString;

#define DDSCopyBoundedStringNextObject(copyBoundedString) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyBoundedString + copyBoundedString->header.size)

typedef struct {
    DDSCopyHeader header;
    unsigned int   size;
} DDSCopyArray;

#define DDSCopyArrayNextObject(copyArray) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyArray + copyArray->header.size)

typedef struct {
    DDSCopyHeader header;
    unsigned int   typeSize;
    unsigned int   arraySize;
    /* array element description follows */
} DDSCopyObjectArray;

#define DDSCopyObjectArrayDescription(copyObjectArray) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyObjectArray + sizeof(DDSCopyObjectArray))

#define DDSCopyObjectArrayNextObject(copyObjectArray) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyObjectArray + copyObjectArray->header.size)

typedef struct {
    DDSCopyHeader header;
    c_type         type;
    unsigned int   size;
} DDSCopySequence;

#define DDSCopySequenceNextObject(copySequence) \
    (DDSCopyHeader *)((PA_ADDRCAST)copySequence + copySequence->header.size)

typedef struct {
    DDSCopyHeader header;
    c_type         type;
    unsigned int   baseTypeSize;
    unsigned int   userTypeSize;
    unsigned int   refCount;
    unsigned int   seqSize;
    /* sequence element description follows */
} DDSCopyObjectSequence;

#define DDSCopyObjectSequenceDescription(copyObjectSequence) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyObjectSequence + sizeof(DDSCopyObjectSequence))

#define DDSCopyObjectSequenceNextObject(copySequence) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyObjectSequence + copyObjectSequence->header.size)

typedef struct {
    DDSCopyHeader header;
    unsigned int   nrOfElements;
} DDSCopyEnum;

#define DDSCopyEnumNextObject(copyEnum) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyEnum + copyEnum->header.size)

typedef struct {
    DDSCopyHeader header;
    unsigned int   refIndex;
} DDSCopyReference;

#define DDSCopyReferenceNextObject(copyReference) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyReference + copyReference->header.size)

#define DDSCopyReferencedObject(copyReference) \
    (DDSCopyHeader *)((PA_ADDRCAST)copyReference - copyReference->refIndex)

OS_API DDS_copyCache
DDS_copyCacheNew (
    c_metaObject object);

OS_API DDS_copyCache
DDS_copyCacheDup (
    DDS_copyCache copyCache);

OS_API void
DDS_copyCacheFree (
    DDS_copyCache copyCache);

OS_API void
DDS_copyCacheIncRef (
    DDS_copyCache copyCache);

OS_API c_ulong
DDS_copyCacheLength (
    DDS_copyCache copyCache);

OS_API void *
DDS_copyCacheCache (
    DDS_copyCache copyCache);

OS_API void
DDS_copyCacheDump (
    DDS_copyCache copyCache);

OS_API unsigned long long
DDS_getUnionDescriptor (
    DDSCopyType ct,
    void         *src);

OS_API c_ulong
DDS_copyCacheGetUserSize (
    DDS_copyCache copyCache);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DDS_COPYCACHE_H */
