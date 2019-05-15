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
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include "sac_object.h"
#include "sac_report.h"
#include "c_base.h"
#include "c_iterator.h"

#include "sac_genericCopyBuffer.h"
#include "sac_genericCopyCache.h"

#define TRACE(function)     /* function */
#define CACHE_BLOCKSIZE     (100)

#define DDSCopyHeader_init(_this,_type,_size) \
        ((DDSCopyHeader *)_this)->copyType = _type; \
        ((DDSCopyHeader *)_this)->size = _size

C_CLASS(DDS_context);
#define DDS_context(o)     ((DDS_context)(o))

C_STRUCT(DDS_context) {
    DDS_copyCache  copyCache;
    c_iter      typeStack;
};

C_STRUCT(DDS_copyCache) {
    void         *cache;   /* start of cache  */
    c_ulong      length;   /* length of cache */
    c_ulong      iWrite;   /* write index     */
    unsigned int refCount;
    unsigned int userSize;
};

C_CLASS(DDS_typeHistory);
#define DDS_typeHistory(o) ((DDS_typeHistory)(o))

C_STRUCT(DDS_typeHistory) {
    c_metaObject    metaObject;
    c_ulong      cacheIndex;
};

DDS_typeHistory
DDS_typeHistoryNew (
    const c_metaObject metaObject,
    c_long index)
{
    DDS_typeHistory history = os_malloc (C_SIZEOF(DDS_typeHistory));
    history->metaObject = metaObject;
    history->cacheIndex = index;
    return history;
}

static c_long DDS_copyCacheWrite(DDS_copyCache copyCache,
                                  void *data, c_long size);
static void DDS_metaObjectBuild(c_type o, DDS_context context);
static void DDS_cacheStructMember(c_member o, DDS_context context);
static void DDS_cacheUnionLabel(c_literal lit, DDS_context ctx);
static unsigned int DDS_cacheObjectUserSize(c_type o);

#if 0
static void
cacheDump (
    DDSCopyHeader *ch,
    unsigned int level)
{
    unsigned int mi;
    DDSCopyStruct *csh;
    DDSCopyStructMember *csm;
    DDSCopyObjectArray *oah;
    DDSCopyType ct;
    unsigned int l;

    ct = ch->copyType;
    for (l = 0; l < level; l++) { printf ("  "); }
    printf ("  T:%d S:%d\n", ct, ch->size);
    for (l = 0; l < level; l++) { printf ("  "); }
    switch (ct) {
    case DDSBoolean: printf ("  Boolean\n"); break;
    case DDSByte: printf ("  Byte\n"); break;
    case DDSChar: printf ("  Char\n"); break;
    case DDSShort: printf ("  Short\n"); break;
    case DDSInt: printf ("  Int\n"); break;
    case DDSLong: printf ("  Long\n"); break;
    case DDSFloat: printf ("  Float\n"); break;
    case DDSDouble: printf ("  Double\n"); break;
    case DDSArrBoolean: printf ("  ArrBoolean\n"); break;
    case DDSArrByte: printf ("  ArrByte\n"); break;
    case DDSArrChar: printf ("  ArrChar\n"); break;
    case DDSArrShort: printf ("  ArrShort\n"); break;
    case DDSArrInt: printf ("  ArrInt\n"); break;
    case DDSArrLong: printf ("  ArrLong\n"); break;
    case DDSArrFloat: printf ("  ArrFloat\n"); break;
    case DDSArrDouble: printf ("  ArrDouble\n"); break;
    case DDSSeqBoolean: printf ("  SeqBoolean\n"); break;
    case DDSSeqByte: printf ("  SeqByte\n"); break;
    case DDSSeqChar: printf ("  SeqChar\n"); break;
    case DDSSeqShort: printf ("  SeqShort\n"); break;
    case DDSSeqInt: printf ("  SeqInt\n"); break;
    case DDSSeqLong: printf ("  SeqLong\n"); break;
    case DDSSeqFloat: printf ("  SeqFloat\n"); break;
    case DDSSeqDouble: printf ("  SeqDouble\n"); break;
    case DDSString: printf ("  String\n"); break;
    case DDSBString: printf ("  BString\n"); break;
    case DDSEnum: printf ("  Enum\n"); break;
    case DDSUnion: printf ("  Union\n"); break;
    case DDSSequence: printf ("  Sequence\n"); break;
    case DDSRecursive: printf ("  Recursive\n"); break;
    case DDSBlackBox:
        printf ("  BlackBox (%d bytes)\n",((DDSCopyBlackBox *)ch)->size);
    break;
    case DDSStruct:
        csh = (DDSCopyStruct *)ch; printf ("  Struct\n");
        for (l = 0; l < level; l++) { printf ("  "); }
        printf ("  M#:%d \n", csh->nrOfMembers);
        csm = (DDSCopyStructMember *)((PA_ADDRCAST)csh +
                                       sizeof (DDSCopyStruct));
        for (mi = 0; mi < csh->nrOfMembers; mi++) {
            for (l = 0; l < level; l++) { printf ("  "); }
            printf ("  M@:%d \n", csm->memberOffset);
            ch = (DDSCopyHeader *)((PA_ADDRCAST)csm +
                                    sizeof (DDSCopyStructMember));
            cacheDump (ch, level+1);
            csm = (DDSCopyStructMember *)((PA_ADDRCAST)ch + ch->size);
        }
    break;
    case DDSArray:
        oah = (DDSCopyObjectArray *)ch;
        printf ("  Array\n");
        for (l = 0; l < level; l++) { printf ("  "); }
        printf ("  E#:%d TS:%d\n", oah->arraySize, oah->typeSize);
        cacheDump ((DDSCopyHeader *)((PA_ADDRCAST)oah +
                                      sizeof(DDSCopyObjectArray)), level+1);
    break;
    }
}
#endif

static int
DDS_isDefinedInScope (
    c_collectionType type)
{
    c_metaObject scope;
    int inScope = 0;

    scope = c_metaObject(type)->definedIn;
    while (scope) {
        if (scope == c_metaObject(c_typeActualType(type->subType))) {
            inScope = 1;
            scope = NULL;
        } else {
            scope = scope->definedIn;
        }
    }
    return inScope;
}

static int
DDS_headerIndex (
    c_metaObject type,
    DDS_context ctx)
{
    c_ulong i;
    DDS_typeHistory history;

    for (i = 0; i < c_iterLength (ctx->typeStack); i++) {
        history = c_iterObject (ctx->typeStack, i);
        if (history->metaObject == type) {
            return history->cacheIndex;
        }
    }
    assert (0);
    return 0;
}

static unsigned int
userSizeCorrection (
    c_type o)
{
    unsigned int size = 0;
    unsigned int cs;
    c_type actual, subType;
    c_ulong i;
    c_member member;
    c_unionCase ucase;

    actual = c_typeActualType(o);

    switch ( c_baseObjectKind(actual) ) {
    case M_STRUCTURE:
        for (i = 0; i < c_structureMemberCount(actual); i++) {
            member = c_structureMember(actual, i);
            size += userSizeCorrection(c_memberType(member));
        }
    break;
    case M_UNION: {
        unsigned int unionValueSizeUser = 0;
        unsigned int unionValueSizeSHM = 0;
        unsigned int align = 1;
        for (i = 0; i < c_unionUnionCaseCount(actual); i++) {
            unsigned a;
            ucase = c_unionUnionCase(actual, i);
            a = c_unionCaseType (ucase)->alignment;
            if (a > align) {
                align = a;
            }
        }
        for (i = 0; i < c_unionUnionCaseCount(actual); i++) {
            ucase = c_unionUnionCase(actual, i);
            cs = DDS_cacheObjectUserSize(c_unionCaseType(ucase));
            if (cs > unionValueSizeUser) {
                unionValueSizeUser = cs;
            }
            cs = c_unionCaseType(ucase)->size;
            if (cs > unionValueSizeSHM) {
                unionValueSizeSHM = cs;
            }
        }
        size = ((unionValueSizeUser + align - 1) & ~(align - 1)) - ((unionValueSizeSHM + align - 1) & ~(align - 1));
    break;
    }
    case M_COLLECTION:
        switch ( c_collectionTypeKind(actual) ) {
        case OSPL_C_SEQUENCE:
            size = DDS_SEQUENCE_CORRECTION;
        break;
        case OSPL_C_ARRAY:
            subType = c_typeActualType(c_collectionTypeSubType(actual));
            size = c_collectionTypeMaxSize(actual) * userSizeCorrection(subType);
        break;
        default:
        break;
        }
    break;
    default:
    break;
    }
    return size;
}

static unsigned int
DDS_cacheObjectUserSize (
    c_type o)
{
    c_type actual = c_typeActualType(o);
    size_t size = actual->size + userSizeCorrection(actual);
    TRACE (printf("DDS_cacheObjectUserSize actual->size = %"PA_PRIuSIZE", total = %"PA_PRIuSIZE"\n", actual->size, size));
    assert (size == (unsigned) size);
    return (unsigned) size;
}

DDS_copyCache
DDS_copyCacheNew (
    c_metaObject object)
{
    DDS_context context;
    DDS_copyCache copyCache = NULL;

    assert (object);

    context = os_malloc(C_SIZEOF(DDS_context));
    copyCache = os_malloc(C_SIZEOF(DDS_copyCache));
    copyCache->cache = os_malloc(CACHE_BLOCKSIZE);
    copyCache->length = CACHE_BLOCKSIZE;
    copyCache->iWrite = 0;
    copyCache->refCount = 1;
    context->copyCache = copyCache;
    context->typeStack = c_iterNew (NULL);
    DDS_metaObjectBuild (c_type(object), context);
    if (copyCache->iWrite < copyCache->length) {
        void *exactCache = os_malloc (copyCache->iWrite);
        memcpy (exactCache, copyCache->cache, copyCache->iWrite);
        os_free (copyCache->cache);
        copyCache->cache = exactCache;
        copyCache->length = copyCache->iWrite;
    }
    copyCache->userSize = DDS_cacheObjectUserSize(c_type(object));
    c_iterFree (context->typeStack);
    TRACE (cacheDump(context->copyCache, 0));
    os_free(context);
    return copyCache;
}

DDS_copyCache
DDS_copyCacheDup (
    DDS_copyCache copyCache)
{
    DDS_copyCache new_cc = NULL;

    if (copyCache){
        new_cc = os_malloc (C_SIZEOF(DDS_copyCache));
        new_cc->cache = os_malloc(copyCache->length);
        memcpy(new_cc->cache,copyCache->cache,copyCache->length);
        new_cc->length   = copyCache->length;
        new_cc->iWrite   = copyCache->iWrite;
        new_cc->refCount = 1;
    }
    return new_cc;
}

void
DDS_copyCacheFree (
    DDS_copyCache copyCache)
{
    assert (copyCache);
    assert(copyCache->refCount > 0);

    copyCache->refCount--;
    if ( copyCache->refCount <= 0 ) {
        os_free (copyCache->cache);
        os_free (copyCache);
    }
}

void
DDS_copyCacheIncRef (
    DDS_copyCache copyCache)
{
    assert (copyCache);
    assert(copyCache->refCount > 0);

    copyCache->refCount++;
}

c_ulong
DDS_copyCacheLength (
    DDS_copyCache copyCache)
{
    assert (copyCache);

    return copyCache->iWrite;
}

void *
DDS_copyCacheCache (
    DDS_copyCache copyCache)
{
    assert (copyCache);

    return copyCache->cache;
}

c_long
DDS_copyCacheWrite (
    DDS_copyCache copyCache,
    void *data,
    c_long size)
{
    c_long additionalLength;
    void *newCache;
    c_long writeIndex = copyCache->iWrite;

    assert (copyCache);
    assert (data);
    assert (size);

    if ((copyCache->iWrite + size) >= copyCache->length) {
        /* If new data does not fit, allocate new cache */
        additionalLength = ((copyCache->iWrite + size)/CACHE_BLOCKSIZE) * CACHE_BLOCKSIZE;
        newCache = os_malloc (copyCache->iWrite + additionalLength);
        memcpy (newCache, copyCache->cache, copyCache->iWrite);
        os_free (copyCache->cache);
        copyCache->cache = newCache;
        copyCache->length = copyCache->iWrite + additionalLength;
    }
    memcpy ((void *)((PA_ADDRCAST)copyCache->cache + copyCache->iWrite), data, size);
    copyCache->iWrite += size;
    return writeIndex;
}

void
DDS_copyCacheUpdateSize (
    DDS_copyCache copyCache,
    c_ulong headerIndex)
{
    unsigned length;
    DDSCopyHeader *hAddr;

    assert (copyCache);

    hAddr = (DDSCopyHeader *)((PA_ADDRCAST)copyCache->cache + headerIndex);
    length = copyCache->iWrite - headerIndex;
    assert (length == (unsigned short) length);
    hAddr->size = (unsigned short) length;
}

void
DDS_copyCacheBackReference (
    DDS_copyCache copyCache,
    c_ulong headerIndex)
{
    DDSCopyReference ref;

    ref.header.copyType = DDSRecursive;
    ref.refIndex = (unsigned int)(copyCache->iWrite - headerIndex);
    headerIndex = DDS_copyCacheWrite (copyCache, &ref, sizeof(ref));
    DDS_copyCacheUpdateSize (copyCache, headerIndex);
}

unsigned long long
DDS_getUnionDescriptor (
    DDSCopyType ct,
    void *src)
{
    unsigned long long discrVal = 0ULL;

    switch ( ct ) {
    case DDSBoolean: discrVal = *(DDS_boolean *)src;   break;
    case DDSByte:    discrVal = *(DDS_octet *)src;     break;
    case DDSChar:    discrVal = *(DDS_char *)src;      break;
    case DDSShort:   discrVal = *(DDS_short *)src;     break;
    case DDSInt:     discrVal = *(DDS_long *)src;      break;
    case DDSLong:    discrVal = *(DDS_long_long *)src; break;
    case DDSEnum:    discrVal = *(DDS_long *)src;      break;
    default:
        SAC_REPORT(DDS_RETCODE_ERROR, "Illegal DDSCopyType (%d) detected.",
                    ct);
        assert(0);
    break;
    }
    return discrVal;
}


c_ulong
DDS_copyCacheGetUserSize (
    DDS_copyCache copyCache)
{
    return copyCache->userSize;
}

static void
DDS_metaObjectBuild(
    c_type o,
    DDS_context context)
{
    DDSCopyType kind;
    c_type type;

    assert (o);
    assert (context);

    /* catch all unsupported types */
    switch (c_baseObjectKind(o)) {
    case M_UNDEFINED:
    case M_ATTRIBUTE:
    case M_CLASS:
    case M_EXCEPTION:
    case M_INTERFACE:
    case M_EXPRESSION:
    case M_OPERATION:
    case M_RELATION:
    case M_PARAMETER:
    case M_BASE:
    case M_COUNT:
    case M_CONSTOPERAND:
    case M_LITERAL:
    case M_MEMBER:
    case M_UNIONCASE:
    case M_CONSTANT:
    case M_MODULE:
        SAC_REPORT(DDS_RETCODE_ERROR, "Unsupported type (%d) detected.",
                    c_baseObjectKind(o));
        assert (FALSE);
    break;
    default:
    break;
    }

    if( !c_typeHasRef(o) ) {
        DDSCopyBlackBox blackBox;
        TRACE (printf ("BlackBox (%d bytes)\n", o->size));
        DDSCopyHeader_init ((DDSCopyHeader *)&blackBox, DDSBlackBox, sizeof(blackBox));
        assert (o->size == (unsigned) o->size);
        blackBox.size = (unsigned) o->size;
        DDS_copyCacheWrite (context->copyCache, &blackBox, sizeof(blackBox));
        return;
    }

    switch (c_baseObjectKind(o)) {
    case M_PRIMITIVE:
    {
        DDSCopyHeader header;

        switch (c_primitiveKind(o)) {
        case P_BOOLEAN:   kind = DDSBoolean; break;
        case P_SHORT:
        case P_USHORT:    kind = DDSShort;   break;
        case P_LONG:
        case P_ULONG:     kind = DDSInt;     break;
        case P_LONGLONG:
        case P_ULONGLONG: kind = DDSLong;    break;
        case P_CHAR:
        case P_OCTET:     kind = DDSChar;    break;
        case P_FLOAT:     kind = DDSFloat;   break;
        case P_DOUBLE:    kind = DDSDouble;  break;
        default:
            SAC_REPORT(DDS_RETCODE_ERROR, "Illegal primitive type (%d) detected.", c_primitiveKind(o));
            kind = DDSBlackBox;
            assert (FALSE);
        }
        DDSCopyHeader_init(&header, kind, sizeof(header));
        DDS_copyCacheWrite(context->copyCache, &header, sizeof(header));
    }
    break;
    case M_COLLECTION:
        if (c_collectionTypeKind(o) == OSPL_C_STRING) {
            if (c_collectionTypeMaxSize(o) > 0) {
                DDSCopyBoundedString header;
                TRACE (printf ("BString\n"));
                DDSCopyHeader_init ((DDSCopyHeader *)&header, DDSBString, sizeof(header));
                header.max = c_collectionTypeMaxSize(o);
                DDS_copyCacheWrite (context->copyCache, &header, sizeof(header));
            } else {
                DDSCopyHeader header;
                TRACE (printf ("String\n"));
                DDSCopyHeader_init (&header, DDSString, sizeof(header));
                DDS_copyCacheWrite (context->copyCache, &header, sizeof(header));
            }
        } else if (c_collectionTypeKind(o) == OSPL_C_SEQUENCE) {
            c_collectionType cType = c_collectionType(o);
            type = c_typeActualType(cType->subType);
            if (c_baseObjectKind(type) == M_PRIMITIVE) {
                DDSCopySequence header;

                switch (c_primitiveKind(type)) {
                case P_BOOLEAN:   kind = DDSSeqBoolean; break;
                case P_SHORT:
                case P_USHORT:    kind = DDSSeqShort;   break;
                case P_LONG:
                case P_ULONG:     kind = DDSSeqInt;     break;
                case P_LONGLONG:
                case P_ULONGLONG: kind = DDSSeqLong;    break;
                case P_CHAR:
                case P_OCTET:     kind = DDSSeqChar;    break;
                case P_FLOAT:     kind = DDSSeqFloat;   break;
                case P_DOUBLE:    kind = DDSSeqDouble;  break;
                default:
                    SAC_REPORT(DDS_RETCODE_ERROR, "Illegal collection type (%d) detected.", c_collectionTypeKind(o));
                    kind = DDSBlackBox;
                    assert (FALSE);
                }
                DDSCopyHeader_init((DDSCopyHeader *)&header, kind, sizeof(header));
                header.type = c_typeActualType(c_collectionTypeSubType(o));
                header.size = c_collectionTypeMaxSize(o);
                DDS_copyCacheWrite(context->copyCache, &header, sizeof(header));
            } else {
                c_collectionType cType = c_collectionType(o);
                DDSCopyObjectSequence objectSeqHeader;
                c_long headerIndex;

                type = c_typeActualType(cType->subType);
                objectSeqHeader.seqSize = c_collectionTypeMaxSize(o);
                objectSeqHeader.type = type;
                assert (type->size == (unsigned) type->size);
                objectSeqHeader.baseTypeSize = (unsigned) type->size;
                objectSeqHeader.userTypeSize = DDS_cacheObjectUserSize(type);

                TRACE (printf ("SEQUENCE\n    size %d\n", cType->maxSize));

                DDSCopyHeader_init ((DDSCopyHeader *)&objectSeqHeader, DDSSequence, sizeof(objectSeqHeader));
                headerIndex = DDS_copyCacheWrite (context->copyCache, &objectSeqHeader, sizeof(objectSeqHeader));
                /* Check if the subtype is in the scope of the subtype,
                 * in that case it is a recursive type.
                 */
                if (DDS_isDefinedInScope (cType)) {
                    DDS_copyCacheBackReference (context->copyCache, DDS_headerIndex(c_metaObject(type), context));
                } else {
                    DDS_metaObjectBuild(type, context);
                }
                DDS_copyCacheUpdateSize(context->copyCache, headerIndex);
            }
        } else if (c_collectionTypeKind(o) == OSPL_C_ARRAY) {
            DDSCopyArray header;
            type = c_typeActualType(c_collectionTypeSubType(o));
            /* array */
            if (c_baseObjectKind(type) == M_PRIMITIVE) {
                switch (c_primitiveKind(type)) {
                case P_BOOLEAN:   kind = DDSArrBoolean; break;
                case P_SHORT:
                case P_USHORT:    kind = DDSArrShort;   break;
                case P_LONG:
                case P_ULONG:     kind = DDSArrInt;     break;
                case P_LONGLONG:
                case P_ULONGLONG: kind = DDSArrLong;    break;
                case P_CHAR:
                case P_OCTET:     kind = DDSArrChar;    break;
                case P_FLOAT:     kind = DDSArrFloat;   break;
                case P_DOUBLE:    kind = DDSArrDouble;  break;
                default:
                    SAC_REPORT(DDS_RETCODE_ERROR, "Illegal collection type (%d) detected.", c_collectionTypeKind(o));
                    kind = DDSBlackBox;
                    assert (FALSE);
                }
                DDSCopyHeader_init((DDSCopyHeader *)&header, kind, sizeof(header));
                header.size = c_collectionTypeMaxSize(o);
                DDS_copyCacheWrite (context->copyCache, &header, sizeof(header));
            } else {
                DDSCopyObjectArray header;
                c_long headerIndex;

                type = c_typeActualType(c_collectionTypeSubType(o));
                header.arraySize = c_collectionTypeMaxSize(o);
                assert (c_typeSize(type) == (unsigned)c_typeSize(type));
                header.typeSize = (unsigned)c_typeSize(type);

                TRACE (printf ("Array\n    size %d\n", header.arraySize));

                DDSCopyHeader_init ((DDSCopyHeader *)&header, DDSArray, sizeof(header));
                headerIndex = DDS_copyCacheWrite (context->copyCache, &header, sizeof(header));

                DDS_metaObjectBuild (type, context);
                DDS_copyCacheUpdateSize (context->copyCache, headerIndex);
            }
        }
    break;
    case M_ENUMERATION:
    {
        DDSCopyEnum copyEnum;

        DDSCopyHeader_init (&copyEnum.header, DDSEnum, sizeof(copyEnum));
        copyEnum.nrOfElements = c_arraySize(c_enumeration(o)->elements);

        TRACE (printf ("Enum\n    Members # %d\n", copyEnum.nrOfElements));

        DDS_copyCacheWrite(context->copyCache, &copyEnum, sizeof(copyEnum));
    }
    break;
    case M_STRUCTURE:
    {
        DDSCopyStruct copyStruct;
        c_long headerIndex;
        c_ulong mi;

        DDSCopyHeader_init(&copyStruct.header, DDSStruct, 0);
        copyStruct.nrOfMembers = c_arraySize(c_structure(o)->members);
        assert (c_typeActualType((c_type)o)->size == (unsigned) c_typeActualType((c_type)o)->size);
        copyStruct.size = (unsigned) c_typeActualType((c_type)o)->size;
        copyStruct.userSize = DDS_cacheObjectUserSize((c_type)o);

        TRACE (printf("Struct userSize = %u\n", copyStruct.userSize));

        headerIndex = DDS_copyCacheWrite (context->copyCache, &copyStruct, sizeof(DDSCopyStruct));
        c_iterInsert (context->typeStack, DDS_typeHistoryNew(c_metaObject(o), headerIndex));

        TRACE (printf ("Struct\n"));

        for (mi = 0; mi < copyStruct.nrOfMembers; mi++) {
            DDS_cacheStructMember (c_structureMember(o,mi), context);
        }
        os_free(c_iterTakeFirst (context->typeStack));
        DDS_copyCacheUpdateSize (context->copyCache, headerIndex);
    }
    break;
    case M_TYPEDEF:
            DDS_metaObjectBuild (c_typeDef(o)->alias, context);
    break;
    case M_UNION:
    {
        DDSCopyUnion copyUnion;
        c_long headerIndex;
        c_ulong mi;

        DDSCopyHeader_init (&copyUnion.header, DDSUnion, 0);
        copyUnion.nrOfCases = c_arraySize(c_union(o)->cases);
        copyUnion.discrType = c_typeActualType(c_union(o)->switchType);
        if (copyUnion.discrType->size > c_type(o)->alignment) {
            assert (copyUnion.discrType->size == (unsigned) copyUnion.discrType->size);
            copyUnion.casesOffset = (unsigned) copyUnion.discrType->size;
        } else {
            assert (c_type(o)->alignment == (unsigned) c_type(o)->alignment);
            copyUnion.casesOffset = (unsigned) c_type(o)->alignment;
        }
        assert (c_typeActualType(c_type(o))->size == (unsigned) c_typeActualType(c_type(o))->size);
        copyUnion.size     = (unsigned) c_typeActualType(c_type(o))->size;
        copyUnion.userSize = DDS_cacheObjectUserSize((c_type)o);

        TRACE (printf("Union userSize = %u\n", copyUnion.userSize));

        headerIndex = DDS_copyCacheWrite (context->copyCache, &copyUnion, sizeof(DDSCopyUnion));

        DDS_metaObjectBuild (c_typeActualType(c_union(o)->switchType), context);

        c_iterInsert(context->typeStack, DDS_typeHistoryNew(c_metaObject(o), headerIndex));
        TRACE (printf ("Union\n"));

        for (mi = 0; mi < copyUnion.nrOfCases; mi++) {
            c_unionCase ucase;
            DDSCopyUnionLabels labels;
            c_ulong l;

            ucase = c_unionUnionCase(o, mi);
            labels.labelCount = c_arraySize (ucase->labels);
            DDS_copyCacheWrite(context->copyCache, &labels, sizeof(labels));

            TRACE (printf ("    labels count = %d\n", labels.labelCount));

            for (l = 0; l < labels.labelCount; l++) {
                DDS_cacheUnionLabel(c_literal(ucase->labels[l]), context);
            }
            DDS_metaObjectBuild (c_unionCaseType(ucase), context);
        }

        os_free(c_iterTakeFirst(context->typeStack));
        DDS_copyCacheUpdateSize(context->copyCache, headerIndex);
    }
    break;
    default:
        SAC_REPORT(DDS_RETCODE_ERROR, "Illegal object type (%d) detected.",
                    c_baseObjectKind(o));
        assert (FALSE);
    break;
    }
}

static void
DDS_cacheStructMember (
    c_member o,
    DDS_context ctx)
{
    DDSCopyStructMember member;

    assert (o->offset == (unsigned)o->offset);
    member.memberOffset = (unsigned)o->offset;
    TRACE (printf ("    Struct Member @ %d\n", member.memberOffset));

    DDS_copyCacheWrite (ctx->copyCache, &member, sizeof(member));
    DDS_metaObjectBuild (c_specifier(o)->type, ctx);
}

static void
DDS_cacheUnionLabel (
    c_literal lit,
    DDS_context ctx)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDSCopyUnionLabel labelVal;

    TRACE (char llstr[36];)

    switch (lit->value.kind) {
    case V_BOOLEAN:
        labelVal.labelVal = lit->value.is.Boolean;
    break;
    case V_SHORT:
        labelVal.labelVal = lit->value.is.Short;
    break;
    case V_LONG:
        labelVal.labelVal = lit->value.is.Long;
    break;
    case V_LONGLONG:
        labelVal.labelVal = lit->value.is.LongLong;
    break;
    case V_USHORT:
        labelVal.labelVal = lit->value.is.UShort;
    break;
    case V_ULONG:
        labelVal.labelVal = lit->value.is.ULong;
    break;
    case V_ULONGLONG:
        labelVal.labelVal = lit->value.is.ULongLong;
    break;
    case V_CHAR:
        labelVal.labelVal = lit->value.is.Char;
    break;
    default:
        result = DDS_RETCODE_ERROR;
        SAC_REPORT(result, "Illegal value kind (%d) detected.",
                   lit->value.kind);
        assert(FALSE);
    break;
    }

    if (result == DDS_RETCODE_OK) {
        TRACE (llstr[35] = '\0'; printf ("    labels value = %s\n", os_lltostr(labelVal.labelVal, &llstr[35])));
        DDS_copyCacheWrite (ctx->copyCache, &labelVal, sizeof(labelVal));
    }
}

