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
#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"

#include "sac_objManag.h"
#include "sac_genericCopyBuffer.h"
#include "sac_genericCopyOut.h"

#include "c_base.h"

#include "os_abstract.h"
#include "sac_report.h"
#include "os_stdlib.h"

#define TRACE(function) /* function */
#define STATIC static

typedef struct {
    DDS_copyCache  copyCache;
    DDSCopyHeader *copyRoutine;
} DDS_genericBufferHeader;

typedef struct {
    DDS_copyCache  copyCache;
    DDSCopyHeader *copyRoutine;
    unsigned int    seqLength;
    unsigned int    elementSize;
} DDS_genericSeqBufferHeader;

typedef void (*DDS_deallocatorFunc)(DDSCopyHeader *ch, void *ptr);

STATIC void DDS_genericFreeStruct       (DDSCopyHeader *ch, void *ptr);
STATIC void DDS_genericFreeUnion        (DDSCopyHeader *ch, void *ptr);
STATIC void DDS_genericFreeString       (DDSCopyHeader *ch, void *ptr);
STATIC void DDS_genericFreeArray        (DDSCopyHeader *ch, void *ptr);
STATIC void DDS_genericFreePrimSequence (DDSCopyHeader *ch, void *ptr);
STATIC void DDS_genericFreeSequence     (DDSCopyHeader *ch, void *ptr);
STATIC void DDS_genericFreeReference    (DDSCopyHeader *ch, void *ptr);

static DDS_deallocatorFunc deallocatorTable[] = {
    /* BlackBox     */ NULL,
    /* Boolean      */ NULL,
    /* Byte         */ NULL,
    /* Char         */ NULL,
    /* Short        */ NULL,
    /* Int          */ NULL,
    /* Long         */ NULL,
    /* Float        */ NULL,
    /* Double       */ NULL,
    /* Boolean[]    */ NULL,
    /* Byte[]       */ NULL,
    /* Char[]       */ NULL,
    /* Short[]      */ NULL,
    /* Int[]        */ NULL,
    /* Long[]       */ NULL,
    /* Float[]      */ NULL,
    /* Double[]     */ NULL,
    /* seq<Boolean> */ DDS_genericFreePrimSequence,
    /* seq<Byte>    */ DDS_genericFreePrimSequence,
    /* seq<Char>    */ DDS_genericFreePrimSequence,
    /* seq<Short>   */ DDS_genericFreePrimSequence,
    /* seq<Int>     */ DDS_genericFreePrimSequence,
    /* seq<long>    */ DDS_genericFreePrimSequence,
    /* seq<Float>   */ DDS_genericFreePrimSequence,
    /* seq<Double>  */ DDS_genericFreePrimSequence,
    /* seq<Enum>    */ DDS_genericFreePrimSequence,
    /* struct       */ DDS_genericFreeStruct,
    /* union        */ DDS_genericFreeUnion,
    /* string       */ DDS_genericFreeString,
    /* string<>     */ DDS_genericFreeString,
    /* array        */ DDS_genericFreeArray,
    /* sequence     */ DDS_genericFreeSequence,
    /* reference    */ DDS_genericFreeReference
    };

STATIC  DDSCopyType
to_copyType(c_type t)
{
    DDSCopyType ct;
    switch(c_baseObject(t)->kind ) {
    case M_ENUMERATION:
        ct = DDSEnum;
    break;
    case M_PRIMITIVE:
        switch (c_primitive (t)->kind) {
        case P_BOOLEAN:
            ct = DDSBoolean;
        break;
        case P_CHAR:
            ct = DDSChar;
        break;
        case P_SHORT:
        case P_USHORT:
            ct = DDSShort;
        break;
        case P_LONG:
        case P_ULONG:
            ct = DDSInt;
        break;
        case P_LONGLONG:
        case P_ULONGLONG:
            ct = DDSLong;
        break;
        default:
            ct = DDSBlackBox;
            assert (0);
        }
    break;
    default:
        ct = DDSBlackBox;
        assert (0);
    }
    return ct;
}

static unsigned
DDS_genericGetUserSize (
    DDSCopyHeader *ch)
{
    size_t size = 0UL;

    switch (ch->copyType) {
    case DDSBlackBox:
        size = ((DDSCopyBlackBox *)ch)->size;
    break;
    case DDSBoolean:
        size = sizeof(DDS_boolean);
    break;
    case DDSByte:
        size = sizeof(DDS_octet);
    break;
    case DDSChar:
    case DDSString:
    case DDSBString:
        size = sizeof(DDS_char);
    break;
    case DDSShort:
        size = sizeof(DDS_short);
    break;
    case DDSInt:
    case DDSEnum:
        size = sizeof(DDS_long);
    break;
    case DDSLong:
        size = sizeof(DDS_long_long);
    break;
    case DDSFloat:
        size = sizeof(DDS_float);
    break;
    case DDSDouble:
        size = sizeof(DDS_double);
    break;
    case DDSArrBoolean:
        size = sizeof(DDS_boolean) * ((DDSCopyArray *)ch)->size;
    break;
    case DDSArrByte:
        size = sizeof(DDS_octet) * ((DDSCopyArray *)ch)->size;
    break;
    case DDSArrChar:
        size = sizeof(DDS_char) * ((DDSCopyArray *)ch)->size;
    break;
    case DDSArrShort:
        size = sizeof(DDS_short) * ((DDSCopyArray *)ch)->size;
    break;
    case DDSArrInt:
        size = sizeof(DDS_long) * ((DDSCopyArray *)ch)->size;
    break;
    case DDSArrLong:
        size = sizeof(DDS_long_long) * ((DDSCopyArray *)ch)->size;
    break;
    case DDSArrFloat:
        size = sizeof(DDS_float) * ((DDSCopyArray *)ch)->size;
    break;
    case DDSArrDouble:
        size = sizeof(DDS_double) * ((DDSCopyArray *)ch)->size;
    break;
    case DDSSeqBoolean:
    case DDSSeqByte:
    case DDSSeqChar:
    case DDSSeqShort:
    case DDSSeqInt:
    case DDSSeqLong:
    case DDSSeqFloat:
    case DDSSeqDouble:
    case DDSSequence:
        size = sizeof(DDSSequenceType);
    break;
    case DDSStruct:
        size = ((DDSCopyStruct *)ch)->userSize;
    break;
    case DDSUnion:
        size = ((DDSCopyUnion *)ch)->userSize;
    break;
    case DDSArray:
        size = ((DDSCopyObjectArray *)ch)->typeSize *
               ((DDSCopyObjectArray *)ch)->arraySize;
    break;
    case DDSRecursive:
        size = sizeof(char *);
    break;
    default:
        assert(0);
    }
    assert (size == (unsigned) size);
    return (unsigned) size;
}



    /* Structured types */
STATIC void
DDS_genericFreeStruct (
    DDSCopyHeader *ch,
    void           *ptr)
{
    unsigned long         mi;
    DDSCopyStruct       *csh;
    DDSCopyStructMember *csm;
    void                 *buffer;
    int                   offset;
    int                   index;
    DDS_deallocatorFunc  deallocator;

    csh    = (DDSCopyStruct *)ch;
    csm    = DDSCopyStructMemberObject (csh);
    buffer = ptr;
    offset = 0;
    index  = 0;

    for (mi = 0; mi < csh->nrOfMembers; mi++) {
        ch = DDSCopyStructMemberDescription (csm);

        if ( offset < (int)(csm->memberOffset - index )) {
            offset = csm->memberOffset - index;
        }

        index = csm->memberOffset;

        buffer = (void *)((PA_ADDRCAST)buffer + offset);
        deallocator = deallocatorTable[ch->copyType];
        if (deallocator) {
            deallocator(ch, buffer);
        }
        offset = DDS_genericGetUserSize(ch);
        csm = (DDSCopyStructMember *)DDSCopyHeaderNextObject (ch);
    }

    memset(ptr, 0, csh->userSize);

    TRACE(printf ("Free Struct = 0x%x\n", ptr));
}

STATIC void
DDS_genericFreeUnion (
    DDSCopyHeader *ch,
    void           *ptr)
{
    unsigned long        co;
    DDSCopyUnion       *cuh;
    DDSCopyHeader      *cdh;
    DDSCopyUnionLabels *csl;
    unsigned long long   discrVal;
    DDSCopyUnionLabels *defaultLabel = NULL;
    int                  active_case = 0;
    void                *buffer;
    DDS_deallocatorFunc deallocator;


    cuh = (DDSCopyUnion *)ch;

    discrVal = DDS_getUnionDescriptor(to_copyType(cuh->discrType), ptr);

    buffer = (void *)((PA_ADDRCAST)ptr + cuh->casesOffset);
    cdh = DDSCopyUnionDiscriminantObject (cuh);
    csl = DDSCopyUnionLabelsObject (cdh);
    co = 0;
    while (co < cuh->nrOfCases) {
        unsigned int label;
        DDSCopyUnionLabel *lab;

        lab = DDSCopyUnionLabelObject (csl);
        if (csl->labelCount) {
            for (label = 0; label < csl->labelCount; label++) {
                if (lab->labelVal == discrVal) {
                    active_case = 1;
                }
                lab++;
            }
        } else {
            defaultLabel = (DDSCopyUnionLabels *)DDSCopyUnionCaseObject(csl);
        }
        ch = DDSCopyUnionCaseObject(csl);
        if (active_case) {
            deallocator = deallocatorTable[ch->copyType];
            if (deallocator) {
                deallocator(ch, buffer);
            }
            co = cuh->nrOfCases;
        } else {
            co++;
        }
        csl = (DDSCopyUnionLabels *)DDSCopyHeaderNextObject (ch);
    }

    if (!active_case && defaultLabel) {
        ch = (DDSCopyHeader *)defaultLabel;
        deallocator = deallocatorTable[ch->copyType];
        if (deallocator) {
            deallocator(ch, buffer);
        }
    }

    memset(ptr, 0, cuh->userSize);

    TRACE(printf ("Free Union 0x%x\n", ptr));
}

    /* String types */
STATIC void
DDS_genericFreeString (
    DDSCopyHeader *ch,
    void           *ptr)
{
    DDS_string *dst = (DDS_string *)ptr;

    OS_UNUSED_ARG(ch);

    DDS_free(*dst);
    *dst = NULL;

    TRACE(printf ("Free string 0x%x\n", ptr));
}

    /* Array of object type */
STATIC void
DDS_genericFreeArray (
    DDSCopyHeader *ch,
    void           *ptr)
{
    DDSCopyObjectArray *ah;
    DDSCopyHeader      *aech;
    void                *buffer;
    unsigned int         offset;
    unsigned int         i;
    DDS_deallocatorFunc deallocator;

    ah          = (DDSCopyObjectArray *)ch;
    aech        = DDSCopyObjectArrayDescription (ah);
    offset      = ah->typeSize + (DDS_genericGetUserSize(aech) - ah->typeSize);
    deallocator = deallocatorTable[aech->copyType];

    if (deallocator) {
        buffer = ptr;
        for (i = 0; i < ah->arraySize; i++) {
            deallocator(aech, buffer);
            buffer = (void *)((PA_ADDRCAST)buffer + offset);
        }
    }

    TRACE(printf ("Free array 0x%x\n", ptr));
}

STATIC void
DDS_genericFreePrimSequence (
    DDSCopyHeader *ch,
    void           *ptr)
{
    DDSSequenceType *seq = (DDSSequenceType *)ptr;

    OS_UNUSED_ARG(ch);

    if (seq->_buffer) {
        DDS_free(seq->_buffer);
        seq->_buffer = NULL;
    }

    TRACE(printf ("Free prim sequence 0x%x\n", ptr));
}

STATIC void
DDS_genericFreeSequence (
    DDSCopyHeader *ch,
    void           *ptr)
{
    DDSCopyObjectSequence *sh;
    DDSCopyHeader *sech;
    DDSSequenceType *seq;
    DDS_deallocatorFunc deallocator;

    seq         = (DDSSequenceType *)ptr;
    sh          = (DDSCopyObjectSequence *)ch;
    sech        = DDSCopyObjectSequenceDescription (sh);
    deallocator = deallocatorTable[sech->copyType];

    if (deallocator && seq->_buffer) {
        DDS_free(seq->_buffer);
        seq->_buffer = NULL;
    }

    TRACE(printf ("Free sequence 0x%x\n", ptr));
}

STATIC void
DDS_genericFreeReference (
    DDSCopyHeader *ch,
    void           *ptr)
{
    DDSCopyReference    *ref;
    DDSCopyHeader       *nch;
    DDS_deallocatorFunc  deallocator;

    ref = (DDSCopyReference *)ch;
    nch = DDSCopyReferencedObject (ref);
    deallocator = deallocatorTable[nch->copyType];

    if (deallocator) {
        deallocator(nch, ptr);
    }

    TRACE(printf ("Free reference 0x%x\n", ptr));
}

void
DDS_genericCopyBufferFreeType (
    DDSCopyHeader *ch,
    void           *ptr)
{
    DDS_deallocatorFunc deallocator;

    deallocator = deallocatorTable[ch->copyType];
    if (deallocator) {
        deallocator(ch, ptr);
    }
}



DDS_ReturnCode_t
DDS_genericCopyBufferFree (
    void *buffer)
{
    DDS_genericBufferHeader *header;
    DDSCopyHeader           *ch;
    DDS_deallocatorFunc      deallocator;

    header = (DDS_genericBufferHeader *)DDS__header(buffer);

    assert(header);

    if (header) {
        ch = header->copyRoutine;
        deallocator = deallocatorTable[ch->copyType];
        if (deallocator) {
            deallocator(ch, buffer);
        }
        DDS_copyCacheFree(header->copyCache);
    }
    return DDS_RETCODE_OK;
}

static DDS_ReturnCode_t
DDS_genericCopySeqBufferFree (
    void *buffer)
{
    DDS_genericSeqBufferHeader *header;
    DDSCopyHeader              *ch;
    DDS_deallocatorFunc         deallocator;
    unsigned int                 i;

    header = (DDS_genericSeqBufferHeader *)DDS__header(buffer);

    assert(header);

    if (header) {
        ch = header->copyRoutine;
        deallocator = deallocatorTable[ch->copyType];
        if (deallocator) {
            for ( i = 0; i < header->seqLength; i++ ) {
                deallocator(ch, buffer);
                buffer = (void *)((PA_ADDRCAST)buffer + header->elementSize);
            }
        }
        DDS_copyCacheFree(header->copyCache);
    }
    return DDS_RETCODE_OK;
}

void *
DDS_genericCopyBufferAlloc (
    DDS_copyCache      copyCache,
    DDSCopyHeader     *copyRoutine,
    DDS_unsigned_long  size,
    DDS_unsigned_long  count)
{
    DDS_genericBufferHeader *header;
    void                     *buffer = NULL;

    assert(copyCache);
    assert(copyRoutine);
    assert(size > 0);
    assert(count > 0);

    if (copyCache && copyRoutine && (size > 0) && (count > 0)) {
        buffer = DDS__malloc((DDS_deallocatorType) DDS_genericCopyBufferFree,
                              sizeof(DDS_genericBufferHeader), size * count);
        if (buffer) {
            header = (DDS_genericBufferHeader *)DDS__header(buffer);
            DDS_copyCacheIncRef(copyCache);
            header->copyCache   = copyCache;
            header->copyRoutine = copyRoutine;
        }
    }

    return buffer;
}


void *
DDS_genericCopyBufferAllocSeqBuffer (
    DDS_copyCache      copyCache,
    DDSCopyHeader     *copyRoutine,
    DDS_unsigned_long  size,
    DDS_unsigned_long  count)
{
    DDS_genericSeqBufferHeader *header;
    void                        *buffer = NULL;

    assert(copyCache);
    assert(copyRoutine);
    assert(size > 0);
    assert(count > 0);

    if (copyCache && copyRoutine && (size > 0) && (count > 0)) {
        buffer = DDS__malloc((DDS_deallocatorType) DDS_genericCopySeqBufferFree,
                              sizeof(DDS_genericSeqBufferHeader), size * count);
        if (buffer) {
            header = (DDS_genericSeqBufferHeader *)DDS__header(buffer);
            DDS_copyCacheIncRef(copyCache);
            header->copyCache   = copyCache;
            header->copyRoutine = copyRoutine;
            header->seqLength   = count;
            header->elementSize = size;
        }
    }

    return buffer;
}


