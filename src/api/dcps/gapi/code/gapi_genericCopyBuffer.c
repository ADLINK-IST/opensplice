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
#include "gapi_objManag.h"
#include "gapi_genericCopyBuffer.h"
#include "gapi_genericCopyOut.h"
#include "gapi_common.h"

#include "c_base.h"

#include "os_abstract.h"
#include "os_report.h"
#include "os_stdlib.h"

#include "gapi.h"

#define TRACE(function) /* function */
#define STATIC static

typedef struct gapi_genericBufferHeader_s {
    gapi_copyCache  copyCache;
    gapiCopyHeader *copyRoutine;
} gapi_genericBufferHeader;

typedef struct gapi_genericSeqBufferHeader_s {
    gapi_copyCache  copyCache;
    gapiCopyHeader *copyRoutine;
    unsigned int    seqLength;
    unsigned int    elementSize;
} gapi_genericSeqBufferHeader;

typedef void (*gapi_deallocatorFunc)(gapiCopyHeader *ch, void *ptr);

STATIC void gapi_genericFreeStruct       (gapiCopyHeader *ch, void *ptr);
STATIC void gapi_genericFreeUnion        (gapiCopyHeader *ch, void *ptr);
STATIC void gapi_genericFreeString       (gapiCopyHeader *ch, void *ptr);
STATIC void gapi_genericFreeArray        (gapiCopyHeader *ch, void *ptr);
STATIC void gapi_genericFreePrimSequence (gapiCopyHeader *ch, void *ptr);
STATIC void gapi_genericFreeSequence     (gapiCopyHeader *ch, void *ptr);
STATIC void gapi_genericFreeReference    (gapiCopyHeader *ch, void *ptr);

static gapi_deallocatorFunc deallocatorTable[] = {
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
    /* seq<Boolean> */ gapi_genericFreePrimSequence,
    /* seq<Byte>    */ gapi_genericFreePrimSequence,
    /* seq<Char>    */ gapi_genericFreePrimSequence,
    /* seq<Short>   */ gapi_genericFreePrimSequence,
    /* seq<Int>     */ gapi_genericFreePrimSequence,
    /* seq<long>    */ gapi_genericFreePrimSequence,
    /* seq<Float>   */ gapi_genericFreePrimSequence,
    /* seq<Double>  */ gapi_genericFreePrimSequence,
    /* seq<Enum>    */ gapi_genericFreePrimSequence,
    /* struct       */ gapi_genericFreeStruct,
    /* union        */ gapi_genericFreeUnion,
    /* string       */ gapi_genericFreeString,
    /* string<>     */ gapi_genericFreeString,
    /* array        */ gapi_genericFreeArray,
    /* sequence     */ gapi_genericFreeSequence,
    /* reference    */ gapi_genericFreeReference
    };

static unsigned int gapi_correctionTable[] = {
    /* BlackBox     */ 0,
    /* Boolean      */ 0,
    /* Byte         */ 0,
    /* Char         */ 0,
    /* Short        */ 0,
    /* Int          */ 0,
    /* Long         */ 0,
    /* Float        */ 0,
    /* Double       */ 0,
    /* Boolean[]    */ 0,
    /* Byte[]       */ 0,
    /* Char[]       */ 0,
    /* Short[]      */ 0,
    /* Int[]        */ 0,
    /* Long[]       */ 0,
    /* Float[]      */ 0,
    /* Double[]     */ 0,
    /* seq<Boolean> */ GAPI_SEQUENCE_CORRECTION,
    /* seq<Byte>    */ GAPI_SEQUENCE_CORRECTION,
    /* seq<Char>    */ GAPI_SEQUENCE_CORRECTION,
    /* seq<Short>   */ GAPI_SEQUENCE_CORRECTION,
    /* seq<Int>     */ GAPI_SEQUENCE_CORRECTION,
    /* seq<long>    */ GAPI_SEQUENCE_CORRECTION,
    /* seq<Float>   */ GAPI_SEQUENCE_CORRECTION,
    /* seq<Double>  */ GAPI_SEQUENCE_CORRECTION,
    /* seq<Enum>    */ GAPI_SEQUENCE_CORRECTION,
    /* struct       */ 0,
    /* union        */ 0,
    /* string       */ 0,
    /* string<>     */ 0,
    /* array        */ 0,
    /* sequence     */ GAPI_SEQUENCE_CORRECTION,
    /* reference    */ 0
    };

STATIC  gapiCopyType
to_copyType(c_type t)
{
    gapiCopyType ct = gapiBlackBox;
    switch(c_baseObject(t)->kind ) {
    case M_ENUMERATION:
        ct = gapiEnum;
    break;
    case M_PRIMITIVE:
        switch (c_primitive (t)->kind) {
        case P_BOOLEAN:
            ct = gapiBoolean;
        break;
        case P_CHAR:
            ct = gapiChar;
        break;
        case P_SHORT:
        case P_USHORT:
            ct = gapiShort;
        break;
        case P_LONG:
        case P_ULONG:
            ct = gapiInt;
        break;
        case P_LONGLONG:
        case P_ULONGLONG:
            ct = gapiLong;
        break;
        default:
            assert (0);
        }
    break;
    default:
        assert (0);
    }
    return ct;
}

STATIC unsigned long
gapi_genericGetUserSize (
    gapiCopyHeader *ch)
{
    unsigned long size = 0UL;

    switch (ch->copyType) {
        case gapiBlackBox:
            size = ((gapiCopyBlackBox *)ch)->size;
            break;
        case gapiBoolean:
            size = sizeof(gapi_boolean);
            break;
        case gapiByte:
            size = sizeof(gapi_octet);
            break;
        case gapiChar:
            size = sizeof(gapi_char);
            break;
        case gapiShort:
            size = sizeof(gapi_short);
            break;
        case gapiInt:
            size = sizeof(gapi_long);
            break;
        case gapiLong:
            size = sizeof(gapi_long_long);
            break;
        case gapiFloat:
            size = sizeof(gapi_float);
            break;
        case gapiDouble:
            size = sizeof(gapi_double);
            break;
        case gapiArrBoolean:
            size = sizeof(gapi_boolean) * ((gapiCopyArray *)ch)->size;
            break;
        case gapiArrByte:
            size = sizeof(gapi_octet) * ((gapiCopyArray *)ch)->size;
            break;
        case gapiArrChar:
            size = sizeof(gapi_char) * ((gapiCopyArray *)ch)->size;
            break;
        case gapiArrShort:
            size = sizeof(gapi_short) * ((gapiCopyArray *)ch)->size;
            break;
        case gapiArrInt:
            size = sizeof(gapi_long) * ((gapiCopyArray *)ch)->size;
            break;
        case gapiArrLong:
            size = sizeof(gapi_long_long) * ((gapiCopyArray *)ch)->size;
            break;
        case gapiArrFloat:
            size = sizeof(gapi_float) * ((gapiCopyArray *)ch)->size;
            break;
        case gapiArrDouble:
            size = sizeof(gapi_double) * ((gapiCopyArray *)ch)->size;
            break;
        case gapiSeqBoolean:
            size = sizeof(gapiSequenceType);
            break;
        case gapiSeqByte:
            size = sizeof(gapiSequenceType);
            break;
        case gapiSeqChar:
            size = sizeof(gapiSequenceType);
            break;
        case gapiSeqShort:
            size = sizeof(gapiSequenceType);
            break;
        case gapiSeqInt:
            size = sizeof(gapiSequenceType);
            break;
        case gapiSeqLong:
            size = sizeof(gapiSequenceType);
            break;
        case gapiSeqFloat:
            size = sizeof(gapiSequenceType);
            break;
        case gapiSeqDouble:
            size = sizeof(gapiSequenceType);
            break;
        case gapiEnum:
            size = sizeof(gapi_long);
            break;
        case gapiStruct:
            size = ((gapiCopyStruct *)ch)->userSize;
            break;
        case gapiUnion:
            size = ((gapiCopyUnion *)ch)->userSize;
            break;
        case gapiString:
            size = sizeof(gapi_char *);
            break;
        case gapiBString:
            size = sizeof(gapi_char *);
            break;
        case gapiArray:
            size = ((gapiCopyObjectArray *)ch)->typeSize *
                   ((gapiCopyObjectArray *)ch)->arraySize;
            break;
        case gapiSequence:
            size = sizeof(gapiSequenceType);
            break;
        case gapiRecursive:
            size = sizeof(char *);
            break;
        default:
            assert(0);
    }

    return size;
}



    /* Structured types */
STATIC void
gapi_genericFreeStruct (
    gapiCopyHeader *ch,
    void           *ptr)
{
    unsigned long         mi;
    gapiCopyStruct       *csh;
    gapiCopyStructMember *csm;
    void                 *buffer;
    int                   offset;
    int                   index;
    gapi_deallocatorFunc  deallocator;

    csh    = (gapiCopyStruct *)ch;
    csm    = gapiCopyStructMemberObject (csh);
    buffer = ptr;
    offset = 0;
    index  = 0;

    for (mi = 0; mi < csh->nrOfMembers; mi++) {
        ch = gapiCopyStructMemberDescription (csm);

        if ( offset < (int)(csm->memberOffset - index )) {
            offset = csm->memberOffset - index;
        }

        index = csm->memberOffset;

        buffer = (void *)((PA_ADDRCAST)buffer + offset);
        deallocator = deallocatorTable[ch->copyType];
        if (deallocator) {
            deallocator(ch, buffer);
        }
        offset = gapi_genericGetUserSize(ch);
        csm = (gapiCopyStructMember *)gapiCopyHeaderNextObject (ch);
    }

    memset(ptr, 0, csh->userSize);

    TRACE(printf ("Free Struct = 0x%x\n", ptr));
}

STATIC void
gapi_genericFreeUnion (
    gapiCopyHeader *ch,
    void           *ptr)
{
    unsigned long        co;
    gapiCopyUnion       *cuh;
    gapiCopyUnionLabels *csl;
    unsigned long long   discrVal;
    gapiCopyUnionLabels *defaultLabel = NULL;
    int                  active_case = 0;
    void                *buffer;
    gapi_deallocatorFunc deallocator;


    cuh = (gapiCopyUnion *)ch;

    discrVal = gapi_getUnionDescriptor(to_copyType(cuh->discrType), ptr);

    buffer = (void *)((PA_ADDRCAST)ptr + cuh->casesOffset);
    csl = gapiCopyUnionLabelsObject (cuh);
    co = 0;
    while (co < cuh->nrOfCases) {
        unsigned int label;
        gapiCopyUnionLabel *lab;

        lab = gapiCopyUnionLabelObject (csl);
        if (csl->labelCount) {
            for (label = 0; label < csl->labelCount; label++) {
                if (lab->labelVal == discrVal) {
                    active_case = 1;
                }
                lab++;
            }
        } else {
            defaultLabel = (gapiCopyUnionLabels *)gapiCopyUnionCaseObject(csl);
        }
        ch = gapiCopyUnionCaseObject(csl);
        if (active_case) {
            deallocator = deallocatorTable[ch->copyType];
            if (deallocator) {
                deallocator(ch, buffer);
            }
            co = cuh->nrOfCases;
        } else {
            co++;
        }
        csl = (gapiCopyUnionLabels *)gapiCopyHeaderNextObject (ch);
    }

    if (!active_case && defaultLabel) {
        ch = (gapiCopyHeader *)defaultLabel;
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
gapi_genericFreeString (
    gapiCopyHeader *ch,
    void           *ptr)
{
    gapi_string *dst = (gapi_string *)ptr;

    gapi_free(*dst);
    *dst = NULL;

    TRACE(printf ("Free string 0x%x\n", ptr));
}

    /* Array of object type */
STATIC void
gapi_genericFreeArray (
    gapiCopyHeader *ch,
    void           *ptr)
{
    gapiCopyObjectArray *ah;
    gapiCopyHeader      *aech;
    void                *buffer;
    unsigned int         offset;
    unsigned int         i;
    gapi_deallocatorFunc deallocator;

    ah          = (gapiCopyObjectArray *)ch;
    aech        = gapiCopyObjectArrayDescription (ah);
    offset      = ah->typeSize + (gapi_genericGetUserSize(aech) - ah->typeSize);
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
gapi_genericFreePrimSequence (
    gapiCopyHeader *ch,
    void           *ptr)
{
    gapiSequenceType *seq = (gapiSequenceType *)ptr;

    if (seq->_buffer) {
        gapi_free(seq->_buffer);
        seq->_buffer = NULL;
    }

    TRACE(printf ("Free prim sequence 0x%x\n", ptr));
}

STATIC void
gapi_genericFreeSequence (
    gapiCopyHeader *ch,
    void           *ptr)
{
    gapiCopyObjectSequence *sh;
    gapiCopyHeader         *sech;
    gapiSequenceType           *seq;
    gapi_deallocatorFunc    deallocator;

    seq         = (gapiSequenceType *)ptr;
    sh          = (gapiCopyObjectSequence *)ch;
    sech        = gapiCopyObjectSequenceDescription (sh);
    deallocator = deallocatorTable[sech->copyType];

    if (deallocator && seq->_buffer) {
        gapi_free(seq->_buffer);
        seq->_buffer = NULL;
    }

    TRACE(printf ("Free sequence 0x%x\n", ptr));
}

STATIC void
gapi_genericFreeReference (
    gapiCopyHeader *ch,
    void           *ptr)
{
    gapiCopyReference    *ref;
    gapiCopyHeader       *nch;
    gapi_deallocatorFunc  deallocator;

    ref = (gapiCopyReference *)ch;
    nch = gapiCopyReferencedObject (ref);
    deallocator = deallocatorTable[nch->copyType];

    if (deallocator) {
        deallocator(nch, ptr);
    }

    TRACE(printf ("Free reference 0x%x\n", ptr));
}

void
gapi_genericCopyBufferFreeType (
    gapiCopyHeader *ch,
    void           *ptr)
{
    gapi_deallocatorFunc deallocator;

    deallocator = deallocatorTable[ch->copyType];
    if (deallocator) {
        deallocator(ch, ptr);
    }
}



gapi_boolean
gapi_genericCopyBufferFree (
    void *buffer)
{
    gapi_genericBufferHeader *header;
    gapiCopyHeader           *ch;
    gapi_deallocatorFunc      deallocator;

    header = (gapi_genericBufferHeader *)gapi__header(buffer);

    assert(header);

    if (header) {
        ch = header->copyRoutine;
        deallocator = deallocatorTable[ch->copyType];
        if (deallocator) {
            deallocator(ch, buffer);
        }
        gapi_copyCacheFree(header->copyCache);
    }
    return TRUE;
}

gapi_boolean
gapi_genericCopySeqBufferFree (
    void *buffer)
{
    gapi_genericSeqBufferHeader *header;
    gapiCopyHeader              *ch;
    gapi_deallocatorFunc         deallocator;
    unsigned int                 i;

    header = (gapi_genericSeqBufferHeader *)gapi__header(buffer);

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
        gapi_copyCacheFree(header->copyCache);
    }
    return TRUE;
}

void *
gapi_genericCopyBufferAlloc (
    gapi_copyCache      copyCache,
    gapiCopyHeader     *copyRoutine,
    gapi_unsigned_long  size,
    gapi_unsigned_long  count)
{
    gapi_genericBufferHeader *header;
    void                     *buffer = NULL;

    assert(copyCache);
    assert(copyRoutine);
    assert(size > 0);
    assert(count > 0);

    if (copyCache && copyRoutine && (size > 0) && (count > 0)) {
        buffer = gapi__malloc(gapi_genericCopyBufferFree,
                              sizeof(gapi_genericBufferHeader), size * count);
        if (buffer) {
            header = (gapi_genericBufferHeader *)gapi__header(buffer);
            gapi_copyCacheIncRef(copyCache);
            header->copyCache   = copyCache;
            header->copyRoutine = copyRoutine;
        } else {
            OS_REPORT(OS_ERROR,
                      "gapi_genericCopyBufferAlloc", 0,
                      "memory allocation failed");
        }
    }

    return buffer;
}


void *
gapi_genericCopyBufferAllocSeqBuffer (
    gapi_copyCache      copyCache,
    gapiCopyHeader     *copyRoutine,
    gapi_unsigned_long  size,
    gapi_unsigned_long  count)
{
    gapi_genericSeqBufferHeader *header;
    void                        *buffer = NULL;

    assert(copyCache);
    assert(copyRoutine);
    assert(size > 0);
    assert(count > 0);

    if (copyCache && copyRoutine && (size > 0) && (count > 0)) {
        buffer = gapi__malloc(gapi_genericCopySeqBufferFree,
                              sizeof(gapi_genericSeqBufferHeader), size * count);
        if (buffer) {
            header = (gapi_genericSeqBufferHeader *)gapi__header(buffer);
            gapi_copyCacheIncRef(copyCache);
            header->copyCache   = copyCache;
            header->copyRoutine = copyRoutine;
            header->seqLength   = count;
            header->elementSize = size;
        } else {
            OS_REPORT(OS_ERROR,
                      "gapi_genericCopyBufferAlloc", 0,
                      "memory allocation failed");
        }
    }

    return buffer;
}


